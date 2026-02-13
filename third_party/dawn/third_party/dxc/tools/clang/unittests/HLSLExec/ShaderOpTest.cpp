///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ShaderOpTest.cpp                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides the implementation to run tests based on descriptions.           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// We need to keep & fix these warnings to integrate smoothly with HLK
#pragma warning(error : 4100 4242 4244 4267 4701 4389)

#include "d3dx12.h"
#include <atlbase.h>
#include <atlenc.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>
#include <windows.h>

#include "ShaderOpTest.h"

#include "HlslTestUtils.h"          // LogCommentFmt
#include "dxc/DXIL/DxilConstants.h" // ComponentType
#include "dxc/Support/Global.h"     // OutputDebugBytes
#include "dxc/Support/dxcapi.use.h" // DxcDllSupport
#include "dxc/dxcapi.h"             // IDxcCompiler

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <intsafe.h>
#include <stdlib.h>
#include <strsafe.h>
#include <xmllite.h>
#pragma comment(lib, "xmllite.lib")

// Duplicate definition of kDxCompilerLib to that in dxcapi.use.cpp
// These tests need the header, but don't want to depend on the source
// Since this is windows only, we only need the windows variant
namespace dxc {
const char *kDxCompilerLib = "dxcompiler.dll";
}

///////////////////////////////////////////////////////////////////////////////
// Useful helper functions.

uint16_t ConvertFloat32ToFloat16(float Value) throw() {
  return DirectX::PackedVector::XMConvertFloatToHalf(Value);
}
float ConvertFloat16ToFloat32(uint16_t Value) throw() {
  return DirectX::PackedVector::XMConvertHalfToFloat(Value);
}

static st::OutputStringFn g_OutputStrFn;
static void *g_OutputStrFnCtx;

void st::SetOutputFn(void *pCtx, OutputStringFn F) {
  g_OutputStrFnCtx = pCtx;
  g_OutputStrFn = F;
}

static void ShaderOpLogFmt(const wchar_t *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::wstring buf(hlsl_test::vFormatToWString(fmt, args));
  va_end(args);
  if (g_OutputStrFn == nullptr)
    WEX::Logging::Log::Comment(buf.data());
  else
    g_OutputStrFn(g_OutputStrFnCtx, buf.data());
}

// Rely on TAEF Verifier helpers.
#define CHECK_HR(x)                                                            \
  {                                                                            \
    if (!g_OutputStrFn)                                                        \
      VERIFY_SUCCEEDED(x);                                                     \
    else {                                                                     \
      HRESULT _check_hr = (x);                                                 \
      if (FAILED(_check_hr))                                                   \
        AtlThrow(x);                                                           \
    }                                                                          \
  }

// Check the specified HRESULT and return the success value.
static HRESULT CHECK_HR_RET(HRESULT hr) {
  CHECK_HR(hr);
  return hr;
}

HRESULT LogIfLost(HRESULT hr, ID3D12Device *pDevice) {
  if (hr == DXGI_ERROR_DEVICE_REMOVED) {
    HRESULT reason = pDevice->GetDeviceRemovedReason();
    LPCWSTR reasonText = L"?";
    if (reason == DXGI_ERROR_DEVICE_HUNG)
      reasonText = L"DXGI_ERROR_DEVICE_HUNG";
    if (reason == DXGI_ERROR_DEVICE_REMOVED)
      reasonText = L"DXGI_ERROR_DEVICE_REMOVED";
    if (reason == DXGI_ERROR_DEVICE_RESET)
      reasonText = L"DXGI_ERROR_DEVICE_RESET";
    if (reason == DXGI_ERROR_DRIVER_INTERNAL_ERROR)
      reasonText = L"DXGI_ERROR_DRIVER_INTERNAL_ERROR";
    if (reason == DXGI_ERROR_INVALID_CALL)
      reasonText = L"DXGI_ERROR_INVALID_CALL";
    ShaderOpLogFmt(L"Device lost: 0x%08x (%s)", reason, reasonText);
  }
  return hr;
}

HRESULT LogIfLost(HRESULT hr, ID3D12Resource *pResource) {
  if (hr == DXGI_ERROR_DEVICE_REMOVED) {
    CComPtr<ID3D12Device> pDevice;
    pResource->GetDevice(__uuidof(ID3D12Device), (void **)&pDevice);
    LogIfLost(hr, pDevice);
  }
  return hr;
}

void RecordTransitionBarrier(ID3D12GraphicsCommandList *pCommandList,
                             ID3D12Resource *pResource,
                             D3D12_RESOURCE_STATES before,
                             D3D12_RESOURCE_STATES after) {
  CD3DX12_RESOURCE_BARRIER barrier(
      CD3DX12_RESOURCE_BARRIER::Transition(pResource, before, after));
  pCommandList->ResourceBarrier(1, &barrier);
}

void ExecuteCommandList(ID3D12CommandQueue *pQueue, ID3D12CommandList *pList) {
  ID3D12CommandList *ppCommandLists[] = {pList};
  pQueue->ExecuteCommandLists(1, ppCommandLists);
}

HRESULT SetObjectName(ID3D12Object *pObject, LPCSTR pName) {
  if (pObject && pName) {
    CA2W WideName(pName);
    return pObject->SetName(WideName);
  }
  return S_FALSE;
}

void WaitForSignal(ID3D12CommandQueue *pCQ, ID3D12Fence *pFence, HANDLE hFence,
                   UINT64 fenceValue) {
  // Signal and increment the fence value.
  const UINT64 fence = fenceValue;
  CHECK_HR(pCQ->Signal(pFence, fence));

  if (pFence->GetCompletedValue() < fenceValue) {
    CHECK_HR(pFence->SetEventOnCompletion(fenceValue, hFence));
    WaitForSingleObject(hFence, INFINITE);
    // CHECK_HR(pCQ->Wait(pFence, fenceValue));
  }
}

void MappedData::dump() const { OutputDebugBytes(m_pData, m_size); }

void MappedData::reset() {
  if (m_pResource != nullptr) {
    m_pResource->Unmap(0, nullptr);
    m_pResource.Release();
  }
  m_pData = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// Helper class for mapped data.

void MappedData::reset(ID3D12Resource *pResource, UINT32 sizeInBytes) {
  reset();
  D3D12_RANGE r;
  r.Begin = 0;
  r.End = sizeInBytes;
  CHECK_HR(LogIfLost(pResource->Map(0, &r, &m_pData), pResource));
  m_pResource = pResource;
  m_size = sizeInBytes;
}

///////////////////////////////////////////////////////////////////////////////
// ShaderOpTest library implementation.

namespace st {

bool UseHardwareDevice(const DXGI_ADAPTER_DESC1 &desc, LPCWSTR AdapterName) {
  if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
    // Don't select the Basic Render Driver adapter.
    return false;
  }

  if (!AdapterName)
    return true;
  return hlsl_test::IsStarMatchWide(AdapterName, wcslen(AdapterName),
                                    desc.Description, wcslen(desc.Description));
}

void GetHardwareAdapter(IDXGIFactory2 *pFactory, LPCWSTR AdapterName,
                        IDXGIAdapter1 **ppAdapter) {
  CComPtr<IDXGIAdapter1> adapter;
  *ppAdapter = nullptr;

  for (UINT adapterIndex = 0;
       DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter);
       ++adapterIndex) {
    DXGI_ADAPTER_DESC1 desc;
    adapter->GetDesc1(&desc);

    if (!UseHardwareDevice(desc, AdapterName)) {
      adapter.Release();
      continue;
    }

    // Check to see if the adapter supports Direct3D 12, but don't create the
    // actual device yet.
    if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0,
                                    _uuidof(ID3D12Device), nullptr))) {
      break;
    }
    adapter.Release();
  }

  *ppAdapter = adapter.Detach();
}

LPCSTR string_table::insert(LPCSTR pValue) {
  std::unordered_set<LPCSTR, HashStr, PredStr>::iterator i =
      m_values.find(pValue);
  if (i == m_values.end()) {
    size_t bufSize = strlen(pValue) + 1;
    std::vector<char> s;
    s.resize(bufSize);
    strcpy_s(s.data(), bufSize, pValue);
    LPCSTR result = s.data();
    m_values.insert(result);
    m_strings.push_back(std::move(s));
    return result;
  } else {
    return *i;
  }
}

LPCSTR string_table::insert(LPCWSTR pValue) {
  CW2A pValueAnsi(pValue);
  return insert(pValueAnsi.m_psz);
}

void CommandListRefs::CreateForDevice(ID3D12Device *pDevice, bool compute) {
  D3D12_COMMAND_LIST_TYPE T = compute ? D3D12_COMMAND_LIST_TYPE_COMPUTE
                                      : D3D12_COMMAND_LIST_TYPE_DIRECT;
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.Type = T;

  if (Queue == nullptr) {
    CHECK_HR(pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&Queue)));
  }
  CHECK_HR(pDevice->CreateCommandAllocator(T, IID_PPV_ARGS(&Allocator)));
  CHECK_HR(pDevice->CreateCommandList(0, T, Allocator, nullptr,
                                      IID_PPV_ARGS(&List)));
}

ShaderOpTest::ShaderOpTest() {
  m_hFence = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  if (m_hFence == nullptr) {
    AtlThrow(HRESULT_FROM_WIN32(GetLastError()));
  }
}

ShaderOpTest::~ShaderOpTest() { CloseHandle(m_hFence); }

