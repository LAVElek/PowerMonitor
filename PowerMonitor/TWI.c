#include <avr/interrupt.h>
#include "Header/TWI.h"

// ����� ��� ���������
volatile static unsigned char twiBuf[TWI_BUFFER_SIZE];

// ���-�� ������������ ����
volatile static unsigned char twiMsgSize;

// ������ ��������
volatile static unsigned char twiState = TWI_NO_STATE;

unsigned char tekByte;

void TWI_MasterInit(unsigned int TWI_speed, // ������� ���� TWI
                    unsigned long MK_speed) // ������� ����������������
{
  TWSR = (0 << TWPS1) | (0 << TWPS0); // ������������ ��� ��������� ������� ����� 1
  TWBR = (MK_speed / TWI_speed - 16) / 2; // ������ ������� ���� TWI
}

/**************************************
**����� �� � ������ ������ TWI ������**
***************************************/
unsigned char TWI_IsBusy()
{
  return (TWCR & (1 << TWIE)); 
}

/**************************************
*****���������� ������ TWI ������******
***************************************/
unsigned char TWI_GetState()
{
  while (TWI_IsBusy());
  return twiState;
}

/**************************************
**********�������� ���������***********
***************************************/
void TWI_SendData(unsigned char *msg, // ��������� - ������ ����
                  unsigned char msgSize) // ����� ���������
{
  while(TWI_IsBusy()); // ���� ������������ TWI ������
  
  twiMsgSize = msgSize;
  for(unsigned char i = 0; i < twiMsgSize; i++)
    twiBuf[i] = msg[i];
  
  twiState = TWI_NO_STATE;
  TWCR = (1 << TWEN) | // ��������� ������ TWI ������
         (1 << TWIE) | // ��������� ����������
         (1 << TWINT) | // ���������� ����������
         (1 << TWSTA); // ��������� �����
}

/**************************************
**********��������� ���������***********
***************************************/
unsigned char TWI_GetData(unsigned char *msg,
                          unsigned char msgSize)
{
  while(TWI_IsBusy()); // ���� ������������ TWI ������
  
  if (twiState == TWI_SUCCESS) // ���� ��������� ������� �������
  {
    for(unsigned char i = 0; i < msgSize; i++)
    {
      msg[i] = twiBuf[i];
    }
  }

  return twiState;
}

/**************************************
*********���������� ����������*********
***************************************/
ISR(TWI_vect)
{
  unsigned char stat;
  
  stat = TWSR & 0xF8;

  switch(stat)
  {
    case TWI_START:  // ��������� ����� ������������
    case TWI_REP_START: // ��������� ��������� ����� ������������
      tekByte = 0;
      // ���� break, �.� ����� ��������� � �������� ������� ����
    case TWI_SLA_W_ACK:      // ��������� ����� � ����� ������ � �������� ACK
    case TWI_SEND_BYTE_ACK:  // ������� ���� � �������� �������������
      if (tekByte < twiMsgSize) // ���� ��� �� ��� ���������
      {
        TWDR = twiBuf[tekByte];  // ��������� ��������� ����
        TWCR = (1 << TWEN) | // ��������� ������ TWI ������
               (1 << TWIE) | // ��������� ����������
               (1 << TWINT);  // ���������� ������� ����������
        tekByte++;
      }
      else
      {
        twiState = TWI_SUCCESS;
        TWCR = (1 << TWEN) | // ��������� ������ TWI ������
               (1 << TWINT) | // ���������� ������� ����������
               (1 << TWSTO) | // ��������� ��������� ����
               (0 << TWIE); // ��������� ����������
      }
      break;

    case TWI_RECEIVE_BYTE_ACK: // ���� ������ � �������� ACK
      twiBuf[tekByte] = TWDR;
      tekByte++;
    // ���� break, �.�. ����� ��������� ���� ����� ������� ��� ���������� �������� ���
    // ���� �������� ����� ���������� ����� ��������� ��� ����� ������� SLA+R
    case TWI_SLA_R_ACK:  // ������� ����� � ����� �� ������ � �������� ACK
      if (tekByte < twiMsgSize - 1) // ���� ������� �� ������������� ���
      {
        TWCR = (1 << TWEN) | // ��������� ������ TWI ������
               (1 << TWIE) | // ��������� ����������
               (1 << TWINT) | // ��������� ������� ����������
               (1 << TWEA);  // ��������� ������������ ��������� �������� ����
      }
      else // � ���� ������� ������������� ��� �� ��� ��������� ���������� ������������� �� ������
      {
        TWCR = (1 << TWEN) | // ��������� ������ TWI ������
               (1 << TWIE) | // ��������� ����������
               (1 << TWINT); // ��������� ������� ����������
      }
      break;
    
    case TWI_RECEIVE_BYTE_NACK: // ���� ������ � �������� NACK
      twiBuf[tekByte] = TWDR;
      twiState = TWI_SUCCESS;
      TWCR = (1 << TWEN) | // ��������� ������ TWI ������
             (0 << TWIE) | // ��������� ����������
             (1 << TWSTO); // ��������� ������ ����
      break;

    case TWI_FAIL:
    case TWI_SLA_W_NACK:
    case TWI_SEND_BYTE_NACK:
    case TWI_SLA_R_NACK:
    default:
      twiState = stat;
      TWCR = (1 << TWEN) | 
             (0 << TWIE) |
             (0 << TWINT) |
             (0 << TWEA) |
             (0 << TWSTA) | 
             (0 << TWSTO) |
             (0 << TWWC);
  }
}