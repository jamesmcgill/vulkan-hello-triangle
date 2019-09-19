:: Generates CMake Cache
:: Deletes any previous cache and build files

@echo off
set BASE_PATH=%~dp0
set BUILD_PATH=%BASE_PATH%build\

:: Delete any existing cache (by deleting entire build directory)
if not exist %BUILD_PATH% goto MISSING_BUILD_DIR
echo.
echo Deleting Build Path...
rmdir /S /Q %BUILD_PATH%
:MISSING_BUILD_DIR
if not exist %BUILD_PATH% mkdir %BUILD_PATH%


:: Generate new cache
:: looks for different versions of MSVC (newest first)
pushd %BUILD_PATH%

echo.
echo Looking for Visual Studio 16 2019
call cmake -G "Visual Studio 16 2019" .. && (
popd
goto SUCCESS
) || (
del CMakeCache.txt
)

echo.
echo Looking for Visual Studio 15 2017
call cmake -G "Visual Studio 15 2017" .. && (
popd
goto SUCCESS
) || (
del CMakeCache.txt
)

echo.
echo Looking for Visual Studio 14 2015
call cmake -G "Visual Studio 14 2015" .. && (
popd
goto SUCCESS
) || (
del CMakeCache.txt
)


:: No MSVC found
popd
echo.
echo.
echo Couldn't find any supported MSVC compiler on the PATH
echo Check you are in the vsdevcmd shell
echo Check this file (build-generate.bat) supports your vsdevcmd compiler version (add if necessary)
exit

:SUCCESS
echo[
echo CMake Cache Generation Done!
echo ----------------------------

