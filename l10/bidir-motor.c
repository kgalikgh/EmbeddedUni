#include <avr/io.h>
#include <inttypes.h>

#define A1 PD6
#define A2 PD5

void timer1_init()
{
  // ustaw tryb licznika
  // COM1A = 10   -- non-inverting mode
  // WGM1  = 1110 -- fast PWM top=ICR1
  // CS1   = 102  -- prescaler 1024
  // ICR1  = 15624
  // częstotliwość 16e6/8*(1+1999) = 1000 Hz
  // wzór: datasheet 20.12.3 str. 164
  ICR1 = 1999;
  TCCR1A = _BV(COM1A1) | _BV(WGM11);
  TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS11);
  // ustaw pin OC1A (PB1) jako wyjście
  DDRB |= _BV(PB1);
}

// inicjalizacja ADC
void adc_init()
{
  ADMUX   = _BV(REFS0); // referencja AVcc, wejście ADC0
  DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN); // włącz ADC
}

int main()
{
  // uruchom licznik
  timer1_init();
  // inicjalizacja adc
  adc_init();
  DDRD |= _BV(A1) | _BV(A2);
  while(1)
  {
    ADCSRA |= _BV(ADSC); // wykonaj konwersję
    while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
    ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
    uint16_t v = ADC; // weź zmierzoną wartość (0..1023)
    if(v >= 512)
    {
      v -= 512;
      PORTD &= ~_BV(A1);
      PORTD |= _BV(A2);
    }
    else
    {
      v = 511 - v;
      PORTD |= _BV(A1);
      PORTD &= ~_BV(A2);
    }
    OCR1A = (uint16_t)(v * 1999.0 / 511.0);
  }
}
