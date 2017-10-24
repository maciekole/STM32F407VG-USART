#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "misc.h"

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

void USART2_IRQHandler(void)
{

}

int main(void)
{

	SysTick_Config(SystemCoreClock/1000);

    while(1)
    {
    }
}
