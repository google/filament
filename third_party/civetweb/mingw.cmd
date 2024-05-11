:: Make sure the extensions are enabled
@verify other 2>nul
@setlocal EnableExtensions EnableDelayedExpansion
@if errorlevel 1 (
  @call :print_usage "Failed to enable extensions"
  @exit /b 1
)

::Change the code page to unicode
@chcp 65001 1>nul 2>nul
@if errorlevel 1 (
  @call :print_usage "Failed to change the code page to unicode"
  @exit /b 1
)

:: Set up some global variables
@set "script_name=%~nx0"
@set "script_folder=%~dp0"
@set "script_folder=%script_folder:~0,-1%"
@set "dependency_path=%TEMP%\mingw-build-dependencies"

:: Check the command line parameters
@set logging_level=1
:options_loop
@if [%1] == [] goto :options_parsed
@set "arg=%~1"
@set one=%arg:~0,1%
@set two=%arg:~0,2%
@set three=%arg:~0,3%
@if /i [%arg%] == [/?] (
  @call :print_usage "Downloads a specific version of MinGW"
  @exit /b 0
)
@if /i [%arg%] == [/q] set quiet=true
@if /i [%two%] == [/v] @if /i not [%three%] == [/ve] @call :verbosity "!arg!"
@if /i [%arg%] == [/version] set "version=%~2" & shift
@if /i [%arg%] == [/arch] set "arch=%~2" & shift
@if /i [%arg%] == [/exceptions] set "exceptions=%~2" & shift
@if /i [%arg%] == [/threading] set "threading=%~2" & shift
@if /i [%arg%] == [/revision] set "revision=%~2" & shift
@if /i not [!one!] == [/] (
  if not defined output_path (
    set output_path=!arg!
  ) else (
    @call :print_usage "Too many output locations: !output_path! !arg!" ^
                       "There should only be one output location"
    @exit /b 1
  )
)
@shift
@goto :options_loop
:options_parsed
@if defined quiet set logging_level=0
@if not defined output_path set "output_path=%script_folder%\mingw-builds"
@set "output_path=%output_path:/=\%"

:: Set up the logging
@set "log_folder=%output_path%\logs"
@call :iso8601 timestamp
@set "log_path=%log_folder%\%timestamp%.log"
@set log_keep=10

:: Get default architecture
@if not defined arch @call :architecture arch

:: Only keep a certain amount of logs
@set /a "log_keep=log_keep-1"
@if not exist %log_folder% @mkdir %log_folder%
@for /f "skip=%log_keep%" %%f in ('dir /b /o-D /tc %log_folder%') do @(
  @call :log 4 "Removing old log file %log_folder%\%%f"
  del %log_folder%\%%f
)

:: Set up some more global variables
@call :windows_version win_ver win_ver_major win_ver_minor win_ver_rev
@call :script_source script_source
@if [%script_source%] == [explorer] (
  set /a "logging_level=logging_level+1"
)

:: Execute the main function
@call :main "%arch%" "%version%" "%threading%" "%exceptions%" "%revision%"
@if errorlevel 1 (
  @call :log 0 "Failed to download MinGW"
  @call :log 0 "View the log at %log_path%"
  @exit /b 1
)

:: Stop the script if the user double clicked
@if [%script_source%] == [explorer] (
  pause
)

@endlocal
@goto :eof

:: -------------------------- Functions start here ----------------------------

