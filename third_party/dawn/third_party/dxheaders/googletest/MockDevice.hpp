// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#ifndef DIRECTX_HEADERS_MOCK_DEVICE_HPP
#define DIRECTX_HEADERS_MOCK_DEVICE_HPP
#include <unordered_map>

#include <directx/d3d12.h>
#include <directx/dxcore.h>
#include <directx/d3dx12.h>
#include "dxguids/dxguids.h"

class MockDevice : public ID3D12Device
{
public: // Constructors and custom functions
    MockDevice(UINT NodeCount = 1)
    : m_NodeCount(NodeCount)
    , m_TileBasedRenderer(m_NodeCount, false)
    , m_UMA(m_NodeCount, false)
    , m_CacheCoherentUMA(m_NodeCount, false)
    , m_IsolatedMMU(m_NodeCount, false)
    , m_ProtectedResourceSessionSupport(m_NodeCount, D3D12_PROTECTED_RESOURCE_SESSION_SUPPORT_FLAG_NONE)
    , m_HeapSerializationTier(m_NodeCount, D3D12_HEAP_SERIALIZATION_TIER_0)
    , m_ProtectedResourceSessionTypeCount(m_NodeCount, 0)
    , m_ProtectedResourceSessionTypes(m_NodeCount)
    {

    }

    void SetNodeCount(UINT NewCount) 
    {
        m_NodeCount = NewCount;
        m_TileBasedRenderer.resize(NewCount);
        m_UMA.resize(NewCount);
        m_CacheCoherentUMA.resize(NewCount);
        m_IsolatedMMU.resize(NewCount);
        m_ProtectedResourceSessionSupport.resize(NewCount);
        m_HeapSerializationTier.resize(NewCount);
        m_ProtectedResourceSessionTypeCount.resize(NewCount);
        m_ProtectedResourceSessionTypes.resize(NewCount);
    }

public: // ID3D12Device
    UINT STDMETHODCALLTYPE GetNodeCount() override 
    {
        return m_NodeCount;
    }

