/*
 * APRS_GUI.c
 *
 *  Created on: Jul 24, 2020
 *      Author: matthewtran
 */

#include <APRSGUI.h>

// constants
static char DIGI[] = "DIGI: ";
static char INFO[] = "INFO: ";
#define START_LEN     6 // length of DIGI and INFO
#define REST_LEN     25 // length from start to right side of screen
#define TEXT_LEN 31 // width of printable area
#define DEST_SRC_FMT "DEST: %-9s SRC: %-9s " // TEXT_LEN long
#define FLASH_COUNT 10 // frames before toggling flash
#define MAX_LEN_CHAR '<'

#define RX_LEN 8 // packets to keep

typedef enum {
	TX_DEST,
	TX_CALL,
	TX_DIGI
} TXGUIPack_idx_t;

typedef enum {
	RX_DEST,
	RX_INFO,
	RX_DIGI
} RXGUIPack_idx_t;

// structs
typedef struct {
	TXGUIPack_idx_t index;
	bool sel_info;

	char dest[APRS_SIGN_LEN];
	uint32_t dest_len;
	char call[APRS_SIGN_LEN];
	uint32_t call_len;
	char digi[APRS_SIGN_LEN * APRS_MAX_DIGI];
	uint32_t digi_idx;
	uint32_t digi_len;
	char info[APRS_MAX_INFO + 1];
	uint32_t info_idx;
	uint32_t info_len;
} TXGUIPack;

typedef struct {
	RXGUIPack_idx_t index;
	bool valid;

	char dest_src[TEXT_LEN + 1]; // fills up entire width
	char digi[APRS_SIGN_LEN * APRS_MAX_DIGI];
	uint32_t digi_idx;
	uint32_t digi_len;
	char info[APRS_MAX_INFO + 1];
	uint32_t info_idx;
	uint32_t info_len;
} RXGUIPack;

// variables
static BitFIFO tx_fifo;
static APRSPacket tx_pack;

static TXGUIPack tx_gui_pack;

static RXGUIPack rx_gui_packs[RX_LEN]; // treated as a FIFO
static uint32_t rx_gui_fifo_idx; // 0-7, marks most recent packet in rx_gui_packs
static uint32_t rx_gui_packs_idx; // 0-7, marks which line on, not idx in FIFO

static bool sel_tx; // if cursor at tx

static bool flash_state;
static uint32_t flash_cnt;

char APRSGUI_cursor;

// public functions
void APRSGUI_init(APRSPacket *init_pack) {
	BitFIFO_init(&tx_fifo, malloc(APRS_FIFO_BYTES), APRS_FIFO_BYTES);
	tx_pack.dest = malloc(APRS_SIGN_LEN);
	tx_pack.callsign = malloc(APRS_SIGN_LEN);
	tx_pack.digi = malloc(APRS_SIGN_LEN * APRS_MAX_DIGI);
	tx_pack.info = malloc(APRS_MAX_INFO + 1);

	tx_gui_pack.index = TX_DEST;
	tx_gui_pack.sel_info = false;
	strcpy(tx_gui_pack.dest, init_pack->dest);
	tx_gui_pack.dest_len = strlen(tx_gui_pack.dest);
	strcpy(tx_gui_pack.call, init_pack->callsign);
	tx_gui_pack.call_len = strlen(tx_gui_pack.call);
	strcpy(tx_gui_pack.digi, init_pack->digi);
	tx_gui_pack.digi_idx = 0;
	tx_gui_pack.digi_len = strlen(tx_gui_pack.digi);
	strcpy(tx_gui_pack.info, init_pack->info);
	tx_gui_pack.info_idx = 0;
	tx_gui_pack.info_len = strlen(tx_gui_pack.info);

	for (uint32_t i = 0; i < RX_LEN; i++) {
		memset(rx_gui_packs + i, 0, sizeof(RXGUIPack));
		rx_gui_packs[i].valid = false;
	}
	rx_gui_fifo_idx = 0;
	rx_gui_packs_idx = 0;

	sel_tx = true;

	flash_state = false;
	flash_cnt = 0;

	APRSGUI_cursor = APRSGUI_DEFAULT_CURSOR;

	SSD1306_clear_display();
	for (uint32_t i = 0; i < SSD1306_WIDTH; i++) { // center line
		SSD1306_draw_pixel(i, 13, true);
		SSD1306_draw_pixel(i, 14, true);
	}
}