void ShaderOpTest::CopyBackResources() {
  CommandListRefs ResCommandList;
  ResCommandList.CreateForDevice(m_pDevice, m_pShaderOp->IsCompute());
  ID3D12GraphicsCommandList *pList = ResCommandList.List;

  pList->SetName(L"ShaderOpTest Resource ReadBack CommandList");
  for (ShaderOpResource &R : m_pShaderOp->Resources) {
    if (!R.ReadBack)
      continue;
    ShaderOpResourceData &D = m_ResourceData[R.Name];
    RecordTransitionBarrier(pList, D.Resource, D.ResourceState,
                            D3D12_RESOURCE_STATE_COPY_SOURCE);
    D.ResourceState = D3D12_RESOURCE_STATE_COPY_SOURCE;
    D3D12_RESOURCE_DESC &Desc = D.ShaderOpRes->Desc;
    if (Desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
      pList->CopyResource(D.ReadBack, D.Resource);
    } else {
      UINT64 rowPitch = Desc.Width * GetByteSizeForFormat(Desc.Format);
      if (rowPitch % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)
        rowPitch += D3D12_TEXTURE_DATA_PITCH_ALIGNMENT -
                    (rowPitch % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
      D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint;
      Footprint.Offset = 0;
      Footprint.Footprint = CD3DX12_SUBRESOURCE_FOOTPRINT(
          Desc.Format, (UINT)Desc.Width, Desc.Height, 1, (UINT)rowPitch);
      CD3DX12_TEXTURE_COPY_LOCATION DstLoc(D.ReadBack, Footprint);
      CD3DX12_TEXTURE_COPY_LOCATION SrcLoc(D.Resource, 0);
      pList->CopyTextureRegion(&DstLoc, 0, 0, 0, &SrcLoc, nullptr);
    }
  }
  pList->Close();
  ExecuteCommandList(ResCommandList.Queue, pList);
  WaitForSignal(ResCommandList.Queue, m_pFence, m_hFence, m_FenceValue++);
}

void ShaderOpTest::CreateCommandList() {
  bool priorQueue = m_CommandList.Queue != nullptr;
  m_CommandList.CreateForDevice(m_pDevice, m_pShaderOp->IsCompute());
  m_CommandList.Allocator->SetName(L"ShaderOpTest Allocator");
  m_CommandList.List->SetName(L"ShaderOpTest CommandList");
  if (!priorQueue)
    m_CommandList.Queue->SetName(L"ShaderOpTest CommandList");
}

void ShaderOpTest::CreateDescriptorHeaps() {
  for (ShaderOpDescriptorHeap &H : m_pShaderOp->DescriptorHeaps) {
    CComPtr<ID3D12DescriptorHeap> pHeap;
    if (H.Desc.NumDescriptors == 0) {
      H.Desc.NumDescriptors = (UINT)H.Descriptors.size();
    }
    CHECK_HR(m_pDevice->CreateDescriptorHeap(&H.Desc, IID_PPV_ARGS(&pHeap)));
    m_DescriptorHeaps.push_back(pHeap);
    m_DescriptorHeapsByName[H.Name] = pHeap;
    SetObjectName(pHeap, H.Name);

    const UINT descriptorSize =
        m_pDevice->GetDescriptorHandleIncrementSize(H.Desc.Type);
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(
        pHeap->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};
    if (H.Desc.Type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV &&
        H.Desc.Type != D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
      gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
          pHeap->GetGPUDescriptorHandleForHeapStart());
    for (ShaderOpDescriptor &D : H.Descriptors) {
      ShaderOpDescriptorData DData;
      DData.Descriptor = &D;
      ID3D12Resource *pResource = nullptr;
      if (H.Desc.Type != D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {
        ShaderOpResource *R = m_pShaderOp->GetResourceByName(D.ResName);
        auto itResData = m_ResourceData.find(D.ResName);
        if (R == nullptr || itResData == m_ResourceData.end()) {
          LPCSTR DescName = D.Name ? D.Name : "[unnamed descriptor]";
          ShaderOpLogFmt(L"Descriptor '%S' references missing resource '%S'",
                         DescName, D.ResName);
          CHECK_HR(E_INVALIDARG);
        }
        DData.ResData = &itResData->second;
        pResource = DData.ResData->Resource;
      }

      if (0 == _stricmp(D.Kind, "UAV")) {
        ID3D12Resource *pCounterResource = nullptr;
        if (D.CounterName && *D.CounterName) {
          ShaderOpResourceData &CounterData = m_ResourceData[D.CounterName];
          pCounterResource = CounterData.Resource;
        }
        ShaderOpResource *R = m_pShaderOp->GetResourceByName(D.ResName);
        // Ensure the TransitionTo state is set for UAV's that will be used as
        // UAV's.
        if (R && R->TransitionTo != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
          ShaderOpLogFmt(L"Resource '%S' used in UAV descriptor, but "
                         L"TransitionTo not set to 'UNORDERED_ACCESS'",
                         D.ResName);
          CHECK_HR(E_FAIL);
        }
        m_pDevice->CreateUnorderedAccessView(pResource, pCounterResource,
                                             &D.UavDesc, cpuHandle);
      } else if (0 == _stricmp(D.Kind, "SRV")) {
        D3D12_SHADER_RESOURCE_VIEW_DESC *pSrvDesc = nullptr;
        if (D.SrvDescPresent) {
          pSrvDesc = &D.SrvDesc;
        }
        m_pDevice->CreateShaderResourceView(pResource, pSrvDesc, cpuHandle);
      } else if (0 == _stricmp(D.Kind, "RTV")) {
        m_pDevice->CreateRenderTargetView(pResource, nullptr, cpuHandle);
      } else if (0 == _stricmp(D.Kind, "CBV")) {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
        cbvDesc.BufferLocation = pResource->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = (UINT)pResource->GetDesc().Width;
        m_pDevice->CreateConstantBufferView(&cbvDesc, cpuHandle);
      } else if (0 == _stricmp(D.Kind, "SAMPLER")) {
        m_pDevice->CreateSampler(&D.SamplerDesc, cpuHandle);
      }

      DData.CPUHandle = cpuHandle;
      cpuHandle = cpuHandle.Offset(descriptorSize);
      if (H.Desc.Type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV &&
          H.Desc.Type != D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {
        DData.GPUHandle = gpuHandle;
        gpuHandle = gpuHandle.Offset(descriptorSize);
      }
      m_DescriptorData[D.Name] = DData;
    }
  }

  // Create query descriptor heap.
  D3D12_QUERY_HEAP_DESC queryHeapDesc;
  ZeroMemory(&queryHeapDesc, sizeof(queryHeapDesc));
  queryHeapDesc.Count = 1;
  queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
  CHECK_HR(
      m_pDevice->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&m_pQueryHeap)));
}

void ShaderOpTest::CreateDevice() {
  if (m_pDevice == nullptr) {
    const D3D_FEATURE_LEVEL FeatureLevelRequired = D3D_FEATURE_LEVEL_11_0;
    CComPtr<IDXGIFactory4> factory;
    CComPtr<ID3D12Device> pDevice;

    CHECK_HR(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
    if (m_pShaderOp->UseWarpDevice) {
      CComPtr<IDXGIAdapter> warpAdapter;
      CHECK_HR(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
      CHECK_HR(D3D12CreateDevice(warpAdapter, FeatureLevelRequired,
                                 IID_PPV_ARGS(&pDevice)));
    } else {
      CComPtr<IDXGIAdapter1> hardwareAdapter;
      GetHardwareAdapter(factory, m_pShaderOp->AdapterName, &hardwareAdapter);
      if (hardwareAdapter == nullptr) {
        CHECK_HR(HRESULT_FROM_WIN32(ERROR_NOT_FOUND));
      }
      CHECK_HR(D3D12CreateDevice(hardwareAdapter, FeatureLevelRequired,
                                 IID_PPV_ARGS(&pDevice)));
    }

    m_pDevice.Attach(pDevice.Detach());
    m_pDevice->SetName(L"ShaderOpTest Device");
  }

  m_FenceValue = 1;
  CHECK_HR(m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                  __uuidof(ID3D12Fence), (void **)&m_pFence));
  m_pFence->SetName(L"ShaderOpTest Fence");
}

static void InitByteCode(D3D12_SHADER_BYTECODE *pBytecode, ID3D10Blob *pBlob) {
  if (pBlob == nullptr) {
    pBytecode->BytecodeLength = 0;
    pBytecode->pShaderBytecode = nullptr;
  } else {
    pBytecode->BytecodeLength = pBlob->GetBufferSize();
    pBytecode->pShaderBytecode = pBlob->GetBufferPointer();
  }
}

template <typename TKey, typename TValue>
TValue map_get_or_null(const std::map<TKey, TValue> &amap, const TKey &key) {
  auto it = amap.find(key);
  if (it == amap.end())
    return nullptr;
  return (*it).second;
}

void ShaderOpTest::CreatePipelineState() {
  CreateRootSignature();
  CreateShaders();
  // Root signature may come from XML, or from shader.
  if (!m_pRootSignature) {
    ShaderOpLogFmt(L"No root signature found\r\n");
    CHECK_HR(E_FAIL);
  }
  if (m_pShaderOp->IsCompute()) {
    CComPtr<ID3D10Blob> pCS;
    pCS = m_Shaders[m_pShaderOp->CS];
    D3D12_COMPUTE_PIPELINE_STATE_DESC CDesc;
    ZeroMemory(&CDesc, sizeof(CDesc));
    CDesc.pRootSignature = m_pRootSignature.p;
    InitByteCode(&CDesc.CS, pCS);
    CHECK_HR(
        m_pDevice->CreateComputePipelineState(&CDesc, IID_PPV_ARGS(&m_pPSO)));
  }
  // Wakanda technology, needs vibranium to work
#if defined(NTDDI_WIN10_VB) && WDK_NTDDI_VERSION >= NTDDI_WIN10_VB
  else if (m_pShaderOp->MS) {
    // A couple types from a future version of d3dx12.h
    typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT<
        D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS>
        CD3DX12_PIPELINE_STATE_STREAM_MS;
    typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT<
        D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS>
        CD3DX12_PIPELINE_STATE_STREAM_AS;

    struct D3DX12_MESH_SHADER_PIPELINE_STATE_DESC {
      CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE RootSignature;
      CD3DX12_PIPELINE_STATE_STREAM_AS AS;
      CD3DX12_PIPELINE_STATE_STREAM_MS MS;
      CD3DX12_PIPELINE_STATE_STREAM_PS PS;
      CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
      CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK SampleMask;

      CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;

    } MDesc = {};

    CComPtr<ID3D10Blob> pAS, pMS, pPS;
    pAS = map_get_or_null(m_Shaders, m_pShaderOp->AS);
    pMS = map_get_or_null(m_Shaders, m_pShaderOp->MS);
    pPS = map_get_or_null(m_Shaders, m_pShaderOp->PS);
    CHECK_HR((m_pShaderOp->AS && !pAS) ? E_FAIL : S_OK);
    CHECK_HR((m_pShaderOp->MS && !pMS) ? E_FAIL : S_OK);
    CHECK_HR((m_pShaderOp->PS && !pPS) ? E_FAIL : S_OK);

    ZeroMemory(&MDesc, sizeof(MDesc));
    MDesc.RootSignature =
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE(m_pRootSignature.p);
    D3D12_SHADER_BYTECODE BC;
    InitByteCode(&BC, pAS);
    MDesc.AS = CD3DX12_PIPELINE_STATE_STREAM_AS(BC);
    InitByteCode(&BC, pMS);
    MDesc.MS = CD3DX12_PIPELINE_STATE_STREAM_MS(BC);
    InitByteCode(&BC, pPS);
    MDesc.PS = CD3DX12_PIPELINE_STATE_STREAM_PS(BC);
    MDesc.PrimitiveTopologyType =
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY(
            m_pShaderOp->PrimitiveTopologyType);
    MDesc.SampleMask =
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK(m_pShaderOp->SampleMask);

    D3D12_RT_FORMAT_ARRAY RtArray;
    ZeroMemory(&RtArray, sizeof(RtArray));
    RtArray.NumRenderTargets = (UINT)m_pShaderOp->RenderTargets.size();
    for (size_t i = 0; i < RtArray.NumRenderTargets; ++i) {
      ShaderOpResource *R =
          m_pShaderOp->GetResourceByName(m_pShaderOp->RenderTargets[i].Name);
      RtArray.RTFormats[i] = R->Desc.Format;
    }
    MDesc.RTVFormats =
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS(RtArray);

    D3D12_PIPELINE_STATE_STREAM_DESC PDesc = {};
    PDesc.SizeInBytes = sizeof(MDesc);
    PDesc.pPipelineStateSubobjectStream = &MDesc;

    CComPtr<ID3D12Device2> pDevice2;
    CHECK_HR(m_pDevice->QueryInterface(&pDevice2));

    CHECK_HR(pDevice2->CreatePipelineState(&PDesc, IID_PPV_ARGS(&m_pPSO)));
  }
#endif
  else {
    CComPtr<ID3D10Blob> pVS, pDS, pHS, pGS, pPS;
    pPS = map_get_or_null(m_Shaders, m_pShaderOp->PS);
    pVS = map_get_or_null(m_Shaders, m_pShaderOp->VS);
    pGS = map_get_or_null(m_Shaders, m_pShaderOp->GS);
    pHS = map_get_or_null(m_Shaders, m_pShaderOp->HS);
    pDS = map_get_or_null(m_Shaders, m_pShaderOp->DS);
    // Check for missing shaders with explicitly requested names
    CHECK_HR((m_pShaderOp->PS && !pPS) ? E_FAIL : S_OK);
    CHECK_HR((m_pShaderOp->VS && !pVS) ? E_FAIL : S_OK);
    CHECK_HR((m_pShaderOp->GS && !pGS) ? E_FAIL : S_OK);
    CHECK_HR((m_pShaderOp->HS && !pHS) ? E_FAIL : S_OK);
    CHECK_HR((m_pShaderOp->DS && !pDS) ? E_FAIL : S_OK);
    D3D12_GRAPHICS_PIPELINE_STATE_DESC GDesc;
    ZeroMemory(&GDesc, sizeof(GDesc));
    InitByteCode(&GDesc.VS, pVS);
    InitByteCode(&GDesc.HS, pHS);
    InitByteCode(&GDesc.DS, pDS);
    InitByteCode(&GDesc.GS, pGS);
    InitByteCode(&GDesc.PS, pPS);
    GDesc.InputLayout.NumElements = (UINT)m_pShaderOp->InputElements.size();
    GDesc.InputLayout.pInputElementDescs = m_pShaderOp->InputElements.data();
    GDesc.PrimitiveTopologyType = m_pShaderOp->PrimitiveTopologyType;
    GDesc.NumRenderTargets = (UINT)m_pShaderOp->RenderTargets.size();
    GDesc.SampleMask = m_pShaderOp->SampleMask;
    for (size_t i = 0; i < m_pShaderOp->RenderTargets.size(); ++i) {
      ShaderOpResource *R =
          m_pShaderOp->GetResourceByName(m_pShaderOp->RenderTargets[i].Name);
      GDesc.RTVFormats[i] = R->Desc.Format;
    }
    GDesc.SampleDesc.Count = 1; // TODO: read from file, set from shader
                                // operation; also apply to count
    GDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(
        D3D12_DEFAULT); // TODO: read from file, set from op
    GDesc.BlendState =
        CD3DX12_BLEND_DESC(D3D12_DEFAULT); // TODO: read from file, set from op

    // TODO: pending values to set
#if 0
    D3D12_STREAM_OUTPUT_DESC           StreamOutput;
    D3D12_DEPTH_STENCIL_DESC           DepthStencilState;
    D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBStripCutValue;
    DXGI_FORMAT                        DSVFormat;
    UINT                               NodeMask;
    D3D12_PIPELINE_STATE_FLAGS         Flags;
#endif
    GDesc.pRootSignature = m_pRootSignature.p;
    CHECK_HR(
        m_pDevice->CreateGraphicsPipelineState(&GDesc, IID_PPV_ARGS(&m_pPSO)));
  }
}

void ShaderOpTest::CreateResources() {
  CommandListRefs ResCommandList;
  ResCommandList.CreateForDevice(m_pDevice, true);
  ResCommandList.Allocator->SetName(
      L"ShaderOpTest Resource Creation Allocation");
  ResCommandList.Queue->SetName(L"ShaderOpTest Resource Creation Queue");
  ResCommandList.List->SetName(L"ShaderOpTest Resource Creation CommandList");

  ID3D12GraphicsCommandList *pList = ResCommandList.List.p;
  std::vector<CComPtr<ID3D12Resource>> intermediates;

  for (ShaderOpResource &R : m_pShaderOp->Resources) {
    if (m_ResourceData.count(R.Name) > 0)
      continue;
    // Initialize the upload resource early, to allow a by-name initializer
    // to set the desired width.
    bool initByName = R.Init && 0 == _stricmp("byname", R.Init);
    bool initZero = R.Init && 0 == _stricmp("zero", R.Init);
    bool initFromBytes = R.Init && 0 == _stricmp("frombytes", R.Init);
    bool hasInit = initByName || initZero || initFromBytes;
    bool isBuffer = R.Desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER;
    std::vector<BYTE> values;
    if (hasInit) {
      if (isBuffer) {
        values.resize((size_t)R.Desc.Width);
      } else {
        // Probably needs more information.
        values.resize((size_t)(R.Desc.Width * R.Desc.Height *
                               GetByteSizeForFormat(R.Desc.Format)));
      }
      if (initZero) {
        memset(values.data(), 0, values.size());
      } else if (initByName) {
        m_InitCallbackFn(R.Name, values, m_pShaderOp);
        if (isBuffer) {
          R.Desc.Width = values.size();
        }
      } else if (initFromBytes) {
        values = R.InitBytes;
        if (R.Desc.Width == 0) {
          if (isBuffer) {
            R.Desc.Width = values.size();
          } else if (R.Desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D) {
            R.Desc.Width = values.size() / GetByteSizeForFormat(R.Desc.Format);
          }
        }
      }
    }
    if (!R.Desc.MipLevels)
      R.Desc.MipLevels = 1;

    CComPtr<ID3D12Resource> pResource;
    CHECK_HR(m_pDevice->CreateCommittedResource(
        &R.HeapProperties, R.HeapFlags, &R.Desc, R.InitialResourceState,
        nullptr, IID_PPV_ARGS(&pResource)));
    ShaderOpResourceData &D = m_ResourceData[R.Name];
    D.ShaderOpRes = &R;
    D.Resource = pResource;
    D.ResourceState = R.InitialResourceState;
    SetObjectName(pResource, R.Name);

    if (hasInit) {
      CComPtr<ID3D12Resource> pIntermediate;
      CD3DX12_HEAP_PROPERTIES upload(D3D12_HEAP_TYPE_UPLOAD);
      D3D12_RESOURCE_DESC uploadDesc = R.Desc;

      // Calculate size required for intermediate buffer
      UINT64 totalBytes;
      m_pDevice->GetCopyableFootprints(&uploadDesc, 0, R.Desc.MipLevels, 0,
                                       nullptr, nullptr, nullptr, &totalBytes);

      if (!isBuffer) {
        // Assuming a simple linear layout here.
        uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        uploadDesc.Width = totalBytes;
        uploadDesc.Height = 1;
        uploadDesc.MipLevels = 1;
        uploadDesc.Format = DXGI_FORMAT_UNKNOWN;
        uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      }
      uploadDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
      CHECK_HR(m_pDevice->CreateCommittedResource(
          &upload, D3D12_HEAP_FLAG_NONE, &uploadDesc,
          D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
          IID_PPV_ARGS(&pIntermediate)));
      intermediates.push_back(pIntermediate);

      char uploadObjectName[128];
      if (R.Name && SUCCEEDED(StringCchPrintfA(
                        uploadObjectName, _countof(uploadObjectName),
                        "Upload resource for %s", R.Name))) {
        SetObjectName(pIntermediate, uploadObjectName);
      }

      D3D12_SUBRESOURCE_DATA transferData[16];
      UINT width = (UINT)R.Desc.Width;
      UINT height = R.Desc.Height;
      UINT pixelSize = GetByteSizeForFormat(R.Desc.Format);
      BYTE *data = values.data();

      VERIFY_IS_TRUE(R.Desc.MipLevels <= 16);

      for (UINT i = 0; i < R.Desc.MipLevels; i++) {
        if (!height)
          height = 1;
        if (!width)
          width = 1;
        transferData[i].pData = data;
        transferData[i].RowPitch = width * pixelSize;
        transferData[i].SlicePitch = width * height * pixelSize;
        data += width * height * pixelSize;
        height >>= 1;
        width >>= 1;
      }

      UpdateSubresources<16>(pList, pResource.p, pIntermediate.p, 0, 0,
                             R.Desc.MipLevels, transferData);
    }

    if (R.ReadBack) {
      CComPtr<ID3D12Resource> pReadbackResource;
      CD3DX12_HEAP_PROPERTIES readback(D3D12_HEAP_TYPE_READBACK);
      UINT64 width = R.Desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER
                         ? R.Desc.Width
                         : (R.Desc.Height * R.Desc.Width *
                            GetByteSizeForFormat(R.Desc.Format));
      CD3DX12_RESOURCE_DESC readbackDesc(CD3DX12_RESOURCE_DESC::Buffer(width));
      readbackDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
      CHECK_HR(m_pDevice->CreateCommittedResource(
          &readback, D3D12_HEAP_FLAG_NONE, &readbackDesc,
          D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
          IID_PPV_ARGS(&pReadbackResource)));
      D.ReadBack = pReadbackResource;

      char readbackObjectName[128];
      if (R.Name && SUCCEEDED(StringCchPrintfA(
                        readbackObjectName, _countof(readbackObjectName),
                        "Readback resource for %s", R.Name))) {
        SetObjectName(pReadbackResource, readbackObjectName);
      }
    }

    if (R.TransitionTo != D.ResourceState) {
      RecordTransitionBarrier(pList, D.Resource, D.ResourceState,
                              R.TransitionTo);
      D.ResourceState = R.TransitionTo;
    }
  }

  // Create a buffer to receive query results.
  {
    CComPtr<ID3D12Resource> pReadbackResource;
    CD3DX12_HEAP_PROPERTIES readback(D3D12_HEAP_TYPE_READBACK);
    CD3DX12_RESOURCE_DESC readbackDesc(CD3DX12_RESOURCE_DESC::Buffer(
        sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS)));
    CHECK_HR(m_pDevice->CreateCommittedResource(
        &readback, D3D12_HEAP_FLAG_NONE, &readbackDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
        IID_PPV_ARGS(&m_pQueryBuffer)));
    SetObjectName(m_pQueryBuffer, "Query Pipeline Readback Buffer");
  }

  CHECK_HR(pList->Close());
  ExecuteCommandList(ResCommandList.Queue, pList);
  WaitForSignal(ResCommandList.Queue, m_pFence, m_hFence, m_FenceValue++);
}

void ShaderOpTest::CreateRootSignature() {
  if (m_pShaderOp->RootSignature == nullptr) {
    // Root signature may be provided as part of shader
    m_pRootSignature.Release();
    return;
  }
  CComPtr<IDxcBlob> pRootSignatureBlob;
  std::string sQuoted;
  sQuoted.reserve(15 + strlen(m_pShaderOp->RootSignature) + 1);
  sQuoted.append("#define main \"");
  sQuoted.append(m_pShaderOp->RootSignature);
  sQuoted.append("\"");
  char *ch = (char *)sQuoted.data();
  while (*ch) {
    if (*ch == '\r' || *ch == '\n')
      *ch = ' ';
    ++ch;
  }

  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcBlobEncoding> pTextBlob;
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CHECK_HR(m_pDxcSupport->CreateInstance(CLSID_DxcLibrary, &pLibrary));
  CHECK_HR(pLibrary->CreateBlobWithEncodingFromPinned(
      sQuoted.c_str(), (UINT32)sQuoted.size(), CP_UTF8, &pTextBlob));
  CHECK_HR(m_pDxcSupport->CreateInstance(CLSID_DxcCompiler, &pCompiler));
  CHECK_HR(pCompiler->Compile(pTextBlob, L"RootSigShader", nullptr,
                              L"rootsig_1_0", nullptr, 0, // args
                              nullptr, 0,                 // defines
                              nullptr, &pResult));
  HRESULT resultCode;
  CHECK_HR(pResult->GetStatus(&resultCode));
  if (FAILED(resultCode)) {
    CComPtr<IDxcBlobEncoding> errors;
    CHECK_HR(pResult->GetErrorBuffer(&errors));
    ShaderOpLogFmt(L"Failed to compile root signature: %*S\r\n",
                   (int)errors->GetBufferSize(),
                   (LPCSTR)errors->GetBufferPointer());
  }
  CHECK_HR(resultCode);
  CHECK_HR(pResult->GetResult(&pRootSignatureBlob));
  CHECK_HR(m_pDevice->CreateRootSignature(
      0, pRootSignatureBlob->GetBufferPointer(),
      pRootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature)));
}

