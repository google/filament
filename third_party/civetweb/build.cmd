:: Make sure the extensions are enabled
@verify other 2>nul
@setlocal EnableDelayedExpansion
@if errorlevel 1 (
  call :print_usage "Failed to enable extensions"
  exit /b 1
)

::Change the code page to unicode
@chcp 65001 1>nul 2>nul
@if errorlevel 1 (
  call :print_usage "Failed to change the code page to unicode"
  exit /b 1
)

:: Set up some global variables
@set project=civetweb
@set "script_name=%~nx0"
@set "script_folder=%~dp0"
@set "script_folder=%script_folder:~0,-1%"
@set "output_path=%script_folder%\output"
@set "build_path=%output_path%\build"
@set "install_path=%output_path%\install"
@set build_shared=OFF
@set build_type=Release
@set dependency_path=%TEMP%\%project%-build-dependencies

:: Check the command line parameters
@set logging_level=1
@set "options=%* "
@if not "!options!"=="!options:/? =!" set usage="Convenience script to build %project% with CMake"
@for %%a in (%options%) do @(
  @set arg=%%~a
  @set arg=!arg: =!
  @set one=!arg:~0,1!
  @set two=!arg:~0,2!
  @if /i [!arg!] == [/q] set quiet=true
  @if /i [!two!] == [/v] call :verbosity "!arg!"
  @if /i [!arg!] == [/s] set build_shared=ON
  @if /i [!arg!] == [/d] set build_type=Debug
  @if /i not [!one!] == [/] (
    if not defined generator (
      set generator=!arg!
    ) else (
      set usage="Too many generators: !method! !arg!" ^
                "There should only be one generator parameter"
    )
  )
)
@if defined quiet (
  set logging_level=0
)
@if not defined generator (
  set generator=MSVC
)
@if /i not [%generator%] == [MinGW] (
  if /i not [%generator%] == [MSVC] (
    call :print_usage "Invalid argument: %generator%"
    exit /b 1
  )
)

:: Set up the logging
@set log_folder=%output_path%\logs
@call :iso8601 timestamp
@set log_path=%log_folder%\%timestamp%.log
@set log_keep=10

:: Only keep a certain amount of logs
@set /a "log_keep=log_keep-1"
@if not exist %log_folder% @mkdir %log_folder%
@for /f "skip=%log_keep%" %%f in ('dir /b /o-D /tc %log_folder%') do @(
  call :log 4 "Removing old log file %log_folder%\%%f"
  del %log_folder%\%%f
)

:: Set up some more global variables
@call :architecture arch
@call :windows_version win_ver win_ver_major win_ver_minor win_ver_rev
@call :script_source script_source
@if [%script_source%] == [explorer] (
  set /a "logging_level=logging_level+1"
)

:: Print the usage or start the script
@set exit_code=0
@if defined usage (
  call :print_usage %usage%
) else (
  call :main
  @if errorlevel 1 (
    @call :log 0 "Failed to build the %project% project"
    @set exit_code=1
  )
)

:: Tell the user where the built files are
@call :log 5
@call :log 0 "The built files are available in %install_path%"

:: Stop the script if the user double clicked
@if [%script_source%] == [explorer] (
  pause
)

@exit /b %exit_code%
@endlocal
@goto :eof

:: -------------------------- Functions start here ----------------------------

:main - Main function that performs the build
@setlocal
@call :log 6
@call :log 2 "Welcome to the %project% build script"
@call :log 6 "------------------------------------"
@call :log 6
@call :log 2 "This script builds the project using CMake"
@call :log 6
@call :log 2 "Generating %generator%..."
@call :log 6
@set methods=dependencies ^
             generate ^
             build ^
             install
@for %%m in (%methods%) do @(
  call :log 3 "Excuting the '%%m' method"
  call :log 8
  call :%%~m
  if errorlevel 1 (
    call :log 0 "Failed to complete the '%%~m' dependency routine"
    call :log 0 "View the log at %log_path%"
    exit /b 1
  )
)
@call :log 6 "------------------------------------"
@call :log 2 "Build complete"
@call :log 6
@endlocal
@goto :eof

