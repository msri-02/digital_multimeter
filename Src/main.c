/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "uart.h"
#include "adc.h"
#include "comparator.h"
#include "utility.h"
#include <math.h>

#define SAMPLE_PERIOD 1200000
#define NUM_SAMPLES 1838
#define MAX_SAMPLES 36765
#define MAX_IDX 9
#define DELAY 100000
#define FREQ_CALIB 3
#define DECIMAL_PLACES 2

/* global variables */

int ccr4_flag;
int ccr1_flag;
uint32_t time;
uint32_t prev_time;
//uint32_t CLOCKCYCLE = 24000000;

void SystemClock_Config(void);

void timer_init(void)
{
	/* configure another gpio pin :(*/
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN; // enable clock for GPIOC
	/* set up GPIOC0 as output */
	GPIOC->MODER &= ~(GPIO_MODER_MODE0); // clear MODE0
	GPIOC->MODER |= (1 << GPIO_MODER_MODE0_Pos); // set MODE0 to 01 (output mode)
	GPIOC->OTYPER &= ~(GPIO_OTYPER_OT0); // set OTYPE0 to push-pull
	GPIOC->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED0); // set OSPEED0 to low speed
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD0); // set PUPD0 to no pull-up/pull-down
	GPIOC->ODR |= (GPIO_ODR_OD0); // set initial value of GPIOC0 to high
	/* end of gpioc config*/

	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

	/* COMPARATOR CLOCK */
	TIM2->CCMR2 |= (1 << TIM_CCMR2_CC4S_Pos);
	/* set filter duration to 4 samples */
	TIM2->CCMR2 &= ~(TIM_CCMR2_IC4F);
	TIM2->CCMR2 |= (0x0011 << TIM_CCMR2_IC4F_Pos);
	/* select the edge of active transition */
	TIM2->CCER &= ~(TIM_CCER_CC4NP | TIM_CCER_CC4P | TIM_CCER_CC4E);
	// set pre-scaler to 0
	TIM2->CCMR2 &= ~(TIM_CCMR2_IC4PSC);
	// enable capture/compare enable
	TIM2->CCER |= TIM_CCER_CC4E;
	TIM2->DIER |= TIM_DIER_CC4IE;
	/* connecting COMP_OUT to TIM2 */
	TIM2->OR1  &= ~(TIM2_OR1_TI4_RMP);
	TIM2->OR1  |= (1 << TIM2_OR1_TI4_RMP_Pos);
	TIM2->SR &= ~(TIM_SR_UIF | TIM_SR_CC4IF); // clear update interrupt flag
	//enable interrupts in NVIC
	NVIC->ISER[0] |= (1 << (TIM2_IRQn & 0x1F));
	__enable_irq();	// enable interrupts globally


	TIM2->CR1 |= (TIM_CR1_CEN);
}

void TIM2_IRQHandler(void)
{
	if(TIM2->SR & TIM_SR_CC4IF) // there is an interrupt
	{
		GPIOC->ODR ^= GPIO_ODR_OD0;
		prev_time = time;
		time = (TIM2->CCR4); // storing timer value in global variable
		TIM2->SR &= ~(TIM_SR_CC4IF); // clear flag
		ccr4_flag = 1;
	}

}

float calculate_RMS(uint32_t avg_volt)
{
	float vrms = 1.1107 * sqrt(avg_volt);
	return vrms;
}

uint32_t calculate_Vpp(uint32_t max, uint32_t min)
{
	uint32_t vpp = max - min;
	return vpp;
}

