#include "LCD.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

_lcd_param lcd_param; // Параметры
_current_font cfont;  // Текущий шрифт

// инициализирует шину управления дисплеем
void init_BUS(unsigned char display_model, unsigned char orientation)
{
  LCD_HI_DDR = 0b11111111; // все порты как выходы
  LCD_HI_PORT = 0b00000000; // на выходе 0
  
  // пины управления настраиваем на выход
  LCD_RS_DDR |= 1 << LCD_RS_PIN;
  LCD_WR_DDR |= 1 << LCD_WR_PIN;
  LCD_REST_DDR |= 1 << LCD_REST_PIN;
  //LCD_CS_DDR |= 1 << LCD_CS_PIN;
  LCD_RD_DDR |= 1 << LCD_RD_PIN;
  
  // устанавливаем начальные значения управляющих выводов
  //LCD_SET_CS();
  LCD_SET_REST();
  LCD_SET_RS();
  LCD_SET_WR();
  LCD_SET_RD();
  
  // железный сброс дисплея
  _delay_ms(100);
  LCD_CLR_REST(); // запускаем сброс
  _delay_ms(10);
  LCD_SET_REST(); // останавливаем
  _delay_ms(10);
  
  // выбираем дисплей
  //LCD_CLR_CS();
  
  // инициализация дисплея
  lcd_param.disp_x_size = 240;
  lcd_param.disp_y_size = 320;
  lcd_param.model = display_model;
  lcd_param.orient = orientation;
  lcd_param._transparent = 0;
  lcd_param.transfer_mode = TRANSFER_MODE_8;
  switch(display_model)
  {
    #ifdef SSD1289
    case SSD1289:
      #include "LCD_SSD1289.h"
      break;
    #endif
    #ifdef ILI9328
    case ILI9328:
      #include "LCD_ILI9328.h"
      break;
    #endif
  }
  LCD_SetFontColor_RGB(255, 255, 255);
  LCD_SetBackColor_RGB(0, 0, 0);
  cfont.font = 0;
}

//========================================================================================================
// Запись в шину данных дисплея. 8-битный режим
//========================================================================================================
void LCD_Write_Bus(unsigned char VH, unsigned char VL)
{
  switch(lcd_param.transfer_mode)
  {
    case TRANSFER_MODE_8:
      LCD_HI_PORT = VH;
      asm volatile ("nop");
      PULSE_WR_LOW();
      asm volatile ("nop");
      LCD_HI_PORT = VL;
      asm volatile ("nop");
      PULSE_WR_LOW();
      asm volatile ("nop");
      break;
    case TRANSFER_MODE_16:
      LCD_HI_PORT = VH;
      LCD_LO_PORT = VL;
      asm volatile ("nop");
      PULSE_WR_LOW();
      asm volatile ("nop");
      break;
  }
}

//========================================================================================================
// Отправка на дисплей данные (старший и младший байт)
//========================================================================================================
void LCD_Write_DATA(unsigned char VH, unsigned char VL)
{
  LCD_SET_RS();
  LCD_Write_Bus(VH, VL);
}

//========================================================================================================
// Отправка на дисплей комманды
//========================================================================================================
void LCD_Write_COM(unsigned char VL)
{
  LCD_CLR_RS();
  LCD_Write_Bus(0x00, VL);
}

//========================================================================================================
// Отправка на дисплей комманды и данные
//========================================================================================================
void LCD_Write_COM_DATA(unsigned char commmand, int data)
{
  LCD_Write_COM(commmand);
  LCD_Write_DATA(data >> 8, data);
}

// устанавливает цвет шрифта.
void LCD_SetFontColor_RGB(unsigned char r, unsigned char g, unsigned char b)
{
  lcd_param.fnt_color_h = ((r & 248) | g >> 5);
  lcd_param.fnt_color_l = ((g & 28) << 3 | b >> 3);
}

//устанавливает цвет шрифта
void LCD_SetFontColor_Word(unsigned int color)
{
  lcd_param.fnt_color_h = (color >> 8);
  lcd_param.fnt_color_l = (color & 0xFF);
}
//устанавливает цвет фона
void LCD_SetBackColor_RGB(unsigned char r, unsigned char g, unsigned char b)
{
  lcd_param.bg_color_h = ((r & 248) | g >> 5);
  lcd_param.bg_color_l = ((g & 28) << 3 | b >> 3);
  lcd_param._transparent = 0;
}

