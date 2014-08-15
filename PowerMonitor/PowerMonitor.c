#define F_CPU 20000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "LCD/LCD.h"
#include "LCD/define_fonts.h"
//#include "header/mUART.h"
#include "DS1307/DS1307.h"
//#include "Header/TWI.h"

//#define UART_SPEED 19200 // скорость UART
#define ADC_DISCR 4585 // дискретность АЦП (4,585 мВ) * 1000, чтобы не работать с дробными числами
#define ADC_TEMP 0b00000100 // настройки регистра АЦП ADMUX для датчик температуры. 4 канал

#define ARC_BEGIN 45 // начало дуги индикатора в градусах
#define ARC_LENGTH 90 // длина дуги индикатора в градусах
#define ARC_LENGTH_HIGH_ZONE 15 // длина красной зоны в градусах
#define RADIUS 100 // радиус индикатора
#define MAX_VALUE_ZONE 20 // максимальное значение на индикаторе
#define MAX_VALUE_ZONE_STR "20" // максимальное значение на индикаторе для отображения

#define BACKGROUND_COLOR VGA_BLUE

#define TEMPERATURE_INTERVAL 1 //периодичность измерения температуры в секундах

//unsigned char ds1307_buf[6];
unsigned char updateTime = 0, count_interrupt_timer0 = 0, temp_inter = 0;
unsigned char time_string[] = "23:59:59";
unsigned int prev_xy[2][2][4] = {{{0, 0, 0, 0},{0, 0, 0, 0}},
                                {{0, 0, 0, 0},{0, 0, 0, 0}}}; // предыдущие координаты указателя индикатора

unsigned int b_x[] = {80, 240}; // координаты центров индикаторов по оси х
unsigned int b_y[] = {108, 216}; // координаты центров индикаторов по оси у

void update_time(unsigned char isFirst)
{
  unsigned char new_second, new_minute, new_hour;
  //unsigned char time_string[8];
  
  // если первый запуск
  new_second = l_get_second();
  time_string[6] = new_second / 10 + '0';
  time_string[7] = new_second % 10 + '0';
  if ((isFirst) || (new_second == 0))
  {
    new_minute = l_get_minute();
    time_string[3] = new_minute / 10 + '0';
    time_string[4] = new_minute % 10 + '0';
    if ((isFirst) || (new_minute == 0))
    {
      new_hour = l_get_hour();
      time_string[0] = new_hour / 10 + '0';
      time_string[1] = new_hour % 10 + '0';
    }
  }
  
  LCD_SetFont(SmallFont);
  LCD_SetFontColor_Word(VGA_BLACK);
  LCD_DrawText(256, 224, time_string);
  updateTime = 0;
  
  
  drawValueZone(0, 0, new_second * 1000);
}

// настройка таймера 0 для обновления времени
void initTimer0()
{
  TIMSK |= (1 << OCIE0); // разрешаем прерывание по совпадению
  TCCR0 = (0 << WGM01) | (0 < WGM00) // нормальный режим работы таймера
          | (0 < COM01) | (0 < COM00) // порт OC0 не активен
          | (1 << CS02) | (0 << CS01) | (1 << CS00); // деление частоты на 1024
  OCR0 = 195; // число для сравнения, т.е. будет прерывание когда счетчик таймера досчитает до 195
              // счетчик таймера достигнет этого значения за 0.01 секунду
  TCNT0 = 0; // обнуляем счетчик
}

// настройка АЦП
void initADC()
{
  ADMUX = (0 << REFS1) | (0 << REFS0) // используем внешний источник опорного напряжения, который равен 4,69В
          | (0 << ADLAR); // 2 старших бита в ADCH
  ADCSRA = (1 << ADEN) // включаем АЦП
           | (1 << ADSC) // выполним сразу преобразование, чтобы дальше оно выполнялось быстрее
           | (0 << ADATE) // автоматический запуск отключен
           | (0 << ADIE) // запрещаем прерывание по окончании преобазования
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // предделитель частоты АЦП 128
}

