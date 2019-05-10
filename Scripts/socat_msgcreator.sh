#!/bin/bash

socat -d -d pty,link=/home/pi/Desktop/SINF/portas/msgcreator,raw,echo=0 pty,link=/home/pi/Desktop/SINF/portas/mainrecebe,raw,echo=0