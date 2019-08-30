#!/bin/bash

if [[ "$KOKORO_BUILD_ID" ]]; then
    echo "Running job $KOKORO_JOB_NAME"
    TARGET=`echo "$KOKORO_JOB_NAME" | awk -F "/" '{print $NF}'`
fi

if [[ "GITHUB_WORKFLOW" ]]; then
    echo "Running job GITHUB_ACTION in workflow $GITHUB_WORKFLOW"
    TARGET=`echo "$GITHUB_ACTION" | awk -F "-" '{print $NF}'`
fi
