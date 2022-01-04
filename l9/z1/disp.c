#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>
#include "hd44780.h"

int hd44780_transmit(char data, FILE *stream)
{
  LCD_WriteData(data);
  return 0;
}

FILE hd44780_file;

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

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}

FILE uart_file;

int main()
{
  // skonfiguruj wyświetlacz
  LCD_Initialize();
  LCD_Clear();
  // skonfiguruj strumienie wyjściowe
  fdev_setup_stream(&hd44780_file, hd44780_transmit, NULL, _FDEV_SETUP_WRITE);
  stdout = stderr = &hd44780_file;
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, NULL, uart_receive, _FDEV_SETUP_RW);
  stdin = &uart_file;
  // program testowy
  char c = 0;
  uint8_t xpos = 0;
  char line[17] = {0};
  // pierwsza linia
  while(1)
  {
    LCD_GoTo(xpos, 0);
    LCD_WriteData(0xff);
    LCD_GoTo(xpos, 0);
    c = getchar();
    if(c == '\r')
    {
      LCD_WriteData(0x20);
      xpos = 0;
      break;
    }
    LCD_WriteData(c);
    if(++xpos >= 16)
    {
      xpos = 0;
    }
  }
   
  // kolejne linie
  while(1)
  {
    LCD_GoTo(xpos, 1);
    LCD_WriteData(0xff);
    LCD_GoTo(xpos, 1);
    c = getchar();
    if(c == '\r')
    {
      LCD_WriteData(0x20);
      line[xpos] = '\0';
      xpos = 0;
      LCD_Home();
      LCD_Clear();
      char* ptr = line;
      while(*ptr != '\0')
      {
        LCD_WriteData(*ptr);
        ptr++;
      }
      continue;
    }
    LCD_WriteData(c);
    line[xpos] = c;
    if(++xpos >= 16)
    {
      xpos = 0;
    }

  } 
}


