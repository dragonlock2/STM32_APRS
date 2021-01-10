/*
 * SSD1306.c
 *
 *  Created on: Jul 22, 2020
 *      Author: matthewtran
 */

#include "SSD1306.h"

// font converted from https://blog.mclemon.io/hacking-a-tiny-new-font-for-the-ssd1306-128x64-oled-screen
const uint32_t font[128] = { // 4x6 font (including spaces) stored in lowest 24 bits, column major order
	0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,
	0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,
	0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,
	0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f7df,
	0x00000000,0x000005c0,0x00003003,0x0001f29f,0x000057ca,0x00012109,0x0001c5cf,0x000000c0,
	0x00011380,0x00000391,0x00005085,0x00004384,0x00000210,0x00004104,0x00000400,0x00003118,
	0x0000f45e,0x000007c2,0x00012559,0x0000a551,0x0001f107,0x00009557,0x0001d55e,0x00003159,
	0x0001f55f,0x0000f557,0x00000280,0x00000290,0x00011284,0x0000a28a,0x00004291,0x00003541,
	0x0001654e,0x0001e15e,0x0000a55f,0x0001144e,0x0000e45f,0x0001555f,0x0000515f,0x0001d54e,
	0x0001f11f,0x000117d1,0x0000f408,0x0001b11f,0x0001041f,0x0001f19f,0x0001f39f,0x0000e44e,
	0x0000215f,0x0001e64e,0x0001635f,0x00009552,0x000017c1,0x0001f40f,0x00007607,0x0001f31f,
	0x0001b11b,0x00003703,0x00013559,0x0001145f,0x00008102,0x0001f451,0x00002042,0x00010410,
	0x00000081,0x0001c59a,0x0000c49f,0x0001248c,0x0001f48c,0x0001668c,0x00005784,0x0001ea8c,
	0x0001c09f,0x00000740,0x0001d810,0x0001231f,0x000107d1,0x0001e39e,0x0001c09e,0x0000c48c,
	0x0000c4be,0x0003e48c,0x0000209c,0x0000a794,0x000127c2,0x0001e40e,0x0000e60e,0x0001e71e,
	0x00012312,0x0001ea06,0x0001679a,0x000116c4,0x000007c0,0x000046d1,0x00002184,0x0001f7df,
};

static uint8_t buff[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
static uint16_t my_x, my_y;

// public functions
void SSD1306_init() {
	SSD1306_write_command(0xAE); // display off
	SSD1306_write_command(0x00); // set lower column start address
	SSD1306_write_command(0x10); // set higher column start address
	SSD1306_write_command(0x20); // set memory addressing mode
	SSD1306_write_command(0x00); // - horizontal addressing mode
	SSD1306_write_command(0x40); // set display start line
	SSD1306_write_command(0x81); // set contrast control
	SSD1306_write_command(0xFF); // - max contrast
	SSD1306_write_command(0xA1); // set segment re-map
	SSD1306_write_command(0xA4); // entire display on (follows RAM)
	SSD1306_write_command(0xA6); // set normal display (non-inverted)
	SSD1306_write_command(0xA8); // set multiplex ratio
	SSD1306_write_command(0x3F); // - 63
	SSD1306_write_command(0xB0); // set page start address
	SSD1306_write_command(0xC8); // set COM output scan direction
	SSD1306_write_command(0xD3); // set display offset
	SSD1306_write_command(0x00); // - no offset
	SSD1306_write_command(0xD5); // set display clock divide ratio/oscillator frequency
	SSD1306_write_command(0xF0); // - divide ratio = 1, oscillator freq = 16 (max)
	SSD1306_write_command(0xD9); // set pre-charge period
	SSD1306_write_command(0x22); // - 34
	SSD1306_write_command(0xDA); // set COM pins hardware configuration
	SSD1306_write_command(0x12); // - idk actually
	SSD1306_write_command(0xDB); // set Vcomh
	SSD1306_write_command(0x20); // - 0.77xVcc
	SSD1306_write_command(0x8D); // charge pump setting
	SSD1306_write_command(0x14); // - enable
	SSD1306_write_command(0xAF); // display on

	my_x = 0;
	my_y = 0;
}

void SSD1306_set_cursor(uint16_t x, uint16_t y) {
	my_x = x;
	my_y = y;
}

void SSD1306_draw_string(char *s, uint32_t len) {
	for (uint32_t i = 0; i < len; i++) {
		SSD1306_draw_char(s[i]);
		my_x += SSD1306_FONT_W;
		if (my_x > SSD1306_WIDTH - SSD1306_FONT_W) {
			my_x = 0;
			my_y += SSD1306_FONT_H;
		}
	}
}

void SSD1306_draw_char(char c) {
	// definitely slower than using byte high fonts, but can squeeze more in
	if (c > 127 || c < 0) { // default should be unsigned char
		c = 0; // just draw this if non-supported char
	}

	uint32_t b = font[(uint32_t) c];
	for (uint8_t x = 0; x < SSD1306_FONT_W; x++) {
		for (uint8_t y = 0; y < SSD1306_FONT_H; y++) {
			SSD1306_draw_pixel(my_x + x, my_y + y, b & 0x1);
			b >>= 1;
		}
	}
}

void SSD1306_draw_pixel(uint16_t x, uint16_t y, bool on) {
//	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
//		return;
//	}

	if (on) {
		buff[x + y / 8 * SSD1306_WIDTH] |= 1 << (y % 8);
	} else {
		buff[x + y / 8 * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}

void SSD1306_clear_display() {
	memset(buff, 0x00, sizeof(buff));
}

// private helpers
void SSD1306_write_command(uint8_t comm) {
	HAL_I2C_Mem_Write(&SSD1306_PORT, SSD1306_ADDR, 0x00, 1, &comm, 1, 10);
}

void SSD1306_TIM_callback() {
	HAL_I2C_Mem_Write_DMA(&SSD1306_PORT, SSD1306_ADDR, 0x40, 1, buff, sizeof(buff));
}
