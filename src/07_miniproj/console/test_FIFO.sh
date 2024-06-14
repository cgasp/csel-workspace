#!/bin/bash

echo -n "$(date '+%H:%M:%S') "; echo "./console freq 5"; ./console freq 5; \
sleep 1; \
echo -n "$(date '+%H:%M:%S') "; echo "./console freq 101"; ./console freq 101; \
sleep 1; \
echo -n "$(date '+%H:%M:%S') "; echo "./console freq -1"; ./console freq -1; \
sleep 1; \
echo -n "$(date '+%H:%M:%S') "; echo "./console freq 20"; ./console freq 20; \
sleep 1; \
echo -n "$(date '+%H:%M:%S') "; echo "./console mode auto"; ./console mode auto; \
sleep 1; \
echo -n "$(date '+%H:%M:%S') "; echo "./console mode manual"; ./console mode manual; \
sleep 1; \
echo -n "$(date '+%H:%M:%S') "; echo "./console mode auto"; ./console mode auto; \
sleep 1; \
echo -n "$(date '+%H:%M:%S') "; echo 'echo "freq 20" > /tmp/miniproj_fifo'; echo "freq 20" > /tmp/miniproj_fifo; \
sleep 1; \
echo -n "$(date '+%H:%M:%S') "; echo 'echo "mode auto" > /tmp/miniproj_fifo'; echo "mode auto" > /tmp/miniproj_fifo; \
sleep 1;

