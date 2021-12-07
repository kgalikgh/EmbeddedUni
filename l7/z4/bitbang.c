#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

#define SS PD4
#define SCK PD7
#define MOSI PD5
#define MISO PD6

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

// inicjalizacja SPI
void spi_init()
{
    // ustaw piny MOSI, SCK i ~SS jako wyjścia
    DDRB |= _BV(DDB4);
    // włącz SPI w trybie master z zegarem 250 kHz
    SPCR = _BV(SPE) | _BV(SPR1);
}

// transfer jednego bajtu
uint8_t spi_transfer(uint8_t data)
{
    uint8_t byte_in = 0;
    PORTD &= ~_BV(SS);
    for(uint8_t bit = 0; bit < 8; bit++)
    {
      if(data & _BV(bit))
      {
        PORTD |= _BV(MOSI);
      }
      else
      {
        PORTD &= ~_BV(MOSI);
      }
      _delay_ms(0.01);
      PORTD |= _BV(SCK);
      
      if(PIND & _BV(MISO))
      {
        byte_in |= _BV(bit);
      }
      
      _delay_ms(0.01);
      PORTD &= ~_BV(SCK);
    }
    PORTD |= _BV(SS);
    return byte_in;
}

int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  DDRD |= _BV(SS) | _BV(MOSI) | _BV(SCK);
  DDRD &= ~_BV(MISO);
  PORTD |= _BV(MISO);
  // zainicjalizuj SPI
  spi_init();
  // program testujący połączenie
  uint8_t v = 0;
  while(1) {
    _delay_ms(500);
    uint8_t w = spi_transfer(v);
    printf("Shifted out: %x Shifted in: %x\r\n", v, w);
    v++;
  }
}
