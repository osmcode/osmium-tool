@ECHO OFF
SETLOCAL

rem Recommended by vcpkg paths is C:\src\vcpkg or C:\dev\vcpkg
rem see: https://github.com/microsoft/vcpkg#quick-start-windows
IF EXIST "C:\vcpkg\vcpkg.exe" (
  SET vcpkg_path=C:\vcpkg
) ELSE (
  IF EXIST "C:\dev\vcpkg\vcpkg.exe" (
    SET vcpkg_path=C:\dev\vcpkg
  ) ELSE (
    IF EXIST "C:\src\vcpkg\vcpkg.exe" (
      SET vcpkg_path=C:\src\vcpkg
    ) ELSE (
	  ECHO vcpkg path doesn't exist
	  EXIT /b 1
	)
  )
)

ECHO install libraries 

%vcpkg_path%\vcpkg.exe install ^
	boost-iterator:x64-windows ^
	boost-program-options:x64-windows ^
	boost-variant:x64-windows ^
	bzip2:x64-windows ^
	expat:x64-windows ^
	lz4:x64-windows ^
	zlib:x64-windows 
	rem --triplet=x64-windows
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO clone dependencies

IF NOT EXIST "..\libosmium\" (
  git clone git@github.com:osmcode/libosmium.git ..\libosmium
  IF %ERRORLEVEL% NEQ 0 GOTO ERROR
)
IF NOT EXIST "..\protozero\" (
  git clone git@github.com:mapbox/protozero.git ..\protozero
  IF %ERRORLEVEL% NEQ 0 GOTO ERROR
)

ECHO cmake build files

if not exist "build\" (
  mkdir build
)

cmake -LA . -DCMAKE_TOOLCHAIN_FILE=%vcpkg_path%\scripts\buildsystems\vcpkg.cmake -DOsmium_DEBUG=TRUE -DPROTOZERO_INCLUDE_DIR=..\protozero\include -B build
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO cmake build binaries
ECHO config^: %config%

SET config=Release
cmake --build build --config %config% --verbose
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO cmake test

ctest --output-on-failure -C %config% --test-dir build
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

:DONE

ECHO build completed successfully, output path^: .\build\src\%config%
rem start .\build\src\%config%

EXIT /b 0

:ERROR
EXIT /b %ERRORLEVEL%