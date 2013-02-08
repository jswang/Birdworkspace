#ifndef FLASH_SPI_MACROS_H_
#define FLASH_SPI_MACROS_H_

#include <stdint.h>
/** Specific ports */
#define CE_OUT 	P4OUT	//Using port 4.3 for chipo select
#define CE_PORT	BIT3
/*******************/

/* TI's workaround Macros*****/
#define st(x) 		do{x} while (__LINE__ == -1)
/*******************/

/*Function and type defintions**********/
typedef struct{
	uint8_t* base_addr; //contains address of one byte
	unsigned int length;
} Mem_block;
uint8_t read_id(void);
uint8_t write(unsigned long addr_write_to, Mem_block* mem);


/* SPI Macros ********/
#define SPI_TURN_CHIP_SELECT_OFF() 		CE_OUT |= CE_PORT
#define SPI_TURN_CHIP_SELECT_ON() 		CE_OUT &= ~CE_PORT
#define SPI_WRITE_BYTE(x)				st(IFG2 &= ~UCB0RXIFG; UCB0TXBUF = x;) 
#define SPI_WAIT_DONE()					while(!(IFG2 & UCB0RXIFG));
#define SPI_READ_BYTE()					UCB0RXBUF


/**********Ramtron specific op-codes***/
#define WREN			0x06
#define WRDI			0x04
#define RDSR			0x05
#define WRSR			0x01
#define	READ			0x03
#define FSTRD			0x0B
#define WRITE			0x02
#define	SLEEP			0xB9
#define RDID			0x9F
			







#endif /*FLASH_SPI_MACROS_H_*/