    HRESULT STDMETHODCALLTYPE CreateCommandQueue(
            _In_  const D3D12_COMMAND_QUEUE_DESC *pDesc,
            REFIID riid,
            _COM_Outptr_  void **ppCommandQueue
    ) override 
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CreateCommandAllocator(
        _In_  D3D12_COMMAND_LIST_TYPE type,
        REFIID riid,
        _COM_Outptr_  void **ppCommandAllocator
    ) override
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CreateGraphicsPipelineState(
         _In_  const D3D12_GRAPHICS_PIPELINE_STATE_DESC *pDesc,
        REFIID riid,
        _COM_Outptr_  void **ppPipelineState
    ) override
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CreateComputePipelineState( 
        _In_  const D3D12_COMPUTE_PIPELINE_STATE_DESC *pDesc,
        REFIID riid,
        _COM_Outptr_  void **ppPipelineState
    ) override
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CreateCommandList( 
        _In_  UINT nodeMask,
        _In_  D3D12_COMMAND_LIST_TYPE type,
        _In_  ID3D12CommandAllocator *pCommandAllocator,
        _In_opt_  ID3D12PipelineState *pInitialState,
        REFIID riid,
        _COM_Outptr_  void **ppCommandList
    ) override 
    {
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE CreateDescriptorHeap( 
        _In_  const D3D12_DESCRIPTOR_HEAP_DESC *pDescriptorHeapDesc,
        REFIID riid,
        _COM_Outptr_  void **ppvHeap
    ) override 
    {
        return S_OK;
    }

    virtual UINT STDMETHODCALLTYPE GetDescriptorHandleIncrementSize( 
        _In_  D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType
    ) override 
    {
        return 0;
    }
        
    virtual HRESULT STDMETHODCALLTYPE CreateRootSignature( 
        _In_  UINT nodeMask,
        _In_reads_(blobLengthInBytes)  const void *pBlobWithRootSignature,
        _In_  SIZE_T blobLengthInBytes,
        REFIID riid,
        _COM_Outptr_  void **ppvRootSignature
    ) override 
    {
        return S_OK;
    }
    
    virtual void STDMETHODCALLTYPE CreateConstantBufferView( 
        _In_opt_  const D3D12_CONSTANT_BUFFER_VIEW_DESC *pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor
    ) override 
    {
        return;
    }
    
    virtual void STDMETHODCALLTYPE CreateShaderResourceView( 
        _In_opt_  ID3D12Resource *pResource,
        _In_opt_  const D3D12_SHADER_RESOURCE_VIEW_DESC *pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor
    ) override 
    {
        return;
    }
    
    virtual void STDMETHODCALLTYPE CreateUnorderedAccessView( 
        _In_opt_  ID3D12Resource *pResource,
        _In_opt_  ID3D12Resource *pCounterResource,
        _In_opt_  const D3D12_UNORDERED_ACCESS_VIEW_DESC *pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor
    ) override 
    {
        return;
    }
    
    virtual void STDMETHODCALLTYPE CreateRenderTargetView( 
        _In_opt_  ID3D12Resource *pResource,
        _In_opt_  const D3D12_RENDER_TARGET_VIEW_DESC *pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor
    ) override 
    {
        return;
    }
    
    virtual void STDMETHODCALLTYPE CreateDepthStencilView( 
        _In_opt_  ID3D12Resource *pResource,
        _In_opt_  const D3D12_DEPTH_STENCIL_VIEW_DESC *pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) override 
    {
        return;
    }
    
    virtual void STDMETHODCALLTYPE CreateSampler( 
        _In_  const D3D12_SAMPLER_DESC *pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) override 
    {
        return;
    }
    
    virtual void STDMETHODCALLTYPE CopyDescriptors( 
        _In_  UINT NumDestDescriptorRanges,
        _In_reads_(NumDestDescriptorRanges)  const D3D12_CPU_DESCRIPTOR_HANDLE *pDestDescriptorRangeStarts,
        _In_reads_opt_(NumDestDescriptorRanges)  const UINT *pDestDescriptorRangeSizes,
        _In_  UINT NumSrcDescriptorRanges,
        _In_reads_(NumSrcDescriptorRanges)  const D3D12_CPU_DESCRIPTOR_HANDLE *pSrcDescriptorRangeStarts,
        _In_reads_opt_(NumSrcDescriptorRanges)  const UINT *pSrcDescriptorRangeSizes,
        _In_  D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType) override 
    {
        return;
    }
    
    virtual void STDMETHODCALLTYPE CopyDescriptorsSimple( 
        _In_  UINT NumDescriptors,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart,
        _In_  D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType) override 
    {
        return;
    }
    
    virtual D3D12_RESOURCE_ALLOCATION_INFO STDMETHODCALLTYPE GetResourceAllocationInfo( 
        _In_  UINT visibleMask,
        _In_  UINT numResourceDescs,
        _In_reads_(numResourceDescs)  const D3D12_RESOURCE_DESC *pResourceDescs) override
    {
        D3D12_RESOURCE_ALLOCATION_INFO mockInfo = {};
        return mockInfo;
    }
    
    virtual D3D12_HEAP_PROPERTIES STDMETHODCALLTYPE GetCustomHeapProperties( 
        _In_  UINT nodeMask,
        D3D12_HEAP_TYPE heapType) override 
    {
        D3D12_HEAP_PROPERTIES mockProps = {};
        return mockProps;
    }
    
    virtual HRESULT STDMETHODCALLTYPE CreateCommittedResource( 
        _In_  const D3D12_HEAP_PROPERTIES *pHeapProperties,
        D3D12_HEAP_FLAGS HeapFlags,
        _In_  const D3D12_RESOURCE_DESC *pDesc,
        D3D12_RESOURCE_STATES InitialResourceState,
        _In_opt_  const D3D12_CLEAR_VALUE *pOptimizedClearValue,
        REFIID riidResource,
        _COM_Outptr_opt_  void **ppvResource) override 
    {
        return S_OK;
    }
    
    virtual HRESULT STDMETHODCALLTYPE CreateHeap( 
        _In_  const D3D12_HEAP_DESC *pDesc,
        REFIID riid,
        _COM_Outptr_opt_  void **ppvHeap) override 
    {
        return S_OK;
    }
    
    virtual HRESULT STDMETHODCALLTYPE CreatePlacedResource( 
        _In_  ID3D12Heap *pHeap,
        UINT64 HeapOffset,
        _In_  const D3D12_RESOURCE_DESC *pDesc,
        D3D12_RESOURCE_STATES InitialState,
        _In_opt_  const D3D12_CLEAR_VALUE *pOptimizedClearValue,
        REFIID riid,
        _COM_Outptr_opt_  void **ppvResource) override 
    {
        return S_OK;
    }
    
    virtual HRESULT STDMETHODCALLTYPE CreateReservedResource( 
        _In_  const D3D12_RESOURCE_DESC *pDesc,
        D3D12_RESOURCE_STATES InitialState,
        _In_opt_  const D3D12_CLEAR_VALUE *pOptimizedClearValue,
        REFIID riid,
        _COM_Outptr_opt_  void **ppvResource) override 
    {
        return S_OK;
    }
    
    virtual HRESULT STDMETHODCALLTYPE CreateSharedHandle( 
        _In_  ID3D12DeviceChild *pObject,
        _In_opt_  const SECURITY_ATTRIBUTES *pAttributes,
        DWORD Access,
        _In_opt_  LPCWSTR Name,
        _Out_  HANDLE *pHandle) override 
    {
        return S_OK;
    }
    
    virtual HRESULT STDMETHODCALLTYPE OpenSharedHandle( 
        _In_  HANDLE NTHandle,
        REFIID riid,
        _COM_Outptr_opt_  void **ppvObj) override 
    {
        return S_OK;
    }
    
    virtual HRESULT STDMETHODCALLTYPE OpenSharedHandleByName( 
        _In_  LPCWSTR Name,
        DWORD Access,
        /* [annotation][out] */ 
        _Out_  HANDLE *pNTHandle) override 
    {
        return S_OK;
    }
    
    virtual HRESULT STDMETHODCALLTYPE MakeResident( 
        UINT NumObjects,
        _In_reads_(NumObjects)  ID3D12Pageable *const *ppObjects) override
    {
        return S_OK;
    }
    
    virtual HRESULT STDMETHODCALLTYPE Evict( 
        UINT NumObjects,
        _In_reads_(NumObjects)  ID3D12Pageable *const *ppObjects) override 
    {
        return S_OK;
    }
    
    virtual HRESULT STDMETHODCALLTYPE CreateFence( 
        UINT64 InitialValue,
        D3D12_FENCE_FLAGS Flags,
        REFIID riid,
        _COM_Outptr_  void **ppFence) override 
    {
        return S_OK;
    }
    
    virtual HRESULT STDMETHODCALLTYPE GetDeviceRemovedReason( void) override 
    {
        return S_OK;
    }
    
    virtual void STDMETHODCALLTYPE GetCopyableFootprints( 
        _In_  const D3D12_RESOURCE_DESC *pResourceDesc,
        _In_range_(0,D3D12_REQ_SUBRESOURCES)  UINT FirstSubresource,
        _In_range_(0,D3D12_REQ_SUBRESOURCES-FirstSubresource)  UINT NumSubresources,
        UINT64 BaseOffset,
        _Out_writes_opt_(NumSubresources)  D3D12_PLACED_SUBRESOURCE_FOOTPRINT *pLayouts,
        _Out_writes_opt_(NumSubresources)  UINT *pNumRows,
        _Out_writes_opt_(NumSubresources)  UINT64 *pRowSizeInBytes,
        _Out_opt_  UINT64 *pTotalBytes) override 
    {
        return;
    }
    
    virtual HRESULT STDMETHODCALLTYPE CreateQueryHeap( 
        _In_  const D3D12_QUERY_HEAP_DESC *pDesc,
        REFIID riid,
        _COM_Outptr_opt_  void **ppvHeap) override 
    {
        return S_OK;    
    }
    
    virtual HRESULT STDMETHODCALLTYPE SetStablePowerState( 
        BOOL Enable) override 
    {
        return S_OK;
    }
    
    virtual HRESULT STDMETHODCALLTYPE CreateCommandSignature( 
        _In_  const D3D12_COMMAND_SIGNATURE_DESC *pDesc,
        _In_opt_  ID3D12RootSignature *pRootSignature,
        REFIID riid,
        _COM_Outptr_opt_  void **ppvCommandSignature) override
    {
        return S_OK;
    }
    
    virtual void STDMETHODCALLTYPE GetResourceTiling( 
        _In_  ID3D12Resource *pTiledResource,
        _Out_opt_  UINT *pNumTilesForEntireResource,
        _Out_opt_  D3D12_PACKED_MIP_INFO *pPackedMipDesc,
        _Out_opt_  D3D12_TILE_SHAPE *pStandardTileShapeForNonPackedMips,
        _Inout_opt_  UINT *pNumSubresourceTilings,
        _In_  UINT FirstSubresourceTilingToGet,
        _Out_writes_(*pNumSubresourceTilings)  D3D12_SUBRESOURCE_TILING *pSubresourceTilingsForNonPackedMips) override 
    {
        return;
    }
    
    virtual LUID STDMETHODCALLTYPE GetAdapterLuid( void) override 
    {
        LUID mockLuid = {};
        return mockLuid;
    }

public: // ID3D12Object
    virtual HRESULT STDMETHODCALLTYPE GetPrivateData( 
        _In_  REFGUID guid,
        _Inout_  UINT *pDataSize,
        _Out_writes_bytes_opt_( *pDataSize )  void *pData) override
    {
        return S_OK;
    }
    
    virtual HRESULT STDMETHODCALLTYPE SetPrivateData( 
        _In_  REFGUID guid,
        _In_  UINT DataSize,
        _In_reads_bytes_opt_( DataSize )  const void *pData) override
    {
        return S_OK;
    }
    
    virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface( 
        _In_  REFGUID guid,
        _In_opt_  const IUnknown *pData) override 
    {
        return S_OK;
    }
    
    virtual HRESULT STDMETHODCALLTYPE SetName( 
        _In_z_  LPCWSTR Name) override 
    {
        return S_OK;
    }

public: // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) override
    {
        *ppvObject = this;
        return S_OK;
    }

    virtual ULONG STDMETHODCALLTYPE AddRef() override 
    {
        // Casual implementation. No actual actions
        return 0;
    }

    virtual ULONG STDMETHODCALLTYPE Release() override 
    {
        return 0;
    }


    // Major function we need to work with
    virtual HRESULT STDMETHODCALLTYPE CheckFeatureSupport( 
        D3D12_FEATURE Feature,
        _Inout_updates_bytes_(FeatureSupportDataSize)  void *pFeatureSupportData,
        UINT FeatureSupportDataSize
    ) override 
    {
        switch( Feature )
        {
            case D3D12_FEATURE_D3D12_OPTIONS:
            {
                D3D12_FEATURE_DATA_D3D12_OPTIONS* pD3D12Options = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS*>(pFeatureSupportData);
                if (FeatureSupportDataSize != sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS))
                {
                    return E_INVALIDARG;
                }
                pD3D12Options->DoublePrecisionFloatShaderOps       = m_DoublePrecisionFloatShaderOps;
                pD3D12Options->OutputMergerLogicOp                 = m_OutputMergerLogicOp;
                pD3D12Options->MinPrecisionSupport                 = m_ShaderMinPrecisionSupport10Bit | m_ShaderMinPrecisionSupport16Bit;
                pD3D12Options->TiledResourcesTier                  = m_TiledResourcesTier;
                pD3D12Options->ResourceBindingTier                 = m_ResourceBindingTier;
                pD3D12Options->PSSpecifiedStencilRefSupported      = (m_FeatureLevel >= D3D_FEATURE_LEVEL_11_1) && m_PSSpecifiedStencilRefSupported;
                pD3D12Options->TypedUAVLoadAdditionalFormats       = (m_FeatureLevel >= D3D_FEATURE_LEVEL_11_0) && m_TypedUAVLoadAdditionalFormats;
                pD3D12Options->ROVsSupported                       = (m_FeatureLevel >= D3D_FEATURE_LEVEL_11_0) && m_ROVsSupported;
                pD3D12Options->ConservativeRasterizationTier       = (m_FeatureLevel >= D3D_FEATURE_LEVEL_11_1) ? m_ConservativeRasterizationTier : D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED;
                pD3D12Options->MaxGPUVirtualAddressBitsPerResource = m_MaxGPUVirtualAddressBitsPerResource;
                pD3D12Options->StandardSwizzle64KBSupported        = m_StandardSwizzle64KBSupported;
    #ifdef DX_ASTC_PROTOTYPE_ENABLED
                pD3D12Options->ASTCProfile                         = ASTCProfile();
    #endif
                pD3D12Options->CrossNodeSharingTier                = m_CrossNodeSharingTier;
                pD3D12Options->CrossAdapterRowMajorTextureSupported = m_CrossAdapterRowMajorTextureSupported;
                pD3D12Options->VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation = m_VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation;
                pD3D12Options->ResourceHeapTier                    = m_ResourceHeapTier;
            } return S_OK;
        
        case D3D12_FEATURE_D3D12_OPTIONS1:
        {
            if (!m_Options1Available) 
            {
                return E_INVALIDARG;
            }
            D3D12_FEATURE_DATA_D3D12_OPTIONS1* pD3D12Options1 = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS1*>(pFeatureSupportData);
            if (FeatureSupportDataSize != sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS1))
            {
                return E_INVALIDARG;
            }
            pD3D12Options1->WaveOps = m_WaveOpsSupported;
            pD3D12Options1->WaveLaneCountMin = m_WaveLaneCountMin;
            pD3D12Options1->WaveLaneCountMax = m_WaveLaneCountMax;
            pD3D12Options1->TotalLaneCount = m_TotalLaneCount;
            pD3D12Options1->ExpandedComputeResourceStates = m_ExpandedComputeResourceStates;
            pD3D12Options1->Int64ShaderOps = m_Int64ShaderOpsSupported;
        } return S_OK;
        
        
        case D3D12_FEATURE_D3D12_OPTIONS2:
        {
            if (!m_Options2Available) 
            {
                return E_INVALIDARG;
            }

            D3D12_FEATURE_DATA_D3D12_OPTIONS2* pD3D12Options2 = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS2*>(pFeatureSupportData);
            if (FeatureSupportDataSize != sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS2))
            {
                return E_INVALIDARG;
            }
            pD3D12Options2->DepthBoundsTestSupported = m_DepthBoundsTestSupport;
            pD3D12Options2->ProgrammableSamplePositionsTier = m_ProgrammableSamplePositionsTier;
        } return S_OK;

