cd ..\..

:: This loops through test_list.txt splitting each line into %%A and %%B, where
:: %%A is the path to the test executable, and %%B are any additional
:: arguments.
for /F "tokens=1,*" %%A in (build\common\test_list.txt) do "out\cmake-release\%%A" %%B || exit /b 1

cd out\cmake-release

exit /b 0
