#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>


#define BTN PC4
#define BTN_DDR DDRC
#define BTN_PORT PORTC
#define BTN_PIN PINC

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB


void setLED(int8_t state){
  if(state)
    LED_PORT |= _BV(LED);
  else
    LED_PORT &= ~_BV(LED);
}

int8_t isButtonPressed(uint8_t pin, uint8_t btn){
  static const int8_t waitTime = 10, tests = 100;
  int8_t count = 0;
  while(!(pin & btn) && tests > count){
    _delay_us(waitTime);
    count++;
  }
  return count==tests;
}

int8_t getState(uint64_t *tab, uint16_t place){
  return (tab[place>>6] & (uint64_t)(1ULL<<(place&63)))>=1;
}

void setState(uint64_t *tab, uint16_t place, uint8_t state){
  if(state)
    tab[place>>6] |= (uint64_t)(1ULL<<(place&63));
  else
    tab[place>>6] &= ~(uint64_t)(1ULL<<(place&63));
}

void timer1_init()
{
  // ustaw tryb licznika
  // WGM1  = 1100 -- CTC top=ICR1
  // CS1   = 010  -- prescalar 8
  // wzór: datasheet 20.12.3 str. 164
  // częstotliwość 16e6/8*(1+1999) ~=~ 1000 Hz
  // ICR1  = 1999
  ICR1 = 1999;
  TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS11);
  TIMSK1 = _BV(OCIE1A); //Timer/Counter1, Output Compare A Match Interrupt Enable
  sei();
}


ISR (TIMER1_COMPA_vect){
  static uint64_t cycle[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  static uint16_t read=0, write=1000;

  setLED(getState(cycle,read));
  setState(cycle,write,!(BTN_PIN & _BV(BTN)));

  if((++read)==1024)read=0;
  if((++write)==1024)write=0;
}

int main() {
  LED_DDR |= _BV(LED);
  BTN_DDR = 0b00000000;
  BTN_PORT = _BV(BTN);

  timer1_init();

  set_sleep_mode(SLEEP_MODE_IDLE);

  while (1) {
    sleep_mode();
  }
}
