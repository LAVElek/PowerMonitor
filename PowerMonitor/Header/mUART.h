// работа с UART

#define mUART_UBRR(speed, xtalCpu) ((xtalCpu/((speed) * 16l)) - 1)

#define mUART_BUFFER_SIZE 100
#define DELAY_FOR_WAIT_DATA 50

extern void mUART_init(unsigned int baudrate);
extern void mUART_putc(char data);
extern void mUART_puts(char* string);
extern void mUART_puti(const int value);
extern void mUART_putu(const unsigned int value);
extern void mUART_putl(const long int value);