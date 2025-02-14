#include "sd_card.h"
#include "hw_config.h"

// SPI interface
spi_t spi1 = {
    .hw_inst = spi1, // SPI1 interface
    .miso_gpio = 31,
    .mosi_gpio = 30,
    .sck_gpio = 29,
    .baud_rate = 1000 * 1000,
    .dma_isr = spi1_dma_isr
};

// SD card configuration
sd_card_t sd_card1 = {
    .pcName = "0:", // Name used to mount the drive
    .spi = &spi1,
    .ss_gpio = 32,
    .use_card_detect = false,
    .set_drive_strength = true
};

// Mount the SD card
void sd_card_mount(void) {
    if (sd_init_driver() != 0) {
        printf("ERROR: Could not initialize SD card\r\n");
    }
}