// устанавливает цвет фона
void LCD_SetBackColor_Word(unsigned int color)
{
  if (color == VGA_TRANSPARENT)
  {
    lcd_param._transparent = 1;
  } 
  else 
  {
    lcd_param.bg_color_h = (color >> 8);
    lcd_param.bg_color_l = (color & 0xFF);
    lcd_param._transparent = 0;
  }
}

//устанавливает курсор в позицию (х1, у1). и выставляет рабочую область с координатами ((х1, у1)(х2, у2))
void LCD_SetWorkAreaXY(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
  // скорректируем значения с учетом смещения начала координат  
  if (lcd_param.orient == LANDSCAPE)
  {
    swap(unsigned int, x1, y1);
    swap(unsigned int, x2, y2)
    y1 = lcd_param.disp_y_size - y1;
    y2 = lcd_param.disp_y_size - y2;
    swap(unsigned int, y1, y2)
  }
  switch(lcd_param.model)
  {
    #ifdef SSD1289
    case SSD1289:
      #include "setxy_SSD1289.h"
      break;
    #endif
    #ifdef ILI9328
      case ILI9328:
      #include "setxy_ILI9328.h"
    break;
    #endif
  }
}

// закрашивает пиксель по координатам (х,у) в цвет color
void LCD_DrawPixel(unsigned int x, unsigned int y, unsigned int color)
{
  LCD_SetWorkAreaXY(x, y, x, y);
  LCD_Write_DATA(color >> 8, color);
}

//очищает экран
void LCD_ClrXY(void)
{
  if (lcd_param.orient == PORTRAIT)
  {
    LCD_SetWorkAreaXY(X_begin, Y_begin, lcd_param.disp_x_size, lcd_param.disp_y_size);
  }
  else
  {
    LCD_SetWorkAreaXY(X_begin, Y_begin, lcd_param.disp_y_size, lcd_param.disp_x_size);
  }
}

// быстрая перерисовка области цветом
void _fast_fill_16(unsigned int ch, unsigned int cl, unsigned long pix)
{
  unsigned long blocks;
  unsigned int i;
  LCD_HI_PORT = ch;
  LCD_LO_PORT = cl;
  blocks = pix / 16;
  for (i = 0; i < blocks; i++)
  {
    PULSE_WR_LOW();
    PULSE_WR_LOW();
    PULSE_WR_LOW();
    PULSE_WR_LOW();
    PULSE_WR_LOW();
    PULSE_WR_LOW();
    PULSE_WR_LOW();
    PULSE_WR_LOW();
    PULSE_WR_LOW();
    PULSE_WR_LOW();
    PULSE_WR_LOW();
    PULSE_WR_LOW();
    PULSE_WR_LOW();
    PULSE_WR_LOW();
    PULSE_WR_LOW();
    PULSE_WR_LOW();
  }
  if ((pix % 16) != 0)
  {
    for (i = 0; i < (pix % 16) + 1; i++)
    {
      PULSE_WR_LOW();
    }
  }
}
//========================================================================================================
// ест много памяти
void _fast_fill_8(unsigned int ch, unsigned long pix)
{
  unsigned long blocks;
  unsigned long i;
  LCD_HI_PORT = ch;
  blocks = pix / 16;
  for (i = 0; i < blocks; i++)
  {
    PULSE_WR_LOW(); PULSE_WR_LOW();
    PULSE_WR_LOW(); PULSE_WR_LOW();
    PULSE_WR_LOW(); PULSE_WR_LOW();
    PULSE_WR_LOW(); PULSE_WR_LOW();
    PULSE_WR_LOW(); PULSE_WR_LOW();
    PULSE_WR_LOW(); PULSE_WR_LOW();
    PULSE_WR_LOW(); PULSE_WR_LOW();
    PULSE_WR_LOW(); PULSE_WR_LOW();
    PULSE_WR_LOW(); PULSE_WR_LOW();
    PULSE_WR_LOW(); PULSE_WR_LOW();
    PULSE_WR_LOW(); PULSE_WR_LOW();
    PULSE_WR_LOW(); PULSE_WR_LOW();
    PULSE_WR_LOW(); PULSE_WR_LOW();
    PULSE_WR_LOW(); PULSE_WR_LOW();
    PULSE_WR_LOW(); PULSE_WR_LOW();
    PULSE_WR_LOW(); PULSE_WR_LOW();
  }
  if ((pix % 16) != 0)
  {
    for (i = 0; i < (pix % 16) + 1; i++)
    {
      PULSE_WR_LOW(); PULSE_WR_LOW();
    }
  }
}

