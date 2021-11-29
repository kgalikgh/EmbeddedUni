#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

#define BUFFER_SIZE 32 

static volatile char rx_buffer[BUFFER_SIZE] = {0};
static volatile char tx_buffer[BUFFER_SIZE] = {0};
static volatile uint8_t rx = 0, irq_rx = 0;
static volatile uint8_t tx = 0, irq_tx = 0;
static volatile uint8_t tx_size = 0, rx_size = 0;
// inicjalizacja UART
void uart_init()
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // wyczyść rejestr UCSR0A
  UCSR0A = 0;
  // włącz odbiornik i nadajnik
  UCSR0B = _BV(RXCIE0) | _BV(TXCIE0) | _BV(UDRIE0) | _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream)
{
  // czekaj aż transmiter gotowy
  tx_buffer[tx] = data;
  if(++tx >= BUFFER_SIZE)
    tx = 0;
  tx_size++;
  tx_buffer[tx] = 0;
  UCSR0B |= _BV(UDRIE0);
  while(tx_size == BUFFER_SIZE);
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
  // czekaj aż znak dostępny
  while (rx_size == 0);
  char c = rx_buffer[rx];
  if(++rx >= BUFFER_SIZE)
      rx = 0;
  rx_size--;
  return c;
}


ISR(USART_RX_vect)
{
    rx_buffer[irq_rx] = UDR0;
    if(++irq_rx >= BUFFER_SIZE)
        irq_rx = 0;
    rx_size++;
}

ISR(USART_TX_vect)
{
    tx_size--;
}

ISR(USART_UDRE_vect)
{
  if(tx_size == 0 || tx_buffer[irq_tx] == '\0')
  {
    UCSR0B &= ~_BV(UDRIE0);
  }
  else
  {
    UDR0 = tx_buffer[irq_tx];
    if(++irq_tx >= BUFFER_SIZE)
        irq_tx = 0;
  }
}

FILE uart_file;

int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  sei();
  set_sleep_mode(SLEEP_MODE_IDLE);
  printf("Hello World!\r\n");
  printf("The quick brown fox jumps over the lazy dog\r\n");
  while(1) {
    //sleep_mode();
    printf("Write values for a and b\r\n");
    uint8_t a,b;
    scanf("%" SCNu8 " %" SCNu8, &a, &b);

    printf("+: %" PRIu8 "\r\n *: %" PRIu8 "\r\n/: %" PRIu8 "\r\n\r\n", a+b,a*b,a/b);

  }
}
