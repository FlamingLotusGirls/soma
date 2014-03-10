// vim:set ts=4 sw=4 ai:
/*
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

#ifndef OLD_PROTO

#include <stdint.h>
#include "crc8.h"
#include "proto2.h"

void proto_init(proto_t *state, uint8_t addr)
{
	state->my_addr = addr;
	state->state = PROTO_STATE_IDLE;
	state->escape = 0;
}

void latch_packet(proto_t *state)
{
	if (! state->latch_callback)
		return;

	state->latch_callback();
}

void rgb_packet(proto_t *state)
{
	if (! state->rgb_callback)
		return;

	if (state->num_colors != 0)
		return;

	state->rgb_callback(state->red, state->green, state->blue);
}

void baud_packet(proto_t *state)
{
	if (! state->baud_callback)
		return;

	// Only support the BAUD packet if sent as a broadcast message; it doesn't
	// make sense to have different LED modules on the bus running at different
	// baud rates.
	if (state->dst_addr != PROTO_ADDR_BCAST)
		return;

	state->baud_callback(state->baud);
}

void noop_packet(proto_t *state)
{
	if (! state->noop_callback)
		return;

	state->noop_callback();
}

void layer3_process_packet(proto_t *state)
{
	if (state->state != PROTO_STATE_END)
		return;

	// Kludge
	if (state->valid_color)
		state->dst_addr = state->my_addr;

	if (state->dst_addr != state->my_addr && state->dst_addr != PROTO_ADDR_BCAST)
		return;

	switch (state->type) {
		case PROTO_CMD_NOOP:
			noop_packet(state);
			break;

		case PROTO_CMD_RGBDATA:
			rgb_packet(state);
			break;

		case PROTO_CMD_LATCH:
			latch_packet(state);
			break;

		case PROTO_CMD_RGBLATCH:
			rgb_packet(state);
			latch_packet(state);
			break;

		case PROTO_CMD_BAUD:
			baud_packet(state);
			break;
	}
}

/*
 * Formats for the valid packets we accept:
 *
 *	NOOP:
 *		1 Byte:  Type
 *		1 Byte:  Address
 *		1 Byte:  CRC
 *		1 Byte:  FrameEnd
 *
 *	LATCH:
 *		1 Byte:  Address
 *		1 Byte:  Type
 *		1 Byte:  CRC
 *		1 Byte:  FrameEnd
 *
 *	RGBDATA, and RGBLATCH:
 *		1 Byte:  Address
 *		1 Byte:  Type
 *		1 Byte:  Number of Colors
 *		N Bytes: Red, Green, Blue tuples, 
 *		1 Byte:  CRC
 *		1 Byte:  FrameEnd
 *
 *	BAUD:
 *		1 Byte:  Address
 *		1 Byte:  Type
 *		3 Bytes: Baud rate, Network Byte Order
 *		1 Byte:  CRC
 *		1 Byte:  FrameEnd
 */

/*
 * We use a single state machine to parse the above packet types, made up of the following states:
 *
 *	IDLE:		Not currently processing a packet. Next byte will be the ADDRESS field
 *	TYPE:		Next byte we receive will be the TYPE field
 *
 *  // BAUD packets
 *	BAUD1:		Next byte we receive will be the 1st BAUD field
 *	BAUD2:		Next byte we receive will be the 2nd BAUD field
 *	BAUD3:		Next byte we receive will be the 3nd BAUD field
 *
 *  // RGB packets
 *	NUM_COLORS:	Next byte we receive will be the number of colors in an RGB packet
 *	RED:		Next byte we recieve will be the RED field
 *	GREEN:		Next byte we recieve will be the GREEN field
 *	BLUE:		Next byte we recieve will be the BLUE field
 *
 *	CRC:		Next byte we receive will be the CRC
 *	END:		All done!  If we receive another byte, it is an error
 *
 *	OVERRUN:	We received a byte after we were in the END state
 *	BADCRC:		The CRC check failed
 *	BADTYPE:	We did not recognize the packet TYPE field
 *	BAD:		Some other unspecified error.
 */