:main - Main function that performs the download
:: %1 - Target architecture
:: %2 - Version of MinGW to get [optional]
:: %3 - Threading model [optional]
:: %4 - Exception model [optional]
:: %5 - Package revision [optional]
@setlocal
@call :log 6
@call :log 2 "Welcome to the MinGW download script"
@call :log 6 "------------------------------------"
@call :log 6
@call :log 2 "This script downloads a specific version of MinGW"
@set "arch=%~1"
@if "%arch%" == "" @exit /b 1
@set "version=%~2"
@set "threading=%~3"
@set "exceptions=%~4"
@set "revision=%~5"
@call :log 3 "arch       = %arch%"
@call :log 3 "version    = %version%"
@call :log 3 "exceptions = %exceptions%"
@call :log 3 "threading  = %threading%"
@call :log 3 "revision   = %revision%"
@call :repository repo
@if errorlevel 1 (
  @call :log 0 "Failed to get the MinGW-builds repository information"
  @exit /b 1
)
@call :resolve slug url "%repo%" "%arch%" "%version%" "%threading%" "%exceptions%" "%revision%"
@if errorlevel 1 (
  @call :log 0 "Failed to resolve the correct URL of MinGW"
  @exit /b 1
)
@call :unpack compiler_path "%url%" "%output_path%\mingw\%slug%"
@if errorlevel 1 (
  @call :log 0 "Failed to unpack the MinGW archive"
  @exit /b 1
)
@rmdir /s /q "%dependency_path%"
@echo.%compiler_path%
@endlocal
@goto :eof

:repository - Gets the MinGW-builds repository
:: %1 - The return variable for the repository file path
@verify other 2>nul
@setlocal EnableDelayedExpansion
@if errorlevel 1 (
  @call :log 0 "Failed to enable extensions"
  @exit /b 1
)
@set "var=%~1"
@if "%var%" == "" @exit /b 1
@call :log 7
@call :log 2 "Getting MinGW repository information"
@set "url=http://downloads.sourceforge.net/project/mingw-w64/Toolchains targetting Win32/Personal Builds/mingw-builds/installer/repository.txt"
@call :log 6
@call :log 1 "Downloading MinGW repository"
@set "file_path=%dependency_path%\mingw-repository.txt"
@call :download "%url%" "%file_path%"
@if errorlevel 1 (
  @call :log 0 "Failed to download the MinGW repository information"
  @exit /b 1
)
@set "repository_path=%dependency_path%\repository.txt"
@del "%repository_path%" 2>nul
@for /f "delims=| tokens=1-6,*" %%a in (%file_path%) do @(
  @set "version=%%~a"
  @set "version=!version: =!"
  @set "arch=%%~b"
  @set "arch=!arch: =!"
  @set "threading=%%~c"
  @set "threading=!threading: =!"
  @set "exceptions=%%~d"
  @set "exceptions=!exceptions: =!"
  @set "revision=%%~e"
  @set "revision=!revision: =!"
  @set "revision=!revision:rev=!"
  @set "url=%%~f"
  @set "url=!url:%%20= !"
  @for /l %%a in (1,1,32) do @if "!url:~-1!" == " " set url=!url:~0,-1!
  @echo !arch!^|!version!^|!threading!^|!exceptions!^|!revision!^|!url!>> "%repository_path%"
)
@del "%file_path%" 2>nul
@endlocal & set "%var%=%repository_path%"
@goto :eof

:resolve - Gets the MinGW-builds repository
:: %1 - The return variable for the MinGW slug
:: %2 - The return variable for the MinGW URL
:: %3 - The repository information to use
:: %4 - Target architecture
:: %5 - Version of MinGW to get [optional]
:: %6 - Threading model [optional]
:: %7 - Exception model [optional]
:: %8 - Package revision [optional]
@setlocal
@set "slug_var=%~1"
@if "%slug_var%" == "" @exit /b 1
@set "url_var=%~2"
@if "%url_var%" == "" @exit /b 1
@set "repository=%~3"
@if "%repository%" == "" @exit /b 1
@set "arch=%~4"
@if "%arch%" == "" @exit /b 1
@call :resolve_version version "%repository%" "%arch%" "%~5"
@if errorlevel 1 @exit /b 1
@call :resolve_threading threading "%repository%" "%arch%" "%version%" "%~6"
@if errorlevel 1 @exit /b 1
@call :resolve_exceptions exceptions "%repository%" "%arch%" "%version%" "%threading%" "%~7"
@if errorlevel 1 @exit /b 1
@call :resolve_revision revision "%repository%" "%arch%" "%version%" "%threading%" "%exceptions%" "%~8"
@if errorlevel 1 @exit /b 1
@call :log 3 "Finding URL"
@for /f "delims=| tokens=1-6" %%a in (%repository%) do @(
  @if "%arch%" == "%%a" (
    @if "%version%" == "%%b" (
      @if "%threading%" == "%%c" (
        @if "%exceptions%" == "%%d" (
          @if "%revision%" == "%%e" (
            @set "url=%%f"
) ) ) ) ) )
@if "%url%" == "" (
  @call :log 0 "Failed to resolve URL"
  @exit /b 1
)
@set slug=gcc-%version%-%arch%-%threading%-%exceptions%-rev%revision%
@call :log 2 "Resolved slug: %slug%"
@call :log 2 "Resolved url: %url%"
@endlocal & set "%slug_var%=%slug%" & set "%url_var%=%url%"
@goto :eof

