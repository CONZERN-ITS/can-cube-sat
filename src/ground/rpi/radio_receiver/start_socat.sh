#!/usr/bin/bash
socat -d -d /dev/serial0,raw,b9600,clocal=0,ixoff=0,ixon=0,cstopb=0,parenb=0,echo=0 tcp-listen:2020,reuseaddr
