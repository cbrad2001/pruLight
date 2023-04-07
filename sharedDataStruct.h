#ifndef _SHARED_DATA_STRUCT_H_
#define _SHARED_DATA_STRUCT_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdalign.h>

// WARNING:
// Fields in the struct must be aligned to match ARM's alignment
//    bool/char, uint8_t:   byte aligned
//    int/long,  uint32_t:  word (4 byte) aligned
//    double,    uint64_t:  dword (8 byte) aligned
// Add padding fields (char _p1) to pad out to alignment.

// My Shared Memory Structure
// ----------------------------------------------------------------
#define NUM_LEDS 8
#define LED_OFF 0
#define LED_ON 1

typedef struct {
    // 8 LEDs on the Neopixel strip. 1 for on, 0 for off.
    bool led[NUM_LEDS];   

    // Only down and right are used for the game
    bool jsDownPressed;     // "Fire" action
    bool jsRightPressed;    // Exits the game

    // Must be dword aligned (multiple of 8)
    alignas(8) uint64_t numMsSinceBigBang;             // Works!
    // _Alignas(uint64_t) uint64_t numMsSinceBigBang;      // Fails!
    
} sharedMemStruct_t;

#endif
