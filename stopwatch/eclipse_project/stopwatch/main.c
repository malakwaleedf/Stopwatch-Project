/*
 * main.c
 *
 *  Created on: Sep 14, 2024
 *      Author: Malak Waleed
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "macros.h"

/*time in seconds variable*/
/*should be 0 as an initial value but due to a glitch in proteus simulation
 * it works well when the initial value is -2
 * -2 -> 2^16 - 2 (unsigned)
 */
unsigned long time = 0xFFFFFFFE;

/*variables representing the time digits*/
/*sec1 -> seconds least significant digit
 *sec2 -> seconds most significant digit
 *min1 -> minutes least significant digit
 *min2 -> minutes most significant digit
 *hr1 -> hours least significant digit
 *hr2 -> hours most significant digit
 */
unsigned char sec1 = 0;
unsigned char sec2 = 0;
unsigned char min1 = 0;
unsigned char min2 = 0;
unsigned char hr1 = 0;
unsigned char hr2 = 0;

/*variable representing the counting mode of the stop watch*/
/*mode_toggle = 0 -> counting up
 *mode_toggle = 1 -> counting down
 */
unsigned char mode_toggle = 0;

/*variable indicates if the stopwatch is paused*/
/*paused = 0 -> not paused
  paused = 1 -> paused
 */
unsigned char paused = 0;

/*variables control buttons pressing (buttons flags)*/
/*variable = 0 -> not pressed
 *variable = 1 -> pressed and purpose done
 */
unsigned char mode_toggle_button = 0;
unsigned char hours_decrement_button = 0;
unsigned char hours_increment_button = 0;
unsigned char minutes_decrement_button = 0;
unsigned char minutes_increment_button = 0;
unsigned char seconds_decrement_button = 0;
unsigned char seconds_increment_button = 0;

ISR(TIMER1_COMPA_vect)
{
	if(mode_toggle == 0)
	{
		/*counting up*/
		time++;
	}
	else
	{
		/*counting down*/
		time--;
	}
}

ISR(INT0_vect)
{
	/*reset time*/
	time = 0;
}

ISR(INT1_vect)
{
	/*stopwatch pause*/
	/*stop timer 1 (no clock source)*/
	CLEAR_BIT(TCCR1B, CS10);
	CLEAR_BIT(TCCR1B, CS11);
	CLEAR_BIT(TCCR1B, CS12);

	paused = 1;
}

ISR(INT2_vect)
{
	/*stopwatch resume*/
	/*reactivate timer 1 (prescaler = 1024)*/
	SET_BIT(TCCR1B, CS10);
	SET_BIT(TCCR1B, CS12);
}
void timer1_ctc_init(void)
{
	/*set FOC1A, FOC1B, WGM12 to choose the compare mode for timer1*/
	SET_BIT(TCCR1A, FOC1A);
	SET_BIT(TCCR1A, FOC1B);
	SET_BIT(TCCR1B, WGM12);

	/*configure the prescaler to 1024*/
	SET_BIT(TCCR1B, CS10);
	SET_BIT(TCCR1B, CS12);

	TCNT1 = 0;

	/*compare value = 15625, to activate interrupt every one second*/
	OCR1A = 15625;

	/*activate timer interruptA mask*/
	SET_BIT(TIMSK, OCIE1A);
}

void external_int0_init(void)
{
	/*configure external interrupt 0 to be triggered by falling edge*/
	SET_BIT(MCUCR, ISC01);

	/*activate external interrupt 0 mask*/
	SET_BIT(GICR, INT0);

	/*set the direction of the pin PD2 in port D to input*/
	CLEAR_BIT(DDRD, PD2);

	/*activate internal pull-up resistor for PD2*/
	SET_BIT(PORTD, PD2);
}

