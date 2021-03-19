/*
Copyright (c) [2012-2020] Microchip Technology Inc.  

    All rights reserved.

    You are permitted to use the accompanying software and its derivatives 
    with Microchip products. See the Microchip license agreement accompanying 
    this software, if any, for additional info regarding your rights and 
    obligations.
    
    MICROCHIP SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT 
    WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT 
    LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT 
    AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP OR ITS
    LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT 
    LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE 
    THEORY FOR ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT 
    LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES, 
    OR OTHER SIMILAR COSTS. 
    
    To the fullest extend allowed by law, Microchip and its licensors 
    liability will not exceed the amount of fees, if any, that you paid 
    directly to Microchip to use this software. 
    
    THIRD PARTY SOFTWARE:  Notwithstanding anything to the contrary, any 
    third party software accompanying this software is subject to the terms 
    and conditions of the third party's license agreement.  To the extent 
    required by third party licenses covering such third party software, 
    the terms of such license will apply in lieu of the terms provided in 
    this notice or applicable license.  To the extent the terms of such 
    third party licenses prohibit any of the restrictions described here, 
    such restrictions will not apply to such third party software.
*/
#include "mcc_generated_files/system/system.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mcc_generated_files/rn487x/rn487x.h"
#include "mcc_generated_files/rn487x/rn487x_interface.h"
#include "util/delay.h"
#include "RGBClick_4x4.h"


#define ADC_MAX_VALUE_for_FORCE_CLICK 0xFCF /* In the test setup, VDD = 3.3V, ADC ref is VDD. 12 bit ADC count is 4095 at 3.3V
                                                 When maximum force is applied on the Force sensor the voltage on ADC pin is 3.261V 
                                                 which gives ADC count ~ 0xFCF. It may vary from setup to setup */


#define SAMPLES  ADC_SAMPNUM_ACC1024_gc /* 1024 samples accumulated */

#define MAX_VOL 3.3
#define MAX_FORCE_PERCENT	100.0


#define COUNTER_to_CHANGE_COLOR 0x0F

#define MAX_COLORS 7


rgb_led_t rgb_array_configure[RGB_CLICK_NUM_LEDS] = {0};
rgb_led_t rgb_array_output[RGB_CLICK_NUM_LEDS] = {0};
uint8_t output_channel = 0;
uint32_t counter = 0;
float strength_percentage;


uint8_t brightness = 0;
uint8_t leds_to_glow = 0;
uint8_t led_seq_to_glow[RGB_CLICK_NUM_LEDS] = {3, 2, 7, 1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 8, 13, 12}; /* LED numbers on 4x4 RGB Click, array with needed sequence */

/* Colors are configured with 5% intensity. Example 100% intensity for blue color is 0x00, 0x00, 0xFF */
uint8_t led_color_array[MAX_COLORS][NEOPIXEL_CHANNELS] =
{
	{0x00, 0x00, 0x0F}, /* Blue */
	{0x00, 0x0F, 0x00}, /* Red */
	{0x0F, 0x00, 0x00}, /* Green */
	{0x0F, 0x0F, 0x00}, /* Yellow */
	{0x00, 0x0F, 0x0F}, /* Magenta */
	{0x0F, 0x00, 0x0F}, /* Cyan */
	{0x0F, 0x0F, 0x0F}  /* White */
};
uint8_t change_color_index = 0;

struct
{
	uint32_t adc_result;
	uint16_t adc_sample;
	uint16_t adc_average_result;
} adc_t;

/***************************************transmit_to_terminal********************************************
Convert float number (calculated strength) to string and send to USART0, (PC serial terminal)
**************************************************************************************************/
void transmit_to_terminal(float number)
{
	char string[50];

	dtostrf(number, 4, 2, string);
	printf("\nStrength:");
    printf(string);
    printf("%%");
  
}

/***************************************transmit_to_terminal********************************************
Convert float number (calculated strength) to string and send to USART1, BLE
**************************************************************************************************/
void transmit_to_BLE(float number)
{
	char string[50];
    
    dtostrf(number, 4, 2, string);
   
    RN487X_SendCmd((const uint8_t *)"Strength:", strlen("Strength:"));
    RN487X_SendCmd((const uint8_t *)string, strlen(string));
   
    RN487X.Write('%');
    RN487X.Write('\r');
    RN487X.Write('\n');	
}

