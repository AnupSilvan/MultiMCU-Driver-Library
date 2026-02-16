/*****************************************************************************
*                      
*
* Module Name	: ext_ram.h
* Created By	: Anup Silvan Mascarenhas
* Module
* Description	: Defines constants and macros for the file ext_ram.c
*
*****************************************************************************/
#ifndef EXT_RAM_H_
#define EXT_RAM_H_

#include "asf.h"

/***** MACROS / DEFINITIONS FOR THE DRIVER *****/
#ifndef CS2_PIN_LOW
#define CS2_PIN_LOW		pio_clear(PIOA, PIO_PA16);
#endif

#ifndef CS2_PIN_HIGH
#define CS2_PIN_HIGH		pio_set(PIOA, PIO_PA16);
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

#ifndef extRAM_MX_BYTE_SIZE
#define extRAM_MX_BYTE_SIZE	512000	//7D000 (Hex) //Bytes // Maximum bytes that RAM has is (4MBit).
#endif

#ifndef extRAM_PAGE_SIZE
#define extRAM_PAGE_SIZE	32	// 32 if (0), 256 if (1)
#endif

#ifndef extRAM_MAX_PAGES
#define extRAM_MAX_PAGES (extRAM_MX_BYTE_SIZE / extRAM_PAGE_SIZE)
#endif

/***** Command Definitions *****/
#define extRAM_CMD_READ_SR			0x05	// Read status register.
#define extRAM_CMD_WRITE_SR			0x01	// Read status register.
#define extRAM_WRITE_TO_MEM			0x02	// Write to memory.
#define extRAM_READ_MEMORY			0x03	// Read memory.
/***** End of command Definitions *****/

/***** DEBUG MESSAGES *****/
#define extRAM_DEBUG_STATUS_REG		(0)
#define extRAM_DEBUG_PWRITE			(0)
#define extRAM_DEBUG_ERROR			(0)
#define extRAM_DEBUG_WRITE			(0)
#define extRAM_DEBUG_READ			(0)
/***** END OF DEBUG MESSAGES *****/


/***** Structure Declarations *****/
typedef struct
{
	U8 data_arr[2];
	U8 data_len;
	
}STS_REG;

typedef struct
{
	U32 regAdds;
	U8 data_arr[1000];
	U16 data_len;
}RAM_MEM;

extern STS_REG RAM_STS_REG;
extern RAM_MEM ramMem;

/***** Function Prototypes *****/
void Configure_CS_Pin_For_extRAM(void);
void extRAM_Write_To_Memory(RAM_MEM *writeMem);
void extRAM_Read_From_Memory(RAM_MEM *readMem);
U8* extRAM_Read_Status_Register(void);
#endif /* EXT_RAM_H_ */