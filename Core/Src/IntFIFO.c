/*
 * IntFIFO.c
 *
 *  Created on: Jul 19, 2020
 *      Author: matthewtran
 */

#include "IntFIFO.h"

void IntFIFO_init(IntFIFO *ififo, int32_t *arr, uint32_t size_bytes) {
	ififo->arr = arr;
	ififo->size = size_bytes >> 2;
	ififo->start = 0;
	ififo->end = 0;
}

bool IntFIFO_is_empty(IntFIFO *ififo) {
	return ififo->start == ififo->end;
}

void IntFIFO_push(IntFIFO *ififo, int32_t val) {
	ififo->arr[ififo->end] = val;
	ififo->end = (ififo->end + 1) % ififo->size;

#ifdef INTFIFO_DEBUG
	if (ififo->start == ififo->end) { // wrapped around, so is full
		Error_Handler();
	}
#endif
}

int32_t IntFIFO_pop(IntFIFO *ififo) {
#ifdef INTFIFO_DEBUG
	if (ififo->start == ififo->end) {
		Error_Handler();
	}
#endif

	int32_t val = ififo->arr[ififo->start];
	ififo->start = (ififo->start + 1) % ififo->size;
	return val;
}