/**************************************RN487x_Setup_Transparent_UART_service***********************
     Configure RN4871/70 in UART Transparent service
     Setup Bluetooth module name
**************************************************************************************************/

void RN487X_Setup_Transparent_UART_service()
{
    uint8_t CommandMode[3]={"$$$"};   
    const char BLE_Name[13]={"tiny2AVR-DEMO"};  
    uint8_t TransparentUartService[6]={"SS,C0\r"};  
    uint8_t RebootCommand[4]={"R,1\r"}; 
    uint8_t FactoryResetCommand[5]={"SF,1\r"}; 

    
    RN487X_Initialize(); /*Initialize the RN487x */
  
    RN487X_SendCmd((const uint8_t *)FactoryResetCommand, 5);  /* Factory reset command */
    RN487X.DelayMs(100);
    RN487X_SendCmd((const uint8_t *)CommandMode, 3); /* Send command mode */
    RN487X.DelayMs(100);
    RN487X_WaitForMsg((const char*)"CMD> ",strlen("CMD> ")); /* Wait for response after command mode */
    
    RN487X_SetName(BLE_Name,13); /* Configure the BLE module name */
   
    RN487X_SendCmd(TransparentUartService, 6);  /* Transparent UART Service mode */
    RN487X.DelayMs(100);
    RN487X_WaitForMsg((const char*)"AOK\r\nCMD> ",strlen("AOK\r\nCMD> ")); /* Wait for response after Transparent UART Service mode */
   
    RN487X_SendCmd(RebootCommand, 4); /* Reboot command */
    RN487X.DelayMs(100);
    RN487X_WaitForMsg((const char*)"Rebooting\r\n",strlen("Rebooting\r\n")); /* Wait for response after Reboot command */
}

/**************************************rgb_pattern_MixColor****************************************
    Different LEDs with different colors, set color for 16 LEDs by
    filling array rgb_array_output[0-15] with different colors
**************************************************************************************************/
void rgb_pattern_MixColor()
{
	uint8_t i, j = 0;
	for(i = 0; i <= RGB_CLICK_NUM_LEDS - 1; i++)
	{
		rgb_array_output[i].green = led_color_array[j][0];
		rgb_array_output[i].red = led_color_array[j][1];
		rgb_array_output[i].blue = led_color_array[j][2];
		j++;
		if(j >= MAX_COLORS)
		{
			j = 0;
		}
	}
	rgb_update(rgb_array_output, RGB_CLICK_NUM_LEDS);
	_delay_ms(1000);
}

/**************************************rgb_pattern_Red_Green_White*********************************
   Same color for all LEDS, color is configured in rgb_array_output[0].
   Colors configured are RED,Green, White
**************************************************************************************************/
void rgb_pattern_Red_Green_White()
{
	//RED
	rgb_array_output[0].green = 0x00;
	rgb_array_output[0].red = 0x0F;
	rgb_array_output[0].blue = 0x00;
	rgb_update_single_color(rgb_array_output[0], RGB_CLICK_NUM_LEDS);
	_delay_ms(500);
	//GREEN
	rgb_array_output[0].green = 0x0F;
	rgb_array_output[0].red = 0x00;
	rgb_array_output[0].blue = 0x00;
	rgb_update_single_color(rgb_array_output[0], RGB_CLICK_NUM_LEDS);
	_delay_ms(500);
	//WHITE
	rgb_array_output[0].green = 0x0F;
	rgb_array_output[0].red = 0x0F;
	rgb_array_output[0].blue = 0x0F;
	rgb_update_single_color(rgb_array_output[0], RGB_CLICK_NUM_LEDS);

}

/**************************************rgb_clear_all_leds******************************************
   Clear output array.
**************************************************************************************************/
void rgb_clear_all_leds()
{
	uint8_t i;

	for(i = 0; i < RGB_CLICK_NUM_LEDS; i++)
	{
		rgb_array_output[i].green = 0;
		rgb_array_output[i].blue = 0;
		rgb_array_output[i].red = 0;
	}
}

