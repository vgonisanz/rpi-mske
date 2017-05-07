/*
* The ARM section of the RAM starts at 0x00000000 and the VideoCore section of the RAM is mapped in only if the system is configured to support a memory mapped display.
* The bus addresses for RAM are set up to map onto the uncached bus address range on the VideoCore starting at 0xC0000000.
*
* Bus:
*   The bus addresses for peripherals are set up to map onto the peripheral bus address range starting at 0x7E000000.
*
* Physical addresses:
*   Peripherals will range from 0x20000000 to 0x20FFFFFF. Use peripherals.h to manage.
*
* Virtual addresses: (Check arm-c-virtual-addresses figure or BCM2835 ARM peripherals)
*   Virtual addresses in user mode (i.e. seen by processes running in ARM Linux) will range between 0x00000000 and 0xBFFFFFFF.
*   Virtual addresses in kernel mode will range between 0xC0000000 and 0xEFFFFFFF.
*
* DMA:  use Direct access memory. DMA allows certain hardware subsystems to access main system memory (RAM) independent of the central processing unit (CPU). It is needed to ...
*
* Memory addresses summary:
*   User process memory     | 0x00000000 - 0x7fffffff    = 0 - 2GB
*     Peripherals           |   0x20000000 - 0x20ffffff
*   Physical memory         | 0x80000000 - 0xa0ffffff
*   Kernel data             | 0xc0000000 - 0xefffffff
*   Kernel code             | 0xf0000000 - 0xffffffff
*/
#ifndef _MEMORY_H
#define _MEMORY_H

/*** Memory management ***/

/* Virtual addresses range defined as 0x80000000 offset */
#define VIRTUAL_ADDRESS_BASE    0x80000000

/* Initialise memory: Set up first 64MB of RAM as unmapped pagetable */
extern int init_memory(void);

/* Convert a physical address to a virtual one */
#define mem_p2v(X) (X+VIRTUAL_ADDRESS_BASE)

/* Convert a virtual address to a physical one by following the page table. Returns physical address, or 0xffffffff if the virtual address does not map - ARM1176-TZJS page 6-39 */
extern unsigned int mem_v2p(unsigned int virtual_address);

#endif /* _MEMORY_H */