:print_usage - Prints the usage of the script
:: %* - message to print, each argument on it's own line
@setlocal
@for %%a in (%*) do @echo.%%~a
@echo.
@echo.build [/?][/v[v...]^|/q][MinGW^|MSVC]
@echo.
@echo.  [MinGW^|(MSVC)]
@echo.              Builds the library with one of the compilers
@echo.  /s          Builds shared libraries
@echo.  /d          Builds a debug variant of the project
@echo.  /v          Sets the output to be more verbose
@echo.  /v[v...]    Extra verbosity, /vv, /vvv, etc
@echo.  /q          Quiets the output
@echo.  /?          Shows this usage message
@echo.
@endlocal
@goto :eof

:dependencies - Installs any prerequisites for the build
@setlocal EnableDelayedExpansion
@if errorlevel 1 (
  call :log 0 "Failed to enable extensions"
  exit /b 1
)
@call :log 5
@call :log 0 "Installing dependencies for %generator%"
@if /i [%generator%] == [MinGW] (
  call :mingw compiler_path
  @if errorlevel 1 (
    @call :log 5
    @call :log 0 "Failed to find MinGW"
    @exit /b 1
  )
  set "PATH=!compiler_path!;%PATH%"
  @call :find_in_path gcc_executable gcc.exe
  @if errorlevel 1 (
    @call :log 5
    @call :log 0 "Failed to find gcc.exe"
    @exit /b 1
  )
)
@if [%reboot_required%] equ [1] call :reboot
@endlocal & set "PATH=%PATH%"
@goto :eof

:generate - Uses CMake to generate the build files
@setlocal EnableDelayedExpansion
@if errorlevel 1 (
  call :log 0 "Failed to enable extensions"
  exit /b 1
)
@call :log 5
@call :log 0 "Generating CMake files for %generator%"
@call :cmake cmake_executable
@if errorlevel 1 (
  @call :log 5
  @call :log 0 "Need CMake to create the build files"
  @exit /b 1
)
@if /i [%generator%] == [MinGW] @(
  @set "generator_var=-G "MinGW Makefiles^""
)
@if /i [%generator%] == [MSVC] @(
  rem We could figure out the correct MSVS generator here
)
@call :iso8601 iso8601
@set output=%temp%\cmake-%iso8601%.log
@if not exist %build_path% mkdir %build_path%
@cd %build_path%
@"%cmake_executable%" ^
  !generator_var! ^
  -DCMAKE_BUILD_TYPE=!build_type! ^
  -DBUILD_SHARED_LIBS=!build_shared! ^
  "%script_folder%" > "%output%"
@if errorlevel 1 (
  @call :log 5
  @call :log 0 "Failed to generate build files with CMake"
  @call :log_append "%output%"
  @cd %script_folder%
  @exit /b 1
)
@cd %script_folder%
@endlocal
@goto :eof

:build - Builds the library
@setlocal EnableDelayedExpansion
@if errorlevel 1 (
  call :log 0 "Failed to enable extensions"
  exit /b 1
)
@call :log 5
@call :log 0 "Building %project% with %generator%"
@if /i [%generator%] == [MinGW] @(
  @call :find_in_path mingw32_make_executable mingw32-make.exe
  @if errorlevel 1 (
    @call :log 5
    @call :log 0 "Failed to find mingw32-make"
    @exit /b 1
  )
  @set "build_command=^"!mingw32_make_executable!^" all test"
)
@if /i [%generator%] == [MSVC] @(
  @call :msbuild msbuild_executable
  @if errorlevel 1 (
    @call :log 5
    @call :log 0 "Failed to find MSBuild"
    @exit /b 1
  )
  @set "build_command=^"!msbuild_executable!^" /m:4 /p:Configuration=%build_type% %project%.sln"
)
@if not defined build_command (
  @call :log 5
  @call :log 0 "No build command for %generator%"
  @exit /b 1
)
@cd %build_path%
@call :iso8601 iso8601
@set output=%temp%\build-%iso8601%.log
@call :log 7
@call :log 2 "Build command: %build_command:"=%"
@%build_command% > "%output%"
@if errorlevel 1 (
  @call :log_append "%output%"
  @call :log 5
  @call :log 0 "Failed to complete the build"
  @exit /b 1
)
@call :log_append "%output%"
@cd %script_folder%
@endlocal
@goto :eof

:install - Installs the built files
@setlocal
@call :log 5
@call :log 0 "Installing built files"
@call :cmake cmake_executable
@if errorlevel 1 (
  @call :log 5
  @call :log 0 "Need CMake to install the built files"
  @exit /b 1
)
@call :iso8601 iso8601
@set output=%temp%\install-%iso8601%.log
@"%cmake_executable%" ^
  "-DCMAKE_INSTALL_PREFIX=%install_path%" ^
  -P "%build_path%/cmake_install.cmake" ^
  > "%output%"
