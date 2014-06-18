// vim:set ts=4 sw=4 ai:
/*
 * Copyright 2009 Erik Gilling
 * Copyright 2014 Michael Toren <mct@toren.net>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "uart.h"
#include "config.h"
#include "pins.h"
#include "gamma_correction_table.h"

#ifdef OLD_PROTO
	#include "proto.h"
#else
	#include "proto2.h"
#endif

uint16_t led_val[3];
uint16_t back_buffer[3];

void handle_rgb(uint8_t red, uint8_t green, uint8_t blue)
{
	back_buffer[0] = gamma_correction[red];
	back_buffer[1] = gamma_correction[green];
	back_buffer[2] = gamma_correction[blue];
}

void handle_latch(void)
{
	led_val[0] = back_buffer[0];
	led_val[1] = back_buffer[1];
	led_val[2] = back_buffer[2];
}

void handle_baud(uint32_t baud)
{
	if (!baud)
		baud = BAUD;
	uart_init(baud);
}

proto_t state = {
	.latch_callback = handle_latch,
	.rgb_callback = handle_rgb,
	.baud_callback = handle_baud,
};

ISR( TIMER0_COMPA_vect )
{
	static uint16_t phase;
	uint16_t tmp_phase;
	uint8_t port;

	PORTD ^= _BV(D_TP1);
	PORTD |= _BV(D_TP2);
	port = PORTC;

	phase++;
	if (phase > 511)
		phase = 1;
	tmp_phase = phase;

	if (led_val[0] >= tmp_phase)
		port |= _BV(C_RED);
	else
		port &= ~_BV(C_RED);

	if (tmp_phase <= led_val[0])
		tmp_phase += 511 - led_val[0];
	else	
		tmp_phase -= led_val[0];

	if (led_val[1] >= tmp_phase)
		port |= _BV(C_GREEN);
	else
		port &= ~_BV(C_GREEN);

	if (tmp_phase <= led_val[1])
		tmp_phase += 511 - led_val[1];
	else	
		tmp_phase -= led_val[1];

	if (led_val[2] >= tmp_phase)
		port |= _BV(C_BLUE);
	else
		port &= ~_BV(C_BLUE);

	PORTC = port;
	PORTD &= ~_BV(D_TP2);
}

void hw_setup(void)
{
	DDRB = _BV(B_TP4) | _BV(B_TP5);
	DDRC = _BV(C_RED) | _BV(C_GREEN) | _BV(C_BLUE) | _BV(C_RED_OD) | _BV(C_GREEN_OD) | _BV(C_BLUE_OD);
	DDRD = _BV(D_DATA_LED) | _BV(D_GREEN_OD2) | _BV(D_BLUE_OD2) | _BV(D_TP1) | _BV(D_TP2) | _BV(D_TP3);

	/* set TC into CTC mode */
	TCCR0A = _BV(WGM01);

	/* enable output compare interrupt */
	TIMSK0 = _BV(OCIE0A);

	/* interrupt at 24 kHz (255 levels at 94 Hz PWM) */
	TCCR0B = _BV(CS01) | _BV(CS00);
	OCR0A = 2;
}

int main(void)
{
	cli();
	hw_setup();
	config_init();
	uart_init(BAUD);
	proto_init(&state, config.addr);
	sei();

	wdt_enable(WDTO_1S);

	while (1) {
		wdt_reset();
		if (uart_char_avail()) {
			PORTD |= _BV(D_DATA_LED);
			PORTD |= _BV(D_TP3);
			proto_recv(&state, UDR0);
			PORTD &= ~_BV(D_TP3);
			PORTD &= ~_BV(D_DATA_LED);
		}
	}
}