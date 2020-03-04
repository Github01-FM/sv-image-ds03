#!/bin/sh

export LD_LIBRARY_PATH=/usr/lib

if [ ! -d /media/mtpfs ]; then
    mkdir -p /media/mtpfs
fi
mtpfs -f -o allow_other /media/mtpfs
