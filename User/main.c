#include "stm32f10x.h"
#include "./usart/bsp_usart.h"
#include "bsp_rtc.h"
#include "./key/bsp_key.h"  
#include "./beep/bsp_beep.h"  
#include "oled.h"
#include "bmp.h"
#include "max30102.h" 
#include "myiic.h"
#include "algorithm.h"
#include "ds18b20.h"
// N = 2^32/365/24/60/60 = 136 年

struct rtc_time systmtime=
{
0,0,0,18,3,2023,0
};
struct rtc_time clocktime=
{
0,0,0,18,3,2023,0
};


extern __IO uint32_t TimeDisplay ;
extern __IO uint32_t TimeAlarm ;
unsigned int timeget=0;
unsigned int mod=1;
unsigned int chose=0;
unsigned int k=0;
unsigned int clear=1;
unsigned int bluetooth=0;
uint32_t aun_ir_buffer[500]; //IR LED sensor data
int32_t n_ir_buffer_length;    //data length
uint32_t aun_red_buffer[500];    //Red LED sensor data
int32_t n_sp02; //SPO2 value
int8_t ch_spo2_valid;   //indicator to show if the SP02 calculation is valid
int32_t n_heart_rate;   //heart rate value
int8_t  ch_hr_valid;    //indicator to show if the heart rate calculation is valid
uint8_t uch_dummy;

#define MAX_BRIGHTNESS 255

