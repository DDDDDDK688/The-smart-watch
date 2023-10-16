/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "./usart/bsp_usart.h"
#include "bsp_key.h"
#include "./beep/bsp_beep.h"
#include "oled.h"


extern uint32_t TimeDisplay;
extern uint32_t TimeAlarm;
extern unsigned int timeget;
extern unsigned int mod;
extern unsigned int chose;
extern unsigned int k;
extern unsigned int clear;
extern unsigned int bluetooth;
/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/**
  * @brief  This function handles RTC interrupt request.
  * @param  None
  * @retval None
  */
void RTC_IRQHandler(void)
{
	  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
	  {
	    /* Clear the RTC Second interrupt */
	    RTC_ClearITPendingBit(RTC_IT_SEC);
	
	    /* Enable time update */
	    TimeDisplay = 1;
	
	    /* Wait until last write operation on RTC registers has finished */
	    RTC_WaitForLastTask();
	  }
		/*闹钟*/
	  if (RTC_GetITStatus(RTC_IT_ALR) != RESET)
		{
			TimeAlarm  = 1 ;
		}
		RTC_ClearITPendingBit(RTC_IT_ALR|RTC_IT_SEC);
}


void KEY1_IRQHandler(void)
{
	if (EXTI_GetITStatus(KEY1_INT_EXTI_LINE) != RESET) 
	{
//		SysTick_Delay_Ms(50);
		if( chose == 1)
		{
				k++;
				if(k==5){k=1;}
		}

			if(TimeAlarm == 1)
			{
				BEEP(OFF);
				TimeAlarm = 0;
				k=0;
			}
    }
	EXTI_ClearITPendingBit(KEY1_INT_EXTI_LINE);
}

void KEY2_IRQHandler(void)
{
    if (EXTI_GetITStatus(KEY2_INT_EXTI_LINE) != RESET) 
		{

			if( chose == 0)
			{
				chose = 1;
				clear=1;
				k=0;//选项初始化
			}
			if( chose == 1 && k == 1)
			{
				timeget = 1;
				chose = 0;
			}
			if( chose == 1 && k == 2)
			{
				timeget = 2;
				chose = 0;
			}
			if( chose == 1 && k == 3)
			{
				mod++;
				if(mod==3){mod=1;}
				clear = 1;
				chose = 0;
			}
			if( chose == 1 && k == 4)
			{
				bluetooth = 1;
				clear = 1;
				chose = 0;
			}
			
		}
		EXTI_ClearITPendingBit(KEY2_INT_EXTI_LINE);
}


//void DEBUG_USART_IRQHandler(void)
//{
//    uint8_t ucTemp;
//    if (USART_GetITStatus(DEBUG_USARTx,USART_IT_RXNE)!=RESET) {
//        ucTemp = USART_ReceiveData( DEBUG_USARTx );
//        USART_SendData(DEBUG_USARTx,ucTemp);//改为USART2
//		printf("Get char：%c\n",ucTemp);
//	
//			switch (ucTemp)
//			{
//			case '2':
//				mod = 1;printf("mod1\n");
//				break;
//			case '1':
//				mod = 2;printf("mod2\n");
//				break;
//			}
//			USART_ClearITPendingBit(DEBUG_USARTx, USART_IT_RXNE);
//    }

//}
