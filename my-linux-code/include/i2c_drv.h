// I2C driver module
// Create a i2c_device_t struct for the device you want to access,
// Then use the functions to initialize it, use it, and cleanup.
#ifndef I2C_DRV_H
#define I2C_DRV_H

enum i2c_hw_bus {
	I2C_HW_BUS_0,
	I2C_HW_BUS_1,
	I2C_HW_BUS_2
};

typedef struct {
	enum i2c_hw_bus hw_bus;
	int address;

	int file_desc;
} i2c_device_t;


void I2cDrv_init(i2c_device_t *pdevice);
void I2cDrv_cleanup(i2c_device_t *pdevice);

unsigned char I2cDrv_read_register(i2c_device_t *pdevice, unsigned char reg_address);
int I2cDrv_read_registers(i2c_device_t *pdevice, unsigned char reg_address, unsigned char buff[], int size);
void I2cDrv_write_register(i2c_device_t *pdevice, unsigned char reg_address, unsigned char value);
void I2cDrv_write_register_check_retry(i2c_device_t *pdevice, unsigned char reg_address, unsigned char value, int retry);

#endif
