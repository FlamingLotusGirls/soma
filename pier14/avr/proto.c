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

#ifdef OLD_PROTO

#include <stddef.h>
#include <stdint.h>

#include "proto.h"
#include "crc8.h"

void proto_init(proto_t *p, uint8_t addr)
{
	p->addr = addr;
	p->state = PROTO_STATE_IDLE;
}

static void proto_handle_packet(proto_t *p)
{
	struct proto_packet *pkt = &p->packet;

	if (pkt->addr != PROTO_ADDR_BCAST && pkt->addr != p->addr)
		return;

	if (pkt->cmd == PROTO_CMD_SYNC)
		if (p->latch_callback)
			p->latch_callback();
}

void proto_recv(proto_t *p, uint8_t c)
{
	switch (p->state) {
	case PROTO_STATE_IDLE:
		if (c == PROTO_SOF) {
			p->crc = crc8_start();
			p->state = PROTO_STATE_ADDR;
			p->packet.sof = c;
		} else if (c == PROTO_SOF_LONG) {
			p->state = PROTO_STATE_ADDR;
			p->packet.sof = c;
		}
		break;

	case PROTO_STATE_ADDR:
		p->crc = crc8_calc(p->crc, c);
		p->packet.addr = c;
		p->state = PROTO_STATE_CMD;
		break;

	case PROTO_STATE_CMD:
		p->crc = crc8_calc(p->crc, c);
		p->packet.cmd = c;
		if (p->packet.sof == PROTO_SOF) {
			p->state = PROTO_STATE_VAL;
		} else if (p->packet.sof == PROTO_SOF_LONG) {
			p->state = PROTO_STATE_LEN;
		}
		break;

	case PROTO_STATE_VAL:
		p->crc = crc8_calc(p->crc, c);
		p->packet.val = c;
		p->state = PROTO_STATE_CRC;
		break;

	case PROTO_STATE_CRC:
		p->crc = crc8_end(p->crc);
		p->packet.crc = c;
		p->state = PROTO_STATE_EOF;
		break;

	case PROTO_STATE_EOF:
		if (c == PROTO_EOF && p->crc == p->packet.crc && p->packet.sof == PROTO_SOF)
			proto_handle_packet(p);

		p->state = PROTO_STATE_IDLE;
		break;

	case PROTO_STATE_LEN:
		p->packet.val = c;
		if (c == 0)
			p->state = PROTO_STATE_IDLE;
		else
			p->state = PROTO_STATE_LONG_DATA0;
		break;

	case PROTO_STATE_LONG_DATA0:
		p->long_val =(uint32_t) c;
		p->state = PROTO_STATE_LONG_DATA1;
		break;

	case PROTO_STATE_LONG_DATA1:
		p->long_val |= (uint32_t)c << 8;
		p->state = PROTO_STATE_LONG_DATA2;
		break;

	case PROTO_STATE_LONG_DATA2:
		p->long_val |= (uint32_t)c << 16;
		p->state = PROTO_STATE_LONG_DATA3;
		break;

	case PROTO_STATE_LONG_DATA3:
		p->long_val |= (uint32_t)c << 24;

		if (p->packet.addr == p->addr && p->rgb_callback)
			p->rgb_callback(((p->long_val >>  0) & 0xff),
							((p->long_val >>  8) & 0xff),
							((p->long_val >> 16) & 0xff));

		p->packet.val--;
		p->packet.addr++;
		if (p->packet.val == 0)
			p->state = PROTO_STATE_EOF;
		else
			p->state = PROTO_STATE_LONG_DATA0;
		break;
	}
}

#endif
