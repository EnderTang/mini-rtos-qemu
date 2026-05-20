#include <stdint.h>
#include "uart.h"

#define UART0_BASE        (0x4000C000UL)
#define UART_DR           (*(volatile uint32_t *)(UART0_BASE + 0x000UL))
#define UART_FR           (*(volatile uint32_t *)(UART0_BASE + 0x018UL))
#define UART_IBRD         (*(volatile uint32_t *)(UART0_BASE + 0x024UL))
#define UART_FBRD         (*(volatile uint32_t *)(UART0_BASE + 0x028UL))
#define UART_LCRH         (*(volatile uint32_t *)(UART0_BASE + 0x02CUL))
#define UART_CTL          (*(volatile uint32_t *)(UART0_BASE + 0x030UL))
#define UART_CC           (*(volatile uint32_t *)(UART0_BASE + 0xFC8UL))

#define UART_FR_TXFF      (1UL << 5)
#define UART_LCRH_WLEN_8  (0x3UL << 5)
#define UART_LCRH_FEN     (1UL << 4)
#define UART_CTL_UARTEN   (1UL << 0)
#define UART_CTL_TXE      (1UL << 8)
#define UART_CTL_RXE      (1UL << 9)

void uart_init(void)
{
    UART_CTL = 0UL;
    UART_CC = 0UL;
    UART_IBRD = 8UL;
    UART_FBRD = 44UL;
    UART_LCRH = UART_LCRH_WLEN_8 | UART_LCRH_FEN;
    UART_CTL = UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE;
}

void uart_putc(char c)
{
    if (c == '\n') {
        uart_putc('\r');
    }

    while ((UART_FR & UART_FR_TXFF) != 0UL) {
    }

    UART_DR = (uint32_t)c;
}

void uart_puts(const char *s)
{
    const char *p = s;

    while ((p != (const char *)0) && (*p != '\0')) {
        uart_putc(*p);
        p++;
    }
}
