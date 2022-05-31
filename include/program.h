#ifndef PROGRAM_H
#define PROGRAM_H
#include "list.h"
#include "tss.h"
#include "thread.h"
#define ListItem2PCB(ADDRESS, LIST_ITEM) ((PCB *)((int)(ADDRESS) - (int)&((PCB *)0)->LIST_ITEM))

typedef struct {
    ListItem all_programs;
    ListItem ready_programs;
    PCB *running;
    int USER_CODE_SELECTOR;
    int USER_DATA_SELECTOR;
    int USER_STACK_SELECTOR;
} ProgramManeger;

void pm_init(ProgramManeger *pm);
// 分配一个PCB

int execute_thread(ProgramManeger *pm, ThreadFunc func, void *parameter, const char *name, int priority);

int execute_process(ProgramManeger *pm, const char *filename, int priority);

PCB *allocate_pcb();
// 归还一个PCB
void release_pcb(PCB *program);

void schedule(ProgramManeger *pm);

void mesa_wakeup(ProgramManeger *pm, PCB *pcb);

void tss_init();

void program_exit();

void pcb_print(PCB *pcb);

int create_process_page_directory(ProgramManeger *pm);

Bool create_user_virtual_pool(ProgramManeger *pm, PCB *process);

void load_process(const char *filename);

void activate_program_page(ProgramManeger *pm, PCB *program);

int pm_fork(ProgramManeger *pm);

Bool copy_process(ProgramManeger *pm, PCB *parent, PCB *child);

void pm_exit(ProgramManeger *pm, int ret);

int pm_wait(ProgramManeger *pm, int *retval);

#endif