#include "../my-linux-code/include/buzzer.h"
#include "../my-linux-code/include/helpers.h"

#include <stdbool.h>
#include <unistd.h>

int main() {
    Buzzer_startListening();

    Note hitSound[4];
    hitSound[0].isNote = true;
    hitSound[0].durationInMs = 100;
    hitSound[0].period = F_NOTE_PERIOD;
    hitSound[0].dutyCycle = F_NOTE_DUTY_CYCLE;

    hitSound[1].isNote = false;
    hitSound[1].durationInMs = 100;

    hitSound[2].isNote = true;
    hitSound[2].durationInMs = 100;
    hitSound[2].period = C_NOTE_PERIOD;
    hitSound[2].dutyCycle = C_NOTE_DUTY_CYCLE;

    hitSound[3].isNote = false;
    hitSound[3].durationInMs = 100;

    Buzzer_addToQueue(hitSound, 4);

    // sleep(2);
    sleepForMs(1000);

    Note missSound[8];
    missSound[0].isNote = true;
    missSound[0].durationInMs = 50;
    missSound[0].period = C_NOTE_PERIOD;
    missSound[0].dutyCycle = C_NOTE_DUTY_CYCLE;

    missSound[1].isNote = false;
    missSound[1].durationInMs = 50;

    missSound[2].isNote = true;
    missSound[2].durationInMs = 50;
    missSound[2].period = C_NOTE_PERIOD;
    missSound[2].dutyCycle = C_NOTE_DUTY_CYCLE;

    missSound[3].isNote = false;
    missSound[3].durationInMs = 50;

    missSound[4].isNote = true;
    missSound[4].durationInMs = 50;
    missSound[4].period = C_NOTE_PERIOD;
    missSound[4].dutyCycle = C_NOTE_DUTY_CYCLE;

    missSound[5].isNote = false;
    missSound[5].durationInMs = 50;

    missSound[6].isNote = true;
    missSound[6].durationInMs = 50;
    missSound[6].period = F_NOTE_PERIOD;
    missSound[6].dutyCycle = F_NOTE_DUTY_CYCLE;

    missSound[7].isNote = false;
    missSound[7].durationInMs = 50;

    Buzzer_addToQueue(missSound, 8);

    sleep(1);

    Buzzer_stopListening();
}