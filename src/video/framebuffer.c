#include <video/framebuffer.h>

#include <common/io.h>

#include <memory/barrier.h>
#include <memory/memory.h>
#include <memory/mailbox.h>

#define MAILBUFFER_SIZE 256
#define MAILBUFFER_ALIGN 16

/* Storage space for the buffer used to pass information between the CPU and VideoCore
 * (channel 8) needs to be aligned to 16 bytes as the bottom 4 bits of the address
 * passed to VideoCore are used for the mailbox number
 */
static volatile unsigned int mailbuffer[MAILBUFFER_SIZE] __attribute__((aligned (MAILBUFFER_ALIGN)));
static unsigned int mailbox_response = 0;

static unsigned int _screen_width = 0;
static unsigned int _screen_height = 0;
static unsigned int _physical_screenbase = 0;
static unsigned int _screensize = 0;
static unsigned int _pitch = 0;
static unsigned int _screen_cols = 0;
static unsigned int _screen_rows = 0;

static unsigned short int _foreground_color = 0xffff;  /* Black */
//static unsigned short int _background_color = 0;

/* Character cells are 6x10: Change for dictionary header */
#define CHARSIZE_X	6
#define CHARSIZE_Y	10

/* Function declaration */
static int get_resolution(unsigned int* width, unsigned int* height);
static int set_up_screen(unsigned int width, unsigned int height);
static int test_print();

/* Function definitions */
int init_framebuffer()
{
  printk("Initializing framebuffer...\n");

  int result = 0;

  /* Write resolution in variables */
  result = get_resolution(&_screen_width, &_screen_height);
  if(result < 0)
  {
    return result;
  }
  printk("Resolution found: %dx%d\n", _screen_width, _screen_height);

  /* Use resolution as read variables */
  result = set_up_screen(_screen_width, _screen_height);

  test_print();
  printk("Framebuffer initialized!\n");
  return result;
}

static int get_resolution(unsigned int* width, unsigned int* height)
{
  /* Physical memory address of the mailbuffer, for passing to VC */
  //unsigned int physical_mb = mem_v2p((unsigned int)mailbuffer); /* No virtual memory yet */

  /* Get the display size */
  mailbuffer[0] = 8 * 4;		// Total size
  mailbuffer[1] = 0;		// Request
  mailbuffer[2] = 0x40003;	// Display size
  mailbuffer[3] = 8;		// Buffer size
  mailbuffer[4] = 0;		// Request size
  mailbuffer[5] = 0;		// Space for horizontal resolution
  mailbuffer[6] = 0;		// Space for vertical resolution
  mailbuffer[7] = 0;		// End tag

  //printk("Writing mailbox!\n");
  write_mailbox(8, (unsigned int)mailbuffer);

  //printk("Reading mailbox!\n");
  mailbox_response = read_mailbox(8);

  if(mailbuffer[1] != REQUEST_SUCCESSFUL)
  {
    printk("Error initializing framebuffer\n");
    return -1;
  }

  *width = mailbuffer[5];
  *height = mailbuffer[6];

  /* Assume qemu, so set as 640x480 */
  if(*width == 0 && *height == 0)
	{
		*width = 640;
		*height = 480;
	}
  return 0;
}

