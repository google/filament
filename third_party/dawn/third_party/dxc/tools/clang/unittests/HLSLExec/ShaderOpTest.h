///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ShaderOpTest.h                                                            //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides definitions for running tests based on operation descriptions.   //
// The common use case is to build a ShaderOp via the deserialization        //
// functions of by initializing the in-memory representation, then           //
// running it with a ShaderOpTest, and verifying the CPU-mapped              //
// results.                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __SHADEROPTEST_H__
#define __SHADEROPTEST_H__

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <unordered_set>
#include <vector>

// We need to keep & fix these warnings to integrate smoothly with HLK
#pragma warning(error : 4100 4146 4242 4244 4267 4701 4389)

///////////////////////////////////////////////////////////////////////////////
// Forward declarations.
namespace dxc {
class DxcDllSupport;
}
struct IStream;
struct IXmlReader;
struct IDxcBlob;

///////////////////////////////////////////////////////////////////////////////
// Useful helper functions.
UINT GetByteSizeForFormat(DXGI_FORMAT value);
HRESULT LogIfLost(HRESULT hr, ID3D12Device *pDevice);
HRESULT LogIfLost(HRESULT hr, ID3D12Resource *pResource);
void RecordTransitionBarrier(ID3D12GraphicsCommandList *pCommandList,
                             ID3D12Resource *pResource,
                             D3D12_RESOURCE_STATES before,
                             D3D12_RESOURCE_STATES after);
void ExecuteCommandList(ID3D12CommandQueue *pQueue, ID3D12CommandList *pList);
HRESULT SetObjectName(ID3D12Object *pObject, LPCSTR pName);
void WaitForSignal(ID3D12CommandQueue *pCQ, ID3D12Fence *pFence, HANDLE hFence,
                   UINT64 fenceValue);

///////////////////////////////////////////////////////////////////////////////
// Helper class for mapped data.
class MappedData {
private:
  CComPtr<ID3D12Resource> m_pResource;
  void *m_pData;
  UINT32 m_size;

public:
  MappedData() : m_pData(nullptr), m_size(0) {}
  MappedData(ID3D12Resource *pResource, UINT32 sizeInBytes)
      : m_pData(nullptr), m_size(0) {
    reset(pResource, sizeInBytes);
  }
  ~MappedData() { reset(); }
  void *data() { return m_pData; }
  UINT32 size() const { return m_size; }
  void dump() const;
  void reset();
  void reset(ID3D12Resource *pResource, UINT32 sizeInBytes);
};

///////////////////////////////////////////////////////////////////////////////
// ShaderOp library definitions.

namespace st {

typedef void(__stdcall *OutputStringFn)(void *, const wchar_t *);
void SetOutputFn(void *pCtx, OutputStringFn F);

bool UseHardwareDevice(const DXGI_ADAPTER_DESC1 &desc, LPCWSTR AdapterName);
void GetHardwareAdapter(IDXGIFactory2 *pFactory, LPCWSTR AdapterName,
                        IDXGIAdapter1 **ppAdapter);

// String table, used to unique strings.
class string_table {
private:
  struct HashStr {
    size_t operator()(LPCSTR a) const {
#ifdef _HASH_SEQ_DEFINED
      return std::_Hash_seq((const unsigned char *)a, strlen(a));
#else
      return std::_Hash_array_representation((const unsigned char *)a,
                                             strlen(a));
#endif
    }
  };
  struct PredStr {
    bool operator()(LPCSTR a, LPCSTR b) const { return strcmp(a, b) == 0; }
  };