void layer3_recv(proto_t *state, const uint8_t input_byte)
{
	switch (state->state) {
		case PROTO_STATE_IDLE:
			state->crc = crc8_calc(0, input_byte);
			state->dst_addr = input_byte;
			state->color_addr = input_byte;
			state->valid_color = 0;
			state->state = PROTO_STATE_TYPE;
			break;

		case PROTO_STATE_TYPE:
			state->crc = crc8_calc(state->crc, input_byte);
			state->type = input_byte;

			switch (state->type) {
				case PROTO_CMD_NOOP:
				case PROTO_CMD_LATCH:
					state->state = PROTO_STATE_CRC;
					break;

				case PROTO_CMD_RGBDATA:
				case PROTO_CMD_RGBLATCH:
					state->state = PROTO_STATE_NUM_COLORS;
					break;

				case PROTO_CMD_BAUD:
					state->state = PROTO_STATE_BAUD1;
					break;

				default:
					state->state = PROTO_STATE_BADTYPE;
			}

			break;



		/* States for PROTO_CMD_BAUD */
		case PROTO_STATE_BAUD1:
			state->crc = crc8_calc(state->crc, input_byte);
			state->baud = input_byte;
			state->state = PROTO_STATE_BAUD2;
			break;

		case PROTO_STATE_BAUD2:
			state->crc = crc8_calc(state->crc, input_byte);
			state->baud <<= 8;
			state->baud += input_byte;
			state->state = PROTO_STATE_BAUD3;
			break;

		case PROTO_STATE_BAUD3:
			state->crc = crc8_calc(state->crc, input_byte);
			state->baud <<= 8;
			state->baud += input_byte;
			state->state = PROTO_STATE_CRC;
			break;



		/* States for PROTO_CMD_RGBDATA and PROTO_CMD_RGBLATCH */
		case PROTO_STATE_NUM_COLORS:
			state->crc = crc8_calc(state->crc, input_byte);
			state->num_colors = input_byte;
			state->state = PROTO_STATE_RED;
			break;

		case PROTO_STATE_RED:
			state->crc = crc8_calc(state->crc, input_byte);
			state->tmp_red = input_byte;
			state->state = PROTO_STATE_GREEN;
			break;

		case PROTO_STATE_GREEN:
			state->crc = crc8_calc(state->crc, input_byte);
			state->tmp_green = input_byte;
			state->state = PROTO_STATE_BLUE;
			break;

		case PROTO_STATE_BLUE:
			state->crc = crc8_calc(state->crc, input_byte);
			state->tmp_blue = input_byte;

			if (state->color_addr == state->my_addr || state->color_addr == PROTO_ADDR_BCAST) {
				state->red   = state->tmp_red;
				state->green = state->tmp_green;
				state->blue  = state->tmp_blue;
				state->valid_color = 1;
			}
			state->color_addr++;

			if (--state->num_colors)
				state->state = PROTO_STATE_RED;
			else
				state->state = PROTO_STATE_CRC;

			break;



		/* Common states for the end of packets */
		case PROTO_STATE_CRC:
			if (input_byte == state->crc)
				state->state = PROTO_STATE_END;
			else
				state->state = PROTO_STATE_BADCRC;
			break;

		case PROTO_STATE_END:
			state->state = PROTO_STATE_OVERRUN;
			break;

		case PROTO_STATE_OVERRUN:
		case PROTO_STATE_BADCRC:
		case PROTO_STATE_BADTYPE:
		case PROTO_STATE_BAD:
			break;
	}
}

void proto_recv(proto_t *state, uint8_t input_byte)
{
	if (state->escape) {
		state->escape = 0;

		switch (input_byte) {
			case FRAME_TEND:
				layer3_recv(state, FRAME_END);
				break;

			case FRAME_TESC:
				layer3_recv(state, FRAME_ESC);
				break;
		}
	}

	else {
		switch (input_byte) {
			case FRAME_ESC:
				state->escape = 1;
				break;

			case FRAME_END:
				layer3_process_packet(state);
				state->state = PROTO_STATE_IDLE;
				break;

			default:
				layer3_recv(state, input_byte);
		}
	}
}

#endif
