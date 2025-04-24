#!/bin/bash

if [[ "$GITHUB_WORKFLOW" ]]; then
    echo "Running workflow $GITHUB_WORKFLOW (event: $GITHUB_EVENT_NAME, action: $GITHUB_ACTION)"
fi
