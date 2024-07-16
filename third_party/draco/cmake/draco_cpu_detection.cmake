if(DRACO_CMAKE_DRACO_CPU_DETECTION_CMAKE_)
  return()
endif() # DRACO_CMAKE_DRACO_CPU_DETECTION_CMAKE_
set(DRACO_CMAKE_DRACO_CPU_DETECTION_CMAKE_ 1)

# Detect optimizations available for the current target CPU.
macro(draco_optimization_detect)
  if(DRACO_ENABLE_OPTIMIZATIONS)
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" cpu_lowercase)
    if(cpu_lowercase MATCHES "^arm|^aarch64")
      set(draco_have_neon ON)
    elseif(cpu_lowercase MATCHES "^x86|amd64")
      set(draco_have_sse4 ON)
    endif()
  endif()

  if(draco_have_neon AND DRACO_ENABLE_NEON)
    list(APPEND draco_defines "DRACO_ENABLE_NEON=1")
  else()
    list(APPEND draco_defines "DRACO_ENABLE_NEON=0")
  endif()

  if(draco_have_sse4 AND DRACO_ENABLE_SSE4_1)
    list(APPEND draco_defines "DRACO_ENABLE_SSE4_1=1")
  else()
    list(APPEND draco_defines "DRACO_ENABLE_SSE4_1=0")
  endif()
endmacro()
