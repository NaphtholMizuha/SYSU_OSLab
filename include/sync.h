#ifndef SYNC_H
#define SYNC_H

#include "os_type.h"
#include "list.h"
#include "asm_utils.h"
typedef uint32 SpinLock;

void spinlock_init(SpinLock *sl);
void lock(SpinLock *sl);
void unlock(SpinLock *sl);

typedef struct {
    uint32 counter;
    ListItem *waiting;
    SpinLock sem_lock;
} Semaphore;

void semaphore_init(Semaphore* s, uint32 counter);
void sm_wait(Semaphore *s);
void sm_signal(Semaphore *s);

#endif