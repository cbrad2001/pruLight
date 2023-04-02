#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>
#include "include/helpers.h"

void sleepForMs(long long delayInMs) 
{    
    const long long NS_PER_MS = 1000 * 1000;    
    const long long NS_PER_SECOND = 1000000000;    
    long long delayNs = delayInMs * NS_PER_MS;    
    int seconds = delayNs / NS_PER_SECOND;    
    int nanoseconds = delayNs % NS_PER_SECOND;    
    struct timespec reqDelay = {seconds, nanoseconds};    
    nanosleep(&reqDelay, (struct timespec *) NULL); 
}

void runCommand(char* command) 
{    
    // Execute the shell command (output into pipe)    
    FILE *pipe = popen(command, "r");    
    // Ignore output of the command; but consume it     
    // so we don't get an error when closing the pipe.    
    char buffer[1024];    
    while (!feof(pipe) && !ferror(pipe)) {        
        if (fgets(buffer, sizeof(buffer), pipe) == NULL)            
            break;        
        // printf("--> %s", buffer);  
        // Uncomment for debugging    
    }    
    // Get the exit code from the pipe; non-zero is an error:    
    int exitCode = WEXITSTATUS(pclose(pipe));    
    if (exitCode != 0) {        
        perror("Unable to execute command:");        
        printf("  command:   %s\n", command);        
        printf("  exit code: %d\n", exitCode);    
    } 
}