#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

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

void io_init()
{
  // ustaw pull-up na PD2 (INT0)
  PORTD |= _BV(PORTD2);
  // ustaw wyzwalanie przerwania na INT0 i INT1 zboczem opadającym 
  EICRA |= _BV(ISC01);
  // odmaskuj przerwania dla INT0 i INT1
  EIMSK |= _BV(INT0);
}
// inicjalizacja ADC
void adc_init()
{
  ADMUX   = _BV(REFS0); // referencja 1.1V, wejście ADC0
  DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN); // włącz ADC
  ADCSRA |= _BV(ADIE);
  ADCSRA |= _BV(ADATE);
  ADCSRB |= _BV(ADTS1);
}

static volatile uint16_t lastVal = 0;

ISR(INT0_vect){
}

ISR(ADC_vect) {
  lastVal = ADC;
}

float toOhm(uint32_t adc)
{
    return (1023.0 - adc)/adc * 10000.0;
}

int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file; 
  // zainicjalizuj wejścia/wyjścia
  io_init();
  // zainicjalizuj adc
  adc_init();
  // odmaskuj przerwania
  sei();
  // program testowy
  ADCSRA |= _BV(ADSC);
  while(1) {
        printf("Ostatni odczyt: %f omów\r\n", toOhm(lastVal));
  }
}

