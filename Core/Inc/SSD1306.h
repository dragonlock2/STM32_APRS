/*
 * SSD1306.h
 *
 *  Created on: Jul 22, 2020
 *      Author: matthewtran
 */

#ifndef INC_SSD1306_H_
#define INC_SSD1306_H_

#include "main.h"
#include <stdbool.h>
#include <string.h>

#define SSD1306_PORT   hi2c1
#define SSD1306_ADDR    0x78 // 0x3C << 1
#define SSD1306_WIDTH    128
#define SSD1306_HEIGHT    64
#define SSD1306_FONT_W     4
#define SSD1306_FONT_H     6

// public functions
void SSD1306_init();
void SSD1306_set_cursor(uint16_t x, uint16_t y);
void SSD1306_draw_string(char *s, uint32_t len);
void SSD1306_draw_char(char c);
void SSD1306_draw_pixel(uint16_t x, uint16_t y, bool on);
void SSD1306_clear_display();

// private helpers
void SSD1306_write_command(uint8_t comm);
void SSD1306_TIM_callback(); // PLACE IN stm32l4xx_it.c

// external
extern TIM_HandleTypeDef htim7;
extern I2C_HandleTypeDef SSD1306_PORT;

#endif /* INC_SSD1306_H_ */
