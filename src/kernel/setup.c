#include "asm_utils.h"
#include "interrupt.h"
#include "std_io.h"
#include "std_lib.h"
#include "program.h"
#include "sync.h"
#include "memory_maneger.h"
#include "syscall.h"

InterruptManeger interrupt_maneger;
Std_IO stdio;
ProgramManeger program_maneger;
MemoryManeger memory_maneger;
TSS tss;

void zero_process() {
    asm_system_call(0, 1, 1, 4, 5, 1);
    asm_halt();
}

void fork_test() {
    int pid = fork();
    if (pid) {
        printf("I am sup-process and return: %d\n", pid);
    } else {
        printf("I am sub-process and return: %d\n", pid);
    }
    asm_halt();
}

void first_process() {
    int pid = fork();
    int retval;
    if (pid) {
        pid = fork();
        if (pid) {
            while ((pid = wait(&retval)) != -1) {
                printf("wait for a child process, pid: %d, return value: %d\n", 
                       pid, retval);
            }
            printf("all child process exit, programs: %d\n", li_size(&program_maneger.all_programs));    
            asm_halt();
        }
        else {
            uint32 tmp = 0xffffff;
            while (tmp)
                --tmp;
            printf("exit, pid: %d\n", program_maneger.running->pid);
            exit(114514);
        }
    }
    else {
        uint32 tmp = 0xffffff;
        while (tmp)
            --tmp;
        printf("exit, pid: %d\n", program_maneger.running->pid);
        exit(-1919810);
    }
}

void zombie() {
    int pid = fork();
    if (pid) {
        exit(114);
    } else {
        asm_halt();
    }
}

void thread_to_exit(void *args) {
    printf("thread is exiting...\n");
    exit(0);
}

void first_thread(void* args) {
    // execute_process(&program_maneger, (char*)zero_process, 2);
    execute_process(&program_maneger, (char*)fork_test, 1);
    // execute_process(&program_maneger, (char*)first_process, 1);
    //execute_thread(&program_maneger, (ThreadFunc)thread_to_exit, 0, "second", 1);
    asm_halt();
}

extern void setup_kernel() {
    // for im
    im_init(&interrupt_maneger);
    enable_time_interrupt();
    set_time_interrupt(&interrupt_maneger, (void *)asm_time_interrupt_handler);
    // for io
    io_init(&stdio);
    io_clear(&stdio);
    pm_init(&program_maneger);
    
    syscall_init();
    set_syscall(0, (int)syscall_0);
    set_syscall(1, (int)syscall_write);
    set_syscall(2, (int)syscall_fork);
    set_syscall(3, (int)syscall_exit);
    set_syscall(4, (int)syscall_wait);

    mm_init(&memory_maneger);
    int pid = execute_thread(&program_maneger, first_thread, nullptr, "first thread", 1);
    if (pid == -1) {
        printf("cannot execute thread\n");
        asm_halt();
    }
    
    ListItem *item = li_front(&(program_maneger.ready_programs));
    PCB *first_thread_pcb = ListItem2PCB(item, tag_in_general_list);
    first_thread_pcb->status = RUNNING;
    li_pop_front(&(program_maneger.ready_programs));
    program_maneger.running = first_thread_pcb;
    asm_switch_thread(0, first_thread_pcb); 
    asm_halt();
}

