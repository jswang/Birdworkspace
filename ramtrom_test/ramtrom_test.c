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
volatile unsigned int checkmain[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


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
	unsigned int test_data[]= {0xEE, 0xCC, 0xDD, 0xAA, 0xBB, 0xAD, 0x28, 0xBB};

	char rxarray[MAX_DATA_CHUNK] = {0};


	//char value = *test will get the first element of test
    WDTCTL = WDTPW + WDTHOLD; // Stop watch-dog timer
  	BCSCTL1 = CALBC1_1MHZ;	//Set DCO to 1MHz, DIVA=1, XT2OFF=1 (XT2 off), XTS=0 (LF mode)
	DCOCTL = CALDCO_1MHZ;	//Set DCO to 1MHz
	BCSCTL1 |= DIVA_3;		//DIVA=8
	BCSCTL2 = (SELM_0 | DIVM_3 | DIVS_0);	//MCLK = DCO/8; SMCLK=DCO
	FCTL2 = (FWKEY + FSSEL_2 + FN1);	// SMCLK/3 for Flash Timing Generator

	// this line is actually necessary even though we are not using port 3
	// if not present, the signal received from the chip looks strange
	P3DIR |= BIT0 | BIT3 | BIT1; //sets all pin3 to output except bit3 and bit1
	//Set chip select value high
	CE_DIR |= CE_PORT; 	//chip enable
	CE_OUT |= CE_PORT;

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
	//wait for t, power up time fore the FM25V20 is operational
	{	int i;
		for (i= 0; i < 10000; i++);
	}
	//*******************************************************//
	//*******************************************************//
	//*******************************************************//
	//*******************************************************//
	//*******************************************************//
	//read_id((uint8_t*) rxarray);
	int h;
	h=1;

	char test[]= {"Hello World, this is a long message"};
	i = 0x101010;
	ramtron_write(i, (uint8_t*) test, (uint8_t) DATA_LENGTH);
	ramtron_read(i, (uint8_t*) rxarray, (uint8_t) DATA_LENGTH);

	h=3;
	//*******************************************************//
	//*******************************************************//
}


void ramtron_write(unsigned long addr_write_to, uint8_t *pData, uint8_t len){
	short sbytes[4] = {0,0,0,0};
	sbytes[0] = (addr_write_to >> 24) & 0xFF; //not necessary but useful to see
	sbytes[1] = (addr_write_to >> 16) & 0xFF;
	sbytes[2] = (addr_write_to >> 8) & 0xFF;
	sbytes[3] =  addr_write_to & 0xFF;
	SPI_TURN_CHIP_SELECT_OFF(); // MODIFIED: add CE drop here
	SPI_TURN_CHIP_SELECT_ON();
	SPI_WRITE_BYTE(WREN);
	SPI_WAIT_DONE();
	SPI_TURN_CHIP_SELECT_OFF();
  	SPI_TURN_CHIP_SELECT_ON();
	SPI_WRITE_BYTE(WRITE);
	SPI_WAIT_DONE();
	SPI_WRITE_BYTE(sbytes[1]);
	SPI_WAIT_DONE();
	SPI_WRITE_BYTE(sbytes[2]);
	SPI_WAIT_DONE();
	SPI_WRITE_BYTE(sbytes[3]);
	SPI_WAIT_DONE();
	/*-------------------------------------------------------------------------------
     *  Inner loop.  This loop executes as long as the SPI access is
     * not interrupted. Loop completes when nothing left to transfer.
     */
	do{
	   	SPI_WRITE_BYTE(*pData);
	   	len--;
	  	SPI_WAIT_DONE();
	   	pData++;
	}while (len);
    SPI_TURN_CHIP_SELECT_OFF();
}

void ramtron_read(unsigned long addr_write_to, uint8_t *pData, uint8_t len){
	short sbytes[4] = {0,0,0,0};
	sbytes[0] = (addr_write_to >> 24) & 0xFF; //not necessary but useful to see
	sbytes[1] = (addr_write_to >> 16) & 0xFF;
	sbytes[2] = (addr_write_to >> 8) & 0xFF;
	sbytes[3] =  addr_write_to & 0xFF;
	SPI_TURN_CHIP_SELECT_OFF();
  	SPI_TURN_CHIP_SELECT_ON();
	SPI_WRITE_BYTE(READ);
	SPI_WAIT_DONE();
	SPI_WRITE_BYTE(sbytes[1]);
	SPI_WAIT_DONE();
	SPI_WRITE_BYTE(sbytes[2]);
	SPI_WAIT_DONE();
	SPI_WRITE_BYTE(sbytes[3]);
	SPI_WAIT_DONE();
	/*-------------------------------------------------------------------------------
     *  Inner loop.  This loop executes as long as the SPI access is
     * not interrupted. Loop completes when nothing left to transfer.
     */
	do{
	   	SPI_WRITE_BYTE(*pData);
	   	len--;
	   	SPI_WAIT_DONE();
	  	*pData = SPI_READ_BYTE();
	   	pData++;
	}while (len);
	SPI_TURN_CHIP_SELECT_OFF();
}

void ramtron_sleep_on(void){
	SPI_TURN_CHIP_SELECT_OFF();
	SPI_TURN_CHIP_SELECT_ON();
	SPI_WRITE_BYTE(SLEEP);
	SPI_TURN_CHIP_SELECT_OFF();
}

void ramtron_wake(void){
	//TODO: add something here? Not sure what, ask Deyu
	// MODIFIED: dummy read will wake up the ramtron and give enough setup time
	uint8_t dummy[8] = {0};
	ramtron_read_id(dummy);
}

void ramtron_read_id(uint8_t *pData){
	uint8_t len= 8; // MODIFIED: changed to an actual length... RDID returns 8 bytes total
	SPI_TURN_CHIP_SELECT_OFF();
  	SPI_TURN_CHIP_SELECT_ON();
  	SPI_WRITE_BYTE(RDID);
	SPI_WAIT_DONE();
	do{
		SPI_WRITE_BYTE(*pData);
		len--;
		SPI_WAIT_DONE();
		*pData = SPI_READ_BYTE();
		pData++;
	}while (len);
	SPI_TURN_CHIP_SELECT_OFF();
}

//CS must go high to reset spi interface for the chip

