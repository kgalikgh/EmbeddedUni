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

void setBarChars()
{
    LCD_WriteCommand(0x40);
    uint8_t num = 0x0;
    for(int j = 0; j < 8; j++)
    {
      LCD_WriteData(num);
    }
    LCD_WriteCommand(0x48);
    num = 0x10;
    for(int j = 0; j < 8; j++)
    {
      LCD_WriteData(num);
    }
    LCD_WriteCommand(0x50);
    num = 0x18;
    for(int j = 0; j < 8; j++)
    {
      LCD_WriteData(num);
    }
    LCD_WriteCommand(0x58);
    num = 0x1c;
    for(int j = 0; j < 8; j++)
    {
      LCD_WriteData(num);
    }
    LCD_WriteCommand(0x60);
    num = 0x1e;
    for(int j = 0; j < 8; j++)
    {
      LCD_WriteData(num);
    }
    LCD_WriteCommand(0x68);
    num = 0x1f;
    for(int j = 0; j < 8; j++)
    {
      LCD_WriteData(num);
    }
}

int main()
{
  // skonfiguruj wyświetlacz
  LCD_Initialize();
  LCD_Clear();
  // skonfiguruj strumienie wyjściowe
  fdev_setup_stream(&hd44780_file, hd44780_transmit, NULL, _FDEV_SETUP_WRITE);
  stdout = stderr = &hd44780_file;
  setBarChars();
  LCD_Home();
  LCD_WriteData(0x0);
  LCD_WriteData(0x1);
  LCD_WriteData(0x2);
  LCD_WriteData(0x3);
  LCD_WriteData(0x4);
  LCD_WriteData(0x5);
  while(1)
  {
    LCD_Home();
    LCD_Clear();
    for(int i = 0; i < 16; i++)
    {
      for(int j = 0; j < 6; j++)
      {
        LCD_GoTo(i, 0);
        LCD_WriteData(j);
        _delay_ms(400);
      }
    }
  }
}


