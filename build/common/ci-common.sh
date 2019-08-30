#!/bin/bash

if [[ "$KOKORO_BUILD_ID" ]]; then
    echo "Running job $KOKORO_JOB_NAME"
    TARGET=`echo "$KOKORO_JOB_NAME" | awk -F "/" '{print $NF}'`
fi

if [[ "GITHUB_WORKFLOW" ]]; then
    echo "Running job $WORKFLOW_OS in workflow $GITHUB_WORKFLOW"
    TARGET=$WORKFLOW_OS
fi
