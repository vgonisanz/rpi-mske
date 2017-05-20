#include <video/palette.h>

u16 rgb_565(const u32 rgb)
{
  /* Extract components */
  u16 R = (rgb >> 16) & 0xFF;
  u16 G = (rgb >>  8) & 0xFF;
  u16 B = (rgb      ) & 0xFF;

  u16 value  = (R & 0xF8) << 8;  /* 5 bits */
  value     |= (G & 0xFC) << 3;  /* 6 bits */
  value     |= (B & 0xF8) >> 3;  /* 5 bits */

  return(value);
}