//очищаем экран
void LCD_ClrScreen(void)
{
  unsigned long int count_pxl;
  
  count_pxl = (unsigned long int)lcd_param.disp_x_size * (unsigned long int)lcd_param.disp_y_size;
  LCD_ClrXY();
  LCD_SET_RS();
  if (lcd_param.transfer_mode == TRANSFER_MODE_16)
  {
    _fast_fill_16(0,0, count_pxl);
  }
  else if (lcd_param.transfer_mode == TRANSFER_MODE_8)
  {
    _fast_fill_8(0, count_pxl);
  }
  else
  {
      unsigned long int i;
      for (i = 0; i < count_pxl; i++)
      {
        LCD_Write_Bus(0,0);
      }
  }
}

//заполняет рабочую область цветом
void LCD_FillScreen_RGB(unsigned char r, unsigned char g, unsigned char b)
{
  unsigned int color;
  color = ((r & 248) << 8 | (g & 252) << 3 | (b & 248) >> 3);
  LCD_FillScreen(color);
}

//заполняет весь экран цветом
void LCD_FillScreen(unsigned int color)
{
  unsigned long int count_pxl;
  unsigned char ch, cl;
  
  ch = (unsigned char)(color >> 8);
  cl = (unsigned char)(color & 0xFF);
  count_pxl = (unsigned long int)lcd_param.disp_x_size * (unsigned long int)lcd_param.disp_y_size;
  LCD_ClrXY();
  LCD_SET_RS();
  if (lcd_param.transfer_mode == TRANSFER_MODE_16)
  {
    _fast_fill_16(ch, cl, count_pxl);
  } 
  else 
    if ((lcd_param.transfer_mode == TRANSFER_MODE_8) && (ch == cl)) 
    {
      _fast_fill_8(ch, count_pxl);
    } 
    else 
    {
      unsigned long int i;
      
      for (i = 0; i < count_pxl; i++)
        {
          LCD_Write_Bus(ch,cl);
        }
    }
}

unsigned int LCD_GetRegister(unsigned char index)
{
  unsigned char retH;
  unsigned char retL;
  unsigned int ret;
  
  LCD_SET_RD();
  
  LCD_Write_COM(0x00);
  
  asm volatile ("nop");
  LCD_Bus_Input(); // input mode
  
  LCD_SET_RS();
  
  asm volatile ("nop");asm volatile ("nop");
  LCD_CLR_RD();
  asm volatile ("nop");asm volatile ("nop");
  retH = PIND;
  LCD_SET_RD();
  
  asm volatile ("nop");asm volatile ("nop");
  LCD_CLR_RD();
  asm volatile ("nop");asm volatile ("nop");
  retL = PIND;
  LCD_SET_RD();

  LCD_Bus_Output(); // Output mode
  
  ret = (retH << 8) | retL;
  return ret;
}

// рисование линии по алгоритму Брезенхэма
void LCD_DrawLineBresen(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color)
{
  int deltax, deltay, error, deltaerr, x, y, incx, incy;
  
  if (x1 == x2)
  {
    incy = y1 < y2 ? 1 : -1;
    for(unsigned int y = y1; y != y2; y += incy)
      LCD_DrawPixel(x1, y, color);
    return;
  }
  
  incx = x1 < x2 ? 1 : -1;
  incy = y1 < y2 ? 1 : -1;
  deltax = abs(x2 - x1);
  deltay = abs(y2 - y1);
  error = 0;
  
  if (deltax > deltay)
  {
    deltaerr = deltay;
    y = y1;
    for (int xx = x1; xx != x2; xx += incx)
    {
      LCD_DrawPixel(xx, y, color);
      error = error + deltaerr;
      if (2 * error >= deltax)
      {
        y += incy;
        error -= deltax;
      }
    }
  }
  else
  {
    deltaerr = deltax;
    x = x1;
    for (int yy = y1; yy != y2; yy += incy)
    {
      LCD_DrawPixel(x, yy, color);
      error = error + deltaerr;
      if (2 * error >= deltay)
      {
        x += incx;
        error -= deltay;
      }
    }
  }
}

