#!/usr/bin/env bash

set -ex

cd backend-test && xcodegen && cd ..
cd gltf-viewer && xcodegen && cd ..
cd hello-ar && xcodegen && cd ..
cd hello-gltf && xcodegen && cd ..
cd hello-pbr && xcodegen && cd ..
cd hello-triangle && xcodegen && cd ..
cd transparent-rendering && xcodegen && cd ..
