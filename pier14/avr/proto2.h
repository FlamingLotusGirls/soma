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

#ifndef __proto2_h__
#define __proto2_h__

#include <stdint.h>

// Line encoding.
// SLIP and KISS use 0xC0 for FRAME_END.  Moving to 0xDA, because it's less
// likely to appear in RGB values and require escaping.
enum {
	FRAME_END	= 0xDA,
	FRAME_ESC	= 0xDB,
	FRAME_TEND	= 0xDC,
	FRAME_TESC	= 0xDD,
};

// Addresses
enum {
	PROTO_ADDR_MASTER = 0x00,
	PROTO_ADDR_BCAST  = 0xff,
};

// Packet types
enum {
	PROTO_CMD_NOOP		= 0x11,
	PROTO_CMD_BAUD		= 0x12,
	PROTO_CMD_LATCH		= 0x13,
	PROTO_CMD_RGBDATA	= 0x14,
	PROTO_CMD_RGBLATCH	= 0x15,
};

// Protocol States
enum {
	PROTO_STATE_IDLE = 0,
	PROTO_STATE_TYPE,
	PROTO_STATE_BAUD1,
	PROTO_STATE_BAUD2,
	PROTO_STATE_BAUD3,
	PROTO_STATE_NUM_COLORS,
	PROTO_STATE_RED,
	PROTO_STATE_GREEN,
	PROTO_STATE_BLUE,
	PROTO_STATE_CRC,
	PROTO_STATE_END,
	PROTO_STATE_OVERRUN,
	PROTO_STATE_BADCRC,
	PROTO_STATE_BADTYPE,
	PROTO_STATE_BAD,
};

typedef struct {
	uint8_t escape,
			state,
			my_addr, dst_addr, type, crc,
			color_addr,
			tmp_red, tmp_green, tmp_blue,
			red, green, blue,
			num_colors, valid_color;
	uint32_t baud;

	void (*noop_callback)(void);
	void (*rgb_callback)(uint8_t red, uint8_t blue, uint8_t green);
	void (*latch_callback)(void);
	void (*baud_callback)(uint32_t baud);
} proto_t;

void proto_init(proto_t *state, uint8_t addr);
void proto_recv(proto_t *state, uint8_t c);

#endif /* __proto2_h__ */
