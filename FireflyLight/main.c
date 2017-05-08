/*
 * FireflyLight.c
 *
 * Created: 08.03.2017 12:18:04
 * Author : Dennis
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#define PRESCALER_MAX 32

typedef struct  
{
	volatile uint16_t position;
	volatile uint8_t prescaler;
	volatile uint8_t count;
	volatile uint8_t doSave;
} sine_t;

volatile sine_t sineA;
volatile sine_t sineB;

/* on the fly calc:
//#define PI 3.14159265f
//value = (sinf(((float)count)*PI/180.f)+1.)*254./2.;  //254 to limit pwm at top
//OCR0A = (uint8_t)lrint(value);

jsfiddle:
var count = 0;
var a=[];
for (var i=0; i<360; i++) {
	var value = (Math.sin(count*3.14159265/180)+1)*254/2;
	console.log(count, value.toFixed(0));
	a.push(parseInt(value.toFixed(0), 10));
	count = (count+1)%360;
}

var pad = function(number) {
	if (number < 10) {
		return "  " + number;
	}
	if (number < 100) {
		return " " + number;
	}
	return number;
}

var res = "";
for (var i=0; i<360; i++) {
	res += pad(a[i]) + ", ";
	if (i != 0 && (i+1) % 20 == 0) {
		res += "\n";
	}
}
console.log(res);
*/

const uint8_t sine[360] PROGMEM = {
	127, 129, 131, 134, 136, 138, 140, 142, 145, 147, 149, 151, 153, 156, 158, 160, 162, 164, 166, 168,
	170, 173, 175, 177, 179, 181, 183, 185, 187, 189, 190, 192, 194, 196, 198, 200, 202, 203, 205, 207,
	209, 210, 212, 214, 215, 217, 218, 220, 221, 223, 224, 226, 227, 228, 230, 231, 232, 234, 235, 236,
	237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 246, 247, 248, 248, 249, 250, 250, 251, 251, 252,
	252, 252, 253, 253, 253, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 253, 253, 253, 252,
	252, 252, 251, 251, 250, 250, 249, 248, 248, 247, 246, 246, 245, 244, 243, 242, 241, 240, 239, 238,
	237, 236, 235, 234, 232, 231, 230, 228, 227, 226, 224, 223, 221, 220, 218, 217, 215, 214, 212, 210,
	209, 207, 205, 203, 202, 200, 198, 196, 194, 192, 191, 189, 187, 185, 183, 181, 179, 177, 175, 173,
	170, 168, 166, 164, 162, 160, 158, 156, 153, 151, 149, 147, 145, 142, 140, 138, 136, 134, 131, 129,
	127, 125, 123, 120, 118, 116, 114, 112, 109, 107, 105, 103, 101,  98,  96,  94,  92,  90,  88,  86,
	 84,  81,  79,  77,  75,  73,  71,  69,  67,  65,  64,  62,  60,  58,  56,  54,  52,  51,  49,  47,
	 45,  44,  42,  40,  39,  37,  36,  34,  33,  31,  30,  28,  27,  26,  24,  23,  22,  20,  19,  18,
	 17,  16,  15,  14,  13,  12,  11,  10,   9,   8,   8,   7,   6,   6,   5,   4,   4,   3,   3,   2,
	  2,   2,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   2,
	  2,   2,   3,   3,   4,   4,   5,   6,   6,   7,   8,   8,   9,  10,  11,  12,  13,  14,  15,  16,
	 17,  18,  19,  20,  22,  23,  24,  26,  27,  28,  30,  31,  33,  34,  36,  37,  39,  40,  42,  44,
  	 45,  47,  49,  51,  52,  54,  56,  58,  60,  62,  63,  65,  67,  69,  71,  73,  75,  77,  79,  81,
     84,  86,  88,  90,  92,  94,  96,  98, 101, 103, 105, 107, 109, 112, 114, 116, 118, 120, 123, 125
};

ISR(TIMER1_COMPA_vect)
{
	if (sineA.count == 0)
	{
		OCR0A = pgm_read_byte(&(sine[sineA.position]));
		OCR0B = OCR0A;
		sineA.position = (sineA.position+1)%360;
	}
	
	if (sineB.count == 0)
	{
		OCR1B = pgm_read_byte(&(sine[sineB.position]));
		sineB.position = (sineB.position+1)%360;
	}
	
	sineA.count = (sineA.count+1) % sineA.prescaler;
	sineB.count = (sineB.count+1) % sineB.prescaler;
}

ISR(INT0_vect)
{
	if (PINB & (1<<PB3))
	{
		sineA.prescaler = (sineA.prescaler+2) % PRESCALER_MAX;
		sineA.doSave = 1;
	}
	else
	{
		sineB.prescaler = (sineB.prescaler+2) % PRESCALER_MAX;
		sineB.doSave = 1;
	}
}

void sleep()
{
	ADCSRA &= ~(1<<ADEN); //disable adc
	ACSR = 0x80; // disable analouge comp
	//MCUCSR |= (1<<JTD); // disable jtag
	
	//MCUCR &= ~0x3;
	//sei();
	//MCUCR |= (1<<SM1) | (1<<SM0); // Power down
	MCUCR |= (1<<SE); // Sleep enable
	asm("sleep");
	MCUCR &= ~(1<<SE); // Sleep disable
}

int main(void)
{
	DDRB |= (1<<PB0) | (1<<PB1) | (1<<PB4);
	PORTB |= (1<<PB2) | (1<<PB3); //pullup int0, select jumper
	TCCR0A |= (1<<COM0A1);// | (1<<COM0B1);
	TCCR0A |= (1<<COM0B1) | (1<<COM0B0); //inverted B
	TCCR0A |= (1<<WGM01) | (1<<WGM00); //fast PWM
	TCCR0B |= (1<<CS00); //prescaler 0
	OCR0A = 0;
	OCR0B = 0xFF;

	TCCR1 |= (1<<CS10);// | (1<<CS12); //prescaler 0
	OCR1A = 0xFF;
	TIMSK |= (1<<OCIE1A); //int on compare match
	
	GTCCR |= (1<<PWM1B); //pwm output
	GTCCR |= (1<<COM1B1) | (1<<COM1B1); //output mode
	OCR1B = 128;
	OCR1C = 0xFF;
	
	MCUCR |= (1<<ISC01); //falling edge
	GIMSK |= (1<<INT0);
	
	//power reduction
	PRR |= (1<<PRUSI) | (1<<PRADC);
	
	sineA.count = 0;
	sineA.position = 0;
	sineA.doSave = 0;
	sineB.count = 0;
	sineB.position = 0;
	sineB.doSave = 0;
	
	sineA.prescaler = 16;
	sineB.prescaler = 10;
	
	eeprom_busy_wait();
	uint8_t val = eeprom_read_byte((uint8_t*)0x00);
	if (val >= 0x00 && val < 0xFF)
	{
		sineA.prescaler = val;
	}
	eeprom_busy_wait();
	val = eeprom_read_byte((uint8_t*)0x01);
	if (val >= 0x00 && val < 0xFF)
	{
		sineB.prescaler = val;
	}
	
	sei();
	
    /* Replace with your application code */
    while (1) 
    {
		if (sineA.doSave)
		{
			eeprom_busy_wait();
			eeprom_update_byte((uint8_t*)0x00, sineA.prescaler);
			sineA.doSave = 0;
		}
		
		if (sineB.doSave)
		{
			eeprom_busy_wait();
			eeprom_update_byte((uint8_t*)0x01, sineB.prescaler);
			sineB.doSave = 0;
		}
		sleep();
    }
}

