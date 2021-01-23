/*
 * LCD.h
 *
 *  Created on: 7 lip 2020
 *      Author: Daniel
 *
 *      18 pin 4x24 Winstar LCD with two E lines, taken from ECR device.
 *		Not sure about exact driver version and LCD model,
 *		but slightly modified version of my HD44780 library works just fine.
 *
 */

#ifndef LCD_H_
#define LCD_H_

#include <avr/io.h>

#define LCD_D7_DDR DDRD
#define LCD_D7_PORT PORTD
#define LCD_D7 (1 << PD7)

#define LCD_D6_DDR DDRD
#define LCD_D6_PORT PORTD
#define LCD_D6 (1 << PD6)

#define LCD_D5_DDR DDRD
#define LCD_D5_PORT PORTD
#define LCD_D5 (1 << PD5)

#define LCD_D4_DDR DDRD
#define LCD_D4_PORT PORTD
#define LCD_D4 (1 << PD4)

#define LCD_RS_DDR DDRB
#define LCD_RS_PORT PORTB
#define LCD_RS (1 << PB2)

#define LCD_E_1_DDR DDRB
#define LCD_E_1_PORT PORTB
#define LCD_E_1 (1 << PB4)

#define LCD_E_2_DDR DDRB
#define LCD_E_2_PORT PORTB
#define LCD_E_2 (1 << PB3)

#define LCD_DISPLAY_1 0
#define LCD_DISPLAY_2 1
#define LCD_DISPLAY_BOTH 2

#define LCD_IN_TEMP_CHAR 0x02
#define LCD_OUT_TEMP_CHAR 0x01
#define LCD_HUM_CHAR 0x05
#define LCD_PRESS_CHAR 0x03
#define LCD_DEG_CHAR 0x04

void lcd_init();
void lcd_goTo(uint8_t x, uint8_t y);
void lcd_write(char *text);
void lcd_clear();

#endif /* LCD_H_ */
