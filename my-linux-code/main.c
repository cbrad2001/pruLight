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
#include "include/analogDisplay.h"

volatile sharedMemStruct_t *pSharedPru0;

int main(void)
{
    printf("[][][] STARTING POINTER GAME [][][]\n");

    PRU_init();
    pSharedPru0 = PRU_getMapping(); // Get access to shared memory for my uses
      
    Game_start();
    Analog_startDisplaying();

    // printf("Initial Memory Hold:\n");         // For Testing
    // printf("    %15s: 0x%02x\n", "isDownPressed", pSharedPru0->jsDownPressed);
    // printf("    %15s: 0x%02x\n", "isRightPressed", pSharedPru0->jsRightPressed);

    // printf("    %15s: 0x%02x\n", "hex0", pSharedPru0->ledColor[0]);
    // printf("    %15s: 0x%02x\n", "hex1", pSharedPru0->ledColor[1]);
    // printf("    %15s: 0x%02x\n", "hex2", pSharedPru0->ledColor[2]);
    // printf("    %15s: 0x%02x\n", "hex3", pSharedPru0->ledColor[3]);
    // printf("    %15s: 0x%02x\n", "hex4", pSharedPru0->ledColor[4]);
    // printf("    %15s: 0x%02x\n", "hex5", pSharedPru0->ledColor[5]);
    // printf("    %15s: 0x%02x\n", "hex6", pSharedPru0->ledColor[6]);
    // printf("    %15s: 0x%02x\n", "hex7", pSharedPru0->ledColor[7]);

    // printf("    %15s: 0x%016llx\n", "numMs", pSharedPru0->numMsSinceBigBang);    // (not necessary, uncomment with struct value for testing)

///
    Analog_stopDisplaying();
    Game_end();
    PRU_free();
}