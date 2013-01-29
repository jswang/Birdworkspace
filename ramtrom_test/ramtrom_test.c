#include "msp430x22x4.h"
unsigned char MST_Data, SLV_Data;
unsigned char next_instruction;
volatile unsigned int count;
volatile unsigned char x;
volatile unsigned char done;
volatile char received;
volatile char transmitBuffer;

//
//                    MSP430F22x4
//                 -----------------
//             /|\|              XIN|-
//              | |                 |
//              --|RST          XOUT|-
//                |                 |
//                |             P3.2|<- Data In (UCB0SOMI)
//                |                 |
//          LED <-|P1.0         P3.1|-> Data Out (UCB0SIMO)
//                |                 |
//CLOCK(UCB0CLK)<-|P3.3         P3.0|-> Chip Enable
//
//int spi_read_jedec();

void main(void) {
	volatile unsigned int i;
	unsigned char rxarray[4] = {0, 0, 0, 0};
    done= 0;
    
    count = 0;
    WDTCTL = WDTPW + WDTHOLD; // Stop watch-dog timer
  	BCSCTL1 = CALBC1_1MHZ;	//Set DCO to 1MHz, DIVA=1, XT2OFF=1 (XT2 off), XTS=0 (LF mode)
	DCOCTL = CALDCO_1MHZ;	//Set DCO to 1MHz
	BCSCTL1 |= DIVA_3;		//DIVA=8
	BCSCTL2 = (SELM_0 | DIVM_3 | DIVS_0);	//MCLK = DCO/8; SMCLK=DCO
	FCTL2 = (FWKEY + FSSEL_2 + FN1);	// SMCLK/3 for Flash Timing Generator
	
	P3DIR |= BIT0 | BIT3 | BIT1; //sets all pin3 to output except bit3 and bit1
	//Set chip select value high
	P3OUT|= BIT0;
	P4OUT |= BIT3; 	//chip enable 
	
	//initialize macro
	UCB0CTL1 = UCSWRST;                           
  	UCB0CTL1 = UCSWRST | UCSSEL1;                 
  	UCB0CTL0 = UCCKPH | UCMSB | UCMST | UCSYNC;   
  	UCB0BR0  = 2;                                 
  	UCB0BR1  = 0;                                 
  	P3SEL |= BIT3 | BIT1 | BIT2;  // P3.1,2,3 USCI_B0 option select            
  	UCB0CTL1 &= ~UCSWRST;  	//Initialize USBO state machine 
 	                      
	/**********************/
	//Call SPI to transmit a bit
	__disable_interrupt();
	P3OUT |= BIT0; 		//Set chip enable high
	P3OUT &= ~BIT0; 	//SEt chip enable low to activate
	P4OUT |= BIT3;
	P4OUT &= ~BIT3;
	
	while(1){
		IFG2 &= ~UCB0RXIFG;	//Reset interrupt flag
		UCB0TXBUF = 0x11; //Send in first command to slave
		while (!(IFG2 & UCB0RXIFG));
		rxarray[0] = UCB0RXBUF;
//		IFG2 &= ~UCB0RXIFG;	//Reset interrupt flag
//		UCB0TXBUF = 0x00; //Send in first command to slave
//		while (!(IFG2 & UCB0RXIFG));
//		rxarray[1] = UCB0RXBUF;
//		UCB0TXBUF = 0x00; //Send in first command to slave
//		while (!(IFG2 & UCB0RXIFG));
//		rxarray[2] = UCB0RXBUF;
//		UCB0TXBUF = 0x00; //Send in first command to slave
//		while (!(IFG2 & UCB0RXIFG));
//		rxarray[3] = UCB0RXBUF;
//		P3OUT |= BIT0;
	}
	IFG2 &= ~UCB0RXIFG;	//Reset interrupt flag
	P3OUT |= BIT0;	//chip select is off
	//re-enable interrupts at the end for future use
	
	
	
	//__bis_SR_register(OSCOFF);	Maybe need it? prolly not
 
    
//    while (!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
//    UCB0TXBUF = 0x00;
//    while (!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
//    UCB0TXBUF = 0xFF;
    
//    while (!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
//    rarray[0] = UCB0RXBUF;  //store the value in received array
//    UCB0TXBUF = 0x00; 
//    while (!(IFG2 & UCB0TXIFG)); 
//    rarray[1] = UCB0RXBUF;
//    UCB0TXBUF = 0x00; 
//    while (!(IFG2 & UCB0TXIFG)); 
//    rarray[2] = UCB0RXBUF;
//    UCB0TXBUF = 0x00; 
//    while (!(IFG2 & UCB0TXIFG)); 
//    rarray[3] = UCB0RXBUF;
//    
	
    //UCB0TXBUF = 0x; //Read manufacturer ID command
    //__bis_SR_register(GIE);	//Enable interrupts
    //spi_read_jedec();
    
     
   while(1);
}

//CS must go high to reset spi interface for the chip 
//TODO: make it a fixed length for bytes coming out. 
// Test for valid RX and TX character
//#pragma vector=USCIAB0RX_VECTOR
//__interrupt void USCIB0RX_ISR(void) {
//	volatile unsigned int i;
//	volatile unsigned int rxbuf; 
//	
//	while (!(IFG2 & UCB0TXIFG)); // USCI_B0 TX buffer ready?
//	rxbuf = UCB0RXBUF;
//	if (rxbuf == 0xBF)	// Test for correct character RX'd
//    	P1OUT |= 0x02;      // If correct, turn on green LED
//    else if (rxbuf == 0x9f) //test to make sure setup was correct, if SIMO attached to SOMI will light up
//    	P1OUT |= 0x03;
//  	else{
//    	P1OUT |= 0x01;     // If incorrect, turn on red LED
//  	}
//}
