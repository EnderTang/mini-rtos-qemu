#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>

#define RTOS_MAX_TASKS        (8U)
#define RTOS_STACK_WORDS      (256U)
#define RTOS_IDLE_PRIORITY    (255U)

typedef void (*rtos_task_fn_t)(void *arg);

typedef enum
{
    RTOS_OK = 0,
    RTOS_ERROR_FULL = -1,
    RTOS_ERROR_PARAM = -2
} rtos_status_t;

typedef enum
{
    RTOS_TASK_UNUSED = 0,
    RTOS_TASK_READY,
    RTOS_TASK_RUNNING,
    RTOS_TASK_BLOCKED
} rtos_task_state_t;

typedef struct
{
    uint32_t *sp;
    uint32_t stack[RTOS_STACK_WORDS];
    rtos_task_fn_t entry;
    void *arg;
    uint32_t wake_tick;
    uint8_t priority;
    rtos_task_state_t state;
} rtos_tcb_t;

extern rtos_tcb_t *g_rtos_current_tcb;
extern rtos_tcb_t *g_rtos_next_tcb;

rtos_status_t rtos_task_create(rtos_task_fn_t entry, void *arg, uint8_t priority);
void rtos_start(void);
void rtos_delay(uint32_t delay_ms);
void rtos_tick_handler(void);
void rtos_schedule_from_isr(void);
void rtos_select_next_task(void);
uint32_t rtos_enter_critical(void);
void rtos_exit_critical(uint32_t state);

#endif
