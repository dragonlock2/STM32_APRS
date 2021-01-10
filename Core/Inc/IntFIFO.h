/*
 * IntFIFO.h
 *
 *  Created on: Jul 19, 2020
 *      Author: matthewtran
 */

#ifndef INC_INTFIFO_H_
#define INC_INTFIFO_H_

// should really use abstraction and make one FIFO class

#include <stdint.h>
#include <stdbool.h>

//#define INTFIFO_DEBUG // enables calling Error_Handler() if out of bounds

typedef struct {
	int32_t *arr;
	uint32_t size; // actual max size is size-1
	uint32_t start, end;
} IntFIFO;

void IntFIFO_init(IntFIFO *ififo, int32_t *arr, uint32_t size_bytes);
bool IntFIFO_is_empty(IntFIFO *ififo);
void IntFIFO_push(IntFIFO *ififo, int32_t val);
int32_t IntFIFO_pop(IntFIFO *ififo);

#ifdef INTFIFO_DEBUG
extern void Error_Handler(void);
#endif

#endif /* INC_INTFIFO_H_ */
