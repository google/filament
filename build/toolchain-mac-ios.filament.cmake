# ==================================================================================================
# Filament-specific settings
# The rest of the toolchain is downloaded from Apple Open Source and appended.
# ==================================================================================================

set(CMAKE_OSX_ARCHITECTURES ${IOS_ARCH} CACHE STRING "Build architecture for iOS")

# Necessary for correct install location
set(DIST_ARCH ${IOS_ARCH})

add_definitions(-DIOS)

set(IOS_MIN_TARGET "12.0")

if(PLATFORM_NAME STREQUAL "iphonesimulator")
    add_definitions(-DFILAMENT_IOS_SIMULATOR)
    # The simulator only supports iOS >= 13.0
    set(IOS_MIN_TARGET "13.0")
endif()

# ==================================================================================================
# End Filament-specific settings
# ==================================================================================================
