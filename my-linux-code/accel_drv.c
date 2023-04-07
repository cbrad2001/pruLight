#include "include/accel_drv.h"
#include "include/i2c_drv.h"
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

// GREEN ()
// - Initial version
#define ZEN_VERSION_GREEN 1000 // 1.0

// RED (2022) 
// - Change FTDI chip from FT230xs to FT231XS
// 	 Adds more (unused) handshaking; no change needed
// - Change GPIO expander for 14-seg display from MCP23017-E/ML to SX1503
// 	 New I2C address 0x20 (same as old)
// 	 Has auto-increment capability
// 	 May make the 14-seg display dimmer
// - Change accelerometor from MMA8452Q to LIS331DLH
#define ZEN_VERSION_RED 1100 // 1.1, or 1.11

// Current Version
#define ZEN_VERSION ZEN_VERSION_GREEN



// HW Specific Values
#if (ZEN_VERSION == ZEN_VERSION_GREEN)
	// For MMA8452Q (Rev 1; GREEN)
	#define I2C_ADDRESS 0x1C
	static i2c_device_t accel_device = {I2C_HW_BUS_1, I2C_ADDRESS, -1};

	#define SIZE_TO_READ 10
	#define REG_READ_START 0x00
	#define REG_XMSB_OFFSET 0x01
	#define REG_XLSB_OFFSET 0x02
	#define REG_YMSB_OFFSET 0x03
	#define REG_YLSB_OFFSET 0x04
	#define REG_ZMSB_OFFSET 0x05
	#define REG_ZLSB_OFFSET 0x06

	#define REG_WHOAMI 0x0D

	#define REG_ENABLE 0x2A
	#define VAL_ENABLE 0x01


#elif (ZEN_VERSION == ZEN_VERSION_RED)
	// For LIS331DLH (Rev 1.1; RED)
	#define I2C_ADDRESS 0x18
	static i2c_device_t accel_device = {I2C_HW_BUS_1, I2C_ADDRESS, -1};

	#define REG_MAKS_AUTO_INC 0x80

	#define SIZE_TO_READ 6
	#define REG_READ_START (0x28 | REG_MAKS_AUTO_INC)
	#define REG_XLSB_OFFSET 0x00
	#define REG_XMSB_OFFSET 0x01
	#define REG_YLSB_OFFSET 0x02
	#define REG_YMSB_OFFSET 0x03
	#define REG_ZLSB_OFFSET 0x04
	#define REG_ZMSB_OFFSET 0x05

	#define REG_WHOAMI 0x0F
	#define WHOAMI_EXPECTED 0x32

	#define REG_ENABLE 0x20
	// 0x20: Normal mode
	// 0x07: Enable Z,Y,X
	#define VAL_ENABLE 0x27
#else
	#error "Unknown Zen Cape Version."
#endif



// 16 bit value (12 bits real), max scale 2G, signed.
#define MAX_ACCEL_READ ((float)((1 << 15) - 1)/2)

#define NUM_BLOCKS_PER_SIDE 5


void AccelDrv_init()
{
	printf("Beginning Accelerometer Initialization...\n");
	I2cDrv_init(&accel_device);
	I2cDrv_write_register(&accel_device, REG_ENABLE, VAL_ENABLE);
	printf("Accelerometer 'who am i': 0x%02x\n", I2cDrv_read_register(&accel_device, REG_WHOAMI));

	unsigned char buff[SIZE_TO_READ];
	int bytes_read = I2cDrv_read_registers(&accel_device, REG_READ_START, buff, SIZE_TO_READ);
	printf("Accelerometer: Read %d bytes (expecting up to %d)\n", bytes_read, SIZE_TO_READ);
	printf("  Data: ");
	for (int i = 0; i < SIZE_TO_READ; i++) {
		printf("0x%02x ", buff[i]);
	}
	printf("\n");
	printf("Done Accelerometer Initialization.\n");
}

void AccelDrv_cleanup()
{
	I2cDrv_cleanup(&accel_device);
}

void AccelDrv_getReading(double *dx, double *dy, double *dz)
{
	unsigned char buff[SIZE_TO_READ];
	int bytes_read = I2cDrv_read_registers(&accel_device, REG_READ_START, buff, SIZE_TO_READ);
	assert(bytes_read == SIZE_TO_READ);

	int16_t x = (buff[REG_XMSB_OFFSET] << 8) | (buff[REG_XLSB_OFFSET]);
	int16_t y = (buff[REG_YMSB_OFFSET] << 8) | (buff[REG_YLSB_OFFSET]);
	int16_t z = (buff[REG_ZMSB_OFFSET] << 8) | (buff[REG_ZLSB_OFFSET]);

	*dx = x / MAX_ACCEL_READ;
	*dy = y / MAX_ACCEL_READ;
	*dz = z / MAX_ACCEL_READ;
}