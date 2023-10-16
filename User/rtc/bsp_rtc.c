/**
  ******************************************************************************
  * @file    bsp_rtc.c
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   stm32 RTC ����
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� F103-ָ���� STM32 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
#include "./usart/bsp_usart.h"
#include "./rtc/bsp_rtc.h"
#include "oled.h"



/* ���жϱ�־���������ж�ʱ��1����ʱ�䱻ˢ��֮����0 */
__IO uint32_t TimeDisplay = 0;

/*���������־�����ж��������¼���1*/
__IO uint32_t TimeAlarm = 0;

/*���ڣ���Ф������ASCII��*/
char const *WEEK_STR[] = {"��", "һ", "��", "��", "��", "��", "��"};
char const *zodiac_sign[] = {"��", "��", "ţ", "��", "��", "��", "��", "��", "��", "��", "��", "��"};

/*Ӣ�ģ����ڣ���Ф������ASCII��*/
char const *en_WEEK_STR[] = { "Sunday","Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char const *en_zodiac_sign[] = {"Pig", "Rat", "Ox", "Tiger", "Rabbit", "Dragon", "Snake", "Horse", "Goat", "Monkey", "Rooster", "Dog"};


/*
 * ��������NVIC_Configuration
 * ����  ������RTC���жϵ����ж����ȼ�Ϊ1�������ȼ�Ϊ0
 * ����  ����
 * ���  ����
 * ����  ���ⲿ����
 */
void RTC_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/*
 * ��������RTC_CheckAndConfig
 * ����  ����鲢����RTC
 * ����  �����ڶ�ȡRTCʱ��Ľṹ��ָ��
 * ���  ����
 * ����  ���ⲿ����
 */
void RTC_CheckAndConfig(struct rtc_time *tm)
{

	struct rtc_time configtime;
   	/*������ʱ��鱸�ݼĴ���BKP_DR1��������ݲ���0xA5A5,
	  ������������ʱ�䲢ѯ���û�����ʱ��*/
	if (BKP_ReadBackupRegister( BKP_DR1 ) != RTC_BKP_DATA1)
	{
		printf("\r\n\r\n RTC not yet configured....");
		printf("\r\n\r\n RTC configured....");

		/* ʹ��tm��ʱ������RTC�Ĵ��� */
		Time_Adjust(tm);
		/*��BKP_DR1�Ĵ���д���־��˵��RTC��������*/
		BKP_WriteBackupRegister(BKP_DR1 , RTC_BKP_DATA1);
	}
	else
	{
		
		/* ʹ�� PWR �� Backup ʱ�� */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
		/* ������� Backup ���� */
		PWR_BackupAccessCmd(ENABLE);
	
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
		{
		    printf("\r\n\r\n Power On Reset occurred....");
		}
		/*����Ƿ�Reset��λ*/
		if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
		{
			printf("\r\n\r\n External Reset occurred....");
		}
		if (RCC_GetFlagStatus(RCC_FLAG_SFTRST) != RESET)
		{
			printf("\r\n\r\n Software reset occurred....");
		}
	
		printf("\r\n No need to configure RTC....");
		
		/*�ȴ��Ĵ���ͬ��*/
		RTC_WaitForSynchro();
		RTC_WaitForLastTask();
		/*����RTC���ж�*/
		RTC_ITConfig(RTC_IT_SEC|RTC_IT_ALR, ENABLE);
		/*�ȴ��ϴ�RTC�Ĵ���д�������*/
		RTC_WaitForLastTask();
		configtime.tm_year = BKP_ReadBackupRegister(BKP_DR2);
		RTC_WaitForLastTask();
		configtime.tm_mon = BKP_ReadBackupRegister(BKP_DR10);
		RTC_WaitForLastTask();
		configtime.tm_mday = BKP_ReadBackupRegister(BKP_DR4);
		RTC_WaitForLastTask();
		configtime.tm_hour = BKP_ReadBackupRegister(BKP_DR5);
		RTC_WaitForLastTask();
		configtime.tm_min = BKP_ReadBackupRegister(BKP_DR6);
		RTC_WaitForLastTask();
		configtime.tm_sec = BKP_ReadBackupRegister(BKP_DR7);
		RTC_WaitForLastTask();
		
		RTC_SetAlarm(mktimev(&configtime)-TIME_ZOOM);
		RTC_WaitForLastTask();
		
	}
	  RCC_ClearFlag();
}
/*
 * ��������RTC_Configuration
 * ����  ������RTC
 * ����  ����
 * ���  ����
 * ����  ���ⲿ����
 */
void RTC_Configuration(void)
{
	/* ʹ�� PWR �� Backup ʱ�� */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	
	/* ������� Backup ���� */
	PWR_BackupAccessCmd(ENABLE);
	
//	/* ��λ Backup ���� */
//	BKP_DeInit();

#ifdef 	RTC_CLOCK_SOURCE_LSE
	/* ʹ�� LSE */
	RCC_LSEConfig(RCC_LSE_ON);
	
	/* �ȴ� LSE ׼���� */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{}
	
	/* ѡ�� LSE ��Ϊ RTC ʱ��Դ */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	
	/* ʹ�� RTC ʱ�� */
	RCC_RTCCLKCmd(ENABLE);
	
	/* �ȴ� RTC �Ĵ��� ͬ��
	 * ��ΪRTCʱ���ǵ��ٵģ��ڻ�ʱ���Ǹ��ٵģ�����Ҫͬ��
	 */
	RTC_WaitForSynchro();
	
	/* ȷ����һ�� RTC �Ĳ������ */
	RTC_WaitForLastTask();
	
	/* ʹ�� RTC ���ж� �� �����ж�*/
	RTC_ITConfig(RTC_IT_SEC|RTC_IT_ALR, ENABLE);
	
	/* ȷ����һ�� RTC �Ĳ������ */
	RTC_WaitForLastTask();
	
	/* ���� RTC ��Ƶ: ʹ RTC ����Ϊ1s  */
	/* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) = 1HZ */
	RTC_SetPrescaler(32767); 
	
	/* ȷ����һ�� RTC �Ĳ������ */
	RTC_WaitForLastTask();
	
#else

	/* ʹ�� LSI */
	RCC_LSICmd(ENABLE);

	/* �ȴ� LSI ׼���� */
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	{}
	
	/* ѡ�� LSI ��Ϊ RTC ʱ��Դ */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	
	/* ʹ�� RTC ʱ�� */
	RCC_RTCCLKCmd(ENABLE);
	
	/* �ȴ� RTC �Ĵ��� ͬ��
	 * ��ΪRTCʱ���ǵ��ٵģ��ڻ�ʱ���Ǹ��ٵģ�����Ҫͬ��
	 */
	RTC_WaitForSynchro();
	
	/* ȷ����һ�� RTC �Ĳ������ */
	RTC_WaitForLastTask();
	
	/* ʹ�� RTC ���ж� */
	RTC_ITConfig(RTC_IT_SEC|RTC_IT_ALR, ENABLE);
	
	/* ȷ����һ�� RTC �Ĳ������ */
	RTC_WaitForLastTask();
	
	/* ���� RTC ��Ƶ: ʹ RTC ����Ϊ1s ,LSIԼΪ40KHz */
	/* RTC period = RTCCLK/RTC_PR = (40 KHz)/(40000-1+1) = 1HZ */	
	RTC_SetPrescaler(40000-1); 
	
	/* ȷ����һ�� RTC �Ĳ������ */
	RTC_WaitForLastTask();
#endif
	
}



/*
 * ��������Time_Regulate_Get
 * ����  �������û�ʹ�ô������õ�ʱ�䣬
 *         �Ա����ת����ʱ����洢��RTC �����Ĵ����С�
 * ����  �����ڶ�ȡRTCʱ��Ľṹ��ָ��
 * ע��  ���ڴ��ڵ�����������ʱ������������Ҫ�ӻس�
 */
void Time_Regulate_Get(struct rtc_time *tm)
{
		uint32_t temp_num = 0;
		uint8_t day_max=0 ;
		printf("\r\n Setting Count Time");
	  do 
	  {
		  printf("\r\n  (Please Set Years),range[1970~2038],please add enter after input:");
			scanf("%d",&temp_num);
			if(temp_num <1970 || temp_num >2038)
			{
				printf("\r\n your inputs is :%d��It is invalid",temp_num);					  
			}
			else
			{	  
				printf("\n\r  The year is setting to: %d\n\r", temp_num);
				tm->tm_year = temp_num;
				break;
			}
	  }while(1);


	 do 
	  {
		  printf("\r\n  (Please Set Months):range[1~12],please add enter after input:");
			scanf("%d",&temp_num);
			if(temp_num <1 || temp_num >12)
			{
				printf("\r\n your inputs is :%d��It is invalid",temp_num);					  
			}
			else
			{	  
				printf("\n\r  The month is setting to: %d\n\r", temp_num);
				tm->tm_mon = temp_num;				
				break;
			}
	  }while(1);
		
		/*�����·ݼ����������*/
		switch(tm->tm_mon)
			{
				case 1:
				case 3:
				case 5:
				case 7:
				case 8:
				case 10:
				case 12:					
						day_max = 31;
					break;
				
				case 4:
				case 6:
				case 9:
				case 11:
						day_max = 30;
					break;
				
				case 2:					
				     /*��������*/
						if((tm->tm_year%4==0) &&
							 ((tm->tm_year%100!=0) || (tm->tm_year%400==0)) &&
							 (tm->tm_mon>2)) 
								{
									day_max = 29;
								} else 
								{
									day_max = 28;
								}
					break;			
			}
	
		do 
	  {				
		  printf("\r\n  (Please Set date),range[1~%d]��please add enter after input:",day_max);
			scanf("%d",&temp_num);
			
			if(temp_num <1 || temp_num >day_max)
			{
				printf("\r\n  your inputs is :%d��It is invalid",temp_num);
			}
			else
			{
				printf("\n\r  The date is setting to: %d\n\r", temp_num);
				tm->tm_mday = temp_num;				
				break;
			}
	  }while(1);
		
		do 
	  {				
			printf("\r\n  (Please Set Hours),range[0~23]��please add enter after input:");
			scanf("%d",&temp_num);
			
			if( temp_num >23)
			{
				printf("\r\n your inputs is:%d��It is invalid",temp_num);
			}
			else
			{
				printf("\n\r  The hour is setting to: %d\n\r", temp_num);
				tm->tm_hour = temp_num;
				break;
			}
	  }while(1);

		do 
	  {				
			printf("\r\n Please Set Minutes,range[0~59]��please add enter after input:");
			scanf("%d",&temp_num);
			
			if( temp_num >59)
			{
				printf("\r\n your inputs is:%d��It is invalid",temp_num);
			}
			else
			{
				printf("\n\r   The minutes is setting to: %d\n\r", temp_num);
				tm->tm_min = temp_num;
				break;
			}
	  }while(1);

		do 
	  {				
			printf("\r\n  Please Set Seconds,range[0~59]��please add enter after input:");
			scanf("%d",&temp_num);
			if( temp_num >59)
			{
				printf("\r\n your inputs is:%d��It is invalid",temp_num);
			}
			else
			{
				printf("\n\r  The seconds is setting to: %d\n\r", temp_num);
				tm->tm_sec = temp_num;
				break;
			}
	  }while(1);
}

/*
 * ��������Time_Show
 * ����  ����ʾ��ǰʱ��ֵ
 * ����  ����
 * ���  ����
 * ����  ���ⲿ����
 */ 
void Time_Show(struct rtc_time *tm)
{	 
	  /* Infinite loop */
	  while (1)
	  {
	    /* ÿ��1s */
	    if (TimeDisplay == 1)
	    {
				/* Display current time */
	      Time_Display( RTC_GetCounter(),tm); 		  
	      TimeDisplay = 0;
	    }
	  }
}


/*
 * ��������Time_Adjust
 * ����  ��ʱ�����
 * ����  �����ڶ�ȡRTCʱ��Ľṹ��ָ�루����ʱ�䣩
 * ���  ����
 * ����  ���ⲿ����
 */
void Time_Adjust(struct rtc_time *tm)
{
	
			/* RTC ���� */
		RTC_Configuration();

	  /* �ȴ�ȷ����һ�β������ */
		RTC_WaitForLastTask();
		  
	  /* �������� */
		GregorianDay(tm);

	  /* �����ڼ���ʱ�����д�뵽RTC�����Ĵ��� */
		RTC_SetCounter(mktimev(tm)-TIME_ZOOM);

	  /* �ȴ�ȷ����һ�β������ */
		RTC_WaitForLastTask();
}
/*
 * ��������Clock_Adjust
 * ����  �����ӵ���
 * ����  �����ڶ�ȡRTCʱ��Ľṹ��ָ�루����ʱ�䣩
 * ���  ����
 * ����  ���ⲿ����
 */

void Clock_Adjust(struct rtc_time *tm)
{		
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
		RTC_Configuration();
		RTC_WaitForLastTask();
		BKP_WriteBackupRegister(BKP_DR2, tm->tm_year );
		RTC_WaitForLastTask();
		BKP_WriteBackupRegister(BKP_DR10, tm->tm_mon );
		RTC_WaitForLastTask();
		BKP_WriteBackupRegister(BKP_DR4, tm->tm_mday );
		RTC_WaitForLastTask();
		BKP_WriteBackupRegister(BKP_DR5, tm->tm_hour );
		RTC_WaitForLastTask();
		BKP_WriteBackupRegister(BKP_DR6, tm->tm_min );
		RTC_WaitForLastTask();
		BKP_WriteBackupRegister(BKP_DR7, tm->tm_sec );
		RTC_WaitForLastTask();
		RTC_SetAlarm(mktimev(tm)-TIME_ZOOM);
		RTC_WaitForLastTask();
		
	

}
 
/*
 * ��������Time_Display
 * ����  ����ʾ��ǰʱ��ֵ
 * ����  ��-TimeVar RTC����ֵ����λΪ s
 * ���  ����
 * ����  ���ڲ�����
 */	
void Time_Display(uint32_t TimeVar,struct rtc_time *tm)
{
		static uint32_t FirstDisplay = 1;
		uint32_t BJ_TimeVar;
		uint8_t size=16; 
		uint8_t str[200]; // �ַ����ݴ�  	

	   /*  �ѱ�׼ʱ��ת��Ϊ����ʱ��*/
	   BJ_TimeVar =TimeVar + TIME_ZOOM;

	   to_tm(BJ_TimeVar, tm);/*�Ѷ�ʱ����ֵת��Ϊ����ʱ��*/	
	
	  printf(" UNIX TimeVar = %d \n The Time is: %d/%d/%d/ %0.2d:%0.2d:%0.2d\r ",TimeVar,
	                    tm->tm_year, tm->tm_mon, tm->tm_mday, 
	                    tm->tm_hour, 
	                    tm->tm_min, tm->tm_sec);
		OLED_ShowNum(48,0,tm->tm_year,4,16);
		OLED_ShowNum(86,0,tm->tm_mon,2,16);
		OLED_ShowNum(108,0,tm->tm_mday,2,16);
		OLED_ShowNum(0,16,tm->tm_hour,2,size);
		OLED_ShowString(16,16,":",16);
		OLED_ShowNum(32,16,tm->tm_min,2,size);
		OLED_ShowString(48,16,":",16);
		OLED_ShowNum(64,16,tm->tm_sec,2,size);	
		OLED_Refresh();	
}

void Clock_Display(uint32_t TimeVar,struct rtc_time *tm)
{
		
		uint32_t BJ_TimeVar;
		uint8_t size=16; 
		uint8_t str[200]; // �ַ����ݴ�  	

	   /*  �ѱ�׼ʱ��ת��Ϊ����ʱ��*/
		BJ_TimeVar =TimeVar + TIME_ZOOM;

		to_tm(BJ_TimeVar, tm);/*�Ѷ�ʱ����ֵת��Ϊ����ʱ��*/	 	  	

	  /* ���ʱ���������ʱ�� */
	  printf("The ClockTime is: %d/%d/%d/ %0.2d:%0.2d:%0.2d\r \n",
	                    tm->tm_year, tm->tm_mon, tm->tm_mday, 
	                    tm->tm_hour, 
	                    tm->tm_min, tm->tm_sec);
	
		OLED_ShowNum(0,48,tm->tm_hour,2,size);
		OLED_ShowString(16,48,":",16);
		OLED_ShowNum(32,48,tm->tm_min,2,size);
		OLED_ShowString(48,48,":",16);
		OLED_ShowNum(64,48,tm->tm_sec,2,size);	
		OLED_Refresh();		
}




