#include "rtc.h"
#include "string.h"


#define RTC_STATUS_INIT_OK              0x1234       /* RTC initialised */
#define RTC_STATUS_TIME_OK              0x4321       /* RTC time OK */
#define	RTC_STATUS_ZERO                 0x0000
#define RTC_STATUS_REG                  RTC_BKP_DR1

#define RTC_OFFSET_YEAR                 1970
#define RTC_SECONDS_PER_DAY             86400
#define RTC_SECONDS_PER_HOUR            3600
#define RTC_SECONDS_PER_MINUTE          60

#define Chine_Compensation              8*3600

#define ZL_Is_LeapYear(year)            ((((year) % 4 == 0) && ((year) % 100 != 0)) || ((year) % 400 == 0))
#define ZL_Year2Days(x)                 ZL_Is_LeapYear(x) ? 366 : 365


RTC_HandleTypeDef hRTC;
RTC_DateTypeDef RTC_DateStruct;
RTC_TimeTypeDef RTC_TimeStruct;


static uint8_t RTC_Months[2][12] = {
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},	/* Not leap year */
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}	/* Leap year */
};


uint8_t ZL_RTC_Init(ZL_RTC_t *ZL_Time)
{

	hRTC.Instance = RTC;
	hRTC.Init.AsynchPrediv = RTC_AUTO_1_SECOND;

	HAL_PWR_EnableBkUpAccess();
	__HAL_RCC_BKP_CLK_ENABLE();
	__HAL_RCC_RTC_ENABLE();

  	HAL_NVIC_SetPriority(RTC_IRQn, 0, 1);
  	HAL_NVIC_EnableIRQ(RTC_IRQn);
	  /* RTC Second IT */
  	//HAL_RTCEx_SetSecond_IT(&hRTC);
  
	uint32_t status = HAL_RTCEx_BKUPRead(&hRTC, RTC_STATUS_REG);
	if (status == RTC_STATUS_TIME_OK) 
  	{
		HAL_RTC_WaitForSynchro(&hRTC);
		__HAL_RCC_CLEAR_RESET_FLAGS();
		return 1;
	}
  	else 
  	{
  	  	HAL_RTC_Init(&hRTC);
  	  	if(HAL_RTC_Init(&hRTC) != HAL_OK)
  	  	{
  	    	Error_Handler();
  	  	}
  	  	ZL_RTC_SetTime(ZL_Time);
		HAL_RTCEx_BKUPWrite(&hRTC, RTC_BKP_DR1, RTC_STATUS_TIME_OK);
		return 0;
	}
}



void ZL_RTC_SetTime(const ZL_RTC_t *ZL_Time)
{
  ZL_RTC_writeCounter(&hRTC, (ZL_RTC_DateToSec(ZL_Time) - Chine_Compensation));
}



void ZL_RTC_GetTime(ZL_RTC_t *ZL_Time)
{
  ZL_RTC_SecToDate(ZL_Time, ZL_RTC_ReadCounter(&hRTC));
}



void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{
  if(rtcHandle->Instance==RTC)
  {
    __HAL_RCC_RTC_DISABLE();
  }
}



void ZL_RTC_writeCounter(RTC_HandleTypeDef* hrtc, uint32_t TimeCounter)
{
  TimeCounter += Chine_Compensation;
  __HAL_RTC_WRITEPROTECTION_DISABLE(hrtc);
  WRITE_REG(hrtc->Instance->CNTH, (TimeCounter >> 16U));
  WRITE_REG(hrtc->Instance->CNTL, (TimeCounter & RTC_CNTL_RTC_CNT));
  __HAL_RTC_WRITEPROTECTION_ENABLE(hrtc);
}



uint32_t ZL_RTC_ReadCounter(RTC_HandleTypeDef* hrtc)
{
  uint16_t counter;
  counter = READ_REG(hrtc->Instance->CNTL);
  return (((uint32_t)READ_REG(hrtc->Instance->CNTH) << 16) | counter);
}



/*  Date ----> Unix Time Stamp second  */
uint32_t ZL_RTC_DateToSec(const ZL_RTC_t* data)
{
	uint32_t days = 0, seconds = 0;
	uint16_t i;
	uint16_t year = (uint16_t) (data->Year + 2000);   //(data->Year + 2000);
	/* Year is below offset year */
	if (year < RTC_OFFSET_YEAR) 
  {
		return 0;
	}
	/* Days in back years */
	for (i = RTC_OFFSET_YEAR; i < year; i++) 
  {
		days += ZL_Year2Days(i);
	}
	/* Days in current year */
	for (i = 1; i < data->Month; i++) 
  {
		days += RTC_Months[ZL_Is_LeapYear(year)][i - 1];
	}
	/* Day starts with 1 */
	days += data->Day - 1;
	seconds = days * RTC_SECONDS_PER_DAY;
	seconds += data->Hours * RTC_SECONDS_PER_HOUR;
	seconds += data->Minutes * RTC_SECONDS_PER_MINUTE;
	seconds += data->Seconds;
	
	/* seconds = days * 86400; */
	return seconds;
}


/*  Unix Time Stamp second ----> Date  */
ZL_RTC_Result_t ZL_RTC_SecToDate(ZL_RTC_t* data, uint32_t unix) 
{
	uint16_t year;
	
	data->Unix = unix;
	data->Seconds = unix % 60;

	unix /= 60;
	data->Minutes = unix % 60;

	unix /= 60;
	data->Hours = unix % 24;

	unix /= 24;
	/* Get week day */
	/* Monday is day one */
	data->WeekDay = (unix + 3) % 7 + 1;

	year = 1970;
	while (1) 
  	{
		if(ZL_Is_LeapYear(year)) 
    	{
			if(unix >= 366)
				unix -= 366;
      		else
				break;
		}
    	else if(unix >= 365)
			unix -= 365;
    	else
			break;
		year++;
	}
	/* Get year in xx format */
	data->Year = (uint8_t) (year - 2000);
	/* Get month */
	for(data->Month = 0; data->Month < 12; data->Month++)
  	{
		if(ZL_Is_LeapYear(year))
    	{
			if(unix >= (uint32_t)RTC_Months[1][data->Month])
				unix -= RTC_Months[1][data->Month];
      		else
				break;
		}
    	else if(unix >= (uint32_t)RTC_Months[0][data->Month])
			unix -= RTC_Months[0][data->Month];
    	else
			break;
	}

	data->Month++;
	data->Day = unix + 1;

	return ZL_RTC_Result_Ok;
}


char *get_eng_week(uint8_t week_num)
{
	char *wday[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
	return wday[week_num-1];
}