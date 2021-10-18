#include <avr/io.h>
#include <util/delay.h>

#define A 1 << 0
#define B 1 << 1
#define C 1 << 2
#define D 1 << 3
#define E 1 << 4
#define F 1 << 5
#define G 1 << 6

static const short nums[10] = 
{
    G,
    A | D | E | F | G,
    C | F,
    E | F,
    A | D | E,
    B | E,
    B,
    D | E | G | F,
    0,
    E
};

int main() {
    UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);
    DDRD = 0xff;
    while(1)
    {
        for(int i = 0; i < 10; i++)
        {
            PORTD = nums[i];
            _delay_ms(1000);
        }
    }
} 
