#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem
#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB
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

static volatile uint32_t measure_ovf = 0;
static volatile uint32_t measure_capt = 0;
static volatile uint32_t counter = 0;

ISR(TIMER1_OVF_vect)
{
  ADMUX = _BV(MUX0) | _BV(REFS0);
  ADCSRA |= _BV(ADSC); // wykonaj konwersję
  while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
  ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
  uint16_t v = ADC; // weź zmierzoną wartość (0..1023)
  measure_ovf += v; 
  counter++;
}

ISR(TIMER1_COMPA_vect)
{
  ADMUX = _BV(MUX0) | _BV(REFS0);
  ADCSRA |= _BV(ADSC); // wykonaj konwersję
  while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
  ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
  uint16_t v = ADC; // weź zmierzoną wartość (0..1023)
  measure_capt += v; 
}    
     
     
     
void timer1_init()
{
  // ustaw tryb licznika
  // COM1A = 10   -- non-inverting mode
  // WGM1  = 1110 -- fast PWM top=ICR1
  // CS1   = 101  -- prescaler 1024
  // ICR1  = 15624
  // częstotliwość 16e6/8*(1+1999) = 1000 Hz
  // wzór: datasheet 20.12.3 str. 164
  ICR1 = 1999;
  TCCR1A = _BV(COM1A1) | _BV(WGM11);
  TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS11);
  TIMSK1 = _BV(TOIE1) | _BV(OCIE1A);
  // ustaw pin OC1A (PB1) jako wyjście
  DDRB |= _BV(PB1);
}

// inicjalizacja ADC
void adc_init()
{
  ADMUX   = _BV(REFS0); // referencja 1.1V, wejście ADC0
  DIDR0   = _BV(ADC0D) | _BV(ADC1D); // wyłącz wejście cyfrowe na ADC0 i ADC1
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
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  printf("Hello\r\n");
  sei();
  while(1)
  {
    if(counter >= 200)
    {
      cli();
      ADMUX = _BV(REFS0);
      ADCSRA |= _BV(ADSC); // wykonaj konwersję
      while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
      ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
      uint16_t v = ADC; // weź zmierzoną wartość (0..1023)
      OCR1A = (uint16_t)(v * 1999.0 / 1024.0);
      printf("v = %"PRIu16 "\r\n", v);
      printf("CAPT Measure: %"PRIu32 "\r\n", (measure_capt / 200) * 5000 / 1024);
      printf("OV Measure: %"PRIu32 "\r\n", (measure_ovf / 200) * 5000 / 1024);
      counter = measure_capt = measure_ovf = 0;
      sei();
    }
  }
}

