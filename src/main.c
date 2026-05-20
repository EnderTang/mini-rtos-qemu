#include "rtos.h"
#include "uart.h"

#define TASK_A_PRIORITY    (3U)
#define TASK_B_PRIORITY    (2U)
#define TASK_C_PRIORITY    (1U)

static void task_a(void *arg);
static void task_b(void *arg);
static void task_c(void *arg);

int main(void)
{
    uart_init();
    uart_puts("\r\nmini-rtos-qemu boot\r\n");

    (void)rtos_task_create(task_a, (void *)0, TASK_A_PRIORITY);
    (void)rtos_task_create(task_b, (void *)0, TASK_B_PRIORITY);
    (void)rtos_task_create(task_c, (void *)0, TASK_C_PRIORITY);

    rtos_start();

    return 0;
}

static void task_a(void *arg)
{
    (void)arg;

    for (;;) {
        uart_puts("Task A\r\n");
        rtos_delay(100UL);
    }
}

static void task_b(void *arg)
{
    (void)arg;

    for (;;) {
        uart_puts("Task B\r\n");
        rtos_delay(300UL);
    }
}

static void task_c(void *arg)
{
    (void)arg;

    for (;;) {
        uart_puts("High priority Task C\r\n");
        rtos_delay(1000UL);
    }
}