/**************************************rgb_display_pattern_per_force*******************************
    Calculate brightness as per applied force by reading adc_t.adc_average_result and right shift(12bit ADC result) by 4 bits as brightness is 8 bit variable.
    Calculate leds_to_glow as per applied force. Maximum 16 LEDS to lit.
    When leds_to_glow < 16, fill array output only for required LEDs with blue color and brightness.
    when leds_to_glow =16, 100% strength, fill array output  for all LEDs with different colors brightness.
    index to change the color is changed in main after certain delay which is defined by COUNTER_to_CHNAGE_COLOR.
**************************************************************************************************/
void rgb_display_pattern_per_force()
{
	uint8_t i;
   
	brightness = (uint8_t)(adc_t.adc_average_result >> 4);  /* Brightness as per ADC result. For RGB LEDs, brightness is 8 bit so shift right the 12bit ADC result by 4 */
	leds_to_glow = 	(uint8_t)((adc_t.adc_average_result * (RGB_CLICK_NUM_LEDS-1)) / ADC_MAX_VALUE_for_FORCE_CLICK);	/* As per ADC result. Maximum LEDS to glow are 16 */ 
	/* When leds_to_glow = 15, All 16 LEDs will be lit */
	rgb_clear_all_leds();
	if(leds_to_glow != RGB_CLICK_NUM_LEDS - 1) /* Force is not 100% so number of LEDs to glow are less than 16 */
	{
		for(i = 0; i <= leds_to_glow ; i++) /* Fill needed LEDs with color and brightness as per force */
		{
			rgb_array_output[led_seq_to_glow[i]].green = 0x00;
			rgb_array_output[led_seq_to_glow[i]].red =  0x00;
			rgb_array_output[led_seq_to_glow[i]].blue = brightness;
		}

	}
	else /* When 100% strength, fill LEDs with different colors and turn ON all 16 LEDS */
	{
		counter++;
		if(counter >= COUNTER_to_CHANGE_COLOR)
		{
			counter = 0;
			change_color_index++;
			if(change_color_index >= MAX_COLORS)
			{
				change_color_index = 0;
			}
		}
		for(i = 0; i <= leds_to_glow ; i++)
		{
			rgb_array_output[led_seq_to_glow[i]].green = led_color_array[change_color_index][0] ;
			rgb_array_output[led_seq_to_glow[i]].red  =  led_color_array[change_color_index][1] ;
			rgb_array_output[led_seq_to_glow[i]].blue =  led_color_array[change_color_index][2] ;
		}

	}
	rgb_update(rgb_array_output, RGB_CLICK_NUM_LEDS);
}

/**************************************main********************************************************
    Initialize peripherals.
    Display RGB LEDs colors.
    Set up RN4871/70 in UART Transparent service.
    Start free-running ADC conversion.
    When ADC result is ready(after accumulating 1024 samples), read accumulated result,
    calculate average result,calculate % strength .
    Update RGB LEDs color and brightness as per applied force on Force sensitive resistor on Force click.
    Send % strength to USART1 and USART0, to transmit to Bluetooth and PC terminal.    
**************************************************************************************************/
int main(void)
{
    SYSTEM_Initialize();
    rgb_pattern_MixColor();
	rgb_pattern_Red_Green_White();
    RN487X_Setup_Transparent_UART_service();
    ADC0_StartConversion(ADC_MUXPOS_AIN6_gc);

    while(1)
    {
        if(ADC0_IsConversionDone())
        {
            adc_t.adc_result = ADC0_GetConversionResult();
            adc_t.adc_average_result = (uint16_t)(adc_t.adc_result >> (SAMPLES));
           	strength_percentage  = (float)(adc_t.adc_average_result * MAX_FORCE_PERCENT) / (ADC_MAX_VALUE_for_FORCE_CLICK);
            rgb_display_pattern_per_force();
            transmit_to_terminal(strength_percentage);
            transmit_to_BLE(strength_percentage);           
            
        }
    }    
}