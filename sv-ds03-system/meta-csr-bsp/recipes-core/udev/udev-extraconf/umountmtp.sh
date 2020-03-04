#!/bin/bash
MTPFS_PID=$(ps | grep mtpfs | grep -v grep | cut -c 1-6)

if [ -n "$MTPFS_PID" ]; then
    umount /media/mtpfs
    echo $MTPFS_PID | xargs kill -s 9
fi
