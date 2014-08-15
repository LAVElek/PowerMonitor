/*
���������� ������� � ����� http://www.embed.com.ua/mikrokontrollernyiy-konstruktor/shina-i2c/
��� ������� � ��������� l_*
������ c DS1307 ����� ����������� ���������� TWI
*/

#include "DS1307.h"

unsigned char ack;
//������� ������ �� ����� �� ������
unsigned char read_clock(unsigned char adr)
{
  unsigned char z;
  do
  {
    ack = 0;
    i2start();
    i2write(0xD0); ack |= i2ack();
    i2write(adr); ack |= i2ack();
    i2start();
    i2write(0xD1); ack |= i2ack();
    z=i2read(); i2nack();
    i2stop();
  }
  while(ack);
  return z;
}

//������� ������ ����� �� ������
void write_clock (unsigned char adr, unsigned char data)
{
  do
  {
    ack=0;
    i2start();
    i2write(0xD0); ack |= i2ack();
    i2write(adr); ack |= i2ack();
    i2write(data); ack |= i2ack();
    i2stop();
  }
  while(ack);
}

// ���������� ������� �������
// time_adr - ����� ��������(�������, ������, ���)
unsigned char l_get_time(unsigned char time_adr)
{
  unsigned char tmp;
  tmp = read_clock(time_adr);
  return (tmp >> 4) * 10 + (tmp & 0xf);
}

// ���������� �������
unsigned char l_get_second()
{
  return l_get_time(R_SECOND);
}

// ���������� �����
unsigned char l_get_minute()
{
  return l_get_time(R_MINUTE);
}
// ���������� ����
unsigned char l_get_hour()
{
  return l_get_time(R_HOUR);
}

// ������� ������������� �����
void l_set_time(unsigned char hour, unsigned char minute, unsigned char second)
{
  write_clock(R_SECOND, ((second / 10) << 4) + second % 10);
  write_clock(R_MINUTE, ((minute / 10) << 4) + minute % 10);
  write_clock(R_HOUR, ((hour / 10) << 4) + hour % 10);
}
//������� ����� �����  
void init_clock (void)
{
    //write_clock(0x00, 0x00);
}