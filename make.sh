#!/bin/sh

# apt-get install g++

c++ -Wpedantic -Wall -Wextra -o texture str.cpp data.cpp image.cpp tga.cpp bmp.cpp wad.cpp lmp.cpp mip.cpp wal.cpp m8.cpp m32.cpp main.cpp
