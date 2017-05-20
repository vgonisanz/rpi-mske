/*
* Framebuffer:
* To manage framebuffer, it is needed to translate physical memory addresses for mailbox to virtual addresses. Mailboxes facilitate communication between the ARM and the VideoCore.
* More info in memory.h and mailbox.h
*/
#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

#include <types.h>

/*
* Allocate and Initialize framebuffer with size and color.
*/
s32 init_framebuffer(u32 width, u32 height, u32 background_color);

/*
* Draw rectangle with color given.
*/
void print_rectangle(u32 width, u32 height, u32 x0, u32 y0, u32 color);

/*
* Draw brackground with color given.
*/
void print_background(u32 color);

/*
* Draw ASCII character.
*/
void print_character(u8 character);

/*
* Change current cursor position, x = cols, y = rows
*/
void set_cursor_position(u32 x, u32 y);

/*
* Change character color from RGB888 to be used in following draws.
*/
void set_foreground_color(u32 color);

/*
* Change background color from RGB888 to be used in following draws.
*/
void set_background_color(u32 color);

#endif	/* _FRAMEBUFFER_H */