        case D3D12_FEATURE_ROOT_SIGNATURE:
        {
            if (!m_RootSignatureAvailable)
            {
                return E_INVALIDARG;
            }
            D3D12_FEATURE_DATA_ROOT_SIGNATURE* pRootSig = static_cast<D3D12_FEATURE_DATA_ROOT_SIGNATURE*>(pFeatureSupportData);
            if (FeatureSupportDataSize != sizeof(D3D12_FEATURE_DATA_ROOT_SIGNATURE))
            {
                return E_INVALIDARG;
            }
            switch (pRootSig->HighestVersion)
            {
            case D3D_ROOT_SIGNATURE_VERSION_1_0:
            case D3D_ROOT_SIGNATURE_VERSION_1_1:
                break;
            default:
                return E_INVALIDARG;
            }
            pRootSig->HighestVersion = min(pRootSig->HighestVersion, m_RootSignatureHighestVersion); 
        } return S_OK;
        

        case D3D12_FEATURE_ARCHITECTURE:
            {
                D3D12_FEATURE_DATA_ARCHITECTURE* pFData =
                    static_cast< D3D12_FEATURE_DATA_ARCHITECTURE* >( pFeatureSupportData );
                if (FeatureSupportDataSize != sizeof( *pFData ))
                {
                    return E_INVALIDARG;
                }

                // Testing only
                // If Architecture1 is available, use data from architecture1
                if (m_Architecture1Available) 
                {
                    D3D12_FEATURE_DATA_ARCHITECTURE1 CurFData;
                    CurFData.NodeIndex = pFData->NodeIndex;
                    
                    HRESULT hr;
                    if (FAILED( hr = CheckFeatureSupport( D3D12_FEATURE_ARCHITECTURE1, &CurFData, sizeof( CurFData ) ) ))
                    {
                        return hr;
                    }

                    pFData->TileBasedRenderer = CurFData.TileBasedRenderer;
                    pFData->UMA = CurFData.UMA;
                    pFData->CacheCoherentUMA = CurFData.CacheCoherentUMA;
                } 
                else // Otherwise, load the data directly
                {
                    // The original procedure will generate and return an E_INVALIDARG error if the NodeIndex is out of scope
                    // Mocking the behavior here by returning the rror
                    if (!(pFData->NodeIndex < m_NodeCount)) 
                    {
                        return E_INVALIDARG;
                    }
                    pFData->TileBasedRenderer = m_TileBasedRenderer[pFData->NodeIndex];
                    pFData->UMA = m_UMA[pFData->NodeIndex];
                    pFData->CacheCoherentUMA = m_CacheCoherentUMA[pFData->NodeIndex];
                    // Note that Architecture doesn't have the IsolatedMMU field.
                }
            } return S_OK;
        case D3D12_FEATURE_ARCHITECTURE1:
            {
                // Mocking the case where ARCHITECTURE1 is not supported
                if (!m_Architecture1Available || !m_ArchitectureSucceed) {
                    return E_INVALIDARG;
                }

                D3D12_FEATURE_DATA_ARCHITECTURE1* pFData =
                    static_cast< D3D12_FEATURE_DATA_ARCHITECTURE1* >( pFeatureSupportData );
                if (FeatureSupportDataSize != sizeof( *pFData ))
                {
                    return E_INVALIDARG;
                }

                // The original procedure will generate and return an E_INVALIDARG error if the NodeIndex is out of scope
                // Mocking the behavior here by returning the rror
                if (!(pFData->NodeIndex < m_NodeCount)) 
                {
                    return E_INVALIDARG;
                }

                UINT localIndex = pFData->NodeIndex;
                pFData->TileBasedRenderer = m_TileBasedRenderer[localIndex];
                pFData->UMA = m_UMA[localIndex];
                pFData->CacheCoherentUMA = m_CacheCoherentUMA[localIndex];
                pFData->IsolatedMMU = m_IsolatedMMU[localIndex];

            } return S_OK;

