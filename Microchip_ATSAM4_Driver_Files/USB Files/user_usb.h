/*****************************************************************************
*                    
*
* Module Name	: user_usb.h
* Created By	: Anup Silvan Mascarenhas
* Module
* Description	: Header file for user_usb.c
				  Defines constants and macros for user_usb.c.
*
*
*****************************************************************************/
#ifndef USER_USB_H_
#define USER_USB_H_

#include "asf.h"

/***** Private Definations *****/
#define TRUE			1
#define FALSE			0

#define DBG_USB_DATA	FALSE
#define MX_uBUF_SIZE	670	// To receive data of up to 24 devices.

/***** Extern Variables *****/
extern uint8_t gb_usbuffer[MX_uBUF_SIZE],	// Store USB data into buffer.
//		gb_usbbuf_len,	// Received data length.
		gb_no_usb_f,	// Flag sets when USB disconnected.
		gb_usbRcmplt_f,	// flag sets when reception completes.
		gb_uframe_rcv_tcmplt_f,		// Flag sets when defined time for USB frame receive is completed.
		gb_usbstart_f;	// Flag used to start & stop reception of usb characters.

extern uint16_t gb_usbbuf_len;
extern uint16_t gb_uframe_rcv_timer;	// Variable used to fill timer value for USB frame reception.

extern volatile bool gb_usb_connect;	// Flag sets when USB is connected.

/************************************************
* Function prototypes
************************************************/
void USB_PutChar(uint8_t ch);
void USB_PutString(const char* st);
uint8_t USB_GetChar(void);

void Read_USB_Config_Data(void);
void USB_Print_ASCII(int data);

#endif /* USER_USB_H_ */