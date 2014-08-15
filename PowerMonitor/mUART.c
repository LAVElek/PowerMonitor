#include <avr/io.h>
#include "header/mUART.h"

void mUART_init(unsigned int baudrate)
{
  UBRRH = (unsigned char)(baudrate>>8);
  UBRRL = (unsigned char) baudrate;
  UCSRC = ( 1 << URSEL ) // для доступа к регистру UCSRC выставляет бит URSEL.
          | ( 0 << UCSZ2 ) | ( 1 << UCSZ1 ) | ( 1 << UCSZ0 ) // количество бит данных равно 8
          | ( 0 << UPM1 ) | ( 0 << UPM0 ) // запрещаем контроль четности
          | ( 0 << USBS ); // 1 стоп бит
  UCSRB = (1 << RXCIE)    // разрешаем прерывание по приему
          | (1 << RXEN)   // разрешаем прием
          | (1 << TXEN);  // разрешаем передачу
}

void mUART_putc(char data)
{
  while (!(UCSRA & (1 << UDRE)) ); //Ожидание опустошения буфера приема
  UDR = data; //Начало передачи данных	
}

void mUART_puts(char *string)
{
  /*
  конец строки сигнаизирует \0
  все что не \0 то true
  */
  while(*string)
  {
    mUART_putc(*string);
    string++;
  }
}

// выводит в mUART int
void mUART_puti(const int value)
{
  char buf[10];

  mUART_puts(itoa(value, buf, 10));
}

// выводит в UART unsigned int
void mUART_putu(const unsigned int value)
{
  char buf[10];

  mUART_puts(utoa(value, buf, 10));
}

// выводит в mUART long int
void mUART_putl(const long int value)
{
  char buf[15];

  mUART_puts(ltoa(value, buf, 10));
}
