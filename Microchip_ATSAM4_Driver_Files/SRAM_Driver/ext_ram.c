/*****************************************************************************
*             
*
* Module Name	: ext_ram.c
* Created By	: Anup Silvan Mascarenhas
* Module
* Description	: Based on SPI protocol the external RAM can be accessed to store
*				  variables and also get the data from this. This file contains
*				  the functions to initialize the RAM, write byte data, write page data
*				  read byte and read page from the RAM.
*
* Controller	: 	ATSAM4E16CA-AUR
*					1024 KB		Flash.
*					128 KB		RAM.
*
*****************************************************************************/

#include "ext_ram.h"
#include "user_uart.h"

uint8_t rm_is_error = 0;					// Check for error page num, byte address, len. if wrong entered.
uint8_t ram_read_arr[extRAM_PAGE_SIZE] = {0};	// Used to read flash data.
uint8_t rm_command_data[8] = {0};		// Used to modify command, page num, byte address.
uint32_t rm_byte_add = 0;				// Holds the starting address of the bytes to write or read.

STS_REG RAM_STS_REG;
RAM_MEM ramMem;

static uint8_t check_error(uint32_t page_num, uint16_t rm_byte_add, uint16_t len);

/*****************************************************************************
* Function name	: void Configure_CS_Pin_For_extRAM(void)
* Returns		: Nothing.
* Arguments		: None.
* Created by	: Anup Silvan Mascarenhas
* Description	: Configure SPI chip select pin.
*               :
* Notes			: It is a user defined Chip Select Pin.
* Global Variables Affected	: NA
*****************************************************************************/
void Configure_CS_Pin_For_extRAM(void)
{
	pmc_enable_periph_clk(ID_PIOA);
	pio_set_output(PIOA, PIO_PA16, HIGH, DISABLE, ENABLE);
}

/*****************************************************************************************
* Function name	: uint8_t* extRAM_Read_Status_Register(void)
* Returns		: uint8_t* ---> returns status register data.
* Arguments		: None.
* Created by	: Anup Silvan Mascarenhas
* Description	: Function is written for reading status register value.
*               :
* Notes			: NA
* Global Variables Affected	: NA
******************************************************************************************/
uint8_t* extRAM_Read_Status_Register(void)
{
	#if extRAM_DEBUG_STATUS_REG
	Print_Message("\nInside extRAM_Read_Status_Register Function.\n");
	#endif

	rm_command_data[0] = extRAM_CMD_READ_SR;

	CS2_PIN_LOW;
	//delay_ms(1);
	U8 stf = Data_To_SPI(rm_command_data, 1);
	while (!spi_is_tx_empty(SPI));
	spi_read_packet(SPI, ram_read_arr, 2);
	//delay_ms(1);
	CS2_PIN_HIGH;
	
	#if extRAM_DEBUG_STATUS_REG
	for (U8 idx = 0; idx < 2; idx ++)
	{
		Print_Number(ram_read_arr[idx]);Print_Message(",");
	}
	#endif

	return ram_read_arr;
}

