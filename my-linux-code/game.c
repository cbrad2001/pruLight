#include "include/game.h"
#include "include/helpers.h"
#include "include/accel_drv.h"
#include "include/pru_code.h"
#include "include/analogDisplay.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <math.h>

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
    Analog_quit();
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
#define GREEN_BR 0xff000000
#define RED 0x000f0000
#define RED_BR 0x00ff0000
#define BLUE 0x00000f00
#define BLUE_BR 0x0000ff00
#define NONE 0x00000000

typedef enum {
    near,
    far,
    very_far,
    hit,
} distance_t;

typedef enum {
    up,
    down,
    level,
    left,
    right,
} direction_t;

typedef struct{
    // uint32_t color;
    direction_t x_dir;
    direction_t y_dir;
    distance_t y_dist;
} neo_display_t;

static void populate_with(direction_t dir_LR, direction_t dir_UD, distance_t dist){

    for (int i = 0; i <= 7; i++){
        pSharedPru0->ledColor[i] = NONE;
    }

    if ( dir_UD == up && dist == very_far){         //VERY FAR AWAY UP
        if (dir_LR == left)
            pSharedPru0->ledColor[7] = RED_BR;
        if (dir_LR == right)
            pSharedPru0->ledColor[7] = GREEN_BR;
        if (dir_LR == level)
            pSharedPru0->ledColor[7] = BLUE_BR;
        return;
    }
    
    if ( dir_UD == down && dist == very_far){         //FAR AWAY DOWN
        if (dir_LR == left)
            pSharedPru0->ledColor[0] = RED_BR;
        if (dir_LR == right)
            pSharedPru0->ledColor[0] = GREEN_BR;
        if (dir_LR == level)
            pSharedPru0->ledColor[0] = BLUE_BR;
        return;
    }

    if ( dir_LR == left && dist == hit){           // HIT but RIGHT
        for (int i = 0; i <= 7; i++)
            pSharedPru0->ledColor[i] = RED;
        
        pSharedPru0->ledColor[3] = RED_BR;
        pSharedPru0->ledColor[4] = RED_BR;
        return;
    }

    if ( dir_UD == level && dir_LR == level && dist == hit){           // HIT
        for (int i = 0; i <= 7; i++)
            pSharedPru0->ledColor[i] = BLUE;
        
        pSharedPru0->ledColor[3] = BLUE_BR;
        pSharedPru0->ledColor[4] = BLUE_BR;
        return;
    }

    if ( dir_LR == right && dist == hit){           // HIT but RIGHT
        for (int i = 0; i <= 7; i++)
            pSharedPru0->ledColor[i] = GREEN;
        
        pSharedPru0->ledColor[3] = GREEN_BR;
        pSharedPru0->ledColor[4] = GREEN_BR;
        return;
    }

    if ( dir_UD == up && dist == far){              //FAR AWAY UP
        if (dir_LR == left){
            pSharedPru0->ledColor[7] = RED;
            pSharedPru0->ledColor[6] = RED_BR;
            pSharedPru0->ledColor[5] = RED;
        }
            
        if (dir_LR == right){
            pSharedPru0->ledColor[7] = GREEN;
            pSharedPru0->ledColor[6] = GREEN_BR;
            pSharedPru0->ledColor[5] = GREEN;
        }
        if (dir_LR == level){
            pSharedPru0->ledColor[7] = BLUE;
            pSharedPru0->ledColor[6] = BLUE_BR;
            pSharedPru0->ledColor[5] = BLUE;
        }
        return;
    }

     if ( dir_UD == down && dist == far){           //FAR AWAY DOWN
        if (dir_LR == left){
            pSharedPru0->ledColor[2] = RED;
            pSharedPru0->ledColor[1] = RED_BR;
            pSharedPru0->ledColor[0] = RED;
        }
            
        if (dir_LR == right){
            pSharedPru0->ledColor[2] = GREEN;
            pSharedPru0->ledColor[1] = GREEN_BR;
            pSharedPru0->ledColor[0] = GREEN;
        }
        if (dir_LR == level){
            pSharedPru0->ledColor[2] = BLUE;
            pSharedPru0->ledColor[1] = BLUE_BR;
            pSharedPru0->ledColor[0] = BLUE;
        }
        return;
    }

    if ( dir_UD == up && dist == near){              // CLOSE UP
        if (dir_LR == left){
            pSharedPru0->ledColor[6] = RED;
            pSharedPru0->ledColor[5] = RED_BR;
            pSharedPru0->ledColor[4] = RED;
        }
            
        if (dir_LR == right){
            pSharedPru0->ledColor[5] = GREEN;
            pSharedPru0->ledColor[4] = GREEN_BR;
            pSharedPru0->ledColor[3] = GREEN;
        }
        if (dir_LR == level){
            pSharedPru0->ledColor[5] = BLUE;
            pSharedPru0->ledColor[4] = BLUE_BR;
            pSharedPru0->ledColor[3] = BLUE;
        }
        return;
    }

    if ( dir_UD == down && dist == near){           // CLOSE DOWN
        if (dir_LR == left){
            pSharedPru0->ledColor[3] = RED;
            pSharedPru0->ledColor[2] = RED_BR;
            pSharedPru0->ledColor[1] = RED;
        }
            
        if (dir_LR == right){
            pSharedPru0->ledColor[3] = GREEN;
            pSharedPru0->ledColor[2] = GREEN_BR;
            pSharedPru0->ledColor[1] = GREEN;
        }
        if (dir_LR == level){
            pSharedPru0->ledColor[3] = BLUE;
            pSharedPru0->ledColor[2] = BLUE_BR;
            pSharedPru0->ledColor[1] = BLUE;
        }
        return;
    }
}

