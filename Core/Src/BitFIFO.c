/*
 * BitFIFO.c
 *
 *  Created on: Jul 14, 2020
 *      Author: matthewtran
 */

#include "BitFIFO.h"

// bitarray implementation from http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
static uint8_t BitFIFO_get(BitFIFO *bfifo, uint32_t i) {
	return bfifo->arr[i >> 3] & (1 << (i%8));
}

static void BitFIFO_set(BitFIFO *bfifo, uint32_t i, uint8_t val) {
	if (val) {
		bfifo->arr[i >> 3] |= 1 << (i%8);
	} else {
		bfifo->arr[i >> 3] &= ~(1 << (i%8));
	}
}

void BitFIFO_init(BitFIFO *bfifo, uint8_t *arr, uint32_t size_bytes) {
	bfifo->arr = arr;
	bfifo->size = size_bytes << 3;
	bfifo->start = 0;
	bfifo->end = 0;
}

void BitFIFO_reinit(BitFIFO *bfifo) {
	bfifo->start = 0;
	bfifo->end = 0;
}

bool BitFIFO_is_empty(BitFIFO *bfifo) {
	return bfifo->start == bfifo->end;
}

void BitFIFO_push(BitFIFO *bfifo, uint8_t val) {
	BitFIFO_set(bfifo, bfifo->end, val);
	bfifo->end = (bfifo->end + 1) % bfifo->size;

#ifdef BITFIFO_DEBUG
	if (bfifo->start == bfifo->end) { // wrapped around, so is full
		Error_Handler();
	}
#endif
}

uint8_t BitFIFO_pop(BitFIFO *bfifo) {
#ifdef BITFIFO_DEBUG
	if (bfifo->start == bfifo->end) {
		Error_Handler();
	}
#endif

	uint8_t val = BitFIFO_get(bfifo, bfifo->start);
	bfifo->start = (bfifo->start + 1) % bfifo->size;
	return val;
}
