#include "main.h"

#include <stdio.h>

int tmain(void) {

    volatile int index = 0;

    while (1) {
        printf("Count of 't': %d\n", index);
        index += 2;
    }

    return 0;
}
