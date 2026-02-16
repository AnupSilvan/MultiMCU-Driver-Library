/*****************************************************************************
*
* Module Name	: rs485_uart.h
* Created By	: Anup Silvan Mascarenhas
* Module
* Description	: Header file for rs485_uart.c
*				  Defines constants and macros for rs485_uart.c.
*
*****************************************************************************/
#ifndef RS485_UART_H_
#define RS485_UART_H_

extern volatile U8 gb_usart_ready_f;	// Flag used to check USART is ready or not for transmission.
extern volatile U8 gb_usart_byte_rec_f;	// Flag used to to set when a single byte received.
extern volatile U8 gb_usart_rec_byte;	// variable used to get a byte data from usart.

void RS485_UART_Init(void);
void Put_Data_On_RS485_UART(U8 *tdata, U8 tbuflen);

#endif /* RS485_UART_H_ */