// Accelerometer module for reading accelerations from the Zen cape.
#ifndef _ACCEL_DRV_H_
#define _ACCEL_DRV_H_

// Start / Stop the accelerometer running thread
void AccelDrv_init();
void AccelDrv_cleanup();

// obtains the current value of the accelerometer 
void AccelDrv_getReading(double *dx, double *dy, double *dz);

#endif