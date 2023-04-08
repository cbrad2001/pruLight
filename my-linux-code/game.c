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

#define HYSTERESIS 0.1

static bool isRunning;
static pthread_t gameThreadId, joystickListenerId;
static sem_t gameRunningSem;
static pthread_mutex_t animationLock;
static volatile sharedMemStruct_t *pSharedPru0;
static double currentX, currentY, currentZ;
static double xPoint, yPoint;
static int currentScore;

static void* gameThread(void *vargp);
static void* joystickListener(void *vargp);
static void generateXYpoint(double *toChangeX, double *toChangeY);
static void hitAnimation(void);
static void missAnimation(void);

// Taken from https://stackoverflow.com/questions/1340729/how-do-you-generate-a-random-double-uniformly-distributed-between-0-and-1-from-c
static double randMToN(double M, double N);

void Game_start(void)
{
    AccelDrv_init();
    srand((unsigned)time(NULL));
    pSharedPru0 = PRU_getMapping(); // Get access to shared memory for my uses

    isRunning = true;
    currentScore = 0;

    sem_init(&gameRunningSem, 0, 0);
    
    if (pthread_mutex_init(&animationLock, NULL) != 0) {
        perror("Failed to initialize animationMutex");
        abort();
    }

    pthread_create(&gameThreadId, NULL, gameThread, NULL);
    pthread_create(&joystickListenerId, NULL, joystickListener, NULL);
}

void Game_end(void)
{
    isRunning = false;
    pthread_join(gameThreadId, NULL);
    pthread_join(joystickListenerId, NULL);
    sem_destroy(&gameRunningSem);
    pthread_mutex_destroy(&animationLock);
    AccelDrv_cleanup();
}

void Game_wait(void)
{
    sem_wait(&gameRunningSem);
}

int Game_getCurrentScore(void)
{
    return currentScore;
}

//temporary
#define GREEN 0x0f000000
#define RED 0x000f0000
#define BLUE 0x00000f00
#define NONE 0x00000000

typedef struct{
    uint32_t color;
    int start;
    int end;
} neo_display_t;

static void populate_with(uint32_t code, int start, int end){
    for (int i = 0; i <= 7; i++){
        if (i >= start && i <= end)
            pSharedPru0->ledColor[i] = code;
        else
            pSharedPru0->ledColor[i] = NONE;
    }
}

static void* gameThread(void *vargp)
{
    neo_display_t toDisplay;

    currentX = 0, currentY = 0, currentZ = 0;
    xPoint = currentX, yPoint = currentY;
    generateXYpoint(&xPoint, &yPoint);

    // Notes: Leaning left gives positive Y values, and right gives negative Y values
    // Tilting up gives negative X values, and down gives positive X values
    isRunning = true;
    while (isRunning) {
        AccelDrv_getReading(&currentX, &currentY, &currentZ);

        printf("    %15s: 0x%02x\n", "isDownPressed", pSharedPru0->jsDownPressed);
        printf("    %15s: 0x%02x\n", "isRightPressed", pSharedPru0->jsRightPressed);

        // Left-Right plane determines COLOR

        if (currentX <= xPoint-HYSTERESIS)      // if left (-1 to 0): red
            toDisplay.color = RED;
        
        if (currentX >= xPoint+HYSTERESIS)      // if right: green
            toDisplay.color = GREEN;           
            
        if (currentX > xPoint-HYSTERESIS && currentX < xPoint+HYSTERESIS) //if centered: blue
            toDisplay.color = BLUE; 

        // Up-Down plane determines LEDS

        if (currentY <= yPoint-HYSTERESIS){  // pointing DOWN, display bottom 3 LEDS
            toDisplay.start = 0; //bottom 3 LEDs
            toDisplay.end = 2;
        }

        if (currentY >= yPoint+HYSTERESIS){  // pointing DOWN, display bottom 3 LEDS
            toDisplay.start = 5; //top 3 LEDs
            toDisplay.end = 7;
        }
        
        if (currentY > yPoint-HYSTERESIS && currentY < yPoint+HYSTERESIS){
            toDisplay.start = 0; // ALL LEDS
            toDisplay.end = 7;
        }

        // POPULATE SHARED MEM WITH DATA

        populate_with(toDisplay.color,toDisplay.start, toDisplay.end);
        
        // printf("Printing hex values:\n");
        // for (int i = 0; i < 8; i++){
        //     printf("hex%i: %02x\t",i, pSharedPru0->ledColor[i]);
        // }

        // Execution should stop here when LED animation is playing 
        // when user "hits" the generated point.
        pthread_mutex_lock(&animationLock);
        pthread_mutex_unlock(&animationLock);

        sleep(1);
    }
    return 0;
}

static void* joystickListener(void *vargp)
{
    isRunning = true;
    while (isRunning) {
        if (pSharedPru0->jsRightPressed) {
            pSharedPru0->jsRightPressed = false;
            Game_end();
        }
        if (pSharedPru0->jsDownPressed) {
            pSharedPru0->jsDownPressed = false;

            // if centered
            if (currentX > xPoint-HYSTERESIS && currentX < xPoint+HYSTERESIS) {
                // Update to new (x, y) coords
                generateXYpoint(&xPoint, &yPoint);
                currentScore += 1;

                // Note: this will briefly lock up game thread while LED animation plays
                hitAnimation();
            }
            else {
                missAnimation();
            }
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

static void hitAnimation(void)
{
    pthread_mutex_lock(&animationLock);
    {
        // TODO: add buzzer code here

        sleep(3); // replace with some sort of lighting animation
    }
    pthread_mutex_unlock(&animationLock);
}

static void missAnimation(void)
{
    // Don't need to trigger animation mutex as right now it only plays a sound to buzzer
    // TODO: add buzzer code here

}