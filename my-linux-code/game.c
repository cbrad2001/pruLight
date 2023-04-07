#include "include/game.h"
#include "include/helpers.h"
#include "include/accel_drv.h"
#include "include/pru_code.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>


static bool isRunning;
static pthread_t gameThreadId, joystickListenerId;
static sem_t mutex;
static volatile sharedMemStruct_t *pSharedPru0;

static void* gameThread(void *vargp);
static void* joystickListener(void *vargp);
static void generateXYpoint(double *toChangeX, double *toChangeY);

// Taken from https://stackoverflow.com/questions/1340729/how-do-you-generate-a-random-double-uniformly-distributed-between-0-and-1-from-c
static double randMToN(double M, double N);

void Game_start(void)
{
    AccelDrv_init();
    srand((unsigned)time(NULL));
    pSharedPru0 = PRU_getMapping(); // Get access to shared memory for my uses
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
    AccelDrv_cleanup();
}

void Game_wait(void)
{
    sem_wait(&mutex);
}

//temporary
#define GREEN 0x0f000000
#define RED 0x000f0000
#define BLUE 0x00000f00

static void populate_with(uint32_t code){
    for (int i = 0; i < 8; i++){
        pSharedPru0->ledColor[i] = code;
    }
}

static void* gameThread(void *vargp)
{
    
    double initX = 0, initY = 0, initZ = 0;

    double xPoint = 0, yPoint = 0;
    generateXYpoint(&xPoint, &yPoint);

    double HYSTERESIS = 0.1;
    isRunning = true;

    // Notes: Leaning left gives positive Y values, and right gives negative Y values
    // Tilting up gives negative X values, and down gives positive X values
    while (isRunning) {
        // TODO: implement accelerometer logic

        AccelDrv_getReading(&initX, &initY, &initZ);

        // if left (-1 to 0): red
        if (initX <= xPoint-HYSTERESIS){
            printf("RED\n");
            populate_with(RED);
        }

        //if right: green
        if (initX >= xPoint+HYSTERESIS){
            printf("GREEN\n");
            populate_with(GREEN);            
        }
    

        //if centered: blue
        if (initX > xPoint-HYSTERESIS && initX < xPoint+HYSTERESIS ){
            printf("BLUE\n");
            populate_with(BLUE);
        }
        
        printf("Printing hex values:\n");
        for (int i = 0; i < 8; i++){
            printf("hex%i: %02x\t",i, pSharedPru0->ledColor[i]);
        }
        


        sleep(1);
    }
    return 0;
}

static void* joystickListener(void *vargp)
{
    while (isRunning) {
        if (pSharedPru0->jsRightPressed) {
            pSharedPru0->jsRightPressed = false;
            Game_end();
        }
        if (pSharedPru0->jsDownPressed) {
            pSharedPru0->jsDownPressed = false;
            // Check for the "Fire" condition here
        }
    }
    return 0;
}

double randMToN(double M, double N)
{
    return M + (rand() / ( RAND_MAX / (N-M) ) ) ;  
}

static void generateXYpoint(double *toChangeX, double *toChangeY)
{
    // Randomly pick a point here
    double randX = randMToN(-0.5, 0.5);
    double randY = randMToN(-0.5, 0.5);

    // Since 90 degrees is +- 1.0, random point must not exceed that value
    if (*toChangeX + randX > 1.0 || *toChangeX + randX < -1.0) {
        *toChangeX -= randX;
    }
    else {
        *toChangeX += randX;
    }

    if (*toChangeY + randY > 1.0 || *toChangeY + randY < -1.0) {
        *toChangeY -= randY;
    }
    else {
        *toChangeY += randY;
    }

    printf("X point: %f\tY point: %f\n", *toChangeX, *toChangeY);
}