        case D3D12_FEATURE_FEATURE_LEVELS:
            {
                D3D12_FEATURE_DATA_FEATURE_LEVELS* pFData =
                    static_cast< D3D12_FEATURE_DATA_FEATURE_LEVELS* >( pFeatureSupportData );
                if (FeatureSupportDataSize != sizeof( *pFData ))
                {
                    return E_INVALIDARG;
                }

                if (pFData->NumFeatureLevels == 0 || pFData->pFeatureLevelsRequested == nullptr)
                {
                    return E_INVALIDARG;
                }

                pFData->MaxSupportedFeatureLevel = D3D_FEATURE_LEVEL(0);
                for (UINT i = 0; i < pFData->NumFeatureLevels; ++i)
                {
                    if (pFData->pFeatureLevelsRequested[i] <= m_FeatureLevel &&
                        pFData->pFeatureLevelsRequested[i] > pFData->MaxSupportedFeatureLevel)
                    {
                        pFData->MaxSupportedFeatureLevel = pFData->pFeatureLevelsRequested[i];
                    }
                }
                return pFData->MaxSupportedFeatureLevel == D3D_FEATURE_LEVEL(0) ?
                    DXGI_ERROR_UNSUPPORTED : S_OK;
            }

        case D3D12_FEATURE_FORMAT_SUPPORT:
            {
                D3D12_FEATURE_DATA_FORMAT_SUPPORT* pFData = 
                    static_cast< D3D12_FEATURE_DATA_FORMAT_SUPPORT* >( pFeatureSupportData );
                if (FeatureSupportDataSize != sizeof( *pFData ))
                {
                    return E_INVALIDARG;
                }
                m_FormatReceived = pFData->Format;
                pFData->Support1 = m_FormatSupport1;
                pFData->Support2 = m_FormatSupport2;
                // Based on the original implementation, if there's no support for the format, return an E_FAIL
                if (m_FormatSupport1 == D3D12_FORMAT_SUPPORT1_NONE && m_FormatSupport2 == D3D12_FORMAT_SUPPORT2_NONE) 
                {
                    return E_FAIL;
                }
            } return S_OK;

