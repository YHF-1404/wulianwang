
#ifndef __gpio_H
#define __gpio_H
#ifdef __cplusplus
 extern "C" {
#endif


#include "main.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_cortex.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_pwr.h"
#include "stm32f1xx_ll_dma.h"

#define LCD_CLK_Pin LL_GPIO_PIN_1
#define LCD_CLK_GPIO_Port GPIOA
#define LCD_LAT_Pin LL_GPIO_PIN_2
#define LCD_LAT_GPIO_Port GPIOA
#define LCD_G2_Pin LL_GPIO_PIN_3
#define LCD_G2_GPIO_Port GPIOA
#define LCD_G1_Pin LL_GPIO_PIN_4
#define LCD_G1_GPIO_Port GPIOA
#define LCD_D_Pin LL_GPIO_PIN_5
#define LCD_D_GPIO_Port GPIOA
#define LCD_C_Pin LL_GPIO_PIN_6
#define LCD_C_GPIO_Port GPIOA
#define LCD_B_Pin LL_GPIO_PIN_7
#define LCD_B_GPIO_Port GPIOA
#define LCD_A_Pin LL_GPIO_PIN_0
#define LCD_A_GPIO_Port GPIOB
#define LCD_OE_Pin LL_GPIO_PIN_1
#define LCD_OE_GPIO_Port GPIOB
#define LCD_R1_Pin LL_GPIO_PIN_10
#define LCD_R1_GPIO_Port GPIOB
#define LCD_R2_Pin LL_GPIO_PIN_11
#define LCD_R2_GPIO_Port GPIOB


void MX_GPIO_Init(void);

#ifdef __cplusplus
}
#endif
#endif 