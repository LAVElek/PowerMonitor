// ���� ���������
#define TWI_FAIL              0x00  // ������ �� ����
#define TWI_START             0x08  // ��� ������ �����
#define TWI_REP_START         0x10  // ��� ������ ��������� �����

#define TWI_SLA_W_ACK         0x18  // ��������� ����� � ����� ������ � �������� ACK
#define TWI_SLA_W_NACK        0x20  // ��������� ����� � ����� ������ � �������� NACK
#define TWI_SEND_BYTE_ACK     0x28  // ������� ���� � �������� �������������
#define TWI_SEND_BYTE_NACK    0x30  // ������� ����, � ������������� �� ��������

#define TWI_SLA_R_ACK         0x40  // ������� ����� � ����� �� ������ � �������� ACK
#define TWI_SLA_R_NACK        0x48  // ������� ����� � ����� �� ������ � �������� NACK
#define TWI_RECEIVE_BYTE_ACK  0x50  // ���� ������ � �������� ACK
#define TWI_RECEIVE_BYTE_NACK 0x58  // ���� ������ � �������� NACK

#define TWI_BUFFER_SIZE 10

#define TWI_SUCCESS 0xff  // �������� ��������� �������
#define TWI_NO_STATE 0xf8 // �������������� ���������

extern void TWI_MasterInit(unsigned int TWI_speed, unsigned long MK_speed);
extern unsigned char TWI_GetState();
extern void TWI_SendData(unsigned char *msg, unsigned char msgSize);
extern unsigned char TWI_GetData(unsigned char *msg, unsigned char msgSize);