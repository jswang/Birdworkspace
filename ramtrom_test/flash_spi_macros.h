#ifndef FLASH_SPI_MACROS_H_
#define FLASH_SPI_MACROS_H_

/** Specific ports */
#define CE_OUT 	P4OUT	//Using port 4.3 for chipo select
#define CE_PORT	BIT3
/*******************/

/* TI's workaround Macros*****/
#define st(x) 		do{x} while (__LINE__ == -1)
/*******************/

/* SPI Macros ********/
#define SPI_TURN_CHIP_SELECT_OFF() 		CE_OUT |= CE_PORT
#define SPI_TURN_CHIP_SELECT_ON() 		CE_OUT &= ~CE_PORT
#define SPI_WRITE_BYTE(x)				st(IFG2 &= ~UCB0RXIFG; UCB0TXBUF = x;) 
#define SPI_WAIT_DONE()					while(!(IFG2 & UCB0RXIFG));
#define SPI_READ_BYTE()					UCB0RXBUF







#endif /*FLASH_SPI_MACROS_H_*/
