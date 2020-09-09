#include "lcd.h"
#include "lcdfont.h"
#include "tim.h"
#include "gpio.h"


#define ROLL_STRING_MAX  200
struct rollstring_t {
	char string[ROLL_STRING_MAX];
	uint8_t size;
	uint16_t length;
	uint8_t width;
	uint16_t offset;
	uint8_t x;
	uint8_t y;
};
struct rollstring_t rollctrl;



uint8_t POINT_COLOR = 0x02;
uint8_t BACK_COLOR  = 0x00; 

_lcd_dev lcddev;


const uint16_t sw[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};


volatile uint8_t LED_Matrix_R_Buf[LED_MATRIX_MAX_WIDTH/8][LED_MATRIX_MAX_HEIGHT];
volatile uint8_t LED_Matrix_G_Buf[LED_MATRIX_MAX_WIDTH/8][LED_MATRIX_MAX_HEIGHT];



void LedMatrixInit(void)
{
	LL_GPIO_ResetOutputPin(LCD_OE_GPIO_Port, LCD_OE_Pin);
	POINT_COLOR = 0x01;
    BACK_COLOR = 0x00;
}



void LedMatrix_Clear(void)
{
	uint8_t i,j;
	for(i=0;i<LED_MATRIX_MAX_HEIGHT;i++)
	{
		for(j=0;j<LED_MATRIX_MAX_WIDTH/8;j++)
		{
			LED_Matrix_R_Buf[j][i] = 0;
			LED_Matrix_G_Buf[j][i] = 0;
		}
	}
}



void LedMatrix_DrawPoint(uint8_t x,uint8_t y,uint8_t t)
{
	uint8_t pos,bx;
	
	if(x>64||y>32)return;
	pos=x/8;
	bx=x%8;
	
	if(t == 0x01)
	{
		LED_Matrix_R_Buf[pos][y] |= (1<<bx);
		LED_Matrix_G_Buf[pos][y] &= ~(1<<bx);
	}
	else if(t == 0x02)
	{
		LED_Matrix_R_Buf[pos][y] &= ~(1<<bx);
		LED_Matrix_G_Buf[pos][y] |= (1<<bx);
	}
	else if(t == 0x03)
	{
		LED_Matrix_R_Buf[pos][y] |= (1<<bx);
		LED_Matrix_G_Buf[pos][y] |= (1<<bx);
	}
    else
	{
		LED_Matrix_R_Buf[pos][y] &= ~(1<<bx);
		LED_Matrix_G_Buf[pos][y] &= ~(1<<bx);
	}
}


inline void scanf_line(uint8_t line)
{
	if(line&0x01)					
		LL_GPIO_SetOutputPin(LCD_A_GPIO_Port, LCD_A_Pin);			
	else															
		LL_GPIO_ResetOutputPin(LCD_A_GPIO_Port, LCD_A_Pin);			
	if(((line&0x02)>>1))											
		LL_GPIO_SetOutputPin(LCD_B_GPIO_Port, LCD_B_Pin);			
	else															
		LL_GPIO_ResetOutputPin(LCD_B_GPIO_Port, LCD_B_Pin);			
	if(((line&0x04)>>2))											
		LL_GPIO_SetOutputPin(LCD_C_GPIO_Port, LCD_C_Pin);			
	else															
		LL_GPIO_ResetOutputPin(LCD_C_GPIO_Port, LCD_C_Pin);			
	if(((line&0x08)>>3))											
		LL_GPIO_SetOutputPin(LCD_D_GPIO_Port, LCD_D_Pin);			
	else															
		LL_GPIO_ResetOutputPin(LCD_D_GPIO_Port, LCD_D_Pin);			
}

  
void LedMatrix_Fill(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot)  
{  
	uint8_t x,y;  
	for(x=x1;x<=x2;x++)
	{
		for(y=y1;y<=y2;y++)
			LedMatrix_DrawPoint(x,y,dot);
	}
}


void LedMatrix_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size)
{    			    
	uint8_t temp,t,t1;
	uint8_t y0=y;
	uint8_t csize;
	csize=(font2size[size][0]/8+((font2size[size][0]%8)?1:0))
	*font2size[size][1];
	chr=chr-' ';
    for(t=0;t<csize;t++)
    {   
		if(size==0)temp=asc2_0806[chr][t];      
		else if(size==1)temp=asc2_1206[chr][t]; 
		else if(size==2)temp=asc2_1608[chr][t];	
		else return;							
        for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)LedMatrix_DrawPoint(x,y,POINT_COLOR);
			else LedMatrix_DrawPoint(x,y,BACK_COLOR);
			temp<<=1;
			y++;
			if((y-y0)==font2size[size][0])
			{
				y=y0;
				x++;
				break;
			}
		}  	 
    }
}


void LedMatrix_ShowString(uint8_t x,uint8_t y,const uint8_t *p,uint8_t size)
{
    while((*p<='~')&&(*p>=' '))
    {
        //if(x>(128-(size/2))){x=0;y+=size;}
        //if(y>(64-size)){y=x=0;OLED_Clear();}
        LedMatrix_ShowChar(x,y,*p,size);
		x+=font2size[size][1];
        p++;
    }
}


uint32_t mypow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}	


void LedMatrix_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;
	uint8_t offset=font2size[size][1];
	
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LedMatrix_ShowChar(x+(offset*t),y,'0',size);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LedMatrix_ShowChar(x+(offset*t),y,temp+'0',size); 
		//x+=offset;
	}
}





