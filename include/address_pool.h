#ifndef ADDRESS_POOL_H
#define ADDRESS_POOL_H
#include "bitmap.h"
typedef struct {
    Bitmap resources;
    int start_addr;
} AddressPool;

void ap_init(AddressPool *ap, char* bitmap, int length, int start_addr);

int ap_allocate(AddressPool *ap, int count);

void ap_release(AddressPool *ap, int addr, int amount);

#endif