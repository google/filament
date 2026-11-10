# This file is included by filamat and glslang because glslang (and thus filamat) require
# iOS 13.0+ whereas the rest of Filament targets iOS 11.0.
# Only ever raises the floor: tvOS builds already target 17.0, and lowering it would
# conflict with the -target triple set by the toolchain (-Werror,-Woverriding-option).
if(IOS AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "13.0")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")
endif()
