# ==================================================================================================
# Filament-specific settings
# The rest of the toolchain is downloaded from Apple Open Source and appended.
# ==================================================================================================

# In theory, we could support older architectures, but only arm64 Apple devices support Metal
set(IOS_ARCH arm64)

set(CMAKE_OSX_ARCHITECTURES ${IOS_ARCH} CACHE STRING "Build architecture for iOS")

# Necessary for correct install location
set(DIST_ARCH arm64)

add_definitions(-DIOS)

# ==================================================================================================
# End Filament-specific settings
# ==================================================================================================
