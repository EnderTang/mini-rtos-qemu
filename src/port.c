#include "port.h"

#define SYST_CSR   (*(volatile uint32_t *)0xE000E010UL)
#define SYST_RVR   (*(volatile uint32_t *)0xE000E014UL)
#define SYST_CVR   (*(volatile uint32_t *)0xE000E018UL)
#define SCB_ICSR   (*(volatile uint32_t *)0xE000ED04UL)
#define SCB_SHPR3  (*(volatile uint32_t *)0xE000ED20UL)

#define SCB_ICSR_PENDSVSET            (1UL << 28)
#define SCB_SHPR3_PENDSV_PRIORITY     (0xFFUL << 16)
#define SCB_SHPR3_SYSTICK_PRIORITY    (0x80UL << 24)

void port_systick_init(void)
{
    SCB_SHPR3 = (SCB_SHPR3 & ~((0xFFUL << 16) | (0xFFUL << 24))) |
                SCB_SHPR3_PENDSV_PRIORITY |
                SCB_SHPR3_SYSTICK_PRIORITY;

    SYST_RVR = (PORT_SYSTICK_CLOCK_HZ / PORT_TICK_HZ) - 1UL;
    SYST_CVR = 0UL;
    SYST_CSR = PORT_SYSTICK_CTRL_CLKSOURCE |
               PORT_SYSTICK_CTRL_TICKINT |
               PORT_SYSTICK_CTRL_ENABLE;
}

void port_trigger_context_switch(void)
{
    SCB_ICSR = SCB_ICSR_PENDSVSET;
}

uint32_t port_enter_critical(void)
{
    uint32_t primask;

    __asm volatile ("mrs %0, primask" : "=r" (primask) :: "memory");
    __asm volatile ("cpsid i" ::: "memory");
    return primask;
}

void port_exit_critical(uint32_t state)
{
    __asm volatile ("msr primask, %0" :: "r" (state) : "memory");
}
