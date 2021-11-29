#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define A 1 << 0
#define B 1 << 1
#define C 1 << 2
#define D 1 << 3
#define E 1 << 4
#define F 1 << 5
#define G 1 << 6
#define DP 1 << 7

#define LA PB1
#define OE PB2

static const uint8_t nums[10] =
 {
     G | DP,
     A | D | E | F | G | DP,
     C | F | DP,
     E | F | DP,
     A | D | E | DP,
     B | E | DP,
     B | DP,
     D | E | G | F | DP,
     DP,
     E | DP
 };

// inicjalizacja SPI
void spi_init()
{
    // ustaw piny MOSI, SCK i ~SS jako wyjścia
    DDRB |= _BV(DDB3) | _BV(DDB5);
    // włącz SPI w trybie master z zegarem 250 kHz
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(DORD) | _BV(SPR1);
}

// transfer jednego bajtu
uint8_t spi_transfer(uint8_t data)
{
    // rozpocznij transmisję
    SPDR = data;
    // czekaj na ukończenie transmisji
    while (!(SPSR & _BV(SPIF)));
    // wyczyść flagę przerwania
    SPSR |= _BV(SPIF);
    // zwróć otrzymane dane
    return SPDR;
}

void timer1_init(){
   // ustaw tryb licznika
   // WGM1  = 0100 -- CTC top=OCR1A
   // CS1   = 100  -- prescalar 256
   // wzór: datasheet 20.12.3 str. 164
   // częstotliwość 16e6/1024*(1+15624) = 1 Hz
   // OCR1A  = 15624
   OCR1A = 15624;
   TCCR1B |= _BV(WGM12) | _BV(CS12) | _BV(CS10);
   TIMSK1 |= _BV(OCIE1A); //Timer/Counter1, Output Compare A Match Interrupt Enable
 }

int main()
{
  // zainicjalizuj SPI
  DDRB |= _BV(PB5);
  spi_init();
  uint8_t counter = 0;
  sei();
  while(1) {
      _delay_ms(1000);
      PORTB |= _BV(OE);
      spi_transfer(~nums[counter]);
      PORTB |= _BV(LA);
      _delay_ms(0.01);
      PORTB &= ~_BV(LA);
      PORTB &= ~_BV(OE); 
      if(++counter >= 10) counter = 0;
  }
}

