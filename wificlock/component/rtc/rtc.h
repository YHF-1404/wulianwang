#ifndef __rtc_H
#define __rtc_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"


extern RTC_HandleTypeDef hRTC;


typedef enum {
	ZL_RTC_Result_Ok,   /*!< Everything OK */
	ZL_RTC_Result_Error /*!< An error occurred */
} ZL_RTC_Result_t;



typedef struct {
	uint8_t Seconds;     /*!< Seconds parameter, from 00 to 59 */
	uint8_t Minutes;     /*!< Minutes parameter, from 00 to 59 */
	uint8_t Hours;       /*!< Hours parameter, 24Hour mode, 00 to 23 */
	uint8_t WeekDay;     /*!< Day in a week, 1 to 7 */
	uint8_t Day;         /*!< Day in a month, 1 to 31 */
	uint8_t Month;       /*!< Month in a year, 1 to 12 */
	uint8_t Year;        /*!< Year parameter, 00 to 99, 00 is 2000 and 99 is 2099 */
	uint32_t Unix;
} ZL_RTC_t;



uint8_t ZL_RTC_Init(ZL_RTC_t *ZL_Time);
ZL_RTC_Result_t ZL_Check_RTCData(const ZL_RTC_t *ZL_Time);
void ZL_RTC_GetTime(ZL_RTC_t *ZL_Time);
void ZL_RTC_SetTime(const ZL_RTC_t *ZL_Time);
uint32_t ZL_RTC_DateToSec(const ZL_RTC_t* data);
ZL_RTC_Result_t ZL_RTC_SecToDate(ZL_RTC_t* data, uint32_t unix);
void ZL_RTC_writeCounter(RTC_HandleTypeDef* hrtc, uint32_t TimeCounter);
uint32_t ZL_RTC_ReadCounter(RTC_HandleTypeDef* hrtc);
char *get_eng_week(uint8_t week_num);

#ifdef __cplusplus
}
#endif
#endif 

