/*
 * Lab2Task1.c
 *
 * Created: 14/07/2022 17:38:17
 * Author : Ciaran.MacNamee
 */ 
/* Mapping of ATmega4809 ports to Arduino UNO (and LED Array):
 * D0 - PORTC Bit 5; D1 - PORTC Bit 4; D2 - PORTA Bit 0; D3 - PORTF Bit 5;
 * D4 - PORTC Bit 6; D5 - PORTB Bit 2; D6 - PORTF Bit 4; D7 - PORTA Bit 1
 * The LED Array has 10 bits so we may also use:
 * LED Array Bit 8 - PORTA Bit 2; LED Array Bit 9 - PORTA Bit 3
 * This is not done here, but could be easily added to this program
 */ 

/* The ATMega4809 on the Arduino Every uses a CPU clock of 20 MHz - hence this is set as F_CPU */
/* Note, however, that by default this is initially divided by 6 for LV compatibility reasons 
 * That's why Clock_Init() disables the clock pre-scaler by writing 0 to the MCLK Control Reg B */
/* Note also that the on-chip high frequency oscillator can run at 16MHz or 20MHz. This is set 
 * by the oscillator FUSE selection set at programming time, not under program control
 * In this module, we use avrdude to select the 20 MHz option */
/* Note that a number of different Port Registers are used for the GPIO pins
 * The LED_Array struct groups the PORTn Register and the corresponding PORT bit together */

#include <avr/io.h>
#include <avr/cpufunc.h>
#define F_CPU 20.0E6
#include <util/delay.h>

/* delay.h is needed for the software delays used later. It needs to have F_CPU defined before it's included */

#define DELAY_QTR_SEC 250000
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
	/* set UNO D0-D7 to all outputs  */
	PORTC.DIR = 0b01110000; /* or 0x70;			PC4-UNO D1 (TXD1), PC5-UNO D0 (RXD1), PC6 - UNO D4  */
	PORTA.DIR = 0b00000011; /* or 0x03;			PA1-UNO D7, PA0 - UNO D2 */
	PORTB.DIR = 0b00000100; /* or 0x04;			PB2 - UNO D5 */
	PORTF.DIR = 0b00110000; /* or 0x30;			PF5 - UNO D3, PF4 UNO D6 */
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

void ClearPorts() {

	PORTC.OUTCLR = 0b00100000;  /* or 0x20;		D0 <- 0 */
	PORTC.OUTCLR = 0b00010000;  /* or 0x10;		D1 <- 0 */
	PORTA.OUTCLR = 0b00000001;  /* or 0x01;		D2 <- 0 */
	PORTF.OUTCLR = 0b00100000;  /* or 0x20;		D3 <- 0 */
	PORTC.OUTCLR = 0b01000000;  /* or 0x40;		D4 <- 0 */
	PORTB.OUTCLR = 0b00000100;  /* or 0x04;		D5 <- 0 */
	PORTF.OUTCLR = 0b00010000;  /* or 0x10;		D6 <- 0 */
	PORTA.OUTCLR = 0b00000010;  /* or 0x02;		D7 <- 0 */
}

void SetPorts() {
	
	PORTC.OUTSET = 0b00100000;  /* or 0x20;		D0 <- 1 */
	PORTC.OUTSET = 0b00010000;  /* or 0x10;		D1 <- 1 */
	PORTA.OUTSET = 0b00000001;  /* or 0x01;		D2 <- 1 */
	PORTF.OUTSET = 0b00100000;  /* or 0x20;		D3 <- 1 */
	PORTC.OUTSET = 0b01000000;  /* or 0x40;		D4 <- 1 */
	PORTB.OUTSET = 0b00000100;  /* or 0x04;		D5 <- 1 */
	PORTF.OUTSET = 0b00010000;  /* or 0x10;		D6 <- 1 */
	PORTA.OUTSET = 0b00000010;  /* or 0x02;		D7 <- 1 */
}