  std::unordered_set<LPCSTR, HashStr, PredStr> m_values;
  std::vector<std::vector<char>> m_strings;

public:
  string_table() {}
  // Disable copy constructor and move constructor.
  string_table(const string_table &) = delete;
  string_table &operator=(const string_table &) = delete;
  string_table(string_table &&) = delete;
  string_table &operator=(string_table &&) = delete;
  LPCSTR insert(LPCSTR pValue);
  LPCSTR insert(LPCWSTR pValue);
};

// Use this class to represent a specific D3D12 resource descriptor.
class ShaderOpDescriptor {
public:
  LPCSTR Name;        // Descriptor name.
  LPCSTR ResName;     // Name of the underlying resource.
  LPCSTR CounterName; // Name of the counter resource, if applicable.
  LPCSTR Kind;        // One of UAV,SRV,CBV
  // Other fields to customize mapping can be added here.
  D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc;
  bool SrvDescPresent;
  D3D12_UNORDERED_ACCESS_VIEW_DESC UavDesc;
  D3D12_SAMPLER_DESC SamplerDesc;
};

// Use this class to represent a heap of D3D12 resource descriptors.
class ShaderOpDescriptorHeap {
public:
  LPCSTR Name;
  D3D12_DESCRIPTOR_HEAP_DESC Desc;
  std::vector<ShaderOpDescriptor> Descriptors;
};

// Use this class to represent a D3D12 resource.
class ShaderOpResource {
public:
  LPCSTR Name;                          // Name for lookups and diagnostics.
  LPCSTR Init;                          // Initialization method.
  D3D12_HEAP_PROPERTIES HeapProperties; // Heap properties for resource.
  D3D12_HEAP_FLAGS HeapFlags;           // Flags.
  D3D12_RESOURCE_DESC Desc;             // Resource description.
  D3D12_RESOURCE_STATES InitialResourceState; // Initial state.
  D3D12_RESOURCE_STATES
      TransitionTo; // State to transition before running shader.
  BOOL ReadBack;    // TRUE to read back to CPU after operations are done.
  std::vector<BYTE> InitBytes;              // Byte payload for initialization.
  D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology; // Primitive topology.
};

// Use this class to represent a shader.
class ShaderOpShader {
public:
  LPCSTR Name;       // Name for lookups and diagnostics.
  LPCSTR EntryPoint; // Entry point function name.
  LPCSTR Target;     // Target profile.
  LPCSTR Text;       // HLSL Shader Text.
  LPCSTR Arguments;  // Command line Arguments.
  LPCSTR Defines;    // HLSL Defines.
  BOOL Compiled;     // Whether text is a base64-encoded value.
  BOOL
      Callback; // Whether a function exists to modify the shader's disassembly.
};

// Use this class to represent a value in the root signature.
class ShaderOpRootValue {
public:
  LPCSTR ResName;  // Resource name.
  LPCSTR HeapName; // Descriptor table, mapping the base of a heap.
  UINT Index;      // Explicit index in root table.
};

// Use this class to represent a render target and its viewport.
class ShaderOpRenderTarget {
public:
  LPCSTR Name; // Render target name
  D3D12_VIEWPORT
      Viewport; // Viewport to use; if Width == 0 use the full render target
};

// Use this class to hold all information needed for a Draw/Dispatch call.
class ShaderOp {
public:
  string_table Strings;
  std::vector<D3D12_INPUT_ELEMENT_DESC> InputElements;
  std::vector<ShaderOpResource> Resources;
  std::vector<ShaderOpDescriptorHeap> DescriptorHeaps;
  std::vector<ShaderOpShader> Shaders;
  std::vector<ShaderOpRootValue> RootValues;
  std::vector<ShaderOpRenderTarget> RenderTargets;
  LPCSTR Name = nullptr;
  LPCSTR RootSignature = nullptr;
  bool UseWarpDevice = true;
  LPCWSTR AdapterName = nullptr;
  LPCSTR CS = nullptr, VS = nullptr, PS = nullptr;
  LPCSTR GS = nullptr, DS = nullptr, HS = nullptr;
  LPCSTR AS = nullptr, MS = nullptr;
  LPCSTR GetString(LPCSTR str) { return Strings.insert(str); }
  UINT DispatchX = 1, DispatchY = 1, DispatchZ = 1;
  D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType =
      D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

  UINT SampleMask = UINT_MAX; // TODO: parse from file
  DXGI_FORMAT RTVFormats[8];  // TODO: parse from file
  bool IsCompute() const { return CS != nullptr; }
  LPCSTR GetShaderText(ShaderOpShader *pShader) {
    if (!pShader || !pShader->Text)
      return nullptr;
    LPCSTR result = pShader->Text;
    if (result[0] == '@') {
      for (auto &&S : Shaders) {
        if (S.Name && 0 == strcmp(S.Name, result + 1))
          return S.Text;
      }
      result = nullptr;
    }
    return result;
  }
  LPCSTR GetShaderArguments(ShaderOpShader *pShader) {
    if (!pShader || !pShader->Arguments) return nullptr;
    LPCSTR result = pShader->Arguments;
    if (result[0] == '@') {
      for (auto && S : Shaders) {
        if (S.Name && 0 == strcmp(S.Name, result + 1))
          return S.Arguments;
      }
      result = nullptr;
    }
    return result;
  }
  ShaderOpDescriptorHeap *GetDescriptorHeapByName(LPCSTR pName) {
    for (auto &&R : DescriptorHeaps) {
      if (R.Name && 0 == strcmp(R.Name, pName))
        return &R;
    }
    return nullptr;
  }
  ShaderOpResource *GetResourceByName(LPCSTR pName) {
    for (auto &&R : Resources) {
      if (R.Name && 0 == strcmp(R.Name, pName))
        return &R;
    }
    return nullptr;
  }
};

// Use this class to hold a set of shader operations.
class ShaderOpSet {
public:
  std::vector<std::unique_ptr<ShaderOp>> ShaderOps;
  ShaderOp *GetShaderOp(LPCSTR pName);
};

// Use this structure to refer to a command allocator/list/queue triple.
struct CommandListRefs {
  CComPtr<ID3D12CommandAllocator> Allocator;
  CComPtr<ID3D12GraphicsCommandList> List;
  CComPtr<ID3D12CommandQueue> Queue;

