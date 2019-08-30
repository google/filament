#!/bin/bash

if [[ "$KOKORO_BUILD_ID" ]]; then
    echo "Running job $KOKORO_JOB_NAME"
    TARGET=`echo "$KOKORO_JOB_NAME" | awk -F "/" '{print $NF}'`
    CONTINUOUS_INTEGRATION=true
fi

if [[ "$GITHUB_WORKFLOW" ]]; then
    echo "Running workflow $GITHUB_WORKFLOW (event: $GITHUB_EVENT_NAME, action: $GITHUB_ACTION)"
    CONTINUOUS_INTEGRATION=true
fi
