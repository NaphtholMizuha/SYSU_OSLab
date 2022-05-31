#ifndef MEMORY_H
#define MEMORY_H
#include "address_pool.h"
enum AddressPoolType {
    USER,
    KERNEL
};

typedef struct {
    int total_memory;
    AddressPool kernel_phy;
    AddressPool user_phy;
    AddressPool kernel_vir;
} MemoryManeger;

void mm_init(MemoryManeger *mm);

int mm_allocate_physical_pages(MemoryManeger *mm, enum AddressPoolType type, int count);

void mm_release_physical_pages(MemoryManeger *mm, enum AddressPoolType type, int paddr, int count);

int mm_get_total_memory(MemoryManeger *mm);

int mm_allocate_pages(MemoryManeger *mm, enum AddressPoolType type, int count);

int mm_allocate_virtual_pages(MemoryManeger *mm, enum AddressPoolType type, int count);

void mm_release_pages(MemoryManeger *mm, enum AddressPoolType type, int vaddr, int count);

void mm_release_virtual_pages(MemoryManeger *mm, enum AddressPoolType type, int vaddr, int count);

int to_pde(int vaddr);

int to_pte(int vaddr);

Bool mm_connect_phy_vir_pages(MemoryManeger *mm, int vaddr, int paddr);

int vaddr2paddr(int vaddr);

#endif