static bool TargetUsesDxil(LPCSTR pText) {
  return (strlen(pText) > 3) && pText[3] >= '6'; // xx_6xx
}

static void splitWStringIntoVectors(LPWSTR str, wchar_t delim,
                                    std::vector<LPCWSTR> &list) {
  if (str) {
    LPWSTR cur = str;
    list.push_back(cur);
    while (*cur != L'\0') {
      if (*cur == delim) {
        list.push_back(cur + 1);
        *(cur) = L'\0';
      }
      cur++;
    }
  }
}
void ShaderOpTest::CreateShaders() {
  for (ShaderOpShader &S : m_pShaderOp->Shaders) {
    CComPtr<ID3DBlob> pCode;
    HRESULT hr = S_OK;
    LPCSTR pText = m_pShaderOp->GetShaderText(&S);
    LPCSTR pArguments = m_pShaderOp->GetShaderArguments(&S);
    if (S.Callback) {
      if (!m_ShaderCallbackFn) {
        ShaderOpLogFmt(
            L"Callback required for shader, but not provided: %S\r\n", S.Name);
        CHECK_HR(E_FAIL);
      }
      m_ShaderCallbackFn(S.Name, pText, (IDxcBlob **)&pCode, m_pShaderOp);
    } else if (S.Compiled) {
      int textLen = (int)strlen(pText);
      int decodedLen = Base64DecodeGetRequiredLength(textLen);
      // Length is an approximation, so we can't creat the final blob yet.
      std::vector<BYTE> decoded;
      decoded.resize(decodedLen);
      if (!Base64Decode(pText, textLen, decoded.data(), &decodedLen)) {
        ShaderOpLogFmt(L"Failed to decode compiled shader: %S\r\n", S.Name);
        CHECK_HR(E_FAIL);
      }
      // decodedLen should have the correct size now.
      CHECK_HR(D3DCreateBlob(decodedLen, &pCode));
      memcpy(pCode->GetBufferPointer(), decoded.data(), decodedLen);
    } else if (TargetUsesDxil(S.Target)) {
      CComPtr<IDxcCompiler> pCompiler;
      CComPtr<IDxcLibrary> pLibrary;
      CComPtr<IDxcBlobEncoding> pTextBlob;
      CComPtr<IDxcOperationResult> pResult;
      CA2W nameW(S.Name);
      CA2W entryPointW(S.EntryPoint);
      CA2W targetW(S.Target);
      CA2W argumentsW(pArguments);

      std::vector<LPCWSTR> argumentsWList;
      splitWStringIntoVectors(argumentsW, L' ', argumentsWList);

      HRESULT resultCode;
      CHECK_HR(m_pDxcSupport->CreateInstance(CLSID_DxcLibrary, &pLibrary));
      CHECK_HR(pLibrary->CreateBlobWithEncodingFromPinned(
          pText, (UINT32)strlen(pText), CP_UTF8, &pTextBlob));
      CHECK_HR(m_pDxcSupport->CreateInstance(CLSID_DxcCompiler, &pCompiler));
      WEX::Logging::Log::Comment(L"Compiling shader:");
      ShaderOpLogFmt(L"\tTarget profile: %S", S.Target);
      if (argumentsWList.size() > 0) {
        ShaderOpLogFmt(L"\tArguments: %S", pArguments);
      }
      CHECK_HR(pCompiler->Compile(pTextBlob, nameW, entryPointW, targetW,
                                  (LPCWSTR *)argumentsWList.data(),
                                  (UINT32)argumentsWList.size(), nullptr, 0,
                                  nullptr, &pResult));
      CHECK_HR(pResult->GetStatus(&resultCode));
      if (FAILED(resultCode)) {
        CComPtr<IDxcBlobEncoding> errors;
        CHECK_HR(pResult->GetErrorBuffer(&errors));
        ShaderOpLogFmt(L"Failed to compile shader: %*S\r\n",
                       (int)errors->GetBufferSize(),
                       errors->GetBufferPointer());
      }
      CHECK_HR(resultCode);
      CHECK_HR(pResult->GetResult((IDxcBlob **)&pCode));
#if 0 // use the following code to test compiled shader
      CComPtr<IDxcBlob> pCode;
      CHECK_HR(pResult->GetResult(&pCode));
      CComPtr<IDxcBlobEncoding> pBlob;
      CHECK_HR(pCompiler->Disassemble((IDxcBlob *)pCode, (IDxcBlobEncoding **)&pBlob));
      CComPtr<IDxcBlobEncoding> pWideBlob;
      pLibrary->GetBlobAsWide(pBlob, &pWideBlob);
      hlsl_test::LogCommentFmt(L"%*s", (int)pWideBlob->GetBufferSize() / 2, (LPCWSTR)pWideBlob->GetBufferPointer());
#endif
    } else {
      CComPtr<ID3DBlob> pError;
      hr = D3DCompile(pText, strlen(pText), S.Name, nullptr, nullptr,
                      S.EntryPoint, S.Target, 0, 0, &pCode, &pError);
      if (FAILED(hr) && pError != nullptr) {
        ShaderOpLogFmt(L"%*S\r\n", (int)pError->GetBufferSize(),
                       ((LPCSTR)pError->GetBufferPointer()));
      }
    }
    CHECK_HR(hr);
    m_Shaders[S.Name] = pCode;

    if (!m_pRootSignature) {
      // Try to create root signature from shader instead.
      HRESULT hr = m_pDevice->CreateRootSignature(
          0, pCode->GetBufferPointer(), pCode->GetBufferSize(),
          IID_PPV_ARGS(&m_pRootSignature));
      if (SUCCEEDED(hr)) {
        ShaderOpLogFmt(L"Root signature created from shader %S\r\n", S.Name);
      }
    }
  }
}

void ShaderOpTest::GetPipelineStats(
    D3D12_QUERY_DATA_PIPELINE_STATISTICS *pStats) {
  MappedData M;
  M.reset(m_pQueryBuffer, sizeof(*pStats));
  memcpy(pStats, M.data(), sizeof(*pStats));
}

void ShaderOpTest::GetReadBackData(LPCSTR pResourceName, MappedData *pData) {
  pResourceName = m_pShaderOp->Strings.insert(pResourceName); // Unique
  ShaderOpResourceData &D = m_ResourceData.at(pResourceName);
  D3D12_RESOURCE_DESC Desc = D.ReadBack->GetDesc();
  UINT32 sizeInBytes = (UINT32)Desc.Width;
  pData->reset(D.ReadBack, sizeInBytes);
}

static void SetDescriptorHeaps(ID3D12GraphicsCommandList *pList,
                               std::vector<ID3D12DescriptorHeap *> &heaps) {
  if (heaps.empty())
    return;
  std::vector<ID3D12DescriptorHeap *> localHeaps;
  for (auto &H : heaps) {
    if (H->GetDesc().Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) {
      localHeaps.push_back(H);
    }
  }
  if (!localHeaps.empty())
    pList->SetDescriptorHeaps((UINT)localHeaps.size(), localHeaps.data());
}

void ShaderOpTest::RunCommandList() {
  ID3D12GraphicsCommandList *pList = m_CommandList.List.p;
  if (m_pShaderOp->IsCompute()) {
    pList->SetPipelineState(m_pPSO);
    SetDescriptorHeaps(pList, m_DescriptorHeaps);
    pList->SetComputeRootSignature(m_pRootSignature);
    SetRootValues(pList, m_pShaderOp->IsCompute());
    pList->Dispatch(m_pShaderOp->DispatchX, m_pShaderOp->DispatchY,
                    m_pShaderOp->DispatchZ);
  } else {
    pList->SetPipelineState(m_pPSO);
    SetDescriptorHeaps(pList, m_DescriptorHeaps);
    pList->SetGraphicsRootSignature(m_pRootSignature);
    SetRootValues(pList, m_pShaderOp->IsCompute());

    D3D12_VIEWPORT viewport;
    if (!m_pShaderOp->RenderTargets.empty()) {
      // Use the first render target to set up the viewport and scissors.
      ShaderOpRenderTarget &rt = m_pShaderOp->RenderTargets[0];
      ShaderOpResource *R = m_pShaderOp->GetResourceByName(rt.Name);
      if (rt.Viewport.Width > 0 && rt.Viewport.Height > 0) {
        memcpy(&viewport, &rt.Viewport, sizeof(rt.Viewport));
      } else {
        memset(&viewport, 0, sizeof(viewport));
        viewport.Height = (FLOAT)R->Desc.Height;
        viewport.Width = (FLOAT)R->Desc.Width;
        viewport.MaxDepth = 1.0f;
      }
      pList->RSSetViewports(1, &viewport);

      D3D12_RECT scissorRect;
      memset(&scissorRect, 0, sizeof(scissorRect));
      scissorRect.right = (LONG)viewport.Width;
      scissorRect.bottom = (LONG)viewport.Height;
      pList->RSSetScissorRects(1, &scissorRect);
    }

    // Indicate that the buffers will be used as render targets.
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[8];
    UINT rtvHandleCount = (UINT)m_pShaderOp->RenderTargets.size();
    for (size_t i = 0; i < rtvHandleCount; ++i) {
      auto &rt = m_pShaderOp->RenderTargets[i].Name;
      ShaderOpDescriptorData &DData = m_DescriptorData[rt];
      rtvHandles[i] = DData.CPUHandle;
      RecordTransitionBarrier(pList, DData.ResData->Resource,
                              DData.ResData->ResourceState,
                              D3D12_RESOURCE_STATE_RENDER_TARGET);
      DData.ResData->ResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }

    pList->OMSetRenderTargets(rtvHandleCount, rtvHandles, FALSE, nullptr);

    const float ClearColor[4] = {0.0f, 0.2f, 0.4f, 1.0f};
    pList->ClearRenderTargetView(rtvHandles[0], ClearColor, 0, nullptr);

#if defined(NTDDI_WIN10_VB) && WDK_NTDDI_VERSION >= NTDDI_WIN10_VB
    if (m_pShaderOp->MS) {
#ifndef NDEBUG
      D3D12_FEATURE_DATA_D3D12_OPTIONS7 O7;
      DXASSERT_LOCALVAR(
          O7,
          SUCCEEDED(m_pDevice->CheckFeatureSupport(
              (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS7, &O7, sizeof(O7))),
          "mesh shader test enabled on platform without mesh support");
#endif
      CComPtr<ID3D12GraphicsCommandList6> pList6;
      CHECK_HR(m_CommandList.List.p->QueryInterface(&pList6));
      pList6->BeginQuery(m_pQueryHeap, D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0);
      pList6->DispatchMesh(1, 1, 1);
      pList6->EndQuery(m_pQueryHeap, D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0);
      pList6->ResolveQueryData(m_pQueryHeap,
                               D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0, 1,
                               m_pQueryBuffer, 0);
    } else
#endif
    {
      // TODO: set all of this from m_pShaderOp.
      ShaderOpResourceData &VBufferData =
          this->m_ResourceData[m_pShaderOp->Strings.insert("VBuffer")];

      D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
      for (ShaderOpResource &resource : m_pShaderOp->Resources) {
        if (_strcmpi(resource.Name, "VBuffer") == 0) {
          topology = resource.PrimitiveTopology;
          break;
        }
      }
      pList->IASetPrimitiveTopology(topology);

      // Calculate the stride in bytes from the inputs, assuming linear &
      // contiguous.
      UINT strideInBytes = 0;
      for (auto &&IE : m_pShaderOp->InputElements) {
        strideInBytes += GetByteSizeForFormat(IE.Format);
      }

      D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
      vertexBufferView.BufferLocation =
          VBufferData.Resource->GetGPUVirtualAddress();
      vertexBufferView.StrideInBytes = strideInBytes;
      vertexBufferView.SizeInBytes = (UINT)VBufferData.ShaderOpRes->Desc.Width;
      pList->IASetVertexBuffers(0, 1, &vertexBufferView);
      UINT vertexCount =
          vertexBufferView.SizeInBytes / vertexBufferView.StrideInBytes;
      UINT instanceCount = 1;
      UINT vertexCountPerInstance = vertexCount / instanceCount;

      pList->BeginQuery(m_pQueryHeap, D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0);
      pList->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);
      pList->EndQuery(m_pQueryHeap, D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0);
      pList->ResolveQueryData(m_pQueryHeap,
                              D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0, 1,
                              m_pQueryBuffer, 0);
    }
  }
  CHECK_HR(pList->Close());
  ExecuteCommandList(m_CommandList.Queue, pList);
  WaitForSignal(m_CommandList.Queue, m_pFence, m_hFence, m_FenceValue++);
}

void ShaderOpTest::RunShaderOp(ShaderOp *pShaderOp) {
  m_pShaderOp = pShaderOp;

  CreateDevice();
  CreateResources();
  CreateDescriptorHeaps();
  CreatePipelineState();
  CreateCommandList();
  RunCommandList();
  CopyBackResources();
}

void ShaderOpTest::RunShaderOp(std::shared_ptr<ShaderOp> ShaderOp) {
  m_OrigShaderOp = ShaderOp;
  RunShaderOp(m_OrigShaderOp.get());
}

void ShaderOpTest::SetRootValues(ID3D12GraphicsCommandList *pList,
                                 bool isCompute) {
  for (size_t i = 0; i < m_pShaderOp->RootValues.size(); ++i) {
    ShaderOpRootValue &V = m_pShaderOp->RootValues[i];
    UINT idx = V.Index == 0 ? (UINT)i : V.Index;
    if (V.ResName) {
      auto r_it = m_ResourceData.find(V.ResName);
      if (r_it == m_ResourceData.end()) {
        ShaderOpLogFmt(L"Root value #%u refers to missing resource %S",
                       (unsigned)i, V.ResName);
        CHECK_HR(E_INVALIDARG);
      }
      // Issue a warning for trying to bind textures (GPU address will return
      // null)
      ShaderOpResourceData &D = r_it->second;
      ID3D12Resource *pRes = D.Resource;
      if (isCompute) {
        switch (D.ShaderOpRes->TransitionTo) {
        case D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER:
          pList->SetComputeRootConstantBufferView(idx,
                                                  pRes->GetGPUVirtualAddress());
          break;
        case D3D12_RESOURCE_STATE_UNORDERED_ACCESS:
          pList->SetComputeRootUnorderedAccessView(
              idx, pRes->GetGPUVirtualAddress());
          break;
        case D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE:
        default:
          pList->SetComputeRootShaderResourceView(idx,
                                                  pRes->GetGPUVirtualAddress());
          break;
        }
      } else {
        switch (D.ShaderOpRes->TransitionTo) {
        case D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER:
          pList->SetGraphicsRootConstantBufferView(
              idx, pRes->GetGPUVirtualAddress());
          break;
        case D3D12_RESOURCE_STATE_UNORDERED_ACCESS:
          pList->SetGraphicsRootUnorderedAccessView(
              idx, pRes->GetGPUVirtualAddress());
          break;
        case D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE:
        default:
          pList->SetGraphicsRootShaderResourceView(
              idx, pRes->GetGPUVirtualAddress());
          break;
        }
      }
    } else if (V.HeapName) {
      D3D12_GPU_DESCRIPTOR_HANDLE heapBase(
          m_DescriptorHeapsByName[V.HeapName]
              ->GetGPUDescriptorHandleForHeapStart());
      if (isCompute) {
        pList->SetComputeRootDescriptorTable(idx, heapBase);
      } else {
        pList->SetGraphicsRootDescriptorTable(idx, heapBase);
      }
    }
  }
}

void ShaderOpTest::SetDevice(ID3D12Device *pDevice) { m_pDevice = pDevice; }

void ShaderOpTest::SetDxcSupport(dxc::DxcDllSupport *pDxcSupport) {
  m_pDxcSupport = pDxcSupport;
}

void ShaderOpTest::SetInitCallback(TInitCallbackFn InitCallbackFn) {
  m_InitCallbackFn = InitCallbackFn;
}
void ShaderOpTest::SetShaderCallback(TShaderCallbackFn ShaderCallbackFn) {
  m_ShaderCallbackFn = ShaderCallbackFn;
}