// рисует прямую по координатам [(x1, y1);(x2, y2)]
// y=((x-x1)*(y2-y1))/(x2-x1)+y1
void LCD_DrawLine(long int x1, long int y1, long int x2, long int y2, unsigned int color, long int increment_x)
{
  unsigned int y;
  unsigned int prev_x, prev_y;
  
  //const long int increment_x = 10;
  //LCD_ClrXY();
  prev_x = 999;
  prev_y = 999;
  if (x1 == x2)
  {
    for(unsigned int i = (y1 < y2 ? y1 : y2); i <= (y1 < y2 ? y2 : y1); i++)
      LCD_DrawPixel(x1, i, color);
  }
  else if (y1 == y2)
  {
    for (unsigned int i = (x1 < x2 ? x1 : x2); i <= (x1 < x2 ? x2 : x1); i++)
      LCD_DrawPixel(i, y1, color);
  }
  else if (x1 > x2)
  {
    for (long int i = x2 * increment_x; i <= x1 * increment_x; i++)
    {
      y = (((x1 * increment_x - i) * (y2 * increment_x - y1 * increment_x)) / (x1 * increment_x - x2 * increment_x) + y1 * increment_x) / increment_x;
      if ((prev_x != (unsigned int) (i / increment_x)) || (prev_y != y))
      {
        prev_x = (unsigned int) (i / increment_x);
        prev_y = y;
        LCD_DrawPixel(prev_x, prev_y, color);
      }
    }
  }
  else
  {
    for (long int  i = x1 * increment_x; i <= x2 * increment_x; i++)
    {
      y = (((i - x1 * increment_x) * (y2 * increment_x - y1 * increment_x)) / (x2 * increment_x - x1 * increment_x) + y1 * increment_x) / increment_x;
      if ((prev_x != (unsigned int) (i / increment_x)) || (prev_y != y))
      {
        prev_x = (unsigned int) (i / increment_x);
        prev_y = y;
        LCD_DrawPixel(prev_x, prev_y, color);
      }        
    }  
  }  
  
}

// рисует прямоугольник
// x1, y1 - координаты левого верхнего угла
// x2, y2 - координаты правого нижнего угла
void LCD_DrawRectangle(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color)
{
  LCD_DrawLine(x1, y1, x2, y1, color, 1);
  LCD_DrawLine(x2, y1, x2, y2, color, 1);
  LCD_DrawLine(x2, y2, x1, y2, color, 1);
  LCD_DrawLine(x1, y2, x1, y1, color, 1);
}