static double getDist(double pt1, double pt2){
    return sqrt((pt1*pt1)-(pt2*pt2));
}

static distance_t convertDistToVal(double dist){
    if (dist > 0.7) 
        return very_far;
    else if (dist > 0.4) 
        return far;
    else 
        return near;
}

static void* gameThread(void *vargp)
{
    neo_display_t toDisplay;

    currentX = 0, currentY = 0, currentZ = 0;
    double dist = 0; 
    xPoint = currentX, yPoint = currentY;
    generateXYpoint(&xPoint, &yPoint);

    // Notes: Leaning left gives positive Y values, and right gives negative Y values
    // Tilting up gives negative X values, and down gives positive X values
    isRunning = true;
    while (isRunning) {
        AccelDrv_getReading(&currentX, &currentY, &currentZ);

        // printf("    %15s: 0x%02x\n", "isDownPressed", pSharedPru0->jsDownPressed);
        // printf("    %15s: 0x%02x\n", "isRightPressed", pSharedPru0->jsRightPressed);

        // Left-Right plane determines COLOR

        if (currentX <= xPoint-HYSTERESIS)      // if left (-1 to 0): red
            toDisplay.x_dir = left;
        
        if (currentX >= xPoint+HYSTERESIS)      // if right: green
            toDisplay.x_dir = right;           
            
        if (currentX > xPoint-HYSTERESIS && currentX < xPoint+HYSTERESIS) //if centered: blue
            toDisplay.x_dir = level; 

        // Up-Down plane determines LEDS

        if (currentY <= yPoint-HYSTERESIS){  // pointing DOWN, display bottom 3 LEDS
            dist = getDist(currentY,yPoint-HYSTERESIS);
            toDisplay.y_dist = convertDistToVal(dist);
            toDisplay.y_dir = down;
        }

        if (currentY >= yPoint+HYSTERESIS){  // pointing UP, display top 3 LEDS
            dist = getDist(currentY,yPoint+HYSTERESIS);
            toDisplay.y_dist = convertDistToVal(dist);
            toDisplay.y_dir = up;
        }
        
        if (currentY > yPoint-HYSTERESIS && currentY < yPoint+HYSTERESIS){
            toDisplay.y_dist = hit;
            toDisplay.y_dir = level;
        }

        // POPULATE SHARED MEM WITH DATA

        populate_with(toDisplay.x_dir,toDisplay.y_dir,toDisplay.y_dist);
        
        // printf("Printing hex values:\n");
        // for (int i = 0; i < 8; i++){
        //     printf("hex%i: %02x\t",i, pSharedPru0->ledColor[i]);
        // }

        // Execution should stop here when LED animation is playing 
        // when user "hits" the generated point.
        pthread_mutex_lock(&animationLock);
        pthread_mutex_unlock(&animationLock);

        sleepForMs(10);
    }
    return 0;
}

static void* joystickListener(void *vargp)
{
    isRunning = true;
    while (isRunning) {
        if (!pSharedPru0->jsRightPressed) {
            printf("Right Joystick pressed! Game ending...\n");
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