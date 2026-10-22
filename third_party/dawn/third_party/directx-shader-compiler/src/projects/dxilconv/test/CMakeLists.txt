# Test runner infrastructure for dxilconv. This configures the dxilconv test trees
# for use by Lit, and delegates to LLVM's lit test handlers.

if (CMAKE_CFG_INTDIR STREQUAL ".")
  set(LLVM_BUILD_MODE ".")
else ()
  set(LLVM_BUILD_MODE "%(build_mode)s")
endif ()

configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/taef/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/taef/lit.site.cfg
  )


# HLSL Change Begin - Add taef tests
add_lit_testsuite("check-dxilconv" "Running lit suite dxilconv"
          ${CMAKE_CURRENT_SOURCE_DIR}/taef
          PARAMS
                 dxilconv_site_config=${CMAKE_CURRENT_BINARY_DIR}/taef/lit.site.cfg
          DEPENDS dxilconv-tests
          ARGS ${CLANG_TEST_EXTRA_ARGS}
        )
# HLSL Change End
