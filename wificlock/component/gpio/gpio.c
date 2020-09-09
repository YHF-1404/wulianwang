#include "gpio.h"

void MX_GPIO_Init(void)
{

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOD);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);

  LL_GPIO_ResetOutputPin(GPIOA, LCD_CLK_Pin|LCD_LAT_Pin|LCD_G2_Pin|LCD_G1_Pin 
                          |LCD_D_Pin|LCD_C_Pin|LCD_B_Pin);

  LL_GPIO_ResetOutputPin(GPIOB, LCD_A_Pin|LCD_OE_Pin|LCD_R1_Pin|LCD_R2_Pin);

  GPIO_InitStruct.Pin = LCD_CLK_Pin|LCD_LAT_Pin|LCD_G2_Pin|LCD_G1_Pin 
                          |LCD_D_Pin|LCD_C_Pin|LCD_B_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LCD_A_Pin|LCD_OE_Pin|LCD_R1_Pin|LCD_R2_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}