@if errorlevel 1 (
  @call :log_append "%output%"
  @call :log 5
  @call :log 0 "Failed to install the files"
  @exit /b 1
)
@call :log_append "%output%"
@endlocal
@goto :eof

:script_source - Determines if the script was ran from the cli or explorer
:: %1 - The return variable [cli|explorer]
@verify other 2>nul
@setlocal EnableDelayedExpansion
@if errorlevel 1 (
  call :log 0 "Failed to enable extensions"
  exit /b 1
)
@call :log 3 "Attempting to detect the script source"
@echo "The invocation command was: '%cmdcmdline%'" >> %log_path%
@for /f "tokens=1-3,*" %%a in ("%cmdcmdline%") do @(
  set cmd=%%~a
  set arg1=%%~b
  set arg2=%%~c
  set rest=%%~d
)
@set quote="
@if "!arg2:~0,1!" equ "!quote!" (
  if "!arg2:~-1!" neq "!quote!" (
    set "arg2=!arg2:~1!"
  )
)
@call :log 4 "cmd  = %cmd%"
@call :log 4 "arg1 = %arg1%"
@call :log 4 "arg2 = %arg2%"
@call :log 4 "rest = %rest%"
@call :log 4 "src  = %~f0"
@if /i "%arg2%" == "call" (
  set script_source=cli
) else (
  @if /i "%arg1%" == "/c" (
    set script_source=explorer
  ) else (
    set script_source=cli
  )
)
@call :log 3 "The script was invoked from %script_source%"
@endlocal & set "%~1=%script_source%"
@goto :eof

:architecture - Finds the system architecture
:: %1 - The return variable [x86|x86_64]
@setlocal
@call :log 3 "Determining the processor architecture"
@set "key=HKLM\System\CurrentControlSet\Control\Session Manager\Environment"
@set "var=PROCESSOR_ARCHITECTURE"
@for /f "skip=2 tokens=2,*" %%a in ('reg query "%key%" /v "%var%"') do @set "arch=%%b"
@if "%arch%" == "AMD64" set arch=x86_64
@call :log 4 "arch = %arch%"
@endlocal & set "%~1=%arch%"
@goto :eof

:md5 - Gets the MD5 checksum for a file
:: %1 - The hash
:: %2 - The file path
@setlocal
@set var=%~1
@set file_path=%~2
@if [%var%] == [] exit /b 1
@if "%file_path%" == "" exit /b 1
@if not exist "%file_path%" exit /b 1
@for /f "skip=3 tokens=1,*" %%a in ('powershell Get-FileHash -Algorithm MD5 "'%file_path%'"') do @set hash=%%b
@if not defined hash (
  call :log 6
  call :log 0 "Failed to get MD5 hash for %file_path%"
  exit /b 1
)
@endlocal & set "%var%=%hash: =%"
@goto :eof

:windows_version - Checks the windows version
:: %1 - The windows version
:: %2 - The major version number return variable
:: %3 - The minor version number return variable
:: %4 - The revision version number return variable
@setlocal
@call :log 3 "Retrieving the Windows version"
@for /f "tokens=2 delims=[]" %%x in ('ver') do @set win_ver=%%x
@set win_ver=%win_ver:Version =%
@set win_ver_major=%win_ver:~0,1%
@set win_ver_minor=%win_ver:~2,1%
@set win_ver_rev=%win_ver:~4%
@call :log 4 "win_ver = %win_ver%"
@endlocal & set "%~1=%win_ver%" ^
          & set "%~2=%win_ver_major%" ^
          & set "%~3=%win_ver_minor%" ^
          & set "%~4=%win_ver_rev%"
@goto :eof

:find_in_path - Finds a program of file in the PATH
@setlocal
@set var=%~1
@set file=%~2
@if [%var%] == [] exit /b 1
@if [%file%] == [] exit /b 1
@call :log 3 "Searching PATH for %file%"
@for %%x in ("%file%") do @set "file_path=%%~f$PATH:x"
@if not defined file_path exit /b 1
@endlocal & set "%var%=%file_path%"
@goto :eof

