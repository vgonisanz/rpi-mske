/*** Memory barriers or data cache invalidation/flushing may be required around mailbox accesses ***/

#ifndef _MEM_BARRIER_H
#define _MEM_BARRIER_H

/* Invalidate Entire Data Cache */
#define invalidate_cache() asm volatile \
		("mcr p15, #0, %[zero], c7, c6, #0" : : [zero] "r" (0) )

/* Clean Entire Data Cache */
#define clear_cache() asm volatile \
		("mcr p15, #0, %[zero], c7, c10, #0" : : [zero] "r" (0) )

/* Clean and Invalidate Entire Data Cache */ /* TODO use before read mailbox and check if really needed */
#define flush_cache() asm volatile \
		("mcr p15, #0, %[zero], c7, c14, #0" : : [zero] "r" (0) )

/* Data Synchronization Barrier: No instruction after the DSB can run until all instructions before it have */
#define data_sb() asm volatile \
		("mcr p15, #0, %[zero], c7, c10, #4" : : [zero] "r" (0) )

/* Data Memory Barrier: No memory access after the DMB can run until all memory accesses before it have completed */
#define data_mb() asm volatile \
		("mcr p15, #0, %[zero], c7, c10, #5" : : [zero] "r" (0) )

#endif /* _MEM_BARRIER_H */
