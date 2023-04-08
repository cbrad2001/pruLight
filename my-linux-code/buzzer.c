/* buzzer.c
*  Code largely inspired by: https://opencoursehub.cs.sfu.ca/bfraser/grav-cms/cmpt433/guides/files/PWMGuide.pdf
*/

#include "include/buzzer.h"
#include "include/helpers.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#define BUZZER_DIR "/dev/bone/pwm/0/a"

#define BUZZER_PERIOD_FILE BUZZER_DIR "/period"
#define BUZZER_DUTY_CYCLE_FILE BUZZER_DIR "/duty_cycle"
#define BUZZER_ENABLE_FILE BUZZER_DIR "/enable"

static Note noteBuffer[1024];
static int bufferPos;
static pthread_t buzzerThreadId;
static bool notesInBuffer, isPlayingNotes;
static bool isRunning;

static void writeToFile(char *fileName, char *value);
static void playNote(Note *note);
static void* buzzerThread(void *vargp);

void Buzzer_startListening(void)
{
    bufferPos = 0;
    notesInBuffer = false;
    isPlayingNotes = false;
    isRunning = true;
    pthread_create(&buzzerThreadId, NULL, buzzerThread, NULL);
}

void Buzzer_stopListening(void)
{
    isRunning = false;
    pthread_join(buzzerThreadId, NULL);
}

void Buzzer_addToQueue(Note toPlay)
{
    // Do not add new notes to the buffer if it is playing notes
    if (!isPlayingNotes) {
        noteBuffer[bufferPos] = toPlay;
        bufferPos += 1;
        notesInBuffer = true;
    }
}

static void* buzzerThread(void *vargp)
{
    while (isRunning)
    {

    }
}

static void playNote(Note *note)
{
    // TODO: play note and sleep for note->durationInMs amount of time then turn off buzzer
    // then remote note from queue
}