/*
 * Project1.c
 *
 * Created: 2/27/2024 11:16:31 AM
 * Author : Westin Gjervold
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#define F_CPU 20.0E6

/* Initialize relevant variables */
#define NUM_LED_BITS 10
#define DELAY_EIGHTH_SEC 125000
#define DELAY_HALF_SEC 500000
#define DELAY_ONE_SEC 1000000
#define ZERO_5V 102
#define ONE_0V	205
#define ONE_5V	307
#define TWO_0V	409
#define TWO_5V	512
#define THREE_0V 614
#define THREE_5V 716
#define FOUR_0V	818
#define FOUR_5V 921
#define FIVE_0V 1023
#define Seven_Tenths 716


struct LED_BITS
{
	PORT_t *LED_PORT;
	uint8_t bit_mapping;
};

struct LED_BITS LED_Array[NUM_LED_BITS] = {
	{&PORTC, 1<<5}, {&PORTC, 1<<4}, {&PORTA, 1<<0}, {&PORTF, 1<<5}, {&PORTC, 1<<6}, {&PORTB, 1<<2}, {&PORTF, 1<<4}, {&PORTA, 1<<1}, {&PORTA, 1<<2}, {&PORTA, 1<<3}
};

void Set_Clear_Ports(uint8_t set);
void Thermometer_Display(void);
void Cylon_Eyes(void);
void Inverse_Cylon_Eyes(void);
void Update_Current_LED(void);
uint16_t adc_result;
uint8_t new_adc_data;
uint8_t current_LED = 0;
uint8_t left_or_right = 1;
uint8_t Inverse_Mode = 0b00000000;

void CLOCK_init (void)
{
	/* Disable CLK_PER Prescaler */
	ccp_write_io( (void *) &CLKCTRL.MCLKCTRLB , (0 << CLKCTRL_PEN_bp));
	/* If set from the fuses during device programming, the CPU will now run at 20MHz (default is /6) */
}

void InitialiseLED_PORT_bits()
{
	uint8_t i;
	for (i = 0; i <= (NUM_LED_BITS - 1); i += 1) {
		LED_Array[i].LED_PORT->DIRSET = LED_Array[i].bit_mapping;
	}
}

void InitialiseButton_PORT_bits(void)
{
	PORTE.DIRCLR = PIN1_bm;		
	PORTE.PIN1CTRL |= PORT_PULLUPEN_bm;
}

void TCA0_init_bits(void)
{
	TCA0.SINGLE.INTCTRL = 0b00000001;	//TCA0.SINGLE.INTCTRL set to Timer Overflow Interrupt enabled.
	TCA0.SINGLE.CTRLB = 0b00000000;		//Select normal mode operation by setting TCA0.SINGLE.CTRLB to 0.
	TCA0.SINGLE.PER = 2445;				//Set the TCA0.SINGLE.PER register to give the desired timer/counter period.
	TCA0.SINGLE.CTRLA = 0b00001111;		//Clock source set to divide by 1024, TCA0 enabled.
}

void ADC0_init(void)
{
	ADC0.CTRLA = 0b00000011;		//10-bit resolution, Free Running Mode selected, ADC0 enabled
	ADC0.CTRLB = 0b00000000;		//Simple conversion selected (no sample accumulation)
	ADC0.CTRLC = 0b01010110;		//SAMPCAP=1; REFSEL: VDD; PRESC set to DIV128
	ADC0.CTRLD = 0b00100000;		//Initial delay set to 16 clocks
	ADC0.MUXPOS = 0b00000011;		//ADC0 AIN3 used
	ADC0.INTCTRL = 0b00000001;		//ADC Interrupt on Result Ready Enabled
	ADC0.COMMAND = 0b00000001;		//Set the STCONV bit in the ADC0.COMMAND register in the initialization to begin the first ADC0 conversion.
}

void RTC_PIT_init()
{
	RTC.CLKSEL = 0b00000001;       //RTC.CLKSEL set for 1024 Hz internal clock selected.
	RTC.PITCTRLA = 0b01101001;     //Use the RTC.PITCTRLA register to select a time-period that will give a 16 second period and also enable the RTC PIT.
	RTC.PITINTCTRL = RTC_PI_bm;	   //Use the RTC.PITINTCTRL to enable the RTC PIT interrupt.
}

