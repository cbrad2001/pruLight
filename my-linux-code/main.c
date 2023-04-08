#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>

#include "include/pru_code.h"
#include "include/accel_drv.h"
#include "include/game.h"

volatile sharedMemStruct_t *pSharedPru0;

int main(void)
{
    printf("Sharing memory with PRU\n");

    PRU_init();
    pSharedPru0 = PRU_getMapping(); // Get access to shared memory for my uses
            // uint32_t ledColor[NUM_LEDS];
            // bool isLedOn;
            // bool jsDownPressed;    
            // bool jsRightPressed;    
            // char _p0;
            // // char _p1, _p2, _p3;
            // // Must be dword aligned (multiple of 8)
            // _Alignas(8) uint64_t numMsSinceBigBang;         
    Game_start();

    // Print out the mem contents:
    printf("From the PRU, memory hold:\n");

    printf("    %15s: 0x%02x\n", "isDownPressed", pSharedPru0->jsDownPressed);
    printf("    %15s: 0x%02x\n", "isRightPressed", pSharedPru0->jsRightPressed);

    printf("    %15s: 0x%02x\n", "hex0", pSharedPru0->ledColor[0]);
    printf("    %15s: 0x%02x\n", "hex1", pSharedPru0->ledColor[1]);
    printf("    %15s: 0x%02x\n", "hex2", pSharedPru0->ledColor[2]);
    printf("    %15s: 0x%02x\n", "hex3", pSharedPru0->ledColor[3]);
    printf("    %15s: 0x%02x\n", "hex4", pSharedPru0->ledColor[4]);
    printf("    %15s: 0x%02x\n", "hex5", pSharedPru0->ledColor[5]);
    printf("    %15s: 0x%02x\n", "hex6", pSharedPru0->ledColor[6]);
    printf("    %15s: 0x%02x\n", "hex7", pSharedPru0->ledColor[7]);

    printf("    %15s: 0x%016llx\n", "numMs", pSharedPru0->numMsSinceBigBang);

    



    Game_end();
    PRU_free();

    // // Drive it
    // for (int i = 0; i < 20; i++) {
    //     // Drive LED
    //     pSharedPru0->isLedOn = (i % 2 == 0);

        
    //     // printf("    %15s: 0x%02x\n", "Accel x coord:", *pSharedPru0->x_val);
    //     // printf("    %15s: 0x%02x\n", "Accel y coord:", *pSharedPru0->y_val);
    //     // printf("    %15s: 0x%02x\n", "Accel z coord:", *pSharedPru0->z_val);

    //     // Print button
    //     printf("Button: %d\n", 
    //         pSharedPru0->jsRightPressed);

    //     // Timing
    //     sleep(1);
    // }

    // Cleanup
    
}