void OLED_Opening(void)
{
	int t=100,i,x0=32,y0=0,x1=96,y1=8;
	OLED_Init();
	OLED_ColorTurn(0);//0正常显示，1 反色显示
	OLED_DisplayTurn(0);//0正常显示 1 屏幕翻转显示
	OLED_Refresh();
	for(i=0;i<2;i++)
		{
			OLED_ShowPicture(x0,y0,x1,y1,BMP1);
			SysTick_Delay_Ms( t );
			OLED_ShowPicture(x0,y0,x1,y1,BMP2);
			SysTick_Delay_Ms( t );
			OLED_ShowPicture(x0,y0,x1,y1,BMP3);
			SysTick_Delay_Ms( t );
			OLED_ShowPicture(x0,y0,x1,y1,BMP4);
			SysTick_Delay_Ms( t );
			OLED_ShowPicture(x0,y0,x1,y1,BMP5);
			SysTick_Delay_Ms( t );
			OLED_ShowPicture(x0,y0,x1,y1,BMP6);
			SysTick_Delay_Ms( t );
			OLED_ShowPicture(x0,y0,x1,y1,BMP7);
			SysTick_Delay_Ms( t );
			OLED_ShowPicture(x0,y0,x1,y1,BMP8);
			SysTick_Delay_Ms( t );
			OLED_ShowPicture(x0,y0,x1,y1,BMP9);
			SysTick_Delay_Ms( t );
		}
		OLED_Refresh();
}
//默认LSE时钟
int main()
{		

	uint32_t clock_timestamp;
	uint32_t current_timestamp;
	uint32_t un_min, un_max, un_prev_data;  
	int i;
	int32_t n_brightness;
	float f_temp;
	u8 temp_num=0;
	u8 temp[6];
	u8 str[100];
	u8 dis_hr=0,dis_spo2=0;

		USART_Config();			
		EXTI_Key_Config();
		BEEP_GPIO_Config();
		RTC_NVIC_Config();
		RTC_CheckAndConfig(&systmtime);
		RTC_WaitForLastTask();
		OLED_Opening();
		OLED_ColorTurn(0);//0正常显示，1 反色显示
		OLED_DisplayTurn(0);//0正常显示 1 屏幕翻转显示
		OLED_ShowString(20,20," Time mod",20);
		OLED_Clear();
	  while (1)
	  {
		 if(bluetooth==1)
		 {
			 unsigned char getmod;
			 if( clear == 1)
			   {
				OLED_Clear();
				OLED_Refresh();
				clear=0;
			   }			 
			OLED_ShowString(0,16,"Bluetooth",16);
			OLED_ShowString(0,32,"setting",16);
			OLED_Refresh();
			getmod = getchar();
			if(getmod == '1')
			 {
				 mod = 1;
				 clear = 1;
			 }
			if(getmod == '2')
			 {
				 mod = 2;
				 clear = 1;
			 }
			 k=0;
			 chose = 0;
			 bluetooth = 0;
			
		 }
		 if( TimeAlarm == 1)
			{
				BEEP(ON);
			}
		   if(mod == 1 && chose ==0 && clear == 1 && bluetooth==0)
		   {
				OLED_Clear();
				OLED_Refresh();
				clear=0;
				k=0;
		   }
		  if(mod == 1 && chose ==0 && clear == 0)
		  {
			  		OLED_ShowString(0,0,"Time:",16);
					OLED_ShowString(0,32,"Clock:",16);
					OLED_ShowString(96,112,"DDK",16);
					if (TimeDisplay == 1)/* 每过1s 更新一次时间*/
					{	
						current_timestamp = RTC_GetCounter();
						Time_Display( current_timestamp,&systmtime); 
						clock_timestamp = RTC_GetAlarm();
						Clock_Display( clock_timestamp,&clocktime);
						
						TimeDisplay = 0;
						
					}
						if( timeget==1 )
						{
							struct rtc_time set_time;
							OLED_Clear();
							OLED_ShowString(0,20,"Setting Time...",16);
							OLED_Refresh();
							Time_Regulate_Get(&set_time);
							Time_Adjust(&set_time);
							BKP_WriteBackupRegister(BKP_DR1, RTC_BKP_DATA1);
							OLED_Clear();
							OLED_ShowString(20,20,"Finished!",16);
							OLED_Refresh();
							SysTick_Delay_Ms(500);
							OLED_Clear();
							OLED_ShowString(0,0,"Time:",16);
							OLED_ShowString(0,32,"Clock:",16);
							OLED_ShowString(96,112,"DDK",16);
							OLED_Refresh();
							timeget=0;
							k=0;
							
						} 
						if( timeget==2  )
						{   
							struct rtc_time set_clock;
							OLED_Clear();
							OLED_ShowString(0,20,"Setting Clock..",16);
							OLED_Refresh();
							Time_Regulate_Get(&set_clock);
							Clock_Adjust(&set_clock);
							OLED_Clear();
							OLED_ShowString(20,20,"Finished!",16);
							OLED_Refresh();
							SysTick_Delay_Ms(500);
							OLED_Clear();
							OLED_ShowString(0,0,"Time:",16);
							OLED_ShowString(0,32,"Clock:",16);
							OLED_ShowString(96,112,"DDK",16);
							OLED_Refresh();
							timeget=0;
							k=0;
				
						} 
			}
			if(mod == 2 && chose == 0 && clear == 1&& bluetooth==0)
			{
				delay_init();
				DS18B20_Init();				
				max30102_init();
				un_min=0x3FFFF;
				un_max=0;
				n_ir_buffer_length=500; //100的缓冲长度存储5秒的样本运行在100sps
										//读取前500个样本，并确定信号范围
				for(i=0;i<n_ir_buffer_length;i++)
				{
					while(MAX30102_INT==1);   //等待，直到中断引脚
					
					max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);
					aun_red_buffer[i] =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];    // 合并值以获得实际的数字
					aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03)<<16) |(long)temp[4]<<8 | (long)temp[5];   // 合并值以获得实际的数字
						
					if(un_min>aun_red_buffer[i])
						un_min=aun_red_buffer[i];    //update signal min
					if(un_max<aun_red_buffer[i])
						un_max=aun_red_buffer[i];    //update signal max
				}
				un_prev_data=aun_red_buffer[i];
				//计算500次样本后的心率和SpO2(前5秒样本)
				maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
				OLED_Clear();
				OLED_Refresh();
				clear=0;
				k=0;
				
			}
			if (mod == 2 && chose == 0 && clear == 0)
			{
				float temper = 0.0;
				i=0;
				un_min=0x3FFFF;
				un_max=0;
				
				//将前100组样本转储到memory中，并将后400组样本移到top中
				for(i=100;i<500;i++)
				{
					aun_red_buffer[i-100]=aun_red_buffer[i];
					aun_ir_buffer[i-100]=aun_ir_buffer[i];
					
					//update the signal min and max
					if(un_min>aun_red_buffer[i])
					un_min=aun_red_buffer[i];
					if(un_max<aun_red_buffer[i])
					un_max=aun_red_buffer[i];
				}
				//在计算心率之前取100组样本
				for(i=400;i<500;i++)
				{
					un_prev_data=aun_red_buffer[i-1];
					while(MAX30102_INT==1);
					max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);
					aun_red_buffer[i] =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];    // 合并值以获得实际的数字
					aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03)<<16) |(long)temp[4]<<8 | (long)temp[5];   // 合并值以获得实际的数字
				
					if(aun_red_buffer[i]>un_prev_data)
					{
						f_temp=aun_red_buffer[i]-un_prev_data;
						f_temp/=(un_max-un_min);
						f_temp*=MAX_BRIGHTNESS;
						n_brightness-=(int)f_temp;
						if(n_brightness<0)
							n_brightness=0;
					}
					else
					{
						f_temp=un_prev_data-aun_red_buffer[i];
						f_temp/=(un_max-un_min);
						f_temp*=MAX_BRIGHTNESS;
						n_brightness+=(int)f_temp;
						if(n_brightness>MAX_BRIGHTNESS)
						n_brightness=MAX_BRIGHTNESS;
					}
					if(ch_hr_valid == 1 && n_heart_rate<120)//**/ ch_hr_valid == 1 && ch_spo2_valid ==1 && n_heart_rate<120 && n_sp02<101
					{
						dis_hr = n_heart_rate;
						dis_spo2 = n_sp02;
					}
					else
					{
						dis_hr = 0;
						dis_spo2 = 0;
					}
				}
				maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
				
				temper=DS18B20_GetTemperture();
				OLED_ShowString(64,0,"Temp:",16);
				OLED_ShowNum(80,16,temper,2,16);
				OLED_Refresh();//更新显示到OLED
				if(dis_hr == 0 && dis_spo2 == 0)  //**dis_hr == 0 && dis_spo2 == 0
					{
						OLED_ShowString(0,0,"HR:",16);
						OLED_ShowString(0,32,"SpO2:",16);
						OLED_ShowString(16,16,"---",16);
						OLED_ShowString(16,48,"---",16);

					}
					else
					{
						OLED_ShowString(0,0,"HR:",16);
						OLED_ShowString(0,32,"SpO2:",16);
						OLED_ShowNum(16,16,dis_hr,3,16);
						OLED_ShowNum(16,48,dis_spo2,3,16);
						
					}
				if (TimeDisplay == 1)
				{
					
					if(temper<0)
					{
						printf("Temp:-");
					}
					else
					{
						printf("Temp:+");
					}
					printf("%.2fDc\r\n",temper);
					if(dis_hr == 0 && dis_spo2 == 0)  //**dis_hr == 0 && dis_spo2 == 0
					{
						printf("HR:---\n");
						printf("SpO2:---");

					}
					else
					{
						printf("HR:%3d\n",dis_hr);
						printf("SpO2:%3d",dis_spo2);
					}
					OLED_Refresh();//更新显示到OLED
					TimeDisplay=0;
				}
			}
			if ( chose == 1 &&clear == 1)
				{
					OLED_Clear();
					OLED_Refresh();
					OLED_ShowString(0,0,"1:Time",16);
					OLED_ShowString(0,16,"2:Clock",16);
					OLED_ShowString(0,32,"3:Mod exchange",16);
					OLED_ShowString(0,48,"4:Bluetooth",16);
					OLED_ShowNum(108,0,k,1,16);
					OLED_Refresh();
					clear=0;
				}			
			if (chose == 1 && clear==0 )
			{
						
					OLED_ShowNum(108,0,k,1,16);
					OLED_Refresh();
				
			}
		}
}



