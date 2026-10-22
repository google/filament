# Test runner infrastructure for Clang. This configures the Clang test trees
# for use by Lit, and delegates to LLVM's lit test handlers.

if (CMAKE_CFG_INTDIR STREQUAL ".")
  set(LLVM_BUILD_MODE ".")
else ()
  set(LLVM_BUILD_MODE "%(build_mode)s")
endif ()

string(REPLACE ${CMAKE_CFG_INTDIR} ${LLVM_BUILD_MODE} CLANG_TOOLS_DIR ${LLVM_RUNTIME_OUTPUT_INTDIR})

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

# HLSL Change begin 
# This must be done before configuring taef_exec's lit.site.cfg.
include(taef_exec/DownloadWarp.cmake)
# HLSL Change end


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

# -------------------------------------------------------------------------
# DXIL Backward Compatibility Section
# -------------------------------------------------------------------------

# Define DXC releases as (NAME VERSION) pairs
set(DXC_RELEASES  
  dxc_2025_07_14 v1.8.2505.1
  dxc_2025_02_20 v1.8.2502
  dxc_2023_08_14 v1.7.2308
  dxc_2021_12_08 v1.6.2112
)

# Download a DXC release
function(add_released_dxc name version)
  ExternalProject_Add(${name}
      URL https://github.com/microsoft/DirectXShaderCompiler/releases/download/${version}/${name}.zip
      DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/tools/clang/test/dxc_releases
      SOURCE_DIR ${CMAKE_BINARY_DIR}/tools/clang/test/dxc_releases/${version}/${name}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      DOWNLOAD_EXTRACT_TIMESTAMP NEW
  )
endfunction()

if(WIN32)
  # Iterate over releases
  list(LENGTH DXC_RELEASES DXC_RELEASES_LENGTH)
  math(EXPR num_pairs "${DXC_RELEASES_LENGTH} / 2")
  math(EXPR loop_end "${num_pairs} - 1")
  foreach(i RANGE 0 ${loop_end})
    math(EXPR idx_name "${i}*2")
    math(EXPR idx_version "${i}*2 + 1")

    list(GET DXC_RELEASES ${idx_name} name)
    list(GET DXC_RELEASES ${idx_version} version)

    add_released_dxc(${name} ${version})

  # Determine target architecture for DXC binaries
    if(CMAKE_GENERATOR MATCHES "Visual Studio")
        # Multi-config generators (VS/Xcode)
        if(NOT DEFINED CMAKE_GENERATOR_PLATFORM)
            message(FATAL_ERROR "CMAKE_GENERATOR_PLATFORM not set for multi-config generator")
        endif()
        set(TARGET_ARCH ${CMAKE_GENERATOR_PLATFORM})
    else()
        # Single-config generators (Ninja, Makefiles)
        string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" SYSTEM_PROCESSOR_LOWER)

        if(SYSTEM_PROCESSOR_LOWER MATCHES "^(x86|i[3-6]86)$")
            set(TARGET_ARCH x86)
        elseif(SYSTEM_PROCESSOR_LOWER MATCHES "^(x64|amd64|x86_64)$")
            set(TARGET_ARCH x64)
        elseif(SYSTEM_PROCESSOR_LOWER MATCHES "^(arm64|aarch64)$")
            set(TARGET_ARCH arm64)
        elseif(SYSTEM_PROCESSOR_LOWER MATCHES "^arm")
            set(TARGET_ARCH arm)
        else()
            message(FATAL_ERROR "Unknown architecture: ${CMAKE_SYSTEM_PROCESSOR}")
        endif()
    endif()

    # Path to dxil.dll for this release
    set(DXC_PATH ${CMAKE_BINARY_DIR}/tools/clang/test/dxc_releases/${version}/${name}/bin/${TARGET_ARCH}/dxil.dll)

    # The only sub-dirs that make sense to run with the released external dxil
    # validator setting are CodeGenDXIL, DXC, and HLSLFileCheckLit.
    # It would be much better to run Unit/TAEF tests too, but those tests don't
    # currently respect the DXC_DXIL_DLL_PATH environment variable.
    # LitDXILValidation is also skipped, since it is a regression test for the
    # latest validator only.

    # Create a lit target for this release
    add_lit_target("check-dxilcompat-${name}"
      "Running clang regression tests with dxil validator from ${name}"
      ${CMAKE_CURRENT_SOURCE_DIR}/CodeGenDXIL
      ${CMAKE_CURRENT_SOURCE_DIR}/DXC
      ${CMAKE_CURRENT_SOURCE_DIR}/HLSLFileCheckLit
      PARAMS ${CLANG_TEST_PARAMS}
      DXC_DXIL_DLL_PATH=${DXC_PATH} 
      DXC_RELEASE_VERSION_TAG=${version}
      DEPENDS ${CLANG_TEST_DEPS} ${name}
      ARGS ${CLANG_TEST_EXTRA_ARGS}
    )
  endforeach()
