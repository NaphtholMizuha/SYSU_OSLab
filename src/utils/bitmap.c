#include "bitmap.h"
#include "std_lib.h"

void bm_init(Bitmap *bm, char *bitmap, int length) {
    bm->bitmap = bitmap;
    bm->length = length;
    
    int bytes = ceil(length, 8);
    memset(bitmap, 0, bytes);

    // ih_init(&bm->heap);
    // qi_init(&bm->q);
}

Bool bm_get(Bitmap *bm, int index) {
    int pos = index / 8;
    int offset = index % 8;

    return (bm->bitmap[pos] & (1 << offset));
}

void bm_set(Bitmap *bm, int index, Bool status) {
    int pos = index / 8;
    int offset = index % 8;

    bm->bitmap[pos] = bm->bitmap[pos] & (~(1 << offset));

    if (status) {
        bm->bitmap[pos] = bm->bitmap[pos] | (1 << offset);
    }
}

int bm_allocate(Bitmap *bm, int count) {
    if (count == 0) {
        return -1;
    }

    int index, empty, start;

    index = 0;
    while (index < bm->length) {
        while (index < bm->length && bm_get(bm, index)) {
            ++index;
        }

        if (index == bm->length) {
            return -1;
        }

        empty = 0;
        start = index;

        while ((index < bm->length) && (!bm_get(bm, index)) && (empty < count)) {
            ++empty;
            ++index;
        }

        if (empty == count) {
            for (int i = 0; i < count; ++i) {
                bm_set(bm, start + i, true);
            }

            return start;
        }
    }
    return -1;
}

void bm_release(Bitmap *bm, int index, int count) {
    for (int i = 0; i < count; ++i) {
        bm_set(bm, index + i, false);
    }
}

int bm_size(Bitmap *bm) {
    return bm->length;
}

