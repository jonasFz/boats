#!/bin/bash
gcc *.c -g -std=c99 -Wall -o boats -I /usr/include/X11 -L /usr/X11/lib -l X11 -L /usr/include/GL -l GL -l GLEW -l GLU -lm
