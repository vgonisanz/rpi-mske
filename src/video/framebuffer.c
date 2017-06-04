#include <video/framebuffer.h>

#include <common/io.h>
#include <video/monospace_font.h>
#include <video/palette.h>

#include <memory/barrier.h>
#include <memory/memory.h>
#include <memory/mailbox.h>

#define MAILBUFFER_SIZE 256
#define MAILBUFFER_ALIGN 16

/********************************/
/*           Variables          */
/********************************/

/* Storage space for the buffer used to pass information between the CPU and VideoCore
 * (channel 8) needs to be aligned to 16 bytes as the bottom 4 bits of the address
 * passed to VideoCore are used for the mailbox number
 */
static volatile u32 mailbuffer[MAILBUFFER_SIZE] __attribute__((aligned (MAILBUFFER_ALIGN)));
static u32 mailbox_response = 0;

static u32 _physical_screenbase = 0;

static u32 _screen_width = 0;     /* Width of the allocated frame buffer */
static u32 _screen_height = 0;    /* Height of the allocated frame buffer */
static u32 _depth = 0;            /* The number of bits per pixel of the requested frame buffer */
static u32 _bytes_per_pixel = 0;   /* Bytes per pixel, used to manage memory address offsets */
static u32 _screensize = 0;       /* The size of the frame buffer: width x height x depth (bytes per pixel) */
static u32 _pitch = 0;            /* Number of bytes between each row of the frame buffer. */

static u32 _character_cols = 0;   /* Number of characters per line (horizontal) |---> */
static u32 _character_rows = 0;   /* Number of character in vertical | */
static u32 _character_pos_x = 0;  /* Current cursor position in horizontal */
static u32 _character_pos_y = 0;  /* Curren cursor position in vertical */

static u32 _foreground_color = 0xFFFFFFFF;  /* White */
static u32 _background_color = 0xFF000000;  /* Black */

/* Function private declaration */
static s32 get_resolution(u32* width, u32* height);
static s32 set_up_screen(u32 width, u32 height, u32 depth);

/*******************************/
/* Function public definitions */
/*******************************/
s32 init_framebuffer(u32 width, u32 height, u32 background_color)
{
  printk("Initializing framebuffer...\n");

  s32 result = 0;

  /* Write resolution in variables */
  result = get_resolution(&_screen_width, &_screen_height);
  if(result < 0)
  {
    return result;
  }
  printk("Initial resolution found is: %dx%d\n", _screen_width, _screen_height);

  /* Set resolution and allocate framebuffer with 16 bpp - RGB (rrrrr gggggg bbbbb) */
  result = set_up_screen(width, height, 16);
  print_background(background_color);
  //print_pixel(0, 2, 0xFF0000);
  //print_pixel(1, 2, 0xFF0000);
  //print_pixel(2, 2, 0xFF0000);
  //print_pixel(3, 2, 0xFF0000);
  print_pixel(10, 10, 0x0000FF);
  print_pixel(width >> 2, height >> 2, 0xFF0000);
  //print_pixel(width - 1, height - 1, 0xFF0000);
  //print_rectangle(width >> 1, height >> 1, 0, 0, 0xFF0000);
  //print_rectangle(width >> 2, height >> 2, 0, 0, 0x00FF00);
  //print_rectangle(width >> 2, height >> 2, width >> 2, height >> 2, 0x00FFFF);
  //print_rectangle(width >> 1, height >> 1, width >> 1, height >> 1, 0xFFFF00);
  print_rectangle(width, 1, 0, height >> 2, 0xFF0000);
  print_rectangle(1, height, (width >> 2) * 3, 0, 0x00FF00);
  //print_rectangle(10, 10, 20, 20, 0x00FF00);
  //set_cursor_position(7, 7);
  set_foreground_color(0x00FF00);
  set_background_color(0x0000FF);
  print_character(2, 10);

  printk("Framebuffer initialized!\n");
  return result;
}

void set_cursor_position(u32 x, u32 y)
{
  if(x > _character_cols)
  {
    x = _character_cols;
  }
  if(y > _character_rows)
  {
    y = _character_rows;
  }
  _character_pos_x = x;
  _character_pos_y = y;
}

void set_foreground_color(u32 color)
{
  _foreground_color = color;
}

void set_background_color(u32 color)
{
  _background_color = color;
}

/* Todo end function adding x, y cursor position to print character, and fix width character pixel to print propertly */
void print_character(u8 character, u8 pixel_width) /* change pixel width for general value resize, adapt to y too, and use in FONT SIZE CALCULATION */
{
  //const int max_character = sizeof(monospace_font) / (FONT_WIDTH * FONT_HEIGHT);

  /* Info needed to print character values */
  u8 *character_pixels = monospace_font[character];
  u32 x0_pixel = _character_pos_x * FONT_WIDTH;  /* Initial col depends character width */
  u32 y0_pixel = _character_pos_y * FONT_HEIGHT;

  for (u32 y = 0; y < FONT_HEIGHT; y++)
  {
    for (u32 x = 0; x < FONT_WIDTH; x++)
    {
      /* Choose foreground or background color */
      u32 ch_color = 0;
      u32 index = x + y * FONT_WIDTH;

      if ( character_pixels[index] == 1 )
      {
        ch_color = _foreground_color;
      }
      else
      {
        ch_color = _background_color;
      }
      for (u32 width = 0; width < pixel_width; width++)
      {
        print_pixel(x0_pixel + x + width, y0_pixel + y, ch_color);
      }
    }
  }
}

