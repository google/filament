# ==================================================================================================
# Filament-specific settings
# The rest of the toolchain is downloaded from Apple Open Source and appended.
# ==================================================================================================

set(CMAKE_OSX_ARCHITECTURES ${IOS_ARCH} CACHE STRING "Build architecture for iOS")

set(IOS_MIN_TARGET "12.0" CACHE STRING "iOS min version to build for")

# Necessary for correct install location
set(DIST_ARCH ${IOS_ARCH})

add_definitions(-DIOS)

# ==================================================================================================
# End Filament-specific settings
# ==================================================================================================
