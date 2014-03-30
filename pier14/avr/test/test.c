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

#include <stdio.h>
#include <stdlib.h>

#include "proto2.h"

void handle_rgb(uint8_t r, uint8_t b, uint8_t g) {
	printf("RGB Handler: %02x %02x %02x\n", r, g, b);
}

void handle_latch(void) {
	printf("LATCH\n");
}

void handle_noop(void) {
	printf("NOOP\n");
}

void handle_baud(uint32_t baud) {
	printf("BAUD %d\n", baud);
}

proto_t state = {
	.latch_callback = handle_latch,
	.rgb_callback = handle_rgb,
	.noop_callback = handle_noop,
	.baud_callback = handle_baud,
};

int main(int argc, char *argv[])
{
	proto_init(&state, 0x10);

	while (1) {
		int c = getc(stdin);

		if (c == EOF) {
			printf("EOF\n");
			break;
		}

		proto_recv(&state, c);
	}

	return 0;
}
