#include "include/accelerometer.h"
#include "include/helpers.h"
#include "include/drumBeats.h"
#include "include/audioMixer.h"
#include "include/periodTimer.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define I2CDRV_LINUX_BUS "/dev/i2c-1" // this is the i2c bus for the accelerometer
#define ACCEL_12C_ADDR ((unsigned char)0x1C)// this is the address of the accelerometer on the i2c bus
#define ACCEL_CTRL_REG ((unsigned char)0x2A) // this is the register to set to enable the accelerometer
#define DEBOUNCE_MS 100
#define FIRST_BYTE_READ_ADDR ((unsigned char)0x00)

// definitions for MSB threshold values
// these are values gathered after a short test, so adjust accordingly
#define X_POS_THRESHOLD ((unsigned char)0x85)
#define X_NEG_THRESHOLD ((unsigned char)0x45)
#define Y_POS_THRESHOLD ((unsigned char)0x60)
#define Y_NEG_THRESHOLD ((unsigned char)0x90)
#define Z_POS_THRESHOLD ((unsigned char)0x80) // under 1G of gravity MSB reads "40"
#define Z_NEG_THRESHOLD ((unsigned char)0x10)

static void* accelThread(void *vargp);

static int initI2cBus(char *bus, int address);
static unsigned char* readMsbValues(int i2cFileDesc, unsigned char startRegAddr);
static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);

static pthread_t accelThreadID;
static bool isRunning;

void Accel_start(void)
{
    runCommand("config-pin P9_17 i2c");
    runCommand("config-pin P9_18 i2c");
    sleepForMs(350);
    isRunning = true;
    pthread_create(&accelThreadID, NULL, accelThread, NULL);
}

void Accel_stop(void)
{
    isRunning = false;
    pthread_join(accelThreadID, NULL);
    runCommand("config-pin P9_17 default");
    runCommand("config-pin P9_18 default");
}

static void* accelThread(void *vargp)
{
    wavedata_t *drumKit = AudioMixer_getDrumkit();  //gathers the previously defined sounds from the drum modules

    int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS, ACCEL_12C_ADDR);
    
    // runCommand("i2cset -y 1 0x1C 0x2A 1");
    writeI2cReg(i2cFileDesc, ACCEL_CTRL_REG, 1);

    printf("Starting accelerometer listener thread!\n");
    while (isRunning)
    {
        Period_markEvent(PERIOD_EVENT_ACCELEROMETER);//specific timer vs the joystick to time each runthru 
        // printf("DEBUG: reading the msb values...\n");
        unsigned char *msbValues = readMsbValues(i2cFileDesc, FIRST_BYTE_READ_ADDR); 
        // printf("DEBUG: read the msb values...\n");
        unsigned char xMsbVal = msbValues[0];
        unsigned char yMsbVal = msbValues[1];
        unsigned char zMsbVal = msbValues[2];

        if (xMsbVal == X_POS_THRESHOLD || xMsbVal == X_NEG_THRESHOLD)   // HI-HAT
        {
            AudioMixer_queueSound(&drumKit[0]);
            sleepForMs(DEBOUNCE_MS);
        }

        if (yMsbVal == Y_POS_THRESHOLD || yMsbVal == Y_NEG_THRESHOLD)   // BASE
        {
            AudioMixer_queueSound(&drumKit[1]);
            sleepForMs(DEBOUNCE_MS);
        }

        if (zMsbVal == Z_POS_THRESHOLD || zMsbVal == Z_NEG_THRESHOLD)   // SNARE
        {
            AudioMixer_queueSound(&drumKit[2]);
            sleepForMs(DEBOUNCE_MS);
        }
        free(msbValues);
        // sleepForMs(10);     // "reasonable poll of input every 10ms  - asn description"
        // seems to become significantly harder to receive drum sounds when this is here
    }

    close(i2cFileDesc);
    return 0;
}

// provided by I2C guide
static int initI2cBus(char *bus, int address)
{
    int i2cFileDesc = open(bus, O_RDWR);
    int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
    if (result < 0)
    {
        perror("Unable to set the I2C device to slave address");
        exit(-1);
    }
    return i2cFileDesc;
}

// provided by I2C guide
static unsigned char* readMsbValues(int i2cFileDesc, unsigned char startRegAddr)
{
    // To read a register, must first write the address
    int res = write(i2cFileDesc, &startRegAddr, sizeof(startRegAddr));
    if (res != sizeof(startRegAddr))
    {
        perror("I2C: Unable to write to i2c register for reading.");
        exit(1);
    }

    // Now read the value and return it
    unsigned char buff[7];
    res = read(i2cFileDesc, &buff, 7);
    if (res != sizeof(buff))
    {
        perror("I2C: Unable to read from i2c register");
        exit(1);
    }
    // printf("DEBUG: read the register at %x\n", startRegAddr);
    
    unsigned char *msbBuff = malloc(sizeof(unsigned char)*3);
    // TODO: extract msb for X, Y, Z and return the buffer

    // zeroth byte is the garbage byte, first byte is the X MSB
    // second byte is the X LSB, third byte is the Y MSB
    // fourth byte is the Y LSB, fifth byte is the Z MSB

    // unsigned char xMsb = buff[1];
    // unsigned char yMsb = buff[3];
    // unsigned char zMsb = buff[5];

    msbBuff[0] = buff[1];
    msbBuff[1] = buff[3];
    msbBuff[2] = buff[5];

    return msbBuff;
}

// provided by I2C guide
static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value)
{
    unsigned char buff[2];
	buff[0] = regAddr;
	buff[1] = value;
	int res = write(i2cFileDesc, buff, 2);
	if (res != 2) 
	{
		perror("Unable to write i2c register");
		exit(-1);
	}
}