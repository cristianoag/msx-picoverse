#include <stdio.h>
#include "pico/stdlib.h"

// Define I2S pins
#define I2S_WSEL_PIN  27  // GPIO pin for I2S left-right clock
#define I2S_DATA_PIN  28  // GPIO pin for I2S data
#define I2S_BCLK_PIN  29  // GPIO pin for I2S bit clock


int main()
{
    stdio_init_all();

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
