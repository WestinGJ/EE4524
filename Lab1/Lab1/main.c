/*
 * Lab1.c
 *
 * Created: 2/6/2024 11:26:37 AM
 * Author : Westin Gjervold
 */ 

#include <avr/io.h>
#define F_CPU 3333333
#include <util/delay.h>
/* delay.h requires that F_CPU be defined before it is included */

#define DELAY_COUNT 6
/* For simulation purposes we do not want a small delay, for a real chip this would be much longer */


int main(void)
{
	
	
	PORTA.DIR = 0b11111111; /* or 0xff;			PA7-PA0 all outputs */
	
	
	while (1)
	{
		move_leds();
	}
	return 1;		/* This line is never reached, but prevents compiler warnings because we are returning data */
}

int move_leds(){
	uint8_t i = 0;
	for (i = 1; i < 128; i = i*2)
		{
			PORTA.OUT = ~i;			/* i goes from 1 -> 2 -> 4 etc */
			_delay_loop_2(DELAY_COUNT);
		}
		
		for (i = 128; i > 1; i= i/2)
		{
			PORTA.OUT = ~i;			/* i goes from 128 -> 64 -> 32 etc */
			_delay_loop_2(DELAY_COUNT);
		}
}

