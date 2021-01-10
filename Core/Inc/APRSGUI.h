/*
 * APRS_GUI.h
 *
 *  Created on: Jul 24, 2020
 *      Author: matthewtran
 */

#ifndef INC_APRSGUI_H_
#define INC_APRSGUI_H_

#include <string.h>

#include "main.h"
#include "AFSK.h"
#include "APRS.h"
#include "SSD1306.h" // assumes 128x64 display and 4x6 font

enum APRSGUI_CONTROL {
	APRSGUI_UP = 1, // repurposing non-printable ASCII
	APRSGUI_DOWN,
	APRSGUI_LEFT,
	APRSGUI_RIGHT,
	APRSGUI_CENTER,
	APRSGUI_SCROLL_LEFT,
	APRSGUI_SCROLL_RIGHT,
	APRSGUI_DELETE
};

#define APRSGUI_DEFAULT_CURSOR '_'
extern char APRSGUI_cursor;

// public functions
void APRSGUI_init(APRSPacket *init_pack);
void APRSGUI_addpack(APRSPacket *pack);
void APRSGUI_render(); // PLACE IN stm32l4xx_it.c

void APRSGUI_char(char c); // user input

void APRSGUI_shift_view(); // call to make cursor visible

// private helpers
void APRSGUI_draw_tx_dest_src();
void APRSGUI_draw_cursor(bool sel);
void APRSGUI_draw_longstring(char *start, char *str, uint32_t str_idx, uint32_t str_len, bool sel, uint32_t tx_max_len);

void APRSGUI_up(); // change line
void APRSGUI_down();
void APRSGUI_left(); // change index
void APRSGUI_right();
void APRSGUI_center(); // send packet
void APRSGUI_scroll(bool right);
void APRSGUI_delete(); // backspace

void APRSGUI_char_helper(char *str, uint32_t *idx, uint32_t *len, uint32_t max_len, char c);
void APRSGUI_scroll_helper(uint32_t *idx, uint32_t *len, bool right);
void APRSGUI_delete_helper(char *str, uint32_t *idx, uint32_t *len);

#endif /* INC_APRSGUI_H_ */
