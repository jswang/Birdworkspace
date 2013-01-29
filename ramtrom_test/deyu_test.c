#include "msp430x22x4.h"

unsigned char MST_Data, SLV_Data;
unsigned char next_instruction;
volatile unsigned int count;
volatile unsigned char x;
volatile unsigned char done;
volatile char received;
volatile char transmitBuffer;

void main(void) {
	done= 0;
	volatile unsigned int i;
	count = 0;
	WDTCTL = WDTPW + WDTHOLD; // Stop watch-dog timer
	P1OUT = 0x00; // P1 setup for LED
	P1DIR |= 0x03; // for the red and green LEDs
	//P2DIR |= 0xFF; // reset button
	//P2OUT |= 0xB1; // keep reset high for normal operation
	P4DIR |= 0xFF;
	P4OUT |= 0x08;
	P3DIR |= 0x0D; // 3.0,2,3 are data out
	P3SEL |= 0x0E; // P3.1,2,3 USCI_B0 option select
	// inactive state of clock is low, msb first, master mode, synchronous mode
	UCB0CTL1 |= UCSWRST;
	for (i = 60; i > 0; i--);

	UCB0CTL0 |= UCMSB + UCMST + UCSYNC; // 3-pin, 8-bit SPI master, master mode
	UCB0CTL1 |= UCSSEL_2; // SMCLK
	UCB0BR0 |= 0x02; //
	UCB0BR1 = 0; //

	UCB0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
	IE2 |= UCB0RXIE; // Enable USCI0 RX interrupt
	//IE2 |= UCB0TXIE;

	P1OUT |= 0x01; // turn on the red LED

	P3OUT |= 0x01;
	for (i = 60000; i > 0; i--);
	for (i = 60000; i > 0; i--);
	P3OUT &= ~0x01; // chip select signal

	/*P3OUT &= ~0x01;
	for (i = 60000; i > 0; i--);
	for (i = 60000; i > 0; i--);
	P3OUT |= 0x01; // chip select signal*/

	for (i = 50; i > 0; i--); // Wait for slave to initialize


	UCB0TXBUF = 0x9F;
	while (!(IFG2 & UCB0TXIFG));
	received= UCB0RXBUF;
	UCB0TXBUF = 0x9F;
	for (i = 10; i > 0; i--);

	while (1){
		while (!(IFG2 & UCB0TXIFG));
		received= UCB0RXBUF;
		UCB0TXBUF = 0x9F;
		transmitBuffer= UCB0TXBUF;

		for (i = 10; i > 0; i--);
	}

	__bis_SR_register(GIE);
	//__bis_SR_register(LPM0_bits);

	while(1);
}

// Test for valid RX and TX character
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCIB0TX_ISR(void) {
	transmitBuffer= UCB0TXBUF;
}

// Test for valid RX and TX character
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCIB0RX_ISR(void) {
	volatile unsigned int i;
	volatile char dataIn;

	while (!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?

	dataIn= UCB0RXBUF;
	if (dataIn == 0x7F) {
		P1OUT |= 0x02; // green LED
	}
	if (dataIn == 0xC2) {
		P1OUT &= ~0x01; // turn off red led
	}

	/*count=0;
	if (count <= 10000) {
		count += 1;
		UCB0TXBUF = 0x9F; // Send next value
	}*/

	UCB0TXBUF = 0xAA; // Send next value

	for (i = 30; i; i--); // Add time between transmissions to make sure slave can keep up
}

