#include <avr/io.h>
#include <util/delay.h>

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB


void rulerCycle()
{
    short status = 1;
    for(int i = 0; i < 8; i++)
    {
        PORTD = status;
        status <<= 1;
        _delay_ms(50);
    }
    status >>= 1;
    for(int i = 6; i >= 0; i--)
    {
        PORTD = status;
        status >>= 1;
        _delay_ms(50);
    }
}

int main() {
    UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);
    DDRD = 0xff;
    while(1)
    {
        rulerCycle();
    }
} 
