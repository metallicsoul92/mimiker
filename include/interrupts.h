#ifndef __SYS_INTERRUPTS_H__
#define __SYS_INTERRUPTS_H__

#include <common.h>

/* Initializes and enables interrupts. */
void intr_init();

/*
 * This is the single interrupt handler procedure. It should not be called
 * manually except by the interrupt handler routine from intr.S.
 */
void intr_dispatcher();

#define intr_disable() __extension__ ({ asm("di"); })
#define intr_enable() __extension__ ({ asm("ei"); })

#endif /* __SYS_INTERRUPTS_H__ */
