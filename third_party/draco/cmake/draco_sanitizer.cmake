if(DRACO_CMAKE_DRACO_SANITIZER_CMAKE_)
  return()
endif() # DRACO_CMAKE_DRACO_SANITIZER_CMAKE_
set(DRACO_CMAKE_DRACO_SANITIZER_CMAKE_ 1)

# Handles the details of enabling sanitizers.
macro(draco_configure_sanitizer)
  if(DRACO_SANITIZE AND NOT MSVC)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      if(DRACO_SANITIZE MATCHES "cfi")
        list(APPEND DRACO_CXX_FLAGS "-flto" "-fno-sanitize-trap=cfi")
        list(APPEND DRACO_EXE_LINKER_FLAGS "-flto" "-fno-sanitize-trap=cfi"
                    "-fuse-ld=gold")
      endif()

      if(${CMAKE_SIZEOF_VOID_P} EQUAL 4
         AND DRACO_SANITIZE MATCHES "integer|undefined")
        list(APPEND DRACO_EXE_LINKER_FLAGS "--rtlib=compiler-rt" "-lgcc_s")
      endif()
    endif()

    list(APPEND DRACO_CXX_FLAGS "-fsanitize=${DRACO_SANITIZE}")
    list(APPEND DRACO_EXE_LINKER_FLAGS "-fsanitize=${DRACO_SANITIZE}")

    # Make sanitizer callstacks accurate.
    list(APPEND DRACO_CXX_FLAGS "-fno-omit-frame-pointer"
                "-fno-optimize-sibling-calls")

    draco_test_cxx_flag(FLAG_LIST_VAR_NAMES DRACO_CXX_FLAGS FLAG_REQUIRED)
    draco_test_exe_linker_flag(FLAG_LIST_VAR_NAME DRACO_EXE_LINKER_FLAGS)
  endif()
endmacro()
