#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define LED PC5
#define LED_DDR DDRC
#define LED_PORT PORTC

#define BTN PD2
#define BTN_PIN PIND
#define BTN_PORT PORTD
#define BTN_DDR DDRD

void timer1_init()
{
  //Częstotliwość 100Hz
  ICR1 = 19999;
  TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS11); //CNC, prescaler = 8
  TIMSK1 = _BV(OCIE1A); //Output Compare A Match Interrupt Enable
}


ISR(TIMER1_COMPA_vect)
{
    static uint8_t vals[100] = {0};
    static uint8_t counter = 0;
    LED_PORT = vals[counter] << PC5;
    vals[counter] = !(BTN_PIN & _BV(BTN));
    if(++counter >= 100) counter = 0;
}

int main() {
  BTN_DDR = 0x0;
  BTN_PORT |= _BV(BTN);
  LED_DDR |= _BV(LED);
  timer1_init();
  sei();

  set_sleep_mode(SLEEP_MODE_IDLE);
  while (1) {  
    sleep_mode();
  }
}
