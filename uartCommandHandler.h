/*
 * commandHandler.h
 *
 *  Created on: 22. juuni 2018
 *      Author: JRE
 */

#ifndef UARTCOMMANDHANDLER_H_
#define UARTCOMMANDHANDLER_H_

#include "typedefs.h"

extern void uartCommandHandler_init(void);
extern void uartCommandHandler_cyclic50ms(void);

extern void uartCommandHandler_setSpeedResponse(void);

#endif /* LOGIC_COMMANDHANDLER_H_ */
