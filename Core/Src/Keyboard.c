/*
 * Keyboard.c
 *
 *  Created on: Aug 22, 2020
 *      Author: matthewtran
 */

#include "Keyboard.h"

// constants
#define NUM_PAGES 4
#define NUM_KEYS  10
#define MAX_SEQ   10

#define WAIT_DUR 40

#define HOLD_START_DUR 20
#define HOLD_DUR       7

static GPIO_TypeDef* const KEY_PORTS[] = {KEY_0_GPIO_Port, KEY_1_GPIO_Port, KEY_2_GPIO_Port,
                             KEY_3_GPIO_Port, KEY_4_GPIO_Port, KEY_5_GPIO_Port,
                             KEY_6_GPIO_Port, KEY_7_GPIO_Port, KEY_8_GPIO_Port,
                                              KEY_9_GPIO_Port};

static const uint16_t KEY_PINS[] = {KEY_0_Pin, KEY_1_Pin, KEY_2_Pin,
                       KEY_3_Pin, KEY_4_Pin, KEY_5_Pin,
                       KEY_6_Pin, KEY_7_Pin, KEY_8_Pin,
                                  KEY_9_Pin};

static char KEY_MAP[NUM_PAGES][NUM_KEYS][MAX_SEQ] = {
{">:",                  {APRSGUI_UP},     {APRSGUI_DELETE},
 {APRSGUI_LEFT},        {APRSGUI_CENTER}, {APRSGUI_RIGHT},
 {APRSGUI_SCROLL_LEFT}, {APRSGUI_DOWN},   {APRSGUI_SCROLL_RIGHT},
                        "-"
},
{"1`",    "2abc", "3def",
 "4ghi",  "5jkl", "6mno",
 "7pqrs", "8tuv", "9wxyz",
          "0 "
},
{"!~",    "@ABC", "#DEF",
 "$GHI",  "%JKL", "^MNO",
 "&PQRS", "*TUV", "(WXYZ",
		  ") "
},
{"-_", "+=", "\\|",
 "[{", "]}", ";:",
 ",<", ".>", "\'\"",
       "/?"
}
};

// vars
static int8_t prev_page_key = -1;

static int8_t prev_key = -1;
static int8_t curr_state = -1;
static int8_t curr_index = 0;

static uint32_t wait_dur = 0;

static uint32_t held_dur = 0;

uint8_t Keyboard_page = 0;

// public functions
void Keyboard_update() {
	// pages
	int8_t page_key = Keyboard_get_page_key();

	if (prev_page_key == -1 && page_key != -1) { // page key pressed
		if (curr_state != -1) {
			APRSGUI_char(KEY_MAP[Keyboard_page][curr_state][curr_index]);
			APRSGUI_cursor = APRSGUI_DEFAULT_CURSOR;
			curr_state = -1;
		}

		if (page_key == 0) {
			if (Keyboard_page == 0) {
				Keyboard_page = NUM_PAGES - 1;
			} else {
				Keyboard_page--;
			}
		} else { // page_key == 1
			if (Keyboard_page == NUM_PAGES - 1) {
				Keyboard_page = 0;
			} else {
				Keyboard_page++;
			}
		}
	}

	prev_page_key = page_key;

	// keys
	int8_t key = Keyboard_get_key();

	if (prev_key == -1 && key != -1) { // key pressed
		if (curr_state == -1) { // haven't pressed key before
			curr_state = key;
			curr_index = 0;
		} else if (key == curr_state) { // have pressed key and same key
			curr_index++;
		} else { // have pressed key but diff key
			APRSGUI_char(KEY_MAP[Keyboard_page][curr_state][curr_index]);
			curr_state = key;
			curr_index = 0;
		}

		if (KEY_MAP[Keyboard_page][curr_state][curr_index + 1] == 0) { // if no more keys
			APRSGUI_char(KEY_MAP[Keyboard_page][curr_state][curr_index]);
			APRSGUI_cursor = APRSGUI_DEFAULT_CURSOR;
			curr_state = -1;
		} else {
			APRSGUI_cursor = KEY_MAP[Keyboard_page][curr_state][curr_index];
			APRSGUI_shift_view();
		}

		wait_dur = 0;
		held_dur = 0;
	}

	if (prev_key != -1 && prev_key == key) { // key held
		wait_dur = 0;
		held_dur++;
		if (held_dur > HOLD_START_DUR + HOLD_DUR) {
			held_dur = HOLD_START_DUR;
			APRSGUI_char(KEY_MAP[Keyboard_page][key][curr_index]);
			// edge case page can change during this which changes char, but leave it
			// also means curr_index can go outside, but that's ok bc it'll send 0s
		}
	}

	if (curr_state != -1) {
		wait_dur++;
		if (wait_dur > WAIT_DUR) {
			APRSGUI_char(KEY_MAP[Keyboard_page][curr_state][curr_index]);
			APRSGUI_cursor = APRSGUI_DEFAULT_CURSOR;
			curr_state = -1;
		}
	}

	prev_key = key;
}

// private helpers
int8_t Keyboard_get_key() {
	for (int8_t i = 0; i < NUM_KEYS; i++) {
		if (HAL_GPIO_ReadPin(KEY_PORTS[i], KEY_PINS[i])) {
			return i;
		}
	}

	return -1;
}

int8_t Keyboard_get_page_key() {
	if (HAL_GPIO_ReadPin(KEY_L_GPIO_Port, KEY_L_Pin)) {
		return 0;
	} else if (HAL_GPIO_ReadPin(KEY_R_GPIO_Port, KEY_R_Pin)) {
		return 1;
	} else {
		return -1;
	}
}
