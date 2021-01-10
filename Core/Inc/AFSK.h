/*
 * AFSK.h
 *
 *  Created on: Jul 13, 2020
 *      Author: matthewtran
 */

#ifndef INC_AFSK_H_
#define INC_AFSK_H_

#include "main.h"
#include <stdbool.h>
#include <malloc.h>

#include "BitFIFO.h"
#include "IntFIFO.h"
#include "APRS.h"

extern DAC_HandleTypeDef hdac1;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim6;

extern bool AFSK_sending;

void AFSK_init();
bool AFSK_send(BitFIFO *bfifo);

// private helpers
void AFSK_send_fillbuff(uint8_t *buff, uint32_t samples);
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac);
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac);

void AFSK_decode_sig(uint8_t *sig_buff, uint32_t len);
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);

#endif /* INC_AFSK_H_ */
