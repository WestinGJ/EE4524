/*
 * Lab3.c
 *
 * Created: 20/07/2022 16:51:28
 * Author : Ciaran.MacNamee
 * Updated: 21/08/2023 12:24:09
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#define F_CPU 20.0E6
#include <util/delay.h>

/* Use a struct to make the association between PORTs and bits connected to the LED array more explicit */
struct LED_BITS
{
	PORT_t *LED_PORT;
	uint8_t bit_mapping;
};

struct LED_BITS LED_Array[10] = {
	{&PORTC, PIN5_bm}, {&PORTC, PIN4_bm}, {&PORTA, PIN0_bm}, {&PORTF, PIN5_bm}, {&PORTC, PIN6_bm}, {&PORTB, PIN2_bm}, {&PORTF, PIN4_bm}, {&PORTA, PIN1_bm}, {&PORTA, PIN2_bm}, {&PORTA, PIN3_bm}
};

void CLOCK_init (void);
void InitialiseLED_PORT_bits(void);
void InitialiseButton_PORT_bits(void);
void Set_Clear_Ports(uint8_t set);
void Toggle_Ports(void);
void RTC_init(void);

void CLOCK_init (void)
{
	/* Disable CLK_PER Prescaler */
	ccp_write_io( (void *) &CLKCTRL.MCLKCTRLB , (0 << CLKCTRL_PEN_bp));
	/* If set from the fuses during device programming, the CPU will now run at 20MHz (default is /6) */
}

void RTC_init()
{
	uint8_t temp1;
	
	/* Initialize 32.768kHz Oscillator: */
	/* Enable 32k ULP oscillator: */
	temp1 = CLKCTRL.OSC32KCTRLA;
	temp1 |= CLKCTRL_RUNSTDBY_bm;
	
	/* Writing to protected register */
	ccp_write_io((void*)&CLKCTRL.OSC32KCTRLA, temp1);
	
	while(CLKCTRL.MCLKSTATUS & CLKCTRL_OSC32KS_bm)
	{
		; /* Wait until OSC32KS becomes 0 */
	}
	
	/* Initialize RTC: */
	while (RTC.STATUS > 0)
	{
		; /* Wait for all register to be synchronized */
	}
	/* 32.768kHz Internal Oscillator (XOSC32K) */
	RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;
	/* Run in debug: enabled */
	
	RTC.DBGCTRL = RTC_DBGRUN_bm;
	RTC.PITINTCTRL = RTC_PI_bm; /* Periodic Interrupt: enabled */
	RTC.PITCTRLA = RTC_PERIOD_CYC8192_gc | RTC_PITEN_bm; /* RTC Clock Cycles 8192, Enable: enabled */
}

void InitialiseLED_PORT_bits()
{
	PORTC.DIR = PIN6_bm | PIN5_bm | PIN4_bm;  /*(1<<6) | (1<<5) | (1<<4); 0x70;*/		/* PC4-UNO D1 (TXD1), PC5-UNO D0 (RXD1), PC6 - UNO D4  */
	PORTA.DIR = PIN3_bm | PIN2_bm | PIN1_bm | PIN0_bm; /*(1<<1) | (1<<0);   0x0f; */      /* PA1-UNO D7, PA0 - UNO D2, PA2- LED8, PA3 - LED9  */
	PORTB.DIR = PIN2_bm; /*0x04;*/		/* PB2 - UNO D5 */
	PORTF.DIR = PIN5_bm | PIN4_bm; /*(1<<5) | (1<<4);   0x30; */		/* PF5 - UNO D3, PF4 UNO D6 */
	/* Later use PIN6_bm etc */
}

void InitialiseButton_PORT_bits(void)
{
	PORTE.DIRCLR = PIN1_bm;					/* PORTE bit 1 is an input */
	PORTE.PIN1CTRL = PORT_PULLUPEN_bm;		/* Enable Pull up resistors on PORTE pins 1 & 2 */
}

/* Function to set or clear all LED port bits, 1 - set, 0 - clear */
void Set_Clear_Ports(uint8_t set) {
	
	uint8_t i;
	
	for (i = 0; i <= 9; i += 1)
	{
		if (set)
			LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
		else
			LED_Array[i].LED_PORT->OUTCLR = LED_Array[i].bit_mapping;
	}
}

void Toggle_Ports(void)
{
	uint8_t i;
	
	for (i = 0; i <= 9; i += 1)
	{
		LED_Array[i].LED_PORT->OUTTGL = LED_Array[i].bit_mapping;
	}
}

int main(void)
{
	CLOCK_init();
	
	/* set UNO D0-D7 to all outputs, also LED8 and LED9  */
	InitialiseLED_PORT_bits();
	/* Initialise Push Button bits and pull up resistors */
	InitialiseButton_PORT_bits();
	
	Set_Clear_Ports(0);
	
	RTC_init();
	
	sei();
		
    while (1) 
    {
    }
}

ISR(RTC_PIT_vect)
{
	static uint8_t PIT_count;
	/* Clear flag by writing '1': */
	RTC.PITINTFLAGS = RTC_PI_bm;
	
	if (++PIT_count > 2)
	{
		PIT_count = 0;
		Toggle_Ports();	
	}
	
}

