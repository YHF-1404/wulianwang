// static void SystemClock_Config(void)
// {
//   RCC_OscInitTypeDef RCC_OscInitStruct = {0};
//   RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
//   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
//   RCC_OscInitStruct.HSEState = RCC_HSE_ON;
//   RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
//   RCC_OscInitStruct.HSIState = RCC_HSI_ON;
//   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
//   RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
//   RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
//   if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
//                               |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;

//   RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
//   RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
//   RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
//   RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

//   if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
//   {
//     Error_Handler();
//   }
// }






    //LedMatrix_ShowString(0, 15, (const uint8_t *)"hahahaha", 1);
    // if(uart1_data->is_rec)
    // {
    //   if(get_data(time_data, (char *)uart1_data->bufer))
    //   {        
    //     LedMatrix_ShowString(0, 0, (const uint8_t *)time_data->time, 2);
	  //     LedMatrix_ShowString(0, 15, (const uint8_t *)time_data->month, 1);
    //     LedMatrix_ShowString(20, 15, (const uint8_t *)time_data->day, 1);
    //     LedMatrix_ShowString(35, 15, (const uint8_t *)time_data->week, 1);

    //     memset((void *)uart1_data, 0, sizeof(data_buf_t));
    //     memset(time_data, 0, sizeof(time_typedef));
    //   }
    // }



  cJSON *root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "time", "19:30");
  cJSON_AddStringToObject(root, "data", "29");
  char *string = cJSON_Print(root);
  //HAL_UART_Transmit(&huart2, (uint8_t *)string, strlen(string), 0xffff);

  //HAL_UART_Transmit(&huart2, (uint8_t *)"-----------------------------------",
                       //sizeof("-----------------------------------"), 0xffff);

  cJSON *deroot = cJSON_Parse(string);
  if(deroot != 0)
  {
    char *destring = cJSON_Print(deroot);
    //HAL_UART_Transmit(&huart2, (uint8_t *)destring, strlen(destring), 0xffff);
    cJSON *js_time = cJSON_GetObjectItem(deroot, "time");
    //HAL_UART_Transmit(&huart2, (uint8_t *)"\n", sizeof("\n"), 0xffff);
    //HAL_UART_Transmit(&huart2, (uint8_t *)js_time->valuestring, strlen(js_time->valuestring), 0xffff);
  }



    //ZL_RTC_t ZL_Time;
    //uint8_t datebuf[100];
    //ZL_RTC_GetTime(&ZL_Time);
    // sprintf((char *)datebuf, "Time:%02d:%02d:%02d  Date:%04d/%02d/%02d %d\0",
    //       ZL_Time.Hours, ZL_Time.Minutes, ZL_Time.Seconds, 2000 + ZL_Time.Year,
    //                 ZL_Time.Month, ZL_Time.Day, ZL_Time.WeekDay);

    // HAL_UART_Transmit(&huart1, datebuf, strlen((char *)datebuf), 0xffff);
    // HAL_UART_Transmit(&huart1, (uint8_t *)"\n", 1, 0xffff);




unsigned char get_data(time_typedef *out_data, char *in_data)
{
  char *PA = NULL;
  char *PB = NULL;

  PA = strstr(in_data, "Time:\"");
  if(PA != NULL)
  {
    PA += strlen("Time:\"");
    PB  = strstr(PA, "\"");
    if(PB != NULL)
    {
      memcpy(out_data->time, PA, PB - PA);
      out_data->time[PB - PA] = '\0';
      PA = strstr(PB, "Day:\"");
      if(PA != NULL)
      {
        PA += strlen("Day:\"");
        PB  = strstr(PA, "\"");
        if(PB != NULL)
        {
          memcpy(out_data->day, PA, PB - PA);
          out_data->day[PB - PA] = '\0';
          PA = strstr(PB, "Mon:\"");
          if(PA != NULL)
          {
            PA += strlen("Mon:\"");
            PB  = strstr(PA, "\"");
            if(PB != NULL)
            {
              memcpy(out_data->month, PA, PB - PA);
              out_data->month[PB - PA] = '\0';
              PA = strstr(PB, "Week:\"");
              if(PA != NULL)
              {
                PA += strlen("Week:\"");
                PB  = strstr(PA, "\"");
                if(PB != NULL)
                {
                  memcpy(out_data->week, PA, PB - PA);
                  out_data->week[PB - PA] = '\0';
                }
                else
                  return 0; 
              }
              else
                return 0;
            }
            else
              return 0;
          }
          else
            return 0;
        }
        else
          return 0;
      }
      else
        return 0;
    }
    else
      return 0;  
  }
  else
    return 0;
return 1;
}