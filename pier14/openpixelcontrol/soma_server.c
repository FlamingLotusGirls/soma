// vim:set ts=4 sw=4 ai et smarttab:

/*
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * Copyright 2013-2014 Michael C. Toren <mct@toren.net>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <sys/time.h>

#include "opc/opc.h"

#ifndef CNEW_RTSCTS
#define CNEW_RTSCTS CRTSCTS
#endif

// For the old protocol
#define PROTO_SOF        'S'
#define PROTO_SOF_LONG   'T'
#define PROTO_EOF        'E'
#define PROTO_CMD_SYNC   0x80
#define PROTO_ADDR_BCAST 0xFF

// For the new protocol
enum {
    FRAME_END   = 0xDA,
    FRAME_ESC   = 0xDB,
    FRAME_TEND  = 0xDC,
    FRAME_TESC  = 0xDD,
};

enum {
    PROTO_CMD_NOOP      = 0x11,
    PROTO_CMD_BAUD      = 0x12,
    PROTO_CMD_LATCH     = 0x13,
    PROTO_CMD_RGBDATA   = 0x14,
    PROTO_CMD_RGBLATCH  = 0x15,
};

#define MAX_LEDS 250
#define BUF_SIZE (MAX_LEDS*4)

struct {
    uint8_t addr;
    pixel p;
} leds[MAX_LEDS];

int num_leds;

uint16_t port = OPC_DEFAULT_PORT;
uint32_t baud = 230400; // 115200*2
char *device, *config;
int old_proto;
int fd;
double sample_time = 5;
uint32_t frames;

opc_source source = -1;

uint8_t crc8_table[] = {
	0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15,
	0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
	0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
	0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
	0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5,
	0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
	0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85,
	0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
	0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
	0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
	0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2,
	0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
	0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32,
	0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
	0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
	0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
	0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c,
	0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
	0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec,
	0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
	0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
	0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
	0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c,
	0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
	0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b,
	0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
	0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
	0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
	0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb,
	0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
	0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb,
	0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3,
};

// http://www.cmrr.umn.edu/~strupp/serial.html
int serial_open(char *device, uint32_t baud)
{
    int fd;
    struct termios options, options_new;

    switch (baud) {
        case 300:    baud = B300;    break;
        case 1200:   baud = B1200;   break;
        case 2400:   baud = B2400;   break;
        case 4800:   baud = B4800;   break;
        case 9600:   baud = B9600;   break;
        case 19200:  baud = B19200;  break;
        case 38400:  baud = B38400;  break;
        case 57600:  baud = B57600;  break;
        case 115200: baud = B115200; break;
        case 230400: baud = B230400; break;

        default:
            fprintf(stderr, "Unspported baud rate\n");
            return -1;
    }

    fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    fcntl(fd, F_SETFL, 0);

    tcgetattr(fd, &options);
    cfsetispeed(&options, baud);        // Input baud
    cfsetospeed(&options, baud);        // Output baud

    options.c_cflag |= CLOCAL;          // Local line - do not change "owner" of port
    options.c_cflag |= CREAD;           // Enable receiver
    options.c_cflag &= ~CSIZE;          // Mask out the character size bits
    options.c_cflag |= CS8;             // 8 data bits
    options.c_cflag &= ~PARENB;         // No parity
    options.c_cflag &= ~CSTOPB;         // 1 stop bit
    options.c_cflag &= ~CNEW_RTSCTS;    // No hardware flow control
    options.c_cflag &= ~HUPCL;          // Do not drop DTR on close

    options.c_lflag &= ~ICANON;         // raw input
    options.c_lflag &= ~ECHO;           // no echo
    options.c_lflag &= ~ECHOE;          // no echo erase
    options.c_lflag &= ~ISIG;           // Disable INTR, SUSP, DSUSP, and QUIT signals

    options.c_iflag &= ~(IXON | IXOFF | IXANY); // disable software flow control

    options.c_oflag &= ~OPOST;          // raw output

    tcsetattr(fd, TCSANOW, &options);
    tcgetattr(fd, &options_new);

    if ((options.c_cflag != options_new.c_cflag) ||
        (options.c_lflag != options_new.c_lflag) ||
        (options.c_iflag != options_new.c_iflag) ||
        (options.c_oflag != options_new.c_oflag))
    {
        fprintf(stderr, "tcsetattr: Some options did not stick?\n");
        exit(1);
    }

    return fd;
}

void serial_write(uint8_t *p, size_t len)
{
    while (len) {
        ssize_t n = write(fd, p, len);

        if (n < 0) {
            perror("write");
            exit(1);
        }

        if (n != len) {
            fprintf(stderr, "warning: Short write: Tired to write %zd bytes, only wrote %zd\n", len, n);
        }

        len -= n;
        p += n;
    }
}

void send_old_short_packet(uint8_t addr, uint8_t command, uint8_t value)
{
    uint8_t buf[2048];
    int buf_len = 0;
    uint8_t crc = 0;

    crc = crc8_table[crc ^ addr   ];
    crc = crc8_table[crc ^ command];
    crc = crc8_table[crc ^ value  ];

    buf[buf_len++] = PROTO_SOF;
    buf[buf_len++] = addr;
    buf[buf_len++] = command;
    buf[buf_len++] = value;
    buf[buf_len++] = crc;
    buf[buf_len++] = PROTO_EOF;

    serial_write(buf, buf_len);
}

void send_old_long_packet(uint8_t addr, uint8_t words, uint8_t *data)
{
    uint8_t buf[2048];
    int buf_len = 0;

    buf[buf_len++] = PROTO_SOF_LONG;
    buf[buf_len++] = addr;
    buf[buf_len++] = 0;
    buf[buf_len++] = words;

    memcpy(buf + buf_len, data, words*4);
    buf_len += words*4;

    buf[buf_len++] = PROTO_EOF;
    serial_write(buf, buf_len);
}

int escape(uint8_t *src, int src_len, uint8_t *dst)
{
    int dst_len = 0;
    int i;

    for (i = 0; i < src_len; i++) {
        switch (src[i]) {
            case FRAME_END:
                dst[dst_len++] = FRAME_ESC;
                dst[dst_len++] = FRAME_TEND;
                break;

            case FRAME_ESC:
                dst[dst_len++] = FRAME_ESC;
                dst[dst_len++] = FRAME_TESC;
                break;

            default:
                dst[dst_len++] = src[i];
                break;
        }
    }

    return dst_len;
}

uint8_t calc_crc(uint8_t *p, int len)
{
    uint8_t crc = 0;
    int i;

    for (i = 0; i < len; i++)
        crc = crc8_table[crc ^ p[i]];

    return crc;
}

void send_rgblatch(uint8_t addr, uint8_t num_colors, uint8_t *data)
{
    uint8_t buf[BUF_SIZE];
    uint8_t escaped_buf[BUF_SIZE*2];

    int buf_len = 0;
    int escaped_buf_len = 0;

    uint8_t crc;

    buf[buf_len++] = addr;                      // addr
    buf[buf_len++] = PROTO_CMD_RGBLATCH;        // type
    buf[buf_len++] = num_colors;                // num_colors

    // color tuples
    memcpy(buf + buf_len, data, num_colors*3);
    buf_len += num_colors*3;

    crc = calc_crc(buf, buf_len);
    buf[buf_len++] = crc;

    escaped_buf_len = escape(buf, buf_len, escaped_buf);
    escaped_buf[escaped_buf_len++] = FRAME_END;
    serial_write(escaped_buf, escaped_buf_len);
}

void refresh()
{
    uint8_t buf[BUF_SIZE];
    int buf_len;

    int first = 1;
    int i;

    uint8_t base_addr;
    uint8_t last_addr;

    for (i = 0; i < num_leds; i++) {
        if (first || last_addr + 1 != leds[i].addr) {
            if (!first) {
                if (old_proto)
                    send_old_long_packet(base_addr, buf_len/4, buf);
                else
                    send_rgblatch(base_addr, buf_len/3, buf);
            }

            base_addr = leds[i].addr;
            buf_len = 0;
        }

        buf[buf_len++] = leds[i].p.r;
        buf[buf_len++] = leds[i].p.g;
        buf[buf_len++] = leds[i].p.b;

        if (old_proto)
            buf[buf_len++] = 0;

        last_addr = leds[i].addr;
        first = 0;
    }

    if (old_proto) {
        send_old_long_packet(base_addr, buf_len/4, buf);
        send_old_short_packet(PROTO_ADDR_BCAST, PROTO_CMD_SYNC, 0);
    }
    else {
        send_rgblatch(base_addr, buf_len/3, buf);
    }
}

void handler(u8 channel, u16 count, pixel* p)
{
    int i;

    for (i = 0; i < count && i < num_leds; i++)
        leds[i].p = p[i];

    //printf("Channel %d, %d pixels\n", channel, count);
    refresh();
    frames++;
}

void usage(char *name)
{
    fprintf(stderr, "Usage: %s -f <configfile> -l <device> [-s <speed>] [-p <port>] [-n <sample_time>]\n", name);
    exit(1);
}

void read_config(char *filename)
{
    FILE *f;
    char *line = NULL;
    size_t n = 0;
    ssize_t len;
    char *tok;
    char *delim = " \t\r\n";
    int line_num = 0;

    int addr;

    f = fopen(filename, "r");
    if (!f) {
        perror("open");
        exit(1);
    }

    while (1) {
        len = getline(&line, &n, f);
        line_num++;

        if (len < 0)
            break;

        if (line[len-1] != '\n') {
            fprintf(stderr, "%s, line %d: Line too long\n", filename, line_num);
            exit(1);
        }

        for (tok = strtok(line, delim); tok; tok = strtok(NULL, delim)) {
            if (tok[0] == '#')
                break;

            addr = strtol(tok, NULL, 0);

            if (! (1 <= addr && addr <= 0xff-1)) {
                fprintf(stderr, "%s, line %d: Unparsed LED address\n", filename, line_num);
                exit(1);
            }

            leds[num_leds].addr = addr;
            leds[num_leds].p.r = 0;
            leds[num_leds].p.g = 0;
            leds[num_leds].p.b = 0;
            num_leds++;

            if (num_leds > MAX_LEDS) {
                fprintf(stderr, "%s: More than %d addresses found?\n", filename, MAX_LEDS);
                exit(1);
            }
        }
    }

    if (!num_leds) {
        fprintf(stderr, "%s: No addresses specified?\n", filename);
        exit(1);
    }

    if (line)
        free(line);

    fclose(f);
}

int main(int argc, char *argv[])
{
    int option_index = 0;
    int c;

    struct option long_options[] = {
        { "line",          required_argument,  NULL, 'l' },
        { "baud",          required_argument,  NULL, 's' },
        { "speed",         required_argument,  NULL, 's' },
        { "file",          required_argument,  NULL, 'f' },
        { "port",          required_argument,  NULL, 'p' },
        { "old_proto",     no_argument,        NULL, 'o' },
        { "old_protocol",  no_argument,        NULL, 'o' },
        { 0, 0, 0, 0}
    };

    char *optstring = "l:s:f:p:on:";

    while (1) {
        c = getopt_long(argc, argv, optstring, long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'f': config      = optarg;       break;
            case 'l': device      = optarg;       break;
            case 's': baud        = atoi(optarg); break;
            case 'p': port        = atoi(optarg); break;
            case 'o': old_proto   = 1;            break;
            case 'n': sample_time = atof(optarg); break;

            default:
                usage(argv[0]);
        }
    }

    if (!config)
        usage(argv[0]);

    if (!device)
        usage(argv[0]);

    if (sample_time <= 0) {
        fprintf(stderr, "sample_time must be greater than 0\n");
        exit(1);
    }

    read_config(config);
    fprintf(stderr, "Using device %s, baud %d\n", device, baud);

    if (old_proto)
        fprintf(stderr, "Using old protocol\n");

    setlinebuf(stdout);

    fd = serial_open(device, baud);
    source = opc_new_source(port);
    refresh();

    while (1) {
        struct timeval now, start, diff;
        double seconds;

        gettimeofday(&start, NULL);
        frames = 0;

        do {
            opc_receive(source, handler, 100);
            gettimeofday(&now, NULL);
            timersub(&now, &start, &diff);
            seconds = diff.tv_sec + diff.tv_usec/1000000;
        } while (seconds < sample_time);

        printf("%.1f frames/second\n", frames/seconds);
    }

    return 0;
}
