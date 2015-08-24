@ECHO OFF
SETLOCAL

ECHO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ %~f0 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

ECHO CWD^: %CD%

SET config=RelWithDebInfo
SET LOCAL_BUILD=1
SET HOME=%CD%


:: OVERRIDE PARAMETERS
:NEXT_ARG

IF '%1'=='' GOTO ARGS_DONE
ECHO setting %1
SET %1
SHIFT
GOTO NEXT_ARG

:ARGS_DONE


SET PATH=C:\Program Files\7-Zip;%PATH%

IF EXIST build ECHO deleting build dir... && RD /S /Q build
IF %ERRORLEVEL% NEQ 0 ECHO error during "delete build dir" && GOTO ERROR

::delete libosmim which gets cloned during build
IF EXIST ..\libosmium ECHO deleting libosmium && RD /S /Q ..\libosmium
IF %ERRORLEVEL% NEQ 0 ECHO error during "delete libosmim" && GOTO ERROR

CALL scripts\build-appveyor.bat
IF %ERRORLEVEL% NEQ 0 ECHO error during "build-appveyor.bat" && GOTO ERROR


GOTO DONE


:ERROR
SET EL=%ERRORLEVEL%
ECHO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ERROR^: %~f0 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ECHO ERRORLEVEL^: %EL%

:DONE
ECHO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DONE %~f0 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CD %HOME%
EXIT /b %EL%
