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
 * Copyright 2014 Michael C. Toren <mct@toren.net>
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
#include <stdint.h>
#include <stdio.h>

#ifndef CNEW_RTSCTS
#define CNEW_RTSCTS CRTSCTS
#endif

uint32_t baud = 230400; // 115200*2
char *device;

// http://www.cmrr.umn.edu/~strupp/serial.html
int serial_open(char *device, uint32_t baud)
{
    int fd;
    struct termios options, options_new;

    switch (baud) {
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

void serial_write(int fd, uint8_t *buf, size_t count)
{
    ssize_t n = write(fd, buf, count);

    if (n < 0) {
        perror("write");
        exit(1);
    }

    if (n != count) {
        fprintf(stderr, "Short write:  Tired to write %zd, only wrote %zd\n", count, n);
        exit(1);
    }
}

void copy(int in, int out)
{
    uint8_t buf[BUFSIZ];
    ssize_t n;

    while (1) {
        n = read(in, buf, BUFSIZ);

        if (n < 0) {
            perror("read");
            exit(1);
        }

        if (n == 0) {
            fprintf(stderr, "EOF\n");
            exit(1);
        }

        serial_write(out, buf, n);
        fprintf(stderr, "Wrote %zd\n", n);
    }
}

void usage(char *name)
{
    fprintf(stderr, "Usage: %s -l <device> [-s <speed>]\n", name);
    exit(1);
}

int main(int argc, char *argv[])
{
    int option_index = 0;
    int c;

    struct option long_options[] = {
        { "line",   required_argument,  NULL, 'l' },
        { "baud",   required_argument,  NULL, 's' },
        { "speed",  required_argument,  NULL, 's' },
        { 0, 0, 0, 0}
    };

    char *optstring = "l:s:";

    while (1) {
        c = getopt_long (argc, argv, optstring, long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'l': device = optarg;       break;
            case 's': baud   = atoi(optarg); break;

            default:
                usage(argv[0]);
        }
    }

    if (!device)
        usage(argv[0]);

    fprintf(stderr, "Using device %s, baud %d\n", device, baud);

    int fd = serial_open(device, baud);
    copy(fileno(stdin), fd);

    return 0;
}