void ShaderOpTest::SetupRenderTarget(ShaderOp *pShaderOp, ID3D12Device *pDevice,
                                     ID3D12CommandQueue *pCommandQueue,
                                     ID3D12Resource *pRenderTarget) {
  SetDevice(pDevice);
  m_CommandList.Queue = pCommandQueue;
  // Simplification - add the render target name if missing, set it up 'by hand'
  // if not.
  if (pShaderOp->RenderTargets.empty()) {
    ShaderOpRenderTarget RT = {};
    RT.Name = pShaderOp->Strings.insert("RTarget");
    pShaderOp->RenderTargets.push_back(RT);
    ShaderOpResource R;
    ZeroMemory(&R, sizeof(R));
    R.Desc = pRenderTarget->GetDesc();
    R.Name = pShaderOp->Strings.insert("RTarget");
    R.HeapFlags = D3D12_HEAP_FLAG_NONE;
    R.Init = nullptr;
    R.InitialResourceState = D3D12_RESOURCE_STATE_PRESENT;
    R.ReadBack = FALSE;
    pShaderOp->Resources.push_back(R);

    ShaderOpResourceData &D = m_ResourceData[R.Name];
    D.ShaderOpRes = &pShaderOp->Resources.back();
    D.Resource = pRenderTarget;
    D.ResourceState = R.InitialResourceState;
  }
  // Create a render target heap to put this in.
  ShaderOpDescriptorHeap *pRtvHeap =
      pShaderOp->GetDescriptorHeapByName("RtvHeap");
  if (pRtvHeap == nullptr) {
    ShaderOpDescriptorHeap H = {};
    ZeroMemory(&H, sizeof(H));
    H.Name = pShaderOp->Strings.insert("RtvHeap");
    H.Desc.NumDescriptors = 1;
    H.Desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    pShaderOp->DescriptorHeaps.push_back(H);
    pRtvHeap = &pShaderOp->DescriptorHeaps.back();
  }
  if (pRtvHeap->Descriptors.empty()) {
    ShaderOpDescriptor D = {};
    ZeroMemory(&D, sizeof(D));
    D.Name = pShaderOp->Strings.insert("RTarget");
    D.ResName = D.Name;
    D.Kind = pShaderOp->Strings.insert("RTV");
    pRtvHeap->Descriptors.push_back(D);
  }
}

void ShaderOpTest::PresentRenderTarget(ShaderOp *pShaderOp,
                                       ID3D12CommandQueue *pCommandQueue,
                                       ID3D12Resource *pRenderTarget) {
  UNREFERENCED_PARAMETER(pShaderOp);
  CommandListRefs ResCommandList;
  ResCommandList.Queue = pCommandQueue;
  ResCommandList.CreateForDevice(m_pDevice, m_pShaderOp->IsCompute());
  ID3D12GraphicsCommandList *pList = ResCommandList.List;

  pList->SetName(L"ShaderOpTest Resource Present CommandList");
  RecordTransitionBarrier(pList, pRenderTarget,
                          D3D12_RESOURCE_STATE_RENDER_TARGET,
                          D3D12_RESOURCE_STATE_PRESENT);
  pList->Close();
  ExecuteCommandList(ResCommandList.Queue, pList);
  WaitForSignal(ResCommandList.Queue, m_pFence, m_hFence, m_FenceValue++);
}

