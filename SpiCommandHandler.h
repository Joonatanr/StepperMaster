/*
 * SpiCommandHandler.h
 *
 *  Created on: 25. okt 2018
 *      Author: JRE
 */

#ifndef SPICOMMANDHANDLER_H_
#define SPICOMMANDHANDLER_H_

#include "typedefs.h"

#define SPI_COMMAND_LENGTH 128u /* Maximum command size is 128 bytes including checksum. */

typedef enum
{
    CMD_NO_COMMAND,
    CMD_REPORT_STATUS,
    CMD_SET_MOTOR_SPEED,
    NUMBER_OF_SPI_COMMANDS
} CommandId;


extern void spiCommandHandler_PrepareCommand(U8 * dest);
extern void spiCommandHandler_init(void);
extern Boolean spiCommandHandler_setCommand(U8 cmd_id, U8 sub, U8 * data, U8 data_len);

#endif /* SPICOMMANDHANDLER_H_ */
