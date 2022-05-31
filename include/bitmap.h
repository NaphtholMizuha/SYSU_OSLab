#ifndef BITMAP_H
#define BITMAP_H
#include "os_constant.h"

typedef struct {
    int length;
    char *bitmap;
} Bitmap;

void bm_init(Bitmap *bm, char *bitmap, int length);

Bool bm_get(Bitmap *bm, int index);

void bm_set(Bitmap *bm, int index, Bool status);

int bm_size(Bitmap *bm);

int bm_allocate(Bitmap *bm, int count);

void bm_release(Bitmap *bm, int index, int count);

#endif