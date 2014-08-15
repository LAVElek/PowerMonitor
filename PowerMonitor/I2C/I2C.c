#define F_CPU 20000000UL
//Набор функций для программной реализации I2C
#include "I2C.h"


void i2start(void)
{
  DDR_I2C  |=  _BV(SDA);
  PORT_I2C |=  _BV(SDA); _delay_us(pause);
  PORT_I2C |=  _BV(SCL); _delay_us(pause);
  PORT_I2C &= ~_BV(SDA); _delay_us(pause);
  PORT_I2C &= ~_BV(SCL); _delay_us(pause);
  DDR_I2C  &= ~_BV(SDA);
}

void i2write(unsigned char byt)
{
  unsigned char k;
  
  DDR_I2C |= _BV(SDA);
  for (k = 0; k < 8; k++)
  {
    if(0x01 & (byt >> (7 - k))) PORT_I2C |= _BV(SDA);
    else PORT_I2C &= ~_BV(SDA);
    _delay_us(pause);
    PORT_I2C |=  _BV(SCL); _delay_us(pause);
    PORT_I2C &= ~ _BV(SCL); _delay_us(pause);
  }
  DDR_I2C &= ~_BV(SDA);
}

void i2stop(void)
{
  DDR_I2C |= _BV(SDA);
  PORT_I2C &= ~_BV(SDA); _delay_us(pause);
  PORT_I2C |=  _BV(SCL); _delay_us(pause);
  PORT_I2C |= _BV(SDA); _delay_us(pause);
  DDR_I2C &= ~_BV(SDA);
}

unsigned char i2ack(void)
{
  unsigned char check, y;
  
  DDR_I2C |= _BV(SDA);
  PORT_I2C |= _BV(SDA); _delay_us(pause);
  DDR_I2C &= ~_BV(SDA);
  PORT_I2C |= _BV(SCL);
  for (y = 3; y > 0; y--)
  {
    _delay_us(pause);
    if(bit_is_clear(PIN_I2C, SDA)) break;//if ((check = SDA) == 0) break;
  }
  check = PIN_I2C & (1<<SDA);
  PORT_I2C &= ~ _BV(SCL); _delay_us(pause);
  return (check);
}

void i2mack(void)
{
  DDR_I2C |= _BV(SDA);
  PORT_I2C &= ~ _BV(SDA); _delay_us(pause);
  PORT_I2C |=  _BV(SCL); _delay_us(pause*2);
  PORT_I2C &= ~ _BV(SCL); _delay_us(pause);
  DDR_I2C &= ~_BV(SDA);
}

void i2nack(void)
{
  unsigned char y;
  
  DDR_I2C |= _BV(SDA);
  PORT_I2C |= _BV(SDA); _delay_us(pause);
  DDR_I2C &= ~_BV(SDA);
  PORT_I2C |=  _BV(SCL);
  for (y = 3; y > 0; y--)
  {
    _delay_us(pause);
    if(bit_is_set(PIN_I2C,SDA)) break;
  }
  PORT_I2C &= ~ _BV(SCL); _delay_us(pause);
}

unsigned char i2read(void)
{
  unsigned char k, d=0;
  
  DDR_I2C |= _BV(SDA);
  PORT_I2C |= _BV(SDA);
  DDR_I2C &= ~_BV(SDA);
  for (k = 0; k < 8; k++)
  {
    PORT_I2C |=  _BV(SCL); _delay_us(pause*2);
    d = d << 1;
    if(bit_is_set(PIN_I2C, SDA)) d |= 0x01;
    else                       d &= ~0x01;
    _delay_us(pause*2);
    PORT_I2C &= ~ _BV(SCL); _delay_us(pause*2);
  }
  return (d);
}