// возвращает напряжение на канале АЦП
unsigned int getADC(unsigned char channel_settings)
{
  ADMUX = channel_settings;
  ADCSRA |= (1 << ADSC); // запускаем преобразование
  
  while ((ADCSRA & (1 << ADIF)) == 0); // ждем окончания преобразования
  return ((unsigned long)ADCW * (unsigned long)ADC_DISCR) / 1000;
}

void update_temperature()
{
  unsigned int adc_res;
  unsigned char temp[] = "12.4";
  
  adc_res = abs(getADC(ADC_TEMP) - 2730);
  temp[0] = (adc_res / 100 > 9 ? adc_res / 1000 : adc_res / 100) + '0';
  temp[1] = (adc_res / 10) % 10 + '0';
  temp[3] = adc_res % 10 + '0';
  LCD_SetFontColor_Word(VGA_LIME);
  LCD_SetFont(BigFont);
  LCD_DrawText(128, 100, temp);
}

ISR(TIMER0_COMP_vect)
{
  // прерывание сра
  // необходимо 100 срабатываний прерывания чтобы прошла секунда
  if(count_interrupt_timer0++ == 100)
  {
    TCNT0 = 0;
    count_interrupt_timer0 = 0;
    updateTime = 1;
    temp_inter++;
  }
}

// отображает значение тока на индикаторе
// zone - номер индикатора
// ampers - протекающий ток в милиамперах
void drawValueZone(unsigned char zone_x, unsigned char zone_y, unsigned int ampers)
{
  unsigned char string_value[] = "99.9";
  unsigned int x1, x2, y1, y2;
  double koef;
  
  string_value[0] = ampers / 10000 + '0';
  string_value[1] = (ampers / 1000) % 10 + '0';
  string_value[2] = '.';
  string_value[3] = (ampers / 100) % 10 + '0';
  
  // закрашиваем предыдущую линию-указатель
  LCD_DrawLineBresen(prev_xy[zone_x][zone_y][0] ,
                     prev_xy[zone_x][zone_y][1] ,
                     prev_xy[zone_x][zone_y][2] ,
                     prev_xy[zone_x][zone_y][3] ,
                     BACKGROUND_COLOR);
  
  LCD_SetFontColor_Word(VGA_RED);
  LCD_SetFont(BigFont);
  LCD_DrawText(b_x[zone_x] - 33, b_y[zone_y] - 18, string_value);
  
  koef = ((double)ampers / 1000) / (double)MAX_VALUE_ZONE;
  if (koef > 1) 
    koef = 1;
  x1 = b_x[zone_x] + (RADIUS - 11) * cos((ARC_BEGIN + ARC_LENGTH - ARC_LENGTH * koef) * M_PI / 180) + 1;
  x2 = b_x[zone_x] + 36 * cos((ARC_BEGIN + ARC_LENGTH - ARC_LENGTH * koef) * M_PI / 180) + 1;
  y1 = -sqrt((RADIUS - 11) * (RADIUS - 11) - (x1 - b_x[zone_x]) * (x1 - b_x[zone_x])) + b_y[zone_y];
  y2 = -sqrt(36 * 36 - (x2 - b_x[zone_x]) * (x2 - b_x[zone_x])) + b_y[zone_y];
  LCD_DrawLineBresen(x1,
                     y1,
                     x2,
                     y2,
                     VGA_PURPLE);
  prev_xy[zone_x][zone_y][0] = x1;
  prev_xy[zone_x][zone_y][1] = y1;
  prev_xy[zone_x][zone_y][2] = x2;
  prev_xy[zone_x][zone_y][3] = y2;
}

