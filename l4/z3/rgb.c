#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

#define LED_PORT PORTC
#define RED PC2
#define GREEN PC1
#define BLUE PC0
#define LED_DDR DDRC
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
  ADMUX   = _BV(REFS0);  // referencja AVcc, wejście BADNGAP 
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN); // włącz ADC
}

FILE uart_file;

void timer1_init()
{
  // ustaw tryb licznika
  // COM1A = 10   -- non-inverting mode
  // WGM1  = 1110 -- fast PWM top=ICR1
  // CS1   = 101  -- prescaler 1024
  // ICR1  = 15624
  // częstotliwość 16e6/(1024*(1+15624)) = 1 Hz
  // wzór: datasheet 20.12.3 str. 164
  ICR1 = 15624;
  TCCR1A = _BV(COM1A1) | _BV(WGM11);
  TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS10);
  // ustaw pin OC1A (PB1) jako wyjście
  DDRB |= _BV(PB1);
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
  // uruchom licznik
  timer1_init();
  // ustaw wypełnienie 50%
  // mierz napięcie
  LED_DDR |= _BV(RED) | _BV(BLUE) | _BV(GREEN);

  ADCSRA |= _BV(ADSC); // wykonaj konwersję
  while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
  ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
  uint32_t v = ADC; // weź zmierzoną wartość (0..1023)
  srand(v);
  OCR1A = ICR1;
  uint16_t vals[25] = {1,3,7,15,30,61,122,244,488,1953,3906,7812,15624,7812,3906,1953,488,244,122,61,30,15,7,3,1};
  uint8_t r,g,b;
  r=g=b=0;
  while(1) {  
    // Wybór koloru
    uint32_t val = rand() % 360;
    if(val < 60)
    {
        r = 255;
        b = 0;
        g = ((val%60) * 255)/60;
    }
    else if(val < 120)
    {
        g = 255;
        b = 0;
        r = ((val%60) * 255)/60;

    }
    else if(val < 180)
    {
        g = 255;
        r = 0;
        b = ((val%60) * 255)/60;

    }
    else if(val < 240)
    {
        b = 255;
        r = 0;
        g = ((val%60) * 255)/60;

    }
    else if(val < 300)
    {
        b = 255;
        g = 0;
        r = ((val%60) * 255)/60;

    }
    else if(val < 360)
    {
        r = 255;
        g = 0;
        b = ((val%60) * 255)/60;

    }

    for(int i = 0; i < 25; i++) {
        while(TCNT1 != 0);
        OCR1A  = vals[i];
        for(int j = 0; j < 250; j++)
        {
            PORTC = 0; 
            for(int i = 0; i < 256; i++)
            {
                if(i > r)
                    PORTC |= _BV(RED);
                if(i > b)
                    PORTC |= _BV(BLUE);
                if(i > g)
                    PORTC |= _BV(GREEN);
                _delay_ms(0.0001);
            }
        }
    
    }
  }
}


