
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers.
#endif

#define UNICODE

#include <windows.h>

#include "dxc\Support\d3dx12.h"
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <dxgi1_4.h>

#include <shellapi.h>
#include <string>
#include <wrl.h>

#include "DXSampleHelper.h"
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

#include "ShaderTesterImpl.h"

#include "dxc/Support/dxcapi.use.h"
#include "dxc/dxcapi.h"
#include <atlbase.h>

static dxc::DxCompilerDllLoader g_DxcDllHelper;

#define VERIFY_SUCCEEDED(expr)                                                 \
  {                                                                            \
    HRESULT Result = expr;                                                     \
    if (FAILED(Result)) {                                                      \
      assert(0 && #expr " failed: Result=%08x");                               \
    }                                                                          \
  }

#ifndef DXIL_FOURCC
#define DXIL_FOURCC(ch0, ch1, ch2, ch3)                                        \
  ((uint32_t)(uint8_t)(ch0) | (uint32_t)(uint8_t)(ch1) << 8 |                  \
   (uint32_t)(uint8_t)(ch2) << 16 | (uint32_t)(uint8_t)(ch3) << 24)
#endif

HRESULT D3DCompileToDxilFromFile(LPCWSTR pShaderTextFilePath,
                                 LPCWSTR pEntryPoint, LPCWSTR pTargetProfile,
                                 const DxcDefine *pDefines, UINT32 defineCount,
                                 ID3DBlob **ppBlob,
                                 IDxcBlobEncoding **ppErrorBlob) {
  VERIFY_SUCCEEDED(g_DxcDllHelper.Initialize());
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcBlobEncoding> pTextBlob(nullptr);
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcIncludeHandler> dxcIncludeHandler;
  VERIFY_SUCCEEDED(
      g_DxcDllHelper.CreateInstance(CLSID_DxcCompiler, &pCompiler));
  VERIFY_SUCCEEDED(g_DxcDllHelper.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  VERIFY_SUCCEEDED(pLibrary->CreateIncludeHandler(&dxcIncludeHandler));
  UINT32 codePage(0);
  VERIFY_SUCCEEDED(
      pLibrary->CreateBlobFromFile(pShaderTextFilePath, &codePage, &pTextBlob));
  VERIFY_SUCCEEDED(pCompiler->Compile(
      pTextBlob, pShaderTextFilePath, pEntryPoint, pTargetProfile, nullptr, 0,
      pDefines, defineCount, dxcIncludeHandler, &pResult));
  HRESULT resultCode;
  VERIFY_SUCCEEDED(pResult->GetStatus(&resultCode));
  VERIFY_SUCCEEDED(pResult->GetErrorBuffer(ppErrorBlob));
  // VERIFY_SUCCEEDED(resultCode);
  if (SUCCEEDED(resultCode)) {
    VERIFY_SUCCEEDED(pResult->GetResult((IDxcBlob **)ppBlob));
  }

  return resultCode;
}

// A more recent Windows SDK than currently required is needed for these.
typedef HRESULT(WINAPI *D3D12EnableExperimentalFeaturesFn)(
    UINT NumFeatures, __in_ecount(NumFeatures) const IID *pIIDs,
    __in_ecount_opt(NumFeatures) void *pConfigurationStructs,
    __in_ecount_opt(NumFeatures) UINT *pConfigurationStructSizes);

static const GUID D3D12ExperimentalShaderModelsID =
    {/* 76f5573e-f13a-40f5-b297-81ce9e18933f */
     0x76f5573e,
     0xf13a,
     0x40f5,
     {0xb2, 0x97, 0x81, 0xce, 0x9e, 0x18, 0x93, 0x3f}};

static HRESULT EnableExperimentalShaderModels() {
#if 1
  HMODULE hRuntime = LoadLibraryW(L"d3d12.dll");
  if (hRuntime == NULL) {
    return HRESULT_FROM_WIN32(GetLastError());
  }

  D3D12EnableExperimentalFeaturesFn pD3D12EnableExperimentalFeatures =
      (D3D12EnableExperimentalFeaturesFn)GetProcAddress(
          hRuntime, "D3D12EnableExperimentalFeatures");
  if (pD3D12EnableExperimentalFeatures == nullptr) {
    std::cerr << "Unable to enable experimental shader models\n";
    FreeLibrary(hRuntime);
    return HRESULT_FROM_WIN32(GetLastError());
  }

  HRESULT hr = pD3D12EnableExperimentalFeatures(
      1, &D3D12ExperimentalShaderModelsID, nullptr, nullptr);
  // FreeLibrary(hRuntime);
  return hr;
#else
  HRESULT hr = D3D12EnableExperimentalFeatures(
      1, &D3D12ExperimentalShaderModelsID, nullptr, nullptr);
#endif
}

void GetHardwareAdapter(IDXGIFactory2 *pFactory, IDXGIAdapter1 **ppAdapter,
                        const std::wstring &namePrefix) {
  ComPtr<IDXGIAdapter1> adapter;
  *ppAdapter = nullptr;

  for (UINT adapterIndex = 0;
       DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter);
       ++adapterIndex) {
    DXGI_ADAPTER_DESC1 desc;
    adapter->GetDesc1(&desc);

    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
      // Don't select the Basic Render Driver adapter.
      // If you want a software adapter, pass in "/warp" on the command line.
      continue;
    }

    // Check to see if the adapter supports Direct3D 12, but don't create the
    // actual device yet.
    if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0,
                                    _uuidof(ID3D12Device), nullptr)) &&
        std::wstring(desc.Description).find(namePrefix) != std::wstring::npos) {
      break;
    }
  }

  *ppAdapter = adapter.Detach();
}