// рисует рабочую область
void drawWorkArea()
{
  const unsigned char count_work_area_x = 2;
  const unsigned char count_work_area_y = 2;
  const unsigned char faska = 3;
  
  unsigned char x, y, i;
  unsigned int x1, x2, y1, y2;
  
  // инициализируем дисплей
  init_BUS(ILI9328, LANDSCAPE);                                                                             
  LCD_ClrScreen();
  
  //------------------------------------
  LCD_FillScreen(BACKGROUND_COLOR);
  LCD_SetBackColor_Word(BACKGROUND_COLOR);
  LCD_FillRectangle(0,220, 250, 240,VGA_GRAY); // прогресс бар
  LCD_FillRectangle(0, 217, 320, 219, VGA_YELLOW); 
  // разделяем рабочую область на 4 части
  LCD_FillRectangle(0, 107, 320, 109, VGA_YELLOW);
  LCD_FillRectangle(159, 0, 161, 216, VGA_YELLOW);
  // вычерчиваем в центре место для температуры
  LCD_FillRectangle(125, 97, 194, 118, VGA_YELLOW);
  // время
  //==================================
  LCD_FillRectangle(254, 220, 320, 240, BACKGROUND_COLOR);
  LCD_FillRectangle(251, 220, 253, 240, VGA_YELLOW);
  //==================================
  
  // рисуем индикаторы
  for(x = 0; x < count_work_area_x; x++)
    for(y = 0; y < count_work_area_y; y++)
    {
      LCD_DrawArc(b_x[x], b_y[y], RADIUS, ARC_BEGIN, ARC_LENGTH, VGA_LIME);
      LCD_DrawArc(b_x[x], b_y[y], RADIUS - 1, ARC_BEGIN, ARC_LENGTH, VGA_LIME);
      LCD_DrawArc(b_x[x], b_y[y], RADIUS - 9, ARC_BEGIN, ARC_LENGTH, VGA_LIME);
      LCD_DrawArc(b_x[x], b_y[y], RADIUS - 10, ARC_BEGIN, ARC_LENGTH, VGA_LIME);
      
      for(char i = 2; i <= 8; i++)
      {
        LCD_DrawArc(b_x[x], b_y[y], (RADIUS - i), ARC_BEGIN, ARC_LENGTH_HIGH_ZONE, VGA_RED);
        LCD_DrawArc(b_x[x], b_y[y], (RADIUS - i), ARC_BEGIN + ARC_LENGTH_HIGH_ZONE, ARC_LENGTH - ARC_LENGTH_HIGH_ZONE, VGA_LIME);
      }
      
      x1 = b_x[x] + (RADIUS - 10) * cos(ARC_BEGIN * M_PI / 180) + 1;
      x2 = b_x[x] + RADIUS * cos(ARC_BEGIN * M_PI / 180) + 1;
      y1 = -sqrt(square(RADIUS - 10) - square(x1 - b_x[x])) + b_y[y];
      y2 = -sqrt(square(RADIUS) - square(x2 - b_x[x])) + b_y[y];
      
      LCD_DrawLine(x1,
                  y1,
                  x2,
                  y2,
                  VGA_LIME,
                  10);
      
      x1 = b_x[x] + (RADIUS - 10) * cos((ARC_BEGIN + ARC_LENGTH_HIGH_ZONE) * M_PI / 180) + 1;
      x2 = b_x[x] + RADIUS * cos((ARC_BEGIN + ARC_LENGTH_HIGH_ZONE) * M_PI / 180) + 1;
      y1 = -sqrt(square(RADIUS - 10) - square(x1 - b_x[x])) + b_y[y];
      y2 = -sqrt(square(RADIUS) - square(x2 - b_x[x])) + b_y[y];
      
      LCD_DrawLine(x1,
                  y1,
                  x2,
                  y2,
                  VGA_LIME,
                  10);
      
      
      x1 = b_x[x] + (RADIUS - 10) * cos((ARC_BEGIN + ARC_LENGTH) * M_PI / 180) - 1;
      x2 = b_x[x] + RADIUS * cos((ARC_BEGIN + ARC_LENGTH) * M_PI / 180) - 1;
      y1 = -sqrt(square(RADIUS - 10) - square(b_x[x] - x1)) + b_y[y];
      y2 = -sqrt(square(RADIUS) - square(b_x[x] - x2)) + b_y[y];
      
      LCD_DrawLine(x1,
                  y1,
                  x2,
                  y2,
                  VGA_LIME,
                  10);
      
      LCD_DrawLine(x1 + 1,
                  y1 - 1,
                  x2 + 1,
                  y2,
                  VGA_LIME,
                  10);
      
      LCD_SetFontColor_Word(VGA_BLACK);
      LCD_SetFont(SmallFont);
      LCD_DrawText(b_x[x] - 72, b_y[y] - 63, "0");
      LCD_DrawText(b_x[x] + 63, b_y[y] - 63, MAX_VALUE_ZONE_STR);
      
      //подпись
      for(i = 0; i < faska; i++)
      {
        LCD_DrawLine(b_x[x] - 35 - i, b_y[y] - 22 + faska - i, b_x[x] - 35 - i, b_y[y], VGA_YELLOW, 1);
        
        LCD_DrawLine(b_x[x] - 35 - i, b_y[y] - 22 + faska - i, b_x[x] - 35 + faska - i, b_y[y] - 22 - i, VGA_YELLOW, 1);
        
        if (i < faska - 1)
          LCD_DrawLine(b_x[x] - 36 - i, b_y[y] - 22 + faska - i, b_x[x] - 36 + faska - i, b_y[y] - 22 - i, VGA_YELLOW, 1);
        
        LCD_DrawLine(b_x[x] + 32 + i, b_y[y] - 22 + faska - i, b_x[x] + 32 + i, b_y[y], VGA_YELLOW, 1);
        
        LCD_DrawLine(b_x[x] + 32 + i, b_y[y] - 22 + faska - i, b_x[x] + 32 - faska + i, b_y[y] - 22 - i, VGA_YELLOW, 1);
        
        if (i < faska - 1)
          LCD_DrawLine(b_x[x] + 32 + i, b_y[y] - 23 + faska - i, b_x[x] + 32 - faska + i, b_y[y] - 23 - i, VGA_YELLOW, 1);
        
        LCD_DrawLine(b_x[x] - 35 + faska - i, b_y[y] - 22 - i, b_x[x] + 32 - faska + i, b_y[y] - 22 - i, VGA_YELLOW, 1);
      }
      
      drawValueZone(x, y, (x + 1) * 7000 + (y + 1) * 3500);
    }
  // первый квадрат
  //==================================================================
}

