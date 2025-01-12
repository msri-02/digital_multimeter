/*
 * utility.h
 *
 *  Created on: May 23, 2023
 *      Author: msrik
 */

#ifndef INC_UTILITY_H_
#define INC_UTILITY_H_

void floatToString(float num, char* result, int decimalPlaces);
void toString(uint32_t value, char *str, int max_index);
uint32_t getfreq(uint32_t timer_count);
uint32_t getmin(uint16_t array[]);
uint32_t getmax(uint16_t array[]);
uint32_t getavg(uint16_t array[]);
uint32_t calibrate(uint32_t dig_volt);
float calibrate_float(float dig_volt);

#endif /* INC_UTILITY_H_ */
