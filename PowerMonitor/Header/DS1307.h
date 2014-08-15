
#define ADRESS 0b00110100 // ����� DS1307
#define SLA_R 0b11010001 // ����� DS1307 c ������������� ����� ������
#define SLA_W 0b11010000 // ����� DS1307 c ������������� ����� ������

#define DS1307_SPEED 50000UL // ������� ������ ����� 100kHz, ������������� � �������� ������� � 2 ����

// ��������
#define R_SECOND 0x00
#define R_MINUTE 0x01
#define R_HOUR 0x02
#define R_WEEK_DAY 0x03
#define R_DAY 0x04
#define R_MONTH 0x05
#define R_YEAR 0x06
#define R_CONTROL 0x07
#define R_BEGIN_DATA 0x08

// ����
#define CONTROL_OUT 7
#define CONTROL_SQWE 4
#define CONTROL_RS1 1
#define CONTROL_RS0 0

// ��� ������
#define MONDEY 1
#define TUESDAY 2
#define WEDNESDAY 3
#define THURSDAY 4
#define FRIDAY 5
#define SATURDAY 6
#define SUNDAY 7


extern void initDS1307(unsigned long MK_Speed);
extern void setTime(unsigned char hour, unsigned char minute, unsigned char second);
extern void setDate(unsigned char day, unsigned char month, unsigned char year, unsigned char day_week);
extern void getTime(unsigned char *time_buf);
extern void getDate(unsigned char *time_buf);