void external_int1_init(void)
{
	/*configure external interrupt 1 to be triggered by raising edge*/
	SET_BIT(MCUCR, ISC11);
	SET_BIT(MCUCR, ISC10);

	/*activate external interrupt 1 mask*/
	SET_BIT(GICR, INT1);

	/*set the direction of the pin PD3 in port D to input*/
	CLEAR_BIT(DDRD, PD3);
}

void external_int2_init(void)
{
	/*configure external interrupt 2 to be triggered by falling edge*/
	CLEAR_BIT(MCUCSR, ISC2);

	/*activate external interrupt 2 mask*/
	SET_BIT(GICR, INT2);

	/*set the direction of the pin PB2 in port B to input*/
	CLEAR_BIT(DDRB, PB2);

	/*activate internal pull-up resistor for PB2*/
	SET_BIT(PORTB, PB2);
}
void time_handle(void)
{
	/*function to get the time digits to be displayed on the 7-segments*/
	/*when time reaches max start over*/
	/*second condition is due to the proteus simulation glitch*/
	if(time > 359999 && time < 0xFFFFFFF0)
	{
		time = 0;
	}
	unsigned long temp = time;
	/*calculations to get time digits*/
	/*hr2 -> 60 * 60 * 10 = 36000*/
	hr2 = (unsigned char) (temp/36000);
	temp = temp - (36000*hr2);
	/*hr1 -> 60 * 60 = 3600*/
	hr1 = (unsigned char) (temp/3600);
	temp = temp - (3600*hr1);
	/*min2 -> 60 * 10 = 600*/
	min2 = (unsigned char) (temp/600);
	temp = temp - (600*min2);
	/*min1 -> 60*/
	min1 = (unsigned char) (temp/60);
	temp = temp - (60*min1);
	/*sec2 -> 10*/
	sec2 = (unsigned char) (temp/10);
	temp = temp - (10*sec2);
	/*sec1 -> 1*/
	sec1 = (unsigned char) temp;
}