void print_pixel(u32 x0, u32 y0, u32 color)
{
  const u32 pixel_offset = ( x0 * _bytes_per_pixel ) + ( y0 * _pitch );

  if(_bytes_per_pixel == 4) /* TODO doesnt work */
  {
    color = color & 0xFFFFFFFF;
    volatile u32 *pixel_address = (u32 *)(_physical_screenbase + pixel_offset);
    *pixel_address = color;
  }
  else if(_bytes_per_pixel == 3)  /* TODO doesnt work */
  {
    color = color & 0xFFFFFF;
    volatile u32 *pixel_address = (u32 *)(_physical_screenbase + pixel_offset);
    *pixel_address = color;
  }
  else if(_bytes_per_pixel == 2)
  {
    const u16 color565 = rgb_565(color);
    volatile u16 *pixel_address = (u16 *)(_physical_screenbase + pixel_offset);
    *pixel_address = color565;
  }
  else if(_bytes_per_pixel == 1)
  {
    // u8 color
    color = color & 0xFF; /* TODO test and convert RGB to gray */
    volatile u8 *pixel_address = (u8 *)(_physical_screenbase + pixel_offset);
    *pixel_address = color;
  }
}

/* TODO test still working */
void print_rectangle(u32 width, u32 height, u32 x0, u32 y0, u32 color)
{
            /* (0, 0) top-left corner */
  u32 row;  /* | vertical offset = y */
  u32 col;  /* - horizontal offset = x */

  /* Draw frontground color */
  for(row = 0; row <= height; row++)
	{
    const u32 abs_row = row + y0;
    for(col = 0; col <= width; col++)
    {
      const u32 abs_col = col + x0;
      print_pixel(abs_col, abs_row, color);
    }
  }
}

void print_background(u32 color)
{
  printk("Print background: 0x%x!\n", color);
  print_rectangle(_screen_width, _screen_height, 0, 0, color);
}

/********************************/
/* Function private definitions */
/********************************/
static s32 get_resolution(u32* width, u32* height)
{
  /* Physical memory address of the mailbuffer, for passing to VC */
  //u32 physical_mb = mem_v2p((u32)mailbuffer); /* No virtual memory yet */

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
  write_mailbox(8, (u32)mailbuffer);

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

s32 set_up_screen(u32 width, u32 height, u32 depth)
{
  /* Data */
  _depth = depth; /* It shall be 32, 24, 16 or 8 bits */

  if(_depth == 32)
  {
    _bytes_per_pixel = 4; /* 4 bytes: 0x00000000 - 0xFFFFFFFF ARGB */
  }
  else if(_depth == 24)
  {
    _bytes_per_pixel = 3; /* 3 bytes: 0x000000 - 0xFFFFFF RGB */
  }
  else if(_depth == 16)
  {
    _bytes_per_pixel = 2; /* 2 bytes: 0x0000 - 0xFFFF RGB565 */
  }
  else if(_depth == 8)
  {
    _bytes_per_pixel = 1; /* 1 bytes: 0x00 - 0xFF Grayscale*/
  }
  else
  {
    _bytes_per_pixel = 0; /* No supported */
    printk("Depth shall be 32, 24, 16 or 8 bits. Aborting...\n");
    return -1;
  }

  printk("Defined depth = %d, so bytes per pixel are: %d\n", _depth, _bytes_per_pixel);

  u32 count; // Count positions
  u32 var;

	u32 c = 1;
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
	mailbuffer[c++] = _depth;		// 16 bpp

	mailbuffer[c++] = 0x00040001;	// Tag id (allocate framebuffer)
	mailbuffer[c++] = 8;		// Value buffer size (bytes)
	mailbuffer[c++] = 4;		// Req. + value length (bytes)
	mailbuffer[c++] = 16;		// Alignment = 16
	mailbuffer[c++] = 0;		// Space for response

	mailbuffer[c++] = 0;		// Terminating tag

	mailbuffer[0] = c*4;		// Buffer size

	write_mailbox(8, (u32)mailbuffer);

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

  write_mailbox(8, (u32)mailbuffer);

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

  /* Update resolution */
  get_resolution(&_screen_width, &_screen_height);

  /* Need to set up max_x/max_y before using console_write */
  _character_cols = (u32)(_screen_width / FONT_WIDTH);
  _character_rows = (u32)(_screen_height / FONT_HEIGHT);

  printk("Resolution is: %dx%d\n", _screen_width, _screen_height);
  printk("Screen row x cols: %dx%d\n", _character_cols, _character_rows);
  printk("Screen set up completed!\n");
  return 0;
}
