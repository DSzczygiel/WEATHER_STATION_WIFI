/*
 * brightnessControl.c
 *
 *  Created on: 18 paü 2020
 *      Author: Daniel
 */

#include "brightnessControl.h"

void bcInit(){
	//ADC settings
	LDR_DDR &= ~(1 << LDR);	//ADC pin as input
	ADCSRA |= (1 << ADEN) | (1 << ADIE);	//Enable ADC and enable ADC Conversion Complete Interrupt
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1);	//ADC prescaler = 64. 8000000/64 = 125kHz
	ADMUX |= (1 << REFS0) | (1 << ADLAR);	//Vcc as reference, adjust conversion result to the left

	//PWM settings
	PWM_DDR |= (1 << PWM);
	TCCR1A |= (1 << COM1A1);	//Non inverting PWM mode
	TCCR1A |= (1 << WGM10);	//8 bit fast PWM mode
	TCCR1B |= (1 << WGM12);	//
	TCCR1B |= (1 << CS11);// | (1 << CS11);	//Prescaler = 8. 8000000/8/256 = ~3900Hz

	OCR1A = 230;
}
/*
 * Read LDR value and set PWM duty cycle
 */
void adjustBrightness(){
	ADCSRA |= (1 << ADSC);
}

ISR(ADC_vect){
	if(ADCH > 10)
		OCR1A = ADCH;
	else
		OCR1A = 10;
}
