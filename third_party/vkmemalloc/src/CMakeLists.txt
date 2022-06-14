set(VMA_LIBRARY_SOURCE_FILES
    VmaUsage.cpp
    VmaUsage.h
    "${PROJECT_SOURCE_DIR}/include/vk_mem_alloc.h"
)

# Visual Studio only debugger symbols
if(WIN32 AND ${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
    set(VMA_LIBRARY_SOURCE_FILES ${VMA_LIBRARY_SOURCE_FILES} vk_mem_alloc.natvis)
endif()

set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_RELWITHDEBINFO_POSTFIX rd)
set(CMAKE_MINSIZEREL_POSTFIX s)

add_library(VulkanMemoryAllocator ${VMA_LIBRARY_SOURCE_FILES})

set_target_properties(
    VulkanMemoryAllocator PROPERTIES

    CXX_EXTENSIONS OFF
    # Use C++14
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(VulkanMemoryAllocator PUBLIC "${PROJECT_SOURCE_DIR}/include")

# Only link to Vulkan if static linking is used
if(${VMA_STATIC_VULKAN_FUNCTIONS})
    target_link_libraries(VulkanMemoryAllocator PUBLIC Vulkan::Vulkan)
endif()

target_compile_definitions(
    VulkanMemoryAllocator

    PUBLIC
    VMA_STATIC_VULKAN_FUNCTIONS=$<BOOL:${VMA_STATIC_VULKAN_FUNCTIONS}>
    VMA_DYNAMIC_VULKAN_FUNCTIONS=$<BOOL:${VMA_DYNAMIC_VULKAN_FUNCTIONS}>
    VMA_DEBUG_ALWAYS_DEDICATED_MEMORY=$<BOOL:${VMA_DEBUG_ALWAYS_DEDICATED_MEMORY}>
    VMA_DEBUG_INITIALIZE_ALLOCATIONS=$<BOOL:${VMA_DEBUG_INITIALIZE_ALLOCATIONS}>
    VMA_DEBUG_GLOBAL_MUTEX=$<BOOL:${VMA_DEBUG_GLOBAL_MUTEX}>
    VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT=$<BOOL:${VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT}>
    VMA_RECORDING_ENABLED=$<BOOL:${VMA_RECORDING_ENABLED}>
)

install(TARGETS VulkanMemoryAllocator DESTINATION "lib")
install(FILES "${PROJECT_SOURCE_DIR}/include/vk_mem_alloc.h" DESTINATION "include")

if(VMA_BUILD_SAMPLE)
    if(WIN32)
        set(VMA_SAMPLE_SOURCE_FILES
            Common.cpp
            Common.h
            SparseBindingTest.cpp
            SparseBindingTest.h
            Tests.cpp
            Tests.h
            VulkanSample.cpp
        )

        add_executable(VmaSample ${VMA_SAMPLE_SOURCE_FILES})
		add_dependencies(VmaSample VulkanMemoryAllocator VmaSampleShaders)
		
        # Visual Studio specific settings
        if(${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
            # Use Unicode instead of multibyte set
            add_compile_definitions(UNICODE _UNICODE)
            
            # Set VmaSample as startup project
            set_property(DIRECTORY "${PROJECT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT "VmaSample")

            # Add C++ warnings and security checks
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive- /sdl /W3")
            
            # Enable multithreaded compiling
            target_compile_options(VmaSample PRIVATE "/MP")

            # Set working directory for Visual Studio debugger
            set_target_properties(
                VmaSample
                PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
            )
        endif()

        set_target_properties(
            VmaSample PROPERTIES

            CXX_EXTENSIONS OFF
            # Use C++14
            CXX_STANDARD 14
            CXX_STANDARD_REQUIRED ON
        )

        target_link_libraries(
            VmaSample
            PRIVATE
            
            VulkanMemoryAllocator
            Vulkan::Vulkan
        )
    else()
        message(STATUS "VmaSample application is not supported to Linux")
    endif()
endif()

if(VMA_BUILD_SAMPLE_SHADERS)
    add_subdirectory(Shaders)
endif()
