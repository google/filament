@echo off
echo HLSL console tools
echo.
echo   hctbld       - sets the current directory to HLSL_BLD_DIR
echo   hctbuild     - builds the product and test binaries
echo   hcthelp      - prints this help message
echo   hctspeak     - says something - useful to call out after a long command
echo   hctshortcut  - creates a desktop shortcut
echo   hctsrc       - sets the current directory to HLSL_SRC_DIR
echo   hcttest      - runs tests
echo   hcttodo      - enumerates TODO comments
echo   hcttools     - changes the directory to console tools
echo   hctvs        - launches Visual Studio with the built solution
echo.
echo Environment variables overrides
echo   BUILD_ARCH - default build architecture, one of Win32, x64, arm, currently '%BUILD_ARCH%'
echo   HCT_EXTRAS - allows scripts to be hooked in, currently '%HCT_EXTRAS%'
echo.
