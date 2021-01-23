/*
 * LCD.c
 *
 *  Created on: 7 lip 2020
 *      Author: Daniel
 */

#include "LCD.h"
#include <util/delay.h>

#define LCD_SET_D7 LCD_D7_PORT |= LCD_D7
#define LCD_CLEAR_D7 LCD_D7_PORT &= ~LCD_D7
#define LCD_SET_D6 LCD_D6_PORT |= LCD_D6
#define LCD_CLEAR_D6 LCD_D6_PORT &= ~LCD_D6
#define LCD_SET_D5 LCD_D5_PORT |= LCD_D5
#define LCD_CLEAR_D5 LCD_D5_PORT &= ~LCD_D5
#define LCD_SET_D4 LCD_D4_PORT |= LCD_D4
#define LCD_CLEAR_D4 LCD_D4_PORT &= ~LCD_D4
#define LCD_SET_E_1 LCD_E_1_PORT |= LCD_E_1
#define LCD_CLEAR_E_1 LCD_E_1_PORT &= ~LCD_E_1
#define LCD_SET_E_2 LCD_E_2_PORT |= LCD_E_2
#define LCD_CLEAR_E_2 LCD_E_2_PORT &= ~LCD_E_2
#define LCD_COMMAND_MODE LCD_RS_PORT &= ~LCD_RS
#define LCD_DATA_MODE LCD_RS_PORT |= LCD_RS

#define LCD_CLEAR 0x01
#define LCD_HOME 0x02

#define LCD_ENTRY_MODE 0x04
#define LCD_ENTRY_INC 0x02
#define LCD_ENTRY_DEC 0x00
#define LCD_ENTRY_SHIFT_CURSOR 0x00
#define LCD_ENTRY_SHIFT_DISPLAY 0x01

#define LCD_DISLPAY_ON_OFF 0x08
#define LCD_DISPLAY_OFF 0x00
#define LCD_DISPLAY_ON 0x04
#define LCD_CURSOR_OFF 0x00
#define LCD_CURSOR_ON 0x02
#define LCD_CURSOR_BLINK 0x01
#define LCD_CURSOR_NOBLINK 0x00

#define LCD_CURSOR_DISPLAY_SHIFT 0x10
#define LCD_SHIFT_CURSOR 0x00
#define LCD_SHIFT_DISPLAY 0x08
#define LCD_SHIFT_LEFT 0x00
#define LCD_SHIFT_RIGHT 0x04

#define LCD_FUNCTION_SET 0x20
#define LCD_4_BIT 0x00
#define LCD_8_BIT 0x10
#define LCD_ONE_LINE 0x00
#define LCD_TWO_LINES 0x08
#define LCD_5X8_CHAR_SIZE 0x00
#define LCD_5X10_CHAR_SIZE 0x04

#define LCD_SET_DDRAM 0x80
#define LCD_SET_CGRAM 0x40

#define LCD_WIDTH 24

uint8_t indoorTempChar[] = {
  0x04,
  0x0A,
  0x11,
  0x1F,
  0x15,
  0x1B,
  0x1B,
  0x00
};

uint8_t outdoorTempChar[] = {
  0x04,
  0x0E,
  0x1F,
  0x1F,
  0x0E,
  0x04,
  0x04,
  0x00,
};

uint8_t humidityChar[] = {
  0x04,
  0x0E,
  0x0E,
  0x1F,
  0x1F,
  0x1F,
  0x0E,
  0x00
};


uint8_t pressureChar[] = {
  0x04,
  0x04,
  0x04,
  0x1F,
  0x0E,
  0x04,
  0x1F,
  0x00
};

uint8_t degreeChar[] = {
  0x06,
  0x09,
  0x09,
  0x06,
  0x00,
  0x00,
  0x00,
  0x00
};


uint8_t currentDisplay = LCD_DISPLAY_1;

void _lcd_enableDisplay(uint8_t display) {
	switch (display) {
	case LCD_DISPLAY_1:
		LCD_SET_E_1;
		break;
	case LCD_DISPLAY_2:
		LCD_SET_E_2;
		break;
	case LCD_DISPLAY_BOTH:
		LCD_SET_E_1;
		LCD_SET_E_2;
		break;
	}

	_delay_us(1);
	LCD_CLEAR_E_1;
	LCD_CLEAR_E_2;
}

