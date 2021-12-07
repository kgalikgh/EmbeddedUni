#include <avr/io.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "i2c.h"

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

#define i2cCheck(code, msg) \
   if ((TWSR & 0xf8) != (code)) { \
     printf(msg " failed, status: %.2x\r\n", TWSR & 0xf8); \
     i2cReset(); \
     return; \
   }
//inicjalizacja UART
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

const uint8_t eeprom_addr = 0xa0;

void readOneByte(uint16_t addr)
{
   i2cStart();
   i2cCheck(0x08, "I2C start")
   i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
   i2cCheck(0x18, "I2C EEPROM write request")
   i2cSend(addr & 0xff);
   i2cCheck(0x28, "I2C EEPROM set address")
   i2cStart();
   i2cCheck(0x10, "I2C second start")
   i2cSend(eeprom_addr | 0x1 | ((addr & 0x100) >> 7));
   i2cCheck(0x40, "I2C EEPROM read request")
   uint8_t data = i2cReadNoAck();
   i2cCheck(0x58, "I2C EEPROM read")
   i2cStop();
   i2cCheck(0xf8, "I2C stop")
   printf("%.3x: %x\r\n", addr, data);
}

void writeOneByte(uint16_t addr, uint8_t val)
{
    i2cStart();
    i2cCheck(0x08, "I2C start")
    i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
    i2cCheck(0x18, "I2C EEPROM write request")
    i2cSend(addr & 0xff);
    i2cCheck(0x28, "I2C EEPROM set address")
    i2cSend(val);
    i2cCheck(0x28, "I2C EEPROM write data");
    i2cStop();
    i2cCheck(0xf8, "I2C stop")
    printf("Written %"PRIu8 " to addr: %"PRIu16"\r\n", val, addr);
}

void readLengthBytes(uint16_t addr, uint8_t length)
{
   uint8_t data = 0;
   uint16_t sum = length + (addr & 0xff) + ((addr >> 8) & 0xff);
   printf(":%.2x%.4x00", length, addr); //start code + byte count + address + record type
   i2cStart();
   i2cCheck(0x08, "I2C start")
   i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
   i2cCheck(0x18, "I2C EEPROM write request")
   i2cSend(addr & 0xff);
   i2cCheck(0x28, "I2C EEPROM set address")
   i2cStart();
   i2cCheck(0x10, "I2C second start")
   i2cSend(eeprom_addr | 0x1 | ((addr & 0x100) >> 7));
   i2cCheck(0x40, "I2C EEPROM read request")
   for(int i = 0; i < length -1; i++)
   {
    data = i2cReadAck();
    sum += data;
    printf("%.2x", data);
    i2cCheck(0x50, "I2C EEPROM read")
   }
   data = i2cReadNoAck();
   sum += data;
   printf("%.2x", data);
   i2cCheck(0x58, "I2C EEPROM read")
   i2cStop();
   i2cCheck(0xf8, "I2C stop")
   uint8_t checksum = ~(sum & 0xff) + 1;
   printf("%.2x\r\n", checksum);
}

uint8_t readByte()
{
    char c1 = getchar();
    if(c1 >= 'a') c1 = 10 + c1 - 'a';
    else c1 = (c1 - '0');
    char c2 = getchar();
    if(c2 >= 'a') c2 = 10 + c2 - 'a';
    else c2 = (c2 - '0');
    return c1 << 4 | c2;
}

void writeContinuously()
{
   while(1)
   {
        uint8_t bytes[64] = {0};
        char c = getchar(); //covers ':' 
        putchar(c);
        uint8_t length = readByte();
        printf("%.2x", length);
        uint8_t addr_high = readByte();
        printf("%.2x", addr_high);
        uint8_t addr_low = readByte();
        printf("%.2x", addr_low);
        uint16_t addr = addr_high << 8 | addr_low;
        uint8_t type = readByte();
        printf("%.2x", type);
        if(type == 1 && (length > 0))
        {
          printf("Invalid input\r\n");
          return;
        }
        
        uint16_t sum = type + addr_low + addr_high + length;
        for(int i = 0; i < length; i++)
        {
            bytes[i] = readByte();
            printf("%.2x", bytes[i]);
            sum += bytes[i];
        }
        uint8_t checksum = readByte();
        printf("%.2x\r\n", checksum);
        sum += checksum;
        if((sum & 0xff) != 0)
        {
          printf("Invalid checksum\r\n");
          return;
        }
        c = getchar();
        if(c != '\r')
        {
          printf("Invalid input\r\n");
          return;
        }
        if(type == 1)
        {
            printf("End of file\r\n");
            return;
        }
        i2cStart();
        i2cCheck(0x08, "I2C start")
        i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
        i2cCheck(0x18, "I2C EEPROM write request")
        i2cSend(addr & 0xff);
        i2cCheck(0x28, "I2C EEPROM set address")
        for(int i = 0; i < length; i++)
        {
            i2cSend(bytes[i]);
            i2cCheck(0x28, "I2C EEPROM write data");
        }
        i2cStop();
        i2cCheck(0xf8, "I2C stop")
        printf("Done\r\n");
    } 
}
void parse(char *cmd)
{
   uint8_t argnum = 0;
   char* args[3] = {0};
   int addr, length, val;
   char* token = strtok(cmd, " ");
   while(token)
   {
       argnum++;
       args[argnum-1] = token;
       token = strtok(NULL, " ");
   }

   if(strcmp(args[0], "read") == 0)
   {
       switch(argnum)
       {
           case 3:
               addr = atoi(args[1]);
               length = atoi(args[2]);
               readLengthBytes(addr & 0xffff, length & 0xff);
               break;

           case 2:
               addr = atoi(args[1]);
               readOneByte(addr & 0xffff);
               break;

           default:
               printf("Incorrect command\r\n");
               break;
       }
   }
   else if(strcmp(args[0], "write") == 0)
   {
       switch(argnum)
       {
           case 3:
               addr = atoi(args[1]);
               val = atoi(args[2]);
               writeOneByte(addr & 0xffff, val & 0xff);
               break;
           case 1:
               writeContinuously();
               break;
           default:
               printf("Incorrect command\r\n");
               break;
       }
   }
   else
   {
       printf("Incorrect command\r\n");
   }
}

int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // zainicjalizuj I2C
  i2cInit();
  // program testowy
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
    cmd[k] = '\0';
    parse(cmd);
    printf("done\r\n");
  }
}

