#!/usr/bin/env bash

set -e

clang-format -style=file -i opengl-with-metal/*.h opengl-with-metal/*.m opengl-with-metal/*.mm

