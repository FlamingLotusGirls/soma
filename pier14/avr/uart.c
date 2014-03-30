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

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"

void uart_init(uint32_t baud)
{
	uint8_t saved_sreg = SREG;
	uint16_t ubrr = FOSC / 16 / baud - 1;

	cli();

	// atmegaX8 style uart
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)ubrr;

	// enable receiver, transmitter
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);

	// disable receive interrupt; we poll below instead, which is much more reliable, apparently?
	UCSR0B &= ~_BV(RXCIE0);

	// Set frame format: 8data, 1stop bit
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);

	// Restore interrupts only if they were previously enabled
	SREG = saved_sreg;
}

void uart_putc(unsigned char data)
{
	while (! (UCSR0A & _BV(UDRE0)))
		;

	UDR0 = data;
}

void uart_puts(const char *s)
{
	while (*s) {
		uart_putc(*s);
		s++;
	}
}

void uart_puts_P(const char *s)
{
	char c;

	while ((c = pgm_read_byte(s)) != '\0') {
		uart_putc(c);
		s++;
	}
}

const uint8_t hexchars[] PROGMEM = "0123456789ABCDEF";

void uart_printhex(uint8_t val)
{
	uart_putc(pgm_read_byte(hexchars + (val >> 4)));
	uart_putc(pgm_read_byte(hexchars + (val & 0xf)));
}

int uart_has_data(void)
{
	return UCSR0A & _BV(RXC0);
}

unsigned char uart_poll_getchar(void)
{
	while (!(UCSR0A & _BV(RXC0)))
		;

	return UDR0;
}
