// analogDisplay.h
// Module to control the i2c 14 segment displays based on the dips
// 
#ifndef _ANALOG_H_
#define _ANALOG_H_

//stop looping the thread for i2c display
void Analog_quit();

// Begin/end the background thread which displays the # of dips on the I2C display
void Analog_startDisplaying(void);
void Analog_stopDisplaying(void);

// Updates the 14 seg display to output the passed in number.
void Analog_updateDisplay(int numToDisplay);

#endif