:administrator_check - Checks for administrator priviledges
@setlocal
@call :log 2 "Checking for administrator priviledges"
@set "key=HKLM\Software\VCA\Tool Chain\Admin Check"
@reg add "%key%" /v Elevated /t REG_DWORD /d 1 /f > nul 2>&1
@if errorlevel 1 exit /b 1
@reg delete "%key%" /va /f > nul 2>&1
@endlocal
@goto :eof

:log_append - Appends another file into the current logging file
:: %1 - the file_path to the file to concatenate
@setlocal
@set "file_path=%~1"
@if [%file_path%] == [] exit /b 1
@call :log 3 "Appending to log: %file_path%"
@call :iso8601 iso8601
@set "temp_log=%temp%\append-%iso8601%.log"
@call :log 4 "Using temp file %temp_log%"
@type "%log_path%" "%file_path%" > "%temp_log%" 2>nul
@move /y "%temp_log%" "%log_path%" 1>nul
@del "%file_path%" 2>nul
@del "%temp_log%" 2>nul
@endlocal
@goto :eof

:iso8601 - Returns the current time in ISO8601 format
:: %1 - the return variable
:: %2 - format [extended|basic*]
:: iso8601 - contains the resulting timestamp
@setlocal
@wmic Alias /? >NUL 2>&1 || @exit /b 1
@set "var=%~1"
@if "%var%" == "" @exit /b 1
@set "format=%~2"
@if "%format%" == "" set format=basic
@for /F "skip=1 tokens=1-6" %%g IN ('wmic Path Win32_UTCTime Get Day^,Hour^,Minute^,Month^,Second^,Year /Format:table') do @(
  @if "%%~l"=="" goto :iso8601_done
  @set "yyyy=%%l"
  @set "mm=00%%j"
  @set "dd=00%%g"
  @set "hour=00%%h"
  @set "minute=00%%i"
  @set "seconds=00%%k"
)
:iso8601_done
@set mm=%mm:~-2%
@set dd=%dd:~-2%
@set hour=%hour:~-2%
@set minute=%minute:~-2%
@set seconds=%seconds:~-2%
@if /i [%format%] == [extended] (
  set iso8601=%yyyy%-%mm%-%dd%T%hour%:%minute%:%seconds%Z
) else (
  if /i [%format%] == [basic] (
    set iso8601=%yyyy%%mm%%dd%T%hour%%minute%%seconds%Z
  ) else (
    @exit /b 1
  )
)
@set iso8601=%iso8601: =0%
@endlocal & set %var%=%iso8601%
@goto :eof

:verbosity - Processes the verbosity parameter '/v[v...]
:: %1 - verbosity given on the command line
:: logging_level - set to the number of v's
@setlocal
@set logging_level=0
@set verbosity=%~1
:verbosity_loop
@set verbosity=%verbosity:~1%
@if not [%verbosity%] == [] @(
  set /a "logging_level=logging_level+1"
  goto verbosity_loop
)
@endlocal & set logging_level=%logging_level%
@goto :eof

:log - Logs a message, depending on verbosity
:: %1 - level
::       [0-4] for CLI logging
::       [5-9] for GUI logging
:: %2 - message to print
@setlocal
@set "level=%~1"
@set "msg=%~2"
@if "%log_folder%" == "" (
  echo Logging was used to early in the script, log_folder isn't set yet
  goto :eof
)
@if "%log_path%" == "" (
  echo Logging was used to early in the script, log_path isn't set yet
  goto :eof
)
@if not exist "%log_folder%" mkdir "%log_folder%"
@if not exist "%log_path%" echo. 1>nul 2>"%log_path%"
@echo.%msg% >> "%log_path%"
@if %level% geq 5 (
  @if [%script_source%] == [explorer] (
    set /a "level=level-5"
  ) else (
    @goto :eof
  )
)
@if "%logging_level%" == "" (
  echo Logging was used to early in the script, logging_level isn't set yet
  goto :eof
)
@if %logging_level% geq %level% echo.%msg% 1>&2
@endlocal
@goto :eof


:start_browser - Opens the default browser to a URL
:: %1 - the url to open
@setlocal
@set url=%~1
@call :log 4 "Opening default browser: %url%"
@start %url%
@endlocal
@goto :eof

