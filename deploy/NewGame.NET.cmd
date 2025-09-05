for /f "tokens=1-3* delims=/: " %%a in ('echo %TIME%') do (set STRTIME=%%a.%%b.%%c)
for /f "tokens=1-3* delims=/: " %%a in ('echo %DATE%') do (set STRDATE=%%a.%%b.%%c)
set SUFFIX=%STRDATE%-%STRTIME%
set TARGET=Results.%SUFFIX%
set CONTINUE=ContinueGame_%SUFFIX%.cmd
mkdir %TARGET%
cd %TARGET%
mkdir saves
copy ..\release\ConsoleSeparate.exe ConsoleSeparate.exe
copy ..\release\holdem.release.exe holdem.console.exe
copy ..\release\holdemdb.release.ini holdemdb.ini
@REM copy ..\release\tee.exe tee.exe
echo cd %TARGET% > ..\%CONTINUE%
echo ConsoleSeparate.exe holdem.console.exe^>^> game.log >> ..\%CONTINUE%
@REM echo pause >> ..\%CONTINUE%
@REM echo del ..\%CONTINUE% >> ..\%CONTINUE%
@echo off
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo Game starting!
echo.
ConsoleSeparate.exe holdem.console.exe Player >> game.log
@REM del ..\%CONTINUE%
@REM pause
