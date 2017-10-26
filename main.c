#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "misc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define LED1 GPIOD, GPIO_Pin_12
#define LED2 GPIOD, GPIO_Pin_13
#define LED3 GPIOD, GPIO_Pin_14
#define LED4 GPIOD, GPIO_Pin_15
#define button GPIOA, GPIO_Pin_0


#define USART_TXBUF_LEN 512	// Transmit buffor length
#define USART_RXBUF_LEN	128	// Receive buffor length

int USART_TX_Empty;
int USART_TX_Busy;
int USART_RX_Empty;
int USART_RX_Busy;

uint32_t USART_TxBuf[USART_TXBUF_LEN];
uint32_t USART_RxBuf[USART_RXBUF_LEN];

void Delay(__IO uint32_t ms)
{
	while(ms--);
}

void LEDS_setup(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_14 | GPIO_Pin_13 | GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/*
 *  Using USART2 | APB1(max. 42MHz) | USART2_TX PD5\PA2 | USART2_RX PD6\PA3
 *
 *  Function initializes USART2
 *
 */
void USART2_setup(void)
{
	// structures
	GPIO_InitTypeDef 	gpio;
	USART_InitTypeDef 	usart;
	NVIC_InitTypeDef 	nvic;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	// USART2 pins configuration
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);

	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
	gpio.GPIO_PuPd = GPIO_PuPd_UP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &gpio);

	// USART2 parameters configuration
	usart.USART_BaudRate = 9600;
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart.USART_Parity = USART_Parity_No;
	usart.USART_StopBits = USART_StopBits_1;
	usart.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &usart);
	USART_Cmd(USART2, ENABLE);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	// USART interrupts configuration
	nvic.NVIC_IRQChannel = USART2_IRQn;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvic);

}

/*
 * 	USART2 interrupt handler
 */
void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)	// If receive interrupt:
	{
		USART_RxBuf[USART_RX_Empty] = USART_ReceiveData(USART2);
		USART_RX_Empty++;

		if(USART_RX_Empty >= USART_RXBUF_LEN)
		{
			USART_RX_Empty = 0;
		}
	}

	if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)	// If transmit interrupt:
	{
		if(USART_TX_Busy != USART_TX_Empty)
		{
			USART_SendData(USART2, USART_TxBuf[USART_TX_Busy]);
			USART_TX_Busy++;

			if(USART_TX_Busy >= USART_TXBUF_LEN)
			{
				USART_TX_Busy = 0;
			}
			else
			{
				USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
			}
		}
	}

}

uint8_t USART_getsinglechar(void)
{
	uint8_t temp;

	if(USART_RX_Empty != USART_RX_Busy)
	{
		temp = USART_RxBuf[USART_RX_Busy];
		USART_RX_Busy++;

		if(USART_RX_Busy >= USART_RXBUF_LEN)
		{
			USART_RX_Busy = 0;
		}
		return temp;
	}
	else return 0;
}

void USART_send(char *data, ...)
{
	char tmp_rs[128];
	__IO int index;
	int i;

	va_list arglist;
	va_start(arglist, data);
	vsprintf(tmp_rs, data, arglist);
	va_end(arglist);

	index = USART_TX_Empty;
	for(i = 0; strlen(tmp_rs); i++)
	{
		USART_TxBuf[index] = tmp_rs[i];
		index++;
		if(index >= USART_TXBUF_LEN)
		{
			index = 0;
		}
	}

	__disable_irq();

	if((USART_TX_Empty == USART_TX_Busy) && (USART2 -> SR & USART_FLAG_TXE))
	{
		USART_TX_Empty = index;
		USART_SendData(USART2, USART_TxBuf[USART_TX_Busy]);
		USART_TX_Busy++;
		if(USART_TX_Busy >= USART_TXBUF_LEN)
		{
			USART_TX_Busy = 0;
		}
		USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
	}
	else
	{
		USART_TX_Empty = index;
	}

	__enable_irq();
}

int main(void)
{

	SysTick_Config(SystemCoreClock/1000);

	GPIO_ToggleBits(LED1);

	USART2_setup();

	GPIO_ToggleBits(LED1);


	Delay(500);
	GPIO_SetBits(LED2 | LED3 | LED4);
	USART_send("ala ma kota");

    while(1)
    {
    }
}
