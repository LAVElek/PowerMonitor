#include <avr/io.h>
#include "header/mUART.h"

void mUART_init(unsigned int baudrate)
{
  UBRRH = (unsigned char)(baudrate>>8);
  UBRRL = (unsigned char) baudrate;
  UCSRC = ( 1 << URSEL ) // ��� ������� � �������� UCSRC ���������� ��� URSEL.
          | ( 0 << UCSZ2 ) | ( 1 << UCSZ1 ) | ( 1 << UCSZ0 ) // ���������� ��� ������ ����� 8
          | ( 0 << UPM1 ) | ( 0 << UPM0 ) // ��������� �������� ��������
          | ( 0 << USBS ); // 1 ���� ���
  UCSRB = (1 << RXCIE)    // ��������� ���������� �� ������
          | (1 << RXEN)   // ��������� �����
          | (1 << TXEN);  // ��������� ��������
}

void mUART_putc(char data)
{
  while (!(UCSRA & (1 << UDRE)) ); //�������� ����������� ������ ������
  UDR = data; //������ �������� ������	
}

void mUART_puts(char *string)
{
  /*
  ����� ������ ������������ \0
  ��� ��� �� \0 �� true
  */
  while(*string)
  {
    mUART_putc(*string);
    string++;
  }
}

// ������� � mUART int
void mUART_puti(const int value)
{
  char buf[10];

  mUART_puts(itoa(value, buf, 10));
}

// ������� � UART unsigned int
void mUART_putu(const unsigned int value)
{
  char buf[10];

  mUART_puts(utoa(value, buf, 10));
}

// ������� � mUART long int
void mUART_putl(const long int value)
{
  char buf[15];

  mUART_puts(ltoa(value, buf, 10));
}
