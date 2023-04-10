#include "include/analogDisplay.h"
#include "include/helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h> 
#include <linux/i2c.h> 
#include <linux/i2c-dev.h>

#define GPIO_EXPORT_FILE "/sys/class/gpio/export"

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0" 
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1" 
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"

#define BUS1_ADDRESS 0x20

#define first_dir  "/sys/class/gpio/gpio61/direction"    //set out
#define second_dir "/sys/class/gpio/gpio44/direction"   //set out

#define first_val "/sys/class/gpio/gpio61/value"    	//set #
#define second_val "/sys/class/gpio/gpio44/value"   	//set #
    // we both have green capes
#define REG_DIRA 0x00
#define REG_DIRB 0x01
#define REG_OUTA 0x14
#define REG_OUTB 0x15

static void* dipHistoryToDisplay(void *vargp);

static pthread_t anDisplayThreadID;
static bool isDisplaying;
static int currentNumber;
// static bool isDisplayActive;

/**
 * ===========================
 * Provided code by I2C Guide:
 * ===========================
*/

static void editReading(char* fileName, char* val)
{
    FILE* pfile = fopen(fileName, "w");
	if (pfile == NULL) 
	{
		printf("ERROR: Unable to open file.\n");
		exit(1);
	}
	fprintf(pfile, "%s", val);          //set the file's contents to the value (on/off for seg display)
	fclose(pfile);
}

// provided code by I2C guide
static int initI2cBus(char* bus, int address)
{
	int i2cFileDesc = open(bus, O_RDWR);
	int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
	if (result < 0) 
	{
		perror("Unable to set I2C device to slave address.");
		exit(-1);
	}
	return i2cFileDesc;
}
// provided code by I2C guide
static void writeI2cReg(int i2cFileDescr, unsigned char regAddr, unsigned char value)
{
	unsigned char buff[2];
	buff[0] = regAddr;
	buff[1] = value;
	int res = write(i2cFileDescr, buff, 2);
	if (res != 2) 
	{
		perror("Unable to write i2c register");
		exit(-1);
	}
}
// SETUP:

// complete the steps from the I2C guide (2.3) to config the board to read values 
// configures the pin, creates the file descriptor and sets the register values
static int init_display()
{
	runCommand("config-pin P9_18 i2c");         //config pins
	runCommand("config-pin P9_17 i2c");
	sleepForMs(350);

	DIR *gpio61Dir = opendir("/sys/class/gpio/gpio61/");
	if (ENOENT == errno)
	{
		editReading(GPIO_EXPORT_FILE,"61");
	}
	DIR *gpio44Dir = opendir("/sys/class/gpio/gpio44/");
	if (ENOENT == errno) 
	{
		editReading(GPIO_EXPORT_FILE,"44");
	}
	closedir(gpio61Dir);
	closedir(gpio44Dir);
	gpio61Dir = NULL;
	gpio44Dir = NULL;

    editReading(first_dir,"out");           //set direction to output
    editReading(second_dir,"out");
	editReading(first_val,"0");             //set value to on
    editReading(second_val,"0");

	int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, BUS1_ADDRESS);

	writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
	writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);

	writeI2cReg(i2cFileDesc, REG_OUTA, 0x00);
	writeI2cReg(i2cFileDesc, REG_OUTB, 0x00);
    
    return i2cFileDesc;
}

/**
 * Main running code
*/
void Analog_quit()
{
    isDisplaying = false;
}

void Analog_startDisplaying(void)
{
	isDisplaying = true;
	currentNumber = 0;
    pthread_create(&anDisplayThreadID, NULL, &dipHistoryToDisplay, NULL);
}

void Analog_stopDisplaying(void)
{
	printf("Shutting down analog display thread!\n");
    isDisplaying = false;
    pthread_join(anDisplayThreadID, NULL);
}

void Analog_updateDisplay(int numToDisplay)
{
	currentNumber = numToDisplay;
}

//lower 8 bits associated with register 0x14.... 
//to be called with REG_OUTA
static int lowerRegisterVal_Hex(int val)
{
    switch (val){
		case 0:
			return 0xA1;
		case 1:
			return 0x80;
		case 2:
			return 0x31;
		case 3:
			return 0xB0;
		case 4:
			return 0x90;
		case 5:
			return 0xB0;
		case 6:
			return 0xB1;
		case 7:
			return 0x80;
		case 8:
			return 0xB1;
		case 9:
			return 0x90;
		default:
			return 0xA1;	
	}
}

//top 8 bits associated with register 0x15.... 
//to be called with REG_OUTB
static int upperRegisterVal_Hex(int val)
{
    switch (val){
		case 0:
			return 0x86;
		case 1:
			return 0x02;
		case 2:
			return 0x0E;
		case 3:
			return 0x0E;
		case 4:
			return 0x8A;
		case 5:
			return 0x8C;
		case 6:
			return 0x8C;
		case 7:	
			return 0x06;
		case 8:
			return 0x8E;
		case 9:
			return 0x8E;
		default:
			return 0x86;
	}
}

// live report of the current dip count onto the zencape's i2c display.
// displays 0 when no dips, up to a maximum 99 
static void* dipHistoryToDisplay(void *vargp)
{
	int i2cFileDesc = init_display();
	isDisplaying = true;
    while(isDisplaying)
	{
        int num_to_display = currentNumber;
        int first_digit = num_to_display / 10;  	//moves the decimal place one to the left
        int second_digit = num_to_display % 10; 	//extracts the first num

        if (num_to_display >= 99) 					//display 01-99
		{ 
			first_digit = 9; 						// dip count over 100 rounds to 99
			second_digit= 9;
		}	
       	
		editReading(first_val,"0");             	//set value to off
		editReading(second_val,"0");				// first digit (upper and lower halfs)
        writeI2cReg(i2cFileDesc, REG_OUTA, lowerRegisterVal_Hex(first_digit));
        writeI2cReg(i2cFileDesc, REG_OUTB, upperRegisterVal_Hex(first_digit));
		editReading(first_val,"1");             	//set value to on
		sleepForMs(5);									// sleep 5 ms after turning on per guide

		editReading(first_val, "0");
		editReading(second_val,"0");
		writeI2cReg(i2cFileDesc, REG_OUTA, lowerRegisterVal_Hex(second_digit));
        writeI2cReg(i2cFileDesc, REG_OUTB, upperRegisterVal_Hex(second_digit));
		editReading(second_val,"1");
		sleepForMs(5);
    }

	editReading(first_val,"0");             // turn off readings on end
    editReading(second_val,"0");
    sleepForMs(5);

	close(i2cFileDesc);     						//cleanup i2c access
    return 0;
}