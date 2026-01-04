/***************************************************************************
 * Module:	AOSL simple bitmap implementations.
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/
#include <string.h>
#include <api/aosl_mm.h>
#include <kernel/bitmap.h>
#include <kernel/bug.h>

#define SIZE_OF_UINT64(bit_cnt) ((bit_cnt + sizeof(uint64_t) - 1) / sizeof(uint64_t))

bitmap_t* bitmap_create(uint8_t bit_cnt)
{
	if (bit_cnt == 0) {
		return NULL;
	}

	int cnt_64 = SIZE_OF_UINT64(bit_cnt);
	bitmap_t *self = (bitmap_t *)aosl_malloc_impl(sizeof(bitmap_t));
	if (!self) {
		return NULL;
	}
	memset(self, 0, sizeof(bitmap_t));

	self->bit_arr = (uint64_t *)aosl_malloc_impl(sizeof(uint64_t) * cnt_64);
	if (!self->bit_arr) {
		goto __tag_failed;
	}
	memset(self->bit_arr, 0, sizeof(uint64_t) * cnt_64);
	self->bit_arr_cnt = cnt_64;
	self->bit_cnt = bit_cnt;
	return self;

__tag_failed:
	bitmap_destroy(self);
	return NULL;
}

void bitmap_destroy(bitmap_t *self)
{
	if (!self) {
		return;
	}

	if (self->bit_arr) {
		aosl_free(self->bit_arr);
		self->bit_arr = NULL;
	}
	aosl_free(self);
}

void bitmap_set(bitmap_t *self, uint8_t i)
{
	if(!self || i >= self->bit_cnt) {
		return;
	}

	int index = i / sizeof(uint64_t);
	int offset = i % sizeof(uint64_t);

	self->bit_arr[index] |= ((uint64_t)1 << offset);
}

void bitmap_clear(bitmap_t *self, uint8_t i)
{
	if(!self || i >= self->bit_cnt) {
		return;
	}

	int index = i / sizeof(uint64_t);
	int offset = i % sizeof(uint64_t);

	self->bit_arr[index] &= ~((uint64_t)1 << offset);
}

void bitmap_reset(bitmap_t *self)
{
	BUG_ON(!self);
	if (!self->bit_arr || !self->bit_arr_cnt) {
		return;
	}
	memset(self->bit_arr, 0, sizeof(uint64_t) * self->bit_arr_cnt);
}

bool bitmap_get(bitmap_t *self, uint8_t i)
{
	BUG_ON(!self);
	BUG_ON(i > self->bit_cnt);

	int index = i / sizeof(uint64_t);
	int offset = i % sizeof(uint64_t);

	return (self->bit_arr[index] & ((uint64_t)1 << offset)) != 0;
}

void bitmap_copy(bitmap_t *self, bitmap_t *src)
{
	BUG_ON(!self || !src);
	BUG_ON(self->bit_cnt < src->bit_cnt);

	bitmap_reset(self);

	memcpy(self->bit_arr, src->bit_arr, sizeof(uint64_t) * src->bit_arr_cnt);
}

int bitmap_find_first_zero_bit(bitmap_t *self)
{
	BUG_ON(!self);

	unsigned int i, j;
	int bitpos = 0;
	for (i = 0; i < self->bit_arr_cnt; i++) {
		for (j = 0; j < sizeof(uint64_t)*8 && bitpos <= self->bit_cnt; j++) {
			if (!(self->bit_arr[i] & ((uint64_t)1 << j))) {
				return bitpos;
			}
			bitpos++;
		}
	}

	return -1;
}
