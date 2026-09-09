set(DXC_PROJECTS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(DXC_PROJECTS_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)
add_subdirectory(include/Tracing) # DxcRuntimeEtw target

if(HLSL_BUILD_DXILCONV)
  add_subdirectory(dxilconv)
endif (HLSL_BUILD_DXILCONV)

endif (WIN32)
