/*
 * brightnessControl.h
 *
 *  Created on: 18 paü 2020
 *      Author: Daniel
 *
 *      LDR based brightness control
 */

#ifndef BRIGHTNESSCONTROL_H_
#define BRIGHTNESSCONTROL_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#define LDR_DDR DDRC
#define PWM_DDR DDRB
#define LDR_PORT PORTC
#define PWM_PORT PORTB
#define LDR PC0
#define PWM PB1

void bcInit();
void adjustBrightness();
void _startConversion();

#endif /* BRIGHTNESSCONTROL_H_ */
