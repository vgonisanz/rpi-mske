#include <common/defs.h>
#include <irq/irq.h>

static mske_interrupt_vector_t mske_irq_vector_table[INTC_IRQ_TOTAL];

/* page 112 */
enum IRQ_Register
{
    IRQ_BASIC_PENDING = (IRQ_BASE + 0x200),
    IRQ_PENDING1      = (IRQ_BASE + 0x204),
    IRQ_PENDING2      = (IRQ_BASE + 0x208),
    IRQ_FIQ_CONTROL   = (IRQ_BASE + 0x20C),
    IRQ_ENABLE1       = (IRQ_BASE + 0x210),
    IRQ_ENABLE2       = (IRQ_BASE + 0x214),
    IRQ_ENABLE_BASIC  = (IRQ_BASE + 0x218),
    IRQ_DISABLE1      = (IRQ_BASE + 0x21C),
    IRQ_DISABLE2      = (IRQ_BASE + 0x220),
    IRQ_DISABLE_BASIC = (IRQ_BASE + 0x224)
};

/**
 *  Enables all IRQ's in the CPU's CPSR register.
 */
static inline void irq_enable() 
{
    asm volatile("mrs     r0,cpsr");      /* Read in the cpsr register */
    asm volatile("bic     r0,r0,#0x80");  /* Clear bit 8, (0x80) -- Causes IRQs to be enabled. */
    asm volatile("msr     cpsr_c, r0");   /* Write it back to the CPSR register */
}

static inline void irq_disable() 
{
    asm volatile("mrs     r0,cpsr");      /* Read in the cpsr register */
    asm volatile("orr     r0,r0,#0x80");  /* Set bit 8, (0x80) -- Causes IRQs to be disabled */
    asm volatile("msr     cpsr_c, r0");   /* Write it back to the CPSR register */
}

#define clz(a) \
({ unsigned long __value, __arg = (a); \
     asm ("clz\t%0, %1": "=r" (__value): "r" (__arg)); \
          __value; })


/* default irq_handler */
static void default_irq_handler(enum mske_irq_vector_id nirq, void *param) 
{
    UNUSED(nirq);
    UNUSED(param);
}

mske_ret_code_t interrupt_controller_init()
{
    int i = 0;

    for(i=0; i<INTC_IRQ_TOTAL; i++)
    {
        mske_irq_vector_table[i].irq_fn_handler = default_irq_handler;
        mske_irq_vector_table[i].param = (void *)0;
    }

    return MSKE_SUCESS;
}

mske_ret_code_t register_interrupt(enum mske_irq_vector_id nirq, mske_fn_interrupt_handler irq_handler, void *param)
{
    irq_disable();
    mske_irq_vector_table[nirq].irq_fn_handler = irq_handler;
    mske_irq_vector_table[nirq].param = param;
    irq_enable();

    return MSKE_SUCESS;
}

mske_ret_code_t enable_irqs(void)
{
    irq_enable();
    
    return MSKE_SUCESS;
}

mske_ret_code_t disable_irqs(void)
{
    irq_disable();

    return MSKE_SUCESS;
}

void enable_irq(enum mske_irq_vector_id vector)
{
    UNUSED(vector);
}

void disable_irq(enum mske_irq_vector_id vector)
{
    UNUSED(vector);
}