void time_dispaly(void)
{
	/*function to display time*/
	/*time digits display on the 6 multiplexed 7-segment*/

	time_handle();

	/*seconds least significant digit display*/
	SET_BIT(PORTA, PA0);
	PORTC = (PORTC & 0xF0) | (sec1 & 0x0F);
	_delay_ms(1);
	CLEAR_BIT(PORTA, PA0);

	/*seconds most significant digit display*/
	SET_BIT(PORTA, PA1);
	PORTC = (PORTC & 0xF0) | (sec2 & 0x0F);
	_delay_ms(2);
	CLEAR_BIT(PORTA, PA1);

	/*minutes least significant digit display*/
	SET_BIT(PORTA, PA2);
	PORTC = (PORTC & 0xF0) | (min1 & 0x0F);
	_delay_ms(2);
	CLEAR_BIT(PORTA, PA2);

	/*minutes most significant digit display*/
	SET_BIT(PORTA, PA3);
	PORTC = (PORTC & 0xF0) | (min2 & 0x0F);
	_delay_ms(2);
	CLEAR_BIT(PORTA, PA3);

	/*hours least significant digit display*/
	SET_BIT(PORTA, PA4);
	PORTC = (PORTC & 0xF0) | (hr1 & 0x0F);
	_delay_ms(2);
	CLEAR_BIT(PORTA, PA4);

	/*hours most significant digit display*/
	SET_BIT(PORTA, PA5);
	PORTC = (PORTC & 0xF0) | (hr2 & 0x0F);
	_delay_ms(2);
	CLEAR_BIT(PORTA, PA5);
}
int main()
{
	/*set I-bit to allow interrupts*/
	SET_BIT(SREG,7);

	/*set the direction of the first 4 pins in port C to output*/
	/*7-segment display pins*/
	DDRC |= 0x0F;

	/*set the direction of the first 6 pins in port A to output*/
	/*7-segment enable pins*/
	DDRA |= 0x3F;

	/*clear the first 6 pins in port A (initial value)*/
	PORTA &= 0xC0;

	/*counting mode toggle button pin configuration*/
	/*set the direction of the pin PB7 pin in port B to input*/
	CLEAR_BIT(DDRB, PB7);
	/*activate internal pull-up resistor for PB2*/
	SET_BIT(PORTB, PB7);

	/*counting mode leds configuration*/
	/*red led indicates counting up mode*/
	/*set the direction of the pin PD4 pin in port D to output*/
	SET_BIT(DDRD, PD4);
	/*clear the pin PD4 in port D (initial value)*/
	CLEAR_BIT(PORTD, PD4);

	/*yellow led indicates counting down mode*/
	/*set the direction of the pin PD5 pin in port D to output*/
	SET_BIT(DDRD, PD5);
	/*clear the pin PD5 in port D (initial value)*/
	CLEAR_BIT(PORTD, PD5);

	/*count down start time adjusting buttons configuration*/
	/*hours decrement button*/
	/*set the direction of the pin PB0 pin in port B to input*/
	CLEAR_BIT(DDRB, PB0);
	/*activate internal pull-up resistor for PB0*/
	SET_BIT(PORTB, PB0);

	/*hours increment button*/
	/*set the direction of the pin PB1 pin in port B to input*/
	CLEAR_BIT(DDRB, PB1);
	/*activate internal pull-up resistor for PB1*/
	SET_BIT(PORTB, PB1);

	/*minutes decrement button*/
	/*set the direction of the pin PB3 pin in port B to input*/
	CLEAR_BIT(DDRB, PB3);
	/*activate internal pull-up resistor for PB3*/
	SET_BIT(PORTB, PB3);

	/*minutes increment button*/
	/*set the direction of the pin PB4 pin in port B to input*/
	CLEAR_BIT(DDRB, PB4);
	/*activate internal pull-up resistor for PB4*/
	SET_BIT(PORTB, PB4);

	/*seconds decrement button*/
	/*set the direction of the pin PB5 pin in port B to input*/
	CLEAR_BIT(DDRB, PB5);
	/*activate internal pull-up resistor for PB5*/
	SET_BIT(PORTB, PB5);

	/*seconds increment button*/
	/*set the direction of the pin PB6 pin in port B to input*/
	CLEAR_BIT(DDRB, PB6);
	/*activate internal pull-up resistor for PB6*/
	SET_BIT(PORTB, PB6);

	/*buzzer pin configuration*/
	/*set the direction of the pin PD0 pin in port D to output*/
	SET_BIT(DDRD, PD0);
	/*clear the pin PD0 in port D (initial value)*/
	CLEAR_BIT(PORTD, PD0);

	time_handle();

	/*timer 1 initialization to compare mode*/
	timer1_ctc_init();


	/*external interrupt 0 initialization*/
	external_int0_init();

	/*external interrupt 1 initialization*/
	external_int1_init();

	/*external interrupt 2 initialization*/
	external_int2_init();

	while(1)
	{
		time_dispaly();

		/*counting mode leds handling*/
		if(mode_toggle == 0)
		{
			/*turn off the yellow led*/
			CLEAR_BIT(PORTD, PD5);
			/*turn on the red led (counting up mode)*/
			SET_BIT(PORTD, PD4);
		}
		else
		{
			/*turn off the red led*/
			CLEAR_BIT(PORTD, PD4);
			/*turn on the yellow led (counting down mode)*/
			SET_BIT(PORTD, PD5);
		}

		/*if counting mode toggle button is pressed*/
		/*pressed -> PB7 in PINB = 0*/
		if(BIT_IS_CLEAR(PINB, PB7))
		{
			_delay_us(30);
			if(BIT_IS_CLEAR(PINB, PB7) && paused)
			{
				if(mode_toggle_button == 0)
				{
					mode_toggle = !mode_toggle;

					mode_toggle_button = 1;
				}
			}
		}
		else
		{
			mode_toggle_button = 0;
		}

		/*if hours decrement button is pressed*/
		/*pressed -> PB0 in PINB = 0*/
		if(BIT_IS_CLEAR(PINB, PB0))
		{
			_delay_us(30);
			if(BIT_IS_CLEAR(PINB, PB0) && paused)
			{
				if(hours_decrement_button == 0)
				{
					/*make sure time is greater or equal one hour
					 * 60 * 60 = 3600
					 */
					if(time >= 3600)
					{
						time = time - 3600;
					}
					hours_decrement_button = 1;
				}
			}
		}
		else
		{
			hours_decrement_button = 0;
		}

		/*if hours increment button is pressed*/
		/*pressed -> PB0 in PINB = 1*/
		if(BIT_IS_CLEAR(PINB, PB1))
		{
			_delay_us(30);
			if(BIT_IS_CLEAR(PINB, PB1) && paused)
			{
				if(hours_increment_button == 0)
				{
					/*make sure time has room for one more hour
					 * 99 * 60 * 60 = 356400
					 */
					if(time< 356400)
					{
						time = time +3600;
					}

					hours_increment_button = 1;
				}
			}
		}
		else
		{
			hours_increment_button = 0;
		}

		/*if minutes decrement button is pressed*/
		/*pressed -> PB3 in PINB = 0*/
		if(BIT_IS_CLEAR(PINB, PB3))
		{
			_delay_us(30);
			if(BIT_IS_CLEAR(PINB, PB3) && paused)
			{
				if(minutes_decrement_button == 0)
				{
					/*make sure time is greater or equal one minute
					 * 60
					 */
					if(time >= 60)
					{
						time = time - 60;
					}
					minutes_decrement_button = 1;
				}
			}
		}
		else
		{
			minutes_decrement_button = 0;
		}

		/*if minutes increment button is pressed*/
		/*pressed -> PB0 in PINB = 4*/
		if(BIT_IS_CLEAR(PINB, PB4))
		{
			_delay_us(30);
			if(BIT_IS_CLEAR(PINB, PB4) && paused)
			{
				if(minutes_increment_button == 0)
				{
					/*make sure time has room for one more minute
					 * 99 * 60 * 60 + 60 * 59 = 359940
					 */
					if(time < 359940)
					{
						time = time + 60;
					}

					minutes_increment_button = 1;
				}
			}
		}
		else
		{
			minutes_increment_button = 0;
		}

		/*if seconds decrement button is pressed*/
		/*pressed -> PB5 in PINB = 0*/
		if(BIT_IS_CLEAR(PINB, PB5))
		{
			_delay_us(30);
			if(BIT_IS_CLEAR(PINB, PB5) && paused)
			{
				if(seconds_decrement_button == 0)
				{
					/*make sure time is greater than 0*/
					if(time > 0)
					{
						time--;
					}
					seconds_decrement_button = 1;
				}
			}
		}
		else
		{
			seconds_decrement_button = 0;
		}

		/*if seconds increment button is pressed*/
		/*pressed -> PB6 in PINB = 0*/
		if(BIT_IS_CLEAR(PINB, PB6))
		{
			_delay_us(30);
			if(BIT_IS_CLEAR(PINB, PB6) && paused)
			{
				if(seconds_increment_button == 0)
				{
					/*make sure time has room for one more second
					 * 99 * 60 * 60 + 60 * 59 + 59 = 359999
					 */
					if(time < 359999)
					{
						time++;
					}

					seconds_increment_button = 1;
				}
			}
		}
		else
		{
			seconds_increment_button = 0;
		}

		if(mode_toggle && time == 0)
		{
			/*stopwatch stop
			 *count down reached 0
			 */
			/*stop timer 1 (no clock source)*/
			CLEAR_BIT(TCCR1B, CS10);
			CLEAR_BIT(TCCR1B, CS11);
			CLEAR_BIT(TCCR1B, CS12);

			/*buzzer on*/
			SET_BIT(PORTD, PD0);
		}
		else
		{
			/*buzzer off*/
			CLEAR_BIT(PORTD, PD0);
		}
	}
}