void ReadFileToBuffer(const std::wstring &path, std::vector<char> &buffer) {
  std::fstream fs(path, std::ios::binary | std::ios::in);
  if (fs.fail()) {
    std::wcerr << L"Could not open file " << path << L"\n";
    exit(1);
  }

  fs.seekg(0, std::ios::end);
  std::streampos size = fs.tellg();
  fs.seekg(0, std::ios::beg);
  buffer.resize(size, 0);
  if (size)
    fs.read(buffer.data(), buffer.size());
}

ShaderTester *ShaderTester::New(const std::wstring &filename) {
  return new ShaderTesterImpl(filename);
}

ShaderTester *ShaderTester::New(void *blob) {
  return new ShaderTesterImpl((ID3DBlob *)blob);
}

ShaderTesterImpl::ShaderTesterImpl(const std::wstring &filename)
    : m_filename(filename) {}

ShaderTesterImpl::ShaderTesterImpl(ID3DBlob *blob) : m_blob(blob) {}

ShaderTesterImpl::~ShaderTesterImpl() { CloseHandle(m_fenceEvent); }

void ShaderTesterImpl::init() {
  initDevice();
  initResources();
  initPipeline();
  initExecution();
}

void ShaderTesterImpl::initDevice() {
  ThrowIfFailed(EnableExperimentalShaderModels());

#if defined(_DEBUG)
  // Enable the D3D12 debug layer.
  {
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
      debugController->EnableDebugLayer();
    }
  }
#endif

  ComPtr<IDXGIFactory4> factory;
  ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

  if (m_namePrefix == L"WARP") {
    ComPtr<IDXGIAdapter> warpAdapter;
    ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
    ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_12_0,
                                    IID_PPV_ARGS(&m_device)));
  } else {
    ComPtr<IDXGIAdapter1> hardwareAdapter;
    GetHardwareAdapter(factory.Get(), &hardwareAdapter, m_namePrefix);
    ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(),
                                    D3D_FEATURE_LEVEL_12_0,
                                    IID_PPV_ARGS(&m_device)));
  }

  {
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {D3D_SHADER_MODEL_6_0};
    if (FAILED(m_device->CheckFeatureSupport(
            D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))) ||
        shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0) {
      std::cerr << "SM6_0 not supported.\n";
    }
  }
}

