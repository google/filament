start /wait testharness.exe --config testsprite2_crashtest.config > testrun.log 2>&1
if %ERRORLEVEL% NEQ 0 echo TEST RUN FAILED (see testrun.log)