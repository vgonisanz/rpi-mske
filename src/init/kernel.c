#if !defined(__cplusplus)
#include <stdbool.h>
#endif
#include <stddef.h>
#include <types.h>
#include <init/meta.h>
#include <common/peripherals.h>
#include <common/delay.h>
#include <common/panic.h>
#include <common/io.h>
#include <gpio/gpio.h>
#include <led/led.h>
#include <uart/uart.h>

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif

static void blink(u8 ntimes, u32 tdelay)
{
    volatile int i = 0;

    for (i=0; i<ntimes; i++)
    {
        set_led_state(true);
        delay(tdelay);
        set_led_state(false);
        delay(tdelay);
    }
}

static void initial_message()
{
    struct meta info;

    get_build_info(&info);

    /* TODO: Write prink routine attaching the uart */

    printk("**** Welcome to rpi-mske kernel ****\n");
    printk("Build: %s ", info.VERSION);
    printk("\n\n\n");
}

void kernel_main(u32 r0, u32 r1, u32 atags)
{
	(void) r0;
	(void) r1;
	(void) atags;

    int uart_dev_id;

    init_led();
    blink(3, 0x10000);

    device_init();
    
    if ((uart_dev_id = uart_register()) < 0)
    {
        kernel_panic();
    }

    initial_message();

    u8 buffer;
    while (true)
    {
        device_read(uart_dev_id, &buffer, 1);
        device_write(uart_dev_id, &buffer, 1);
    }
}
