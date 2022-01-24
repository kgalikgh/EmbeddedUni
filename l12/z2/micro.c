#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <math.h>

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
  ADMUX   = _BV(REFS0) | _BV(MUX0); // referencja AVcc, wejście ADC1
  DIDR0   = _BV(ADC1D); // wyłącz wejście cyfrowe na ADC1
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN) | _BV(ADIE); // włącz ADC + interrupt
}

FILE uart_file;

void timer1_init(){
  // ustaw tryb licznika
  // WGM1  = 0100 -- CTC top=OCR1A
  // CS1   = 001  -- prescalar 1
  // wzór: datasheet 20.12.3 str. 164
  // częstotliwość 16e6/1*(1+1999) = 8000 Hz
  // OCR1A  = 1999
  OCR1A = 1999;
  TCCR1B |= _BV(WGM12) | _BV(CS10);
  TIMSK1 |= _BV(OCIE1A); //Timer/Counter1, Output Compare A Match Interrupt Enable
}

ISR (TIMER1_COMPA_vect){
  ADCSRA |= _BV(ADSC); // wykonaj konwersję
}

volatile float squareSum = 0.0;
volatile uint32_t count = 0;

float squareAverage(){
  return sqrt(squareSum/count);
}

ISR(ADC_vect){
  //dBFS = 20*log(measure / MAXval)
  // float tmp = 20.0 * log10f((float)ADC/1023.0);
  float tmp = ADC - 512.0;//okolo 2.5V w ADC
  squareSum += tmp*tmp;
  count++;
  // printf("Odczytano: %"PRIu16"\r\n", ADC);
}

int main()
{
  sei();
  timer1_init();
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // zainicjalizuj ADC
  adc_init();
  // mierz napięcie

  set_sleep_mode(SLEEP_MODE_IDLE);
  while(1) {
    // sleep_mode();

    printf("dB = %f\r\n", 20.0 * log10f(squareAverage()/512.0));
    count = 0;
    squareSum = 0.0;
    _delay_ms(200);
  }
}

