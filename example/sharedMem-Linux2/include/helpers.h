// Global helper functions... so that there is no module calling overlap for generic functions
// Time support
#ifndef HELPERS_H
#define HELPERS_H

#define SEC_PER_MIN 60
#define MS_PER_SEC 1000

// sleep program execution for a given number of ms
void sleepForMs(long long delayInMs);

// run a console command
void runCommand(char* command); 

#endif