#include <avr/interrupt.h>
#include "Header/TWI.h"

// буфер для сообщения
volatile static unsigned char twiBuf[TWI_BUFFER_SIZE];

// кол-во передаваемых байт
volatile static unsigned char twiMsgSize;

// статус передачи
volatile static unsigned char twiState = TWI_NO_STATE;

unsigned char tekByte;

void TWI_MasterInit(unsigned int TWI_speed, // частота шины TWI
                    unsigned long MK_speed) // частота микроконтроллера
{
  TWSR = (0 << TWPS1) | (0 << TWPS0); // предделитель для настройки частоты равен 1
  TWBR = (MK_speed / TWI_speed - 16) / 2; // задаем частоту шины TWI
}

/**************************************
**Занят ли в данный момент TWI модуль**
***************************************/
unsigned char TWI_IsBusy()
{
  return (TWCR & (1 << TWIE)); 
}

/**************************************
*****возвращает статус TWI модуля******
***************************************/
unsigned char TWI_GetState()
{
  while (TWI_IsBusy());
  return twiState;
}

/**************************************
**********передача сообщения***********
***************************************/
void TWI_SendData(unsigned char *msg, // сообщение - массив байт
                  unsigned char msgSize) // длина сообщения
{
  while(TWI_IsBusy()); // ждем освобождения TWI модуля
  
  twiMsgSize = msgSize;
  for(unsigned char i = 0; i < twiMsgSize; i++)
    twiBuf[i] = msg[i];
  
  twiState = TWI_NO_STATE;
  TWCR = (1 << TWEN) | // разрешаем работу TWI модулю
         (1 << TWIE) | // разрешаем прерывания
         (1 << TWINT) | // сбрасываем прерывание
         (1 << TWSTA); // формируем СТАРТ
}

/**************************************
**********получение сообщения***********
***************************************/
unsigned char TWI_GetData(unsigned char *msg,
                          unsigned char msgSize)
{
  while(TWI_IsBusy()); // ждем освобождения TWI модуля
  
  if (twiState == TWI_SUCCESS) // если сообщение успешно принято
  {
    for(unsigned char i = 0; i < msgSize; i++)
    {
      msg[i] = twiBuf[i];
    }
  }

  return twiState;
}

/**************************************
*********Обработчик прерываний*********
***************************************/
ISR(TWI_vect)
{
  unsigned char stat;
  
  stat = TWSR & 0xF8;

  switch(stat)
  {
    case TWI_START:  // состояние СТАРТ сформировано
    case TWI_REP_START: // состояние ПОВТОРНЫЙ СТАРТ сформировано
      tekByte = 0;
      // нету break, т.к сразу переходим к передаче массива байт
    case TWI_SLA_W_ACK:      // отправили адрес с битом записи и получили ACK
    case TWI_SEND_BYTE_ACK:  // послали байт и получили подтверждение
      if (tekByte < twiMsgSize) // если еще не все отправили
      {
        TWDR = twiBuf[tekByte];  // загружаем очередной байт
        TWCR = (1 << TWEN) | // разрешаем работу TWI модулю
               (1 << TWIE) | // разрешаем прерывания
               (1 << TWINT);  // сбрасываем текущее прерывание
        tekByte++;
      }
      else
      {
        twiState = TWI_SUCCESS;
        TWCR = (1 << TWEN) | // разрешаем работу TWI модуля
               (1 << TWINT) | // сбрасываем текущее прерывание
               (1 << TWSTO) | // формируем состояние стоп
               (0 << TWIE); // запрещаем прерывания
      }
      break;

    case TWI_RECEIVE_BYTE_ACK: // байт принят и передано ACK
      twiBuf[tekByte] = TWDR;
      tekByte++;
    // нету break, т.к. после получения бита нужно сказать как обработать следующи бит
    // теже действия нужно произвести после получения АСК после команды SLA+R
    case TWI_SLA_R_ACK:  // послали адрес с битом на чтение и получили ACK
      if (tekByte < twiMsgSize - 1) // если получен не предпоследний бит
      {
        TWCR = (1 << TWEN) | // разрешаем работу TWI модуля
               (1 << TWIE) | // разрешаем прерывания
               (1 << TWINT) | // сбрасыаем текущее прерывание
               (1 << TWEA);  // разрешаем подтверждать следующий принятый байт
      }
      else // а если получен предпоследний бит то при получении следующего подтверждение не делаем
      {
        TWCR = (1 << TWEN) | // разрешаем работу TWI модуля
               (1 << TWIE) | // разрешаем прерывания
               (1 << TWINT); // сбрасыаем текущее прерывание
      }
      break;
    
    case TWI_RECEIVE_BYTE_NACK: // байт принят и передано NACK
      twiBuf[tekByte] = TWDR;
      twiState = TWI_SUCCESS;
      TWCR = (1 << TWEN) | // разрешаем работу TWI модуля
             (0 << TWIE) | // разрешаем прерывания
             (1 << TWSTO); // формируем сигнал СТОП
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