@ECHO OFF
SETLOCAL
SET EL=0

ECHO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ %~f0 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

IF NOT DEFINED LOCAL_BUILD SET LOCAL_BUILD=0

::show all availble env vars
SET
ECHO cmake on AppVeyor
cmake -version
IF LOCAL_BUILD EQU 0 IF %ERRORLEVEL% NEQ 0 GOTO ERROR

CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

SET PATH=%CD%\cmake-3.1.0-win32-x86\bin;%PATH%
SET LODEPSDIR=%CD%\libosmium-deps
::libexpat.dll
SET PATH=%LODEPSDIR%\expat\lib;%PATH%
::zlibwapi.dll
SET PATH=%LODEPSDIR%\zlib\lib;%PATH%
::convert backslashes in bzip2 path to forward slashes
::cmake cannot find it otherwise
SET LIBBZIP2=%LODEPSDIR%\bzip2\lib\libbz2.lib
SET LIBBZIP2=%LIBBZIP2:\=/%

ECHO downloading cmake ...
IF EXIST cm.7z (ECHO already downloaded) ELSE (powershell Invoke-WebRequest https://mapbox.s3.amazonaws.com/windows-builds/windows-build-deps/cmake-3.1.0-win32-x86.7z -OutFile cm.7z)
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO downloading binary dependencies ...
IF EXIST lodeps.7z (ECHO already downloaded) ELSE (powershell Invoke-WebRequest https://mapbox.s3.amazonaws.com/windows-builds/windows-build-deps/libosmium-deps-win-14.0-x64.7z -OutFile lodeps.7z)
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO extracting cmake ...
IF EXIST cmake-3.1.0-win32-x86 (ECHO already extracted) ELSE (7z x -y cm.7z | %windir%\system32\FIND "ing archive")
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO extracting binary dependencies ...
IF EXIST libosmium-deps (ECHO already extracted) ELSE (7z x -y lodeps.7z | %windir%\system32\FIND "ing archive")
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO %LODEPSDIR%
DIR %LODEPSDIR%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
ECHO our own cmake && cmake -version
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

CD ..
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
git clone --depth 1 https://github.com/osmcode/libosmium.git
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

CD osmium-tool
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

MKDIR build
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

CD build
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO %config%

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

ECHO building ...
msbuild osmium.sln /p:Configuration=%config% /toolsversion:14.0 /p:Platform=x64 /p:PlatformToolset=v140
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO running tests ...
ctest --output-on-failure -C %config%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR


GOTO DONE

:ERROR
SET EL=%ERRORLEVEL%
ECHO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ERROR^: %~f0 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ECHO ERRORLEVEL^: %EL%

:DONE
ECHO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DONE %~f0 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CD %HOME%
EXIT /b %EL%

