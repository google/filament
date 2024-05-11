# Determine if we should print to the output
if (CIVETWEB_ENABLE_THIRD_PARTY_OUTPUT)
  set(THIRD_PARTY_LOGGING 0)
else()
  set(THIRD_PARTY_LOGGING 1)
endif()

# We use the check unit testing framework for our C unit tests
include(ExternalProject)
#if(NOT WIN32)
#  # Apply the patch to check to fix CMake building on OS X
#  set(CHECK_PATCH_COMMAND patch
#     ${CIVETWEB_THIRD_PARTY_DIR}/src/check-unit-test-framework/CMakeLists.txt
#     ${CMAKE_SOURCE_DIR}/cmake/check/c82fe8888aacfe784476112edd3878256d2e30bc.patch
#   )
#else()
#  set(CHECK_PATCH_COMMAND "")
#endif()
ExternalProject_Add(check-unit-test-framework
  DEPENDS civetweb-c-library

## Use an official, released check version:
#  URL "https://codeload.github.com/libcheck/check/zip/${CIVETWEB_CHECK_VERSION}"
#  DOWNLOAD_NAME "${CIVETWEB_CHECK_VERSION}.zip"
#  URL_MD5 ${CIVETWEB_CHECK_MD5_HASH}

## Use a civetweb specific patched version
URL "https://github.com/civetweb/check/archive/master.zip"
DOWNLOAD_NAME "master.zip"
# <Edit this file to flush AppVeyor build cache and force reloading check>

  PREFIX "${CIVETWEB_THIRD_PARTY_DIR}"
  BUILD_IN_SOURCE 1
  PATCH_COMMAND ${CHECK_PATCH_COMMAND}
  CMAKE_ARGS
    "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
    "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}"
    "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
  LOG_DOWNLOAD ${THIRD_PARTY_LOGGING}
  LOG_UPDATE ${THIRD_PARTY_LOGGING}
  LOG_CONFIGURE ${THIRD_PARTY_LOGGING}
  LOG_BUILD ${THIRD_PARTY_LOGGING}
  LOG_TEST ${THIRD_PARTY_LOGGING}
  LOG_INSTALL ${THIRD_PARTY_LOGGING})

ExternalProject_Get_Property(check-unit-test-framework INSTALL_DIR)
set(CHECK_INSTALL_DIR ${INSTALL_DIR})
unset(INSTALL_DIR)
link_directories("${CHECK_INSTALL_DIR}/lib")
include_directories("${CHECK_INSTALL_DIR}/include")
if ((WIN32 AND MINGW) OR APPLE)
  set(CHECK_LIBRARIES "${CHECK_LIBRARIES};${CHECK_INSTALL_DIR}/lib/libcheck.a")
  set(CHECK_LIBRARIES "${CHECK_LIBRARIES};${CHECK_INSTALL_DIR}/lib/libcompat.a")
elseif (WIN32)
  set(CHECK_LIBRARIES "${CHECK_LIBRARIES};${CHECK_INSTALL_DIR}/lib/check.lib")
  set(CHECK_LIBRARIES "${CHECK_LIBRARIES};${CHECK_INSTALL_DIR}/lib/compat.lib")
else()
  set(CHECK_LIBRARIES "${CHECK_INSTALL_DIR}/lib/libcheck.a")
endif()
find_package(LibM)
if (LIBM_FOUND)
  set(CHECK_LIBRARIES "${CHECK_LIBRARIES};LIBM::LIBM")
endif()
find_package(LibRt)
if (LIBRT_FOUND)
  set(CHECK_LIBRARIES "${CHECK_LIBRARIES};LIBRT::LIBRT")
endif()

# Build the C unit tests
add_library(shared-c-unit-tests STATIC shared.c)
target_include_directories(
  shared-c-unit-tests PUBLIC
  ${PROJECT_SOURCE_DIR}/include)

add_library(public-func-c-unit-tests STATIC public_func.c)
if (BUILD_SHARED_LIBS)
  target_compile_definitions(public-func-c-unit-tests PRIVATE CIVETWEB_DLL_IMPORTS)
