#include "../my-linux-code/include/analogDisplay.h"

#include <stdlib.h>
#include <unistd.h>

int main()
{
    Analog_startDisplaying();

    for (int i = 0; i < 5; i++) {
        Analog_updateDisplay(i);
        sleep(1);
    }

    Analog_stopDisplaying();

    return 0;
}