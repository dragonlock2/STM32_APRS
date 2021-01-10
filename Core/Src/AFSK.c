/*
 * AFSK.c
 *
 *  Created on: Jul 14, 2020
 *      Author: matthewtran
 */

#include "AFSK.h"

const uint8_t AFSK_SINE_LOOKUP[256] = {
	128, 131, 134, 137, 140, 143, 146, 149, 152, 156, 159, 162, 165, 168, 171, 174,
	176, 179, 182, 185, 188, 191, 193, 196, 199, 201, 204, 206, 209, 211, 213, 216,
	218, 220, 222, 224, 226, 228, 230, 232, 234, 236, 237, 239, 240, 242, 243, 245,
	246, 247, 248, 249, 250, 251, 252, 252, 253, 254, 254, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 254, 254, 253, 252, 252, 251, 250, 249, 248, 247,
	246, 245, 243, 242, 240, 239, 237, 236, 234, 232, 230, 228, 226, 224, 222, 220,
	218, 216, 213, 211, 209, 206, 204, 201, 199, 196, 193, 191, 188, 185, 182, 179,
	176, 174, 171, 168, 165, 162, 159, 156, 152, 149, 146, 143, 140, 137, 134, 131,
	128, 124, 121, 118, 115, 112, 109, 106, 103,  99,  96,  93,  90,  87,  84,  81,
	79,  76,  73,  70,  67,  64,  62,  59,  56,  54,  51,  49,  46,  44,  42,  39,
	37,  35,  33,  31,  29,  27,  25,  23,  21,  19,  18,  16,  15,  13,  12,  10,
	9,   8,   7,   6,   5,   4,   3,   3,   2,   1,   1,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   1,   1,   2,   3,   3,   4,   5,   6,   7,   8,
	9,  10,  12,  13,  15,  16,  18,  19,  21,  23,  25,  27,  29,  31,  33,  35,
	37,  39,  42,  44,  46,  49,  51,  54,  56,  59,  62,  64,  67,  70,  73,  76,
	79,  81,  84,  87,  90,  93,  96,  99, 103, 106, 109, 112, 115, 118, 121, 124
};

#define DAC_REST 127 // based on AFSK_SINE_LOOKUP

// REQUIREMENTS
// Timers used for DAC and ADC are set to overflow at FS
// DMA request for DMA set in circular mode

// constants
#define SEND_FREQ_SHIFT    5 // samples_per_bit = 1 << FREQ_SHIFT
#define RECV_FREQ_SHIFT    3
#define BAUD            1200 // bps
#define FREQ_MARK       1200 // Hz, 1
#define FREQ_SPACE      2200 // Hz, 0
#define RECV_DELAY         4 // k in sin(n)sin(n-k) decoding
#define ADC_REST         127 // center value of ADC

// derived constants
#define SEND_FS   (BAUD << SEND_FREQ_SHIFT) // Hz, BAUD << FREQ_SHIFT
#define RECV_FS   (BAUD << RECV_FREQ_SHIFT)
#define DPH_MARK                  134217728 // FREQ_MARK  / FS * (1 << 32) (using 2*pi rad = 2^32)
#define DPH_SPACE                 246065835 // FREQ_SPACE / FS * (1 << 32) (using 2*pi rad = 2^32)

// variables
static uint8_t send_buff[2048]; // DAC, needs to be multiple of 1 << (FREQ_MUL+1)
static uint32_t phase;
static BitFIFO *send_fifo;

static uint8_t recv_buff[2048]; // ADC, 0.11s latency, too large and packet might be overwritten
static int32_t prev_pll, pll;
static bool prev_bit, curr_nrzi;
#define SIG_FIFO_SIZE sizeof(int32_t) * (RECV_DELAY + 1) // bytes, 1 int32_t wasted
static IntFIFO sig_fifo;

static int32_t x_prev, x_curr; // x[n-1], x[n]
static int32_t y_prev, y_curr; // y[n-1], y[n]

// global variables
bool AFSK_sending;

