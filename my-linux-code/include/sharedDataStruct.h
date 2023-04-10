#ifndef _SHARED_DATA_STRUCT_H_
#define _SHARED_DATA_STRUCT_H_

#include <stdbool.h>
#include <stdint.h>

#define NUM_LEDS 8
#define LED_OFF 0
#define LED_ON 1

// WARNING:
// Fields in the struct must be aligned to match ARM's alignment
//    bool/char, uint8_t:   byte aligned
//    int/long,  uint32_t:  word (4 byte) aligned
//    double,    uint64_t:  dword (8 byte) aligned
// Add padding fields (char _p1) to pad out to alignment.

// My Shared Memory Structure
// ----------------------------------------------------------------
typedef struct {
    bool jsDownPressed;     // "Fire" action    each 1B
    bool jsRightPressed;    // Exits the game

    _Alignas(4) uint32_t ledColor[NUM_LEDS];

    // Must be dword aligned (multiple of 8)
    // _Alignas(8) uint64_t numMsSinceBigBang;                // Works! (not necessary, uncomment for testing)    
} sharedMemStruct_t;

#endif
