#ifndef __MAIN_H
#define __MAIN_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"



#define BUFFER_SIZE 100

typedef struct {
  uint8_t is_rec;
  uint8_t len;
  uint8_t buffer[BUFFER_SIZE];
} data_buf_t;

typedef struct {
   char time[16];
   char day[16];
   char month[16];
   char week[16];
} time_typedef;


typedef enum
{
  DisMode = 0,
  SynMode
}DeviceMode_t;



void Error_Handler(void);

#ifdef __cplusplus
}
#endif
#endif 