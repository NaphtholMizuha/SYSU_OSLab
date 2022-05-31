#include "address_pool.h"
#include "os_type.h"

void ap_init(AddressPool *ap, char* bitmap, int length, int start_addr) {
    bm_init(&ap->resources, bitmap, length);
    ap->start_addr = start_addr;
}

int ap_allocate(AddressPool *ap, int count) {
    uint32 start = bm_allocate(&ap->resources, count);
    return (start == -1) ? -1 : (start * PAGE_SIZE + ap->start_addr);
}

void ap_release(AddressPool *ap, int addr, int amount) {
    bm_release(&ap->resources, (addr - ap->start_addr) / PAGE_SIZE, amount);
}
