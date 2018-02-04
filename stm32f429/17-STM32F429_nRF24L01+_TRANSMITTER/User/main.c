/**
 *	Keil project for NRF24L01+ transceiver
 *
 *	Transmitter code
 *
 *	Before you start, select your target, on the right of the "Load" button
 *
 *	@author		Tilen Majerle
 *	@email		tilen@majerle.eu
 *	@website	http://stm32f4-discovery.com
 *	@ide		Keil uVision 5
 *	@packs		STM32F4xx Keil packs version 2.2.0 or greater required
 *	@stdperiph	STM32F4xx Standard peripheral drivers version 1.4.0 or greater required
 */
/* Include core modules */
#include "stm32f4xx.h"
/* Include my libraries here */
#include "defines.h"
#include "tm_stm32f4_nrf24l01.h"
#include "tm_stm32f4_delay.h"
#include "tm_stm32f4_usart.h"
#include <stdio.h>

#include "tm_stm32f4_nrf24l01.h"

/* SPI chip enable pin */
#ifndef NRF24L01_CSN_PIN
#define NRF24L01_CSN_PORT			GPIOA	//GPIOD
#define NRF24L01_CSN_PIN			GPIO_PIN_12	//GPIO_PIN_7
#endif

/* Chip enable for transmitting */
#ifndef NRF24L01_CE_PIN
#define NRF24L01_CE_PORT			GPIOA //GPIOD
#define NRF24L01_CE_PIN				GPIO_PIN_11 //GPIO_PIN_8
#endif

/* Pins configuration */
#define NRF24L01_CE_LOW				TM_GPIO_SetPinLow(NRF24L01_CE_PORT, NRF24L01_CE_PIN)
#define NRF24L01_CE_HIGH			TM_GPIO_SetPinHigh(NRF24L01_CE_PORT, NRF24L01_CE_PIN)

/* My address */
uint8_t MyAddress[] = {
	0xE7,
	0xE7,
	0xE7,
	0xE7,
	0xE7
};
/* Receiver address */
uint8_t TxAddress[] = {
	0x7E,
	0x7E,
	0x7E,
	0x7E,
	0x7E
};

uint8_t dataOut[32], dataIn[32];

int main(void) {
	TM_NRF24L01_Transmit_Status_t transmissionStatus;
	char str[40];
	
	/* Initialize system */
	SystemInit();
	
	/* Initialize system and Delay functions */
	TM_DELAY_Init();
	
	/* Initialize onboard leds */
	//TM_DISCO_LedInit();
	
	/* Initialize USART, TX: PB6, RX: PB7 */
	TM_USART_Init(USART2, TM_USART_PinsPack_1, 115200);
	
	/* Initialize NRF24L01+ on channel 15 and 32bytes of payload */
	/* By default 2Mbps data rate and 0dBm output power */
	/* NRF24L01 goes to RX mode by default */
	TM_NRF24L01_Init(15, 32);
	
	/* Set 1MBps data rate and -18dBm output power */
	TM_NRF24L01_SetRF(TM_NRF24L01_DataRate_1M, TM_NRF24L01_OutputPower_M18dBm);
	
	/* Set my address, 5 bytes */
	TM_NRF24L01_SetMyAddress(MyAddress);
	
	/* Set TX address, 5 bytes */
	TM_NRF24L01_SetTxAddress(TxAddress);
	
//	while (1) {
//		/* Chip enable put to low, disable it */
//		NRF24L01_CE_LOW;
//		
//		/* Go to power up tx mode */
//		//TM_NRF24L01_PowerUpTx();
//		
//		/* Send payload to nRF24L01+ */
//		NRF24L01_CSN_LOW;
//		/* Send write payload command */
//		TM_SPI_Send(NRF24L01_SPI, 0x55);
//		/* Disable SPI */
//		NRF24L01_CSN_HIGH;
//		
//		/* Send data! */
//		NRF24L01_CE_HIGH;
//	}
	
	/* Reset counter */
	TM_DELAY_SetTime(2001);
	
	while (1) {
		/* Every 2 seconds */
		if (TM_DELAY_Time() > 2000) {
			
			/* Fill data with something */
			sprintf((char *)dataOut, "abcdefghijklmnoszxABCDEFCBDA");
			
			/* Display on USART */
			TM_USART_Puts(USART2, "pinging: ");
			
			/* Reset time, start counting microseconds */
			TM_DELAY_SetTime(0);
			
			/* Transmit data, goes automatically to TX mode */
			TM_NRF24L01_Transmit(dataOut);
			
			/* Turn on led to indicate sending */
			//TM_DISCO_LedOn(LED_GREEN);
			
			uint8_t reg;
			/* Wait for data to be sent */
			do {
				/* Get transmission status */
				transmissionStatus = TM_NRF24L01_GetTransmissionStatus();
				reg = TM_NRF24L01_ReadRegister(0x00);
			} while (transmissionStatus == TM_NRF24L01_Transmit_Status_Sending);
			
			/* Turn off led */
			//TM_DISCO_LedOff(LED_GREEN);
			
			/* Go back to RX mode */
			TM_NRF24L01_PowerUpRx();
			
			/* Wait received data, wait max 100ms, if time is larger, then data were probably lost */
			while (!TM_NRF24L01_DataReady() && TM_DELAY_Time() < 100);
			
			/* Format time */
			sprintf(str, "%d ms", TM_DELAY_Time());
			
			/* Show ping time */
			TM_USART_Puts(USART2, str);
			
			/* Get data from NRF2L01+ */
			TM_NRF24L01_GetData(dataIn);
			
			/* Check transmit status */
			if (transmissionStatus == TM_NRF24L01_Transmit_Status_Ok) {
				/* Transmit went OK */
				TM_USART_Puts(USART2, ": OK\n");
			} else if (transmissionStatus == TM_NRF24L01_Transmit_Status_Lost) {
				/* Message was LOST */
				TM_USART_Puts(USART2, ": LOST\n");
			} else {
				/* This should never happen */
				TM_USART_Puts(USART2, ": SENDING\n");
			}
		}
	}
}
