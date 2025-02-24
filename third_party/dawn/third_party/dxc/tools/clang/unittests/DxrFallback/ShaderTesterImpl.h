#pragma once
#include "ShaderTester.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class ShaderTesterImpl : public ShaderTester {
public:
  ShaderTesterImpl(const std::wstring &filename);
  ShaderTesterImpl(ID3DBlob *blob);
  virtual ~ShaderTesterImpl();

  void setDevice(const std::wstring &namePrefix) override;
  void runShader(int initialShaderId, const std::vector<int> &input,
                 std::vector<int> &output) override;

private:
  void init();
  void initDevice();
  void initResources();
  void initPipeline();
  void initExecution();

  void printLog(int *log);

  std::wstring m_filename;
  ComPtr<ID3DBlob> m_blob;
  std::wstring m_namePrefix;

  ComPtr<ID3D12Device> m_device;

  ComPtr<ID3D12PipelineState> m_computeState;
  ComPtr<ID3D12RootSignature> m_computeRootSignature;

  // Resources
  int m_bufferSize = 64 * 1024;
  ComPtr<ID3D12Resource> m_input;
  ComPtr<ID3D12Resource> m_output;
  ComPtr<ID3D12Resource> m_one;
  ComPtr<ID3D12Resource> m_uploadInput;
  ComPtr<ID3D12Resource> m_uploadOne;
  ComPtr<ID3D12Resource> m_readback;
  ComPtr<ID3D12DescriptorHeap> m_uavHeap;
  UINT m_uavDescriptorSize;

  // Execution
  ComPtr<ID3D12CommandAllocator> m_commandAllocator;
  ComPtr<ID3D12CommandQueue> m_commandQueue;
  ComPtr<ID3D12GraphicsCommandList> m_commandList;

  // Synchronization objects.
  ComPtr<ID3D12Fence> m_fence;
  HANDLE m_fenceEvent;
  UINT64 m_fenceValue;
};
