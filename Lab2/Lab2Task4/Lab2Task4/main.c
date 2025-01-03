/*
 * Lab2Task4.c
 *
 * Created: 15/07/2022 14:13:00
 * Author : Ciaran.MacNamee
 *
 * Mapping of ATmega4809 ports to Arduino UNO (and LED Array):
 * D0 - PORTC Bit 5; D1 - PORTC Bit 4; D2 - PORTA Bit 0; D3 - PORTF Bit 5;
 * D4 - PORTC Bit 6; D5 - PORTB Bit 2; D6 - PORTF Bit 4; D7 - PORTA Bit 1
 * The LED Array has 10 bits so we may also use:
 * LED Array Bit 8 - PORTA Bit 2; LED Array Bit 9 - PORTA Bit 3 - these are used in this example
 *
 * This program uses an array of struct to group the LED PORT bits together
 * It also initialises the PORTE pin bits which are connected to the Shield Push Buttons
 */ 

#include <avr/io.h>
#include <avr/cpufunc.h>
#define F_CPU 20.0E6
#include <util/delay.h>

#define NUM_LED_BITS 10

/* Use a struct to make the association between PORTs and bits connected to the LED array more explicit */
struct LED_BITS
{
	PORT_t *LED_PORT;
	uint8_t bit_mapping;
};

struct LED_BITS LED_Array[NUM_LED_BITS] = {
	{&PORTC, 1<<5}, {&PORTC, 1<<4}, {&PORTA, 1<<0}, {&PORTF, 1<<5}, {&PORTC, 1<<6}, {&PORTB, 1<<2}, {&PORTF, 1<<4}, {&PORTA, 1<<1}, {&PORTA, 1<<2}, {&PORTA, 1<<3}
};

/* Note that we could also use PIN4_bm in place if 1<<4 etc - try it.  */

#define DELAY_EIGHTH_SEC 125000
#define DELAY_HALF_SEC 500000
#define DELAY_ONE_SEC 1000000

void CLOCK_init (void);
void InitialiseLED_PORT_bits(void);
void InitialiseButton_PORT_bits(void);
void Set_Clear_Ports(uint8_t set);
void move_leds(void);


void CLOCK_init (void)
{
	/* Disable CLK_PER Prescaler */
	ccp_write_io( (void *) &CLKCTRL.MCLKCTRLB , (0 << CLKCTRL_PEN_bp));
	/* If set from the fuses during device programming, the CPU will now run at 20MHz (default is /6) */
}

void InitialiseLED_PORT_bits()
{
	/* This has the same effect as PORTS_init() in Lab2Task2, but uses an array for the LED bits */
	uint8_t i;
	
	for (i = 0; i <= (NUM_LED_BITS - 1); i += 1) {
		LED_Array[i].LED_PORT->DIRSET = LED_Array[i].bit_mapping;
	}
}


void InitialiseButton_PORT_bits(void)
{
	PORTE.DIRCLR = PIN1_bm | PIN2_bm;		/* PORTE bit 1 and bit 2 are inputs */
	PORTE.PIN1CTRL |= PORT_PULLUPEN_bm;		/* Enable Pull up resistors on PORTE pins 1 & 2 */
	PORTE.PIN2CTRL |= PORT_PULLUPEN_bm;
}

int main(void)
{
	CLOCK_init();
	
	/* set UNO D0-D7 to all outputs, also LED8 and LED9  */
	InitialiseLED_PORT_bits();
	/* Initialise Push Button bits and pull up resistors */
	InitialiseButton_PORT_bits();
	
	Set_Clear_Ports(1);
	_delay_us(DELAY_ONE_SEC);
	
	Set_Clear_Ports(0);
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
	if (PORTE.IN & PIN1_bm){
	for (i = 0; i < (NUM_LED_BITS - 1); i += 1)
		{
			if (i == 0){
				LED_Array[i+1].LED_PORT->OUTCLR = LED_Array[i+1].bit_mapping;
				LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
			}
			else {
				LED_Array[i-1].LED_PORT->OUTCLR = LED_Array[i-1].bit_mapping;
				LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
			}
		
			_delay_us(DELAY_EIGHTH_SEC);
		}
		for (i = (NUM_LED_BITS - 1); i > 0; i -= 1)
		{
			if (i == (NUM_LED_BITS - 1)){
				LED_Array[i-1].LED_PORT->OUTCLR = LED_Array[i-1].bit_mapping;
				LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
			}
			else {
				LED_Array[i+1].LED_PORT->OUTCLR = LED_Array[i+1].bit_mapping;
				LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
			}
			
			_delay_us(DELAY_EIGHTH_SEC);
		}
	}
	else{
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
}


/* Function to set or clear all LED port bits, 1 - set, 0 - clear */ 
void Set_Clear_Ports(uint8_t set) {
	
	uint8_t i;
	
	for (i = 0; i <= (NUM_LED_BITS - 1); i += 1)
	{
		if (set)
			LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
		else
			LED_Array[i].LED_PORT->OUTCLR = LED_Array[i].bit_mapping;
	}
}

