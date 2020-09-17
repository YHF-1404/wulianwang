#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "rtc.h"
#include "lcd.h"
#include "cJSON.h"
#include "iwdg.h"

#include <string.h>
#include <stdlib.h>


#define   CharToNum(x)                    ((x) - '0')
#define   Char_Is_Num(x)                  ((x) >= '0' && (x) <= '9')


//#define DEBUG_GETTIME

volatile data_buf_t *uart1_data;
time_typedef *time_data;

static uint8_t status;
volatile uint8_t current_data;

DeviceMode_t cur_mode;


static void SystemClock_Config(void);
void put_to_screen(uint8_t indata, GPIO_TypeDef *GPIO_port, uint32_t GPIO_pin);
uint8_t StrToInt(char* str);
void decode_time(char *text, ZL_RTC_t *ZL_Time);
void print_syn_msg(void);



ZL_RTC_t ZL_Set_Time = 
{
  .Year = 19,
  .Month = 6,
  .Day = 29,
  .WeekDay = 6,
  .Hours = 4,
  .Minutes = 30,
  .Seconds = 30,
};


int main(void)
{
  cur_mode = DisMode;
  ZL_RTC_t NowTime;
  uint8_t Disbuf[100];

  memset(Disbuf, '\0', sizeof(Disbuf));
  uart1_data = (volatile data_buf_t*)malloc(sizeof(data_buf_t));
  time_data = (time_typedef*)malloc(sizeof(time_typedef));

  HAL_Init();
  SystemClock_Config();
  
  MX_GPIO_Init();
  MX_TIM3_Init();
  MX_IWDG_Init();
  MX_USART1_UART_Init();
  LedMatrixInit();
  HAL_TIM_Base_Start_IT(&htim3);
  ZL_RTC_Init(&ZL_Set_Time);
  HAL_UART_Receive_IT(&huart1, &current_data, 1);
  HAL_Delay(300);
  
  while (1)
  {
    HAL_IWDG_Refresh(&hiwdg);
    switch (cur_mode)
    {
      case DisMode:
      {
        ZL_RTC_GetTime(&NowTime);
        sprintf((char *)Disbuf, "%02d:%02d:%02d\0", NowTime.Hours, NowTime.Minutes, NowTime.Seconds);
        LedMatrix_ShowString(0, 0, (const uint8_t *)Disbuf, 2);
        memset(Disbuf, '\0', sizeof(Disbuf));
        
        sprintf((char *)Disbuf, " %02d/%02d %s\0", NowTime.Month, NowTime.Day, get_eng_week(NowTime.WeekDay));
        LedMatrix_ShowString(0, 18, (const uint8_t *)Disbuf, 1);
        memset(Disbuf, '\0', sizeof(Disbuf));
        lcdbuf.is_update = 1;
      }break;

      case SynMode:
      {
        HAL_TIM_Base_Stop_IT(&htim3);
        lcdbuf.is_update = 0;
        decode_time(uart1_data->buffer , &ZL_Set_Time);
        ZL_RTC_SetTime(&ZL_Set_Time);
        memset((void *)uart1_data, 0, sizeof(data_buf_t));
        HAL_Delay(300);
        HAL_TIM_Base_Start_IT(&htim3);
        cur_mode = DisMode;
      }break;
      
      default:
        break;
    }
  }
}



static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}



inline void put_to_screen(uint8_t indata, GPIO_TypeDef *GPIO_port, uint32_t GPIO_pin)
{
  if(indata & 0x01)
    LL_GPIO_SetOutputPin(GPIO_port, GPIO_pin);
  else
    LL_GPIO_ResetOutputPin(GPIO_port, GPIO_pin);
}



uint8_t StrToInt(char* str)
{
	uint32_t val = 0;
	uint32_t i = 0;
	
	while(Char_Is_Num(*(str + i)))
    {
	    val = val * 10 + CharToNum(*(str + i));
	    i++;
	}

    return val;
}

