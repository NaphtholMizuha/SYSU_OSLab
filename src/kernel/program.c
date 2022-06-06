#include "program.h"
#include "std_lib.h"
#include "interrupt.h"
#include "asm_utils.h"
#include "std_io.h"
#include "thread.h"
#include "os_constant.h"
#include "os_modules.h"
#include "list.h"
#include "memory_maneger.h"
#include "process.h"
#include "syscall.h"
#define PCB_SIZE 4096

char PCB_SET[PCB_SIZE * MAX_PROGRAM_AMOUNT]; // 存放PCB的数组，预留了MAX_PROGRAM_AMOUNT个PCB的大小空间。
bool PCB_SET_STATUS[MAX_PROGRAM_AMOUNT];     // PCB的分配状态，true表示已经分配，false表示未分配。

void pm_init(ProgramManeger *pm) {
    li_init(&pm->all_programs);
    li_init(&pm->ready_programs);
    pm->running = nullptr;
    for (int i = 0; i < MAX_PROGRAM_AMOUNT; ++i) {
        PCB_SET_STATUS[i] = false;
    }

    int selector;

    selector = asm_add_global_descriptor(USER_CODE_LOW, USER_CODE_HIGH);
    pm->USER_CODE_SELECTOR = (selector << 3) | 0x3;

    selector = asm_add_global_descriptor(USER_DATA_LOW, USER_DATA_HIGH);
    pm->USER_DATA_SELECTOR = (selector << 3) | 0x3;

    selector = asm_add_global_descriptor(USER_STACK_LOW, USER_STACK_HIGH);
    pm->USER_STACK_SELECTOR = (selector << 3) | 0x3;

    tss_init();
}

PCB *allocate_pcb() {
    for (int i = 0; i < MAX_PROGRAM_AMOUNT; ++i) {
        if (!PCB_SET_STATUS[i]) {
            PCB_SET_STATUS[i] = true;
            return (PCB*)((int)PCB_SET + PCB_SIZE * i);
        }
    }
    return nullptr;
}

void release_pcb(PCB *program) {
    int index = ((int)program - (int)PCB_SET) / PCB_SIZE;
    PCB_SET_STATUS[index] = false;
    li_erase(&program_maneger.all_programs, &program->tag_in_all_list);
}

int execute_thread(ProgramManeger *pm, ThreadFunc func, void *parameter, const char *name, int priority) {
    bool status = get_interrupt_status(&interrupt_maneger);

    disable_interrupt(&interrupt_maneger);

    PCB *thread = allocate_pcb();

    if (!thread) {
        return -1;
    }

    memset(thread, 0, PCB_SIZE);

    for (int i = 0; i < MAX_PROGRAM_NAME && name[i]; ++i) {
        thread->name[i] = name[i];
    }

    thread->status = READY;
    thread->priority = priority;
    thread->ticks = priority * 10;
    thread->ticks_passed_by = 0;
    thread->pid = ((int)thread - (int)PCB_SET) / PCB_SIZE;

    thread->stack = (int *)((int)thread + PAGE_SIZE - sizeof(ProcessStartStack));
    thread->stack -= 7;
    thread->stack[0] = 0;
    thread->stack[1] = 0;
    thread->stack[2] = 0;
    thread->stack[3] = 0;
    thread->stack[4] = (int)func;
    thread->stack[5] = (int)program_exit;
    thread->stack[6] = (int)parameter;

    li_push_back(&(pm->all_programs), &(thread->tag_in_all_list));
    li_push_back(&(pm->ready_programs), &(thread->tag_in_general_list));

    set_interrupt_status(&interrupt_maneger, status);

    return thread->pid;
}

