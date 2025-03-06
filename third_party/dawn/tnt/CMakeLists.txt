# TODO: This causes a compilation error in dawn. Disabling it now.
# This is temporary and should be removed.
set(TINT_BUILD_GLSL_VALIDATOR OFF)

set(TINT_BUILD_TESTS OFF)
set(TINT_BUILD_BENCHMARKS OFF)
set(TINT_BUILD_IR_BINARY OFF)

set(DAWN_BUILD_TESTS OFF)
set(DAWN_BUILD_SAMPLES OFF)
set(DAWN_ABSEIL_DIR ${EXTERNAL}/abseil)
set(DAWN_SPIRV_TOOLS_DIR ${EXTERNAL}/spirv-tools/)
set(DAWN_SPIRV_HEADERS_DIR ${EXTERNAL}/spirv-headers)
set(DAWN_GLSLANG_DIR ${EXTERNAL}/glslang/)

add_subdirectory(../  ${PROJECT_BINARY_DIR}/third_party/dawn)