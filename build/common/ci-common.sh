#!/bin/bash

if [[ "$GITHUB_WORKFLOW" ]]; then
    echo "Running workflow $GITHUB_WORKFLOW (event: $GITHUB_EVENT_NAME, action: $GITHUB_ACTION)"
    CONTINUOUS_INTEGRATION=true

    # Force Java to be Java 11 minimum, it defaults to 8 in GitHub runners for some platforms
    export JAVA_HOME=${JAVA_HOME_11_X64}
    java_version=$(java -version 2>&1 | head -1 | cut -d'"' -f2 | sed '/^1\./s///' | cut -d'.' -f1)
    if [[ "$java_version" < 11 ]]; then
        echo "Android builds require Java 11, found version ${java_version} instead"
        exit 0
    fi
fi
