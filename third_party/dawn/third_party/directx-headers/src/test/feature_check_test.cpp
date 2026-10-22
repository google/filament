// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef _WIN32
#include <wsl/winadapter.h>
#endif

#include <iostream>
#include <directx/d3d12.h>
#include <directx/dxcore.h>
#include <directx/d3dx12.h>
#include "dxguids/dxguids.h"

// -----------------------------------------------------------------------------------------------------------------
// Helper Macros for verifying feature check results
// -----------------------------------------------------------------------------------------------------------------

// Verifies the correctness of a feature check that should not fail
#define VERIFY_FEATURE_CHECK_NO_DEFAULT(Feature) \
{ \
    if (features.Feature() != Data.Feature) \
    { \
        std::cout << "Verification failed: " << #Feature << std::endl \
                  << "Old API: " << Data.Feature << std::endl \
                  << "New API: " << features.Feature() << std::endl; \
        return -1; \
    } \
}

// Verifies the return value from a feature check with a different name from the old API that should not fail
#define VERIFY_RENAMED_FEATURE_CHECK_NO_DEFAULT(NewFeature, OldFeature) \
{ \
    if (features.NewFeature() != Data.OldFeature) \
    { \
        std::cout << "Verification failed: " << #NewFeature << std::endl \
                  << "Old API: " << Data.OldFeature << std::endl \
                  << "New API: " << features.NewFeature() << std::endl; \
        return -1; \
    } \
}

// Verifies if the feature check returns the correct default value upon failure
#define VERIFY_DEFAULT(Feature, Default) \
{ \
    if (features.Feature() != Default) \
    { \
        std::cout << "Verification failed: " << #Feature << std::endl \
                  << "Default: " << Default << std::endl \
                  << "Actual: " << features.Feature() << std::endl; \
        return -1; \
    } \
}

// Verifies the result of a feature check.
// If the feature check failed because it's not supported by runtime, ensures that it returns the default value
#define VERIFY_FEATURE_CHECK(Feature, Default) \
{ \
    if (FAILED(InitResult)) \
    { \
        VERIFY_DEFAULT(Feature, Default); \
    } \
    else \
    { \
        VERIFY_FEATURE_CHECK_NO_DEFAULT(Feature); \
    } \
}

// Verifies the result of a feature check. The requested capability may have a different name in the new API
// If the feature check failed because it's not supported by runtime, ensures that it returns the default value
#define VERIFY_RENAMED_FEATURE_CHECK(Feature, OldName, Default) \
{ \
    if (FAILED(InitResult)) \
    { \
        VERIFY_DEFAULT(Feature, Default); \
    } \
    else \
    { \
        VERIFY_RENAMED_FEATURE_CHECK_NO_DEFAULT(Feature, OldName); \
    } \
}

