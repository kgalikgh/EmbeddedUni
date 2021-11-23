#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem
#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB
#define SIZE 200 
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
  ADMUX   = _BV(REFS0) | 0xe; // referencja AVcc, wejście BADNGAP 
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN) ; // włącz ADC 
}

FILE uart_file;

float variance(uint16_t tab[])
{
    float sum = 0.0;
    for(int i = 0; i < SIZE; i++)
    {
        sum += (float)tab[i];
    }
    float avg = (float)sum/SIZE;
    float accumulator = 0.0;
    for(int i = 0; i < SIZE; i++)
    {
        float x = (float)tab[i] - avg;
        accumulator += (x * x);
    }
    return accumulator/SIZE;
}
static uint16_t noNoiseRed[SIZE] = {0};
static uint16_t noiseRed[SIZE] = {0};
static volatile uint16_t counter = 0;
ISR(ADC_vect)
{
    noiseRed[counter++] = ADC;
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

  printf("Bez Noise Reduction\r\n");
  //Kalibracja
  for(int i = 0; i < 10; i++)
  {
      ADCSRA |= _BV(ADSC); // wykonaj konwersję
      while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
      ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
  }
  //Pomiar bez redukcji
  for(int i = 0; i < SIZE; i++)
  {
      ADCSRA |= _BV(ADSC); // wykonaj konwersję
      while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
      ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
      noNoiseRed[i] = ADC; // weź zmierzoną wartość (0..1023)
  }
  float var1 = variance(noNoiseRed);
  
  //Pomiar z redukcją
  ADCSRA |= _BV(ADIE);
  sei();
  set_sleep_mode(SLEEP_MODE_ADC);
  while(counter < SIZE)
  {
      ADCSRA |= _BV(ADSC);
      sleep_mode();
  }  
  ADCSRA &= ~_BV(ADIE);

  float var2 = variance(noiseRed);
  printf("Wariancja bez noise reduction: %f, wariancja z noise reduction: %f\r\n", var1,var2);
  set_sleep_mode(SLEEP_MODE_IDLE); 
  _delay_ms(5000);
  while(1){
     sleep_mode();
  }
}