void APRSGUI_addpack(APRSPacket *pack) {
	if (rx_gui_fifo_idx == 0) {
		rx_gui_fifo_idx = RX_LEN - 1;
	} else {
		rx_gui_fifo_idx--;
	}

	RXGUIPack *rxpk = rx_gui_packs + rx_gui_fifo_idx;

	sprintf(rxpk->dest_src, DEST_SRC_FMT, pack->dest, pack->callsign);
	strcpy(rxpk->digi, pack->digi);
	strcpy(rxpk->info, pack->info);

	rxpk->index = RX_DEST;
	rxpk->valid = true;
	rxpk->digi_idx = 0;
	rxpk->digi_len = strlen(rxpk->digi);
	rxpk->info_idx = 0;
	rxpk->info_len = strlen(rxpk->info);

	rx_gui_packs_idx++;
	if (rx_gui_packs_idx >= RX_LEN) {
		rx_gui_packs_idx = RX_LEN - 1;
	}
}

void APRSGUI_render() {
	// cursor flashing
	flash_cnt++;
	if (flash_cnt >= FLASH_COUNT) {
		flash_cnt = 0;
		flash_state = !flash_state;
	}

	// tx
	SSD1306_set_cursor(0, 0);
	APRSGUI_draw_cursor(sel_tx && !tx_gui_pack.sel_info);
	if (tx_gui_pack.index < TX_DIGI) {
		APRSGUI_draw_tx_dest_src();
	} else {
		APRSGUI_draw_longstring(DIGI, tx_gui_pack.digi, tx_gui_pack.digi_idx, tx_gui_pack.digi_len, sel_tx && !tx_gui_pack.sel_info, APRS_SIGN_LEN * APRS_MAX_DIGI - 1);
	}

	SSD1306_set_cursor(0, SSD1306_FONT_H);
	APRSGUI_draw_cursor(sel_tx && tx_gui_pack.sel_info);
	APRSGUI_draw_longstring(INFO, tx_gui_pack.info, tx_gui_pack.info_idx, tx_gui_pack.info_len, sel_tx && tx_gui_pack.sel_info, APRS_MAX_INFO);

	// rx
	for (uint32_t i = 0; i < RX_LEN; i++) {
		SSD1306_set_cursor(0, 16 + i * SSD1306_FONT_H);

		APRSGUI_draw_cursor(!sel_tx && (i == rx_gui_packs_idx));

		RXGUIPack *pack = rx_gui_packs + ((i + rx_gui_fifo_idx) % RX_LEN);
		if (pack->valid) {
			switch (pack->index) {
				case RX_DEST:
					SSD1306_draw_string(pack->dest_src, TEXT_LEN);
					break;
				case RX_INFO:
					APRSGUI_draw_longstring(INFO, pack->info, pack->info_idx, pack->info_len, false, 0);
					break;
				case RX_DIGI:
					APRSGUI_draw_longstring(DIGI, pack->digi, pack->digi_idx, pack->digi_len, false, 0);
					break;
			}
		}
	}
}

