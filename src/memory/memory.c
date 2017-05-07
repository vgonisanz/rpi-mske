#include <memory/memory.h>

#include <common/io.h>

#define MEMORY_SIZE 64
#define MEMORY_ALIGN 256


/* Translation table 0: First 64 MB in memory. Needs to be aligned to its size (i.e: 64*4 bytes) */
unsigned int pagetable0[MEMORY_SIZE]	__attribute__ ((aligned (MEMORY_ALIGN)));

/* Need to access the page table, etc as physical memory */
static unsigned int *pagetable = (unsigned int * const) mem_p2v(0x4000); /* 16k */

void init_memory(void)
{
  printk("Initializing memory...\n");

  unsigned int x;
	unsigned int pagetable0_address;

	/* Translation table 0 - covers the first 64 MB, currently nothing mapped in it. */
	for(x = 0; x < MEMORY_SIZE; x++)
	{
		pagetable0[x] = 0;
	}
  printk("Created pagetable with %d MBs.\n", MEMORY_SIZE);

	/* Get physical address of pagetable 0 */
	pagetable0_address = mem_v2p((unsigned int) &pagetable0);

	/* Use translation table 0 up to 64MB */
	asm volatile("mcr p15, 0, %[n], c2, c0, 2" : : [n] "r" (6));

	/* Translation table 0 - ARM1176JZF-S manual, 3-57 */
	asm volatile("mcr p15, 0, %[addr], c2, c0, 0" : : [addr] "r" (pagetable0_address));

	/* Invalidate the translation lookaside buffer (TLB): ARM1176JZF-S manual, p. 3-86 */
	asm volatile("mcr p15, 0, %[data], c8, c7, 0" : : [data] "r" (0));

  printk("Memory initialized!\n");
}

unsigned int mem_v2p(unsigned int virtual_address)
{
  unsigned int pagetable_data = pagetable[virtual_address >> 20];
  unsigned int type_of_entry = pagetable_data & 3;

  /* Translation fault */
	if(type_of_entry == 0)
	{
		return 0xffffffff;
	}

  /* Coarse page table */
  if(type_of_entry == 1)
	{
    unsigned int coarse_pagetable_data;
    coarse_pagetable_data = ((unsigned int *)(VIRTUAL_ADDRESS_BASE + (pagetable_data & 0xfffffc00)))[(virtual_address >> 12) & 0xff];

    /* Nothing mapped */
  	if((coarse_pagetable_data & 3) == 0)
  	{
  		return 0xffffffff;
  	}

    /* Small (4k) page */
  	if(coarse_pagetable_data & 2)
  	{
  		return (coarse_pagetable_data & 0xfffff000) + (virtual_address & 0xfff);
  	}

  	/* Large 64k page */
  	return (coarse_pagetable_data & 0xffff0000) + (virtual_address & 0xffff);
  }

  /* Sections */
	if(type_of_entry == 2)
	{
    unsigned int physical_address;

		/* SuperSection base */
		physical_address = pagetable_data & 0xfff00000;

		if(pagetable_data & (1<<18))
		{
			/* 16MB Supersection */
			physical_address += virtual_address & 0x00ffffff;
		}
		else
		{
			/* 1MB Section */
			physical_address += virtual_address & 0x000fffff;
		}
		return physical_address;
	}

  /* Reserved */
  if(type_of_entry == 3)
  {
    return 0xffffffff;
  }
  /* Unknown problem, cannot come here */
  return 0xffffffff;
}