:find_cmake - Finds cmake on the command line or in the registry
:: %1 - the cmake file path
@setlocal
@set var=%~1
@if [%var%] == [] exit /b 1
@call :log 6
@call :log 6 "Finding CMake"
@call :log 6 "--------------"
@call :find_in_path cmake_executable cmake.exe
@if not errorlevel 1 goto found_cmake
@for /l %%i in (5,-1,0) do @(
@for /l %%j in (9,-1,0) do @(
@for /l %%k in (9,-1,0) do @(
@for %%l in (HKCU HKLM) do @(
@for %%m in (SOFTWARE SOFTWARE\Wow6432Node) do @(
  @reg query "%%l\%%m\Kitware\CMake %%i.%%j.%%k" /ve > nul 2>nul
  @if not errorlevel 1 (
    @for /f "skip=2 tokens=2,*" %%a in ('reg query "%%l\%%m\Kitware\CMake %%i.%%j.%%k" /ve') do @(
      @if exist "%%b\bin\cmake.exe" (
        @set "cmake_executable=%%b\bin\cmake.exe"
        goto found_cmake
      )
    )
  )
)))))
@call :log 5
@call :log 0 "Failed to find cmake"
@exit /b 1
:found_cmake
@endlocal & set "%var%=%cmake_executable%"
@goto :eof

:cmake - Finds cmake and installs it if necessary
:: %1 - the cmake file path
@setlocal
@set var=%~1
@if [%var%] == [] exit /b 1
@call :log 6
@call :log 6 "Checking for CMake"
@call :log 6 "------------------"
@call :find_cmake cmake_executable cmake.exe
@if not errorlevel 1 goto got_cmake
@set checksum=C00267A3D3D9619A7A2E8FA4F46D7698
@set version=3.2.2
@call :install_nsis cmake http://www.cmake.org/files/v%version:~0,3%/cmake-%version%-win32-x86.exe %checksum%
@if errorlevel 1 (
  call :log 5
  call :log 0 "Failed to install cmake"
  @exit /b 1
)
@call :find_cmake cmake_executable cmake.exe
@if not errorlevel 1 goto got_cmake
@call :log 5
@call :log 0 "Failed to check for cmake"
@exit /b 1
:got_cmake
@endlocal & set "%var%=%cmake_executable%"
@goto :eof

:mingw - Finds MinGW, installing it if needed
:: %1 - the compiler path that should be added to PATH
@setlocal EnableDelayedExpansion
@if errorlevel 1 (
  @call :log 5
  @call :log 0 "Failed to enable extensions"
  @exit /b 1
)
@set var=%~1
@if [%var%] == [] exit /b 1
@call :log 6
@call :log 6 "Checking for MinGW"
@call :log 6 "------------------"
@call :find_in_path gcc_executable gcc.exe
@if not errorlevel 1 (
  @for %%a in ("%gcc_executable%") do @set "compiler_path=%%~dpa"
  goto got_mingw
)
@call :log 7
@call :log 2 "Downloading MinGW"
@if %logging_level% leq 1 set "logging=/q"
@if %logging_level% gtr 1 set "logging=/v"
@set output_path=
@for /f %%a in ('call
    "%script_folder%\mingw.cmd"
    %logging%
    /arch "%arch%"
    "%dependency_path%"'
) do @set "compiler_path=%%a\"
@if not defined compiler_path (
  @call :log_append "%output%"
  @call :log 5
  @call :log 0 "Failed to download MinGW"
  @exit /b 1
)
:got_mingw
@call :log 5
@call :log 0 "Found MinGW: %compiler_path%gcc.exe"
@endlocal & set "%var%=%compiler_path%"
@goto :eof

:msbuild - Finds MSBuild
:: %1 - the path to MSBuild executable
@setlocal
@set var=%~1
@if [%var%] == [] exit /b 1
@call :find_in_path msbuild_executable msbuild.exe
@if not errorlevel 1 goto got_msbuild
@for /l %%i in (20,-1,4) do @(
@for /l %%j in (9,-1,0) do @(
@for %%k in (HKCU HKLM) do @(
@for %%l in (SOFTWARE SOFTWARE\Wow6432Node) do @(
  @reg query "%%k\%%l\Microsoft\MSBuild\%%i.%%j" /v MSBuildOverrideTasksPath > nul 2>nul
  @if not errorlevel 1 (
    @for /f "skip=2 tokens=2,*" %%a in ('reg query "%%k\%%l\Microsoft\MSBuild\%%i.%%j" /v MSBuildOverrideTasksPath') do @(
      @if exist "%%bmsbuild.exe" (
        @set "msbuild_executable=%%bmsbuild.exe"
        goto got_msbuild
      )
    )
  )
))))
@call :log 5
@call :log 0 "Failed to check for MSBuild"
@exit /b 1
:got_msbuild
@endlocal & set "%var%=%msbuild_executable%"
@goto :eof

