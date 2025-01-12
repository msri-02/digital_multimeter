/*
 * adc.h
 *
 *  Created on: May 8, 2023
 *      Author: msrik
 */

#ifndef INC_ADC_H_
#define INC_ADC_H_

#include "stm32l4xx_hal.h"

void ADC_init(void);
void ADC1_2_IRQHandler(void);

extern uint16_t digi_conv;
extern int adc_flag;


#endif /* INC_ADC_H_ */
