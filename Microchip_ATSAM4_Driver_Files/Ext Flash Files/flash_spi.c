/***************************************************************************************
*							  
*
* Module Name	: user_spi.c
* Created By	: Anup Silvan Mascarenhas
* Module
* Description	: Functions related External flash memory read, write and its settings
* 				  Page settings, page erase and chip erase are written. And also SPI
* 				  communication settings also written.
*
* Device Used	:
* Controller	: STM32F207VET6
*                 512 KB -----> Flash Memory.
*                 128 KB -----> RAM Memory.
*
*
****************************************************************************************/
#include "flash_spi.h"
#include "string.h"

#include "user_uart.h"

uint8_t command_data[8] = {0};		// Used to modify command, page num, byte address.
uint8_t is_error = 0;					// Check for error page num, byte address, len. if wrong entered.
uint8_t fwrite_arr[PAGE_SIZE] = {0};	// Used to write flash data.
uint8_t fread_arr[PAGE_SIZE] = {0};	// Used to read flash data.
uint8_t temp_arr[PAGE_SIZE] = {0};	// Used to write_read flash data (Temporary array).

uint32_t page_num = 0;				// Variable to enter page number for write and read.
uint32_t byte_add = 0;					// Holds the starting address of the bytes to write or read.
uint32_t add_counts = 0;				// Variable holds address counter.
uint32_t temp_len = 0;					// Temporary length value.
uint32_t ADD_BYTE = 0;				// Holds 4 Byte of command, page num and address data.

int idx = 0;							// Used in for loop for indexing.

/***** Global Variables *****/
uint8_t gb_fbyte_read_cmplt_f = 0;	// Flag sets when multiple bytes read completes.
uint8_t gb_fRead_Array[MX_READ_ONCE]={0};	// Array used when reading multiple bytes at a time.

/***** Function Protocol *****/
static void configure_spi_wp_pin(void);

/*****************************************************************************
* Function name	: void Flash_Initialization(void)
* Returns		: None.
* Arguments		: None.
* Created by	: Anup Silvan Mascarenhas
* Description	: Presently default settings are there.
*               :
* Notes			: NA
* Global Variables Affected	: NA
*****************************************************************************/
void Flash_Initialization(void)
{
	uint8_t* iData;
	U16 lcl_page_size = 0;
	iData = 0;
	
	configure_spi_wp_pin();
	iData = Read_Status_Register();

	if ((iData[0] & 0x01) == 1)
	{
		#if DEBUG_FLASH
		Print_Message("\nDevice is configured for power of 2 binary page size");
		#endif
		
		lcl_page_size = BINARY;
	}
	else
	{
		#if DEBUG_FLASH
		Print_Message("\nDevice is configured for standard DataFlash page size");
		#endif
		
		lcl_page_size = STANDARD;
	}
	
	if (lcl_page_size != PAGE_SIZE)
	{
		if (PAGE_SIZE == BINARY)
		Configure_Page_Size('B');
		else
		Configure_Page_Size('S');
		
		Flash_Software_Reset();
		Wait_For_Flash_Ready();
	}
}

/*****************************************************************************
* Function name	: void Configure_Page_Size(char ps)
* Returns		: None.
* Arguments		: char ps	---> Send character ASCII Capital 'S' or 'B' to
* 				  select standard or binary page size.
* Created by	: Anup Silvan Mascarenhas
* Description	: Function is written for configuring Page size.
*               :
* Notes			: NA
* Global Variables Affected	: NA
*****************************************************************************/
void Configure_Page_Size(char ps)
{
	#if DEBUG_FLASH
	Print_Message("\nInside Configure_Page_Size Function.");
	#endif

	command_data[0] = 0x3D;
	command_data[1] = 0x2A;
	command_data[2] = 0x80;
	if (ps == 'S')	// If Page size is Standard.
	{
		command_data[3] = 0xA7;
	}
	else if (ps == 'B')	// if Page size is binary.
	{
		command_data[3] = 0xA6;
	}

	CS_PIN_LOW;
	delay_ms(1);
	Data_To_SPI(command_data, 4);
	while (!spi_is_tx_empty(SPI));
	CS_PIN_HIGH;

	Wait_For_Flash_Ready();
}