        case D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS:
            {
                D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS* pFData =
                    static_cast< D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS* >( pFeatureSupportData );
                if (FeatureSupportDataSize != sizeof( *pFData ))
                {
                    return E_INVALIDARG;
                }

                m_FormatReceived = pFData->Format;
                m_SampleCountReceived = pFData->SampleCount;
                m_MultisampleQualityLevelFlagsReceived = pFData->Flags;

                // The original check implementation may return E_FAIL
                // Valid results are non-negative values including 0, smaller than D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT
                if (!m_MultisampleQualityLevelsSucceed) 
                {
                    // NumQualityLevels will be set to 0 should the check fails
                    pFData->NumQualityLevels = 0;
                    return E_FAIL;
                }
                
                pFData->NumQualityLevels = m_NumQualityLevels;
            } return S_OK;
        case D3D12_FEATURE_FORMAT_INFO:
            {
                D3D12_FEATURE_DATA_FORMAT_INFO* pFData = 
                    static_cast< D3D12_FEATURE_DATA_FORMAT_INFO* > ( pFeatureSupportData );
                if (FeatureSupportDataSize != sizeof( *pFData ))
                {
                    return E_INVALIDARG;
                }

                m_FormatReceived = pFData->Format;
                
                // If the format is not supported, an E_INVALIDARG will be returned
                if (!m_DXGIFormatSupported) 
                {
                    return E_INVALIDARG;
                }

                pFData->PlaneCount = m_PlaneCount;

            } return S_OK;
        case D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT:
            {
                D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT* pFData =
                    static_cast< D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT* >( pFeatureSupportData );
                if (FeatureSupportDataSize != sizeof( *pFData ))
                {
                    return E_INVALIDARG;
                }
                pFData->MaxGPUVirtualAddressBitsPerProcess = m_MaxGPUVirtualAddressBitsPerProcess;
                pFData->MaxGPUVirtualAddressBitsPerResource = m_MaxGPUVirtualAddressBitsPerResource;

            } return S_OK;
        case D3D12_FEATURE_SHADER_MODEL:
            {
                D3D12_FEATURE_DATA_SHADER_MODEL* pSM =
                    static_cast<D3D12_FEATURE_DATA_SHADER_MODEL*>(pFeatureSupportData);
                if (FeatureSupportDataSize != sizeof(*pSM))
                {
                    return E_INVALIDARG;
                }
                switch (pSM->HighestShaderModel)
                {
                case D3D_SHADER_MODEL_5_1:
                case D3D_SHADER_MODEL_6_0:
                case D3D_SHADER_MODEL_6_1:
                case D3D_SHADER_MODEL_6_2:
                case D3D_SHADER_MODEL_6_3:
                case D3D_SHADER_MODEL_6_4:
                case D3D_SHADER_MODEL_6_5:
                case D3D_SHADER_MODEL_6_6:
                case D3D_SHADER_MODEL_6_7:
                    break;
                default:
                    return E_INVALIDARG;
                }
                pSM->HighestShaderModel = min(pSM->HighestShaderModel,m_HighestSupportedShaderModel);
            } return S_OK;
        case D3D12_FEATURE_SHADER_CACHE:
            {
                if (!m_ShaderCacheAvailable) 
                {
                    return E_INVALIDARG;
                }
                D3D12_FEATURE_DATA_SHADER_CACHE* pFlags =
                    static_cast<D3D12_FEATURE_DATA_SHADER_CACHE*>(pFeatureSupportData);
                if (FeatureSupportDataSize != sizeof(*pFlags))
                {
                    return E_INVALIDARG;
                }
               pFlags->SupportFlags = m_ShaderCacheSupportFlags;
            } return S_OK;
        case D3D12_FEATURE_COMMAND_QUEUE_PRIORITY:
            {
                if (!m_CommandQueuePriorityAvailable)
                {
                    return E_INVALIDARG;
                }
                D3D12_FEATURE_DATA_COMMAND_QUEUE_PRIORITY* pFlags =
                    static_cast<D3D12_FEATURE_DATA_COMMAND_QUEUE_PRIORITY*>(pFeatureSupportData);
                if (FeatureSupportDataSize != sizeof(*pFlags))
                {
                    return E_INVALIDARG;
                }

                if (pFlags->CommandListType >= D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE+1)
                {
                    return E_INVALIDARG;
                }

                if (pFlags->Priority == D3D12_COMMAND_QUEUE_PRIORITY_NORMAL || pFlags->Priority == D3D12_COMMAND_QUEUE_PRIORITY_HIGH)
                {
                    pFlags->PriorityForTypeIsSupported = TRUE;
                }
                else if (pFlags->Priority == D3D12_COMMAND_QUEUE_PRIORITY_GLOBAL_REALTIME)
                {
                    pFlags->PriorityForTypeIsSupported = m_GlobalRealtimeCommandQueueSupport; // Simplified
                }
                else
                {
                    return E_INVALIDARG;
                }

            } return S_OK;
        
        case D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_SUPPORT:
            {
                if (!m_ProtectedResourceSessionAvailable) 
                {
                    return E_INVALIDARG;
                }
                D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_SUPPORT* pProtectedResourceSessionSupport =
                    static_cast<D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_SUPPORT*>(pFeatureSupportData);
                if (   FeatureSupportDataSize != sizeof(*pProtectedResourceSessionSupport)
                    || pProtectedResourceSessionSupport->NodeIndex >= GetNodeCount())
                {
                    return E_INVALIDARG;
                }

                pProtectedResourceSessionSupport->Support = m_ContentProtectionSupported ?
                    m_ProtectedResourceSessionSupport[pProtectedResourceSessionSupport->NodeIndex]
                    : D3D12_PROTECTED_RESOURCE_SESSION_SUPPORT_FLAG_NONE;

            } return S_OK;
        
