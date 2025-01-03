/*
 * Lab2Task3.c
 *
 * Created: 15/07/2022 12:41:39, Updated 10/08/2023
 * Author : Ciaran.MacNamee
 *
 * Mapping of ATmega4809 ports to Arduino UNO (and LED Array):
 * D0 - PORTC Bit 5; D1 - PORTC Bit 4; D2 - PORTA Bit 0; D3 - PORTF Bit 5;
 * D4 - PORTC Bit 6; D5 - PORTB Bit 2; D6 - PORTF Bit 4; D7 - PORTA Bit 1
 * The LED Array has 10 bits so we may also use:
 * LED Array Bit 8 - PORTA Bit 2; LED Array Bit 9 - PORTA Bit 3
 *
 * The ATMega4809 on the Arduino Every uses a CPU clock of 20 MHz - hence this is set as F_CPU *
 * Note, however, that by default this is initially divided by 6 for LV compatibility reasons *
 * The Prescaler is disabled here to disable the division by 6. 
 * It is assumed that the high frequency oscillator is set for 20MHz during device programming.
 * Different Port Registers used for the GPIO pins - these are organised in arrays for easier coding.
 */ 

#include <avr/io.h>
#include <avr/cpufunc.h>
#define F_CPU 20.0E6
#include <util/delay.h>

#define NUM_LED_BITS 10

/* LED_PORT is an array of PORTs references to make bit setting/clearing simpler */
/* bit_mapping is an array that identifies the PORT bits corresponding to each array element */

/* Use a struct to make the association between PORTs and bits connected to the LED array more explicit */
struct LED_BITS
{
	PORT_t *LED_PORT;
	uint8_t bit_mapping;
};

struct LED_BITS LED_Array[NUM_LED_BITS] = {
	{&PORTC, 1<<5}, {&PORTC, 1<<4}, {&PORTA, 1<<0}, {&PORTF, 1<<5}, {&PORTC, 1<<6}, {&PORTB, 1<<2}, {&PORTF, 1<<4}, {&PORTA, 1<<1}, {&PORTA, 1<<2}, {&PORTA, 1<<3}
};
	
#define DELAY_EIGHTH_SEC 125000
#define DELAY_ONE_SEC 1000000

void CLOCK_init (void);
void PORTS_init(void);
void ClearPorts(void);
void SetPorts(void);
void move_leds(void);


void CLOCK_init (void)
{
	/* Disable CLK_PER Prescaler */
	ccp_write_io( (void *) &CLKCTRL.MCLKCTRLB , (0 << CLKCTRL_PEN_bp));
	/* If set from the fuses during device programming, the CPU will now run at 20MHz (default is /6) */
}

void PORTS_init(void)
{
/* This has the same effect as PORTS_init() in Lab2Task2, but uses an array for the LED bits */
	uint8_t i;
	
	for (i = 0; i <= (NUM_LED_BITS-1); i += 1) {
		LED_Array[i].LED_PORT->DIRSET = LED_Array[i].bit_mapping;
	} 
}

int main(void)
{
	CLOCK_init();
	
	/* set UNO D0-D7 to all outputs  */
	PORTS_init();
		
	SetPorts();
	_delay_us(DELAY_ONE_SEC);
	
	ClearPorts();
		
	_delay_us(DELAY_ONE_SEC);
		
	while(1)
	{
		move_leds();
	}
	return 1;
}

void move_leds(void)
{
	uint8_t i;
	
	for (i = 0; i < (NUM_LED_BITS - 1); i += 1)
	{
		if (i == 0){
			LED_Array[i+1].LED_PORT->OUTSET = LED_Array[i+1].bit_mapping;
			LED_Array[i].LED_PORT->OUTCLR = LED_Array[i].bit_mapping;
		}
		else {
			LED_Array[i-1].LED_PORT->OUTSET = LED_Array[i-1].bit_mapping;
			LED_Array[i].LED_PORT->OUTCLR = LED_Array[i].bit_mapping;
		}
		
		_delay_us(DELAY_EIGHTH_SEC);
	}
	for (i = (NUM_LED_BITS - 1); i > 0; i -= 1)
	{
		if (i == (NUM_LED_BITS - 1)){
			LED_Array[i-1].LED_PORT->OUTSET = LED_Array[i-1].bit_mapping;
			LED_Array[i].LED_PORT->OUTCLR = LED_Array[i].bit_mapping;
		}
		else {
			LED_Array[i+1].LED_PORT->OUTSET = LED_Array[i+1].bit_mapping;
			LED_Array[i].LED_PORT->OUTCLR = LED_Array[i].bit_mapping;
		}
		
		_delay_us(DELAY_EIGHTH_SEC);
	}
}

void ClearPorts() {
	
	uint8_t i;
	
	for (i = 0; i <= (NUM_LED_BITS - 1); i += 1) {
		LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
	}
}

void SetPorts() {
	
	uint8_t i;
	
	for (i = 0; i <= (NUM_LED_BITS - 1); i += 1) {
		LED_Array[i].LED_PORT->OUTCLR = LED_Array[i].bit_mapping;
	}
}