/***********************************************************************************
* Function name	: uint8_t Flash_Byte_Write(int loc, uint8_t *fdata, uint32_t len)
* Returns		: uint8_t ---> returns 1,2,3 if error occurs. else returns 0;
* Arguments		: int loc ---> Send byte location.
* 				  uint8_t *fdata ---> pointer holds an address of data buffer.
* 				  uint16_t len	---> Send total length to be written.
* Created by	: Anup Silvan Mascarenhas
* Description	: Function is written for Writing a Byte data or multiple bytes.
* 				  Developer can write from 0th location to MAX_Byte size.
* 				  Only need to send byte location , data and length of data. Pages
* 				  are selected automatically based on byte location/address.
*               :
* Notes			: NA
* Global Variables Affected	: NA
***********************************************************************************/
uint8_t Flash_Byte_Write(int loc, uint8_t *fdata, uint32_t len)
{
	#if DEBUG_FLASH_BWRITE
	Print_Message("\nInside Flash_Byte_Write Function");
	#endif

// 	if ((loc > MX_BYTE_SIZE) || (len < 1))
// 		return 1;

	page_num = (loc/PAGE_SIZE);
	byte_add = loc-(page_num*PAGE_SIZE);

	#if DEBUG_FLASH_BWRITE
	Print_Message("\nEntered write location : ");
	Print_Number(loc);
	Print_Message("\nEntered total length : ");
	Print_Number(len);
	#endif

	add_counts = 0;
	while (1)
	{
		#if DEBUG_FLASH_BWRITE
		Print_Message("\nPage Number : ");
		Print_Number(page_num);
		Print_Message("\nByte location : ");
		Print_Number(byte_add);
		Print_Message("\nWritable data is-->\n");
		#endif

		temp_len = 0;
		for (uint16_t idx = byte_add; idx < PAGE_SIZE; idx++)
		{
			#if DEBUG_FLASH_BWRITE
			UART_Debug_PutChar(fdata[add_counts]);
			#endif

			add_counts++;
			temp_len++;

			if (add_counts >= len)
			{
				break;
			}
		}
		#if DEBUG_FLASH_BWRITE
		Print_Message("\nAddress counter : ");
		Print_Number(add_counts);
		Print_Message("\nTemp length : ");
		Print_Number(temp_len);
		#endif

		command_data[0] = CMD_RD_MOD_WR;
		if (PAGE_SIZE == STANDARD)
		{
			ADD_BYTE = page_num;
			ADD_BYTE <<= 10;
			ADD_BYTE |= (byte_add & 0x03FF);
		}
		else if (PAGE_SIZE == BINARY)
		{
			ADD_BYTE = page_num;
			ADD_BYTE <<= 9;
			ADD_BYTE |= (byte_add & 0x01FF);
		}
		
// 		Print_Message("\n ================================ ");
// 		Print_Message("\naddress byte : ");
// 		Print_Number(ADD_BYTE);
// 		Print_Message("\n ================================ ");
		
		command_data[1] = (uint8_t)((ADD_BYTE & 0xFF0000)>>16);
		command_data[2] = (uint8_t)((ADD_BYTE & 0x00FF00)>>8);
		command_data[3] = (uint8_t)(ADD_BYTE & 0x0000FF);

		CS_PIN_LOW;
		delay_ms(1);
		Data_To_SPI(command_data, 4);
		while (!spi_is_tx_empty(SPI));
		Data_To_SPI(&fdata[(add_counts-temp_len)], temp_len);
		while (!spi_is_tx_empty(SPI));
		CS_PIN_HIGH;

		Wait_For_Flash_Ready();

		if (add_counts < len)
		{
			page_num++;
			byte_add=0;
		}
		else
			break;
	}

	return 0;
}

