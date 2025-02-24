#!/usr/bin/env bash

MY_DIR=$(dirname "$(readlink "$0")")
gsutil cp -r -z html -a public-read  "$MY_DIR/html" gs://chrome-infra-docs/flat/depot_tools/docs/
