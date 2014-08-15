#ifndef DS1307_H
#define DS1307_H

#include <avr/io.h>

// регистры
#define R_SECOND 0x00
#define R_MINUTE 0x01
#define R_HOUR 0x02
#define R_WEEK_DAY 0x03
#define R_DAY 0x04
#define R_MONTH 0x05
#define R_YEAR 0x06
#define R_CONTROL 0x07
#define R_BEGIN_DATA 0x08

// биты
#define CONTROL_OUT 7
#define CONTROL_SQWE 4
#define CONTROL_RS1 1
#define CONTROL_RS0 0

// дни недели
#define MONDEY 1
#define TUESDAY 2
#define WEDNESDAY 3
#define THURSDAY 4
#define FRIDAY 5
#define SATURDAY 6
#define SUNDAY 7

#include "../I2C/I2C.h"

void init_clock (void);
unsigned char l_get_second();
unsigned char l_get_minute();
unsigned char l_get_hour();
void l_set_time(unsigned char hour, unsigned char minute, unsigned char second);

#endif