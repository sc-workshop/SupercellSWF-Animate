@echo off
setlocal enabledelayedexpansion

set "GN_EXECUTABLE=%~1"
set "SKIA_ROOT=%~2"
set "SKIA_BUILD_DIR=%~3"
set "SKIA_ARGS=%~4"
set "BUILD_TYPE=%~5"

if /I "%BUILD_TYPE%"=="Release" (
    set "SKIA_ARGS=%SKIA_ARGS% extra_cflags=[\"/MD\"] extra_cflags_cc=[\"/MD\"]"
)

echo Running GN gen for %SKIA_ROOT%...
"%GN_EXECUTABLE%" gen "%SKIA_BUILD_DIR%" --root="%SKIA_ROOT%" --args="%SKIA_ARGS%"

endlocal