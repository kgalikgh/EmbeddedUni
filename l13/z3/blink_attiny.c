#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define LED PB2
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BTN PA0
#define BTN_DDR DDRA
#define BTN_PORT PORTA
#define BTN_PIN PINA

void timer1_init()
{
  ICR1 = 1249;
  TIMSK1 = _BV(OCIE1A);
  TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS11);
}

static volatile uint8_t vals[100] = {0};
static volatile uint8_t counter = 0;

ISR(TIM1_COMPA_vect)
{
  if(vals[counter])
    LED_PORT |= _BV(LED);
  else
    LED_PORT &= ~_BV(LED);
  if(!(BTN_PIN & _BV(BTN)))
  {
    vals[counter] = 1;
  }
  else
  {
    vals[counter] = 0;
  }
  if(++counter>=100) counter = 0;
} 

int main() {
  BTN_DDR = 0x0;
  BTN_PORT |= _BV(BTN);
  LED_DDR |= _BV(LED);
  LED_PORT |= _BV(LED);
  
  timer1_init();
  sei();
  set_sleep_mode(SLEEP_MODE_IDLE);
  while (1) {
    sleep_mode();
  }
}

