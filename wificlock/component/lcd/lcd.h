#ifndef __LED_H
#define __LED_H

#include "gpio.h"


#define LED_MATRIX_MAX_WIDTH   64
#define LED_MATRIX_MAX_HEIGHT  32


typedef struct
{
	volatile uint8_t LED_Matrix_R_Buf[LED_MATRIX_MAX_WIDTH/8][LED_MATRIX_MAX_HEIGHT];
	volatile uint8_t LED_Matrix_G_Buf[LED_MATRIX_MAX_WIDTH/8][LED_MATRIX_MAX_HEIGHT];
	volatile uint8_t is_update;
}lcd_data_t;
extern lcd_data_t lcdbuf;


extern uint8_t  POINT_COLOR;
extern uint8_t  BACK_COLOR;

// extern  volatile uint8_t LED_Matrix_R_Buf[LED_MATRIX_MAX_WIDTH/8][LED_MATRIX_MAX_HEIGHT];
// extern  volatile uint8_t LED_Matrix_G_Buf[LED_MATRIX_MAX_WIDTH/8][LED_MATRIX_MAX_HEIGHT];


void LedMatrixInit(void);
//void LedSetLight(uint16_t val);
void LedMatrix_Clear(void);
void scanf_line(uint8_t line);
void LedMatrix_Fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dot);
void LedMatrix_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size);
void LedMatrix_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);
void LedMatrix_ShowString(uint8_t x, uint8_t y, const uint8_t *p, uint8_t size);
void LedMatrix_RollStringExce(void);
uint8_t LedMatrix_RollStart(uint8_t x, uint8_t y, uint8_t length, char *p, uint8_t size);
void LedMatrix_DrawBMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t BMP[]);
void LedMatrix_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LedMatrix_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LedMatrix_Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r);


#endif























