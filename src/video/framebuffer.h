/*
* Framebuffer:
* To manage framebuffer, it is needed to translate physical memory addresses for mailbox to virtual addresses. Mailboxes facilitate communication between the ARM and the VideoCore.
* More info in memory.h and mailbox.h
*/
#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

/* */

int init_framebuffer();

#endif	/* _FRAMEBUFFER_H */
