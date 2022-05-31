#include "memory_maneger.h"
#include "std_io.h"
#include "asm_utils.h"
#include "os_constant.h"
#include "std_lib.h"
#include "os_modules.h"

void mm_init(MemoryManeger *mm) {
    mm->total_memory = 0;
    mm->total_memory = mm_get_total_memory(mm);

    int used_memory = 256 * PAGE_SIZE + 0x100000;

    if (mm->total_memory < used_memory) {
        printf("memory is too small, halt.\n");
        asm_halt();
    }

    int free_memory = mm->total_memory - used_memory;

    int free_pages = free_memory / PAGE_SIZE;
    int kernel_pages = free_pages / 2;
    int user_pages = free_pages - kernel_pages;

    int kernel_phy_start_addr = used_memory;
    int user_phy_start_addr = used_memory + kernel_pages * PAGE_SIZE;

    int kernel_phy_bitmap_start = BITMAP_START_ADDRESS;
    int user_phy_bitmap_start = kernel_phy_bitmap_start + ceil(kernel_pages, 8);
    int kernel_vir_bitmap_start = user_phy_bitmap_start + ceil(user_pages, 8);
    
    ap_init(&mm->kernel_phy, (char *)kernel_phy_bitmap_start, kernel_pages, kernel_phy_start_addr);
    ap_init(&mm->user_phy, (char *)user_phy_bitmap_start, user_pages, user_phy_start_addr);
    ap_init(&mm->kernel_vir, (char *)kernel_vir_bitmap_start, kernel_pages, KERNEL_VIRTUAL_START); 

    printf("total memory: %d bytes (%d MiB)\n",
        mm->total_memory,
        mm->total_memory / 1024 / 1024);

    printf("kernel pool\n"
           "    start addr: 0x%x\n"
           "    total pages: %d (%d MiB)\n"
           "    bitmap start addr: 0x%x\n",
           kernel_phy_start_addr,
           kernel_pages, kernel_pages * PAGE_SIZE / 1024 / 1024,
           kernel_phy_bitmap_start);

    printf("user pool\n"
           "    start addr: 0x%x\n"
           "    total pages: %d (%d MiB)\n"
           "    bitmap start addr: 0x%x\n",
           user_phy_start_addr,
           user_pages, user_pages * PAGE_SIZE / 1024 / 1024,
           user_phy_bitmap_start);

    printf("kernel virtual pool\n"
           "    start addr: 0x%x\n"
           "    total pages: %d (%d MiB)\n"
           "    bitmap start addr: 0x%x\n",
           KERNEL_VIRTUAL_START,
           user_pages, kernel_pages * PAGE_SIZE / 1024 / 1024,
           kernel_vir_bitmap_start);
}

int mm_allocate_physical_pages(MemoryManeger *mm, enum AddressPoolType type, int count) {
    int start = -1;

    if (type == KERNEL) {
        start = ap_allocate(&mm->kernel_phy, count);
    } else if (type == USER) {
        start = ap_allocate(&mm->user_phy, count);
    }

    return (start == -1) ? 0:start;
}

void mm_release_physical_pages(MemoryManeger *mm, enum AddressPoolType type, int paddr, int count) {
    if (type == KERNEL) {
        ap_release(&mm->kernel_phy, paddr, count);
    } else if (type == USER) {
        ap_release(&mm->user_phy, paddr, count);
    }
}

int mm_get_total_memory(MemoryManeger *mm) {
    if (!mm->total_memory) {
        int memory = *((int*)MEMORY_SIZE_ADDRESS);
        int low = memory & 0xffff;
        int high = (memory >> 16) & 0xffff;

        mm->total_memory = low * 1024 + high * 64 * 1024;
    }

    return mm->total_memory;
}

int mm_allocate_pages(MemoryManeger *mm, enum AddressPoolType type, int count) {
    int virtual_address = mm_allocate_virtual_pages(mm, type, count);

    if (!virtual_address) {
        return 0;
    }

    Bool flag;
    int phy_page_addr;
    int vaddr = virtual_address;

    for (int i = 0; i < count; ++i, vaddr += PAGE_SIZE) {
        flag = 0;
        phy_page_addr = mm_allocate_physical_pages(mm, type, 1);
        if (phy_page_addr) {
            flag = mm_connect_phy_vir_pages(mm, vaddr, phy_page_addr);
        } else {
            flag = 0;
        }

        if (!flag) {
            mm_release_pages(mm, type, virtual_address, i);
            mm_release_virtual_pages(mm, type, virtual_address + i * PAGE_SIZE, count - i);
            return 0;
        }
    }
    
    return virtual_address;
}

int mm_allocate_virtual_pages(MemoryManeger *mm, enum AddressPoolType type, int count) {
    int start = -1;
    
    if (type == KERNEL) {
        start = ap_allocate(&mm->kernel_vir, count);
    } else if (type == USER) {
        start = ap_allocate(&program_maneger.running->user_virtual, count);
    }

    return (start == -1) ? 0:start;
}

Bool mm_connect_phy_vir_pages(MemoryManeger *mm, int vaddr, int paddr) {
    int *pde = (int *)to_pde(vaddr);
    int *pte = (int *)to_pte(vaddr);

    if (!(*pde & 0x00000001)) {
        int page = mm_allocate_physical_pages(mm, KERNEL, 1);

        if (!page) {
            return false;
        }

        *pde = page | 0x7;

        char *page_ptr = (char *)(((int)pte) & 0xfffff000);
        memset(page_ptr, 0, PAGE_SIZE);
    }
    *pte = paddr | 0x7;

    return true;
}

int to_pde(int vaddr) {
    return (0xfffff000 + (((vaddr & 0xffc00000) >> 22) * 4));
}

int to_pte(int vaddr) {
    return (0xffc00000 + ((vaddr & 0xffc00000) >> 10) + (((vaddr & 0x003ff000) >> 12) * 4));
}

void mm_release_pages(MemoryManeger *mm, enum AddressPoolType type, int vaddr, int count) {
    int vaddr_ = vaddr;
    int *pte;

    for (int i = 0; i < count; ++i, vaddr_ += PAGE_SIZE) {
        mm_release_physical_pages(mm, type, vaddr2paddr(vaddr_), 1);

        pte = (int *)to_pte(vaddr);
        *pte = 0;
    }

    mm_release_virtual_pages(mm, type, vaddr, count);
}

int vaddr2paddr(int vaddr) {
    int *pte = (int *)to_pte(vaddr);
    int page = (*pte) & 0xfffff000;
    int offset = vaddr & 0xfff;
    return (page + offset);
}

void mm_release_virtual_pages(MemoryManeger *mm, enum AddressPoolType type, int vaddr, int count) {
    if (type == KERNEL) {
        ap_release(&mm->kernel_vir, vaddr, count);
    } else if (type == USER) {
        ap_release(&program_maneger.running->user_virtual, vaddr, count);
    }
}