/*****************************************************************************************
* Function name	: uint8_t* extRAM_Write_Status_Register(void)
* Returns		: Nothing.
* Arguments		: U8 *stsReg ---> Pointer variable which points to the array.
* Created by	: Anup Silvan Mascarenhas
* Description	: Function is written to update the bits in the status register.
*               :
* Notes			: NA
* Global Variables Affected	: NA
******************************************************************************************/
void extRAM_Write_Status_Register(STS_REG *stsReg)
{
	#if extRAM_DEBUG_STATUS_REG
	Print_Message("\nInside extRAM_Write_Status_Register Function.\n");
	#endif

	rm_command_data[0] = extRAM_CMD_WRITE_SR;

	CS2_PIN_LOW;
	//delay_ms(1);
	Data_To_SPI(rm_command_data, 1);
	//while (!spi_is_tx_empty(SPI));
	Data_To_SPI(stsReg->data_arr, stsReg->data_len);
	//while (!spi_is_tx_empty(SPI));
	CS2_PIN_HIGH;
}

 /*****************************************************************************************
// * Function name	: uint8_t extRAM_Page_Write(uint32_t page_num, uint16_t rm_byte_add,
// * 				  uint8_t *data, uint16_t len)
// * Returns		: uint8_t ---> returns 1,2,3 if error occurs. else returns 0;
// * Arguments		: uint32_t page_num ---> Send Page number to be write.
// * 				  uint16_t rm_byte_add ---> Send Byte address of the page.
// * 				  uint8_t *data ---> Holds the address of the data buffer.
// * 				  uint16_t len	---> Send total length to be writen.
// * Created by		: Anup Silvan Mascarenhas
// * Description	: Function is written for writing data to a page.
// * 				  Developer can write from 0th location to maximum of PAGE_SIZE.
// *               
// * Notes			: NA
// * Global Variables Affected	: NA
// ******************************************************************************************
// uint8_t extRAM_Page_Write(uint32_t page_num, uint16_t rm_byte_add, uint8_t *data, uint16_t len)
// {
// 	#if extRAM_DEBUG_PWRITE
// 	Print_Message("\nInside extRAM_Page_Write Function.");
// 	#endif
// 
// 	rm_is_error = check_error(page_num, rm_byte_add, len);
// 	if (rm_is_error != 0)
// 	{
// 		return rm_is_error;
// 	}
// 
// 	#if extRAM_DEBUG_PWRITE
// 	Print_Message("\nPage Number : ");
// 	Print_Number(page_num);
// 	Print_Message("\nByte Address : ");
// 	Print_Number(rm_byte_add);
// 	Print_Message("\nEntered length : ");
// 	Print_Number(len);
// 	Print_Message("\nData to be written is\n");
// 	for (U16 idx = rm_byte_add; idx < len; idx++)
// 	{
// 		UART_Debug_PutChar(data[idx - rm_byte_add]);
// 	}
// 	#endif
// 
// 	rm_command_data[0] = extRAM_WRITE_TO_MEM;
// 	if (extRAM_PAGE_SIZE == STANDARD)
// 	{
// 		ADD_BYTE = page_num;
// 		ADD_BYTE <<= 10;
// 		ADD_BYTE |= (rm_byte_add & 0x03FF);
// 	}
// 	else if (PAGE_SIZE == BINARY)
// 	{
// 		ADD_BYTE = page_num;
// 		ADD_BYTE <<= 9;
// 		ADD_BYTE |= (rm_byte_add & 0x01FF);
// 	}
// 	rm_command_data[1] = (uint8_t)((ADD_BYTE & 0xFF0000)>>16);
// 	rm_command_data[2] = (uint8_t)((ADD_BYTE & 0x00FF00)>>8);
// 	rm_command_data[3] = (uint8_t)(ADD_BYTE & 0x0000FF);
// 
// 	CS2_PIN_LOW;
// 	delay_ms(1);
// 	Data_To_SPI(rm_command_data, 4);
// 	while (!spi_is_tx_empty(SPI));
// 	Data_To_SPI(data, len);
// 	while (!spi_is_tx_empty(SPI));
// 	CS2_PIN_HIGH;
// 
// 	Wait_For_Flash_Ready();
// 
// 	return 0;
// }
// 
// /*****************************************************************************************
// * Function name	: static uint8_t check_error(uint32_t page_num, uint16_t rm_byte_add, uint16_t len)
// * Returns		: uint8_t ---> returns 1,2,3 if error occurs. else returns 0;
// * Arguments		: uint32_t page_num ---> Send Page number
// * 				  uint16_t rm_byte_add ---> Send Byte address of the page.
// * 				  uint16_t len	---> Send total length
// * Created by	: Anup Silvan Mascarenhas
// * Description	: Function is written for error checking.
// *               :
// * Notes			: NA
// * Global Variables Affected	: NA
// ******************************************************************************************
// static uint8_t check_error(uint32_t page_num, uint16_t rm_byte_add, uint16_t len)
// {
// 	if (page_num > extRAM_MAX_PAGES)
// 	{
// 		#if extRAM_DEBUG_ERROR
// 		Print_Message("\nPage number is greater than extRAM_MAX_PAGES");
// 		#endif
// 		return 1;	// As flash has 8196 pages, page number can not be greater than this.
// 	}
// 	if (rm_byte_add > (extRAM_PAGE_SIZE-1))
// 	{
// 		#if extRAM_DEBUG_ERROR
// 		Print_Message("\nByte Address is greater than extRAM_PAGE_SIZE");
// 		#endif
// 		return 2;	// Address can't be greater than Page size.
// 	}
// 	if ((len < 1) || (len > extRAM_PAGE_SIZE) || ((rm_byte_add + len) > extRAM_PAGE_SIZE))
// 	{
// 		#if extRAM_DEBUG_ERROR
// 		Print_Message("\nLength & Byte address can't be greater than extRAM_PAGE_SIZE");
// 		#endif
// 		return 3;	// Length or (Length + Byte address) can't be greater than Page size.
// 	}
// 
// 	return 0;
// }

*/

