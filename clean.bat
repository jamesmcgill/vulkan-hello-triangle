:: Deletes any previous cache and build files by deleting entire
:: build directories created by either cmake and cl build scripts

@echo off
set BASE_PATH=%~dp0
set CMAKE_BUILD_PATH=%BASE_PATH%build\
set CL_BUILD_PATH=%BASE_PATH%clbuild\


if not exist %CMAKE_BUILD_PATH% goto NO_CMAKE_DIR
echo Deleting CMake Build Path...
rmdir /S /Q %CMAKE_BUILD_PATH%
:NO_CMAKE_DIR

if not exist %CL_BUILD_PATH% goto NO_CL_DIR
echo Deleting CL Build Path...
rmdir /S /Q %CL_BUILD_PATH%
:NO_CL_DIR
