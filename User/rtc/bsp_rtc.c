/**
  ******************************************************************************
  * @file    bsp_rtc.c
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   stm32 RTC 驱动
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 F103-指南者 STM32 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
#include "./usart/bsp_usart.h"
#include "./rtc/bsp_rtc.h"
#include "oled.h"



/* 秒中断标志，进入秒中断时置1，当时间被刷新之后清0 */
__IO uint32_t TimeDisplay = 0;

/*闹钟响铃标志，在中断中闹钟事件致1*/
__IO uint32_t TimeAlarm = 0;

/*星期，生肖用文字ASCII码*/
char const *WEEK_STR[] = {"日", "一", "二", "三", "四", "五", "六"};
char const *zodiac_sign[] = {"猪", "鼠", "牛", "虎", "兔", "龙", "蛇", "马", "羊", "猴", "鸡", "狗"};

/*英文，星期，生肖用文字ASCII码*/
char const *en_WEEK_STR[] = { "Sunday","Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char const *en_zodiac_sign[] = {"Pig", "Rat", "Ox", "Tiger", "Rabbit", "Dragon", "Snake", "Horse", "Goat", "Monkey", "Rooster", "Dog"};


/*
 * 函数名：NVIC_Configuration
 * 描述  ：配置RTC秒中断的主中断优先级为1，次优先级为0
 * 输入  ：无
 * 输出  ：无
 * 调用  ：外部调用
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
 * 函数名：RTC_CheckAndConfig
 * 描述  ：检查并配置RTC
 * 输入  ：用于读取RTC时间的结构体指针
 * 输出  ：无
 * 调用  ：外部调用
 */
void RTC_CheckAndConfig(struct rtc_time *tm)
{

	struct rtc_time configtime;
   	/*在启动时检查备份寄存器BKP_DR1，如果内容不是0xA5A5,
	  则需重新配置时间并询问用户调整时间*/
	if (BKP_ReadBackupRegister( BKP_DR1 ) != RTC_BKP_DATA1)
	{
		printf("\r\n\r\n RTC not yet configured....");
		printf("\r\n\r\n RTC configured....");

		/* 使用tm的时间配置RTC寄存器 */
		Time_Adjust(tm);
		/*向BKP_DR1寄存器写入标志，说明RTC已在运行*/
		BKP_WriteBackupRegister(BKP_DR1 , RTC_BKP_DATA1);
	}
	else
	{
		
		/* 使能 PWR 和 Backup 时钟 */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
		/* 允许访问 Backup 区域 */
		PWR_BackupAccessCmd(ENABLE);
	
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
		{
		    printf("\r\n\r\n Power On Reset occurred....");
		}
		/*检查是否Reset复位*/
		if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
		{
			printf("\r\n\r\n External Reset occurred....");
		}
		if (RCC_GetFlagStatus(RCC_FLAG_SFTRST) != RESET)
		{
			printf("\r\n\r\n Software reset occurred....");
		}
	
		printf("\r\n No need to configure RTC....");
		
		/*等待寄存器同步*/
		RTC_WaitForSynchro();
		RTC_WaitForLastTask();
		/*允许RTC秒中断*/
		RTC_ITConfig(RTC_IT_SEC|RTC_IT_ALR, ENABLE);
		/*等待上次RTC寄存器写操作完成*/
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
 * 函数名：RTC_Configuration
 * 描述  ：配置RTC
 * 输入  ：无
 * 输出  ：无
 * 调用  ：外部调用
 */
void RTC_Configuration(void)
{
	/* 使能 PWR 和 Backup 时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	
	/* 允许访问 Backup 区域 */
	PWR_BackupAccessCmd(ENABLE);
	
//	/* 复位 Backup 区域 */
//	BKP_DeInit();

#ifdef 	RTC_CLOCK_SOURCE_LSE
	/* 使能 LSE */
	RCC_LSEConfig(RCC_LSE_ON);
	
	/* 等待 LSE 准备好 */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{}
	
	/* 选择 LSE 作为 RTC 时钟源 */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	
	/* 使能 RTC 时钟 */
	RCC_RTCCLKCmd(ENABLE);
	
	/* 等待 RTC 寄存器 同步
	 * 因为RTC时钟是低速的，内环时钟是高速的，所以要同步
	 */
	RTC_WaitForSynchro();
	
	/* 确保上一次 RTC 的操作完成 */
	RTC_WaitForLastTask();
	
	/* 使能 RTC 秒中断 与 闹钟中断*/
	RTC_ITConfig(RTC_IT_SEC|RTC_IT_ALR, ENABLE);
	
	/* 确保上一次 RTC 的操作完成 */
	RTC_WaitForLastTask();
	
	/* 设置 RTC 分频: 使 RTC 周期为1s  */
	/* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) = 1HZ */
	RTC_SetPrescaler(32767); 
	
	/* 确保上一次 RTC 的操作完成 */
	RTC_WaitForLastTask();
	
#else

	/* 使能 LSI */
	RCC_LSICmd(ENABLE);

	/* 等待 LSI 准备好 */
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	{}
	
	/* 选择 LSI 作为 RTC 时钟源 */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	
	/* 使能 RTC 时钟 */
	RCC_RTCCLKCmd(ENABLE);
	
	/* 等待 RTC 寄存器 同步
	 * 因为RTC时钟是低速的，内环时钟是高速的，所以要同步
	 */
	RTC_WaitForSynchro();
	
	/* 确保上一次 RTC 的操作完成 */
	RTC_WaitForLastTask();
	
	/* 使能 RTC 秒中断 */
	RTC_ITConfig(RTC_IT_SEC|RTC_IT_ALR, ENABLE);
	
	/* 确保上一次 RTC 的操作完成 */
	RTC_WaitForLastTask();
	
	/* 设置 RTC 分频: 使 RTC 周期为1s ,LSI约为40KHz */
	/* RTC period = RTCCLK/RTC_PR = (40 KHz)/(40000-1+1) = 1HZ */	
	RTC_SetPrescaler(40000-1); 
	
	/* 确保上一次 RTC 的操作完成 */
	RTC_WaitForLastTask();
#endif
	
}



/*
 * 函数名：Time_Regulate_Get
 * 描述  ：保存用户使用串口设置的时间，
 *         以便后面转化成时间戳存储到RTC 计数寄存器中。
 * 输入  ：用于读取RTC时间的结构体指针
 * 注意  ：在串口调试助手输入时，输入完数字要加回车
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
				printf("\r\n your inputs is :%d，It is invalid",temp_num);					  
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
				printf("\r\n your inputs is :%d，It is invalid",temp_num);					  
			}
			else
			{	  
				printf("\n\r  The month is setting to: %d\n\r", temp_num);
				tm->tm_mon = temp_num;				
				break;
			}
	  }while(1);
		
		/*根据月份计算最大日期*/
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
				     /*计算闰年*/
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
		  printf("\r\n  (Please Set date),range[1~%d]，please add enter after input:",day_max);
			scanf("%d",&temp_num);
			
			if(temp_num <1 || temp_num >day_max)
			{
				printf("\r\n  your inputs is :%d，It is invalid",temp_num);
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
			printf("\r\n  (Please Set Hours),range[0~23]，please add enter after input:");
			scanf("%d",&temp_num);
			
			if( temp_num >23)
			{
				printf("\r\n your inputs is:%d，It is invalid",temp_num);
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
			printf("\r\n Please Set Minutes,range[0~59]，please add enter after input:");
			scanf("%d",&temp_num);
			
			if( temp_num >59)
			{
				printf("\r\n your inputs is:%d，It is invalid",temp_num);
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
			printf("\r\n  Please Set Seconds,range[0~59]，please add enter after input:");
			scanf("%d",&temp_num);
			if( temp_num >59)
			{
				printf("\r\n your inputs is:%d，It is invalid",temp_num);
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
 * 函数名：Time_Show
 * 描述  ：显示当前时间值
 * 输入  ：无
 * 输出  ：无
 * 调用  ：外部调用
 */ 
void Time_Show(struct rtc_time *tm)
{	 
	  /* Infinite loop */
	  while (1)
	  {
	    /* 每过1s */
	    if (TimeDisplay == 1)
	    {
				/* Display current time */
	      Time_Display( RTC_GetCounter(),tm); 		  
	      TimeDisplay = 0;
	    }
	  }
}


/*
 * 函数名：Time_Adjust
 * 描述  ：时间调节
 * 输入  ：用于读取RTC时间的结构体指针（北京时间）
 * 输出  ：无
 * 调用  ：外部调用
 */
void Time_Adjust(struct rtc_time *tm)
{
	
			/* RTC 配置 */
		RTC_Configuration();

	  /* 等待确保上一次操作完成 */
		RTC_WaitForLastTask();
		  
	  /* 计算星期 */
		GregorianDay(tm);

	  /* 由日期计算时间戳并写入到RTC计数寄存器 */
		RTC_SetCounter(mktimev(tm)-TIME_ZOOM);

	  /* 等待确保上一次操作完成 */
		RTC_WaitForLastTask();
}
/*
 * 函数名：Clock_Adjust
 * 描述  ：闹钟调节
 * 输入  ：用于读取RTC时间的结构体指针（北京时间）
 * 输出  ：无
 * 调用  ：外部调用
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
 * 函数名：Time_Display
 * 描述  ：显示当前时间值
 * 输入  ：-TimeVar RTC计数值，单位为 s
 * 输出  ：无
 * 调用  ：内部调用
 */	
void Time_Display(uint32_t TimeVar,struct rtc_time *tm)
{
		static uint32_t FirstDisplay = 1;
		uint32_t BJ_TimeVar;
		uint8_t size=16; 
		uint8_t str[200]; // 字符串暂存  	

	   /*  把标准时间转换为北京时间*/
	   BJ_TimeVar =TimeVar + TIME_ZOOM;

	   to_tm(BJ_TimeVar, tm);/*把定时器的值转换为北京时间*/	
	
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
		uint8_t str[200]; // 字符串暂存  	

	   /*  把标准时间转换为北京时间*/
		BJ_TimeVar =TimeVar + TIME_ZOOM;

		to_tm(BJ_TimeVar, tm);/*把定时器的值转换为北京时间*/	 	  	

	  /* 输出时间戳，公历时间 */
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




