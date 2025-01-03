/*
 * MotorTest2.c
 *
 * Created: 4/30/2024 11:12:46 AM
 * Author : west3
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#define F_CPU 20000000
#include <util/delay.h>

#define F_PER 20000000
#define MAXCOUNT 20
#define STEP_SIZE 63

//#define BIT_NAMING 1

void CLOCK_init (void);

void WG_SSPWM_init(void);

int main(void)
{
	uint16_t servoCounter = 0;
	
	CLOCK_init();
	
	WG_SSPWM_init();
	
	while (1)
	{
		for (servoCounter = 0; servoCounter < MAXCOUNT; servoCounter++)
		{
			TCA0.SINGLE.CMP1BUF = 1250 + (STEP_SIZE * servoCounter);
			_delay_ms(50);
		}
		for (servoCounter = MAXCOUNT; servoCounter > 1; servoCounter--)
		{
			TCA0.SINGLE.CMP1BUF = 1250 + (STEP_SIZE * servoCounter);
			_delay_ms(50);
		}
		
	}
}

void CLOCK_init (void)
{
	/* Disable CLK_PER Prescaler */
	ccp_write_io( (void *) &CLKCTRL.MCLKCTRLB , (0 << CLKCTRL_PEN_bp));
	/* If set from the fuses during device programming, the CPU will now run at 20MHz (default is /6) */
}



void WG_SSPWM_init(void)
{
	#ifdef BITNAMING
	PORTA.DIRSET = PIN1_bm;		// WO-1 used for PWM

	TCA0.SINGLE.CTRLA = (TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm);
	
	TCA0.SINGLE.CTRLB = (TCA_SINGLE_CMP1EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc);
	/* Frequency generation mode, with two output pins selected - this takes over the PORT pins */
	TCA0.SINGLE.PER = 24999; /* It's up to the designer to verify the target frequency for the end use */
	/* CMP0 gives the frequency. CMP1 is an offset (delayed) version */
	TCA0.SINGLE.CMP1 = 1250;		/* Start with 0 duty cycle */
	/* No interrupts used here ! */

	#else
	PORTA.DIRSET = PIN1_bm;		// WO-1 used for PWM
	TCA0.SINGLE.CTRLA = 0b00001001;  /* (TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm); */
	/*  we need 50Hz, 20ms period */
	TCA0.SINGLE.CTRLB = 0b00100011;  /* (TCA_SINGLE_CMP1EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc); */
	/* Frequency generation mode, with two output pins selected - this takes over the PORT pins */
	TCA0.SINGLE.PER = 24999; /* It's up to the designer to verify the target frequency for the end use */
	/* CMP0 gives the frequency. CMP1 is an offset (delayed) version */
	TCA0.SINGLE.CMP1 = 1250;		/* Start with 0 duty cycle */
	/* No interrupts used here ! */
	
	#endif

}

