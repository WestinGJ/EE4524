/*
 * twi_1621.c
 *
 * Created: 17/10/2023 15:17:45
 * Author : Ciaran.MacNamee
 */ 

#define F_CPU 20000000
#define USART3_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <stdio.h>
#include "twi_host.h"


#define I2C_BASE_ADDRESS                                    0x4c  
//address with 7 bits, could also be 0x48 depending on how Shield jumper position

 

/* DS1621 Command bytes (Control Reg addresses) */
#define READ_TEMP			0xaa 
#define ACCESS_CONFIG		0xac
#define START_CONVERT_T		0xee
#define STOP_CONVERT_T		0x22

/* Serial comms related globals */

unsigned char qcntr = 0,sndcntr = 0;   /*indexes into the queue*/
unsigned char queue[50];       /*character queue*/


void CLOCK_init (void);
static void USART3_init(void);
void sendmsg (char *s);
      


int main(void)
{
	uint8_t status_value, results[20], num_bytes = 0;
	
    I2C_0_init();
	
	char	ch;
	char	str_buffer[60];
		
	CLOCK_init();
	USART3_init();

	sei(); /* Enable Global Interrupts */

    while (1) 
    {
		 if (USART3.STATUS & USART_RXCIF_bm)
		 {	/* If a character has been received, read it - this structure allows other code to run */
			 ch = USART3.RXDATAL;
			 switch (ch)
			 {
				case 'a':
					status_value = do_I2C_write_param(ACCESS_CONFIG, 0);
					if (status_value) {
						sprintf(str_buffer, "I2C write completed - ACK\n");
						sendmsg(str_buffer);
					}
					else {
						sprintf(str_buffer, "I2C write completed - NACK or other error\n");
						sendmsg(str_buffer);	
					}
					num_bytes = 1;
					status_value = do_I2C_read_number(num_bytes, results);
					if (status_value == 0) {
						sprintf(str_buffer, "I2C READ - NACK or other error\n");
						sendmsg(str_buffer);
					}
					else {
						sprintf(str_buffer, "result1 = %x\t result2 = %x\n", results[0], results[1]);
						sendmsg(str_buffer);
					}
					break;
					
				case 'g':
				case 'G':
					status_value = do_I2C_write_param(START_CONVERT_T, 1);
					if (status_value) {
						sprintf(str_buffer, "I2C START_CONVERT completed - ACK\n");
						sendmsg(str_buffer);
					}
					else {
						sprintf(str_buffer, "I2C START_CONVERT - NACK or other error\n");
						sendmsg(str_buffer);
					}
					break;
					
				case 't':
				case 'T':
					status_value = do_I2C_write_param(READ_TEMP, 0);
					if (status_value) {
						sprintf(str_buffer, "I2C write completed - ACK\n");
						sendmsg(str_buffer);
					}
					else {
						sprintf(str_buffer, "I2C write completed - NACK or other error\n");
						sendmsg(str_buffer);
					}
					num_bytes = 2;
					status_value = do_I2C_read_number(num_bytes, results);
					if (status_value == 0) {
						sprintf(str_buffer, "I2C READ - NACK or other error\n");
						sendmsg(str_buffer);
					}
					else {
						sprintf(str_buffer, "result1 = %x\t result2 = %x\n", results[0], results[1]);
						sendmsg(str_buffer);
					}
				
					break;
				case 'b':
				case 'B':
					sprintf(str_buffer, "That was a B or a b\n");
					sendmsg(str_buffer);
					break;
				default:
					sprintf(str_buffer, "That was neither 'a' or 'b'\n");
					sendmsg(str_buffer);
					break;
			 }
		 }
		
    }
}


void CLOCK_init (void)
{
	/* Do not use low frequency clock, disable CLK_PER Prescaler */
	ccp_write_io( (void *) &CLKCTRL.MCLKCTRLB , (0 << CLKCTRL_PEN_bp));
	/* If set from the fuses during progamming, the CPU will now run at 20MHz (default is /6) */
}

static void USART3_init(void)
{
	PORTB.DIR &= ~PIN5_bm;		/* this is the RX input */
	PORTB.DIR |= PIN4_bm;		/* this is the TX output */
	USART3.BAUD = (uint16_t)USART3_BAUD_RATE(115200);
	USART3.CTRLB |= (USART_TXEN_bm | USART_RXEN_bm);
	PORTMUX.USARTROUTEA |= PORTMUX_USART3_ALT1_gc;
	
	USART3.CTRLA = USART_TXCIE_bm;
}

/*this function loads the queue and */
/*starts the sending process*/
void sendmsg (char *s)
{
	//if (qcntr == sndcntr)
	qcntr = 0;    /*preset indices*/
	sndcntr = 1;  /*set to one because first character already sent*/
	
	while (*s)
		queue[qcntr++] = *s++;   /*put characters into queue*/

	
	USART3.TXDATAL = queue[0];  /*send first character to start process*/
}

ISR(USART3_TXC_vect)
{
	/*send next character and increment index*/
	USART3.STATUS |= USART_TXCIF_bm;
	if (qcntr != sndcntr)
		USART3.TXDATAL = queue[sndcntr++];
	/* Stop sending when the queue is empty. TXC interrupts only happen when a character 
	   has been transmitted. Stopping sending stops the interrupts */
}