:unpack - Unpacks the MinGW archive
:: %1 - The return variable name for the compiler path
:: %2 - The filepath or URL of the archive
:: %3 - The folder to unpack to
@verify other 2>nul
@setlocal EnableDelayedExpansion
@if errorlevel 1 (
  @call :log 0 "Failed to enable extensions"
  @exit /b 1
)
@set "var=%~1"
@if "%var%" == "" @exit /b 1
@set "archive_path=%~2"
@if "%archive_path%" == "" @exit /b 1
@set "folder_path=%~3"
@if "%folder_path%" == "" @exit /b 1
@set "compiler_path=%folder_path%\bin"
@if exist "%compiler_path%" goto :unpack_done
@call :log 7
@call :log 2 "Unpacking MinGW archive"
@set "http=%archive_path:~0,4%"
@if "%http%" == "http" (
  @set "url=%archive_path%"
  @for /f %%a in ("!url: =-!") do @set "file_name=%%~na"
  @for /f %%a in ("!url: =-!") do @set "file_ext=%%~xa"
  @set "archive_path=%dependency_path%\!file_name!!file_ext!"
  @if not exist "!archive_path!" (
    @call :log 6
    @call :log 1 "Downloading MinGW archive"
    @call :download "!url!" "!archive_path!"
    @if errorlevel 1 (
      @del "!archive_path!" 2>nul
      @call :log 0 "Failed to download: !file_name!!file_ext!"
      @exit /b 1
    )
  )
)
@if not exist "%archive_path%" (
  @call :log 0 "The archive did not exist to unpack: %archive_path%"
  @exit /b 1
)
@for /f %%a in ("%archive_path: =-%") do @set "file_name=%%~na"
@for /f %%a in ("%archive_path: =-%") do @set "file_ext=%%~xa"
@call :log 6
@call :log 1 "Unpacking MinGW %file_name%%file_ext%"
@call :find_sevenzip sevenzip_executable
@if errorlevel 1 (
  @call :log 0 "Need 7zip to unpack the MinGW archive"
  @exit /b 1
)
@call :iso8601 iso8601
@for /f %%a in ("%folder_path%") do @set "tmp_path=%%~dpatmp-%iso8601%"
@"%sevenzip_executable%" x -y "-o%tmp_path%" "%archive_path%" > nul
@if errorlevel 1 (
  @rmdir /s /q "%folder_path%"
  @call :log 0 "Failed to unpack the MinGW archive"
  @exit /b 1
)
@set "expected_path=%tmp_path%\mingw64"
@if not exist "%expected_path%" (
  @set "expected_path=%tmp_path%\mingw32"
)
@move /y "%expected_path%" "%folder_path%" > nul
@if errorlevel 1 (
  @rmdir /s /q "%tmp_path%" 2>nul
  @call :log 0 "Failed to move MinGW folder"
  @call :log 0 "%expected_path%"
  @call :log 0 "%folder_path%"
  @exit /b 1
)
@rmdir /s /q %tmp_path%
@set "compiler_path=%folder_path%\bin"
:unpack_done
@if not exist "%compiler_path%\gcc.exe" (
  @call :log 0 "Failed to find gcc: %compiler_path%"
  @exit /b 1
)
@endlocal & set "%var%=%compiler_path%"
@goto :eof

