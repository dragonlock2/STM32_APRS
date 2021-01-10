/*
 * Keyboard.h
 *
 *  Created on: Aug 22, 2020
 *      Author: matthewtran
 */

#ifndef INC_KEYBOARD_H_
#define INC_KEYBOARD_H_

#include "stdint.h"
#include "APRSGUI.h"

extern uint8_t Keyboard_page;

// public functions
void Keyboard_update();

// private helpers
int8_t Keyboard_get_key();
int8_t Keyboard_get_page_key();

#endif /* INC_KEYBOARD_H_ */
