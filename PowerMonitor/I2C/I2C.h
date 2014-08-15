#ifndef I2C_H
#define I2C_H

#include <avr/io.h>
#include <util/delay.h>

#define DDR_I2C  DDRB
#define PORT_I2C PORTB
#define PIN_I2C  PINB
#define SDA      PB1
#define SCL      PB0
#define pause    5

void i2start (void);
void i2stop (void);
void i2mack (void);
void i2nack (void);
void i2write (unsigned char);
unsigned char i2read (void);
unsigned char i2ack (void);

#endif
