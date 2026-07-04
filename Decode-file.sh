#!/bin/bash

#bin/Decode253 fd:00:0d:8b:20:64:36:00:03:02:58:d8:0e

#INFILE=Comun1-rosa.cat253-ALL.txt
INFILE=Comun1-rosa.cat253.txt
[ "$1" != "" ] && INFILE="$1"


cat "$INFILE" | while read dataline ; do
  if [ ${#dataline} -gt 51 ] ; then # skip empty packets
    prefix=`echo ${dataline:0:50} | tr '\t' ' '`
    datahex="${dataline:51}"
    echo "-==-==-==-==-==-==-==-==|${prefix}|==-==-==-==-==-==-==-==-"
    bin/Decode253  "${datahex}"
  fi
done