void schedule(ProgramManeger *pm) {
    bool status = get_interrupt_status(&interrupt_maneger);
    disable_interrupt();

    if (li_size(&(pm->ready_programs)) == 0) {
        set_interrupt_status(&interrupt_maneger, status);
        return;
    }

    if (pm->running->status == RUNNING) {
        pm->running->status = READY;
        pm->running->ticks = pm->running->priority * 10;
        li_push_back(&(pm->ready_programs), &(pm->running->tag_in_general_list));
    } else if (pm->running->status == DEAD) {
        if (!pm->running->page_directory_addr) {
            release_pcb(pm->running);
        } else {
            ListItem* item = li_front(&pm->all_programs)->next;
            PCB *temp;
            Bool hasParent = false; 
            while (item) {
                temp = ListItem2PCB(item, tag_in_all_list);
                if (temp->pid == pm->running->parent_pid) {
                    hasParent = true;
                    break;
                }
            }

            if (!hasParent) {
                release_pcb(pm->running);
            }
        }
    }

    ListItem *item = li_front(&(pm->ready_programs));
    PCB *next = ListItem2PCB(item, tag_in_general_list);
    PCB *cur = pm->running;

    next->status = RUNNING;
    pm->running = next;
    li_pop_front(&(pm->ready_programs));

    activate_program_page(pm, next);

    asm_switch_thread(cur, next);

    set_interrupt_status(&interrupt_maneger, status);
}

void program_exit() {
    PCB *thread = program_maneger.running;
    thread->status = DEAD;

    if (thread->pid) {
        schedule(&program_maneger);
    } else {
        disable_interrupt();
        asm_halt();
    }
}

void mesa_wakeup(ProgramManeger *pm, PCB *pcb) {
    pcb->status = READY;
    li_push_front(&pm->ready_programs, &(pcb->tag_in_general_list));
}

void tss_init() {
    int size = sizeof(TSS);
    int address = (int)&tss;

    memset((char *)address, 0, size);
    tss.ss0 = STACK_SELECTOR;

    int low, high, limit;

    limit = size - 1;
    low = (address << 16) | (limit & 0xff);
    high = (address & 0xff000000) | ((address & 0x00ff0000) >> 16) | ((limit & 0xff00) << 16) | 0x00008900;

    int selector = asm_add_global_descriptor(low, high);

    asm_ltr(selector << 3);
    tss.ioMap = address + size;
}

int execute_process(ProgramManeger *pm, const char *filename, int priority) {
    Bool status = get_interrupt_status(&interrupt_maneger);
    disable_interrupt();

    int pid = execute_thread(pm, (ThreadFunc)load_process, (void *)filename, filename, priority);

    if (pid == -1) {
        set_interrupt_status(&interrupt_maneger, status);
        return -1;
    }

    PCB *process = ListItem2PCB(li_back(&pm->all_programs), tag_in_all_list);

    process->page_directory_addr = create_process_page_directory(pm);

    if (!process->page_directory_addr) {
        process->status = DEAD;
        set_interrupt_status(&interrupt_maneger, status);
        return -1;
    }

    Bool res = create_user_virtual_pool(pm, process);

    if (!res) {
        process->status = DEAD;
        set_interrupt_status(&interrupt_maneger, status);
        return -1;
    }

    set_interrupt_status(&interrupt_maneger, status);
    return pid;
}

int create_process_page_directory(ProgramManeger *pm) {
    int vaddr = mm_allocate_pages(&memory_maneger, KERNEL, 1);
    if (!vaddr) {
        return 0;
    }

    memset((char *)vaddr, 0, PAGE_SIZE);

    int *src = (int *)(0xfffff000 + 0x300 * 4);
    int *dst = (int *)(vaddr + 0x300 * 4);
    for (int i = 0; i < 256; ++i) {
        dst[i] = src[i];
    }

    ((int*)vaddr)[1023] = vaddr2paddr(vaddr) | 0x7;

    return vaddr;
}

Bool create_user_virtual_pool(ProgramManeger *pm, PCB *process) {
    int sources_count = (0xc0000000 - USER_VADDR_START) / PAGE_SIZE;
    int bitmap_length = ceil(sources_count, 8);

    int pages_count = ceil(bitmap_length, PAGE_SIZE);

    int start = mm_allocate_pages(&memory_maneger, KERNEL, pages_count);

    if (!start) {
        return false;
    }

    memset((char *)start, 0, PAGE_SIZE * pages_count);
    ap_init(&process->user_virtual, (char *)start, bitmap_length, USER_VADDR_START);

    return true;
}