// функция рисует дугу. рисование идет против часовой стрелки
// x, y - центр окружности описывающей дугу
// r - радиус окружности
// b_аngle - начальный угол дуги в грудусах
// angle - общий угол дуги в гружусах
void LCD_DrawArc(int x, int y, int r, unsigned int b_angle, unsigned int angle, unsigned int color)
{
  int b_x; // начальное значение проекции дуги на ось X
  int e_x; // значение по оси х в конечной точке
  int y_new;   // рассчитанное значение по оси Y
  unsigned int prev_x, prev_y;
  unsigned int end_angle;
  const float increment_x = 0.5;
  
  // определяем начальное значение проекции по оси X
  b_x = x + r * cos(b_angle * M_PI / 180);
  
  // определяем конечное значение по оси x
  end_angle = b_angle + angle;
  if (end_angle > 360)
    end_angle -= 360;
  e_x = x + r * cos(end_angle * M_PI / 180);
  
  // если начало в верхней части
  prev_x = 999;
  prev_y = 999;
  if (b_angle <= 180)
  {
    if ((end_angle > 180) || (end_angle < b_angle))
    {
      //1
      for (float i = b_x; i >= x - r; i -= increment_x)
      {
        y_new = -sqrt(square(r) - square(i - x)) + y;
        if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
        {
          LCD_DrawPixel(i, y_new, color);
          prev_x = i;
          prev_y = y_new;
        }
      }
      // если конец дуги в нижней части
      if ((end_angle > 180) && (end_angle <= 360))
      {
        for (float i = x - r; i <= e_x; i += increment_x)
        {
          y_new = sqrt(square(r) - square(i - x)) + y;
          if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
          {
            LCD_DrawPixel(i, y_new, color);
            prev_x = i;
            prev_y = y_new;
          }
        }
      }
      else // если конец дуги в верхней части
      {
        for (float i = x - r; i <= x + r; i += increment_x)
        {
          y_new = sqrt(square(r) - square(i - x)) + y;
          if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
          {
            LCD_DrawPixel(i, y_new, color);
            prev_x = i;
            prev_y = y_new;
          }
        }
        for (float i = x + r; i >= e_x; i -= increment_x)
        {
          y_new = -sqrt(square(r) - square(i - x)) + y;
          if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
          {
            LCD_DrawPixel(i, y_new, color);
            prev_x = i;
            prev_y = y_new;
          }
        }
      }
    }
    else
    {
      for (float i = b_x; i >= e_x; i -= increment_x)
      {
        y_new = -sqrt(square(r) - square(i - x)) + y;
        if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
        {
          LCD_DrawPixel(i, y_new, color);
          prev_x = i;
          prev_y = y_new;
        }         
      }
    }
  }
  else
  {
    if (end_angle >= b_angle)
    {
      //1
      for (float i = b_x; i <= e_x; i += increment_x)
      {
        y_new = sqrt(square(r) - square(i - x)) + y;
        if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
        {
          LCD_DrawPixel(i, y_new, color);
          prev_x = i;
          prev_y = y_new;
        }
      }
    }
    else
    {
      for (float i = b_x; i <= x + r; i += increment_x)
      {
        y_new = sqrt(square(r) - square(i - x)) + y;
        if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
        {
          LCD_DrawPixel(i, y_new, color);
          prev_x = i;
          prev_y = y_new;
        }
      }
      if (end_angle <= 180)
      {
        for (float i = x + r; i >= e_x; i -= increment_x)
        {
          y_new = - sqrt(square(r) - square(i - x)) + y;
          if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
          {
            LCD_DrawPixel(i, y_new, color);
            prev_x = i;
            prev_y = y_new;
          }
        }
      }
      else
      {
        for (float i = x + r; i >= x - r; i -= increment_x)
        {
          y_new = - sqrt(square(r) - square(i - x)) + y;
          if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
          {
            LCD_DrawPixel(i, y_new, color);
            prev_x = i;
            prev_y = y_new;
          }
        }
        for (float i = x - r; i <= e_x; i += increment_x)
        {
          y_new = sqrt(square(r) - square(i - x)) + y;
          if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
          {
            LCD_DrawPixel(i, y_new, color);
            prev_x = i;
            prev_y = y_new;
          }
        }
      }
    }
  }
}

