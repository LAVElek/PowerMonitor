/*
мо€ реализаци€ работы с часами DS1307 при помощи аппаратного TWI
*/
#include "Header/DS1307.h"
#include "header/TWI.h"

unsigned char buf[TWI_BUFFER_SIZE];

void initDS1307(unsigned long MK_Speed)
{
  // инициализируем TWI
  TWI_MasterInit(DS1307_SPEED, MK_Speed);
  buf[0] = SLA_W;
  buf[1] = R_CONTROL; // адрес регистра с настройками
  buf[2] = (1 << CONTROL_SQWE) | // включаем выход счетчика
           (0 << CONTROL_RS1) | (0 << CONTROL_RS0); // частота счетчика 1√ц
  TWI_SendData(buf, 3);
}

// выставл€ем врем€
void setTime(unsigned char hour,
             unsigned char minute,
             unsigned char second)
{
  buf[0] = SLA_W;
  buf[1] = R_SECOND;
  buf[2] = ((second / 10) << 4) + second % 10; // R_SECOND
  buf[3] = ((minute / 10) << 4) + minute % 10; // R_MINUTE - после записи адрес регистра увеличиваетс€ автоматически
  buf[4] = ((hour / 10) << 4) + hour % 10; // R_HOUR
  TWI_SendData(buf, 5);
}

// получаем врем€
void getTime(unsigned char *time_buf)
{
  buf[0] = SLA_W;
  buf[1] = R_SECOND;
  TWI_SendData(buf, 2);
  buf[0] = SLA_R;
  buf[1] = 0;
  buf[2] = 0;
  buf[3] = 0;
  TWI_SendData(buf, 4);
  if (TWI_GetState() == TWI_SUCCESS)
  {
    TWI_GetData(buf, 5);
    time_buf[0] = (buf[3] >> 4) * 10 + (buf[3] & 0xf); // часы
    time_buf[1] = (buf[2] >> 4) * 10 + (buf[2] & 0xf); // минуты
    time_buf[2] = (buf[1] >> 4) * 10 + (buf[1] & 0xf); // секунды
  }
}

// устанавливаем дату
void setDate(unsigned char day,
             unsigned char month,
             unsigned char year,
             unsigned char day_week)
{
  buf[0] = SLA_W;
  buf[1] = R_WEEK_DAY;
  buf[2] = day_week; // R_WEEK_DAY
  buf[3] = ((day / 10) << 4) + day % 10; // R_DAY
  buf[4] = ((month / 10) << 4) + month % 10; // R_MONTH
  buf[5] = ((year / 10) << 4) + year % 10; // R_YEAR
  TWI_SendData(buf, 6);
}

// получает дату
void getDate(unsigned char *time_buf)
{
  buf[0] = SLA_W;
  buf[1] = R_WEEK_DAY;
  TWI_SendData(buf, 2);
  buf[0] = SLA_R;
  buf[1] = 0;
  buf[2] = 0;
  buf[3] = 0;
  buf[4] = 0;
  TWI_SendData(buf, 5);
  if (TWI_GetState() == TWI_SUCCESS)
  {
    TWI_GetData(buf, 5);
    time_buf[0] = buf[1]; // день недели
    time_buf[1] = (buf[2] >> 4) * 10 + (buf[2] & 0xf); // день мес€ца
    time_buf[2] = (buf[3] >> 4) * 10 + (buf[3] & 0xf); // мес€ц
    time_buf[3] = (buf[4] >> 4) * 10 + (buf[4] & 0xf); // год
  }
}