:find_sevenzip - Finds (or downloads) the 7zip executable
:: %1 - The return variable for the 7zip executable path
@setlocal
@set "var=%~1"
@if "%var%" == "" @exit /b 1
@call :log 2 "Finding 7zip"
@call :find_in_path sevenzip_executable 7z.exe
@if not errorlevel 1 goto :find_sevenzip_done
@call :find_in_path sevenzip_executable 7za.exe
@if not errorlevel 1 goto :find_sevenzip_done
@set checksum=2FAC454A90AE96021F4FFC607D4C00F8
@set "url=http://7-zip.org/a/7za920.zip"
@for /f %%a in ("%url: =-%") do @set "file_name=%%~na"
@for /f %%a in ("%url: =-%") do @set "file_ext=%%~xa"
@set "archive_path=%dependency_path%\%file_name%%file_ext%"
@if not exist "%archive_path%" (
  @call :log 6
  @call :log 1 "Downloading 7zip archive"
  @call :download "%url%" "%archive_path%" %checksum%
  @if errorlevel 1 (
    @del "%archive_path%" 2>nul
    @call :log 0 "Failed to download: %file_name%%file_ext%"
    @exit /b 1
  )
)
@set "sevenzip_path=%dependency_path%\sevenzip"
@if not exist "%sevenzip_path%" (
  @call :unzip "%archive_path%" "%sevenzip_path%"
  @if errorlevel 1 (
    @call :log 0 "Failed to unzip the7zip archive"
    @exit /b 1
  )
)
@set "sevenzip_executable=%sevenzip_path%\7za.exe"
@if not exist "%sevenzip_executable%" (
  @call :log 0 "Failed to find unpacked 7zip: %sevenzip_executable%"
  @exit /b 1
)
:find_sevenzip_done
@call :log 2 "Found 7zip: %sevenzip_executable%"
@endlocal & set "%var%=%sevenzip_executable%"
@goto :eof

:unzip - Unzips a .zip archive
:: %1 - The archive to unzip
:: %2 - The location to unzip to
@setlocal
@set "archive_path=%~1"
@if "%archive_path%" == "" @exit /b 1
@set "folder_path=%~2"
@if "%folder_path%" == "" @exit /b 1
@for /f %%a in ("%archive_path: =-%") do @set "file_name=%%~na"
@for /f %%a in ("%archive_path: =-%") do @set "file_ext=%%~xa"
@call :log 2 "Unzipping: %file_name%%file_ext%"
@call :iso8601 iso8601
@set "log_path=%temp%\unzip-%iso8601%-%file_name%.log"
@powershell ^
  Add-Type -assembly "system.io.compression.filesystem"; ^
  [io.compression.zipfile]::ExtractToDirectory(^
    '%archive_path%', '%folder_path%') 2>"%log_path%"
@if errorlevel 1 (
  @call :log 0 "Failed to unzip: %file_name%%file_ext%"
  @call :log_append "%log_path%"
  @exit /b 1
)
@endlocal
@goto :eof

:resolve_version - Gets the version of the MinGW compiler
:: %1 - The return variable for the version
:: %2 - The repository information to use
:: %3 - The architecture of the compiler
:: %4 - Version of MinGW to get [optional]
@verify other 2>nul
@setlocal EnableDelayedExpansion
@if errorlevel 1 (
  @call :log 0 "Failed to enable extensions"
  @exit /b 1
)
@set "var=%~1"
@if "%var%" == "" @exit /b 1
@set "repository=%~2"
@if "%repository%" == "" @exit /b 1
@set "arch=%~3"
@if "%arch%" == "" @exit /b 1
@set "version=%~4"
@if not "%version%" == "" goto :resolve_version_done
:: Find the latest version
@call :log 3 "Finding latest version"
@set version=0.0.0
@for /f "delims=| tokens=1-6" %%a in (%repository%) do @(
  @if "%arch%" == "%%a" (
    @call :version_compare result "%version%" "%%b"
    @if errorlevel 1 (
      @call :log 0 "Failed to compare versions: %version% %%a"
      @exit /b 1
    )
    @if !result! lss 0 set version=%%b
  )
)
:resolve_version_done
@if "%version%" == "" (
  @call :log 0 "Failed to resolve latest version number"
  @exit /b 1
)
@call :log 2 "Resolved version: %version%"
@endlocal & set "%var%=%version%"
@goto :eof

