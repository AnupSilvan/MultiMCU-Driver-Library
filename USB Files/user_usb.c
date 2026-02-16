/*****************************************************************************
*
* Module Name	: user_usb.c
* Created By	: Anup Silvan Mascarenhas
* Module
* Description	:	Declaration and definitions of USB CDC (Device mode) related
*					user friendly functions.
*
* Controller	: 	ATSAM4S8B
*					512 KB		Flash.
*					128 KB		RAM.
*
*****************************************************************************/
#include "user_usb.h"
#include "udd.h"
#include "udi_cdc.h"
#include "tgmath.h"
#include "user_uart.h"
/***** must include variable *****/
static volatile bool cdc_enable_f = false;

/***** Local Variables *****/
uint8_t usbData = 0;
uint8_t gb_usbstart_f = 0;
uint8_t usbcbuf[1] = {0};
U16 usbidx = 0;
 
/***** Global Variables *****/
uint8_t gb_usbuffer[MX_uBUF_SIZE] = {0},	// Store USB data into buffer.
		gb_no_usb_f = 0,	// Flag sets when USB disconnected.
		gb_usbRcmplt_f = 0,	// flag sets when reception completes.
		gb_uframe_rcv_tcmplt_f = 0;		// Flag sets when defined time for USB frame receive is completed.

uint16_t   gb_usbbuf_len = 0;	// Received data length.


uint16_t gb_uframe_rcv_timer = 0;	// Variable used to fill timer value for USB frame reception.

volatile bool gb_usb_connect = 0;	// Flag sets when USB is connected.

/*********************************************
* USB Driver generated Function.
**********************************************/
bool my_callback_cdc_enable(void)
{
	cdc_enable_f = true;
	gb_usb_connect = true;
	return true;
}
/*********************************************
* USB Driver generated Function.
**********************************************/
void my_callback_cdc_disable(void)
{
	cdc_enable_f = false;
}


/*****************************************************************************
* Function name	: void USB_PutChar(uint8_t ch)
* Returns		: Nothing.
* Arguments    	: character ch.
* Created by	: Anup Silvan Mascarenhas
*
* Description	:	Sends the character to the USB line.
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/
void USB_PutChar(uint8_t ch)
{
	udi_cdc_putc(ch);
}

/*****************************************************************************
* Function name	: void USB_PutString(const char* st)
* Returns		: Nothing.
* Arguments    	: const char* [constant character pointer which points to array/or entered string.]
* Created by	: Anup Silvan Mascarenhas
*
* Description	:	Sends the String to the USB line.
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/
void USB_PutString(const char* st)
{
	while(*st)
	{
		udi_cdc_putc(*st++);
	}
}

/*****************************************************************************
* Function name	: uint8_t USB_GetChar(void)
* Returns		: uint8_t (returns a byte data.)
* Arguments    	: None.
* Created by	: Anup Silvan Mascarenhas
*
* Description	:	Used to receive a character from the USB line.
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/
uint8_t USB_GetChar(void)
{
	if (cdc_enable_f)
	{
		return udi_cdc_getc();
	}

	return 0xAA;
}

/*****************************************************************************
* Function name	: void Read_USB_Config_Data(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created by	: Anup Silvan Mascarenhas
*
* Description	:	Get Data from the utility window using USB.
*               :
* Notes			: Call this function in while loop or timer interrupt routine.
* Global Variables Affected : gb_usbbuf_len ---> update received buffer length
							  gb_usbuffer[] ---> buffer to store received characters.
							  gb_usbRcmplt_f ---> flag sets when reception complete.
*****************************************************************************/
void Read_USB_Config_Data(void)
{
	//usbData = USB_GetChar();
	udi_cdc_read_no_polling(&usbcbuf[0], 1);
	usbData = usbcbuf[0];
	usbcbuf[0] = 0;

	if ((usbData == '#') && (gb_usbstart_f == 0))	// Check start of frame is received or not?
	{
		// Start storing data;
		usbidx = 0;			// Set index to zero.
		gb_usbbuf_len = 0;	// Set number of bytes received variable to 0.
		gb_usbstart_f = 1;		// Enable flag to start reception.
		gb_no_usb_f = 0;	// Reset flag to verify USB is connected or not?
		gb_uframe_rcv_timer = MX_uBUF_SIZE;	// Fill the timer to receive whole data within defined time.
	}
	else if ((usbData == '$') && ((gb_usbuffer[4] << 0 | gb_usbuffer[3] << 8) == gb_usbbuf_len + 1))	// Check for end of frame and whole length received or not.
	{
		// Stop storing.
		gb_usbstart_f = 0;		// Reset this flag for next cycle.
		gb_usbuffer[usbidx] = usbData;	// Put received bytes to buffer.
		gb_usbbuf_len++;	// Increment length as byte received.
		gb_usbRcmplt_f = 1;	// Flag set after complete reception of frame.
		gb_uframe_rcv_timer = 0;	// Fill the timer with 0 value to indicate, received whole data within defined time.

	}
	if (gb_usbstart_f == 1)	// If USB data receive flag enabled ?
	{
		gb_usbuffer[usbidx++] = usbData;	// Put received bytes to buffer.
		gb_usbbuf_len++;	// Increment length as byte received.
	}
	else
	{
		if (usbData == 0xAA)	// If data received is 0xAA
		{
			gb_no_usb_f = 1;	// Flag set to indicate that USB not connected.
		}
	}
}

/*****************************************************************************
* Function name	: void USB_Print_ASCII(int data)
* Returns		: Nothing.
* Arguments    	: int data ---> Put an integer data.
* Created by	: Anup Silvan Mascarenhas
*
*
* Description	:	Send ASCII Characters to USB Port.
*
*
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/
void USB_Print_ASCII(int data)
{
	char len = 0;
	int len_data = 0;
	int quotient = 0;

	len_data = data;

	if (data >= 10)
	{
		do
		{
			quotient = len_data/10;
			len++;
			len_data = quotient;
		} while (quotient >= 10);
	}
	len++;

	while (len > 0)
	{
		USB_PutChar((data/(int)pow(10, --len))%10+0x30);
	}
}