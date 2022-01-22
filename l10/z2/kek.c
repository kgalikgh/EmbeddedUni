#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem
#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB
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


// inicjalizacja ADC
void adc_init()
{
    ADMUX = _BV(REFS0);              // referencja 1.1V, wejście ADC0
    DIDR0 = _BV(ADC0D) | _BV(ADC1D); // wyłącz wejście cyfrowe na ADC0
    // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
    ADCSRA |= _BV(ADEN);                           // włącz ADC
}

void timer1_init()
{
    // ustaw tryb licznika
    // COM1A = 10   -- non-inverting mode
    // WGM1  = 1000 -- phase and frequency correct PWM top=ICR1
    // CS1   = 010  -- prescaler 64
    // ICR1  = 256
    // częstotliwość 16e6/(2 * N * TOP) =16e6/(2 * 64 * 256) = ~488 Hz
    ICR1 = 256;
    TCCR1A = _BV(COM1A1);
    TCCR1B = _BV(WGM13) | _BV(CS11) | _BV(CS10);
    // OVF CAPT interrupt
    TIMSK1 = _BV(TOIE1) | _BV(ICIE1);
    // ustaw pin OC1A (PB1) jako wyjście
    DDRB |= _BV(PB1);
}

uint32_t get_adc(uint8_t idx)
{
    ADMUX = _BV(REFS0) | (1 & idx); // referencja 1.1V, wejście ADC1
    ADCSRA |= _BV(ADSC);            // wykonaj konwersję
    while (!(ADCSRA & _BV(ADIF)))
        ;                // czekaj na wynik
    ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
    return ADC;
}

static volatile uint32_t ovf_val = 0;
static volatile uint32_t ovf_count = 0;
ISR(TIMER1_OVF_vect)
{
    ovf_val += get_adc(1);
    ovf_count++;
}

static volatile uint32_t capt_val = 0;
static volatile uint32_t capt_count = 0;
ISR(TIMER1_CAPT_vect)
{
    capt_val += get_adc(1);
    capt_count++;
}

int main()
{
    uart_init();
    adc_init();
    timer1_init();

    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    printf("Start!\r\n");

    sei();

    while (1)
    {
        cli();
        _delay_ms(10);
        uint32_t val = get_adc(0);

        OCR1A = val * ICR1 / 1024;

        ovf_val /= ovf_count;
        capt_val /= capt_count;
        uint32_t ovf = ovf_val * 5000lu / 1024lu;
        uint32_t capt = capt_val * 5000lu / 1024lu;
        printf("Wyniki pomiarów w środku otwarcia oraz zamknięcia tranzystora: %lumV %lumV, ilość pomiarów: %lu\r\n", ovf, capt, ovf_count);
        ovf_count = 0;
        capt_count = 0;
        sei();
        _delay_ms(500);
    }
}
