#!/bin/bash

socat -d -d pty,link=/home/pi/Desktop/SINF/portas/mainenvia,raw,echo=0 pty,link=/home/pi/Desktop/SINF/portas/rgbmatrix,raw,echo=0