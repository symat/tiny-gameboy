/*
 * Define a very minimal Two-wire-interface library instead of using e.g. Wire, as
 * we only need this to work on ATTiny85 and we only need I2C master mode.
 * 
 */


#include <avr/io.h>

#ifndef __SH1106__
#define __SH1106__

#define WIDTH 132
#define HEIGHT 64

// the SH1106 has two 7bit I2C address, 60 (0111100) or 61 (0111101)
#define ADDRESS_AND_WRITE 0b01111000
#define ADDRESS_AND_READ 0b01111001

#define I2C_DELAY 5

// Co = 1, D/C = 0 - the next two bytes are data byte and an other control byte
#define SH1106_COMMANDS 0b10000000

// Co = 0, D/C = 0 - the next is the last control byte, only data bytes will follow
#define SH1106_LAST_COMMAND 0b00000000


void i2c_write_data(uint8_t *data, uint8_t len);
void init_display();
void i2c_ack();
void i2c_transfer();
void i2c_start_write();
void i2c_stop();

void init_display() {
  USICR=(1<<USIWM1)|(1<<USICS1)|(1<<USICLK);  //TWI mode
  DDRB=(1<<PB0)|(1<<PB2); //SDA & SCL direction as output
  PORTB=(1<<PB0)|(1<<PB2); //SDA & SCL default state

  const uint8_t data[6] = {
    SH1106_COMMANDS,
    0xAE,                 // Display off
    SH1106_COMMANDS, 
    0xA1,                 // Flip horizontal (segment re-map: left)
    SH1106_LAST_COMMAND, 
    0xAF                  // Display on
  };
  i2c_write_data(data, sizeof(data));
}


void i2c_ack()
{
  DDRB&=~(1<<PB0);                               //Change direction of SDA to receive acknowledge bit
  USISR|=(1<<USICNT3)|(1<<USICNT2)|(1<<USICNT1); //Set counter to 1110 to force it to overflow when ACK bit is received
  i2c_transfer();                                //Byte transmission
  DDRB|=(1<<PB0);                                //Change direction of SDA to send data
}


void i2c_transfer()
{
  do {
    USICR|=(1<<USITC);      //Clock signal from SCL
    while((PINB&(1<<PB2))); //Waiting for SCL to go high
    delayMicroseconds(I2C_DELAY);
    USICR|=(1<<USITC);  //Toggle SCL to go low
    delayMicroseconds(I2C_DELAY);
  } while(!(USISR&(1<<USIOIF)));  //Repeat clock generation at SCL until the counter overflows and a byte is transferred
  delayMicroseconds(I2C_DELAY);
  USISR|=(1<<USIOIF);      //Clear overflow flag
}

void i2c_write_data(uint8_t *data, uint8_t len)
{
  i2c_start_write();
  for(uint8_t i=0; i < len; i++) {
    USIDR=data[i];     //Placing byte in Data register
    i2c_transfer();    //Transfer the data placed in register      
    i2c_ack();
  }
  i2c_stop();
}


void i2c_start_write()
{
  PORTB&=~(1<<PB0);          //Pulling SDA line low
  delayMicroseconds(I2C_DELAY);
  PORTB&=~(1<<PB2);          //Pulling SLC line low
  delayMicroseconds(I2C_DELAY);
  while(USISIF==1);          //detection of start condition
  USIDR = ADDRESS_AND_WRITE; //address of slave and select write operation
  i2c_transfer();
  i2c_ack();
}



void i2c_stop()
{
  PORTB|=(1<<PB2);       //Pulling SDA high 
  delayMicroseconds(I2C_DELAY);
  PORTB|=(1<<PB0);       //Pulling SCL low
  delayMicroseconds(I2C_DELAY);
}

#endif __SH1106__
