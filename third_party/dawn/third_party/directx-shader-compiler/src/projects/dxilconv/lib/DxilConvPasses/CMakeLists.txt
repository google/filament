# Build DxilConvPasses.lib.

add_dxilconv_project_library(DxilConvPasses
  NormalizeDxil.cpp
  ScopeNestedCFG.cpp
  InitializePasses.cpp
  ScopeNestInfo.cpp
  DxilCleanup.cpp
)

target_include_directories(DxilConvPasses
PUBLIC
  $<BUILD_INTERFACE:${DXILCONV_PROJECT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${DXILCONV_PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
