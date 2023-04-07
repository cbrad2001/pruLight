#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>

#include "include/pru_code.h"

static volatile sharedMemStruct_t *pSharedPru0;
static volatile void *pPruBase;

// Return the address of the PRU's base memory
static volatile void* getPruMmapAddr(void)
{
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        perror("ERROR: could not open /dev/mem");
        exit(EXIT_FAILURE);
    }

    // Points to start of PRU memory.
    volatile void* pPruBase2 = mmap(0, PRU_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PRU_ADDR);
    if (pPruBase == MAP_FAILED) {
        perror("ERROR: could not map memory");
        exit(EXIT_FAILURE);
    }
    close(fd);

    return pPruBase2;
}

static void freePruMmapAddr(volatile void* pPruBase2)
{
    if (munmap((void*) pPruBase2, PRU_LEN)) {
        perror("PRU munmap failed");
        exit(EXIT_FAILURE);
    }
}

volatile sharedMemStruct_t* PRU_getMapping(){
    return pSharedPru0;
}

void PRU_init(){
    // Get access to shared memory for my uses
    pPruBase = getPruMmapAddr();
    pSharedPru0 = PRU0_MEM_FROM_BASE(pPruBase);
}

void PRU_free(){
    freePruMmapAddr(pPruBase);
}