:resolve_threading - Gets the threading model of the MinGW compiler
:: %1 - The return variable for the threading model
:: %2 - The repository information to use
:: %3 - The architecture of the compiler
:: %4 - The version of the compiler
:: %5 - threading model of MinGW to use [optional]
@verify other 2>nul
@setlocal EnableDelayedExpansion
@if errorlevel 1 (
  @call :log 0 "Failed to enable extensions"
  @exit /b 1
)
@set "var=%~1"
@if "%var%" == "" @exit /b 1
@set "repository=%~2"
@if "%repository%" == "" @exit /b 1
@set "arch=%~3"
@if "%arch%" == "" @exit /b 1
@set "version=%~4"
@if "%version%" == "" @exit /b 1
@set "threading=%~5"
@if not "%threading%" == "" goto :resolve_threading_done
@call :log 3 "Finding best threading model"
@for /f "delims=| tokens=1-6" %%a in (%repository%) do @(
  @if "%arch%" == "%%a" (
    @if "%version%" == "%%b" (
      @if not defined threading (
        @set "threading=%%c"
      )
      @if "%%c" == "posix" (
        @set "threading=%%c"
) ) ) )
:resolve_threading_done
@if "%threading%" == "" (
  @call :log 0 "Failed to resolve the best threading model"
  @exit /b 1
)
@call :log 2 "Resolved threading model: %threading%"
@endlocal & set "%var%=%threading%"
@goto :eof

:resolve_exceptions - Gets the exception model of the MinGW compiler
:: %1 - The return variable for the exception model
:: %2 - The repository information to use
:: %3 - The architecture of the compiler
:: %4 - The version of the compiler
:: %4 - The threading model of the compiler
:: %5 - exception model of MinGW to use [optional]
@verify other 2>nul
@setlocal EnableDelayedExpansion
@if errorlevel 1 (
  @call :log 0 "Failed to enable extensions"
  @exit /b 1
)
@set "var=%~1"
@if "%var%" == "" @exit /b 1
@set "repository=%~2"
@if "%repository%" == "" @exit /b 1
@set "arch=%~3"
@if "%arch%" == "" @exit /b 1
@set "version=%~4"
@if "%version%" == "" @exit /b 1
@set "threading=%~5"
@if "%threading%" == "" @exit /b 1
@set "exceptions=%~6"
@if not "%exceptions%" == "" goto :resolve_exceptions_done
@call :log 3 "Finding best exception model"
@for /f "delims=| tokens=1-6" %%a in (%repository%) do @(
  @if "%arch%" == "%%a" (
    @if "%version%" == "%%b" (
      @if "%threading%" == "%%c" (
        @if not defined exceptions (
          @set "exceptions=%%d"
        )
        @if "%%d" == "dwarf" (
          @set "exceptions=%%d"
        )
        @if "%%d" == "seh" (
          @set "exceptions=%%d"
) ) ) ) )
:resolve_exceptions_done
@if "%exceptions%" == "" (
  @call :log 0 "Failed to resolve the best exception model"
  @exit /b 1
)
@call :log 2 "Resolved exception model: %exceptions%"
@endlocal & set "%var%=%exceptions%"
@goto :eof