        case D3D12_FEATURE_D3D12_OPTIONS3:
        {
            if (!m_Options3Available) 
            {
                return E_INVALIDARG;
            }
            D3D12_FEATURE_DATA_D3D12_OPTIONS3* pD3D12Options3 = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS3*>(pFeatureSupportData);
            if (FeatureSupportDataSize != sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS3))
            {
                return E_INVALIDARG;
            }
            pD3D12Options3->CopyQueueTimestampQueriesSupported = m_CopyQueueTimestampQueriesSupported;
            pD3D12Options3->CastingFullyTypedFormatSupported = m_CastingFullyTypedFormatsSupported;
            pD3D12Options3->WriteBufferImmediateSupportFlags = m_GetCachedWriteBufferImmediateSupportFlags;
            pD3D12Options3->ViewInstancingTier = m_ViewInstancingTier;
            pD3D12Options3->BarycentricsSupported = m_BarycentricsSupported;

        } return S_OK;
        case D3D12_FEATURE_EXISTING_HEAPS:
            {
                if (!m_ExistingHeapsAvailable)
                {
                    return E_INVALIDARG;
                }
                auto* pSupport = static_cast<D3D12_FEATURE_DATA_EXISTING_HEAPS*>(pFeatureSupportData);
                if (FeatureSupportDataSize != sizeof(*pSupport))
                {
                    return E_INVALIDARG;
                }
                pSupport->Supported = m_ExistingHeapCaps;
            } return S_OK;
        case D3D12_FEATURE_D3D12_OPTIONS4:
            {
                if (!m_Options4Available)
                {
                    return E_INVALIDARG;
                }
                auto* pD3D12Options4 = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS4*>(pFeatureSupportData);
                if (FeatureSupportDataSize != sizeof(*pD3D12Options4))
                {
                    return E_INVALIDARG;
                }

                // Reserved Buffer Placement was cut, except for 64KB Aligned MSAA Textures
                pD3D12Options4->MSAA64KBAlignedTextureSupported = m_MSAA64KBAlignedTextureSupported;
                pD3D12Options4->SharedResourceCompatibilityTier = m_SharedResourceCompatibilityTier; // Simplified
                pD3D12Options4->Native16BitShaderOpsSupported = m_Native16BitShaderOpsSupported;
            } return S_OK;
        case D3D12_FEATURE_SERIALIZATION:
            {
                if (!m_SerializationAvailable)
                {
                    return E_INVALIDARG;
                }
                auto* pSerialization = static_cast<D3D12_FEATURE_DATA_SERIALIZATION*>(pFeatureSupportData);
                if (FeatureSupportDataSize != sizeof(*pSerialization))
                {
                    return E_INVALIDARG;
                }

                const UINT NodeIndex = pSerialization->NodeIndex;
                if (NodeIndex >= m_NodeCount) 
                {
                    return E_INVALIDARG;
                }
                pSerialization->HeapSerializationTier = m_HeapSerializationTier[NodeIndex];
            } return S_OK;
        case D3D12_FEATURE_CROSS_NODE:
            {
                if (!m_CrossNodeAvailable)
                {
                    return E_INVALIDARG;
                }
                auto* pCrossNode = static_cast<D3D12_FEATURE_DATA_CROSS_NODE*>(pFeatureSupportData);
                if (FeatureSupportDataSize != sizeof(*pCrossNode))
                {
                    return E_INVALIDARG;
                }

                pCrossNode->SharingTier = m_CrossNodeSharingTier;
                pCrossNode->AtomicShaderInstructions = m_AtomicShaderInstructions;
            } return S_OK;
        case D3D12_FEATURE_D3D12_OPTIONS5:
            {
                if (!m_Options5Available)
                {
                    return E_INVALIDARG;
                }
                auto* pD3D12Options5 = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS5*>(pFeatureSupportData);
                if (FeatureSupportDataSize != sizeof(*pD3D12Options5))
                {
                    return E_INVALIDARG;
                }

                pD3D12Options5->RaytracingTier = m_RaytracingTier;
                pD3D12Options5->RenderPassesTier = m_RenderPassesTier;
                pD3D12Options5->SRVOnlyTiledResourceTier3 = m_SRVOnlyTiledResourceTier3;
            } return S_OK;
        case D3D12_FEATURE_DISPLAYABLE:
            {
                if (!m_DisplayableAvailable) 
                {
                    return E_INVALIDARG;
                }
                auto* pD3D12Displayable = static_cast<D3D12_FEATURE_DATA_DISPLAYABLE*>(pFeatureSupportData);
                if (FeatureSupportDataSize != sizeof(*pD3D12Displayable)) // Feature_D3D1XDisplayable
                {
                    return E_INVALIDARG;
                }

                pD3D12Displayable->DisplayableTexture = m_DisplayableTexture;
                pD3D12Displayable->SharedResourceCompatibilityTier = m_SharedResourceCompatibilityTier;
            } return S_OK;
        
        case D3D12_FEATURE_D3D12_OPTIONS6:
        {
            if (!m_Options6Available) 
            {
                return E_INVALIDARG;
            }

            auto* pD3D12Options6 = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS6*>(pFeatureSupportData);
            if (FeatureSupportDataSize != sizeof(*pD3D12Options6))
            {
                return E_INVALIDARG;
            }
            pD3D12Options6->AdditionalShadingRatesSupported = m_AdditionalShadingRatesSupported;
            pD3D12Options6->BackgroundProcessingSupported = m_BackgroundProcessingSupported;
            pD3D12Options6->PerPrimitiveShadingRateSupportedWithViewportIndexing = m_PerPrimitiveShadingRateSupportedWithViewportIndexing;
            pD3D12Options6->ShadingRateImageTileSize = m_ShadingRateImageTileSize;
            pD3D12Options6->VariableShadingRateTier = m_VariableShadingRateTier;
        } return S_OK;
        case D3D12_FEATURE_QUERY_META_COMMAND:
        {
            if (m_QueryMetaCommandAvailable)
            {
                if (FeatureSupportDataSize != sizeof(D3D12_FEATURE_DATA_QUERY_META_COMMAND))
                {
                    return E_INVALIDARG;
                }
                
                // Only checks inputs and outputs
                auto* pQueryData = static_cast<D3D12_FEATURE_DATA_QUERY_META_COMMAND*>(pFeatureSupportData);
                m_CommandID = pQueryData->CommandId;
                m_pQueryInputData = pQueryData->pQueryInputData;
                m_NodeMask = pQueryData->NodeMask;
                m_QueryInputDataSizeInBytes = pQueryData->QueryInputDataSizeInBytes;
                
                pQueryData->QueryOutputDataSizeInBytes = m_QueryOutputDataSizeInBytes;
                pQueryData->pQueryOutputData = m_pQueryOutputData;
            }
            else
            {
                return E_INVALIDARG;
            }
        } return S_OK;
        case D3D12_FEATURE_D3D12_OPTIONS7:
        {
            if (!m_Options7Available)
            {
                return E_INVALIDARG;
            }
            auto* pD3D12Options7 = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS7*>(pFeatureSupportData);
            if (FeatureSupportDataSize != sizeof(*pD3D12Options7))
            {
                return E_INVALIDARG;
            }

            pD3D12Options7->MeshShaderTier = m_MeshShaderTier;
            pD3D12Options7->SamplerFeedbackTier = m_SamplerFeedbackTier;
        } return S_OK;
        case D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_TYPE_COUNT:
            {
                if (!m_ProtectedResourceSessionTypeCountAvailable)
                {
                    return E_INVALIDARG;
                }
                auto* pProtectedResourceSessionTypesCount =
                    static_cast<D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_TYPE_COUNT*>(pFeatureSupportData);
                if (   FeatureSupportDataSize != sizeof(*pProtectedResourceSessionTypesCount)
                    || pProtectedResourceSessionTypesCount->NodeIndex >= GetNodeCount())
                {
                    return E_INVALIDARG;
                }

                pProtectedResourceSessionTypesCount->Count = m_ProtectedResourceSessionTypeCount[pProtectedResourceSessionTypesCount->NodeIndex];
            } return S_OK;
        case D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_TYPES:
            {
                if (!m_ProtectedResourceSessionTypesAvailable)
                {
                    return E_INVALIDARG;
                }
                auto* pProtectedResourceSessionTypes =
                    static_cast<D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_TYPES*>(pFeatureSupportData);
                if (   FeatureSupportDataSize != sizeof(*pProtectedResourceSessionTypes)
                    || pProtectedResourceSessionTypes->NodeIndex >= GetNodeCount())
                {
                    return E_INVALIDARG;
                }
                UINT ExpectedCount = m_ProtectedResourceSessionTypeCount[pProtectedResourceSessionTypes->NodeIndex];
                if (pProtectedResourceSessionTypes->Count != ExpectedCount)
                {
                    return E_INVALIDARG;
                }

                if (ExpectedCount > 0)
                {
                    memcpy(pProtectedResourceSessionTypes->pTypes, m_ProtectedResourceSessionTypes[pProtectedResourceSessionTypes->NodeIndex].data(), ExpectedCount * sizeof(*pProtectedResourceSessionTypes->pTypes));
                }

            } return S_OK;
        
        case D3D12_FEATURE_D3D12_OPTIONS8:
        {
            if (!m_Options8Available)
            {
                return E_INVALIDARG;
            }
            D3D12_FEATURE_DATA_D3D12_OPTIONS8 *pD3D12Options8 = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS8*>(pFeatureSupportData);
            if (FeatureSupportDataSize != sizeof(*pD3D12Options8))
            {
                return E_INVALIDARG;
            }

            pD3D12Options8->UnalignedBlockTexturesSupported = m_UnalignedBlockTexturesSupported;
        } return S_OK;
        case D3D12_FEATURE_D3D12_OPTIONS9:
        {
            if (!m_Options9Available)
            {
                return E_INVALIDARG;
            }
            D3D12_FEATURE_DATA_D3D12_OPTIONS9 *pD3D12Options9 = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS9*>(pFeatureSupportData);
            if (FeatureSupportDataSize != sizeof(*pD3D12Options9))
            {
                return E_INVALIDARG;
            }

            pD3D12Options9->AtomicInt64OnGroupSharedSupported = m_AtomicInt64OnGroupSharedSupported;
            pD3D12Options9->AtomicInt64OnTypedResourceSupported = m_AtomicInt64OnTypedResourceSupported;
            pD3D12Options9->DerivativesInMeshAndAmplificationShadersSupported = m_DerivativesInMeshAndAmplificationShadersSupported;
            pD3D12Options9->MeshShaderPipelineStatsSupported = m_MeshShaderPipelineStatsSupported;
            pD3D12Options9->MeshShaderSupportsFullRangeRenderTargetArrayIndex = m_MeshShaderSupportsFullRangeRenderTargetArrayIndex;
            pD3D12Options9->WaveMMATier = m_WaveMMATier;
        } return S_OK;
        case D3D12_FEATURE_D3D12_OPTIONS10:
        {
            if (!m_Options10Available)
            {
                return E_INVALIDARG;
            }
            D3D12_FEATURE_DATA_D3D12_OPTIONS10* pD3D12Options10 = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS10*>(pFeatureSupportData);
            if (FeatureSupportDataSize != sizeof(*pD3D12Options10))
            {
                return E_INVALIDARG;
            }

            pD3D12Options10->MeshShaderPerPrimitiveShadingRateSupported = m_MeshShaderPerPrimitiveShadingRateSupported;
            pD3D12Options10->VariableRateShadingSumCombinerSupported = m_VariableRateShadingSumCombinerSupported;
        } return S_OK;
        case D3D12_FEATURE_D3D12_OPTIONS11:
        {
            if (!m_Options11Available)
            {
                return E_INVALIDARG;
            }
            D3D12_FEATURE_DATA_D3D12_OPTIONS11* pD3D12Options11 = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS11*>(pFeatureSupportData);
            if (FeatureSupportDataSize != sizeof(*pD3D12Options11))
            {
                return E_INVALIDARG;
            }

            pD3D12Options11->AtomicInt64OnDescriptorHeapResourceSupported = m_AtomicInt64OnDescriptorHeapResourceSupported;
        } return S_OK;
        case D3D12_FEATURE_D3D12_OPTIONS12:
        {
            if (!m_Options12Available)
            {
                return E_INVALIDARG;
            }
			D3D12_FEATURE_DATA_D3D12_OPTIONS12* pD3D12Options12 = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS12*>(pFeatureSupportData);
			if (FeatureSupportDataSize != sizeof(*pD3D12Options12))
			{
				return E_INVALIDARG;
			}

            pD3D12Options12->MSPrimitivesPipelineStatisticIncludesCulledPrimitives = m_MSPrimitivesPipelineStatisticIncludesCulledPrimitives;
            pD3D12Options12->EnhancedBarriersSupported = m_EnhancedBarriersSupported;
        } return S_OK;

        default:
            return E_INVALIDARG;
        }
    }

