/*
 * utility.c
 *
 *  Created on: May 23, 2023
 *      Author: msrik
 */
#include <stdint.h>
#define NUMSAMPLES 1838
uint32_t CLOCKCYCLE = 24000000;

void floatToString(float num, char* result, int decimalPlaces) {
	int i = 0;
	int integerPart = (int)num;
	float decimalPart = num - integerPart;
	// Handle negative numbers
	if (integerPart < 0) {
	result[i++] = '-';
	integerPart = -integerPart;
	decimalPart = -decimalPart;
	}
	// Convert integer part to string
	int j = i;
	do {
	result[j++] = integerPart % 10 + '0';
	integerPart /= 10;
	} while (integerPart);
	// Reverse the integer part
	for (int k = i; k < j / 2; k++) {
	char temp = result[k];
	result[k] = result[j - k - 1];
	result[j - k - 1] = temp;
	}
	// Add decimal point if required
	if (decimalPlaces > 0) {
	result[j++] = '.';
	}
	// Convert decimal part to string
	for (int k = 0; k < decimalPlaces; k++) {
	decimalPart *= 10;
	int digit = (int)decimalPart % 10;
	result[j++] = digit + '0';
	decimalPart -= digit;
	}
	// Null terminate the string
	result[j] = '\0';
}

void toString(uint32_t value, char *str, int max_index)

{
	char nums[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	if (value == 0)
	{
		str[0] = '0';
		str[1] = '\0';
	}
	else
	{
		uint32_t num = value;
		str[max_index - 1] = '\0'; // setting index 8 to null
		int idx = max_index - 2;   // start adding from the end of the array
		int size = 0 ;
		/* adding individual characters into string */
		while(num)
		{
			char toprint = nums[num % 10];
			str[idx] = toprint;
			idx -= 1;
			num /= 10;
			size += 1;
		}
		/* moving stuff so it can be printed */
		if(size < max_index - 1) // if the number is less than 8 digits
		{
			int gap = max_index - size - 1;
			for(int i = 0; i < size + 1; i++)
			{
				str[i] = str[i+ gap];
			}
			str[size] = '\0';
		}
	}
}

uint32_t getfreq(uint32_t timer_count)
{
	uint32_t freq = CLOCKCYCLE/timer_count;
	return freq;
}

uint32_t getmin(uint16_t array[])
{
	uint32_t min = array[0];
	for(int i = 0; i < NUMSAMPLES; i++)
	{
		if(array[i] < min)
		{
			min = array[i];
		}
	}
	return min;
}
uint32_t getmax(uint16_t array[])
{
	uint32_t max = 0;
	for(int i = 0; i < NUMSAMPLES; i++)
	{
		if(array[i] > max)
		{
			max = array[i];
		}
	}
	return max;
}

uint32_t getavg(uint16_t array[])
{
	uint32_t avg = 0;
	for(int i = 0; i < NUMSAMPLES; i++)
	{
		avg += array[i];
	}
	avg = avg / NUMSAMPLES;
	return avg;

}

uint32_t calibrate(uint32_t dig_volt)
{
	int32_t calib_voltage = (820 * dig_volt) - 6000; // old was 13350
	if(calib_voltage < 0)
	{
		calib_voltage = 0;
	}
	else
	{
		calib_voltage /= 1000;
	}
	return (uint32_t)calib_voltage;
}

float calibrate_float(float dig_volt)
{
	float calib_voltage = (820 * dig_volt) - 6449;
	if(calib_voltage < 0)
	{
		calib_voltage = 0;
	}
	else
	{
		calib_voltage /= 1000;
	}
	return calib_voltage;
}
