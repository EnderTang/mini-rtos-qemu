# mini-rtos-qemu

A small teaching RTOS for QEMU emulated ARM Cortex-M3. It is bare metal code: no Linux kernel, no FreeRTOS source, no malloc, and no libc dependency.

The first target is `lm3s6965evb` instead of `mps2-an385` because QEMU's Stellaris LM3S board has a simple PL011-style UART0 at `0x4000C000` that maps cleanly to `-serial mon:stdio` on a headless VPS.

## Install

```sh
sudo apt update
sudo apt install -y qemu-system-arm gcc-arm-none-eabi make gdb-multiarch
```

## Build

```sh
make
```

Outputs are written to `build/`:

- `build/mini_rtos.elf`
- `build/mini_rtos.bin`
- `build/mini_rtos.map`

## Run

```sh
make run
```

Equivalent QEMU command:

```sh
qemu-system-arm \
  -M lm3s6965evb \
  -cpu cortex-m3 \
  -kernel build/mini_rtos.elf \
  -nographic \
  -serial mon:stdio
```

Expected terminal output:

```text
mini-rtos-qemu boot
High priority Task C
Task B
Task A
Task A
Task A
Task B
...
```

Stop QEMU with `Ctrl-A`, then `X`.

## Debug

Terminal 1:

```sh
make debug
```

Terminal 2:

```sh
make gdb
```

Useful GDB commands:

```gdb
b Reset_Handler
b SysTick_Handler
b PendSV_Handler
continue
info registers
```

## Current Implementation

- Cortex-M vector table in `src/startup.c`
- `Reset_Handler` copies `.data`, zeros `.bss`, then calls `main`
- `HardFault_Handler`, `SysTick_Handler`, and assembly `PendSV_Handler`
- MSP is used for reset and exceptions; PSP is used for thread mode tasks
- Static task table, fixed task count, fixed per-task stack
- `rtos_task_create()`
- `rtos_start()`
- `rtos_delay()`
- Critical sections with PRIMASK
- SysTick at 1 kHz
- Simple priority scheduler
- PendSV context switch saving and restoring R4-R11
- Three demo tasks:
  - Task A prints every 100 ms
  - Task B prints every 300 ms
  - High priority Task C prints every 1000 ms

Priority rule: smaller numeric priority means higher scheduling priority. For example, priority `1` runs before priority `2`.

## Project Layout

```text
mini-rtos-qemu/
├── Makefile
├── linker.ld
├── README.md
├── src/
│   ├── main.c
│   ├── startup.c
│   ├── rtos.c
│   ├── rtos.h
│   ├── port.c
│   ├── port.h
│   ├── uart.c
│   └── uart.h
└── asm/
    └── context_switch.s
```

## Key Interview Points

PendSV is used for context switching because it can be configured as the lowest-priority exception. SysTick, UART interrupts, and other urgent exceptions can finish first, then PendSV performs the heavier scheduler switch at a controlled priority.

Only R4-R11 are saved by the RTOS because Cortex-M hardware automatically stacks R0-R3, R12, LR, PC, and xPSR on exception entry. R4-R11 are callee-saved registers, so the software context switch must preserve them.

MSP and PSP are separate stack pointers. MSP is used by reset and exception handlers. PSP is used by normal thread-mode tasks. This keeps task stacks isolated from exception handling stack usage.

SysTick and PendSV work together: SysTick increments the RTOS tick, wakes delayed tasks, and requests PendSV if a different task should run. PendSV then performs the actual register save/restore.

## Extension Roadmap

- commit 1: boot + uart
- commit 2: cooperative scheduler
- commit 3: SysTick + PendSV preemption
- commit 4: priority scheduler
- commit 5: semaphore
- commit 6: mutex + priority inheritance
- commit 7: fixed block memory pool
- commit 8: message queue
- commit 9: MISRA cleanup + tests

Semaphore extension: add a small counting object with a wait list of blocked TCB indexes. `sem_wait()` blocks the current task when the count is zero, and `sem_post()` wakes the highest-priority waiter.

Mutex extension: store owner TCB and lock count. For priority inheritance, temporarily raise the owner's effective priority when a higher-priority task blocks on the mutex.

Fixed block memory pool extension: pre-split a static byte array into equal blocks and manage it with a free-list. Allocation and free stay deterministic.

Message queue extension: use a static ring buffer plus task wait lists for senders and receivers. Keep element size fixed in the first version.
