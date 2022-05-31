#include "sync.h"
#include "thread.h"
#include "os_modules.h"
void spinlock_init(SpinLock *bolt) {
    *bolt = 0;
}

void lock(SpinLock *bolt) {
    SpinLock key = 1;
    do {
        asm_atomic_exchange(&key, bolt); // 原子性交换
    } while(key);
}

void unlock(SpinLock *bolt) {
    *bolt = 0;
}

void semaphore_init(Semaphore* s, uint32 counter) {
    s->counter = counter;
    spinlock_init(&s->sem_lock);
    li_init(s->waiting);
}

void sm_wait(Semaphore *s) {
    PCB *cur = nullptr;

    while (true) {
        lock(&s->sem_lock);

        if (s->counter > 0) {
            s->counter -= 1;
            unlock(&s->sem_lock);
            return;
        }

        cur = program_maneger.running;
        li_push_back(s->waiting, &(cur->tag_in_general_list));
        cur->status = BLOCKED;

        unlock(&s->sem_lock);
        schedule(&program_maneger);
    }
}

void sm_signal(Semaphore *s) {
    lock(&s->sem_lock);
    ++s->counter;
    if (li_size(s->waiting)) {
        PCB *program = ListItem2PCB(li_front(s->waiting), tag_in_general_list);
        li_pop_front(s->waiting);
        unlock(&s->sem_lock);
        mesa_wakeup(&program_maneger, program);
    } else {
        unlock(&s->sem_lock);
    }
}