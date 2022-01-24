#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

// inicjalizacja UART
void uart_init()
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
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

// inicjalizacja ADC
void adc_init()
{
  ADMUX   = _BV(REFS0) | _BV(MUX0); // referencja 1.1V, wejście ADC1
  DIDR0   = _BV(ADC1D); // wyłącz wejście cyfrowe na ADC0
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADIE);
  ADCSRA |= _BV(ADEN); // włącz ADC
}

FILE uart_file;

void timer1_init()
{
  // częstotliwość 16e6/(8*(1+249)) = 8000 Hz
  OCR1A = 249;
  TCCR1B |= _BV(WGM12) | _BV(CS11);
  TIMSK1 = _BV(OCIE1A);
}

static volatile uint16_t counter = 0;
static volatile double squareSum = 0.0;

ISR(TIMER1_COMPA_vect)
{
  ADCSRA |= _BV(ADSC); // wykonaj konwersję
}

ISR(ADC_vect)
{
  uint16_t v = ADC; // weź zmierzoną wartość (0..1023)
  double scaled = (double)v - 512.0;
  counter++;
  squareSum += scaled * scaled;  
}

float calculateAvg()
{
  double avg = sqrt(squareSum/counter);
  counter = 0; 
  squareSum = 0.0;
  return avg;
}

int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // zainicjalizuj ADC
  adc_init();
  // mierz napięcie
  timer1_init();
  sei();
  while(1) {
    _delay_ms(500);
    cli();
    printf("counter: %"PRIu16" sum: %f\r\n", counter, squareSum);
    double v = 20.0 * log10f(calculateAvg()/512.0);
    printf("avg: %lf\r\n", v);
    sei();
  }
}


