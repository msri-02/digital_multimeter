/*
 * adc.c
 *
 *  Created on: May 8, 2023
 *      Author: msrik
 */


#include "adc.h"

/* global variables */
uint16_t digi_conv;
int adc_flag;

void ADC_init(void)
{
	  /* GPIO CONFIG */
	  // configure GPIOA for channel 5
	  RCC -> AHB2ENR  |= (RCC_AHB2ENR_GPIOAEN);

	  // GPIOA configuration for analog input pin
	  GPIOA->MODER    &= ~(GPIO_MODER_MODE0);         //clear registers
	  GPIOA->MODER    |= (3 << GPIO_MODER_MODE0_Pos); // set to analog mode (11)
	  GPIOA->ASCR     |= (GPIO_ASCR_ASC0);            // connect to the adc input
	  GPIOA->OSPEEDR  &= ~(GPIO_OSPEEDR_OSPEED0);
	  GPIOA->OSPEEDR  |= (3 << GPIO_OSPEEDR_OSPEED0_Pos);	  //set to high speed (11)

	  /* ADC Configuration*/
	  RCC -> AHB2ENR |= RCC_AHB2ENR_ADCEN; // configure clock

	  ADC123_COMMON->CCR = (1 << ADC_CCR_CKMODE_Pos); // adc will run at the same speed as CPU(HCLK/1), prescaler =1
	  ADC1->CR &= ~(ADC_CR_DEEPPWD);                  // power up the adc
	  ADC1->CR |= (ADC_CR_ADVREGEN);

	  for(uint32_t i = 4000; i >0; i-- );             //wait for at least 20 us
	  ADC1->CR &= ~(ADC_CR_ADEN |ADC_CR_ADCALDIF);    //ensure ADC IS disabled and single ended mode
	  ADC1->CR |= ADC_CR_ADCAL;                       // start calibration

	  while(ADC1->CR & ADC_CR_ADCAL);                 // wait for calibration to finish
	  ADC1->DIFSEL &= ~(ADC_DIFSEL_DIFSEL_5);

	  //enable ADC
	  ADC1->ISR |= (ADC_ISR_ADRDY);                   // clear ready bit with a 1
	  ADC1->CR  |= ADC_CR_ADEN;                       // enable ADC
	  while(!(ADC1->ISR & ADC_ISR_ADRDY));            // wait for ready flag = 1
	  ADC1->ISR |= (ADC_ISR_ADRDY);
	  ADC1->SQR1 = (5 << ADC_SQR1_SQ1_Pos);           // set sequence 1 for 1 conversion on channel 5

	  // right align, single conversion
	  ADC1->CFGR = 0;                                 // setting everything to zero
	  ADC1->SMPR1 = 0x111;                           // setting to the fastest clock time


	  // enable interrupts for the end of conversion
	  ADC1->IER |= (ADC_IER_EOC);
	  ADC1->ISR |= (ADC_ISR_EOC);     // clear flag with a 1

	  // enable interrupts for end of conversion
	  NVIC -> ISER[0] = (1 << (ADC1_2_IRQn & 0x1F));

	  __enable_irq(); // enable interrupts globally

	  //start a conversion
	  ADC1->CR |= ADC_CR_ADSTART;

}

void ADC1_2_IRQHandler(void)
{
	if(ADC1->ISR & ADC_ISR_EOC) // at end of the conversion
	{
		digi_conv = ADC1->DR; // save the digital conversion
		adc_flag = 1; // set the global flag tells us conversion is done
	}
}
