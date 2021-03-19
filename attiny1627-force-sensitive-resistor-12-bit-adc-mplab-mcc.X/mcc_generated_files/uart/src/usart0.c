/**
  @Company
    Microchip Technology Inc.

  @Description
    This Source file provides APIs.
    Generation Information :
    Driver Version    :   1.0.0
*/
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


#include "../usart0.h"
#define RECEIVE_ERROR_MASK 0x46

#if defined(__GNUC__)

int USART0_printCHAR(char character, FILE *stream)
{
    while(!(USART0_IsTxReady()));
    USART0_Write(character);
    return 0;
}

FILE USART0_stream = FDEV_SETUP_STREAM(USART0_printCHAR, NULL, _FDEV_SETUP_WRITE);

#elif defined(__ICCAVR__)

int putchar(int outChar)
{
    USART0_Write(outChar);
    return outChar;
}
#endif

static void DefaultFramingErrorCallback(void);
static void DefaultOverrunErrorCallback(void);
static void DefaultParityErrorCallback(void);
void (*USART0_framing_err_cb)(void) = &DefaultFramingErrorCallback;
void (*USART0_overrun_err_cb)(void) = &DefaultOverrunErrorCallback;
void (*USART0_parity_err_cb)(void) = &DefaultParityErrorCallback;

const struct UART_INTERFACE USART0_Interface = {
  .Initialize = USART0_Initialize,
  .Write = USART0_Write,
  .Read = USART0_Read,
  .RxCompleteCallbackRegister = NULL,
  .TxCompleteCallbackRegister = NULL,
  .ErrorCallbackRegister = NULL,
  .FramingErrorCallbackRegister = USART0_FramingErrorCallbackRegister,
  .OverrunErrorCallbackRegister = USART0_OverrunErrorCallbackRegister,
  .ParityErrorCallbackRegister = USART0_ParityErrorCallbackRegister,
  .ChecksumErrorCallbackRegister = NULL,
  .IsRxReady = USART0_IsRxReady,
  .IsTxReady = USART0_IsTxReady,
  .IsTxDone = USART0_IsTxDone
};

void USART0_Initialize(void)
{
    //set baud rate register
    USART0.BAUD = (uint16_t)USART0_BAUD_RATE(115200);
	
    // ABEIE disabled; DREIE disabled; LBME disabled; RS485 DISABLE; RXCIE disabled; RXSIE disabled; TXCIE disabled; 
    USART0.CTRLA = 0x0;
	
    // MPCM disabled; ODME disabled; RXEN enabled; RXMODE NORMAL; SFDEN disabled; TXEN enabled; 
    USART0.CTRLB = 0xC0;
	
    // CMODE Asynchronous Mode; UCPHA enabled; UDORD disabled; CHSIZE Character size: 8 bit; PMODE No Parity; SBMODE 1 stop bit; 
    USART0.CTRLC = 0x3;
	
    //DBGCTRL_DBGRUN
    USART0.DBGCTRL = 0x0;
	
    //EVCTRL_IREI
    USART0.EVCTRL = 0x0;
	
    //RXPLCTRL_RXPL
    USART0.RXPLCTRL = 0x0;
	
    //TXPLCTRL_TXPL
    USART0.TXPLCTRL = 0x0;
	

#if defined(__GNUC__)
    stdout = &USART0_stream;
#endif

}

void USART0_FramingErrorCallbackRegister(void* callbackHandler)
{
    USART0_framing_err_cb=callbackHandler;
}

void USART0_OverrunErrorCallbackRegister(void* callbackHandler)
{
    USART0_overrun_err_cb=callbackHandler;
}

void USART0_ParityErrorCallbackRegister(void* callbackHandler)
{
    USART0_parity_err_cb=callbackHandler;
}

static void DefaultFramingErrorCallback(void)
{
    /* Add your interrupt code here or use USART0.FramingCallbackRegister function to use Custom ISR */
}

static void DefaultOverrunErrorCallback(void)
{
   /* Add your interrupt code here or use USART0.OverrunCallbackRegister function to use Custom ISR */
}

static void DefaultParityErrorCallback(void)
{
    /* Add your interrupt code here or use USART0.ParityCallbackRegister function to use Custom ISR */
}

void USART0_Enable(void)
{
    USART0.CTRLB |= USART_RXEN_bm | USART_TXEN_bm;
}

void USART0_EnableRx(void)
{
    USART0.CTRLB |= USART_RXEN_bm;
}

void USART0_EnableTx(void)
{
    USART0.CTRLB |= USART_TXEN_bm;
}

void USART0_Disable(void)
{
    USART0.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);
}

uint8_t USART0_GetData(void)
{
    return USART0.RXDATAL;
}

bool USART0_IsTxReady(void)
{
    return (USART0.STATUS & USART_DREIF_bm);
}

bool USART0_IsRxReady(void)
{
    return (USART0.STATUS & USART_RXCIF_bm);
}

bool USART0_IsTxBusy(void)
{
    return (!(USART0.STATUS & USART_TXCIF_bm));
}

bool USART0_IsTxDone(void)
{
    return (USART0.STATUS & USART_TXCIF_bm);
}

void USART0_ErrorCheck(void)
{
    uint8_t errorMask = USART0.RXDATAH;
    if(errorMask & RECEIVE_ERROR_MASK)
    {
        if(errorMask & USART_PERR_bm)
        {
            USART0_parity_err_cb();
        } 
        if(errorMask & USART_FERR_bm)
        {
            USART0_framing_err_cb();
        }
        if(errorMask & USART_BUFOVF_bm)
        {
            USART0_overrun_err_cb();
        }
    }
    
}

uint8_t USART0_Read(void)
{       
    return USART0.RXDATAL;
}

void USART0_Write(const uint8_t data)
{
    USART0.TXDATAL = data;
}