void ShaderTesterImpl::initResources() {
  // Create the compute resources
  {
    CD3DX12_HEAP_PROPERTIES heapProps;
    CD3DX12_RESOURCE_DESC resDesc;

    UINT64 bufferSizeInBytes = m_bufferSize * sizeof(int);
    ThrowIfFailed(m_device->CreateCommittedResource(
        &(heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
        D3D12_HEAP_FLAG_NONE,
        &(resDesc = CD3DX12_RESOURCE_DESC::Buffer(
              bufferSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
        IID_PPV_ARGS(&m_input)));
    NAME_D3D12_OBJECT(m_input);

    ThrowIfFailed(m_device->CreateCommittedResource(
        &(heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
        D3D12_HEAP_FLAG_NONE,
        &(resDesc = CD3DX12_RESOURCE_DESC::Buffer(
              bufferSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
        IID_PPV_ARGS(&m_output)));
    NAME_D3D12_OBJECT(m_output);

    ThrowIfFailed(m_device->CreateCommittedResource(
        &(heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
        D3D12_HEAP_FLAG_NONE,
        &(resDesc = CD3DX12_RESOURCE_DESC::Buffer(
              sizeof(int), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_one)));
    NAME_D3D12_OBJECT(m_one);

    ThrowIfFailed(m_device->CreateCommittedResource(
        &(heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
        D3D12_HEAP_FLAG_NONE,
        &(resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSizeInBytes)),
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_uploadInput)));
    NAME_D3D12_OBJECT(m_uploadInput);

    ThrowIfFailed(m_device->CreateCommittedResource(
        &(heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
        D3D12_HEAP_FLAG_NONE,
        &(resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(int))),
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_uploadOne)));
    NAME_D3D12_OBJECT(m_uploadOne);

    ThrowIfFailed(m_device->CreateCommittedResource(
        &(heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK)),
        D3D12_HEAP_FLAG_NONE,
        &(resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSizeInBytes)),
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_readback)));
    NAME_D3D12_OBJECT(m_readback);
  }

  // Create a UAV heap.
  {
    D3D12_DESCRIPTOR_HEAP_DESC uavHeapDesc = {};
    uavHeapDesc.NumDescriptors = 3;
    uavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    uavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(
        m_device->CreateDescriptorHeap(&uavHeapDesc, IID_PPV_ARGS(&m_uavHeap)));

    m_uavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  }

  // Create compute UAVs
  {
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = m_bufferSize;
    uavDesc.Buffer.StructureByteStride = sizeof(int);
    uavDesc.Buffer.CounterOffsetInBytes = 0;
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

    CD3DX12_CPU_DESCRIPTOR_HANDLE uavHandle(
        m_uavHeap->GetCPUDescriptorHandleForHeapStart(), 0,
        m_uavDescriptorSize);
    m_device->CreateUnorderedAccessView(m_input.Get(), nullptr, &uavDesc,
                                        uavHandle);

    uavHandle.Offset(1, m_uavDescriptorSize);
    m_device->CreateUnorderedAccessView(m_output.Get(), nullptr, &uavDesc,
                                        uavHandle);

    uavHandle.Offset(1, m_uavDescriptorSize);
    uavDesc.Buffer.NumElements = 1;
    m_device->CreateUnorderedAccessView(m_one.Get(), nullptr, &uavDesc,
                                        uavHandle);
  }
}

void ShaderTesterImpl::initPipeline() {
  // Compute root signature.
  {
    CD3DX12_ROOT_PARAMETER1 rootParameters[2];
    CD3DX12_DESCRIPTOR_RANGE1 uavs(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3, 0);
    rootParameters[0].InitAsDescriptorTable(1, &uavs); // register u0 : input
                                                       // register u1 : output
                                                       // register u2 : one
    rootParameters[1].InitAsConstants(1, 0); // register b0 : initialStateId

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC computeRootSignatureDesc;
    computeRootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(
        &computeRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature,
        &error));
    ThrowIfFailed(m_device->CreateRootSignature(
        0, signature->GetBufferPointer(), signature->GetBufferSize(),
        IID_PPV_ARGS(&m_computeRootSignature)));
    NAME_D3D12_OBJECT(m_computeRootSignature);
  }

  // Create compute pipeline
  {
    //#if defined(_DEBUG)
    //    // Enable better shader debugging with the graphics debugging tools.
    //    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    //#else
    //    UINT compileFlags = 0;
    //#endif
    // Load and compile shaders.
    // ComPtr<ID3DBlob> computeShader;
    // ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr,
    // "CSMain", "cs_6_0", compileFlags, 0, &computeShader, nullptr));

    CD3DX12_SHADER_BYTECODE bytecode;
    std::vector<char> computeShaderBuffer;
    ComPtr<ID3DBlob> computeShaderBlob;
    if (m_blob) {
      bytecode = CD3DX12_SHADER_BYTECODE(m_blob.Get());
    } else if (m_filename.find(L".cso") != std::wstring::npos) {
      ReadFileToBuffer(m_filename, computeShaderBuffer);
      bytecode = CD3DX12_SHADER_BYTECODE(computeShaderBuffer.data(),
                                         computeShaderBuffer.size());
    } else {
      HRESULT DxilResult(S_OK);
      ComPtr<IDxcBlobEncoding> errors;
      DxilResult =
          D3DCompileToDxilFromFile(m_filename.c_str(), L"CSMain", L"cs_6_0",
                                   nullptr, 0, &computeShaderBlob, &errors);
      if (!SUCCEEDED(DxilResult)) {
        OutputDebugStringA((LPCSTR)errors->GetBufferPointer());
      }
      ThrowIfFailed(DxilResult);
      bytecode = CD3DX12_SHADER_BYTECODE(computeShaderBlob.Get());
    }

    // Describe and create the compute pipeline state object (PSO).
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = m_computeRootSignature.Get();
    computePsoDesc.CS = bytecode;
    ThrowIfFailed(m_device->CreateComputePipelineState(
        &computePsoDesc, IID_PPV_ARGS(&m_computeState)));
    NAME_D3D12_OBJECT(m_computeState);
  }
}

void ShaderTesterImpl::initExecution() {
  // Describe and create the command queue.
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  ThrowIfFailed(
      m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
  ThrowIfFailed(m_device->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

  // Create the command list.
  ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                            m_commandAllocator.Get(), nullptr,
                                            IID_PPV_ARGS(&m_commandList)));

  // Command lists are created in the recording state, but there is nothing
  // to record yet. The main loop expects it to be closed, so close it now.
  ThrowIfFailed(m_commandList->Close());

  // Create synchronization objects
  {
    ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                        IID_PPV_ARGS(&m_fence)));
    m_fenceValue = 1;

    // Create an event handle to use for frame synchronization.
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr) {
      ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }
  }
}