void move_leds(void){
	uint8_t i;
	for (i=0; i < 7; i++)
	{
		if (i == 0) {
			PORTC.OUT = PORTC.OUT & 0b11101111;  /* or 0xef; or ~0x10;		D1 <- 0 */
			PORTC.OUT = PORTC.OUT | 0b00100000;  /* or 0x20;		D0 <- 1 */
		}
		else if (i == 1) {
			PORTC.OUT = PORTC.OUT & 0b11011111;  /* or 0xdf; or ~0x20;		D0 <- 0 */
			PORTC.OUT = PORTC.OUT | 0b00010000;  /* or 0x10;		D1 <- 1 */
		}
		else if (i == 2) {
			PORTC.OUT = PORTC.OUT & 0b11101111;  /* or 0xef; or ~0x10;		D1 <- 0 */
			PORTA.OUT = PORTA.OUT | 0b00000001;  /* or 0x01;		D2 <- 1 */
		}
		else if (i == 3) {
			PORTA.OUT = PORTA.OUT & 011111110;  /* or 0xfe; or ~0x01;		D2 <- 0 */
			PORTF.OUT = PORTF.OUT | 0b00100000;  /* or 0x20;		D3 <- 1 */
		}
		else if (i == 4) {
			PORTF.OUT = PORTF.OUT & 011011111;  /* 0xdf; or ~0x20;		D3 <- 0 */
			PORTC.OUT = PORTC.OUT | 0b01000000;  /* or 0x40;		D4 <- 1 */
		}
		else if (i == 5) {
			PORTC.OUT = PORTC.OUT & 0b10111111;  /* 0xbf; or ~0x40;		D4 <- 0 */
			PORTB.OUT = PORTB.OUT | 0b00000100;  /* or 0x04;		D5 <- 1 */
		}
		else {
			PORTB.OUT = PORTB.OUT & 0b11111011;  /* 0xfb or ~0x04;		D5 <- 0 */
			PORTF.OUT = PORTF.OUT | 0b00010000;  /* or 0x10;		D6 <- 1 */
		}
		
		_delay_us(DELAY_QTR_SEC);
	}
	for (i=7; i > 0; i--)
	{
		if (i == 7){
			PORTF.OUT = PORTF.OUT & 0b11101111;  /* or 0xef or ~0x10;		D6 <- 0 */
			PORTA.OUT = PORTA.OUT | 0b00000010;  /* or 0x02;		D7 <- 1 */
		}
		else if (i == 6) {
			PORTA.OUT = PORTA.OUT & 0b11111101;  /* or 0xfd; or 0x02;		D7 <- 0 */
			PORTF.OUT = PORTF.OUT | 0b00010000;  /* or 0x10;		D6 <- 1 */
		}
		else if (i == 5) {
			PORTF.OUT = PORTF.OUT & 0b11101111;  /* or 0xef or ~0x10;		D6 <- 0 */
			PORTB.OUT = PORTB.OUT | 0b00000100;  /* or 0x04;		D5 <- 1 */
		}
		else if (i == 4) {
			PORTB.OUT = PORTB.OUT & 0b11111011;  /* or 0xfb or ~0x04;		D5 <- 0 */
			PORTC.OUT = PORTC.OUT | 0b01000000;  /* or 0x40;		D4 <- 1 */
		}
		else if (i == 3) {
			PORTC.OUT = PORTC.OUT & 0b10111111;  /* or 0xbf; or ~0x40;		D4 <- 0 */
			PORTF.OUT = PORTF.OUT | 0b00100000;  /* or 0x20;		D3 <- 1 */
		}
		else if (i == 2) {
			PORTF.OUT = PORTF.OUT & 0b11011111;  /* or 0xdf; or ~0x20;		D3 <- 0 */
			PORTA.OUT = PORTA.OUT | 0b00000001;  /* or 0x01;		D2 <- 1 */
		}
		else {
			PORTA.OUT = PORTA.OUT & 0b11111110;  /* or 0xfe; or ~0x01;		D2 <- 0 */
			PORTC.OUT = PORTC.OUT | 0b00010000;  /* or 0x10;		D1 <- 1 */
		}
		
		_delay_us(DELAY_QTR_SEC);
	}
}

