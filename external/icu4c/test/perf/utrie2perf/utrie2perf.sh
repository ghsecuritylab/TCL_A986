#!/bin/sh
set -ex

PERF=~/svn.icu/utf8-dev/test/perf/utrie2perf/utrie2perf

for file in udhr_eng.txt \
            udhr_deu.txt \
            udhr_fra.txt \
            udhr_rus.txt \
            udhr_tha.txt \
            udhr_jpn.txt \
            udhr_cmn.txt \
            udhr_jpn.html; do
  $PERF CheckFCD            -f ~/udhr/$file -v -e UTF-8 --passes 3 --iterations 30000
# $PERF CheckFCDAlwaysGet   -f ~/udhr/$file -v -e UTF-8 --passes 3 --iterations 30000
# $PERF CheckFCDUTF8        -f ~/udhr/$file -v -e UTF-8 --passes 3 --iterations 30000
  $PERF ToNFC               -f ~/udhr/$file -v -e UTF-8 --passes 3 --iterations 30000
  $PERF GetBiDiClass        -f ~/udhr/$file -v -e UTF-8 --passes 3 --iterations 30000
done