ShaderOp *ShaderOpSet::GetShaderOp(LPCSTR pName) {
  for (auto &S : ShaderOps) {
    if (S->Name && 0 == _stricmp(pName, S->Name)) {
      return S.get();
    }
  }
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// ShaderOpTest library implementation for deserialization.

#pragma region Parsing support

// Use this class to initialize a ShaderOp object from an XML document.
class ShaderOpParser {
private:
  string_table *m_pStrings;
  bool ReadAtElementName(IXmlReader *pReader, LPCWSTR pName);
  HRESULT ReadAttrStr(IXmlReader *pReader, LPCWSTR pAttrName, LPCSTR *ppValue);
  HRESULT ReadAttrBOOL(IXmlReader *pReader, LPCWSTR pAttrName, BOOL *pValue,
                       BOOL defaultValue = FALSE);
  HRESULT ReadAttrUINT64(IXmlReader *pReader, LPCWSTR pAttrName, UINT64 *pValue,
                         UINT64 defaultValue = 0);
  HRESULT ReadAttrUINT16(IXmlReader *pReader, LPCWSTR pAttrName, UINT16 *pValue,
                         UINT16 defaultValue = 0);
  HRESULT ReadAttrUINT(IXmlReader *pReader, LPCWSTR pAttrName, UINT *pValue,
                       UINT defaultValue = 0);
  HRESULT ReadAttrFloat(IXmlReader *pReader, LPCWSTR pAttrName, float *pValue,
                        float defaultValue = 0);
  void ReadElementContentStr(IXmlReader *pReader, LPCSTR *ppValue);
  void ParseDescriptor(IXmlReader *pReader, ShaderOpDescriptor *pDesc);
  void ParseDescriptorHeap(IXmlReader *pReader, ShaderOpDescriptorHeap *pHeap);
  void ParseInputElement(IXmlReader *pReader,
                         D3D12_INPUT_ELEMENT_DESC *pInputElement);
  void
  ParseInputElements(IXmlReader *pReader,
                     std::vector<D3D12_INPUT_ELEMENT_DESC> *pInputElements);
  void ParseRenderTargets(IXmlReader *pReader,
                          std::vector<ShaderOpRenderTarget> *pRenderTargets);
  void ParseRenderTarget(IXmlReader *pReader,
                         ShaderOpRenderTarget *pRenderTarget);
  void ParseViewport(IXmlReader *pReader, D3D12_VIEWPORT *pViewport);
  void ParseRootValue(IXmlReader *pReader, ShaderOpRootValue *pRootValue);
  void ParseRootValues(IXmlReader *pReader,
                       std::vector<ShaderOpRootValue> *pRootValues);
  void ParseResource(IXmlReader *pReader, ShaderOpResource *pResource);
  void ParseShader(IXmlReader *pReader, ShaderOpShader *pShader);

public:
  void ParseShaderOpSet(IStream *pStream, ShaderOpSet *pShaderOpSet);
  void ParseShaderOpSet(IXmlReader *pReader, ShaderOpSet *pShaderOpSet);
  void ParseShaderOp(IXmlReader *pReader, ShaderOp *pShaderOp);
};

void ParseShaderOpSetFromStream(IStream *pStream,
                                st::ShaderOpSet *pShaderOpSet) {
  ShaderOpParser parser;
  parser.ParseShaderOpSet(pStream, pShaderOpSet);
}

void ParseShaderOpSetFromXml(IXmlReader *pReader,
                             st::ShaderOpSet *pShaderOpSet) {
  ShaderOpParser parser;
  parser.ParseShaderOpSet(pReader, pShaderOpSet);
}

enum class ParserEnumKind {
  INPUT_CLASSIFICATION,
  DXGI_FORMAT,
  HEAP_TYPE,
  CPU_PAGE_PROPERTY,
  MEMORY_POOL,
  RESOURCE_DIMENSION,
  TEXTURE_LAYOUT,
  RESOURCE_FLAG,
  HEAP_FLAG,
  RESOURCE_STATE,
  DESCRIPTOR_HEAP_TYPE,
  DESCRIPTOR_HEAP_FLAG,
  SRV_DIMENSION,
  UAV_DIMENSION,
  PRIMITIVE_TOPOLOGY,
  PRIMITIVE_TOPOLOGY_TYPE,
  FILTER,
  TEXTURE_ADDRESS_MODE,
  COMPARISON_FUNC,
};

struct ParserEnumValue {
  LPCWSTR Name;
  UINT Value;
};

struct ParserEnumTable {
  size_t ValueCount;
  const ParserEnumValue *Values;
  ParserEnumKind Kind;
};

static const ParserEnumValue INPUT_CLASSIFICATION_TABLE[] = {
    {L"INSTANCE", D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA},
    {L"VERTEX", D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA}};

static const ParserEnumValue DXGI_FORMAT_TABLE[] = {
    {L"UNKNOWN", DXGI_FORMAT_UNKNOWN},
    {L"R32G32B32A32_TYPELESS", DXGI_FORMAT_R32G32B32A32_TYPELESS},
    {L"R32G32B32A32_FLOAT", DXGI_FORMAT_R32G32B32A32_FLOAT},
    {L"R32G32B32A32_UINT", DXGI_FORMAT_R32G32B32A32_UINT},
    {L"R32G32B32A32_SINT", DXGI_FORMAT_R32G32B32A32_SINT},
    {L"R32G32B32_TYPELESS", DXGI_FORMAT_R32G32B32_TYPELESS},
    {L"R32G32B32_FLOAT", DXGI_FORMAT_R32G32B32_FLOAT},
    {L"R32G32B32_UINT", DXGI_FORMAT_R32G32B32_UINT},
    {L"R32G32B32_SINT", DXGI_FORMAT_R32G32B32_SINT},
    {L"R16G16B16A16_TYPELESS", DXGI_FORMAT_R16G16B16A16_TYPELESS},
    {L"R16G16B16A16_FLOAT", DXGI_FORMAT_R16G16B16A16_FLOAT},
    {L"R16G16B16A16_UNORM", DXGI_FORMAT_R16G16B16A16_UNORM},
    {L"R16G16B16A16_UINT", DXGI_FORMAT_R16G16B16A16_UINT},
    {L"R16G16B16A16_SNORM", DXGI_FORMAT_R16G16B16A16_SNORM},
    {L"R16G16B16A16_SINT", DXGI_FORMAT_R16G16B16A16_SINT},
    {L"R32G32_TYPELESS", DXGI_FORMAT_R32G32_TYPELESS},
    {L"R32G32_FLOAT", DXGI_FORMAT_R32G32_FLOAT},
    {L"R32G32_UINT", DXGI_FORMAT_R32G32_UINT},
    {L"R32G32_SINT", DXGI_FORMAT_R32G32_SINT},
    {L"R32G8X24_TYPELESS", DXGI_FORMAT_R32G8X24_TYPELESS},
    {L"D32_FLOAT_S8X24_UINT", DXGI_FORMAT_D32_FLOAT_S8X24_UINT},
    {L"R32_FLOAT_X8X24_TYPELESS", DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS},
    {L"X32_TYPELESS_G8X24_UINT", DXGI_FORMAT_X32_TYPELESS_G8X24_UINT},
    {L"R10G10B10A2_TYPELESS", DXGI_FORMAT_R10G10B10A2_TYPELESS},
    {L"R10G10B10A2_UNORM", DXGI_FORMAT_R10G10B10A2_UNORM},
    {L"R10G10B10A2_UINT", DXGI_FORMAT_R10G10B10A2_UINT},
    {L"R11G11B10_FLOAT", DXGI_FORMAT_R11G11B10_FLOAT},
    {L"R8G8B8A8_TYPELESS", DXGI_FORMAT_R8G8B8A8_TYPELESS},
    {L"R8G8B8A8_UNORM", DXGI_FORMAT_R8G8B8A8_UNORM},
    {L"R8G8B8A8_UNORM_SRGB", DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
    {L"R8G8B8A8_UINT", DXGI_FORMAT_R8G8B8A8_UINT},
    {L"R8G8B8A8_SNORM", DXGI_FORMAT_R8G8B8A8_SNORM},
    {L"R8G8B8A8_SINT", DXGI_FORMAT_R8G8B8A8_SINT},
    {L"R16G16_TYPELESS", DXGI_FORMAT_R16G16_TYPELESS},
    {L"R16G16_FLOAT", DXGI_FORMAT_R16G16_FLOAT},
    {L"R16G16_UNORM", DXGI_FORMAT_R16G16_UNORM},
    {L"R16G16_UINT", DXGI_FORMAT_R16G16_UINT},
    {L"R16G16_SNORM", DXGI_FORMAT_R16G16_SNORM},
    {L"R16G16_SINT", DXGI_FORMAT_R16G16_SINT},
    {L"R32_TYPELESS", DXGI_FORMAT_R32_TYPELESS},
    {L"D32_FLOAT", DXGI_FORMAT_D32_FLOAT},
    {L"R32_FLOAT", DXGI_FORMAT_R32_FLOAT},
    {L"R32_UINT", DXGI_FORMAT_R32_UINT},
    {L"R32_SINT", DXGI_FORMAT_R32_SINT},
    {L"R24G8_TYPELESS", DXGI_FORMAT_R24G8_TYPELESS},
    {L"D24_UNORM_S8_UINT", DXGI_FORMAT_D24_UNORM_S8_UINT},
    {L"R24_UNORM_X8_TYPELESS", DXGI_FORMAT_R24_UNORM_X8_TYPELESS},
    {L"X24_TYPELESS_G8_UINT", DXGI_FORMAT_X24_TYPELESS_G8_UINT},
    {L"R8G8_TYPELESS", DXGI_FORMAT_R8G8_TYPELESS},
    {L"R8G8_UNORM", DXGI_FORMAT_R8G8_UNORM},
    {L"R8G8_UINT", DXGI_FORMAT_R8G8_UINT},
    {L"R8G8_SNORM", DXGI_FORMAT_R8G8_SNORM},
    {L"R8G8_SINT", DXGI_FORMAT_R8G8_SINT},
    {L"R16_TYPELESS", DXGI_FORMAT_R16_TYPELESS},
    {L"R16_FLOAT", DXGI_FORMAT_R16_FLOAT},
    {L"D16_UNORM", DXGI_FORMAT_D16_UNORM},
    {L"R16_UNORM", DXGI_FORMAT_R16_UNORM},
    {L"R16_UINT", DXGI_FORMAT_R16_UINT},
    {L"R16_SNORM", DXGI_FORMAT_R16_SNORM},
    {L"R16_SINT", DXGI_FORMAT_R16_SINT},
    {L"R8_TYPELESS", DXGI_FORMAT_R8_TYPELESS},
    {L"R8_UNORM", DXGI_FORMAT_R8_UNORM},
    {L"R8_UINT", DXGI_FORMAT_R8_UINT},
    {L"R8_SNORM", DXGI_FORMAT_R8_SNORM},
    {L"R8_SINT", DXGI_FORMAT_R8_SINT},
    {L"A8_UNORM", DXGI_FORMAT_A8_UNORM},
    {L"R1_UNORM", DXGI_FORMAT_R1_UNORM},
    {L"R9G9B9E5_SHAREDEXP", DXGI_FORMAT_R9G9B9E5_SHAREDEXP},
    {L"R8G8_B8G8_UNORM", DXGI_FORMAT_R8G8_B8G8_UNORM},
    {L"G8R8_G8B8_UNORM", DXGI_FORMAT_G8R8_G8B8_UNORM},
    {L"BC1_TYPELESS", DXGI_FORMAT_BC1_TYPELESS},
    {L"BC1_UNORM", DXGI_FORMAT_BC1_UNORM},
    {L"BC1_UNORM_SRGB", DXGI_FORMAT_BC1_UNORM_SRGB},
    {L"BC2_TYPELESS", DXGI_FORMAT_BC2_TYPELESS},
    {L"BC2_UNORM", DXGI_FORMAT_BC2_UNORM},
    {L"BC2_UNORM_SRGB", DXGI_FORMAT_BC2_UNORM_SRGB},
    {L"BC3_TYPELESS", DXGI_FORMAT_BC3_TYPELESS},
    {L"BC3_UNORM", DXGI_FORMAT_BC3_UNORM},
    {L"BC3_UNORM_SRGB", DXGI_FORMAT_BC3_UNORM_SRGB},
    {L"BC4_TYPELESS", DXGI_FORMAT_BC4_TYPELESS},
    {L"BC4_UNORM", DXGI_FORMAT_BC4_UNORM},
    {L"BC4_SNORM", DXGI_FORMAT_BC4_SNORM},
    {L"BC5_TYPELESS", DXGI_FORMAT_BC5_TYPELESS},
    {L"BC5_UNORM", DXGI_FORMAT_BC5_UNORM},
    {L"BC5_SNORM", DXGI_FORMAT_BC5_SNORM},
    {L"B5G6R5_UNORM", DXGI_FORMAT_B5G6R5_UNORM},
    {L"B5G5R5A1_UNORM", DXGI_FORMAT_B5G5R5A1_UNORM},
    {L"B8G8R8A8_UNORM", DXGI_FORMAT_B8G8R8A8_UNORM},
    {L"B8G8R8X8_UNORM", DXGI_FORMAT_B8G8R8X8_UNORM},
    {L"R10G10B10_XR_BIAS_A2_UNORM", DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM},
    {L"B8G8R8A8_TYPELESS", DXGI_FORMAT_B8G8R8A8_TYPELESS},
    {L"B8G8R8A8_UNORM_SRGB", DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
    {L"B8G8R8X8_TYPELESS", DXGI_FORMAT_B8G8R8X8_TYPELESS},
    {L"B8G8R8X8_UNORM_SRGB", DXGI_FORMAT_B8G8R8X8_UNORM_SRGB},
    {L"BC6H_TYPELESS", DXGI_FORMAT_BC6H_TYPELESS},
    {L"BC6H_UF16", DXGI_FORMAT_BC6H_UF16},
    {L"BC6H_SF16", DXGI_FORMAT_BC6H_SF16},
    {L"BC7_TYPELESS", DXGI_FORMAT_BC7_TYPELESS},
    {L"BC7_UNORM", DXGI_FORMAT_BC7_UNORM},
    {L"BC7_UNORM_SRGB", DXGI_FORMAT_BC7_UNORM_SRGB},
    {L"AYUV", DXGI_FORMAT_AYUV},
    {L"Y410", DXGI_FORMAT_Y410},
    {L"Y416", DXGI_FORMAT_Y416},
    {L"NV12", DXGI_FORMAT_NV12},
    {L"P010", DXGI_FORMAT_P010},
    {L"P016", DXGI_FORMAT_P016},
    {L"420_OPAQUE", DXGI_FORMAT_420_OPAQUE},
    {L"YUY2", DXGI_FORMAT_YUY2},
    {L"Y210", DXGI_FORMAT_Y210},
    {L"Y216", DXGI_FORMAT_Y216},
    {L"NV11", DXGI_FORMAT_NV11},
    {L"AI44", DXGI_FORMAT_AI44},
    {L"IA44", DXGI_FORMAT_IA44},
    {L"P8", DXGI_FORMAT_P8},
    {L"A8P8", DXGI_FORMAT_A8P8},
    {L"B4G4R4A4_UNORM", DXGI_FORMAT_B4G4R4A4_UNORM},
    {L"P208", DXGI_FORMAT_P208},
    {L"V208", DXGI_FORMAT_V208},
    {L"V408", DXGI_FORMAT_V408}};

static const ParserEnumValue HEAP_TYPE_TABLE[] = {
    {L"DEFAULT", D3D12_HEAP_TYPE_DEFAULT},
    {L"UPLOAD", D3D12_HEAP_TYPE_UPLOAD},
    {L"READBACK", D3D12_HEAP_TYPE_READBACK},
    {L"CUSTOM", D3D12_HEAP_TYPE_CUSTOM}};

static const ParserEnumValue CPU_PAGE_PROPERTY_TABLE[] = {
    {L"UNKNOWN", D3D12_CPU_PAGE_PROPERTY_UNKNOWN},
    {L"NOT_AVAILABLE", D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE},
    {L"WRITE_COMBINE", D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE},
    {L"WRITE_BACK", D3D12_CPU_PAGE_PROPERTY_WRITE_BACK}};

static const ParserEnumValue MEMORY_POOL_TABLE[] = {
    {L"UNKNOWN", D3D12_MEMORY_POOL_UNKNOWN},
    {L"L0 ", D3D12_MEMORY_POOL_L0},
    {L"L1", D3D12_MEMORY_POOL_L1}};

static const ParserEnumValue RESOURCE_DIMENSION_TABLE[] = {
    {L"UNKNOWN", D3D12_RESOURCE_DIMENSION_UNKNOWN},
    {L"BUFFER", D3D12_RESOURCE_DIMENSION_BUFFER},
    {L"TEXTURE1D", D3D12_RESOURCE_DIMENSION_TEXTURE1D},
    {L"TEXTURE2D", D3D12_RESOURCE_DIMENSION_TEXTURE2D},
    {L"TEXTURE3D", D3D12_RESOURCE_DIMENSION_TEXTURE3D}};

static const ParserEnumValue TEXTURE_LAYOUT_TABLE[] = {
    {L"UNKNOWN", D3D12_TEXTURE_LAYOUT_UNKNOWN},
    {L"ROW_MAJOR", D3D12_TEXTURE_LAYOUT_ROW_MAJOR},
    {L"UNDEFINED_SWIZZLE", D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE},
    {L"STANDARD_SWIZZLE", D3D12_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE}};

static const ParserEnumValue RESOURCE_FLAG_TABLE[] = {
    {L"NONE", D3D12_RESOURCE_FLAG_NONE},
    {L"ALLOW_RENDER_TARGET", D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET},
    {L"ALLOW_DEPTH_STENCIL", D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL},
    {L"ALLOW_UNORDERED_ACCESS", D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS},
    {L"DENY_SHADER_RESOURCE", D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE},
    {L"ALLOW_CROSS_ADAPTER", D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER},
    {L"ALLOW_SIMULTANEOUS_ACCESS",
     D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS}};

static const ParserEnumValue HEAP_FLAG_TABLE[] = {
    {L"NONE", D3D12_HEAP_FLAG_NONE},
    {L"SHARED", D3D12_HEAP_FLAG_SHARED},
    {L"DENY_BUFFERS", D3D12_HEAP_FLAG_DENY_BUFFERS},
    {L"ALLOW_DISPLAY", D3D12_HEAP_FLAG_ALLOW_DISPLAY},
    {L"SHARED_CROSS_ADAPTER", D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER},
    {L"DENY_RT_DS_TEXTURES", D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES},
    {L"DENY_NON_RT_DS_TEXTURES", D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES},
    {L"ALLOW_ALL_BUFFERS_AND_TEXTURES",
     D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES},
    {L"ALLOW_ONLY_BUFFERS", D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS},
    {L"ALLOW_ONLY_NON_RT_DS_TEXTURES",
     D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES},
    {L"ALLOW_ONLY_RT_DS_TEXTURES", D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES}};

static const ParserEnumValue RESOURCE_STATE_TABLE[] = {
    {L"COMMON", D3D12_RESOURCE_STATE_COMMON},
    {L"VERTEX_AND_CONSTANT_BUFFER",
     D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER},
    {L"INDEX_BUFFER", D3D12_RESOURCE_STATE_INDEX_BUFFER},
    {L"RENDER_TARGET", D3D12_RESOURCE_STATE_RENDER_TARGET},
    {L"UNORDERED_ACCESS", D3D12_RESOURCE_STATE_UNORDERED_ACCESS},
    {L"DEPTH_WRITE", D3D12_RESOURCE_STATE_DEPTH_WRITE},
    {L"DEPTH_READ", D3D12_RESOURCE_STATE_DEPTH_READ},
    {L"NON_PIXEL_SHADER_RESOURCE",
     D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE},
    {L"PIXEL_SHADER_RESOURCE", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
    {L"STREAM_OUT", D3D12_RESOURCE_STATE_STREAM_OUT},
    {L"INDIRECT_ARGUMENT", D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT},
    {L"COPY_DEST", D3D12_RESOURCE_STATE_COPY_DEST},
    {L"COPY_SOURCE", D3D12_RESOURCE_STATE_COPY_SOURCE},
    {L"RESOLVE_DEST", D3D12_RESOURCE_STATE_RESOLVE_DEST},
    {L"RESOLVE_SOURCE", D3D12_RESOURCE_STATE_RESOLVE_SOURCE},
    {L"GENERIC_READ", D3D12_RESOURCE_STATE_GENERIC_READ},
    {L"PRESENT", D3D12_RESOURCE_STATE_PRESENT},
    {L"PREDICATION", D3D12_RESOURCE_STATE_PREDICATION}};

static const ParserEnumValue DESCRIPTOR_HEAP_TYPE_TABLE[] = {
    {L"CBV_SRV_UAV", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV},
    {L"SAMPLER", D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER},
    {L"RTV", D3D12_DESCRIPTOR_HEAP_TYPE_RTV},
    {L"DSV", D3D12_DESCRIPTOR_HEAP_TYPE_DSV}};

static const ParserEnumValue DESCRIPTOR_HEAP_FLAG_TABLE[] = {
    {L"NONE", D3D12_DESCRIPTOR_HEAP_FLAG_NONE},
    {L"SHADER_VISIBLE", D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE}};

static const ParserEnumValue SRV_DIMENSION_TABLE[] = {
    {L"UNKNOWN", D3D12_SRV_DIMENSION_UNKNOWN},
    {L"BUFFER", D3D12_SRV_DIMENSION_BUFFER},
    {L"TEXTURE1D", D3D12_SRV_DIMENSION_TEXTURE1D},
    {L"TEXTURE1DARRAY", D3D12_SRV_DIMENSION_TEXTURE1DARRAY},
    {L"TEXTURE2D", D3D12_SRV_DIMENSION_TEXTURE2D},
    {L"TEXTURE2DARRAY", D3D12_SRV_DIMENSION_TEXTURE2DARRAY},
    {L"TEXTURE2DMS", D3D12_SRV_DIMENSION_TEXTURE2DMS},
    {L"TEXTURE2DMSARRAY", D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY},
    {L"TEXTURE3D", D3D12_SRV_DIMENSION_TEXTURE3D},
    {L"TEXTURECUBE", D3D12_SRV_DIMENSION_TEXTURECUBE},
    {L"TEXTURECUBEARRAY", D3D12_SRV_DIMENSION_TEXTURECUBEARRAY}};

static const ParserEnumValue UAV_DIMENSION_TABLE[] = {
    {L"UNKNOWN", D3D12_UAV_DIMENSION_UNKNOWN},
    {L"BUFFER", D3D12_UAV_DIMENSION_BUFFER},
    {L"TEXTURE1D", D3D12_UAV_DIMENSION_TEXTURE1D},
    {L"TEXTURE1DARRAY", D3D12_UAV_DIMENSION_TEXTURE1DARRAY},
    {L"TEXTURE2D", D3D12_UAV_DIMENSION_TEXTURE2D},
    {L"TEXTURE2DARRAY", D3D12_UAV_DIMENSION_TEXTURE2DARRAY},
    {L"TEXTURE3D", D3D12_UAV_DIMENSION_TEXTURE3D}};

static const ParserEnumValue PRIMITIVE_TOPOLOGY_TABLE[] = {
    {L"UNDEFINED", D3D_PRIMITIVE_TOPOLOGY_UNDEFINED},
    {L"POINTLIST", D3D_PRIMITIVE_TOPOLOGY_POINTLIST},
    {L"LINELIST", D3D_PRIMITIVE_TOPOLOGY_LINELIST},
    {L"LINESTRIP", D3D_PRIMITIVE_TOPOLOGY_LINESTRIP},
    {L"TRIANGLELIST", D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST},
    {L"TRIANGLESTRIP", D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP},
    {L"LINELIST_ADJ", D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ},
    {L"LINESTRIP_ADJ", D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ},
    {L"TRIANGLELIST_ADJ", D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ},
    {L"TRIANGLESTRIP_ADJ", D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ},
    {L"1_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST},
    {L"2_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST},
    {L"3_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST},
    {L"4_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST},
    {L"5_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST},
    {L"6_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST},
    {L"7_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST},
    {L"8_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST},
    {L"9_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST},
    {L"10_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST},
    {L"11_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST},
    {L"12_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST},
    {L"13_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST},
    {L"14_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST},
    {L"15_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST},
    {L"16_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST},
    {L"17_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST},
    {L"18_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST},
    {L"19_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST},
    {L"20_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST},
    {L"21_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST},
    {L"22_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST},
    {L"23_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST},
    {L"24_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST},
    {L"25_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST},
    {L"26_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST},
    {L"27_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST},
    {L"28_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST},
    {L"29_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST},
    {L"30_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST},
    {L"31_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST},
    {L"32_CONTROL_POINT_PATCHLIST",
     D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST}};

static const ParserEnumValue PRIMITIVE_TOPOLOGY_TYPE_TABLE[] = {
    {L"UNDEFINED", D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED},
    {L"POINT", D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT},
    {L"LINE", D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE},
    {L"TRIANGLE", D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE},
    {L"PATCH", D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH}};

static const ParserEnumValue FILTER_TABLE[] = {
    {L"MIN_MAG_MIP_POINT", D3D12_FILTER_MIN_MAG_MIP_POINT},
    {L"MIN_MAG_POINT_MIP_LINEAR", D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR},
    {L"MIN_POINT_MAG_LINEAR_MIP_POINT",
     D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT},
    {L"MIN_POINT_MAG_MIP_LINEAR", D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR},
    {L"MIN_LINEAR_MAG_MIP_POINT", D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT},
    {L"MIN_LINEAR_MAG_POINT_MIP_LINEAR",
     D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR},
    {L"MIN_MAG_LINEAR_MIP_POINT", D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT},
    {L"MIN_MAG_MIP_LINEAR", D3D12_FILTER_MIN_MAG_MIP_LINEAR},
    {L"ANISOTROPIC", D3D12_FILTER_ANISOTROPIC},
    {L"COMPARISON_MIN_MAG_MIP_POINT",
     D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT},
    {L"COMPARISON_MIN_MAG_POINT_MIP_LINEAR",
     D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR},
    {L"COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT",
     D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT},
    {L"COMPARISON_MIN_POINT_MAG_MIP_LINEAR",
     D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR},
    {L"COMPARISON_MIN_LINEAR_MAG_MIP_POINT",
     D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT},
    {L"COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR",
     D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR},
    {L"COMPARISON_MIN_MAG_LINEAR_MIP_POINT",
     D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT},
    {L"COMPARISON_MIN_MAG_MIP_LINEAR",
     D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR},
    {L"COMPARISON_ANISOTROPIC", D3D12_FILTER_COMPARISON_ANISOTROPIC},
    {L"MINIMUM_MIN_MAG_MIP_POINT", D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT},
    {L"MINIMUM_MIN_MAG_POINT_MIP_LINEAR",
     D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR},
    {L"MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT",
     D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT},
    {L"MINIMUM_MIN_POINT_MAG_MIP_LINEAR",
     D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR},
    {L"MINIMUM_MIN_LINEAR_MAG_MIP_POINT",
     D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT},
    {L"MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR",
     D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR},
    {L"MINIMUM_MIN_MAG_LINEAR_MIP_POINT",
     D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT},
    {L"MINIMUM_MIN_MAG_MIP_LINEAR", D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR},
    {L"MINIMUM_ANISOTROPIC", D3D12_FILTER_MINIMUM_ANISOTROPIC},
    {L"MAXIMUM_MIN_MAG_MIP_POINT", D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT},
    {L"MAXIMUM_MIN_MAG_POINT_MIP_LINEAR",
     D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR},
    {L"MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT",
     D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT},
    {L"MAXIMUM_MIN_POINT_MAG_MIP_LINEAR",
     D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR},
    {L"MAXIMUM_MIN_LINEAR_MAG_MIP_POINT",
     D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT},
    {L"MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR",
     D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR},
    {L"MAXIMUM_MIN_MAG_LINEAR_MIP_POINT",
     D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT},
    {L"MAXIMUM_MIN_MAG_MIP_LINEAR", D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR},
    {L"MAXIMUM_ANISOTROPIC", D3D12_FILTER_MAXIMUM_ANISOTROPIC},
};

static const ParserEnumValue TEXTURE_ADDRESS_MODE_TABLE[] = {
    {L"WRAP", D3D12_TEXTURE_ADDRESS_MODE_WRAP},
    {L"MIRROR", D3D12_TEXTURE_ADDRESS_MODE_MIRROR},
    {L"CLAMP", D3D12_TEXTURE_ADDRESS_MODE_CLAMP},
    {L"BORDER", D3D12_TEXTURE_ADDRESS_MODE_BORDER},
    {L"MIRROR_ONCE", D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE},
};

static const ParserEnumValue COMPARISON_FUNC_TABLE[] = {
    {L"NEVER", D3D12_COMPARISON_FUNC_NEVER},
    {L"LESS", D3D12_COMPARISON_FUNC_LESS},
    {L"EQUAL", D3D12_COMPARISON_FUNC_EQUAL},
    {L"LESS_EQUAL", D3D12_COMPARISON_FUNC_LESS_EQUAL},
    {L"GREATER", D3D12_COMPARISON_FUNC_GREATER},
    {L"NOT_EQUAL", D3D12_COMPARISON_FUNC_NOT_EQUAL},
    {L"GREATER_EQUAL", D3D12_COMPARISON_FUNC_GREATER_EQUAL},
    {L"ALWAYS", D3D12_COMPARISON_FUNC_ALWAYS},
};

static const ParserEnumTable g_ParserEnumTables[] = {
    {_countof(INPUT_CLASSIFICATION_TABLE), INPUT_CLASSIFICATION_TABLE,
     ParserEnumKind::INPUT_CLASSIFICATION},
    {_countof(DXGI_FORMAT_TABLE), DXGI_FORMAT_TABLE,
     ParserEnumKind::DXGI_FORMAT},
    {_countof(HEAP_TYPE_TABLE), HEAP_TYPE_TABLE, ParserEnumKind::HEAP_TYPE},
    {_countof(CPU_PAGE_PROPERTY_TABLE), CPU_PAGE_PROPERTY_TABLE,
     ParserEnumKind::CPU_PAGE_PROPERTY},
    {_countof(MEMORY_POOL_TABLE), MEMORY_POOL_TABLE,
     ParserEnumKind::MEMORY_POOL},
    {_countof(RESOURCE_DIMENSION_TABLE), RESOURCE_DIMENSION_TABLE,
     ParserEnumKind::RESOURCE_DIMENSION},
    {_countof(TEXTURE_LAYOUT_TABLE), TEXTURE_LAYOUT_TABLE,
     ParserEnumKind::TEXTURE_LAYOUT},
    {_countof(RESOURCE_FLAG_TABLE), RESOURCE_FLAG_TABLE,
     ParserEnumKind::RESOURCE_FLAG},
    {_countof(HEAP_FLAG_TABLE), HEAP_FLAG_TABLE, ParserEnumKind::HEAP_FLAG},
    {_countof(RESOURCE_STATE_TABLE), RESOURCE_STATE_TABLE,
     ParserEnumKind::RESOURCE_STATE},
    {_countof(DESCRIPTOR_HEAP_TYPE_TABLE), DESCRIPTOR_HEAP_TYPE_TABLE,
     ParserEnumKind::DESCRIPTOR_HEAP_TYPE},
    {_countof(DESCRIPTOR_HEAP_FLAG_TABLE), DESCRIPTOR_HEAP_FLAG_TABLE,
     ParserEnumKind::DESCRIPTOR_HEAP_FLAG},
    {_countof(SRV_DIMENSION_TABLE), SRV_DIMENSION_TABLE,
     ParserEnumKind::SRV_DIMENSION},
    {_countof(UAV_DIMENSION_TABLE), UAV_DIMENSION_TABLE,
     ParserEnumKind::UAV_DIMENSION},
    {_countof(PRIMITIVE_TOPOLOGY_TABLE), PRIMITIVE_TOPOLOGY_TABLE,
     ParserEnumKind::PRIMITIVE_TOPOLOGY},
    {_countof(PRIMITIVE_TOPOLOGY_TYPE_TABLE), PRIMITIVE_TOPOLOGY_TYPE_TABLE,
     ParserEnumKind::PRIMITIVE_TOPOLOGY_TYPE},
    {_countof(FILTER_TABLE), FILTER_TABLE, ParserEnumKind::FILTER},
    {_countof(TEXTURE_ADDRESS_MODE_TABLE), TEXTURE_ADDRESS_MODE_TABLE,
     ParserEnumKind::TEXTURE_ADDRESS_MODE},
    {_countof(COMPARISON_FUNC_TABLE), COMPARISON_FUNC_TABLE,
     ParserEnumKind::COMPARISON_FUNC},
};

static HRESULT GetEnumValue(LPCWSTR name, ParserEnumKind K, UINT *pValue) {
  for (size_t i = 0; i < _countof(g_ParserEnumTables); ++i) {
    const ParserEnumTable &T = g_ParserEnumTables[i];
    if (T.Kind != K) {
      continue;
    }
    for (size_t j = 0; j < T.ValueCount; ++j) {
      if (_wcsicmp(name, T.Values[j].Name) == 0) {
        *pValue = T.Values[j].Value;
        return S_OK;
      }
    }
  }
  return E_INVALIDARG;
}

template <typename T>
static HRESULT GetEnumValueT(LPCWSTR name, ParserEnumKind K, T *pValue) {
  UINT u;
  HRESULT hr = GetEnumValue(name, K, &u);
  *pValue = (T)u;
  return hr;
}

template <typename T>
static HRESULT ReadAttrEnumT(IXmlReader *pReader, LPCWSTR pAttrName,
                             ParserEnumKind K, T *pValue, T defaultValue,
                             LPCWSTR pStripPrefix = nullptr) {
  if (S_FALSE ==
      CHECK_HR_RET(pReader->MoveToAttributeByName(pAttrName, nullptr))) {
    *pValue = defaultValue;
    return S_FALSE;
  }
  LPCWSTR pText;
  CHECK_HR(pReader->GetValue(&pText, nullptr));
  if (pStripPrefix && *pStripPrefix &&
      _wcsnicmp(pStripPrefix, pText, wcslen(pStripPrefix)) == 0)
    pText += wcslen(pStripPrefix);
  CHECK_HR(GetEnumValueT(pText, K, pValue));
  CHECK_HR(pReader->MoveToElement());
  return S_OK;
}

static HRESULT
ReadAttrINPUT_CLASSIFICATION(IXmlReader *pReader, LPCWSTR pAttrName,
                             D3D12_INPUT_CLASSIFICATION *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::INPUT_CLASSIFICATION,
                       pValue, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);
}

static HRESULT
ReadAttrDESCRIPTOR_HEAP_TYPE(IXmlReader *pReader, LPCWSTR pAttrName,
                             D3D12_DESCRIPTOR_HEAP_TYPE *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::DESCRIPTOR_HEAP_TYPE,
                       pValue, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

static HRESULT
ReadAttrDESCRIPTOR_HEAP_FLAGS(IXmlReader *pReader, LPCWSTR pAttrName,
                              D3D12_DESCRIPTOR_HEAP_FLAGS *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::DESCRIPTOR_HEAP_FLAG,
                       pValue, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
}

static HRESULT ReadAttrDXGI_FORMAT(IXmlReader *pReader, LPCWSTR pAttrName,
                                   DXGI_FORMAT *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::DXGI_FORMAT, pValue,
                       DXGI_FORMAT_UNKNOWN, L"DXGI_FORMAT_");
}

static HRESULT ReadAttrHEAP_TYPE(IXmlReader *pReader, LPCWSTR pAttrName,
                                 D3D12_HEAP_TYPE *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::HEAP_TYPE, pValue,
                       D3D12_HEAP_TYPE_DEFAULT);
}

static HRESULT ReadAttrCPU_PAGE_PROPERTY(IXmlReader *pReader, LPCWSTR pAttrName,
                                         D3D12_CPU_PAGE_PROPERTY *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::CPU_PAGE_PROPERTY,
                       pValue, D3D12_CPU_PAGE_PROPERTY_UNKNOWN);
}

static HRESULT ReadAttrMEMORY_POOL(IXmlReader *pReader, LPCWSTR pAttrName,
                                   D3D12_MEMORY_POOL *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::MEMORY_POOL, pValue,
                       D3D12_MEMORY_POOL_UNKNOWN);
}

static HRESULT ReadAttrRESOURCE_DIMENSION(IXmlReader *pReader,
                                          LPCWSTR pAttrName,
                                          D3D12_RESOURCE_DIMENSION *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::RESOURCE_DIMENSION,
                       pValue, D3D12_RESOURCE_DIMENSION_BUFFER);
}

static HRESULT ReadAttrTEXTURE_LAYOUT(IXmlReader *pReader, LPCWSTR pAttrName,
                                      D3D12_TEXTURE_LAYOUT *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::TEXTURE_LAYOUT,
                       pValue, D3D12_TEXTURE_LAYOUT_UNKNOWN);
}

static HRESULT ReadAttrRESOURCE_FLAGS(IXmlReader *pReader, LPCWSTR pAttrName,
                                      D3D12_RESOURCE_FLAGS *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::RESOURCE_FLAG,
                       pValue, D3D12_RESOURCE_FLAG_NONE);
}

static HRESULT ReadAttrHEAP_FLAGS(IXmlReader *pReader, LPCWSTR pAttrName,
                                  D3D12_HEAP_FLAGS *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::HEAP_FLAG, pValue,
                       D3D12_HEAP_FLAG_NONE);
}

static HRESULT ReadAttrRESOURCE_STATES(IXmlReader *pReader, LPCWSTR pAttrName,
                                       D3D12_RESOURCE_STATES *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::RESOURCE_STATE,
                       pValue, D3D12_RESOURCE_STATE_COMMON);
}

static HRESULT ReadAttrSRV_DIMENSION(IXmlReader *pReader, LPCWSTR pAttrName,
                                     D3D12_SRV_DIMENSION *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::SRV_DIMENSION,
                       pValue, D3D12_SRV_DIMENSION_BUFFER);
}

static HRESULT ReadAttrUAV_DIMENSION(IXmlReader *pReader, LPCWSTR pAttrName,
                                     D3D12_UAV_DIMENSION *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::UAV_DIMENSION,
                       pValue, D3D12_UAV_DIMENSION_BUFFER);
}

static HRESULT ReadAttrPRIMITIVE_TOPOLOGY(IXmlReader *pReader,
                                          LPCWSTR pAttrName,
                                          D3D_PRIMITIVE_TOPOLOGY *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::PRIMITIVE_TOPOLOGY,
                       pValue, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

static HRESULT
ReadAttrPRIMITIVE_TOPOLOGY_TYPE(IXmlReader *pReader, LPCWSTR pAttrName,
                                D3D12_PRIMITIVE_TOPOLOGY_TYPE *pValue) {
  return ReadAttrEnumT(pReader, pAttrName,
                       ParserEnumKind::PRIMITIVE_TOPOLOGY_TYPE, pValue,
                       D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
}

static HRESULT ReadAttrFILTER(IXmlReader *pReader, LPCWSTR pAttrName,
                              D3D12_FILTER *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::FILTER, pValue,
                       D3D12_FILTER_ANISOTROPIC);
}

static HRESULT
ReadAttrTEXTURE_ADDRESS_MODE(IXmlReader *pReader, LPCWSTR pAttrName,
                             D3D12_TEXTURE_ADDRESS_MODE *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::TEXTURE_ADDRESS_MODE,
                       pValue, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
}

static HRESULT ReadAttrCOMPARISON_FUNC(IXmlReader *pReader, LPCWSTR pAttrName,
                                       D3D12_COMPARISON_FUNC *pValue) {
  return ReadAttrEnumT(pReader, pAttrName, ParserEnumKind::COMPARISON_FUNC,
                       pValue, D3D12_COMPARISON_FUNC_LESS_EQUAL);
}

HRESULT ShaderOpParser::ReadAttrStr(IXmlReader *pReader, LPCWSTR pAttrName,
                                    LPCSTR *ppValue) {
  if (S_FALSE ==
      CHECK_HR_RET(pReader->MoveToAttributeByName(pAttrName, nullptr))) {
    *ppValue = nullptr;
    return S_FALSE;
  }
  LPCWSTR pValue;
  CHECK_HR(pReader->GetValue(&pValue, nullptr));
  *ppValue = m_pStrings->insert(pValue);
  CHECK_HR(pReader->MoveToElement());
  return S_OK;
}

HRESULT ShaderOpParser::ReadAttrBOOL(IXmlReader *pReader, LPCWSTR pAttrName,
                                     BOOL *pValue, BOOL defaultValue) {
  if (S_FALSE ==
      CHECK_HR_RET(pReader->MoveToAttributeByName(pAttrName, nullptr))) {
    *pValue = defaultValue;
    return S_FALSE;
  }
  LPCWSTR pText;
  CHECK_HR(pReader->GetValue(&pText, nullptr));
  if (_wcsicmp(pText, L"true") == 0) {
    *pValue = TRUE;
  } else {
    *pValue = FALSE;
  }
  CHECK_HR(pReader->MoveToElement());
  return S_OK;
}

HRESULT ShaderOpParser::ReadAttrUINT64(IXmlReader *pReader, LPCWSTR pAttrName,
                                       UINT64 *pValue, UINT64 defaultValue) {
  if (S_FALSE ==
      CHECK_HR_RET(pReader->MoveToAttributeByName(pAttrName, nullptr))) {
    *pValue = defaultValue;
    return S_FALSE;
  }
  LPCWSTR pText;
  CHECK_HR(pReader->GetValue(&pText, nullptr));
  long long ll = _wtoll(pText);
  if (errno == ERANGE)
    CHECK_HR(E_INVALIDARG);
  *pValue = ll;
  CHECK_HR(pReader->MoveToElement());
  return S_OK;
}

HRESULT ShaderOpParser::ReadAttrUINT(IXmlReader *pReader, LPCWSTR pAttrName,
                                     UINT *pValue, UINT defaultValue) {
  UINT64 u64;
  HRESULT hrRead =
      CHECK_HR_RET(ReadAttrUINT64(pReader, pAttrName, &u64, defaultValue));
  CHECK_HR(UInt64ToUInt(u64, pValue));
  return hrRead;
}

HRESULT ShaderOpParser::ReadAttrUINT16(IXmlReader *pReader, LPCWSTR pAttrName,
                                       UINT16 *pValue, UINT16 defaultValue) {
  UINT64 u64;
  HRESULT hrRead =
      CHECK_HR_RET(ReadAttrUINT64(pReader, pAttrName, &u64, defaultValue));
  CHECK_HR(UInt64ToUInt16(u64, pValue));
  return hrRead;
}

HRESULT ShaderOpParser::ReadAttrFloat(IXmlReader *pReader, LPCWSTR pAttrName,
                                      float *pValue, float defaultValue) {
  if (S_FALSE ==
      CHECK_HR_RET(pReader->MoveToAttributeByName(pAttrName, nullptr))) {
    *pValue = defaultValue;
    return S_FALSE;
  }
  LPCWSTR pText;
  CHECK_HR(pReader->GetValue(&pText, nullptr));
  float d = (float)_wtof(pText);
  if (errno == ERANGE)
    CHECK_HR(E_INVALIDARG);
  *pValue = d;
  CHECK_HR(pReader->MoveToElement());
  return S_OK;
}

void ShaderOpParser::ReadElementContentStr(IXmlReader *pReader,
                                           LPCSTR *ppValue) {
  *ppValue = nullptr;
  if (pReader->IsEmptyElement())
    return;
  UINT startDepth;
  XmlNodeType nt;
  CHECK_HR(pReader->GetDepth(&startDepth));
  std::wstring value;
  for (;;) {
    UINT depth;
    CHECK_HR(pReader->Read(&nt));
    CHECK_HR(pReader->GetDepth(&depth));
    if (nt == XmlNodeType_EndElement && depth == startDepth + 1)
      break;
    if (nt == XmlNodeType_CDATA || nt == XmlNodeType_Text ||
        nt == XmlNodeType_Whitespace) {
      LPCWSTR pText;
      CHECK_HR(pReader->GetValue(&pText, nullptr));
      value += pText;
    }
  }
  *ppValue = m_pStrings->insert(value.c_str());
}

void ShaderOpParser::ParseDescriptor(IXmlReader *pReader,
                                     ShaderOpDescriptor *pDesc) {
  if (!ReadAtElementName(pReader, L"Descriptor"))
    return;
  CHECK_HR(ReadAttrStr(pReader, L"Name", &pDesc->Name));
  CHECK_HR(ReadAttrStr(pReader, L"ResName", &pDesc->ResName));
  CHECK_HR(ReadAttrStr(pReader, L"CounterName", &pDesc->CounterName));
  CHECK_HR(ReadAttrStr(pReader, L"Kind", &pDesc->Kind));
  bool isSRV = pDesc->Kind && 0 == _stricmp(pDesc->Kind, "SRV");
  bool isUAV = pDesc->Kind && 0 == _stricmp(pDesc->Kind, "UAV");
  bool isCBV = pDesc->Kind && 0 == _stricmp(pDesc->Kind, "CBV");
  bool isSAMPLER = pDesc->Kind && 0 == _stricmp(pDesc->Kind, "SAMPLER");
  pDesc->SrvDescPresent = false;
  DXGI_FORMAT *pFormat = nullptr;
  if (isSRV) {
    // D3D12_SHADER_RESOURCE_VIEW_DESC
    pFormat = &pDesc->SrvDesc.Format;
  } else if (isUAV) {
    // D3D12_UNORDERED_ACCESS_VIEW_DESC - default for parsing
    pFormat = &pDesc->UavDesc.Format;
  }
  HRESULT hrFormat = E_FAIL;
  if (isSRV || isUAV) {
    hrFormat = ReadAttrDXGI_FORMAT(pReader, L"Format", pFormat);
    CHECK_HR(hrFormat);
  }
  if (isSRV) {
    pDesc->SrvDesc.Shader4ComponentMapping =
        D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    pDesc->SrvDescPresent |=
        S_OK == CHECK_HR_RET(ReadAttrSRV_DIMENSION(
                    pReader, L"Dimension", &pDesc->SrvDesc.ViewDimension));
    switch (pDesc->SrvDesc.ViewDimension) {
    case D3D12_SRV_DIMENSION_BUFFER:
      pDesc->SrvDescPresent |=
          S_OK ==
          CHECK_HR_RET(ReadAttrUINT64(pReader, L"FirstElement",
                                      &pDesc->SrvDesc.Buffer.FirstElement));
      LPCSTR pFlags;
      pDesc->SrvDescPresent |=
          S_OK == CHECK_HR_RET(ReadAttrStr(pReader, L"Flags", &pFlags));
      if (pFlags && *pFlags && 0 == _stricmp(pFlags, "RAW")) {
        pDesc->SrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
      } else {
        pDesc->SrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
      }
      pDesc->SrvDescPresent |=
          S_OK ==
          CHECK_HR_RET(ReadAttrUINT(pReader, L"NumElements",
                                    &pDesc->SrvDesc.Buffer.NumElements));
      pDesc->SrvDescPresent |=
          S_OK == CHECK_HR_RET(
                      ReadAttrUINT(pReader, L"StructureByteStride",
                                   &pDesc->SrvDesc.Buffer.StructureByteStride));
      break;
    default:
      CHECK_HR(E_NOTIMPL);
    }
  } else if (isUAV) {
    CHECK_HR(ReadAttrUAV_DIMENSION(pReader, L"Dimension",
                                   &pDesc->UavDesc.ViewDimension));
    switch (pDesc->UavDesc.ViewDimension) {
    case D3D12_UAV_DIMENSION_BUFFER:
      CHECK_HR(ReadAttrUINT64(pReader, L"FirstElement",
                              &pDesc->UavDesc.Buffer.FirstElement));
      CHECK_HR(ReadAttrUINT(pReader, L"NumElements",
                            &pDesc->UavDesc.Buffer.NumElements));
      CHECK_HR(ReadAttrUINT(pReader, L"StructureByteStride",
                            &pDesc->UavDesc.Buffer.StructureByteStride));
      CHECK_HR(ReadAttrUINT64(pReader, L"CounterOffsetInBytes",
                              &pDesc->UavDesc.Buffer.CounterOffsetInBytes));
      LPCSTR pFlags;
      CHECK_HR(ReadAttrStr(pReader, L"Flags", &pFlags));
      if (pFlags && *pFlags && 0 == _stricmp(pFlags, "RAW")) {
        pDesc->UavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
      } else {
        pDesc->UavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
      }
      if (hrFormat == S_FALSE &&
          pDesc->UavDesc.Buffer.Flags & D3D12_BUFFER_UAV_FLAG_RAW) {
        pDesc->UavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
      }
      break;
    case D3D12_UAV_DIMENSION_TEXTURE1D:
      CHECK_HR(ReadAttrUINT(pReader, L"MipSlice",
                            &pDesc->UavDesc.Texture1D.MipSlice));
      break;
    case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
      CHECK_HR(ReadAttrUINT(pReader, L"MipSlice",
                            &pDesc->UavDesc.Texture1DArray.MipSlice));
      CHECK_HR(ReadAttrUINT(pReader, L"FirstArraySlice",
                            &pDesc->UavDesc.Texture1DArray.FirstArraySlice));
      CHECK_HR(ReadAttrUINT(pReader, L"ArraySize",
                            &pDesc->UavDesc.Texture1DArray.ArraySize));
      break;
    case D3D12_UAV_DIMENSION_TEXTURE2D:
      CHECK_HR(ReadAttrUINT(pReader, L"MipSlice",
                            &pDesc->UavDesc.Texture2D.MipSlice));
      CHECK_HR(ReadAttrUINT(pReader, L"PlaneSlice",
                            &pDesc->UavDesc.Texture2D.PlaneSlice));
      break;
    case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
      CHECK_HR(ReadAttrUINT(pReader, L"MipSlice",
                            &pDesc->UavDesc.Texture2DArray.MipSlice));
      CHECK_HR(ReadAttrUINT(pReader, L"FirstArraySlice",
                            &pDesc->UavDesc.Texture2DArray.FirstArraySlice));
      CHECK_HR(ReadAttrUINT(pReader, L"ArraySize",
                            &pDesc->UavDesc.Texture2DArray.ArraySize));
      CHECK_HR(ReadAttrUINT(pReader, L"PlaneSlice",
                            &pDesc->UavDesc.Texture2DArray.PlaneSlice));
      break;
    case D3D12_UAV_DIMENSION_TEXTURE3D:
      CHECK_HR(ReadAttrUINT(pReader, L"MipSlice",
                            &pDesc->UavDesc.Texture3D.MipSlice));
      CHECK_HR(ReadAttrUINT(pReader, L"FirstWSlice",
                            &pDesc->UavDesc.Texture3D.FirstWSlice));
      CHECK_HR(
          ReadAttrUINT(pReader, L"WSize", &pDesc->UavDesc.Texture3D.WSize));
      break;
    }
  } else if (isCBV) {
  } else if (isSAMPLER) {
    // Defaults:
    //                [ Filter = FILTER_ANISOTROPIC,
    //                  AddressU = TEXTURE_ADDRESS_WRAP,
    //                  AddressV = TEXTURE_ADDRESS_WRAP,
    //                  AddressW = TEXTURE_ADDRESS_WRAP,
    //                  MipLODBias = 0,
    //                  MaxAnisotropy = 16,
    //                  ComparisonFunc = COMPARISON_LESS_EQUAL,
    //                  BorderColor = STATIC_BORDER_COLOR_OPAQUE_WHITE,
    //                  MinLOD = 0.f,
    //                  MaxLOD = 3.402823466e+38f ] )
    CHECK_HR(ReadAttrFILTER(pReader, L"Filter", &pDesc->SamplerDesc.Filter));
    CHECK_HR(ReadAttrTEXTURE_ADDRESS_MODE(pReader, L"AddressU",
                                          &pDesc->SamplerDesc.AddressU));
    CHECK_HR(ReadAttrTEXTURE_ADDRESS_MODE(pReader, L"AddressV",
                                          &pDesc->SamplerDesc.AddressV));
    CHECK_HR(ReadAttrTEXTURE_ADDRESS_MODE(pReader, L"AddressW",
                                          &pDesc->SamplerDesc.AddressW));
    CHECK_HR(ReadAttrFloat(pReader, L"MipLODBias",
                           &pDesc->SamplerDesc.MipLODBias, 0.0F));
    CHECK_HR(ReadAttrUINT(pReader, L"MaxAnisotropy",
                          &pDesc->SamplerDesc.MaxAnisotropy, 16));
    CHECK_HR(ReadAttrCOMPARISON_FUNC(pReader, L"ComparisonFunc",
                                     &pDesc->SamplerDesc.ComparisonFunc));
    CHECK_HR(ReadAttrFloat(pReader, L"BorderColorR",
                           &pDesc->SamplerDesc.BorderColor[0], 1.0F));
    CHECK_HR(ReadAttrFloat(pReader, L"BorderColorG",
                           &pDesc->SamplerDesc.BorderColor[1], 1.0F));
    CHECK_HR(ReadAttrFloat(pReader, L"BorderColorB",
                           &pDesc->SamplerDesc.BorderColor[2], 1.0F));
    CHECK_HR(ReadAttrFloat(pReader, L"BorderColorA",
                           &pDesc->SamplerDesc.BorderColor[3], 1.0F));
    CHECK_HR(
        ReadAttrFloat(pReader, L"MinLOD", &pDesc->SamplerDesc.MinLOD, 0.0F));
    CHECK_HR(ReadAttrFloat(pReader, L"MaxLOD", &pDesc->SamplerDesc.MaxLOD,
                           3.402823466e+38f));
  }

  // If either is missing, set one from the other.
  if (pDesc->Name && !pDesc->ResName)
    pDesc->ResName = pDesc->Name;
  if (pDesc->ResName && !pDesc->Name)
    pDesc->Name = pDesc->ResName;
  LPCSTR K = pDesc->Kind;
  if (K == nullptr) {
    ShaderOpLogFmt(L"Descriptor '%S' is missing Kind attribute.", pDesc->Name);
    CHECK_HR(E_INVALIDARG);
  } else if (0 != _stricmp(K, "UAV") && 0 != _stricmp(K, "SRV") &&
             0 != _stricmp(K, "CBV") && 0 != _stricmp(K, "RTV") &&
             0 != _stricmp(K, "SAMPLER")) {
    ShaderOpLogFmt(L"Descriptor '%S' references unknown kind '%S'", pDesc->Name,
                   K);
    CHECK_HR(E_INVALIDARG);
  }
}

void ShaderOpParser::ParseDescriptorHeap(IXmlReader *pReader,
                                         ShaderOpDescriptorHeap *pHeap) {
  if (!ReadAtElementName(pReader, L"DescriptorHeap"))
    return;
  CHECK_HR(ReadAttrStr(pReader, L"Name", &pHeap->Name));
  HRESULT hrFlags =
      ReadAttrDESCRIPTOR_HEAP_FLAGS(pReader, L"Flags", &pHeap->Desc.Flags);
  CHECK_HR(hrFlags);
  CHECK_HR(ReadAttrUINT(pReader, L"NodeMask", &pHeap->Desc.NodeMask));
  CHECK_HR(
      ReadAttrUINT(pReader, L"NumDescriptors", &pHeap->Desc.NumDescriptors));
  CHECK_HR(ReadAttrDESCRIPTOR_HEAP_TYPE(pReader, L"Type", &pHeap->Desc.Type));
  if (pHeap->Desc.Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV && hrFlags == S_FALSE)
    pHeap->Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  if (pReader->IsEmptyElement())
    return;

  UINT startDepth;
  XmlNodeType nt;
  CHECK_HR(pReader->GetDepth(&startDepth));
  std::wstring value;
  for (;;) {
    UINT depth;
    CHECK_HR(pReader->Read(&nt));
    CHECK_HR(pReader->GetDepth(&depth));
    if (nt == XmlNodeType_EndElement && depth == startDepth + 1)
      break;
    if (nt == XmlNodeType_Element) {
      LPCWSTR pLocalName;
      CHECK_HR(pReader->GetLocalName(&pLocalName, nullptr));
      if (0 == wcscmp(pLocalName, L"Descriptor")) {
        ShaderOpDescriptor D;
        ParseDescriptor(pReader, &D);
        pHeap->Descriptors.push_back(D);
      }
    }
  }
}

void ShaderOpParser::ParseInputElement(
    IXmlReader *pReader, D3D12_INPUT_ELEMENT_DESC *pInputElement) {
  if (!ReadAtElementName(pReader, L"InputElement"))
    return;
  CHECK_HR(ReadAttrStr(pReader, L"SemanticName", &pInputElement->SemanticName));
  CHECK_HR(
      ReadAttrUINT(pReader, L"SemanticIndex", &pInputElement->SemanticIndex));
  CHECK_HR(ReadAttrDXGI_FORMAT(pReader, L"Format", &pInputElement->Format));
  CHECK_HR(ReadAttrUINT(pReader, L"InputSlot", &pInputElement->InputSlot));
  CHECK_HR(ReadAttrUINT(pReader, L"AlignedByteOffset",
                        &pInputElement->AlignedByteOffset,
                        D3D12_APPEND_ALIGNED_ELEMENT));
  CHECK_HR(ReadAttrINPUT_CLASSIFICATION(pReader, L"InputSlotClass",
                                        &pInputElement->InputSlotClass));
  CHECK_HR(ReadAttrUINT(pReader, L"InstanceDataStepRate",
                        &pInputElement->InstanceDataStepRate));
}

void ShaderOpParser::ParseInputElements(
    IXmlReader *pReader,
    std::vector<D3D12_INPUT_ELEMENT_DESC> *pInputElements) {
  if (!ReadAtElementName(pReader, L"InputElements"))
    return;

  if (pReader->IsEmptyElement())
    return;

  UINT startDepth;
  XmlNodeType nt;
  CHECK_HR(pReader->GetDepth(&startDepth));
  for (;;) {
    UINT depth;
    CHECK_HR(pReader->Read(&nt));
    CHECK_HR(pReader->GetDepth(&depth));
    if (nt == XmlNodeType_EndElement && depth == startDepth + 1)
      return;
    if (nt == XmlNodeType_Element) {
      LPCWSTR pLocalName;
      CHECK_HR(pReader->GetLocalName(&pLocalName, nullptr));
      if (0 == wcscmp(pLocalName, L"InputElement")) {
        D3D12_INPUT_ELEMENT_DESC desc;
        ParseInputElement(pReader, &desc);
        pInputElements->push_back(desc);
      }
    }
  }
}

void ShaderOpParser::ParseRenderTargets(
    IXmlReader *pReader, std::vector<ShaderOpRenderTarget> *pRenderTargets) {
  if (!ReadAtElementName(pReader, L"RenderTargets"))
    return;
  if (pReader->IsEmptyElement())
    return;

  UINT startDepth;
  XmlNodeType nt;
  CHECK_HR(pReader->GetDepth(&startDepth));
  for (;;) {
    UINT depth;
    CHECK_HR(pReader->Read(&nt));
    CHECK_HR(pReader->GetDepth(&depth));
    if (nt == XmlNodeType_EndElement && depth == startDepth + 1)
      return;
    if (nt == XmlNodeType_Element) {
      LPCWSTR pLocalName;
      CHECK_HR(pReader->GetLocalName(&pLocalName, nullptr));
      if (0 == wcscmp(pLocalName, L"RenderTarget")) {
        ShaderOpRenderTarget RT;
        ZeroMemory(&RT, sizeof(RT));
        ParseRenderTarget(pReader, &RT);
        pRenderTargets->push_back(RT);
      }
    }
  }
}

void ShaderOpParser::ParseRenderTarget(IXmlReader *pReader,
                                       ShaderOpRenderTarget *pRenderTarget) {
  if (!ReadAtElementName(pReader, L"RenderTarget"))
    return;

  CHECK_HR(ReadAttrStr(pReader, L"Name", &pRenderTarget->Name));

  if (pReader->IsEmptyElement())
    return;

  UINT startDepth;
  XmlNodeType nt;
  CHECK_HR(pReader->GetDepth(&startDepth));
  for (;;) {
    UINT depth;
    CHECK_HR(pReader->Read(&nt));
    CHECK_HR(pReader->GetDepth(&depth));
    if (nt == XmlNodeType_EndElement && depth == startDepth + 1)
      return;
    if (nt == XmlNodeType_Element) {
      LPCWSTR pLocalName;
      CHECK_HR(pReader->GetLocalName(&pLocalName, nullptr));
      if (0 == wcscmp(pLocalName, L"Viewport")) {
        ParseViewport(pReader, &pRenderTarget->Viewport);
      }
    }
  }
}

void ShaderOpParser::ParseViewport(IXmlReader *pReader,
                                   D3D12_VIEWPORT *pViewport) {
  if (!ReadAtElementName(pReader, L"Viewport"))
    return;

  CHECK_HR(ReadAttrFloat(pReader, L"TopLeftX", &pViewport->TopLeftX));
  CHECK_HR(ReadAttrFloat(pReader, L"TopLeftY", &pViewport->TopLeftY));
  CHECK_HR(ReadAttrFloat(pReader, L"Width", &pViewport->Width));
  CHECK_HR(ReadAttrFloat(pReader, L"Height", &pViewport->Height));
  CHECK_HR(ReadAttrFloat(pReader, L"MinDepth", &pViewport->MinDepth));
  CHECK_HR(ReadAttrFloat(pReader, L"MaxDepth", &pViewport->MaxDepth));

  if (pReader->IsEmptyElement())
    return;

  UINT startDepth;
  XmlNodeType nt;
  CHECK_HR(pReader->GetDepth(&startDepth));
  for (;;) {
    UINT depth;
    CHECK_HR(pReader->Read(&nt));
    CHECK_HR(pReader->GetDepth(&depth));
    if (nt == XmlNodeType_EndElement && depth == startDepth + 1)
      return;
  }
}

void ShaderOpParser::ParseRootValue(IXmlReader *pReader,
                                    ShaderOpRootValue *pRootValue) {
  if (!ReadAtElementName(pReader, L"RootValue"))
    return;
  CHECK_HR(ReadAttrStr(pReader, L"ResName", &pRootValue->ResName));
  CHECK_HR(ReadAttrStr(pReader, L"HeapName", &pRootValue->HeapName));
  CHECK_HR(ReadAttrUINT(pReader, L"Index", &pRootValue->Index));
}

void ShaderOpParser::ParseRootValues(
    IXmlReader *pReader, std::vector<ShaderOpRootValue> *pRootValues) {
  if (!ReadAtElementName(pReader, L"RootValues"))
    return;

  if (pReader->IsEmptyElement())
    return;

  UINT startDepth;
  XmlNodeType nt;
  CHECK_HR(pReader->GetDepth(&startDepth));
  for (;;) {
    UINT depth;
    CHECK_HR(pReader->Read(&nt));
    CHECK_HR(pReader->GetDepth(&depth));
    if (nt == XmlNodeType_EndElement && depth == startDepth + 1)
      return;
    if (nt == XmlNodeType_Element) {
      LPCWSTR pLocalName;
      CHECK_HR(pReader->GetLocalName(&pLocalName, nullptr));
      if (0 == wcscmp(pLocalName, L"RootValue")) {
        ShaderOpRootValue V;
        ParseRootValue(pReader, &V);
        pRootValues->push_back(V);
      }
    }
  }
}

void ShaderOpParser::ParseShaderOpSet(IStream *pStream,
                                      ShaderOpSet *pShaderOpSet) {
  CComPtr<IXmlReader> pReader;
  CHECK_HR(CreateXmlReader(__uuidof(IXmlReader), (void **)&pReader, nullptr));
  CHECK_HR(pReader->SetInput(pStream));
  ParseShaderOpSet(pReader, pShaderOpSet);
}

void ShaderOpParser::ParseShaderOpSet(IXmlReader *pReader,
                                      ShaderOpSet *pShaderOpSet) {
  if (!ReadAtElementName(pReader, L"ShaderOpSet"))
    return;
  UINT startDepth;
  CHECK_HR(pReader->GetDepth(&startDepth));
  XmlNodeType nt = XmlNodeType_Element;
  for (;;) {
    if (nt == XmlNodeType_Element) {
      LPCWSTR pLocalName;
      CHECK_HR(pReader->GetLocalName(&pLocalName, nullptr));
      if (0 == wcscmp(pLocalName, L"ShaderOp")) {
        pShaderOpSet->ShaderOps.emplace_back(std::make_unique<ShaderOp>());
        ParseShaderOp(pReader, pShaderOpSet->ShaderOps.back().get());
      }
    } else if (nt == XmlNodeType_EndElement) {
      UINT depth;
      CHECK_HR(pReader->GetDepth(&depth));
      if (depth == startDepth + 1)
        return;
    }
    CHECK_HR(pReader->Read(&nt));
  }
}

void ShaderOpParser::ParseShaderOp(IXmlReader *pReader, ShaderOp *pShaderOp) {
  m_pStrings = &pShaderOp->Strings;

  // Look for a ShaderOp element.
  if (!ReadAtElementName(pReader, L"ShaderOp"))
    return;
  CHECK_HR(ReadAttrStr(pReader, L"Name", &pShaderOp->Name));
  CHECK_HR(ReadAttrStr(pReader, L"CS", &pShaderOp->CS));
  CHECK_HR(ReadAttrStr(pReader, L"AS", &pShaderOp->AS));
  CHECK_HR(ReadAttrStr(pReader, L"MS", &pShaderOp->MS));
  CHECK_HR(ReadAttrStr(pReader, L"VS", &pShaderOp->VS));
  CHECK_HR(ReadAttrStr(pReader, L"HS", &pShaderOp->HS));
  CHECK_HR(ReadAttrStr(pReader, L"DS", &pShaderOp->DS));
  CHECK_HR(ReadAttrStr(pReader, L"GS", &pShaderOp->GS));
  CHECK_HR(ReadAttrStr(pReader, L"PS", &pShaderOp->PS));
  CHECK_HR(ReadAttrUINT(pReader, L"DispatchX", &pShaderOp->DispatchX, 1));
  CHECK_HR(ReadAttrUINT(pReader, L"DispatchY", &pShaderOp->DispatchY, 1));
  CHECK_HR(ReadAttrUINT(pReader, L"DispatchZ", &pShaderOp->DispatchZ, 1));
  CHECK_HR(ReadAttrPRIMITIVE_TOPOLOGY_TYPE(pReader, L"TopologyType",
                                           &pShaderOp->PrimitiveTopologyType));
  UINT startDepth;
  CHECK_HR(pReader->GetDepth(&startDepth));
  XmlNodeType nt = XmlNodeType_Element;
  for (;;) {
    if (nt == XmlNodeType_Element) {
      LPCWSTR pLocalName;
      CHECK_HR(pReader->GetLocalName(&pLocalName, nullptr));
      if (0 == wcscmp(pLocalName, L"InputElements")) {
        ParseInputElements(pReader, &pShaderOp->InputElements);
      } else if (0 == wcscmp(pLocalName, L"Shader")) {
        ShaderOpShader shader;
        ParseShader(pReader, &shader);
        pShaderOp->Shaders.push_back(shader);
      } else if (0 == wcscmp(pLocalName, L"RootSignature")) {
        ReadElementContentStr(pReader, &pShaderOp->RootSignature);
      } else if (0 == wcscmp(pLocalName, L"RenderTargets")) {
        ParseRenderTargets(pReader, &pShaderOp->RenderTargets);
      } else if (0 == wcscmp(pLocalName, L"Resource")) {
        ShaderOpResource resource;
        ParseResource(pReader, &resource);
        pShaderOp->Resources.push_back(resource);
      } else if (0 == wcscmp(pLocalName, L"DescriptorHeap")) {
        ShaderOpDescriptorHeap heap;
        ParseDescriptorHeap(pReader, &heap);
        pShaderOp->DescriptorHeaps.push_back(heap);
      } else if (0 == wcscmp(pLocalName, L"RootValues")) {
        ParseRootValues(pReader, &pShaderOp->RootValues);
      }
    } else if (nt == XmlNodeType_EndElement) {
      UINT depth;
      CHECK_HR(pReader->GetDepth(&depth));
      if (depth == startDepth + 1)
        return;
    }

    if (S_FALSE == CHECK_HR_RET(pReader->Read(&nt)))
      return;
  }
}

LPCWSTR SkipByteInitSeparators(LPCWSTR pText) {
  while (*pText && (*pText == L' ' || *pText == L'\t' || *pText == L'\r' ||
                    *pText == L'\n' || *pText == L'{' || *pText == L'}' ||
                    *pText == L','))
    ++pText;
  return pText;
}
LPCWSTR FindByteInitSeparators(LPCWSTR pText) {
  while (*pText && !(*pText == L' ' || *pText == L'\t' || *pText == L'\r' ||
                     *pText == L'\n' || *pText == L'{' || *pText == L'}' ||
                     *pText == L','))
    ++pText;
  return pText;
}

using namespace hlsl;

DXIL::ComponentType GetCompType(LPCWSTR pText, LPCWSTR pEnd) {
  // if no prefix shown, use it as a default
  if (pText == pEnd)
    return DXIL::ComponentType::F32;
  // check if suffix starts with (half)
  if (wcsncmp(pText, L"(half)", 6) == 0) {
    return DXIL::ComponentType::F16;
  }
  switch (*(pEnd - 1)) {
  case L'h':
  case L'H':
    return DXIL::ComponentType::F16;
  case L'l':
  case L'L':
    return DXIL::ComponentType::F64;
  case L'u':
  case L'U':
    return DXIL::ComponentType::U32;
  case L'i':
  case L'I':
    return DXIL::ComponentType::I32;
  case L'f':
  case L'F':
  default:
    return DXIL::ComponentType::F32;
  }
}

void ParseDataFromText(LPCWSTR pText, LPCWSTR pEnd,
                       DXIL::ComponentType compType, std::vector<BYTE> &V) {
  BYTE *pB;
  if (compType == DXIL::ComponentType::F16 ||
      compType == DXIL::ComponentType::F32) {
    float fVal;
    size_t wordSize = pEnd - pText;
    if (wordSize >= 3 && 0 == _wcsnicmp(pEnd - 3, L"nan", 3)) {
      fVal = NAN;
    } else if (wordSize >= 4 && 0 == _wcsnicmp(pEnd - 4, L"-inf", 4)) {
      fVal = -(INFINITY);
    } else if ((wordSize >= 3 && 0 == _wcsnicmp(pEnd - 3, L"inf", 3)) ||
               (wordSize >= 4 && 0 == _wcsnicmp(pEnd - 4, L"+inf", 4))) {
      fVal = INFINITY;
    } else if (wordSize >= 7 && 0 == _wcsnicmp(pEnd - 7, L"-denorm", 7)) {
      fVal = -(FLT_MIN / 2);
    } else if (wordSize >= 6 && 0 == _wcsnicmp(pEnd - 6, L"denorm", 6)) {
      fVal = (FLT_MIN / 2);
    } else {
      fVal = wcstof(pText, nullptr);
    }

    if (compType == DXIL::ComponentType::F16) {
      uint16_t fp16Val = ConvertFloat32ToFloat16(fVal);
      pB = (BYTE *)&fp16Val;
      V.insert(V.end(), pB, pB + sizeof(uint16_t));
    } else {
      pB = (BYTE *)&fVal;
      V.insert(V.end(), pB, pB + sizeof(float));
    }
  } else if (compType == DXIL::ComponentType::I32) {
    int val = _wtoi(pText);
    pB = (BYTE *)&val;
    V.insert(V.end(), pB, pB + sizeof(int32_t));
  } else if (compType == DXIL::ComponentType::U32) {
    long long llval = _wtoll(pText);
    if (errno == ERANGE)
      CHECK_HR(E_INVALIDARG);
    unsigned int val = 0;
    if (llval > UINT32_MAX)
      CHECK_HR(E_INVALIDARG);
    val = (unsigned int)llval;
    pB = (BYTE *)&val;
    V.insert(V.end(), pB, pB + sizeof(uint32_t));
  } else {
    DXASSERT_ARGS(false, "Unsupported stream component type : %u", compType);
  }
}

void ShaderOpParser::ParseResource(IXmlReader *pReader,
                                   ShaderOpResource *pResource) {
  if (!ReadAtElementName(pReader, L"Resource"))
    return;
  CHECK_HR(ReadAttrStr(pReader, L"Name", &pResource->Name));
  CHECK_HR(ReadAttrStr(pReader, L"Init", &pResource->Init));
  CHECK_HR(ReadAttrBOOL(pReader, L"ReadBack", &pResource->ReadBack));

  CHECK_HR(
      ReadAttrHEAP_TYPE(pReader, L"HeapType", &pResource->HeapProperties.Type));
  CHECK_HR(ReadAttrCPU_PAGE_PROPERTY(
      pReader, L"CPUPageProperty", &pResource->HeapProperties.CPUPageProperty));
  CHECK_HR(
      ReadAttrMEMORY_POOL(pReader, L"MemoryPoolPreference",
                          &pResource->HeapProperties.MemoryPoolPreference));
  CHECK_HR(ReadAttrUINT(pReader, L"CreationNodeMask",
                        &pResource->HeapProperties.CreationNodeMask));
  CHECK_HR(ReadAttrUINT(pReader, L"VisibleNodeMask",
                        &pResource->HeapProperties.VisibleNodeMask));
  // D3D12_RESOURCE_DESC Desc;
  CHECK_HR(ReadAttrRESOURCE_DIMENSION(pReader, L"Dimension",
                                      &pResource->Desc.Dimension));
  CHECK_HR(ReadAttrUINT64(pReader, L"Alignment", &pResource->Desc.Alignment));
  CHECK_HR(ReadAttrUINT64(pReader, L"Width", &pResource->Desc.Width));
  CHECK_HR(ReadAttrUINT(pReader, L"Height", &pResource->Desc.Height));
  CHECK_HR(ReadAttrUINT16(pReader, L"DepthOrArraySize",
                          &pResource->Desc.DepthOrArraySize));
  CHECK_HR(ReadAttrUINT16(pReader, L"MipLevels", &pResource->Desc.MipLevels));
  CHECK_HR(ReadAttrDXGI_FORMAT(pReader, L"Format", &pResource->Desc.Format));
  CHECK_HR(
      ReadAttrUINT(pReader, L"SampleCount", &pResource->Desc.SampleDesc.Count));
  CHECK_HR(ReadAttrUINT(pReader, L"SampleQual",
                        &pResource->Desc.SampleDesc.Quality));
  CHECK_HR(ReadAttrTEXTURE_LAYOUT(pReader, L"Layout", &pResource->Desc.Layout));
  CHECK_HR(ReadAttrRESOURCE_FLAGS(pReader, L"Flags", &pResource->Desc.Flags));

  CHECK_HR(ReadAttrHEAP_FLAGS(pReader, L"HeapFlags", &pResource->HeapFlags));
  CHECK_HR(ReadAttrRESOURCE_STATES(pReader, L"InitialResourceState",
                                   &pResource->InitialResourceState));
  CHECK_HR(ReadAttrRESOURCE_STATES(pReader, L"TransitionTo",
                                   &pResource->TransitionTo));

  CHECK_HR(ReadAttrPRIMITIVE_TOPOLOGY(pReader, L"Topology",
                                      &pResource->PrimitiveTopology));

  // Set some fixed values.
  if (pResource->Desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
    pResource->Desc.Height = 1;
    pResource->Desc.DepthOrArraySize = 1;
    pResource->Desc.MipLevels = 1;
    pResource->Desc.Format = DXGI_FORMAT_UNKNOWN;
    pResource->Desc.SampleDesc.Count = 1;
    pResource->Desc.SampleDesc.Quality = 0;
    pResource->Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  }
  if (pResource->Desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D) {
    if (pResource->Desc.Height == 0)
      pResource->Desc.Height = 1;
    if (pResource->Desc.DepthOrArraySize == 0)
      pResource->Desc.DepthOrArraySize = 1;
    if (pResource->Desc.SampleDesc.Count == 0)
      pResource->Desc.SampleDesc.Count = 1;
  }
  if (pResource->Desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D) {
    if (pResource->Desc.DepthOrArraySize == 0)
      pResource->Desc.DepthOrArraySize = 1;
    if (pResource->Desc.SampleDesc.Count == 0)
      pResource->Desc.SampleDesc.Count = 1;
  }

  // If the resource has text, that goes into the bytes initialization area.
  if (pReader->IsEmptyElement())
    return;
  std::vector<BYTE> &V = pResource->InitBytes;
  XmlNodeType nt;
  CHECK_HR(pReader->GetNodeType(&nt));
  for (;;) {
    if (nt == XmlNodeType_EndElement) {
      return;
    }
    if (nt == XmlNodeType_Text) {
      // Handle the byte payload. '{', '}', ',', whitespace - these are all
      // separators and are ignored in terms of structure. We simply read
      // literals, figure out their type based on suffix, and write the bytes
      // into the target array.
      LPCWSTR pText;
      pReader->GetValue(&pText, nullptr);
      while (*pText) {
        pText = SkipByteInitSeparators(pText);
        if (!*pText)
          continue;
        LPCWSTR pEnd = FindByteInitSeparators(pText);
        // Consider looking for prefixes/suffixes to handle bases and types.
        DXIL::ComponentType compType = GetCompType(pText, pEnd);
        ParseDataFromText(pText, pEnd, compType, V);
        pText = pEnd;
      }
    }
    if (S_FALSE == CHECK_HR_RET(pReader->Read(&nt)))
      return;
  }
}

void ShaderOpParser::ParseShader(IXmlReader *pReader, ShaderOpShader *pShader) {
  if (!ReadAtElementName(pReader, L"Shader"))
    return;
  CHECK_HR(ReadAttrStr(pReader, L"Name", &pShader->Name));
  CHECK_HR(ReadAttrStr(pReader, L"EntryPoint", &pShader->EntryPoint));
  CHECK_HR(ReadAttrStr(pReader, L"Target", &pShader->Target));
  CHECK_HR(ReadAttrStr(pReader, L"Arguments", &pShader->Arguments));
  CHECK_HR(ReadAttrBOOL(pReader, L"Compiled", &pShader->Compiled));
  CHECK_HR(ReadAttrBOOL(pReader, L"Callback", &pShader->Callback));

  ReadElementContentStr(pReader, &pShader->Text);
  bool hasText = pShader->Text && *pShader->Text;
  if (hasText) {
    LPCSTR pCheck;
    CHECK_HR(ReadAttrStr(pReader, L"Text", &pCheck));
    if (pCheck && *pCheck) {
      ShaderOpLogFmt(L"Shader %S has text content and a Text attribute; it "
                     L"should only have one",
                     pShader->Name);
      CHECK_HR(E_INVALIDARG);
    }
  } else {
    CHECK_HR(ReadAttrStr(pReader, L"Text", &pShader->Text));
  }

  if (pShader->EntryPoint == nullptr)
    pShader->EntryPoint = m_pStrings->insert("main");
}

bool ShaderOpParser::ReadAtElementName(IXmlReader *pReader, LPCWSTR pName) {
  XmlNodeType nt;
  CHECK_HR(pReader->GetNodeType(&nt));
  for (;;) {
    if (nt == XmlNodeType_Element) {
      LPCWSTR pLocalName;
      CHECK_HR(pReader->GetLocalName(&pLocalName, nullptr));
      if (0 == wcscmp(pLocalName, pName)) {
        return true;
      }
    }
    if (S_FALSE == CHECK_HR_RET(pReader->Read(&nt)))
      return false;
  }
}

std::shared_ptr<ShaderOpTestResult>
RunShaderOpTestAfterParse(ID3D12Device *pDevice, dxc::DxcDllSupport &support,
                          LPCSTR pName,
                          st::ShaderOpTest::TInitCallbackFn pInitCallback,
                          st::ShaderOpTest::TShaderCallbackFn pShaderCallback,
                          std::shared_ptr<st::ShaderOpSet> ShaderOpSet) {
  st::ShaderOp *pShaderOp;
  if (pName == nullptr) {
    if (ShaderOpSet->ShaderOps.size() != 1) {
      VERIFY_FAIL(L"Expected a single shader operation.");
    }
    pShaderOp = ShaderOpSet->ShaderOps[0].get();
  } else {
    pShaderOp = ShaderOpSet->GetShaderOp(pName);
  }
  if (pShaderOp == nullptr) {
    std::string msg = "Unable to find shader op ";
    msg += pName;
    msg += "; available ops";
    const char sep = ':';
    for (auto &pAvailOp : ShaderOpSet->ShaderOps) {
      msg += sep;
      msg += pAvailOp->Name ? pAvailOp->Name : "[n/a]";
    }
    CA2W msgWide(msg.c_str());
    VERIFY_FAIL(msgWide.m_psz);
  }

  // This won't actually be used since we're supplying the device,
  // but let's make it consistent.
  pShaderOp->UseWarpDevice = hlsl_test::GetTestParamUseWARP(true);

  std::shared_ptr<st::ShaderOpTest> test = std::make_shared<st::ShaderOpTest>();
  test->SetDxcSupport(&support);
  test->SetInitCallback(pInitCallback);
  test->SetShaderCallback(pShaderCallback);
  test->SetDevice(pDevice);
  test->RunShaderOp(pShaderOp);

  std::shared_ptr<ShaderOpTestResult> result =
      std::make_shared<ShaderOpTestResult>();
  result->ShaderOpSet = ShaderOpSet;
  result->Test = test;
  result->ShaderOp = pShaderOp;
  return result;
}

std::shared_ptr<ShaderOpTestResult>
RunShaderOpTestAfterParse(ID3D12Device *pDevice, dxc::DxcDllSupport &support,
                          LPCSTR pName,
                          st::ShaderOpTest::TInitCallbackFn pInitCallback,
                          std::shared_ptr<st::ShaderOpSet> ShaderOpSet) {
  return RunShaderOpTestAfterParse(pDevice, support, pName, pInitCallback,
                                   nullptr, ShaderOpSet);
}

std::shared_ptr<ShaderOpTestResult>
RunShaderOpTest(ID3D12Device *pDevice, dxc::DxcDllSupport &support,
                IStream *pStream, LPCSTR pName,
                st::ShaderOpTest::TInitCallbackFn pInitCallback) {
  DXASSERT_NOMSG(pStream != nullptr);
  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());
  return RunShaderOpTestAfterParse(pDevice, support, pName, pInitCallback,
                                   ShaderOpSet);
}

#pragma endregion Parsing support

} // namespace st
