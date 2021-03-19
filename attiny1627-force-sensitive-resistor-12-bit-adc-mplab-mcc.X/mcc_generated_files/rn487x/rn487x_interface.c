/**
 * \file rn487x_interface.c
 * \brief This file provides and interface between the RN487X and the hardware.
 */
/*
    (c) 2019 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/
#include <string.h>
#include "rn487x_interface.h"
#include "../uart/usart1.h"
#include "../timer/delay.h"
#include "../system/system.h"

static bool connected = false; //**< RN487X connection state */

/**
 * \ingroup RN487X_INTERFACE
 * \brief Initiate Hardware Reset of RN487X
 * 
 * This API is used to issue a hardware Reset to the RN487X device.
 *
 * 
 * \param value true - Reset State false - Normal State
 * \return Nothing
 */
static void RN487X_Reset(bool value);

/**
 * \ingroup RN487X_INTERFACE
 * \brief Notify RN487X to expect incoming data
 * 
 * This API notifies the RN487X device to expect incoming Data.
 * 
 * \param value true - RX Incoming false - No Data Incoming
 * \return Nothing
 * 
 */
static void RN487X_IndicateRx(bool value);

/**
 * \ingroup RN487X_INTERFACE
 * \brief Set the RN487X System Configuration Mode
 * 
 * This API sets the System Configuration Mode to Either Normal application Mode 
 * or Test Mode/Flash Update/EEPROM configuration
 * 
 * \param mode APPLICATION_MODE - Normal Application Mode TEST_MODE - Test Mode/Flash Update/EEPROM Configuration
 * \return Nothing
 */
static void RN487X_SetSystemMode(RN487X_SYSTEM_MODES_t mode);

/**
 * \ingroup RN487X_INTERFACE
 * \brief Handle RN487X Status Message
 * 
 * This API is used to handle incoming status messages.
 * It prints all status messages, If DISCONNECT or STREAM_OPEN is received it manages
 * the state of bool connected.
 * 
 * \param message Passed status message
 * \return Nothing
 */
static void RN487X_MessageHandler(char* message);


/*****************************************************
*   Driver Instance Declaration(s) API(s)
******************************************************/  

const iRN487X_FunctionPtrs_t RN487X = {
    .Write = USART1_Write,
    .Read = USART1_Read,
    .TransmitDone = USART1_IsTxReady,
    .DataReady = USART1_IsRxReady,
    .IndicateRx = RN487X_IndicateRx,
    .ResetModule = RN487X_Reset,
    .SetSystemMode = RN487X_SetSystemMode,
    .DelayMs = DELAY_milliseconds,
    .AsyncHandler = RN487X_MessageHandler
};

/*****************************************************
*   Driver Public API
******************************************************/  

bool RN487X_IsConnected(void)
{
    return connected;
}

/*****************************************************
*   Driver Implementation Private API(s)
******************************************************/  

static void RN487X_Reset(bool value)
{
    if (true == value)
    {
        BT_RST_SetLow();
    }
    else
    {
        BT_RST_SetHigh();
    }
}

static void RN487X_IndicateRx(bool value)
{
    // Implement desired support code for RX Indicator
}

static void RN487X_SetSystemMode(RN487X_SYSTEM_MODES_t mode)
{
    // Implement desired support code for RX Indicator
}


static void RN487X_MessageHandler(char* message)
{
    if (strstr(message, "DISCONNECT"))
    {
        connected = false;
    }
    else if (strstr(message, "STREAM_OPEN"))
    {
        connected = true;
    }
    else
    {
        // Left Intentionally Blank: For General Messages
    }
}
