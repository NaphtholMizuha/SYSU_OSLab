#ifndef ASM_UTILS_H
#define ASM_UTILS_H

#include "os_type.h"

extern void asm_hello_world();
extern void asm_lidt(uint32 start, uint16 limit);
extern void asm_unhandled_interrupt();
extern void asm_halt();
extern void asm_out_port(uint16 port, uint8 value);
extern void asm_in_port(uint16 port, uint8 *value);
extern void asm_enable_interrupt();
extern void asm_time_interrupt_handler();
extern int asm_interrupt_status();
extern void asm_disable_interrupt();
extern void asm_switch_thread(void *cur, void *next);
extern void asm_atomic_exchange(uint32 *reg, uint32 *mem);
extern void asm_init_page_reg(int *directory);
extern int asm_system_call(int index, int first, int second, int third, int forth, int fifth);
extern int asm_system_call_handler();
extern int asm_add_global_descriptor(int low, int high);
extern void asm_ltr(int tr);
extern void asm_start_process(int stack);
extern void asm_update_cr3(int address);

#endif