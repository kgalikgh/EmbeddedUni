#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include "i2c.h"
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
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
  UCSR0B =  _BV(RXEN0) | _BV(TXEN0);
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
uint8_t i2cReadByte(uint16_t addr){
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

uint8_t i2cWriteByte(uint16_t addr, uint8_t val){
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

void getDate()
{
  uint8_t days = i2cReadByte(0x4);
  uint8_t month = i2cReadByte(0x5);
  uint8_t year = i2cReadByte(0x6);
  printf("%.2x-%.2x-2%.3x\r\n", days, month, year);
}

void getTime()
{
  uint8_t seconds = i2cReadByte(0x0);
  uint8_t minutes = i2cReadByte(0x1);
  uint8_t hours = i2cReadByte(0x2);
  printf("%.2x:%.2x:%.2x\r\n",hours, minutes, seconds);
}

void setTime(char *time)
{
    unsigned int seconds, minutes, hours;
    hours = ((time[0]-'0') << 4) + (time[1]-'0');
    minutes = ((time[2]-'0') << 4) + (time[3]-'0');
    seconds = ((time[5]-'0') << 4) + (time[6]-'0');
    i2cWriteByte(0x0, seconds);
    _delay_ms(0.01);
    i2cWriteByte(0x1, minutes);
    _delay_ms(0.01);
    i2cWriteByte(0x2, hours);
    _delay_ms(0.01);
}


void setDate(char *date)
{
    unsigned int days, months, years;
    days = ((date[0]-'0') << 4) + (date[1]-'0');
    months = ((date[2]-'0') << 4) + (date[3]-'0');
    years = (date[5]-'0') << 12  | (date[6]-'0') << 8 | (date[7]-'0') << 4 | (date[8]-'0');
    i2cWriteByte(0x4, days);
    _delay_ms(0.01);
    i2cWriteByte(0x5, months);
    _delay_ms(0.01);
    i2cWriteByte(0x6, years);
    _delay_ms(0.01);
}

void parse(char *cmd)
{ 
   uint8_t argnum = 0;
   char* args[3] = {0};
   char* token = strtok(cmd, " ");
   while(token)
   {
       argnum++;
       args[argnum-1] = token;
       token = strtok(NULL, " ");
   }

   if(strcmp(args[0], "date") == 0)
   {
      if(argnum != 1)
      {
          printf("Incorrect command\r\n");
          return;
      } 
      getDate();
   }
   else if(strcmp(args[0], "time") == 0)
   {
      if(argnum != 1)
      {
          printf("Incorrect command\r\n");
          return;
      } 
      getTime();
   }
   else if(strcmp(args[0], "set") == 0)
   {
        if(argnum != 3)
        {
          printf("Incorrect command\r\n");
          return;
        }
        if(strcmp(args[1], "date") == 0)
        {
           setDate(args[2]);
        }
        else if(strcmp(args[1], "time") == 0)
        {
          setTime(args[2]);
        }
   }
   else
   {
       printf("Incorrect command\r\n");
   } 
}



int main()
{
  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  i2cInit();
  char cmd[64] = {0};
  while(1) {
    printf("ready\r\n");
    char c = getchar();
    uint8_t k = 0;
    while(c != '\r')
    {
        cmd[k++] = c;
        c = getchar();
    }
    cmd[k] = '\0';parse(cmd);
    printf("done\r\n");}
}