int main(void)
{
  // начальная настройка портов
  //        76543210
  //        |-
  //        ||-
  //        |||-
  //        ||||- ADC4 - датчик температуры
  //        |||||- RD
  //        ||||||-
  //        |||||||-
  //        ||||||||-
  PORTA = 0b00000000;
  DDRA =  0b00001000;

  //        76543210
  //        |-
  //        ||-
  //        |||-
  //        ||||- WR
  //        |||||- RS
  //        ||||||- внешнее прерывание INT2
  //        |||||||- SDA
  //        ||||||||- SCL
  PORTB = 0b00000000;
  DDRB =  0b00011001;

  //        76543210
  //        |-
  //        ||-
  //        |||-
  //        ||||-
  //        |||||-
  //        ||||||-
  //        |||||||- REST
  //        ||||||||- 
  PORTC = 0b00000000;
  DDRC =  0b00000010;
  
  //        76543210
  //        |- DB15
  //        ||- DB14
  //        |||- DB13
  //        ||||- DB12
  //        |||||- DB11
  //        ||||||- DB10
  //        |||||||- DB09
  //        ||||||||- DB08
  PORTD = 0b00000000;
  DDRD =  0b11111111;
  
  // готовим экран
  drawWorkArea();
  
  //l_set_time(20, 23, 00);
  
  update_time(1);
  
  initTimer0();
  initADC();
  
  update_temperature();
  
  // разрешаем прерывания
  sei();
  
  while(1)
  {
    if(updateTime)
    {
      update_time(0);
    }
    if (temp_inter == TEMPERATURE_INTERVAL)
    {
      temp_inter = 0;
      update_temperature();
    }
  }
}