# Build ShaderBinary.lib.

find_package(D3D12 REQUIRED)

add_dxilconv_project_library(ShaderBinary
  ShaderBinary.cpp
)

target_include_directories(ShaderBinary
PUBLIC
  $<BUILD_INTERFACE:${DXILCONV_PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
PRIVATE
  ${D3D12_INCLUDE_DIRS}
)
