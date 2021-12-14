#include "FreeRTOS.h"
#include "task.h"
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "queue.h"
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#ifndef BAUD
#define BAUD 9600
#endif
#include <util/setbaud.h>

#define BUFF_SIZE 32

int uart_transmit(char c, FILE *stream);
int uart_receive(FILE *stream);

FILE uart_file = FDEV_SETUP_STREAM(uart_transmit, uart_receive, _FDEV_SETUP_RW);
static QueueHandle_t input;
static QueueHandle_t output;

void uart_init() {
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;
#if USE_2X
  UCSR0A |= _BV(U2X0);
#else
  UCSR0A &= ~(_BV(U2X0));
#endif
  UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
  UCSR0B = _BV(UDRIE0) | _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0); /* Enable RX and TX */
  input = xQueueCreate(BUFF_SIZE, sizeof(uint8_t));
  output = xQueueCreate(BUFF_SIZE, sizeof(uint8_t));
  sei();
}

ISR(USART_RX_vect)
{
  uint8_t c = UDR0;
  xQueueSendFromISR(output, &c, NULL);
}

ISR(USART_UDRE_vect)
{
  uint8_t c;
  if(xQueueReceiveFromISR(input, &c, NULL))
  {
    UDR0 = c;
  }
  else
  {
    UCSR0B &= ~_BV(UDRIE0);
  }
}

int uart_transmit(char c, FILE *stream) {
  xQueueSend(input, (void*) &c, portMAX_DELAY);
  UCSR0B |= _BV(UDRIE0);
  return 0;
}

int uart_receive(FILE *stream) {
  char c;
  xQueueReceive(output, (void*) &c, portMAX_DELAY);
  return c;
}