:resolve_revision - Gets the revision of the MinGW compiler
:: %1 - The return variable for the revision
:: %2 - The repository information to use
:: %3 - The architecture of the compiler
:: %4 - The version of the compiler
:: %4 - The threading model of the compiler
:: %4 - The exception model of the compiler
:: %5 - revision of the MinGW package to use [optional]
@verify other 2>nul
@setlocal EnableDelayedExpansion
@if errorlevel 1 (
  @call :log 0 "Failed to enable extensions"
  @exit /b 1
)
@set "var=%~1"
@if "%var%" == "" @exit /b 1
@set "repository=%~2"
@if "%repository%" == "" @exit /b 1
@set "arch=%~3"
@if "%arch%" == "" @exit /b 1
@set "version=%~4"
@if "%version%" == "" @exit /b 1
@set "threading=%~5"
@if "%threading%" == "" @exit /b 1
@set "exceptions=%~6"
@if "%exceptions%" == "" @exit /b 1
@set "revision=%~7"
@if not "%revision%" == "" goto :resolve_revision_done
@call :log 3 "Finding latest revision"
@for /f "delims=| tokens=1-6" %%a in (%repository%) do @(
  @if "%arch%" == "%%a" (
    @if "%version%" == "%%b" (
      @if "%threading%" == "%%c" (
        @if "%exceptions%" == "%%d" (
          @if "%%e" gtr "%revision%" (
            @set "revision=%%e"
) ) ) ) ) )
:resolve_revision_done
@if "%revision%" == "" (
  @call :log 0 "Failed to resolve latest revision"
  @exit /b 1
)
@call :log 2 "Resolved revision: %revision%"
@endlocal & set "%var%=%revision%"
@goto :eof

:version_compare - Compares two semantic version numbers
:: %1 - The return variable:
::        - < 0 : if %2 < %3
::        -   0 : if %2 == %3
::        - > 0 : if %2 > %3
:: %2 - The first version to compare
:: %3 - The second version to compare
@setlocal
@set "var=%~1"
@if "%var%" == "" @exit /b 1
@set "lhs=%~2"
@if "%lhs%" == "" @exit /b 1
@set "rhs=%~3"
@if "%lhs%" == "" @exit /b 1
@set result=0
@for /f "delims=. tokens=1-6" %%a in ("%lhs%.%rhs%") do @(
  @if %%a lss %%d (
    set result=-1
    goto :version_compare_done
  ) else (
    @if %%a gtr %%d (
      set result=1
      goto :version_compare_done
    ) else (
      @if %%b lss %%e (
        set result=-1
        goto :version_compare_done
      ) else (
        @if %%b gtr %%e (
          set result=1
          goto :version_compare_done
        ) else (
          @if %%c lss %%f (
            set result=-1
            goto :version_compare_done
          ) else (
            @if %%c gtr %%f (
              set result=1
              goto :version_compare_done
            )
          )
        )
      )
    )
  )
)
:version_compare_done
@endlocal & set "%var%=%result%"
@goto :eof

:print_usage - Prints the usage of the script
:: %* - message to print, each argument on it's own line
@setlocal
@for %%a in (%*) do @echo.%%~a
@echo.
@echo.build [/?][/v[v...]^|/q][/version][/arch a][/threading t]
@echo.      [/exceptions e][/revision r] location
@echo.
@echo.  /version v  The version of MinGW to download
@echo.  /arch a     The target architecture [i686^|x86_64]
@echo.  /threading t
@echo.              Threading model to use [posix^|win32]
@echo.  /exceptions e
@echo.              Exception model to use [sjlj^|seh^|dwarf]
@echo.  /revision e Revision of the release to use
@echo.  /v          Sets the output to be more verbose
@echo.  /v[v...]    Extra verbosity, /vv, /vvv, etc
@echo.  /q          Quiets the output
@echo.  /?          Shows this usage message
@echo.
@endlocal
@goto :eof

