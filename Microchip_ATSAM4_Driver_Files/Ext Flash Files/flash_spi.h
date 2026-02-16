/*****************************************************************************
*							 
*
* Module Name	: user_spi.h
* Created By	: Anup Silvan Mascarenhas
* Module
* Description	: Header file for user_spi.c
*                 Defines constants and macros for user_spi.c
*
*
* REVISION HISTORY
* Version		: 1.0
* Revision Date	: 20-09-2022
* Changes		: NA
*****************************************************************************/
#ifndef A_FLASH_SPI_FLASH_SPI_H_
#define A_FLASH_SPI_FLASH_SPI_H_

#include "asf.h"

#ifndef CS_PIN_LOW
#define CS_PIN_LOW		pio_clear(PIOA, PIO_PA15);
#endif

#ifndef CS_PIN_HIGH
#define CS_PIN_HIGH		pio_set(PIOA, PIO_PA15);
#endif

#ifndef WP_PIN_RESET
#define WP_PIN_RESET	pio_clear(PIOD, PIO_PD27);
#endif

#ifndef WP_PIN_SET
#define WP_PIN_SET		pio_clear(PIOD, PIO_PD27);
#endif

#ifndef Data_To_SPI
#define Data_To_SPI(data, len)	(spi_write_packet(SPI, data, len))
#endif

#ifndef MX_BYTE_SIZE
#define MX_BYTE_SIZE	34603008	// Maximum bytes has flash memory i.e (33MB).
#endif

#ifndef STANDARD
#define STANDARD	528
#endif

#ifndef BINARY
#define BINARY		512
#endif

#ifndef PAGE_SIZE
#define PAGE_SIZE	BINARY /*STANDARD*/
#endif

#ifndef MAX_PAGES
#define MAX_PAGES (MX_BYTE_SIZE / PAGE_SIZE)
//#define MAX_PAGES	8196	// Maximum pages flash memory has.
#endif

#ifndef MX_READ_ONCE
#define MX_READ_ONCE	(PAGE_SIZE * 5)	// Read Upto 5 pages at once.
#endif

/***** DEBUG Definitions *****/
#define DEBUG_FLASH_BWRITE	0
#define DEBUG_FLASH_BREAD	0
#define DEBUG_FLASH_PWRITE	0
#define DEBUG_FLASH_PREAD	0
#define DEBUG_FLASH			0
#define DEBUG_FLASH_ERROR	0
#define DEBUG_FLASH_STATUS	0
/***** End of DEBUG Definitions *****/

/***** Command Definitions *****/
#define CMD_PAGE_ERASE		0x81
#define CMD_PW_BUF1			0x82	// With built in erase.
#define CMD_PW_BUF2			0x85	// Without built in erase.
#define CMD_READ_SR			0xD7	// Read status register.
#define CMD_MMP_READ		0xD2	// Main memory page read command.
#define CMD_RD_MOD_WR		0x58	// Read Modify Write command.
/***** End of command Definitions *****/

/***** Function Prototypes *****/
void Flash_Initialization(void);
void Wait_For_Flash_Ready(void);
void Flash_Software_Reset(void);
void Chip_Erase(void);
void Configure_Page_Size(char ps);

U8 Flash_Byte_Write(int loc, U8* fdata, U32 len);
U8 Flash_Byte_Read(int loc, U32 len);
U8 Flash_Page_Write(U32 page_num, U16 byte_add, U8 *data, U16 len);
U8 Flash_Page_Read(U32 page_num, U16 byte_add, U8 *data, U16 len);
U8 Erase_Page(U32 page_num);
U8 Is_Flash_Ready(void);
U8 check_error(U32 page_num, U16 byte_add, U16 len);
U8* Read_Status_Register(void);
/***** End of Function Prototypes *****/

extern U8 gb_fbyte_read_cmplt_f;
extern U8 gb_fRead_Array[MX_READ_ONCE];
extern int idx;
#endif /* A_FLASH_SPI_FLASH_SPI_H_ */
