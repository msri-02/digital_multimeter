/*
 * comparator.c
 *
 *  Created on: May 15, 2023
 *      Author: msrik
 */

#include "comparator.h"

void comp_init(void)
{
	/* GPIOB PB0 output configuration
	 * GPIOB PB2 input configuration
	 * GPIOB PB1 as reference voltage configuration*/
	RCC -> AHB2ENR |= (RCC_AHB2ENR_GPIOBEN);
	GPIOB->MODER   &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE2| GPIO_MODER_MODE1); // clearing the registers
	// set PB0 to alternate function mode, PB2 to analog mode, PB1 to analog mode
	GPIOB->MODER   |= (2 << GPIO_MODER_MODE0_Pos| 3 << GPIO_MODER_MODE2_Pos| 3 << GPIO_MODER_MODE1_Pos);
	GPIOB->OTYPER  &= ~(GPIO_OTYPER_OT0);
	GPIOB->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED0 | GPIO_OSPEEDR_OSPEED2| GPIO_OSPEEDR_OSPEED1);
	GPIOB->OSPEEDR |=  (3 << GPIO_OSPEEDR_OSPEED0_Pos | 3 << GPIO_OSPEEDR_OSPEED2_Pos| 3 << GPIO_OSPEEDR_OSPEED1_Pos); // set to high speed
	GPIOB->AFR[0]  &= ~(GPIO_AFRL_AFSEL0);         // clear register
	GPIOB->AFR[0]  |=  (12 << GPIO_AFRL_AFSEL0_Pos);   // set AF12 for PB0

	/* comparator configuration */
	COMP1->CSR |= (6 << COMP_CSR_INMSEL_Pos); // set the input selector to PB1
	COMP1->CSR &= ~(COMP_CSR_SCALEN);          // set bit to 01
	COMP1->CSR |= (COMP_CSR_BRGEN);
	COMP1->CSR &= ~(COMP_CSR_POLARITY);       // set polarity to 0
	COMP1->CSR |= (COMP_CSR_INPSEL);          // set to 1, selecting PB2 as the input pin
	COMP1->CSR |= (3 << COMP_CSR_HYST_Pos);   // set to high hysterisis (11) for now
	COMP1->CSR &= ~(COMP_CSR_PWRMODE);        // set to high speed mode


	COMP1->CSR |= (COMP_CSR_EN);              // start comparing
}


void COMP1_IRQHandler(void)
{

}