endif()
# -------------------------------------------------------------------------

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


# add_lit_testsuites doesn't generate targets for Visual Studio or Xcode builds
# (since these IDEs cannot handle the huge number of targets it generates).
#
# We must manually generate targets that we want to expose when using these
# generators.
if(CMAKE_CONFIGURATION_TYPES)

  # Add the unit test suite
  add_lit_target("check-clang-unit" "Running lit suite clang-unit"
            ${CMAKE_CURRENT_SOURCE_DIR}/Unit
            PARAMS ${CLANG_TEST_PARAMS}
            DEPENDS ClangUnitTests
            ARGS ${CLANG_TEST_EXTRA_ARGS}
          )

  # Add TAEF targets
  if(WIN32)
    add_lit_target("check-clang-taef" "Running lit suite hlsl"
              ${CMAKE_CURRENT_SOURCE_DIR}/taef
              PARAMS ${CLANG_TEST_PARAMS}
              DEPENDS ClangHLSLTests
              ARGS ${CLANG_TEST_EXTRA_ARGS}
            )
    set(TAEF_EXEC_ADAPTER "" CACHE STRING "adapter for taef exec test")

    add_lit_target("check-clang-taef-exec" "Running lit suite hlsl execution test"
              ${CMAKE_CURRENT_SOURCE_DIR}/taef_exec
              PARAMS ${CLANG_TEST_PARAMS}
              adapter=${TAEF_EXEC_ADAPTER}
              DEPENDS ExecHLSLTests dxexp
              ARGS ${CLANG_TEST_EXTRA_ARGS}
            )
  endif()
endif()

# -------------------------------------------------------------------------
# External Validator Testing
# -------------------------------------------------------------------------

# The only sub-dirs that make sense to run with the external dxil validator
# setting are CodeGenDXIL, DXC, HLSLFileCheckLit, and LitDXILValidation.
# It would be much better to run Unit/TAEF tests too, but those tests don't
# currently respect the DXC_DXIL_DLL_PATH environment variable.

add_lit_target("check-extdxil"
  "Running clang regression tests with built external dxil validator"
  ${CMAKE_CURRENT_SOURCE_DIR}/CodeGenDXIL
  ${CMAKE_CURRENT_SOURCE_DIR}/DXC
  ${CMAKE_CURRENT_SOURCE_DIR}/HLSLFileCheckLit
  ${CMAKE_CURRENT_SOURCE_DIR}/LitDXILValidation
  PARAMS ${CLANG_TEST_PARAMS}
  # Set path to external validator build target
  "DXC_DXIL_DLL_PATH=$<TARGET_FILE:dxildll>"
  DEPENDS ${CLANG_TEST_DEPS}
  ARGS ${CLANG_TEST_EXTRA_ARGS}
)
# -------------------------------------------------------------------------

# HLSL Change End
