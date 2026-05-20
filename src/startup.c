#include <stdint.h>
#include "rtos.h"
#include "uart.h"

extern uint32_t _estack;
extern uint32_t _etext;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

void Reset_Handler(void);
void HardFault_Handler(void);
void SysTick_Handler(void);
void PendSV_Handler(void);
int main(void);

void Default_Handler(void);

__attribute__((section(".isr_vector")))
void (* const g_vector_table[])(void) =
{
    (void (*)(void))(&_estack),
    Reset_Handler,
    Default_Handler,
    HardFault_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    0,
    0,
    0,
    0,
    Default_Handler,
    Default_Handler,
    0,
    PendSV_Handler,
    SysTick_Handler
};

void Reset_Handler(void)
{
    uint32_t *src = &_etext;
    uint32_t *dst = &_sdata;

    while (dst < &_edata) {
        *dst = *src;
        dst++;
        src++;
    }

    dst = &_sbss;
    while (dst < &_ebss) {
        *dst = 0UL;
        dst++;
    }

    (void)main();

    for (;;) {
    }
}

void SysTick_Handler(void)
{
    rtos_tick_handler();
}

void HardFault_Handler(void)
{
    uart_puts("\r\nHardFault\r\n");
    for (;;) {
    }
}

void Default_Handler(void)
{
    for (;;) {
    }
}