// functions
void AFSK_init() {
	// Start sending
	HAL_TIM_Base_Start(&htim6);
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
	HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_8B_R, DAC_REST);
	AFSK_sending = false;
	// DSP variables
	prev_pll = 0;
	pll = 0;
	prev_bit = false;
	curr_nrzi = true;
	IntFIFO_init(&sig_fifo, malloc(SIG_FIFO_SIZE), SIG_FIFO_SIZE);
	for (uint32_t i = 0; i < RECV_DELAY; i++) {
		IntFIFO_push(&sig_fifo, 0);
	}
	// Start receiving
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) recv_buff, sizeof(recv_buff));
	HAL_TIM_Base_Start(&htim2);
}

bool AFSK_send(BitFIFO *bfifo) {
	if (AFSK_sending) {
		return false;
	}

	send_fifo = bfifo;
	phase = 0;
	AFSK_sending = 1;
	AFSK_send_fillbuff(send_buff, sizeof(send_buff));

	HAL_GPIO_WritePin(TX_PTT_GPIO_Port, TX_PTT_Pin, 1);
	HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*) send_buff, sizeof(send_buff), DAC_ALIGN_8B_R);

	return true;
}

// private helpers
void AFSK_send_fillbuff(uint8_t *buff, uint32_t samples) {
	uint32_t sig_index = 0;
	for (uint32_t i = 0; i < (samples >> SEND_FREQ_SHIFT); i++) {
		if (BitFIFO_is_empty(send_fifo)) {
			break;
		}

		uint32_t dph = BitFIFO_pop(send_fifo) ? DPH_MARK : DPH_SPACE;
		for (sig_index = i << SEND_FREQ_SHIFT; sig_index < ((i + 1) << SEND_FREQ_SHIFT); sig_index++) {
			phase += dph;
			buff[sig_index] = AFSK_SINE_LOOKUP[phase >> 24]; // 8-bit sine lookup
		}
	}

	for (; sig_index < samples; sig_index++) {
		buff[sig_index] = DAC_REST; // at rest level if no more bits to send
	}
}

void AFSK_decode_sig(uint8_t *sig_buff, uint32_t len) {
	for (uint32_t i = 0; i < len; i++) {
		// simplified demodulation and 1200Hz IIR filter, designed for 9600Hz sample rate
		int32_t sig = sig_buff[i] - ADC_REST;
		x_prev = x_curr;
		x_curr = (IntFIFO_pop(&sig_fifo) * sig) >> 2; // x[0]*0.29289322, precalculating
		y_prev = y_curr;
		y_curr = x_prev + x_curr + (y_prev >> 1); // x[0]*0.29289322 + x[-1]*0.29289322 + y[-1]*0.41421356
		bool bit = y_curr > 0;
		IntFIFO_push(&sig_fifo, sig);

		// PLL, NRZI2NRZ, sample
		if (bit != prev_bit) {
			pll >>= 1; // divide by 2 to nudge to 0
		}
		prev_pll = pll;
		pll += 1 << (32 - RECV_FREQ_SHIFT);
		if (pll < 0 && prev_pll > 0) { // overflow, so SAMPLE!
			APRS_decode_update(curr_nrzi == bit); // also do NRZI2NRZ while you at it
			curr_nrzi = bit;
		}
		prev_bit = bit;
	}
}

// sending interrupts
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
	if (!AFSK_sending) {
		HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
		HAL_DAC_Start(&hdac1, DAC_CHANNEL_1); // stopping DMA stops DAC too
		HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_8B_R, DAC_REST);
		HAL_GPIO_WritePin(TX_PTT_GPIO_Port, TX_PTT_Pin, 0);
	}

	AFSK_send_fillbuff(send_buff, sizeof(send_buff) >> 1);
	if (BitFIFO_is_empty(send_fifo)) {
		AFSK_sending = false;
	}
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
	if (!AFSK_sending) {
		HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
		HAL_DAC_Start(&hdac1, DAC_CHANNEL_1); // stopping DMA stops DAC too
		HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_8B_R, DAC_REST);
		HAL_GPIO_WritePin(TX_PTT_GPIO_Port, TX_PTT_Pin, 0);
	}

	AFSK_send_fillbuff(send_buff + (sizeof(send_buff) >> 1), sizeof(send_buff) >> 1);
	if (BitFIFO_is_empty(send_fifo)) {
		AFSK_sending = false;
	}
}

// receiving interrupts
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
	AFSK_decode_sig(recv_buff, sizeof(recv_buff) >> 1);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	AFSK_decode_sig(recv_buff + (sizeof(recv_buff) >> 1), sizeof(recv_buff) >> 1);
}
