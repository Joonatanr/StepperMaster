/*
 * comm.h
 *
 *  Created on: 28. mai 2017
 *      Author: Joonatan
 */

#ifndef HW_COMM_H_
#define HW_COMM_H_

#include "typedefs.h"

#define UART_BUF_LEN 64

typedef Boolean (*UartCommandHandler)(char *buf, U16 msg_len);

extern UartCommandHandler CmdHandlerFunc;

extern void uartmgr_send_char(char c);
extern void uartmgr_send_str(const char * str);
extern U8 uartmgr_receiveData(char * dest);
extern void uartmgr_send_number(long nr);
extern void uartmgr_send_rn(void);
extern void uartmgr_send_str_async(const char * str, U16 len);
extern void uartmgr_send_data(const U8 * data, U16 len);

extern void uartmgr_init(void);
extern void uartmgr_cyclic(void);

#endif /* HW_COMM_H_ */
