:: Generates CMake Cache
:: Deletes any previous cache and build files

@echo off
set BASE_PATH=%~dp0
set BUILD_PATH=%BASE_PATH%build\
set GENERATOR="Visual Studio 16 2019"

:: This script needs vcpkg and needs to know where it is installed
if [%VCPKG_PATH%] == [] goto MISSING_VCPKG

:: Delete any existing cache (by deleting entire build directory)
if not exist %BUILD_PATH% goto MISSING_BUILD_DIR
echo.
echo Deleting Build Path...
rmdir /S /Q %BUILD_PATH%
:MISSING_BUILD_DIR
if not exist %BUILD_PATH% mkdir %BUILD_PATH%


:: Generate new cache
pushd %BUILD_PATH%
echo.
echo Using generator: %GENERATOR%
call cmake -A x64 -G %GENERATOR% .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%/scripts/buildsystems/vcpkg.cmake && (
popd
goto SUCCESS
) || (
del CMakeCache.txt
)

:: CMake failed if we reached this point
popd
echo.
echo.
echo If CMake errors are related to not finding any supported MSVC compiler
echo Check you are in the vsdevcmd shell
echo Check this file (build-generate.bat) supports your vsdevcmd compiler version (add if necessary)
exit


:MISSING_VCPKG
echo.
echo This script assumes vcpkg is used to provide build dependencies
echo (https://github.com/Microsoft/vcpkg)
echo - Please install vcpkg and create an environment variable VCPKG_PATH pointing to it
echo.
exit

:SUCCESS
echo[
echo CMake Cache Generation Done!
echo ----------------------------

