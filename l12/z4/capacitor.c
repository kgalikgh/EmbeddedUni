#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <util/delay.h>

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
  ADMUX   = _BV(REFS0); // referencja Vcc, wejście ADC0
  DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN); // włącz ADC
}

FILE uart_file;

double current(uint16_t v1, uint16_t v2)
{
  double  conv1 = (v1 * 5.0) / 1024.0; 
  double  conv2 = (v2 * 5.0) / 1024.0; 
  double  temp = (conv2-conv1) * 10.0; //10 = 10e6  / 10e5 * 10e-6
  return 130.0/temp;
}
//I = C * dV/dt
//A = C * V/(S * 10e-6)
//C[uF] = A[uA] * dt[uS] / dV[uV]
//C[uF] = A[uA] * dt[uS] / dV[uV] * 10e3 
//C[F] = A[A]/ (dV[V]/dt[S])
//C[uF] = A[uA]/(dV[uV] * 10e5)

int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // zainicjalizuj ADC
  adc_init();
  uint16_t v1 = 0, v2 = 0;
  // pierwszy pomiar 
  ADCSRA |= _BV(ADSC); // wykonaj konwersję
  while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
  ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
  v2 = ADC; // weź zmierzoną wartość (0..1023)
  uint16_t counter = 0;
  while(1) {
    _delay_us(100000);
    v1 = v2;
    ADCSRA |= _BV(ADSC); // wykonaj konwersję
    while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
    ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
    v2 = ADC; // weź zmierzoną wartość (0..1023)
    if(v2 <= 805)
      printf("ADC: %"PRIu16" Capacitance: %lfuF\r\n",v2, current(v1,v2));
  }
}


