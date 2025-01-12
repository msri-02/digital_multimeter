/*
 * uart.h
 *
 *  Created on: May 3, 2023
 *      Author: msrik
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "stm32l4xx_hal.h"

extern int dc_mode_flag;
extern int ac_mode_flag;

void UART_init(void);
void UART_print_char(char string);
void UART_print(char* word);
void USART_ESC_Code(char* code);
void print_grid();
void ac_graph_value(char* freq_str, char* vrms, char* vpp, uint32_t voltage);
void graph_value(uint32_t voltage, char* volt_str);
void reset();
void clear_screen();

#endif /* INC_UART_H_ */