:download - Downloads a file from the internet
:: %1 - the url of the file to download
:: %2 - the file to download to
:: %3 - the MD5 checksum of the file (optional)
@setlocal EnableDelayedExpansion
@if errorlevel 1 (
  call :print_usage "Failed to enable extensions"
  exit /b 1
)
@set url=%~1
@set file_path=%~2
@set checksum=%~3
@for %%a in (%file_path%) do @set dir_path=%%~dpa
@for %%a in (%file_path%) do @set file_name=%%~nxa
@if [%url%] == [] exit /b 1
@if [%file_path%] == [] exit /b 1
@if [%dir_path%] == [] exit /b 1
@if [%file_name%] == [] exit /b 1
@if not exist "%dir_path%" mkdir "%dir_path%"
@call :log 1 "Downloading %url%"
@call :iso8601 iso8601
@set temp_path=%temp%\download-%iso8601%-%file_name%
@call :log 3 "Using temp file %temp_path%"
@powershell Invoke-WebRequest "%url%" -OutFile %temp_path%
@if errorlevel 1 (
  call :log 0 "Failed to download %url%"
  exit /b 1
)
@if [%checksum%] neq [] (
  @call :log 4 "Checking %checksum% against %temp_path%"
  @call :md5 hash "%temp_path%"
  if "!hash!" neq "%checksum%" (
    call :log 0 "Failed to match checksum: %temp_path%"
    call :log 0 "Hash    : !hash!"
    call :log 0 "Checksum: %checksum%"
    exit /b 1
  ) else (
    call :log 3 "Checksum matched: %temp_path%"
    call :log 3 "Hash    : !hash!"
    call :log 3 "Checksum: %checksum%"
  )
)
@call :log 4 "Renaming %temp_path% to %file_path%"
@move /y "%temp_path%" "%file_path%" 1>nul
@endlocal
@goto :eof

:install_msi - Installs a dependency from an Microsoft Installer package (.msi)
:: %1 - [string] name of the project to install
:: %2 - The location of the .msi, a url must start with 'http://' or file_path
:: %3 - The checksum of the msi (optional)
@setlocal
@set name=%~1
@set file_path=%~2
@set checksum=%~3
@set msi=%~nx2
@set msi_path=%dependency_path%\%msi%
@if [%name%] == [] exit /b 1
@if [%file_path%] == [] exit /b 1
@if [%msi%] == [] exit /b 1
@if [%msi_path%] == [] exit /b 1
@for %%x in (msiexec.exe) do @set "msiexec_path=%%~f$PATH:x"
@if "msiexec_path" == "" (
  call :log 0 "Failed to find the Microsoft package installer (msiexec.exe)"
  call :log 6
  call :log 0 "Please install it from the Microsoft Download center"
  call :log 6
  choice /C YN /T 60 /D N /M "Would you like to go there now?"
  if !errorlevel! equ 1 call :start_browser ^
    "http://search.microsoft.com/DownloadResults.aspx?q=Windows+Installer"
  exit /b 1
)
@call :log 6
@call :log 1 "Installing the '%name%' dependency"
@call :log 6 "-------------------------------------"
@call :administrator_check
@if errorlevel 1 (
  call :log 0 "You must run %~nx0 in elevated mode to install '%name%'"
  call :log 5 "Right-Click and select 'Run as Administrator'
  call :log 0 "Install the dependency manually by running %file_path%"
  @exit /b 740
)
@if [%file_path:~0,4%] == [http] (
  if not exist "%msi_path%" (
    call :download "%file_path%" "%msi_path%" %checksum%
    if errorlevel 1 (
      call :log 0 "Failed to download the %name% dependency"
      exit /b 1
    )
  )
) else (
  call :log 2 "Copying MSI %file_path% to %msi_path%"
  call :log 7
  if not exist "%msi_path%" (
    xcopy /q /y /z "%file_path%" "%msi_path%" 1>nul
    if errorlevel 1 (
      call :log 0 "Failed to copy the Microsoft Installer"
      exit /b 1
    )
  )
)
@call :log 1 "Running the %msi%"
@call :log 6
@set msi_log=%temp%\msiexec-%timestamp%.log
@call :log 3 "Logging to: %msi_log%"
@msiexec /i "%msi_path%" /passive /log "%msi_log%" ALLUSERS=1
@set msi_errorlevel=%errorlevel%
@call :log_append "%msi_log%"
@if %msi_errorlevel% equ 0 goto install_msi_success
@if %msi_errorlevel% equ 3010 goto install_msi_success_reboot
@if %msi_errorlevel% equ 1641 goto install_msi_success_reboot
@if %msi_errorlevel% equ 3015 goto install_msi_in_progress_reboot
@if %msi_errorlevel% equ 1615 goto install_msi_in_progress_reboot
@call :log 0 "Microsoft Installer failed: %msi_errorlevel%"
@call :log 0 "Install the dependency manually by running %msi_path%"
@exit /b 1
:install_msi_in_progress_reboot
@call :log 0 "The installation requires a reboot to continue"
@call :log 5
@call :reboot
@exit /b 1
:install_msi_success_reboot
@call :log 3 "The installation requires a reboot to be fully functional"
@set reboot_required=1
:install_msi_success
@call :log 2 "Successfully installed %name%"
@call :log 7
@endlocal & set reboot_required=%reboot_required%
@goto :eof

