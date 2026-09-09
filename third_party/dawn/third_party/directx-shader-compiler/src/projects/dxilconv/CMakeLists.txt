set(DXILCONV_PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(DXILCONV_PROJECT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

macro(add_dxilconv_project_library name)
  add_llvm_library(${name} ${ARGN})
#  add_definitions(/D_ITERATOR_DEBUG_LEVEL=0)
  set_output_directory(${name} ${LLVM_RUNTIME_OUTPUT_INTDIR} ${LLVM_LIBRARY_OUTPUT_INTDIR})  
  set_target_properties(${name} PROPERTIES FOLDER "Dxilconv libraries")
endmacro(add_dxilconv_project_library)

macro(add_dxilconv_project_executable name)
  add_llvm_executable(${name} ${ARGN})
  set_target_properties(${name} PROPERTIES FOLDER "Dxilconv executables")
endmacro(add_dxilconv_project_executable)

macro(add_dxilconv_project_test_library name)
  add_dxilconv_project_library(${name} ${ARGN})
  set_target_properties(${name} PROPERTIES FOLDER "Dxilconv tests")
endmacro(add_dxilconv_project_test_library)


if(WIN32)
  add_subdirectory(lib)
  add_subdirectory(tools)
  if (HLSL_INCLUDE_TESTS)
    add_subdirectory(unittests)
    add_subdirectory(test)
  endif (HLSL_INCLUDE_TESTS)
endif()