// функция закрашивает дугу. рисование идет против часовой стрелки
// x, y - центр окружности описывающей дугу
// r - радиус окружности
// b_аngle - начальный угол дуги в грудусах
// angle - общий угол дуги в гружусах
void LCD_FillArc(int x, int y, int r, unsigned int b_angle, unsigned int angle, unsigned int color)
{
  int b_x; // начальное значение проекции дуги на ось X
  int e_x; // значение по оси х в конечной точке
  int y_new;   // рассчитанное значение по оси Y
  unsigned int prev_x, prev_y;
  unsigned int end_angle;
  const float increment_x = 0.01;
  
  // определяем начальное значение проекции по оси X
  b_x = x + r * cos(b_angle * M_PI / 180);
  
  // определяем конечное значение по оси x
  end_angle = b_angle + angle;
  if (end_angle > 360)
  end_angle -= 360;
  e_x = x + r * cos(end_angle * M_PI / 180);
  
  // если начало в верхней части
  prev_x = 999;
  prev_y = 999;
  if (b_angle <= 180)
  {
    if ((end_angle >= 180) || (end_angle <= b_angle))
    {
      //1
      for (float i = b_x; i >= x - r; i -= increment_x)
      {
        y_new = -sqrt(square(r) - square(i - x)) + y;
        if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
        {
          LCD_DrawLine(x, y, i, y_new, color, 10);
          prev_x = i;
          prev_y = y_new;
        }
      }
      // если конец дуги в нижней части
      if ((end_angle > 180) && (end_angle <= 360))
      {
        for (float i = x - r; i <= e_x; i += increment_x)
        {
          y_new = sqrt(square(r) - square(i - x)) + y;
          if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
          {
            LCD_DrawLine(x, y, i, y_new, color, 10);
            prev_x = i;
            prev_y = y_new;
          }
        }
      }
      else // если конец дуги в верхней части
      {
        for (float i = x - r; i <= x + r; i += increment_x)
        {
          y_new = sqrt(square(r) - square(i - x)) + y;
          if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
          {
            LCD_DrawLine(x, y, i, y_new, color, 10);
            prev_x = i;
            prev_y = y_new;
          }
        }
        for (float i = x + r; i >= e_x; i -= increment_x)
        {
          y_new = -sqrt(square(r) - square(i - x)) + y;
          if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
          {
            LCD_DrawLine(x, y, i, y_new, color, 10);
            prev_x = i;
            prev_y = y_new;
          }
        }
      }
    }
    else
    {
      for (float i = b_x; i >= e_x; i -= increment_x)
      {
        y_new = -sqrt(square(r) - square(i - x)) + y;
        if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
        {
          LCD_DrawLine(x, y, i, y_new, color, 10);
          prev_x = i;
          prev_y = y_new;
        }
      }
    }
  }
  else
  {
    if (end_angle >= b_angle)
    {
      //1
      for (float i = b_x; i <= e_x; i += increment_x)
      {
        y_new = sqrt(square(r) - square(i - x)) + y;
        if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
        {
          LCD_DrawLine(x, y, i, y_new, color, 10);
          prev_x = i;
          prev_y = y_new;
        }
      }
    }
    else
    {
      for (float i = b_x; i <= x + r; i += increment_x)
      {
        y_new = sqrt(square(r) - square(i - x)) + y;
        if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
        {
          LCD_DrawLine(x, y, i, y_new, color, 10);
          prev_x = i;
          prev_y = y_new;
        }
      }
      if (end_angle <= 180)
      {
        for (float i = x + r; i >= e_x; i -= increment_x)
        {
          y_new = - sqrt(square(r) - square(i - x)) + y;
          if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
          {
            LCD_DrawLine(x, y, i, y_new, color, 10);
            prev_x = i;
            prev_y = y_new;
          }
        }
      }
      else
      {
        for (float i = x + r; i >= x - r; i -= increment_x)
        {
          y_new = - sqrt(square(r) - square(i - x)) + y;
          if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
          {
            LCD_DrawLine(x, y, i, y_new, color, 10);
            prev_x = i;
            prev_y = y_new;
          }
        }
        for (float i = x - r; i <= e_x; i += increment_x)
        {
          y_new = sqrt(square(r) - square(i - x)) + y;
          if ((prev_x != (unsigned int)i) || (prev_y != (unsigned int)y_new))
          {
            LCD_DrawLine(x, y, i, y_new, color, 10);
            prev_x = i;
            prev_y = y_new;
          }
        }
      }
    }
  }
}

// рисует закрашенный прямоугольник
// x1, y1 - координаты левого верхнего угла
// x2, y2 - координаты правого нижнего угла
void LCD_FillRectangle(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color)
{
  LCD_SetWorkAreaXY(x1, y1, x2, y2);
  for (unsigned int i = x1 < x2 ? x1 : x2; i <= ((x1 < x2) ? x2 : x1); i++)
    for (unsigned int j = y1 < y2 ? y1 : y2; j <= ((y1 < y2) ? y2 : y1); j++)
    {
      LCD_Write_DATA(color >> 8, color);
      //_delay_ms(200);
    }     
}

