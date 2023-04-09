#include "../my-linux-code/include/buzzer.h"

#include <stdbool.h>
#include <unistd.h>

int main() {
    Buzzer_startListening();

    Note hitSound[4];
    hitSound[0].isNote = true;
    hitSound[0].durationInMs = 200;
    hitSound[0].period = C_NOTE_PERIOD;
    hitSound[0].dutyCycle = C_NOTE_DUTY_CYCLE;

    hitSound[1].isNote = false;
    hitSound[1].durationInMs = 200;

    hitSound[2].isNote = true;
    hitSound[2].durationInMs = 200;
    hitSound[2].period = C_NOTE_PERIOD;
    hitSound[2].dutyCycle = C_NOTE_DUTY_CYCLE;

    hitSound[3].isNote = false;
    hitSound[3].durationInMs = 200;

    Buzzer_addToQueue(hitSound, 4);

    sleep(1);

    Buzzer_stopListening();
}