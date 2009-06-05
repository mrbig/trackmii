#!/bin/bash

gcc -c -DLIN=1 -I../SDK/CHeaders/XPLM/ -I../SDK/CHeaders/Widgets/ trackmii_plugin.c
gcc -c -DLIN=1 -I../SDK/CHeaders/XPLM/ -I../SDK/CHeaders/Widgets/ pose.c
ld -shared -lm -lcwiid -o trackmii_plugin.xpl  trackmii_plugin.o pose.o
cp trackmii_plugin.xpl /opt/X-Plane\ 9\ Demo/Resources/plugins/
