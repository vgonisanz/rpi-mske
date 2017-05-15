/*
* Mailbox:
* Each mailbox is an 8-deep FIFO of 32-bit words, which can be read (popped)/written (pushed) by the ARM and VideoCore and defines the following channels:
*   0: Power management
*   1: Framebuffer
*   2: Virtual UART
*   3: VCHIQ
*   4: LEDs
*   5: Buttons
*   6: Touch screen
*   7:
*   8: Property tags (ARM -> VideoCore)
*   9: Property tags (VideoCore -> ARM)
*
* Mailbox   Peek  Read/Write  Status  Sender  Config
*      0    0x10  0x00        0x18    0x14    0x1c
*      1    0x30  0x20        0x38    0x34    0x3c
*
* All registers are unsigned int (32 bits) little-endian.
*
* More info: https://github.com/raspberrypi/firmware/wiki/Accessing-mailboxes
*
*/
#ifndef _MAILBOX_H
#define _MAILBOX_H

#include <common/peripherals.h>

/* Base address for the mailbox registers */
#define MAILBOX_ADDRESS_BASE 0xB880
#define MAILBOX0_ADDRESS    (PERIPHERALS_BASE + MAILBOX_ADDRESS_BASE)

/* Mailbox full: Bit 31 is set in the status register if there is no space to write into the mailbox */
#define MAILBOX_FULL 0x80000000

/* Mailbox empty: Bit 30 is set in the status register if there is nothing to read from the mailbox */
#define MAILBOX_EMPTY 0x40000000

/* REQUEST ID */
#define REQUEST_SUCCESSFUL 0x80000000
#define REQUEST_ERROR 0x80000001

/* Define channels */
#define MAILBOX_CHANNEL_ARM2VC 8
#define MAILBOX_CHANNEL_VC2ARM 9

#define READ_MAILBOX_TIMEOUT 1<<25

typedef struct
{
  unsigned int read;
  unsigned int unused1;
  unsigned int unused2;
  unsigned int unused3;
  unsigned int poll;
  unsigned int sender;
  unsigned int status;
  unsigned int configuration;
  unsigned int write;
} mske_mailbox;

/* Read a channel. The channel must be zero to 15 */
extern unsigned int read_mailbox(unsigned int channel);

/* Write the given data to the channel. The channel must be zero to 15 and value in last 4 bits */
extern int write_mailbox(unsigned int channel, unsigned int data);

#endif	/* _MAILBOX_H */
