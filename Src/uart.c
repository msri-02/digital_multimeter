/*
 * uart.c
 *
 *  Created on: May 3, 2023
 *      Author: msrik
 */
#include "uart.h"
#define USARTDIV 0xD0UL
#define ESC_CHAR 0x1B
#define XAXIS_LINE   "|-----|-----|-----|-----|-----|"
#define XAXIS_LABELS "0    0.6   1.2   1.8   2.5   3.1"
#define DC_VOLTAGE "DC Voltage:"
#define AC_VOLTAGE "AC Voltage:\n"
#define VRMS "Vrms:"
#define VPP "Vpp:"
#define FLUSH_LEFT "[15D"
#define AC_FLUSH_LEFT "[16D"
#define DC 'd'
#define AC 'a'

int dc_mode_flag;
int ac_mode_flag;

void UART_init(void)
{
	/* enable clock for GPIOA and USART2*/
	RCC->AHB2ENR |=  (RCC_AHB2ENR_GPIOAEN);
	RCC->APB1ENR1|= (RCC_APB1ENR1_USART2EN);

	/* set to alternate function mode */
	GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
	GPIOA->MODER |= (GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1);


	/* enable alternate function registers */
	GPIOA->AFR[0]   	&= ~(GPIO_AFRL_AFSEL2_Msk | GPIO_AFRL_AFSEL3_Msk);	// clear AFR
	GPIOA->AFR[0]		|=  ( (0x7UL << GPIO_AFRL_AFSEL2_Pos) |				// set PA2, PA3
							  (0x7UL << GPIO_AFRL_AFSEL3_Pos));

	/* set to high speed (11) */
	GPIOA->OSPEEDR &=  ~(GPIO_OSPEEDR_OSPEED2 | GPIO_OSPEEDR_OSPEED3);
	GPIOA->OSPEEDR |=   (GPIO_OSPEEDR_OSPEED2 | GPIO_OSPEEDR_OSPEED3);

	/* program the M bits in USART_CR1 to define the word length */
	USART2->CR1 &= ~(USART_CR1_M); // bit 28: set to 00 (8 data bits) 1 char is 8 bits ??????

	USART2->CR1 &= ~(USART_CR1_UE); // disable UE to write BRR

	/* select the desired baud rate using the USART_BRR register */
	//USART2->BRR  &= ~(USART_BRR_DIV_MANTISSA| USART_BRR_DIV_FRACTION); // clear bits [15:4] and [3:0] in BRR
	USART2->BRR  = (USARTDIV); // set bits of BRR to USARTDIV

	/* program the number of stop bits in USART_CR2. */
	USART2->CR2 &= ~(USART_CR2_STOP);               // setting stop bit to 1 (00)

	/*enable the USART by writing the UE bit in USART_CR1 register to 1*/
	USART2->CR1 |= (USART_CR1_UE);                 // setting bit 0 to 1

	/*set the TE bit in USART_CR1 to send an idle frame as first transmission*/
	USART2->CR1 |= (USART_CR1_TE);               // setting transmission enable to  ?


	/* enable interrupts */
	USART2->CR1 |= USART_CR1_RXNEIE;
	NVIC->ISER[1] = (1 << (USART2_IRQn & 0x1F));
	__enable_irq();
	USART2->CR1 |= (USART_CR1_RE);               // setting reception enable to 1   ?
}

void UART_print_char(char string) // right now just printing one character
{
	/* check the TXE flag*/
	// wait until it is ready to be written to, if it is empty then write to it
	while (!(USART_ISR_TXE & USART2->ISR));
	USART2->TDR  = string;           // write a char
}

void UART_print(char* string)
{
	int i = 0;
	while(string[i] != '\0')
	{
		UART_print_char(string[i]);
		i += 1;
	}
}

void USART_ESC_Code(char* code)
{
	/* printing an escape character 0x1B*/
	while (!(USART_ISR_TXE & USART2->ISR));
		USART2->TDR  = ESC_CHAR;
	UART_print(code);
}


void print_grid()
{
	/* prints the starting axis */
	USART_ESC_Code("[H");   // move to top left corner
	USART_ESC_Code("[10B"); // move down 10 spaces
	UART_print(XAXIS_LINE); // print the x axis line
	USART_ESC_Code("[1B");  // move down 1
	USART_ESC_Code("[37D"); // move left 37 spaces
	UART_print(XAXIS_LABELS);
}

void ac_graph_value(char* freq_str, char* vrms, char* vpp, uint32_t voltage)
{
	if(ac_mode_flag == 1)
	{
		USART_ESC_Code("[H");
		print_grid();
		USART_ESC_Code("[H"); // move to top left corner
		USART_ESC_Code("[2K");
		UART_print(AC_VOLTAGE); // print the mode
		// print the RMS value, all caps is the string, lower case is the number
		USART_ESC_Code(FLUSH_LEFT);
		UART_print("Frequency: ");
		UART_print(freq_str);
		UART_print_char(' ');
		UART_print(VRMS);
		UART_print(vrms);
		UART_print_char(' ');
		//print the VPP value
		UART_print(VPP);
		UART_print(vpp);
		USART_ESC_Code("[H");
		USART_ESC_Code("[8B"); // move down 1
		USART_ESC_Code("[2K");
		for(int i = 0; i < voltage; i++)
		{
			UART_print_char('#');
		}
	}
}

void graph_value(uint32_t voltage, char* volt_str)
{
	if(dc_mode_flag == 1)
	{
		USART_ESC_Code("[H"); // move to top left corner
		print_grid();
		USART_ESC_Code("[H");
		UART_print(DC_VOLTAGE);
		UART_print(volt_str);
//		UART_print_char(volt_str[0]);
//		UART_print_char('.');
//		UART_print_char(volt_str[1]);
		USART_ESC_Code("[1B"); // move down 1
		USART_ESC_Code("[2K");
		USART_ESC_Code(FLUSH_LEFT);
		USART_ESC_Code("[8B"); // move down 1
		USART_ESC_Code("[2K");
		int count = voltage;
		for(int i = 0; i < count; i++)
		{
			UART_print_char('#');
		}
	}
}

void reset()
{
	USART_ESC_Code("[H");
	USART_ESC_Code("[2K");
	USART_ESC_Code("[1B"); // move down 1
	USART_ESC_Code("[2K");
}
/* clears screen and resets to top left */
void clear_screen()
{
	USART_ESC_Code("[2J");
	USART_ESC_Code("[H");
}

/* sets flags based on keyboard input */
void USART2_IRQHandler(void)
{
	if ((USART2->ISR & USART_ISR_RXNE) != 0)
	{
		char input = USART2->RDR;
		if(input == DC)
		{
			dc_mode_flag = 1;
			ac_mode_flag = 0;
		}
		else if(input == AC)
		{
			ac_mode_flag = 1;
			dc_mode_flag = 0;
		}
	}
}
