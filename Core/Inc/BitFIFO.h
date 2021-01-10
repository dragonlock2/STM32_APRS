/*
 * BitFIFO.h
 *
 *  Created on: Jul 14, 2020
 *      Author: matthewtran
 */

#ifndef INC_BITFIFO_H_
#define INC_BITFIFO_H_

#include <stdint.h>
#include <stdbool.h>

//#define BITFIFO_DEBUG // enables calling Error_Handler() if out of bounds

typedef struct {
	uint8_t *arr;
	uint32_t size; // in bits, actual max size is size-1
	uint32_t start, end;
} BitFIFO;

void BitFIFO_init(BitFIFO *bfifo, uint8_t *arr, uint32_t size_bytes);
void BitFIFO_reinit(BitFIFO *bfifo);
bool BitFIFO_is_empty(BitFIFO *bfifo);
void BitFIFO_push(BitFIFO *bfifo, uint8_t val);
uint8_t BitFIFO_pop(BitFIFO *bfifo); // careful returns either 0 or nonzero

#ifdef BITFIFO_DEBUG
extern void Error_Handler(void);
#endif

#endif /* INC_BITFIFO_H_ */
