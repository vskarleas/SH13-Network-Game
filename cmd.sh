#! /bin/sh
gcc -o sh13 -I/us/include/SDL2 sh13_4.c -1SDL2_image -LSDL2_ttf -LSDL2 - Ipthread
gcc -o server server_4.c