#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

// inicjalizacja UART
void uart_init(){
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

void timer1_init(){
  TCCR1B |= _BV(ICES1); // Input Capture Edge Select to rising edge
  TIMSK1 |= _BV(ICIE1); // Input Capture Interrupt Enable

  // ustaw tryb licznika
  // WGM1  = 0100 -- CTC top=OCR1A
  // CS1   = 100  -- prescalar 256
  // wzór: datasheet 20.12.3 str. 164
  // częstotliwość 16e6/256*(1+62499) = 1 Hz
  // OCR1A  = 62499
  OCR1A = 62499;
  TCCR1B |= _BV(WGM12) | _BV(CS12);
  TIMSK1 |= _BV(OCIE1A); //Timer/Counter1, Output Compare A Match Interrupt Enable
}

volatile uint32_t Hz = 0;

ISR (TIMER1_COMPA_vect){
  printf("Odczytano: %u Hz\r\n",Hz);
  Hz = 0;
}

ISR(TIMER1_CAPT_vect){
  Hz++;
}

int main()
{
  sei();
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  timer1_init();


  DDRD |= _BV(PD2);
  set_sleep_mode(SLEEP_MODE_IDLE);
  while(1) {
    // sleep_mode();
    //PORTD ^= _BV(PD2);//miganie dioda skierowana na fotorezystor
    //_delay_us(1000);//zmieniac ta wartosc -> zmienia ilosc Hz na wyjsciu
  }
}
