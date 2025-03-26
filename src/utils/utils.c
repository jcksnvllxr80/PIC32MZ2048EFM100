#include "utils.h"

void delay_ms(unsigned int milliseconds) {
    unsigned int count;
    while (milliseconds--) {
        for (count = 0; count < TICKS_PER_SECOND; count++) {
            __asm__("NOP"); // No operation (adjust for your clock speed)
        }
    }
}