:install_nsis - Installs a dependency from an Nullsoft Installer package (.exe)
:: %1 - [string] name of the project to install
:: %2 - The location of the .exe, a url must start with 'http://' or file_path
:: %3 - The checksum of the exe (optional)
@setlocal
@set name=%~1
@set file_path=%~2
@set checksum=%~3
@set exe=%~nx2
@set exe_path=%dependency_path%\%exe%
@if [%name%] == [] exit /b 1
@if [%file_path%] == [] exit /b 1
@if [%exe%] == [] exit /b 1
@if [%exe_path%] == [] exit /b 1
@call :log 6
@call :log 1 "Installing the '%name%' dependency"
@call :log 6 "-------------------------------------"
@call :administrator_check
@if errorlevel 1 (
  call :log 0 "You must run %~nx0 in elevated mode to install '%name%'"
  call :log 5 "Right-Click and select 'Run as Administrator'
  call :log 0 "Install the dependency manually by running %file_path%"
  @exit /b 740
)
@if [%file_path:~0,4%] == [http] (
  if not exist "%exe_path%" (
    call :download "%file_path%" "%exe_path%" %checksum%
    if errorlevel 1 (
      call :log 0 "Failed to download the %name% dependency"
      exit /b 1
    )
  )
) else (
  call :log 2 "Copying installer %file_path% to %exe_path%"
  call :log 7
  if not exist "%exe_path%" (
    xcopy /q /y /z "%file_path%" "%exe_path%" 1>nul
    if errorlevel 1 (
      call :log 0 "Failed to copy the Nullsoft Installer"
      exit /b 1
    )
  )
)
@call :log 1 "Running the %exe%"
@call :log 6
@"%exe_path%" /S
@set nsis_errorlevel=%errorlevel%
@if %nsis_errorlevel% equ 0 goto install_nsis_success
@if %nsis_errorlevel% equ 3010 goto install_nsis_success_reboot
@if %nsis_errorlevel% equ 1641 goto install_nsis_success_reboot
@if %nsis_errorlevel% equ 3015 goto install_nsis_in_progress_reboot
@if %nsis_errorlevel% equ 1615 goto install_nsis_in_progress_reboot
@call :log 0 "Nullsoft Installer failed: %nsis_errorlevel%"
@call :log 0 "Install the dependency manually by running %exe_path%"
@exit /b 1
:install_nsis_in_progress_reboot
@call :log 0 "The installation requires a reboot to continue"
@call :log 5
@call :reboot
@exit /b 1
:install_nsis_success_reboot
@call :log 3 "The installation requires a reboot to be fully functional"
@set reboot_required=1
:install_nsis_success
@call :log 2 "Successfully installed %name%"
@call :log 7
@endlocal & set reboot_required=%reboot_required%
@goto :eof

:reboot - Asks the user if they would like to reboot then stops the script
@setlocal
@call :log 6 "-------------------------------------------"
@choice /C YN /T 60 /D N /M "The %method% requires a reboot, reboot now?"
@set ret=%errorlevel%
@call :log 6
@if %ret% equ 1 (
  @shutdown /r
) else (
  @call :log 0 "You will need to reboot to complete the %method%"
  @call :log 5
)
@endlocal
@goto :eof
