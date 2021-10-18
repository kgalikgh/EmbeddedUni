#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>
#include <inttypes.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem
#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

#define DOT_DELAY 500 
#define DASH_DELAY 2000
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

static const char* mappings[] = {
        ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", 
        "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.."
  };

void text2morse()
{
    printf("Reading: ");
    char text[256];
    scanf("%256s", text);
    printf("\r\n");
    printf("Writing in morse: %s\r\n", text);
    char* c = text;
    while(*c != '\0')
    {
        const char* morse = mappings[(int)(*c - 'a')];
        while(*morse != '\0')
        {
            LED_PORT |= _BV(LED);
            if(*morse == '.')
            {
                _delay_ms(DOT_DELAY);
            }
            else
            {
                _delay_ms(DASH_DELAY);
            }
            LED_PORT &= ~_BV(LED);
            _delay_ms(500);
            morse++;
        }
        c++;
    }
    printf("Done\r\n");
}

int main() {
  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  LED_DDR |= _BV(LED);
  text2morse();
  while (1) {
  }
}