void APRSGUI_char(char c) {
	if (c < ' ') { // non-printables
		switch (c) {
			case APRSGUI_UP:
				APRSGUI_up();
				break;
			case APRSGUI_DOWN:
				APRSGUI_down();
				break;
			case APRSGUI_LEFT:
				APRSGUI_left();
				break;
			case APRSGUI_RIGHT:
				APRSGUI_right();
				break;
			case APRSGUI_CENTER:
				APRSGUI_center();
				break;
			case APRSGUI_SCROLL_LEFT:
				APRSGUI_scroll(false);
				break;
			case APRSGUI_SCROLL_RIGHT:
				APRSGUI_scroll(true);
				break;
			case APRSGUI_DELETE:
				APRSGUI_delete();
				break;
		}
		return;
	}

	// shouldn't, but trusting the user to enter in callsigns and digis properly
	if (sel_tx) {
		if (tx_gui_pack.sel_info) {
			APRSGUI_char_helper(tx_gui_pack.info, &tx_gui_pack.info_idx, &tx_gui_pack.info_len, APRS_MAX_INFO, c);
		} else {
			switch (tx_gui_pack.index) {
				case TX_DEST:
					if (tx_gui_pack.dest_len < APRS_SIGN_LEN - 1) {
						tx_gui_pack.dest[tx_gui_pack.dest_len] = c;
						tx_gui_pack.dest_len++;
						tx_gui_pack.dest[tx_gui_pack.dest_len] = 0;
					}
					break;
				case TX_CALL:
					if (tx_gui_pack.call_len < APRS_SIGN_LEN - 1) {
						tx_gui_pack.call[tx_gui_pack.call_len] = c;
						tx_gui_pack.call_len++;
						tx_gui_pack.call[tx_gui_pack.call_len] = 0;
					}
					break;
				case TX_DIGI:
					APRSGUI_char_helper(tx_gui_pack.digi, &tx_gui_pack.digi_idx, &tx_gui_pack.digi_len, APRS_SIGN_LEN * APRS_MAX_DIGI, c);
					break;
			}
		}
	}
}

void APRSGUI_shift_view() {
	uint32_t *idx = 0;
	uint32_t *len = 0;

	if (sel_tx) {
		if (tx_gui_pack.sel_info) {
			idx = &tx_gui_pack.info_idx;
			len = &tx_gui_pack.info_len;
		} else if (tx_gui_pack.index == TX_DIGI){
			idx = &tx_gui_pack.digi_idx;
			len = &tx_gui_pack.digi_len;
		}
	}

	if (idx) {
		if ((*len > (REST_LEN - 1)) && ((*len - *idx) >= (REST_LEN - 2))) { // make sure cursor within view
			*idx = *len - REST_LEN + 2;
		}
	}
}

// private helpers
void APRSGUI_draw_tx_dest_src() {
	char cursor;

	SSD1306_draw_string("DEST: ", 6);
	uint32_t dest_len = strlen(tx_gui_pack.dest);
	SSD1306_draw_string(tx_gui_pack.dest, dest_len);
	cursor = flash_state ? (!tx_gui_pack.sel_info && tx_gui_pack.index == TX_DEST ? (dest_len >= APRS_SIGN_LEN - 1 ? MAX_LEN_CHAR : APRSGUI_cursor) : ' ') : ' ';
	SSD1306_draw_string(&cursor, 1);
	for (uint32_t i = dest_len; i < APRS_SIGN_LEN - 1; i++) {
		SSD1306_draw_string(" ", 1);
	}

	SSD1306_draw_string("SRC: ", 5);
	uint32_t src_len = strlen(tx_gui_pack.call);
	SSD1306_draw_string(tx_gui_pack.call, src_len);
	cursor = flash_state ? (!tx_gui_pack.sel_info && tx_gui_pack.index == TX_CALL ? (src_len >= APRS_SIGN_LEN - 1 ? MAX_LEN_CHAR : APRSGUI_cursor) : ' ') : ' ';
	SSD1306_draw_string(&cursor, 1);
	for (uint32_t i = src_len; i < APRS_SIGN_LEN - 1; i++) {
		SSD1306_draw_string(" ", 1);
	}
}

void APRSGUI_draw_cursor(bool sel) {
	SSD1306_draw_string(sel ? ">" : " ", 1);
}

