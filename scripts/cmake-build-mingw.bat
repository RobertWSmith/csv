@echo on
setlocal

:: default target to 'all', but allow the first argument to be the target
if not [%1] == [] (
  set target=%1
  set build_options=
) else (
  set target=all
  set build_options=-j4
)

pushd %~dp0
set script_dir=%CD%

cd ..
set PROJECT_ROOT=%CD%

for %%I in (.) do set PROJECT_DIRNAME=%%~nxI

cd ..
set BUILD_DIR=%CD%\%PROJECT_DIRNAME%-build-mingw

if not "%target" == "all" (
  set clean_first=
) else (
  set clean_first=--clean-first
  mkdir "%BUILD_DIR% > nul 2>&1
)

cd %BUILD_DIR%

@echo on

cmake --build "%BUILD_DIR%" %clean_first% --target %target% -- %build_options%