void _lcd_writeNibble(uint8_t nibble) {
	if (nibble & 0x01)
		LCD_SET_D4;
	else
		LCD_CLEAR_D4;

	if (nibble & 0x02)
		LCD_SET_D5;
	else
		LCD_CLEAR_D5;

	if (nibble & 0x04)
		LCD_SET_D6;
	else
		LCD_CLEAR_D6;

	if (nibble & 0x08)
		LCD_SET_D7;
	else
		LCD_CLEAR_D7;
}

void _lcd_writeByte(uint8_t byte, uint8_t display) {
	_lcd_writeNibble(byte >> 4);
	_lcd_enableDisplay(display);
	_lcd_writeNibble(byte);
	_lcd_enableDisplay(display);
	_delay_us(100);
}

void _lcd_writeCommand(uint8_t command, uint8_t display) {
	LCD_COMMAND_MODE;
	_lcd_writeByte(command, display);
}
static void _lcd_writeData(uint8_t data, uint8_t display) {
	LCD_DATA_MODE;
	_lcd_writeByte(data, display);
}

void lcd_goTo(uint8_t x, uint8_t y) {
	if(y < 2)
		currentDisplay = LCD_DISPLAY_1;
	else
		currentDisplay = LCD_DISPLAY_2;
	_lcd_writeCommand(LCD_SET_DDRAM | (x + (y * 0x40)), currentDisplay);
}

void lcd_write(char *text) {
	while (*text)
		_lcd_writeData(*text++, currentDisplay);
}

void lcd_clear(){
	_lcd_writeCommand(LCD_CLEAR, LCD_DISPLAY_BOTH);
	_delay_ms(5);
}

void lcd_setCustomCharacter(uint8_t character[8], uint8_t address, uint8_t display){
	_lcd_writeCommand(LCD_SET_CGRAM | (address << 3), display);
	for(uint8_t i=0; i<8; i++){
		_lcd_writeData(character[i], display);
	}
}

void lcd_init() {
	LCD_D4_DDR |= LCD_D4;
	LCD_D5_DDR |= LCD_D5;
	LCD_D6_DDR |= LCD_D6;
	LCD_D7_DDR |= LCD_D7;
	LCD_RS_DDR |= LCD_RS;
	LCD_E_1_DDR |= LCD_E_1;
	LCD_E_2_DDR |= LCD_E_2;
	_delay_ms(20);
	LCD_CLEAR_D4;
	LCD_CLEAR_D5;
	LCD_CLEAR_D6;
	LCD_CLEAR_D7;
	LCD_CLEAR_E_1;
	LCD_CLEAR_E_2;
	LCD_RS_PORT &= ~LCD_RS;

	_lcd_writeNibble(0x03);
	_lcd_enableDisplay(LCD_DISPLAY_BOTH);
	_delay_ms(5);
	_lcd_writeNibble(0x03);
	_lcd_enableDisplay(LCD_DISPLAY_BOTH);
	_delay_ms(5);
	_lcd_writeNibble(0x03);
	_lcd_enableDisplay(LCD_DISPLAY_BOTH);
	_delay_ms(5);
	_lcd_writeNibble(0x02);
	_lcd_enableDisplay(LCD_DISPLAY_BOTH);
	_delay_ms(5);

	_lcd_writeCommand(
			LCD_FUNCTION_SET | LCD_4_BIT | LCD_5X8_CHAR_SIZE
					| LCD_TWO_LINES, LCD_DISPLAY_BOTH);
	_lcd_writeCommand(LCD_DISLPAY_ON_OFF | LCD_DISPLAY_OFF, LCD_DISPLAY_BOTH);
	_lcd_writeCommand(LCD_CLEAR, LCD_DISPLAY_BOTH);
	_delay_ms(5);

	_lcd_writeCommand(
			LCD_ENTRY_MODE | LCD_ENTRY_SHIFT_CURSOR | LCD_ENTRY_INC, LCD_DISPLAY_BOTH);
	_lcd_writeCommand(
			LCD_DISLPAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF
					| LCD_CURSOR_NOBLINK, LCD_DISPLAY_BOTH);

	lcd_setCustomCharacter(outdoorTempChar, LCD_OUT_TEMP_CHAR, 2);
	lcd_setCustomCharacter(indoorTempChar, LCD_IN_TEMP_CHAR, 2);
	lcd_setCustomCharacter(humidityChar, LCD_HUM_CHAR, 2);
	lcd_setCustomCharacter(pressureChar, LCD_PRESS_CHAR, 2);
	lcd_setCustomCharacter(degreeChar, LCD_DEG_CHAR, 2);

}
