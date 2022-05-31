#include "syscall.h"
#include "std_lib.h"
#include "interrupt.h"
#include "os_modules.h"
#include "os_constant.h"
#include "asm_utils.h"

int syscall_table[MAX_SYSCALL];

void syscall_init() {
    memset((char *)syscall_table, 0, sizeof(int) * MAX_SYSCALL);
    set_interrupt_descriptor(&interrupt_maneger, 0x80, (uint32)asm_system_call_handler, 3);
}

Bool set_syscall(int index, int function) {
    syscall_table[index] = function;
    return true;
}

int syscall_0(int first, int second, int third, int forth, int fifth) {
    printf("[syscall_0] recieved: %d %d %d %d %d\n", first, second, third, forth, fifth);
    return first + second + third + forth + fifth;
}

int write(const char *str) {
    return asm_system_call(1, (int)str, 0, 0, 0, 0);
}

int syscall_write(const char *str) {
    return print_str(&stdio, str);
}

int fork() {
    return asm_system_call(2, 0, 0, 0, 0, 0);
}

int syscall_fork() {
    return pm_fork(&program_maneger);
}

void exit(int ret) {
    asm_system_call(3, ret, 0, 0, 0, 0);
}

void syscall_exit(int ret) {
    pm_exit(&program_maneger, ret);
}

int wait(int *retval) {
    return asm_system_call(4, (int)retval, 0, 0, 0, 0);
}

int syscall_wait(int *retval) {
    return pm_wait(&program_maneger, retval);
}