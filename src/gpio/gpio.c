#include <common/delay.h>
#include "gpio.h"

/* page 90 of BCM2835-ARM-Peripherals.pdf */
/* These are relative offset to GPIO_BASE address */
enum
{
    /* function selector */
    GPIO_FSEL0      = (GPIO_BASE + 0x00),
    GPIO_FSEL1      = (GPIO_BASE + 0x04),
    GPIO_FSEL2      = (GPIO_BASE + 0x08),
    GPIO_FSEL3      = (GPIO_BASE + 0x0C),
    GPIO_FSEL4      = (GPIO_BASE + 0x10),
    GPIO_FSEL5      = (GPIO_BASE + 0x14),

    /* set and clear pin output */
    GPIO_SET0       = (GPIO_BASE + 0x1C),
    GPIO_SET1       = (GPIO_BASE + 0x20),
    GPIO_CLR0       = (GPIO_BASE + 0x28),
    GPIO_CLR1       = (GPIO_BASE + 0x2C),

    /* pulldown control registers */
    GPIO_PUD        = (GPIO_BASE + 0x94), /* control all pins */
    GPIO_PUDCLK0    = (GPIO_BASE + 0x98), /* enable clock 0 */
    GPIO_PUDCLK1    = (GPIO_BASE + 0x9C), /* enable clock 1 */
};

void set_gpio_function(u32 pin, enum GPIO_Funcs fn)
{
    //volatile u32 *fsel = &(volatile u32 *)(GPIO_FSEL0[pin / 10]);
    u32 shift = (pin % 10) * 3;
    u32 mask = ~(7 << shift);
    mmio_write(GPIO_FSEL0, (*(volatile u32 *)GPIO_FSEL0 & mask) | (fn << shift));
    //*fsel = (*fsel & mask) | (fn << shift);
}

void set_gpio_state(u32 pin, bool state)
{
    /* set or clear */
    state ? mmio_write(((volatile u32 *)GPIO_SET0)[pin / 32], (1 << (pin % 32)))
            : mmio_write(((volatile u32 *)GPIO_CLR0)[pin / 32], (1 << (pin % 32)));
}

/* 
 * The GPIO Pull-up/down Clock Registers control the actuation of internal pull-downs on
 * the respective GPIO pins. These registers must be used in conjunction with the GPPUD
 * register to effect GPIO Pull-up/down changes. The following sequence of events is
 * required:
 *  1. Write to GPPUD to set the required control signal (i.e. Pull-up or Pull-Down or neither
 *     to remove the current Pull-up/down)
 *  2. Wait 150 cycles – this provides the required set-up time for the control signal
 *  3. Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads you wish to
 *     modify – NOTE only the pads which receive a clock will be modified, all others will
 *     retain their previous state.
 *  4. Wait 150 cycles – this provides the required hold time for the control signal
 *  5. Write to GPPUD to remove the control signal
 *  6. Write to GPPUDCLK0/1 to remove the clock
*/
void set_gpio_pull_up_down(u32 pin, enum PullUpDown fn)
{
    mmio_write(GPIO_PUD, (u32)fn);
    delay(150);

    pin < 32 ? mmio_write(GPIO_PUDCLK0, (1 << (pin % 32))) : mmio_write(GPIO_PUDCLK1, (1 << (pin % 32)));
    delay(150);

    /* clear action and remove clock */
    mmio_write(GPIO_PUD, (u32)OFF);
    pin < 32 ? mmio_write(GPIO_PUDCLK0, 0x00000000) : mmio_write(GPIO_PUDCLK1, 0x00000000);
}