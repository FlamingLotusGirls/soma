#!/usr/bin/perl
# vi:set ai sw=4 ts=4 et smarttab:
#
# Copyright 2014 Michael Toren <mct@toren.net>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

use Data::Dumper;
use Getopt::Long qw(:config  gnu_getopt);
use Device::SerialPort;

use strict;
use warnings;

$|++;

my @crc8_table = (
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
    0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
    0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
    0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2, 0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
    0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
    0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
    0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
    0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
    0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
    0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
    0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3,
);

my $FRAME_END    = 0xDA;
my $FRAME_ESC    = 0xDB;
my $FRAME_TEND   = 0xDC;
my $FRAME_TESC   = 0xDD;
my $NOOP         = 0x11;
my $BAUD         = 0x12;
my $LATCH        = 0x13;
my $RGBDATA      = 0x14;
my $RGBLATCH     = 0x15;

my $device  = (glob "/dev/ttyUSB*")[0] || "";
my $verbose = 0;
my $dry_run = 0;
my $raw     = 0;
my $baud    = 230400; # 115200*2
my $allbaud;
my $loop;
my $port;
my $help;
my $sendbytes;

my @allbauds = (9600, 19200, 38400, 115200, 230400);

sub open_serial {
    print STDERR "Opening $device at $baud\n"
        if $verbose;

    $port = new Device::SerialPort($device) or die "$device: $!";
    die "No ioctl?" unless $port->can_ioctl();
    $port->baudrate($baud);
    $port->databits(8); 
    $port->parity("none"); 
    $port->stopbits(1); 
    $port->handshake("none"); 
    $port->write_settings;

}

sub send_raw_bytes {
    s/[\[\],]//g for (@ARGV);

    my @data = map { /^0/ ? oct : $_ } @ARGV;
    my $buf = join '', map { chr } @data;

    print ">> ";
    print map { sprintf "%02x ", ord($_) } split //, $buf;
    print "\n";

    $port->write($buf);
}

sub escape {
    my @out;
    for my $i (@_) {
        if ($i == $FRAME_END) {
            push @out, $FRAME_ESC, $FRAME_TEND;
        }
        elsif ($i == $FRAME_ESC) {
            push @out, $FRAME_ESC, $FRAME_TESC;
        }
        else {
            push @out, $i;
        }
    }
    return @out;
}

sub packet {
    my $crc = 0;
    my @buf;

    $_[0] = oct($_[0]) if $_[0] =~ /^0/; # support decimal, binary, octal, and hex

    for my $i (@_) {
        $crc = $crc8_table[ $crc ^ $i ];
        push @buf, $i;
    }
    push @buf, $crc;

    @buf = ($FRAME_END, escape(@buf), $FRAME_END);

    if ($raw) {
        print map { chr } @buf;
    }

    if ($verbose) {
        print STDERR ">> ";
        print STDERR map { sprintf "%02x ", $_ } @buf;
        print STDERR "\n";
    }


    unless ($dry_run) {
        my $buf = join '', map { chr } @buf;

        if ($allbaud) {
            for my $baud (@allbauds) {
                printf STDERR "Baud $baud\n";
                $port->baudrate($baud);
                $port->write_settings;
                $port->write($buf);
            }
        }

        else {
            $port->write($buf);
        }
    }
}

sub usage {
    die <<EOT

Usage: $0 [--device=SERIAL] [--baud=BAUD] [--dry-run] [--verbose] [--raw] CMDS

Where CMDS are:

    NOOP,ADDR
    LATCH,ADDR
    BAUD,NEW_RATE
    RGB,ADDR,0xffffff,[...]
    RGBLATCH,ADDR,0xffffff,[...]

EOT
}

##

my $result = GetOptions (
        "device|line|l=s"   => \$device,
        "verbose|v+"        => \$verbose,
        "dry-run|n"         => \$dry_run,
        "baud|s=i"          => \$baud,
        "raw|r+"            => \$raw,
        "loop=f"            => \$loop,
        "help|h"            => \$help,
        "sendbytes"         => \$sendbytes,
        "allbaud|a"         => \$allbaud,
    ) or die;

die "Neither a device, nor dry-run specified\n"
    unless ($device or $dry_run);

if (!@ARGV or $help) {
    usage;
}

if ($raw) {
    $verbose = 0;
}

unless ($dry_run) {
    open_serial;
}

if ($sendbytes) {
    send_raw_bytes;
    exit;
}

do {
    for my $i (@ARGV) {
        my ($cmd, $data) = split /[=,]/, $i, 2;
        my @data = split /,/, $data || "";
        $cmd = lc $cmd;

        if ($cmd =~ /^noo?p$/) {
            my $addr = shift @data || 0xff;
            packet $addr, $NOOP;
        }

        elsif ($cmd eq "latch") {
            my $addr = shift @data || 0xff;
            packet $addr, $LATCH;
        }

        elsif ($cmd eq "baud") {
            my $rate = shift @data or die "No baudrate specified\n";
            packet 0xff, $BAUD, ($rate >> 16 & 0xff),
                                ($rate >>  8 & 0xff),
                                ($rate >>  0 & 0xff);
        }

        elsif ($cmd =~ /^(rgb|rgbdata|rgblatch)$/) {
            $cmd = ($cmd eq "rgblatch") ? $RGBLATCH : $RGBDATA;
            my $base_addr = shift @data;

            my @colors;
            my $num_colors;
            for my $i (@data) {
                $i =~ s/_//g;
                die "Color format error" unless $i =~ /^(?:0x)? ([0-9a-f]{2}) ([0-9a-f]{2}) ([0-9a-f]{2}) $/xi;
                push @colors, hex $1;
                push @colors, hex $2;
                push @colors, hex $3;
                $num_colors++;
            }

            die "No colors specified" unless @colors;
            packet $base_addr, $cmd, $num_colors, @colors;
        }

        elsif ($cmd eq "sleep") {
            my $time = shift @data or die;
            select undef, undef, undef, $time;
        }

        else {
            die "Unknown packet type: $cmd\n";
        }

        select undef, undef, undef, $loop if $loop;
    }
} while (defined $loop);
