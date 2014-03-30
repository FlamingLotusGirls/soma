#!/bin/bash

cd $(dirname $0)

./send.pl -v --loop 0.75 \
	rgblatch,0xff,01_00_00	\
	rgblatch,0xff,00_01_00	\
	rgblatch,0xff,00_00_01	\
	\
	rgblatch,0xff,00_00_00	\
	\
	rgblatch,0xff,ff_00_00	\
	rgblatch,0xff,00_ff_00	\
	rgblatch,0xff,00_00_ff	\
	\
	rgblatch,0xff,00_00_00	\
	\
	rgblatch,0xff,ff_ff_ff	\
	\
	rgblatch,0xff,00_00_00	\
