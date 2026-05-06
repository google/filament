#include "../../webgpu.h"

// Compile-test the instantiation of all of the macros, and spot-check types
int main(void) {
    {
        WGPUStringView s = WGPU_STRING_VIEW_INIT;
        s.length = WGPU_STRLEN;
    }
    {
        WGPUTextureViewDescriptor a;
        a.mipLevelCount = WGPU_MIP_LEVEL_COUNT_UNDEFINED;
        a.arrayLayerCount = WGPU_ARRAY_LAYER_COUNT_UNDEFINED;
    }
    {
        WGPUTexelCopyBufferLayout a;
        uint32_t x = a.bytesPerRow = WGPU_COPY_STRIDE_UNDEFINED;
        uint32_t y = a.rowsPerImage = WGPU_COPY_STRIDE_UNDEFINED;
    }
    {
        WGPURenderPassColorAttachment a;
        a.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    }
    {
        WGPUBindGroupEntry a;
        uint64_t x = a.size = WGPU_WHOLE_SIZE;
    }
    {
        uint64_t x = WGPU_WHOLE_MAP_SIZE;
    }
    {
        WGPULimits a;
        uint32_t x = a.maxTextureDimension2D = WGPU_LIMIT_U32_UNDEFINED;
        uint64_t y = a.maxBufferSize = WGPU_LIMIT_U64_UNDEFINED;
    }
    {
        WGPUPassTimestampWrites a;
        a.beginningOfPassWriteIndex = WGPU_QUERY_SET_INDEX_UNDEFINED;
        a.endOfPassWriteIndex = WGPU_QUERY_SET_INDEX_UNDEFINED;
    }

    // Simple chaining smoke test
    {
        // It's not valid to use both WGSL and SPIRV but this test doesn't care.
        WGPUShaderSourceSPIRV descSPIRV = WGPU_SHADER_SOURCE_SPIRV_INIT;
        WGPUShaderSourceWGSL descWGSL = WGPU_SHADER_SOURCE_WGSL_INIT;
        WGPUShaderModuleDescriptor desc = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;

        // Test of linking one extension to another.
        descWGSL.chain.next = &descSPIRV.chain;
        // Test of linking base struct to an extension.
        desc.nextInChain = &descWGSL.chain;

        // Also test the alternate linking style using a cast.
#ifdef __cplusplus
#    if __cplusplus >= 201103L
        static_assert(offsetof(WGPUShaderSourceWGSL, chain) == 0, "");
#    endif
        descWGSL.chain.next = reinterpret_cast<WGPUChainedStruct*>(&descSPIRV);
        desc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&descWGSL);
#else
        descWGSL.chain.next = (WGPUChainedStruct*) &descSPIRV;
        desc.nextInChain = (WGPUChainedStruct*) &descWGSL;
#endif
    }

    // Check that generated initializers are valid
    #include "./init_tests_autogen.inl"
}
