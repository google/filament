#!/bin/bash
#
# Copyright 2021 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

sleep_duration=$1
if [ -z "${sleep_duration}" ]; then
    echo "No sleep_duration provided"
    exit 1
fi

storage_dir=$2
if [ -z "${storage_dir}" ]; then
    echo "No storage_dir provided"
    exit 1
fi

start_time=$SECONDS
while true; do
    pid=$(pidof com.android.angle.test:test_process)
    case $pid in
        ''|*[!0-9]*) echo pid is not a number ;;
        *) echo com.android.angle.test:test_process $pid >> ${storage_dir}/gpumem.txt ;;
    esac
    dumpsys gpu --gpumem >> ${storage_dir}/gpumem.txt
    time_elapsed=$(( SECONDS - start_time ))
    echo "time_elapsed: $time_elapsed" >> ${storage_dir}/gpumem.txt
    sleep ${sleep_duration};
done
