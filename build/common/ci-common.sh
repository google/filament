#!/bin/bash

if [[ "$KOKORO_BUILD_ID" ]]; then
    echo "Running job $KOKORO_JOB_NAME"
    TARGET=`echo "$KOKORO_JOB_NAME" | awk -F "/" '{print $NF}'`
    CONTINUOUS_INTEGRATION=true
fi

if [[ "$GITHUB_WORKFLOW" ]]; then
    echo "Running job $TARGET in workflow $GITHUB_WORKFLOW"
    # $TARGET is set by the GitHub workflow
    CONTINUOUS_INTEGRATION=true
fi
