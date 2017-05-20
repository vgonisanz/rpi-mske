/* 16-bits per pixel - RGB (rrrrr gggggg bbbbb) = RGB565:
* - Color palette of 32×64×32 = 65,536 colors.
* - 5 bits allocated for the Red and Blue color components (32 levels each) and 6 bits for the Green component (64 levels).
* RED     = 0b1111100000000000
* GREEN   = 0b0000011111100000
* BLUE    = 0b0000000000011111
*/
#ifndef _PALETTE_H
#define _PALETTE_H

#include <types.h>

/*
* Convert a RGB888 hexadecimal color into RGB565
*/
u16 rgb_565(const u32 rgb);

#endif	/* _PALETTE_H */
