@echo off
REM We need admin rights, otherwise the random state cannot be written
REM Thanks to http://stackoverflow.com/a/10052222/1531708

:: BatchGotAdmin
:-------------------------------------
REM  --> Check for permissions
    IF "%PROCESSOR_ARCHITECTURE%" EQU "amd64" (
>nul 2>&1 "%SYSTEMROOT%\SysWOW64\cacls.exe" "%SYSTEMROOT%\SysWOW64\config\system"
) ELSE (
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
)

REM --> If error flag set, we do not have admin.
if '%errorlevel%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    set params = %*:"=""
    echo UAC.ShellExecute "cmd.exe", "/c ""%~s0"" %params%", "", "runas", 1 >> "%temp%\getadmin.vbs"

    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit /B

:gotAdmin
    pushd "%CD%"
    CD /D "%~dp0"
:-------------------------------------- 

del server.*

c:\OpenSSL-Win32\bin\openssl.exe genrsa -des3 -out server.key 4096

c:\OpenSSL-Win32\bin\openssl.exe req -sha256 -new -key server.key -out server.csr -utf8

copy server.key server.key.orig

c:\OpenSSL-Win32\bin\openssl.exe rsa -in server.key.orig -out server.key

echo [ v3_ca ] > server.ext.txt
echo [ req ] >> server.ext.txt
echo req_extensions = my_extensions >> server.ext.txt
echo [ my_extensions ] >> server.ext.txt
echo extendedKeyUsage=serverAuth >> server.ext.txt
echo crlDistributionPoints=URI:http://localhost/crl.pem >> server.ext.txt

c:\OpenSSL-Win32\bin\openssl.exe x509 -req -days 365 -extensions v3_ca -extfile server.ext.txt -in server.csr -signkey server.key -out server.crt

copy server.crt server.pem

type server.key >> server.pem
