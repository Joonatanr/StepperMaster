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

typedef enum
{
    SPI_RESPONSE_ACK,
    SPI_RESPONSE_NACK,
    NUMBER_OF_SPI_RESPONSE_CODES
} Spi_ResponseCode;

extern void spiCommandHandler_prepareCommand(U8 * dest);
extern void spiCommandHandler_init(void);
extern Boolean spiCommandHandler_setNextCommand(U8 cmd_id, U8 sub, U8 * data, U8 data_len);
extern void spiCommandHandler_ResponseReceived(U8 * data);
extern void spiCommandHandler_cyclic10ms(void);

#endif /* SPICOMMANDHANDLER_H_ */
