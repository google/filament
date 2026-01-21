# This file is included by filamat and glslang because glslang (and thus filamat) require
# iOS 13.0+ whereas the rest of Filament targets iOS 11.0.
if(IOS)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")
endif()
