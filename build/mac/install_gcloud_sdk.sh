#!/bin/bash

set -e

curl -OL https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/google-cloud-sdk-268.0.0-darwin-x86_64.tar.gz
gunzip google-cloud-sdk-268.0.0-darwin-x86_64.tar.gz
tar -zxvf google-cloud-sdk-268.0.0-darwin-x86_64.tar 2>/dev/null
export PATH="$PWD/google-cloud-sdk/bin:$PATH"
