#!/bin/bash
./testharness.exe --config testsprite2_crashtest.config > testrun.log 2>&1
if [ "$?" != "0" ]; then
  echo TEST RUN FAILED (see testrun.log)
  # report error code to CI
fi