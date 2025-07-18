# We have an import of opencv with many third party libraries stripped out.
# The option overrides prevent opencv from attempting to build these libraries.

option(WITH_JPEG "Disable jpeg lib in opencv" OFF)
option(WITH_FLATBUFFERS "Disable flatbuffers lib in opencv" OFF)
option(WITH_TIFF "Disable tiff lib in opencv" OFF)
option(WITH_WEBP "Disable webp lib in opencv" OFF)
option(WITH_JPEGXL "Disable jpegxl lib in opencv" OFF)
option(WITH_OPENJPEG "Disable openjpeg lib in opencv" OFF)
option(WITH_JASPER "Disable jasper lib in opencv" OFF)
option(WITH_OPENEXR "Disable openexr lib in opencv" OFF)
option(WITH_PROTOBUF "Disable protobuf lib in opencv" OFF)
option(WITH_CAROTENE "Disable carotene lib in opencv" OFF)
option(WITH_ITT "Disable itt lib in opencv" OFF)
option(WITH_IPP "Disable ipp libs in opencv" OFF)
option(BUILD_PNG "Disable libpng build in opencv" OFF)

# Override opencv cache variables to let it know where zlib is.
set(ZLIB_LIBRARY zlib CACHE INTERNAL "")
set(ZLIB_LIBRARIES ${ZLIB_LIBRARY})
set(ZLIB_INCLUDE_DIR "${EXTERNAL}/zlib" CACHE INTERNAL "")
set(ZLIB_FOUND true)

# Override opencv cache variables to let it know where libpng is.
set(PNG_LIBRARY png CACHE INTERNAL "")
set(PNG_LIBRARIES ${PNG_LIBRARY})
set(PNG_INCLUDE_DIR "${EXTERNAL}/libpng" CACHE INTERNAL "")
set(PNG_DEFINITIONS "")
set(PNG_FOUND true)

add_subdirectory(${EXTERNAL}/opencv ${CMAKE_BINARY_DIR}/opencv)