void ShaderTesterImpl::setDevice(const std::wstring &namePrefix) {
  m_namePrefix = namePrefix;
}

void ShaderTesterImpl::runShader(int initialShaderId,
                                 const std::vector<int> &input,
                                 std::vector<int> &output) {
  if (!m_device)
    init();

  //////////////////////////////////////////////////////////////////////////
  // Dispatch compute shader
  //////////////////////////////////////////////////////////////////////////

  // Command list allocators can only be reset when the associated
  // command lists have finished execution on the GPU; apps should use
  // fences to determine GPU execution progress.
  ThrowIfFailed(m_commandAllocator->Reset());

  // However, when ExecuteCommandList() is called on a particular command
  // list, that command list can then be reset at any time and must be before
  // re-recording.
  ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

  m_commandList->SetPipelineState(m_computeState.Get());
  m_commandList->SetComputeRootSignature(m_computeRootSignature.Get());

  ID3D12DescriptorHeap *ppHeaps[] = {m_uavHeap.Get()};
  m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

  CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandle(
      m_uavHeap->GetGPUDescriptorHandleForHeapStart(), 0, m_uavDescriptorSize);
  m_commandList->SetComputeRootDescriptorTable(0, uavHandle);
  m_commandList->SetComputeRoot32BitConstant(1, initialShaderId, 0);

  // Upload some data
  CD3DX12_RANGE readRange(0, 0);
  int *pUpload = nullptr;
  m_uploadInput->Map(0, &readRange, (void **)&pUpload);
  pUpload[0] = 0;
  memcpy(pUpload + 1, input.data(), input.size() * sizeof(int));
  m_uploadInput->Unmap(0, nullptr);

  m_uploadOne->Map(0, &readRange, (void **)&pUpload);
  pUpload[0] = 1;
  m_uploadOne->Unmap(0, nullptr);

  // Copy it to the input buffer
  CD3DX12_RESOURCE_BARRIER bar;
  m_commandList->ResourceBarrier(
      1, &(bar = CD3DX12_RESOURCE_BARRIER::Transition(
               m_input.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
               D3D12_RESOURCE_STATE_COPY_DEST)));
  m_commandList->CopyResource(m_input.Get(), m_uploadInput.Get());
  m_commandList->ResourceBarrier(
      1, &(bar = CD3DX12_RESOURCE_BARRIER::Transition(
               m_input.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
               D3D12_RESOURCE_STATE_UNORDERED_ACCESS)));
  m_commandList->ResourceBarrier(
      1, &(bar = CD3DX12_RESOURCE_BARRIER::Transition(
               m_one.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
               D3D12_RESOURCE_STATE_COPY_DEST)));
  m_commandList->CopyResource(m_one.Get(), m_uploadOne.Get());
  m_commandList->ResourceBarrier(
      1, &(bar = CD3DX12_RESOURCE_BARRIER::Transition(
               m_one.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
               D3D12_RESOURCE_STATE_UNORDERED_ACCESS)));

  m_commandList->Dispatch(1, 1, 1);

  m_commandList->ResourceBarrier(
      1, &(bar = CD3DX12_RESOURCE_BARRIER::Transition(
               m_output.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
               D3D12_RESOURCE_STATE_COPY_SOURCE)));
  m_commandList->CopyResource(m_readback.Get(), m_output.Get());
  m_commandList->ResourceBarrier(
      1, &(bar = CD3DX12_RESOURCE_BARRIER::Transition(
               m_output.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE,
               D3D12_RESOURCE_STATE_UNORDERED_ACCESS)));

  ThrowIfFailed(m_commandList->Close());

  // Execute the command list.
  ID3D12CommandList *ppCommandLists[] = {m_commandList.Get()};
  m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  //////////////////////////////////////////////////////////////////////////
  // Synchronize
  //////////////////////////////////////////////////////////////////////////

  // Signal and increment the fence value.
  const UINT64 oldFenceValue = m_fenceValue;
  ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), oldFenceValue));
  m_fenceValue++;

  // Wait until the previous frame is finished.
  if (m_fence->GetCompletedValue() < oldFenceValue) {
    ThrowIfFailed(m_fence->SetEventOnCompletion(oldFenceValue, m_fenceEvent));
    WaitForSingleObject(m_fenceEvent, INFINITE);
  }

  //////////////////////////////////////////////////////////////////////////
  // Readback
  //////////////////////////////////////////////////////////////////////////
  {
    int *pReadback = nullptr;
    CD3DX12_RANGE readRange(0, m_bufferSize);
    m_readback->Map(0, &readRange, (void **)&pReadback);
    output.assign(pReadback, pReadback + m_bufferSize);
    CD3DX12_RANGE writeRange(0, 0);
    m_readback->Unmap(0, &writeRange);
  }
}

void ShaderTesterImpl::printLog(int *log) {
  int *pos = log;
  int count = pos[0];
  std::cout << count << ": ";
  pos++;

  for (int i = 0; i < count; ++i)
    std::cout << pos[i] << " ";
  std::cout << "\n";
}