#include "msp430x22x4.h"
#include "flash_spi_macros.h"
#include <stdint.h>
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
//CLOCK(UCB0CLK)<-|P3.3         P4.3|-> Chip Enable
//
//int spi_read_jedec();



void main(void) {
	unsigned long i;
	char test[] = "Hello world! ";
	int length = 5;
	Mem_block mem;
	mem.length = length;
	mem.base_addr = (uint8_t*)test;
	//char value = *test will get the first eleme t of test
//	unsigned char rxarray[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    WDTCTL = WDTPW + WDTHOLD; // Stop watch-dog timer
  	BCSCTL1 = CALBC1_1MHZ;	//Set DCO to 1MHz, DIVA=1, XT2OFF=1 (XT2 off), XTS=0 (LF mode)
	DCOCTL = CALDCO_1MHZ;	//Set DCO to 1MHz
	BCSCTL1 |= DIVA_3;		//DIVA=8
	BCSCTL2 = (SELM_0 | DIVM_3 | DIVS_0);	//MCLK = DCO/8; SMCLK=DCO
	FCTL2 = (FWKEY + FSSEL_2 + FN1);	// SMCLK/3 for Flash Timing Generator

/*************************test area*********/
	i = 0x3456;

	write(i, &mem);
/***********************/




	// this line is actually necessary even though we are not using port 3
	// if not present, the signal received from the chip looks strange
	P3DIR |= BIT0 | BIT3 | BIT1; //sets all pin3 to output except bit3 and bit1
	//Set chip select value high
	//P3OUT|= BIT0;
	P4DIR |= BIT3; 	//chip enable
	P4OUT |= BIT3;

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
	P4OUT |= BIT3;
	P4OUT &= ~BIT3; 	//Set chip enable low to activate
	//wait for t, power up time fore the FM25V20 is operational
	{	int i;
		for (i= 0; i < 10000; i++);
	}
	read_id();

}

uint8_t write(unsigned long addr_write_to, Mem_block* mem){
	short sbytes[4] = {0,0,0,0};
	uint8_t* i;
	sbytes[0] = (addr_write_to >> 24) & 0xFF; //not necessary but useful to see
	sbytes[1] = (addr_write_to >> 16) & 0xFF;
	sbytes[2] = (addr_write_to >> 8) & 0xFF;
	sbytes[3] =  addr_write_to & 0xFF;
	SPI_TURN_CHIP_SELECT_OFF();
  	SPI_TURN_CHIP_SELECT_ON();
  	SPI_WRITE_BYTE(WREN);
	SPI_WAIT_DONE();
	SPI_WRITE_BYTE(WRITE);
	SPI_WAIT_DONE();
	SPI_WRITE_BYTE(sbytes[1]);
	SPI_WAIT_DONE();
	SPI_WRITE_BYTE(sbytes[2]);
	SPI_WAIT_DONE();
	SPI_WRITE_BYTE(sbytes[3]);
	SPI_WAIT_DONE();

	for (i = mem->base_addr; i < mem->base_addr + mem->length; i++){
		SPI_WRITE_BYTE(*i);
		SPI_WAIT_DONE();
	}


}

uint8_t sleep_on(void){
	SPI_TURN_CHIP_SELECT_OFF();
	SPI_TURN_CHIP_SELECT_ON();
	SPI_WRITE_BYTE(SLEEP);
	SPI_TURN_CHIP_SELECT_OFF();
}

uint8_t wake(void){
	read_id();
}

uint8_t read_id(void){
	volatile unsigned int rxarray[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned int i;
	SPI_TURN_CHIP_SELECT_OFF();
  	SPI_TURN_CHIP_SELECT_ON();
  	SPI_WRITE_BYTE(RDID);
	SPI_WAIT_DONE();
	//first received is dummy
	for (i =0; i < 9; i++){
		SPI_WRITE_BYTE(0x00);
		SPI_WAIT_DONE();
		rxarray[i] = SPI_READ_BYTE();
	}
	SPI_TURN_CHIP_SELECT_OFF();
	return 0;
}

static uint8_t spiRegAccess(uint8_t writeValue)
{
	uint8_t readValue;
	/* turn chip select "off" and then "on" to clear any current SPI access */
  	SPI_TURN_CHIP_SELECT_OFF();
  	SPI_TURN_CHIP_SELECT_ON();

	/*
   *  Send the byte value to write.  If this operation is a read, this value
   *  is not used and is just dummy data.  Wait for SPI access to complete.
   */
   	SPI_WRITE_BYTE(writeValue);
  	SPI_WAIT_DONE();

  	/*
   *  If this is a read operation, SPI data register now contains the register
   *  value which will be returned.  For a read operation, it contains junk info
   *  that is not used.
   */
  	readValue = SPI_READ_BYTE();
	return 0;
}


//CS must go high to reset spi interface for the chip

//	while(1){
//		{int i;
//			for (i= 0; i < 50; i++);
//		}
//		P4OUT &= ~BIT3; 	//SEt chip enable low to activate
//			//wait for t, power up time fore the FM25V20 is operational
//
//		IFG2 &= ~UCB0RXIFG;	//Reset interrupt flag
//		UCB0TXBUF = 0x9F; //Send in first command to slave
//		while (!(IFG2 & UCB0RXIFG));
//		rxarray[0] = UCB0RXBUF;
//		UCB0TXBUF = 0x00; //Send in first command to slave
//		while (!(IFG2 & UCB0RXIFG));
//		rxarray[1] = UCB0RXBUF;
//		UCB0TXBUF = 0x00; //Send in first command to slave
//		while (!(IFG2 & UCB0RXIFG));
//		rxarray[2] = UCB0RXBUF;
//		UCB0TXBUF = 0x00; //Send in first command to slave
//		while (!(IFG2 & UCB0RXIFG));
//		rxarray[3] = UCB0RXBUF;
//		UCB0TXBUF = 0x00; //Send in first command to slave
//		while (!(IFG2 & UCB0RXIFG));
//		rxarray[4] = UCB0RXBUF;
//		UCB0TXBUF = 0x00; //Send in first command to slave
//		while (!(IFG2 & UCB0RXIFG));
//		rxarray[5] = UCB0RXBUF;
//		UCB0TXBUF = 0x00; //Send in first command to slave
//		while (!(IFG2 & UCB0RXIFG));
//		rxarray[6] = UCB0RXBUF;
//		UCB0TXBUF = 0x00; //Send in first command to slave
//		while (!(IFG2 & UCB0RXIFG));
//		rxarray[7] = UCB0RXBUF;
//		UCB0TXBUF = 0x00; //Send in first command to slave
//		while (!(IFG2 & UCB0RXIFG));
//		rxarray[8] = UCB0RXBUF;
//		UCB0TXBUF = 0x00; //Send in first   to slave
//		while (!(IFG2 & UCB0RXIFG));
//		rxarray[9] = UCB0RXBUF;
//		P4OUT |= BIT3;
//
//	}
//	IFG2 &= ~UCB0RXIFG;	//Reset interrupt flag
//	P3OUT |= BIT0;	//chip select is off
//	//re-enable interrupts at the end for future use
//   while(1);
