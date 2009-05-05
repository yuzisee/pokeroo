#! /bin/sh

SUFFIX=`date '+%F..-%H.%M.%S.%N'`
TARGET=Results.$SUFFIX
CONTINUE=ContinueGame_${SUFFIX}.cmd
mkdir $TARGET
cd $TARGET
mkdir saves
cp -v ../release/consoleseparate.py .
cp -v ../release/holdem holdem.console
cp -v ../release/holdemdb.release.ini holdemdb.ini

echo cd $TARGET > ../$CONTINUE
echo 'python consoleseparate.py holdem.console >> game.log' >> ../$CONTINUE
#@REM echo pause >> ..\%CONTINUE%
#@REM echo del ..\%CONTINUE% >> ..\%CONTINUE%
#@echo off
echo
echo
echo
echo
echo
echo
echo
echo
echo
echo
echo
echo
echo
echo
echo
echo Game starting!
echo
python consoleseparate.py holdem.console Player >> game.log
#@REM del ..\%CONTINUE%
#@REM pause
