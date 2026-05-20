.syntax unified
.cpu cortex-m3
.thumb

.global PendSV_Handler
.global port_start_first_task

.extern g_rtos_current_tcb
.extern g_rtos_next_tcb

/* Start the first thread using PSP. MSP remains reserved for exceptions. */
.thumb_func
port_start_first_task:
    ldr r0, =g_rtos_current_tcb
    ldr r0, [r0]
    ldr r0, [r0]
    ldmia r0!, {r4-r11}

    /* Initial stack frame layout now starts at R0. */
    ldr r1, [r0, #20]
    ldr r2, [r0, #24]
    adds r3, r0, #32
    msr psp, r3

    ldr r0, [r0, #0]
    movs r3, #2
    msr control, r3
    isb
    mov lr, r1
    bx r2

/* PendSV runs at the lowest exception priority and performs the context switch. */
.thumb_func
PendSV_Handler:
    cpsid i

    mrs r0, psp
    cbz r0, pend_sv_restore

    stmdb r0!, {r4-r11}
    ldr r1, =g_rtos_current_tcb
    ldr r2, [r1]
    cbz r2, pend_sv_restore
    str r0, [r2]

pend_sv_restore:
    ldr r1, =g_rtos_next_tcb
    ldr r2, [r1]
    ldr r3, =g_rtos_current_tcb
    str r2, [r3]
    ldr r0, [r2]
    ldmia r0!, {r4-r11}
    msr psp, r0

    cpsie i
    bx lr
