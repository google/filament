# Test runner infrastructure for Clang. This configures the Clang test trees
# for use by Lit, and delegates to LLVM's lit test handlers.

if (CMAKE_CFG_INTDIR STREQUAL ".")
  set(LLVM_BUILD_MODE ".")
else ()
  set(LLVM_BUILD_MODE "%(build_mode)s")
endif ()

string(REPLACE ${CMAKE_CFG_INTDIR} ${LLVM_BUILD_MODE} CLANG_TOOLS_DIR ${LLVM_RUNTIME_OUTPUT_INTDIR})

set(HLSL_HEADERS_DIR ${LLVM_SOURCE_DIR}/tools/clang/lib/Headers/hlsl) # HLSL Change

configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  )

configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/Unit/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/Unit/lit.site.cfg
  )

configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/taef/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/taef/lit.site.cfg
  )

configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/taef_exec/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/taef_exec/lit.site.cfg
  )

option(CLANG_TEST_USE_VG "Run Clang tests under Valgrind" OFF)
if(CLANG_TEST_USE_VG)
  set(CLANG_TEST_EXTRA_ARGS ${CLANG_TEST_EXTRA_ARGS} "--vg")
endif ()

list(APPEND CLANG_TEST_DEPS
  clang clang-headers
  clang-check clang-format
  c-index-test diagtool
  clang-tblgen
  )

if (CLANG_ENABLE_ARCMT)
  list(APPEND CLANG_TEST_DEPS
    arcmt-test
    c-arcmt-test
  )
endif ()

if (ENABLE_CLANG_EXAMPLES)
  list(APPEND CLANG_TEST_DEPS
    clang-interpreter
    PrintFunctionNames
    )
endif ()

if (ENABLE_CLANG_STATIC_ANALYZER AND ENABLE_CLANG_EXAMPLES)
  list(APPEND CLANG_TEST_DEPS
    SampleAnalyzerPlugin
    )
endif ()

set(CLANG_TEST_PARAMS
  clang_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  )

if( NOT CLANG_BUILT_STANDALONE )
  list(APPEND CLANG_TEST_DEPS
    llvm-config
    llc opt FileCheck count not llvm-symbolizer llvm-profdata llvm-objdump
    )
endif()

# HLSL Change Begin
# Explicitly overriding check-clang dependencies for HLSL
set(CLANG_TEST_DEPS dxc dxa dxopt dxl dxv dxr dxcompiler clang-tblgen llvm-config opt FileCheck count not ClangUnitTests)
if (WIN32)
list(APPEND CLANG_TEST_DEPS
     dxc_batch ExecHLSLTests dxildll
    )
else()
list(APPEND CLANG_TEST_DEPS
     dxildll
    )
endif()
add_custom_target(clang-test-depends DEPENDS ${CLANG_TEST_DEPS})
set_target_properties(clang-test-depends PROPERTIES FOLDER "Clang tests")
# HLSL Change End


add_lit_testsuite(check-clang "Running the Clang regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  #LIT ${LLVM_LIT}
  PARAMS ${CLANG_TEST_PARAMS}
    skip_taef_exec=False
  DEPENDS ${CLANG_TEST_DEPS}
  ARGS ${CLANG_TEST_EXTRA_ARGS}
  )
set_target_properties(check-clang PROPERTIES FOLDER "Clang tests")

# Add a legacy target spelling: clang-test
add_custom_target(clang-test)
add_dependencies(clang-test check-clang)
set_target_properties(clang-test PROPERTIES FOLDER "Clang tests")

# HLSL Change Begin - Generate lit targets for test subdirectories.

set(CLANG_TEST_PARAMS
  ${CLANG_TEST_PARAMS}
  clang_unit_site_config=${CMAKE_CURRENT_BINARY_DIR}/Unit/lit.site.cfg
  clang_taef_site_config=${CMAKE_CURRENT_BINARY_DIR}/taef/lit.site.cfg
  no_priority=True
  clang_taef_exec_site_config=${CMAKE_CURRENT_BINARY_DIR}/taef_exec/lit.site.cfg
  )

add_lit_testsuites(CLANG ${CMAKE_CURRENT_SOURCE_DIR}
  PARAMS ${CLANG_TEST_PARAMS}
  DEPENDS ${CLANG_TEST_DEPS}
  FOLDER "Clang tests/Suites"
)

# Manually generate targets that we need to expose in visual studio builds.

# The code below here _ONLY_ executes when building with Visual Studio or Xcode.
if (NOT CMAKE_CONFIGURATION_TYPES)
  return()
endif()

# Add the unit test suite
add_lit_target("check-clang-unit" "Running lit suite clang-unit"
          ${CMAKE_CURRENT_SOURCE_DIR}/Unit
          PARAMS ${CLANG_TEST_PARAMS}
          DEPENDS ClangUnitTests
          ARGS ${CLANG_TEST_EXTRA_ARGS}
        )

# Add TAEF targets
if (WIN32)
  add_lit_target("check-clang-taef" "Running lit suite hlsl"
            ${CMAKE_CURRENT_SOURCE_DIR}/taef
            PARAMS ${CLANG_TEST_PARAMS}
            DEPENDS ClangHLSLTests
            ARGS ${CLANG_TEST_EXTRA_ARGS}
          )
  set(TAEF_EXEC_ADAPTER "" CACHE STRING "adapter for taef exec test")

  # Use a custom target so we can depend on it and re-run the cmake logic which downloads warp
  # from nuget if requested.
  add_custom_target(WarpFromNuget
            COMMAND "${CMAKE_COMMAND}"
            -DCMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR} 
            -DBUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DBINARY_DIR=${CMAKE_BINARY_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/modules/nuget.cmake")

  add_lit_target("check-clang-taef-exec" "Running lit suite hlsl execution test"
            ${CMAKE_CURRENT_SOURCE_DIR}/taef_exec
            PARAMS ${CLANG_TEST_PARAMS}
                adapter=${TAEF_EXEC_ADAPTER}
            DEPENDS ExecHLSLTests dxexp
            ARGS ${CLANG_TEST_EXTRA_ARGS}
          )

  add_lit_target("check-clang-taef-exec-warp" "Running lit suite hlsl execution test with D3D WARP from nuget"
            ${CMAKE_CURRENT_SOURCE_DIR}/taef_exec
            PARAMS ${CLANG_TEST_PARAMS}
                adapter=${TAEF_EXEC_ADAPTER}
            DEPENDS ExecHLSLTests dxexp WarpFromNuget
            ARGS ${CLANG_TEST_EXTRA_ARGS}
          )
endif()
# HLSL Change End
