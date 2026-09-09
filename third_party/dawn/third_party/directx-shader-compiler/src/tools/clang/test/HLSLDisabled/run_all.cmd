@echo off
rem We can't run the whole directory at once because crashes stop execution.
for /f "usebackq delims=|" %%f in (`dir /b "%~dp0\*.hlsl"`) do (
    %HLSL_SRC_DIR%\utils\hct\hcttest.cmd file-check "%~dp0\%%f"
)