public: // For simplicity, allow tests to set the internal state values for this mock class
    // General
    UINT m_NodeCount; // Simulated number of computing nodes

    // 0: Options
    bool m_D3D12OptionsAvailable = true; 
    BOOL m_DoublePrecisionFloatShaderOps = false;
    BOOL m_OutputMergerLogicOp = false;
    D3D12_SHADER_MIN_PRECISION_SUPPORT m_ShaderMinPrecisionSupport10Bit = D3D12_SHADER_MIN_PRECISION_SUPPORT_NONE;
    D3D12_SHADER_MIN_PRECISION_SUPPORT m_ShaderMinPrecisionSupport16Bit = D3D12_SHADER_MIN_PRECISION_SUPPORT_NONE;
    D3D12_TILED_RESOURCES_TIER m_TiledResourcesTier = D3D12_TILED_RESOURCES_TIER_NOT_SUPPORTED;
    D3D12_RESOURCE_BINDING_TIER m_ResourceBindingTier = (D3D12_RESOURCE_BINDING_TIER)0;
    BOOL m_PSSpecifiedStencilRefSupported = false;
    BOOL m_TypedUAVLoadAdditionalFormats = false;
    BOOL m_ROVsSupported = false;
    D3D12_CONSERVATIVE_RASTERIZATION_TIER m_ConservativeRasterizationTier = D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED;
    UINT m_MaxGPUVirtualAddressBitsPerResource = 0;
    BOOL m_StandardSwizzle64KBSupported = false;
    D3D12_CROSS_NODE_SHARING_TIER m_CrossNodeSharingTier = D3D12_CROSS_NODE_SHARING_TIER_NOT_SUPPORTED;
    BOOL m_CrossAdapterRowMajorTextureSupported = false;
    BOOL m_VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation = false;
    D3D12_RESOURCE_HEAP_TIER m_ResourceHeapTier = (D3D12_RESOURCE_HEAP_TIER)0;

    // 1: Architecture & 16: Architecture1
    bool m_ArchitectureSucceed = true;
    bool m_Architecture1Available = false; // Mock the case where Architecture1 is not supported
    std::vector<BOOL> m_TileBasedRenderer;
    std::vector<BOOL> m_UMA;
    std::vector<BOOL> m_CacheCoherentUMA;
    std::vector<BOOL> m_IsolatedMMU;

    // 2: Feature Levels
    bool m_FeatureLevelsAvailable = true;
    D3D_FEATURE_LEVEL m_FeatureLevel = D3D_FEATURE_LEVEL_12_0; // Set higher to allow other tests to pass correctly

    // 3: Feature Format Support
    // Forwarding call only. Make sure that the input parameters are correctly forwarded
    bool m_FormatSupportAvailable = true;
    DXGI_FORMAT m_FormatReceived;
    D3D12_FORMAT_SUPPORT1 m_FormatSupport1 = D3D12_FORMAT_SUPPORT1_NONE;
    D3D12_FORMAT_SUPPORT2 m_FormatSupport2 = D3D12_FORMAT_SUPPORT2_NONE;

    // 4: Multisample Quality Levels
    bool m_MultisampleQualityLevelsSucceed = true;
    UINT m_SampleCountReceived;
    D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS m_MultisampleQualityLevelFlagsReceived;
    UINT m_NumQualityLevels = 0;

    // 5: Format Info
    bool m_DXGIFormatSupported = true;
    UINT m_PlaneCount = 0;

    // 6: GPU Virtual Address Support
    bool m_GPUVASupportAvailable = true;
    UINT m_MaxGPUVirtualAddressBitsPerProcess = 0;

    // 7: Shader Model
    D3D_SHADER_MODEL m_HighestSupportedShaderModel = D3D_SHADER_MODEL_5_1;

    // 8: Options1
    bool m_Options1Available = true;
    bool m_WaveOpsSupported = false;
    UINT m_WaveLaneCountMin = 0;
    UINT m_WaveLaneCountMax = 0;
    UINT m_TotalLaneCount = 0;
    bool m_ExpandedComputeResourceStates = false;
    bool m_Int64ShaderOpsSupported = false;

    // 10: Protected Resource Session Support
    bool m_ProtectedResourceSessionAvailable = true;
    bool m_ContentProtectionSupported = true;
    std::vector<D3D12_PROTECTED_RESOURCE_SESSION_SUPPORT_FLAGS> m_ProtectedResourceSessionSupport;

    // 12: Root Signature
    bool m_RootSignatureAvailable = true;
    D3D_ROOT_SIGNATURE_VERSION m_RootSignatureHighestVersion = (D3D_ROOT_SIGNATURE_VERSION)0;

    // 18: D3D12 Options2
    bool m_Options2Available = true;
    bool m_DepthBoundsTestSupport = false;
    D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER m_ProgrammableSamplePositionsTier = D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_NOT_SUPPORTED;

    // 19: Shader Cache
    bool m_ShaderCacheAvailable = true;
    D3D12_SHADER_CACHE_SUPPORT_FLAGS m_ShaderCacheSupportFlags = D3D12_SHADER_CACHE_SUPPORT_NONE; // Lazy implementation

    // 20: Command Queue Priority
    bool m_CommandQueuePriorityAvailable = true;
    bool m_GlobalRealtimeCommandQueueSupport = false;

    // 21: Options3
    bool m_Options3Available = true;
    bool m_CopyQueueTimestampQueriesSupported = false;
    bool m_CastingFullyTypedFormatsSupported = false;
    D3D12_COMMAND_LIST_SUPPORT_FLAGS m_GetCachedWriteBufferImmediateSupportFlags = D3D12_COMMAND_LIST_SUPPORT_FLAG_NONE;
    D3D12_VIEW_INSTANCING_TIER m_ViewInstancingTier = D3D12_VIEW_INSTANCING_TIER_NOT_SUPPORTED;
    bool m_BarycentricsSupported = false;

    // 22: Existing Heaps
    bool m_ExistingHeapsAvailable = true;
    bool m_ExistingHeapCaps = false;

    // 23: Options4
    bool m_Options4Available = true;
    bool m_MSAA64KBAlignedTextureSupported = false;
    D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER m_SharedResourceCompatibilityTier = D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER_0;
    bool m_Native16BitShaderOpsSupported = false;

    // 24: Serialization
    bool m_SerializationAvailable = true;
    std::vector<D3D12_HEAP_SERIALIZATION_TIER> m_HeapSerializationTier;

    // 25: Cross Node
    bool m_CrossNodeAvailable = true;
    bool m_AtomicShaderInstructions = false;

    // 27: Options5
    bool m_Options5Available = true;
    bool m_SRVOnlyTiledResourceTier3 = false;
    D3D12_RENDER_PASS_TIER m_RenderPassesTier = D3D12_RENDER_PASS_TIER_0;
    D3D12_RAYTRACING_TIER m_RaytracingTier = D3D12_RAYTRACING_TIER_NOT_SUPPORTED;

    // 28: Displayable
    bool m_DisplayableAvailable = true;
    bool m_DisplayableTexture = false;
    // SharedResourceCompatibilityTier is located in Options4

    // 30: Options6
    bool m_Options6Available = true;
    bool m_AdditionalShadingRatesSupported = false;
    bool m_PerPrimitiveShadingRateSupportedWithViewportIndexing = false;
    D3D12_VARIABLE_SHADING_RATE_TIER m_VariableShadingRateTier = D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED;
    UINT m_ShadingRateImageTileSize = 0;
    bool m_BackgroundProcessingSupported = false;

    // 31: Query Meta Command
    bool m_QueryMetaCommandAvailable = true;
    GUID m_CommandID = {};
    UINT m_NodeMask = 0;
    const void* m_pQueryInputData = nullptr;
    SIZE_T m_QueryInputDataSizeInBytes = 0;
    void* m_pQueryOutputData = nullptr;
    SIZE_T m_QueryOutputDataSizeInBytes = 0;

    // 32: Options7
    bool m_Options7Available = true;
    D3D12_MESH_SHADER_TIER m_MeshShaderTier = D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;
    D3D12_SAMPLER_FEEDBACK_TIER m_SamplerFeedbackTier = D3D12_SAMPLER_FEEDBACK_TIER_NOT_SUPPORTED;

    // 33: Protected Resource Session Type Count
    bool m_ProtectedResourceSessionTypeCountAvailable = true;
    std::vector<UINT> m_ProtectedResourceSessionTypeCount;

    // 34: Protected Resource Session Types
    bool m_ProtectedResourceSessionTypesAvailable = true;
    std::vector<std::vector<GUID>> m_ProtectedResourceSessionTypes;

    // 36: Options8
    bool m_Options8Available = true;
    bool m_UnalignedBlockTexturesSupported = false;

    // 37: Options9
    bool m_Options9Available = true;
    bool m_MeshShaderPipelineStatsSupported = false;
    bool m_MeshShaderSupportsFullRangeRenderTargetArrayIndex = false;
    bool m_AtomicInt64OnTypedResourceSupported = false;
    bool m_AtomicInt64OnGroupSharedSupported = false;
    bool m_DerivativesInMeshAndAmplificationShadersSupported = false;
    D3D12_WAVE_MMA_TIER m_WaveMMATier = D3D12_WAVE_MMA_TIER_NOT_SUPPORTED;

    // 39: Options10
    bool m_Options10Available = true;
    bool m_VariableRateShadingSumCombinerSupported = false;
    bool m_MeshShaderPerPrimitiveShadingRateSupported = false;

    // 40: Options11
    bool m_Options11Available = true;
    bool m_AtomicInt64OnDescriptorHeapResourceSupported = false;

    // 41: Options12
    bool m_Options12Available = true;
    D3D12_TRI_STATE m_MSPrimitivesPipelineStatisticIncludesCulledPrimitives = D3D12_TRI_STATE_UNKNOWN;
    bool m_EnhancedBarriersSupported = false;
};

#endif