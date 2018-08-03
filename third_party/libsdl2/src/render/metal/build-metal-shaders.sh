#!/bin/bash

set -x
set -e
cd `dirname "$0"`

generate_shaders()
{
    platform=$1
    /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/usr/bin/metal -std=$platform-metal1.1 -Wall -O3 -o ./sdl.air ./SDL_shaders_metal.metal || exit $?
    /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/usr/bin/metal-ar rc sdl.metalar sdl.air || exit $?
    /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/usr/bin/metallib -o sdl.metallib sdl.metalar || exit $?
    xxd -i sdl.metallib | perl -w -p -e 's/\Aunsigned /const unsigned /;' >./SDL_shaders_metal_$platform.h
    rm -f sdl.air sdl.metalar sdl.metallib
}

generate_shaders osx
generate_shaders ios