/*************************************************************************************
* Function name	: uint8_t Flash_Byte_Read(int loc, uint32_t len)
* Returns		: uint8_t ---> returns 1 if error occurs. else returns 0;
* Arguments		: int loc ---> Send byte location.
* 				  uint16_t len	---> Send total length to be Read.
* Created by	: Anup Silvan Mascarenhas
* Description	: Function is written for Reading a Byte data or multiple bytes.
* 				  Developer can Read from 0th location to maximum of gb_fRead_Array[]
* 				  size, gb_fRead_Array[size] can be user dependent.
* 				  Only need to send byte location and length of data. Pages
* 				  are selected automatically based on byte location/address.
*               :
* Notes			: NA
* Global Variables Affected	: gb_fRead_Array[]	---> Stores flash data.
* 							  gb_fbyte_read_cmplt_f ---> flag sets when read completed.
**************************************************************************************/
uint8_t Flash_Byte_Read(int loc, uint32_t len)
{
	#if DEBUG_FLASH_BREAD
	Print_Message("\nInside Flash_Byte_Read Function.\n");
	#endif

	if ((loc > MX_BYTE_SIZE) || (len < 1))
		return 1;

	page_num = (loc/PAGE_SIZE);
	byte_add = loc-(page_num*PAGE_SIZE);

	#if DEBUG_FLASH_BREAD
	Print_Message("\nRead location : ");
	Print_Number(loc);
	Print_Message("\nRead length Data : ");
	Print_Number(len);
	#endif

	add_counts = 0;
	while (1)
	{
		#if DEBUG_FLASH_BREAD
		Print_Message("\nPage Number : ");
		Print_Number(page_num);
		Print_Message("\nByte location : ");
		Print_Number(byte_add);
		#endif

		temp_len = 0;
		for (int idx = byte_add; idx < PAGE_SIZE; idx++)
		{
			add_counts++;
			temp_len++;

			if (add_counts >= len)
			{
				break;
			}
		}

		Flash_Page_Read(page_num, byte_add, &gb_fRead_Array[(add_counts-temp_len)], temp_len);

		if (add_counts < len)
		{
			page_num++;
			byte_add=0;
		}
		else
			break;
	}

	gb_fbyte_read_cmplt_f = 1;

	return 0;
}

/*****************************************************************************************
* Function name	: uint8_t Flash_Page_Write(uint32_t page_num, uint16_t byte_add,
* 				  uint8_t *data, uint16_t len)
* Returns		: uint8_t ---> returns 1,2,3 if error occurs. else returns 0;
* Arguments		: uint32_t page_num ---> Send Page number to be write.
* 				  uint16_t byte_add ---> Send Byte address of the page.
* 				  uint8_t *data ---> Holds the address of the data buffer.
* 				  uint16_t len	---> Send total length to be write.
* Created by	: Anup Silvan Mascarenhas
* Description	: Function is written for writing data to a page.
* 				  Developer can write from 0th location to maximum of PAGE_SIZE.
*               :
* Notes			: NA
* Global Variables Affected	: NA
******************************************************************************************/
uint8_t Flash_Page_Write(uint32_t page_num, uint16_t byte_add, uint8_t *data, uint16_t len)
{
	#if DEBUG_FLASH_PWRITE
	Print_Message("\nInside Flash_Page_Write Function.");
	#endif

	is_error = check_error(page_num, byte_add, len);
	if (is_error != 0)
	{
		return is_error;
	}

	#if DEBUG_FLASH_PWRITE
	Print_Message("\nPage Number : ");
	Print_Number(page_num);
	Print_Message("\nByte Address : ");
	Print_Number(byte_add);
	Print_Message("\nEntered length : ");
	Print_Number(len);
	Print_Message("\nData to be written is\n");
	for (idx = byte_add; idx < len; idx++)
	{
		if ((idx % 84) == 0)
		{
			Print_Message("\n");
		}
		//UART_Debug_PutChar(data[idx - byte_add]);
		Print_Number(data[idx - byte_add]);
		Print_Message(",");
	}
	#endif

	command_data[0] = CMD_PW_BUF1;
	if (PAGE_SIZE == STANDARD)
	{
		ADD_BYTE = page_num;
		ADD_BYTE <<= 10;
		ADD_BYTE |= (byte_add & 0x03FF);
	}
	else if (PAGE_SIZE == BINARY)
	{
		ADD_BYTE = page_num;
		ADD_BYTE <<= 9;
		ADD_BYTE |= (byte_add & 0x01FF);
	}
	command_data[1] = (uint8_t)((ADD_BYTE & 0xFF0000)>>16);
	command_data[2] = (uint8_t)((ADD_BYTE & 0x00FF00)>>8);
	command_data[3] = (uint8_t)(ADD_BYTE & 0x0000FF);

	CS_PIN_LOW;
	delay_ms(1);
	Data_To_SPI(command_data, 4);
	while (!spi_is_tx_empty(SPI));
	Data_To_SPI(data, len);
	while (!spi_is_tx_empty(SPI));
	CS_PIN_HIGH;

	Wait_For_Flash_Ready();

	return 0;
}

