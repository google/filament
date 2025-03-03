# This CMake module is responsible for interpreting the user defined LLVM_
# options and executing the appropriate CMake commands to realize the users'
# selections.

# This is commonly needed so make sure it's defined before we include anything
# else.
string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)

include(HandleLLVMStdlib)
include(AddLLVMDefinitions)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

if(NOT LLVM_FORCE_USE_OLD_TOOLCHAIN)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
      message(FATAL_ERROR "Host GCC version must be at least 4.7!")
    endif()
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.1)
      message(FATAL_ERROR "Host Clang version must be at least 3.1!")
    endif()

    if (CMAKE_CXX_SIMULATE_ID MATCHES "MSVC")
      if (CMAKE_CXX_SIMULATE_VERSION VERSION_LESS 18.0)
        message(FATAL_ERROR "Host Clang must have at least -fms-compatibility-version=18.0")
      endif()
      set(CLANG_CL 1)
    elseif(NOT LLVM_ENABLE_LIBCXX)
      # Otherwise, test that we aren't using too old of a version of libstdc++
      # with the Clang compiler. This is tricky as there is no real way to
      # check the version of libstdc++ directly. Instead we test for a known
      # bug in libstdc++4.6 that is fixed in libstdc++4.7.
      set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
      set(OLD_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
      set(CMAKE_REQUIRED_FLAGS "-std=c++0x")
      check_cxx_source_compiles("
#include <atomic>
std::atomic<float> x(0.0f);
int main() { return (float)x; }"
        LLVM_NO_OLD_LIBSTDCXX)
      if(NOT LLVM_NO_OLD_LIBSTDCXX)
        message(FATAL_ERROR "Host Clang must be able to find libstdc++4.7 or newer!")
      endif()
      set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
      set(CMAKE_REQUIRED_LIBRARIES ${OLD_CMAKE_REQUIRED_LIBRARIES})
    endif()
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 18.0)
      message(FATAL_ERROR "Host Visual Studio must be at least 2013")
    elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 18.0.31101)
      message(WARNING "Host Visual Studio should at least be 2013 Update 4 (MSVC 18.0.31101)"
        "  due to miscompiles from earlier versions")
    endif()
  endif()
endif()