int main(void)
{
	CLOCK_init();						// Set up clock counter
	InitialiseButton_PORT_bits();		// Set up port to read button
	InitialiseLED_PORT_bits();			// Set bits on ports as outputs to drive LEDS
	TCA0_init_bits(); 					// Set up the Timer Counter A
	ADC0_init();						// Set up the ADC
	RTC_PIT_init();						// Set up RTC periodic interrupt
	sei();								// Enable interrupts
	
    while (1)							// Main Loop
    {		
		if (!(PORTE.IN & PIN1_bm)){		//Test PORTE bit 1
			if(new_adc_data == 1){		//Check for new ADC result available
				Thermometer_Display();	//Display the ADC0 output in thermometer format
				new_adc_data = 0;		//Clear new ADC result available flag
			}
		}
    }
}
/* Timer TCA0 ISR */
ISR(TCA0_OVF_vect)
{
	if(PORTE.IN & PIN1_bm){					//Test PORTE bit 1
		if (!Inverse_Mode){					//If 1 (Button NOT pressed) Display Cylon eyes output on LED array.
			Set_Clear_Ports(0);
			Cylon_Eyes();
		}
		else{
			Set_Clear_Ports(1);
			Inverse_Cylon_Eyes();
		}
		Update_Current_LED();
	}
	TCA0.SINGLE.INTFLAGS = 0b00000001;		//Clear Interrupt Flag
}
/* ADC ISR */
ISR(ADC0_RESRDY_vect)
{	
	adc_result = ADC0.RES;				//Read ADC result and store in a global variable
	if (adc_result > Seven_Tenths){		//Set Timer TCA0 ISR time value based on ADC0 result
		TCA0.SINGLE.PERBUF = 4890;		//Set up time for 0.5sec time interval
	}
	else{
		TCA0.SINGLE.PERBUF = 19561;		//Set up time for 0.125sec time interval
	}
	new_adc_data = 1;					//Set new ADC result available flag
}
/* RTC ISR */
ISR(RTC_PIT_vect)
{
	RTC.PITINTFLAGS = 0b00000001;  //Clear RTC PIT Timer interrupt flag
	Inverse_Mode ^= 0b00000001;   //Change the state of the display mode – cylon or inverted cylon display
}

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

void Thermometer_Display(void){
	Set_Clear_Ports(0);
	if (adc_result < ZERO_5V) {
		for(int i = 0; i < 1; i++){
			LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
		}
	}
	else if (adc_result < ONE_0V) {
		for(int i = 0; i < 2; i++){
			LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
		}
	}
	else if (adc_result < ONE_5V) {
		for(int i = 0; i < 3; i++){
			LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
		}
	}
	else if (adc_result < TWO_0V) {
		for(int i = 0; i < 4; i++){
			LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
		}
	}
	else if (adc_result < TWO_5V) {
		for(int i = 0; i < 5; i++){
			LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
		}
	}
	else if (adc_result < THREE_0V) {
		for(int i = 0; i < 6; i++){
			LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
		}
	}
	else if (adc_result < THREE_5V) {
		for(int i = 0; i < 7; i++){
			LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
		}
	}
	else if (adc_result < FOUR_0V) {
		for(int i = 0; i < 8; i++){
			LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
		}
	}
	else if (adc_result < FOUR_5V) {
		for(int i = 0; i < 9; i++){
			LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
		}
	}
	else {
		for(int i = 0; i < 10; i++){
			LED_Array[i].LED_PORT->OUTSET = LED_Array[i].bit_mapping;
		}
	}
}

void Cylon_Eyes(void){
	if(left_or_right == 1){
		if(current_LED == 9){
			LED_Array[current_LED-1].LED_PORT->OUTSET = LED_Array[current_LED-1].bit_mapping;
			left_or_right = 0;
		}
		else{
			LED_Array[current_LED+1].LED_PORT->OUTSET = LED_Array[current_LED+1].bit_mapping;
		}
	}
	else if(left_or_right == 0){
		if(current_LED == 0){
			LED_Array[current_LED+1].LED_PORT->OUTSET = LED_Array[current_LED+1].bit_mapping;
			left_or_right = 1;
		}
		else{
			LED_Array[current_LED-1].LED_PORT->OUTSET = LED_Array[current_LED-1].bit_mapping;
		}
	}
}

void Inverse_Cylon_Eyes(void){
	if(left_or_right == 1){
		if(current_LED == 9){
			LED_Array[current_LED-1].LED_PORT->OUTCLR = LED_Array[current_LED-1].bit_mapping;
			left_or_right = 0;
		}
		else{
			LED_Array[current_LED+1].LED_PORT->OUTCLR = LED_Array[current_LED+1].bit_mapping;
		}
	}
	else if(left_or_right == 0){
		if(current_LED == 0){
			LED_Array[current_LED+1].LED_PORT->OUTCLR = LED_Array[current_LED+1].bit_mapping;
			left_or_right = 1;
		}
		else{
			LED_Array[current_LED-1].LED_PORT->OUTCLR = LED_Array[current_LED-1].bit_mapping;
		}
	}
}

void Update_Current_LED(void){
	if(left_or_right == 0){
		current_LED--;
	}
	else if(left_or_right == 1){
		current_LED++;
	}
}