endif()
target_include_directories(
  public-func-c-unit-tests PUBLIC
  ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(public-func-c-unit-tests civetweb-c-library ${CHECK_LIBRARIES})
add_dependencies(public-func-c-unit-tests check-unit-test-framework)

add_library(public-server-c-unit-tests STATIC public_server.c)
if (BUILD_SHARED_LIBS)
  target_compile_definitions(public-server-c-unit-tests PRIVATE CIVETWEB_DLL_IMPORTS)
endif()
target_include_directories(
  public-server-c-unit-tests PUBLIC
  ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(public-server-c-unit-tests civetweb-c-library ${CHECK_LIBRARIES})
add_dependencies(public-server-c-unit-tests check-unit-test-framework)

add_library(private-c-unit-tests STATIC private.c)
target_include_directories(
  private-c-unit-tests PUBLIC
  ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(private-c-unit-tests ${CHECK_LIBRARIES})
add_dependencies(private-c-unit-tests check-unit-test-framework)

add_library(timer-c-unit-tests STATIC timertest.c)
target_include_directories(
  timer-c-unit-tests PUBLIC
  ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(timer-c-unit-tests ${CHECK_LIBRARIES})
add_dependencies(timer-c-unit-tests check-unit-test-framework)

add_library(exe-c-unit-tests STATIC private_exe.c)
if (BUILD_SHARED_LIBS)
  target_compile_definitions(exe-c-unit-tests PRIVATE)
endif()
target_include_directories(
  exe-c-unit-tests PUBLIC
  ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(exe-c-unit-tests civetweb-c-library ${CHECK_LIBRARIES})
add_dependencies(exe-c-unit-tests check-unit-test-framework)

add_executable(main-c-unit-test main.c)
target_link_libraries(main-c-unit-test
  shared-c-unit-tests
  public-func-c-unit-tests
  public-server-c-unit-tests
  private-c-unit-tests
  timer-c-unit-tests
  exe-c-unit-tests
  ${CHECK_LIBRARIES})
add_dependencies(main-c-unit-test check-unit-test-framework)

# Add a check command that builds the dependent test program
add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND}
  DEPENDS main-c-unit-test)

# A macro for adding tests
macro(civetweb_add_test suite test_case)
  set(test "test-${suite}-${test_case}")
  string(TOLOWER "${test}" test)
  string(REGEX REPLACE "[^-A-Za-z0-9]" "-" test "${test}")
  add_test(
    NAME ${test}
    COMMAND main-c-unit-test "--test-dir=${CMAKE_CURRENT_SOURCE_DIR}" "--suite=${suite}" "--test-case=${test_case}")
  if (WIN32)
    string(REPLACE ";" "\\;" test_path "$ENV{PATH}")
    set_tests_properties(${test} PROPERTIES
      ENVIRONMENT "PATH=${test_path}\\;$<TARGET_FILE_DIR:civetweb-c-library>")
  endif()
endmacro(civetweb_add_test)


# Tests of private functions
civetweb_add_test(Private "HTTP Message")
civetweb_add_test(Private "HTTP Keep Alive")
civetweb_add_test(Private "URL Parsing 1")
civetweb_add_test(Private "URL Parsing 2")
civetweb_add_test(Private "URL Parsing 3")
civetweb_add_test(Private "Internal Parsing 1")
civetweb_add_test(Private "Internal Parsing 2")
civetweb_add_test(Private "Internal Parsing 3")
civetweb_add_test(Private "Internal Parsing 4")
civetweb_add_test(Private "Internal Parsing 5")
civetweb_add_test(Private "Internal Parsing 6")
civetweb_add_test(Private "Encode Decode")
civetweb_add_test(Private "Mask Data")
civetweb_add_test(Private "Date Parsing")
civetweb_add_test(Private "SHA1")
civetweb_add_test(Private "Config Options")

# Public API function tests
civetweb_add_test(PublicFunc "Version")
civetweb_add_test(PublicFunc "Options")
civetweb_add_test(PublicFunc "MIME types")
civetweb_add_test(PublicFunc "strcasecmp")
civetweb_add_test(PublicFunc "URL encoding decoding")
civetweb_add_test(PublicFunc "Cookies and variables")
civetweb_add_test(PublicFunc "MD5")
civetweb_add_test(PublicFunc "Aux functions")

# Public API server tests
civetweb_add_test(PublicServer "Check test environment")
civetweb_add_test(PublicServer "Init library")
civetweb_add_test(PublicServer "Start threads")
civetweb_add_test(PublicServer "Minimal HTTP Server")
civetweb_add_test(PublicServer "Minimal HTTPS Server")
civetweb_add_test(PublicServer "Minimal HTTP Client")
civetweb_add_test(PublicServer "Minimal HTTPS Client")
civetweb_add_test(PublicServer "Start Stop HTTP Server")
civetweb_add_test(PublicServer "Start Stop HTTP Server IPv6")
civetweb_add_test(PublicServer "Start Stop HTTPS Server")
civetweb_add_test(PublicServer "TLS Server Client")
civetweb_add_test(PublicServer "Server Requests")
civetweb_add_test(PublicServer "Store Body")
civetweb_add_test(PublicServer "Handle Form")
civetweb_add_test(PublicServer "HTTP Authentication")
civetweb_add_test(PublicServer "HTTP Keep Alive")
civetweb_add_test(PublicServer "Error handling")
civetweb_add_test(PublicServer "Error logging")
civetweb_add_test(PublicServer "Limit speed")
civetweb_add_test(PublicServer "Large file")
civetweb_add_test(PublicServer "File in memory")

# Timer tests
civetweb_add_test(Timer "Timer Single Shot")
civetweb_add_test(Timer "Timer Periodic")
civetweb_add_test(Timer "Timer Mixed")

# Tests with main.c
#civetweb_add_test(EXE "Helper funcs")


# Add the coverage command(s)
if (${CMAKE_BUILD_TYPE} MATCHES "[Cc]overage")
  find_program(GCOV_EXECUTABLE gcov)
  find_program(LCOV_EXECUTABLE lcov)
  find_program(GENHTML_EXECUTABLE genhtml)
  find_program(CTEST_EXECUTABLE ctest)
  if (GCOV_EXECUTABLE AND LCOV_EXECUTABLE AND GENHTML_EXECUTABLE AND CTEST_EXECUTABLE AND HAVE_C_FLAG_COVERAGE)
    add_custom_command(
      OUTPUT ${CMAKE_BINARY_DIR}/lcov/index.html
      COMMAND ${LCOV_EXECUTABLE} -q -z -d .
      COMMAND ${LCOV_EXECUTABLE} -q --no-external -c -b "${CMAKE_SOURCE_DIR}" -d . -o before.lcov -i
      COMMAND ${CTEST_EXECUTABLE} --force-new-ctest-process
      COMMAND ${LCOV_EXECUTABLE} -q --no-external -c -b "${CMAKE_SOURCE_DIR}" -d . -o after.lcov
      COMMAND ${LCOV_EXECUTABLE} -q -a before.lcov -a after.lcov --output-file final.lcov
      COMMAND ${LCOV_EXECUTABLE} -q -r final.lcov "'${CMAKE_SOURCE_DIR}/unittest/*'" -o final.lcov
      COMMAND ${GENHTML_EXECUTABLE} final.lcov -o lcov --demangle-cpp --sort -p "${CMAKE_SOURCE_DIR}" -t benchmark
      DEPENDS main-c-unit-test
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Running LCOV"
    )
    add_custom_target(coverage
      DEPENDS ${CMAKE_BINARY_DIR}/lcov/index.html
      COMMENT "LCOV report at lcov/index.html"
    )
    message(STATUS "Coverage command added")
  else()
    if (HAVE_C_FLAG_COVERAGE)
      set(C_FLAG_COVERAGE_MESSAGE supported)
    else()
      set(C_FLAG_COVERAGE_MESSAGE unavailable)
    endif()
    message(WARNING
      "Coverage command not available:\n"
      "  gcov: ${GCOV_EXECUTABLE}\n"
      "  lcov: ${LCOV_EXECUTABLE}\n"
      "  genhtml: ${GENHTML_EXECUTABLE}\n"
      "  ctest: ${CTEST_EXECUTABLE}\n"
      "  --coverage flag: ${C_FLAG_COVERAGE_MESSAGE}")
  endif()
endif()