void load_process(const char *filename) {
    disable_interrupt(&interrupt_maneger);

    PCB *process = program_maneger.running;
    ProcessStartStack *interrupt_stack =
        (ProcessStartStack *)((int)process + PAGE_SIZE - sizeof(ProcessStartStack));

    interrupt_stack->edi = 0;
    interrupt_stack->esi = 0;
    interrupt_stack->ebp = 0;
    interrupt_stack->esp_dummy = 0;
    interrupt_stack->ebx = 0;
    interrupt_stack->edx = 0;
    interrupt_stack->ecx = 0;
    interrupt_stack->eax = 0;
    interrupt_stack->gs = 0;

    interrupt_stack->fs = program_maneger.USER_DATA_SELECTOR;
    interrupt_stack->es = program_maneger.USER_DATA_SELECTOR;
    interrupt_stack->ds = program_maneger.USER_DATA_SELECTOR;

    interrupt_stack->eip = (int)filename;
    interrupt_stack->cs = program_maneger.USER_CODE_SELECTOR;
    interrupt_stack->eflags = (0 << 12) | (1 << 9) | (1 << 1);

    interrupt_stack->esp = mm_allocate_pages(&memory_maneger, USER, 1);

    if (interrupt_stack->esp == 0) {
        printf("cannot build process due to memory full.");
        process->status = DEAD;
        asm_halt();
    }

    interrupt_stack->esp += PAGE_SIZE;

    int *user_stack = (int *)interrupt_stack->esp;
    user_stack -= 3;
    user_stack[0] = (int)exit;
    user_stack[1] = 0;
    user_stack[2] = 0;

    interrupt_stack->esp = (int)user_stack;

    interrupt_stack->ss = program_maneger.USER_STACK_SELECTOR;

    asm_start_process((int)interrupt_stack);
}

void activate_program_page(ProgramManeger *pm, PCB *program) {
    int paddr = PAGE_DIRECTORY;

    if (program->page_directory_addr) {
        tss.esp0 = (int)program + PAGE_SIZE;
        paddr = vaddr2paddr(program->page_directory_addr);
    }

    asm_update_cr3(paddr);
}

int pm_fork(ProgramManeger *pm) {
    Bool status = get_interrupt_status(&interrupt_maneger);
    disable_interrupt();

    PCB *parent = pm->running;
    if (!parent->page_directory_addr) {
        set_interrupt_status(&interrupt_maneger, status);
        return -1;
    }

    int pid = execute_process(pm, "", 0);
    if (pid == -1) {
        set_interrupt_status(&interrupt_maneger, status);
        return -1;
    }
    
    PCB *child = ListItem2PCB(li_back(&pm->all_programs), tag_in_all_list);
    Bool flag = copy_process(pm, parent, child);
    
    if (!flag) {
        child->status = DEAD;
        set_interrupt_status(&interrupt_maneger, status);
        return -1;
    }

    set_interrupt_status(&interrupt_maneger, status);
    return pid;
}

