# Builds dxbc2dxil.exe

find_package(D3D12 REQUIRED)

add_dxilconv_project_executable(dxbc2dxil
  dxbc2dxil.cpp
)

target_link_libraries(dxbc2dxil PRIVATE
  DxbcConverter
  LLVMDxcSupport
  LLVMDxilContainer
  LLVMMSSupport
  LLVMSupport
)

add_dependencies(dxbc2dxil DxbcConverter)

target_include_directories(dxbc2dxil PRIVATE
    ${D3D12_INCLUDE_DIRS}
)