uint8_t LedMatrix_RollStart(uint8_t x,uint8_t y,uint8_t width,char *p,uint8_t size)
{
	uint8_t i=0;
	
	rollctrl.width = width;
	rollctrl.size = size;
	rollctrl.x = x;
	rollctrl.y = y;
	while((*p<='~')&&(*p>=' '))
	{
		rollctrl.string[i++] = *p;
		p++;
		if(i>ROLL_STRING_MAX) return 1;
	}
	rollctrl.length = i*font2size[rollctrl.size][1];
	rollctrl.offset = 0;
	return 0;
}



void LedMatrix_RollStringExce(void)
{
	uint8_t x=rollctrl.x;
	uint8_t i;
	uint8_t width = font2size[rollctrl.size][1];
	uint8_t csize=(font2size[rollctrl.size][0]/8+((font2size[rollctrl.size][0]%8)?1:0))*width;
	uint8_t chr = rollctrl.offset/(uint16_t)width;
	uint8_t s1 = rollctrl.offset%(uint16_t)width;
	uint8_t s2;
	uint8_t s3;
	
	uint8_t diachar = rollctrl.string[chr]-' ';
	uint8_t y=rollctrl.y;
	uint8_t y0=y;
	
	if(s1!=0)
	{
		chr++;
		s2 = (rollctrl.width-width+s1)/(uint16_t)width;
		s3 = rollctrl.width-width+s1-(s2*width);
		s1 = (font2size[rollctrl.size][0]/8+((font2size[rollctrl.size][0]%8)?1:0))*s1;
		for(i=s1;i<csize;i++)
		{
			uint8_t temp,t1;
			if(rollctrl.size==0)temp=asc2_0806[diachar][i];
			else if(rollctrl.size==1)temp=asc2_1206[diachar][i];
			else if(rollctrl.size==2)temp=asc2_1608[diachar][i];
			else return;
			for(t1=0;t1<8;t1++)
			{
				if(temp&0x80)LedMatrix_DrawPoint(x,y,POINT_COLOR);
				else LedMatrix_DrawPoint(x,y,BACK_COLOR);
				temp<<=1;
				y++;
				if((y-y0)==font2size[rollctrl.size][0])
				{
					y=y0;
					x++;
					break;
				}
			}
		}
	}
	else
	{
		s2 = (rollctrl.width)/(uint16_t)width;
		s3 = rollctrl.width-(s2*width);
	}
	for(i=0;i<s2;i++)
	{
		LedMatrix_ShowChar(x, rollctrl.y, *(rollctrl.string+chr), rollctrl.size);
		x+=width;
		chr++;
	}
	diachar = rollctrl.string[chr]-' ';
	s3 = (font2size[rollctrl.size][0]/8+((font2size[rollctrl.size][0]%8)?1:0))*s3;
	for(i=0;i<s3;i++)
	{
		uint8_t temp,t1;
		if(rollctrl.size==0)temp=asc2_0806[diachar][i];
		else if(rollctrl.size==1)temp=asc2_1206[diachar][i];
		else if(rollctrl.size==2)temp=asc2_1608[diachar][i];
		else return;
        for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)LedMatrix_DrawPoint(x,y,POINT_COLOR);
			else LedMatrix_DrawPoint(x,y,BACK_COLOR);
			temp<<=1;
			y++;
			if((y-y0)==font2size[rollctrl.size][0])
			{
				y=y0;
				x++;
				break;
			}
		}
	}
	
	if(++rollctrl.offset>(rollctrl.length-rollctrl.width))rollctrl.offset=0;
}




void LedMatrix_DrawBMP(uint8_t x0, uint8_t y0,uint8_t x1, uint8_t y1,const uint8_t BMP[])
{
	unsigned char x,y;
	
//	if(y1%8==0) y=y1/8;
//	else y=y1/8+1;
	for(y=y0;y<y1;y++)
	{	
		for(x=x0;x<x1;x++)
		{
			//OLED_GRAM[x][y] = BMP[j++];
		}
	}
}


void LedMatrix_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1;
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; 
	else if(delta_x==0)incx=0;
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if(delta_y==0)incy=0;
	else{incy=-1;delta_y=-delta_y;}
	if( delta_x>delta_y)distance=delta_x;
	else distance=delta_y;
	for(t=0;t<=distance+1;t++ )
	{
		LedMatrix_DrawPoint(uRow,uCol,POINT_COLOR);
		xerr+=delta_x ;
		yerr+=delta_y ;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}


void LedMatrix_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LedMatrix_DrawLine(x1,y1,x2,y1);
	LedMatrix_DrawLine(x1,y1,x1,y2);
	LedMatrix_DrawLine(x1,y2,x2,y2);
	LedMatrix_DrawLine(x2,y1,x2,y2);
}


void LedMatrix_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);   
	while(a<=b)
	{
		LedMatrix_DrawPoint(x0+a,y0-b,POINT_COLOR);
 		LedMatrix_DrawPoint(x0+b,y0-a,POINT_COLOR);           
		LedMatrix_DrawPoint(x0+b,y0+a,POINT_COLOR);               
		LedMatrix_DrawPoint(x0+a,y0+b,POINT_COLOR); 
		LedMatrix_DrawPoint(x0-a,y0+b,POINT_COLOR);       
 		LedMatrix_DrawPoint(x0-b,y0+a,POINT_COLOR);             
		LedMatrix_DrawPoint(x0-a,y0-b,POINT_COLOR);             
  		LedMatrix_DrawPoint(x0-b,y0-a,POINT_COLOR);     	         
		a++;
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		}
	}
}