/*****************************************************************************************
* Function name	: uint8_t Flash_Page_Read(uint32_t page_num, uint16_t byte_add,
* 				  uint8_t *data, uint16_t len)
* Returns		: uint8_t ---> returns 1,2,3 if error occurs. else returns 0;
* Arguments		: uint32_t page_num ---> Send Page number to be read.
* 				  uint16_t byte_add ---> Send Byte address of the page.
* 				  uint8_t *data ---> Holds the address of the read data buffer.
* 				  uint16_t len	---> Send total length to be read.
* Created by	: Anup Silvan Mascarenhas
* Description	: Function is written for reading data from the page.
*               :
* Notes			: NA
* Global Variables Affected	: NA
******************************************************************************************/
uint8_t Flash_Page_Read(uint32_t page_num, uint16_t byte_add, uint8_t *data, uint16_t len)
{
	#if DEBUG_FLASH_PREAD
	Print_Message("\nInside Flash_Page_Read Function.\n");
	#endif

// 	is_error = check_error(page_num, byte_add, len);
// 	if (is_error != 0)
// 	{
// 		return is_error;
// 	}

	#if DEBUG_FLASH_PWRITE
	Print_Message("\nPage Number : ");
	Print_Number(page_num);
	Print_Message("\nByte Address : ");
	Print_Number(byte_add);
	Print_Message("\nEntered length : ");
	Print_Number(len);
	#endif

	for (idx=0; idx<PAGE_SIZE; idx++)
	{
		fread_arr[idx] = 0;
	}

	command_data[0] = CMD_MMP_READ;
	if (PAGE_SIZE == STANDARD)
	{
		ADD_BYTE = page_num;
		ADD_BYTE <<= 10;
		ADD_BYTE |= (byte_add & 0x03FF);
	}
	else if (PAGE_SIZE == BINARY)
	{
		ADD_BYTE = page_num;
		ADD_BYTE <<= 9;
		ADD_BYTE |= (byte_add & 0x01FF);
	}
	command_data[1] = (uint8_t)((ADD_BYTE & 0xFF0000)>>16);
	command_data[2] = (uint8_t)((ADD_BYTE & 0x00FF00)>>8);
	command_data[3] = (uint8_t)(ADD_BYTE & 0x0000FF);

	/***** Four dummy bytes *****/
	command_data[4] = 0xFF;
	command_data[5] = 0xFF;
	command_data[6] = 0xFF;
	command_data[7] = 0xFF;
	/***** End of four dummy bytes *****/

	CS_PIN_LOW;
	delay_ms(1);
	Data_To_SPI(command_data, 8);
 	while (!spi_is_tx_empty(SPI));
	spi_read_packet(SPI, fread_arr, len);
	/***** Wait till the reception complete. *****/
	delay_ms(1);
	CS_PIN_HIGH;

	#if DEBUG_FLASH_PREAD
	Print_Message("\n\nPage value are\n");
	for (idx=0; idx<len; idx++)
	{
		UART_Debug_PutChar(fread_arr[idx]);
	}
	#endif

	memcpy(data+0, fread_arr+0, len);
	
// 	Print_Message("\n --------------------------- ");
// 	for (idx=0; idx<len; idx++)
// 	{
// 		Print_Number(data[idx]);
// 	}

	Wait_For_Flash_Ready();

	return 0;
}

