@ECHO OFF
SETLOCAL
SET EL=0

ECHO ~~~~~~ %~f0 ~~~~~~


::show all available env vars
SET
ECHO cmake on AppVeyor
cmake -version

ECHO activating VS cmd prompt && CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

SET otdir=%CD%
SET PATH=%otdir%\cmake-3.1.0-win32-x86\bin;%PATH%
SET LODEPSDIR=%otdir%\libosmium-deps
::libexpat.dll
SET PATH=%LODEPSDIR%\expat\lib;%PATH%
::zlibwapi.dll
SET PATH=%LODEPSDIR%\zlib\lib;%PATH%
::convert backslashes in bzip2 path to forward slashes
::cmake cannot find it otherwise
SET LIBBZIP2=%LODEPSDIR%\bzip2\lib\libbz2.lib
SET LIBBZIP2=%LIBBZIP2:\=/%

IF NOT EXIST cm.7z ECHO downloading cmake... && powershell Invoke-WebRequest https://mapbox.s3.amazonaws.com/windows-builds/windows-build-deps/cmake-3.1.0-win32-x86.7z -OutFile cm.7z
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

IF NOT EXIST lodeps.7z ECHO downloading binary dependencies... && powershell Invoke-WebRequest https://mapbox.s3.amazonaws.com/windows-builds/windows-build-deps/libosmium-deps-win-14.0-x64.7z -OutFile lodeps.7z
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

IF NOT EXIST cmake-3.1.0-win32-x86 ECHO extracting cmake... && 7z x cm.7z | %windir%\system32\find "ing archive"
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

IF NOT EXIST %LODEPSDIR% ECHO extracting binary dependencies... && 7z x lodeps.7z | %windir%\system32\find "ing archive"
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO %LODEPSDIR%
DIR %LODEPSDIR%
::TREE %LODEPSDIR%
ECHO our own cmake
cmake -version

CD %otdir%\..

IF NOT EXIST libosmium ECHO cloning libosmium && git clone --depth 1 https://github.com/osmcode/libosmium.git
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
CD libosmium
git fetch
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
git pull
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

CD %otdir%
IF EXIST build ECHO deleting build dir... && RD /Q /S build
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

MKDIR build
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

CD build
ECHO config^: %config%

cmake .. -LA -G "Visual Studio 14 Win64" ^
-DOsmium_DEBUG=TRUE ^
-DCMAKE_BUILD_TYPE=%config% ^
-DBOOST_ROOT=%LODEPSDIR%\boost ^
-DBoost_PROGRAM_OPTIONS_LIBRARY=%LODEPSDIR%\boost\lib\libboost_program_options-vc140-mt-1_58.lib ^
-DZLIB_LIBRARY=%LODEPSDIR%\zlib\lib\zlibwapi.lib ^
-DZLIB_INCLUDE_DIR=%LODEPSDIR%\zlib\include ^
-DEXPAT_LIBRARY=%LODEPSDIR%\expat\lib\libexpat.lib ^
-DEXPAT_INCLUDE_DIR=%LODEPSDIR%\expat\include ^
-DBZIP2_LIBRARIES=%LIBBZIP2% ^
-DBZIP2_INCLUDE_DIR=%LODEPSDIR%\bzip2\include
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

msbuild osmium.sln ^
/p:Configuration=%config% ^
/toolsversion:14.0 ^
/p:Platform=x64 ^
/p:PlatformToolset=v140
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ctest --output-on-failure -C %config%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR


GOTO DONE

:ERROR
ECHO ~~~~~~ ERROR %~f0 ~~~~~~
SET EL=%ERRORLEVEL%

:DONE
IF %EL% NEQ 0 ECHO. && ECHO !!! ERRORLEVEL^: %EL% !!! && ECHO.
ECHO ~~~~~~ DONE %~f0 ~~~~~~

EXIT /b %EL%
