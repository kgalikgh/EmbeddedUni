#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <time.h>
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
void print_int8()
{
    printf("Printing for uint8_t\r\n");
    int8_t a;
    scanf("%"SCNd8, &a);
    int8_t b;
    scanf("%"SCNd8, &b);

    printf("%"PRId8 "\r\n", a+b);
    printf("%"PRId8 "\r\n", a*b);
    printf("%"PRId8 "\r\n", a/b);
}

void print_int16()
{
    printf("Printing for uint16_t\r\n");
    int16_t a;
    scanf("%"SCNd16, &a);
    int16_t b;
    scanf("%"SCNd16, &b);

    printf("%"PRId16 "\r\n", a+b);
    printf("%"PRId16 "\r\n", a*b);
    printf("%"PRId16 "\r\n", a/b);
}

void print_int32()
{
    printf("Printing for uint32_t\r\n");
    int32_t a;
    scanf("%"SCNd32, &a);
    int32_t b;
    scanf("%"SCNd32, &b);

    printf("%"PRId32"\r\n", a+b);
    printf("%"PRId32"\r\n", a*b);
    printf("%"PRId32"\r\n", a/b);
}

void print_int64()
{
    printf("Printing for uint64_t\r\n");
    int64_t a;
    scanf("%ld", &a);
    int64_t b;
    scanf("%ld", &b);
    printf("%ld\r\n", a+b);
    printf("%ld\r\n", a*b);
    printf("%ld\r\n", a/b);
}

void print_float()
{
    printf("Printing for float\r\n");
    float a;
    scanf("%f", &a);
    float b;
    scanf("%f", &b);
    printf("%f\r\n", a+b);
    printf("%f\r\n", a*b);
    printf("%f\r\n", a/b);
}

FILE uart_file;

int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // program testowy
  print_int8();
  print_int16();
  print_int32();
  print_int64();
  print_float();
  while(1) {
  }
}

