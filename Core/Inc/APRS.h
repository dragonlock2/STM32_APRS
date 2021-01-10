/*
 * APRS.h
 *
 *  Created on: Jul 15, 2020
 *      Author: matthewtran
 */

#ifndef INC_APRS_H_
#define INC_APRS_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <main.h>
#include <malloc.h>

#include "BitFIFO.h"

typedef struct {
	char *dest;
	char *callsign;
	char *digi; // max 8 digipeaters
	char *info; // max 256 bytes
} APRSPacket;

// constants
extern APRSPacket APRS_DEFAULT_PACKET;

#define APRS_SIGN_LEN  10 // max 6-char sign + '-' + 2-digit ssid + '\0'
#define APRS_MAX_DIGI   8 // 56 byte digi field
#define APRS_MAX_INFO 256 // 256 byte info field

#define APRS_PREFLAGS   40
#define APRS_POSTFLAGS  40
#define APRS_FIFO_BYTES 512 // 40 pre/postflags, 2 flags, 330 bytes packet + bitstuffing, round up

#define APRS_MIN_BITS 144 // 7+7+1+1+2 = 18 bytes
#define APRS_MAX_BITS 2640 // 7+7+56+1+1+256+2 = 330 bytes

// variables
extern APRSPacket APRS_decoded_packet;
extern bool APRS_decoded_packet_valid;

// public functions
void APRS_init();
void APRS_print(APRSPacket *pack);
void APRS_encode(BitFIFO *bfifo, APRSPacket *pack);
void APRS_decode(uint8_t *bites, uint32_t len);

// private helpers
void APRS_update_fcs(uint8_t b, uint16_t *crc);

void APRS_encode_insert_byte(BitFIFO *bfifo, uint8_t b);
void APRS_encode_stuff_byte(BitFIFO *bfifo, uint8_t b, uint8_t *stuff_count, bool *curr_nrzi);
void APRS_encode_stuff_byte_update_fcs(BitFIFO *bfifo, uint8_t b, uint8_t *stuff_count, bool *curr_nrzi, uint16_t *crc);
void APRS_encode_insert_callsign(BitFIFO *bfifo, char *sign, uint8_t *stuff_count, bool *curr_nrzi, uint16_t *crc, bool end);

bool APRS_decode_callsign(uint8_t *bites, char *dest);
void APRS_decode_update(bool bit);

// external
extern void Error_Handler(void);
extern void APRSGUI_addpack(APRSPacket *pack);

#endif /* INC_APRS_H_ */