void APRSGUI_draw_longstring(char *start, char *str, uint32_t str_idx, uint32_t str_len, bool sel, uint32_t tx_max_len) {
	SSD1306_draw_string(start, START_LEN);

	char left_indic = flash_state ? '<' : ' ';
	char right_indic = flash_state ? '>' : ' ';
	char cursor = flash_state ? (str_len >= tx_max_len ? MAX_LEN_CHAR : APRSGUI_cursor) : ' ';

	uint32_t rest_len = REST_LEN; // max characters can print
	uint32_t print_len = str_len - str_idx;

	if (str_idx > 0) {
		SSD1306_draw_string(&left_indic, 1);
		rest_len--;
	}

	if (print_len <= rest_len) {
		SSD1306_draw_string(str + str_idx, print_len);

		if (sel && print_len < rest_len) { // only draw cursor if selected and room for it
			SSD1306_draw_string(&cursor, 1);
			rest_len--;
		}

		for (; print_len < rest_len; print_len++) {
			SSD1306_draw_string(" ", 1);
		}
	} else {
		SSD1306_draw_string(str + str_idx, rest_len - 1);
		SSD1306_draw_string(&right_indic, 1);
	}
}

void APRSGUI_up() {
	if (sel_tx) {
		if (tx_gui_pack.sel_info) {
			tx_gui_pack.sel_info = false;
		} else {
			// do nothing
			sel_tx = false;
			rx_gui_packs_idx = RX_LEN - 1;
			RXGUIPack *pack = rx_gui_packs + ((rx_gui_packs_idx + rx_gui_fifo_idx) % RX_LEN);
			while (!pack->valid) {
				if (rx_gui_packs_idx == 0) {
					sel_tx = true;
					tx_gui_pack.sel_info = true;
					break;
				}
				rx_gui_packs_idx--;
				pack = rx_gui_packs + ((rx_gui_packs_idx + rx_gui_fifo_idx) % RX_LEN);
			}
		}
	} else {
		if (rx_gui_packs_idx == 0) {
			sel_tx = true;
			tx_gui_pack.sel_info = true;
		} else {
			rx_gui_packs_idx--;
		}
	}
}

void APRSGUI_down() {
	if (sel_tx) {
		if (tx_gui_pack.sel_info) {
			sel_tx = false;
			rx_gui_packs_idx = 0;

			RXGUIPack *pack = rx_gui_packs + ((rx_gui_packs_idx + rx_gui_fifo_idx) % RX_LEN);
			if (!pack->valid) {
				sel_tx = true;
				tx_gui_pack.sel_info = false;;
			}
		} else {
			tx_gui_pack.sel_info = true;
		}
	} else {
		if (rx_gui_packs_idx >= RX_LEN - 1) {
			sel_tx = true;
			tx_gui_pack.sel_info = false;
		} else {
			rx_gui_packs_idx++;

			RXGUIPack *pack = rx_gui_packs + ((rx_gui_packs_idx + rx_gui_fifo_idx) % RX_LEN);
			if (!pack->valid) {
				sel_tx = true;
				tx_gui_pack.sel_info = false;
			}
		}
	}
}

void APRSGUI_left() {
	if (sel_tx) {
		if (tx_gui_pack.sel_info) {
			// do nothing
		} else {
			if (tx_gui_pack.index == TX_DEST) {
				tx_gui_pack.index = TX_DIGI;
			} else {
				tx_gui_pack.index--;
			}
		}
	} else {
		RXGUIPack *pack = rx_gui_packs + ((rx_gui_packs_idx + rx_gui_fifo_idx) % RX_LEN);
		if (pack->index == RX_DEST) {
			pack->index = RX_DIGI;
		} else {
			pack->index--;
		}
	}
}

void APRSGUI_right() {
	if (sel_tx) {
		if (tx_gui_pack.sel_info) {
			// do nothing
		} else {
			if (tx_gui_pack.index == TX_DIGI) {
				tx_gui_pack.index = TX_DEST;
			} else {
				tx_gui_pack.index++;
			}
		}
	} else {
		RXGUIPack *pack = rx_gui_packs + ((rx_gui_packs_idx + rx_gui_fifo_idx) % RX_LEN);
		if (pack->index == RX_DIGI) {
			pack->index = RX_DEST;
		} else {
			pack->index++;
		}
	}
}