/*****************************************************************************************
* Function name	: void extRAM_Write_To_Memory(RAM_MEM *writeMem)
* Returns		: NA
* Arguments		: RAM_MEM *writeMem -> holds address of memory
* Created by	: Anup Silvan Mascarenhas
* Description	: Function is written to write and save inside external memory 
*               :
* Notes			: NA
* Global Variables Affected	: NA
*******************************************************************************************/
void extRAM_Write_To_Memory(RAM_MEM *writeMem)
{
	#if extRAM_DEBUG_WRITE
	Print_Message("\nInside extRAM_Write_To_Memory Function.");
	#endif

	rm_command_data[0] = extRAM_WRITE_TO_MEM;
	
	rm_command_data[1] = (uint8_t)((writeMem->regAdds & 0x00FF0000) >> 16) ;
	rm_command_data[2] = (uint8_t)((writeMem->regAdds & 0x0000FF00) >> 8) ;
	rm_command_data[3] = (uint8_t)((writeMem->regAdds & 0x000000FF)) ;
	
	CS2_PIN_LOW;
	//delay_ms(1);
	Data_To_SPI(rm_command_data, 4);
	while (!spi_is_tx_empty(SPI));
	Data_To_SPI(writeMem->data_arr, writeMem->data_len);
	while (!spi_is_tx_empty(SPI));
	CS2_PIN_HIGH;
}
/*****************************************************************************************
* Function name	: void extRAM_Read_From_Memory(RAM_MEM *readMem)
* Returns		: NA
* Arguments		: RAM_MEM *readMem -> holds address of memory
* Created by	: Anup Silvan Mascarenhas
* Description	: Function is written to Read from external memory 
*               :
* Notes			: NA
* Global Variables Affected	: NA
*******************************************************************************************/
void extRAM_Read_From_Memory(RAM_MEM *readMem)
{
	#if extRAM_DEBUG_READ
	Print_Message("\nInside extRAM_Read_From_Memory Function.");
	#endif

	rm_command_data[0] = extRAM_READ_MEMORY;
	
	rm_command_data[1] = (uint8_t)((readMem->regAdds & 0x00FF0000) >> 16) ;
	rm_command_data[2] = (uint8_t)((readMem->regAdds & 0x0000FF00) >> 8) ;
	rm_command_data[3] = (uint8_t)((readMem->regAdds & 0x000000FF)) ;

	CS2_PIN_LOW;
	//delay_ms(1);
	Data_To_SPI(rm_command_data, 4);
	while (!spi_is_tx_empty(SPI));
	spi_read_packet(SPI, readMem->data_arr , readMem->data_len);
	/***** Wait till the reception complete. *****/
	//delay_ms(1);
	CS2_PIN_HIGH;
}