/*****************************************************************************************
* Function name	: uint8_t check_error(uint32_t page_num, uint16_t byte_add, uint16_t len)
* Returns		: uint8_t ---> returns 1,2,3 if error occurs. else returns 0;
* Arguments		: uint32_t page_num ---> Send Page number
* 				  uint16_t byte_add ---> Send Byte address of the page.
* 				  uint16_t len	---> Send total length
* Created by	: Anup Silvan Mascarenhas
* Description	: Function is written for error checking.
*               :
* Notes			: NA
* Global Variables Affected	: NA
******************************************************************************************/
uint8_t check_error(uint32_t page_num, uint16_t byte_add, uint16_t len)
{
	if (page_num > MAX_PAGES)
	{
		#if DEBUG_FLASH_ERROR
		Print_Message("\nPage number is greater than MAX_PAGES");
		#endif
		return 1;	// As flash has 8196 pages, page number can not be greater than this.
	}
	if (byte_add > (PAGE_SIZE-1))
	{
		#if DEBUG_FLASH_ERROR
		Print_Message("\nByte Address is greater than PAGE_SIZE");
		#endif
		return 2;	// Address can't be greater than Page size.
	}
	if ((len < 1) || (len > PAGE_SIZE) || ((byte_add + len) > PAGE_SIZE))
	{
		#if DEBUG_FLASH_ERROR
		Print_Message("\nLength & Byte address can't be greater than PAGE_SIZE");
		#endif
		return 3;	// Length or (Length + Byte address) can't be greater than Page size.
	}

	return 0;
}

/*****************************************************************************************
* Function name	: uint8_t Erase_Page(uint32_t page_num)
* Returns		: uint8_t ---> returns 1,2,3 if error occurs. else returns 0;
* Arguments		: uint32_t page_num ---> Send Page number to erase.
* Created by	: Anup Silvan Mascarenhas
* Description	: Function is written for erasing the page.
*               :
* Notes			: NA
* Global Variables Affected	: NA
******************************************************************************************/
uint8_t Erase_Page(uint32_t page_num)
{
	#if DEBUG_FLASH
	Print_Message("\nInside Erase_Page Function.\nPlease Wait...");
	#endif

	is_error = check_error(page_num, 0, 1);
	if (is_error != 0)
	{
		return is_error;
	}

	byte_add = 0;
	command_data[0] = CMD_PAGE_ERASE;
	if (PAGE_SIZE == STANDARD)
	{
		ADD_BYTE = page_num;
		ADD_BYTE <<= 10;
		ADD_BYTE |= (byte_add | 0x03FF);
	}
	else if (PAGE_SIZE == BINARY)
	{
		ADD_BYTE = page_num;
		ADD_BYTE <<= 9;
		ADD_BYTE |= (byte_add | 0x01FF);
	}
	command_data[1] = (uint8_t)((ADD_BYTE & 0xFF0000)>>16);
	command_data[2] = (uint8_t)((ADD_BYTE & 0x00FF00)>>8);
	command_data[3] = (uint8_t)(ADD_BYTE & 0x0000FF);

	CS_PIN_LOW;
	delay_ms(1);
	Data_To_SPI(command_data, 4);
	while (!spi_is_tx_empty(SPI));
	CS_PIN_HIGH;

	Wait_For_Flash_Ready();

	#if DEBUG_FLASH
	Print_Message("\nPage Erase Complete");
	#endif

	return 0;
}

