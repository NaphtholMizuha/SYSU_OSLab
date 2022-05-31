#ifndef THREAD_H
#define THREAD_H
#include "os_constant.h"
#include "list.h"
#include "std_io.h"
#include "address_pool.h"
enum ProgramStatus {
    CRATED,
    RUNNING,
    READY,
    BLOCKED,
    DEAD
};

typedef void (*ThreadFunc)(void *);

typedef struct PCB {
    int *stack; // stack pointer 'esp'
    char name[MAX_PROGRAM_NAME + 1]; // process name
    enum ProgramStatus status; // process status
    int priority;
    int pid;
    int ticks; // total time of time slice
    int ticks_passed_by;
    ListItem tag_in_general_list;
    ListItem tag_in_all_list;

    int page_directory_addr;
    AddressPool user_virtual;

    int parent_pid;

    int ret_value;
} PCB;


#endif