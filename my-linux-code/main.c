#include <stdint.h>
#include <stdbool.h>
#include <time.h>


#include "include/accelerometer.h"


// #include <pru_cfg.h>
// #include "resource_table_empty.h"
// #include "../sharedMem-Linux/sharedDataStruct.h"

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

// Shared Memory Configuration
// -----------------------------------------------------------
#define THIS_PRU_DRAM       0x00000         // Address of DRAM
#define OFFSET              0x200           // Skip 0x100 for Stack, 0x100 for Heap (from makefile)
#define THIS_PRU_DRAM_USABLE (THIS_PRU_DRAM + OFFSET)

// This works for both PRU0 and PRU1 as both map their own memory to 0x0000000
volatile sharedMemStruct_t *pSharedMemStruct = (volatile void *)THIS_PRU_DRAM_USABLE;


#define RANGE_MIN -0.5
#define RANGE_MAX 0.5

typedef struct{
    double x,
    double y
} Points;

Points p;


void init(void)
{
     // Initialize:
    pSharedMemStruct->isLedOn = true;
    pSharedMemStruct->isButtonPressed = false;
    pSharedMemStruct->smileCount = 0x5566;
    pSharedMemStruct->numMsSinceBigBang = 0x0000111122223333;

}

double generate_points(){
    double rand_val = 0.0;
    srand(time(NULL));
    rand_val = (double)rand()/RAND_MAX*1.0-0.5;
    printf("rand val: %f", rand_val);
    return rand_val;
}

void populate_points(){
    p.x = generate_points();    //generates two random values in the range
    p.y = generate_points();
}




void main(void)
{
   
    while (true) { 

        // Drive LED from shared memory
        if (pSharedMemStruct->isLedOn) {
            __R30 |= DIGIT_ON_OFF_MASK;
        } else {
            __R30 &= ~DIGIT_ON_OFF_MASK;
        }

        // Sample button state to shared memory
        pSharedMemStruct->isButtonPressed = (__R31 & JOYSTICK_RIGHT_MASK) != 0;
    }
}
