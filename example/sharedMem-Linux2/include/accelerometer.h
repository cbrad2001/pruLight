/* accelerometer.h
*
*  This file defines the API endpoints for the module that reads from the integrated
*  accelerometer from the Zen Cape Green on the I2C bus of the BeagleBone Green.
*/

#ifndef _ACCEL_H_
#define _ACCEL_H_

// Starts and stops the bakground thread that listens to accelerometer activity.
void Accel_start(void);
void Accel_stop(void);

// Saves a reading of the directional MSB values from the accelerometer to the passed in pointers.
void Accel_getReading(unsigned char *dx, unsigned char *dy, unsigned char *dz);

// Saves a reading of the initial directional MSB values from the accelerometer to the passed in pointers.
void Accel_getInitialReading(unsigned char *dx, unsigned char *dy, unsigned char *dz);

#endif