// коды состояний
#define TWI_FAIL              0x00  // ошибка на шине
#define TWI_START             0x08  // был сделан старт
#define TWI_REP_START         0x10  // был сделан повторный старт

#define TWI_SLA_W_ACK         0x18  // отправили адрес с битом записи и получили ACK
#define TWI_SLA_W_NACK        0x20  // отправили адрес с битом записи и получили NACK
#define TWI_SEND_BYTE_ACK     0x28  // послали байт и получили подтверждение
#define TWI_SEND_BYTE_NACK    0x30  // послали байт, а подтверждение не получили

#define TWI_SLA_R_ACK         0x40  // послали адрес с битом на чтение и получили ACK
#define TWI_SLA_R_NACK        0x48  // послали адрес с битом на чтение и получили NACK
#define TWI_RECEIVE_BYTE_ACK  0x50  // байт принят и передано ACK
#define TWI_RECEIVE_BYTE_NACK 0x58  // байт принят и передано NACK

#define TWI_BUFFER_SIZE 10

#define TWI_SUCCESS 0xff  // передача завершена успешно
#define TWI_NO_STATE 0xf8 // неопределенное состояние

extern void TWI_MasterInit(unsigned int TWI_speed, unsigned long MK_speed);
extern unsigned char TWI_GetState();
extern void TWI_SendData(unsigned char *msg, unsigned char msgSize);
extern unsigned char TWI_GetData(unsigned char *msg, unsigned char msgSize);