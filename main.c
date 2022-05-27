#include <stddef.h>
#include "stm32l1xx.h"
#include "nucleo152start.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void USART2_Init(void);
int USART2_write(int ch);
char USART2_read(void);
void delay_Ms(int delay);

/**
 **===========================================================================
 **
 ** Abstract: main program
 **
 **===========================================================================
 */
int main(void) {
	int result=0;
	double lux=0;
	/* Configure the system clock to 32 MHz and update SystemCoreClock */
	SetSysClock();
	SystemCoreClockUpdate();

	RCC->AHBENR |= 1; //enable GPIOA clock
	RCC->AHBENR |= 4; //enable GPIOA clock
	GPIOA->MODER |= 0x3; //PA0 analog (A0)

//setup ADC1. p272
	RCC->APB2ENR |= 0x00000200; //enable ADC1 clock
	ADC1->CR2 = 0; //bit 1=0: Single conversion mode
	ADC1->SQR3 = 10;
	ADC1->SQR1 = 0;
	ADC1->CR2 |= 1;

	char buf[] = "";
	int lux_degree = 0;
	int lux_decimals = 0;

	USART2_Init();
	/* Infinite loop */
	while (1) {
		ADC1->CR2 |= 0x40000000;
		while (!(ADC1->SR & 2)) {
		} //wait for conversion complete
		result = ADC1->DR;

		//EQUATION
		lux=-0.5825*(double)result+2537.7;

		// Print Decimal Value
		lux = lux * 100; //remove decimals and 34.54 = 3454
		lux_degree = (int) lux / 100;
		lux_decimals = abs((int) lux % 100);
		sprintf(buf, "%d.%d lux ", lux_degree, lux_decimals);

		int len = 0;
		while (buf[len] != '\0')
			len++;

		for (int i = 0; i < len; i++) {
			USART2_write(buf[i]);
		}

		USART2_write('\n');
		USART2_write('\r');

		for (int i = 0; i < len; i++) {
			buf[i]=0;
		}
		lux=0;
		lux_degree=0;
		lux_decimals=0;
		result=0;

		delay_Ms(500);

	}
}

void USART2_Init(void) {

	RCC->APB1ENR |= 0x00020000; //set bit 17 (USART2 EN)
	RCC->AHBENR |= 0x00000001; //enable GPIOA port clock bit 0 (GPIOA EN)
	GPIOA->AFR[0] = 0x00000700; //GPIOx_AFRL p.188,AF7 p.177
	GPIOA->AFR[0] |= 0x00007000; //GPIOx_AFRL p.188,AF7 p.177
	GPIOA->MODER |= 0x00000020; //MODER2=PA2(TX) to mode 10=alternate function mode. p184
	GPIOA->MODER |= 0x00000080; //MODER2=PA3(RX) to mode 10=alternate function mode. p184

	USART2->BRR = 0x00000D05; //11500 BAUD and crystal 32MHz. p710, 116
	USART2->CR1 = 0x00000008; //TE bit. p739-740. Enable transmit
	USART2->CR1 |= 0x00000004; //RE bit. p739-740. Enable receiver
	USART2->CR1 |= 0x00002000; //UE bit. p739-740. Uart enable
}

int USART2_write(int ch) {
//wait while TX buffer is empty
	while (!(USART2->SR & 0x0080)) {
	} //TXE: Transmit data register empty. p736-737
	USART2->DR = (ch); //p739
	return ch;
}

char USART2_read() {
	char data = 0;
//wait while RX buffer data is ready to be read
	while (!(USART2->SR & 0x0020)) {
	} //Bit 5 RXNE: Read data register not empty
	data = USART2->DR; //p739
	return data;
}

void delay_Ms(int delay) {
	int i = 0;
	for (; delay > 0; delay--)
		for (i = 0; i < 2460; i++)
			; //measured with oscilloscope
}
