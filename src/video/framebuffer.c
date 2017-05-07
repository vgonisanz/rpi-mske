#include <video/framebuffer.h>

#include <common/io.h>

#include <memory/barrier.h>
#include <memory/memory.h>
#include <memory/mailbox.h>

#define MAILBUFFER_SIZE 256
#define MAILBUFFER_ALIGN 16

int init_framebuffer()
{
  printk("Initializing framebuffer...\n");

  int result = 0;
  unsigned int var;
	unsigned int count;
	unsigned int physical_screenbase;

	/* Storage space for the buffer used to pass information between the CPU and VideoCore
	 * (channel 8) needs to be aligned to 16 bytes as the bottom 4 bits of the address
	 * passed to VideoCore are used for the mailbox number
   */
	volatile unsigned int mailbuffer[256] __attribute__((aligned (16)));

	/* Physical memory address of the mailbuffer, for passing to VC */
	unsigned int physical_mb = mem_v2p((unsigned int)mailbuffer);

	/* Get the display size */
	mailbuffer[0] = 8 * 4;		// Total size
	mailbuffer[1] = 0;		// Request
	mailbuffer[2] = 0x40003;	// Display size
	mailbuffer[3] = 8;		// Buffer size
	mailbuffer[4] = 0;		// Request size
	mailbuffer[5] = 0;		// Space for horizontal resolution
	mailbuffer[6] = 0;		// Space for vertical resolution
	mailbuffer[7] = 0;		// End tag

  printk("Writing mailbox!\n");
	write_mailbox(8, physical_mb);

  printk("Reading mailbox!\n");
	var = read_mailbox(8);

  /* Valid response in data structure TODO check why 80000000 */
  if(mailbuffer[1] != 0x80000000)
    result = -1;
    printk("Error initializing framebuffer, address: %x\n", mailbuffer[1]);
    return result;

  unsigned int screen_width = mailbuffer[5];
  unsigned int screen_height = mailbuffer[6];
  printk("Resolution found: %dx%d", screen_width, screen_height);

  printk("Framebuffer initialized!\n");
  return result;
}