:script_source - Determines if the script was ran from the cli or explorer
:: %1 - The return variable [cli|explorer]
@verify other 2>nul
@setlocal EnableDelayedExpansion
@if errorlevel 1 (
  @call :log 0 "Failed to enable extensions"
  @exit /b 1
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
:: %1 - The return variable [i686|x86_64]
@setlocal
@call :log 3 "Determining the processor architecture"
@set "key=HKLM\System\CurrentControlSet\Control\Session Manager\Environment"
@set "var=PROCESSOR_ARCHITECTURE"
@for /f "skip=2 tokens=2,*" %%a in ('reg query "%key%" /v "%var%"') do @set "arch=%%b"
@if "%arch%" == "AMD64" set arch=x86_64
@if "%arch%" == "x64" set arch=i686
@call :log 4 "arch = %arch%"
@endlocal & set "%~1=%arch%"
@goto :eof

:md5 - Gets the MD5 checksum for a file
:: %1 - The hash
:: %2 - The file path
@setlocal
@set "var=%~1"
@set "file_path=%~2"
@if "%var%" == "" @exit /b 1
@if "%file_path%" == "" @exit /b 1
@if not exist "%file_path%" @exit /b 1
@for /f "skip=3 tokens=1,*" %%a in ('powershell Get-FileHash -Algorithm MD5 "'%file_path%'"') do @set hash=%%b
@if not defined hash (
  @call :log 6
  @call :log 0 "Failed to get MD5 hash for %file_path%"
  @exit /b 1
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
:: %1 - return variable of the file path
@setlocal
@set "var=%~1"
@if "%var%" == "" @exit /b 1
@set "file=%~2"
@if "%file%" == "" @exit /b 1
@call :log 3 "Searching PATH for %file%"
@for %%x in ("%file%") do @set "file_path=%%~f$PATH:x"
@if not defined file_path @exit /b 1
@endlocal & set "%var%=%file_path%"
@goto :eof

:log_append - Appends another file into the current logging file
:: %1 - the file_path to the file to concatenate
@setlocal
@set "file_path=%~1"
@if "%file_path%" == "" @exit /b 1
@call :log 3 "Appending to log: %file_path%"
@call :iso8601 iso8601
@set temp_log=%temp%\append-%iso8601%.log
@call :log 4 "Using temp file %temp_log%"
@type "%log_path%" "%file_path%" > "%temp_log%" 2>nul
@move /y "%temp_log%" "%log_path%" 1>nul
@del "%file_path% 2>nul
@del "%temp_log% 2>nul
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

:download - Downloads a file from the internet
:: %1 - the url of the file to download
:: %2 - the file to download to
:: %3 - the MD5 checksum of the file (optional)
@setlocal EnableDelayedExpansion
@if errorlevel 1 (
  @call :print_usage "Failed to enable extensions"
  @exit /b 1
)
@set "url=%~1"
@set "file_path=%~2"
@set "checksum=%~3"
@for %%a in (%file_path%) do @set dir_path=%%~dpa
@for %%a in (%file_path%) do @set file_name=%%~nxa
@if "%url%" == "" @exit /b 1
@if "%file_path%" == "" @exit /b 1
@if "%dir_path%" == "" @exit /b 1
@if "%file_name%" == "" @exit /b 1
@if not exist "%dir_path%" mkdir "%dir_path%"
@call :log 2 "Downloading %url%"
@call :iso8601 iso8601
@set "temp_path=%temp%\download-%iso8601%-%file_name%"
@set "log_path=%temp%\download-%iso8601%-log-%file_name%"
@call :log 4 "Using temp file %temp_path%"
@powershell Invoke-WebRequest "'%url%'" ^
  -OutFile "'%temp_path%'" ^
  -UserAgent [Microsoft.PowerShell.Commands.PSUserAgent]::IE ^
  1>nul 2>"%log_path%"
@if errorlevel 1 (
  @call :log 0 "Failed to download %url%"
  @call :log_append "%log_path%"
  @exit /b 1
)
@if [%checksum%] neq [] (
  @call :log 4 "Checking %checksum% against %temp_path%"
  @call :md5 hash "%temp_path%"
  if "!hash!" neq "%checksum%" (
    @call :log 0 "Failed to match checksum: %temp_path%"
    @call :log 0 "Hash    : !hash!"
    @call :log 0 "Checksum: %checksum%"
    @exit /b 1
  ) else (
    @call :log 3 "Checksum matched: %temp_path%"
    @call :log 3 "Hash    : !hash!"
    @call :log 3 "Checksum: %checksum%"
  )
)
@call :log 4 "Renaming %temp_path% to %file_path%"
@move /y "%temp_path%" "%file_path%" 1>nul
@endlocal
@goto :eof
