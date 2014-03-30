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

#ifndef __proto_h__
#define __proto_h__

#include <stdint.h>

#define PROTO_ADDR_MASTER	0x00
#define PROTO_ADDR_BCAST	0xff

#define PROTO_SOF			0x53	// 'S'
#define PROTO_SOF_LONG		0x54	// 'T'
#define PROTO_EOF			0x45	// 'E'

struct proto_packet {
	uint8_t sof;
	uint8_t	addr;
	uint8_t	cmd;
	uint8_t	val;
	uint8_t	crc;
	uint8_t eof;
} __attribute__((packed));

enum proto_cmd {
	PROTO_CMD_SYNC = 0x80,
};

enum proto_state {
	PROTO_STATE_IDLE,
	PROTO_STATE_ADDR,
	PROTO_STATE_CMD,
	PROTO_STATE_VAL,
	PROTO_STATE_CRC,
	PROTO_STATE_EOF,
	PROTO_STATE_LEN,
	PROTO_STATE_LONG_DATA0,
	PROTO_STATE_LONG_DATA1,
	PROTO_STATE_LONG_DATA2,
	PROTO_STATE_LONG_DATA3,
};

typedef struct proto {
	struct proto_packet	packet;
	enum proto_state	state;
	uint8_t addr;
	uint8_t crc;
	uint32_t long_val;

	void (*noop_callback)(void);
	void (*rgb_callback)(uint8_t red, uint8_t blue, uint8_t green);
	void (*latch_callback)(void);
	void (*baud_callback)(uint32_t baud);
} proto_t;

void proto_init(struct proto *p, uint8_t addr);
void proto_recv(struct proto *p, uint8_t proto);

#endif /* __proto_h__ */
