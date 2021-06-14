set(VMA_LIBRARY_SOURCE_FILES
    VmaUsage.cpp
)

add_library(VulkanMemoryAllocator ${VMA_LIBRARY_SOURCE_FILES})

set_target_properties(
    VulkanMemoryAllocator PROPERTIES

    CXX_EXTENSIONS OFF
    # Use C++14
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(VulkanMemoryAllocator PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Only link to Vulkan if static linking is used
if (NOT ${VMA_DYNAMIC_VULKAN_FUNCTIONS})
    target_link_libraries(VulkanMemoryAllocator PUBLIC Vulkan::Vulkan)
endif()

target_compile_definitions(
    VulkanMemoryAllocator

    PUBLIC
    VMA_USE_STL_CONTAINERS=$<BOOL:${VMA_USE_STL_CONTAINERS}>
    VMA_DYNAMIC_VULKAN_FUNCTIONS=$<BOOL:${VMA_DYNAMIC_VULKAN_FUNCTIONS}>
    VMA_DEBUG_ALWAYS_DEDICATED_MEMORY=$<BOOL:${VMA_DEBUG_ALWAYS_DEDICATED_MEMORY}>
    VMA_DEBUG_INITIALIZE_ALLOCATIONS=$<BOOL:${VMA_DEBUG_INITIALIZE_ALLOCATIONS}>
    VMA_DEBUG_GLOBAL_MUTEX=$<BOOL:${VMA_DEBUG_GLOBAL_MUTEX}>
    VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT=$<BOOL:${VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT}>
    VMA_RECORDING_ENABLED=$<BOOL:${VMA_RECORDING_ENABLED}>
)

if (VMA_BUILD_SAMPLE)
    if(WIN32)
        set(VMA_SAMPLE_SOURCE_FILES
            Common.cpp
            SparseBindingTest.cpp
            Tests.cpp
            VulkanSample.cpp
        )

        add_executable(VmaSample ${VMA_SAMPLE_SOURCE_FILES})

        # Visual Studio specific settings
        if(${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
            # Use Unicode instead of multibyte set
            add_compile_definitions(UNICODE _UNICODE)
            
            # Set VmaSample as startup project
            set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "VmaSample")
            
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

if(VMA_BUILD_REPLAY)
    add_subdirectory(VmaReplay)
endif()
