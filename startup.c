#include <stdint.h>

// Linker-provided symbol: top of stack (defined in linker script)
extern long _estack;

// Forward declaration of your reset handler (defined in main.c or here)
extern void _reset(void);


// A generic handler for any interrupt you haven't implemented yet.
// If an unexpected interrupt fires and jumps here, it just spins,
// which is far safer than jumping into undefined memory.
void Default_Handler(void) {
    for (;;) {}
}

// Weak aliases: if you never define e.g. SysTick_Handler elsewhere,
// it silently falls back to Default_Handler instead of failing to link.
#define WEAK_ALIAS __attribute__((weak, alias("Default_Handler")))

void NMI_Handler(void) WEAK_ALIAS;
void HardFault_Handler(void) WEAK_ALIAS;
void MemManage_Handler(void) WEAK_ALIAS;
void BusFault_Handler(void) WEAK_ALIAS;
void UsageFault_Handler(void) WEAK_ALIAS;
void SVC_Handler(void) WEAK_ALIAS;
void DebugMon_Handler(void) WEAK_ALIAS;
void PendSV_Handler(void) WEAK_ALIAS;
void SysTick_Handler(void) WEAK_ALIAS;


__attribute__((section(".vectors"), used))
void (* const vector_table[])(void) = {
    (void (*)(void)) &_estack,   // 0: initial stack pointer
    _reset,                      // 1: Reset_Handler
    NMI_Handler,                 // 2
    HardFault_Handler,           // 3
    MemManage_Handler,           // 4
    BusFault_Handler,            // 5
    UsageFault_Handler,          // 6
    0, 0, 0, 0,                  // 7-10: reserved
    SVC_Handler,                 // 11
    DebugMon_Handler,            // 12
    0,                           // 13: reserved
    PendSV_Handler,              // 14
    SysTick_Handler,             // 15
    // 16 onward = peripheral IRQs (EXTI, TIM, USART, etc).
    // Not needed for a polling-delay blinky — leave as Default_Handler
    // if you fill them in, or omit; unused table entries default to 0
    // as long as you never enable that interrupt in NVIC.
};