// Initialize the feature support data structure to be used in the original CheckFeatureSupport API
#define INITIALIZE_FEATURE_SUPPORT_DATA(FeatureName) \
HRESULT InitResult; \
if (FAILED(InitResult = device->CheckFeatureSupport(D3D12_FEATURE_##FeatureName, &Data, sizeof(D3D12_FEATURE_DATA_##FeatureName)))) \
{ \
    std::cout << "Feature not supported by device: " << #FeatureName << std::endl; \
}

// Prints out the mismatched feature check result to stdout
#define PRINT_CHECK_FAILED_MESSAGE(FeatureName, OldFeature, NewFeature) \
std::cout << "Verification failed: " << #FeatureName << std::endl \
          << "Old API: " << OldFeature << std::endl \
          << "New API: " << NewFeature << std::endl;


// -----------------------------------------------------------------------------------------------------------------
// Main function
// -----------------------------------------------------------------------------------------------------------------
int main()
{
    ID3D12Device *device = nullptr;

    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) 
    {
        return -1;
    }

    // Initialize the new FeatureSupport class
    // Notice that it is shared among all feature checks
    CD3DX12FeatureSupport features;
    if (FAILED(features.Init(device)))
    {
        return -1;
    }

    UINT NodeCount = device->GetNodeCount();

    // Each feature comes with two code sections
    // The first section shows how to use the API's of the new FeautreSupport class
    // The second section contains tests that ensures the new API returns the same result as the old API
    // 0: D3D12_OPTIONS
    {
        BOOL DoublePrecisionFloatShaderOps = features.DoublePrecisionFloatShaderOps();
        BOOL OutputMergerLogicOp = features.OutputMergerLogicOp();
        D3D12_TILED_RESOURCES_TIER TiledResourceTier = features.TiledResourcesTier();
        D3D12_RESOURCE_BINDING_TIER ResourceBindingTier = features.ResourceBindingTier();
        BOOL PSSpecifiedStencilRefSupported = features.PSSpecifiedStencilRefSupported();
        BOOL TypedUAVLoadAdditionalFormats = features.TypedUAVLoadAdditionalFormats();
        BOOL ROVsSupported = features.ROVsSupported();
        D3D12_CONSERVATIVE_RASTERIZATION_TIER ConservativeRasterizationTier = features.ConservativeRasterizationTier();
        BOOL StandardSwizzle64KBSupported = features.StandardSwizzle64KBSupported();
        BOOL CrossAdapterRowMajorTextureSupported = features.CrossAdapterRowMajorTextureSupported();
        BOOL VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation = features.VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation();
        D3D12_RESOURCE_HEAP_TIER ResourceHeapTier = features.ResourceHeapTier();
        D3D12_CROSS_NODE_SHARING_TIER CrossNodeSharingTier = features.CrossNodeSharingTier();
        UINT MaxGPUVirtualAddressBitsPerResource = features.MaxGPUVirtualAddressBitsPerResource();

        D3D12_FEATURE_DATA_D3D12_OPTIONS Data;
        device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &Data, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS));
        VERIFY_FEATURE_CHECK_NO_DEFAULT(DoublePrecisionFloatShaderOps);
        VERIFY_FEATURE_CHECK_NO_DEFAULT(OutputMergerLogicOp);
        VERIFY_FEATURE_CHECK_NO_DEFAULT(TiledResourcesTier);
        VERIFY_FEATURE_CHECK_NO_DEFAULT(ResourceBindingTier);
        VERIFY_FEATURE_CHECK_NO_DEFAULT(PSSpecifiedStencilRefSupported);
        VERIFY_FEATURE_CHECK_NO_DEFAULT(TypedUAVLoadAdditionalFormats);
        VERIFY_FEATURE_CHECK_NO_DEFAULT(ROVsSupported);
        VERIFY_FEATURE_CHECK_NO_DEFAULT(ConservativeRasterizationTier);
        VERIFY_FEATURE_CHECK_NO_DEFAULT(StandardSwizzle64KBSupported);
        VERIFY_FEATURE_CHECK_NO_DEFAULT(CrossAdapterRowMajorTextureSupported);
        VERIFY_FEATURE_CHECK_NO_DEFAULT(VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation);
        VERIFY_FEATURE_CHECK_NO_DEFAULT(ResourceHeapTier);
        VERIFY_FEATURE_CHECK_NO_DEFAULT(CrossNodeSharingTier);
        VERIFY_FEATURE_CHECK_NO_DEFAULT(MaxGPUVirtualAddressBitsPerResource);
    }

    // 2: Feature Levels
    {
        D3D_FEATURE_LEVEL HighestLevelSupported = features.MaxSupportedFeatureLevel();
        
        D3D12_FEATURE_DATA_FEATURE_LEVELS Data;
        D3D_FEATURE_LEVEL KnownFeatureLevels[] =
        {
            D3D_FEATURE_LEVEL_1_0_CORE,
            D3D_FEATURE_LEVEL_9_1,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_2
        };
        Data.pFeatureLevelsRequested = KnownFeatureLevels;
        Data.NumFeatureLevels = sizeof(KnownFeatureLevels) / sizeof(D3D_FEATURE_LEVEL);
        device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &Data, sizeof(D3D12_FEATURE_DATA_FEATURE_LEVELS));
        VERIFY_FEATURE_CHECK_NO_DEFAULT(MaxSupportedFeatureLevel);
    }

    // 3: Format Support
    {
        D3D12_FORMAT_SUPPORT1 Support1;
        D3D12_FORMAT_SUPPORT2 Support2;
        if (FAILED(features.FormatSupport(DXGI_FORMAT_R16G16B16A16_FLOAT, Support1, Support2))) {
            std::cout << "Error: FormatSupport failed" << std::endl;
            return -1;
        }

        D3D12_FEATURE_DATA_FORMAT_SUPPORT Data;
        Data.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &Data, sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT)))) 
        {
            if (Support1 != Data.Support1)
            {
                std::cout << "Verification failed: " << "FormatSupport1" << std::endl 
                          << "Old API: " << Data.Support1 << std::endl 
                          << "New API: " << Support1 << std::endl; 
                return -1;
            }

            if (Support2 != Data.Support2)
            {
                std::cout << "Verification failed: " << "FormatSupport2" << std::endl 
                          << "Old API: " << Data.Support2 << std::endl 
                          << "New API: " << Support2 << std::endl; 
                return -1;
            }
        }
    }

    // 4: Multisample Quality Support
    {
        UINT NumQualityLevels;
        if (FAILED(features.MultisampleQualityLevels(DXGI_FORMAT_R16G16B16A16_FLOAT, 1, D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE, NumQualityLevels))) {
            std::cout << "Error: MultisampleQualityLevels failed" << std::endl;
            return -1;
        }

        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS Data;
        Data.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        Data.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
        Data.SampleCount = 1;
        device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &Data, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
        if (NumQualityLevels != Data.NumQualityLevels)
        {
            std::cout << "Verification failed: " << "MultisampleQualityLevels" << std::endl 
                        << "Old API: " << Data.NumQualityLevels << std::endl 
                        << "New API: " << NumQualityLevels << std::endl; 
            return -1;
        }
    }

    // 5: Format Info
    {
        UINT8 PlaneCount;
        if (FAILED(features.FormatInfo(DXGI_FORMAT_R16G16B16A16_FLOAT, PlaneCount))) 
        {
            std::cout << "Error: FormatInfo failed" << std::endl;
            return -1;
        }

        D3D12_FEATURE_DATA_FORMAT_INFO Data;
        Data.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO, &Data, sizeof(D3D12_FEATURE_DATA_FORMAT_INFO));
        if (PlaneCount != Data.PlaneCount)
        {
            PRINT_CHECK_FAILED_MESSAGE(FormatInfo, Data.PlaneCount, PlaneCount);
        }
    }

    // 6: GPU Virtual Address Support
    {
        UINT MaxGPUVABitsPerProcess = features.MaxGPUVirtualAddressBitsPerProcess();
        UINT MaxGPUVABitsPerResource = features.MaxGPUVirtualAddressBitsPerResource();

        D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT Data;
        device->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, &Data, sizeof(D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT));
        VERIFY_FEATURE_CHECK_NO_DEFAULT(MaxGPUVirtualAddressBitsPerProcess);
        VERIFY_FEATURE_CHECK_NO_DEFAULT(MaxGPUVirtualAddressBitsPerResource);
    }

    // 7: Shader Model
    {
        D3D_SHADER_MODEL HighestShaderModel = features.HighestShaderModel();

        D3D12_FEATURE_DATA_SHADER_MODEL Data;
        D3D_SHADER_MODEL KnownShaderModels[] =
        {
            D3D_SHADER_MODEL_6_7,
            D3D_SHADER_MODEL_6_6,
            D3D_SHADER_MODEL_6_5,
            D3D_SHADER_MODEL_6_4,
            D3D_SHADER_MODEL_6_3,
            D3D_SHADER_MODEL_6_2,
            D3D_SHADER_MODEL_6_1,
            D3D_SHADER_MODEL_6_0,
            D3D_SHADER_MODEL_5_1
        };
        UINT NumKnownShaderModels = sizeof(KnownShaderModels) / sizeof(D3D_SHADER_MODEL);
        for (UINT i = 0; i < NumKnownShaderModels; i++)
        {
            Data.HighestShaderModel = D3D_SHADER_MODEL_6_7; // Highest Known Shader Model
            if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &Data, sizeof(D3D12_FEATURE_DATA_SHADER_MODEL)))) 
            {
                VERIFY_FEATURE_CHECK_NO_DEFAULT(HighestShaderModel);
            }
        }
    }
    
    // 8: Options1
    {
        BOOL WaveOps = features.WaveOps();
        UINT WaveLaneCountMin = features.WaveLaneCountMin();
        UINT WaveLaneCountMax = features.WaveLaneCountMax();

        D3D12_FEATURE_DATA_D3D12_OPTIONS1 Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(D3D12_OPTIONS1);
        VERIFY_FEATURE_CHECK(WaveOps, false);
        VERIFY_FEATURE_CHECK(WaveLaneCountMax, 0);
        VERIFY_FEATURE_CHECK(WaveLaneCountMin, 0);
    }

    // 10: Protected Resource Session Support
    for (UINT NodeId = 0; NodeId < NodeCount; NodeId++)
    {
        D3D12_PROTECTED_RESOURCE_SESSION_SUPPORT_FLAGS ProtectedResourceSessionSupport = features.ProtectedResourceSessionSupport(NodeId);

        D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_SUPPORT Data;
        Data.NodeIndex = NodeId;
        INITIALIZE_FEATURE_SUPPORT_DATA(PROTECTED_RESOURCE_SESSION_SUPPORT);
        VERIFY_RENAMED_FEATURE_CHECK(ProtectedResourceSessionSupport, Support, D3D12_PROTECTED_RESOURCE_SESSION_SUPPORT_FLAG_NONE);
    }

    // 12: Root Signature
    {
        D3D_ROOT_SIGNATURE_VERSION HighestRootSignatureVersion = features.HighestRootSignatureVersion();

        D3D12_FEATURE_DATA_ROOT_SIGNATURE Data;
        Data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1; // Highest Known Version
        INITIALIZE_FEATURE_SUPPORT_DATA(ROOT_SIGNATURE);
        VERIFY_RENAMED_FEATURE_CHECK(HighestRootSignatureVersion, HighestVersion, (D3D_ROOT_SIGNATURE_VERSION)0);
    }

    // 16: Architecture1
    for (UINT NodeId = 0; NodeId < NodeCount; NodeId++)
    {
        BOOL IsolatedMMU = features.IsolatedMMU(NodeId);
        BOOL TileBasedRenderer = features.TileBasedRenderer(NodeId);
        BOOL UMA = features.UMA(NodeId);
        BOOL CacheCoherentUMA = features.CacheCoherentUMA(NodeId);

        D3D12_FEATURE_DATA_ARCHITECTURE1 Data;
        Data.NodeIndex = NodeId;
        INITIALIZE_FEATURE_SUPPORT_DATA(ARCHITECTURE1);
        if (FAILED(InitResult)) 
        {
            D3D12_FEATURE_DATA_ARCHITECTURE Data;
            Data.NodeIndex = NodeId;
            INITIALIZE_FEATURE_SUPPORT_DATA(ARCHITECTURE);
            VERIFY_FEATURE_CHECK_NO_DEFAULT(TileBasedRenderer);
            VERIFY_FEATURE_CHECK_NO_DEFAULT(UMA);
            VERIFY_FEATURE_CHECK_NO_DEFAULT(CacheCoherentUMA);
        }
        else
        {
            VERIFY_FEATURE_CHECK(TileBasedRenderer, false);
            VERIFY_FEATURE_CHECK(UMA, false);
            VERIFY_FEATURE_CHECK(CacheCoherentUMA, false);
        }
        VERIFY_FEATURE_CHECK(IsolatedMMU, false);
    }

    // 18: Options2
    {
        BOOL DepthBoundsTestSupported = features.DepthBoundsTestSupported();
        D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER ProgrammableSamplePositionsTier = features.ProgrammableSamplePositionsTier();

        D3D12_FEATURE_DATA_D3D12_OPTIONS2 Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(D3D12_OPTIONS2);
        VERIFY_FEATURE_CHECK(DepthBoundsTestSupported, false);
        VERIFY_FEATURE_CHECK(ProgrammableSamplePositionsTier, D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_NOT_SUPPORTED);
    }

    // 19: Shader Cache
    {
        D3D12_SHADER_CACHE_SUPPORT_FLAGS ShaderCacheSupportFlags = features.ShaderCacheSupportFlags();

        D3D12_FEATURE_DATA_SHADER_CACHE Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(SHADER_CACHE);
        VERIFY_RENAMED_FEATURE_CHECK(ShaderCacheSupportFlags, SupportFlags, D3D12_SHADER_CACHE_FLAG_NONE);
    }

    // 20: Command Queue Prioirity
    {
        // Basic check
        BOOL CommandQueuePrioritySupportedPositive = features.CommandQueuePrioritySupported(D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL);
        // Check for Global Realtime support
        BOOL CommandQueuePrioritySupportedRealtime = features.CommandQueuePrioritySupported(D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_QUEUE_PRIORITY_GLOBAL_REALTIME);
        // Test on invalid inputs
        BOOL CommandQueuePrioritySupportedInvalid = features.CommandQueuePrioritySupported((D3D12_COMMAND_LIST_TYPE)7, 0);

        D3D12_FEATURE_DATA_COMMAND_QUEUE_PRIORITY Data;
        {
            Data.CommandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
            Data.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            INITIALIZE_FEATURE_SUPPORT_DATA(COMMAND_QUEUE_PRIORITY);
            if (CommandQueuePrioritySupportedPositive != Data.PriorityForTypeIsSupported)
            {
                PRINT_CHECK_FAILED_MESSAGE(CommandQueuePriority, Data.PriorityForTypeIsSupported, CommandQueuePrioritySupportedPositive);
            }
        }

        {
            Data.CommandListType = D3D12_COMMAND_LIST_TYPE_COPY;
            Data.Priority = D3D12_COMMAND_QUEUE_PRIORITY_GLOBAL_REALTIME;
            INITIALIZE_FEATURE_SUPPORT_DATA(COMMAND_QUEUE_PRIORITY);
            if (CommandQueuePrioritySupportedRealtime != Data.PriorityForTypeIsSupported)
            {
                PRINT_CHECK_FAILED_MESSAGE(CommandQueuePriority, Data.PriorityForTypeIsSupported, CommandQueuePrioritySupportedRealtime);
            }
        }

        if (CommandQueuePrioritySupportedInvalid)
        {
            std::cout << "Error: Wrong result from Command Queue Priority - Invalid Test Case\n"
                      << "New API returned true on invalid Command List Types\n"
                      << "Check if the list of command list types has updated\n";
            return -1;
        }
    }

    // 21: Options3
    {
        BOOL CopyQueueTimestampQueriesSupported = features.CopyQueueTimestampQueriesSupported();
        BOOL CastingFullyTypedFormatSupported = features.CastingFullyTypedFormatSupported();
        D3D12_COMMAND_LIST_SUPPORT_FLAGS WriteBufferImmediateSupportFlags = features.WriteBufferImmediateSupportFlags();
        D3D12_VIEW_INSTANCING_TIER ViewInstancingTier = features.ViewInstancingTier();
        BOOL BarycentricsSupported = features.BarycentricsSupported();

        D3D12_FEATURE_DATA_D3D12_OPTIONS3 Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(D3D12_OPTIONS3);
        VERIFY_FEATURE_CHECK(CopyQueueTimestampQueriesSupported, false);
        VERIFY_FEATURE_CHECK(CastingFullyTypedFormatSupported, false);
        VERIFY_FEATURE_CHECK(WriteBufferImmediateSupportFlags, D3D12_COMMAND_LIST_SUPPORT_FLAG_NONE);
        VERIFY_FEATURE_CHECK(ViewInstancingTier, D3D12_VIEW_INSTANCING_TIER_NOT_SUPPORTED);
        VERIFY_FEATURE_CHECK(BarycentricsSupported, false);
    }

    // 22: Existing Heaps
    {
        BOOL ExistingHeapsSupported = features.ExistingHeapsSupported();
        
        D3D12_FEATURE_DATA_EXISTING_HEAPS Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(EXISTING_HEAPS);
        VERIFY_RENAMED_FEATURE_CHECK(ExistingHeapsSupported, Supported, false);
    }

    // 23: D3D12 Options4
    {
        BOOL MSAA64KBAlignedTextureSupported = features.MSAA64KBAlignedTextureSupported();
        D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER SharedResourceCompatibilityTier = features.SharedResourceCompatibilityTier();
        BOOL Native16BitShaderOpsSupported = features.Native16BitShaderOpsSupported();

        D3D12_FEATURE_DATA_D3D12_OPTIONS4 Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(D3D12_OPTIONS4);
        VERIFY_FEATURE_CHECK(MSAA64KBAlignedTextureSupported, false);
        VERIFY_FEATURE_CHECK(SharedResourceCompatibilityTier, D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER_0);
        VERIFY_FEATURE_CHECK(Native16BitShaderOpsSupported, false);
    }

    // 24: Serialization
    {
        D3D12_HEAP_SERIALIZATION_TIER HeapSerializationTier = features.HeapSerializationTier();
        D3D12_FEATURE_DATA_SERIALIZATION Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(SERIALIZATION);
        VERIFY_FEATURE_CHECK(HeapSerializationTier, D3D12_HEAP_SERIALIZATION_TIER_0);
    }

    // 25: Cross Node
    {
        BOOL CrossNodeAtomicShaderInstructions = features.CrossNodeAtomicShaderInstructions();
        D3D12_FEATURE_DATA_CROSS_NODE Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(CROSS_NODE);
        VERIFY_RENAMED_FEATURE_CHECK(CrossNodeAtomicShaderInstructions, AtomicShaderInstructions, false);
    }

    // 27: Options5
    {
        BOOL SRVOnlyTiledResourceTier3 = features.SRVOnlyTiledResourceTier3();
        D3D12_RENDER_PASS_TIER RenderPassesTier = features.RenderPassesTier();
        D3D12_RAYTRACING_TIER RaytracingTier = features.RaytracingTier();

        D3D12_FEATURE_DATA_D3D12_OPTIONS5 Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(D3D12_OPTIONS5);
        VERIFY_FEATURE_CHECK(SRVOnlyTiledResourceTier3, false);
        VERIFY_FEATURE_CHECK(RenderPassesTier, D3D12_RENDER_PASS_TIER_0);
        VERIFY_FEATURE_CHECK(RaytracingTier, D3D12_RAYTRACING_TIER_NOT_SUPPORTED);
    }

    // 28: Displayable
    {
        BOOL DisplayableTexture = features.DisplayableTexture();

        D3D12_FEATURE_DATA_DISPLAYABLE Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(DISPLAYABLE);
        VERIFY_FEATURE_CHECK(DisplayableTexture, false);
    }

    // 30: Options6
    {
        BOOL AdditionalShadingRatesSupported = features.AdditionalShadingRatesSupported();
        BOOL PerPrimitiveShadingRateSupportedWithViewportIndexing = features.PerPrimitiveShadingRateSupportedWithViewportIndexing();
        D3D12_VARIABLE_SHADING_RATE_TIER VariableShadingRateTier = features.VariableShadingRateTier();
        BOOL ShadingRateImageTileSize = features.ShadingRateImageTileSize();
        BOOL BackgroundProcessingSupported = features.BackgroundProcessingSupported();

        D3D12_FEATURE_DATA_D3D12_OPTIONS6 Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(D3D12_OPTIONS6);
        VERIFY_FEATURE_CHECK(AdditionalShadingRatesSupported, false);
        VERIFY_FEATURE_CHECK(PerPrimitiveShadingRateSupportedWithViewportIndexing, false);
        VERIFY_FEATURE_CHECK(VariableShadingRateTier, D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED);
        VERIFY_FEATURE_CHECK(ShadingRateImageTileSize, false);
        VERIFY_FEATURE_CHECK(BackgroundProcessingSupported, false);
    }

    // 31: Query Metacommand
    // The function is simply a forwarding call to the old API
    // Takes in a pointer to D3D12_FEATURE_DATA_QUERY_METACOMMAND as input parameter
    // No need to test specifically

    // 32: Options7
    {
        D3D12_MESH_SHADER_TIER MeshShaderTier = features.MeshShaderTier();
        D3D12_SAMPLER_FEEDBACK_TIER SamplerFeedbackTier = features.SamplerFeedbackTier();

        D3D12_FEATURE_DATA_D3D12_OPTIONS7 Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(D3D12_OPTIONS7);
        VERIFY_FEATURE_CHECK(MeshShaderTier, D3D12_MESH_SHADER_TIER_NOT_SUPPORTED);
        VERIFY_FEATURE_CHECK(SamplerFeedbackTier, D3D12_SAMPLER_FEEDBACK_TIER_NOT_SUPPORTED);
    }

    // 33: Session Type Count
    for (UINT NodeId = 0; NodeId < NodeCount; NodeId++)
    {
        UINT ProtectedResourceSessionTypeCount = features.ProtectedResourceSessionTypeCount(NodeId);

        D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_TYPE_COUNT Data;
        Data.NodeIndex = NodeId;
        INITIALIZE_FEATURE_SUPPORT_DATA(PROTECTED_RESOURCE_SESSION_TYPE_COUNT);
        VERIFY_RENAMED_FEATURE_CHECK(ProtectedResourceSessionTypeCount, Count, 0);
    }
    
    // 34: Session Types
    for (UINT NodeId = 0; NodeId < NodeCount; NodeId++)
    {
        std::vector<GUID> ProtectedResourceSessionTypes = features.ProtectedResourceSessionTypes(NodeId);

        D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_TYPE_COUNT CountData;
        CountData.NodeIndex = NodeId;
        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_TYPE_COUNT, &CountData, sizeof(D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_TYPE_COUNT))))
        {
            if (ProtectedResourceSessionTypes.size() != 0)
            {
                std::cout << "Error: Wrong result from Protected Resource Session Types on Node " << NodeId << std::endl
                          << "Type count check failed but types vector is not empty\n";
                return -1;
            }
            continue;
        }
        
        if (CountData.Count != ProtectedResourceSessionTypes.size())
        {
            std::cout << "Error: Wrong result from Protected Resource Session Types on Node " << NodeId << std::endl
                      << "Types vector size does not equal type count\n";
            return -1;
        }

        D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_TYPES Data;
        Data.NodeIndex = NodeId;
        Data.Count = CountData.Count;
        Data.pTypes = new GUID[Data.Count];
        INITIALIZE_FEATURE_SUPPORT_DATA(PROTECTED_RESOURCE_SESSION_TYPES);
        if (SUCCEEDED(InitResult))
        {
            for (unsigned int i = 0; i < Data.Count; i++)
            {
                if (ProtectedResourceSessionTypes[i] != Data.pTypes[i])
                {
                    std::cout << "Error: Result mismatch from Proteted Resource Session Types\n"
                              << "Node Index: " << NodeId << std::endl
                              << "Type Index: " << i << std::endl;
                    return -1;
                }
            }
        }
    }

    // 36: Options8
    {
        BOOL UnalignedBlockTexturesSupported = features.UnalignedBlockTexturesSupported();

        D3D12_FEATURE_DATA_D3D12_OPTIONS8 Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(D3D12_OPTIONS8);
        VERIFY_FEATURE_CHECK(UnalignedBlockTexturesSupported, false);
    }

    // 37: Options9
    {
        BOOL MeshShaderPipelineStatsSupported = features.MeshShaderPipelineStatsSupported();
        BOOL MeshShaderSupportsFullRangeRenderTargetArrayIndex = features.MeshShaderSupportsFullRangeRenderTargetArrayIndex();
        D3D12_WAVE_MMA_TIER WaveMMATier = features.WaveMMATier();

        D3D12_FEATURE_DATA_D3D12_OPTIONS9 Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(D3D12_OPTIONS9);
        VERIFY_FEATURE_CHECK(MeshShaderPipelineStatsSupported, false);
        VERIFY_FEATURE_CHECK(MeshShaderSupportsFullRangeRenderTargetArrayIndex, false);
        VERIFY_FEATURE_CHECK(WaveMMATier, D3D12_WAVE_MMA_TIER_NOT_SUPPORTED);
    }

    // 39: Options10
    {
        BOOL VariableRateShadingSumCombinerSupported = features.VariableRateShadingSumCombinerSupported();
        BOOL MeshShaderPerPrimitiveShadingRateSupported = features.MeshShaderPerPrimitiveShadingRateSupported();

        D3D12_FEATURE_DATA_D3D12_OPTIONS10 Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(D3D12_OPTIONS10);
        VERIFY_FEATURE_CHECK(VariableRateShadingSumCombinerSupported, false);
        VERIFY_FEATURE_CHECK(MeshShaderPerPrimitiveShadingRateSupported, false);
    }

    // 40: Options11
    {
        BOOL AtomicInt64OnDescriptorHeapResourceSupported = features.AtomicInt64OnDescriptorHeapResourceSupported();

        D3D12_FEATURE_DATA_D3D12_OPTIONS11 Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(D3D12_OPTIONS11);
        VERIFY_FEATURE_CHECK(AtomicInt64OnDescriptorHeapResourceSupported, false);
    }

    // 41: Options12
    {
        D3D12_TRI_STATE MSPrimitivesPipelineStatisticIncludesCulledPrimitives = features.MSPrimitivesPipelineStatisticIncludesCulledPrimitives();
        BOOL EnhancedBarriersSupported = features.EnhancedBarriersSupported();

        D3D12_FEATURE_DATA_D3D12_OPTIONS12 Data;
        INITIALIZE_FEATURE_SUPPORT_DATA(D3D12_OPTIONS12);
        VERIFY_FEATURE_CHECK(MSPrimitivesPipelineStatisticIncludesCulledPrimitives, D3D12_TRI_STATE_UNKNOWN);
        VERIFY_FEATURE_CHECK(EnhancedBarriersSupported, false);
    }

    std::cout << "Test completed with no errors." << std::endl;
    return 0;
}