if( LLVM_ENABLE_ASSERTIONS )
  # MSVC doesn't like _DEBUG on release builds. See PR 4379.
  # HLSL Note: the above comment referrs to llvm.org problem, not pull request: 
  #            https://bugs.llvm.org/show_bug.cgi?id=4379
  if( NOT MSVC )
    add_definitions( -D_DEBUG )
  endif()
  # On non-Debug builds cmake automatically defines NDEBUG, so we
  # explicitly undefine it:
  if( NOT uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG" )
    add_definitions( -UNDEBUG )
    # Also remove /D NDEBUG to avoid MSVC warnings about conflicting defines.
    foreach (flags_var_to_scrub
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS_MINSIZEREL
        CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS_MINSIZEREL)
      string (REGEX REPLACE "(^| )[/-]D *NDEBUG($| )" " "
        "${flags_var_to_scrub}" "${${flags_var_to_scrub}}")
    endforeach()
  endif()

  if (LLVM_ASSERTIONS_TRAP)
    add_definitions( -DLLVM_ASSERTIONS_TRAP )
  endif()
  if (LLVM_ASSERTIONS_NO_STRINGS)
    add_definitions( -DLLVM_ASSERTIONS_NO_STRINGS )
  endif()
else()
  # Disable assertions in Debug builds
  if( uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG" )
    add_definitions( -DNDEBUG )
  endif()
endif()

string(TOUPPER "${LLVM_ABI_BREAKING_CHECKS}" uppercase_LLVM_ABI_BREAKING_CHECKS)

if( uppercase_LLVM_ABI_BREAKING_CHECKS STREQUAL "WITH_ASSERTS" )
  if( LLVM_ENABLE_ASSERTIONS )
    set( LLVM_ENABLE_ABI_BREAKING_CHECKS 1 )
  endif()
elseif( uppercase_LLVM_ABI_BREAKING_CHECKS STREQUAL "FORCE_ON" )
  set( LLVM_ENABLE_ABI_BREAKING_CHECKS 1 )
elseif( uppercase_LLVM_ABI_BREAKING_CHECKS STREQUAL "FORCE_OFF" )
  # We don't need to do anything special to turn off ABI breaking checks.
elseif( NOT DEFINED LLVM_ABI_BREAKING_CHECKS )
  # Treat LLVM_ABI_BREAKING_CHECKS like "FORCE_OFF" when it has not been
  # defined.
else()
  message(FATAL_ERROR "Unknown value for LLVM_ABI_BREAKING_CHECKS: \"${LLVM_ABI_BREAKING_CHECKS}\"!")
endif()

if(WIN32)
  set(LLVM_HAVE_LINK_VERSION_SCRIPT 0)
  if(CYGWIN)
    set(LLVM_ON_WIN32 0)
    set(LLVM_ON_UNIX 1)
  else(CYGWIN)
    set(LLVM_ON_WIN32 1)
    set(LLVM_ON_UNIX 0)
  endif(CYGWIN)
else(WIN32)
  if(UNIX)
    set(LLVM_ON_WIN32 0)
    set(LLVM_ON_UNIX 1)
    if(APPLE)
      set(LLVM_HAVE_LINK_VERSION_SCRIPT 0)
    else(APPLE)
      set(LLVM_HAVE_LINK_VERSION_SCRIPT 1)
    endif(APPLE)
  else(UNIX)
    MESSAGE(SEND_ERROR "Unable to determine platform")
  endif(UNIX)
endif(WIN32)

set(EXEEXT ${CMAKE_EXECUTABLE_SUFFIX})
set(LTDL_SHLIB_EXT ${CMAKE_SHARED_LIBRARY_SUFFIX})

# We use *.dylib rather than *.so on darwin.
set(LLVM_PLUGIN_EXT ${CMAKE_SHARED_LIBRARY_SUFFIX})

if(APPLE)
  # Darwin-specific linker flags for loadable modules.
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-flat_namespace -Wl,-undefined -Wl,suppress")
endif()

# Pass -Wl,-z,defs. This makes sure all symbols are defined. Otherwise a DSO
# build might work on ELF but fail on MachO/COFF.
if(NOT (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR WIN32 OR CYGWIN OR
        ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD") AND
   NOT LLVM_USE_SANITIZER)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,defs")
endif()


function(append value)
  foreach(variable ${ARGN})
    set(${variable} "${${variable}} ${value}" PARENT_SCOPE)
  endforeach(variable)
endfunction()

function(append_if condition value)
  if (${condition})
    foreach(variable ${ARGN})
      set(${variable} "${${variable}} ${value}" PARENT_SCOPE)
    endforeach(variable)
  endif()
endfunction()

macro(add_flag_if_supported flag name)
  check_c_compiler_flag("-Werror ${flag}" "C_SUPPORTS_${name}")
  append_if("C_SUPPORTS_${name}" "${flag}" CMAKE_C_FLAGS)
  check_cxx_compiler_flag("-Werror ${flag}" "CXX_SUPPORTS_${name}")
  append_if("CXX_SUPPORTS_${name}" "${flag}" CMAKE_CXX_FLAGS)
endmacro()

function(add_flag_or_print_warning flag name)
  check_c_compiler_flag("-Werror ${flag}" "C_SUPPORTS_${name}")
  check_cxx_compiler_flag("-Werror ${flag}" "CXX_SUPPORTS_${name}")
  if (C_SUPPORTS_${name} AND CXX_SUPPORTS_${name})
    message(STATUS "Building with ${flag}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" PARENT_SCOPE)
  else()
    message(WARNING "${flag} is not supported.")
  endif()
endfunction()

if( LLVM_USE_LINKER )
  set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -fuse-ld=${LLVM_USE_LINKER}")
  check_cxx_source_compiles("int main() { return 0; }" CXX_SUPPORTS_CUSTOM_LINKER)
  if ( NOT CXX_SUPPORTS_CUSTOM_LINKER )
    message(FATAL_ERROR "Host compiler does not support '-fuse-ld=${LLVM_USE_LINKER}'")
  endif()
  set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
  append("-fuse-ld=${LLVM_USE_LINKER}"
    CMAKE_EXE_LINKER_FLAGS CMAKE_MODULE_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS)
endif()

if( LLVM_ENABLE_PIC )
  if( XCODE )
    # Xcode has -mdynamic-no-pic on by default, which overrides -fPIC. I don't
    # know how to disable this, so just force ENABLE_PIC off for now.
    message(WARNING "-fPIC not supported with Xcode.")
  else()
    if( NOT WIN32 AND NOT CYGWIN )
      # On Windows all code is PIC. MinGW warns if -fPIC is used.
      add_flag_or_print_warning("-fPIC" FPIC)
    endif()

    if( (MINGW AND NOT CLANG) OR CYGWIN )
      # MinGW warns if -fvisibility-inlines-hidden is used.
    else()
      check_cxx_compiler_flag("-fvisibility-inlines-hidden" SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG)
      append_if(SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG "-fvisibility-inlines-hidden -fvisibility=hidden" CMAKE_CXX_FLAGS)
    endif()
  endif()
endif()

if( CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT WIN32 )
  # TODO: support other platforms and toolchains.
  if( LLVM_BUILD_32_BITS )
    message(STATUS "Building 32 bits executables and libraries.")
    add_llvm_definitions( -m32 )
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -m32")
  endif( LLVM_BUILD_32_BITS )
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT WIN32 )

if (LLVM_BUILD_STATIC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

if( XCODE )
  # For Xcode enable several build settings that correspond to
  # many warnings that are on by default in Clang but are
  # not enabled for historical reasons.  For versions of Xcode
  # that do not support these options they will simply
  # be ignored.
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_ABOUT_RETURN_TYPE "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_ABOUT_MISSING_NEWLINE "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_UNUSED_VALUE "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_UNUSED_VARIABLE "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_SIGN_COMPARE "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_UNUSED_FUNCTION "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_INITIALIZER_NOT_FULLY_BRACKETED "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_HIDDEN_VIRTUAL_FUNCTIONS "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_UNINITIALIZED_AUTOS "YES")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_WARN_BOOL_CONVERSION "YES")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_WARN_EMPTY_BODY "YES")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_WARN_ENUM_CONVERSION "YES")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_WARN_INT_CONVERSION "YES")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_WARN_CONSTANT_CONVERSION "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_NON_VIRTUAL_DESTRUCTOR "YES")
endif()