#define DEBUG_GETTIME
void decode_time(char *text, ZL_RTC_t *ZL_Time)
{
	//char *out;
  cJSON *deroot;

	deroot=cJSON_Parse(text);
	if (!deroot)
	{
		//os_printf("Error before: [%s]\n",cJSON_GetErrorPtr());
    cJSON_Delete(deroot);
	}
  else
	{
    cJSON *jsYear = cJSON_GetObjectItem(deroot, "Year");
    cJSON *jsMon = cJSON_GetObjectItem(deroot, "Mon");
    cJSON *jsDay = cJSON_GetObjectItem(deroot, "Day");
    cJSON *jsHour = cJSON_GetObjectItem(deroot, "Hour");
    cJSON *jsMin = cJSON_GetObjectItem(deroot, "Min");
    cJSON *jsSec = cJSON_GetObjectItem(deroot, "Sec");

    ZL_Time->Year = StrToInt(jsYear->valuestring) - 2000;
    ZL_Time->Month = StrToInt(jsMon->valuestring);
    ZL_Time->Day = StrToInt(jsDay->valuestring);
    ZL_Time->Hours = StrToInt(jsHour->valuestring);
    ZL_Time->Minutes = StrToInt(jsMin->valuestring);
    ZL_Time->Seconds = StrToInt(jsSec->valuestring);

    #ifdef DEBUG_GETTIME
      char debug_timebuf[50];
      char debug_databuf[50];

      sprintf(debug_timebuf, "%02d:%02d:%02d", ZL_Time->Hours, ZL_Time->Minutes, ZL_Time->Seconds);
      LedMatrix_ShowString(0, 0, (const uint8_t *)debug_timebuf, 2);

      sprintf(debug_databuf, "%04d:%02d:%02d", ZL_Time->Year, ZL_Time->Month, ZL_Time->Day);
      LedMatrix_ShowString(0, 15, (const uint8_t *)debug_databuf, 1);
    #endif

		cJSON_Delete(deroot);
	}
}


void print_syn_msg(void)
{
  uint8_t Disbuf[20];
  sprintf((char *)Disbuf, "Syncing\0");
  LedMatrix_ShowString(0, 0, (const uint8_t *)Disbuf, 1);
  memset(Disbuf, '\0', sizeof(Disbuf));
  
  sprintf((char *)Disbuf, "......\0");
  LedMatrix_ShowString(0, 20, (const uint8_t *)Disbuf, 1);
  memset(Disbuf, '\0', sizeof(Disbuf));
}



HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim == &htim3)
  {
    if(lcdbuf.is_update)
    {
      uint8_t outbuffer[4];
	    static uint8_t row = 0;
		  if(row>15) row = 0;

		  for(uint8_t col = 0; col < 8; ++col)
		  {
        LL_GPIO_SetOutputPin(LCD_OE_GPIO_Port, LCD_OE_Pin);
		  	outbuffer[0] = lcdbuf.LED_Matrix_R_Buf[col][row];
		  	outbuffer[1] = lcdbuf.LED_Matrix_R_Buf[col][row+16];
		  	outbuffer[2] = lcdbuf.LED_Matrix_G_Buf[col][row];
		  	outbuffer[3] = lcdbuf.LED_Matrix_G_Buf[col][row+16];			
		  	for(uint8_t i = 0 ; i < 8 ; ++i)
		  	{
          LL_GPIO_ResetOutputPin(LCD_CLK_GPIO_Port, LCD_CLK_Pin);
          put_to_screen(outbuffer[0], LCD_R1_GPIO_Port, LCD_R1_Pin);
          put_to_screen(outbuffer[1], LCD_R2_GPIO_Port, LCD_R2_Pin);
          put_to_screen(outbuffer[2], LCD_G1_GPIO_Port, LCD_G1_Pin);
          put_to_screen(outbuffer[3], LCD_G2_GPIO_Port, LCD_G2_Pin);
          LL_GPIO_SetOutputPin(LCD_CLK_GPIO_Port, LCD_CLK_Pin);
  
          for(uint8_t i = 0; i < 4; ++i)
            outbuffer[i] >>= 1;
		  	}			
        LL_GPIO_SetOutputPin(LCD_LAT_GPIO_Port, LCD_LAT_Pin);
        LL_GPIO_ResetOutputPin(LCD_LAT_GPIO_Port, LCD_LAT_Pin);
		  }
      LL_GPIO_ResetOutputPin(LCD_OE_GPIO_Port, LCD_OE_Pin);
		  scanf_line(row);
		  row++;
      //lcdbuf.is_update = 0;
    }
  }
}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART1)
  {
    uart1_data->buffer[uart1_data->len++] = current_data;
    switch(status)                 //set the tail 0x0A 0x0D  
		{
			case 0:
			{
				if(current_data == 0x0A)
				  	status = 1;
			}break;
			case 1:
			{
				status = 0;
				if(current_data == 0x0D)
				{
					uart1_data->buffer[uart1_data->len + 1] = '\0';
					uart1_data->is_rec = 1;
          cur_mode = SynMode;
				}
			}break;
		}
    HAL_UART_Receive_IT(&huart1, &current_data, 1);
  }
}


// void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
// {
//   ZL_RTC_t ZL_Time;
//   uint8_t datebuf[100];
//   ZL_RTC_GetTime(&ZL_Time);

// 	if(hrtc == &hRTC)
// 	{
//     sprintf((char *)datebuf, "%02d:%02d:%02d\0", ZL_Time.Hours, ZL_Time.Minutes, ZL_Time.Seconds);
//     LedMatrix_ShowString(0, 0, (const uint8_t *)datebuf, 2);
// 	}
// }



void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */


