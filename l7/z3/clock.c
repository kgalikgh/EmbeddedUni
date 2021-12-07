#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include "i2c.h"
#include <string.h>
#include <util/delay.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

// inicjalizacja UART
void uart_init()
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // wyczyść rejestr UCSR0A
  UCSR0A = 0;
  // włącz odbiornik i nadajnik
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream)
{
  // czekaj aż transmiter gotowy
  while(!(UCSR0A & _BV(UDRE0)));
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}

FILE uart_file;

const uint8_t clock_addr = 0xd0;

#define i2cCheck(code, msg) \
if ((TWSR & 0xf8) != (code)) { \
    printf(msg " failed, status: %.2x\r\n", TWSR & 0xf8); \
    i2cReset(); \
    return -1; \
}

uint8_t i2cRead(uint16_t addr){
    i2cStart();
    i2cCheck(0x08, "I2C start")
    i2cSend(clock_addr | ((addr & 0x100) >> 7));
    i2cCheck(0x18, "I2C EEPROM write request")
    i2cSend(addr & 0xff);
    i2cCheck(0x28, "I2C EEPROM set address")        
    i2cStart();
    i2cCheck(0x10, "I2C second start")
    i2cSend(clock_addr | 0x1 | ((addr & 0x100) >> 7));
    i2cCheck(0x40, "I2C EEPROM read request")
    uint8_t data = i2cReadNoAck();
    i2cCheck(0x58, "I2C EEPROM read")
    i2cStop();
    i2cCheck(0xf8, "I2C stop")
    return data;
}

uint8_t i2cWrite(uint16_t addr, uint8_t val){
    i2cStart();
    i2cCheck(0x08, "I2C start")
    i2cSend(clock_addr | ((addr & 0x100) >> 7));
    i2cCheck(0x18, "I2C EEPROM write request")
    i2cSend(addr & 0xff);
    i2cCheck(0x28, "I2C EEPROM set address")
    i2cSend(val);
    i2cCheck(0x28, "I2C EEPROM send data")
    i2cStop();
    i2cCheck(0xf8, "I2C stop")
    return 0;
}

void getTime(){
    uint8_t hour = read(0x02),
            min = read(0x01),
            sec = read(0x00);
    printf("%c%x:%c%x:%c%x\r\n",(hour < 16) ? '0' : 0,hour,(min < 16) ? '0' : 0,min,(sec < 16) ? '0' : 0,sec);
}

void getDate(){
    uint8_t day = read(0x04),
            month = read(0x05),
            year = read(0x06);
    printf("%c%x-%c%x-20%c%x\r\n",(day < 16) ? '0' : 0,day,(month < 16) ? '0' : 0,month,(year < 16) ? '0' : 0,year);
}

void setTime(uint8_t h, uint8_t m, uint8_t s){
    if(h != -1){
        _delay_us(20);
        send(0x02,h);
    }
    if(m != -1){
        _delay_us(20);
        send(0x01,m);
    }
    if(s != -1){
        _delay_us(20);
        send(0x00,s);
    }
}

void setDate(uint8_t d, uint8_t m, uint8_t y){
    if(d != -1){
        _delay_us(20);
        send(0x04,d);
    }
    if(m != -1){
        _delay_us(20);
        send(0x05,m);
    }
    if(y != -1){
        _delay_us(20);
        send(0x06,y);
    }
}

void parseSetTime(char val[]){//HH:MM:SS
    uint8_t hour    = (val[0]-'0')*16 + (val[1]-'0'),
            min     = (val[3]-'0')*16 + (val[4]-'0'),
            sec     = (val[6]-'0')*16 + (val[7]-'0');
    setTime(hour,min,sec);
}
void parseSetDate(char val[]){//DD-MM-YYYY
    uint8_t day     = (val[0]-'0')*16 + (val[1]-'0'),
            month   = (val[3]-'0')*16 + (val[4]-'0'),
            year    = (val[8]-'0')*16 + (val[9]-'0');
    setDate(day,month,year);
}

int main()
{
  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  i2cInit();

//   char* TIME = __TIME__;
//   char* DATE = __DATE__;

//   printf("%s %s\r\n",TIME,DATE);//19:57:47 Dec  5 2021
  getTime();
  parseSetTime(__TIME__);

  while(1) {
    char command[10];
    scanf("%s",command);
    printf("%s ", command);
    if(strcmp(command,"date")==0){
        printf("\n\r");
        getDate();
    }
    if(strcmp(command,"time")==0){
        printf("\n\r");
        getTime();
    }
    if(strcmp(command,"set")==0){
        char s[10], val[20];
        scanf("%s %s",s,val);
        printf("%s %s", command,val);
        if(strcmp(s,"date")==0){
            printf("\n\r");
            parseSetDate(val);
        }
        if(strcmp(s,"time")==0){
            printf("\n\r");
            parseSetTime(val);
        }    
    }
  }
}
