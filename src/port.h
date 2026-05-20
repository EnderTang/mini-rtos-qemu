#ifndef PORT_H
#define PORT_H

#include <stdint.h>

#define PORT_SYSTICK_CTRL_ENABLE      (1UL << 0)
#define PORT_SYSTICK_CTRL_TICKINT     (1UL << 1)
#define PORT_SYSTICK_CTRL_CLKSOURCE   (1UL << 2)
#define PORT_SYSTICK_CLOCK_HZ         (12000000UL)
#define PORT_TICK_HZ                  (1000UL)
#define PORT_INITIAL_XPSR             (0x01000000UL)
#define PORT_EXC_RETURN_THREAD_PSP    (0xFFFFFFFDUL)

void port_systick_init(void);
void port_trigger_context_switch(void);
void port_start_first_task(void);
uint32_t port_enter_critical(void);
void port_exit_critical(uint32_t state);

#endif
