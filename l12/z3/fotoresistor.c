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

FILE uart_file;

void timer1_init()
{
  //16e6/(1024 * (1 + 15624)) = 1Hz
  TCCR1B |= _BV(ICES1);
  TIMSK1 |= _BV(ICIE1) | _BV(OCIE1A);
  TCCR1B = _BV(WGM12) | _BV(CS10) | _BV(CS12);
  OCR1A = 15624; 
}

static volatile uint16_t counter = 0;
ISR(TIMER1_COMPA_vect)
{
  printf("Freq: %"PRIu16"\r\n",counter); 
  counter = 0;
}

ISR(TIMER1_CAPT_vect)
{
  counter++;
}


int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // zainicjalizuj ADC
  timer1_init();
  DDRD = _BV(PD7);
  sei();
  while(1) {
    //PORTD ^= _BV(PD7);
    //_delay_us(2000);
  }
}


