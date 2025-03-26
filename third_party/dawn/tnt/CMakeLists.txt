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
set(DAWN_DXC_ENABLE_ASSERTS_IN_NDEBUG OFF)

#set(TINT_BUILD_CMD_TOOLS OFF)

if (FILAMENT_BUILD_FILAMAT)
    set(TINT_BUILD_SPV_READER ON)
endif ()

add_subdirectory(../  ${PROJECT_BINARY_DIR}/third_party/dawn)

# Need to install libtint for filamat on Android.
# See third_party/dawn/tnt/CMakeLists.txt and android/filamat-android/CMakeLists.txt.
if (ANDROID AND FILAMENT_BUILD_FILAMAT)
    set(COMBINED_TARGET libtint_combined)
    set(COMBINED_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${COMBINED_TARGET}.a")

    # TODO: maybe use this target in place of libtint in filamat proper. (it's smaller)
    set(LIBTINT_DEPS
        tint_lang_core
        tint_lang_core_constant
        tint_lang_core_intrinsic
        tint_lang_core_type
        tint_lang_spirv_reader
        tint_lang_spirv_reader_ast_lower
        tint_lang_spirv_reader_ast_parser
        tint_lang_spirv_reader_common
        tint_lang_spirv_writer
        tint_lang_wgsl
        tint_lang_wgsl_ast
        tint_lang_wgsl_ast_transform
        tint_lang_wgsl_features
        tint_lang_wgsl_intrinsic
        tint_lang_wgsl_program
        tint_lang_wgsl_resolver
        tint_lang_wgsl_sem
        tint_lang_wgsl_writer
        tint_lang_wgsl_writer_ast_printer
        tint_results
        tint_utils
        tint_utils_diagnostic
        tint_utils_ice
        tint_utils_result
        tint_utils_rtti
        tint_utils_strconv
        tint_utils_symbol
        tint_utils_text
    )

    # Use the following no-op definitions to introduce a library and its dependency.
    add_library(${COMBINED_TARGET} STATIC dummy.cpp)
    target_link_libraries(${COMBINED_TARGET} PRIVATE
        ${LIBTINT_DEPS}
    )
    combine_static_libs(${COMBINED_TARGET} "${COMBINED_OUTPUT}" "${LIBTINT_DEPS}")
    install(FILES "${COMBINED_OUTPUT}" DESTINATION lib/${DIST_DIR})
endif()
