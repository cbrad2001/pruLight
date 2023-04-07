#include "include/game.h"
#include "include/helpers.h"
#include "include/accelerometer.h"
#include "include/sharedDataStruct.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

// General PRU Memomry Sharing Routine
// ----------------------------------------------------------------
#define PRU_ADDR      0x4A300000   // Start of PRU memory Page 184 am335x TRM
#define PRU_LEN       0x80000      // Length of PRU memory
#define PRU0_DRAM     0x00000      // Offset to DRAM
#define PRU1_DRAM     0x02000
#define PRU_SHAREDMEM 0x10000      // Offset to shared memory
#define PRU_MEM_RESERVED 0x200     // Amount used by stack and heap

// Convert base address to each memory section
#define PRU0_MEM_FROM_BASE(base) ( (base) + PRU0_DRAM + PRU_MEM_RESERVED)
#define PRU1_MEM_FROM_BASE(base) ( (base) + PRU1_DRAM + PRU_MEM_RESERVED)
#define PRUSHARED_MEM_FROM_BASE(base) ( (base) + PRU_SHAREDMEM)

static bool isRunning;
static pthread_t gameThreadId, joystickListenerId;
static sem_t mutex;
static volatile sharedMemStruct_t *pSharedPru0;

static void* gameThread(void *vargp);
static void* joystickListener(void *vargp);

// Return the address of the PRU's base memory
volatile void* getPruMmapAddr(void)
{
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        perror("ERROR: could not open /dev/mem");
        exit(EXIT_FAILURE);
    }

    // Points to start of PRU memory.
    volatile void* pPruBase = mmap(0, PRU_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PRU_ADDR);
    if (pPruBase == MAP_FAILED) {
        perror("ERROR: could not map memory");
        exit(EXIT_FAILURE);
    }
    close(fd);

    return pPruBase;
}

void freePruMmapAddr(volatile void* pPruBase)
{
    if (munmap((void*) pPruBase, PRU_LEN)) {
        perror("PRU munmap failed");
        exit(EXIT_FAILURE);
    }
}


void Game_start(void)
{
    isRunning = true;
    sem_init(&mutex, 0, 0);
    pthread_create(&gameThreadId, NULL, gameThread, NULL);
    pthread_create(&joystickListenerId, NULL, joystickListener, NULL);
}

void Game_end(void)
{
    isRunning = false;
    pthread_join(gameThreadId, NULL);
    pthread_join(joystickListenerId, NULL);
    sem_destroy(&mutex);
}

void Game_wait(void)
{
    sem_wait(&mutex);
}

static void* gameThread(void *vargp)
{
    // Get access to shared memory for my uses
    volatile void *pPruBase = getPruMmapAddr();
    pSharedPru0 = PRU0_MEM_FROM_BASE(pPruBase);

    char initX, initY, initZ;
    Accel_getInitialReading(&initX, &initY, &initZ);

    while (isRunning) {
        // TODO: implement accelerometer logic
    }
}

static void* joystickListener(void *vargp)
{
    while (isRunning) {
        if (pSharedPru0->jsRightPressed) {
            pSharedPru0->jsRightPressed = false;
            Game_end();
        }
        sleepForMs(5);
    }
}