void APRSGUI_center() {
	if (sel_tx && !AFSK_sending) {
		strcpy(tx_pack.dest, tx_gui_pack.dest);
		strcpy(tx_pack.callsign, tx_gui_pack.call);
		strcpy(tx_pack.digi, tx_gui_pack.digi);
		strcpy(tx_pack.info, tx_gui_pack.info);

		APRS_encode(&tx_fifo, &tx_pack);
		AFSK_send(&tx_fifo);
	}
}

void APRSGUI_scroll(bool right) {
	if (sel_tx) {
		if (tx_gui_pack.sel_info) {
			APRSGUI_scroll_helper(&tx_gui_pack.info_idx, &tx_gui_pack.info_len, right);
		} else if (tx_gui_pack.index == TX_DIGI) {
			APRSGUI_scroll_helper(&tx_gui_pack.digi_idx, &tx_gui_pack.digi_len, right);
		}
	} else {
		RXGUIPack *pack = rx_gui_packs + ((rx_gui_packs_idx + rx_gui_fifo_idx) % RX_LEN);
		switch (pack->index) {
			case RX_DEST:
				// do nothing
				break;
			case RX_INFO:
				APRSGUI_scroll_helper(&pack->info_idx, &pack->info_len, right);
				break;
			case RX_DIGI:
				APRSGUI_scroll_helper(&pack->digi_idx, &pack->digi_len, right);
				break;
		}
	}
}

void APRSGUI_delete() {
	if (sel_tx) {
		if (tx_gui_pack.sel_info) {
			APRSGUI_delete_helper(tx_gui_pack.info, &tx_gui_pack.info_idx, &tx_gui_pack.info_len);
		} else {
			switch (tx_gui_pack.index) {
				case TX_DEST:
					if (tx_gui_pack.dest_len > 0) {
						tx_gui_pack.dest_len--;
						tx_gui_pack.dest[tx_gui_pack.dest_len] = 0;
					}
					break;
				case TX_CALL:
					if (tx_gui_pack.call_len > 0) {
						tx_gui_pack.call_len--;
						tx_gui_pack.call[tx_gui_pack.call_len] = 0;
					}
					break;
				case TX_DIGI:
					APRSGUI_delete_helper(tx_gui_pack.digi, &tx_gui_pack.digi_idx, &tx_gui_pack.digi_len);
					break;
			}
		}
	}
}

void APRSGUI_char_helper(char *str, uint32_t *idx, uint32_t *len, uint32_t max_len, char c) {
	if ((*len > REST_LEN) && ((*len - *idx) >= (REST_LEN - 1))) { // make sure cursor within view
		*idx = *len - REST_LEN + 2;
	}

	if (*len >= max_len) { // check room to add
		return;
	}

	str[*len] = c;
	(*len)++;
	str[*len] = 0; // do not assume null terminator exists

	if ((*len > REST_LEN) && ((*len - *idx) >= (REST_LEN - 1))) { // make sure cursor within view, again
		*idx = *len - REST_LEN + 2;
	}
}

void APRSGUI_scroll_helper(uint32_t *idx, uint32_t *len, bool right) {
	if (right) {
		if (*idx + 1 < *len) {
			if (*idx == 0 && *len > 2) { // move over one extra
				(*idx)++;
			}
			(*idx)++;
		}
	} else {
		if (*idx > 0) {
			(*idx)--;
		}
	}
}

void APRSGUI_delete_helper(char *str, uint32_t *idx, uint32_t *len) {
	if (*len == 0) { // check stuff to delete
		return;
	}

	if ((*len > REST_LEN) && ((*len - *idx) >= (REST_LEN - 1))) { // make sure cursor within view
		*idx = *len - REST_LEN + 2;
	}

	str[*len - 1] = 0;
	(*len)--;

	if (*idx > 0 && *len > 0) {
		(*idx)--;
	}
}
