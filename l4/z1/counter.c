#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

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

void timer1_init()
{
  // ustaw tryb licznika
  // WGM1  = 0000 -- normal
  // CS1   = 001  -- prescaler 1
  TCCR1B = _BV(CS10);
}
FILE uart_file;

void int8test()
{
    printf("ready (int8)\r\n");
    volatile int8_t a,b,c;
    scanf("%"SCNd8" %"SCNd8, &a,&b);
    TCNT1 = 0;
    uint16_t beg = TCNT1; // wartość licznika przed czekaniem
    c = a + b;
    uint16_t end = TCNT1; // wartość licznika po czekaniu
    printf("value: %"PRId8" time in cycles: %"PRIu16"\r\n", c, end-beg);
    beg = TCNT1; // wartość licznika przed czekaniem
    c = a * b;
    end = TCNT1; // wartość licznika po czekaniu
    printf("value: %"PRId8" time in cycles: %"PRIu16"\r\n", c, end-beg);
    beg = TCNT1; // wartość licznika przed czekaniem
    c = a / b;
    end = TCNT1; // wartość licznika po czekaniu
    printf("value: %"PRId8" time in cycles: %"PRIu16"\r\n", c, end-beg);
}
void int16test()
{
    printf("ready (int16)\r\n");
    volatile int16_t a,b,c;
    scanf("%"SCNd16" %"SCNd16, &a,&b);
    TCNT1 = 0;
    uint16_t beg = TCNT1; // wartość licznika przed czekaniem
    c = a + b;
    uint16_t end = TCNT1; // wartość licznika po czekaniu
    printf("value: %"PRId16" time in cycles: %"PRIu16"\r\n", c, end-beg);
    beg = TCNT1; // wartość licznika przed czekaniem
    c = a * b;
    end = TCNT1; // wartość licznika po czekaniu
    printf("value: %"PRId16" time in cycles: %"PRIu16"\r\n", c, end-beg);
    beg = TCNT1; // wartość licznika przed czekaniem
    c = a / b;
    end = TCNT1; // wartość licznika po czekaniu
    printf("value: %"PRId16" time in cycles: %"PRIu16"\r\n", c, end-beg);
}
void int32test()
{
    printf("ready (int32)\r\n");
    volatile int32_t a,b,c;
    scanf("%"SCNd32" %"SCNd32, &a,&b);
    TCNT1 = 0;
    uint16_t beg = TCNT1; // wartość licznika przed czekaniem
    c = a + b;
    uint16_t end = TCNT1; // wartość licznika po czekaniu
    printf("value: %"PRId32" time in cycles: %"PRIu16"\r\n", c, end-beg);
    beg = TCNT1; // wartość licznika przed czekaniem
    c = a * b;
    end = TCNT1; // wartość licznika po czekaniu
    printf("value: %"PRId32" time in cycles: %"PRIu16"\r\n", c, end-beg);
    beg = TCNT1; // wartość licznika przed czekaniem
    c = a / b;
    end = TCNT1; // wartość licznika po czekaniu
    printf("value: %"PRId32" time in cycles: %"PRIu16"\r\n", c, end-beg);
}
void float_test()
{
    printf("ready (float)\r\n");
    volatile float a,b,c;
    scanf("%f %f", &a,&b);
    TCNT1 = 0;
    uint16_t beg = TCNT1; // wartość licznika przed czekaniem
    c = a + b;
    uint16_t end = TCNT1; // wartość licznika po czekaniu
    printf("value: %f time in cycles: %"PRIu16"\r\n", c, end-beg);
    beg = TCNT1; // wartość licznika przed czekaniem
    c = a * b;
    end = TCNT1; // wartość licznika po czekaniu
    printf("value: %f time in cycles: %"PRIu16"\r\n", c, end-beg);
    beg = TCNT1; // wartość licznika przed czekaniem
    c = a / b;
    end = TCNT1; // wartość licznika po czekaniu
    printf("value: %f time in cycles: %"PRIu16"\r\n", c, end-beg);
}

int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // zainicjalizuj licznik
  timer1_init();
  // program testowy
  while(1) {
    int8test();
    int16test();
    int32test();
    float_test();
    _delay_ms(1000);
  }
}
