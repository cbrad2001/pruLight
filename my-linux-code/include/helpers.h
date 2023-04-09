// Global helper functions... so that there is no module calling overlap for generic functions
// Time support
#ifndef HELPERS_H
#define HELPERS_H
// LED VALUES
#define GREEN 0x0f000000
#define GREEN_BR 0xff000000
#define RED 0x000f0000
#define RED_BR 0x00ff0000
#define BLUE 0x00000f00
#define BLUE_BR 0x0000ff00
#define NONE 0x00000000

#define SEC_PER_MIN 60
#define MS_PER_SEC 1000

// sleep program execution for a given number of ms
void sleepForMs(long long delayInMs);

// run a console command
void runCommand(char* command); 

#endif