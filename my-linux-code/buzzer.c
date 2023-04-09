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

static Note noteBuffer[50];
static int bufferPos;
static pthread_t buzzerThreadId;
static bool notesInBuffer, isPlayingNotes;
static bool isRunning;

static void writeToFile(char *fileName, char *value);
static void playNotesInBuffer(void);
static void* buzzerThread(void *vargp);

void Buzzer_startListening(void)
{
    bufferPos = 0;
    notesInBuffer = false;
    isPlayingNotes = false;
    isRunning = true;
    runCommand("config-pin p9_22 pwm");
    pthread_create(&buzzerThreadId, NULL, buzzerThread, NULL);
}

void Buzzer_stopListening(void)
{
    isRunning = false;
    pthread_join(buzzerThreadId, NULL);
}

void Buzzer_addToQueue(Note *toPlay, int numToPlay)
{
    // Do not add new notes to the buffer if it is playing notes
    if (!isPlayingNotes) {
        // noteBuffer[bufferPos] = toPlay;
        // bufferPos += 1;
        // notesInBuffer = true;
        for (int i = 0; i < numToPlay; i++) {
            noteBuffer[i] = toPlay[i];
        }
        bufferPos = numToPlay - 1;
        notesInBuffer = true;
    }
}

static void* buzzerThread(void *vargp)
{
    while (isRunning)
    {
        if (notesInBuffer) {
            playNotesInBuffer();
        }
        sleepForMs(10);
    }
    return 0;
}

static void playNotesInBuffer(void)
{
    char periodBuf[10];
    char dutyBuf[10];
    isPlayingNotes = true;
    for (int i = 0; i < bufferPos; i++) {
        if (noteBuffer[i].isNote) {
            // TODO: play the note
            sprintf(periodBuf, "%d", noteBuffer[i].period);
            sprintf(dutyBuf, "%d", noteBuffer[i].dutyCycle);
            writeToFile(BUZZER_DUTY_CYCLE_FILE, "0");
            writeToFile(BUZZER_PERIOD_FILE, periodBuf);
            writeToFile(BUZZER_DUTY_CYCLE_FILE, dutyBuf);
            writeToFile(BUZZER_ENABLE_FILE, "1");
        }
        sleepForMs(noteBuffer[i].durationInMs);
        writeToFile(BUZZER_ENABLE_FILE, "0");
    }
    notesInBuffer = false;
}

static void writeToFile(char *filename, char *dataToWrite)
{
    FILE *toWrite = fopen(filename, "w");
    if (toWrite == NULL)
    {
        fprintf(stderr, "Error opening %s. Exiting.\n", filename);
        abort();
    }
    fprintf(toWrite, "%s", dataToWrite);
    fclose(toWrite);
}