/*****************************************************************************************
* Function name	: uint8_t* Read_Status_Register(void)
* Returns		: uint8_t* ---> returns status register data.
* Arguments		: None.
* Created by	: Anup Silvan Mascarenhas
* Description	: Function is written for reading status register value.
*               :
* Notes			: NA
* Global Variables Affected	: NA
******************************************************************************************/
uint8_t* Read_Status_Register(void)
{
	#if DEBUG_FLASH_STATUS
	Print_Message("\nInside Read_Status_Register Function.\n");
	#endif

	byte_add = 0;
	command_data[0] = CMD_READ_SR;

	CS_PIN_LOW;
	delay_ms(1);
	U8 stf = Data_To_SPI(command_data, 1);
	while (!spi_is_tx_empty(SPI));
	spi_read_packet(SPI, fread_arr, 2);
	delay_ms(1);
	CS_PIN_HIGH;
	
	#if DEBUG_FLASH_STATUS
	for (idx=0; idx<2; idx++)
	{
		Print_Number(fread_arr[idx]);Print_Message(",");
	}
	#endif

	return fread_arr;
}

/*****************************************************************************************
* Function name	: uint8_t Is_Flash_Ready(void)
* Returns		: uint8_t ---> returns 1 if device ready else 0 if busy.
* Arguments		: None.
* Created by	: Anup Silvan Mascarenhas
* Description	: Function is written for reading status register value.
*               :
* Notes			: NA
* Global Variables Affected	: NA
******************************************************************************************/
uint8_t Is_Flash_Ready(void)
{
	uint8_t* sData;
	sData = 0;
	sData = Read_Status_Register();

	return ((sData[1] & 0x80)? 1: 0);
}

/*****************************************************************************************
* Function name	: void Chip_Erase(void)
* Returns		: Nothing.
* Arguments		: None.
* Created by	: Anup Silvan Mascarenhas
* Description	: Function is written for erasing whole chip.
*               :
* Notes			: NA
* Global Variables Affected	: NA
******************************************************************************************/
void Chip_Erase(void)
{
	#if DEBUG_FLASH
	Print_Message("\nInside Chip_Erase Function.\nPlease Wait...");
	#endif

	command_data[0] = 0xC7;
	command_data[1] = 0x94;
	command_data[2] = 0x80;
	command_data[3] = 0x9A;

	CS_PIN_LOW;
	delay_ms(1);
	Data_To_SPI(command_data, 4);
	while (!spi_is_tx_empty(SPI));
	CS_PIN_HIGH;

	Wait_For_Flash_Ready();

	#if DEBUG_FLASH
	Print_Message("\nChip_Erase Successful.");
	#endif
}

/*****************************************************************************************
* Function name	: void Wait_For_Flash_Ready(void)
* Returns		: Nothing.
* Arguments		: None.
* Created by	: Anup Silvan Mascarenhas
* Description	: Function waits untill flash ready.
*               :
* Notes			: NA
* Global Variables Affected	: NA
******************************************************************************************/
void Wait_For_Flash_Ready(void)
{
	while (!Is_Flash_Ready());
}

/*****************************************************************************
* Function name	: static void configure_spi_cs_pin(void)
* Returns		: Nothing.
* Arguments		: None.
* Created by	: Anup Silvan Mascarenhas
* Description	: Configure Flash write protect pin.
*               :
* Notes			: NA.
* Global Variables Affected	: NA
*****************************************************************************/
static void configure_spi_wp_pin(void)
{
	pmc_enable_periph_clk(ID_PIOD);
	pio_set_output(PIOD, PIO_PD27, HIGH, DISABLE, ENABLE);
}

/*****************************************************************************
* Function name	: void Flash_Software_Reset(void)
* Returns		: Nothing.
* Arguments		: None.
* Created by	: Anup Silvan Mascarenhas
* Description	: The Software Reset command allows a program or erase
*				  operation in progress to be ended abruptly and returns
*				  the device to an idle state.
*               :
* Notes			: NA.
* Global Variables Affected	: NA
*****************************************************************************/
void Flash_Software_Reset(void)
{
	command_data[0] = 0xF0;
	command_data[1] = 0x00;
	command_data[2] = 0x00;
	command_data[3] = 0x00;
	
	CS_PIN_LOW;
	delay_ms(1);
	Data_To_SPI(command_data, 4);
	while (!spi_is_tx_empty(SPI));
	CS_PIN_HIGH;
}