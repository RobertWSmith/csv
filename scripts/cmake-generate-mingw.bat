@echo off
setlocal

pushd %~dp0
set script_dir=%CD%

:: cleanup cmake generated files and log files
call clean.bat

cd ..
set PROJECT_ROOT=%CD%

for %%I in (.) do set PROJECT_DIRNAME=%%~nxI

cd ..
set BUILD_DIR=%CD%\%PROJECT_DIRNAME%-build
mkdir "%BUILD_DIR% > nul 2>&1

cd %BUILD_DIR%

:: turn echo on for cmake command, it needs to be seen if issues arise
@echo on

:: funny trick, adding the caret is how multiline windows commands can represent
:: a single line command
cmake -G "MinGW Makefiles"^
 -D CMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON^
 -D BUILD_SHARED_LIBS:BOOL=OFF^
 -D CMAKE_BUILD_TYPE:STRING=Debug^
 %PROJECT_ROOT%
