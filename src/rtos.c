#include "rtos.h"
#include "port.h"

#define RTOS_INVALID_TASK_INDEX        (0xFFFFFFFFUL)
#define RTOS_STACK_ALIGN_MASK          (0x7UL)
#define RTOS_INITIAL_LR                (0xFFFFFFFDUL)

static rtos_tcb_t g_tasks[RTOS_MAX_TASKS];
static uint32_t g_task_count;
static volatile uint32_t g_tick_count;
static uint8_t g_scheduler_started;

rtos_tcb_t *g_rtos_current_tcb;
rtos_tcb_t *g_rtos_next_tcb;

static void rtos_idle_task(void *arg);
static uint32_t rtos_pick_next_index(void);
static void rtos_task_exit_trap(void);

rtos_status_t rtos_task_create(rtos_task_fn_t entry, void *arg, uint8_t priority)
{
    rtos_tcb_t *tcb;
    uint32_t *sp;
    uint32_t state;

    if (entry == (rtos_task_fn_t)0) {
        return RTOS_ERROR_PARAM;
    }

    state = rtos_enter_critical();

    if (g_task_count >= RTOS_MAX_TASKS) {
        rtos_exit_critical(state);
        return RTOS_ERROR_FULL;
    }

    tcb = &g_tasks[g_task_count];
    g_task_count++;

    tcb->entry = entry;
    tcb->arg = arg;
    tcb->priority = priority;
    tcb->wake_tick = 0UL;
    tcb->state = RTOS_TASK_READY;

    sp = &tcb->stack[RTOS_STACK_WORDS];
    sp = (uint32_t *)((uint32_t)sp & ~RTOS_STACK_ALIGN_MASK);

    /* Hardware exception frame, restored automatically by exception return. */
    *(--sp) = PORT_INITIAL_XPSR;
    *(--sp) = (uint32_t)entry;
    *(--sp) = (uint32_t)rtos_task_exit_trap;
    *(--sp) = 0x12121212UL;
    *(--sp) = 0x03030303UL;
    *(--sp) = 0x02020202UL;
    *(--sp) = 0x01010101UL;
    *(--sp) = (uint32_t)arg;

    /* Software-saved callee registers R4-R11. */
    *(--sp) = 0x11111111UL;
    *(--sp) = 0x10101010UL;
    *(--sp) = 0x09090909UL;
    *(--sp) = 0x08080808UL;
    *(--sp) = 0x07070707UL;
    *(--sp) = 0x06060606UL;
    *(--sp) = 0x05050505UL;
    *(--sp) = 0x04040404UL;

    tcb->sp = sp;

    rtos_exit_critical(state);
    return RTOS_OK;
}

void rtos_start(void)
{
    (void)rtos_task_create(rtos_idle_task, (void *)0, RTOS_IDLE_PRIORITY);

    g_scheduler_started = 1U;
    rtos_select_next_task();
    g_rtos_current_tcb = g_rtos_next_tcb;

    port_systick_init();
    port_start_first_task();

    for (;;) {
        /* The first task should never return here. */
    }
}

void rtos_delay(uint32_t delay_ms)
{
    uint32_t state;

    if ((delay_ms == 0UL) || (g_scheduler_started == 0U)) {
        return;
    }

    state = rtos_enter_critical();
    g_rtos_current_tcb->wake_tick = g_tick_count + delay_ms;
    g_rtos_current_tcb->state = RTOS_TASK_BLOCKED;
    rtos_select_next_task();
    rtos_exit_critical(state);

    port_trigger_context_switch();
}

void rtos_tick_handler(void)
{
    uint32_t i;

    g_tick_count++;

    for (i = 0UL; i < g_task_count; i++) {
        if ((g_tasks[i].state == RTOS_TASK_BLOCKED) &&
            ((int32_t)(g_tick_count - g_tasks[i].wake_tick) >= 0)) {
            g_tasks[i].state = RTOS_TASK_READY;
        }
    }

    rtos_schedule_from_isr();
}

void rtos_schedule_from_isr(void)
{
    rtos_select_next_task();

    if (g_rtos_next_tcb != g_rtos_current_tcb) {
        port_trigger_context_switch();
    }
}

void rtos_select_next_task(void)
{
    uint32_t next;

    next = rtos_pick_next_index();
    if (next != RTOS_INVALID_TASK_INDEX) {
        if ((g_rtos_current_tcb != (rtos_tcb_t *)0) &&
            (g_rtos_current_tcb->state == RTOS_TASK_RUNNING)) {
            g_rtos_current_tcb->state = RTOS_TASK_READY;
        }

        g_rtos_next_tcb = &g_tasks[next];
        g_rtos_next_tcb->state = RTOS_TASK_RUNNING;
    }
}

uint32_t rtos_enter_critical(void)
{
    return port_enter_critical();
}

void rtos_exit_critical(uint32_t state)
{
    port_exit_critical(state);
}

static uint32_t rtos_pick_next_index(void)
{
    uint32_t i;
    uint32_t best_index = RTOS_INVALID_TASK_INDEX;
    uint8_t best_priority = RTOS_IDLE_PRIORITY;

    for (i = 0UL; i < g_task_count; i++) {
        if ((g_tasks[i].state == RTOS_TASK_READY) ||
            (g_tasks[i].state == RTOS_TASK_RUNNING)) {
            if ((best_index == RTOS_INVALID_TASK_INDEX) ||
                (g_tasks[i].priority < best_priority)) {
                best_priority = g_tasks[i].priority;
                best_index = i;
            }
        }
    }

    return best_index;
}

static void rtos_idle_task(void *arg)
{
    (void)arg;

    for (;;) {
        __asm volatile ("wfi");
    }
}

static void rtos_task_exit_trap(void)
{
    for (;;) {
        /* A teaching RTOS requires tasks to be infinite loops. */
    }
}