int set_up_screen(unsigned int width, unsigned int height)
{
  unsigned int count; // Count positions
  unsigned int var;

	unsigned int c = 1;
	mailbuffer[c++] = 0;		// Request

	mailbuffer[c++] = 0x00048003;	// Tag id (set physical size)
	mailbuffer[c++] = 8;		// Value buffer size (bytes)
	mailbuffer[c++] = 8;		// Req. + value length (bytes)
	mailbuffer[c++] = width;		// Horizontal resolution
	mailbuffer[c++] = height;		// Vertical resolution

	mailbuffer[c++] = 0x00048004;	// Tag id (set virtual size)
	mailbuffer[c++] = 8;		// Value buffer size (bytes)
	mailbuffer[c++] = 8;		// Req. + value length (bytes)
	mailbuffer[c++] = width;		// Horizontal resolution
	mailbuffer[c++] = height;		// Vertical resolution

	mailbuffer[c++] = 0x00048005;	// Tag id (set depth)
	mailbuffer[c++] = 4;		// Value buffer size (bytes)
	mailbuffer[c++] = 4;		// Req. + value length (bytes)
	mailbuffer[c++] = 16;		// 16 bpp

	mailbuffer[c++] = 0x00040001;	// Tag id (allocate framebuffer)
	mailbuffer[c++] = 8;		// Value buffer size (bytes)
	mailbuffer[c++] = 4;		// Req. + value length (bytes)
	mailbuffer[c++] = 16;		// Alignment = 16
	mailbuffer[c++] = 0;		// Space for response

	mailbuffer[c++] = 0;		// Terminating tag

	mailbuffer[0] = c*4;		// Buffer size

	write_mailbox(8, (unsigned int)mailbuffer);

	var = read_mailbox(8);

	/* Valid response in data structure */
  if(mailbuffer[1] != REQUEST_SUCCESSFUL)
  {
    printk("Fail to set up screen\n");
    return -1;
  }

	count = 2;	/* First tag */
	while((var = mailbuffer[count]))
	{
    /* Look for allocate buffer tag */
		if(var == 0x40001)
    {
      printk("Allocate buffer tag found!\n");
      break;
    }

    printk("Skipping tag: %x\n", var);
		/* Skip tag and check next: 1 (tag) + 2 (buffer size/value size) + specified buffer size */
		count += 3+(mailbuffer[count+1]>>2);

		if(count > c)
		{
        printk("Invalid tags. Set up failed.\n");
        return -2;
    }
	}

  /* 8 bytes, plus MSB set to indicate a response */
  if(mailbuffer[count+2] != 0x80000008)
  {
    printk("Invalid tag response: %x\n", mailbuffer[count+2]);
    return -3;
  }

  /* Framebuffer address/size in response */
  _physical_screenbase = mailbuffer[count+3];
  _screensize = mailbuffer[count+4];

  if(_physical_screenbase == 0 || _screensize == 0)
  {
    printk("Invalid tag data\n");
    return -4;
  }
  printk("Screen initialized at: 0x%x with size: %d\n", _physical_screenbase, _screensize);

  /* physical_screenbase is the address of the screen in RAM screenbase needs to be the screen address in virtual memory */
  //screenbase=mem_p2v(physical_screenbase);

  /* Get the framebuffer pitch (bytes per line) */
  mailbuffer[0] = 7 * 4;		// Total size
  mailbuffer[1] = 0;		// Request
  mailbuffer[2] = 0x40008;	// Display size
  mailbuffer[3] = 4;		// Buffer size
  mailbuffer[4] = 0;		// Request size
  mailbuffer[5] = 0;		// Space for pitch
  mailbuffer[6] = 0;		// End tag

  write_mailbox(8, (unsigned int)mailbuffer);

  var = read_mailbox(8);

  /* 4 bytes, plus MSB set to indicate a response */
  if(mailbuffer[4] != 0x80000004)
  {
    printk("Invalid pitch response: %x\n", mailbuffer[4]);
    return -5;
  }

  _pitch = mailbuffer[5];
  if(_pitch == 0)
  {
    printk("Invalid pitch data\n");
    return -5;
  }

  printk("Pitch is: %d\n", _pitch);

  /* Need to set up max_x/max_y before using console_write */
  _screen_cols = _screen_width / CHARSIZE_X;
  _screen_rows = _screen_height / CHARSIZE_Y;

  printk("Screen row x cols: %dx%d\n", _screen_cols, _screen_rows);
  printk("Screen set up completed!\n");
  return 0;
}

static int test_print()
{
  printk("Test color!\n");

  volatile unsigned short int *ptr; /* Memory screen pointer */

  // Try red character:
  //unsigned int character = 41; /* A */
  _foreground_color = 0b1111100000000000;  /* RED */
  //unsigned int background_color = 0b0000000000000000;

  // Buffer index in memory
  unsigned int buffer_index = 0;
  unsigned int row = 0;

  /* Draw character pixels */
  for(row = 0; row < CHARSIZE_Y; row++)
	{
    ptr = (unsigned short int *)(_physical_screenbase + buffer_index);
    *ptr = _foreground_color;
  }
  printk("Test color end!\n");
  return 0;
}
