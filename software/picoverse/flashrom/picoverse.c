#include <stdio.h>
#include <stdlib.h>
#include <tusb.h>
#include <bsp/board.h>
#include <ff.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "hardware/watchdog.h"
#include "hardware/vreg.h"
#include "hardware/pio.h"
#include "hardware/structs/xip_ctrl.h"
#include "hardware/regs/xip.h"
#include "hardware/flash.h"
#include "hardware/dma.h"
#include "bootsel_button.h"
#include "flash.h"
#include "picoverse.h"

#define VER_MAJOR 1
#define VER_MINOR 0
#define VER_PATCH 0

static FATFS filesystem;

// values from the linker
extern uint32_t __FLASH_START[];
extern uint32_t __FLASH_LEN[];
extern uint32_t __DRIVE_START[];
extern uint32_t __DRIVE_LEN[];
extern uint32_t __DRIVE_END[];

void fatal(int flashes) {
    while(1) {
        for (int i=0;i<flashes;i++) {
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            sleep_ms(100);
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
            sleep_ms(200);   
        }
        sleep_ms(500);
    }
}

void format() {
    FRESULT res;        /* API result code */
    FIL fp;
    BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */
    MKFS_PARM params = {
        FM_FAT,
        1,
        0,
        0,
        0
    };
    res = f_mkfs("", &params, work, sizeof(work));
    if (res) fatal(res);
    f_mount(&filesystem, "", 1);
    f_setlabel("PICOVERSE");
    f_open(&fp, "README.TXT", FA_CREATE_ALWAYS|FA_WRITE);
    f_printf(&fp, "Welcome to MSX PICOVERSE %d.%d.%d\n", VER_MAJOR, VER_MINOR, VER_PATCH );
    f_printf(&fp, "Copy your ROMs and config files here for processing.\n");
    f_close(&fp);

}

// Check the bootsel button. If pressed for ~10 seconds, reformat and reboot
static void button_task(void) {
    static uint64_t long_push = 0;

    if (bb_get_bootsel_button()) {
        long_push++;
    } else {
        long_push = 0;
    }
    if (long_push > 125000) { // Long-push BOOTSEL button
        // turn on the LED
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        // wait for button release
        while(bb_get_bootsel_button());
        // turn off LED
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        flash_format();
        flash_init();
        format();
        // and reset
        watchdog_enable(1, 1);
        while(1);
    }
}

// flash the LED 
void led_task() {
    static int64_t timer = 0;
    static bool led = false;

    if (to_ms_since_boot(get_absolute_time()) -  timer > 500) {
        gpio_put(PICO_DEFAULT_LED_PIN, led);
        timer = to_ms_since_boot(get_absolute_time()) + 500;
        led = !led;
    }
}

void usb_mode() {
    board_init();
    tud_init(BOARD_TUD_RHPORT);
    stdio_init_all();  
    f_unmount("");
    while(1) // the mainloop
    {
        button_task();
        tud_task(); // device task
        led_task();
    } 
}

void debug(const char *msg) {
    #ifdef DEBUG_TO_FILE
        FIL fp;
        UINT l;
        f_open(&fp, "DEBUG.TXT", FA_WRITE|FA_OPEN_APPEND);
        f_printf(&fp, "%06d: %s\n", to_ms_since_boot(get_absolute_time()), msg);
        f_close(&fp);
    #endif
    }

int main()
{
    stdio_init_all();
    flash_init();
    usb_mode();
}