# On Win32 using MS tools, provide an option to set the number of parallel jobs
# to use.
if( MSVC_IDE )
  set(LLVM_COMPILER_JOBS "0" CACHE STRING
    "Number of parallel compiler jobs. 0 means use all processors. Default is 0.")
  if( NOT LLVM_COMPILER_JOBS STREQUAL "1" )
    if( LLVM_COMPILER_JOBS STREQUAL "0" )
      add_llvm_definitions( /MP )
    else()
      message(STATUS "Number of parallel compiler jobs set to " ${LLVM_COMPILER_JOBS})
      add_llvm_definitions( /MP${LLVM_COMPILER_JOBS} )
    endif()
  else()
    message(STATUS "Parallel compilation disabled")
  endif()
endif()

if( MSVC )
  include(ChooseMSVCCRT)

  if( NOT (${CMAKE_VERSION} VERSION_LESS 2.8.11) )
    # set stack reserved size to ~10MB
    # CMake previously automatically set this value for MSVC builds, but the
    # behavior was changed in CMake 2.8.11 (Issue 12437) to use the MSVC default
    # value (1 MB) which is not enough for us in tasks such as parsing recursive
    # C++ templates in Clang.
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10000000")
  endif()

  if( MSVC11 )
    add_llvm_definitions(-D_VARIADIC_MAX=10)
  endif()
  
  # Add definitions that make MSVC much less annoying.
  add_llvm_definitions(
    # For some reason MS wants to deprecate a bunch of standard functions...
    -D_CRT_SECURE_NO_DEPRECATE
    -D_CRT_SECURE_NO_WARNINGS
    -D_CRT_NONSTDC_NO_DEPRECATE
    -D_CRT_NONSTDC_NO_WARNINGS
    -D_SCL_SECURE_NO_DEPRECATE
    -D_SCL_SECURE_NO_WARNINGS
    )

  set(msvc_warning_flags
    # Disabled warnings.
    -wd4146 # Suppress 'unary minus operator applied to unsigned type, result still unsigned'
    -wd4180 # Suppress 'qualifier applied to function type has no meaning; ignored'
    -wd4244 # Suppress ''argument' : conversion from 'type1' to 'type2', possible loss of data'
    -wd4258 # Suppress ''var' : definition from the for loop is ignored; the definition from the enclosing scope is used'
    -wd4267 # Suppress ''var' : conversion from 'size_t' to 'type', possible loss of data'
    -wd4291 # Suppress ''declaration' : no matching operator delete found; memory will not be freed if initialization throws an exception'
    -wd4345 # Suppress 'behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized'
    -wd4351 # Suppress 'new behavior: elements of array 'array' will be default initialized'
    -wd4355 # Suppress ''this' : used in base member initializer list'
    -wd4456 # Suppress 'declaration of 'var' hides local variable'
    -wd4457 # Suppress 'declaration of 'var' hides function parameter'
    -wd4458 # Suppress 'declaration of 'var' hides class member'
    -wd4459 # Suppress 'declaration of 'var' hides global declaration'
    -wd4503 # Suppress ''identifier' : decorated name length exceeded, name was truncated'
    -wd4624 # Suppress ''derived class' : destructor could not be generated because a base class destructor is inaccessible'
    -wd4722 # Suppress 'function' : destructor never returns, potential memory leak
    -wd4800 # Suppress ''type' : forcing value to bool 'true' or 'false' (performance warning)'
    -wd4100 # Suppress 'unreferenced formal parameter'
    -wd4127 # Suppress 'conditional expression is constant'
    -wd4512 # Suppress 'assignment operator could not be generated'
    -wd4505 # Suppress 'unreferenced local function has been removed'
    -wd4610 # Suppress '<class> can never be instantiated'
    -wd4510 # Suppress 'default constructor could not be generated'
    -wd4702 # Suppress 'unreachable code'
    -wd4245 # Suppress 'signed/unsigned mismatch'
    -wd4706 # Suppress 'assignment within conditional expression'
    -wd4310 # Suppress 'cast truncates constant value'
    -wd4701 # Suppress 'potentially uninitialized local variable'
    -wd4703 # Suppress 'potentially uninitialized local pointer variable'
    -wd4389 # Suppress 'signed/unsigned mismatch'
    -wd4611 # Suppress 'interaction between '_setjmp' and C++ object destruction is non-portable'
    -wd4805 # Suppress 'unsafe mix of type <type> and type <type> in operation'
    -wd4204 # Suppress 'nonstandard extension used : non-constant aggregate initializer'

	# Ideally, we'd like this warning to be enabled, but MSVC 2013 doesn't
	# support the 'aligned' attribute in the way that clang sources requires (for
	# any code that uses the LLVM_ALIGNAS macro), so this is must be disabled to
	# avoid unwanted alignment warnings.
	# When we switch to requiring a version of MSVC that supports the 'alignas'
	# specifier (MSVC 2015?) this warning can be re-enabled.
    -wd4324 # Suppress 'structure was padded due to __declspec(align())'

    # Promoted warnings.
    # HLSL Change - don't do this - -w14062 # Promote 'enumerator in switch of enum is not handled' to level 1 warning.

    # Promoted warnings to errors.
    -we4238 # Promote 'nonstandard extension used : class rvalue used as lvalue' to error.
    )

  # HLSL Changes Start
  if (HLSL_ENABLE_ANALYZE)
    append("/analyze" CMAKE_CXX_FLAGS)
  endif ()

  # Change release to always build debug information out-of-line, but
  # also enable Reference optimization, ie dead function elimination.
  if (NOT CMAKE_MSVC_DEBUG_INFORMATION_FORMAT)
    append("/Zi" CMAKE_CXX_FLAGS_RELEASE)
    append("/DEBUG /OPT:REF" CMAKE_SHARED_LINKER_FLAGS_RELEASE)
    append("/DEBUG /OPT:REF" CMAKE_EXE_LINKER_FLAGS_RELEASE)
  endif()

  # HLSL Changes End

  # Enable warnings
  if (LLVM_ENABLE_WARNINGS)
    append("/W4" msvc_warning_flags)
    # CMake appends /W3 by default, and having /W3 followed by /W4 will result in 
    # cl : Command line warning D9025 : overriding '/W3' with '/W4'.  Since this is
    # a command line warning and not a compiler warning, it cannot be suppressed except
    # by fixing the command line.
    string(REGEX REPLACE " /W[0-4]" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REGEX REPLACE " /W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    if (LLVM_ENABLE_PEDANTIC)
      # No MSVC equivalent available
    endif (LLVM_ENABLE_PEDANTIC)

    if (CLANG_CL)
      append("-Wall -W -Wno-unused-parameter -Wwrite-strings -Wimplicit-fallthrough" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
      append("-Wcast-qual" CMAKE_CXX_FLAGS)

      # Disable unknown pragma warnings because the output is just too long with them.
      append("-Wno-unknown-pragmas" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)

      add_flag_if_supported("-Wno-unused-but-set-variable" UNUSED_BUT_SET_VARIABLE)
      append("-Wno-switch" CMAKE_CXX_FLAGS)

      append("-Wmissing-field-initializers" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)

      # enable warnings explicitly.
      append("-Wnonportable-include-path -Wunused-function" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
      append("-Wtrigraphs -Wconstant-logical-operand -Wunused-variable" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    endif (CLANG_CL)
  endif (LLVM_ENABLE_WARNINGS)
  if (LLVM_ENABLE_WERROR)
    append("/WX" msvc_warning_flags)
  endif (LLVM_ENABLE_WERROR)

  foreach(flag ${msvc_warning_flags})
    append("${flag}" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
  endforeach(flag)

  # Disable sized deallocation if the flag is supported. MSVC fails to compile
  # the operator new overload in User otherwise.
  check_c_compiler_flag("/WX /Zc:sizedDealloc-" SUPPORTS_SIZED_DEALLOC)
  append_if(SUPPORTS_SIZED_DEALLOC "/Zc:sizedDealloc-" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)

elseif( LLVM_COMPILER_IS_GCC_COMPATIBLE )
  if (LLVM_ENABLE_WARNINGS)
    append("-Wall -W -Wno-unused-parameter -Wwrite-strings -Wimplicit-fallthrough" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    append("-Wcast-qual" CMAKE_CXX_FLAGS)

    # Disable unknown pragma warnings because the output is just too long with them.
    append("-Wno-unknown-pragmas" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)

    if (MINGW)
      append("-Wno-implicit-fallthrough" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
      append("-Wno-missing-exception-spec" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
      append("-Wno-reorder-ctor" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
      append("-Wno-sign-compare" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
      append("-Wno-unused-const-variable" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
      append("-Wno-unused-function" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    endif()

    add_flag_if_supported("-Wno-unused-but-set-variable" UNUSED_BUT_SET_VARIABLE)
    add_flag_if_supported("-Wno-deprecated-copy" DEPRECATED_COPY)

    # Colorize GCC output even with ninja's stdout redirection.
    if (CMAKE_COMPILER_IS_GNUCXX)
       append("-fdiagnostics-color" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    endif (CMAKE_COMPILER_IS_GNUCXX)

    # Turn off missing field initializer warnings for gcc to avoid noise from
    # false positives with empty {}. Turn them on otherwise (they're off by
    # default for clang).
    check_cxx_compiler_flag("-Wmissing-field-initializers" CXX_SUPPORTS_MISSING_FIELD_INITIALIZERS_FLAG)
    if (CXX_SUPPORTS_MISSING_FIELD_INITIALIZERS_FLAG)
      if (CMAKE_COMPILER_IS_GNUCXX)
        append("-Wno-missing-field-initializers" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
      else()
        append("-Wmissing-field-initializers" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
      endif()
    endif()

    append_if(LLVM_ENABLE_PEDANTIC "-pedantic -Wno-long-long" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    # add_flag_if_supported("-Wcovered-switch-default" COVERED_SWITCH_DEFAULT_FLAG) # HLSL Change
    append("-Wno-switch" CMAKE_CXX_FLAGS) # HLSL Change
    append_if(USE_NO_UNINITIALIZED "-Wno-uninitialized" CMAKE_CXX_FLAGS)
    append_if(USE_NO_MAYBE_UNINITIALIZED "-Wno-maybe-uninitialized" CMAKE_CXX_FLAGS)

    # HLSL Change Starts

    # Windows' and by extension WinAdapter's non-Windows implementation for IUnknown
    # use virtual methods without virtual destructor, as that would add two extra
    # function-pointers to the vtable in turn offsetting those for every subclass,
    # resulting in ABI mismatches:
    # https://github.com/microsoft/DirectXShaderCompiler/issues/3783.
    # The -Wnon-virtual-dtor warning is disabled to allow this, conforming
    # with MSVC behaviour.

    # # Check if -Wnon-virtual-dtor warns even though the class is marked final.
    # # If it does, don't add it. So it won't be added on clang 3.4 and older.
    # # This also catches cases when -Wnon-virtual-dtor isn't supported by
    # # the compiler at all.
    # set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
    # set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -std=c++11 -Werror=non-virtual-dtor")
    # CHECK_CXX_SOURCE_COMPILES("class base {public: virtual void anchor();protected: ~base();};
    #                            class derived final : public base { public: ~derived();};
    #                            int main() { return 0; }"
    #                           CXX_WONT_WARN_ON_FINAL_NONVIRTUALDTOR)
    # set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
    # append_if(CXX_WONT_WARN_ON_FINAL_NONVIRTUALDTOR
    #           "-Wnon-virtual-dtor" CMAKE_CXX_FLAGS)

    # HLSL Change Ends

    # Check if -Wcomment is OK with an // comment ending with '\' if the next
    # line is also a // comment.
    set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -Werror -Wcomment")
    CHECK_C_SOURCE_COMPILES("// \\\\\\n//\\nint main() {return 0;}"
                            C_WCOMMENT_ALLOWS_LINE_WRAP)
    set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
    if (NOT C_WCOMMENT_ALLOWS_LINE_WRAP)
      append("-Wno-comment" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    endif()
  endif (LLVM_ENABLE_WARNINGS)
  append_if(LLVM_ENABLE_WERROR "-Werror" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
  if (NOT LLVM_ENABLE_TIMESTAMPS)
    add_flag_if_supported("-Werror=date-time" WERROR_DATE_TIME)
  endif ()

  if (LLVM_ENABLE_MODULES)
    set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -fmodules -fcxx-modules")
    # Check that we can build code with modules enabled, and that repeatedly
    # including <cassert> still manages to respect NDEBUG properly.
    CHECK_CXX_SOURCE_COMPILES("#undef NDEBUG
                               #include <cassert>
                               #define NDEBUG
                               #include <cassert>
                               int main() { assert(this code is not compiled); }"
                               CXX_SUPPORTS_MODULES)
    set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
    if (CXX_SUPPORTS_MODULES)
      append_if(CXX_SUPPORTS_MODULES "-fmodules" CMAKE_C_FLAGS)
      append_if(CXX_SUPPORTS_MODULES "-fmodules -fcxx-modules" CMAKE_CXX_FLAGS)
    else()
      message(FATAL_ERROR "LLVM_ENABLE_MODULES is not supported by this compiler")
    endif()
  endif(LLVM_ENABLE_MODULES)
endif( MSVC )

macro(append_common_sanitizer_flags)
  # Append -fno-omit-frame-pointer and turn on debug info to get better
  # stack traces.
  add_flag_if_supported("-fno-omit-frame-pointer" FNO_OMIT_FRAME_POINTER)
  if (NOT uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG" AND
      NOT uppercase_CMAKE_BUILD_TYPE STREQUAL "RELWITHDEBINFO")
    add_flag_if_supported("-gline-tables-only" GLINE_TABLES_ONLY)
  endif()
  # Use -O1 even in debug mode, otherwise sanitizers slowdown is too large.
  if (uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG" AND LLVM_OPTIMIZE_SANITIZED_BUILDS)
    add_flag_if_supported("-O1" O1)
  endif()
endmacro()

# Turn on sanitizers if necessary.
if(LLVM_USE_SANITIZER)
  if (LLVM_ON_UNIX)
    if (LLVM_USE_SANITIZER STREQUAL "Address")
      append_common_sanitizer_flags()
      append("-fsanitize=address" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    elseif (LLVM_USE_SANITIZER MATCHES "Memory(WithOrigins)?")
      append_common_sanitizer_flags()
      append("-fsanitize=memory" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
      if(LLVM_USE_SANITIZER STREQUAL "MemoryWithOrigins")
        append("-fsanitize-memory-track-origins" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
      endif()
    elseif (LLVM_USE_SANITIZER STREQUAL "Undefined")
      append_common_sanitizer_flags()
      append("-fsanitize=undefined -fno-sanitize=vptr,function,alignment -fno-sanitize-recover=all"
              CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    elseif (LLVM_USE_SANITIZER STREQUAL "Thread")
      append_common_sanitizer_flags()
      append("-fsanitize=thread" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    elseif (LLVM_USE_SANITIZER STREQUAL "Address;Undefined" OR
            LLVM_USE_SANITIZER STREQUAL "Undefined;Address")
      append_common_sanitizer_flags()
      append("-fsanitize=address,undefined -fno-sanitize=vptr,function,alignment -fno-sanitize-recover=all"
              CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    else()
      message(FATAL_ERROR "Unsupported value of LLVM_USE_SANITIZER: ${LLVM_USE_SANITIZER}")
    endif()
  else()
    if (LLVM_USE_SANITIZER STREQUAL "Address")
      append("-fsanitize=address" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    else()
      message(FATAL_ERROR "Unsupported value of LLVM_USE_SANITIZER: ${LLVM_USE_SANITIZER}")
    endif()
  endif()
  if (LLVM_USE_SANITIZE_COVERAGE)
    append("-fsanitize-coverage=edge,indirect-calls,8bit-counters,trace-cmp" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
  endif()
endif()

# Turn on -gsplit-dwarf if requested
if(LLVM_USE_SPLIT_DWARF)
  add_definitions("-gsplit-dwarf")
endif()

add_llvm_definitions( -D__STDC_CONSTANT_MACROS )
add_llvm_definitions( -D__STDC_FORMAT_MACROS )
add_llvm_definitions( -D__STDC_LIMIT_MACROS )

# clang doesn't print colored diagnostics when invoked from Ninja
if (UNIX AND
    CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND # HLSL Change - Update to CMake 3.13.4
    CMAKE_GENERATOR STREQUAL "Ninja")
  append("-fcolor-diagnostics" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
endif()

# HLSL Change Starts
# Enable -fms-extensions for clang to use MS uuid extensions for COM.
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  append("-fms-extensions -Wno-language-extension-token" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
endif()
# HLSL Change Ends

# Add flags for add_dead_strip().
# FIXME: With MSVS, consider compiling with /Gy and linking with /OPT:REF?
# But MinSizeRel seems to add that automatically, so maybe disable these
# flags instead if LLVM_NO_DEAD_STRIP is set.
if(NOT CYGWIN AND NOT WIN32)
  if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND
     NOT uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    check_c_compiler_flag("-Werror -fno-function-sections" C_SUPPORTS_FNO_FUNCTION_SECTIONS)
    if (C_SUPPORTS_FNO_FUNCTION_SECTIONS)
      # Don't add -ffunction-section if it can be disabled with -fno-function-sections.
      # Doing so will break sanitizers.
      add_flag_if_supported("-ffunction-sections" FFUNCTION_SECTIONS)
    endif()
    add_flag_if_supported("-fdata-sections" FDATA_SECTIONS)
  endif()
endif()

if(CYGWIN OR MINGW)
  # Prune --out-implib from executables. It doesn't make sense even
  # with --export-all-symbols.
  string(REGEX REPLACE "-Wl,--out-implib,[^ ]+ " " "
    CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE}")
  string(REGEX REPLACE "-Wl,--out-implib,[^ ]+ " " "
    CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE}")
endif()

if(MSVC)
  # Remove flags here, for exceptions and RTTI.
  # Each target property or source property should be responsible to control
  # them.
  # CL.EXE complains to override flags like "/GR /GR-".
  string(REGEX REPLACE "(^| ) */EH[-cs]+ *( |$)" "\\1 \\2" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  string(REGEX REPLACE "(^| ) */GR-? *( |$)" "\\1 \\2" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

# Provide public options to globally control RTTI and EH
option(LLVM_ENABLE_EH "Enable Exception handling" OFF)
option(LLVM_ENABLE_RTTI "Enable run time type information" OFF)
if(LLVM_ENABLE_EH AND NOT LLVM_ENABLE_RTTI)
  message(FATAL_ERROR "Exception handling requires RTTI. You must set LLVM_ENABLE_RTTI to ON")
endif()

if (MINGW)
  if (LLVM_ENABLE_EH)
    append("-fexceptions" CMAKE_CXX_FLAGS)
  endif()
  if (LLVM_ENABLE_RTTI)
    append("-frtti" CMAKE_CXX_FLAGS)
  endif()
endif()

# HLSL Change Begin
option(LLVM_ENABLE_LTO "Enable building with LTO" ${HLSL_OFFICIAL_BUILD})
if (LLVM_ENABLE_LTO)
  if(MSVC)
    if (CMAKE_CONFIGURATION_TYPES)
      set(_SUFFIX _RELEASE)
    endif()
    append("/GL" CMAKE_C_FLAGS${_SUFFIX} CMAKE_CXX_FLAGS${_SUFFIX})
    append("/LTCG" CMAKE_MODULE_LINKER_FLAGS${_SUFFIX} CMAKE_MODULE_LINKER_FLAGS${_SUFFIX} CMAKE_EXE_LINKER_FLAGS${_SUFFIX})
  else()
    add_flag_if_supported("-flto" SUPPORTS_FLTO)
  endif()
endif()
# HLSL Change End

option(LLVM_BUILD_INSTRUMENTED "Build LLVM and tools with PGO instrumentation (experimental)" Off)
mark_as_advanced(LLVM_BUILD_INSTRUMENTED)
append_if(LLVM_BUILD_INSTRUMENTED "-fprofile-instr-generate='${LLVM_PROFILE_FILE_PATTERN}'"
  CMAKE_CXX_FLAGS
  CMAKE_C_FLAGS
  CMAKE_EXE_LINKER_FLAGS
  CMAKE_SHARED_LINKER_FLAGS)

option(LLVM_BUILD_INSTRUMENTED_COVERAGE "Build LLVM and tools with Code Coverage instrumentation (experimental)" Off)
mark_as_advanced(LLVM_BUILD_INSTRUMENTED_COVERAGE)
append_if(LLVM_BUILD_INSTRUMENTED_COVERAGE "-fprofile-instr-generate='${LLVM_PROFILE_FILE_PATTERN}' -fcoverage-mapping"
  CMAKE_CXX_FLAGS
  CMAKE_C_FLAGS
  CMAKE_EXE_LINKER_FLAGS
  CMAKE_SHARED_LINKER_FLAGS)

# Plugin support
# FIXME: Make this configurable.
if(WIN32 OR CYGWIN)
  if(BUILD_SHARED_LIBS)
    set(LLVM_ENABLE_PLUGINS ON)
  else()
    set(LLVM_ENABLE_PLUGINS OFF)
  endif()
else()
  set(LLVM_ENABLE_PLUGINS ON)
endif()