int main(void)
{
  typedef enum{
	  GET_MODE,
	  DC_MODE,
	  AC_MODE,
	  AC_DISPLAY,
	  DELAY_STATE
  }state_var_type;

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  SystemClock_Config();
  UART_init();
  print_grid();
  ADC_init();
  comp_init();
  timer_init();

  state_var_type state = GET_MODE;
  uint16_t dc_samples[NUM_SAMPLES];
  uint16_t ac_samples[MAX_SAMPLES];
  uint32_t timetoprint;
  uint32_t freq;
  uint32_t vpp;

  float adcavg = 0;
  float ac_adcavg = 0;
  float vrms;
  float vppf;
  int idx = 0;

  while (1)
  {
	  switch(state)
	  {
	  	  /* user presses 'a' or 'd' key to select mode */
	  	  case GET_MODE:
	  		  if(dc_mode_flag)
	  		  {
	  			  state = DC_MODE;
	  		  }
	  		  else if(ac_mode_flag)
	  		  {
	  			  state = AC_MODE;
	  		  }
	  		  break;
	  	 /* sample and get averages */
	  	  case DC_MODE:
	  		  TIM2->CR1 |= (TIM_CR1_CEN);
	  		  if(adc_flag == 1) // checking if the flag is set
	  		  {
	  			  // save the converted value into an array
	  			  adcavg += digi_conv;
	  			  dc_samples[idx] = digi_conv;
	  			  idx += 1;
	  			  adc_flag = 0;                 // reset the flag
	  			  ADC1 -> CR |= ADC_CR_ADSTART; // start another conversion
	  			  state = DC_MODE;
	  		  }
	  		  if(idx == NUM_SAMPLES) // finished sampling for 50 ms
	  		  {
	  			  uint32_t avg = calibrate((getavg(dc_samples)));
	  			  float avgfloat = (float)avg / 1000;
	  			  avg /= 1000;
	  			  idx = 0;
	  			  char avg_str[MAX_IDX + 1];
	  			  floatToString(avgfloat, avg_str, DECIMAL_PLACES);
	  			  graph_value((uint32_t)(avgfloat * 11), avg_str);
	  			  state = DELAY_STATE;
	  		  }
	  		  //state = DC_MODE; // might ahve commented this out?
	  		  break;
	  	  case AC_MODE:
	  		  TIM2->CR1 |= (TIM_CR1_CEN);
	  		  /* print the timer value */
			  if(ccr4_flag == 1)
			  {
				/* getting the period between waves */
				timetoprint = (time - prev_time);
				freq = getfreq(timetoprint) + FREQ_CALIB;
				ccr4_flag = 0;
				state = AC_MODE;
			  }
			  if(adc_flag == 1) // checking if the flag is set
			  {
				  // second approach
				  ac_adcavg += (calibrate(digi_conv) * calibrate(digi_conv));
				  // save the converted value into an array
				  ac_samples[idx] = digi_conv;
				  idx += 1;
				  adc_flag = 0;                 // reset the flag
				  ADC1 -> CR |= ADC_CR_ADSTART; // start another conversion
				  state = AC_MODE;
			  }
			  if(idx == MAX_SAMPLES)
			  {
				  state = AC_DISPLAY;
			  }
			  break;
	  	  case AC_DISPLAY:
	  		  /* will be used for rms*/
	  		  ac_adcavg /= MAX_SAMPLES;
	  		  /* Vpp calculations and string conversion */
			  uint32_t max = calibrate((getmax(ac_samples)));
			  uint32_t min = calibrate((getmin(ac_samples)));
			  vppf = ((float)max - (float)min)/1000;
			  char vppf_str[MAX_IDX + 1];
			  floatToString(vppf, vppf_str, DECIMAL_PLACES);

			  /* frequency string conversion */
	  	      char freq_str[MAX_IDX + 1];
	  	      toString(freq, freq_str, MAX_IDX);

	  	  	  /* VRMS calculations and string conversion */
  			  char vrms_str[MAX_IDX + 1];
  			  vrms = sqrt(ac_adcavg)/1000;
  			  floatToString(vrms, vrms_str, DECIMAL_PLACES);

  			  /* graph it in the UART */
  			  ac_graph_value(freq_str, vrms_str, vppf_str, (int)(vrms*10));

  			  /* reset the counter */
  			  idx = 0;
  			  state = DELAY_STATE;
	  		  break;
	  	  case DELAY_STATE:
	  		  for(int j = 0; j < DELAY; j++);
	  		  print_grid();
	  		  state = GET_MODE;
	  		  break;
	  	  default:
	  		  state = GET_MODE;
	  		  break;
	  }
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_9;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
