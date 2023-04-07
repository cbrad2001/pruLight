#include "../my-linux-code/include/accel_drv.h"
#include "../my-linux-code/include/helpers.h"

#include <stdlib.h>
#include <stdio.h>

int main()
{
    AccelDrv_init();
    double x, y, z;
    while (1) {
        AccelDrv_getReading(&x, &y, &z);
        printf("X: %.2f | Y: %.2f | Z: %.2f\n", x, y, z);
        sleepForMs(500);
    }

    // Unreachable code
    AccelDrv_cleanup();
}