#include <stdint.h>
#include <stdbool.h>
#include <pru_cfg.h>
#include "resource_table_empty.h"
#include "../my-linux-code/include/sharedDataStruct.h"

// Reference for shared RAM:
// https://markayoder.github.io/PRUCookbook/05blocks/blocks.html#_controlling_the_pwm_frequency

/*
    1.  Set both GPIO pins to be controlled by PRU0
            config-pin p8_12 pruout
            config-pin p8_15 pruin

    Display something on 14-seg display:
    2a. Enable I2C:
            config-pin P9_18 i2c
            config-pin P9_17 i2c

    2b. Enable I2C chip & set pattern; Pick correct board
        ZEN CAPE GREEN:
            i2cset -y 1 0x20 0x00 0x00
            i2cset -y 1 0x20 0x01 0x00
            i2cset -y 1 0x20 0x14 0x1E
            i2cset -y 1 0x20 0x15 0x78
        ZEN CAPE RED
            i2cset -y 1 0x20 0x02 0x00
            i2cset -y 1 0x20 0x03 0x00
            i2cset -y 1 0x20 0x00 0x0f
            i2cset -y 1 0x20 0x01 0x5e
*/

#define STR_LEN         8       // # LEDs in our string
#define oneCyclesOn     700/5   // Stay on 700ns
#define oneCyclesOff    800/5
#define zeroCyclesOn    350/5
#define zeroCyclesOff   600/5
#define resetCycles     60000/5 // Must be at least 50u, use 60u

#define DATA_PIN 15       // Bit number to output on

// GPIO Configuration
// ----------------------------------------------------------
volatile register uint32_t __R30;   // output GPIO register
volatile register uint32_t __R31;   // input GPIO register

// GPIO Output: P8_12 = pru0_pru_r30_14 
//   = LEDDP2 (Turn on/off right 14-seg digit) on Zen cape
#define DIGIT_ON_OFF_MASK (1 << 14)
// GPIO Input: P8_15 = pru0_pru_r31_15 
//   = JSRT (Joystick Right) on Zen Cape
//   (Happens to be bit 15 and p8_15; just a coincidence; see P8 header info sheet)
#define JOYSTICK_RIGHT_MASK (1 << 15)

// GPIO Input P8_16 = pru0_pru_r31_14
//   = JSDN (Joystick Down) on Zen Cape
// #define JOYSTICK_DOWN_MASK (1 << 14)


// Shared Memory Configuration
// -----------------------------------------------------------
#define THIS_PRU_DRAM       0x00000         // Address of DRAM
#define OFFSET              0x200           // Skip 0x100 for Stack, 0x100 for Heap (from makefile)
#define THIS_PRU_DRAM_USABLE (THIS_PRU_DRAM + OFFSET)





// This works for both PRU0 and PRU1 as both map their own memory to 0x0000000
volatile sharedMemStruct_t *pSharedMemStruct = (volatile void *)THIS_PRU_DRAM_USABLE;


void lightLED(void){
    for(int j = 0; j < STR_LEN; j++) {
        for(int i = 31; i >= 0; i--) {
            if(pSharedMemStruct->ledColor[j] & ((uint32_t)0x1 << i)) {
                __R30 |= 0x1<<DATA_PIN;      // Set the GPIO pin to 1
                __delay_cycles(oneCyclesOn-1);
                __R30 &= ~(0x1<<DATA_PIN);   // Clear the GPIO pin
                __delay_cycles(oneCyclesOff-2);
            } else {
                __R30 |= 0x1<<DATA_PIN;      // Set the GPIO pin to 1
                __delay_cycles(zeroCyclesOn-1);
                __R30 &= ~(0x1<<DATA_PIN);   // Clear the GPIO pin
                __delay_cycles(zeroCyclesOff-2);
            }
        }
    }

     __R30 &= ~(0x1<<DATA_PIN);   // Clear the GPIO pin
    __delay_cycles(resetCycles);

    __halt();
}

void main(void)
{
    // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

    // COLOURS
    // - 1st element in array is 1st (bottom) on LED strip; last element is last on strip (top)
    // - Bits: {Green/8 bits} {Red/8 bits} {Blue/8 bits} {White/8 bits}
    // uint32_t color[STR_LEN] = {
    //     // 0x0f000000, // Green
    //     // 0x000f0000, // Red
    //     // 0x00000f00, // Blue
    //     // 0x0000000f, // White
    //     // 0x0f0f0f00, // White (via RGB)
    //     // 0x0f0f0000, // Yellow
    //     // 0x000f0f00, // Purple
    //     // 0x0f000f00, // Teal

    //     // Try these; they are birght! 
    //     // (You'll need to comment out some of the above)
    //     0xff000000, // Green Bright
    //     0x00ff0000, // Red Bright
    //     0x0000ff00, // Blue Bright
    //     0xffffff00, // White
    //     0xff0000ff, // Green Bright w/ Bright White
    //     0x00ff00ff, // Red Bright w/ Bright White
    //     0x0000ffff, // Blue Bright w/ Bright White
    //     0xffffffff, // White w/ Bright White
    // }; 
    
    // Initialize:
    for (int i = 0; i < STR_LEN; i++){
        pSharedMemStruct->ledColor[i] = 0x000f0000;
    }
    pSharedMemStruct->isLedOn = true;
    pSharedMemStruct->jsDownPressed = false;
    pSharedMemStruct->jsRightPressed = false;

    
    // pSharedMemStruct->smileCount = 0x5566;
    pSharedMemStruct->numMsSinceBigBang = 0x0000111122223333;

    

    while (true) { 

        lightLED();


        // // Drive LED from shared memory
        if (pSharedMemStruct->isLedOn) {
            __R30 |= DIGIT_ON_OFF_MASK;
        } else {
            __R30 &= ~DIGIT_ON_OFF_MASK;
        }

        // Sample button state to shared memory
        pSharedMemStruct->jsRightPressed = (__R31 & JOYSTICK_RIGHT_MASK) != 0;
        // pSharedMemStruct->jsDownPressed = (__R31 & JOYSTICK_DOWN_MASK) != 0;
    }
}