  void CreateForDevice(ID3D12Device *pDevice, bool compute);
};

// Use this class to run the operation described in a ShaderOp object.
class ShaderOpTest {
public:
  typedef std::function<void(LPCSTR Name, std::vector<BYTE> &Data,
                             ShaderOp *pShaderOp)>
      TInitCallbackFn;
  typedef std::function<void(LPCSTR Name, LPCSTR pText, IDxcBlob **ppShaderBlob,
                             ShaderOp *pShaderOp)>
      TShaderCallbackFn;
  void GetPipelineStats(D3D12_QUERY_DATA_PIPELINE_STATISTICS *pStats);
  void GetReadBackData(LPCSTR pResourceName, MappedData *pData);
  void RunShaderOp(ShaderOp *pShaderOp);
  void RunShaderOp(std::shared_ptr<ShaderOp> pShaderOp);
  void SetDevice(ID3D12Device *pDevice);
  void SetDxcSupport(dxc::DxcDllSupport *pDxcSupport);
  void SetInitCallback(TInitCallbackFn InitCallbackFn);
  void SetShaderCallback(TShaderCallbackFn ShaderCallbackFn);
  void SetupRenderTarget(ShaderOp *pShaderOp, ID3D12Device *pDevice,
                         ID3D12CommandQueue *pCommandQueue,
                         ID3D12Resource *pRenderTarget);
  void PresentRenderTarget(ShaderOp *pShaderOp,
                           ID3D12CommandQueue *pCommandQueue,
                           ID3D12Resource *pRenderTarget);

private:
  struct ShaderOpResourceData {
    ShaderOpResource *ShaderOpRes;
    D3D12_RESOURCE_STATES ResourceState;
    CComPtr<ID3D12Resource> ReadBack;
    CComPtr<ID3D12Resource> Resource;
    CComPtr<ID3D12Resource> View;
  };
  struct ShaderOpDescriptorData {
    ShaderOpDescriptor *Descriptor;
    ShaderOpResourceData *ResData;
    D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle;
  };
  CComPtr<ID3D12Device> m_pDevice;
  CComPtr<ID3D12Fence> m_pFence;
  CComPtr<ID3D12PipelineState> m_pPSO;
  CComPtr<ID3D12RootSignature> m_pRootSignature;
  CComPtr<ID3D12QueryHeap> m_pQueryHeap;
  CComPtr<ID3D12Resource> m_pQueryBuffer;
  dxc::DxcDllSupport *m_pDxcSupport = nullptr;
  CommandListRefs m_CommandList;
  HANDLE m_hFence;
  ShaderOp *m_pShaderOp;
  UINT64 m_FenceValue;
  std::map<LPCSTR, CComPtr<ID3D10Blob>> m_Shaders;
  std::map<LPCSTR, CComPtr<ID3D12DescriptorHeap>> m_DescriptorHeapsByName;
  std::map<LPCSTR, ShaderOpResourceData> m_ResourceData;
  std::map<LPCSTR, ShaderOpDescriptorData> m_DescriptorData;
  std::vector<ID3D12DescriptorHeap *> m_DescriptorHeaps;
  std::shared_ptr<ShaderOp> m_OrigShaderOp;
  TInitCallbackFn m_InitCallbackFn = nullptr;
  TShaderCallbackFn m_ShaderCallbackFn = nullptr;
  void CopyBackResources();
  void CreateCommandList();
  void CreateDescriptorHeaps();
  void CreateDevice();
  void CreatePipelineState();
  void CreateResources();
  void CreateRootSignature();
  void CreateShaders();
  void RunCommandList();
  void SetRootValues(ID3D12GraphicsCommandList *pList, bool isCompute);
};

// Deserialize a ShaderOpSet from a stream.
void ParseShaderOpSetFromStream(IStream *pStream, ShaderOpSet *pShaderOpSet);

// Deserialize a ShaderOpSet from an IXmlReader instance.
void ParseShaderOpSetFromXml(IXmlReader *pReader, ShaderOpSet *pShaderOpSet);

} // namespace st

#endif // __SHADEROPTEST_H__