void LCD_SetFont(const unsigned char *font)
{
  cfont.font = font;
  cfont.x_size = pgm_read_byte(&cfont.font[0]);
  cfont.y_size = pgm_read_byte(&cfont.font[1]);
  cfont.offset = pgm_read_byte(&cfont.font[2]);
  cfont.numchars = pgm_read_byte(&cfont.font[3]);
}

// рисует символ ch в позиции (x, y)
void LCD_DrawChar(unsigned int x, unsigned int y, unsigned char ch)
{
  unsigned char sim;
  unsigned int pos;
  unsigned char tmp_x;
  
  tmp_x = ceil(cfont.x_size / 8);
  pos = (ch - cfont.offset) * (tmp_x * cfont.y_size) + 4;
  LCD_SetWorkAreaXY(x, y, x + cfont.x_size - 1, y + cfont.y_size - 1);
  if (lcd_param.orient == LANDSCAPE)
  {
    for (unsigned char i = 0; i < tmp_x * cfont.y_size; i++)
    {
      sim = pgm_read_byte(&cfont.font[pos + i]);
      for (unsigned char j = 8 ; j > 0; j--)
      {
        if ((sim & (1 << (j - 1))) != 0)
        {
          LCD_Write_DATA(lcd_param.fnt_color_h, lcd_param.fnt_color_l);
        }
        else
        {
          LCD_Write_DATA(lcd_param.bg_color_h, lcd_param.bg_color_l);
        }
      }
    }
  }
}

/*void LCD_DrawChar(unsigned int x, unsigned int y, unsigned char ch)
{
  unsigned char sim;
  unsigned int pos;
  unsigned char tmp_x;
  unsigned char index_bit;
  unsigned char last_bit;
  
  tmp_x = ceil(cfont.x_size / 8) + 1;
  pos = (ch - cfont.offset) * (tmp_x * cfont.y_size) + 4;
  LCD_SetWorkAreaXY(x, y, x + cfont.x_size - 1, y + cfont.y_size - 1);
  if (lcd_param.orient == LANDSCAPE)
  {
    index_bit = 0;
    for (unsigned char i = 0; i < tmp_x * cfont.y_size; i++)
    {
      index_bit++;
      sim = pgm_read_byte(&cfont.font[pos + i]);
      if (index_bit < tmp_x)
      {
        last_bit = 0;
      }
      else
      {
        last_bit = tmp_x * 8 - cfont.x_size;
      }
      for (unsigned char j = 8 ; j > last_bit; j--)
      {
        if ((sim & (1 << (j - 1))) != 0)
        {
          LCD_Write_DATA(lcd_param.fnt_color_h, lcd_param.fnt_color_l);
        }
        else
        {
          LCD_Write_DATA(lcd_param.bg_color_h, lcd_param.bg_color_l);
        }
      }
    }
  }
}*/

// вывод текста
void LCD_DrawText(unsigned int x, unsigned int y, unsigned char* text)
{
  unsigned int tek_x, tek_y;
  unsigned int tek_char = 0;
  
  tek_x = x;
  tek_y = y;
  while(text[tek_char] != '\0')
  {
    LCD_DrawChar(tek_x, tek_y, text[tek_char]);
    tek_char++;
    tek_x += cfont.x_size;
  }
}

// вывод на экран числа формата unsigned int
void LCD_DrawNumberUInt(unsigned int x, unsigned int y, unsigned int val)
{
  char buf[6];

  LCD_DrawText(x, y, utoa(val, buf, 10));
}

// вывод на экран числа формата int
void LCD_DrawNumberInt(unsigned int x, unsigned int y, int val)
{
  char buf[6];

  LCD_DrawText(x, y, itoa(val, buf, 10));
}

// вывод на экран числа формата char
void LCD_DrawNumberChar(unsigned int x, unsigned int y, signed char val)
{
  char buf[4];

  LCD_DrawText(x, y, itoa(val, buf, 10));
}

// вывод на экран числа формата unsigned char
void LCD_DrawNumberUChar(unsigned int x, unsigned int y, unsigned char val)
{
  char buf[4];

  LCD_DrawText(x, y, utoa(val, buf, 10));
}