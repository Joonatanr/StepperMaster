/*
 * spidrv.c
 *
 *  Created on: Sep 23, 2018
 *      Author: Joonatan
 */


#include "spidrv.h"
#include "driverlib.h"
#include "uartmgr.h"
#include "SpiCommandHandler.h"

/*
 * P1.5 - CLK
 * P1.6 - MOSI
 * P1.7 - MISO
 */


/* SPI Configuration Parameter */
const eUSCI_SPI_MasterConfig spiMasterConfig =
{
     EUSCI_B_SPI_CLOCKSOURCE_SMCLK, 12000000, 1000000, /* SPI clock is set to 1 MHz */
     EUSCI_B_SPI_MSB_FIRST,
     EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT,
     EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH, EUSCI_B_SPI_3PIN
};

/* DMA Control Table */
/* Basically this directive makes sure that the DMA control table ends up on an address divisible by 1024. */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN(MSP_EXP432P401RLP_DMAControlTable, 1024)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=1024
#elif defined(__GNUC__)
__attribute__ ((aligned (1024)))
#elif defined(__CC_ARM)
__align(1024)
#endif
static DMA_ControlTable MSP_EXP432P401RLP_DMAControlTable[32];

#define MAP_SPI_MSG_LENGTH    SPI_COMMAND_LENGTH


/* Private function forward declarations */

Private void handleSpiComplete(void);
Private void startSpiCommunication(void);
Private void setCSPin(Boolean mode);

/* Private variable definitions */
Private U8 priv_tx_data[MAP_SPI_MSG_LENGTH] = { 0 };
Private U8 priv_rx_data[MAP_SPI_MSG_LENGTH] = { 0 };

volatile U8 priv_isr_flag = 0u;


/* Configure this as master DMA. */
Public void spidrv_init(void)
{
    /* Configure CLK, MOSI & MISO for SPI0 (EUSCI_B0) */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1, GPIO_PIN5 | GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configure CS pin as output. */
    /* TODO */
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN7);

    /* Configuring SPI master module */
    MAP_SPI_initMaster(EUSCI_B0_BASE, &spiMasterConfig);

    /* Enable SPI module */
    MAP_SPI_enableModule(EUSCI_B0_BASE);

    /* Configuring DMA module */
    MAP_DMA_enableModule();
    MAP_DMA_setControlBase(MSP_EXP432P401RLP_DMAControlTable);

    /* Assign DMA channel 0 to EUSCI_B0_TX0, channel 1 to EUSCI_B0_RX0 */
    MAP_DMA_assignChannel(DMA_CH0_EUSCIB0TX0);
    MAP_DMA_assignChannel(DMA_CH1_EUSCIB0RX0);
}


/* TODO : Review this. */
Public void spidrv_cyclic50ms(void)
{
    static U8 counter;

    if (priv_isr_flag)
    {
        priv_isr_flag = 0u;
        handleSpiComplete();
    }

    /* Currently we start a new SPI communication every 500 ms. */
    if (++counter > 10u)
    {
        counter = 0u;

        startSpiCommunication();
    }
}


/*********************** Private function definitions **********************/

Private void startSpiCommunication(void)
{
    /* Retrieve command from the command handler. */
    spiCommandHandler_prepareCommand(priv_tx_data);

    setCSPin(TRUE);

    //Small delay for slave to start up transfer.
    MAP_DMA_disableChannel(1);

    /* Start Tx */
    MAP_DMA_disableChannel(0);
    __delay_cycles(4000);

    /* Setup the TX transfer characteristics & buffers */
    MAP_DMA_setChannelControl( DMA_CH0_EUSCIB0TX0 | UDMA_PRI_SELECT,
                               UDMA_SIZE_8 | UDMA_SRC_INC_8 | UDMA_DST_INC_NONE | UDMA_ARB_1);

    MAP_DMA_setChannelTransfer(DMA_CH0_EUSCIB0TX0 | UDMA_PRI_SELECT,
                               UDMA_MODE_BASIC,
                               priv_tx_data,
                               (void *) MAP_SPI_getTransmitBufferAddressForDMA(EUSCI_B0_BASE),
                               MAP_SPI_MSG_LENGTH);

    /* Setup the RX transfer characteristics & buffers */
    MAP_DMA_setChannelControl( DMA_CH1_EUSCIB0RX0 | UDMA_PRI_SELECT,
                               UDMA_SIZE_8 | UDMA_SRC_INC_NONE | UDMA_DST_INC_8 | UDMA_ARB_1);

    MAP_DMA_setChannelTransfer(DMA_CH1_EUSCIB0RX0 | UDMA_PRI_SELECT,
                               UDMA_MODE_BASIC,
                               (void *) MAP_SPI_getReceiveBufferAddressForDMA(EUSCI_B0_BASE),
                               priv_rx_data,
                               MAP_SPI_MSG_LENGTH);

    /* TODO : Not sure if DMA interrupt assigning should be done here. */

    /* Enable DMA interrupt */
    MAP_DMA_assignInterrupt(INT_DMA_INT1, 1);
    MAP_DMA_clearInterruptFlag(DMA_CH1_EUSCIB0RX0 & 0x0F);

    /* Assigning/Enabling Interrupts */
    MAP_Interrupt_enableInterrupt(INT_DMA_INT1);
    MAP_DMA_enableInterrupt(INT_DMA_INT1);

    /* Initial idea is that we test by sending/receiving a message every 1 second. */
    /* Start Rx */
    MAP_DMA_enableChannel(1);

    /* Start Tx */
    MAP_DMA_enableChannel(0);
}


Private void handleSpiComplete(void)
{
    setCSPin(FALSE);
    spiCommandHandler_ResponseReceived(priv_rx_data);

    /* TODO : Is this necessary? */
    /* Enabling Interrupts again...*/
    MAP_Interrupt_enableInterrupt(INT_DMA_INT1);
    MAP_DMA_enableInterrupt(INT_DMA_INT1);
}


/* Drive chip select pin low or high. */
Private void setCSPin(Boolean mode)
{
    if(mode)
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN7);
    }
    else
    {
        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN7);
    }
}


/* TODO : Should process the received data... */
void DMA_INT1_IRQHandler(void)
{
    priv_isr_flag = 1u;

    MAP_DMA_clearInterruptFlag(0);
    MAP_DMA_clearInterruptFlag(1);

    /* Disable the interrupt to allow execution */
    MAP_Interrupt_disableInterrupt(INT_DMA_INT1);
    MAP_DMA_disableInterrupt(INT_DMA_INT1);
}