Bool copy_process(ProgramManeger *pm, PCB *parent, PCB *child) {
    ProcessStartStack *child_pss = 
        (ProcessStartStack *)((int)child + PAGE_SIZE - sizeof(ProcessStartStack));
    ProcessStartStack *parent_pss =
        (ProcessStartStack *)((int)parent + PAGE_SIZE - sizeof(ProcessStartStack));
    memcpy(parent_pss, child_pss, sizeof(ProcessStartStack));
    child_pss->eax = 0;
    child->stack = (int *)child_pss - 7;
    child->stack[0] = 0;
    child->stack[1] = 0;
    child->stack[2] = 0;
    child->stack[3] = 0;
    child->stack[4] = (int)asm_start_process;
    child->stack[5] = 0;    // return value of asm_start_process
    child->stack[6] = (int)child_pss; // argument of asm_start_process

    child->status = READY;
    child->parent_pid = parent->pid;
    child->priority = parent->priority;
    child->ticks = parent->ticks;
    child->ticks_passed_by = parent->ticks_passed_by;
    strcpy(parent->name, child->name);
    // duplicate user virtual address pool
    int bitmap_length = parent->user_virtual.resources.length;
    int bitmap_bytes = ceil(bitmap_length, 8);
    memcpy(parent->user_virtual.resources.bitmap, child->user_virtual.resources.bitmap, bitmap_bytes);

    // allocate a page in memory as buffer
    char *buffer = (char *)mm_allocate_pages(&memory_maneger, KERNEL, 1);
    if (!buffer) {
        child->status = DEAD;
        return false;
    }

    int child_page_dir_paddr = vaddr2paddr(child->page_directory_addr);
    int parent_page_dir_paddr = vaddr2paddr(parent->page_directory_addr);
    int *child_page_dir = (int *)child->page_directory_addr;
    int *parent_page_dir = (int *)parent->page_directory_addr;

    memset((void *)child->page_directory_addr, 0, 768 * 4);
    for (int i = 0; i < 768; ++i) {
        if (!(parent_page_dir[i] & 0x1)) {
            continue;
        }

        int paddr = mm_allocate_physical_pages(&memory_maneger, USER, 1);
        if (!paddr) {
            child->status = DEAD;
            return false;
        }

        int pde = parent_page_dir[i];
        int *page_table_vaddr = (int *)(0xffc00000 + (i << 12));

        asm_update_cr3(child_page_dir_paddr);

        child_page_dir[i] = (pde & 0x00000fff) | paddr;
        memset(page_table_vaddr, 0, PAGE_SIZE);

        asm_update_cr3(parent_page_dir_paddr);
    }

    for (int i = 0; i < 768; ++i) {
        if (!(parent_page_dir[i] & 0x01)) {
            continue;
        }

        int *page_table_vaddr = (int *)(0xffc00000 + (i << 12));

        for (int j = 0; j < 1024; ++j) {
            if (!(page_table_vaddr[j] & 0x1)) {
                continue;
            }

            int paddr = mm_allocate_physical_pages(&memory_maneger, USER, 1);
            if (!paddr) {
                child->status = DEAD;
                return false;
            }
            void *page_vaddr = (void *)((i << 22) + (j << 12));
            int pte = page_table_vaddr[j];
            memcpy(page_vaddr, buffer, PAGE_SIZE);
            asm_update_cr3(child_page_dir_paddr);
            page_table_vaddr[j] = (pte & 0x00000fff) | paddr;
            memcpy(buffer, page_vaddr, PAGE_SIZE);

            asm_update_cr3(parent_page_dir_paddr);
        }
    }

    mm_release_pages(&memory_maneger, KERNEL, (int)buffer, 1);
    return true;
}

void pm_exit(ProgramManeger *pm, int ret) {
    disable_interrupt();
    PCB *program = pm->running;
    program->ret_value = ret;
    program->status = DEAD;

    int *page_dir, *page;
    int paddr;

    if (program->page_directory_addr) {
        page_dir = (int *)program->page_directory_addr;
        for (int i = 0; i < 768; ++i) {
            if (!(page_dir[i] & 0x01)) {
                continue;
            }
            page = (int *)(0xffc00000 + (i << 12));

            for (int j = 0; j < 1024; ++j) {
                if (!(page[j] & 0x01)) {
                    continue;
                }

                paddr = vaddr2paddr((i << 22) + (j << 12));
                mm_release_physical_pages(&memory_maneger, USER, paddr, 1);
            }
            paddr = vaddr2paddr((int)page);
            mm_release_physical_pages(&memory_maneger, USER, paddr, 1);
        }

        mm_release_pages(&memory_maneger, KERNEL, (int)page_dir, 1);
        int bitmap_bytes = ceil(program->user_virtual.resources.length, 8);
        int bitmap_pages = ceil(bitmap_bytes, PAGE_SIZE);
        mm_release_pages(&memory_maneger, KERNEL, (int)program->user_virtual.resources.bitmap, bitmap_pages);
    }
    schedule(pm);
}

int pm_wait(ProgramManeger *pm, int *retval) {
    PCB *child;
    ListItem *item;
    Bool interrupt, flag;

    while (true) {
        interrupt = get_interrupt_status(&interrupt_maneger);
        disable_interrupt();
        item = li_front(&pm->all_programs)->next;

        flag = true;
        while (item) {
            child = ListItem2PCB(item, tag_in_all_list);
            if (child->parent_pid == pm->running->pid) {
                flag = false;
                if (child->status == DEAD) {
                    break;
                }
            }
            item = item->next;
        }
        
        if (item) {
            if (retval) {
                *retval = child->ret_value;
            }
            int pid = child->pid;
            release_pcb(child);
            set_interrupt_status(&interrupt_maneger, interrupt);
            return pid;
        } else {
            if (flag) {
                set_interrupt_status(&interrupt_maneger, interrupt);
                return -1;
            } else {
                set_interrupt_status(&interrupt_maneger, interrupt);
                schedule(pm);
            }
        }
    }
}
