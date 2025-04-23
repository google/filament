///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ExecutionTest.cpp                                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// These tests run by executing compiled programs, and thus involve more     //
// moving parts, like the runtime and drivers.                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// We need to keep & fix these warnings to integrate smoothly with HLK
#pragma warning(error : 4100 4146 4242 4244 4267 4701 4389 4018)

// *** THIS FILE CANNOT TAKE ANY LLVM DEPENDENCIES  *** //

// clang-format off
// Includes on Windows are highly order dependent.
#include <algorithm>
#include <memory>
#include <array>
#include <vector>
#include <array>
#include <string>
#include <map>
#include <unordered_set>
#include <sstream>
#include <iomanip>
#include "dxc/Test/CompilationResult.h"
#include "dxc/Test/HLSLTestData.h"
#include <Shlwapi.h>
#include <atlcoll.h>
#include <locale>
#include <algorithm>
#include <type_traits>
#include <bitset>

#undef _read
#include "dxc/Test/DxcTestUtils.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/Test/HlslTestUtils.h"
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/Unicode.h"

//
// d3d12.h and dxgi1_4.h are included in the Windows 10 SDK
// https://msdn.microsoft.com/en-us/library/windows/desktop/dn899120(v=vs.85).aspx
// https://developer.microsoft.com/en-US/windows/downloads/windows-10-sdk
//
#include <d3d12.h>
#include <dxgi1_4.h>
#include <DXGIDebug.h>
#include "dxc/Support/d3dx12.h"
#include <DirectXMath.h>
#include <strsafe.h>
#include <d3dcompiler.h>
#include <wincodec.h>
#include "ShaderOpTest.h"
#include <libloaderapi.h>
#include <DirectXPackedVector.h>
// clang-format on

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "version.lib")

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

// Used to create D3D12SDKConfiguration to enable AgilitySDK programmatically.
typedef HRESULT(WINAPI *D3D12GetInterfaceFn)(REFCLSID rclsid, REFIID riid,
                                             void **ppvDebug);

#ifndef __ID3D12SDKConfiguration_INTERFACE_DEFINED__
// Copied from AgilitySDK D3D12.h to programmatically enable when in developer
// mode.
#define __ID3D12SDKConfiguration_INTERFACE_DEFINED__

EXTERN_C const GUID DECLSPEC_SELECTANY IID_ID3D12SDKConfiguration = {
    0xe9eb5314,
    0x33aa,
    0x42b2,
    {0xa7, 0x18, 0xd7, 0x7f, 0x58, 0xb1, 0xf1, 0xc7}};
EXTERN_C const GUID DECLSPEC_SELECTANY CLSID_D3D12SDKConfiguration = {
    0x7cda6aca,
    0xa03e,
    0x49c8,
    {0x94, 0x58, 0x03, 0x34, 0xd2, 0x0e, 0x07, 0xce}};

MIDL_INTERFACE("e9eb5314-33aa-42b2-a718-d77f58b1f1c7")
ID3D12SDKConfiguration : public IUnknown {
public:
  virtual HRESULT STDMETHODCALLTYPE SetSDKVersion(UINT SDKVersion,
                                                  LPCSTR SDKPath) = 0;
};
#endif /* __ID3D12SDKConfiguration_INTERFACE_DEFINED__ */

using namespace DirectX;
using namespace hlsl_test;

template <typename TSequence, typename T>
static bool contains(TSequence s, const T &val) {
  return std::cend(s) != std::find(std::cbegin(s), std::cend(s), val);
}

template <typename InputIterator, typename T>
static bool contains(InputIterator b, InputIterator e, const T &val) {
  return e != std::find(b, e, val);
}

static HRESULT ReportLiveObjects() {
  CComPtr<IDXGIDebug1> pDebug;
  IFR(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug)));
  IFR(pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL));
  return S_OK;
}

static void WriteInfoQueueMessages(void *pStrCtx,
                                   st::OutputStringFn pOutputStrFn,
                                   ID3D12InfoQueue *pInfoQueue) {
  bool allMessagesOK = true;
  UINT64 count = pInfoQueue->GetNumStoredMessages();
  CAtlArray<BYTE> message;
  for (UINT64 i = 0; i < count; ++i) {
    // 'GetMessageA' rather than 'GetMessage' is an artifact of user32 headers.
    SIZE_T msgLen = 0;
    if (FAILED(pInfoQueue->GetMessageA(i, nullptr, &msgLen))) {
      allMessagesOK = false;
      continue;
    }
    if (message.GetCount() < msgLen) {
      if (!message.SetCount(msgLen)) {
        allMessagesOK = false;
        continue;
      }
    }
    D3D12_MESSAGE *pMessage = (D3D12_MESSAGE *)message.GetData();
    if (FAILED(pInfoQueue->GetMessageA(i, pMessage, &msgLen))) {
      allMessagesOK = false;
      continue;
    }
    CA2W msgW(pMessage->pDescription);
    pOutputStrFn(pStrCtx, msgW.m_psz);
    pOutputStrFn(pStrCtx, L"\r\n");
  }
  if (!allMessagesOK) {
    pOutputStrFn(pStrCtx, L"Failed to retrieve some messages.\r\n");
  }
}

class CComContext {
private:
  bool m_init;

public:
  CComContext() : m_init(false) {}
  ~CComContext() { Dispose(); }
  void Dispose() {
    if (!m_init)
      return;
    m_init = false;
    CoUninitialize();
  }
  HRESULT Init() {
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr)) {
      m_init = true;
    }
    return hr;
  }
};

static void SavePixelsToFile(LPCVOID pPixels, DXGI_FORMAT format,
                             UINT32 m_width, UINT32 m_height,
                             LPCWSTR pFileName) {
  CComContext ctx;
  CComPtr<IWICImagingFactory> pFactory;
  CComPtr<IWICBitmap> pBitmap;
  CComPtr<IWICBitmapEncoder> pEncoder;
  CComPtr<IWICBitmapFrameEncode> pFrameEncode;
  CComPtr<IStream> pStream;

  struct PF {
    DXGI_FORMAT Format;
    GUID PixelFormat;
    UINT32 PixelSize;
    bool operator==(DXGI_FORMAT F) const { return F == Format; }
  } Vals[] = {// Add more pixel format mappings as needed.
              {DXGI_FORMAT_R8G8B8A8_UNORM, GUID_WICPixelFormat32bppRGBA, 4}};
  PF *pFormat = std::find(Vals, Vals + _countof(Vals), format);

  VERIFY_SUCCEEDED(ctx.Init());
  VERIFY_SUCCEEDED(
      CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
                       IID_IWICImagingFactory, (LPVOID *)&pFactory));
  VERIFY_ARE_NOT_EQUAL(pFormat, Vals + _countof(Vals));
  VERIFY_SUCCEEDED(pFactory->CreateBitmapFromMemory(
      m_width, m_height, pFormat->PixelFormat, m_width * pFormat->PixelSize,
      m_width * m_height * pFormat->PixelSize,
      const_cast<BYTE *>((const BYTE *)(pPixels)), &pBitmap));
  VERIFY_SUCCEEDED(
      pFactory->CreateEncoder(GUID_ContainerFormatBmp, nullptr, &pEncoder));
  VERIFY_SUCCEEDED(SHCreateStreamOnFileEx(pFileName, STGM_WRITE, STGM_CREATE, 0,
                                          nullptr, &pStream));
  VERIFY_SUCCEEDED(pEncoder->Initialize(pStream, WICBitmapEncoderNoCache));
  VERIFY_SUCCEEDED(pEncoder->CreateNewFrame(&pFrameEncode, nullptr));
  VERIFY_SUCCEEDED(pFrameEncode->Initialize(nullptr));
  VERIFY_SUCCEEDED(pFrameEncode->WriteSource(pBitmap, nullptr));
  VERIFY_SUCCEEDED(pFrameEncode->Commit());
  VERIFY_SUCCEEDED(pEncoder->Commit());
  VERIFY_SUCCEEDED(pStream->Commit(STGC_DEFAULT));
}

#if WDK_NTDDI_VERSION <= NTDDI_WIN10_RS2
#define D3D12_FEATURE_D3D12_OPTIONS3 ((D3D12_FEATURE)21)
#define NTDDI_WIN10_RS3 0x0A000004 /* ABRACADABRA_WIN10_RS2 */
typedef enum D3D12_COMMAND_LIST_SUPPORT_FLAGS {
  D3D12_COMMAND_LIST_SUPPORT_FLAG_NONE = 0,
  D3D12_COMMAND_LIST_SUPPORT_FLAG_DIRECT =
      (1 << D3D12_COMMAND_LIST_TYPE_DIRECT),
  D3D12_COMMAND_LIST_SUPPORT_FLAG_BUNDLE =
      (1 << D3D12_COMMAND_LIST_TYPE_BUNDLE),
  D3D12_COMMAND_LIST_SUPPORT_FLAG_COMPUTE =
      (1 << D3D12_COMMAND_LIST_TYPE_COMPUTE),
  D3D12_COMMAND_LIST_SUPPORT_FLAG_COPY = (1 << D3D12_COMMAND_LIST_TYPE_COPY),
  D3D12_COMMAND_LIST_SUPPORT_FLAG_VIDEO_DECODE = (1 << 4),
  D3D12_COMMAND_LIST_SUPPORT_FLAG_VIDEO_PROCESS = (1 << 5)
} D3D12_COMMAND_LIST_SUPPORT_FLAGS;

typedef enum D3D12_VIEW_INSTANCING_TIER {
  D3D12_VIEW_INSTANCING_TIER_NOT_SUPPORTED = 0,
  D3D12_VIEW_INSTANCING_TIER_1 = 1,
  D3D12_VIEW_INSTANCING_TIER_2 = 2,
  D3D12_VIEW_INSTANCING_TIER_3 = 3
} D3D12_VIEW_INSTANCING_TIER;

typedef struct D3D12_FEATURE_DATA_D3D12_OPTIONS3 {
  BOOL CopyQueueTimestampQueriesSupported;
  BOOL CastingFullyTypedFormatSupported;
  DWORD WriteBufferImmediateSupportFlags;
  D3D12_VIEW_INSTANCING_TIER ViewInstancingTier;
  BOOL BarycentricsSupported;
} D3D12_FEATURE_DATA_D3D12_OPTIONS3;
#endif

#if WDK_NTDDI_VERSION <= NTDDI_WIN10_RS3
#define D3D12_FEATURE_D3D12_OPTIONS4 ((D3D12_FEATURE)23)
typedef enum D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER {
  D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER_0,
  D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER_1,
} D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER;

typedef struct D3D12_FEATURE_DATA_D3D12_OPTIONS4 {
  BOOL ReservedBufferPlacementSupported;
  3D12_SHARED_RESOURCE_COMPATIBILITY_TIER SharedResourceCompatibilityTier;
  BOOL Native16BitShaderOpsSupported;
} D3D12_FEATURE_DATA_D3D12_OPTIONS4;

#endif

// Virtual class to compute the expected result given a set of inputs
struct TableParameter;

class ExecutionTest {
public:
  BEGIN_TEST_CLASS(ExecutionTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()
  TEST_CLASS_SETUP(ExecutionTestClassSetup)

  TEST_METHOD(BasicComputeTest);
  TEST_METHOD(BasicTriangleTest);
  TEST_METHOD(BasicTriangleOpTest);

  TEST_METHOD(BasicTriangleOpTestHalf);

  TEST_METHOD(OutOfBoundsTest);
  TEST_METHOD(SaturateTest);
  TEST_METHOD(SignTest);
  TEST_METHOD(Int64Test);
  TEST_METHOD(LifetimeIntrinsicTest)
  TEST_METHOD(WaveIntrinsicsTest);
  TEST_METHOD(WaveIntrinsicsDDITest);
  TEST_METHOD(WaveIntrinsicsInPSTest);
  TEST_METHOD(WaveSizeTest);
  TEST_METHOD(WaveSizeRangeTest);
  TEST_METHOD(PartialDerivTest);
  TEST_METHOD(DerivativesTest);
  TEST_METHOD(ComputeSampleTest);
  TEST_METHOD(ATOProgOffset);
  TEST_METHOD(ATOSampleCmpLevelTest);
  TEST_METHOD(ATOWriteMSAATest);
  TEST_METHOD(ATORawGather);
  TEST_METHOD(AtomicsTest);
  TEST_METHOD(Atomics64Test);
  TEST_METHOD(AtomicsRawHeap64Test);
  TEST_METHOD(AtomicsTyped64Test);
  TEST_METHOD(AtomicsShared64Test);
  TEST_METHOD(AtomicsFloatTest);
  TEST_METHOD(HelperLaneTest);
  TEST_METHOD(HelperLaneTestWave);
  TEST_METHOD(SignatureResourcesTest)
  TEST_METHOD(DynamicResourcesTest)
  TEST_METHOD(DynamicResourcesDynamicIndexingTest)

  TEST_METHOD(QuadReadTest)
  TEST_METHOD(QuadAnyAll);

  TEST_METHOD(CBufferTestHalf);

  TEST_METHOD(BasicShaderModel61);
  TEST_METHOD(BasicShaderModel63);

  BEGIN_TEST_METHOD(WaveIntrinsicsActiveIntTest)
  TEST_METHOD_PROPERTY(
      L"DataSource",
      L"Table:ShaderOpArithTable.xml#WaveIntrinsicsActiveIntTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(WaveIntrinsicsActiveUintTest)
  TEST_METHOD_PROPERTY(
      L"DataSource",
      L"Table:ShaderOpArithTable.xml#WaveIntrinsicsActiveUintTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(WaveIntrinsicsPrefixIntTest)
  TEST_METHOD_PROPERTY(
      L"DataSource",
      L"Table:ShaderOpArithTable.xml#WaveIntrinsicsPrefixIntTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(WaveIntrinsicsPrefixUintTest)
  TEST_METHOD_PROPERTY(
      L"DataSource",
      L"Table:ShaderOpArithTable.xml#WaveIntrinsicsPrefixUintTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(WaveIntrinsicsSM65IntTest)
  TEST_METHOD_PROPERTY(
      L"DataSource",
      L"Table:ShaderOpArithTable.xml#WaveIntrinsicsMultiPrefixIntTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(WaveIntrinsicsSM65UintTest)
  TEST_METHOD_PROPERTY(
      L"DataSource",
      L"Table:ShaderOpArithTable.xml#WaveIntrinsicsMultiPrefixUintTable")
  END_TEST_METHOD()

  // TAEF data-driven tests.
  BEGIN_TEST_METHOD(UnaryFloatOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#UnaryFloatOpTable")
  END_TEST_METHOD()
  BEGIN_TEST_METHOD(BinaryFloatOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#BinaryFloatOpTable")
  END_TEST_METHOD()
  BEGIN_TEST_METHOD(TertiaryFloatOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#TertiaryFloatOpTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(UnaryHalfOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#UnaryHalfOpTable")
  END_TEST_METHOD()
  BEGIN_TEST_METHOD(BinaryHalfOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#BinaryHalfOpTable")
  END_TEST_METHOD()
  BEGIN_TEST_METHOD(TertiaryHalfOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#TertiaryHalfOpTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(UnaryIntOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#UnaryIntOpTable")
  END_TEST_METHOD()
  BEGIN_TEST_METHOD(BinaryIntOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#BinaryIntOpTable")
  END_TEST_METHOD()
  BEGIN_TEST_METHOD(TertiaryIntOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#TertiaryIntOpTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(UnaryUintOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#UnaryUintOpTable")
  END_TEST_METHOD()
  BEGIN_TEST_METHOD(BinaryUintOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#BinaryUintOpTable")
  END_TEST_METHOD()
  BEGIN_TEST_METHOD(TertiaryUintOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#TertiaryUintOpTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(UnaryInt16OpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#UnaryInt16OpTable")
  END_TEST_METHOD()
  BEGIN_TEST_METHOD(BinaryInt16OpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#BinaryInt16OpTable")
  END_TEST_METHOD()
  BEGIN_TEST_METHOD(TertiaryInt16OpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#TertiaryInt16OpTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(UnaryUint16OpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#UnaryUint16OpTable")
  END_TEST_METHOD()
  BEGIN_TEST_METHOD(BinaryUint16OpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#BinaryUint16OpTable")
  END_TEST_METHOD()
  BEGIN_TEST_METHOD(TertiaryUint16OpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#TertiaryUint16OpTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(DotTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#DotOpTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(Dot2AddHalfTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#Dot2AddHalfOpTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(Dot4AddI8PackedTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#Dot4AddI8PackedOpTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(Dot4AddU8PackedTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#Dot4AddU8PackedOpTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(Msad4Test)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#Msad4Table")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(DenormBinaryFloatOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#DenormBinaryFloatOpTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(DenormTertiaryFloatOpTest)
  TEST_METHOD_PROPERTY(
      L"DataSource", L"Table:ShaderOpArithTable.xml#DenormTertiaryFloatOpTable")
  END_TEST_METHOD()

  TEST_METHOD(BarycentricsTest);

  TEST_METHOD(ComputeRawBufferLdStI32);
  TEST_METHOD(ComputeRawBufferLdStFloat);

  TEST_METHOD(ComputeRawBufferLdStI64);
  TEST_METHOD(ComputeRawBufferLdStDouble);

  TEST_METHOD(ComputeRawBufferLdStI16);
  TEST_METHOD(ComputeRawBufferLdStHalf);

  TEST_METHOD(GraphicsRawBufferLdStI32);
  TEST_METHOD(GraphicsRawBufferLdStFloat);

  TEST_METHOD(GraphicsRawBufferLdStI64);
  TEST_METHOD(GraphicsRawBufferLdStDouble);

  TEST_METHOD(GraphicsRawBufferLdStI16);
  TEST_METHOD(GraphicsRawBufferLdStHalf);
  TEST_METHOD(IsNormalTest);

  BEGIN_TEST_METHOD(PackUnpackTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:ShaderOpArithTable.xml#PackUnpackOpTable")
  END_TEST_METHOD()

  dxc::DxcDllSupport m_support;

  bool m_D3DInitCompleted = false;
  bool m_ExperimentalModeEnabled = false;
  bool m_AgilitySDKEnabled = false;

  const float ClearColor[4] = {0.0f, 0.2f, 0.4f, 1.0f};

  bool DivergentClassSetup() {
    // Run this only once.
    if (!m_D3DInitCompleted) {
      m_D3DInitCompleted = true;

      HMODULE hRuntime = LoadLibraryW(L"d3d12.dll");
      if (hRuntime == NULL)
        return false;
      // Do not: FreeLibrary(hRuntime);
      // If we actually free the library, it defeats the purpose of
      // EnableAgilitySDK and EnableExperimentalMode.

      HRESULT hr;
      hr = EnableAgilitySDK(hRuntime);
      if (FAILED(hr)) {
        LogCommentFmt(L"Unable to enable Agility SDK - 0x%08x.", hr);
      } else if (hr == S_FALSE) {
        LogCommentFmt(L"Agility SDK not enabled.");
      } else {
        LogCommentFmt(L"Agility SDK enabled.");
      }

      hr = EnableExperimentalMode(hRuntime);
      if (FAILED(hr)) {
        LogCommentFmt(L"Unable to enable shader experimental mode - 0x%08x.",
                      hr);
      } else if (hr == S_FALSE) {
        LogCommentFmt(L"Experimental mode not enabled.");
      } else {
        LogCommentFmt(L"Experimental mode enabled.");
      }

      hr = EnableDebugLayer();
      if (FAILED(hr)) {
        LogCommentFmt(L"Unable to enable debug layer - 0x%08x.", hr);
      } else if (hr == S_FALSE) {
        LogCommentFmt(L"Debug layer not enabled.");
      } else {
        LogCommentFmt(L"Debug layer enabled.");
      }
    }

    return true;
  }

  std::wstring DxcBlobToWide(IDxcBlob *pBlob) {
    if (!pBlob)
      return std::wstring();

    CComPtr<IDxcBlobWide> pBlobWide;
    if (SUCCEEDED(pBlob->QueryInterface(&pBlobWide)))
      return std::wstring(pBlobWide->GetStringPointer(),
                          pBlobWide->GetStringLength());

    CComPtr<IDxcBlobEncoding> pBlobEncoding;
    IFT(pBlob->QueryInterface(&pBlobEncoding));
    BOOL known;
    UINT32 codePage;
    IFT(pBlobEncoding->GetEncoding(&known, &codePage));
    if (!known) {
      throw std::runtime_error("unknown codepage for blob.");
    }

    std::wstring result;
    if (codePage == DXC_CP_WIDE) {
      const wchar_t *text = (const wchar_t *)pBlob->GetBufferPointer();
      size_t length = pBlob->GetBufferSize() / 2;
      if (length >= 1 && text[length - 1] == L'\0')
        length -= 1; // Exclude null-terminator
      result.resize(length);
      memcpy(&result[0], text, length);
      return result;
    }
    if (codePage == CP_UTF8) {
      const char *text = (const char *)pBlob->GetBufferPointer();
      size_t length = pBlob->GetBufferSize();
      if (length >= 1 && text[length - 1] == '\0')
        length -= 1; // Exclude null-terminator
      if (length == 0)
        return std::wstring();
      int wideLength = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                             text, (int)length, nullptr, 0);
      result.resize(wideLength);
      ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, (int)length,
                            &result[0], wideLength);
      return result;
    }
    throw std::runtime_error("Unsupported codepage.");
  }

  // Do not remove the following line - it is used by TranslateExecutionTest.py
  // MARKER: ExecutionTest/DxilConf Shared Implementation Start

  // This is defined in d3d.h for Windows 10 Anniversary Edition SDK, but we
  // only require the Windows 10 SDK.
  typedef enum D3D_SHADER_MODEL {
    D3D_SHADER_MODEL_5_1 = 0x51,
    D3D_SHADER_MODEL_6_0 = 0x60,
    D3D_SHADER_MODEL_6_1 = 0x61,
    D3D_SHADER_MODEL_6_2 = 0x62,
    D3D_SHADER_MODEL_6_3 = 0x63,
    D3D_SHADER_MODEL_6_4 = 0x64,
    D3D_SHADER_MODEL_6_5 = 0x65,
    D3D_SHADER_MODEL_6_6 = 0x66,
    D3D_SHADER_MODEL_6_7 = 0x67,
    D3D_SHADER_MODEL_6_8 = 0x68,
    D3D_SHADER_MODEL_6_9 = 0x69,
  } D3D_SHADER_MODEL;

  static const D3D_SHADER_MODEL HIGHEST_SHADER_MODEL = D3D_SHADER_MODEL_6_9;

  bool UseDxbc() {
#ifdef _HLK_CONF
    return false;
#else
    return GetTestParamBool(L"DXBC");
#endif
  }

  bool UseWarpByDefault() {
#ifdef _HLK_CONF
    return false;
#else
    return true;
#endif
  }

  bool UseDebugIfaces() { return true; }

  bool SaveImages() { return GetTestParamBool(L"SaveImages"); }

  // Base class used by raw gather test for polymorphic assignments
  struct RawGatherTexture {
    // Set Element <i> to a format-appropriate value derived from 2D coords
    // <x,y>
    virtual void SetElement(int i, int x, int y) = 0;
    // Retrieve pointer to the elements
    virtual void *GetElements() = 0;
    // Get dimensions/format
    virtual unsigned GetXDim() = 0;
    virtual unsigned GetYDim() = 0;
    virtual DXGI_FORMAT GetFormat() = 0;
  };

  template <typename GatherType>
  void DoRawGatherTest(ID3D12Device *pDevice, RawGatherTexture *rawTex,
                       DXGI_FORMAT viewFormat);
  void RunResourceTest(ID3D12Device *pDevice, const char *pShader,
                       const wchar_t *sm, bool isDynamic);

  template <class T1, class T2>
  void WaveIntrinsicsActivePrefixTest(TableParameter *pParameterList,
                                      size_t numParameter, bool isPrefix);

  template <typename T>
  void WaveIntrinsicsMultiPrefixOpTest(TableParameter *pParameterList,
                                       size_t numParameters);

  void BasicTriangleTestSetup(LPCSTR OpName, LPCWSTR FileName,
                              D3D_SHADER_MODEL testModel);

  void RunBasicShaderModelTest(D3D_SHADER_MODEL shaderModel);

  enum class RawBufferLdStType { I32, Float, I64, Double, I16, Half };

  template <class Ty> struct RawBufferLdStTestData {
    Ty v1, v2[2], v3[3], v4[4];
  };

  template <class Ty> struct RawBufferLdStUavData {
    RawBufferLdStTestData<Ty> input, output, srvOut;
  };

  template <class Ty>
  void RunComputeRawBufferLdStTest(D3D_SHADER_MODEL shaderModel,
                                   RawBufferLdStType dataType,
                                   const char *shaderOpName,
                                   const RawBufferLdStTestData<Ty> &testData);

  template <class Ty>
  void RunGraphicsRawBufferLdStTest(D3D_SHADER_MODEL shaderModel,
                                    RawBufferLdStType dataType,
                                    const char *shaderOpName,
                                    const RawBufferLdStTestData<Ty> &testData);

  template <class Ty>
  void
  VerifyRawBufferLdStTestResults(const std::shared_ptr<st::ShaderOpTest> test,
                                 const RawBufferLdStTestData<Ty> &testData);

  bool SetupRawBufferLdStTest(D3D_SHADER_MODEL shaderModel,
                              RawBufferLdStType dataType,
                              CComPtr<ID3D12Device> &pDevice,
                              CComPtr<IStream> &pStream, const char *&sTy,
                              const char *&additionalOptions);

  template <class Ty>
  void RunBasicShaderModelTest(CComPtr<ID3D12Device> pDevice,
                               const char *pShaderModelStr, const char *pShader,
                               Ty *pInputDataPairs, unsigned inputDataCount);

  template <class Ty> const wchar_t *BasicShaderModelTest_GetFormatString();

  void CompileFromText(LPCSTR pText, LPCWSTR pEntryPoint,
                       LPCWSTR pTargetProfile, ID3DBlob **ppBlob,
                       LPCWSTR *pOptions = nullptr, int numOptions = 0) {
    VERIFY_SUCCEEDED(m_support.Initialize());
    CComPtr<IDxcCompiler> pCompiler;
    CComPtr<IDxcLibrary> pLibrary;
    CComPtr<IDxcBlobEncoding> pTextBlob;
    CComPtr<IDxcOperationResult> pResult;
    HRESULT resultCode;
    VERIFY_SUCCEEDED(m_support.CreateInstance(CLSID_DxcCompiler, &pCompiler));
    VERIFY_SUCCEEDED(m_support.CreateInstance(CLSID_DxcLibrary, &pLibrary));
    VERIFY_SUCCEEDED(pLibrary->CreateBlobWithEncodingFromPinned(
        pText, (UINT32)strlen(pText), CP_UTF8, &pTextBlob));
    VERIFY_SUCCEEDED(pCompiler->Compile(pTextBlob, L"hlsl.hlsl", pEntryPoint,
                                        pTargetProfile, pOptions, numOptions,
                                        nullptr, 0, nullptr, &pResult));
    VERIFY_SUCCEEDED(pResult->GetStatus(&resultCode));
    if (FAILED(resultCode)) {
#ifndef _HLK_CONF
      CComPtr<IDxcBlobEncoding> errors;
      VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&errors));
      LogCommentFmt(L"Failed to compile shader: %s",
                    DxcBlobToWide(errors).data());
#endif
    }
    VERIFY_SUCCEEDED(resultCode);
    VERIFY_SUCCEEDED(pResult->GetResult((IDxcBlob **)ppBlob));
  }

  void CreateCommandQueue(ID3D12Device *pDevice, LPCWSTR pName,
                          ID3D12CommandQueue **ppCommandQueue,
                          D3D12_COMMAND_LIST_TYPE type) {
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = type;
    VERIFY_SUCCEEDED(
        pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(ppCommandQueue)));
    VERIFY_SUCCEEDED((*ppCommandQueue)->SetName(pName));
  }

  void CreateComputeCommandQueue(ID3D12Device *pDevice, LPCWSTR pName,
                                 ID3D12CommandQueue **ppCommandQueue) {
    CreateCommandQueue(pDevice, pName, ppCommandQueue,
                       D3D12_COMMAND_LIST_TYPE_COMPUTE);
  }

  void CreateComputePSO(ID3D12Device *pDevice,
                        ID3D12RootSignature *pRootSignature, LPCSTR pShader,
                        LPCWSTR pTargetProfile,
                        ID3D12PipelineState **ppComputeState,
                        LPCWSTR *pOptions = nullptr, int numOptions = 0) {
    CComPtr<ID3DBlob> pComputeShader;

    // Load and compile shaders.
    if (UseDxbc()) {
#ifndef _HLK_CONF
      DXBCFromText(pShader, L"main", pTargetProfile, &pComputeShader);
#endif
    } else {
      CompileFromText(pShader, L"main", pTargetProfile, &pComputeShader,
                      pOptions, numOptions);
    }

    // Describe and create the compute pipeline state object (PSO).
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = pRootSignature;
    computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(pComputeShader);

    VERIFY_SUCCEEDED(pDevice->CreateComputePipelineState(
        &computePsoDesc, IID_PPV_ARGS(ppComputeState)));
  }

  bool CreateDevice(ID3D12Device **ppDevice,
                    D3D_SHADER_MODEL testModel = D3D_SHADER_MODEL_6_0,
                    bool skipUnsupported = true) {
    if (testModel > HIGHEST_SHADER_MODEL) {
      UINT minor = (UINT)testModel & 0x0f;
      LogCommentFmt(L"Installed SDK does not support "
                    L"shader model 6.%1u",
                    minor);

      if (skipUnsupported) {
        WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
      }

      return false;
    }
    CComPtr<IDXGIFactory4> factory;
    CComPtr<ID3D12Device> pDevice;

    *ppDevice = nullptr;

    VERIFY_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
    if (GetTestParamUseWARP(UseWarpByDefault())) {
      CComPtr<IDXGIAdapter> warpAdapter;
      VERIFY_SUCCEEDED(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
      HRESULT createHR = D3D12CreateDevice(warpAdapter, D3D_FEATURE_LEVEL_11_0,
                                           IID_PPV_ARGS(&pDevice));
      if (FAILED(createHR)) {
        LogCommentFmt(L"The available version of WARP does not support d3d12.");

        if (skipUnsupported) {
          WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
        }

        return false;
      }

      if (GetModuleHandleW(L"d3d10warp.dll") != NULL) {
        WCHAR szFullModuleFilePath[MAX_PATH] = L"";
        GetModuleFileNameW(GetModuleHandleW(L"d3d10warp.dll"),
                           szFullModuleFilePath, sizeof(szFullModuleFilePath));
        WEX::Logging::Log::Comment(WEX::Common::String().Format(
            L"WARP driver loaded from: %S", szFullModuleFilePath));
      }

    } else {
      CComPtr<IDXGIAdapter1> hardwareAdapter;
      WEX::Common::String AdapterValue;
      HRESULT hr = WEX::TestExecution::RuntimeParameters::TryGetValue(
          L"Adapter", AdapterValue);
      if (SUCCEEDED(hr)) {
        st::GetHardwareAdapter(factory, AdapterValue, &hardwareAdapter);
      } else {
        WEX::Logging::Log::Comment(
            L"Using default hardware adapter with D3D12 support.");
      }

      VERIFY_SUCCEEDED(D3D12CreateDevice(
          hardwareAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice)));
    }
    // retrieve adapter information
    LUID adapterID = pDevice->GetAdapterLuid();
    CComPtr<IDXGIAdapter> adapter;
    factory->EnumAdapterByLuid(adapterID, IID_PPV_ARGS(&adapter));
    DXGI_ADAPTER_DESC AdapterDesc;
    VERIFY_SUCCEEDED(adapter->GetDesc(&AdapterDesc));
    LogCommentFmt(L"Using Adapter:%s", AdapterDesc.Description);

    if (pDevice == nullptr)
      return false;

    if (!UseDxbc()) {
      // Check for DXIL support.
      typedef struct D3D12_FEATURE_DATA_SHADER_MODEL {
        D3D_SHADER_MODEL HighestShaderModel;
      } D3D12_FEATURE_DATA_SHADER_MODEL;
      const UINT D3D12_FEATURE_SHADER_MODEL = 7;
      D3D12_FEATURE_DATA_SHADER_MODEL SMData;
      SMData.HighestShaderModel = testModel;
      if (FAILED(pDevice->CheckFeatureSupport(
              (D3D12_FEATURE)D3D12_FEATURE_SHADER_MODEL, &SMData,
              sizeof(SMData))) ||
          SMData.HighestShaderModel < testModel) {
        UINT minor = (UINT)testModel & 0x0f;
        LogCommentFmt(L"The selected device does not support "
                      L"shader model 6.%1u",
                      minor);

        if (skipUnsupported) {
          WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
        }

        return false;
      }
    }

    if (UseDebugIfaces()) {
      CComPtr<ID3D12InfoQueue> pInfoQueue;
      if (SUCCEEDED(pDevice->QueryInterface(&pInfoQueue))) {
        pInfoQueue->SetMuteDebugOutput(FALSE);
      }
    }

    *ppDevice = pDevice.Detach();
    return true;
  }

  void CreateGraphicsCommandQueue(ID3D12Device *pDevice,
                                  ID3D12CommandQueue **ppCommandQueue) {
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ;
    VERIFY_SUCCEEDED(
        pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(ppCommandQueue)));
  }

  void CreateGraphicsCommandQueueAndList(
      ID3D12Device *pDevice, ID3D12CommandQueue **ppCommandQueue,
      ID3D12CommandAllocator **ppAllocator,
      ID3D12GraphicsCommandList **ppCommandList, ID3D12PipelineState *pPSO) {
    CreateGraphicsCommandQueue(pDevice, ppCommandQueue);
    VERIFY_SUCCEEDED(pDevice->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(ppAllocator)));
    VERIFY_SUCCEEDED(pDevice->CreateCommandList(
        0, D3D12_COMMAND_LIST_TYPE_DIRECT, *ppAllocator, pPSO,
        IID_PPV_ARGS(ppCommandList)));
  }

  void CreateGraphicsPSO(ID3D12Device *pDevice,
                         D3D12_INPUT_LAYOUT_DESC *pInputLayout,
                         ID3D12RootSignature *pRootSignature, LPCSTR pShaders,
                         ID3D12PipelineState **ppPSO) {
    CComPtr<ID3DBlob> vertexShader;
    CComPtr<ID3DBlob> pixelShader;

    if (UseDxbc()) {
#ifndef _HLK_CONF
      DXBCFromText(pShaders, L"VSMain", L"vs_6_0", &vertexShader);
      DXBCFromText(pShaders, L"PSMain", L"ps_6_0", &pixelShader);
#endif
    } else {
      CompileFromText(pShaders, L"VSMain", L"vs_6_0", &vertexShader);
      CompileFromText(pShaders, L"PSMain", L"ps_6_0", &pixelShader);
    }

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = *pInputLayout;
    psoDesc.pRootSignature = pRootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader);
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    VERIFY_SUCCEEDED(
        pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(ppPSO)));
  }

  void CreateRenderTargetAndReadback(ID3D12Device *pDevice,
                                     ID3D12DescriptorHeap *pHeap, UINT width,
                                     UINT height,
                                     ID3D12Resource **ppRenderTarget,
                                     ID3D12Resource **ppBuffer) {
    const DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    const size_t formatElementSize = 4;
    CComPtr<ID3D12Resource> pRenderTarget;
    CComPtr<ID3D12Resource> pBuffer;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        pHeap->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_HEAP_PROPERTIES rtHeap(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC rtDesc(
        CD3DX12_RESOURCE_DESC::Tex2D(format, width, height));
    CD3DX12_CLEAR_VALUE rtClearVal(format, ClearColor);
    rtDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    VERIFY_SUCCEEDED(pDevice->CreateCommittedResource(
        &rtHeap, D3D12_HEAP_FLAG_NONE, &rtDesc, D3D12_RESOURCE_STATE_COPY_DEST,
        &rtClearVal, IID_PPV_ARGS(&pRenderTarget)));
    pDevice->CreateRenderTargetView(pRenderTarget, nullptr, rtvHandle);
    // rtvHandle.Offset(1, rtvDescriptorSize);  // Not needed for a single
    // resource.

    CD3DX12_HEAP_PROPERTIES readHeap(D3D12_HEAP_TYPE_READBACK);
    CD3DX12_RESOURCE_DESC readDesc(
        CD3DX12_RESOURCE_DESC::Buffer(width * height * formatElementSize));
    VERIFY_SUCCEEDED(pDevice->CreateCommittedResource(
        &readHeap, D3D12_HEAP_FLAG_NONE, &readDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&pBuffer)));

    *ppRenderTarget = pRenderTarget.Detach();
    *ppBuffer = pBuffer.Detach();
  }

  void CreateRootSignatureFromDesc(ID3D12Device *pDevice,
                                   const D3D12_ROOT_SIGNATURE_DESC *pDesc,
                                   ID3D12RootSignature **pRootSig) {
    CComPtr<ID3DBlob> signature;
    CComPtr<ID3DBlob> error;
    VERIFY_SUCCEEDED(D3D12SerializeRootSignature(
        pDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    VERIFY_SUCCEEDED(pDevice->CreateRootSignature(
        0, signature->GetBufferPointer(), signature->GetBufferSize(),
        IID_PPV_ARGS(pRootSig)));
  }

  void CreateRootSignatureFromRanges(
      ID3D12Device *pDevice, ID3D12RootSignature **pRootSig,
      CD3DX12_DESCRIPTOR_RANGE *resRanges, UINT resCt,
      CD3DX12_DESCRIPTOR_RANGE *sampRanges = nullptr, UINT sampCt = 0,
      D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) {
    UINT paramCt = 0;
    CD3DX12_ROOT_PARAMETER rootParameters[2];
    rootParameters[paramCt++].InitAsDescriptorTable(
        resCt, resRanges, D3D12_SHADER_VISIBILITY_ALL);
    if (sampCt)
      rootParameters[paramCt++].InitAsDescriptorTable(
          sampCt, sampRanges, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(paramCt, rootParameters, 0, nullptr, flags);
    CreateRootSignatureFromDesc(pDevice, &rootSignatureDesc, pRootSig);
  }

  void CreateRtvDescriptorHeap(ID3D12Device *pDevice, UINT numDescriptors,
                               ID3D12DescriptorHeap **pRtvHeap,
                               UINT *rtvDescriptorSize) {
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = numDescriptors;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    VERIFY_SUCCEEDED(
        pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(pRtvHeap)));

    if (rtvDescriptorSize != nullptr) {
      *rtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(
          D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }
  }

#if defined(NTDDI_WIN10_CU) && WDK_NTDDI_VERSION >= NTDDI_WIN10_CU
  // Copy common fields from desc0 to desc1 and zero out the new one
  void CopyDesc0ToDesc1(D3D12_RESOURCE_DESC1 &desc1,
                        const D3D12_RESOURCE_DESC &desc0) {
    desc1.Dimension = desc0.Dimension;
    desc1.Alignment = desc0.Alignment;
    desc1.Width = desc0.Width;
    desc1.Height = desc0.Height;
    desc1.DepthOrArraySize = desc0.DepthOrArraySize;
    desc1.MipLevels = desc0.MipLevels;
    desc1.Format = desc0.Format;
    desc1.SampleDesc = desc0.SampleDesc;
    desc1.Layout = desc0.Layout;
    desc1.Flags = desc0.Flags;
    desc1.SamplerFeedbackMipRegion = {};
  }
#endif

  // Create resources for the given <resDesc> described main resource
  // creating and returning the resource, the upload resource,
  // and the readback resource if requested, populating with <values> of size
  // <valueSizeInBytes> using <pCommandList> and <pDevice>
  // A pointer to a single <castFormat> target may be specified
  // where CreateCommittedResource3 is available
  void CreateTestResources(ID3D12Device *pDevice,
                           ID3D12GraphicsCommandList *pCommandList,
                           LPCVOID values, UINT64 valueSizeInBytes,
                           D3D12_RESOURCE_DESC resDesc,
                           ID3D12Resource **ppResource,
                           ID3D12Resource **ppUploadResource,
                           ID3D12Resource **ppReadBuffer = nullptr,
                           DXGI_FORMAT *castFormat = nullptr) {
    CComPtr<ID3D12Resource> pResource;
    CComPtr<ID3D12Resource> pReadBuffer;
    CComPtr<ID3D12Resource> pUploadResource;
    D3D12_SUBRESOURCE_DATA transferData;
    D3D12_HEAP_PROPERTIES defaultHeapProperties =
        CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_HEAP_PROPERTIES uploadHeapProperties =
        CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC uploadBufferDesc =
        CD3DX12_RESOURCE_DESC::Buffer(valueSizeInBytes);
    CD3DX12_HEAP_PROPERTIES readHeap(D3D12_HEAP_TYPE_READBACK);
    CD3DX12_RESOURCE_DESC readDesc(
        CD3DX12_RESOURCE_DESC::Buffer(valueSizeInBytes));

    pDevice->GetCopyableFootprints(&resDesc, 0, 1 /*mipleveles*/, 0, nullptr,
                                   nullptr, nullptr, &uploadBufferDesc.Width);
    uploadBufferDesc.Height = 1;

#if defined(NTDDI_WIN10_CU) && WDK_NTDDI_VERSION >= NTDDI_WIN10_CU
    if (castFormat) {
      CComPtr<ID3D12Device10> pDevice10;
      // Copy resDesc0 to resDesc1 zeroing anything new
      D3D12_RESOURCE_DESC1 resDesc1;
      memset(&resDesc1, 0, sizeof(resDesc1));
      CopyDesc0ToDesc1(resDesc1, resDesc);
      VERIFY_SUCCEEDED(pDevice->QueryInterface(IID_PPV_ARGS(&pDevice10)));
      VERIFY_SUCCEEDED(pDevice10->CreateCommittedResource3(
          &defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &resDesc1,
          D3D12_BARRIER_LAYOUT_COPY_DEST, nullptr, nullptr, 1, castFormat,
          IID_PPV_ARGS(&pResource)));
    } else
#else
    UNREFERENCED_PARAMETER(castFormat);
#endif
    {
      VERIFY_SUCCEEDED(pDevice->CreateCommittedResource(
          &defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &resDesc,
          D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&pResource)));
    }

    if (ppUploadResource)
      VERIFY_SUCCEEDED(pDevice->CreateCommittedResource(
          &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc,
          D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
          IID_PPV_ARGS(&pUploadResource)));

    if (ppReadBuffer)
      VERIFY_SUCCEEDED(pDevice->CreateCommittedResource(
          &readHeap, D3D12_HEAP_FLAG_NONE, &readDesc,
          D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&pReadBuffer)));

    if (ppUploadResource) {
      transferData.pData = values;
      transferData.RowPitch = (LONG_PTR)(valueSizeInBytes / resDesc.Height);
      transferData.SlicePitch = (LONG_PTR)valueSizeInBytes;

      UpdateSubresources<1>(pCommandList, pResource.p, pUploadResource.p, 0, 0,
                            1, &transferData);
      if (resDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
        RecordTransitionBarrier(pCommandList, pResource,
                                D3D12_RESOURCE_STATE_COPY_DEST,
                                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
      else
        RecordTransitionBarrier(pCommandList, pResource,
                                D3D12_RESOURCE_STATE_COPY_DEST,
                                D3D12_RESOURCE_STATE_COMMON);
    }

    *ppResource = pResource.Detach();
    if (ppUploadResource)
      *ppUploadResource = pUploadResource.Detach();
    if (ppReadBuffer)
      *ppReadBuffer = pReadBuffer.Detach();
  }

  void CreateTestUavs(ID3D12Device *pDevice,
                      ID3D12GraphicsCommandList *pCommandList, LPCVOID values,
                      UINT64 valueSizeInBytes, ID3D12Resource **ppUavResource,
                      ID3D12Resource **ppUploadResource = nullptr,
                      ID3D12Resource **ppReadBuffer = nullptr) {
    D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
        valueSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    CreateTestResources(pDevice, pCommandList, values, valueSizeInBytes,
                        bufferDesc, ppUavResource, ppUploadResource,
                        ppReadBuffer);
  }

  // Create and return descriptor heaps for the given device
  // with the given number of resources and samples.
  // using some reasonable defaults
  void CreateDefaultDescHeaps(ID3D12Device *pDevice, int NumResources,
                              int NumSamplers, ID3D12DescriptorHeap **ppResHeap,
                              ID3D12DescriptorHeap **ppSampHeap) {
    // Describe and create descriptor heaps.
    ID3D12DescriptorHeap *pResHeap, *pSampHeap;
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = NumResources;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    VERIFY_SUCCEEDED(
        pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pResHeap)));

    heapDesc.NumDescriptors = NumSamplers;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    VERIFY_SUCCEEDED(
        pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pSampHeap)));

    *ppResHeap = pResHeap;
    *ppSampHeap = pSampHeap;
  }

  void CreateSRV(ID3D12Device *pDevice,
                 CD3DX12_CPU_DESCRIPTOR_HANDLE &baseHandle, DXGI_FORMAT format,
                 D3D12_SRV_DIMENSION viewDimension, UINT numElements,
                 UINT stride, const CComPtr<ID3D12Resource> pResource) {
    UINT descriptorSize = pDevice->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    // Create SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = viewDimension;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    switch (viewDimension) {
    case D3D12_SRV_DIMENSION_BUFFER:
      srvDesc.Buffer.FirstElement = 0;
      srvDesc.Buffer.NumElements = numElements;
      srvDesc.Buffer.StructureByteStride = stride;
      if (format == DXGI_FORMAT_R32_TYPELESS && stride == 0)
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
      else
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
      break;
    case D3D12_SRV_DIMENSION_TEXTURE1D:
      srvDesc.Texture1D.MostDetailedMip = 0;
      srvDesc.Texture1D.MipLevels = 1;
      srvDesc.Texture1D.ResourceMinLODClamp = 0;
      break;
    case D3D12_SRV_DIMENSION_TEXTURE2D:
      srvDesc.Texture2D.MostDetailedMip = 0;
      srvDesc.Texture2D.MipLevels = 1;
      srvDesc.Texture2D.PlaneSlice = 0;
      srvDesc.Texture2D.ResourceMinLODClamp = 0;
      break;
    }
    pDevice->CreateShaderResourceView(pResource, &srvDesc, baseHandle);
    baseHandle.Offset(descriptorSize);
  }

  void CreateRawSRV(ID3D12Device *pDevice,
                    CD3DX12_CPU_DESCRIPTOR_HANDLE &heapStart, UINT numElements,
                    const CComPtr<ID3D12Resource> pResource) {
    CreateSRV(pDevice, heapStart, DXGI_FORMAT_R32_TYPELESS,
              D3D12_SRV_DIMENSION_BUFFER, numElements, 0, pResource);
  }

  void CreateStructSRV(ID3D12Device *pDevice,
                       CD3DX12_CPU_DESCRIPTOR_HANDLE &heapStart,
                       UINT numElements, UINT stride,
                       const CComPtr<ID3D12Resource> pResource) {
    CreateSRV(pDevice, heapStart, DXGI_FORMAT_UNKNOWN,
              D3D12_SRV_DIMENSION_BUFFER, numElements, stride, pResource);
  }

  void CreateTypedSRV(ID3D12Device *pDevice,
                      CD3DX12_CPU_DESCRIPTOR_HANDLE &heapStart,
                      UINT numElements, DXGI_FORMAT format,
                      const CComPtr<ID3D12Resource> pResource) {
    CreateSRV(pDevice, heapStart, format, D3D12_SRV_DIMENSION_BUFFER,
              numElements, 0, pResource);
  }

  void CreateTex1DSRV(ID3D12Device *pDevice,
                      CD3DX12_CPU_DESCRIPTOR_HANDLE &heapStart,
                      UINT numElements, DXGI_FORMAT format,
                      const CComPtr<ID3D12Resource> pResource) {
    CreateSRV(pDevice, heapStart, format, D3D12_SRV_DIMENSION_TEXTURE1D,
              numElements, 0, pResource);
  }

  void CreateTex2DSRV(ID3D12Device *pDevice,
                      CD3DX12_CPU_DESCRIPTOR_HANDLE &heapStart,
                      DXGI_FORMAT format,
                      const CComPtr<ID3D12Resource> pResource) {
    CreateSRV(pDevice, heapStart, format, D3D12_SRV_DIMENSION_TEXTURE2D,
              0 /*numElements*/, 0 /*stride*/, pResource);
  }

  void CreateUAV(ID3D12Device *pDevice,
                 CD3DX12_CPU_DESCRIPTOR_HANDLE &baseHandle, DXGI_FORMAT format,
                 D3D12_UAV_DIMENSION viewDimension, UINT numElements,
                 UINT stride, const CComPtr<ID3D12Resource> pResource) {
    UINT descriptorSize = pDevice->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = format;
    uavDesc.ViewDimension = viewDimension;
    switch (viewDimension) {
    case D3D12_UAV_DIMENSION_BUFFER:
      uavDesc.Buffer.FirstElement = 0;
      uavDesc.Buffer.NumElements = numElements;
      uavDesc.Buffer.StructureByteStride = stride;
      if (format == DXGI_FORMAT_R32_TYPELESS && stride == 0)
        uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
      else
        uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
      break;
    case D3D12_UAV_DIMENSION_TEXTURE1D:
      uavDesc.Texture1D.MipSlice = 0;
      break;
    case D3D12_UAV_DIMENSION_TEXTURE2D:
      uavDesc.Texture2D.MipSlice = 0;
      uavDesc.Texture2D.PlaneSlice = 0;
      break;
    case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
      uavDesc.Texture2DArray.MipSlice = 0;
      uavDesc.Texture2DArray.PlaneSlice = 0;
      uavDesc.Texture2DArray.FirstArraySlice = 0;
      uavDesc.Texture2DArray.ArraySize = numElements;
      break;
    default:
      break;
    }
    pDevice->CreateUnorderedAccessView(pResource, nullptr, &uavDesc,
                                       baseHandle);
    baseHandle.Offset(descriptorSize);
  }

  void CreateRawUAV(ID3D12Device *pDevice,
                    CD3DX12_CPU_DESCRIPTOR_HANDLE &heapStart, UINT numElements,
                    const CComPtr<ID3D12Resource> pResource) {
    CreateUAV(pDevice, heapStart, DXGI_FORMAT_R32_TYPELESS,
              D3D12_UAV_DIMENSION_BUFFER, numElements, 0 /*stride*/, pResource);
  }

  void CreateStructUAV(ID3D12Device *pDevice,
                       CD3DX12_CPU_DESCRIPTOR_HANDLE &heapStart,
                       UINT numElements, UINT stride,
                       const CComPtr<ID3D12Resource> pResource) {
    CreateUAV(pDevice, heapStart, DXGI_FORMAT_UNKNOWN,
              D3D12_UAV_DIMENSION_BUFFER, numElements, stride, pResource);
  }

  void CreateTypedUAV(ID3D12Device *pDevice,
                      CD3DX12_CPU_DESCRIPTOR_HANDLE &heapStart,
                      UINT numElements, DXGI_FORMAT format,
                      const CComPtr<ID3D12Resource> pResource) {
    CreateUAV(pDevice, heapStart, format, D3D12_UAV_DIMENSION_BUFFER,
              numElements, 0 /*stride*/, pResource);
  }

  void CreateTex1DUAV(ID3D12Device *pDevice,
                      CD3DX12_CPU_DESCRIPTOR_HANDLE &heapStart,
                      DXGI_FORMAT format,
                      const CComPtr<ID3D12Resource> pResource) {
    CreateUAV(pDevice, heapStart, format, D3D12_UAV_DIMENSION_TEXTURE1D,
              0 /*numElements*/, 0 /*stride*/, pResource);
  }

  void CreateTex2DUAV(ID3D12Device *pDevice,
                      CD3DX12_CPU_DESCRIPTOR_HANDLE &heapStart,
                      DXGI_FORMAT format,
                      const CComPtr<ID3D12Resource> pResource) {
    CreateUAV(pDevice, heapStart, format, D3D12_UAV_DIMENSION_TEXTURE2D,
              0 /*numElements*/, 0 /*stride*/, pResource);
  }

  void CreateTex2DArrayUAV(ID3D12Device *pDevice,
                           CD3DX12_CPU_DESCRIPTOR_HANDLE &heapStart,
                           UINT numElements, DXGI_FORMAT format,
                           const CComPtr<ID3D12Resource> pResource) {
    CreateUAV(pDevice, heapStart, format, D3D12_UAV_DIMENSION_TEXTURE2DARRAY,
              numElements, 0 /*stride*/, pResource);
  }

  void CreateTex2DMSUAV(ID3D12Device *pDevice,
                        CD3DX12_CPU_DESCRIPTOR_HANDLE &heapStart,
                        DXGI_FORMAT format,
                        const CComPtr<ID3D12Resource> pResource) {
    CreateUAV(pDevice, heapStart, format,
              (D3D12_UAV_DIMENSION)6 /*D3D12_UAV_DIMENSION_TEXTURE2DMS*/,
              0 /*numElements*/, 0 /*stride*/, pResource);
  }

  // Create Samplers for <pDevice> given the filter and border color information
  // provided using some reasonable defaults
  void CreateDefaultSamplers(ID3D12Device *pDevice,
                             D3D12_CPU_DESCRIPTOR_HANDLE heapStart,
                             D3D12_FILTER filters[],
                             float *perSamplerBorderColors, int NumSamplers) {

    CD3DX12_CPU_DESCRIPTOR_HANDLE sampHandle(heapStart);
    UINT descriptorSize = pDevice->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    D3D12_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    D3D12_TEXTURE_ADDRESS_MODE addrMode =
        perSamplerBorderColors ? D3D12_TEXTURE_ADDRESS_MODE_BORDER
                               : D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampDesc.AddressU = sampDesc.AddressV = sampDesc.AddressW = addrMode;
    sampDesc.MipLODBias = 0;
    sampDesc.MaxAnisotropy = 1;
    sampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_EQUAL;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = 0;

    for (int i = 0; i < NumSamplers; i++) {
      sampDesc.Filter = filters[i];
      if (perSamplerBorderColors) {
        for (int j = 0; j < 4; j++)
          sampDesc.BorderColor[j] = perSamplerBorderColors[i];
      }

      pDevice->CreateSampler(&sampDesc, sampHandle);
      sampHandle = sampHandle.Offset(descriptorSize);
    }
  }

  template <typename TVertex, int len>
  void CreateVertexBuffer(ID3D12Device *pDevice, TVertex (&vertices)[len],
                          ID3D12Resource **ppVertexBuffer,
                          D3D12_VERTEX_BUFFER_VIEW *pVertexBufferView) {
    size_t vertexBufferSize = sizeof(vertices);
    CComPtr<ID3D12Resource> pVertexBuffer;
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc(
        CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize));
    VERIFY_SUCCEEDED(pDevice->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&pVertexBuffer)));

    UINT8 *pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);
    VERIFY_SUCCEEDED(pVertexBuffer->Map(
        0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, vertices, vertexBufferSize);
    pVertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    pVertexBufferView->BufferLocation = pVertexBuffer->GetGPUVirtualAddress();
    pVertexBufferView->StrideInBytes = sizeof(TVertex);
    pVertexBufferView->SizeInBytes = (UINT)vertexBufferSize;

    *ppVertexBuffer = pVertexBuffer.Detach();
  }

  // Requires Anniversary Edition headers, so simplifying things for current
  // setup.
  const UINT D3D12_FEATURE_D3D12_OPTIONS1 = 8;
  struct D3D12_FEATURE_DATA_D3D12_OPTIONS1 {
    BOOL WaveOps;
    UINT WaveLaneCountMin;
    UINT WaveLaneCountMax;
    UINT TotalLaneCount;
    BOOL ExpandedComputeResourceStates;
    BOOL Int64ShaderOps;
  };

  bool IsDeviceBasicAdapter(ID3D12Device *pDevice) {
    CComPtr<IDXGIFactory4> factory;
    VERIFY_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
    LUID adapterID = pDevice->GetAdapterLuid();
    CComPtr<IDXGIAdapter1> adapter;
    factory->EnumAdapterByLuid(adapterID, IID_PPV_ARGS(&adapter));
    DXGI_ADAPTER_DESC1 AdapterDesc;
    VERIFY_SUCCEEDED(adapter->GetDesc1(&AdapterDesc));
    return (AdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) ||
           (AdapterDesc.VendorId == 0x1414 &&
            (AdapterDesc.DeviceId == 0x8c || AdapterDesc.DeviceId == 0x8d));
  }

  bool DoesDeviceSupportInt64(ID3D12Device *pDevice) {
    D3D12_FEATURE_DATA_D3D12_OPTIONS1 O;
    if (FAILED(pDevice->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS1, &O, sizeof(O))))
      return false;
    return O.Int64ShaderOps != FALSE;
  }

  bool DoesDeviceSupportDouble(ID3D12Device *pDevice) {
    D3D12_FEATURE_DATA_D3D12_OPTIONS O;
    if (FAILED(pDevice->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS, &O, sizeof(O))))
      return false;
    return O.DoublePrecisionFloatShaderOps != FALSE;
  }

  bool DoesDeviceSupportWaveOps(ID3D12Device *pDevice) {
    D3D12_FEATURE_DATA_D3D12_OPTIONS1 O;
    if (FAILED(pDevice->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS1, &O, sizeof(O))))
      return false;
    return O.WaveOps != FALSE;
  }

  bool DoesDeviceSupportBarycentrics(ID3D12Device *pDevice) {
    D3D12_FEATURE_DATA_D3D12_OPTIONS3 O;
    if (FAILED(pDevice->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS3, &O, sizeof(O))))
      return false;
    return O.BarycentricsSupported != FALSE;
  }

  bool DoesDeviceSupportNative16bitOps(ID3D12Device *pDevice) {
    D3D12_FEATURE_DATA_D3D12_OPTIONS4 O;
    if (FAILED(pDevice->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS4, &O, sizeof(O))))
      return false;
    return O.Native16BitShaderOpsSupported != FALSE;
  }

  bool DoesDeviceSupportMeshShaders(ID3D12Device *pDevice) {
#if defined(NTDDI_WIN10_VB) && WDK_NTDDI_VERSION >= NTDDI_WIN10_VB
    D3D12_FEATURE_DATA_D3D12_OPTIONS7 O7;
    if (FAILED(pDevice->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS7, &O7, sizeof(O7))))
      return false;
    return O7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;
#else
    UNREFERENCED_PARAMETER(pDevice);
    return false;
#endif
  }

  bool DoesDeviceSupportRayTracing(ID3D12Device *pDevice) {
#if WDK_NTDDI_VERSION > NTDDI_WIN10_RS4
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 O5;
    if (FAILED(pDevice->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS5, &O5, sizeof(O5))))
      return false;
    return O5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
#else
    UNREFERENCED_PARAMETER(pDevice);
    return false;
#endif
  }

  bool DoesDeviceSupportMeshAmpDerivatives(ID3D12Device *pDevice) {
#if defined(NTDDI_WIN10_FE) && WDK_NTDDI_VERSION >= NTDDI_WIN10_FE
    D3D12_FEATURE_DATA_D3D12_OPTIONS7 O7;
    D3D12_FEATURE_DATA_D3D12_OPTIONS9 O9;
    if (FAILED(pDevice->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS7, &O7, sizeof(O7))) ||
        FAILED(pDevice->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS9, &O9, sizeof(O9))))
      return false;
    return O7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED &&
           O9.DerivativesInMeshAndAmplificationShadersSupported != FALSE;
#else
    UNREFERENCED_PARAMETER(pDevice);
    return false;
#endif
  }

  bool DoesDeviceSupportTyped64Atomics(ID3D12Device *pDevice) {
#if defined(NTDDI_WIN10_FE) && WDK_NTDDI_VERSION >= NTDDI_WIN10_FE
    D3D12_FEATURE_DATA_D3D12_OPTIONS9 O9;
    if (FAILED(pDevice->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS9, &O9, sizeof(O9))))
      return false;
    return O9.AtomicInt64OnTypedResourceSupported != FALSE;
#else
    UNREFERENCED_PARAMETER(pDevice);
    return false;
#endif
  }

  bool DoesDeviceSupportHeap64Atomics(ID3D12Device *pDevice) {
#if defined(NTDDI_WIN10_CO) && WDK_NTDDI_VERSION >= NTDDI_WIN10_CO
    D3D12_FEATURE_DATA_D3D12_OPTIONS11 O11;
    if (FAILED(pDevice->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS11, &O11, sizeof(O11))))
      return false;
    return O11.AtomicInt64OnDescriptorHeapResourceSupported != FALSE;
#else
    UNREFERENCED_PARAMETER(pDevice);
    return false;
#endif
  }

  bool DoesDeviceSupportShared64Atomics(ID3D12Device *pDevice) {
#if defined(NTDDI_WIN10_FE) && WDK_NTDDI_VERSION >= NTDDI_WIN10_FE
    D3D12_FEATURE_DATA_D3D12_OPTIONS9 O9;
    if (FAILED(pDevice->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS9, &O9, sizeof(O9))))
      return false;
    return O9.AtomicInt64OnGroupSharedSupported != FALSE;
#else
    UNREFERENCED_PARAMETER(pDevice);
    return false;
#endif
  }

  bool DoesDeviceSupportAdvancedTexOps(ID3D12Device *pDevice) {
#if defined(NTDDI_WIN10_CU) && WDK_NTDDI_VERSION >= NTDDI_WIN10_CU
    D3D12_FEATURE_DATA_D3D12_OPTIONS14 O14;
    if (FAILED(pDevice->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS14, &O14, sizeof(O14))))
      return false;
    return O14.AdvancedTextureOpsSupported != FALSE;
#else
    UNREFERENCED_PARAMETER(pDevice);
    return false;
#endif
  }

  bool DoesDeviceSupportWritableMSAA(ID3D12Device *pDevice) {
#if defined(NTDDI_WIN10_CU) && WDK_NTDDI_VERSION >= NTDDI_WIN10_CU
    D3D12_FEATURE_DATA_D3D12_OPTIONS14 O14;
    if (FAILED(pDevice->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS14, &O14, sizeof(O14))))
      return false;
    return O14.WriteableMSAATexturesSupported != FALSE;
#else
    UNREFERENCED_PARAMETER(pDevice);
    return false;
#endif
  }

  bool DoesDeviceSupportEnhancedBarriers(ID3D12Device *pDevice) {
#if defined(NTDDI_WIN10_CU) && WDK_NTDDI_VERSION >= NTDDI_WIN10_CU
    D3D12_FEATURE_DATA_D3D12_OPTIONS12 O12;
    if (FAILED(pDevice->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS12, &O12, sizeof(O12))))
      return false;
    return O12.EnhancedBarriersSupported != FALSE;
#else
    UNREFERENCED_PARAMETER(pDevice);
    return false;
#endif
  }

  bool DoesDeviceSupportRelaxedFormatCasting(ID3D12Device *pDevice) {
#if defined(NTDDI_WIN10_CU) && WDK_NTDDI_VERSION >= NTDDI_WIN10_CU
    D3D12_FEATURE_DATA_D3D12_OPTIONS12 O12;
    if (!DoesDeviceSupportEnhancedBarriers(pDevice))
      return false;

    if (FAILED(pDevice->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS12, &O12, sizeof(O12))))
      return false;
    return O12.RelaxedFormatCastingSupported != FALSE;
#else
    UNREFERENCED_PARAMETER(pDevice);
    return false;
#endif
  }

  bool IsFallbackPathEnabled() {
    // Enable fallback paths with: /p:"EnableFallback=1"
    UINT EnableFallbackValue = 0;
    WEX::TestExecution::RuntimeParameters::TryGetValue(L"EnableFallback",
                                                       EnableFallbackValue);
    return EnableFallbackValue != 0;
  }

#ifndef _HLK_CONF
  void DXBCFromText(LPCSTR pText, LPCWSTR pEntryPoint, LPCWSTR pTargetProfile,
                    ID3DBlob **ppBlob) {
    CW2A pEntryPointA(pEntryPoint);
    CW2A pTargetProfileA(pTargetProfile);
    CComPtr<ID3DBlob> pErrors;
    D3D_SHADER_MACRO d3dMacro[2];
    ZeroMemory(d3dMacro, sizeof(d3dMacro));
    d3dMacro[0].Definition = "1";
    d3dMacro[0].Name = "USING_DXBC";
    HRESULT hr =
        D3DCompile(pText, strlen(pText), "hlsl.hlsl", d3dMacro, nullptr,
                   pEntryPointA, pTargetProfileA, 0, 0, ppBlob, &pErrors);
    if (pErrors != nullptr) {
      CA2W errors((char *)pErrors->GetBufferPointer());
      LogCommentFmt(L"Compilation failure: %s", errors.m_szBuffer);
    }
    VERIFY_SUCCEEDED(hr);
  }
#endif

  HRESULT EnableDebugLayer() {
    // The debug layer does net yet validate DXIL programs that require
    // rewriting, but basic logging should work properly.
    HRESULT hr = S_FALSE;
    if (UseDebugIfaces()) {
      CComPtr<ID3D12Debug> debugController;
      hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
      if (SUCCEEDED(hr)) {
        debugController->EnableDebugLayer();
        hr = S_OK;
      }
    }
    return hr;
  }

  static std::wstring GetModuleName() {
    wchar_t moduleName[MAX_PATH + 1] = {0};
    DWORD length = GetModuleFileNameW(NULL, moduleName, MAX_PATH);
    if (length == 0 || length == MAX_PATH) {
      return std::wstring(); // Error condition
    }
    return std::wstring(moduleName, length);
  }

  static std::wstring ComputeSDKFullPath(std::wstring SDKPath) {
    std::wstring modulePath = GetModuleName();
    size_t pos = modulePath.rfind('\\');
    if (pos == std::wstring::npos)
      return SDKPath;
    if (SDKPath.substr(0, 2) != L".\\")
      return SDKPath;
    return modulePath.substr(0, pos) + SDKPath.substr(1);
  }

  static UINT GetD3D12SDKVersion(std::wstring SDKPath) {
    // Try to automatically get the D3D12SDKVersion from the DLL
    UINT SDKVersion = 0;
    std::wstring D3DCorePath = ComputeSDKFullPath(SDKPath);
    D3DCorePath.append(L"D3D12Core.dll");
    HMODULE hCore = LoadLibraryW(D3DCorePath.c_str());
    if (hCore) {
      if (UINT *pSDKVersion = (UINT *)GetProcAddress(hCore, "D3D12SDKVersion"))
        SDKVersion = *pSDKVersion;
      FreeModule(hCore);
    }
    return SDKVersion;
  }

  static HRESULT EnableAgilitySDK(HMODULE hRuntime, UINT SDKVersion,
                                  LPCWSTR SDKPath) {
    D3D12GetInterfaceFn pD3D12GetInterface =
        (D3D12GetInterfaceFn)GetProcAddress(hRuntime, "D3D12GetInterface");
    CComPtr<ID3D12SDKConfiguration> pD3D12SDKConfiguration;
    IFR(pD3D12GetInterface(CLSID_D3D12SDKConfiguration,
                           IID_PPV_ARGS(&pD3D12SDKConfiguration)));
    IFR(pD3D12SDKConfiguration->SetSDKVersion(SDKVersion, CW2A(SDKPath)));

    // Currently, it appears that the SetSDKVersion will succeed even when
    // D3D12Core is not found, or its version doesn't match.  When that's the
    // case, will cause a failure in the very next thing that actually requires
    // D3D12Core.dll to be loaded instead.  So, we attempt to clear experimental
    // features next, which is a valid use case and a no-op at this point.  This
    // requires D3D12Core to be loaded.  If this fails, we know the AgilitySDK
    // setting actually failed.
    D3D12EnableExperimentalFeaturesFn pD3D12EnableExperimentalFeatures =
        (D3D12EnableExperimentalFeaturesFn)GetProcAddress(
            hRuntime, "D3D12EnableExperimentalFeatures");
    if (pD3D12EnableExperimentalFeatures == nullptr) {
      // If this failed, D3D12 must be too old for AgilitySDK.  But if that's
      // the case, creating D3D12SDKConfiguration should have failed.  So while
      // this case shouldn't be hit, fail if it is.
      return HRESULT_FROM_WIN32(GetLastError());
    }
    return pD3D12EnableExperimentalFeatures(0, nullptr, nullptr, nullptr);
  }

  static HRESULT EnableExperimentalShaderModels(HMODULE hRuntime) {
    D3D12EnableExperimentalFeaturesFn pD3D12EnableExperimentalFeatures =
        (D3D12EnableExperimentalFeaturesFn)GetProcAddress(
            hRuntime, "D3D12EnableExperimentalFeatures");
    if (pD3D12EnableExperimentalFeatures == nullptr) {
      return HRESULT_FROM_WIN32(GetLastError());
    }
    return pD3D12EnableExperimentalFeatures(1, &D3D12ExperimentalShaderModelsID,
                                            nullptr, nullptr);
  }

  static HRESULT EnableExperimentalShaderModels() {
    HMODULE hRuntime = LoadLibraryW(L"d3d12.dll");
    if (hRuntime == NULL)
      return E_FAIL;
    return EnableExperimentalShaderModels(hRuntime);
  }

  HRESULT EnableAgilitySDK(HMODULE hRuntime) {
    // D3D12SDKVersion > 1 will use provided version, otherwise, auto-detect.
    // D3D12SDKVersion == 1 means fail if we can't auto-detect.
    UINT SDKVersion = 0;
    WEX::TestExecution::RuntimeParameters::TryGetValue(L"D3D12SDKVersion",
                                                       SDKVersion);

    // SDKPath must be relative path from .exe, which means relative to
    // TE.exe location, and must start with ".\\", such as with the
    // default: ".\\D3D12\\"
    WEX::Common::String SDKPath;
    if (SUCCEEDED(WEX::TestExecution::RuntimeParameters::TryGetValue(
            L"D3D12SDKPath", SDKPath))) {
      // Make sure path ends in backslash
      if (!SDKPath.IsEmpty() && SDKPath.Right(1) != "\\") {
        SDKPath.Append("\\");
      }
    }
    if (SDKPath.IsEmpty()) {
      SDKPath = L".\\D3D12\\";
    }

    bool mustFind = SDKVersion > 0;
    if (SDKVersion <= 1) {
      // lookup version from D3D12Core.dll
      SDKVersion = GetD3D12SDKVersion((LPCWSTR)SDKPath);
      if (mustFind && SDKVersion == 0) {
        LogErrorFmt(L"Agility SDK not found in relative path: %s",
                    (LPCWSTR)SDKPath);
        return E_FAIL;
      }
    }

    // Not found, not asked for.
    if (SDKVersion == 0)
      return S_FALSE;

    HRESULT hr = EnableAgilitySDK(hRuntime, SDKVersion, (LPCWSTR)SDKPath);
    if (FAILED(hr)) {
      // If SDKVersion provided, fail if not successful.
      // 1 means we should find it, and fill in the version automatically.
      if (mustFind) {
        LogErrorFmt(L"Failed to set Agility SDK version %d at path: %s",
                    SDKVersion, (LPCWSTR)SDKPath);
        return hr;
      }
      return S_FALSE;
    }
    if (hr == S_OK) {
      LogCommentFmt(L"Agility SDK version set to: %d", SDKVersion);
      m_AgilitySDKEnabled = true;
    }
    return hr;
  }

  HRESULT EnableExperimentalMode(HMODULE hRuntime) {
    if (m_ExperimentalModeEnabled) {
      return S_OK;
    }

#ifdef _FORCE_EXPERIMENTAL_SHADERS
    bool bExperimentalShaderModels = true;
#else
    bool bExperimentalShaderModels = GetTestParamBool(L"ExperimentalShaders");
#endif // _FORCE_EXPERIMENTAL_SHADERS

    HRESULT hr = S_FALSE;
    if (bExperimentalShaderModels) {
      hr = EnableExperimentalShaderModels(hRuntime);
      if (SUCCEEDED(hr)) {
        m_ExperimentalModeEnabled = true;
      }
    }

    return hr;
  }

  struct FenceObj {
    HANDLE m_fenceEvent = NULL;
    CComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;
    ~FenceObj() {
      if (m_fenceEvent)
        CloseHandle(m_fenceEvent);
    }
  };

  void InitFenceObj(ID3D12Device *pDevice, FenceObj *pObj) {
    pObj->m_fenceValue = 1;
    VERIFY_SUCCEEDED(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                          IID_PPV_ARGS(&pObj->m_fence)));
    // Create an event handle to use for frame synchronization.
    pObj->m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (pObj->m_fenceEvent == nullptr) {
      VERIFY_SUCCEEDED(HRESULT_FROM_WIN32(GetLastError()));
    }
  }

  void ReadHlslDataIntoNewStream(LPCWSTR relativePath, IStream **ppStream) {
    VERIFY_SUCCEEDED(m_support.Initialize());
    CComPtr<IDxcLibrary> pLibrary;
    CComPtr<IDxcBlobEncoding> pBlob;
    CComPtr<IStream> pStream;
    std::wstring path = GetPathToHlslDataFile(relativePath, HLSLDATAFILEPARAM,
                                              DEFAULT_EXEC_TEST_DIR);
    VERIFY_SUCCEEDED(m_support.CreateInstance(CLSID_DxcLibrary, &pLibrary));
    VERIFY_SUCCEEDED(
        pLibrary->CreateBlobFromFile(path.c_str(), nullptr, &pBlob));
    VERIFY_SUCCEEDED(pLibrary->CreateStreamFromBlobReadOnly(pBlob, &pStream));
    *ppStream = pStream.Detach();
  }

  void RecordRenderAndReadback(ID3D12GraphicsCommandList *pList,
                               ID3D12DescriptorHeap *pRtvHeap,
                               UINT rtvDescriptorSize, UINT instanceCount,
                               D3D12_VERTEX_BUFFER_VIEW *pVertexBufferView,
                               ID3D12RootSignature *pRootSig,
                               ID3D12Resource *pRenderTarget,
                               ID3D12Resource *pReadBuffer) {
    D3D12_RESOURCE_DESC rtDesc = pRenderTarget->GetDesc();
    D3D12_VIEWPORT viewport;
    D3D12_RECT scissorRect;

    memset(&viewport, 0, sizeof(viewport));
    viewport.Height = (float)rtDesc.Height;
    viewport.Width = (float)rtDesc.Width;
    viewport.MaxDepth = 1.0f;
    memset(&scissorRect, 0, sizeof(scissorRect));
    scissorRect.right = (long)rtDesc.Width;
    scissorRect.bottom = rtDesc.Height;
    if (pRootSig != nullptr) {
      pList->SetGraphicsRootSignature(pRootSig);
    }
    pList->RSSetViewports(1, &viewport);
    pList->RSSetScissorRects(1, &scissorRect);

    // Indicate that the buffer will be used as a render target.
    RecordTransitionBarrier(pList, pRenderTarget,
                            D3D12_RESOURCE_STATE_COPY_DEST,
                            D3D12_RESOURCE_STATE_RENDER_TARGET);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        pRtvHeap->GetCPUDescriptorHandleForHeapStart(), 0, rtvDescriptorSize);
    pList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    pList->ClearRenderTargetView(rtvHandle, ClearColor, 0, nullptr);
    pList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pList->IASetVertexBuffers(0, 1, pVertexBufferView);
    pList->DrawInstanced(3, instanceCount, 0, 0);

    // Transition to copy source and copy into read-back buffer.
    RecordTransitionBarrier(pList, pRenderTarget,
                            D3D12_RESOURCE_STATE_RENDER_TARGET,
                            D3D12_RESOURCE_STATE_COPY_SOURCE);

    // Copy into read-back buffer.
    UINT64 rowPitch = rtDesc.Width * 4;
    if (rowPitch % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)
      rowPitch += D3D12_TEXTURE_DATA_PITCH_ALIGNMENT -
                  (rowPitch % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint;
    Footprint.Offset = 0;
    Footprint.Footprint = CD3DX12_SUBRESOURCE_FOOTPRINT(
        DXGI_FORMAT_R8G8B8A8_UNORM, (UINT)rtDesc.Width, rtDesc.Height, 1,
        (UINT)rowPitch);
    CD3DX12_TEXTURE_COPY_LOCATION DstLoc(pReadBuffer, Footprint);
    CD3DX12_TEXTURE_COPY_LOCATION SrcLoc(pRenderTarget, 0);
    pList->CopyTextureRegion(&DstLoc, 0, 0, 0, &SrcLoc, nullptr);
  }

  void RunRWByteBufferComputeTest(ID3D12Device *pDevice, LPCSTR shader,
                                  std::vector<uint32_t> &values);
  void RunLifetimeIntrinsicTest(ID3D12Device *pDevice, LPCSTR shader,
                                D3D_SHADER_MODEL shaderModel, bool useLibTarget,
                                LPCWSTR *pOptions, int numOptions,
                                std::vector<uint32_t> &values);
  void RunLifetimeIntrinsicComputeTest(
      ID3D12Device *pDevice, LPCSTR pShader,
      CComPtr<ID3D12DescriptorHeap> &pUavHeap,
      CComPtr<ID3D12RootSignature> &pRootSignature, LPCWSTR pTargetProfile,
      LPCWSTR *pOptions, int numOptions, std::vector<uint32_t> &values);
  void RunLifetimeIntrinsicLibTest(ID3D12Device *pDevice0, LPCSTR pShader,
                                   CComPtr<ID3D12RootSignature> &pRootSignature,
                                   LPCWSTR pTargetProfile, LPCWSTR *pOptions,
                                   int numOptions);

  void SetDescriptorHeap(ID3D12GraphicsCommandList *pCommandList,
                         ID3D12DescriptorHeap *pHeap) {
    ID3D12DescriptorHeap *const pHeaps[1] = {pHeap};
    pCommandList->SetDescriptorHeaps(1, pHeaps);
  }

  void WaitForSignal(ID3D12CommandQueue *pCQ, FenceObj &FO) {
    ::WaitForSignal(pCQ, FO.m_fence, FO.m_fenceEvent, FO.m_fenceValue++);
  }
};
#define WAVE_INTRINSIC_DXBC_GUARD                                              \
  "#ifdef USING_DXBC\r\n"                                                      \
  "uint WaveGetLaneIndex() { return 1; }\r\n"                                  \
  "uint WaveReadLaneFirst(uint u) { return u; }\r\n"                           \
  "bool WaveIsFirstLane() { return true; }\r\n"                                \
  "uint WaveGetLaneCount() { return 1; }\r\n"                                  \
  "uint WaveReadLaneAt(uint n, uint u) { return u; }\r\n"                      \
  "bool WaveActiveAnyTrue(bool b) { return b; }\r\n"                           \
  "bool WaveActiveAllTrue(bool b) { return false; }\r\n"                       \
  "uint WaveActiveAllEqual(uint u) { return u; }\r\n"                          \
  "uint4 WaveActiveBallot(bool b) { return 1; }\r\n"                           \
  "uint WaveActiveCountBits(uint u) { return 1; }\r\n"                         \
  "uint WaveActiveSum(uint u) { return 1; }\r\n"                               \
  "uint WaveActiveProduct(uint u) { return 1; }\r\n"                           \
  "uint WaveActiveBitAnd(uint u) { return 1; }\r\n"                            \
  "uint WaveActiveBitOr(uint u) { return 1; }\r\n"                             \
  "uint WaveActiveBitXor(uint u) { return 1; }\r\n"                            \
  "uint WaveActiveMin(uint u) { return 1; }\r\n"                               \
  "uint WaveActiveMax(uint u) { return 1; }\r\n"                               \
  "uint WavePrefixCountBits(uint u) { return 1; }\r\n"                         \
  "uint WavePrefixSum(uint u) { return 1; }\r\n"                               \
  "uint WavePrefixProduct(uint u) { return 1; }\r\n"                           \
  "uint QuadReadLaneAt(uint a, uint u) { return 1; }\r\n"                      \
  "uint QuadReadAcrossX(uint u) { return 1; }\r\n"                             \
  "uint QuadReadAcrossY(uint u) { return 1; }\r\n"                             \
  "uint QuadReadAcrossDiagonal(uint u) { return 1; }\r\n"                      \
  "#endif\r\n"

static void SetupComputeValuePattern(std::vector<uint32_t> &values,
                                     size_t count) {
  values.resize(count); // one element per dispatch group, in bytes
  for (size_t i = 0; i < count; ++i) {
    values[i] = (uint32_t)i;
  }
}

bool ExecutionTest::ExecutionTestClassSetup() { return DivergentClassSetup(); }

void ExecutionTest::RunRWByteBufferComputeTest(ID3D12Device *pDevice,
                                               LPCSTR pShader,
                                               std::vector<uint32_t> &values) {
  static const int DispatchGroupX = 1;
  static const int DispatchGroupY = 1;
  static const int DispatchGroupZ = 1;

  CComPtr<ID3D12GraphicsCommandList> pCommandList;
  CComPtr<ID3D12CommandQueue> pCommandQueue;
  CComPtr<ID3D12DescriptorHeap> pUavHeap;
  CComPtr<ID3D12CommandAllocator> pCommandAllocator;
  FenceObj FO;

  const UINT valueSizeInBytes = (UINT)values.size() * sizeof(uint32_t);
  CreateComputeCommandQueue(
      pDevice, L"RunRWByteBufferComputeTest Command Queue", &pCommandQueue);
  InitFenceObj(pDevice, &FO);

  // Describe and create a UAV descriptor heap.
  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors = 1;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

  VERIFY_SUCCEEDED(
      pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pUavHeap)));

  // Create root signature.
  CComPtr<ID3D12RootSignature> pRootSignature;
  {
    CD3DX12_DESCRIPTOR_RANGE ranges[1];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, 0);

    CD3DX12_ROOT_PARAMETER rootParameters[1];
    rootParameters[0].InitAsDescriptorTable(1, &ranges[0],
                                            D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr,
                           D3D12_ROOT_SIGNATURE_FLAG_NONE);

    CreateRootSignatureFromDesc(pDevice, &rootSignatureDesc, &pRootSignature);
  }

  // Create pipeline state object.
  CComPtr<ID3D12PipelineState> pComputeState;
  CreateComputePSO(pDevice, pRootSignature, pShader, L"cs_6_0", &pComputeState);

  // Create a command allocator and list for compute.
  VERIFY_SUCCEEDED(pDevice->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&pCommandAllocator)));
  VERIFY_SUCCEEDED(pDevice->CreateCommandList(
      0, D3D12_COMMAND_LIST_TYPE_COMPUTE, pCommandAllocator, pComputeState,
      IID_PPV_ARGS(&pCommandList)));
  pCommandList->SetName(
      L"ExecutionTest::RunRWByteButterComputeTest Command List");

  // Set up UAV resource.
  CComPtr<ID3D12Resource> pUavResource;
  CComPtr<ID3D12Resource> pReadBuffer;
  CComPtr<ID3D12Resource> pUploadResource;
  CreateTestUavs(pDevice, pCommandList, values.data(), valueSizeInBytes,
                 &pUavResource, &pUploadResource, &pReadBuffer);
  VERIFY_SUCCEEDED(pUavResource->SetName(L"RunRWByteBufferComputeText UAV"));
  VERIFY_SUCCEEDED(
      pReadBuffer->SetName(L"RunRWByteBufferComputeText UAV Read Buffer"));
  VERIFY_SUCCEEDED(pUploadResource->SetName(
      L"RunRWByteBufferComputeText UAV Upload Buffer"));

  // Close the command list and execute it to perform the GPU setup.
  pCommandList->Close();
  ExecuteCommandList(pCommandQueue, pCommandList);
  WaitForSignal(pCommandQueue, FO);
  VERIFY_SUCCEEDED(pCommandAllocator->Reset());
  VERIFY_SUCCEEDED(pCommandList->Reset(pCommandAllocator, pComputeState));

  // Run the compute shader and copy the results back to readable memory.
  {
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = (UINT)values.size();
    uavDesc.Buffer.StructureByteStride = 0;
    uavDesc.Buffer.CounterOffsetInBytes = 0;
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
    CD3DX12_CPU_DESCRIPTOR_HANDLE uavHandle(
        pUavHeap->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandleGpu(
        pUavHeap->GetGPUDescriptorHandleForHeapStart());
    pDevice->CreateUnorderedAccessView(pUavResource, nullptr, &uavDesc,
                                       uavHandle);
    SetDescriptorHeap(pCommandList, pUavHeap);
    pCommandList->SetComputeRootSignature(pRootSignature);
    pCommandList->SetComputeRootDescriptorTable(0, uavHandleGpu);
  }
  pCommandList->Dispatch(DispatchGroupX, DispatchGroupY, DispatchGroupZ);
  RecordTransitionBarrier(pCommandList, pUavResource,
                          D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                          D3D12_RESOURCE_STATE_COPY_SOURCE);
  pCommandList->CopyResource(pReadBuffer, pUavResource);
  pCommandList->Close();
  ExecuteCommandList(pCommandQueue, pCommandList);
  WaitForSignal(pCommandQueue, FO);
  {
    MappedData mappedData(pReadBuffer, valueSizeInBytes);
    uint32_t *pData = (uint32_t *)mappedData.data();
    memcpy(values.data(), pData, (size_t)valueSizeInBytes);
  }
  WaitForSignal(pCommandQueue, FO);
}

void ExecutionTest::RunLifetimeIntrinsicComputeTest(
    ID3D12Device *pDevice, LPCSTR pShader,
    CComPtr<ID3D12DescriptorHeap> &pUavHeap,
    CComPtr<ID3D12RootSignature> &pRootSignature, LPCWSTR pTargetProfile,
    LPCWSTR *pOptions, int numOptions, std::vector<uint32_t> &values) {
  // Create command queue.
  CComPtr<ID3D12CommandQueue> pCommandQueue;
  CreateComputeCommandQueue(pDevice, L"RunLifetimeIntrinsicTest Command Queue",
                            &pCommandQueue);

  FenceObj FO;
  InitFenceObj(pDevice, &FO);

  // Compile shader "main" and create pipeline state object.
  CComPtr<ID3D12PipelineState> pComputeState;
  CreateComputePSO(pDevice, pRootSignature, pShader, pTargetProfile,
                   &pComputeState, pOptions, numOptions);

  // Create a command allocator and list for compute.
  CComPtr<ID3D12CommandAllocator> pCommandAllocator;
  CComPtr<ID3D12GraphicsCommandList> pCommandList;
  VERIFY_SUCCEEDED(pDevice->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&pCommandAllocator)));
  VERIFY_SUCCEEDED(pDevice->CreateCommandList(
      0, D3D12_COMMAND_LIST_TYPE_COMPUTE, pCommandAllocator, pComputeState,
      IID_PPV_ARGS(&pCommandList)));
  pCommandList->SetName(
      L"ExecutionTest::RunLifetimeIntrinsicTest Command List");

  // Set up UAV resource.
  const UINT valueSizeInBytes = (UINT)values.size() * sizeof(uint32_t);
  CComPtr<ID3D12Resource> pUavResource;
  CComPtr<ID3D12Resource> pReadBuffer;
  CComPtr<ID3D12Resource> pUploadResource;
  CreateTestUavs(pDevice, pCommandList, values.data(), valueSizeInBytes,
                 &pUavResource, &pUploadResource, &pReadBuffer);
  VERIFY_SUCCEEDED(pUavResource->SetName(L"RunLifetimeIntrinsicTest UAV"));
  VERIFY_SUCCEEDED(
      pReadBuffer->SetName(L"RunLifetimeIntrinsicTest UAV Read Buffer"));
  VERIFY_SUCCEEDED(
      pUploadResource->SetName(L"RunLifetimeIntrinsicTest UAV Upload Buffer"));

  // Close the command list and execute it to perform the GPU setup.
  pCommandList->Close();
  ExecuteCommandList(pCommandQueue, pCommandList);
  WaitForSignal(pCommandQueue, FO);
  VERIFY_SUCCEEDED(pCommandAllocator->Reset());
  VERIFY_SUCCEEDED(pCommandList->Reset(pCommandAllocator, pComputeState));

  // Run the compute shader and copy the results back to readable memory.
  {
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = (UINT)values.size();
    uavDesc.Buffer.StructureByteStride = 0;
    uavDesc.Buffer.CounterOffsetInBytes = 0;
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
    CD3DX12_CPU_DESCRIPTOR_HANDLE uavHandle(
        pUavHeap->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandleGpu(
        pUavHeap->GetGPUDescriptorHandleForHeapStart());
    pDevice->CreateUnorderedAccessView(pUavResource, nullptr, &uavDesc,
                                       uavHandle);
    SetDescriptorHeap(pCommandList, pUavHeap);
    pCommandList->SetComputeRootSignature(pRootSignature);
    pCommandList->SetComputeRootDescriptorTable(0, uavHandleGpu);
  }

  static const int DispatchGroupX = 1;
  static const int DispatchGroupY = 1;
  static const int DispatchGroupZ = 1;
  pCommandList->Dispatch(DispatchGroupX, DispatchGroupY, DispatchGroupZ);
  RecordTransitionBarrier(pCommandList, pUavResource,
                          D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                          D3D12_RESOURCE_STATE_COPY_SOURCE);
  pCommandList->CopyResource(pReadBuffer, pUavResource);
  pCommandList->Close();
  ExecuteCommandList(pCommandQueue, pCommandList);
  WaitForSignal(pCommandQueue, FO);
  {
    MappedData mappedData(pReadBuffer, valueSizeInBytes);
    uint32_t *pData = (uint32_t *)mappedData.data();
    memcpy(values.data(), pData, (size_t)valueSizeInBytes);
  }
  WaitForSignal(pCommandQueue, FO);
}

void ExecutionTest::RunLifetimeIntrinsicLibTest(
    ID3D12Device *pDevice0, LPCSTR pShader,
    CComPtr<ID3D12RootSignature> &pRootSignature, LPCWSTR pTargetProfile,
    LPCWSTR *pOptions, int numOptions) {
  CComPtr<ID3D12Device5> pDevice;
  VERIFY_SUCCEEDED(pDevice0->QueryInterface(IID_PPV_ARGS(&pDevice)));

  // Create command queue.
  CComPtr<ID3D12CommandQueue> pCommandQueue;
  CreateCommandQueue(pDevice, L"RunLifetimeIntrinsicTest Command Queue",
                     &pCommandQueue, D3D12_COMMAND_LIST_TYPE_DIRECT);

  FenceObj FO;
  InitFenceObj(pDevice, &FO);

  // Compile raygen shader.
  CComPtr<ID3DBlob> pShaderLib;
  CompileFromText(pShader, L"RayGen", pTargetProfile, &pShaderLib, pOptions,
                  numOptions);

  // Describe and create the RT pipeline state object (RTPSO).
  CD3DX12_STATE_OBJECT_DESC stateObjectDesc(
      D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
  auto lib = stateObjectDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
  CD3DX12_SHADER_BYTECODE byteCode(pShaderLib);
  lib->SetDXILLibrary(&byteCode);
  lib->DefineExport(L"RayGen");

  const int payloadCount = 4;
  const int attributeCount = 2;
  const int maxRecursion = 2;
  stateObjectDesc.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>()
      ->Config(payloadCount * sizeof(float), attributeCount * sizeof(float));
  stateObjectDesc
      .CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>()
      ->Config(maxRecursion);

  // Create (local!) root sig subobject and associate with  shader.
  auto localRootSigSubObj =
      stateObjectDesc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
  localRootSigSubObj->SetRootSignature(pRootSignature);
  auto x = stateObjectDesc.CreateSubobject<
      CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
  x->SetSubobjectToAssociate(*localRootSigSubObj);
  x->AddExport(L"RayGen");

  CComPtr<ID3D12StateObject> pStateObject;
  VERIFY_SUCCEEDED(
      pDevice->CreateStateObject(stateObjectDesc, IID_PPV_ARGS(&pStateObject)));

  // Create a command allocator and list.
  CComPtr<ID3D12CommandAllocator> pCommandAllocator;
  CComPtr<ID3D12GraphicsCommandList4> pCommandList;
  VERIFY_SUCCEEDED(pDevice->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCommandAllocator)));
  VERIFY_SUCCEEDED(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                              pCommandAllocator, nullptr,
                                              IID_PPV_ARGS(&pCommandList)));
  pCommandList->SetPipelineState1(pStateObject);
  pCommandList->SetName(
      L"ExecutionTest::RunLifetimeIntrinsicTest Command List");

  // Close the command list and execute it to kick-off compilation in the
  // driver. NOTE: We don't care about anything else, so we're not setting up
  // any resources and don't actually execute the shader.
  pCommandList->Close();
  ExecuteCommandList(pCommandQueue, pCommandList);
  WaitForSignal(pCommandQueue, FO);
}

void ExecutionTest::RunLifetimeIntrinsicTest(ID3D12Device *pDevice,
                                             LPCSTR pShader,
                                             D3D_SHADER_MODEL shaderModel,
                                             bool useLibTarget,
                                             LPCWSTR *pOptions, int numOptions,
                                             std::vector<uint32_t> &values) {
  LPCWSTR pTargetProfile;
  switch (shaderModel) {
  default:
    pTargetProfile = useLibTarget ? L"lib_6_3" : L"cs_6_0";
    break; // Default to 6.3 for lib, 6.0 otherwise.
  case D3D_SHADER_MODEL_6_0:
    pTargetProfile = useLibTarget ? L"lib_6_0" : L"cs_6_0";
    break;
  case D3D_SHADER_MODEL_6_3:
    pTargetProfile = useLibTarget ? L"lib_6_3" : L"cs_6_3";
    break;
  case D3D_SHADER_MODEL_6_5:
    pTargetProfile = useLibTarget ? L"lib_6_5" : L"cs_6_5";
    break;
  case D3D_SHADER_MODEL_6_6:
    pTargetProfile = useLibTarget ? L"lib_6_6" : L"cs_6_6";
    break;
  }

  // Describe a UAV descriptor heap.
  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors = 1;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

  // Create the UAV descriptor heap.
  CComPtr<ID3D12DescriptorHeap> pUavHeap;
  VERIFY_SUCCEEDED(
      pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pUavHeap)));

  // Create root signature.
  CComPtr<ID3D12RootSignature> pRootSignature;
  {
    CD3DX12_DESCRIPTOR_RANGE ranges[1];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, 0);

    CD3DX12_ROOT_PARAMETER rootParameters[1];
    rootParameters[0].InitAsDescriptorTable(1, &ranges[0],
                                            D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    D3D12_ROOT_SIGNATURE_FLAGS rootSigFlag =
        useLibTarget ? D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE
                     : D3D12_ROOT_SIGNATURE_FLAG_NONE;
    rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr,
                           rootSigFlag);

    CreateRootSignatureFromDesc(pDevice, &rootSignatureDesc, &pRootSignature);
  }

  if (useLibTarget) {
    RunLifetimeIntrinsicLibTest(pDevice, pShader, pRootSignature,
                                pTargetProfile, pOptions, numOptions);
  } else {
    RunLifetimeIntrinsicComputeTest(pDevice, pShader, pUavHeap, pRootSignature,
                                    pTargetProfile, pOptions, numOptions,
                                    values);
  }
}

TEST_F(ExecutionTest, LifetimeIntrinsicTest) {
  // The only thing we test here is that existence of lifetime intrinsics or
  // their fallback replacement (store undef or store zeroinitializer) do not
  // cause any issues in the runtime and driver stack.
  // The easiest way to force placement of intrinsics is to create an array in
  // a local scope that is dynamically indexed. It must not be optimized away,
  // so we do some bogus initialization that prevents this. Since all the code
  // is guarded by a conditional that is dynamically always false, the actual
  // effect of the shader is that the same value that was read is written back.
  static const char *pShader = R"(
    RWByteAddressBuffer g_bab : register(u0);

    void fn(uint GI) {
      const uint addr = GI * 4;
      const int val = g_bab.Load(addr);
      int res = val;
      if (val < 0) { // Never true.
        int arr[200];
        for (int i = 0; i < 200; ++i) {
            arr[i] = arr[val - i];
        }
        res += arr[val];
      }
      g_bab.Store(addr, (uint)res);
    }

    [numthreads(8,8,1)]
    void main(uint GI : SV_GroupIndex) {
      fn(GI);
    }

    [shader("raygeneration")]
    void RayGen() {
      const uint d = DispatchRaysIndex().x;
      const uint g = g > 64 ? 63 : g;
      fn(g);
    }
  )";
  static const int NumThreadsX = 8;
  static const int NumThreadsY = 8;
  static const int NumThreadsZ = 1;
  static const int ThreadsPerGroup = NumThreadsX * NumThreadsY * NumThreadsZ;
  static const int DispatchGroupCount = 1;

  CComPtr<ID3D12Device> pDevice;
  bool bSM_6_6_Supported = CreateDevice(&pDevice, D3D_SHADER_MODEL_6_6, false);
  bool bSM_6_3_Supported = bSM_6_6_Supported;
  if (!bSM_6_6_Supported) {
    // Try 6.3 for downlevel DXR case
    bSM_6_3_Supported = CreateDevice(&pDevice, D3D_SHADER_MODEL_6_3, false);
  }
  if (!bSM_6_3_Supported) {
    // Otherwise, 6.0 better be supported for compute case
    VERIFY_IS_TRUE(CreateDevice(&pDevice, D3D_SHADER_MODEL_6_0, false));
  }
  bool bDXRSupported =
      bSM_6_3_Supported && DoesDeviceSupportRayTracing(pDevice);

  if (!bSM_6_6_Supported) {
    WEX::Logging::Log::Comment(
        L"Native lifetime markers skipped, device does not support SM 6.6");
  }
  if (!bDXRSupported) {
    WEX::Logging::Log::Comment(
        L"DXR lifetime tests skipped, device does not support DXR");
  }

  std::vector<uint32_t> values;
  SetupComputeValuePattern(values, ThreadsPerGroup * DispatchGroupCount);

  // Run a number of tests for different configurations that will cause
  // lifetime intrinsics to be:
  // - placed directly
  // - translated to an undef store
  // - translated to a zeroinitializer store
  // against compute and DXR targets, downlevel and SM 6.6:
  // - downlevel: cs_6_0, lib_6_3 (DXR)
  // - cs_6_6, lib_6_6 (DXR)

  VERIFY_ARE_EQUAL(values[1], (uint32_t)1);

  LPCWSTR optsBase[] = {L"-enable-lifetime-markers"};
  LPCWSTR optsZeroStore[] = {L"-enable-lifetime-markers",
                             L"-force-zero-store-lifetimes"};

  WEX::Logging::Log::Comment(L"==== cs_6_0 with default translation");
  RunLifetimeIntrinsicTest(pDevice, pShader, D3D_SHADER_MODEL_6_0, false,
                           optsBase, _countof(optsBase), values);
  VERIFY_ARE_EQUAL(values[1], (uint32_t)1);

  if (bDXRSupported) {
    WEX::Logging::Log::Comment(L"==== DXR lib_6_3 with default translation");
    RunLifetimeIntrinsicTest(pDevice, pShader, D3D_SHADER_MODEL_6_3, true,
                             optsBase, _countof(optsBase), values);
    VERIFY_ARE_EQUAL(values[1], (uint32_t)1);
  }

  WEX::Logging::Log::Comment(L"==== cs_6_0 with zeroinitializer translation");
  RunLifetimeIntrinsicTest(pDevice, pShader, D3D_SHADER_MODEL_6_0, false,
                           optsZeroStore, _countof(optsZeroStore), values);
  VERIFY_ARE_EQUAL(values[1], (uint32_t)1);

  if (bDXRSupported) {
    WEX::Logging::Log::Comment(
        L"==== DXR lib_6_3 with zeroinitializer translation");
    RunLifetimeIntrinsicTest(pDevice, pShader, D3D_SHADER_MODEL_6_3, true,
                             optsZeroStore, _countof(optsZeroStore), values);
    VERIFY_ARE_EQUAL(values[1], (uint32_t)1);
  }

  if (bSM_6_6_Supported) {
    WEX::Logging::Log::Comment(L"==== cs_6_6 with zeroinitializer translation");
    RunLifetimeIntrinsicTest(pDevice, pShader, D3D_SHADER_MODEL_6_6, false,
                             optsZeroStore, _countof(optsZeroStore), values);
    VERIFY_ARE_EQUAL(values[1], (uint32_t)1);

    if (bDXRSupported) {
      WEX::Logging::Log::Comment(
          L"==== DXR lib_6_6 with zeroinitializer translation");
      RunLifetimeIntrinsicTest(pDevice, pShader, D3D_SHADER_MODEL_6_6, true,
                               optsZeroStore, _countof(optsZeroStore), values);
      VERIFY_ARE_EQUAL(values[1], (uint32_t)1);
    }

    WEX::Logging::Log::Comment(L"==== cs_6_6 with native lifetime markers");
    RunLifetimeIntrinsicTest(pDevice, pShader, D3D_SHADER_MODEL_6_6, false,
                             optsBase, _countof(optsBase), values);
    VERIFY_ARE_EQUAL(values[1], (uint32_t)1);

    if (bDXRSupported) {
      WEX::Logging::Log::Comment(
          L"==== DXR lib_6_6 with native lifetime markers");
      RunLifetimeIntrinsicTest(pDevice, pShader, D3D_SHADER_MODEL_6_6, true,
                               optsBase, _countof(optsBase), values);
      VERIFY_ARE_EQUAL(values[1], (uint32_t)1);
    }
  }
}

TEST_F(ExecutionTest, BasicComputeTest) {
#ifndef _HLK_CONF
  //
  // BasicComputeTest is a simple compute shader that can be used as the basis
  // for more interesting compute execution tests.
  // The HLSL is compatible with shader models <=5.1 to allow using the DXBC
  // rendering code paths for comparison.
  //
  static const char pShader[] = "RWByteAddressBuffer g_bab : register(u0);\r\n"
                                "[numthreads(8,8,1)]\r\n"
                                "void main(uint GI : SV_GroupIndex) {"
                                "  uint addr = GI * 4;\r\n"
                                "  uint val = g_bab.Load(addr);\r\n"
                                "  DeviceMemoryBarrierWithGroupSync();\r\n"
                                "  g_bab.Store(addr, val + 1);\r\n"
                                "}";
  static const int NumThreadsX = 8;
  static const int NumThreadsY = 8;
  static const int NumThreadsZ = 1;
  static const int ThreadsPerGroup = NumThreadsX * NumThreadsY * NumThreadsZ;
  static const int DispatchGroupCount = 1;

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice))
    return;

  std::vector<uint32_t> values;
  SetupComputeValuePattern(values, ThreadsPerGroup * DispatchGroupCount);
  VERIFY_ARE_EQUAL(values[0], (uint32_t)0);
  RunRWByteBufferComputeTest(pDevice, pShader, values);
  VERIFY_ARE_EQUAL(values[0], (uint32_t)1);
#endif
}

TEST_F(ExecutionTest, BasicTriangleTest) {
#ifndef _HLK_CONF
  static const UINT FrameCount = 2;
  static const UINT m_width = 320;
  static const UINT m_height = 200;
  static const float m_aspectRatio =
      static_cast<float>(m_width) / static_cast<float>(m_height);

  struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
  };

  // Pipeline objects.
  CComPtr<ID3D12Device> pDevice;
  CComPtr<ID3D12Resource> pRenderTarget;
  CComPtr<ID3D12CommandAllocator> pCommandAllocator;
  CComPtr<ID3D12CommandQueue> pCommandQueue;
  CComPtr<ID3D12RootSignature> pRootSig;
  CComPtr<ID3D12DescriptorHeap> pRtvHeap;
  CComPtr<ID3D12PipelineState> pPipelineState;
  CComPtr<ID3D12GraphicsCommandList> pCommandList;
  CComPtr<ID3D12Resource> pReadBuffer;
  UINT rtvDescriptorSize;

  CComPtr<ID3D12Resource> pVertexBuffer;
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

  // Synchronization objects.
  FenceObj FO;

  // Shaders.
  static const char pShaders[] =
      "struct PSInput {\r\n"
      "  float4 position : SV_POSITION;\r\n"
      "  float4 color : COLOR;\r\n"
      "};\r\n\r\n"
      "PSInput VSMain(float4 position : POSITION, float4 color : COLOR) {\r\n"
      "  PSInput result;\r\n"
      "\r\n"
      "  result.position = position;\r\n"
      "  result.color = color;\r\n"
      "  return result;\r\n"
      "}\r\n\r\n"
      "float4 PSMain(PSInput input) : SV_TARGET {\r\n"
      "  return 1; //input.color;\r\n"
      "};\r\n";

  if (!CreateDevice(&pDevice))
    return;

  struct BasicTestChecker {
    CComPtr<ID3D12Device> m_pDevice;
    CComPtr<ID3D12InfoQueue> m_pInfoQueue;
    bool m_OK = false;
    void SetOK(bool value) { m_OK = value; }
    BasicTestChecker(ID3D12Device *pDevice) : m_pDevice(pDevice) {
      if (FAILED(m_pDevice.QueryInterface(&m_pInfoQueue)))
        return;
      m_pInfoQueue->PushEmptyStorageFilter();
      m_pInfoQueue->PushEmptyRetrievalFilter();
    }
    ~BasicTestChecker() {
      if (!m_OK && m_pInfoQueue != nullptr) {
        UINT64 count = m_pInfoQueue->GetNumStoredMessages();
        bool invalidBytecodeFound = false;
        CAtlArray<BYTE> m_pBytes;
        for (UINT64 i = 0; i < count; ++i) {
          SIZE_T len = 0;
          if (FAILED(m_pInfoQueue->GetMessageA(i, nullptr, &len)))
            continue;
          if (m_pBytes.GetCount() < len && !m_pBytes.SetCount(len))
            continue;
          D3D12_MESSAGE *pMsg = (D3D12_MESSAGE *)m_pBytes.GetData();
          if (FAILED(m_pInfoQueue->GetMessageA(i, pMsg, &len)))
            continue;
          if (pMsg->ID ==
                  D3D12_MESSAGE_ID_CREATEVERTEXSHADER_INVALIDSHADERBYTECODE ||
              pMsg->ID ==
                  D3D12_MESSAGE_ID_CREATEPIXELSHADER_INVALIDSHADERBYTECODE) {
            invalidBytecodeFound = true;
            break;
          }
        }
        if (invalidBytecodeFound) {
          LogCommentFmt(L"%s", L"Found an invalid bytecode message. This "
                               L"typically indicates that experimental mode "
                               L"is not set up properly.");
          if (!GetTestParamBool(L"ExperimentalShaders")) {
            LogCommentFmt(
                L"Note that the ExperimentalShaders test parameter isn't set.");
          }
        } else {
          LogCommentFmt(L"Did not find corrupt pixel or vertex shaders in "
                        L"queue - dumping complete queue.");
          WriteInfoQueueMessages(nullptr, OutputFn, m_pInfoQueue);
        }
      }
    }
    static void __stdcall OutputFn(void *pCtx, const wchar_t *pMsg) {
      UNREFERENCED_PARAMETER(pCtx);
      LogCommentFmt(L"%s", pMsg);
    }
  };
  BasicTestChecker BTC(pDevice);
  {
    InitFenceObj(pDevice, &FO);
    CreateRtvDescriptorHeap(pDevice, FrameCount, &pRtvHeap, &rtvDescriptorSize);
    CreateRenderTargetAndReadback(pDevice, pRtvHeap, m_width, m_height,
                                  &pRenderTarget, &pReadBuffer);

    // Create an empty root signature.
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(
        0, nullptr, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    CreateRootSignatureFromDesc(pDevice, &rootSignatureDesc, &pRootSig);

    // Create the pipeline state, which includes compiling and loading shaders.
    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
    D3D12_INPUT_LAYOUT_DESC InputLayout = {inputElementDescs,
                                           _countof(inputElementDescs)};
    CreateGraphicsPSO(pDevice, &InputLayout, pRootSig, pShaders,
                      &pPipelineState);

    CreateGraphicsCommandQueueAndList(pDevice, &pCommandQueue,
                                      &pCommandAllocator, &pCommandList,
                                      pPipelineState);

    // Define the geometry for a triangle.
    Vertex triangleVertices[] = {
        {{0.0f, 0.25f * m_aspectRatio, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{0.25f, -0.25f * m_aspectRatio, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{-0.25f, -0.25f * m_aspectRatio, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}};

    CreateVertexBuffer(pDevice, triangleVertices, &pVertexBuffer,
                       &vertexBufferView);
    WaitForSignal(pCommandQueue, FO);
  }

  // Render and execute the command list.
  RecordRenderAndReadback(pCommandList, pRtvHeap, rtvDescriptorSize, 1,
                          &vertexBufferView, pRootSig, pRenderTarget,
                          pReadBuffer);
  VERIFY_SUCCEEDED(pCommandList->Close());
  ExecuteCommandList(pCommandQueue, pCommandList);

  // Wait for previous frame.
  WaitForSignal(pCommandQueue, FO);

  // At this point, we've verified that execution succeeded with DXIL.
  BTC.SetOK(true);

  // Read back to CPU and examine contents.
  {
    MappedData data(pReadBuffer, m_width * m_height * 4);
    const uint32_t *pPixels = (uint32_t *)data.data();
    if (SaveImages()) {
      SavePixelsToFile(pPixels, DXGI_FORMAT_R8G8B8A8_UNORM, m_width, m_height,
                       L"basic.bmp");
    }
    uint32_t top = pPixels[m_width / 2]; // Top center.
    uint32_t mid =
        pPixels[m_width / 2 + m_width * (m_height / 2)]; // Middle center.
    VERIFY_ARE_EQUAL(0xff663300, top);                   // clear color
    VERIFY_ARE_EQUAL(0xffffffff, mid);                   // white
  }
#endif
}

TEST_F(ExecutionTest, Int64Test) {
  static const char pShader[] = "RWByteAddressBuffer g_bab : register(u0);\r\n"
                                "[numthreads(8,8,1)]\r\n"
                                "void main(uint GI : SV_GroupIndex) {"
                                "  uint addr = GI * 4;\r\n"
                                "  uint val = g_bab.Load(addr);\r\n"
                                "  uint64_t u64 = val;\r\n"
                                "  u64 *= val;\r\n"
                                "  g_bab.Store(addr, (uint)(u64 >> 32));\r\n"
                                "}";
  static const int NumThreadsX = 8;
  static const int NumThreadsY = 8;
  static const int NumThreadsZ = 1;
  static const int ThreadsPerGroup = NumThreadsX * NumThreadsY * NumThreadsZ;
  static const int DispatchGroupCount = 1;

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice))
    return;

  if (!DoesDeviceSupportInt64(pDevice)) {
    // Optional feature, so it's correct to not support it if declared as such.
    WEX::Logging::Log::Comment(L"Device does not support int64 operations.");
    return;
  }
  std::vector<uint32_t> values;
  SetupComputeValuePattern(values, ThreadsPerGroup * DispatchGroupCount);
  VERIFY_ARE_EQUAL(values[0], (uint32_t)0);
  RunRWByteBufferComputeTest(pDevice, pShader, values);
  VERIFY_ARE_EQUAL(values[0], (uint32_t)0);
}

TEST_F(ExecutionTest, SignTest) {
  static const char pShader[] = "RWByteAddressBuffer g_bab : register(u0);\r\n"
                                "[numthreads(8,1,1)]\r\n"
                                "void main(uint GI : SV_GroupIndex) {"
                                "  uint addr = GI * 4;\r\n"
                                "  int val = g_bab.Load(addr);\r\n"
                                "  g_bab.Store(addr, (uint)(sign(val)));\r\n"
                                "}";

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice))
    return;

  const uint32_t neg1 = (uint32_t)-1;
  uint32_t origValues[] = {(uint32_t)-3, (uint32_t)-2, neg1, 0, 1, 2, 3, 4};
  std::vector<uint32_t> values(origValues, origValues + _countof(origValues));

  RunRWByteBufferComputeTest(pDevice, pShader, values);
  VERIFY_ARE_EQUAL(values[0], neg1);
  VERIFY_ARE_EQUAL(values[1], neg1);
  VERIFY_ARE_EQUAL(values[2], neg1);
  VERIFY_ARE_EQUAL(values[3], (uint32_t)0);
  VERIFY_ARE_EQUAL(values[4], (uint32_t)1);
  VERIFY_ARE_EQUAL(values[5], (uint32_t)1);
  VERIFY_ARE_EQUAL(values[6], (uint32_t)1);
  VERIFY_ARE_EQUAL(values[7], (uint32_t)1);
}

TEST_F(ExecutionTest, WaveIntrinsicsDDITest) {
#ifndef _HLK_CONF
  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice))
    return;
  D3D12_FEATURE_DATA_D3D12_OPTIONS1 O;
  if (FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS1, &O, sizeof(O))))
    return;
  bool waveSupported = O.WaveOps;
  UINT laneCountMin = O.WaveLaneCountMin;
  UINT laneCountMax = O.WaveLaneCountMax;
  LogCommentFmt(L"WaveOps %i, WaveLaneCountMin %u, WaveLaneCountMax %u",
                waveSupported, laneCountMin, laneCountMax);
  VERIFY_IS_TRUE(laneCountMin <= laneCountMax);
  if (waveSupported) {
    VERIFY_IS_TRUE(laneCountMin > 0 && laneCountMax > 0);
  } else {
    VERIFY_IS_TRUE(laneCountMin == 0 && laneCountMax == 0);
  }
#endif
}

TEST_F(ExecutionTest, WaveIntrinsicsTest) {
#ifndef _HLK_CONF
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  struct PerThreadData {
    uint32_t id, flags, laneIndex, laneCount, firstLaneId, preds, firstlaneX,
        lane1X;
    uint32_t allBC, allSum, allProd, allAND, allOR, allXOR, allMin, allMax;
    uint32_t pfBC, pfSum, pfProd;
    uint32_t ballot[4];
    uint32_t diver;  // divergent value, used in calculation
    int32_t i_diver; // divergent value, used in calculation
    int32_t i_allMax, i_allMin, i_allSum, i_allProd;
    int32_t i_pfSum, i_pfProd;
  };
  static const char pShader[] = WAVE_INTRINSIC_DXBC_GUARD
      "struct PerThreadData {\r\n"
      " uint id, flags, laneIndex, laneCount, firstLaneId, preds, firstlaneX, "
      "lane1X;\r\n"
      " uint allBC, allSum, allProd, allAND, allOR, allXOR, allMin, allMax;\r\n"
      " uint pfBC, pfSum, pfProd;\r\n"
      " uint4 ballot;\r\n"
      " uint diver;\r\n"
      " int i_diver;\r\n"
      " int i_allMax, i_allMin, i_allSum, i_allProd;\r\n"
      " int i_pfSum, i_pfProd;\r\n"
      "};\r\n"
      "RWStructuredBuffer<PerThreadData> g_sb : register(u0);\r\n"
      "[numthreads(8,8,1)]\r\n"
      "void main(uint GI : SV_GroupIndex, uint3 GTID : SV_GroupThreadID) {"
      "  PerThreadData pts = g_sb[GI];\r\n"
      "  uint diver = GTID.x + 2;\r\n"
      "  pts.diver = diver;\r\n"
      "  pts.flags = 0;\r\n"
      "  pts.preds = 0;\r\n"
      "  if (WaveIsFirstLane()) pts.flags |= 1;\r\n"
      "  pts.laneIndex = WaveGetLaneIndex();\r\n"
      "  pts.laneCount = WaveGetLaneCount();\r\n"
      "  pts.firstLaneId = WaveReadLaneFirst(pts.id);\r\n"
      "  pts.preds |= ((WaveActiveAnyTrue(diver == 1) ? 1 : 0) << 0);\r\n"
      "  pts.preds |= ((WaveActiveAllTrue(diver == 1) ? 1 : 0) << 1);\r\n"
      "  pts.preds |= ((WaveActiveAllEqual(diver) ? 1 : 0) << 2);\r\n"
      "  pts.preds |= ((WaveActiveAllEqual(GTID.z) ? 1 : 0) << 3);\r\n"
      "  pts.preds |= ((WaveActiveAllEqual(WaveReadLaneFirst(diver)) ? 1 : 0) "
      "<< 4);\r\n"
      "  pts.ballot = WaveActiveBallot(diver > 3);\r\n"
      "  pts.firstlaneX = WaveReadLaneFirst(GTID.x);\r\n"
      "  pts.lane1X = WaveReadLaneAt(GTID.x, 1);\r\n"
      "\r\n"
      "  pts.allBC = WaveActiveCountBits(diver > 3);\r\n"
      "  pts.allSum = WaveActiveSum(diver);\r\n"
      "  pts.allProd = WaveActiveProduct(diver);\r\n"
      "  pts.allAND = WaveActiveBitAnd(diver);\r\n"
      "  pts.allOR = WaveActiveBitOr(diver);\r\n"
      "  pts.allXOR = WaveActiveBitXor(diver);\r\n"
      "  pts.allMin = WaveActiveMin(diver);\r\n"
      "  pts.allMax = WaveActiveMax(diver);\r\n"
      "\r\n"
      "  pts.pfBC = WavePrefixCountBits(diver > 3);\r\n"
      "  pts.pfSum = WavePrefixSum(diver);\r\n"
      "  pts.pfProd = WavePrefixProduct(diver);\r\n"
      "\r\n"
      "  int i_diver = pts.i_diver;\r\n"
      "  pts.i_allMax = WaveActiveMax(i_diver);\r\n"
      "  pts.i_allMin = WaveActiveMin(i_diver);\r\n"
      "  pts.i_allSum = WaveActiveSum(i_diver);\r\n"
      "  pts.i_allProd = WaveActiveProduct(i_diver);\r\n"
      "  pts.i_pfSum = WavePrefixSum(i_diver);\r\n"
      "  pts.i_pfProd = WavePrefixProduct(i_diver);\r\n"
      "\r\n"
      "  g_sb[GI] = pts;\r\n"
      "}";
  static const int NumtheadsX = 8;
  static const int NumtheadsY = 8;
  static const int NumtheadsZ = 1;
  static const int ThreadsPerGroup = NumtheadsX * NumtheadsY * NumtheadsZ;
  static const int DispatchGroupCount = 1;

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice))
    return;

  if (!DoesDeviceSupportWaveOps(pDevice)) {
    // Optional feature, so it's correct to not support it if declared as such.
    WEX::Logging::Log::Comment(L"Device does not support wave operations.");
    return;
  }

  std::vector<PerThreadData> values;
  values.resize(ThreadsPerGroup * DispatchGroupCount);
  for (size_t i = 0; i < values.size(); ++i) {
    memset(&values[i], 0, sizeof(PerThreadData));
    values[i].id = (uint32_t)i;
    values[i].i_diver = (int)i;
    values[i].i_diver *= (i % 2) ? 1 : -1;
  }

  static const int DispatchGroupX = 1;
  static const int DispatchGroupY = 1;
  static const int DispatchGroupZ = 1;

  CComPtr<ID3D12GraphicsCommandList> pCommandList;
  CComPtr<ID3D12CommandQueue> pCommandQueue;
  CComPtr<ID3D12DescriptorHeap> pUavHeap;
  CComPtr<ID3D12CommandAllocator> pCommandAllocator;
  FenceObj FO;
  bool dxbc = UseDxbc();

  const size_t valueSizeInBytes = values.size() * sizeof(PerThreadData);
  CreateComputeCommandQueue(pDevice, L"WaveIntrinsicsTest Command Queue",
                            &pCommandQueue);
  InitFenceObj(pDevice, &FO);

  // Describe and create a UAV descriptor heap.
  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors = 1;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  VERIFY_SUCCEEDED(
      pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pUavHeap)));

  // Create root signature.
  CComPtr<ID3D12RootSignature> pRootSignature;
  {
    CD3DX12_DESCRIPTOR_RANGE ranges[1];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, 0);

    CD3DX12_ROOT_PARAMETER rootParameters[1];
    rootParameters[0].InitAsDescriptorTable(1, &ranges[0],
                                            D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr,
                           D3D12_ROOT_SIGNATURE_FLAG_NONE);

    CComPtr<ID3DBlob> signature;
    CComPtr<ID3DBlob> error;
    VERIFY_SUCCEEDED(D3D12SerializeRootSignature(
        &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    VERIFY_SUCCEEDED(pDevice->CreateRootSignature(
        0, signature->GetBufferPointer(), signature->GetBufferSize(),
        IID_PPV_ARGS(&pRootSignature)));
  }

  // Create pipeline state object.
  CComPtr<ID3D12PipelineState> pComputeState;
  CreateComputePSO(pDevice, pRootSignature, pShader, L"cs_6_0", &pComputeState);

  // Create a command allocator and list for compute.
  VERIFY_SUCCEEDED(pDevice->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&pCommandAllocator)));
  VERIFY_SUCCEEDED(pDevice->CreateCommandList(
      0, D3D12_COMMAND_LIST_TYPE_COMPUTE, pCommandAllocator, pComputeState,
      IID_PPV_ARGS(&pCommandList)));

  // Set up UAV resource.
  CComPtr<ID3D12Resource> pUavResource;
  CComPtr<ID3D12Resource> pReadBuffer;
  CComPtr<ID3D12Resource> pUploadResource;
  CreateTestUavs(pDevice, pCommandList, values.data(), (UINT)valueSizeInBytes,
                 &pUavResource, &pUploadResource, &pReadBuffer);

  // Close the command list and execute it to perform the GPU setup.
  pCommandList->Close();
  ExecuteCommandList(pCommandQueue, pCommandList);
  WaitForSignal(pCommandQueue, FO);
  VERIFY_SUCCEEDED(pCommandAllocator->Reset());
  VERIFY_SUCCEEDED(pCommandList->Reset(pCommandAllocator, pComputeState));

  // Run the compute shader and copy the results back to readable memory.
  {
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = (UINT)values.size();
    uavDesc.Buffer.StructureByteStride = sizeof(PerThreadData);
    uavDesc.Buffer.CounterOffsetInBytes = 0;
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
    CD3DX12_CPU_DESCRIPTOR_HANDLE uavHandle(
        pUavHeap->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandleGpu(
        pUavHeap->GetGPUDescriptorHandleForHeapStart());
    pDevice->CreateUnorderedAccessView(pUavResource, nullptr, &uavDesc,
                                       uavHandle);
    SetDescriptorHeap(pCommandList, pUavHeap);
    pCommandList->SetComputeRootSignature(pRootSignature);
    pCommandList->SetComputeRootDescriptorTable(0, uavHandleGpu);
  }
  pCommandList->Dispatch(DispatchGroupX, DispatchGroupY, DispatchGroupZ);
  RecordTransitionBarrier(pCommandList, pUavResource,
                          D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                          D3D12_RESOURCE_STATE_COPY_SOURCE);
  pCommandList->CopyResource(pReadBuffer, pUavResource);
  pCommandList->Close();
  ExecuteCommandList(pCommandQueue, pCommandList);
  WaitForSignal(pCommandQueue, FO);
  {
    MappedData mappedData(pReadBuffer, (UINT)valueSizeInBytes);
    PerThreadData *pData = (PerThreadData *)mappedData.data();
    memcpy(values.data(), pData, valueSizeInBytes);

    // Gather some general data.
    // The 'firstLaneId' captures a unique number per first-lane per wave.
    // Counting the number distinct firstLaneIds gives us the number of waves.
    std::vector<uint32_t> firstLaneIds;
    for (size_t i = 0; i < values.size(); ++i) {
      PerThreadData &pts = values[i];
      uint32_t firstLaneId = pts.firstLaneId;
      if (!contains(firstLaneIds, firstLaneId)) {
        firstLaneIds.push_back(firstLaneId);
      }
    }

    // Waves should cover 4 threads or more.
    LogCommentFmt(L"Found %u distinct lane ids: %u", firstLaneIds.size());
    if (!dxbc) {
      VERIFY_IS_GREATER_THAN_OR_EQUAL(values.size() / 4, firstLaneIds.size());
    }

    // Now, group threads into waves.
    std::map<uint32_t, std::unique_ptr<std::vector<PerThreadData *>>> waves;
    for (size_t i = 0; i < firstLaneIds.size(); ++i) {
      waves[firstLaneIds[i]] = std::make_unique<std::vector<PerThreadData *>>();
    }
    for (size_t i = 0; i < values.size(); ++i) {
      PerThreadData &pts = values[i];
      std::unique_ptr<std::vector<PerThreadData *>> &wave =
          waves[pts.firstLaneId];
      wave->push_back(&pts);
    }

    // Verify that all the wave values are coherent across the wave.
    for (size_t i = 0; i < values.size(); ++i) {
      PerThreadData &pts = values[i];
      std::unique_ptr<std::vector<PerThreadData *>> &wave =
          waves[pts.firstLaneId];
      // Sort the lanes by increasing lane ID.
      struct LaneIdOrderPred {
        bool operator()(PerThreadData *a, PerThreadData *b) {
          return a->laneIndex < b->laneIndex;
        }
      };
      std::sort(wave.get()->begin(), wave.get()->end(), LaneIdOrderPred());

      // Verify some interesting properties of the first lane.
      uint32_t pfBC, pfSum, pfProd;
      int32_t i_pfSum, i_pfProd;
      int32_t i_allMax, i_allMin;
      {
        PerThreadData *ptdFirst = wave->front();
        VERIFY_IS_TRUE(0 != (ptdFirst->flags & 1)); // FirstLane sets this bit.
        VERIFY_IS_TRUE(0 == ptdFirst->pfBC);
        VERIFY_IS_TRUE(0 == ptdFirst->pfSum);
        VERIFY_IS_TRUE(1 == ptdFirst->pfProd);
        VERIFY_IS_TRUE(0 == ptdFirst->i_pfSum);
        VERIFY_IS_TRUE(1 == ptdFirst->i_pfProd);
        pfBC = (ptdFirst->diver > 3) ? 1 : 0;
        pfSum = ptdFirst->diver;
        pfProd = ptdFirst->diver;
        i_pfSum = ptdFirst->i_diver;
        i_pfProd = ptdFirst->i_diver;
        i_allMax = i_allMin = ptdFirst->i_diver;
      }

      // Calculate values which take into consideration all lanes.
      uint32_t preds = 0;
      preds |= 1 << 1; // AllTrue starts true, switches to false if needed.
      preds |= 1 << 2; // AllEqual starts true, switches to false if needed.
      preds |= 1 << 3; // WaveActiveAllEqual(GTID.z) is always true
      preds |=
          1
          << 4; // (WaveActiveAllEqual(WaveReadLaneFirst(diver)) is always true
      uint32_t ballot[4] = {0, 0, 0, 0};
      int32_t i_allSum = 0, i_allProd = 1;
      for (size_t n = 0; n < wave->size(); ++n) {
        std::vector<PerThreadData *> &lanes = *wave.get();
        // pts.preds |= ((WaveActiveAnyTrue(diver == 1) ? 1 : 0) << 0);
        if (lanes[n]->diver == 1)
          preds |= (1 << 0);
        // pts.preds |= ((WaveActiveAllTrue(diver == 1) ? 1 : 0) << 1);
        if (lanes[n]->diver != 1)
          preds &= ~(1 << 1);
        // pts.preds |= ((WaveActiveAllEqual(diver) ? 1 : 0) << 2);
        if (lanes[0]->diver != lanes[n]->diver)
          preds &= ~(1 << 2);
        // pts.ballot = WaveActiveBallot(diver > 3);\r\n"
        if (lanes[n]->diver > 3) {
          // This is the uint4 result layout:
          // .x -> bits  0 .. 31
          // .y -> bits 32 .. 63
          // .z -> bits 64 .. 95
          // .w -> bits 96 ..127
          uint32_t component = lanes[n]->laneIndex / 32;
          uint32_t bit = lanes[n]->laneIndex % 32;
          ballot[component] |= 1 << bit;
        }
        i_allMax = std::max(lanes[n]->i_diver, i_allMax);
        i_allMin = std::min(lanes[n]->i_diver, i_allMin);
        i_allProd *= lanes[n]->i_diver;
        i_allSum += lanes[n]->i_diver;
      }

      for (size_t n = 1; n < wave->size(); ++n) {
        // 'All' operations are uniform across the wave.
        std::vector<PerThreadData *> &lanes = *wave.get();
        VERIFY_IS_TRUE(
            0 == (lanes[n]->flags & 1)); // non-firstlanes do not set this bit
        VERIFY_ARE_EQUAL(lanes[0]->allBC, lanes[n]->allBC);
        VERIFY_ARE_EQUAL(lanes[0]->allSum, lanes[n]->allSum);
        VERIFY_ARE_EQUAL(lanes[0]->allProd, lanes[n]->allProd);
        VERIFY_ARE_EQUAL(lanes[0]->allAND, lanes[n]->allAND);
        VERIFY_ARE_EQUAL(lanes[0]->allOR, lanes[n]->allOR);
        VERIFY_ARE_EQUAL(lanes[0]->allXOR, lanes[n]->allXOR);
        VERIFY_ARE_EQUAL(lanes[0]->allMin, lanes[n]->allMin);
        VERIFY_ARE_EQUAL(lanes[0]->allMax, lanes[n]->allMax);
        VERIFY_ARE_EQUAL(i_allMax, lanes[n]->i_allMax);
        VERIFY_ARE_EQUAL(i_allMin, lanes[n]->i_allMin);
        VERIFY_ARE_EQUAL(i_allProd, lanes[n]->i_allProd);
        VERIFY_ARE_EQUAL(i_allSum, lanes[n]->i_allSum);

        // first-lane reads and uniform reads are uniform across the wave.
        VERIFY_ARE_EQUAL(lanes[0]->firstlaneX, lanes[n]->firstlaneX);
        VERIFY_ARE_EQUAL(lanes[0]->lane1X, lanes[n]->lane1X);

        // the lane count is uniform across the wave.
        VERIFY_ARE_EQUAL(lanes[0]->laneCount, lanes[n]->laneCount);

        // The predicates are uniform across the wave.
        VERIFY_ARE_EQUAL(lanes[n]->preds, preds);

        // the lane index is distinct per thread.
        for (size_t prior = 0; prior < n; ++prior) {
          VERIFY_ARE_NOT_EQUAL(lanes[prior]->laneIndex, lanes[n]->laneIndex);
        }
        // Ballot results are uniform across the wave.
        VERIFY_ARE_EQUAL(0, memcmp(ballot, lanes[n]->ballot, sizeof(ballot)));

        // Keep running total of prefix calculation. Prefix values are exclusive
        // to the executing lane.
        VERIFY_ARE_EQUAL(pfBC, lanes[n]->pfBC);
        VERIFY_ARE_EQUAL(pfSum, lanes[n]->pfSum);
        VERIFY_ARE_EQUAL(pfProd, lanes[n]->pfProd);
        VERIFY_ARE_EQUAL(i_pfSum, lanes[n]->i_pfSum);
        VERIFY_ARE_EQUAL(i_pfProd, lanes[n]->i_pfProd);
        pfBC += (lanes[n]->diver > 3) ? 1 : 0;
        pfSum += lanes[n]->diver;
        pfProd *= lanes[n]->diver;
        i_pfSum += lanes[n]->i_diver;
        i_pfProd *= lanes[n]->i_diver;
      }
      // TODO: add divergent branching and verify that the otherwise uniform
      // values properly diverge
    }

    // Compare each value of each per-thread element.
    for (size_t i = 0; i < values.size(); ++i) {
      PerThreadData &pts = values[i];
      VERIFY_ARE_EQUAL(i, pts.id); // ID is unchanged.
    }
  }
#endif
}

// This test is assuming that the adapter implements WaveReadLaneFirst correctly
TEST_F(ExecutionTest, WaveIntrinsicsInPSTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  struct Vertex {
    XMFLOAT3 position;
  };

  struct PerPixelData {
    XMFLOAT4 position;
    uint32_t id, flags, laneIndex, laneCount, firstLaneId, sum1;
    uint32_t id0, id1, id2, id3;
    uint32_t acrossX, acrossY, acrossDiag, quadActiveCount;
  };

  const UINT RTWidth = 128;
  const UINT RTHeight = 128;

  // Shaders.
  static const char pShaders[] = WAVE_INTRINSIC_DXBC_GUARD
      "struct PSInput {\r\n"
      "  float4 position : SV_POSITION;\r\n"
      "};\r\n\r\n"
      "PSInput VSMain(float4 position : POSITION) {\r\n"
      "  PSInput result;\r\n"
      "\r\n"
      "  result.position = position;\r\n"
      "  return result;\r\n"
      "}\r\n\r\n"
      "uint pos_to_id(float4 pos) { return pos.x * 128 + pos.y; }\r\n"
      "struct PerPixelData {\r\n"
      " float4 position;\r\n"
      " uint id, flags, laneIndex, laneCount, firstLaneId, sum1;\r\n"
      " uint id0, id1, id2, id3;\r\n"
      " uint acrossX, acrossY, acrossDiag, quadActiveCount;\r\n"
      "};\r\n"
      "AppendStructuredBuffer<PerPixelData> g_sb : register(u1);\r\n"
      "float4 PSMain(PSInput input) : SV_TARGET {\r\n"
      "  uint one = 1;\r\n"
      "  PerPixelData d;\r\n"
      "  d.position = input.position;\r\n"
      "  d.id = pos_to_id(input.position);\r\n"
      "  d.flags = 0;\r\n"
      "  if (WaveIsFirstLane()) d.flags |= 1;\r\n"
      "  d.laneIndex = WaveGetLaneIndex();\r\n"
      "  d.laneCount = WaveGetLaneCount();\r\n"
      "  d.firstLaneId = WaveReadLaneFirst(d.id);\r\n"
      "  d.sum1 = WaveActiveSum(one);\r\n"
      "  d.id0 = QuadReadLaneAt(d.id, 0);\r\n"
      "  d.id1 = QuadReadLaneAt(d.id, 1);\r\n"
      "  d.id2 = QuadReadLaneAt(d.id, 2);\r\n"
      "  d.id3 = QuadReadLaneAt(d.id, 3);\r\n"
      "  d.acrossX = QuadReadAcrossX(d.id);\r\n"
      "  d.acrossY = QuadReadAcrossY(d.id);\r\n"
      "  d.acrossDiag = QuadReadAcrossDiagonal(d.id);\r\n"
      "  d.quadActiveCount = one + QuadReadAcrossX(one) + QuadReadAcrossY(one) "
      "+ QuadReadAcrossDiagonal(one);\r\n"
      "  g_sb.Append(d);\r\n"
      "  return 1;\r\n"
      "};\r\n";

  CComPtr<ID3D12Device> pDevice;
  CComPtr<ID3D12CommandQueue> pCommandQueue;
  CComPtr<ID3D12DescriptorHeap> pUavHeap, pRtvHeap;
  CComPtr<ID3D12CommandAllocator> pCommandAllocator;
  CComPtr<ID3D12GraphicsCommandList> pCommandList;
  CComPtr<ID3D12PipelineState> pPSO;
  CComPtr<ID3D12Resource> pRenderTarget, pReadBuffer;
  UINT rtvDescriptorSize;
  CComPtr<ID3D12Resource> pVertexBuffer;
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

  if (!CreateDevice(&pDevice))
    return;
  if (!DoesDeviceSupportWaveOps(pDevice)) {
    // Optional feature, so it's correct to not support it if declared as such.
    WEX::Logging::Log::Comment(L"Device does not support wave operations.");
    return;
  }

  FenceObj FO;
  InitFenceObj(pDevice, &FO);

  // Describe and create a UAV descriptor heap.
  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors = 1;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  VERIFY_SUCCEEDED(
      pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pUavHeap)));

  CreateRtvDescriptorHeap(pDevice, 1, &pRtvHeap, &rtvDescriptorSize);
  CreateRenderTargetAndReadback(pDevice, pRtvHeap, RTHeight, RTWidth,
                                &pRenderTarget, &pReadBuffer);

  // Create root signature: one UAV.
  CComPtr<ID3D12RootSignature> pRootSignature;
  {
    CD3DX12_DESCRIPTOR_RANGE ranges[1];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0, 0);

    CD3DX12_ROOT_PARAMETER rootParameters[1];
    rootParameters[0].InitAsDescriptorTable(1, &ranges[0],
                                            D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(
        _countof(rootParameters), rootParameters, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CreateRootSignatureFromDesc(pDevice, &rootSignatureDesc, &pRootSignature);
  }

  D3D12_INPUT_ELEMENT_DESC elementDesc[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
  D3D12_INPUT_LAYOUT_DESC InputLayout = {elementDesc, _countof(elementDesc)};
  CreateGraphicsPSO(pDevice, &InputLayout, pRootSignature, pShaders, &pPSO);

  CreateGraphicsCommandQueueAndList(pDevice, &pCommandQueue, &pCommandAllocator,
                                    &pCommandList, pPSO);

  // Single triangle covering half the target.
  Vertex vertices[] = {
      {{-1.0f, 1.0f, 0.0f}}, {{1.0f, 1.0f, 0.0f}}, {{-1.0f, -1.0f, 0.0f}}};
  const UINT TriangleCount = _countof(vertices) / 3;

  CreateVertexBuffer(pDevice, vertices, &pVertexBuffer, &vertexBufferView);

  bool dxbc = UseDxbc();

  // Set up UAV resource.
  std::vector<PerPixelData> values;
  values.resize(RTWidth * RTHeight * 2);
  UINT valueSizeInBytes = (UINT)values.size() * sizeof(PerPixelData);
  memset(values.data(), 0, valueSizeInBytes);
  CComPtr<ID3D12Resource> pUavResource;
  CComPtr<ID3D12Resource> pUavReadBuffer;
  CComPtr<ID3D12Resource> pUploadResource;
  CreateTestUavs(pDevice, pCommandList, values.data(), valueSizeInBytes,
                 &pUavResource, &pUploadResource, &pUavReadBuffer);

  // Set up the append counter resource.
  CComPtr<ID3D12Resource> pUavCounterResource;
  CComPtr<ID3D12Resource> pReadCounterBuffer;
  CComPtr<ID3D12Resource> pUploadCounterResource;
  BYTE zero[sizeof(UINT)] = {0};
  CreateTestUavs(pDevice, pCommandList, zero, sizeof(zero),
                 &pUavCounterResource, &pUploadCounterResource,
                 &pReadCounterBuffer);

  // Close the command list and execute it to perform the GPU setup.
  pCommandList->Close();
  ExecuteCommandList(pCommandQueue, pCommandList);
  WaitForSignal(pCommandQueue, FO);
  VERIFY_SUCCEEDED(pCommandAllocator->Reset());
  VERIFY_SUCCEEDED(pCommandList->Reset(pCommandAllocator, pPSO));

  pCommandList->SetGraphicsRootSignature(pRootSignature);
  SetDescriptorHeap(pCommandList, pUavHeap);
  {
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = (UINT)values.size();
    uavDesc.Buffer.StructureByteStride = sizeof(PerPixelData);
    uavDesc.Buffer.CounterOffsetInBytes = 0;
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
    CD3DX12_CPU_DESCRIPTOR_HANDLE uavHandle(
        pUavHeap->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandleGpu(
        pUavHeap->GetGPUDescriptorHandleForHeapStart());
    pDevice->CreateUnorderedAccessView(pUavResource, pUavCounterResource,
                                       &uavDesc, uavHandle);
    pCommandList->SetGraphicsRootDescriptorTable(0, uavHandleGpu);
  }
  RecordRenderAndReadback(pCommandList, pRtvHeap, rtvDescriptorSize,
                          TriangleCount, &vertexBufferView, nullptr,
                          pRenderTarget, pReadBuffer);
  RecordTransitionBarrier(pCommandList, pUavResource,
                          D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                          D3D12_RESOURCE_STATE_COPY_SOURCE);
  RecordTransitionBarrier(pCommandList, pUavCounterResource,
                          D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                          D3D12_RESOURCE_STATE_COPY_SOURCE);
  pCommandList->CopyResource(pUavReadBuffer, pUavResource);
  pCommandList->CopyResource(pReadCounterBuffer, pUavCounterResource);
  VERIFY_SUCCEEDED(pCommandList->Close());
  LogCommentFmt(L"Rendering to %u by %u", RTWidth, RTHeight);
  ExecuteCommandList(pCommandQueue, pCommandList);
  WaitForSignal(pCommandQueue, FO);
  {
    MappedData data(pReadBuffer, RTWidth * RTHeight * 4);
    const uint32_t *pPixels = (uint32_t *)data.data();
    if (SaveImages()) {
      SavePixelsToFile(pPixels, DXGI_FORMAT_R8G8B8A8_UNORM, RTWidth, RTHeight,
                       L"psintrin.bmp");
    }
  }

  uint32_t appendCount;
  {
    MappedData mappedData(pReadCounterBuffer, sizeof(uint32_t));
    appendCount = *((uint32_t *)mappedData.data());
    LogCommentFmt(L"%u elements in append buffer", appendCount);
  }

  {
    MappedData mappedData(pUavReadBuffer, (UINT32)values.size());
    PerPixelData *pData = (PerPixelData *)mappedData.data();
    memcpy(values.data(), pData, valueSizeInBytes);

    // DXBC is handy to test pipeline setup, but interesting functions are
    // stubbed out, so there is no point in further validation.
    if (dxbc)
      return;

    uint32_t maxActiveLaneCount = 0;
    uint32_t maxLaneCount = 0;
    for (uint32_t i = 0; i < appendCount; ++i) {
      maxActiveLaneCount = std::max(maxActiveLaneCount, values[i].sum1);
      maxLaneCount = std::max(maxLaneCount, values[i].laneCount);
    }

    uint32_t peerOfHelperLanes = 0;
    for (uint32_t i = 0; i < appendCount; ++i) {
      if (values[i].sum1 != maxActiveLaneCount) {
        ++peerOfHelperLanes;
      }
    }

    LogCommentFmt(
        L"Found: %u threads. Waves reported up to %u total lanes, up "
        L"to %u active lanes, and %u threads had helper/inactive lanes.",
        appendCount, maxLaneCount, maxActiveLaneCount, peerOfHelperLanes);

    // Group threads into quad invocations.
    uint32_t singlePixelCount = 0;
    uint32_t multiPixelCount = 0;
    std::unordered_set<uint32_t> ids;
    std::multimap<uint32_t, PerPixelData *> idGroups;
    std::multimap<uint32_t, PerPixelData *> firstIdGroups;
    for (uint32_t i = 0; i < appendCount; ++i) {
      ids.insert(values[i].id);
      idGroups.insert(std::make_pair(values[i].id, &values[i]));
      firstIdGroups.insert(std::make_pair(values[i].firstLaneId, &values[i]));
    }
    for (uint32_t id : ids) {
      if (idGroups.count(id) == 1)
        ++singlePixelCount;
      else
        ++multiPixelCount;
    }
    LogCommentFmt(L"%u pixels were processed by a single thread. %u "
                  L"invocations were for shared pixels.",
                  singlePixelCount, multiPixelCount);

    // Multiple threads may have tried to shade the same pixel. (Is this true
    // even if we have only one triangle?) Where every pixel is distinct, it's
    // very straightforward to validate.
    {
      auto cur = firstIdGroups.begin(), end = firstIdGroups.end();
      while (cur != end) {
        bool simpleWave = true;
        uint32_t firstId = (*cur).first;
        auto groupEnd = cur;
        while (groupEnd != end && (*groupEnd).first == firstId) {
          if (idGroups.count((*groupEnd).second->id) > 1)
            simpleWave = false;
          ++groupEnd;
        }
        if (simpleWave) {
          // Break the wave into quads.
          struct QuadData {
            unsigned count;
            PerPixelData *data[4];
          };
          std::map<uint32_t, QuadData> quads;
          for (auto i = cur; i != groupEnd; ++i) {
            // assuming that it is a simple wave, idGroups has a unique id for
            // each entry.
            uint32_t laneId = (*i).second->id;
            uint32_t laneIds[4] = {(*i).second->id0, (*i).second->id1,
                                   (*i).second->id2, (*i).second->id3};
            // Since this is a simple wave, each lane has an unique id and
            // therefore should not have any ids in there.
            VERIFY_IS_TRUE(quads.find(laneId) == quads.end());
            // check if QuadReadLaneAt is returning same values in a single
            // quad.
            bool newQuad = true;
            for (unsigned quadIndex = 0; quadIndex < 4; ++quadIndex) {
              auto match = quads.find(laneIds[quadIndex]);
              if (match != quads.end()) {
                (*match).second.data[(*match).second.count++] = (*i).second;
                newQuad = false;
                break;
              }
              auto quadMemberData = idGroups.find(laneIds[quadIndex]);
              if (quadMemberData != idGroups.end()) {
                VERIFY_IS_TRUE((*quadMemberData).second->id0 == laneIds[0]);
                VERIFY_IS_TRUE((*quadMemberData).second->id1 == laneIds[1]);
                VERIFY_IS_TRUE((*quadMemberData).second->id2 == laneIds[2]);
                VERIFY_IS_TRUE((*quadMemberData).second->id3 == laneIds[3]);
              }
            }
            if (newQuad) {
              QuadData qdata;
              qdata.count = 1;
              qdata.data[0] = (*i).second;
              quads.insert(std::make_pair(laneId, qdata));
            }
          }
          for (auto quadPair : quads) {
            unsigned count = quadPair.second.count;
            // There could be only one pixel data on the edge of the triangle
            if (count < 2)
              continue;
            PerPixelData **data = quadPair.second.data;
            bool isTop[4];
            bool isLeft[4];
            PerPixelData helperData;
            memset(&helperData, 0, sizeof(helperData));
            PerPixelData *layout[4]; // tl,tr,bl,br
            memset(layout, 0, sizeof(layout));
            auto fnToLayout = [&](bool top, bool left) -> PerPixelData ** {
              int idx = top ? 0 : 2;
              idx += left ? 0 : 1;
              return &layout[idx];
            };
            auto fnToLayoutData = [&](bool top, bool left) -> PerPixelData * {
              PerPixelData **pResult = fnToLayout(top, left);
              if (*pResult == nullptr)
                return &helperData;
              return *pResult;
            };
            VERIFY_IS_TRUE(count <= 4);
            if (count == 2) {
              isTop[0] = data[0]->position.y < data[1]->position.y;
              isTop[1] = (data[0]->position.y == data[1]->position.y)
                             ? isTop[0]
                             : !isTop[0];
              isLeft[0] = data[0]->position.x < data[1]->position.x;
              isLeft[1] = (data[0]->position.x == data[1]->position.x)
                              ? isLeft[0]
                              : !isLeft[0];
            } else {
              // with at least three samples, we have distinct x and y
              // coordinates.
              float left = std::min(data[0]->position.x, data[1]->position.x);
              left = std::min(data[2]->position.x, left);
              float top = std::min(data[0]->position.y, data[1]->position.y);
              top = std::min(data[2]->position.y, top);
              for (unsigned i = 0; i < count; ++i) {
                isTop[i] = data[i]->position.y == top;
                isLeft[i] = data[i]->position.x == left;
              }
            }
            for (unsigned i = 0; i < count; ++i) {
              *(fnToLayout(isTop[i], isLeft[i])) = data[i];
            }

            // Finally, we have a proper quad reconstructed. Validate.
            for (unsigned i = 0; i < count; ++i) {
              PerPixelData *d = data[i];
              VERIFY_ARE_EQUAL(d->id0, fnToLayoutData(true, true)->id);
              VERIFY_ARE_EQUAL(d->id1, fnToLayoutData(true, false)->id);
              VERIFY_ARE_EQUAL(d->id2, fnToLayoutData(false, true)->id);
              VERIFY_ARE_EQUAL(d->id3, fnToLayoutData(false, false)->id);
              VERIFY_ARE_EQUAL(d->acrossX,
                               fnToLayoutData(isTop[i], !isLeft[i])->id);
              VERIFY_ARE_EQUAL(d->acrossY,
                               fnToLayoutData(!isTop[i], isLeft[i])->id);
              VERIFY_ARE_EQUAL(d->acrossDiag,
                               fnToLayoutData(!isTop[i], !isLeft[i])->id);
              VERIFY_ARE_EQUAL(d->quadActiveCount, count);
            }
          }
        }
        cur = groupEnd;
      }
    }

    // TODO: provide validation for quads where the same pixel was shaded
    // multiple times
    //
    // Consider: for pixels that were shaded multiple times, check whether
    // some grouping of threads into quads satisfies all value requirements.
  }
}

struct ShaderOpTestResult {
  st::ShaderOp *ShaderOp;
  std::shared_ptr<st::ShaderOpSet> ShaderOpSet;
  std::shared_ptr<st::ShaderOpTest> Test;
};

struct SPrimitives {
  float f_float;
  float f_float2;
  float f_float_o;
  float f_float2_o;
};

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
  pShaderOp->UseWarpDevice = GetTestParamUseWARP(true);

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

TEST_F(ExecutionTest, OutOfBoundsTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  // Single operation test at the moment.
  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice))
    return;

  std::shared_ptr<ShaderOpTestResult> test =
      RunShaderOpTest(pDevice, m_support, pStream, "OOB", nullptr);
  MappedData data;
  // Read back to CPU and examine contents - should get pure red.
  {
    MappedData data;
    test->Test->GetReadBackData("RTarget", &data);
    const uint32_t *pPixels = (uint32_t *)data.data();
    uint32_t first = *pPixels;
    VERIFY_ARE_EQUAL(0xff0000ff,
                     first); // pure red - only first component is read
  }
}

TEST_F(ExecutionTest, SaturateTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  // Single operation test at the moment.
  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice))
    return;

  std::shared_ptr<ShaderOpTestResult> test =
      RunShaderOpTest(pDevice, m_support, pStream, "Saturate", nullptr);
  MappedData data;
  test->Test->GetReadBackData("U0", &data);
  const float *pValues = (float *)data.data();
  // Everything is zero except for 1.5f and +Inf, which saturate to 1.0f
  const float ExpectedCases[9] = {
      0.0f, 0.0f, 0.0f, 0.0f, // -inf, -1.5, -denorm, -0
      0.0f, 0.0f, 1.0f, 1.0f, // 0, denorm, 1.5f, inf
      0.0f                    // nan
  };
  for (size_t i = 0; i < _countof(ExpectedCases); ++i) {
    VERIFY_IS_TRUE(ifdenorm_flushf_eq(*pValues, ExpectedCases[i]));
    ++pValues;
  }
}

void ExecutionTest::BasicTriangleTestSetup(LPCSTR ShaderOpName,
                                           LPCWSTR FileName,
                                           D3D_SHADER_MODEL testModel) {
#ifdef _HLK_CONF
  UNREFERENCED_PARAMETER(ShaderOpName);
  UNREFERENCED_PARAMETER(FileName);
  UNREFERENCED_PARAMETER(testModel);
#else
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  // Single operation test at the moment.
  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, testModel))
    return;

  // As this is used, 6.2 requirement always comes with requiring native 16-bit
  // ops
  if (testModel == D3D_SHADER_MODEL_6_2 &&
      !DoesDeviceSupportNative16bitOps(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support native 16-bit operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  std::shared_ptr<ShaderOpTestResult> test =
      RunShaderOpTest(pDevice, m_support, pStream, ShaderOpName, nullptr);
  MappedData data;
  D3D12_RESOURCE_DESC &D = test->ShaderOp->GetResourceByName("RTarget")->Desc;
  UINT width = (UINT)D.Width;
  UINT height = D.Height;
  test->Test->GetReadBackData("RTarget", &data);
  const uint32_t *pPixels = (uint32_t *)data.data();
  if (SaveImages()) {
    SavePixelsToFile(pPixels, DXGI_FORMAT_R8G8B8A8_UNORM, 320, 200, FileName);
  }
  uint32_t top = pPixels[width / 2];                        // Top center.
  uint32_t mid = pPixels[width / 2 + width * (height / 2)]; // Middle center.
  VERIFY_ARE_EQUAL(0xff663300, top);                        // clear color
  VERIFY_ARE_EQUAL(0xffffffff, mid);                        // white

  // This is the basic validation test for shader operations, so it's good to
  // check this here at least for this one test case.
  data.reset();
  test.reset();
  ReportLiveObjects();
#endif
}

TEST_F(ExecutionTest, BasicTriangleOpTest) {
  BasicTriangleTestSetup("Triangle", L"basic-triangle.bmp",
                         D3D_SHADER_MODEL_6_0);
}

TEST_F(ExecutionTest, BasicTriangleOpTestHalf) {
  BasicTriangleTestSetup("TriangleHalf", L"basic-triangle-half.bmp",
                         D3D_SHADER_MODEL_6_2);
}

void VerifyDerivResults_PS_60(const float *pPixels, UINT offsetCenter) {

  // pixel at the center
  float CenterDDXFine = pPixels[offsetCenter];
  float CenterDDYFine = pPixels[offsetCenter + 1];
  float CenterDDXCoarse = pPixels[offsetCenter + 2];
  float CenterDDYCoarse = pPixels[offsetCenter + 3];

  LogCommentFmt(
      L"center  ddx_fine: %8f, ddy_fine: %8f, ddx_coarse: %8f, ddy_coarse: %8f",
      CenterDDXFine, CenterDDYFine, CenterDDXCoarse, CenterDDYCoarse);

  // The texture for the 9 pixels in the center should look like the following

  // 256   32  64
  // 2048 256 512
  // 1   .125 .25

  // In D3D12 there is no guarantee of how the adapter is grouping 2x2 pixels
  // for pixel shaders and shader model 6.0.
  // So for fine derivatives there can be up to two possible results for the
  // center pixel, while for coarse derivatives there can be up to six possible
  // results.
  int ulpTolerance = 1;
  // 512 - 256 or 2048 - 256
  bool left = CompareFloatULP(CenterDDXFine, -1792.0f, ulpTolerance);
  VERIFY_IS_TRUE(left || CompareFloatULP(CenterDDXFine, 256.0f, ulpTolerance));
  // 256 - 32 or 256 - .125
  bool top = CompareFloatULP(CenterDDYFine, 224.0f, ulpTolerance);
  VERIFY_IS_TRUE(top || CompareFloatULP(CenterDDYFine, -255.875, ulpTolerance));

  if (top && left) {
    VERIFY_IS_TRUE((CompareFloatULP(CenterDDXCoarse, -224.0f, ulpTolerance) ||
                    CompareFloatULP(CenterDDXCoarse, -1792.0f, ulpTolerance)) &&
                   (CompareFloatULP(CenterDDYCoarse, 224.0f, ulpTolerance) ||
                    CompareFloatULP(CenterDDYCoarse, 1792.0f, ulpTolerance)));
  } else if (top) { // top right quad
    VERIFY_IS_TRUE((CompareFloatULP(CenterDDXCoarse, 256.0f, ulpTolerance) ||
                    CompareFloatULP(CenterDDXCoarse, 32.0f, ulpTolerance)) &&
                   (CompareFloatULP(CenterDDYCoarse, 224.0f, ulpTolerance) ||
                    CompareFloatULP(CenterDDYCoarse, 448.0f, ulpTolerance)));
  } else if (left) { // bottom left quad
    VERIFY_IS_TRUE((CompareFloatULP(CenterDDXCoarse, -1792.0f, ulpTolerance) ||
                    CompareFloatULP(CenterDDXCoarse, -.875f, ulpTolerance)) &&
                   (CompareFloatULP(CenterDDYCoarse, -2047.0f, ulpTolerance) ||
                    CompareFloatULP(CenterDDYCoarse, -255.875f, ulpTolerance)));
  } else { // bottom right
    VERIFY_IS_TRUE((CompareFloatULP(CenterDDXCoarse, 256.0f, ulpTolerance) ||
                    CompareFloatULP(CenterDDXCoarse, .125f, ulpTolerance)) &&
                   (CompareFloatULP(CenterDDYCoarse, -255.875f, ulpTolerance) ||
                    CompareFloatULP(CenterDDYCoarse, -511.75f, ulpTolerance)));
  }
}

void VerifyDerivResults_CS_AS_MS_66(const float *pPixels, UINT offsetCenter) {

  // pixel at the center
  float CenterDDXFine = pPixels[offsetCenter];
  float CenterDDYFine = pPixels[offsetCenter + 1];
  float CenterDDXCoarse = pPixels[offsetCenter + 2];
  float CenterDDYCoarse = pPixels[offsetCenter + 3];

  LogCommentFmt(
      L"center  ddx_fine: %8f, ddy_fine: %8f, ddx_coarse: %8f, ddy_coarse: %8f",
      CenterDDXFine, CenterDDYFine, CenterDDXCoarse, CenterDDYCoarse);

  // The 4x4 texture used to calculate the derivatives looks like this:
  // .125   .25    .5    1
  //    2     4    16   32
  //   32    64  *128* 256
  //  256   512  1024 2048
  //
  // We are checking the derivate values calculated at the texture
  // center pixel (2,2).

  // In D3D12 for shader model 6.6 compute, mesh and amplification shaders
  // the quad grouping is well defined. There is one possible result for
  // fine derivatives and 2 possible results for coarse derivatives.
  int ulpTolerance = 1;

  // 256 - 128
  VERIFY_IS_TRUE(CompareFloatULP(CenterDDXFine, 128.0f, ulpTolerance));
  // 1024 - 128
  VERIFY_IS_TRUE(CompareFloatULP(CenterDDYFine, 896.0f, ulpTolerance));

  // 256 - 128 or 2048 - 1024
  VERIFY_IS_TRUE(CompareFloatULP(CenterDDXCoarse, 128.0f, ulpTolerance) ||
                 CompareFloatULP(CenterDDXCoarse, 1024.0f, ulpTolerance));
  // 1024 - 128 or 2048 - 256
  VERIFY_IS_TRUE(CompareFloatULP(CenterDDYCoarse, 896.0f, ulpTolerance) ||
                 CompareFloatULP(CenterDDYCoarse, 1792.0f, ulpTolerance));
}

// Rendering two right triangles forming a square and assigning a texture value
// for each pixel to calculate derivates.
TEST_F(ExecutionTest, PartialDerivTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice))
    return;

  std::shared_ptr<ShaderOpTestResult> test =
      RunShaderOpTest(pDevice, m_support, pStream, "DerivFine", nullptr);
  MappedData data;
  D3D12_RESOURCE_DESC &D = test->ShaderOp->GetResourceByName("RTarget")->Desc;
  UINT width = (UINT)D.Width;
  UINT height = D.Height;
  UINT pixelSize = GetByteSizeForFormat(D.Format) / 4;

  test->Test->GetReadBackData("RTarget", &data);
  const float *pPixels = (float *)data.data();

  UINT centerIndex = (UINT64)width * height / 2 - width / 2;
  UINT offsetCenter = centerIndex * pixelSize;

  VerifyDerivResults_PS_60(pPixels, offsetCenter);
}

struct Dispatch {
  int width, height, depth;
};

std::shared_ptr<st::ShaderOpTest> RunDispatch(ID3D12Device *pDevice,
                                              dxc::DxcDllSupport &support,
                                              st::ShaderOp *pShaderOp,
                                              const Dispatch D) {
  char compilerOptions[256];

  std::shared_ptr<st::ShaderOpTest> test = std::make_shared<st::ShaderOpTest>();
  test->SetDxcSupport(&support);
  test->SetInitCallback(nullptr);
  test->SetDevice(pDevice);

  // format compiler args
  VERIFY_IS_TRUE(sprintf_s(compilerOptions, sizeof(compilerOptions),
                           "-D DISPATCHX=%d -D DISPATCHY=%d -D DISPATCHZ=%d ",
                           D.width, D.height, D.depth));

  for (st::ShaderOpShader &S : pShaderOp->Shaders)
    S.Arguments = compilerOptions;

  pShaderOp->DispatchX = D.width;
  pShaderOp->DispatchY = D.height;
  pShaderOp->DispatchZ = D.depth;

  test->RunShaderOp(pShaderOp);

  return test;
}

UINT DerivativesTest_GetCenterIndex(Dispatch &D) {
  if (D.height == 1) {
    // 1D Quads - Find center, truncate to the previous multiple of 16 to get
    // to the start of the repeating pattern, and then add 12 to get to the
    // middle (2,2) pixel of the pattern. The values are stored in Z-order.
    return (((UINT64)D.width / 2) & ~0xF) + 12;
  } else {
    // To find roughly the center, divide the height and width in
    // half, truncate to the previous multiple of 4 to get to the start of the
    // repeating pattern and then add 2 rows to get to the second row of quads
    // and 2 to get to the first texel of the second row of that quad row
    UINT centerRow = ((D.height / 2UL) & ~0x3) + 2;
    UINT centerCol = ((D.width / 2UL) & ~0x3) + 2;
    return centerRow * D.width + centerCol;
  }
}

void DerivativesTest_DebugOutput(Dispatch &D,
                                 std::shared_ptr<st::ShaderOpTest> &Test,
                                 const float *pPixels, UINT centerIndex) {
#ifdef DERIVATIVES_TEST_DEBUG
  LogCommentFmt(L"------------------------------------");
  MappedData dataDbg;
  Test->GetReadBackData("U3", &dataDbg);
  UINT *pCoords = (UINT *)dataDbg.data();

  LogCommentFmt(L"DISPATCH %d x %d x %d", D.width, D.height, D.depth);
  for (int j = 0; j < D.height; j++) {
    for (int i = 0; i < D.width; i++) {
      UINT index = (j * 4) * D.width + i * 4;
      LogCommentFmt(L"%3d (%2d, %2d, %2d)\t ddx_fine: %8f, ddy_fine: %8f, "
                    L"ddx_coarse: %8f, ddy_coarse: %8f",
                    pCoords[index], pCoords[index + 1], pCoords[index + 2],
                    pCoords[index + 3], pPixels[index], pPixels[index + 1],
                    pPixels[index + 2], pPixels[index + 3]);
    }
  }
  LogCommentFmt(L"CENTER %d", centerIndex);
  LogCommentFmt(L"------------------------------------");
#else
  UNREFERENCED_PARAMETER(D);
  UNREFERENCED_PARAMETER(Test);
  UNREFERENCED_PARAMETER(pPixels);
  UNREFERENCED_PARAMETER(centerIndex);
#endif
}

TEST_F(ExecutionTest, DerivativesTest) {
  const UINT pixelSize = 4; // always float4

  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL_6_6))
    return;

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());

  st::ShaderOp *pShaderOp = ShaderOpSet->GetShaderOp("Derivatives");

  std::vector<Dispatch> dispatches = {{40, 1, 1},  {1000, 1, 1}, {32, 32, 1},
                                      {16, 64, 1}, {4, 12, 4},   {4, 64, 1},
                                      {16, 16, 3}, {32, 8, 2},   {8, 8, 1}};

  std::vector<Dispatch> meshDispatches = {// (X * Y * Z) must be <= 128
                                          {60, 1, 1}, {128, 1, 1}, {8, 8, 1},
                                          {16, 8, 1}, {8, 4, 2},   {10, 10, 1},
                                          {4, 16, 2}, {4, 16, 2}};

  pShaderOp->UseWarpDevice = GetTestParamUseWARP(true);

  MappedData data;

  for (Dispatch &D : dispatches) {
    // Test Compute Shader
    std::shared_ptr<st::ShaderOpTest> test =
        RunDispatch(pDevice, m_support, pShaderOp, D);

    test->GetReadBackData("U0", &data);
    float *pPixels = (float *)data.data();

    UINT centerIndex = DerivativesTest_GetCenterIndex(D);

    DerivativesTest_DebugOutput(D, test, pPixels, centerIndex);

    UINT offsetCenter = centerIndex * pixelSize;
    LogCommentFmt(L"Verifying derivatives in compute shader results");
    VerifyDerivResults_CS_AS_MS_66(pPixels, offsetCenter);
  }

  if (DoesDeviceSupportMeshAmpDerivatives(pDevice)) {
    // Disable CS so mesh goes forward
    pShaderOp->CS = nullptr;

    for (Dispatch &D : meshDispatches) {
      std::shared_ptr<st::ShaderOpTest> test =
          RunDispatch(pDevice, m_support, pShaderOp, D);

      test->GetReadBackData("U1", &data);
      const float *pPixels = (float *)data.data();
      UINT centerIndex = DerivativesTest_GetCenterIndex(D);

      DerivativesTest_DebugOutput(D, test, pPixels, centerIndex);

      UINT offsetCenter = centerIndex * pixelSize;
      LogCommentFmt(L"Verifying derivatives in mesh shader results");
      VerifyDerivResults_CS_AS_MS_66(pPixels, offsetCenter);

      test->GetReadBackData("U2", &data);
      pPixels = (float *)data.data();
      LogCommentFmt(L"Verifying derivatives in amplification shader results");
      VerifyDerivResults_CS_AS_MS_66(pPixels, offsetCenter);
    }
  }
}

// Verify the results for the quad starting with the given index
void VerifyQuadReadResults(const UINT *pPixels, UINT quadIndex) {
  for (UINT i = 0; i < 4; i++) {
    UINT ix = quadIndex + i;
    UINT lix = pPixels[4 * ix];
    VERIFY_ARE_EQUAL(pPixels[4 * ix + 1], (lix ^ 1)); // ReadAcrossX
    VERIFY_ARE_EQUAL(pPixels[4 * ix + 2], (lix ^ 2)); // ReadAcrossY
    VERIFY_ARE_EQUAL(pPixels[4 * ix + 3], (lix ^ 3)); // ReadAcrossDiagonal
  }
}

TEST_F(ExecutionTest, QuadReadTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice))
    return;

  if (!DoesDeviceSupportWaveOps(pDevice)) {
    WEX::Logging::Log::Comment(L"Device does not support wave operations.");
    return;
  }

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());

  st::ShaderOp *pShaderOp = ShaderOpSet->GetShaderOp("QuadRead");
  LPCSTR CS = pShaderOp->CS;

  struct Dispatch {
    int x, y, z;
    int mx, my, mz;
  };
  // std::vector<std::tuple<int, int, int, int, int>> dispatches =
  std::vector<Dispatch> dispatches = {
      {32, 32, 1, 8, 8, 1},
      {64, 4, 1, 64, 2, 1},
      {64, 1, 1, 64, 1, 1},
      {16, 16, 3, 4, 4, 3},
  };

  for (Dispatch &D : dispatches) {

    UINT width = D.x;
    UINT height = D.y;
    UINT depth = D.z;

    UINT mwidth = D.mx;
    UINT mheight = D.my;
    UINT mdepth = D.mz;
    // format compiler args
    char compilerOptions[256];
    VERIFY_IS_TRUE(
        sprintf_s(compilerOptions, sizeof(compilerOptions),
                  "-D DISPATCHX=%d -D DISPATCHY=%d -D DISPATCHZ=%d "
                  "-D MESHDISPATCHX=%d -D MESHDISPATCHY=%d -D MESHDISPATCHZ=%d",
                  width, height, depth, mwidth, mheight, mdepth));

    for (st::ShaderOpShader &S : pShaderOp->Shaders)
      S.Arguments = compilerOptions;

    pShaderOp->DispatchX = width;
    pShaderOp->DispatchY = height;
    pShaderOp->DispatchZ = depth;

    // Test Compute Shader
    pShaderOp->CS = CS;
    std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
        pDevice, m_support, "QuadRead", nullptr, ShaderOpSet);
    MappedData data;

    test->Test->GetReadBackData("U0", &data);
    const UINT *pPixels = (UINT *)data.data();

    // To find roughly the center for compute, divide the pixel count in half
    // and truncate to next lowest power of 4 to start at a quad
    UINT offsetCenter = ((UINT64)(width * height * depth) / 2) & ~0x3;

    // Test first, second and center quads
    LogCommentFmt(L"Verifying QuadRead* in compute shader results");
    VerifyQuadReadResults(pPixels, 0);
    VerifyQuadReadResults(pPixels, 4);
    VerifyQuadReadResults(pPixels, offsetCenter);

    if (DoesDeviceSupportMeshAmpDerivatives(pDevice)) {
      offsetCenter = ((UINT64)(mwidth * mheight * mdepth) / 2) & ~0x3;

      // Disable CS so mesh goes forward
      pShaderOp->CS = nullptr;
      test = RunShaderOpTestAfterParse(pDevice, m_support, "QuadRead", nullptr,
                                       ShaderOpSet);
      test->Test->GetReadBackData("U1", &data);
      pPixels = (UINT *)data.data();
      // Test first, second and center quads
      LogCommentFmt(L"Verifying QuadRead* in mesh shader results");
      VerifyQuadReadResults(pPixels, 0);
      VerifyQuadReadResults(pPixels, 4);
      VerifyQuadReadResults(pPixels, offsetCenter);

      test->Test->GetReadBackData("U2", &data);
      pPixels = (UINT *)data.data();
      // Test first, second and center quads
      LogCommentFmt(L"Verifying QuadRead* in amplification shader results");
      VerifyQuadReadResults(pPixels, 0);
      VerifyQuadReadResults(pPixels, 4);
      VerifyQuadReadResults(pPixels, offsetCenter);
    }
  }
}

void VerifySampleResults(const UINT *pPixels, UINT width) {
  UINT xlod = 0;
  UINT ylod = 0;
  // Each pixel contains 4 samples and 4 LOD calculations.
  // 2 of these (called 'left' and 'right') have X values that vary and a
  // constant Y. 2 others (called 'top' and 'bot') have Y values that vary and a
  // constant X. Only one of the X variant sample results and one of the Y
  // variant results are actually reported for the pixel. The other 2 serve as
  // "helpers" to the other pixels in the quad. On the left side of the quad,
  // the 'left' samples are reported. On the top of the quad, the 'top' samples
  // are reported and so on. The varying coordinate values alternate between
  // zero and a value whose magnitude increases with the index. As a result, the
  // LOD level should steadily increase. Due to vagaries of implementation, the
  // same derivatives in both directions might result in different levels for
  // different locations in the quad. So only comparisons between sample results
  // and LOD calculations and ensuring that the LOD increased and reaches the
  // max can be tested reliably.

  // The results are stored in quad z-order (top-left (#0), top-right (#1),
  // bottom-left (#2), bottom-right (#3)) and need to be evaluated as such.
  // The X-derivative-LOD should not decrease when going from quad pixel #0->#1,
  // #2->#3, or #3->#0. For #1->#2 the end of the typewriter "line" is reached
  // and zags left resulting in a smaller x value. So, it is absolutely valid
  // for the X-derivative-LOD to decrease. Therefore, the test skips
  // verification of X-derivative-LOD on quad pixel #2.

  for (unsigned i = 0; i < width; i++) {
    // CalculateLOD and Sample from texture with mip levels containing LOD index
    // should match
    VERIFY_ARE_EQUAL(pPixels[4 * i + 0], pPixels[4 * i + 1]);
    VERIFY_ARE_EQUAL(pPixels[4 * i + 2], pPixels[4 * i + 3]);
    // Make sure LODs are ever climbing as magnitudes increase
    if (i % 4 != 2) { // skip X-derivative-LOD verification on quad pixel #2
      VERIFY_IS_TRUE(pPixels[4 * i] >= xlod);
    }
    xlod = pPixels[4 * i];
    VERIFY_IS_TRUE(pPixels[4 * i + 2] >= ylod);
    ylod = pPixels[4 * i + 2];
  }
  // Make sure we reached the max lod level for both tracks
  VERIFY_ARE_EQUAL(xlod, 6u);
  VERIFY_ARE_EQUAL(ylod, 6u);
}

TEST_F(ExecutionTest, ComputeSampleTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL_6_6))
    return;

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());

  st::ShaderOp *pShaderOp = ShaderOpSet->GetShaderOp("ComputeSample");

  // Initialize texture with the LOD number in each corresponding mip level
  auto SampleInitFn = [&](LPCSTR Name, std::vector<BYTE> &Data,
                          st::ShaderOp *pShaderOp) {
    UNREFERENCED_PARAMETER(pShaderOp);
    VERIFY_ARE_EQUAL(0, _stricmp(Name, "T0"));
    D3D12_RESOURCE_DESC &texDesc = pShaderOp->GetResourceByName("T0")->Desc;
    UINT texWidth = (UINT)texDesc.Width;
    UINT texHeight = (UINT)texDesc.Height;
    size_t size = sizeof(float) * texWidth * texHeight * 2;
    Data.resize(size);
    float *pPrimitives = (float *)Data.data();
    float lod = 0.0;
    int ix = 0;
    while (texHeight > 0 && texWidth > 0) {
      if (!texHeight)
        texHeight = 1;
      if (!texWidth)
        texWidth = 1;
      for (size_t j = 0; j < texHeight; ++j) {
        for (size_t i = 0; i < texWidth; ++i) {
          pPrimitives[ix++] = lod;
        }
      }
      lod += 1.0;
      texHeight >>= 1;
      texWidth >>= 1;
    }
  };
  LPCSTR CS2 = nullptr, AS2 = nullptr, MS2 = nullptr;
  for (st::ShaderOpShader &S : pShaderOp->Shaders) {
    if (!strcmp(S.Name, "CS2"))
      CS2 = S.Name;
    if (!strcmp(S.Name, "AS2"))
      AS2 = S.Name;
    if (!strcmp(S.Name, "MS2"))
      MS2 = S.Name;
  }

  // Test 1D compute shader
  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
      pDevice, m_support, "ComputeSample", SampleInitFn, ShaderOpSet);
  MappedData data;

  test->Test->GetReadBackData("U0", &data);
  const UINT *pPixels = (UINT *)data.data();

  LogCommentFmt(L"Verifying 1D Compute Shader");
  // CSMain1D has [NumThreads(336, 1, 1)]
  VerifySampleResults(pPixels, 336);

  // Test 2D compute shader
  pShaderOp->CS = CS2;

  test.reset();
  test = RunShaderOpTestAfterParse(pDevice, m_support, "ComputeSample",
                                   SampleInitFn, ShaderOpSet);

  test->Test->GetReadBackData("U0", &data);
  pPixels = (UINT *)data.data();

  LogCommentFmt(L"Verifying 2D Compute Shader");
  // CSMain2D has [NumThreads(84, 4, 3)]
  VerifySampleResults(pPixels, 84 * 4);

  if (DoesDeviceSupportMeshAmpDerivatives(pDevice)) {
    // Disable CS so mesh goes forward
    pShaderOp->CS = nullptr;
    test = RunShaderOpTestAfterParse(pDevice, m_support, "ComputeSample",
                                     SampleInitFn, ShaderOpSet);
    test->Test->GetReadBackData("U1", &data);
    pPixels = (UINT *)data.data();

    LogCommentFmt(L"Verifying 1D mesh shader");
    // MSMain1D has [NumThreads(116, 1, 1)]
    VerifySampleResults(pPixels, 116);

    test->Test->GetReadBackData("U2", &data);
    pPixels = (UINT *)data.data();

    LogCommentFmt(L"Verifying 1D amplification shader");
    // ASMain1D has [NumThreads(116, 1, 1)]
    VerifySampleResults(pPixels, 116);

    pShaderOp->AS = AS2;
    pShaderOp->MS = MS2;
    test = RunShaderOpTestAfterParse(pDevice, m_support, "ComputeSample",
                                     SampleInitFn, ShaderOpSet);
    test->Test->GetReadBackData("U1", &data);
    pPixels = (UINT *)data.data();

    LogCommentFmt(L"Verifying 2D mesh shader");
    // MSMain2D has [NumThreads(42, 2, 1)]
    VerifySampleResults(pPixels, 42 * 2);

    test->Test->GetReadBackData("U2", &data);
    pPixels = (UINT *)data.data();

    LogCommentFmt(L"Verifying 2D amplification shader");
    // ASMain2D has [NumThreads(42, 2, 1)]
    VerifySampleResults(pPixels, 42 * 2);
  }
}

TEST_F(ExecutionTest, ATOWriteMSAATest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  //  #define WRITEMSAA_FALLBACK

  CComPtr<ID3D12Device> pDevice;
#ifdef WRITEMSAA_FALLBACK
  D3D_SHADER_MODEL sm = D3D_SHADER_MODEL_6_6;
#else
  D3D_SHADER_MODEL sm = D3D_SHADER_MODEL_6_7;
#endif
  if (!CreateDevice(&pDevice, sm))
    return;

#ifndef WRITEMSAA_FALLBACK
  if (!DoesDeviceSupportAdvancedTexOps(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support Advanced Texture Operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  if (!DoesDeviceSupportWritableMSAA(pDevice)) {
    WEX::Logging::Log::Comment(L"Device does not support Writable MSAA.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }
#endif

  static const char pWriteShader[] =
      "#define SAMPLES 4\n"
      "RWStructuredBuffer<float> g_out : register(u0);\n"
      "#if  __SHADER_TARGET_MAJOR > 6 || (__SHADER_TARGET_MAJOR == 6 && "
      "__SHADER_TARGET_MINOR >= 7)\n"
      "RWTexture2DMS<float, 4> g_texms : register(u1);\n"
      "#else\n"
      "RWTexture2DArray<float> g_texms : register(u1);\n"
      "#endif\n"
      "[NumThreads(32, 32, 1)]\n"
      "void main(uint3 id : SV_GroupThreadID) {\n"
      "  for(uint i = 0; i < SAMPLES; i++) {\n"
      "#if  __SHADER_TARGET_MAJOR > 6 || (__SHADER_TARGET_MAJOR == 6 && "
      "__SHADER_TARGET_MINOR >= 7)\n"
      "    g_texms.sample[i][id.xy] = id.x*id.y*(i+1);\n"
      "#else\n"
      "    g_texms[uint3(id.xy, i)] = id.x*id.y*(i+1);\n"
      "#endif\n"
      "  }\n"
      "}";

  static const char pCopyShader[] =
      "#define SAMPLES 4\n"
      "RWStructuredBuffer<float> g_out : register(u0);\n"
      "#if  __SHADER_TARGET_MAJOR > 6 || (__SHADER_TARGET_MAJOR == 6 && "
      "__SHADER_TARGET_MINOR >= 7)\n"
      "RWTexture2DMS<float, 4> g_texms : register(u1);\n"
      "#else\n"
      "RWTexture2DArray<float> g_texms : register(u1);\n"
      "#endif\n"
      "[NumThreads(32, 32, 1)]\n"
      "  void main(uint3 id : SV_GroupThreadID) {\n"
      "  for(uint i = 0; i < SAMPLES; i++) {\n"
      "#if  __SHADER_TARGET_MAJOR > 6 || (__SHADER_TARGET_MAJOR == 6 && "
      "__SHADER_TARGET_MINOR >= 7)\n"
      "    g_out[i*32*32 + id.y*32 + id.x] = g_texms.sample[i][id.xy];\n"
      "#else\n"
      "    g_out[i*32*32 + id.y*32 + id.x] = g_texms[uint3(id.xy, i)];\n"
      "#endif\n"
      "  }"
      "}";

  static const int NumThreadsX = 32;
  static const int NumThreadsY = 32;

#ifdef WRITEMSAA_FALLBACK
  static const int NumSamples = 4;
  static const int ArraySize = 4;
#else
  static const int NumSamples = 4;
  static const int ArraySize = 1;
#endif
  static const int ThreadsPerGroup = NumThreadsX * NumThreadsY;
  const size_t valueSize = NumSamples * ThreadsPerGroup;
  const size_t valueSizeInBytes = valueSize * sizeof(float);

  static const int DispatchGroupX = 1;
  static const int DispatchGroupY = 1;
  static const int DispatchGroupZ = 1;

  CComPtr<ID3D12CommandQueue> pCommandQueue;
  CComPtr<ID3D12CommandAllocator> pCommandAllocator;
  FenceObj FO;

  CreateComputeCommandQueue(pDevice, L"WriteMSAA Queue", &pCommandQueue);
  InitFenceObj(pDevice, &FO);

  // Create root signature.
  CComPtr<ID3D12RootSignature> pRootSignature;
  CD3DX12_DESCRIPTOR_RANGE ranges[2];
  ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);
  ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0);

  CreateRootSignatureFromRanges(pDevice, &pRootSignature, ranges, 2);

  VERIFY_SUCCEEDED(pDevice->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&pCommandAllocator)));

  // Create command list and resources
  CComPtr<ID3D12GraphicsCommandList> pCommandList;
  VERIFY_SUCCEEDED(pDevice->CreateCommandList(
      0, D3D12_COMMAND_LIST_TYPE_COMPUTE, pCommandAllocator, nullptr,
      IID_PPV_ARGS(&pCommandList)));

  // Set up Output Resource
  CComPtr<ID3D12Resource> pOutputResource;
  CComPtr<ID3D12Resource> pOutputReadBuffer;
  CComPtr<ID3D12Resource> pOutputUploadResource;

  float outVals[valueSize];
  int ix = 0;
  for (int i = 0; i < NumSamples; i++)
    for (int j = 0; j < NumThreadsY; j++)
      for (int k = 0; k < NumThreadsX; k++)
        outVals[ix++] = (float)ix + 5;
  CreateTestUavs(pDevice, pCommandList, outVals, sizeof(outVals),
                 &pOutputResource, &pOutputUploadResource, &pOutputReadBuffer);

  // Set up texture Resource.
  CComPtr<ID3D12Resource> pUavResource;
  float values[valueSize];
  memset(values, 0xc, valueSizeInBytes);

#ifdef WRITEMSAA_FALLBACK
  int numsamp = 1;
#else
  int numsamp = NumSamples;
#endif

  D3D12_RESOURCE_DESC tex2dDesc = CD3DX12_RESOURCE_DESC::Tex2D(
      DXGI_FORMAT_R32_FLOAT, NumThreadsX, NumThreadsY, ArraySize, 1, numsamp, 0,
      D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS |
          D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
  CreateTestResources(pDevice, pCommandList, values, valueSizeInBytes,
                      tex2dDesc, &pUavResource, nullptr);

  // Close the command list and execute it to perform the resource uploads
  pCommandList->Close();
  ID3D12CommandList *ppCommandLists[] = {pCommandList};
  pCommandQueue->ExecuteCommandLists(1, ppCommandLists);
  WaitForSignal(pCommandQueue, FO);

  // Create shaders
#ifdef WRITEMSAA_FALLBACK
  const wchar_t *target = L"cs_6_6";
#else
  const wchar_t *target = L"cs_6_7";
#endif

  CComPtr<ID3D12PipelineState> pWritePSO;
  CreateComputePSO(pDevice, pRootSignature, pWriteShader, target, &pWritePSO);
  CComPtr<ID3D12PipelineState> pCopyPSO;
  CreateComputePSO(pDevice, pRootSignature, pCopyShader, target, &pCopyPSO);

  // Reset commandlist to write PSO
  VERIFY_SUCCEEDED(pCommandList->Reset(pCommandAllocator, pWritePSO));

  // Describe and create a UAV descriptor heap.
  CComPtr<ID3D12DescriptorHeap> pUavHeap;
  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors = 2;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  VERIFY_SUCCEEDED(
      pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pUavHeap)));

  CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(
      pUavHeap->GetCPUDescriptorHandleForHeapStart());
  CreateStructUAV(pDevice, cpuHandle, valueSize, sizeof(float),
                  pOutputResource);
#ifdef WRITEMSAA_FALLBACK
  CreateTex2DArrayUAV(pDevice, cpuHandle, NumSamples, DXGI_FORMAT_R32_FLOAT,
                      pUavResource);
#else
  CreateTex2DMSUAV(pDevice, cpuHandle, DXGI_FORMAT_R32_FLOAT, pUavResource);
#endif

  // Set Heaps, Rootsignature and table
  ID3D12DescriptorHeap *const pHeaps[1] = {pUavHeap};
  pCommandList->SetDescriptorHeaps(1, pHeaps);
  pCommandList->SetComputeRootSignature(pRootSignature);
  pCommandList->SetComputeRootDescriptorTable(
      0, pUavHeap->GetGPUDescriptorHandleForHeapStart());

  // dispatch and close write shader
  pCommandList->Dispatch(DispatchGroupX, DispatchGroupY, DispatchGroupZ);
  pCommandList->Close();

  pCommandQueue->ExecuteCommandLists(1, ppCommandLists);
  WaitForSignal(pCommandQueue, FO);

  // Create copy command list
  VERIFY_SUCCEEDED(pCommandList->Reset(pCommandAllocator, pCopyPSO));

  // Set Rootsignature and descriptor tables
  SetDescriptorHeap(pCommandList, pUavHeap);
  pCommandList->SetComputeRootSignature(pRootSignature);

  pCommandList->SetComputeRootDescriptorTable(
      0, pUavHeap->GetGPUDescriptorHandleForHeapStart());

  // Run Copy shader and copy the results back to readable memory
  pCommandList->Dispatch(DispatchGroupX, DispatchGroupY, DispatchGroupZ);

  CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      pOutputResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
      D3D12_RESOURCE_STATE_COPY_SOURCE);
  pCommandList->ResourceBarrier(1, &barrier);
  pCommandList->CopyResource(pOutputReadBuffer, pOutputResource);

  pCommandList->Close();

  pCommandQueue->ExecuteCommandLists(1, ppCommandLists);
  WaitForSignal(pCommandQueue, FO);

  MappedData mappedData(pOutputReadBuffer, valueSize * sizeof(float));
  float *pData = (float *)mappedData.data();
  ix = 0;
  for (int i = 0; i < NumSamples; i++)
    for (int j = 0; j < NumThreadsY; j++)
      for (int k = 0; k < NumThreadsX; k++)
        VERIFY_ARE_EQUAL(pData[ix++], j * k * (i + 1));
}

// Used to determine how an out of bounds offset should be converted
constexpr int ClampOffset(int offset) {
  unsigned shift = ((unsigned)offset) << 28;
  return ((int)shift) >> 28;
}

// Determine if the values in pPixels correspond to the expected locations
// encoded into a uint based on the coordinates and offsets that were provided.
void VerifyProgOffsetResults(unsigned *pPixels, bool bCheckDeriv) {
  // Check that each element matches the expected value given the offset
  unsigned ix = 0;
  int coords[18] = {100, 150, 200, 250, 300, 350, 400, 450, 500,
                    550, 600, 650, 700, 750, 800, 850, 900, 950};
  int offsets[18] = {
      ClampOffset(-9), -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7,
      ClampOffset(8)};
  for (unsigned y = 0; y < _countof(coords); y++) {
    for (unsigned x = 0; x < _countof(coords); x++) {
      unsigned cmp = (coords[y] + offsets[y]) * 1000 + coords[x] + offsets[x];
      if (bCheckDeriv) {
        VERIFY_ARE_EQUAL(pPixels[2 * 4 * ix + 0], cmp); // Sample
        VERIFY_ARE_EQUAL(pPixels[2 * 4 * ix + 1], 1U);  // SampleCmp
      }
      VERIFY_ARE_EQUAL(pPixels[2 * 4 * ix + 2], 1U);  // SampleCmpLevel
      VERIFY_ARE_EQUAL(pPixels[2 * 4 * ix + 3], 1U);  // SampleCmpLevelZero
      VERIFY_ARE_EQUAL(pPixels[2 * 4 * ix + 4], cmp); // Load
      if (bCheckDeriv) {
        VERIFY_ARE_EQUAL(pPixels[2 * 4 * ix + 5], cmp); // SampleBias
      }
      VERIFY_ARE_EQUAL(pPixels[2 * 4 * ix + 6], cmp); // SampleGrad
      VERIFY_ARE_EQUAL(pPixels[2 * 4 * ix + 7], cmp); // SampleLevel
      ix++;
    }
  }
}

// Fills a 1000x1000 float texture with index values increasing in row-major
// order The shader then uses non-immediate offsets extending from -9 to 8 to
// access these using Load, Sample, SampleCmp and variants thereof. The test
// verifies that the locations accessed correspond to where they should.
TEST_F(ExecutionTest, ATOProgOffset) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());

  st::ShaderOp *pShaderOp = ShaderOpSet->GetShaderOp("ProgOffset");

  auto SampleInitFn = [&](LPCSTR Name, std::vector<BYTE> &Data,
                          st::ShaderOp *pShaderOp) {
    UNREFERENCED_PARAMETER(pShaderOp);
    D3D12_RESOURCE_DESC &texDesc = pShaderOp->GetResourceByName(Name)->Desc;
    UINT texWidth = (UINT)texDesc.Width;
    UINT texHeight = (UINT)texDesc.Height;
    size_t size = sizeof(float) * texWidth * texHeight;
    Data.resize(size);
    float *pPrimitives = (float *)Data.data();
    int ix = 0;
    for (size_t j = 0; j < texHeight; ++j) {
      for (size_t i = 0; i < texWidth; ++i) {
        pPrimitives[ix] = float(ix);
        ix++;
      }
    }
  };

  bool bTestsSkipped = true;
  D3D_SHADER_MODEL TestShaderModels[] = {
      D3D_SHADER_MODEL_6_5, D3D_SHADER_MODEL_6_6, D3D_SHADER_MODEL_6_7};
  for (unsigned i = 0; i < _countof(TestShaderModels); i++) {
    D3D_SHADER_MODEL sm = TestShaderModels[i];

    CComPtr<ID3D12Device> pDevice;
    if (!CreateDevice(&pDevice, sm, /*skipUnsupported*/ false)) {
      LogCommentFmt(L"Device does not support shader model 6.%1u",
                    ((UINT)sm & 0x0f));
      break;
    }
    if (sm >= D3D_SHADER_MODEL_6_7 &&
        !DoesDeviceSupportAdvancedTexOps(pDevice)) {
      LogCommentFmt(L"Device does not support Advanced Texture Ops");
      break;
    }

    bool bSupportMSASDeriv = DoesDeviceSupportMeshAmpDerivatives(pDevice);

    bool bCheckDerivCS = sm >= D3D_SHADER_MODEL_6_6;
    bool bCheckDerivMSAS = bCheckDerivCS && bSupportMSASDeriv;

    if (bCheckDerivCS && !bSupportMSASDeriv) {
      LogCommentFmt(L"Device does not support derivatives in Mesh and "
                    L"Amplification shaders");
    }

    switch (sm) {
    case D3D_SHADER_MODEL_6_5:
      pShaderOp->CS = pShaderOp->GetString("CS");
      pShaderOp->PS = pShaderOp->GetString("PS");
      pShaderOp->MS = pShaderOp->GetString("MS");
      pShaderOp->AS = pShaderOp->GetString("AS");
      break;
    case D3D_SHADER_MODEL_6_6:
      pShaderOp->CS = pShaderOp->GetString("CS66");
      pShaderOp->PS = pShaderOp->GetString("PS");
      if (bCheckDerivMSAS) {
        pShaderOp->MS = pShaderOp->GetString("MS66D");
        pShaderOp->AS = pShaderOp->GetString("AS66D");
      } else {
        pShaderOp->MS = pShaderOp->GetString("MS66");
        pShaderOp->AS = pShaderOp->GetString("AS66");
      }
      break;
    case D3D_SHADER_MODEL_6_7:
      pShaderOp->CS = pShaderOp->GetString("CS67");
      pShaderOp->PS = pShaderOp->GetString("PS67");
      if (bCheckDerivMSAS) {
        pShaderOp->MS = pShaderOp->GetString("MS67D");
        pShaderOp->AS = pShaderOp->GetString("AS67D");
      } else {
        pShaderOp->MS = pShaderOp->GetString("MS67");
        pShaderOp->AS = pShaderOp->GetString("AS67");
      }
      break;
    }

    // Test compute shader
    std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
        pDevice, m_support, "ProgOffset", SampleInitFn, ShaderOpSet);
    MappedData data;

    test->Test->GetReadBackData("U0", &data);
    VerifyProgOffsetResults((UINT *)data.data(), bCheckDerivCS);

    // Disable CS so graphics shaders go forward
    pShaderOp->CS = nullptr;

    if (DoesDeviceSupportMeshShaders(pDevice)) {
      test = RunShaderOpTestAfterParse(pDevice, m_support, "ProgOffset",
                                       SampleInitFn, ShaderOpSet);

      // PS
      test->Test->GetReadBackData("U0", &data);
      VerifyProgOffsetResults((UINT *)data.data(), true);

      // MS
      test->Test->GetReadBackData("U1", &data);
      VerifyProgOffsetResults((UINT *)data.data(), bCheckDerivMSAS);

      // AS
      test->Test->GetReadBackData("U2", &data);
      VerifyProgOffsetResults((UINT *)data.data(), bCheckDerivMSAS);
    }

    // Disable MS so PS goes forward
    pShaderOp->MS = nullptr;
    test = RunShaderOpTestAfterParse(pDevice, m_support, "ProgOffset",
                                     SampleInitFn, ShaderOpSet);

    test->Test->GetReadBackData("U0", &data);
    VerifyProgOffsetResults((UINT *)data.data(), true);

    bTestsSkipped = false;
  }

  if (bTestsSkipped) {
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
  }
}

// A mipmapped texture containing the value of LOD at each location in each
// level is used to sample at each level using SampleCmpLevel and confirm
// that the correct level is used for the comparison.
TEST_F(ExecutionTest, ATOSampleCmpLevelTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL_6_7))
    return;

  if (!DoesDeviceSupportAdvancedTexOps(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support Advanced Texture Operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());

  st::ShaderOp *pShaderOp = ShaderOpSet->GetShaderOp("SampleCmpLevel");

  // Initialize texture with the LOD number in each corresponding mip level
  auto SampleInitFn = [&](LPCSTR Name, std::vector<BYTE> &Data,
                          st::ShaderOp *pShaderOp) {
    UNREFERENCED_PARAMETER(pShaderOp);
    D3D12_RESOURCE_DESC &texDesc = pShaderOp->GetResourceByName(Name)->Desc;
    UINT texWidth = (UINT)texDesc.Width;
    UINT texHeight = (UINT)texDesc.Height;
    size_t size = sizeof(float) * texWidth * texHeight * 2;
    Data.resize(size);
    float *pPrimitives = (float *)Data.data();
    float val = 0.5;
    int ix = 0;
    while (texHeight > 0 && texWidth > 0) {
      if (!texHeight)
        texHeight = 1;
      if (!texWidth)
        texWidth = 1;
      for (size_t j = 0; j < texHeight; ++j) {
        for (size_t i = 0; i < texWidth; ++i) {
          pPrimitives[ix++] = val;
        }
      }
      val += 1.0;
      texHeight >>= 1;
      texWidth >>= 1;
    }
  };

  // Test compute shader
  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
      pDevice, m_support, "SampleCmpLevel", SampleInitFn, ShaderOpSet);
  MappedData data;

  test->Test->GetReadBackData("U0", &data);
  const UINT *pPixels = (UINT *)data.data();

  // Check that each LOD matches what's expected
  unsigned count = 2 * 7;
  // Since the results consist of a boolean, which should be true followed by
  // the result of a sampcmplvl, the only result expected is 1.
  for (unsigned i = 0; i < count; i++)
    VERIFY_ARE_EQUAL(pPixels[i], 1U);

  if (DoesDeviceSupportMeshShaders(pDevice)) {
    // Disable CS so mesh goes forward
    pShaderOp->CS = nullptr;
    test = RunShaderOpTestAfterParse(pDevice, m_support, "SampleCmpLevel",
                                     SampleInitFn, ShaderOpSet);

    test->Test->GetReadBackData("U0", &data);
    pPixels = (UINT *)data.data();

    for (unsigned i = 0; i < count; i++)
      VERIFY_ARE_EQUAL(pPixels[i], 1U);

    test->Test->GetReadBackData("U1", &data);
    pPixels = (UINT *)data.data();

    for (unsigned i = 0; i < count; i++)
      VERIFY_ARE_EQUAL(pPixels[i], 1U);

    test->Test->GetReadBackData("U2", &data);
    pPixels = (UINT *)data.data();

    for (unsigned i = 0; i < count; i++)
      VERIFY_ARE_EQUAL(pPixels[i], 1U);
  }
}

template <unsigned RSize> struct IntR {
  unsigned R : RSize;
  void SetChannels(unsigned R, unsigned G, unsigned B, unsigned A) {
    this->R = R;
    UNREFERENCED_PARAMETER(G);
    UNREFERENCED_PARAMETER(B);
    UNREFERENCED_PARAMETER(A);
  }
  static unsigned GetRSize() { return RSize; }
  static unsigned GetGSize() { return 0; }
  static unsigned GetBSize() { return 0; }
  static unsigned GetASize() { return 0; }
};

template <unsigned RSize, unsigned GSize> struct IntRG {
  unsigned R : RSize;
  unsigned G : GSize;
  void SetChannels(unsigned R, unsigned G, unsigned B, unsigned A) {
    this->R = R;
    this->G = G;
    UNREFERENCED_PARAMETER(B);
    UNREFERENCED_PARAMETER(A);
  }
  static unsigned GetRSize() { return RSize; }
  static unsigned GetGSize() { return GSize; }
  static unsigned GetBSize() { return 0; }
  static unsigned GetASize() { return 0; }
};

template <unsigned RSize, unsigned GSize, unsigned BSize> struct IntRGB {
  unsigned R : RSize;
  unsigned G : GSize;
  unsigned B : BSize;
  void SetChannels(unsigned R, unsigned G, unsigned B, unsigned A) {
    this->R = R;
    this->G = G;
    this->B = B;
    UNREFERENCED_PARAMETER(A);
  }
  static unsigned GetRSize() { return RSize; }
  static unsigned GetGSize() { return GSize; }
  static unsigned GetBSize() { return BSize; }
  static unsigned GetASize() { return 0; }
};

template <unsigned RSize, unsigned GSize, unsigned BSize, unsigned ASize>
struct IntRGBA {
  unsigned R : RSize;
  unsigned G : GSize;
  unsigned B : BSize;
  unsigned A : ASize;

  void SetChannels(unsigned R, unsigned G, unsigned B, unsigned A) {
    this->R = R;
    this->G = G;
    this->B = B;
    this->A = A;
  }
  static unsigned GetRSize() { return RSize; }
  static unsigned GetGSize() { return GSize; }
  static unsigned GetBSize() { return BSize; }
  static unsigned GetASize() { return ASize; }
};

struct IntRGBA10XRA2UNORM {
  uint32_t RGBA;
  void SetChannels(float R, float G, float B, float A) {
    uint32_t ur, ug, ub, ua;
    // Conversion values taken from XR documentation
    ur = GetMantissa(R * 510 + 385);
    ub = GetMantissa(B * 510 + 385);
    ug = GetMantissa(G * 510 + 385);
    ua = (uint32_t)A;

    // Cast off all but the 10 MSB and shift for packing
    ur = (ur & 0x7fE000) >> 13;
    ug = (ur & 0x7fE000) >> 3;
    ub = (ur & 0x7fE000) << 7;
    ua = (ua & 0x3) << 30;

    RGBA = ur | ug | ub | ua;
  }
};

struct Float32R {
  float R;
  void SetChannels(float R, float G, float B, float A) {
    this->R = R;
    UNREFERENCED_PARAMETER(G);
    UNREFERENCED_PARAMETER(B);
    UNREFERENCED_PARAMETER(A);
  }
};

struct Float32RG {
  float R, G;
  void SetChannels(float R, float G, float B, float A) {
    this->R = R;
    this->G = G;
    UNREFERENCED_PARAMETER(B);
    UNREFERENCED_PARAMETER(A);
  }
};

struct Float16R {
  uint16_t R;
  void SetChannels(float R, float G, float B, float A) {
    this->R = ConvertFloat32ToFloat16(R);
    UNREFERENCED_PARAMETER(G);
    UNREFERENCED_PARAMETER(B);
    UNREFERENCED_PARAMETER(A);
  }
};

struct Float16RG {
  uint16_t R, G;
  void SetChannels(float R, float G, float B, float A) {
    this->R = ConvertFloat32ToFloat16(R);
    this->G = ConvertFloat32ToFloat16(G);
    UNREFERENCED_PARAMETER(B);
    UNREFERENCED_PARAMETER(A);
  }
};

// No Float16RGB needed

struct Float16RGBA {
  uint16_t R, G, B, A;
  void SetChannels(float R, float G, float B, float A) {
    this->R = ConvertFloat32ToFloat16(R);
    this->G = ConvertFloat32ToFloat16(G);
    this->B = ConvertFloat32ToFloat16(B);
    this->A = ConvertFloat32ToFloat16(A);
  }
};

struct FloatR11G11B10 {
  uint32_t RGB;
  void SetChannels(float R, float G, float B, float A) {
    uint32_t ur, ug, ub;
    // Shift and mask so as to place R: 0-10, G: 11-21, B: 22-31
    // Sign and lesser-significant mantissa bits are truncated
    ur = (ConvertFloat32ToFloat16(R) >> 4) & 0x000007FF;
    ug = (ConvertFloat32ToFloat16(G) << 7) & 0x003FF800;
    ub = (ConvertFloat32ToFloat16(B) << 17) & 0xFFC00000;
    UNREFERENCED_PARAMETER(A);
    RGB = ur | ug | ub;
  }
};

struct FloatRGBE {
  uint32_t RGBE;
  // Conversion logic taken from miniengine PixelPacking header
  void SetChannels(UINT R, UINT G, UINT B, UINT A) {
    union {
      uint32_t i;
      float f;
    } ur, ug, ub, maxChannel, nextPow2;
    ur.f = (float)R;
    ug.f = (float)G;
    ub.f = (float)B;
    maxChannel.f = std::max(ur.f, std::max(ug.f, ub.f));
    // nextPow2 has to have the biggest exponent plus 1 (and nothing in the
    // mantissa)
    nextPow2.i = (maxChannel.i + 0x800000) & 0x7F800000;

    // By adding nextPow2, all channels have the same exponent, shifting their
    // mantissa bits to the right to accomodate it.  This also shifts in the
    // implicit '1' bit of all channels. The largest channel will always have
    // the high bit set.
    ur.f += nextPow2.f;
    ug.f += nextPow2.f;
    ub.f += nextPow2.f;
    UNREFERENCED_PARAMETER(A);

    ur.i = (ur.i << 9) >> 23;
    ug.i = (ug.i << 9) >> 23;
    ub.i = (ub.i << 9) >> 23;

    uint32_t e = ConvertFloat32ToFloat16(nextPow2.f) << 17;
    RGBE = ur.i | ug.i << 9 | ub.i << 18 | e;
  }

  static unsigned GetRSize() { return 9; }
  static unsigned GetGSize() { return 9; }
  static unsigned GetBSize() { return 9; }
  static unsigned GetASize() { return 0; }
};

template <typename RGBAType, unsigned xdim, unsigned ydim>
struct RawFloatTexture : public ExecutionTest::RawGatherTexture {
  DXGI_FORMAT m_format;
  RGBAType RGBA[xdim * ydim];
  RawFloatTexture(DXGI_FORMAT format) : m_format(format) {}
  // Set i'th element to floatified x,y and some derived values
  virtual void SetElement(int i, int x, int y) override {
    float r = (float)x;
    float g = (float)y;
    // provide some different values just to fill in b and a
    float b = (float)(x + y) * 0.5f;
    float a = (float)(x + y) * 0.1f;
    RGBA[i].SetChannels(r, g, b, a);
  }
  void *GetElements() override { return (void *)RGBA; }
  unsigned GetXDim() override { return xdim; }
  unsigned GetYDim() override { return ydim; }
  DXGI_FORMAT GetFormat() override { return m_format; };
};

template <unsigned xdim, unsigned ydim>
struct RawFloatR11G11B10ATexture : public ExecutionTest::RawGatherTexture {
  FloatR11G11B10 RGBA[xdim * ydim];
  // Set i'th element to floatified x,y and some derived values
  virtual void SetElement(int i, int x, int y) override {
    float r = (float)x;
    float g = (float)y;
    float b = (float)(x + y) * 0.5f;
    RGBA[i].SetChannels(r, g, b, 0);
  }
  void *GetElements() override { return (void *)RGBA; }
  unsigned GetXDim() override { return xdim; }
  unsigned GetYDim() override { return ydim; }
  DXGI_FORMAT GetFormat() override { return DXGI_FORMAT_R11G11B10_FLOAT; };
};

template <typename RGBAType, unsigned xdim, unsigned ydim>
struct RawIntTexture : public ExecutionTest::RawGatherTexture {
  bool m_isSigned;
  bool m_isNorm;
  unsigned m_maxVal;
  DXGI_FORMAT m_format;
  RGBAType RGBA[xdim * ydim];
  RawIntTexture(bool isSigned, bool isNorm, int maxVal, DXGI_FORMAT format)
      : m_isSigned(isSigned), m_isNorm(isNorm), m_maxVal(maxVal + 2),
        m_format(format) {
    if (isSigned)
      m_maxVal /= 2;
  }
  // Set i'th element to values scaled per max dimentions for norms, shifted for
  // signed but otherwise just the x and y values themselves
  virtual void SetElement(int i, int x, int y) override {
    double fr = x;
    double fg = y;
    // provide some different values just to fill in b and a
    double fb = x + 2;
    double fa = y + 2;
    // If signed, get some unsigned values in there
    if (m_isSigned) {
      fr -= m_maxVal;
      fg -= m_maxVal;
      fb -= m_maxVal;
      fa -= m_maxVal;
    }
    // If normalized, scale to given range
    if (m_isNorm) {
      fr /= m_maxVal;
      fg /= m_maxVal;
      fb /= m_maxVal;
      fa /= m_maxVal;

      fr *= (1 << (RGBAType::GetRSize() - m_isSigned - 1));
      fg *= (1 << (RGBAType::GetGSize() - m_isSigned - 1));
      fb *= (1 << (RGBAType::GetBSize() - m_isSigned - 1));
      fa *= (1 << (RGBAType::GetASize() - 1));
    }
    RGBA[i].SetChannels((UINT)fr, (UINT)fg, (UINT)fb, (UINT)fa);
  }
  void *GetElements() override { return (void *)RGBA; }
  unsigned GetXDim() override { return xdim; }
  unsigned GetYDim() override { return ydim; }
  DXGI_FORMAT GetFormat() override { return m_format; };
};

template <unsigned xdim, unsigned ydim>
struct RawR10G10B10XRA2Texture : public ExecutionTest::RawGatherTexture {
  unsigned m_maxVal;
  DXGI_FORMAT m_format;
  IntRGBA10XRA2UNORM RGBA[xdim * ydim];
  RawR10G10B10XRA2Texture(int maxVal, DXGI_FORMAT format)
      : m_maxVal((maxVal + 2) / 2), m_format(format) {}
  // Set i'th element to values scaled and shifted for available range
  virtual void SetElement(int i, int x, int y) override {
    double fr = x;
    double fg = y;
    // provide some different values just to fill in b and a
    double fb = x + 2;
    double fa = y + 2;

    // Shift RGB to valid range which will be -0.75 - 1.25
    fr -= m_maxVal * .75;
    fg -= m_maxVal * .75;
    fb -= m_maxVal * .75;

    // normalize to something that will fit in the limited range
    fr /= m_maxVal;
    fg /= m_maxVal;
    fb /= m_maxVal;
    fa /= m_maxVal * 2;

    fa *= 3; // scale to max in range

    RGBA[i].SetChannels((float)fr, (float)fg, (float)fb, (float)fa);
  }
  void *GetElements() override { return (void *)RGBA; }
  unsigned GetXDim() override { return xdim; }
  unsigned GetYDim() override { return ydim; }
  DXGI_FORMAT GetFormat() override { return m_format; };
};

// #define RAWGATHER_FALLBACK // Enable to use pre-6.7 fallback mechanisms to
// vet raw gather tests

// Create a single resource of <resFormat> and alias it to a view of
// <viewFormat> Then execute a shader that uses raw gather to copy the values
// into a UAV Verify that the UAV has the same values as passed in.
template <typename GatherType>
void ExecutionTest::DoRawGatherTest(ID3D12Device *pDevice,
                                    RawGatherTexture *rawTex,
                                    DXGI_FORMAT viewFormat) {

  DXGI_FORMAT resFormat = rawTex->GetFormat();
#ifdef RAWGATHER_FALLBACK
  // There is no uint64 version of Gather, so 64-bit fallback needs to use Loads
  const char shaderTemplate64[] =
      "Texture2D<uint%d_t> g_tex : register(t0);\n"
      "RWStructuredBuffer<uint%d_t> g_out : register(u0);\n"
      "SamplerState g_samp : register(s0);\n"
      "[NumThreads(32, 32, 1)]\n"
      "void main(uint3 id : SV_GroupThreadID, uint ix : SV_GroupIndex) {\n"
      "  //uint%d_t4 res = g_tex.%s(g_samp, (id.xy+0.5)/31.0);\n"
      "  g_out[4*ix+0] = g_tex.Load(uint3(id.x, id.y+1, 0));\n"
      "  g_out[4*ix+1] = g_tex.Load(uint3(id.x+1, id.y+1, 0));\n"
      "  g_out[4*ix+2] = g_tex.Load(uint3(id.x+1, id.y, 0));\n"
      "  g_out[4*ix+3] = g_tex.Load(uint3(id.x, id.y, 0));\n"
      "}";
#endif
  const char shaderTemplate[] =
      "Texture2D<uint%d_t> g_tex : register(t0);\n"
      "RWStructuredBuffer<uint%d_t> g_out : register(u0);\n"
      "SamplerState g_samp : register(s0);\n"
      "[NumThreads(32, 32, 1)]\n"
      "void main(uint3 id : SV_GroupThreadID, uint ix : SV_GroupIndex) {\n"
      "  uint%d_t4 res = g_tex.%s(g_samp, (id.xy+0.5)/31.0);\n"
      "  g_out[4*ix+0] = res.x;\n"
      "  g_out[4*ix+1] = res.y;\n"
      "  g_out[4*ix+2] = res.z;\n"
      "  g_out[4*ix+3] = res.w;\n"
      "}";

  char pShader[sizeof(shaderTemplate) +
               200]; // A little padding to account for variations
  UINT uintSize = sizeof(GatherType) * 8; // bytes to bits

  const char *gatherFuncName = "GatherRaw";
#ifdef RAWGATHER_FALLBACK
  gatherFuncName = "Gather";
  if (sizeof(GatherType) == 8)
    VERIFY_IS_GREATER_THAN(sprintf(pShader, shaderTemplate64, uintSize,
                                   uintSize, uintSize, gatherFuncName),
                           0);
  else
#endif
    VERIFY_IS_GREATER_THAN(sprintf(pShader, shaderTemplate, uintSize, uintSize,
                                   uintSize, gatherFuncName),
                           0);

  const UINT xDim = rawTex->GetXDim();
  const UINT yDim = rawTex->GetYDim();
  const UINT valueSize = xDim * yDim;
  const UINT valueSizeInBytes = valueSize * sizeof(GatherType);

  CComPtr<ID3D12CommandQueue> pCommandQueue;
  CComPtr<ID3D12CommandAllocator> pCommandAllocator;
  FenceObj FO;

  CreateComputeCommandQueue(pDevice, L"RawGather Queue", &pCommandQueue);
  InitFenceObj(pDevice, &FO);

  // Create root signature.
  CComPtr<ID3D12RootSignature> pRootSignature;
  CD3DX12_DESCRIPTOR_RANGE ranges[2];
  CD3DX12_DESCRIPTOR_RANGE srange[1];
  ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
  ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);
  srange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0);

  CreateRootSignatureFromRanges(pDevice, &pRootSignature, ranges, 2, srange, 1);

  VERIFY_SUCCEEDED(pDevice->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&pCommandAllocator)));

  // Create command list and resources
  CComPtr<ID3D12GraphicsCommandList> pCommandList;
  VERIFY_SUCCEEDED(pDevice->CreateCommandList(
      0, D3D12_COMMAND_LIST_TYPE_COMPUTE, pCommandAllocator, nullptr,
      IID_PPV_ARGS(&pCommandList)));

  // Set up castable format list (of one) if possible, or else just alias the
  // formats with the expectation that unsupported cases won't be used by the
  // caller
  DXGI_FORMAT *castableFmt = nullptr;
  if (DoesDeviceSupportEnhancedBarriers(pDevice))
    castableFmt = &viewFormat;
  else
    resFormat = viewFormat;

  // Set up texture to be raw gathered from
  CComPtr<ID3D12Resource> pTexResource;
  CComPtr<ID3D12Resource> pTexUploadResource;
  int ix = 0;
  for (UINT y = 0; y < yDim; y++)
    for (UINT x = 0; x < xDim; x++)
      rawTex->SetElement(ix++, x, y);
  D3D12_RESOURCE_DESC tex2dDesc = CD3DX12_RESOURCE_DESC::Tex2D(
      resFormat, xDim, yDim, 1 /* sampCt */, 1 /* mipCt */);

  CreateTestResources(pDevice, pCommandList, rawTex->GetElements(),
                      valueSizeInBytes, tex2dDesc, &pTexResource,
                      &pTexUploadResource, nullptr /*pReadBufer*/, castableFmt);

  // Set up Output Resource
  CComPtr<ID3D12Resource> pOutputResource;
  CComPtr<ID3D12Resource> pOutputReadBuffer;
  CComPtr<ID3D12Resource> pOutputUploadResource;

  // 4x because gather produces four result values
  GatherType *outVals = new GatherType[valueSize * 4];
  memset(outVals, 0xd,
         valueSizeInBytes * 4); // 0xd to give a sentinal value for failures
  CreateTestUavs(pDevice, pCommandList, outVals, valueSizeInBytes * 4,
                 &pOutputResource, &pOutputUploadResource, &pOutputReadBuffer);
  delete[] outVals;

  // Close the command list and execute it to perform the resource uploads
  pCommandList->Close();
  ID3D12CommandList *ppCommandLists[] = {pCommandList};
  pCommandQueue->ExecuteCommandLists(1, ppCommandLists);
  WaitForSignal(pCommandQueue, FO);

  // Create shaders
#ifdef RAWGATHER_FALLBACK
  const wchar_t *target = L"cs_6_2";
#else
  const wchar_t *target = L"cs_6_7";
#endif

  LPCWSTR opts[] = {L"-enable-16bit-types"};

  CComPtr<ID3D12PipelineState> pPSO;
  CreateComputePSO(pDevice, pRootSignature, pShader, target, &pPSO, opts,
                   _countof(opts));

  // Reset commandlist to shader PSO
  VERIFY_SUCCEEDED(pCommandList->Reset(pCommandAllocator, pPSO));

  // Describe and create a resource descriptor heap.
  CComPtr<ID3D12DescriptorHeap> pResHeap;
  CComPtr<ID3D12DescriptorHeap> pSampHeap;
  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors = 2;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  VERIFY_SUCCEEDED(
      pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pResHeap)));

  // Describe and create a sampler descriptor heap.
  heapDesc.NumDescriptors = 1;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
  VERIFY_SUCCEEDED(
      pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pSampHeap)));

  CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(
      pResHeap->GetCPUDescriptorHandleForHeapStart());
  CreateTex2DSRV(pDevice, cpuHandle, viewFormat, pTexResource);
  CreateStructUAV(pDevice, cpuHandle, 4 * valueSize, sizeof(GatherType),
                  pOutputResource);

  D3D12_FILTER filters[] = {D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
                            D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT};
  CreateDefaultSamplers(pDevice,
                        pSampHeap->GetCPUDescriptorHandleForHeapStart(),
                        filters, nullptr /*perSampleBorderColors*/, 1);

  // Set Heaps, Rootsignature and table
  ID3D12DescriptorHeap *const pHeaps[2] = {pResHeap, pSampHeap};
  pCommandList->SetDescriptorHeaps(2, pHeaps);
  pCommandList->SetComputeRootSignature(pRootSignature);
  pCommandList->SetComputeRootDescriptorTable(
      0, pResHeap->GetGPUDescriptorHandleForHeapStart());
  pCommandList->SetComputeRootDescriptorTable(
      1, pSampHeap->GetGPUDescriptorHandleForHeapStart());

  // dispatch and close shader
  pCommandList->Dispatch(1, 1, 1);

  // Copy the results back to readable memory
  CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      pOutputResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
      D3D12_RESOURCE_STATE_COPY_SOURCE);
  pCommandList->ResourceBarrier(1, &barrier);
  pCommandList->CopyResource(pOutputReadBuffer, pOutputResource);

  pCommandList->Close();

  pCommandQueue->ExecuteCommandLists(1, ppCommandLists);
  WaitForSignal(pCommandQueue, FO);

  MappedData mappedData(pOutputReadBuffer, 4 * valueSizeInBytes);
  GatherType *pData = (GatherType *)mappedData.data();
  GatherType *texVals = (GatherType *)rawTex->GetElements();
  UINT yCt = yDim;
  UINT xCt = xDim;
#ifdef RAWGATHER_FALLBACK
  // 64-bit fallback uses Load, which doesn't support clamp addressing. so don't
  // test it
  if (sizeof(GatherType) == 8) {
    yCt--;
    xCt--;
  }
#endif
  for (UINT y = 0; y < yCt; y++) {
    UINT yp1 = y + 1 >= yDim ? y : y + 1;
    for (UINT x = 0; x < xCt; x++) {
      UINT xp1 = x + 1 >= xDim ? x : x + 1;
      // Because this order may be unexpected, I'll quote the spec:
      // "The four samples that would contribute to filtering are placed into
      // xyzw
      //  in counter clockwise order starting with the sample to the lower left"
      VERIFY_ARE_EQUAL(pData[4 * (32 * y + x) + 0], texVals[yp1 * xDim + x]);
      VERIFY_ARE_EQUAL(pData[4 * (32 * y + x) + 1], texVals[yp1 * xDim + xp1]);
      VERIFY_ARE_EQUAL(pData[4 * (32 * y + x) + 2], texVals[y * xDim + xp1]);
      VERIFY_ARE_EQUAL(pData[4 * (32 * y + x) + 3], texVals[y * xDim + x]);
    }
  }
}

// Create textures of various types and alias them to the unsigned integer
// format that has the same element size and initializes them with various
// values, The shader code copies the results of raw gather to an unsigned
// integer UAV The UAV contents are compared to the values assigned to the
// texture A few levels of support are available: pre-6.7 fallback - fakey hand
// waving to make it look like it's doing the right thing 6.7 support only - No
// casting ability of resources to views beyond native support, but GatherRaw is
// available 6.7 + Enh. Barriers - Same formats can be cast as in native, but
// use new createcommittedresource3() 6.7 + Enh. Barriers + Relaxed Cast - All
// format casting and raw gathering of all
TEST_F(ExecutionTest, ATORawGather) {

  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

#ifdef RAWGATHER_FALLBACK
  D3D_SHADER_MODEL sm = D3D_SHADER_MODEL_6_6;
#else
  D3D_SHADER_MODEL sm = D3D_SHADER_MODEL_6_7;
#endif
  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, sm))
    return;

#ifndef RAWGATHER_FALLBACK
  if (!DoesDeviceSupportAdvancedTexOps(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support Advanced Texture Operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }
#endif

  static const int NumThreadsX = 32;
  static const int NumThreadsY = 32;

  // Create an array of texture variants with the raw texture base class
  // Then plug them into DoRawGather to perform the test and evaluate the
  // results for each
  RawIntTexture<IntRG<32, 32>, NumThreadsX, NumThreadsY> R32G32_TYPELESS(
      false, false, NumThreadsX, DXGI_FORMAT_R32G32_TYPELESS);
  RawIntTexture<IntRG<32, 32>, NumThreadsX, NumThreadsY> R32G32_UINT(
      false, false, NumThreadsX, DXGI_FORMAT_R32G32_UINT);
  RawIntTexture<IntRG<32, 32>, NumThreadsX, NumThreadsY> R32G32_SINT(
      true, false, NumThreadsX, DXGI_FORMAT_R32G32_SINT);

  RawIntTexture<IntRGBA<16, 16, 16, 16>, NumThreadsX, NumThreadsY>
      R16G16B16A16_TYPELESS(false, false, NumThreadsX,
                            DXGI_FORMAT_R16G16B16A16_TYPELESS);
  RawIntTexture<IntRGBA<16, 16, 16, 16>, NumThreadsX, NumThreadsY>
      R16G16B16A16_UINT(false, false, NumThreadsX,
                        DXGI_FORMAT_R16G16B16A16_UINT);
  RawIntTexture<IntRGBA<16, 16, 16, 16>, NumThreadsX, NumThreadsY>
      R16G16B16A16_SINT(true, false, NumThreadsX,
                        DXGI_FORMAT_R16G16B16A16_SINT);
  RawIntTexture<IntRGBA<16, 16, 16, 16>, NumThreadsX, NumThreadsY>
      R16G16B16A16_UNORM(false, true, NumThreadsX,
                         DXGI_FORMAT_R16G16B16A16_UNORM);
  RawIntTexture<IntRGBA<16, 16, 16, 16>, NumThreadsX, NumThreadsY>
      R16G16B16A16_SNORM(true, true, NumThreadsX,
                         DXGI_FORMAT_R16G16B16A16_SNORM);
  RawFloatTexture<Float16RGBA, NumThreadsX, NumThreadsY> R16G16B16A16_FLOAT(
      DXGI_FORMAT_R16G16B16A16_FLOAT);
  RawFloatTexture<Float32RG, NumThreadsX, NumThreadsY> R32G32_FLOAT(
      DXGI_FORMAT_R32G32_FLOAT);

  RawGatherTexture *Int64Textures[] = {
      &R32G32_TYPELESS,       &R32G32_UINT,        &R32G32_SINT,
      &R16G16B16A16_TYPELESS, &R16G16B16A16_UINT,  &R16G16B16A16_SINT,
      &R16G16B16A16_UNORM,    &R16G16B16A16_SNORM, &R16G16B16A16_FLOAT,
      &R32G32_FLOAT};

  RawIntTexture<IntR<32>, NumThreadsX, NumThreadsY> R32_TYPELESS(
      false, false, NumThreadsX, DXGI_FORMAT_R32_TYPELESS);
  RawIntTexture<IntR<32>, NumThreadsX, NumThreadsY> R32_SINT(
      true, false, NumThreadsX, DXGI_FORMAT_R32_SINT);
  RawIntTexture<IntR<32>, NumThreadsX, NumThreadsY> R32_UINT(
      true, false, NumThreadsX, DXGI_FORMAT_R32_UINT);

  RawIntTexture<IntRGBA<10, 10, 10, 2>, NumThreadsX, NumThreadsY>
      R10G10B10A2_TYPELESS(false, false, NumThreadsX,
                           DXGI_FORMAT_R10G10B10A2_TYPELESS);
  RawIntTexture<IntRGBA<10, 10, 10, 2>, NumThreadsX, NumThreadsY>
      R10G10B10A2_UNORM(false, true, NumThreadsX,
                        DXGI_FORMAT_R10G10B10A2_UNORM);
  RawIntTexture<IntRGBA<10, 10, 10, 2>, NumThreadsX, NumThreadsY>
      R10G10B10A2_UINT(false, false, NumThreadsX, DXGI_FORMAT_R10G10B10A2_UINT);
  RawR10G10B10XRA2Texture<NumThreadsX, NumThreadsY>
      R10G10B10A2_XR_BIAS_A2_UNORM(NumThreadsX,
                                   DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM);
  RawIntTexture<FloatRGBE, NumThreadsX, NumThreadsY> R9G9B9E5_SHAREDEXP(
      false, false, NumThreadsX, DXGI_FORMAT_R9G9B9E5_SHAREDEXP);

  RawIntTexture<IntRGBA<8, 8, 8, 8>, NumThreadsX, NumThreadsY>
      R8G8B8A8_TYPELESS(false, false, NumThreadsX,
                        DXGI_FORMAT_R8G8B8A8_TYPELESS);
  RawIntTexture<IntRGBA<8, 8, 8, 8>, NumThreadsX, NumThreadsY> R8G8B8A8_UNORM(
      false, true, NumThreadsX, DXGI_FORMAT_R8G8B8A8_UNORM);
  RawIntTexture<IntRGBA<8, 8, 8, 8>, NumThreadsX, NumThreadsY>
      R8G8B8A8_UNORM_SRGB(false, true, NumThreadsX, DXGI_FORMAT_R8G8B8A8_UNORM);
  RawIntTexture<IntRGBA<8, 8, 8, 8>, NumThreadsX, NumThreadsY> R8G8B8A8_UINT(
      false, false, NumThreadsX, DXGI_FORMAT_R8G8B8A8_UINT);
  RawIntTexture<IntRGBA<8, 8, 8, 8>, NumThreadsX, NumThreadsY> R8G8B8A8_SNORM(
      true, true, NumThreadsX, DXGI_FORMAT_R8G8B8A8_SNORM);
  RawIntTexture<IntRGBA<8, 8, 8, 8>, NumThreadsX, NumThreadsY> R8G8B8A8_SINT(
      true, false, NumThreadsX, DXGI_FORMAT_R8G8B8A8_SINT);

  RawIntTexture<IntRG<16, 16>, NumThreadsX, NumThreadsY> R16G16_TYPELESS(
      false, false, NumThreadsX, DXGI_FORMAT_R16G16_TYPELESS);
  RawIntTexture<IntRG<16, 16>, NumThreadsX, NumThreadsY> R16G16_UNORM(
      false, true, NumThreadsX, DXGI_FORMAT_R16G16_UNORM);
  RawIntTexture<IntRG<16, 16>, NumThreadsX, NumThreadsY> R16G16_UINT(
      false, false, NumThreadsX, DXGI_FORMAT_R16G16_UINT);
  RawIntTexture<IntRG<16, 16>, NumThreadsX, NumThreadsY> R16G16_SNORM(
      true, true, NumThreadsX, DXGI_FORMAT_R16G16_SNORM);
  RawIntTexture<IntRG<16, 16>, NumThreadsX, NumThreadsY> R16G16_SINT(
      true, false, NumThreadsX, DXGI_FORMAT_R16G16_SINT);

  RawIntTexture<IntRGBA<8, 8, 8, 8>, NumThreadsX, NumThreadsY>
      B8G8R8A8_TYPELESS(false, false, NumThreadsX,
                        DXGI_FORMAT_B8G8R8A8_TYPELESS);
  RawIntTexture<IntRGBA<8, 8, 8, 8>, NumThreadsX, NumThreadsY> B8G8R8A8_UNORM(
      false, true, NumThreadsX, DXGI_FORMAT_B8G8R8A8_UNORM);
  RawIntTexture<IntRGBA<8, 8, 8, 8>, NumThreadsX, NumThreadsY>
      B8G8R8A8_UNORM_SRGB(false, true, NumThreadsX,
                          DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);

  RawIntTexture<IntRGBA<8, 8, 8, 8>, NumThreadsX, NumThreadsY>
      B8G8R8X8_TYPELESS(false, false, NumThreadsX,
                        DXGI_FORMAT_B8G8R8X8_TYPELESS);
  RawIntTexture<IntRGBA<8, 8, 8, 8>, NumThreadsX, NumThreadsY> B8G8R8X8_UNORM(
      false, true, NumThreadsX, DXGI_FORMAT_B8G8R8X8_UNORM);
  RawIntTexture<IntRGBA<8, 8, 8, 8>, NumThreadsX, NumThreadsY>
      B8G8R8X8_UNORM_SRGB(false, true, NumThreadsX,
                          DXGI_FORMAT_B8G8R8X8_UNORM_SRGB);

  RawFloatTexture<Float32R, NumThreadsX, NumThreadsY> R32_FLOAT(
      DXGI_FORMAT_R32_FLOAT);
  RawFloatR11G11B10ATexture<NumThreadsX, NumThreadsY> R11G11B10_FLOAT;
  RawFloatTexture<Float16RG, NumThreadsX, NumThreadsY> R16G16_FLOAT(
      DXGI_FORMAT_R16G16_FLOAT);

  RawGatherTexture *Int32Textures[] = {&R32_TYPELESS,
                                       &R32_UINT,
                                       &R32_SINT,
                                       &R10G10B10A2_TYPELESS,
                                       &R10G10B10A2_UNORM,
                                       &R10G10B10A2_UINT,
                                       &R10G10B10A2_XR_BIAS_A2_UNORM,
                                       &R9G9B9E5_SHAREDEXP,
                                       &R8G8B8A8_TYPELESS,
                                       &R8G8B8A8_UNORM,
                                       &R8G8B8A8_UNORM_SRGB,
                                       &R8G8B8A8_UINT,
                                       &R8G8B8A8_SNORM,
                                       &R8G8B8A8_SINT,
                                       &R16G16_TYPELESS,
                                       &R16G16_UNORM,
                                       &R16G16_UINT,
                                       &R16G16_SNORM,
                                       &R16G16_SINT,
                                       &B8G8R8A8_TYPELESS,
                                       &B8G8R8A8_UNORM,
                                       &B8G8R8A8_UNORM_SRGB,
                                       &B8G8R8X8_TYPELESS,
                                       &B8G8R8X8_UNORM,
                                       &B8G8R8X8_UNORM_SRGB,
                                       &R32_FLOAT,
                                       &R11G11B10_FLOAT,
                                       &R16G16_FLOAT};

  RawIntTexture<IntR<16>, NumThreadsX, NumThreadsY> R16_TYPELESS(
      false, false, NumThreadsX, DXGI_FORMAT_R16_TYPELESS);
  RawIntTexture<IntR<16>, NumThreadsX, NumThreadsY> R16_SINT(
      true, false, NumThreadsX, DXGI_FORMAT_R16_SINT);
  RawIntTexture<IntR<16>, NumThreadsX, NumThreadsY> R16_UINT(
      true, false, NumThreadsX, DXGI_FORMAT_R16_UINT);
  RawIntTexture<IntR<16>, NumThreadsX, NumThreadsY> R16_UNORM(
      false, true, NumThreadsX, DXGI_FORMAT_R16_UNORM);
  RawIntTexture<IntR<16>, NumThreadsX, NumThreadsY> R16_SNORM(
      true, true, NumThreadsX, DXGI_FORMAT_R16_SNORM);
  RawFloatTexture<Float16R, NumThreadsX, NumThreadsY> R16_FLOAT(
      DXGI_FORMAT_R16_FLOAT);

  RawIntTexture<IntRG<8, 8>, NumThreadsX, NumThreadsY> R8G8_TYPELESS(
      false, false, NumThreadsX, DXGI_FORMAT_R8G8_TYPELESS);
  RawIntTexture<IntRG<8, 8>, NumThreadsX, NumThreadsY> R8G8_UINT(
      false, false, NumThreadsX, DXGI_FORMAT_R8G8_UINT);
  RawIntTexture<IntRG<8, 8>, NumThreadsX, NumThreadsY> R8G8_SINT(
      true, false, NumThreadsX, DXGI_FORMAT_R8G8_SINT);
  RawIntTexture<IntRG<8, 8>, NumThreadsX, NumThreadsY> R8G8_UNORM(
      false, true, NumThreadsX, DXGI_FORMAT_R8G8_UNORM);
  RawIntTexture<IntRG<8, 8>, NumThreadsX, NumThreadsY> R8G8_SNORM(
      true, true, NumThreadsX, DXGI_FORMAT_R8G8_SNORM);
  RawIntTexture<IntRGB<5, 6, 5>, NumThreadsX, NumThreadsY> B5G6R5_UNORM(
      false, true, NumThreadsX, DXGI_FORMAT_B5G6R5_UNORM);
  RawIntTexture<IntRGBA<5, 5, 5, 1>, NumThreadsX, NumThreadsY> B5G5R5A1_UNORM(
      false, true, NumThreadsX, DXGI_FORMAT_B5G5R5A1_UNORM);
  RawIntTexture<IntRGBA<4, 4, 4, 4>, NumThreadsX, NumThreadsY> B4G4R4A4_UNORM(
      false, true, NumThreadsX, DXGI_FORMAT_B4G4R4A4_UNORM);

  RawGatherTexture *Int16Textures[] = {
      &R16_TYPELESS,   &R16_UINT,      &R16_SINT,     &R16_UNORM,
      &R16_SNORM,      &R8G8_TYPELESS, &R8G8_UINT,    &R8G8_SINT,
      &R8G8_UNORM,     &R8G8_SNORM,    &B5G6R5_UNORM, &B5G5R5A1_UNORM,
      &B4G4R4A4_UNORM, &R16_FLOAT};

  bool canCast = DoesDeviceSupportRelaxedFormatCasting(pDevice);
  int int32Ct = canCast ? _countof(Int32Textures)
                        : 3; // The first three are already castable to UINT32

  for (int i = 0; i < int32Ct; i++) {
    DoRawGatherTest<uint32_t>(pDevice, Int32Textures[i], DXGI_FORMAT_R32_UINT);
  }

  if (DoesDeviceSupportNative16bitOps(pDevice)) {
    int int16Ct = canCast ? _countof(Int16Textures)
                          : 5; // The first five are already castable to UINT16
    for (int i = 0; i < int16Ct; i++) {
      DoRawGatherTest<uint16_t>(pDevice, Int16Textures[i],
                                DXGI_FORMAT_R16_UINT);
    }
  }
  if (DoesDeviceSupportInt64(pDevice)) {
    int int64Ct = canCast ? _countof(Int64Textures)
                          : 3; // The first three are already castable to UINT64
    for (int i = 0; i < int64Ct; i++) {
      DoRawGatherTest<uint64_t>(pDevice, Int64Textures[i],
                                DXGI_FORMAT_R32G32_UINT);
    }
  }
}

// Executing a simple binop to verify shadel model 6.1 support; runs with
// ShaderModel61.CoreRequirement
TEST_F(ExecutionTest, BasicShaderModel61) {
  RunBasicShaderModelTest(D3D_SHADER_MODEL_6_1);
}

// Executing a simple binop to verify shadel model 6.3 support; runs with
// ShaderModel63.CoreRequirement
TEST_F(ExecutionTest, BasicShaderModel63) {
  RunBasicShaderModelTest(D3D_SHADER_MODEL_6_3);
}

void ExecutionTest::RunBasicShaderModelTest(D3D_SHADER_MODEL shaderModel) {

  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, shaderModel)) {
    return;
  }

  std::string pShaderModelStr;
  if (shaderModel == D3D_SHADER_MODEL_6_1) {
    pShaderModelStr = "cs_6_1";
  } else if (shaderModel == D3D_SHADER_MODEL_6_3) {
    pShaderModelStr = "cs_6_3";
  } else {
    DXASSERT_NOMSG("Invalid Shader Model Parameter");
    pShaderModelStr = "";
  }

  const char shaderTemplate[] =
      "struct SBinaryOp { %s input1; %s input2; %s output; };"
      "RWStructuredBuffer<SBinaryOp> g_buf : register(u0);"
      "[numthreads(8,8,1)]"
      "void main(uint GI : SV_GroupIndex) {"
      "    SBinaryOp l = g_buf[GI];"
      "    l.output = l.input1 + l.input2;"
      "    g_buf[GI] = l;"
      "}";
  char shader[sizeof(shaderTemplate) + 50];

  // Run simple shader with float data types
  const char *sTy = "float";
  float inputFloatPairs[] = {1.5f, -2.8f, 3.23e-5f, 6.0f, 181.621f, 14.978f};
  VERIFY_IS_TRUE(sprintf(shader, shaderTemplate, sTy, sTy, sTy) > 0);
  WEX::Logging::Log::Comment(L"BasicShaderModel float");
  RunBasicShaderModelTest<float>(pDevice, pShaderModelStr.c_str(), shader,
                                 inputFloatPairs,
                                 sizeof(inputFloatPairs) / (2 * sizeof(float)));

  // Run simple shader with double data types
  if (DoesDeviceSupportDouble(pDevice)) {
    const char *sTy = "double";
    double inputDoublePairs[] = {1.5891020, -2.8,      3.23e-5,
                                 1 / 3,     181.91621, 14.654978};
    VERIFY_IS_TRUE(sprintf(shader, shaderTemplate, sTy, sTy, sTy) > 0);
    WEX::Logging::Log::Comment(L"BasicShaderModel double");
    RunBasicShaderModelTest<double>(
        pDevice, pShaderModelStr.c_str(), shader, inputDoublePairs,
        sizeof(inputDoublePairs) / (2 * sizeof(double)));
  } else {
    // Optional feature, so it's correct to not support it if declared as such.
    WEX::Logging::Log::Comment(L"Device does not support double operations.");
  }

  // Run simple shader with int64 types
  if (DoesDeviceSupportInt64(pDevice)) {
    const char *sTy = "int64_t";
    int64_t inputInt64Pairs[] = {1, -100, 6814684, -9814810, 654, 1021248900};
    VERIFY_IS_TRUE(sprintf(shader, shaderTemplate, sTy, sTy, sTy) > 0);
    WEX::Logging::Log::Comment(L"BasicShaderModel int64_t");
    RunBasicShaderModelTest<int64_t>(
        pDevice, pShaderModelStr.c_str(), shader, inputInt64Pairs,
        sizeof(inputInt64Pairs) / (2 * sizeof(int64_t)));
  } else {
    // Optional feature, so it's correct to not support it if declared as such.
    WEX::Logging::Log::Comment(L"Device does not support int64 operations.");
  }
}

template <class Ty>
const wchar_t *ExecutionTest::BasicShaderModelTest_GetFormatString() {
  DXASSERT_NOMSG("Unsupported type");
  return "";
}

template <>
const wchar_t *ExecutionTest::BasicShaderModelTest_GetFormatString<float>() {
  return L"element #%u: input1 = %6.8f, input1 = %6.8f, output = %6.8f, "
         L"expected = %6.8f";
}

template <>
const wchar_t *ExecutionTest::BasicShaderModelTest_GetFormatString<double>() {
  return BasicShaderModelTest_GetFormatString<float>();
}

template <>
const wchar_t *ExecutionTest::BasicShaderModelTest_GetFormatString<int64_t>() {
  return L"element #%u: input1 = %ld, input1 = %ld, output = %ld, expected = "
         L"%ld";
}

template <class Ty>
void ExecutionTest::RunBasicShaderModelTest(CComPtr<ID3D12Device> pDevice,
                                            const char *pShaderModelStr,
                                            const char *pShader,
                                            Ty *pInputDataPairs,
                                            unsigned inputDataCount) {
  struct SBinaryOp {
    Ty input1;
    Ty input2;
    Ty output;
  };

  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "BinaryFPOp",
      // this callbacked is called when the test is creating the resource to run
      // the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        UNREFERENCED_PARAMETER(Name);
        pShaderOp->Shaders.at(0).Target = pShaderModelStr;
        pShaderOp->Shaders.at(0).Text = pShader;
        size_t size = sizeof(SBinaryOp) * inputDataCount;
        Data.resize(size);
        SBinaryOp *pPrimitives = (SBinaryOp *)Data.data();
        Ty *pIn = pInputDataPairs;
        for (size_t i = 0; i < inputDataCount; i++, pIn += 2) {
          SBinaryOp *p = &pPrimitives[i];
          p->input1 = pIn[0];
          p->input2 = pIn[1];
        }
      });

  VERIFY_SUCCEEDED(S_OK);

  MappedData data;
  test->Test->GetReadBackData("SBinaryFPOp", &data);
  SBinaryOp *pPrimitives = (SBinaryOp *)data.data();

  const wchar_t *formatStr = BasicShaderModelTest_GetFormatString<Ty>();
  Ty *pIn = pInputDataPairs;

  for (unsigned i = 0; i < inputDataCount; i++, pIn += 2) {
    Ty expValue = pIn[0] + pIn[1];
    SBinaryOp *p = &pPrimitives[i];

    LogCommentFmt(formatStr, i, pIn[0], pIn[1], p->output, expValue);
    VERIFY_ARE_EQUAL(p->output, expValue);
  }
}

// Resource structure for data-driven tests.

struct SUnaryFPOp {
  float input;
  float output;
};

struct SBinaryFPOp {
  float input1;
  float input2;
  float output1;
  float output2;
};

struct STertiaryFPOp {
  float input1;
  float input2;
  float input3;
  float output;
};

struct SUnaryHalfOp {
  uint16_t input;
  uint16_t output;
};

struct SBinaryHalfOp {
  uint16_t input1;
  uint16_t input2;
  uint16_t output1;
  uint16_t output2;
};

struct STertiaryHalfOp {
  uint16_t input1;
  uint16_t input2;
  uint16_t input3;
  uint16_t output;
};

struct SUnaryIntOp {
  int input;
  int output;
};

struct SUnaryUintOp {
  unsigned int input;
  unsigned int output;
};

struct SBinaryIntOp {
  int input1;
  int input2;
  int output1;
  int output2;
};

struct STertiaryIntOp {
  int input1;
  int input2;
  int input3;
  int output;
};

struct SBinaryUintOp {
  unsigned int input1;
  unsigned int input2;
  unsigned int output1;
  unsigned int output2;
};

struct STertiaryUintOp {
  unsigned int input1;
  unsigned int input2;
  unsigned int input3;
  unsigned int output;
};

struct SUnaryInt16Op {
  short input;
  short output;
};

struct SUnaryUint16Op {
  unsigned short input;
  unsigned short output;
};

struct SBinaryInt16Op {
  short input1;
  short input2;
  short output1;
  short output2;
};

struct STertiaryInt16Op {
  short input1;
  short input2;
  short input3;
  short output;
};

struct SBinaryUint16Op {
  unsigned short input1;
  unsigned short input2;
  unsigned short output1;
  unsigned short output2;
};

struct STertiaryUint16Op {
  unsigned short input1;
  unsigned short input2;
  unsigned short input3;
  unsigned short output;
};

namespace WMMA {
#define MEM_TYPE_NAMES(XMAC)                                                   \
  XMAC(BUFFER)                                                                 \
  XMAC(GROUPSHARED)

#define FRAGMENT_OP_NAMES(XMAC)                                                \
  XMAC(LEFT_COL_SUMACCUMULATE)                                                 \
  XMAC(RIGHT_ROW_SUMACCUMULATE)

#define MATH_OP_NAMES(XMAC)                                                    \
  XMAC(MULTIPLY)                                                               \
  XMAC(MULTIPLY_ACCUMULATE)                                                    \
  XMAC(ADD_MATRIX)                                                             \
  XMAC(BROADCAST_ADD_LEFT_COL)                                                 \
  XMAC(BROADCAST_ADD_RIGHT_ROW)

#define LOAD_STORE_OP_NAMES(XMAC)                                              \
  XMAC(LOAD_LEFT_START)                                                        \
  XMAC(LOAD_RIGHT_START)                                                       \
  XMAC(LOAD_LEFT_STRIDE_P4)                                                    \
  XMAC(LOAD_RIGHT_STRIDE_P4)                                                   \
  XMAC(LOAD_LEFT_STRIDE_X2)                                                    \
  XMAC(LOAD_RIGHT_STRIDE_X2)                                                   \
  XMAC(LOAD_LEFT_ALIGNMENT)                                                    \
  XMAC(LOAD_RIGHT_ALIGNMENT)                                                   \
  XMAC(LOAD_LEFT_TRANSPOSE)                                                    \
  XMAC(LOAD_RIGHT_TRANSPOSE)                                                   \
  XMAC(LOAD_LEFT_ALLPARAMS)                                                    \
  XMAC(LOAD_RIGHT_ALLPARAMS)                                                   \
  XMAC(STORE_LEFT_STRIDE_P4)                                                   \
  XMAC(STORE_RIGHT_STRIDE_P4)                                                  \
  XMAC(STORE_LEFT_STRIDE_X2)                                                   \
  XMAC(STORE_RIGHT_STRIDE_X2)                                                  \
  XMAC(STORE_LEFT_ALIGNMENT)                                                   \
  XMAC(STORE_RIGHT_ALIGNMENT)                                                  \
  XMAC(STORE_LEFT_TRANSPOSE)                                                   \
  XMAC(STORE_RIGHT_TRANSPOSE)                                                  \
  XMAC(STORE_LEFT_ALLPARAMS)                                                   \
  XMAC(STORE_RIGHT_ALLPARAMS)

#define LOAD_STORE_ACCUM_OP_NAMES(XMAC)                                        \
  XMAC(LOAD_START)                                                             \
  XMAC(LOAD_STRIDE_P4)                                                         \
  XMAC(LOAD_STRIDE_X2)                                                         \
  XMAC(LOAD_ALIGNMENT)                                                         \
  XMAC(LOAD_TRANSPOSE)                                                         \
  XMAC(LOAD_ALLPARAMS)                                                         \
  XMAC(STORE_STRIDE_P4)                                                        \
  XMAC(STORE_STRIDE_X2)                                                        \
  XMAC(STORE_ALIGNMENT)                                                        \
  XMAC(STORE_TRANSPOSE)                                                        \
  XMAC(STORE_ALLPARAMS)

#define SCALAR_OP_NAMES(XMAC)                                                  \
  XMAC(SCALAR_MUL)                                                             \
  XMAC(SCALAR_DIV)                                                             \
  XMAC(SCALAR_ADD)                                                             \
  XMAC(SCALAR_SUB)                                                             \
  XMAC(SCALAR_FILL)

#define XMACENUM(name) name,
#define XMACSTRING(name) #name,

enum : int { MEM_TYPE_NAMES(XMACENUM) NUM_MEM_TYPES };

enum : int { FRAGMENT_OP_NAMES(XMACENUM) NUM_ROWCOL_OPS };

enum : int { MATH_OP_NAMES(XMACENUM) NUM_MATRIX_OPS };

enum : int { LOAD_STORE_OP_NAMES(XMACENUM) TOTAL_LOAD_STORE_OUTPUTS };

enum : int {
  LOAD_STORE_ACCUM_OP_NAMES(XMACENUM) TOTAL_ACCUM_LOAD_STORE_OUTPUTS
};

enum : int { SCALAR_OP_NAMES(XMACENUM) SCALAR_NUM_OUTPUTS };

const char *memTypeStrs[2] = {MEM_TYPE_NAMES(XMACSTRING)};

const char *rowColEnumStrs[NUM_ROWCOL_OPS] = {FRAGMENT_OP_NAMES(XMACSTRING)};

const char *mathOpEnumStrs[NUM_MATRIX_OPS] = {MATH_OP_NAMES(XMACSTRING)};

const char *loadStoreEnumStrs[TOTAL_LOAD_STORE_OUTPUTS] = {
    LOAD_STORE_OP_NAMES(XMACSTRING)};

const char *accumLoadStoreEnumStrs[TOTAL_ACCUM_LOAD_STORE_OUTPUTS] = {
    LOAD_STORE_ACCUM_OP_NAMES(XMACSTRING)};

const char *scalarEnumStrs[SCALAR_NUM_OUTPUTS] = {SCALAR_OP_NAMES(XMACSTRING)};

#undef MATH_OP_NAMES
#undef LOAD_STORE_OP_NAMES
#undef LOAD_STORE_ACCUM_OP_NAMES
#undef SCALAR_OP_NAMES
#undef XMACENUM
#undef XMACSTRING
} // namespace WMMA

// representation for HLSL float vectors
struct SDotOp {
  XMFLOAT4 input1;
  XMFLOAT4 input2;
  float o_dot2;
  float o_dot3;
  float o_dot4;
};

struct Half2 {
  uint16_t x;
  uint16_t y;

  Half2() = default;

  Half2(const Half2 &) = default;
  Half2 &operator=(const Half2 &) = default;

  Half2(Half2 &&) = default;
  Half2 &operator=(Half2 &&) = default;

  constexpr Half2(uint16_t _x, uint16_t _y) : x(_x), y(_y) {}
  explicit Half2(const uint16_t *pArray) : x(pArray[0]), y(pArray[1]) {}
};

struct SDot2AddHalfOp {
  Half2 input1;
  Half2 input2;
  float acc;
  float result;
};

struct SDot4AddI8PackedOp {
  uint32_t input1;
  uint32_t input2;
  int32_t acc;
  int32_t result;
};

struct SDot4AddU8PackedOp {
  uint32_t input1;
  uint32_t input2;
  uint32_t acc;
  uint32_t result;
};

struct SMsad4 {
  unsigned int ref;
  XMUINT2 src;
  XMUINT4 accum;
  XMUINT4 result;
};

struct SPackUnpackOpOutPacked {
  uint32_t packedUint32;
  uint32_t packedInt32;
  uint32_t packedUint16;
  uint32_t packedInt16;

  uint32_t packedClampedUint32;
  uint32_t packedClampedInt32;
  uint32_t packedClampedUint16;
  uint32_t packedClampedInt16;
};

struct SPackUnpackOpOutUnpacked {
  std::array<uint32_t, 4> outputUint32;
  std::array<int32_t, 4> outputInt32;
  std::array<uint16_t, 4> outputUint16;
  std::array<int16_t, 4> outputInt16;

  std::array<uint32_t, 4> outputClampedUint32;
  std::array<int32_t, 4> outputClampedInt32;
  std::array<uint16_t, 4> outputClampedUint16;
  std::array<int16_t, 4> outputClampedInt16;
};

// Parameter representation for taef data-driven tests
struct TableParameter {
  LPCWSTR m_name;
  enum TableParameterType {
    INT8,
    INT16,
    INT32,
    UINT,
    FLOAT,
    HALF,
    DOUBLE,
    STRING,
    BOOL,
    INT8_TABLE,
    INT16_TABLE,
    INT32_TABLE,
    FLOAT_TABLE,
    HALF_TABLE,
    DOUBLE_TABLE,
    STRING_TABLE,
    UINT8_TABLE,
    UINT16_TABLE,
    UINT32_TABLE,
    BOOL_TABLE
  };
  TableParameter(LPCWSTR name, TableParameterType type, bool required)
      : m_name(name), m_type(type), m_required(required) {}
  TableParameterType m_type;
  bool m_required; // required parameter
  int8_t m_int8;
  int16_t m_int16;
  int m_int32;
  unsigned int m_uint;
  float m_float;
  uint16_t m_half; // no such thing as half type in c++. Use int16 instead
  double m_double;
  bool m_bool;
  WEX::Common::String m_str;
  std::vector<int8_t> m_int8Table;
  std::vector<int16_t> m_int16Table;
  std::vector<int> m_int32Table;
  std::vector<uint8_t> m_uint8Table;
  std::vector<uint16_t> m_uint16Table;
  std::vector<unsigned int> m_uint32Table;
  std::vector<float> m_floatTable;
  std::vector<uint16_t> m_halfTable; // no such thing as half type in c++
  std::vector<double> m_doubleTable;
  std::vector<bool> m_boolTable;
  std::vector<WEX::Common::String> m_StringTable;
};

class TableParameterHandler {
private:
  HRESULT ParseTableRow();

public:
  TableParameter *m_table;
  size_t m_tableSize;
  TableParameterHandler(TableParameter *pTable, size_t size)
      : m_table(pTable), m_tableSize(size) {
    clearTableParameter();
    VERIFY_SUCCEEDED(ParseTableRow());
  }

  TableParameter *GetTableParamByName(LPCWSTR name) {
    for (size_t i = 0; i < m_tableSize; ++i) {
      if (_wcsicmp(name, m_table[i].m_name) == 0) {
        return &m_table[i];
      }
    }
    DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
    return nullptr;
  }

  void clearTableParameter() {
    for (size_t i = 0; i < m_tableSize; ++i) {
      m_table[i].m_int32 = 0;
      m_table[i].m_uint = 0;
      m_table[i].m_double = 0;
      m_table[i].m_bool = false;
      m_table[i].m_str = WEX::Common::String();
    }
  }

  template <class T1> std::vector<T1> *GetDataArray(LPCWSTR name) {
    return nullptr;
  }

  template <> std::vector<int> *GetDataArray(LPCWSTR name) {
    for (size_t i = 0; i < m_tableSize; ++i) {
      if (_wcsicmp(name, m_table[i].m_name) == 0) {
        return &(m_table[i].m_int32Table);
      }
    }
    DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
    return nullptr;
  }

  template <> std::vector<int8_t> *GetDataArray(LPCWSTR name) {
    for (size_t i = 0; i < m_tableSize; ++i) {
      if (_wcsicmp(name, m_table[i].m_name) == 0) {
        return &(m_table[i].m_int8Table);
      }
    }
    DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
    return nullptr;
  }

  template <> std::vector<int16_t> *GetDataArray(LPCWSTR name) {
    for (size_t i = 0; i < m_tableSize; ++i) {
      if (_wcsicmp(name, m_table[i].m_name) == 0) {
        return &(m_table[i].m_int16Table);
      }
    }
    DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
    return nullptr;
  }

  template <> std::vector<unsigned int> *GetDataArray(LPCWSTR name) {
    for (size_t i = 0; i < m_tableSize; ++i) {
      if (_wcsicmp(name, m_table[i].m_name) == 0) {
        return &(m_table[i].m_uint32Table);
      }
    }
    DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
    return nullptr;
  }

  template <> std::vector<float> *GetDataArray(LPCWSTR name) {
    for (size_t i = 0; i < m_tableSize; ++i) {
      if (_wcsicmp(name, m_table[i].m_name) == 0) {
        return &(m_table[i].m_floatTable);
      }
    }
    DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
    return nullptr;
  }

  // TODO: uin16_t may be used to represent two different types when we
  // introduce uint16
  template <> std::vector<uint16_t> *GetDataArray(LPCWSTR name) {
    for (size_t i = 0; i < m_tableSize; ++i) {
      if (_wcsicmp(name, m_table[i].m_name) == 0) {
        return &(m_table[i].m_halfTable);
      }
    }
    DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
    return nullptr;
  }

  template <> std::vector<double> *GetDataArray(LPCWSTR name) {
    for (size_t i = 0; i < m_tableSize; ++i) {
      if (_wcsicmp(name, m_table[i].m_name) == 0) {
        return &(m_table[i].m_doubleTable);
      }
    }
    DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
    return nullptr;
  }

  template <> std::vector<bool> *GetDataArray(LPCWSTR name) {
    for (size_t i = 0; i < m_tableSize; ++i) {
      if (_wcsicmp(name, m_table[i].m_name) == 0) {
        return &(m_table[i].m_boolTable);
      }
    }
    DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
    return nullptr;
  }
};

static TableParameter UnaryFPOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::FLOAT_TABLE, true},
    {L"Validation.Expected1", TableParameter::FLOAT_TABLE, true},
    {L"Validation.Type", TableParameter::STRING, true},
    {L"Validation.Tolerance", TableParameter::DOUBLE, true},
    {L"Warp.Version", TableParameter::UINT, false}};

static TableParameter BinaryFPOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::FLOAT_TABLE, true},
    {L"Validation.Input2", TableParameter::FLOAT_TABLE, true},
    {L"Validation.Expected1", TableParameter::FLOAT_TABLE, true},
    {L"Validation.Expected2", TableParameter::FLOAT_TABLE, false},
    {L"Validation.Type", TableParameter::STRING, true},
    {L"Validation.Tolerance", TableParameter::DOUBLE, true},
};

static TableParameter TertiaryFPOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::FLOAT_TABLE, true},
    {L"Validation.Input2", TableParameter::FLOAT_TABLE, true},
    {L"Validation.Input3", TableParameter::FLOAT_TABLE, true},
    {L"Validation.Expected1", TableParameter::FLOAT_TABLE, true},
    {L"Validation.Type", TableParameter::STRING, true},
    {L"Validation.Tolerance", TableParameter::DOUBLE, true},
};

static TableParameter UnaryHalfOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"ShaderOp.Arguments", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::HALF_TABLE, true},
    {L"Validation.Expected1", TableParameter::HALF_TABLE, true},
    {L"Validation.Type", TableParameter::STRING, true},
    {L"Validation.Tolerance", TableParameter::DOUBLE, true},
    {L"Warp.Version", TableParameter::UINT, false}};

static TableParameter BinaryHalfOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"ShaderOp.Arguments", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::HALF_TABLE, true},
    {L"Validation.Input2", TableParameter::HALF_TABLE, true},
    {L"Validation.Expected1", TableParameter::HALF_TABLE, true},
    {L"Validation.Expected2", TableParameter::HALF_TABLE, false},
    {L"Validation.Type", TableParameter::STRING, true},
    {L"Validation.Tolerance", TableParameter::DOUBLE, true},
};

static TableParameter TertiaryHalfOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"ShaderOp.Arguments", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::HALF_TABLE, true},
    {L"Validation.Input2", TableParameter::HALF_TABLE, true},
    {L"Validation.Input3", TableParameter::HALF_TABLE, true},
    {L"Validation.Expected1", TableParameter::HALF_TABLE, true},
    {L"Validation.Type", TableParameter::STRING, true},
    {L"Validation.Tolerance", TableParameter::DOUBLE, true},
};

static TableParameter UnaryIntOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::INT32_TABLE, true},
    {L"Validation.Expected1", TableParameter::INT32_TABLE, true},
    {L"Validation.Tolerance", TableParameter::INT32, true},
};

static TableParameter UnaryUintOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::UINT32_TABLE, true},
    {L"Validation.Expected1", TableParameter::UINT32_TABLE, true},
    {L"Validation.Tolerance", TableParameter::INT32, true},
};

static TableParameter BinaryIntOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::INT32_TABLE, true},
    {L"Validation.Input2", TableParameter::INT32_TABLE, true},
    {L"Validation.Expected1", TableParameter::INT32_TABLE, true},
    {L"Validation.Expected2", TableParameter::INT32_TABLE, false},
    {L"Validation.Tolerance", TableParameter::INT32, true},
};

static TableParameter TertiaryIntOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::INT32_TABLE, true},
    {L"Validation.Input2", TableParameter::INT32_TABLE, true},
    {L"Validation.Input3", TableParameter::INT32_TABLE, true},
    {L"Validation.Expected1", TableParameter::INT32_TABLE, true},
    {L"Validation.Tolerance", TableParameter::INT32, true},
};

static TableParameter BinaryUintOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::UINT32_TABLE, true},
    {L"Validation.Input2", TableParameter::UINT32_TABLE, true},
    {L"Validation.Expected1", TableParameter::UINT32_TABLE, true},
    {L"Validation.Expected2", TableParameter::UINT32_TABLE, false},
    {L"Validation.Tolerance", TableParameter::INT32, true},
};

static TableParameter TertiaryUintOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::UINT32_TABLE, true},
    {L"Validation.Input2", TableParameter::UINT32_TABLE, true},
    {L"Validation.Input3", TableParameter::UINT32_TABLE, true},
    {L"Validation.Expected1", TableParameter::UINT32_TABLE, true},
    {L"Validation.Tolerance", TableParameter::INT32, true},
};

static TableParameter UnaryInt16OpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"ShaderOp.Arguments", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::INT16_TABLE, true},
    {L"Validation.Expected1", TableParameter::INT16_TABLE, true},
    {L"Validation.Tolerance", TableParameter::INT32, true},
};

static TableParameter UnaryUint16OpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"ShaderOp.Arguments", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::UINT16_TABLE, true},
    {L"Validation.Expected1", TableParameter::UINT16_TABLE, true},
    {L"Validation.Tolerance", TableParameter::INT32, true},
};

static TableParameter BinaryInt16OpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"ShaderOp.Arguments", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::INT16_TABLE, true},
    {L"Validation.Input2", TableParameter::INT16_TABLE, true},
    {L"Validation.Expected1", TableParameter::INT16_TABLE, true},
    {L"Validation.Expected2", TableParameter::INT16_TABLE, false},
    {L"Validation.Tolerance", TableParameter::INT32, true},
};

static TableParameter TertiaryInt16OpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"ShaderOp.Arguments", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::INT16_TABLE, true},
    {L"Validation.Input2", TableParameter::INT16_TABLE, true},
    {L"Validation.Input3", TableParameter::INT16_TABLE, true},
    {L"Validation.Expected1", TableParameter::INT16_TABLE, true},
    {L"Validation.Tolerance", TableParameter::INT32, true},
};

static TableParameter BinaryUint16OpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"ShaderOp.Arguments", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::UINT16_TABLE, true},
    {L"Validation.Input2", TableParameter::UINT16_TABLE, true},
    {L"Validation.Expected1", TableParameter::UINT16_TABLE, true},
    {L"Validation.Expected2", TableParameter::UINT16_TABLE, false},
    {L"Validation.Tolerance", TableParameter::INT32, true},
};

static TableParameter TertiaryUint16OpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"ShaderOp.Arguments", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::UINT16_TABLE, true},
    {L"Validation.Input2", TableParameter::UINT16_TABLE, true},
    {L"Validation.Input3", TableParameter::UINT16_TABLE, true},
    {L"Validation.Expected1", TableParameter::UINT16_TABLE, true},
    {L"Validation.Tolerance", TableParameter::INT32, true},
};

static TableParameter DotOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::STRING_TABLE, true},
    {L"Validation.Input2", TableParameter::STRING_TABLE, true},
    {L"Validation.Expected1", TableParameter::STRING_TABLE, true},
    {L"Validation.Expected2", TableParameter::STRING_TABLE, true},
    {L"Validation.Expected3", TableParameter::STRING_TABLE, true},
    {L"Validation.Type", TableParameter::STRING, true},
    {L"Validation.Tolerance", TableParameter::DOUBLE, true},
};

static TableParameter Dot2AddHalfOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"ShaderOp.Arguments", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::STRING_TABLE, true},
    {L"Validation.Input2", TableParameter::STRING_TABLE, true},
    {L"Validation.Input3", TableParameter::FLOAT_TABLE, true},
    {L"Validation.Expected1", TableParameter::FLOAT_TABLE, true},
    {L"Validation.Type", TableParameter::STRING, true},
    {L"Validation.Tolerance", TableParameter::DOUBLE, true},
};

static TableParameter Dot4AddI8PackedOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::UINT32_TABLE, true},
    {L"Validation.Input2", TableParameter::UINT32_TABLE, true},
    {L"Validation.Input3", TableParameter::INT32_TABLE, true},
    {L"Validation.Expected1", TableParameter::INT32_TABLE, true},
};

static TableParameter Dot4AddU8PackedOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::UINT32_TABLE, true},
    {L"Validation.Input2", TableParameter::UINT32_TABLE, true},
    {L"Validation.Input3", TableParameter::UINT32_TABLE, true},
    {L"Validation.Expected1", TableParameter::UINT32_TABLE, true},
};

static TableParameter Msad4OpParameters[] = {
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.Tolerance", TableParameter::DOUBLE, true},
    {L"Validation.Input1", TableParameter::UINT32_TABLE, true},
    {L"Validation.Input2", TableParameter::STRING_TABLE, true},
    {L"Validation.Input3", TableParameter::STRING_TABLE, true},
    {L"Validation.Expected1", TableParameter::STRING_TABLE, true}};

static TableParameter WaveIntrinsicsActiveIntParameters[] = {
    {L"ShaderOp.Name", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.NumInputSet", TableParameter::UINT, true},
    {L"Validation.InputSet1", TableParameter::INT32_TABLE, true},
    {L"Validation.InputSet2", TableParameter::INT32_TABLE, false},
    {L"Validation.InputSet3", TableParameter::INT32_TABLE, false},
    {L"Validation.InputSet4", TableParameter::INT32_TABLE, false}};

static TableParameter WaveIntrinsicsPrefixIntParameters[] = {
    {L"ShaderOp.Name", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.NumInputSet", TableParameter::UINT, true},
    {L"Validation.InputSet1", TableParameter::INT32_TABLE, true},
    {L"Validation.InputSet2", TableParameter::INT32_TABLE, false},
    {L"Validation.InputSet3", TableParameter::INT32_TABLE, false},
    {L"Validation.InputSet4", TableParameter::INT32_TABLE, false}};

static TableParameter WaveIntrinsicsActiveUintParameters[] = {
    {L"ShaderOp.Name", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.NumInputSet", TableParameter::UINT, true},
    {L"Validation.InputSet1", TableParameter::UINT32_TABLE, true},
    {L"Validation.InputSet2", TableParameter::UINT32_TABLE, false},
    {L"Validation.InputSet3", TableParameter::UINT32_TABLE, false},
    {L"Validation.InputSet4", TableParameter::UINT32_TABLE, false}};

static TableParameter WaveIntrinsicsPrefixUintParameters[] = {
    {L"ShaderOp.Name", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.NumInputSet", TableParameter::UINT, true},
    {L"Validation.InputSet1", TableParameter::UINT32_TABLE, true},
    {L"Validation.InputSet2", TableParameter::UINT32_TABLE, false},
    {L"Validation.InputSet3", TableParameter::UINT32_TABLE, false},
    {L"Validation.InputSet4", TableParameter::UINT32_TABLE, false}};

static TableParameter WaveIntrinsicsMultiPrefixIntParameters[] = {
    {L"ShaderOp.Name", TableParameter::STRING, true},
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.Keys", TableParameter::INT32_TABLE, true},
    {L"Validation.Values", TableParameter::INT32_TABLE, true},
};

static TableParameter WaveIntrinsicsMultiPrefixUintParameters[] = {
    {L"ShaderOp.Name", TableParameter::STRING, true},
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.Keys", TableParameter::UINT32_TABLE, true},
    {L"Validation.Values", TableParameter::UINT32_TABLE, true},
};

static TableParameter WaveIntrinsicsActiveBoolParameters[] = {
    {L"ShaderOp.Name", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.NumInputSet", TableParameter::UINT, true},
    {L"Validation.InputSet1", TableParameter::BOOL_TABLE, true},
    {L"Validation.InputSet2", TableParameter::BOOL_TABLE, false},
    {L"Validation.InputSet3", TableParameter::BOOL_TABLE, false},
};

static TableParameter CBufferTestHalfParameters[] = {
    {L"Validation.InputSet", TableParameter::HALF_TABLE, true},
};

static TableParameter DenormBinaryFPOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"ShaderOp.Arguments", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::STRING_TABLE, true},
    {L"Validation.Input2", TableParameter::STRING_TABLE, true},
    {L"Validation.Expected1", TableParameter::STRING_TABLE, true},
    {L"Validation.Expected2", TableParameter::STRING_TABLE, false},
    {L"Validation.Type", TableParameter::STRING, true},
    {L"Validation.Tolerance", TableParameter::DOUBLE, true},
};

static TableParameter DenormTertiaryFPOpParameters[] = {
    {L"ShaderOp.Target", TableParameter::STRING, true},
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"ShaderOp.Arguments", TableParameter::STRING, true},
    {L"Validation.Input1", TableParameter::STRING_TABLE, true},
    {L"Validation.Input2", TableParameter::STRING_TABLE, true},
    {L"Validation.Input3", TableParameter::STRING_TABLE, true},
    {L"Validation.Expected1", TableParameter::STRING_TABLE, true},
    {L"Validation.Expected2", TableParameter::STRING_TABLE, false},
    {L"Validation.Type", TableParameter::STRING, true},
    {L"Validation.Tolerance", TableParameter::DOUBLE, true},
};

static TableParameter PackUnpackOpParameters[] = {
    {L"ShaderOp.Text", TableParameter::STRING, true},
    {L"Validation.Type", TableParameter::STRING, true},
    {L"Validation.Tolerance", TableParameter::UINT, true},
    {L"Validation.Input", TableParameter::UINT32_TABLE, true},
};

static bool IsHexString(PCWSTR str, uint16_t *value) {
  std::wstring wString(str);
  wString.erase(std::remove(wString.begin(), wString.end(), L' '),
                wString.end());
  LPCWSTR wstr = wString.c_str();
  if (wcsncmp(wstr, L"0x", 2) == 0 || wcsncmp(wstr, L"0b", 2) == 0) {
    *value = (uint16_t)wcstol(wstr, NULL, 0);
    return true;
  }
  return false;
}

static HRESULT ParseDataToFloat(PCWSTR str, float &value) {
  std::wstring wString(str);
  wString.erase(std::remove(wString.begin(), wString.end(), L' '),
                wString.end());
  wString.erase(std::remove(wString.begin(), wString.end(), L'\n'),
                wString.end());
  PCWSTR wstr = wString.data();
  if (_wcsicmp(wstr, L"NaN") == 0) {
    value = NAN;
  } else if (_wcsicmp(wstr, L"-inf") == 0) {
    value = -(INFINITY);
  } else if (_wcsicmp(wstr, L"inf") == 0) {
    value = INFINITY;
  } else if (_wcsicmp(wstr, L"-denorm") == 0) {
    value = -(FLT_MIN / 2);
  } else if (_wcsicmp(wstr, L"denorm") == 0) {
    value = FLT_MIN / 2;
  } else if (_wcsicmp(wstr, L"-0.0f") == 0 || _wcsicmp(wstr, L"-0.0") == 0 ||
             _wcsicmp(wstr, L"-0") == 0) {
    value = -0.0f;
  } else if (_wcsicmp(wstr, L"0.0f") == 0 || _wcsicmp(wstr, L"0.0") == 0 ||
             _wcsicmp(wstr, L"0") == 0) {
    value = 0.0f;
  } else if (_wcsnicmp(wstr, L"0x", 2) ==
             0) { // For hex values, take values literally
    unsigned temp_i = std::stoul(wstr, nullptr, 16);
    value = (float &)temp_i;
  } else {
    // evaluate the expression of wstring
    double val = _wtof(wstr);
    if (val == 0) {
      LogErrorFmt(L"Failed to parse parameter %s to float", wstr);
      return E_FAIL;
    }
    value = (float)val;
  }
  return S_OK;
}

static HRESULT ParseDataToUint(PCWSTR str, unsigned int &value) {
  std::wstring wString(str);
  wString.erase(std::remove(wString.begin(), wString.end(), L' '),
                wString.end());
  PCWSTR wstr = wString.data();
  // evaluate the expression of string
  if (_wcsicmp(wstr, L"0") == 0 || _wcsicmp(wstr, L"0x00000000") == 0) {
    value = 0;
    return S_OK;
  }
  wchar_t *end;
  unsigned int val = std::wcstoul(wstr, &end, 0);
  if (val == 0) {
    LogErrorFmt(L"Failed to parse parameter %s to int", wstr);
    return E_FAIL;
  }
  value = val;
  return S_OK;
}

static HRESULT ParseDataToVectorFloat(PCWSTR str, float *ptr, size_t count) {
  std::wstring wstr(str);
  size_t curPosition = 0;
  // parse a string of dot product separated by commas
  for (size_t i = 0; i < count; ++i) {
    size_t nextPosition = wstr.find(L",", curPosition);
    if (FAILED(ParseDataToFloat(
            wstr.substr(curPosition, nextPosition - curPosition).data(),
            *(ptr + i)))) {
      return E_FAIL;
    }
    curPosition = nextPosition + 1;
  }
  return S_OK;
}

static HRESULT ParseDataToVectorHalf(PCWSTR str, uint16_t *ptr, size_t count) {
  std::wstring wstr(str);
  size_t curPosition = 0;
  // parse a string of dot product separated by commas
  for (size_t i = 0; i < count; ++i) {
    size_t nextPosition = wstr.find(L",", curPosition);
    float floatValue;
    if (FAILED(ParseDataToFloat(
            wstr.substr(curPosition, nextPosition - curPosition).data(),
            floatValue))) {
      return E_FAIL;
    }
    *(ptr + i) = ConvertFloat32ToFloat16(floatValue);
    curPosition = nextPosition + 1;
  }
  return S_OK;
}

static HRESULT ParseDataToVectorUint(PCWSTR str, unsigned int *ptr,
                                     size_t count) {
  std::wstring wstr(str);
  size_t curPosition = 0;
  // parse a string of dot product separated by commas
  for (size_t i = 0; i < count; ++i) {
    size_t nextPosition = wstr.find(L",", curPosition);
    if (FAILED(ParseDataToUint(
            wstr.substr(curPosition, nextPosition - curPosition).data(),
            *(ptr + i)))) {
      return E_FAIL;
    }
    curPosition = nextPosition + 1;
  }
  return S_OK;
}

HRESULT TableParameterHandler::ParseTableRow() {
  TableParameter *table = m_table;
  for (unsigned int i = 0; i < m_tableSize; ++i) {
    switch (table[i].m_type) {
    case TableParameter::INT8:
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           table[i].m_int32)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int16
        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_int8 = (int8_t)(table[i].m_int32);
      break;
    case TableParameter::INT16:
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           table[i].m_int32)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int16
        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_int16 = (short)(table[i].m_int32);
      break;
    case TableParameter::INT32:
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           table[i].m_int32)) &&
          table[i].m_required) {
        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      break;
    case TableParameter::UINT:
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           table[i].m_uint)) &&
          table[i].m_required) {
        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      break;
    case TableParameter::DOUBLE:
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(
              table[i].m_name, table[i].m_double)) &&
          table[i].m_required) {
        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      break;
    case TableParameter::STRING:
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           table[i].m_str)) &&
          table[i].m_required) {
        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      break;
    case TableParameter::BOOL:
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           table[i].m_str)) &&
          table[i].m_bool) {
        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      break;
    case TableParameter::INT8_TABLE: {
      WEX::TestExecution::TestDataArray<int> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {

        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      // TryGetValue does not suppport reading from int8
      table[i].m_int8Table.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_int8Table[j] = (int8_t)tempTable[j];
      }
      break;
    }
    case TableParameter::INT16_TABLE: {
      WEX::TestExecution::TestDataArray<int> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      // TryGetValue does not suppport reading from int8
      table[i].m_int16Table.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_int16Table[j] = (int16_t)tempTable[j];
      }
      break;
    }
    case TableParameter::INT32_TABLE: {
      WEX::TestExecution::TestDataArray<int> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int8
        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_int32Table.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_int32Table[j] = tempTable[j];
      }
      break;
    }
    case TableParameter::UINT8_TABLE: {
      WEX::TestExecution::TestDataArray<int> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {

        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      // TryGetValue does not suppport reading from int8
      table[i].m_int8Table.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_int8Table[j] = (uint8_t)tempTable[j];
      }
      break;
    }
    case TableParameter::UINT16_TABLE: {
      WEX::TestExecution::TestDataArray<int> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      // TryGetValue does not suppport reading from int8
      table[i].m_uint16Table.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_uint16Table[j] = (uint16_t)tempTable[j];
      }
      break;
    }
    case TableParameter::UINT32_TABLE: {
      WEX::TestExecution::TestDataArray<unsigned int> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int8
        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_uint32Table.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_uint32Table[j] = tempTable[j];
      }
      break;
    }
    case TableParameter::FLOAT_TABLE: {
      WEX::TestExecution::TestDataArray<WEX::Common::String> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int8
        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_floatTable.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        ParseDataToFloat(tempTable[j], table[i].m_floatTable[j]);
      }
      break;
    }
    case TableParameter::HALF_TABLE: {
      WEX::TestExecution::TestDataArray<WEX::Common::String> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int8
        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_halfTable.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        uint16_t value = 0;
        if (IsHexString(tempTable[j], &value)) {
          table[i].m_halfTable[j] = value;
        } else {
          float val;
          ParseDataToFloat(tempTable[j], val);
          if (isdenorm(val))
            table[i].m_halfTable[j] =
                signbit(val) ? Float16NegDenorm : Float16PosDenorm;
          else
            table[i].m_halfTable[j] = ConvertFloat32ToFloat16(val);
        }
      }
      break;
    }
    case TableParameter::DOUBLE_TABLE: {
      WEX::TestExecution::TestDataArray<double> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int8
        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_doubleTable.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_doubleTable[j] = tempTable[j];
      }
      break;
    }
    case TableParameter::BOOL_TABLE: {
      WEX::TestExecution::TestDataArray<bool> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int8
        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_boolTable.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_boolTable[j] = tempTable[j];
      }
      break;
    }
    case TableParameter::STRING_TABLE: {
      WEX::TestExecution::TestDataArray<WEX::Common::String> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int8
        LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_StringTable.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_StringTable[j] = tempTable[j];
      }
      break;
    }
    default:
      DXASSERT_NOMSG("Invalid Parameter Type");
    }
    if (errno == ERANGE) {
      LogErrorFmt(L"got out of range value for table %s", table[i].m_name);
      return E_FAIL;
    }
  }
  return S_OK;
}

static bool CompareOutputWithExpectedValueInt(int output, int ref,
                                              int tolerance) {
  return ((output - ref) <= tolerance) && ((ref - output) <= tolerance);
}

static bool VerifyOutputWithExpectedValueInt(int output, int ref,
                                             int tolerance) {
  return VERIFY_IS_TRUE(
      CompareOutputWithExpectedValueInt(output, ref, tolerance));
}

static bool CompareOutputWithExpectedValueUInt(uint32_t output, uint32_t ref,
                                               uint32_t tolerance) {
  if (output > ref)
    return output - ref <= tolerance;
  else
    return ref - output <= tolerance;
}

static bool VerifyOutputWithExpectedValueUInt(uint32_t output, uint32_t ref,
                                              uint32_t tolerance) {
  return VERIFY_IS_TRUE(
      CompareOutputWithExpectedValueUInt(output, ref, tolerance));
}

enum class ToleranceType {
  EPSILON,
  RELATIVE_EPSILON,
  ULP,
  TOLERANCE_TYPE_COUNT
};

ToleranceType ToleranceStringToEnum(LPCWSTR toleranceType) {
  if (_wcsicmp(toleranceType, L"Relative") == 0) {
    return ToleranceType::RELATIVE_EPSILON;
  } else if (_wcsicmp(toleranceType, L"Epsilon") == 0) {
    return ToleranceType::EPSILON;
  } else if (_wcsicmp(toleranceType, L"ULP") == 0) {
    return ToleranceType::ULP;
  } else {
    LogErrorFmt(L"Failed to read comparison type %S", toleranceType);
    return ToleranceType::TOLERANCE_TYPE_COUNT;
  }
}

static bool CompareOutputWithExpectedValueFloat(
    float output, float ref, LPCWSTR type, double tolerance,
    hlsl::DXIL::Float32DenormMode mode = hlsl::DXIL::Float32DenormMode::Any) {
  if (_wcsicmp(type, L"Relative") == 0) {
    return CompareFloatRelativeEpsilon(output, ref, (int)tolerance, mode);
  } else if (_wcsicmp(type, L"Epsilon") == 0) {
    return CompareFloatEpsilon(output, ref, (float)tolerance, mode);
  } else if (_wcsicmp(type, L"ULP") == 0) {
    return CompareFloatULP(output, ref, (int)tolerance, mode);
  } else {
    LogErrorFmt(L"Failed to read comparison type %S", type);
  }

  return false;
}

static bool VerifyOutputWithExpectedValueFloat(
    float output, float ref, LPCWSTR type, double tolerance,
    hlsl::DXIL::Float32DenormMode mode = hlsl::DXIL::Float32DenormMode::Any) {
  return VERIFY_IS_TRUE(
      CompareOutputWithExpectedValueFloat(output, ref, type, tolerance, mode));
}

static bool CompareOutputWithExpectedValueHalf(uint16_t output, uint16_t ref,
                                               LPCWSTR type, double tolerance) {
  if (_wcsicmp(type, L"Relative") == 0) {
    return CompareHalfRelativeEpsilon(output, ref, (int)tolerance);
  } else if (_wcsicmp(type, L"Epsilon") == 0) {
    return CompareHalfEpsilon(output, ref, (float)tolerance);
  } else if (_wcsicmp(type, L"ULP") == 0) {
    return CompareHalfULP(output, ref, (float)tolerance);
  } else {
    LogErrorFmt(L"Failed to read comparison type %S", type);
    return false;
  }
}

static bool VerifyOutputWithExpectedValueHalf(uint16_t output, uint16_t ref,
                                              LPCWSTR type, double tolerance) {
  return VERIFY_IS_TRUE(
      CompareOutputWithExpectedValueHalf(output, ref, type, tolerance));
}

template <typename T>
static bool CompareOutputWithExpectedValue(T output, T ref,
                                           LPCWSTR toleranceType,
                                           double tolerance) {
  if (std::is_same<T, DirectX::PackedVector::HALF>::value) { // uint16 treated
                                                             // as half
    return CompareOutputWithExpectedValueHalf((uint16_t)output, (uint16_t)ref,
                                              toleranceType, tolerance);
  } else if (std::is_integral<T>::value &&
             std::is_signed<T>::value) { // signed ints
    return CompareOutputWithExpectedValueInt((int)output, (int)ref,
                                             (int)tolerance);
  } else if (std::is_integral<T>::value) { // unsigned ints
    return CompareOutputWithExpectedValueUInt((uint32_t)output, (uint32_t)ref,
                                              (uint32_t)tolerance);
  } else if (std::is_floating_point<T>::value) { // floating point
    return CompareOutputWithExpectedValueFloat((float)output, (float)ref,
                                               toleranceType, tolerance);
  }

  DXASSERT_NOMSG("Invalid Parameter Type");
}

// Check an array of values against an array of expected values. verify they
// match within the given constraints.
template <typename T>
bool VerifyArrayWithExpectedValue(T *arr, T *expected, size_t num_elements,
                                  LPCWSTR type, double tolerance) {
  ToleranceType toleranceType = ToleranceStringToEnum(type);

  bool passed = true;
  for (size_t i = 0; i < num_elements; ++i) {
    passed = CompareOutputWithExpectedValue<T>(arr[i], expected[i],
                                               toleranceType, tolerance) &&
             passed;

    if (!passed) {
      break;
    }
  }
  VERIFY_IS_TRUE(passed);

  return passed;
};

TEST_F(ExecutionTest, UnaryFloatOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice)) {
    return;
  }
  // Read data from the table
  int tableSize = sizeof(UnaryFPOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(UnaryFPOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);

  std::vector<float> *Validation_Input =
      &(handler.GetTableParamByName(L"Validation.Input1")->m_floatTable);
  std::vector<float> *Validation_Expected =
      &(handler.GetTableParamByName(L"Validation.Expected1")->m_floatTable);

  LPCWSTR Validation_Type =
      handler.GetTableParamByName(L"Validation.Type")->m_str;
  double Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_double;

  size_t count = Validation_Input->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "UnaryFPOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SUnaryFPOp"));
        size_t size = sizeof(SUnaryFPOp) * count;
        Data.resize(size);
        SUnaryFPOp *pPrimitives = (SUnaryFPOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SUnaryFPOp *p = &pPrimitives[i];
          p->input = (*Validation_Input)[i % Validation_Input->size()];
        }
        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SUnaryFPOp", &data);

  SUnaryFPOp *pPrimitives = (SUnaryFPOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  for (unsigned i = 0; i < count; ++i) {
    SUnaryFPOp *p = &pPrimitives[i];
    float val = (*Validation_Expected)[i % Validation_Expected->size()];
    LogCommentFmt(
        L"element #%u, input = %6.8f, output = %6.8f, expected = %6.8f", i,
        p->input, p->output, val);
    VerifyOutputWithExpectedValueFloat(p->output, val, Validation_Type,
                                       Validation_Tolerance);
  }
}

TEST_F(ExecutionTest, BinaryFloatOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice)) {
    return;
  }
  // Read data from the table
  int tableSize = sizeof(BinaryFPOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(BinaryFPOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);

  std::vector<float> *Validation_Input1 =
      &(handler.GetTableParamByName(L"Validation.Input1")->m_floatTable);
  std::vector<float> *Validation_Input2 =
      &(handler.GetTableParamByName(L"Validation.Input2")->m_floatTable);

  std::vector<float> *Validation_Expected1 =
      &(handler.GetTableParamByName(L"Validation.Expected1")->m_floatTable);

  std::vector<float> *Validation_Expected2 =
      &(handler.GetTableParamByName(L"Validation.Expected2")->m_floatTable);

  LPCWSTR Validation_Type =
      handler.GetTableParamByName(L"Validation.Type")->m_str;
  double Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_double;
  size_t count = Validation_Input1->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "BinaryFPOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SBinaryFPOp"));
        size_t size = sizeof(SBinaryFPOp) * count;
        Data.resize(size);
        SBinaryFPOp *pPrimitives = (SBinaryFPOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SBinaryFPOp *p = &pPrimitives[i];
          p->input1 = (*Validation_Input1)[i % Validation_Input1->size()];
          p->input2 = (*Validation_Input2)[i % Validation_Input2->size()];
        }

        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SBinaryFPOp", &data);

  SBinaryFPOp *pPrimitives = (SBinaryFPOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  unsigned numExpected = Validation_Expected2->size() == 0 ? 1 : 2;
  if (numExpected == 2) {
    for (unsigned i = 0; i < count; ++i) {
      SBinaryFPOp *p = &pPrimitives[i];
      float val1 = (*Validation_Expected1)[i % Validation_Expected1->size()];
      float val2 = (*Validation_Expected2)[i % Validation_Expected2->size()];
      LogCommentFmt(
          L"element #%u, input1 = %6.8f, input2 = %6.8f, output1 = "
          L"%6.8f, expected1 = %6.8f, output2 = %6.8f, expected2 = %6.8f",
          i, p->input1, p->input2, p->output1, val1, p->output2, val2);
      VerifyOutputWithExpectedValueFloat(p->output1, val1, Validation_Type,
                                         Validation_Tolerance);
      VerifyOutputWithExpectedValueFloat(p->output2, val2, Validation_Type,
                                         Validation_Tolerance);
    }
  } else if (numExpected == 1) {
    for (unsigned i = 0; i < count; ++i) {
      SBinaryFPOp *p = &pPrimitives[i];
      float val1 = (*Validation_Expected1)[i % Validation_Expected1->size()];
      LogCommentFmt(L"element #%u, input1 = %6.8f, input2 = %6.8f, output1 = "
                    L"%6.8f, expected1 = %6.8f",
                    i, p->input1, p->input2, p->output1, val1);
      VerifyOutputWithExpectedValueFloat(p->output1, val1, Validation_Type,
                                         Validation_Tolerance);
    }
  } else {
    LogErrorFmt(L"Unexpected number of expected values for operation %i",
                numExpected);
  }
}

TEST_F(ExecutionTest, TertiaryFloatOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice)) {
    return;
  }
  // Read data from the table

  int tableSize = sizeof(TertiaryFPOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(TertiaryFPOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);

  std::vector<float> *Validation_Input1 =
      &(handler.GetTableParamByName(L"Validation.Input1")->m_floatTable);
  std::vector<float> *Validation_Input2 =
      &(handler.GetTableParamByName(L"Validation.Input2")->m_floatTable);
  std::vector<float> *Validation_Input3 =
      &(handler.GetTableParamByName(L"Validation.Input3")->m_floatTable);

  std::vector<float> *Validation_Expected =
      &(handler.GetTableParamByName(L"Validation.Expected1")->m_floatTable);

  LPCWSTR Validation_Type =
      handler.GetTableParamByName(L"Validation.Type")->m_str;
  double Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_double;
  size_t count = Validation_Input1->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "TertiaryFPOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "STertiaryFPOp"));
        size_t size = sizeof(STertiaryFPOp) * count;
        Data.resize(size);
        STertiaryFPOp *pPrimitives = (STertiaryFPOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          STertiaryFPOp *p = &pPrimitives[i];
          p->input1 = (*Validation_Input1)[i % Validation_Input1->size()];
          p->input2 = (*Validation_Input2)[i % Validation_Input2->size()];
          p->input3 = (*Validation_Input3)[i % Validation_Input3->size()];
        }

        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("STertiaryFPOp", &data);

  STertiaryFPOp *pPrimitives = (STertiaryFPOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;

  for (unsigned i = 0; i < count; ++i) {
    STertiaryFPOp *p = &pPrimitives[i];
    float val = (*Validation_Expected)[i % Validation_Expected->size()];
    LogCommentFmt(L"element #%u, input1 = %6.8f, input2 = %6.8f, input3 = "
                  L"%6.8f, output1 = "
                  L"%6.8f, expected = %6.8f",
                  i, p->input1, p->input2, p->input3, p->output, val);
    VerifyOutputWithExpectedValueFloat(p->output, val, Validation_Type,
                                       Validation_Tolerance);
  }
}

TEST_F(ExecutionTest, UnaryHalfOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_2)) {
    return;
  }

  if (!DoesDeviceSupportNative16bitOps(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support native 16-bit operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  // Read data from the table
  int tableSize = sizeof(UnaryHalfOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(UnaryHalfOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);
  CW2A Arguments(handler.GetTableParamByName(L"ShaderOp.Arguments")->m_str);

  std::vector<uint16_t> *Validation_Input =
      &(handler.GetTableParamByName(L"Validation.Input1")->m_halfTable);
  std::vector<uint16_t> *Validation_Expected =
      &(handler.GetTableParamByName(L"Validation.Expected1")->m_halfTable);

  LPCWSTR Validation_Type =
      handler.GetTableParamByName(L"Validation.Type")->m_str;
  double Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_double;

  size_t count = Validation_Input->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "UnaryFPOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SUnaryFPOp"));
        size_t size = sizeof(SUnaryHalfOp) * count;
        Data.resize(size);
        SUnaryHalfOp *pPrimitives = (SUnaryHalfOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SUnaryHalfOp *p = &pPrimitives[i];
          p->input = (*Validation_Input)[i % Validation_Input->size()];
        }
        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
        pShaderOp->Shaders.at(0).Arguments = Arguments.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SUnaryFPOp", &data);

  SUnaryHalfOp *pPrimitives = (SUnaryHalfOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  for (unsigned i = 0; i < count; ++i) {
    SUnaryHalfOp *p = &pPrimitives[i];
    uint16_t expected = (*Validation_Expected)[i % Validation_Input->size()];
    LogCommentFmt(L"element #%u, input = %6.8f(0x%04x), output = "
                  L"%6.8f(0x%04x), expected = %6.8f(0x%04x)",
                  i, ConvertFloat16ToFloat32(p->input), p->input,
                  ConvertFloat16ToFloat32(p->output), p->output,
                  ConvertFloat16ToFloat32(expected), expected);
    VerifyOutputWithExpectedValueHalf(p->output, expected, Validation_Type,
                                      Validation_Tolerance);
  }
}

TEST_F(ExecutionTest, BinaryHalfOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_2)) {
    return;
  }

  if (!DoesDeviceSupportNative16bitOps(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support native 16-bit operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  // Read data from the table
  int tableSize = sizeof(BinaryHalfOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(BinaryHalfOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);
  CW2A Arguments(handler.GetTableParamByName(L"ShaderOp.Arguments")->m_str);

  std::vector<uint16_t> *Validation_Input1 =
      &(handler.GetTableParamByName(L"Validation.Input1")->m_halfTable);
  std::vector<uint16_t> *Validation_Input2 =
      &(handler.GetTableParamByName(L"Validation.Input2")->m_halfTable);

  std::vector<uint16_t> *Validation_Expected1 =
      &(handler.GetTableParamByName(L"Validation.Expected1")->m_halfTable);

  std::vector<uint16_t> *Validation_Expected2 =
      &(handler.GetTableParamByName(L"Validation.Expected2")->m_halfTable);

  LPCWSTR Validation_Type =
      handler.GetTableParamByName(L"Validation.Type")->m_str;
  double Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_double;
  size_t count = Validation_Input1->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "BinaryFPOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SBinaryFPOp"));
        size_t size = sizeof(SBinaryHalfOp) * count;
        Data.resize(size);
        SBinaryHalfOp *pPrimitives = (SBinaryHalfOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SBinaryHalfOp *p = &pPrimitives[i];
          p->input1 = (*Validation_Input1)[i % Validation_Input1->size()];
          p->input2 = (*Validation_Input2)[i % Validation_Input2->size()];
        }

        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
        pShaderOp->Shaders.at(0).Arguments = Arguments.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SBinaryFPOp", &data);

  SBinaryHalfOp *pPrimitives = (SBinaryHalfOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  unsigned numExpected = Validation_Expected2->size() == 0 ? 1 : 2;
  if (numExpected == 2) {
    for (unsigned i = 0; i < count; ++i) {
      SBinaryHalfOp *p = &pPrimitives[i];
      uint16_t expected1 =
          (*Validation_Expected1)[i % Validation_Input1->size()];
      uint16_t expected2 =
          (*Validation_Expected2)[i % Validation_Input2->size()];
      LogCommentFmt(L"element #%u, input1 = %6.8f(0x%04x), input2 = "
                    L"%6.8f(0x%04x), output1 = "
                    L"%6.8f(0x%04x), expected1 = %6.8f(0x%04x), output2 = "
                    L"%6.8f(0x%04x), expected2 = %6.8f(0x%04x)",
                    i, ConvertFloat16ToFloat32(p->input1), p->input1,
                    ConvertFloat16ToFloat32(p->input2), p->input2,
                    ConvertFloat16ToFloat32(p->output1), p->output1,
                    ConvertFloat16ToFloat32(p->output2), p->output2,
                    ConvertFloat16ToFloat32(expected1), expected1,
                    ConvertFloat16ToFloat32(expected2), expected2);
      VerifyOutputWithExpectedValueHalf(p->output1, expected1, Validation_Type,
                                        Validation_Tolerance);
      VerifyOutputWithExpectedValueHalf(p->output2, expected2, Validation_Type,
                                        Validation_Tolerance);
    }
  } else if (numExpected == 1) {
    for (unsigned i = 0; i < count; ++i) {
      uint16_t expected =
          (*Validation_Expected1)[i % Validation_Input1->size()];
      SBinaryHalfOp *p = &pPrimitives[i];
      LogCommentFmt(L"element #%u, input = %6.8f(0x%04x), output = "
                    L"%6.8f(0x%04x), expected = %6.8f(0x%04x)",
                    i, ConvertFloat16ToFloat32(p->input1), p->input1,
                    ConvertFloat16ToFloat32(p->output1), p->output1,
                    ConvertFloat16ToFloat32(expected), expected);
      VerifyOutputWithExpectedValueHalf(p->output1, expected, Validation_Type,
                                        Validation_Tolerance);
    }
  } else {
    LogErrorFmt(L"Unexpected number of expected values for operation %i",
                numExpected);
  }
}

TEST_F(ExecutionTest, TertiaryHalfOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_2)) {
    return;
  }

  if (!DoesDeviceSupportNative16bitOps(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support native 16-bit operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  // Read data from the table
  int tableSize = sizeof(TertiaryHalfOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(TertiaryHalfOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);
  CW2A Arguments(handler.GetTableParamByName(L"ShaderOp.Arguments")->m_str);

  std::vector<uint16_t> *Validation_Input1 =
      &(handler.GetTableParamByName(L"Validation.Input1")->m_halfTable);
  std::vector<uint16_t> *Validation_Input2 =
      &(handler.GetTableParamByName(L"Validation.Input2")->m_halfTable);
  std::vector<uint16_t> *Validation_Input3 =
      &(handler.GetTableParamByName(L"Validation.Input3")->m_halfTable);

  std::vector<uint16_t> *Validation_Expected =
      &(handler.GetTableParamByName(L"Validation.Expected1")->m_halfTable);

  LPCWSTR Validation_Type =
      handler.GetTableParamByName(L"Validation.Type")->m_str;
  double Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_double;
  size_t count = Validation_Input1->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "TertiaryFPOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "STertiaryFPOp"));
        size_t size = sizeof(STertiaryHalfOp) * count;
        Data.resize(size);
        STertiaryHalfOp *pPrimitives = (STertiaryHalfOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          STertiaryHalfOp *p = &pPrimitives[i];
          p->input1 = (*Validation_Input1)[i % Validation_Input1->size()];
          p->input2 = (*Validation_Input2)[i % Validation_Input2->size()];
          p->input3 = (*Validation_Input3)[i % Validation_Input3->size()];
        }

        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
        pShaderOp->Shaders.at(0).Arguments = Arguments.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("STertiaryFPOp", &data);

  STertiaryHalfOp *pPrimitives = (STertiaryHalfOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;

  for (unsigned i = 0; i < count; ++i) {
    STertiaryHalfOp *p = &pPrimitives[i];
    uint16_t expected = (*Validation_Expected)[i % Validation_Expected->size()];
    LogCommentFmt(L"element #%u,  input1 = %6.8f(0x%04x), input2 = "
                  L"%6.8f(0x%04x), input3 = %6.8f(0x%04x), output = "
                  L"%6.8f(0x%04x), expected = %6.8f(0x%04x)",
                  i, ConvertFloat16ToFloat32(p->input1), p->input1,
                  ConvertFloat16ToFloat32(p->input2), p->input2,
                  ConvertFloat16ToFloat32(p->input3), p->input3,
                  ConvertFloat16ToFloat32(p->output), p->output,
                  ConvertFloat16ToFloat32(expected), expected);
    VerifyOutputWithExpectedValueHalf(p->output, expected, Validation_Type,
                                      Validation_Tolerance);
  }
}

TEST_F(ExecutionTest, UnaryIntOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice)) {
    return;
  }
  // Read data from the table

  int tableSize = sizeof(UnaryIntOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(UnaryIntOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);

  std::vector<int> *Validation_Input =
      &handler.GetTableParamByName(L"Validation.Input1")->m_int32Table;
  std::vector<int> *Validation_Expected =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_int32Table;
  int Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_int32;
  size_t count = Validation_Input->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "UnaryIntOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SUnaryIntOp"));
        size_t size = sizeof(SUnaryIntOp) * count;
        Data.resize(size);
        SUnaryIntOp *pPrimitives = (SUnaryIntOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SUnaryIntOp *p = &pPrimitives[i];
          int val = (*Validation_Input)[i % Validation_Input->size()];
          p->input = val;
        }
        // use shader data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SUnaryIntOp", &data);

  SUnaryIntOp *pPrimitives = (SUnaryIntOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  for (unsigned i = 0; i < count; ++i) {
    SUnaryIntOp *p = &pPrimitives[i];
    int val = (*Validation_Expected)[i % Validation_Expected->size()];
    LogCommentFmt(L"element #%u, input = %11i(0x%08x), output = %11i(0x%08x), "
                  L"expected = %11i(0x%08x)",
                  i, p->input, p->input, p->output, p->output, val, val);
    VerifyOutputWithExpectedValueInt(p->output, val, Validation_Tolerance);
  }
}

TEST_F(ExecutionTest, UnaryUintOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice)) {
    return;
  }
  // Read data from the table

  int tableSize = sizeof(UnaryUintOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(UnaryUintOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);

  std::vector<unsigned int> *Validation_Input =
      &handler.GetTableParamByName(L"Validation.Input1")->m_uint32Table;
  std::vector<unsigned int> *Validation_Expected =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_uint32Table;
  int Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_int32;
  size_t count = Validation_Input->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "UnaryUintOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SUnaryUintOp"));
        size_t size = sizeof(SUnaryUintOp) * count;
        Data.resize(size);
        SUnaryUintOp *pPrimitives = (SUnaryUintOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SUnaryUintOp *p = &pPrimitives[i];
          unsigned int val = (*Validation_Input)[i % Validation_Input->size()];
          p->input = val;
        }
        // use shader data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SUnaryUintOp", &data);

  SUnaryUintOp *pPrimitives = (SUnaryUintOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  for (unsigned i = 0; i < count; ++i) {
    SUnaryUintOp *p = &pPrimitives[i];
    unsigned int val = (*Validation_Expected)[i % Validation_Expected->size()];
    LogCommentFmt(L"element #%u, input = %11u(0x%08x), output = %11u(0x%08x), "
                  L"expected = %11u(0x%08x)",
                  i, p->input, p->input, p->output, p->output, val, val);
    VerifyOutputWithExpectedValueInt(p->output, val, Validation_Tolerance);
  }
}

TEST_F(ExecutionTest, BinaryIntOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice)) {
    return;
  }
  // Read data from the table
  size_t tableSize = sizeof(BinaryIntOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(BinaryIntOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);

  std::vector<int> *Validation_Input1 =
      &handler.GetTableParamByName(L"Validation.Input1")->m_int32Table;
  std::vector<int> *Validation_Input2 =
      &handler.GetTableParamByName(L"Validation.Input2")->m_int32Table;
  std::vector<int> *Validation_Expected1 =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_int32Table;
  std::vector<int> *Validation_Expected2 =
      &handler.GetTableParamByName(L"Validation.Expected2")->m_int32Table;
  int Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_int32;
  size_t count = Validation_Input1->size();

  size_t numExpected = Validation_Expected2->size() == 0 ? 1 : 2;

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "BinaryIntOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SBinaryIntOp"));
        size_t size = sizeof(SBinaryIntOp) * count;
        Data.resize(size);
        SBinaryIntOp *pPrimitives = (SBinaryIntOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SBinaryIntOp *p = &pPrimitives[i];
          int val1 = (*Validation_Input1)[i % Validation_Input1->size()];
          int val2 = (*Validation_Input2)[i % Validation_Input2->size()];
          p->input1 = val1;
          p->input2 = val2;
        }

        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SBinaryIntOp", &data);

  SBinaryIntOp *pPrimitives = (SBinaryIntOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;

  if (numExpected == 2) {
    for (unsigned i = 0; i < count; ++i) {
      SBinaryIntOp *p = &pPrimitives[i];
      int val1 = (*Validation_Expected1)[i % Validation_Expected1->size()];
      int val2 = (*Validation_Expected2)[i % Validation_Expected2->size()];
      LogCommentFmt(L"element #%u, input1 = %11i(0x%08x), input2 = "
                    L"%11i(0x%08x), output1 = "
                    L"%11i(0x%08x), expected1 = %11i(0x%08x), output2 = "
                    L"%11i(0x%08x), expected2 = %11i(0x%08x)",
                    i, p->input1, p->input1, p->input2, p->input2, p->output1,
                    p->output1, val1, val1, p->output2, p->output2, val2, val2);
      VerifyOutputWithExpectedValueInt(p->output1, val1, Validation_Tolerance);
      VerifyOutputWithExpectedValueInt(p->output2, val2, Validation_Tolerance);
    }
  } else if (numExpected == 1) {
    for (unsigned i = 0; i < count; ++i) {
      SBinaryIntOp *p = &pPrimitives[i];
      int val1 = (*Validation_Expected1)[i % Validation_Expected1->size()];
      LogCommentFmt(L"element #%u, input1 = %11i(0x%08x), input2 = "
                    L"%11i(0x%08x), output = "
                    L"%11i(0x%08x), expected = %11i(0x%08x)",
                    i, p->input1, p->input1, p->input2, p->input2, p->output1,
                    p->output1, val1, val1);
      VerifyOutputWithExpectedValueInt(p->output1, val1, Validation_Tolerance);
    }
  } else {
    LogErrorFmt(L"Unexpected number of expected values for operation %i",
                numExpected);
  }
}

TEST_F(ExecutionTest, TertiaryIntOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice)) {
    return;
  }
  // Read data from the table
  size_t tableSize = sizeof(TertiaryIntOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(TertiaryIntOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);

  std::vector<int> *Validation_Input1 =
      &handler.GetTableParamByName(L"Validation.Input1")->m_int32Table;
  std::vector<int> *Validation_Input2 =
      &handler.GetTableParamByName(L"Validation.Input2")->m_int32Table;
  std::vector<int> *Validation_Input3 =
      &handler.GetTableParamByName(L"Validation.Input3")->m_int32Table;
  std::vector<int> *Validation_Expected =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_int32Table;
  int Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_int32;
  size_t count = Validation_Input1->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "TertiaryIntOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "STertiaryIntOp"));
        size_t size = sizeof(STertiaryIntOp) * count;
        Data.resize(size);
        STertiaryIntOp *pPrimitives = (STertiaryIntOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          STertiaryIntOp *p = &pPrimitives[i];
          int val1 = (*Validation_Input1)[i % Validation_Input1->size()];
          int val2 = (*Validation_Input2)[i % Validation_Input2->size()];
          int val3 = (*Validation_Input3)[i % Validation_Input3->size()];
          p->input1 = val1;
          p->input2 = val2;
          p->input3 = val3;
        }

        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("STertiaryIntOp", &data);

  STertiaryIntOp *pPrimitives = (STertiaryIntOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  for (unsigned i = 0; i < count; ++i) {
    STertiaryIntOp *p = &pPrimitives[i];
    int val1 = (*Validation_Expected)[i % Validation_Expected->size()];
    LogCommentFmt(L"element #%u, input1 = %11i(0x%08x), input2 = "
                  L"%11i(0x%08x), input3= %11i(0x%08x), output = "
                  L"%11i(0x%08x), expected = %11i(0x%08x)",
                  i, p->input1, p->input1, p->input2, p->input2, p->input3,
                  p->input3, p->output, p->output, val1, val1);
    VerifyOutputWithExpectedValueInt(p->output, val1, Validation_Tolerance);
  }
}

TEST_F(ExecutionTest, BinaryUintOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice)) {
    return;
  }
  // Read data from the table
  size_t tableSize = sizeof(BinaryUintOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(BinaryUintOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);

  std::vector<unsigned int> *Validation_Input1 =
      &handler.GetTableParamByName(L"Validation.Input1")->m_uint32Table;
  std::vector<unsigned int> *Validation_Input2 =
      &handler.GetTableParamByName(L"Validation.Input2")->m_uint32Table;
  std::vector<unsigned int> *Validation_Expected1 =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_uint32Table;
  std::vector<unsigned int> *Validation_Expected2 =
      &handler.GetTableParamByName(L"Validation.Expected2")->m_uint32Table;
  int Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_int32;
  size_t count = Validation_Input1->size();
  int numExpected = Validation_Expected2->size() == 0 ? 1 : 2;
  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "BinaryUintOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SBinaryUintOp"));
        size_t size = sizeof(SBinaryUintOp) * count;
        Data.resize(size);
        SBinaryUintOp *pPrimitives = (SBinaryUintOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SBinaryUintOp *p = &pPrimitives[i];
          unsigned int val1 =
              (*Validation_Input1)[i % Validation_Input1->size()];
          unsigned int val2 =
              (*Validation_Input2)[i % Validation_Input2->size()];
          p->input1 = val1;
          p->input2 = val2;
        }

        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SBinaryUintOp", &data);

  SBinaryUintOp *pPrimitives = (SBinaryUintOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  if (numExpected == 2) {
    for (unsigned i = 0; i < count; ++i) {
      SBinaryUintOp *p = &pPrimitives[i];
      unsigned int val1 =
          (*Validation_Expected1)[i % Validation_Expected1->size()];
      unsigned int val2 =
          (*Validation_Expected2)[i % Validation_Expected2->size()];
      LogCommentFmt(L"element #%u, input1 = %11u(0x%08x), input2 = "
                    L"%11u(0x%08x), output1 = "
                    L"%11u(0x%08x), expected1 = %11u(0x%08x), output2 = "
                    L"%11u(0x%08x), expected2 = %11u(0x%08x)",
                    i, p->input1, p->input1, p->input2, p->input2, p->output1,
                    p->output1, val1, val1, p->output2, p->output2, val2, val2);
      VerifyOutputWithExpectedValueInt(p->output1, val1, Validation_Tolerance);
      VerifyOutputWithExpectedValueInt(p->output2, val2, Validation_Tolerance);
    }
  } else if (numExpected == 1) {
    for (unsigned i = 0; i < count; ++i) {
      SBinaryUintOp *p = &pPrimitives[i];
      unsigned int val1 =
          (*Validation_Expected1)[i % Validation_Expected1->size()];
      LogCommentFmt(L"element #%u, input1 = %11u(0x%08x), input2 = "
                    L"%11u(0x%08x), output = "
                    L"%11u(0x%08x), expected = %11u(0x%08x)",
                    i, p->input1, p->input1, p->input2, p->input2, p->output1,
                    p->output1, val1, val1);
      VerifyOutputWithExpectedValueInt(p->output1, val1, Validation_Tolerance);
    }
  } else {
    LogErrorFmt(L"Unexpected number of expected values for operation %i",
                numExpected);
  }
}

TEST_F(ExecutionTest, TertiaryUintOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice)) {
    return;
  }
  // Read data from the table
  size_t tableSize = sizeof(TertiaryUintOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(TertiaryUintOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);

  std::vector<unsigned int> *Validation_Input1 =
      &handler.GetTableParamByName(L"Validation.Input1")->m_uint32Table;
  std::vector<unsigned int> *Validation_Input2 =
      &handler.GetTableParamByName(L"Validation.Input2")->m_uint32Table;
  std::vector<unsigned int> *Validation_Input3 =
      &handler.GetTableParamByName(L"Validation.Input3")->m_uint32Table;
  std::vector<unsigned int> *Validation_Expected =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_uint32Table;
  int Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_int32;
  size_t count = Validation_Input1->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "TertiaryUintOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "STertiaryUintOp"));
        size_t size = sizeof(STertiaryUintOp) * count;
        Data.resize(size);
        STertiaryUintOp *pPrimitives = (STertiaryUintOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          STertiaryUintOp *p = &pPrimitives[i];
          unsigned int val1 =
              (*Validation_Input1)[i % Validation_Input1->size()];
          unsigned int val2 =
              (*Validation_Input2)[i % Validation_Input2->size()];
          unsigned int val3 =
              (*Validation_Input3)[i % Validation_Input3->size()];
          p->input1 = val1;
          p->input2 = val2;
          p->input3 = val3;
        }

        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("STertiaryUintOp", &data);

  STertiaryUintOp *pPrimitives = (STertiaryUintOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  for (unsigned i = 0; i < count; ++i) {
    STertiaryUintOp *p = &pPrimitives[i];
    unsigned int val1 = (*Validation_Expected)[i % Validation_Expected->size()];
    LogCommentFmt(L"element #%u, input1 = %11u(0x%08x), input2 = "
                  L"%11u(0x%08x), input3 = %11u(0x%08x), output = "
                  L"%11u(0x%08x), expected = %11u(0x%08x)",
                  i, p->input1, p->input1, p->input2, p->input2, p->input3,
                  p->input3, p->output, p->output, val1, val1);
    VerifyOutputWithExpectedValueInt(p->output, val1, Validation_Tolerance);
  }
}

// 16 bit integer type tests
TEST_F(ExecutionTest, UnaryInt16OpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_2)) {
    return;
  }

  if (!DoesDeviceSupportNative16bitOps(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support native 16-bit operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  // Read data from the table
  int tableSize = sizeof(UnaryInt16OpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(UnaryInt16OpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);
  CW2A Arguments(handler.GetTableParamByName(L"ShaderOp.Arguments")->m_str);

  std::vector<short> *Validation_Input =
      &handler.GetTableParamByName(L"Validation.Input1")->m_int16Table;
  std::vector<short> *Validation_Expected =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_int16Table;
  int Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_int32;
  size_t count = Validation_Input->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "UnaryIntOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SUnaryIntOp"));
        size_t size = sizeof(SUnaryInt16Op) * count;
        Data.resize(size);
        SUnaryInt16Op *pPrimitives = (SUnaryInt16Op *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SUnaryInt16Op *p = &pPrimitives[i];
          p->input = (*Validation_Input)[i % Validation_Input->size()];
        }
        // use shader data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
        pShaderOp->Shaders.at(0).Arguments = Arguments.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SUnaryIntOp", &data);

  SUnaryInt16Op *pPrimitives = (SUnaryInt16Op *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  for (unsigned i = 0; i < count; ++i) {
    SUnaryInt16Op *p = &pPrimitives[i];
    short val = (*Validation_Expected)[i % Validation_Expected->size()];
    LogCommentFmt(L"element #%u, input = %5hi(0x%08x), output = %5hi(0x%08x), "
                  L"expected = %5hi(0x%08x)",
                  i, p->input, p->input, p->output, p->output, val, val);
    VerifyOutputWithExpectedValueInt(p->output, val, Validation_Tolerance);
  }
}

TEST_F(ExecutionTest, UnaryUint16OpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_2)) {
    return;
  }

  if (!DoesDeviceSupportNative16bitOps(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support native 16-bit operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  // Read data from the table
  int tableSize = sizeof(UnaryUint16OpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(UnaryUint16OpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);
  CW2A Arguments(handler.GetTableParamByName(L"ShaderOp.Arguments")->m_str);

  std::vector<unsigned short> *Validation_Input =
      &handler.GetTableParamByName(L"Validation.Input1")->m_uint16Table;
  std::vector<unsigned short> *Validation_Expected =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_uint16Table;
  int Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_int32;
  size_t count = Validation_Input->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "UnaryUintOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SUnaryUintOp"));
        size_t size = sizeof(SUnaryUint16Op) * count;
        Data.resize(size);
        SUnaryUint16Op *pPrimitives = (SUnaryUint16Op *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SUnaryUint16Op *p = &pPrimitives[i];
          p->input = (*Validation_Input)[i % Validation_Input->size()];
        }
        // use shader data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
        pShaderOp->Shaders.at(0).Arguments = Arguments.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SUnaryUintOp", &data);

  SUnaryUint16Op *pPrimitives = (SUnaryUint16Op *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  for (unsigned i = 0; i < count; ++i) {
    SUnaryUint16Op *p = &pPrimitives[i];
    unsigned short val =
        (*Validation_Expected)[i % Validation_Expected->size()];
    LogCommentFmt(L"element #%u, input = %5hu(0x%08x), output = %5hu(0x%08x), "
                  L"expected = %5hu(0x%08x)",
                  i, p->input, p->input, p->output, p->output, val, val);
    VerifyOutputWithExpectedValueInt(p->output, val, Validation_Tolerance);
  }
}

TEST_F(ExecutionTest, BinaryInt16OpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_2)) {
    return;
  }

  if (!DoesDeviceSupportNative16bitOps(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support native 16-bit operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  // Read data from the table
  size_t tableSize = sizeof(BinaryInt16OpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(BinaryInt16OpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);
  CW2A Arguments(handler.GetTableParamByName(L"ShaderOp.Arguments")->m_str);

  std::vector<short> *Validation_Input1 =
      &handler.GetTableParamByName(L"Validation.Input1")->m_int16Table;
  std::vector<short> *Validation_Input2 =
      &handler.GetTableParamByName(L"Validation.Input2")->m_int16Table;
  std::vector<short> *Validation_Expected1 =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_int16Table;
  std::vector<short> *Validation_Expected2 =
      &handler.GetTableParamByName(L"Validation.Expected2")->m_int16Table;
  int Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_int32;
  size_t count = Validation_Input1->size();

  size_t numExpected = Validation_Expected2->size() == 0 ? 1 : 2;

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "BinaryIntOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SBinaryIntOp"));
        size_t size = sizeof(SBinaryInt16Op) * count;
        Data.resize(size);
        SBinaryInt16Op *pPrimitives = (SBinaryInt16Op *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SBinaryInt16Op *p = &pPrimitives[i];
          p->input1 = (*Validation_Input1)[i % Validation_Input1->size()];
          p->input2 = (*Validation_Input2)[i % Validation_Input2->size()];
        }

        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
        pShaderOp->Shaders.at(0).Arguments = Arguments.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SBinaryIntOp", &data);

  SBinaryInt16Op *pPrimitives = (SBinaryInt16Op *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;

  if (numExpected == 2) {
    for (unsigned i = 0; i < count; ++i) {
      SBinaryInt16Op *p = &pPrimitives[i];
      short val1 = (*Validation_Expected1)[i % Validation_Expected1->size()];
      short val2 = (*Validation_Expected2)[i % Validation_Expected2->size()];
      LogCommentFmt(L"element #%u, input1 = %5hi(0x%08x), input2 = "
                    L"%5hi(0x%08x), output1 = "
                    L"%5hi(0x%08x), expected1 = %5hi(0x%08x), output2 = "
                    L"%5hi(0x%08x), expected2 = %5hi(0x%08x)",
                    i, p->input1, p->input1, p->input2, p->input2, p->output1,
                    p->output1, val1, val1, p->output2, p->output2, val2, val2);
      VerifyOutputWithExpectedValueInt(p->output1, val1, Validation_Tolerance);
      VerifyOutputWithExpectedValueInt(p->output2, val2, Validation_Tolerance);
    }
  } else if (numExpected == 1) {
    for (unsigned i = 0; i < count; ++i) {
      SBinaryInt16Op *p = &pPrimitives[i];
      short val1 = (*Validation_Expected1)[i % Validation_Expected1->size()];
      LogCommentFmt(L"element #%u, input1 = %5hi(0x%08x), input2 = "
                    L"%5hi(0x%08x), output = "
                    L"%5hi(0x%08x), expected = %5hi(0x%08x)",
                    i, p->input1, p->input1, p->input2, p->input2, p->output1,
                    p->output1, val1, val1);
      VerifyOutputWithExpectedValueInt(p->output1, val1, Validation_Tolerance);
    }
  } else {
    LogErrorFmt(L"Unexpected number of expected values for operation %i",
                numExpected);
  }
}

TEST_F(ExecutionTest, TertiaryInt16OpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_2)) {
    return;
  }

  if (!DoesDeviceSupportNative16bitOps(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support native 16-bit operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  // Read data from the table
  size_t tableSize = sizeof(TertiaryInt16OpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(TertiaryInt16OpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);
  CW2A Arguments(handler.GetTableParamByName(L"ShaderOp.Arguments")->m_str);

  std::vector<short> *Validation_Input1 =
      &handler.GetTableParamByName(L"Validation.Input1")->m_int16Table;
  std::vector<short> *Validation_Input2 =
      &handler.GetTableParamByName(L"Validation.Input2")->m_int16Table;
  std::vector<short> *Validation_Input3 =
      &handler.GetTableParamByName(L"Validation.Input3")->m_int16Table;
  std::vector<short> *Validation_Expected =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_int16Table;
  int Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_int32;
  size_t count = Validation_Input1->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "TertiaryIntOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "STertiaryIntOp"));
        size_t size = sizeof(STertiaryInt16Op) * count;
        Data.resize(size);
        STertiaryInt16Op *pPrimitives = (STertiaryInt16Op *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          STertiaryInt16Op *p = &pPrimitives[i];
          p->input1 = (*Validation_Input1)[i % Validation_Input1->size()];
          p->input2 = (*Validation_Input2)[i % Validation_Input2->size()];
          p->input3 = (*Validation_Input3)[i % Validation_Input3->size()];
        }

        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
        pShaderOp->Shaders.at(0).Arguments = Arguments.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("STertiaryIntOp", &data);

  STertiaryInt16Op *pPrimitives = (STertiaryInt16Op *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  for (unsigned i = 0; i < count; ++i) {
    STertiaryInt16Op *p = &pPrimitives[i];
    short val1 = (*Validation_Expected)[i % Validation_Expected->size()];
    LogCommentFmt(L"element #%u, input1 = %11i(0x%08x), input2 = "
                  L"%11i(0x%08x), input3= %11i(0x%08x), output = "
                  L"%11i(0x%08x), expected = %11i(0x%08x)",
                  i, p->input1, p->input1, p->input2, p->input2, p->input3,
                  p->input3, p->output, p->output, val1, val1);
    VerifyOutputWithExpectedValueInt(p->output, val1, Validation_Tolerance);
  }
}

TEST_F(ExecutionTest, BinaryUint16OpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_2)) {
    return;
  }

  if (!DoesDeviceSupportNative16bitOps(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support native 16-bit operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  // Read data from the table
  size_t tableSize = sizeof(BinaryUint16OpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(BinaryUint16OpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);
  CW2A Arguments(handler.GetTableParamByName(L"ShaderOp.Arguments")->m_str);

  std::vector<unsigned short> *Validation_Input1 =
      &handler.GetTableParamByName(L"Validation.Input1")->m_uint16Table;
  std::vector<unsigned short> *Validation_Input2 =
      &handler.GetTableParamByName(L"Validation.Input2")->m_uint16Table;
  std::vector<unsigned short> *Validation_Expected1 =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_uint16Table;
  std::vector<unsigned short> *Validation_Expected2 =
      &handler.GetTableParamByName(L"Validation.Expected2")->m_uint16Table;
  int Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_int32;
  size_t count = Validation_Input1->size();
  int numExpected = Validation_Expected2->size() == 0 ? 1 : 2;
  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "BinaryUintOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SBinaryUintOp"));
        size_t size = sizeof(SBinaryUint16Op) * count;
        Data.resize(size);
        SBinaryUint16Op *pPrimitives = (SBinaryUint16Op *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SBinaryUint16Op *p = &pPrimitives[i];
          p->input1 = (*Validation_Input1)[i % Validation_Input1->size()];
          p->input2 = (*Validation_Input2)[i % Validation_Input2->size()];
        }

        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
        pShaderOp->Shaders.at(0).Arguments = Arguments.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SBinaryUintOp", &data);

  SBinaryUint16Op *pPrimitives = (SBinaryUint16Op *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  if (numExpected == 2) {
    for (unsigned i = 0; i < count; ++i) {
      SBinaryUint16Op *p = &pPrimitives[i];
      unsigned short val1 =
          (*Validation_Expected1)[i % Validation_Expected1->size()];
      unsigned short val2 =
          (*Validation_Expected2)[i % Validation_Expected2->size()];
      LogCommentFmt(L"element #%u, input1 = %5hu(0x%08x), input2 = "
                    L"%5hu(0x%08x), output1 = "
                    L"%5hu(0x%08x), expected1 = %5hu(0x%08x), output2 = "
                    L"%5hu(0x%08x), expected2 = %5hu(0x%08x)",
                    i, p->input1, p->input1, p->input2, p->input2, p->output1,
                    p->output1, val1, val1, p->output2, p->output2, val2, val2);
      VerifyOutputWithExpectedValueInt(p->output1, val1, Validation_Tolerance);
      VerifyOutputWithExpectedValueInt(p->output2, val2, Validation_Tolerance);
    }
  } else if (numExpected == 1) {
    for (unsigned i = 0; i < count; ++i) {
      SBinaryUint16Op *p = &pPrimitives[i];
      unsigned short val1 =
          (*Validation_Expected1)[i % Validation_Expected1->size()];
      LogCommentFmt(L"element #%u, input1 = %5hu(0x%08x), input2 = "
                    L"%5hu(0x%08x), output = "
                    L"%5hu(0x%08x), expected = %5hu(0x%08x)",
                    i, p->input1, p->input1, p->input2, p->input2, p->output1,
                    p->output1, val1, val1);
      VerifyOutputWithExpectedValueInt(p->output1, val1, Validation_Tolerance);
    }
  } else {
    LogErrorFmt(L"Unexpected number of expected values for operation %i",
                numExpected);
  }
}

TEST_F(ExecutionTest, TertiaryUint16OpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_2)) {
    return;
  }

  if (!DoesDeviceSupportNative16bitOps(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support native 16-bit operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  // Read data from the table
  size_t tableSize =
      sizeof(TertiaryUint16OpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(TertiaryUint16OpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);
  CW2A Arguments(handler.GetTableParamByName(L"ShaderOp.Arguments")->m_str);

  std::vector<unsigned short> *Validation_Input1 =
      &handler.GetTableParamByName(L"Validation.Input1")->m_uint16Table;
  std::vector<unsigned short> *Validation_Input2 =
      &handler.GetTableParamByName(L"Validation.Input2")->m_uint16Table;
  std::vector<unsigned short> *Validation_Input3 =
      &handler.GetTableParamByName(L"Validation.Input3")->m_uint16Table;
  std::vector<unsigned short> *Validation_Expected =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_uint16Table;
  int Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_int32;
  size_t count = Validation_Input1->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "TertiaryUintOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "STertiaryUintOp"));
        size_t size = sizeof(STertiaryUint16Op) * count;
        Data.resize(size);
        STertiaryUint16Op *pPrimitives = (STertiaryUint16Op *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          STertiaryUint16Op *p = &pPrimitives[i];
          p->input1 = (*Validation_Input1)[i % Validation_Input1->size()];
          p->input2 = (*Validation_Input2)[i % Validation_Input2->size()];
          p->input3 = (*Validation_Input3)[i % Validation_Input3->size()];
        }

        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
        pShaderOp->Shaders.at(0).Arguments = Arguments.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("STertiaryUintOp", &data);

  STertiaryUint16Op *pPrimitives = (STertiaryUint16Op *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  for (unsigned i = 0; i < count; ++i) {
    STertiaryUint16Op *p = &pPrimitives[i];
    unsigned short val1 =
        (*Validation_Expected)[i % Validation_Expected->size()];
    LogCommentFmt(L"element #%u, input1 = %5hu(0x%08x), input2 = "
                  L"%5hu(0x%08x), input3 = %5hu(0x%08x), output = "
                  L"%5hu(0x%08x), expected = %5hu(0x%08x)",
                  i, p->input1, p->input1, p->input2, p->input2, p->input3,
                  p->input3, p->output, p->output, val1, val1);
    VerifyOutputWithExpectedValueInt(p->output, val1, Validation_Tolerance);
  }
}

template <typename T1, typename T2, typename TYPE_ACC>
void MatrixMultiplyAndAddMatrix(int DIM_M, int DIM_N, int dim_k, T1 *leftMatrix,
                                T2 *rightMatrix, TYPE_ACC *resultMatrix) {
  using namespace DirectX::PackedVector;
  using T1_PROMOTED =
      typename std::conditional<std::is_same<T1, HALF>::value, float, T1>::type;
  using T2_PROMOTED =
      typename std::conditional<std::is_same<T2, HALF>::value, float, T2>::type;

  for (int i = 0; i < DIM_M; ++i) {
    for (int j = 0; j < DIM_N; ++j) {
      int ij = i * DIM_N + j;

      for (int k = 0; k < dim_k; ++k) {
        int ik = i * dim_k + k;
        int kj = k * DIM_N + j;

        T1_PROMOTED leftMatrixVal = leftMatrix[ik];
        T2_PROMOTED rightMatrixVal = rightMatrix[kj];
        if (typeid(T1) == typeid(HALF)) {
          DXASSERT_NOMSG(typeid(T2) == typeid(HALF));
          leftMatrixVal = static_cast<T1_PROMOTED>(
              ConvertFloat16ToFloat32(static_cast<HALF>(leftMatrix[ik])));
          rightMatrixVal = static_cast<T2_PROMOTED>(
              ConvertFloat16ToFloat32(static_cast<HALF>(rightMatrix[kj])));
        }

        if (typeid(TYPE_ACC) == typeid(HALF)) {
          resultMatrix[ij] = ConvertFloat32ToFloat16(
              ConvertFloat16ToFloat32(static_cast<HALF>(resultMatrix[ij])) +
              ConvertFloat16ToFloat32(
                  ConvertFloat32ToFloat16(static_cast<float>(leftMatrixVal) *
                                          static_cast<float>(rightMatrixVal))));
        } else {
          resultMatrix[ij] += static_cast<TYPE_ACC>(leftMatrixVal) *
                              static_cast<TYPE_ACC>(rightMatrixVal);
        }
      }
    }
  }
}

template <typename T1, typename T2, typename TYPE_ACC>
void MatrixMultiplyByMatrix(int DIM_M, int DIM_N, int k, T1 *leftMatrix,
                            T2 *rightMatrix, TYPE_ACC *resultMatrix) {
  memset(resultMatrix, 0, DIM_M * DIM_N * sizeof(TYPE_ACC));
  MatrixMultiplyAndAddMatrix<T1, T2, TYPE_ACC>(DIM_M, DIM_N, k, leftMatrix,
                                               rightMatrix, resultMatrix);
}

template <typename T>
void MatrixAddMatrix(int DIM_M, int DIM_N, T *matrixToAdd, T *resultMatrix) {
  using namespace DirectX::PackedVector;

  for (size_t i = 0; i < (size_t)(DIM_M * DIM_N); ++i) {
    if (typeid(T) == typeid(HALF)) {
      resultMatrix[i] = ConvertFloat32ToFloat16(
          ConvertFloat16ToFloat32(static_cast<HALF>(resultMatrix[i])) +
          ConvertFloat16ToFloat32(static_cast<HALF>(matrixToAdd[i])));
    } else {
      resultMatrix[i] += matrixToAdd[i];
    }
  }
}

template <typename T>
void MatrixAddColumn(int DIM_M, int DIM_N, T *leftCol, T *resultMatrix) {
  using namespace DirectX::PackedVector;

  for (int i = 0; i < DIM_M; ++i) {
    for (int j = 0; j < DIM_N; ++j) {
      int ij = i * DIM_N + j;

      if (typeid(T) == typeid(HALF)) {
        resultMatrix[ij] = ConvertFloat32ToFloat16(
            ConvertFloat16ToFloat32(static_cast<HALF>(resultMatrix[ij])) +
            ConvertFloat16ToFloat32(static_cast<HALF>(leftCol[i])));
      } else {
        resultMatrix[ij] += leftCol[i];
      }
    }
  }
}

template <typename T>
void MatrixAddRow(int DIM_M, int DIM_N, T *rightRow, T *resultMatrix) {
  using namespace DirectX::PackedVector;

  for (int i = 0; i < DIM_M; ++i) {
    for (int j = 0; j < DIM_N; ++j) {
      int ij = i * DIM_N + j;
      if (typeid(T) == typeid(HALF)) {
        resultMatrix[ij] = ConvertFloat32ToFloat16(
            ConvertFloat16ToFloat32(static_cast<HALF>(resultMatrix[ij])) +
            ConvertFloat16ToFloat32(static_cast<HALF>(rightRow[j])));
      } else {
        resultMatrix[ij] += rightRow[j];
      }
    }
  }
}

template <typename TYPE_ACC, typename T>
void MatrixSumColumns(int DIM_M, int k, TYPE_ACC *leftCol, T *inMatrix) {
  using namespace DirectX::PackedVector;
  using T_PROMOTED =
      typename std::conditional<std::is_same<T, HALF>::value, float, T>::type;

  for (int i = 0; i < DIM_M; ++i) {
    for (int j = 0; j < k; ++j) {
      int ij = i * k + j;
      T_PROMOTED inMatrixP = static_cast<T_PROMOTED>(inMatrix[ij]);

      if (typeid(T) == typeid(HALF)) {
        inMatrixP = static_cast<T_PROMOTED>(
            ConvertFloat16ToFloat32(static_cast<HALF>(inMatrix[ij])));
      }

      if (typeid(TYPE_ACC) == typeid(HALF)) {
        leftCol[i] = ConvertFloat32ToFloat16(
            ConvertFloat16ToFloat32(static_cast<HALF>(leftCol[i])) + inMatrixP);
      } else {
        leftCol[i] += static_cast<TYPE_ACC>(inMatrixP);
      }
    }
  }
}

template <typename TYPE_ACC, typename T>
void MatrixSumRows(int DIM_N, int k, TYPE_ACC *rightRow, T *inMatrix) {
  using namespace DirectX::PackedVector;
  using T_PROMOTED =
      typename std::conditional<std::is_same<T, HALF>::value, float, T>::type;

  for (int i = 0; i < k; ++i) {
    for (int j = 0; j < DIM_N; ++j) {
      int ij = i * DIM_N + j;
      T_PROMOTED inMatrixP = static_cast<T_PROMOTED>(inMatrix[ij]);

      if (typeid(T) == typeid(HALF)) {
        inMatrixP = static_cast<T_PROMOTED>(
            ConvertFloat16ToFloat32(static_cast<HALF>(inMatrix[ij])));
      }

      if (typeid(TYPE_ACC) == typeid(HALF)) {
        rightRow[j] = ConvertFloat32ToFloat16(
            ConvertFloat16ToFloat32(static_cast<HALF>(rightRow[j])) +
            inMatrixP);
      } else {
        rightRow[j] += static_cast<TYPE_ACC>(inMatrixP);
      }
    }
  }
}

template <typename T>
void MatrixMultiplyByScalar(int DIM_M, int DIM_N, T scalar, T *resultMatrix) {
  for (int i = 0; i < DIM_M; ++i) {
    for (int j = 0; j < DIM_N; ++j) {
      int ij = i * DIM_N + j;
      resultMatrix[ij] *= scalar;
    }
  }
}

template <>
void MatrixMultiplyByScalar<DirectX::PackedVector::HALF>(
    int DIM_M, int DIM_N, DirectX::PackedVector::HALF scalar,
    DirectX::PackedVector::HALF *resultMatrix) {
  for (int i = 0; i < DIM_M; ++i) {
    for (int j = 0; j < DIM_N; ++j) {
      int ij = i * DIM_N + j;
      resultMatrix[ij] =
          ConvertFloat32ToFloat16(ConvertFloat16ToFloat32(resultMatrix[ij]) *
                                  ConvertFloat16ToFloat32(scalar));
    }
  }
}

template <typename T>
void MatrixDivideByScalar(int DIM_M, int DIM_N, T scalar, T *resultMatrix) {
  for (int i = 0; i < DIM_M; ++i) {
    for (int j = 0; j < DIM_N; ++j) {
      int ij = i * DIM_N + j;
      resultMatrix[ij] /= scalar;
    }
  }
}

template <>
void MatrixDivideByScalar<DirectX::PackedVector::HALF>(
    int DIM_M, int DIM_N, DirectX::PackedVector::HALF scalar,
    DirectX::PackedVector::HALF *resultMatrix) {
  for (int i = 0; i < DIM_M; ++i) {
    for (int j = 0; j < DIM_N; ++j) {
      int ij = i * DIM_N + j;
      resultMatrix[ij] =
          ConvertFloat32ToFloat16(ConvertFloat16ToFloat32(resultMatrix[ij]) /
                                  ConvertFloat16ToFloat32(scalar));
    }
  }
}

template <typename T>
void MatrixAddScalar(int DIM_M, int DIM_N, T scalar, T *resultMatrix) {
  for (int i = 0; i < DIM_M; ++i) {
    for (int j = 0; j < DIM_N; ++j) {
      int ij = i * DIM_N + j;
      resultMatrix[ij] += scalar;
    }
  }
}

template <>
void MatrixAddScalar<DirectX::PackedVector::HALF>(
    int DIM_M, int DIM_N, DirectX::PackedVector::HALF scalar,
    DirectX::PackedVector::HALF *resultMatrix) {
  for (int i = 0; i < DIM_M; ++i) {
    for (int j = 0; j < DIM_N; ++j) {
      int ij = i * DIM_N + j;
      resultMatrix[ij] =
          ConvertFloat32ToFloat16(ConvertFloat16ToFloat32(resultMatrix[ij]) +
                                  ConvertFloat16ToFloat32(scalar));
    }
  }
}

template <typename T>
void MatrixSubtractScalar(int DIM_M, int DIM_N, T scalar, T *resultMatrix) {
  for (int i = 0; i < DIM_M; ++i) {
    for (int j = 0; j < DIM_N; ++j) {
      int ij = i * DIM_N + j;
      resultMatrix[ij] -= scalar;
    }
  }
}

template <>
void MatrixSubtractScalar<DirectX::PackedVector::HALF>(
    int DIM_M, int DIM_N, DirectX::PackedVector::HALF scalar,
    DirectX::PackedVector::HALF *resultMatrix) {
  for (int i = 0; i < DIM_M; ++i) {
    for (int j = 0; j < DIM_N; ++j) {
      int ij = i * DIM_N + j;
      resultMatrix[ij] =
          ConvertFloat32ToFloat16(ConvertFloat16ToFloat32(resultMatrix[ij]) -
                                  ConvertFloat16ToFloat32(scalar));
    }
  }
}

template <typename T>
void VectorMultiplyByScalar(int DIM, T scalar, T *rowCol) {
  for (int i = 0; i < DIM; ++i) {
    rowCol[i] *= scalar;
  }
}

template <>
void VectorMultiplyByScalar<DirectX::PackedVector::HALF>(
    int DIM, DirectX::PackedVector::HALF scalar,
    DirectX::PackedVector::HALF *rowCol) {
  for (int i = 0; i < DIM; ++i) {
    rowCol[i] = ConvertFloat32ToFloat16(ConvertFloat16ToFloat32(rowCol[i]) *
                                        ConvertFloat16ToFloat32(scalar));
  }
}

template <typename T> void VectorDivideByScalar(int DIM, T scalar, T *rowCol) {
  for (int i = 0; i < DIM; ++i) {
    rowCol[i] /= scalar;
  }
}

template <>
void VectorDivideByScalar<DirectX::PackedVector::HALF>(
    int DIM, DirectX::PackedVector::HALF scalar,
    DirectX::PackedVector::HALF *rowCol) {
  for (int i = 0; i < DIM; ++i) {
    rowCol[i] = ConvertFloat32ToFloat16(ConvertFloat16ToFloat32(rowCol[i]) /
                                        ConvertFloat16ToFloat32(scalar));
  }
}

template <typename T> void VectorAddScalar(int DIM, T scalar, T *rowCol) {
  for (int i = 0; i < DIM; ++i) {
    rowCol[i] += scalar;
  }
}

template <>
void VectorAddScalar<DirectX::PackedVector::HALF>(
    int DIM, DirectX::PackedVector::HALF scalar,
    DirectX::PackedVector::HALF *rowCol) {
  for (int i = 0; i < DIM; ++i) {
    rowCol[i] = ConvertFloat32ToFloat16(ConvertFloat16ToFloat32(rowCol[i]) +
                                        ConvertFloat16ToFloat32(scalar));
  }
}

template <typename T> void VectorSubtractScalar(int DIM, T scalar, T *rowCol) {
  for (int i = 0; i < DIM; ++i) {
    rowCol[i] -= scalar;
  }
}

template <>
void VectorSubtractScalar<DirectX::PackedVector::HALF>(
    int DIM, DirectX::PackedVector::HALF scalar,
    DirectX::PackedVector::HALF *rowCol) {
  for (int i = 0; i < DIM; ++i) {
    rowCol[i] = ConvertFloat32ToFloat16(ConvertFloat16ToFloat32(rowCol[i]) -
                                        ConvertFloat16ToFloat32(scalar));
  }
}

template <typename T>
void GenerateMatrix(T *dst, size_t num_elements, double start_val,
                    double end_val) {
  for (size_t i = 0; i < num_elements; ++i) {
    double t = i / (num_elements - 1.0);
    dst[i] = static_cast<T>((1.0 - t) * start_val + t * end_val);
  }
}

template <>
void GenerateMatrix<DirectX::PackedVector::HALF>(
    DirectX::PackedVector::HALF *dst, size_t num_elements, double start_val,
    double end_val) {
  std::vector<DirectX::PackedVector::HALF> matrix(num_elements);

  for (size_t i = 0; i < num_elements; ++i) {
    double t = i / (num_elements - 1.0);
    dst[i] =
        ConvertFloat32ToFloat16((float)((1.0 - t) * start_val + t * end_val));
  }
}

template <typename T> void FillMatrix(T *dst, size_t num_elements, T val) {
  for (size_t i = 0; i < num_elements; ++i) {
    dst[i] = val;
  }
}

void ConvertRangeFloatToHalf(DirectX::PackedVector::HALF *dst, float *src,
                             size_t count) {
  for (size_t i = 0u; i < count; ++i) {
    *dst = ConvertFloat32ToFloat16(*src);
    ++dst;
    ++src;
  }
}

void ConvertRangeHalfToFloat(float *dst, DirectX::PackedVector::HALF *src,
                             size_t count) {
  for (size_t i = 0u; i < count; ++i) {
    *dst = ConvertFloat16ToFloat32(*src);
    ++dst;
    ++src;
  }
}

#ifndef NDEBUG
// Fuction to print out a matrix, used for debugging
template <typename T> void PrintMat(T *mat, int rows = 16, int cols = 16) {
  std::cout << "====================\n";
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      if (typeid(T) == typeid(unsigned char))
        std::cout << (unsigned)mat[i * cols + j] << ", ";
      else if (typeid(T) == typeid(signed char))
        std::cout << (signed)mat[i * cols + j] << ", ";
      else if (typeid(T) == typeid(DirectX::PackedVector::HALF))
        std::cout << ConvertFloat16ToFloat32(
                         static_cast<DirectX::PackedVector::HALF>(
                             mat[i * cols + j]))
                  << ", ";
      else
        std::cout << mat[i * cols + j] << ", ";
    }
    std::cout << std::endl;
  }

  std::cout << "====================\n";
}
// Force instantion to enable calling of PrintMat from debugger
template void PrintMat<float>(float *mat, int rows, int cols);
template void PrintMat<int32_t>(int32_t *mat, int rows, int cols);
template void PrintMat<unsigned char>(unsigned char *mat, int rows, int cols);
template void PrintMat<signed char>(signed char *mat, int rows, int cols);
template void
PrintMat<DirectX::PackedVector::HALF>(DirectX::PackedVector::HALF *mat,
                                      int rows, int cols);
#endif

template <typename T>
void LoadStoreRowCol(int M, int N, bool LEFT, int MEM_TYPE, size_t start,
                     uint32_t alignmentOrGsharedOffset, uint32_t elementStride,
                     BYTE *src, BYTE *dst, bool testStore = false) {

  // For groupshared we repurpose the alignment arg to give coverage to store
  // offsets.
  if (testStore && MEM_TYPE == WMMA::GROUPSHARED) {
    dst += alignmentOrGsharedOffset;
  }

  // Left matrix is MxK, right is KxN
  uint32_t elementsToProcess = LEFT ? M : N;
  size_t bytesToLoadNextElement = testStore ? sizeof(T) : elementStride;
  size_t bytesToStoreNextElement = testStore ? elementStride : sizeof(T);

  // we don't test "start" param for stores because in the shaders
  // it's already tested because we store to an offset in the output buffer
  size_t curElementLoadOffset = (testStore ? 0 : start);
  size_t curElementStoreOffset = 0;

  for (size_t i = 0; i < elementsToProcess; ++i) {
    // This could just be a memcpy but not sure about the state of elementStride
    // yet which may be a parameter
    memcpy(dst + curElementStoreOffset, src + curElementLoadOffset, sizeof(T));

    curElementLoadOffset += bytesToLoadNextElement;
    curElementStoreOffset += bytesToStoreNextElement;
  }
}

template <typename T>
void LoadStoreMat(int M, int N, bool LEFT, int MEM_TYPE, uint32_t K, uint32_t k,
                  size_t start, uint32_t stride,
                  uint32_t alignmentOrGsharedOffset, bool transpose, BYTE *src,
                  BYTE *dst, bool testStore = false) {

  // For groupshared we repurpose the alignment arg to give coverage to store
  // offsets.
  if (testStore && MEM_TYPE == WMMA::GROUPSHARED) {
    dst += alignmentOrGsharedOffset;
  }

  const size_t lStride = (K * sizeof(T));
  const size_t rStride = (N * sizeof(T));
  size_t bytesToLoadNextElement = sizeof(T);
  size_t bytesToStoreNextElement = sizeof(T);

  uint32_t rows; // first dimension iterated over
  uint32_t cols; // second dimension iterated over
  size_t bytesToLoadNextArray;
  size_t bytesToStoreNextArray;

  if (LEFT) {
    // Left matrix is MxK, right is KxN
    rows = M;
    cols = k;

    if (testStore) // test storing to left matrix
    {
      bytesToLoadNextArray = lStride;
      bytesToStoreNextArray = stride;
    } else // test loading from left matrix
    {
      bytesToLoadNextArray = stride;
      bytesToStoreNextArray = lStride;
    }
  } else {
    // Right matrix is KxN
    rows = k;
    cols = N;

    if (testStore) // test storing to right matrix
    {
      bytesToLoadNextArray = rStride;
      bytesToStoreNextArray = stride;
    } else // test loading from right matrix
    {
      bytesToLoadNextArray = stride;
      bytesToStoreNextArray = rStride;
    }
  }

  if (transpose) {
    if (testStore) // we store transposed, but load normally
    {
      std::swap(bytesToStoreNextArray, bytesToStoreNextElement);
    } else // we load transposed, but store normally
    {
      std::swap(bytesToLoadNextArray, bytesToLoadNextElement);
    }
  }

  // we don't test "start" param for stores because in the shaders
  // it's already tested because we store to an offset in the output buffer
  size_t curArrayLoadOffset = (testStore ? 0 : start);
  size_t curArrayStoreOffset = 0;

  for (size_t i = 0; i < rows; ++i) {
    size_t curElementLoadOffset = curArrayLoadOffset;
    size_t curElementStoreOffset = curArrayStoreOffset;

    for (size_t j = 0; j < cols; ++j) {
      memcpy(dst + curElementStoreOffset, src + curElementLoadOffset,
             sizeof(T));

      // we load from and store to next column
      curElementLoadOffset += bytesToLoadNextElement;
      curElementStoreOffset += bytesToStoreNextElement;
    }

    curArrayLoadOffset += bytesToLoadNextArray;
    curArrayStoreOffset += bytesToStoreNextArray;
  }
}

TEST_F(ExecutionTest, DotTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice)) {
    return;
  }

  int tableSize = sizeof(DotOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(DotOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);

  std::vector<WEX::Common::String> *Validation_Input1 =
      &handler.GetTableParamByName(L"Validation.Input1")->m_StringTable;
  std::vector<WEX::Common::String> *Validation_Input2 =
      &handler.GetTableParamByName(L"Validation.Input2")->m_StringTable;
  std::vector<WEX::Common::String> *Validation_dot2 =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_StringTable;
  std::vector<WEX::Common::String> *Validation_dot3 =
      &handler.GetTableParamByName(L"Validation.Expected2")->m_StringTable;
  std::vector<WEX::Common::String> *Validation_dot4 =
      &handler.GetTableParamByName(L"Validation.Expected3")->m_StringTable;

  PCWSTR Validation_type =
      handler.GetTableParamByName(L"Validation.Type")->m_str;
  double tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_double;
  size_t count = Validation_Input1->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "DotOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SDotOp"));
        size_t size = sizeof(SDotOp) * count;
        Data.resize(size);
        SDotOp *pPrimitives = (SDotOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SDotOp *p = &pPrimitives[i];
          XMFLOAT4 val1, val2;
          VERIFY_SUCCEEDED(ParseDataToVectorFloat((*Validation_Input1)[i],
                                                  (float *)&val1, 4));
          VERIFY_SUCCEEDED(ParseDataToVectorFloat((*Validation_Input2)[i],
                                                  (float *)&val2, 4));
          p->input1 = val1;
          p->input2 = val2;
        }
        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SDotOp", &data);

  SDotOp *pPrimitives = (SDotOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  for (size_t i = 0; i < count; ++i) {
    SDotOp *p = &pPrimitives[i];
    float dot2, dot3, dot4;
    VERIFY_SUCCEEDED(ParseDataToFloat((*Validation_dot2)[i], dot2));
    VERIFY_SUCCEEDED(ParseDataToFloat((*Validation_dot3)[i], dot3));
    VERIFY_SUCCEEDED(ParseDataToFloat((*Validation_dot4)[i], dot4));
    LogCommentFmt(L"element #%u, input1 = (%f, %f, %f, %f), input2 = (%f, %f, "
                  L"%f, %f), \n dot2 = %f, dot2_expected = %f, dot3 = %f, "
                  L"dot3_expected = %f, dot4 = %f, dot4_expected = %f",
                  i, p->input1.x, p->input1.y, p->input1.z, p->input1.w,
                  p->input2.x, p->input2.y, p->input2.z, p->input2.w, p->o_dot2,
                  dot2, p->o_dot3, dot3, p->o_dot4, dot4);
    VerifyOutputWithExpectedValueFloat(p->o_dot2, dot2, Validation_type,
                                       tolerance);
    VerifyOutputWithExpectedValueFloat(p->o_dot3, dot3, Validation_type,
                                       tolerance);
    VerifyOutputWithExpectedValueFloat(p->o_dot4, dot4, Validation_type,
                                       tolerance);
  }
}

TEST_F(ExecutionTest, Dot2AddHalfTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_4, false)) {
    return;
  }

  if (!DoesDeviceSupportNative16bitOps(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support native 16-bit operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  int tableSize = sizeof(Dot2AddHalfOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(Dot2AddHalfOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);
  CW2A Arguments(handler.GetTableParamByName(L"ShaderOp.Arguments")->m_str);

  std::vector<WEX::Common::String> *validation_input1 =
      &handler.GetTableParamByName(L"Validation.Input1")->m_StringTable;
  std::vector<WEX::Common::String> *validation_input2 =
      &handler.GetTableParamByName(L"Validation.Input2")->m_StringTable;
  std::vector<float> *validation_acc =
      &handler.GetTableParamByName(L"Validation.Input3")->m_floatTable;
  std::vector<float> *validation_result =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_floatTable;

  PCWSTR Validation_type =
      handler.GetTableParamByName(L"Validation.Type")->m_str;
  double tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_double;
  size_t count = validation_input1->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "Dot2AddHalfOp",
      // this callback is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SDot2AddHalfOp"));
        size_t size = sizeof(SDot2AddHalfOp) * count;
        Data.resize(size);
        SDot2AddHalfOp *pPrimitives = (SDot2AddHalfOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SDot2AddHalfOp *p = &pPrimitives[i];
          Half2 val1, val2;
          VERIFY_SUCCEEDED(ParseDataToVectorHalf((*validation_input1)[i],
                                                 (uint16_t *)&val1, 2));
          VERIFY_SUCCEEDED(ParseDataToVectorHalf((*validation_input2)[i],
                                                 (uint16_t *)&val2, 2));
          p->input1 = val1;
          p->input2 = val2;
          p->acc = (*validation_acc)[i];
        }
        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
        pShaderOp->Shaders.at(0).Arguments = Arguments.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SDot2AddHalfOp", &data);

  SDot2AddHalfOp *pPrimitives = (SDot2AddHalfOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  for (size_t i = 0; i < count; ++i) {
    SDot2AddHalfOp *p = &pPrimitives[i];
    float expectedResult = (*validation_result)[i];
    float input1x = ConvertFloat16ToFloat32(p->input1.x);
    float input1y = ConvertFloat16ToFloat32(p->input1.y);
    float input2x = ConvertFloat16ToFloat32(p->input2.x);
    float input2y = ConvertFloat16ToFloat32(p->input2.y);
    LogCommentFmt(
        L"element #%u, input1 = (%f, %f), input2 = (%f, %f), acc = %f\n"
        L"result = %f, result_expected = %f",
        i, input1x, input1y, input2x, input2y, p->acc, p->result,
        expectedResult);
    VerifyOutputWithExpectedValueFloat(p->result, expectedResult,
                                       Validation_type, tolerance);
  }
}

TEST_F(ExecutionTest, Dot4AddI8PackedTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_4, false)) {
    return;
  }

  int tableSize = sizeof(Dot4AddI8PackedOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(Dot4AddI8PackedOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);

  std::vector<uint32_t> *validation_input1 =
      &handler.GetTableParamByName(L"Validation.Input1")->m_uint32Table;
  std::vector<uint32_t> *validation_input2 =
      &handler.GetTableParamByName(L"Validation.Input2")->m_uint32Table;
  std::vector<int32_t> *validation_acc =
      &handler.GetTableParamByName(L"Validation.Input3")->m_int32Table;
  std::vector<int32_t> *validation_result =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_int32Table;

  size_t count = validation_input1->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "Dot4AddI8PackedOp",
      // this callback is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SDot4AddI8PackedOp"));
        size_t size = sizeof(SDot4AddI8PackedOp) * count;
        Data.resize(size);
        SDot4AddI8PackedOp *pPrimitives = (SDot4AddI8PackedOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SDot4AddI8PackedOp *p = &pPrimitives[i];
          p->input1 = (*validation_input1)[i];
          p->input2 = (*validation_input2)[i];
          p->acc = (*validation_acc)[i];
        }
        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SDot4AddI8PackedOp", &data);

  SDot4AddI8PackedOp *pPrimitives = (SDot4AddI8PackedOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  for (size_t i = 0; i < count; ++i) {
    SDot4AddI8PackedOp *p = &pPrimitives[i];
    int32_t expectedResult = (*validation_result)[i];
    LogCommentFmt(L"element #%u, input1 = %u, input2 = %u, acc = %d \n"
                  L"result = %d, result_expected = %d",
                  i, p->input1, p->input2, p->acc, p->result, expectedResult);
    VerifyOutputWithExpectedValueInt(p->result, expectedResult, 0);
  }
}

TEST_F(ExecutionTest, Dot4AddU8PackedTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_4, false)) {
    return;
  }

  int tableSize = sizeof(Dot4AddU8PackedOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(Dot4AddU8PackedOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);

  std::vector<uint32_t> *validation_input1 =
      &handler.GetTableParamByName(L"Validation.Input1")->m_uint32Table;
  std::vector<uint32_t> *validation_input2 =
      &handler.GetTableParamByName(L"Validation.Input2")->m_uint32Table;
  std::vector<uint32_t> *validation_acc =
      &handler.GetTableParamByName(L"Validation.Input3")->m_uint32Table;
  std::vector<uint32_t> *validation_result =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_uint32Table;

  size_t count = validation_input1->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "Dot4AddU8PackedOp",
      // this callback is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SDot4AddU8PackedOp"));
        size_t size = sizeof(SDot4AddU8PackedOp) * count;
        Data.resize(size);
        SDot4AddU8PackedOp *pPrimitives = (SDot4AddU8PackedOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SDot4AddU8PackedOp *p = &pPrimitives[i];
          p->input1 = (*validation_input1)[i];
          p->input2 = (*validation_input2)[i];
          p->acc = (*validation_acc)[i];
        }
        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SDot4AddU8PackedOp", &data);

  SDot4AddU8PackedOp *pPrimitives = (SDot4AddU8PackedOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  for (size_t i = 0; i < count; ++i) {
    SDot4AddU8PackedOp *p = &pPrimitives[i];
    uint32_t expectedResult = (*validation_result)[i];
    LogCommentFmt(L"element #%u, input1 = %u, input2 = %u, acc = %u \n"
                  L"result = %u, result_expected = %u, ",
                  i, p->input1, p->input2, p->acc, p->result, expectedResult);
    VerifyOutputWithExpectedValueUInt(p->result, expectedResult, 0);
  }
}

TEST_F(ExecutionTest, Msad4Test) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice)) {
    return;
  }
  size_t tableSize = sizeof(Msad4OpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(Msad4OpParameters, tableSize);

  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);
  double tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_double;

  std::vector<unsigned int> *Validation_Reference =
      &handler.GetTableParamByName(L"Validation.Input1")->m_uint32Table;
  std::vector<WEX::Common::String> *Validation_Source =
      &handler.GetTableParamByName(L"Validation.Input2")->m_StringTable;
  std::vector<WEX::Common::String> *Validation_Accum =
      &handler.GetTableParamByName(L"Validation.Input3")->m_StringTable;
  std::vector<WEX::Common::String> *Validation_Expected =
      &handler.GetTableParamByName(L"Validation.Expected1")->m_StringTable;

  size_t count = Validation_Expected->size();

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "Msad4",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SMsad4"));
        size_t size = sizeof(SMsad4) * count;
        Data.resize(size);
        SMsad4 *pPrimitives = (SMsad4 *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SMsad4 *p = &pPrimitives[i];
          XMUINT2 src;
          XMUINT4 accum;
          VERIFY_SUCCEEDED(ParseDataToVectorUint((*Validation_Source)[i],
                                                 (unsigned int *)&src, 2));
          VERIFY_SUCCEEDED(ParseDataToVectorUint((*Validation_Accum)[i],
                                                 (unsigned int *)&accum, 4));
          p->ref = (*Validation_Reference)[i];
          p->src = src;
          p->accum = accum;
        }
        // use shader from data table
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SMsad4", &data);

  SMsad4 *pPrimitives = (SMsad4 *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;
  for (size_t i = 0; i < count; ++i) {
    SMsad4 *p = &pPrimitives[i];
    XMUINT4 result;
    VERIFY_SUCCEEDED(ParseDataToVectorUint((*Validation_Expected)[i],
                                           (unsigned int *)&result, 4));
    LogCommentFmt(
        L"element #%u, ref = %u(0x%08x), src = %u(0x%08x), %u(0x%08x), "
        L"accum = %u(0x%08x), %u(0x%08x), %u(0x%08x), %u(0x%08x),\n"
        L"result = %u(0x%08x), %u(0x%08x), %u(0x%08x), %u(0x%08x),\n"
        L"expected = %u(0x%08x), %u(0x%08x), %u(0x%08x), %u(0x%08x)",
        i, p->ref, p->ref, p->src.x, p->src.x, p->src.y, p->src.y, p->accum.x,
        p->accum.x, p->accum.y, p->accum.y, p->accum.z, p->accum.z, p->accum.w,
        p->accum.w, p->result.x, p->result.x, p->result.y, p->result.y,
        p->result.z, p->result.z, p->result.w, p->result.w, result.x, result.x,
        result.y, result.y, result.z, result.z, result.w, result.w);

    int toleranceInt = (int)tolerance;
    VerifyOutputWithExpectedValueInt(p->result.x, result.x, toleranceInt);
    VerifyOutputWithExpectedValueInt(p->result.y, result.y, toleranceInt);
    VerifyOutputWithExpectedValueInt(p->result.z, result.z, toleranceInt);
    VerifyOutputWithExpectedValueInt(p->result.w, result.w, toleranceInt);
  }
}

TEST_F(ExecutionTest, DenormBinaryFloatOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_2)) {
    return;
  }

  // Read data from the table
  int tableSize = sizeof(DenormBinaryFPOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(DenormBinaryFPOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);
  CW2A Arguments(handler.GetTableParamByName(L"ShaderOp.Arguments")->m_str);

  std::vector<WEX::Common::String> *Validation_Input1 =
      &(handler.GetTableParamByName(L"Validation.Input1")->m_StringTable);
  std::vector<WEX::Common::String> *Validation_Input2 =
      &(handler.GetTableParamByName(L"Validation.Input2")->m_StringTable);

  std::vector<WEX::Common::String> *Validation_Expected1 =
      &(handler.GetTableParamByName(L"Validation.Expected1")->m_StringTable);
  // two expected outputs for any mode
  std::vector<WEX::Common::String> *Validation_Expected2 =
      &(handler.GetTableParamByName(L"Validation.Expected2")->m_StringTable);

  LPCWSTR Validation_Type =
      handler.GetTableParamByName(L"Validation.Type")->m_str;
  double Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_double;
  size_t count = Validation_Input1->size();

  using namespace hlsl::DXIL;
  Float32DenormMode mode = Float32DenormMode::Any;
  if (strcmp(Arguments.m_psz, "-denorm preserve") == 0) {
    mode = Float32DenormMode::Preserve;
  } else if (strcmp(Arguments.m_psz, "-denorm ftz") == 0) {
    mode = Float32DenormMode::FTZ;
  }
  if (mode == Float32DenormMode::Any) {
    DXASSERT(Validation_Expected2->size() == Validation_Expected1->size(),
             "must have same number of expected values");
  }

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "BinaryFPOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "SBinaryFPOp"));
        size_t size = sizeof(SBinaryFPOp) * count;
        Data.resize(size);
        SBinaryFPOp *pPrimitives = (SBinaryFPOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          SBinaryFPOp *p = &pPrimitives[i];
          PCWSTR str1 = (*Validation_Input1)[i % Validation_Input1->size()];
          PCWSTR str2 = (*Validation_Input2)[i % Validation_Input2->size()];
          float val1, val2;
          VERIFY_SUCCEEDED(ParseDataToFloat(str1, val1));
          VERIFY_SUCCEEDED(ParseDataToFloat(str2, val2));
          p->input1 = val1;
          p->input2 = val2;
        }

        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
        pShaderOp->Shaders.at(0).Arguments = Arguments.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("SBinaryFPOp", &data);

  SBinaryFPOp *pPrimitives = (SBinaryFPOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;

  for (unsigned i = 0; i < count; ++i) {
    SBinaryFPOp *p = &pPrimitives[i];
    if (mode == Float32DenormMode::Any) {
      LPCWSTR str1 = (*Validation_Expected1)[i % Validation_Expected1->size()];
      LPCWSTR str2 = (*Validation_Expected2)[i % Validation_Expected2->size()];
      float val1;
      float val2;
      VERIFY_SUCCEEDED(ParseDataToFloat(str1, val1));
      VERIFY_SUCCEEDED(ParseDataToFloat(str2, val2));
      LogCommentFmt(L"element #%u, input1 = %6.8f, input2 = %6.8f, output = "
                    L"%6.8f, expected = %6.8f(%x) or %6.8f(%x)",
                    i, p->input1, p->input2, p->output1, val1, *(int *)&val1,
                    val2, *(int *)&val2);
      VERIFY_IS_TRUE(
          CompareOutputWithExpectedValueFloat(p->output1, val1, Validation_Type,
                                              Validation_Tolerance, mode) ||
          CompareOutputWithExpectedValueFloat(p->output1, val2, Validation_Type,
                                              Validation_Tolerance, mode));
    } else {
      LPCWSTR str1 = (*Validation_Expected1)[i % Validation_Expected1->size()];
      float val1;
      VERIFY_SUCCEEDED(ParseDataToFloat(str1, val1));
      LogCommentFmt(L"element #%u, input1 = %6.8f, input2 = %6.8f, output = "
                    L"%6.8f, expected = %6.8f(%a)",
                    i, p->input1, p->input2, p->output1, val1, *(int *)&val1);
      VerifyOutputWithExpectedValueFloat(p->output1, val1, Validation_Type,
                                         Validation_Tolerance, mode);
    }
  }
}

TEST_F(ExecutionTest, DenormTertiaryFloatOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_2)) {
    return;
  }

  // Read data from the table
  int tableSize = sizeof(DenormTertiaryFPOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(DenormTertiaryFPOpParameters, tableSize);

  CW2A Target(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);
  CW2A Arguments(handler.GetTableParamByName(L"ShaderOp.Arguments")->m_str);

  std::vector<WEX::Common::String> *Validation_Input1 =
      &(handler.GetTableParamByName(L"Validation.Input1")->m_StringTable);
  std::vector<WEX::Common::String> *Validation_Input2 =
      &(handler.GetTableParamByName(L"Validation.Input2")->m_StringTable);
  std::vector<WEX::Common::String> *Validation_Input3 =
      &(handler.GetTableParamByName(L"Validation.Input3")->m_StringTable);

  std::vector<WEX::Common::String> *Validation_Expected1 =
      &(handler.GetTableParamByName(L"Validation.Expected1")->m_StringTable);

  // two expected outputs for any mode
  std::vector<WEX::Common::String> *Validation_Expected2 =
      &(handler.GetTableParamByName(L"Validation.Expected2")->m_StringTable);
  LPCWSTR Validation_Type =
      handler.GetTableParamByName(L"Validation.Type")->m_str;
  double Validation_Tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_double;
  size_t count = Validation_Input1->size();

  using namespace hlsl::DXIL;
  Float32DenormMode mode = Float32DenormMode::Any;
  if (strcmp(Arguments.m_psz, "-denorm preserve") == 0) {
    mode = Float32DenormMode::Preserve;
  } else if (strcmp(Arguments.m_psz, "-denorm ftz") == 0) {
    mode = Float32DenormMode::FTZ;
  }
  if (mode == Float32DenormMode::Any) {
    DXASSERT(Validation_Expected2->size() == Validation_Expected1->size(),
             "must have same number of expected values");
  }

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "TertiaryFPOp",
      // this callbacked is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(0 == _stricmp(Name, "STertiaryFPOp"));
        size_t size = sizeof(STertiaryFPOp) * count;
        Data.resize(size);
        STertiaryFPOp *pPrimitives = (STertiaryFPOp *)Data.data();
        for (size_t i = 0; i < count; ++i) {
          STertiaryFPOp *p = &pPrimitives[i];
          PCWSTR str1 = (*Validation_Input1)[i % Validation_Input1->size()];
          PCWSTR str2 = (*Validation_Input2)[i % Validation_Input2->size()];
          PCWSTR str3 = (*Validation_Input3)[i % Validation_Input3->size()];
          float val1, val2, val3;
          VERIFY_SUCCEEDED(ParseDataToFloat(str1, val1));
          VERIFY_SUCCEEDED(ParseDataToFloat(str2, val2));
          VERIFY_SUCCEEDED(ParseDataToFloat(str3, val3));
          p->input1 = val1;
          p->input2 = val2;
          p->input3 = val3;
        }

        // use shader from data table
        pShaderOp->Shaders.at(0).Target = Target.m_psz;
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
        pShaderOp->Shaders.at(0).Arguments = Arguments.m_psz;
      });

  MappedData data;
  test->Test->GetReadBackData("STertiaryFPOp", &data);

  STertiaryFPOp *pPrimitives = (STertiaryFPOp *)data.data();
  WEX::TestExecution::DisableVerifyExceptions dve;

  for (unsigned i = 0; i < count; ++i) {
    STertiaryFPOp *p = &pPrimitives[i];
    if (mode == Float32DenormMode::Any) {
      LPCWSTR str1 = (*Validation_Expected1)[i % Validation_Expected1->size()];
      LPCWSTR str2 = (*Validation_Expected2)[i % Validation_Expected2->size()];
      float val1;
      float val2;
      VERIFY_SUCCEEDED(ParseDataToFloat(str1, val1));
      VERIFY_SUCCEEDED(ParseDataToFloat(str2, val2));
      LogCommentFmt(L"element #%u, input1 = %6.8f, input2 = %6.8f, input3 = "
                    L"%6.8f, output = "
                    L"%6.8f, expected = %6.8f(%x) or %6.8f(%x)",
                    i, p->input1, p->input2, p->input3, p->output, val1,
                    *(int *)&val1, val2, *(int *)&val2);
      VERIFY_IS_TRUE(
          CompareOutputWithExpectedValueFloat(p->output, val1, Validation_Type,
                                              Validation_Tolerance, mode) ||
          CompareOutputWithExpectedValueFloat(p->output, val2, Validation_Type,
                                              Validation_Tolerance, mode));
    } else {
      LPCWSTR str1 = (*Validation_Expected1)[i % Validation_Expected1->size()];
      float val1;
      VERIFY_SUCCEEDED(ParseDataToFloat(str1, val1));
      LogCommentFmt(L"element #%u, input1 = %6.8f, input2 = %6.8f, input3 = "
                    L"%6.8f, output = "
                    L"%6.8f, expected = %6.8f(%a)",
                    i, p->input1, p->input2, p->input3, p->output, val1,
                    *(int *)&val1);
      VerifyOutputWithExpectedValueFloat(p->output, val1, Validation_Type,
                                         Validation_Tolerance, mode);
    }
  }
}

// Setup for wave intrinsics tests
enum class ShaderOpKind {
  WaveSum,
  WaveProduct,
  WaveActiveMax,
  WaveActiveMin,
  WaveCountBits,
  WaveActiveAllEqual,
  WaveActiveAnyTrue,
  WaveActiveAllTrue,
  WaveActiveBitOr,
  WaveActiveBitAnd,
  WaveActiveBitXor,
  ShaderOpInvalid
};

struct ShaderOpKindPair {
  LPCWSTR name;
  ShaderOpKind kind;
};

static ShaderOpKindPair ShaderOpKindTable[] = {
    {L"WaveActiveSum", ShaderOpKind::WaveSum},
    {L"WaveActiveUSum", ShaderOpKind::WaveSum},
    {L"WaveActiveProduct", ShaderOpKind::WaveProduct},
    {L"WaveActiveUProduct", ShaderOpKind::WaveProduct},
    {L"WaveActiveMax", ShaderOpKind::WaveActiveMax},
    {L"WaveActiveUMax", ShaderOpKind::WaveActiveMax},
    {L"WaveActiveMin", ShaderOpKind::WaveActiveMin},
    {L"WaveActiveUMin", ShaderOpKind::WaveActiveMin},
    {L"WaveActiveCountBits", ShaderOpKind::WaveCountBits},
    {L"WaveActiveAllEqual", ShaderOpKind::WaveActiveAllEqual},
    {L"WaveActiveAnyTrue", ShaderOpKind::WaveActiveAnyTrue},
    {L"WaveActiveAllTrue", ShaderOpKind::WaveActiveAllTrue},
    {L"WaveActiveBitOr", ShaderOpKind::WaveActiveBitOr},
    {L"WaveActiveBitAnd", ShaderOpKind::WaveActiveBitAnd},
    {L"WaveActiveBitXor", ShaderOpKind::WaveActiveBitXor},
    {L"WavePrefixSum", ShaderOpKind::WaveSum},
    {L"WavePrefixUSum", ShaderOpKind::WaveSum},
    {L"WavePrefixProduct", ShaderOpKind::WaveProduct},
    {L"WavePrefixUProduct", ShaderOpKind::WaveProduct},
    {L"WavePrefixMax", ShaderOpKind::WaveActiveMax},
    {L"WavePrefixUMax", ShaderOpKind::WaveActiveMax},
    {L"WavePrefixMin", ShaderOpKind::WaveActiveMin},
    {L"WavePrefixUMin", ShaderOpKind::WaveActiveMin},
    {L"WavePrefixCountBits", ShaderOpKind::WaveCountBits}};

ShaderOpKind GetShaderOpKind(LPCWSTR str) {
  for (size_t i = 0; i < sizeof(ShaderOpKindTable) / sizeof(ShaderOpKindPair);
       ++i) {
    if (_wcsicmp(ShaderOpKindTable[i].name, str) == 0) {
      return ShaderOpKindTable[i].kind;
    }
  }
  DXASSERT_ARGS(false, "Invalid ShaderOp name: %s", str);
  return ShaderOpKind::ShaderOpInvalid;
}

template <typename InType, typename OutType, ShaderOpKind kind>
struct computeExpected {
  OutType operator()(const std::vector<InType> &inputs,
                     const std::vector<int> &masks, int maskValue,
                     unsigned int index) {
    return 0;
  }
};

template <typename InType, typename OutType>
struct computeExpected<InType, OutType, ShaderOpKind::WaveSum> {
  OutType operator()(const std::vector<InType> &inputs,
                     const std::vector<int> &masks, int maskValue,
                     unsigned int index) {
    OutType sum = 0;
    for (size_t i = 0; i < index; ++i) {
      if (masks.at(i) == maskValue) {
        sum += inputs.at(i);
      }
    }
    return sum;
  }
};

template <typename InType, typename OutType>
struct computeExpected<InType, OutType, ShaderOpKind::WaveProduct> {
  OutType operator()(const std::vector<InType> &inputs,
                     const std::vector<int> &masks, int maskValue,
                     unsigned int index) {
    OutType prod = 1;
    for (size_t i = 0; i < index; ++i) {
      if (masks.at(i) == maskValue) {
        prod *= inputs.at(i);
      }
    }
    return prod;
  }
};

template <typename InType, typename OutType>
struct computeExpected<InType, OutType, ShaderOpKind::WaveActiveMax> {
  OutType operator()(const std::vector<InType> &inputs,
                     const std::vector<int> &masks, int maskValue,
                     unsigned int index) {
    OutType maximum = std::numeric_limits<OutType>::min();
    for (size_t i = 0; i < index; ++i) {
      if (masks.at(i) == maskValue && inputs.at(i) > maximum)
        maximum = inputs.at(i);
    }
    return maximum;
  }
};

template <typename InType, typename OutType>
struct computeExpected<InType, OutType, ShaderOpKind::WaveActiveMin> {
  OutType operator()(const std::vector<InType> &inputs,
                     const std::vector<int> &masks, int maskValue,
                     unsigned int index) {
    OutType minimum = std::numeric_limits<OutType>::max();
    for (size_t i = 0; i < index; ++i) {
      if (masks.at(i) == maskValue && inputs.at(i) < minimum)
        minimum = inputs.at(i);
    }
    return minimum;
  }
};

template <typename InType, typename OutType>
struct computeExpected<InType, OutType, ShaderOpKind::WaveCountBits> {
  OutType operator()(const std::vector<InType> &inputs,
                     const std::vector<int> &masks, int maskValue,
                     unsigned int index) {
    OutType count = 0;
    for (size_t i = 0; i < index; ++i) {
      if (masks.at(i) == maskValue && inputs.at(i) > 3) {
        count++;
      }
    }
    return count;
  }
};

// In HLSL, boolean is represented in a 4 byte (uint32) format,
// So we cannot use c++ bool type to represent bool in HLSL
// HLSL returns 0 for false and 1 for true
template <typename InType, typename OutType>
struct computeExpected<InType, OutType, ShaderOpKind::WaveActiveAnyTrue> {
  OutType operator()(const std::vector<InType> &inputs,
                     const std::vector<int> &masks, int maskValue,
                     unsigned int index) {
    for (size_t i = 0; i < index; ++i) {
      if (masks.at(i) == maskValue && inputs.at(i) != 0) {
        return 1;
      }
    }
    return 0;
  }
};

template <typename InType, typename OutType>
struct computeExpected<InType, OutType, ShaderOpKind::WaveActiveAllTrue> {
  OutType operator()(const std::vector<InType> &inputs,
                     const std::vector<int> &masks, int maskValue,
                     unsigned int index) {
    for (size_t i = 0; i < index; ++i) {
      if (masks.at(i) == maskValue && inputs.at(i) == 0) {
        return 0;
      }
    }
    return 1;
  }
};

template <typename InType, typename OutType>
struct computeExpected<InType, OutType, ShaderOpKind::WaveActiveAllEqual> {
  OutType operator()(const std::vector<InType> &inputs,
                     const std::vector<int> &masks, int maskValue,
                     unsigned int index) {
    const InType *val = nullptr;
    for (size_t i = 0; i < index; ++i) {
      if (masks.at(i) == maskValue) {
        if (val && *val != inputs.at(i)) {
          return 0;
        }
        val = &inputs.at(i);
      }
    }
    return 1;
  }
};

template <typename InType, typename OutType>
struct computeExpected<InType, OutType, ShaderOpKind::WaveActiveBitOr> {
  OutType operator()(const std::vector<InType> &inputs,
                     const std::vector<int> &masks, int maskValue,
                     unsigned int index) {
    OutType bits = 0x00000000;
    for (size_t i = 0; i < index; ++i) {
      if (masks.at(i) == maskValue) {
        bits |= inputs.at(i);
      }
    }
    return bits;
  }
};

template <typename InType, typename OutType>
struct computeExpected<InType, OutType, ShaderOpKind::WaveActiveBitAnd> {
  OutType operator()(const std::vector<InType> &inputs,
                     const std::vector<int> &masks, int maskValue,
                     unsigned int index) {
    OutType bits = 0xffffffff;
    for (size_t i = 0; i < index; ++i) {
      if (masks.at(i) == maskValue) {
        bits &= inputs.at(i);
      }
    }
    return bits;
  }
};

template <typename InType, typename OutType>
struct computeExpected<InType, OutType, ShaderOpKind::WaveActiveBitXor> {
  OutType operator()(const std::vector<InType> &inputs,
                     const std::vector<int> &masks, int maskValue,
                     unsigned int index) {
    OutType bits = 0x00000000;
    for (size_t i = 0; i < index; ++i) {
      if (masks.at(i) == maskValue) {
        bits ^= inputs.at(i);
      }
    }
    return bits;
  }
};

// Mask functions used to control active lanes
static int MaskAll(int i) {
  UNREFERENCED_PARAMETER(i);
  return 1;
}

static int MaskEveryOther(int i) { return i % 2 == 0 ? 1 : 0; }

static int MaskEveryThird(int i) { return i % 3 == 0 ? 1 : 0; }

typedef int (*MaskFunction)(int);
static MaskFunction MaskFunctionTable[] = {MaskAll, MaskEveryOther,
                                           MaskEveryThird};

template <typename InType, typename OutType>
static OutType computeExpectedWithShaderOp(const std::vector<InType> &inputs,
                                           const std::vector<int> &masks,
                                           int maskValue, unsigned int index,
                                           LPCWSTR str) {
  ShaderOpKind kind = GetShaderOpKind(str);
  switch (kind) {
  case ShaderOpKind::WaveSum:
    return computeExpected<InType, OutType, ShaderOpKind::WaveSum>()(
        inputs, masks, maskValue, index);
  case ShaderOpKind::WaveProduct:
    return computeExpected<InType, OutType, ShaderOpKind::WaveProduct>()(
        inputs, masks, maskValue, index);
  case ShaderOpKind::WaveActiveMax:
    return computeExpected<InType, OutType, ShaderOpKind::WaveActiveMax>()(
        inputs, masks, maskValue, index);
  case ShaderOpKind::WaveActiveMin:
    return computeExpected<InType, OutType, ShaderOpKind::WaveActiveMin>()(
        inputs, masks, maskValue, index);
  case ShaderOpKind::WaveCountBits:
    return computeExpected<InType, OutType, ShaderOpKind::WaveCountBits>()(
        inputs, masks, maskValue, index);
  case ShaderOpKind::WaveActiveBitOr:
    return computeExpected<InType, OutType, ShaderOpKind::WaveActiveBitOr>()(
        inputs, masks, maskValue, index);
  case ShaderOpKind::WaveActiveBitAnd:
    return computeExpected<InType, OutType, ShaderOpKind::WaveActiveBitAnd>()(
        inputs, masks, maskValue, index);
  case ShaderOpKind::WaveActiveBitXor:
    return computeExpected<InType, OutType, ShaderOpKind::WaveActiveBitXor>()(
        inputs, masks, maskValue, index);
  case ShaderOpKind::WaveActiveAnyTrue:
    return computeExpected<InType, OutType, ShaderOpKind::WaveActiveAnyTrue>()(
        inputs, masks, maskValue, index);
  case ShaderOpKind::WaveActiveAllTrue:
    return computeExpected<InType, OutType, ShaderOpKind::WaveActiveAllTrue>()(
        inputs, masks, maskValue, index);
  case ShaderOpKind::WaveActiveAllEqual:
    return computeExpected<InType, OutType, ShaderOpKind::WaveActiveAllEqual>()(
        inputs, masks, maskValue, index);
  default:
    DXASSERT_ARGS(false, "Invalid ShaderOp Name: %s", str);
    return (OutType)0;
  }
};

// A framework for testing individual wave intrinsics tests.
// This test case is assuming that functions 1) WaveIsFirstLane and 2)
// WaveGetLaneIndex are correct for all lanes.
template <class T1, class T2>
void ExecutionTest::WaveIntrinsicsActivePrefixTest(
    TableParameter *pParameterList, size_t numParameter, bool isPrefix) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  // Resource representation for compute shader
  // firstLaneId is used to group different waves
  // laneIndex is used to identify lane within the wave.
  // Lane ids are not necessarily in same order as thread ids.
  struct PerThreadData {
    unsigned firstLaneId;
    unsigned laneIndex;
    int mask;
    T1 input;
    T2 output;
  };

  unsigned int NumThreadsX = 8;
  unsigned int NumThreadsY = 12;
  unsigned int NumThreadsZ = 1;

  static const unsigned int ThreadsPerGroup =
      NumThreadsX * NumThreadsY * NumThreadsZ;
  static const unsigned int DispatchGroupCount = 1;
  static const unsigned int ThreadCount = ThreadsPerGroup * DispatchGroupCount;
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice)) {
    return;
  }
  if (!DoesDeviceSupportWaveOps(pDevice)) {
    // Optional feature, so it's correct to not support it if declared as such.
    WEX::Logging::Log::Comment(L"Device does not support wave operations.");
    return;
  }

  TableParameterHandler handler(pParameterList, numParameter);

  unsigned int numInputSet =
      handler.GetTableParamByName(L"Validation.NumInputSet")->m_uint;

  // Obtain the list of input lists
  std::vector<std::vector<T1> *> InputDataList;
  for (unsigned int i = 0; i < numInputSet; ++i) {
    std::wstring inputName = L"Validation.InputSet";
    inputName.append(std::to_wstring(i + 1));
    InputDataList.push_back(handler.GetDataArray<T1>(inputName.data()));
  }
  CW2A Text(handler.GetTableParamByName(L"ShaderOp.text")->m_str);

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());

  // Running compute shader for each input set with different masks
  for (size_t setIndex = 0; setIndex < numInputSet; ++setIndex) {
    for (size_t maskIndex = 0;
         maskIndex < sizeof(MaskFunctionTable) / sizeof(MaskFunction);
         ++maskIndex) {
      std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
          pDevice, m_support, "WaveIntrinsicsOp",
          // this callbacked is called when the test
          // is creating the resource to run the test
          [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
            VERIFY_IS_TRUE(0 == _stricmp(Name, "SWaveIntrinsicsOp"));
            size_t size = sizeof(PerThreadData) * ThreadCount;
            Data.resize(size);
            PerThreadData *pPrimitives = (PerThreadData *)Data.data();
            // 4 different inputs for each operation test
            size_t index = 0;
            std::vector<T1> *IntList = InputDataList[setIndex];
            while (index < ThreadCount) {
              PerThreadData *p = &pPrimitives[index];
              p->firstLaneId = 0xFFFFBFFF;
              p->laneIndex = 0xFFFFBFFF;
              p->mask = MaskFunctionTable[maskIndex]((int)index);
              p->input = (*IntList)[index % IntList->size()];
              p->output = 0xFFFFBFFF;
              index++;
            }
            // use shader from data table
            pShaderOp->Shaders.at(0).Text = Text.m_psz;
          },
          ShaderOpSet);

      // Check the value
      MappedData data;
      test->Test->GetReadBackData("SWaveIntrinsicsOp", &data);

      PerThreadData *pPrimitives = (PerThreadData *)data.data();
      WEX::TestExecution::DisableVerifyExceptions dve;

      // Grouping data by waves
      std::vector<int> firstLaneIds;
      for (size_t i = 0; i < ThreadCount; ++i) {
        PerThreadData *p = &pPrimitives[i];
        int firstLaneId = p->firstLaneId;
        if (!contains(firstLaneIds, firstLaneId)) {
          firstLaneIds.push_back(firstLaneId);
        }
      }

      std::map<int, std::unique_ptr<std::vector<PerThreadData *>>> waves;
      for (size_t i = 0; i < firstLaneIds.size(); ++i) {
        waves[firstLaneIds.at(i)] =
            std::make_unique<std::vector<PerThreadData *>>();
      }

      for (size_t i = 0; i < ThreadCount; ++i) {
        PerThreadData *p = &pPrimitives[i];
        waves[p->firstLaneId].get()->push_back(p);
      }

      // validate for each wave
      for (size_t i = 0; i < firstLaneIds.size(); ++i) {
        // collect inputs and masks for a given wave
        std::vector<PerThreadData *> *waveData =
            waves[firstLaneIds.at(i)].get();
        std::vector<T1> inputList(waveData->size());
        std::vector<int> maskList(waveData->size(), -1);
        std::vector<T2> outputList(waveData->size());
        // sort inputList and masklist by lane id. input for each lane can be
        // computed for its group index
        for (size_t j = 0, end = waveData->size(); j < end; ++j) {
          unsigned laneID = waveData->at(j)->laneIndex;
          // ensure that each lane ID is unique and within the range
          VERIFY_IS_TRUE(0 <= laneID && laneID < waveData->size());
          VERIFY_IS_TRUE(maskList.at(laneID) == -1);
          maskList.at(laneID) = waveData->at(j)->mask;
          inputList.at(laneID) = waveData->at(j)->input;
          outputList.at(laneID) = waveData->at(j)->output;
        }
        std::wstring inputStr = L"Wave Inputs:  ";
        std::wstring maskStr = L"Wave Masks:   ";
        std::wstring outputStr = L"Wave Outputs: ";
        // append input string and mask string in lane id order
        for (size_t j = 0, end = waveData->size(); j < end; ++j) {
          maskStr.append(std::to_wstring(maskList.at(j)));
          maskStr.append(L" ");
          inputStr.append(std::to_wstring(inputList.at(j)));
          inputStr.append(L" ");
          outputStr.append(std::to_wstring(outputList.at(j)));
          outputStr.append(L" ");
        }

        LogCommentFmt(inputStr.data());
        LogCommentFmt(maskStr.data());
        LogCommentFmt(outputStr.data());
        LogCommentFmt(L"\n");
        // Compute expected output for a given inputs, masks, and index
        for (size_t laneIndex = 0, laneEnd = inputList.size();
             laneIndex < laneEnd; ++laneIndex) {
          T2 expected;
          // WaveActive is equivalent to WavePrefix lane # lane count
          unsigned index =
              isPrefix ? (unsigned)laneIndex : (unsigned)inputList.size();
          if (maskList.at(laneIndex) == 1) {
            expected = computeExpectedWithShaderOp<T1, T2>(
                inputList, maskList, 1, index,
                handler.GetTableParamByName(L"ShaderOp.Name")->m_str);
          } else {
            expected = computeExpectedWithShaderOp<T1, T2>(
                inputList, maskList, 0, index,
                handler.GetTableParamByName(L"ShaderOp.Name")->m_str);
          }
          // TODO: use different comparison for floating point inputs
          bool equal = outputList.at(laneIndex) == expected;
          if (!equal) {
            LogCommentFmt(L"lane%d: %4d, Expected : %4d", laneIndex,
                          outputList.at(laneIndex), expected);
          }
          VERIFY_IS_TRUE(equal);
        }
      }
    }
  }
}

TEST_F(ExecutionTest, WaveIntrinsicsActiveIntTest) {
  WaveIntrinsicsActivePrefixTest<int, int>(
      WaveIntrinsicsActiveIntParameters,
      sizeof(WaveIntrinsicsActiveIntParameters) / sizeof(TableParameter),
      /*isPrefix*/ false);
}

TEST_F(ExecutionTest, WaveIntrinsicsActiveUintTest) {
  WaveIntrinsicsActivePrefixTest<unsigned int, unsigned int>(
      WaveIntrinsicsActiveUintParameters,
      sizeof(WaveIntrinsicsActiveUintParameters) / sizeof(TableParameter),
      /*isPrefix*/ false);
}

TEST_F(ExecutionTest, WaveIntrinsicsPrefixIntTest) {
  WaveIntrinsicsActivePrefixTest<int, int>(
      WaveIntrinsicsPrefixIntParameters,
      sizeof(WaveIntrinsicsPrefixIntParameters) / sizeof(TableParameter),
      /*isPrefix*/ true);
}

TEST_F(ExecutionTest, WaveIntrinsicsPrefixUintTest) {
  WaveIntrinsicsActivePrefixTest<unsigned int, unsigned int>(
      WaveIntrinsicsPrefixUintParameters,
      sizeof(WaveIntrinsicsPrefixUintParameters) / sizeof(TableParameter),
      /*isPrefix*/ true);
}

template <typename T>
static T GetWaveMultiPrefixInitialAccumValue(LPCWSTR testName) {
  if (_wcsicmp(testName, L"WaveMultiPrefixProduct") == 0 ||
      _wcsicmp(testName, L"WaveMultiPrefixUProduct") == 0) {
    return static_cast<T>(1);
  } else if (_wcsicmp(testName, L"WaveMultiPrefixSum") == 0 ||
             _wcsicmp(testName, L"WaveMultiPrefixUSum") == 0 ||
             _wcsicmp(testName, L"WaveMultiPrefixBitOr") == 0 ||
             _wcsicmp(testName, L"WaveMultiPrefixUBitOr") == 0 ||
             _wcsicmp(testName, L"WaveMultiPrefixBitXor") == 0 ||
             _wcsicmp(testName, L"WaveMultiPrefixUBitXor") == 0 ||
             _wcsicmp(testName, L"WaveMultiPrefixCountBits") == 0 ||
             _wcsicmp(testName, L"WaveMultiPrefixUCountBits") == 0) {
    return static_cast<T>(0);
  } else if (_wcsicmp(testName, L"WaveMultiPrefixBitAnd") == 0 ||
             _wcsicmp(testName, L"WaveMultiPrefixUBitAnd") == 0) {
    return static_cast<T>(-1);
  } else {
    return static_cast<T>(0);
  }
}

template <typename T>
std::function<T(T, T)> GetWaveMultiPrefixReferenceFunction(LPCWSTR testName) {
  if (_wcsicmp(testName, L"WaveMultiPrefixProduct") == 0 ||
      _wcsicmp(testName, L"WaveMultiPrefixUProduct") == 0) {
    return [](T lhs, T rhs) -> T { return lhs * rhs; };
  } else if (_wcsicmp(testName, L"WaveMultiPrefixSum") == 0 ||
             _wcsicmp(testName, L"WaveMultiPrefixUSum") == 0) {
    return [](T lhs, T rhs) -> T { return lhs + rhs; };
  } else if (_wcsicmp(testName, L"WaveMultiPrefixBitAnd") == 0 ||
             _wcsicmp(testName, L"WaveMultiPrefixUBitAnd") == 0) {
    return [](T lhs, T rhs) -> T { return lhs & rhs; };
  } else if (_wcsicmp(testName, L"WaveMultiPrefixBitOr") == 0 ||
             _wcsicmp(testName, L"WaveMultiPrefixUBitOr") == 0) {
    return [](T lhs, T rhs) -> T { return lhs | rhs; };
  } else if (_wcsicmp(testName, L"WaveMultiPrefixBitXor") == 0 ||
             _wcsicmp(testName, L"WaveMultiPrefixUBitXor") == 0) {
    return [](T lhs, T rhs) -> T { return lhs ^ rhs; };
  } else if (_wcsicmp(testName, L"WaveMultiPrefixCountBits") == 0 ||
             _wcsicmp(testName, L"WaveMultiPrefixUCountBits") == 0) {
    // For CountBits, each lane contributes a boolean value. The test input is
    // a zero or non-zero integer. If the input is a non-zero value then the
    // condition is true, thus we contribute one to the bit count.
    return [](T lhs, T rhs) -> T { return lhs + (rhs ? 1 : 0); };
  } else {
    return [](T lhs, T rhs) -> T {
      UNREFERENCED_PARAMETER(lhs);
      UNREFERENCED_PARAMETER(rhs);
      return 0;
    };
  }
}

template <class T>
void ExecutionTest::WaveIntrinsicsMultiPrefixOpTest(
    TableParameter *pParameterList, size_t numParameters) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  struct PerThreadData {
    uint32_t key;
    uint32_t firstLaneId;
    uint32_t laneId;
    uint32_t mask;
    T value;
    T result;
  };

  constexpr size_t NumThreadsX = 8;
  constexpr size_t NumThreadsY = 12;
  constexpr size_t NumThreadsZ = 1;

  constexpr size_t ThreadsPerGroup = NumThreadsX * NumThreadsY * NumThreadsZ;
  constexpr size_t DispatchGroupSize = 1;
  constexpr size_t ThreadCount = ThreadsPerGroup * DispatchGroupSize;

  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;

  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL_6_5)) {
    return;
  }

  if (!DoesDeviceSupportWaveOps(pDevice)) {
    // Optional feature, so it's correct to not support it if declared as such.
    WEX::Logging::Log::Comment(L"Device does not support wave operations.");
    return;
  }

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());

  TableParameterHandler handler(pParameterList, numParameters);
  CW2A shaderSource(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);
  CW2A shaderProfile(handler.GetTableParamByName(L"ShaderOp.Target")->m_str);
  auto testName = handler.GetTableParamByName(L"ShaderOp.Name")->m_str;

  std::vector<T> *keys = handler.GetDataArray<T>(L"Validation.Keys");
  std::vector<T> *values = handler.GetDataArray<T>(L"Validation.Values");

  for (size_t maskIndex = 0; maskIndex < _countof(MaskFunctionTable);
       ++maskIndex) {
    std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
        pDevice, m_support, "WaveIntrinsicsOp",
        [&](LPCSTR name, std::vector<BYTE> &data, st::ShaderOp *pShaderOp) {
          UNREFERENCED_PARAMETER(name);

          const size_t dataSize = sizeof(PerThreadData) * ThreadCount;

          data.resize(dataSize);
          PerThreadData *pThreadData =
              reinterpret_cast<PerThreadData *>(data.data());

          for (size_t i = 0; i != ThreadCount; ++i) {
            pThreadData[i].key = keys->at(i % keys->size());
            pThreadData[i].value = values->at(i % values->size());
            pThreadData[i].firstLaneId = 0xdeadbeef;
            pThreadData[i].laneId = 0xdeadbeef;
            pThreadData[i].mask = MaskFunctionTable[maskIndex]((int)i);
            pThreadData[i].result = 0xdeadbeef;
          }

          pShaderOp->Shaders.at(0).Text = shaderSource;
          pShaderOp->Shaders.at(0).Target = shaderProfile;
        },
        ShaderOpSet);

    MappedData mappedData;
    test->Test->GetReadBackData("SWaveIntrinsicsOp", &mappedData);
    PerThreadData *resultData =
        reinterpret_cast<PerThreadData *>(mappedData.data());

    // Partition our data into waves
    std::map<uint32_t, std::vector<PerThreadData *>> waves;

    for (size_t i = 0, e = ThreadCount; i != e; ++i) {
      PerThreadData *elt = &resultData[i];

      // Basic sanity checks
      VERIFY_IS_TRUE(elt->firstLaneId != 0xdeadbeef);
      VERIFY_IS_TRUE(elt->laneId != 0xdeadbeef);

      waves[elt->firstLaneId].push_back(elt);
    }

    // Verify each wave
    auto refFn = GetWaveMultiPrefixReferenceFunction<T>(testName);

    for (auto &w : waves) {
      std::vector<PerThreadData *> &waveData = w.second;

      struct {
        bool operator()(PerThreadData *a, PerThreadData *b) const {
          return (a->laneId < b->laneId);
        }
      } compare;
      // Need to sort based on the lane id
      std::sort(waveData.begin(), waveData.end(), compare);

      LogCommentFmt(
          L"LaneId    Mask      Key       Value     Result    Expected");
      LogCommentFmt(
          L"--------  --------  --------  --------  --------  --------");
      for (size_t i = 0, e = waveData.size(); i != e; ++i) {
        PerThreadData *data = waveData[i];

        // Compute prefix operation over each previous lane element that has the
        // same key value, and is part of the same active thread group
        T accum = GetWaveMultiPrefixInitialAccumValue<T>(testName);
        for (unsigned j = 0; j < i; ++j) {
          if (waveData[j]->key == data->key &&
              waveData[j]->mask == data->mask) {
            accum = refFn(accum, waveData[j]->value);
          }
        }

        LogCommentFmt(L"%08X  %08X  %08X  %08X  %08X  %08X", data->laneId,
                      data->mask, data->key, data->value, data->result, accum);

        VERIFY_IS_TRUE(accum == data->result);
      }
      LogCommentFmt(L"\n");
    }
  }
}

TEST_F(ExecutionTest, WaveIntrinsicsSM65IntTest) {
  WaveIntrinsicsMultiPrefixOpTest<int>(
      WaveIntrinsicsMultiPrefixIntParameters,
      _countof(WaveIntrinsicsMultiPrefixIntParameters));
}

TEST_F(ExecutionTest, WaveIntrinsicsSM65UintTest) {
  WaveIntrinsicsMultiPrefixOpTest<unsigned>(
      WaveIntrinsicsMultiPrefixUintParameters,
      _countof(WaveIntrinsicsMultiPrefixUintParameters));
}

TEST_F(ExecutionTest, CBufferTestHalf) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  // Single operation test at the moment.
  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL_6_2))
    return;

  if (!DoesDeviceSupportNative16bitOps(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support native 16-bit operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  uint16_t InputData[] = {0x3F80, 0x3F00, 0x3D80, 0x7BFF};

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "CBufferTestHalf",
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        UNREFERENCED_PARAMETER(pShaderOp);
        VERIFY_IS_TRUE(0 == _stricmp(Name, "CB0"));
        // use shader from data table.
        Data.resize(sizeof(InputData));
        uint16_t *pData = (uint16_t *)Data.data();
        for (size_t i = 0; i < 4; ++i, ++pData) {
          *pData = InputData[i];
        }
      });
  {
    MappedData data;
    test->Test->GetReadBackData("RTarget", &data);
    const uint16_t *pPixels = (uint16_t *)data.data();

    for (int i = 0; i < 4; ++i) {
      uint16_t output = *(pPixels + i);
      float outputFloat = ConvertFloat16ToFloat32(output);
      float inputFloat = ConvertFloat16ToFloat32(InputData[i]);
      LogCommentFmt(
          L"element #%u: input = %6.8f(0x%04x), output = %6.8f(0x%04x)", i,
          inputFloat, InputData[i], outputFloat, output);
      VERIFY_ARE_EQUAL(inputFloat, outputFloat);
    }
  }
}

void TestBarycentricVariant(bool checkOrdering,
                            std::shared_ptr<ShaderOpTestResult> test) {
  MappedData data;
  D3D12_RESOURCE_DESC &D = test->ShaderOp->GetResourceByName("RTarget")->Desc;
  UINT width = (UINT)D.Width;
  UINT height = D.Height;
  UINT pixelSize = GetByteSizeForFormat(D.Format);

  test->Test->GetReadBackData("RTarget", &data);

  const float *pPixels = (float *)data.data();
  // Get the vertex of barycentric coordinate using VBuffer
  MappedData triangleData;
  test->Test->GetReadBackData("VBuffer", &triangleData);
  const float *pTriangleData = (float *)triangleData.data();
  // get the size of the input data
  unsigned triangleVertexSizeInFloat = 0;
  for (auto element : test->ShaderOp->InputElements)
    triangleVertexSizeInFloat += GetByteSizeForFormat(element.Format) / 4;

  XMFLOAT2 p0(pTriangleData[0], pTriangleData[1]);
  XMFLOAT2 p1(pTriangleData[triangleVertexSizeInFloat],
              pTriangleData[triangleVertexSizeInFloat + 1]);
  XMFLOAT2 p2(pTriangleData[triangleVertexSizeInFloat * 2],
              pTriangleData[triangleVertexSizeInFloat * 2 + 1]);

  // Seems like the 3 floats must add up to 1 to get accurate results.
  XMFLOAT3 barycentricWeights[4] = {
      XMFLOAT3(0.4f, 0.2f, 0.4f), XMFLOAT3(0.5f, 0.25f, 0.25f),
      XMFLOAT3(0.25f, 0.5f, 0.25f), XMFLOAT3(0.25f, 0.25f, 0.50f)};

  float tolerance = 0.02f;
  for (unsigned i = 0; i < sizeof(barycentricWeights) / sizeof(XMFLOAT3); ++i) {
    float w0 = barycentricWeights[i].x;
    float w1 = barycentricWeights[i].y;
    float w2 = barycentricWeights[i].z;
    float x1 = w0 * p0.x + w1 * p1.x + w2 * p2.x;
    float y1 = w0 * p0.y + w1 * p1.y + w2 * p2.y;
    // map from x1 y1 to rtv pixels
    int pixelX = (int)round((x1 + 1) * (width - 1) / 2.0);
    int pixelY = (int)round((1 - y1) * (height - 1) / 2.0);
    int offset = pixelSize * (pixelX + pixelY * width) / sizeof(pPixels[0]);
    LogCommentFmt(L"location  %u %u, value %f, %f, %f", pixelX, pixelY,
                  pPixels[offset], pPixels[offset + 1], pPixels[offset + 2]);
    if (!checkOrdering) {
      VERIFY_IS_TRUE(CompareFloatEpsilon(pPixels[offset], w0, tolerance));
      VERIFY_IS_TRUE(CompareFloatEpsilon(pPixels[offset + 1], w1, tolerance));
      VERIFY_IS_TRUE(CompareFloatEpsilon(pPixels[offset + 2], w2, tolerance));
    } else {
      // If the ordering constraint is met, then this pixel's RGBA should be
      // all 1.0's since the shader only returns float4<1.0,1.0,1.0,1.0> when
      // this condition is met.
      VERIFY_IS_TRUE(CompareFloatEpsilon(pPixels[offset], 0.0, tolerance));
      VERIFY_IS_TRUE(CompareFloatEpsilon(pPixels[offset + 1], 0.5, tolerance));
      VERIFY_IS_TRUE(CompareFloatEpsilon(pPixels[offset + 2], 1.0, tolerance));
      VERIFY_IS_TRUE(CompareFloatEpsilon(pPixels[offset + 3], 1.0, tolerance));
    }
  }
}

st::ShaderOpTest::TInitCallbackFn
MakeBarycentricsResourceInitCallbackFn(int &vertexShift) {
  return [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
    std::vector<float> bary = {0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                               1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                               -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
    const int barysize = 21;

    UNREFERENCED_PARAMETER(pShaderOp);
    VERIFY_IS_TRUE(0 == _stricmp(Name, "VBuffer"));
    size_t size = sizeof(float) * barysize;
    Data.resize(size);
    float *vb = (float *)Data.data();
    for (size_t i = 0; i < barysize; ++i) {
      float *p = &vb[i];
      float tempfloat = bary[(i + (7 * vertexShift)) % barysize];
      *p = tempfloat;
    }
  };
}

TEST_F(ExecutionTest, BarycentricsTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL_6_1))
    return;

  if (!DoesDeviceSupportBarycentrics(pDevice)) {
    WEX::Logging::Log::Comment(L"Device does not support barycentrics.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  DXASSERT_NOMSG(pStream != nullptr);
  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());
  st::ShaderOp *pShaderOp = ShaderOpSet->GetShaderOp("Barycentrics");

  int test_iteration = 0;
  auto ResourceCallbackFnNoShift =
      MakeBarycentricsResourceInitCallbackFn(test_iteration);

  std::shared_ptr<ShaderOpTestResult> test =
      RunShaderOpTestAfterParse(pDevice, m_support, "Barycentrics",
                                ResourceCallbackFnNoShift, ShaderOpSet);
  TestBarycentricVariant(false, test);

  // Now test that barycentric ordering is consistent
  LogCommentFmt(L"Now testing that the barycentric ordering constraint is "
                L"upheld for each pixel...");
  pShaderOp->VS = pShaderOp->GetString("VSordering");
  pShaderOp->PS = pShaderOp->GetString("PSordering");
  for (; test_iteration < 3; test_iteration++) {
    auto ResourceCallbackFn =
        MakeBarycentricsResourceInitCallbackFn(test_iteration);

    std::shared_ptr<ShaderOpTestResult> test2 = RunShaderOpTestAfterParse(
        pDevice, m_support, "Barycentrics", ResourceCallbackFn, ShaderOpSet);
    TestBarycentricVariant(true, test2);
  }
}

static const char RawBufferTestShaderDeclarations[] =
    "// Note: COMPONENT_TYPE and COMPONENT_SIZE will be defined via compiler "
    "option -D\r\n"
    "typedef COMPONENT_TYPE scalar; \r\n"
    "typedef vector<COMPONENT_TYPE, 2> vector2; \r\n"
    "typedef vector<COMPONENT_TYPE, 3> vector3; \r\n"
    "typedef vector<COMPONENT_TYPE, 4> vector4; \r\n"
    "\r\n"
    "struct TestData { \r\n"
    "  scalar  v1; \r\n"
    "  vector2 v2; \r\n"
    "  vector3 v3; \r\n"
    "  vector4 v4; \r\n"
    "}; \r\n"
    "\r\n"
    "struct UavData {\r\n"
    "  TestData input; \r\n"
    "  TestData output; \r\n"
    "  TestData srvOut; \r\n"
    "}; \r\n"
    "\r\n"
    "ByteAddressBuffer           srv0 : register(t0); \r\n"
    "StructuredBuffer<TestData>  srv1 : register(t1); \r\n"
    "ByteAddressBuffer           srv2 : register(t2); \r\n"
    "StructuredBuffer<TestData>  srv3 : register(t3); \r\n"
    "\r\n"
    "RWByteAddressBuffer         uav0 : register(u0); \r\n"
    "RWStructuredBuffer<UavData> uav1 : register(u1); \r\n"
    "RWByteAddressBuffer         uav2 : register(u2); \r\n"
    "RWStructuredBuffer<UavData> uav3 : register(u3); \r\n";

static const char RawBufferTestShaderBody[] =
    "  // offset of 'out' in 'UavData'\r\n"
    "  const int out_offset = COMPONENT_SIZE * 10; \r\n"
    "\r\n"
    "  // offset of 'srv_out' in 'UavData'\r\n"
    "  const int srv_out_offset = COMPONENT_SIZE * 10 * 2; \r\n"
    "\r\n"
    "  // offsets within the 'Data' struct\r\n"
    "  const int v1_offset = 0; \r\n"
    "  const int v2_offset = COMPONENT_SIZE; \r\n"
    "  const int v3_offset = COMPONENT_SIZE * 3; \r\n"
    "  const int v4_offset = COMPONENT_SIZE * 6; \r\n"
    "\r\n"
    "  uav0.Store(srv_out_offset + v1_offset, srv0.Load<scalar>(v1_offset)); "
    "\r\n"
    "  uav0.Store(srv_out_offset + v2_offset, srv0.Load<vector2>(v2_offset)); "
    "\r\n"
    "  uav0.Store(srv_out_offset + v3_offset, srv0.Load<vector3>(v3_offset)); "
    "\r\n"
    "  uav0.Store(srv_out_offset + v4_offset, srv0.Load<vector4>(v4_offset)); "
    "\r\n"
    "\r\n"
    "  uav1[0].srvOut.v1 = srv1[0].v1; \r\n"
    "  uav1[0].srvOut.v2 = srv1[0].v2; \r\n"
    "  uav1[0].srvOut.v3 = srv1[0].v3; \r\n"
    "  uav1[0].srvOut.v4 = srv1[0].v4; \r\n"
    "\r\n"
    "  uav2.Store(srv_out_offset + v1_offset, srv2.Load<scalar>(v1_offset)); "
    "\r\n"
    "  uav2.Store(srv_out_offset + v2_offset, srv2.Load<vector2>(v2_offset)); "
    "\r\n"
    "  uav2.Store(srv_out_offset + v3_offset, srv2.Load<vector3>(v3_offset)); "
    "\r\n"
    "  uav2.Store(srv_out_offset + v4_offset, srv2.Load<vector4>(v4_offset)); "
    "\r\n"
    "\r\n"
    "  uav3[0].srvOut.v1 = srv3[0].v1; \r\n"
    "  uav3[0].srvOut.v2 = srv3[0].v2; \r\n"
    "  uav3[0].srvOut.v3 = srv3[0].v3; \r\n"
    "  uav3[0].srvOut.v4 = srv3[0].v4; \r\n"
    "\r\n"
    "  uav0.Store(out_offset + v1_offset, uav0.Load<scalar>(v1_offset)); \r\n"
    "  uav0.Store(out_offset + v2_offset, uav0.Load<vector2>(v2_offset)); \r\n"
    "  uav0.Store(out_offset + v3_offset, uav0.Load<vector3>(v3_offset)); \r\n"
    "  uav0.Store(out_offset + v4_offset, uav0.Load<vector4>(v4_offset)); \r\n"
    "\r\n"
    "  uav1[0].output.v1 = uav1[0].input.v1; \r\n"
    "  uav1[0].output.v2 = uav1[0].input.v2; \r\n"
    "  uav1[0].output.v3 = uav1[0].input.v3; \r\n"
    "  uav1[0].output.v4 = uav1[0].input.v4; \r\n"
    "\r\n"
    "  uav2.Store(out_offset + v1_offset, uav2.Load<scalar>(v1_offset)); \r\n"
    "  uav2.Store(out_offset + v2_offset, uav2.Load<vector2>(v2_offset)); \r\n"
    "  uav2.Store(out_offset + v3_offset, uav2.Load<vector3>(v3_offset)); \r\n"
    "  uav2.Store(out_offset + v4_offset, uav2.Load<vector4>(v4_offset)); \r\n"
    "\r\n"
    "  uav3[0].output.v1 = uav3[0].input.v1; \r\n"
    "  uav3[0].output.v2 = uav3[0].input.v2; \r\n"
    "  uav3[0].output.v3 = uav3[0].input.v3; \r\n"
    "  uav3[0].output.v4 = uav3[0].input.v4; \r\n";

static const char RawBufferTestComputeShaderTemplate[] =
    "%s\r\n" // <- RawBufferTestShaderDeclarations
    "[numthreads(1, 1, 1)]\r\n"
    "void main(uint GI : SV_GroupIndex) {\r\n"
    "%s\r\n" // <- RawBufferTestShaderBody
    "};";

static const char RawBufferTestGraphicsPixelShaderTemplate[] =
    "%s\r\n" // <- RawBufferTestShaderDeclarations
    "struct PSInput { \r\n"
    "  float4 pos : SV_POSITION; \r\n"
    "}; \r\n"
    "uint4 main(PSInput input) : SV_TARGET{ \r\n"
    "  if (input.pos.x + input.pos.y == 1.0f) { // pixel { 0.5, 0.5, 0 } \r\n"
    "%s\r\n" // <- RawBufferTestShaderBody
    "  } \r\n"
    "  return uint4(1, 2, 3, 4); \r\n"
    "};";

TEST_F(ExecutionTest, ComputeRawBufferLdStI32) {
  RawBufferLdStTestData<int32_t> data = {
      1, {2, -1}, {256, -10517, 980}, {465, 13, -89, MAXUINT32 / 2}};
  RunComputeRawBufferLdStTest<int32_t>(D3D_SHADER_MODEL_6_2,
                                       RawBufferLdStType::I32,
                                       "ComputeRawBufferLdSt32Bit", data);
}

TEST_F(ExecutionTest, ComputeRawBufferLdStFloat) {
  RawBufferLdStTestData<float> data = {
      3e-10f,
      {1.5f, -1.99988f},
      {256.0f, -105.17f, 980.0f},
      {465.1652f, -1.5694e2f, -0.8543e-2f, 1333.5f}};
  RunComputeRawBufferLdStTest<float>(D3D_SHADER_MODEL_6_2,
                                     RawBufferLdStType::Float,
                                     "ComputeRawBufferLdSt32Bit", data);
}

TEST_F(ExecutionTest, ComputeRawBufferLdStI64) {
  RawBufferLdStTestData<int64_t> data = {
      1, {2, -1}, {256, -105171532, 980}, {465, 13, -89, MAXUINT64 / 2}};
  RunComputeRawBufferLdStTest<int64_t>(D3D_SHADER_MODEL_6_3,
                                       RawBufferLdStType::I64,
                                       "ComputeRawBufferLdSt64Bit", data);
}

TEST_F(ExecutionTest, ComputeRawBufferLdStDouble) {
  RawBufferLdStTestData<double> data = {
      3e-10,
      {1.5, -1.99988},
      {256.0, -105.17, 980.0},
      {465.1652, -1.5694e2, -0.8543e-2, 1333.5}};
  RunComputeRawBufferLdStTest<double>(D3D_SHADER_MODEL_6_3,
                                      RawBufferLdStType::I64,
                                      "ComputeRawBufferLdSt64Bit", data);
}

TEST_F(ExecutionTest, ComputeRawBufferLdStI16) {
  RawBufferLdStTestData<int16_t> data = {
      1, {2, -1}, {256, -10517, 980}, {465, 13, -89, MAXUINT16 / 2}};
  RunComputeRawBufferLdStTest<int16_t>(D3D_SHADER_MODEL_6_2,
                                       RawBufferLdStType::I16,
                                       "ComputeRawBufferLdSt16Bit", data);
}

TEST_F(ExecutionTest, ComputeRawBufferLdStHalf) {
  RawBufferLdStTestData<float> floatData = {
      3e-10f,
      {1.5f, -1.99988f},
      {256.0f, 105.17f, 980.0f},
      {465.1652f, -1.5694e2f, -0.8543e-2f, 1333.5f}};
  RawBufferLdStTestData<uint16_t> halfData;
  for (unsigned i = 0; i < sizeof(floatData) / sizeof(float); i++) {
    ((uint16_t *)&halfData)[i] =
        ConvertFloat32ToFloat16(((float *)&floatData)[i]);
  }
  RunComputeRawBufferLdStTest<uint16_t>(D3D_SHADER_MODEL_6_2,
                                        RawBufferLdStType::Half,
                                        "ComputeRawBufferLdSt16Bit", halfData);
}

TEST_F(ExecutionTest, GraphicsRawBufferLdStI32) {
  RawBufferLdStTestData<int32_t> data = {
      1, {2, -1}, {256, -10517, 980}, {465, 13, -89, MAXUINT32 / 2}};
  RunGraphicsRawBufferLdStTest<int32_t>(D3D_SHADER_MODEL_6_2,
                                        RawBufferLdStType::I32,
                                        "GraphicsRawBufferLdSt32Bit", data);
}

TEST_F(ExecutionTest, GraphicsRawBufferLdStFloat) {
  RawBufferLdStTestData<float> data = {
      3e-10f,
      {1.5f, -1.99988f},
      {256.0f, -105.17f, 980.0f},
      {465.1652f, -1.5694e2f, -0.8543e-2f, 1333.5f}};
  RunGraphicsRawBufferLdStTest<float>(D3D_SHADER_MODEL_6_2,
                                      RawBufferLdStType::Float,
                                      "GraphicsRawBufferLdSt32Bit", data);
}

TEST_F(ExecutionTest, GraphicsRawBufferLdStI64) {
  RawBufferLdStTestData<int64_t> data = {
      1, {2, -1}, {256, -105171532, 980}, {465, 13, -89, MAXUINT64 / 2}};
  RunGraphicsRawBufferLdStTest<int64_t>(D3D_SHADER_MODEL_6_3,
                                        RawBufferLdStType::I64,
                                        "GraphicsRawBufferLdSt64Bit", data);
}

TEST_F(ExecutionTest, GraphicsRawBufferLdStDouble) {
  RawBufferLdStTestData<double> data = {
      3e-10,
      {1.5, -1.99988},
      {256.0, -105.17, 980.0},
      {465.1652, -1.5694e2, -0.8543e-2, 1333.5}};
  RunGraphicsRawBufferLdStTest<double>(D3D_SHADER_MODEL_6_3,
                                       RawBufferLdStType::Double,
                                       "GraphicsRawBufferLdSt64Bit", data);
}

TEST_F(ExecutionTest, GraphicsRawBufferLdStI16) {
  RawBufferLdStTestData<int16_t> data = {
      1, {2, -1}, {256, -10517, 980}, {465, 13, -89, MAXUINT16 / 2}};
  RunGraphicsRawBufferLdStTest<int16_t>(D3D_SHADER_MODEL_6_2,
                                        RawBufferLdStType::I16,
                                        "GraphicsRawBufferLdSt16Bit", data);
}

TEST_F(ExecutionTest, GraphicsRawBufferLdStHalf) {
  RawBufferLdStTestData<float> floatData = {
      3e-10f,
      {1.5f, -1.99988f},
      {256.0f, 105.17f, 0.0f},
      {465.1652f, -1.5694e2f, -0.8543e-2f, 1333.5f}};
  RawBufferLdStTestData<uint16_t> halfData;
  for (unsigned i = 0; i < sizeof(floatData) / sizeof(float); i++) {
    ((uint16_t *)&halfData)[i] =
        ConvertFloat32ToFloat16(((float *)&floatData)[i]);
  }
  RunGraphicsRawBufferLdStTest<uint16_t>(
      D3D_SHADER_MODEL_6_2, RawBufferLdStType::Half,
      "GraphicsRawBufferLdSt16Bit", halfData);
}

bool ExecutionTest::SetupRawBufferLdStTest(D3D_SHADER_MODEL shaderModel,
                                           RawBufferLdStType dataType,
                                           CComPtr<ID3D12Device> &pDevice,
                                           CComPtr<IStream> &pStream,
                                           const char *&sTy,
                                           const char *&additionalOptions) {
  if (!CreateDevice(&pDevice, shaderModel)) {
    return false;
  }

  additionalOptions = "";

  switch (dataType) {
  case RawBufferLdStType::I64:
    if (!DoesDeviceSupportInt64(pDevice)) {
      WEX::Logging::Log::Comment(L"Device does not support int64 operations.");
      WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
      return false;
    }
    sTy = "int64_t";
    break;
  case RawBufferLdStType::Double:
    if (!DoesDeviceSupportDouble(pDevice)) {
      WEX::Logging::Log::Comment(L"Device does not support double operations.");
      WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
      return false;
    }
    sTy = "double";
    break;
  case RawBufferLdStType::I16:
  case RawBufferLdStType::Half:
    if (!DoesDeviceSupportNative16bitOps(pDevice)) {
      WEX::Logging::Log::Comment(
          L"Device does not support native 16-bit operations.");
      WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
      return false;
    }
    additionalOptions = "-enable-16bit-types";
    sTy = (dataType == RawBufferLdStType::I16 ? "int16_t" : "half");
    break;
  case RawBufferLdStType::I32:
    sTy = "int32_t";
    break;
  case RawBufferLdStType::Float:
    sTy = "float";
    break;
  default:
    DXASSERT_NOMSG("Invalid RawBufferLdStType");
  }

  // read shader config
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  return true;
}

template <class Ty>
void ExecutionTest::VerifyRawBufferLdStTestResults(
    const std::shared_ptr<st::ShaderOpTest> test,
    const RawBufferLdStTestData<Ty> &testData) {
  // read buffers back & verify expected values
  static const int UavBufferCount = 4;
  char bufferName[11] = "UAVBufferX";

  for (unsigned i = 0; i < UavBufferCount; i++) {
    MappedData dataUav;
    RawBufferLdStUavData<Ty> *pOutData;

    bufferName[sizeof(bufferName) - 2] = (char)(i + '0');

    test->GetReadBackData(bufferName, &dataUav);
    VERIFY_ARE_EQUAL(sizeof(RawBufferLdStUavData<Ty>), dataUav.size());
    pOutData = (RawBufferLdStUavData<Ty> *)dataUav.data();

    LogCommentFmt(L"Verifying UAVBuffer%d Load -> UAVBuffer%d Store", i, i);
    // scalar
    VERIFY_ARE_EQUAL(pOutData->output.v1, testData.v1);
    // vector 2
    VERIFY_ARE_EQUAL(pOutData->output.v2[0], testData.v2[0]);
    VERIFY_ARE_EQUAL(pOutData->output.v2[1], testData.v2[1]);
    // vector 3
    VERIFY_ARE_EQUAL(pOutData->output.v3[0], testData.v3[0]);
    VERIFY_ARE_EQUAL(pOutData->output.v3[1], testData.v3[1]);
    VERIFY_ARE_EQUAL(pOutData->output.v3[2], testData.v3[2]);
    // vector 4
    VERIFY_ARE_EQUAL(pOutData->output.v4[0], testData.v4[0]);
    VERIFY_ARE_EQUAL(pOutData->output.v4[1], testData.v4[1]);
    VERIFY_ARE_EQUAL(pOutData->output.v4[2], testData.v4[2]);
    VERIFY_ARE_EQUAL(pOutData->output.v4[3], testData.v4[3]);

    // verify SRV Store
    LogCommentFmt(L"Verifying SRVBuffer%d Load -> UAVBuffer%d Store", i, i);
    // scalar
    VERIFY_ARE_EQUAL(pOutData->srvOut.v1, testData.v1);
    // vector 2
    VERIFY_ARE_EQUAL(pOutData->srvOut.v2[0], testData.v2[0]);
    VERIFY_ARE_EQUAL(pOutData->srvOut.v2[1], testData.v2[1]);
    // vector 3
    VERIFY_ARE_EQUAL(pOutData->srvOut.v3[0], testData.v3[0]);
    VERIFY_ARE_EQUAL(pOutData->srvOut.v3[1], testData.v3[1]);
    VERIFY_ARE_EQUAL(pOutData->srvOut.v3[2], testData.v3[2]);
    // vector 4
    VERIFY_ARE_EQUAL(pOutData->srvOut.v4[0], testData.v4[0]);
    VERIFY_ARE_EQUAL(pOutData->srvOut.v4[1], testData.v4[1]);
    VERIFY_ARE_EQUAL(pOutData->srvOut.v4[2], testData.v4[2]);
    VERIFY_ARE_EQUAL(pOutData->srvOut.v4[3], testData.v4[3]);
  }
}

template <class Ty>
void ExecutionTest::RunComputeRawBufferLdStTest(
    D3D_SHADER_MODEL shaderModel, RawBufferLdStType dataType,
    const char *shaderOpName, const RawBufferLdStTestData<Ty> &testData) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  CComPtr<ID3D12Device> pDevice;
  CComPtr<IStream> pStream;
  const char *sTy = nullptr;
  const char *additionalOptions = nullptr;

  if (!SetupRawBufferLdStTest(shaderModel, dataType, pDevice, pStream, sTy,
                              additionalOptions)) {
    return;
  }

  // format shader source
  char rawBufferTestShaderText[sizeof(RawBufferTestComputeShaderTemplate) +
                               sizeof(RawBufferTestShaderDeclarations) +
                               sizeof(RawBufferTestShaderBody)];
  VERIFY_IS_TRUE(sprintf_s(rawBufferTestShaderText,
                           sizeof(rawBufferTestShaderText),
                           RawBufferTestComputeShaderTemplate,
                           RawBufferTestShaderDeclarations,
                           RawBufferTestShaderBody) != -1);

  // format compiler args
  char compilerOptions[256];
  VERIFY_IS_TRUE(sprintf_s(compilerOptions, sizeof(compilerOptions),
                           "-D COMPONENT_TYPE=%s -D COMPONENT_SIZE=%d %s", sTy,
                           (int)sizeof(Ty), additionalOptions) != -1);

  // run the shader
  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, shaderOpName,
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(((0 == strncmp(Name, "SRVBuffer", 9)) ||
                        (0 == strncmp(Name, "UAVBuffer", 9))) &&
                       (Name[9] >= '0' && Name[9] <= '3'));
        pShaderOp->Shaders.at(0).Arguments = compilerOptions;
        pShaderOp->Shaders.at(0).Text = rawBufferTestShaderText;

        VERIFY_IS_TRUE(sizeof(RawBufferLdStTestData<Ty>) <= Data.size());
        RawBufferLdStTestData<Ty> *pInData =
            (RawBufferLdStTestData<Ty> *)Data.data();
        memcpy(pInData, &testData, sizeof(RawBufferLdStTestData<Ty>));
      });

  // verify expected values
  VerifyRawBufferLdStTestResults<Ty>(test->Test, testData);
}

template <class Ty>
void ExecutionTest::RunGraphicsRawBufferLdStTest(
    D3D_SHADER_MODEL shaderModel, RawBufferLdStType dataType,
    const char *shaderOpName, const RawBufferLdStTestData<Ty> &testData) {

  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  CComPtr<ID3D12Device> pDevice;
  CComPtr<IStream> pStream;
  const char *sTy = nullptr;
  const char *additionalOptions = nullptr;

  if (!SetupRawBufferLdStTest(shaderModel, dataType, pDevice, pStream, sTy,
                              additionalOptions)) {
    return;
  }

  // format shader source
  char rawBufferTestPixelShaderText
      [sizeof(RawBufferTestGraphicsPixelShaderTemplate) +
       sizeof(RawBufferTestShaderDeclarations) +
       sizeof(RawBufferTestShaderBody)];
  VERIFY_IS_TRUE(sprintf_s(rawBufferTestPixelShaderText,
                           sizeof(rawBufferTestPixelShaderText),
                           RawBufferTestGraphicsPixelShaderTemplate,
                           RawBufferTestShaderDeclarations,
                           RawBufferTestShaderBody) != -1);

  // format compiler args
  char compilerOptions[256];
  VERIFY_IS_TRUE(sprintf_s(compilerOptions, sizeof(compilerOptions),
                           "-D COMPONENT_TYPE=%s -D COMPONENT_SIZE=%d %s", sTy,
                           (int)sizeof(Ty), additionalOptions) != -1);

  // run the shader
  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, shaderOpName,
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE(((0 == strncmp(Name, "SRVBuffer", 9)) ||
                        (0 == strncmp(Name, "UAVBuffer", 9))) &&
                       (Name[9] >= '0' && Name[9] <= '3'));
        // pixel shader is at index 1, vertex shader at index 0
        pShaderOp->Shaders.at(1).Arguments = compilerOptions;
        pShaderOp->Shaders.at(1).Text = rawBufferTestPixelShaderText;

        VERIFY_IS_TRUE(sizeof(RawBufferLdStTestData<Ty>) <= Data.size());
        RawBufferLdStTestData<Ty> *pInData =
            (RawBufferLdStTestData<Ty> *)Data.data();
        memcpy(pInData, &testData, sizeof(RawBufferLdStTestData<Ty>));
      });

  // verify expected values
  VerifyRawBufferLdStTestResults<Ty>(test->Test, testData);
}

template <typename T> uint32_t pack(std::array<T, 4> unpackedVals) {
  uint32_t dst = 0;
  constexpr uint32_t bitMask = 0xFF;
  for (uint32_t i = 0U; i < 4U; ++i) {
    dst |= (unpackedVals[i] & bitMask) << (i * 8);
  }

  return dst;
}

template <typename T> uint32_t pack_clamp_u8(std::array<T, 4> unpackedVals) {
  int32_t clamp_min = std::numeric_limits<uint8_t>::min();
  int32_t clamp_max = std::numeric_limits<uint8_t>::max();

  uint32_t dst = 0;
  for (uint32_t i = 0U; i < 4U; ++i) {
    int32_t clamped =
        std::min(std::max((int32_t)unpackedVals[i], clamp_min), clamp_max);
    dst |= ((uint8_t)clamped) << (i * 8);
  }

  return dst;
}

template <typename T> uint32_t pack_clamp_s8(std::array<T, 4> unpackedVals) {
  int32_t clamp_min = std::numeric_limits<int8_t>::min();
  int32_t clamp_max = std::numeric_limits<int8_t>::max();

  uint32_t dst = 0;
  for (uint32_t i = 0U; i < 4U; ++i) {
    int32_t clamped =
        std::min(std::max((int32_t)unpackedVals[i], clamp_min), clamp_max);
    dst |= ((uint8_t)clamped) << (i * 8);
  }

  return dst;
}

template <typename T> std::array<T, 4> unpack_u(uint32_t packedVal) {
  std::array<T, 4> ret;
  ret[0] = (uint8_t)((packedVal & 0x000000FF) >> 0);
  ret[1] = (uint8_t)((packedVal & 0x0000FF00) >> 8);
  ret[2] = (uint8_t)((packedVal & 0x00FF0000) >> 16);
  ret[3] = (uint8_t)((packedVal & 0xFF000000) >> 24);

  return ret;
}

template <typename T> std::array<T, 4> unpack_s(uint32_t packedVal) {
  std::array<T, 4> ret;
  ret[0] = (int8_t)((packedVal & 0x000000FF) >> 0);
  ret[1] = (int8_t)((packedVal & 0x0000FF00) >> 8);
  ret[2] = (int8_t)((packedVal & 0x00FF0000) >> 16);
  ret[3] = (int8_t)((packedVal & 0xFF000000) >> 24);

  return ret;
}

TEST_F(ExecutionTest, PackUnpackTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;

#ifdef PACKUNPACK_PLACEHOLDER
  string args = "-enable-16bit-types -DPACKUNPACK_PLACEHOLDER";
  string target = "cs_6_2";

  if (!CreateDevice(&pDevice)) {
    return;
  }
#else
  string args = "-enable-16bit-types";
  string target = "cs_6_6";

  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL_6_6)) {
    return;
  }
#endif

  if (!DoesDeviceSupportNative16bitOps(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support native 16-bit operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  int tableSize = sizeof(PackUnpackOpParameters) / sizeof(TableParameter);
  TableParameterHandler handler(PackUnpackOpParameters, tableSize);

  CW2A Text(handler.GetTableParamByName(L"ShaderOp.Text")->m_str);

  std::vector<uint32_t> *validation_input =
      &handler.GetTableParamByName(L"Validation.Input")->m_uint32Table;
  uint32_t validation_tolerance =
      handler.GetTableParamByName(L"Validation.Tolerance")->m_uint;

  size_t count = validation_input->size();
  std::vector<SPackUnpackOpOutPacked> expectedPacked(count / 4);
  std::vector<SPackUnpackOpOutUnpacked> expectedUnpacked(count / 4);

  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTest(
      pDevice, m_support, pStream, "PackUnpackOp",
      // this callback is called when the test
      // is creating the resource to run the test
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        if (0 == _stricmp(Name, "g_bufIn")) {
          size_t size = sizeof(uint32_t) * 4 * count;
          Data.resize(size);
          uint32_t *pPrimitives = (uint32_t *)Data.data();

          for (size_t i = 0; i < count / 4; ++i) {
            uint32_t *p = &pPrimitives[i * 4];
            uint32_t x = (*validation_input)[i * 4 + 0];
            uint32_t y = (*validation_input)[i * 4 + 1];
            uint32_t z = (*validation_input)[i * 4 + 2];
            uint32_t w = (*validation_input)[i * 4 + 3];

            p[0] = x;
            p[1] = y;
            p[2] = z;
            p[3] = w;

            std::array<uint32_t, 4> inputUint32 = {x, y, z, w};
            std::array<int32_t, 4> inputInt32 = {(int32_t)x, (int32_t)y,
                                                 (int32_t)z, (int32_t)w};
            std::array<uint16_t, 4> inputUint16 = {(uint16_t)x, (uint16_t)y,
                                                   (uint16_t)z, (uint16_t)w};
            std::array<int16_t, 4> inputInt16 = {(int16_t)x, (int16_t)y,
                                                 (int16_t)z, (int16_t)w};

            // Pack unclamped
            expectedPacked[i].packedUint32 = pack(inputUint32);
            expectedPacked[i].packedInt32 = pack(inputInt32);
            expectedPacked[i].packedUint16 = pack(inputUint16);
            expectedPacked[i].packedInt16 = pack(inputInt16);
            // pack clamped
            expectedPacked[i].packedClampedUint32 = pack_clamp_u8(inputInt32);
            expectedPacked[i].packedClampedInt32 = pack_clamp_s8(inputInt32);
            expectedPacked[i].packedClampedUint16 = pack_clamp_u8(inputInt16);
            expectedPacked[i].packedClampedInt16 = pack_clamp_s8(inputInt16);

            // unpack
            expectedUnpacked[i].outputUint32 =
                unpack_u<uint32_t>(expectedPacked[i].packedUint32);
            expectedUnpacked[i].outputInt32 =
                unpack_s<int32_t>(expectedPacked[i].packedInt32);
            expectedUnpacked[i].outputUint16 =
                unpack_u<uint16_t>(expectedPacked[i].packedUint16);
            expectedUnpacked[i].outputInt16 =
                unpack_s<int16_t>(expectedPacked[i].packedInt16);
            expectedUnpacked[i].outputClampedUint32 =
                unpack_u<uint32_t>(expectedPacked[i].packedClampedUint32);
            expectedUnpacked[i].outputClampedInt32 =
                unpack_s<int32_t>(expectedPacked[i].packedClampedInt32);
            expectedUnpacked[i].outputClampedUint16 =
                unpack_u<uint16_t>(expectedPacked[i].packedClampedUint16);
            expectedUnpacked[i].outputClampedInt16 =
                unpack_s<int16_t>(expectedPacked[i].packedClampedInt16);
          }
        } else {
          std::fill(Data.begin(), Data.end(), (BYTE)0);
        }

        // use shader from data table
        pShaderOp->Shaders.at(0).Target = target.c_str();
        pShaderOp->Shaders.at(0).Text = Text.m_psz;
        pShaderOp->Shaders.at(0).Arguments = args.c_str();
      });

  MappedData packedData;
  test->Test->GetReadBackData("g_bufOutPacked", &packedData);
  SPackUnpackOpOutPacked *readBackPacked =
      (SPackUnpackOpOutPacked *)packedData.data();

  MappedData unpackedData;
  test->Test->GetReadBackData("g_bufOutPackedUnpacked", &unpackedData);
  SPackUnpackOpOutUnpacked *readBackUnpacked =
      (SPackUnpackOpOutUnpacked *)unpackedData.data();

  for (size_t i = 0; i < count / 4; ++i) {
    VerifyOutputWithExpectedValueUInt(readBackPacked[i].packedUint32,
                                      expectedPacked[i].packedUint32,
                                      validation_tolerance);
    VerifyOutputWithExpectedValueInt(readBackPacked[i].packedInt32,
                                     expectedPacked[i].packedInt32,
                                     validation_tolerance);
    VerifyOutputWithExpectedValueUInt(readBackPacked[i].packedUint16,
                                      expectedPacked[i].packedUint16,
                                      validation_tolerance);
    VerifyOutputWithExpectedValueInt(readBackPacked[i].packedInt16,
                                     expectedPacked[i].packedInt16,
                                     validation_tolerance);
    VerifyOutputWithExpectedValueUInt(readBackPacked[i].packedClampedUint32,
                                      expectedPacked[i].packedClampedUint32,
                                      validation_tolerance);
    VerifyOutputWithExpectedValueInt(readBackPacked[i].packedClampedInt32,
                                     expectedPacked[i].packedClampedInt32,
                                     validation_tolerance);
    VerifyOutputWithExpectedValueUInt(readBackPacked[i].packedClampedUint16,
                                      expectedPacked[i].packedClampedUint16,
                                      validation_tolerance);
    VerifyOutputWithExpectedValueInt(readBackPacked[i].packedClampedInt16,
                                     expectedPacked[i].packedClampedInt16,
                                     validation_tolerance);

    for (uint32_t j = 0; j < 4; ++j) {
      VerifyOutputWithExpectedValueUInt(readBackUnpacked[i].outputUint32[j],
                                        expectedUnpacked[i].outputUint32[j],
                                        validation_tolerance);
      VerifyOutputWithExpectedValueInt(readBackUnpacked[i].outputInt32[j],
                                       expectedUnpacked[i].outputInt32[j],
                                       validation_tolerance);
      VerifyOutputWithExpectedValueUInt(readBackUnpacked[i].outputUint16[j],
                                        expectedUnpacked[i].outputUint16[j],
                                        validation_tolerance);
      VerifyOutputWithExpectedValueInt(readBackUnpacked[i].outputInt16[j],
                                       expectedUnpacked[i].outputInt16[j],
                                       validation_tolerance);
      VerifyOutputWithExpectedValueUInt(
          readBackUnpacked[i].outputClampedUint32[j],
          expectedUnpacked[i].outputClampedUint32[j], validation_tolerance);
      VerifyOutputWithExpectedValueInt(
          readBackUnpacked[i].outputClampedInt32[j],
          expectedUnpacked[i].outputClampedInt32[j], validation_tolerance);
      VerifyOutputWithExpectedValueUInt(
          readBackUnpacked[i].outputClampedUint16[j],
          expectedUnpacked[i].outputClampedUint16[j], validation_tolerance);
      VerifyOutputWithExpectedValueInt(
          readBackUnpacked[i].outputClampedInt16[j],
          expectedUnpacked[i].outputClampedInt16[j], validation_tolerance);
    }
  }
}

// This test expects a <pShader> that retrieves a signal value from each of a
// few resources that are initialized here. <isDynamic> determines if it uses
// the 6.6 Dynamic Resources feature. Values are read back from the result UAV
// and compared to the expected signals
void ExecutionTest::RunResourceTest(ID3D12Device *pDevice, const char *pShader,
                                    const wchar_t *sm, bool isDynamic) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  const int NumSRVs = 3;
  const int NumUAVs = 4;
  const int NumResources = NumSRVs + NumUAVs;
  const int NumSamplers = 2;
  const int valueSize = 16;

  static const int DispatchGroupX = 1;
  static const int DispatchGroupY = 1;
  static const int DispatchGroupZ = 1;

  CComPtr<ID3D12GraphicsCommandList> pCommandList;
  CComPtr<ID3D12CommandQueue> pCommandQueue;
  CComPtr<ID3D12CommandAllocator> pCommandAllocator;
  FenceObj FO;

  UINT valueSizeInBytes = valueSize * sizeof(float);
  CreateComputeCommandQueue(pDevice, L"DynamicResourcesTest Command Queue",
                            &pCommandQueue);
  InitFenceObj(pDevice, &FO);

  // Create root signature.
  CComPtr<ID3D12RootSignature> pRootSignature;
  if (!isDynamic) {
    // Not dynamic, create a range for each resource and from them, the root
    // signature
    CD3DX12_DESCRIPTOR_RANGE ranges[NumResources];
    CD3DX12_DESCRIPTOR_RANGE srange[NumSamplers];
    for (int i = 0; i < NumSRVs; i++)
      ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, i, 0);

    for (int i = NumSRVs; i < NumResources; i++)
      ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, i - NumSRVs, 0);

    for (int i = 0; i < NumSamplers; i++)
      srange[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, i, 0);

    CreateRootSignatureFromRanges(pDevice, &pRootSignature, ranges,
                                  NumResources, srange, NumSamplers);
  } else {
    // Dynamic just requires the flags indicating that the builtin arrays should
    // be accessible
#if !defined(D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED)
#define D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED            \
  (D3D12_ROOT_SIGNATURE_FLAGS)0x400
#define D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED                \
  (D3D12_ROOT_SIGNATURE_FLAGS)0x800
#endif
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(
        0, nullptr, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
            D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED);
    CreateRootSignatureFromDesc(pDevice, &rootSignatureDesc, &pRootSignature);
  }

  // Create pipeline state object.
  CComPtr<ID3D12PipelineState> pComputeState;
  CreateComputePSO(pDevice, pRootSignature, pShader, sm, &pComputeState);

  // Create a command allocator and list for compute.
  VERIFY_SUCCEEDED(pDevice->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&pCommandAllocator)));
  VERIFY_SUCCEEDED(pDevice->CreateCommandList(
      0, D3D12_COMMAND_LIST_TYPE_COMPUTE, pCommandAllocator, pComputeState,
      IID_PPV_ARGS(&pCommandList)));

  // Set up SRV resources
  CComPtr<ID3D12Resource> pSRVResources[NumSRVs];
  CComPtr<ID3D12Resource> pUAVResources[NumUAVs];
  CComPtr<ID3D12Resource> pUploadResources[NumResources];
  {
    D3D12_RESOURCE_DESC bufDesc =
        CD3DX12_RESOURCE_DESC::Buffer(valueSizeInBytes);
    float values[valueSize];
    for (int i = 0; i < NumSRVs - 1; i++) {
      for (int j = 0; j < valueSize; j++)
        values[j] = 10.0f + i;
      CreateTestResources(pDevice, pCommandList, values, valueSizeInBytes,
                          bufDesc, &pSRVResources[i], &pUploadResources[i]);
    }
    D3D12_RESOURCE_DESC tex2dDesc =
        CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_FLOAT, 4, 4);
    for (int j = 0; j < valueSize; j++)
      values[j] = 10.0 + (NumSRVs - 1);
    CreateTestResources(pDevice, pCommandList, values, valueSizeInBytes,
                        tex2dDesc, &pSRVResources[NumSRVs - 1],
                        &pUploadResources[NumSRVs - 1]);
  }

  // Set up UAV resources
  CComPtr<ID3D12Resource> pReadBuffer;
  float values[valueSize];
  for (int i = 0; i < NumUAVs - 2; i++) {
    for (int j = 0; j < valueSize; j++)
      values[j] = 20.0f + i;
    CreateTestUavs(pDevice, pCommandList, values, valueSizeInBytes,
                   &pUAVResources[i], &pUploadResources[NumSRVs + i]);
  }
  for (int j = 0; j < valueSize; j++)
    values[j] = 20.0 + (NumUAVs - 1);
  CreateTestUavs(pDevice, pCommandList, values, valueSizeInBytes,
                 &pUAVResources[NumUAVs - 2],
                 &pUploadResources[NumResources - 2], &pReadBuffer);

  for (int j = 0; j < valueSize; j++)
    values[j] = 20.0 + (NumUAVs - 2);
  D3D12_RESOURCE_DESC tex1dDesc =
      CD3DX12_RESOURCE_DESC::Tex1D(DXGI_FORMAT_R32_FLOAT, valueSize, 1, 0,
                                   D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
  CreateTestResources(pDevice, pCommandList, values, valueSizeInBytes,
                      tex1dDesc, &pUAVResources[NumUAVs - 1],
                      &pUploadResources[NumResources - 1]);

  // Close the command list and execute it to perform the GPU setup.
  pCommandList->Close();
  ExecuteCommandList(pCommandQueue, pCommandList);
  WaitForSignal(pCommandQueue, FO);
  VERIFY_SUCCEEDED(pCommandAllocator->Reset());
  VERIFY_SUCCEEDED(pCommandList->Reset(pCommandAllocator, pComputeState));

  CComPtr<ID3D12DescriptorHeap> pResHeap;
  CComPtr<ID3D12DescriptorHeap> pSampHeap;
  CreateDefaultDescHeaps(pDevice, NumSRVs + NumUAVs, NumSamplers, &pResHeap,
                         &pSampHeap);

  // Create Rootsignature and descriptor tables
  {
    ID3D12DescriptorHeap *descHeaps[2] = {pResHeap, pSampHeap};
    pCommandList->SetDescriptorHeaps(2, descHeaps);
    pCommandList->SetComputeRootSignature(pRootSignature);

    if (!isDynamic) {
      // Only non-dynamic resources require descriptortables
      pCommandList->SetComputeRootDescriptorTable(
          0, pResHeap->GetGPUDescriptorHandleForHeapStart());
      pCommandList->SetComputeRootDescriptorTable(
          1, pSampHeap->GetGPUDescriptorHandleForHeapStart());
    }
  }
  CD3DX12_CPU_DESCRIPTOR_HANDLE baseHandle(
      pResHeap->GetCPUDescriptorHandleForHeapStart());
  // Create SRVs
  CreateRawSRV(pDevice, baseHandle, valueSize, pSRVResources[0]);
  CreateStructSRV(pDevice, baseHandle, valueSize, sizeof(float),
                  pSRVResources[1]);
  CreateTex2DSRV(pDevice, baseHandle, DXGI_FORMAT_R32_FLOAT, pSRVResources[2]);
  // Create UAVs
  CreateRawUAV(pDevice, baseHandle, valueSize, pUAVResources[0]);
  CreateStructUAV(pDevice, baseHandle, valueSize, sizeof(float),
                  pUAVResources[1]);
  CreateTypedUAV(pDevice, baseHandle, valueSize, DXGI_FORMAT_R32_FLOAT,
                 pUAVResources[2]);
  CreateTex1DUAV(pDevice, baseHandle, DXGI_FORMAT_R32_FLOAT, pUAVResources[3]);

  D3D12_FILTER filters[] = {D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
                            D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT};
  float perSampleBorderColors[] = {30.0, 31.0};
  CreateDefaultSamplers(pDevice,
                        pSampHeap->GetCPUDescriptorHandleForHeapStart(),
                        filters, perSampleBorderColors, NumSamplers);

  // Run the compute shader and copy the results back to readable memory.
  pCommandList->Dispatch(DispatchGroupX, DispatchGroupY, DispatchGroupZ);

  RecordTransitionBarrier(pCommandList, pUAVResources[NumUAVs - 2],
                          D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                          D3D12_RESOURCE_STATE_COPY_SOURCE);
  pCommandList->CopyResource(pReadBuffer, pUAVResources[NumUAVs - 2]);

  pCommandList->Close();
  ExecuteCommandList(pCommandQueue, pCommandList);
  WaitForSignal(pCommandQueue, FO);

  MappedData data(pReadBuffer, valueSize * sizeof(float));
  const float *pData = (float *)data.data();
  LogCommentFmt(L"Verify bound resources are properly selected");
  VERIFY_ARE_EQUAL(pData[0], 10);
  VERIFY_ARE_EQUAL(pData[1], 11);
  VERIFY_ARE_EQUAL(pData[2], 12);

  VERIFY_ARE_EQUAL(pData[3], 20);
  VERIFY_ARE_EQUAL(pData[4], 21);
  VERIFY_ARE_EQUAL(pData[5], 22);
  VERIFY_ARE_EQUAL(pData[6], 30);
  VERIFY_ARE_EQUAL(pData[7], 1); // samplecmp 1 means it matched 31
}

TEST_F(ExecutionTest, SignatureResourcesTest) {
  std::string pShader =
      "ByteAddressBuffer         g_rawBuf      : register(t0);\n"
      "StructuredBuffer<float>   g_structBuf   : register(t1);\n"
      "Texture2D<float>          g_tex         : register(t2);\n"
      "RWByteAddressBuffer       g_rwRawBuf    : register(u0);\n"
      "RWStructuredBuffer<float> g_rwStructBuf : register(u1);\n"
      "RWBuffer<float>           g_result      : register(u2);\n"
      "RWTexture1D<float>        g_rwTex       : register(u3);\n"
      "SamplerState              g_samp        : register(s0);\n"
      "SamplerComparisonState    g_sampCmp     : register(s1);\n"
      "[NumThreads(1, 1, 1)]\n"
      "void main(uint ix : SV_GroupIndex) {\n"
      "  g_result[0] = g_rawBuf.Load<float>(0);\n"
      "  g_result[1] = g_structBuf.Load(0);\n"
      "  g_result[2] = g_tex.Load(0);\n"
      "  g_result[3] = g_rwRawBuf.Load<float>(0);\n"
      "  g_result[4] = g_rwStructBuf.Load(0);\n"
      "  g_result[5] = g_rwTex.Load(0);\n"
      "  g_result[6] = g_tex.SampleLevel(g_samp, -0.5, 0);\n"
      "  g_result[7] = g_tex.SampleCmpLevelZero(g_sampCmp, -0.5, 31.0);\n"
      "}\n";

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL_6_6))
    return;

  RunResourceTest(pDevice, pShader.c_str(), L"cs_6_6", /*isDynamic*/ false);
}

TEST_F(ExecutionTest, DynamicResourcesTest) {
  static const char pShader[] =
      "static ByteAddressBuffer         g_rawBuf      = "
      "ResourceDescriptorHeap[0];\n"
      "static StructuredBuffer<float>   g_structBuf   = "
      "ResourceDescriptorHeap[1];\n"
      "static Texture2D<float>          g_tex         = "
      "ResourceDescriptorHeap[2];\n"
      "static RWByteAddressBuffer       g_rwRawBuf    = "
      "ResourceDescriptorHeap[3];\n"
      "static RWStructuredBuffer<float> g_rwStructBuf = "
      "ResourceDescriptorHeap[4];\n"
      "static RWBuffer<float>           g_result      = "
      "ResourceDescriptorHeap[5];\n"
      "static RWTexture1D<float>        g_rwTex       = "
      "ResourceDescriptorHeap[6];\n"
      "static SamplerState              g_samp        = "
      "SamplerDescriptorHeap[0];\n"
      "static SamplerComparisonState    g_sampCmp     = "
      "SamplerDescriptorHeap[1];\n"
      "[NumThreads(1, 1, 1)]\n"
      "void main(uint ix : SV_GroupIndex) {\n"
      "  g_result[0] = g_rawBuf.Load<float>(0);\n"
      "  g_result[1] = g_structBuf.Load(0);\n"
      "  g_result[2] = g_tex.Load(0);\n"
      "  g_result[3] = g_rwRawBuf.Load<float>(0);\n"
      "  g_result[4] = g_rwStructBuf.Load(0);\n"
      "  g_result[5] = g_rwTex.Load(0);\n"
      "  g_result[6] = g_tex.SampleLevel(g_samp, -0.5, 0);\n"
      "  g_result[7] = g_tex.SampleCmpLevelZero(g_sampCmp, -0.5, 31.0);\n"
      "}\n";

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL_6_6))
    return;

  // ResourceDescriptorHeap/SamplerDescriptorHeap requires Resource Binding Tier
  // 3
  D3D12_FEATURE_DATA_D3D12_OPTIONS devOptions;
  VERIFY_SUCCEEDED(
      pDevice->CheckFeatureSupport((D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS,
                                   &devOptions, sizeof(devOptions)));
  if (devOptions.ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_3) {
    WEX::Logging::Log::Comment(
        L"Device does not support Resource Binding Tier 3");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  RunResourceTest(pDevice, pShader, L"cs_6_6", /*isDynamic*/ true);
}

// void ExecutionTest::TestComputeShaderDynamicResourcesUniformIndexing()

void EnableShaderBasedValidation() {
  CComPtr<ID3D12Debug> spDebugController0;
  CComPtr<ID3D12Debug1> spDebugController1;
  VERIFY_SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&spDebugController0)));
  VERIFY_SUCCEEDED(
      spDebugController0->QueryInterface(IID_PPV_ARGS(&spDebugController1)));
  spDebugController1->SetEnableGPUBasedValidation(true);
}

void VerifyFloatArraysAreEqual(const float *resultFloats,
                               float *expectedResults,
                               int expectedResultsSize) {
  for (int j = 0; j < expectedResultsSize; j++) {
    VERIFY_ARE_EQUAL(resultFloats[j], expectedResults[j]);
  }
}

TEST_F(ExecutionTest, DynamicResourcesDynamicIndexingTest) {
  // EnableShaderBasedValidation();
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());
  st::ShaderOp *pShaderOp =
      ShaderOpSet->GetShaderOp("DynamicResourcesDynamicIndexing");
  vector<st::ShaderOpRootValue> fallbackRootValues = pShaderOp->RootValues;

  bool Skipped = true;

  // D3D_SHADER_MODEL TestShaderModels[] = {D3D_SHADER_MODEL_6_0}; // FALLBACK
  D3D_SHADER_MODEL TestShaderModels[] = {D3D_SHADER_MODEL_6_6,
                                         D3D_SHADER_MODEL_6_0};

  const int expectedResultsSize = 16;
  float expectedResultsUniform[expectedResultsSize] = {
      10.0, 10.0, 12.0, 12.0, 14.0, 14.0, 20.0, 20.0,
      22.0, 22.0, 24.0, 24.0, 30.0, 30.0, 32.0, 32.0};

  float expectedResultsNonUniform[expectedResultsSize] = {
      10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 20.0, 21.0,
      22.0, 23.0, 24.0, 25.0, 30.0, 31.0, 32.0, 33.0};

  // TestShaderModels will be an array, where the first x models are
  // "non-fallback", and the rest of the models are "fallback". If
  // TestShaderModels has length y, and a test loops through all shader models,
  // a convention to test based on whether fallback is enabled or not is to
  // limit the loop like this: unsigned num_models_to_test =
  // ExecutionTest::IsFallbackPathEnabled() ? y : x;
  unsigned num_models_to_test = ExecutionTest::IsFallbackPathEnabled() ? 2 : 1;
  for (unsigned i = 0; i < num_models_to_test; i++) {
    D3D_SHADER_MODEL sm = TestShaderModels[i];
    LogCommentFmt(L"\r\nVerifying Dynamic Resources Dynamic Indexing in shader "
                  L"model 6.%1u",
                  ((UINT)sm & 0x0f));

    CComPtr<ID3D12Device> pDevice;
    if (!CreateDevice(&pDevice, sm, false /* skipUnsupported */)) {
      continue;
    }
    D3D12_FEATURE_DATA_D3D12_OPTIONS devOptions;
    VERIFY_SUCCEEDED(
        pDevice->CheckFeatureSupport((D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS,
                                     &devOptions, sizeof(devOptions)));
    if (devOptions.ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_3) {
      WEX::Logging::Log::Comment(
          L"Device does not support Resource Binding Tier 3");
      WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
      return;
    }

    for (unsigned int non_uniform_bit = 0; non_uniform_bit < 2;
         non_uniform_bit++) {
      float *expectedResults =
          non_uniform_bit ? expectedResultsNonUniform : expectedResultsUniform;

      LogCommentFmt(L"Testing %s Resource Indexing.",
                    non_uniform_bit ? L"NonUniform" : L"Uniform");

      // Add compile options
      std::string compilerOptions = "";
      if (sm == D3D_SHADER_MODEL_6_0)
        compilerOptions += " -D FALLBACK=1";
      if (non_uniform_bit)
        compilerOptions += " -D NON_UNIFORM=1";

      // by default a root value is added.
      // remove the root value if this is the non-fallback path
      if (sm == D3D_SHADER_MODEL_6_6) {
        pShaderOp->RootValues.clear();
      } else {
        pShaderOp->RootValues = fallbackRootValues;
      }

      // Update shader target in xml.
      for (st::ShaderOpShader &S : pShaderOp->Shaders) {
        S.Arguments = NULL;
        if (!compilerOptions.empty()) {
          S.Arguments = pShaderOp->GetString(compilerOptions.c_str());
        }
        // Set the target correctly. Setting here permanently overwrites
        // the Target string even in future iterations.
        if (sm == D3D_SHADER_MODEL_6_0) {
          std::string Target(S.Target);
          Target[Target.length() - 1] = '0';
          S.Target = pShaderOp->GetString(Target.c_str());
        } else if (sm == D3D_SHADER_MODEL_6_6) {
          std::string Target(S.Target);
          Target[Target.length() - 1] = '6';
          S.Target = pShaderOp->GetString(Target.c_str());
        }
      }

      // Test Compute shader
      {
        pShaderOp->CS = pShaderOp->GetString("CS66");
        std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
            pDevice, m_support, "DynamicResourcesDynamicIndexing", nullptr,
            ShaderOpSet);

        MappedData resultData;
        test->Test->GetReadBackData("g_result", &resultData);
        const float *resultCSFloats = (float *)resultData.data();

        VerifyFloatArraysAreEqual(resultCSFloats, expectedResults,
                                  expectedResultsSize);
      }

      // Test Vertex + Pixel shader
      {
        pShaderOp->CS = nullptr;
        pShaderOp->VS = pShaderOp->GetString("VS66");
        pShaderOp->PS = pShaderOp->GetString("PS66");
        std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
            pDevice, m_support, "DynamicResourcesDynamicIndexing", nullptr,
            ShaderOpSet);

        MappedData resultVSData;
        MappedData resultPSData;
        test->Test->GetReadBackData("g_resultVS", &resultVSData);
        test->Test->GetReadBackData("g_resultPS", &resultPSData);
        const float *resultVSFloats = (float *)resultVSData.data();
        const float *resultPSFloats = (float *)resultPSData.data();
        D3D12_QUERY_DATA_PIPELINE_STATISTICS Stats;
        test->Test->GetPipelineStats(&Stats);

        // VS
        VerifyFloatArraysAreEqual(resultVSFloats, expectedResults,
                                  expectedResultsSize);

        // PS
        VerifyFloatArraysAreEqual(resultPSFloats, expectedResults,
                                  expectedResultsSize);
      }
      Skipped = false;
    }
  }

  if (Skipped) {
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
  }
}

#define MAX_WAVESIZE 128

#define stringify2(arg) #arg
#define stringify(arg) stringify2(arg)

void RunWaveSizeTest(UINT minWaveSize, UINT maxWaveSize,
                     std::shared_ptr<st::ShaderOpSet> ShaderOpSet,
                     CComPtr<ID3D12Device> pDevice,
                     dxc::DxcDllSupport &m_support) {
  // format shader source
  const char waveSizeTestShader[] =
      R"(struct TestData { 
        uint count; 
      };
      RWStructuredBuffer<TestData> data : register(u0); 

      // Note: WAVESIZE will be defined via compiler option -D
      WAVE_SIZE_ATTR
      [numthreads()" stringify(MAX_WAVESIZE) R"(*2,1,1)]
      void main() {
        data[0].count = WaveGetLaneCount();
      })";

  struct WaveSizeTestData {
    uint32_t count;
  };

  for (UINT waveSize = minWaveSize; waveSize <= maxWaveSize; waveSize *= 2) {
    // format compiler args
    char compilerOptions[64];
    VERIFY_IS_TRUE(sprintf_s(compilerOptions, sizeof(compilerOptions),
                             "-D WAVE_SIZE_ATTR=[wavesize(%d)]",
                             waveSize) != -1);

    // run the shader
    std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
        pDevice, m_support, "WaveSizeTest",
        [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
          VERIFY_IS_TRUE((0 == strncmp(Name, "UAVBuffer0", 10)));
          pShaderOp->Shaders.at(0).Arguments = compilerOptions;
          pShaderOp->Shaders.at(0).Text = waveSizeTestShader;

          VERIFY_IS_TRUE(sizeof(WaveSizeTestData) * MAX_WAVESIZE <=
                         Data.size());
          WaveSizeTestData *pInData = (WaveSizeTestData *)Data.data();
          memset(pInData, 0, sizeof(WaveSizeTestData) * MAX_WAVESIZE);
        },
        ShaderOpSet);

    // verify expected values
    MappedData dataUav;
    WaveSizeTestData *pOutData;

    test->Test->GetReadBackData("UAVBuffer0", &dataUav);
    VERIFY_ARE_EQUAL(sizeof(WaveSizeTestData) * MAX_WAVESIZE, dataUav.size());
    pOutData = (WaveSizeTestData *)dataUav.data();

    LogCommentFmt(L"Verifying test result for wave size %d", waveSize);

    VERIFY_ARE_EQUAL(pOutData[0].count, waveSize);
  }
}

bool TestShaderRangeAgainstRequirements(UINT shaderminws, UINT shadermaxws,
                                        UINT minws, UINT maxws) {
  if (shaderminws > maxws) {
    return false;
  }
  if (shadermaxws < minws) {
    return false;
  }
  return true;
}

void ExecuteWaveSizeRangeInstance(UINT minWaveSize, UINT maxWaveSize,
                                  std::shared_ptr<st::ShaderOpSet> ShaderOpSet,
                                  CComPtr<ID3D12Device> pDevice,
                                  dxc::DxcDllSupport &m_support,
                                  UINT minShaderWaveSize,
                                  UINT maxShaderWaveSize,
                                  UINT prefShaderWaveSize, bool usePreferred) {

  // format shader source
  const char waveSizeTestShader[] =
      R"(struct TestData { 
        uint count; 
      };
      RWStructuredBuffer<TestData> data : register(u0); 

      // Note: WAVE_SIZE_ATTR will be defined via compiler option -D
      WAVE_SIZE_ATTR
      [numthreads()" stringify(MAX_WAVESIZE) R"(*2,1,1)]
      void main(uint3 tid : SV_DispatchThreadID) {
        if (tid.x == 0 && tid.y == 0 && tid.z == 0) {
          data[0].count = WaveGetLaneCount();
        }
      })";

  // format compiler args
  char compilerOptions[64];
  if (usePreferred) {
    // putting spaces in between the %d's below will cause compilation issues.
    VERIFY_IS_TRUE(sprintf_s(compilerOptions, sizeof(compilerOptions),
                             "-D WAVE_SIZE_ATTR=[wavesize(%d,%d,%d)]",
                             minShaderWaveSize, maxShaderWaveSize,
                             prefShaderWaveSize) != -1);
    LogCommentFmt(L"Verifying wave size range test results for (min, max, "
                  L"preferred): (%d, %d, %d)",
                  minShaderWaveSize, maxShaderWaveSize, prefShaderWaveSize);
  } else {
    VERIFY_IS_TRUE(sprintf_s(compilerOptions, sizeof(compilerOptions),
                             "-D WAVE_SIZE_ATTR=[wavesize(%d,%d)]",
                             minShaderWaveSize, maxShaderWaveSize) != -1);
    LogCommentFmt(
        L"Verifying wave size range test results for (min, max): (%d, %d)",
        minShaderWaveSize, maxShaderWaveSize);
  }

  struct WaveSizeTestData {
    uint32_t count;
  };

  // run the shader
  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
      pDevice, m_support, "WaveSizeTest",
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
        VERIFY_IS_TRUE((0 == strncmp(Name, "UAVBuffer0", 10)));
        pShaderOp->Shaders.at(0).Arguments = compilerOptions;
        pShaderOp->Shaders.at(0).Text = waveSizeTestShader;
        pShaderOp->Shaders.at(0).Target = "cs_6_8";

        VERIFY_IS_TRUE(sizeof(WaveSizeTestData) * MAX_WAVESIZE <= Data.size());
        WaveSizeTestData *pInData = (WaveSizeTestData *)Data.data();
        memset(pInData, 0, sizeof(WaveSizeTestData) * MAX_WAVESIZE);
      },
      ShaderOpSet);

  // verify expected values
  MappedData dataUav;
  WaveSizeTestData *pOutData;

  // at this point we assume that the waverange size that
  // the shader specifies is legal.
  test->Test->GetReadBackData("UAVBuffer0", &dataUav);
  VERIFY_ARE_EQUAL(sizeof(WaveSizeTestData) * MAX_WAVESIZE, dataUav.size());
  pOutData = (WaveSizeTestData *)dataUav.data();

  unsigned count = pOutData[0].count;
  if (usePreferred && prefShaderWaveSize >= minWaveSize &&
      prefShaderWaveSize <= maxWaveSize) {
    VERIFY_ARE_EQUAL(count, prefShaderWaveSize);
  } else {
    VERIFY_IS_GREATER_THAN_OR_EQUAL(count, minWaveSize);
    VERIFY_IS_LESS_THAN_OR_EQUAL(count, maxWaveSize);
  }
}

void RunWaveSizeRangeTest(UINT minWaveSize, UINT maxWaveSize,
                          std::shared_ptr<st::ShaderOpSet> ShaderOpSet,
                          CComPtr<ID3D12Device> pDevice,
                          dxc::DxcDllSupport &m_support) {

  for (UINT minShaderWaveSize = 4; minShaderWaveSize <= maxWaveSize;
       minShaderWaveSize *= 2) {
    for (UINT maxShaderWaveSize = minShaderWaveSize * 2;
         maxShaderWaveSize <= 128; maxShaderWaveSize *= 2) {
      // Only allow valid shader wave ranges
      bool AcceptedByRuntime = TestShaderRangeAgainstRequirements(
          minShaderWaveSize, maxShaderWaveSize, minWaveSize, maxWaveSize);
      if (!AcceptedByRuntime) {
        continue;
      }

      ExecuteWaveSizeRangeInstance(
          minWaveSize, maxWaveSize, ShaderOpSet, pDevice, m_support,
          minShaderWaveSize, maxShaderWaveSize,
          /* prefShaderWaveSize won't be used, so set it to minShaderWaveSize*/
          minShaderWaveSize, false);

      for (UINT prefShaderWaveSize = minShaderWaveSize;
           prefShaderWaveSize <= maxShaderWaveSize; prefShaderWaveSize *= 2) {

        ExecuteWaveSizeRangeInstance(
            minWaveSize, maxWaveSize, ShaderOpSet, pDevice, m_support,
            minShaderWaveSize, maxShaderWaveSize, prefShaderWaveSize, true);
      }
    }
  }
}

void ExecutionTest::WaveSizeTest() {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL_6_6,
                    /*skipUnsupported*/ false)) {
    return;
  }

  // Check Wave support
  if (!DoesDeviceSupportWaveOps(pDevice)) {
    // Optional feature, so it's correct to not support it if declared as such.
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  // Get supported wave sizes
  D3D12_FEATURE_DATA_D3D12_OPTIONS1 waveOpts;
  VERIFY_SUCCEEDED(
      pDevice->CheckFeatureSupport((D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS1,
                                   &waveOpts, sizeof(waveOpts)));
  UINT minWaveSize = waveOpts.WaveLaneCountMin;
  UINT maxWaveSize = waveOpts.WaveLaneCountMax;

  DXASSERT_NOMSG(minWaveSize <= maxWaveSize);
  DXASSERT((minWaveSize & (minWaveSize - 1)) == 0, "must be a power of 2");
  DXASSERT((maxWaveSize & (maxWaveSize - 1)) == 0, "must be a power of 2");

  // read shader config
  CComPtr<IStream> pStream;
  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());

  LogCommentFmt(L"Testing WaveSize attribute for shader model 6.6.");
  RunWaveSizeTest(minWaveSize, maxWaveSize, ShaderOpSet, pDevice, m_support);
}

void ExecutionTest::WaveSizeRangeTest() {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL_6_8,
                    /*skipUnsupported*/ false)) {
    return;
  }

  // Check Wave support
  if (!DoesDeviceSupportWaveOps(pDevice)) {
    // Optional feature, so it's correct to not support it if declared as such.
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  // Get supported wave sizes
  D3D12_FEATURE_DATA_D3D12_OPTIONS1 waveOpts;
  VERIFY_SUCCEEDED(
      pDevice->CheckFeatureSupport((D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS1,
                                   &waveOpts, sizeof(waveOpts)));
  UINT minWaveSize = waveOpts.WaveLaneCountMin;
  UINT maxWaveSize = waveOpts.WaveLaneCountMax;

  DXASSERT_NOMSG(minWaveSize <= maxWaveSize);
  DXASSERT((minWaveSize & (minWaveSize - 1)) == 0, "must be a power of 2");
  DXASSERT((maxWaveSize & (maxWaveSize - 1)) == 0, "must be a power of 2");

  // read shader config
  CComPtr<IStream> pStream;
  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());

  LogCommentFmt(L"Testing WaveSize Range attribute for shader model 6.8.");
  RunWaveSizeTest(minWaveSize, maxWaveSize, ShaderOpSet, pDevice, m_support);

  RunWaveSizeRangeTest(minWaveSize, maxWaveSize, ShaderOpSet, pDevice,
                       m_support);
}

// Atomic operation testing

// Atomic tests take a single integer index as input and contort it into some
// kind of interesting contributor to the operation in question.
// So each vertex, pixel, thread, or other will have a unique index that
// produces a contributing value to the calculation which is stored in a small
// resource

// For arithmetic or bitwise operations, each contributor accumulates to the
// same location in the resource indexed by the operation type. Addition is in
// index 0 umin/umax are in 1 and 2 and so on.

// To make sure that the most significant bits are involved in the calculation,
// particularly in the case of 64-bit values, each contributing value is
// duplicated to the lower and upper halves of the value. There is an exception
// to this when addition exceeds the available size and also for compare and
// exchange explained below.

// For compare and exchange operations, 64 output locations are shared by the
// various lanes. Each lane attempts to write to a location that is shared with
// several others. The first one to write to it determines its contents, which
// will be the lane index <ix> in the upper bits and the output location index
// in the lower bits. This ensures that the compare operations consider the
// upper bits in the comparison. The initial compare store is followed by a
// compare exchange that compares for the value the current lane would have
// assigned there. Finally, the output of the cmpxchg is used to determine if
// the current lane should perform the final unconditional exchange. The values
// are verified by checking the lower bits for the matching location index and
// ensuring that the upper bits undergoing the same transformation result in the
// location index. For lane index <ix> the location is calculated and final
// result assigned as if by this code:
//    g_outputBuf[(ix/3)%64] = (ix << shBits) | ((ix/3)%64);

bool AtomicResultMatches(const BYTE *uResults, uint64_t gold, size_t size) {
  if (memcmp(uResults, &gold, size)) {
    if (size == 4)
      LogCommentFmt(L"  value %d is not %d", ((const uint32_t *)uResults)[0],
                    (uint32_t)gold);
    else
      LogCommentFmt(L"  value %lld is not %lld",
                    ((const uint64_t *)uResults)[0], gold);
    return false;
  }
  return true;
}

// Used to duplicate the lower half bits into the upper half bits of an integer
// To verify that the full value is being considered, many tests duplicate the
// results into the upper half
#define SHIFT(val, bits)                                                       \
  (((val) & ((1ULL << (bits)) - 1ULL)) | ((uint64_t)(val) << (bits)))

// Symbolic constants for the results
#define ADD_IDX 0
#define UMIN_IDX 1
#define UMAX_IDX 2
#define AND_IDX 3
#define OR_IDX 4
#define XOR_IDX 5

#define SMIN_IDX 0
#define SMAX_IDX 1

// Verify results for atomic operations. <uResults> and <sResults> are pointers
// to the readback resource sections containing unsigned and signed integers
// respectively. <pXchg> is a poiner to the readback resource containing the
// results of the compare and exchange operations tests. <stride> is the number
// of bytes between results for all of the results pointers. <maxIdx> is the
// number of indices that went into the results which is used to determine what
// the results should be. <bitSize> is the size in bits of the produced results,
// either 32 or 64.
void VerifyAtomicResults(const BYTE *uResults, const BYTE *sResults,
                         const BYTE *pXchg, size_t stride, uint64_t maxIdx,
                         size_t bitSize) {
  // Each atomic test performs the test on the value in the lower half
  // and also duplicated in the upper half of the value. The SHIFT macros
  // account for this. This is to verify that the upper bits are considered
  uint64_t shBits = bitSize / 2;
  size_t byteSize = bitSize / 8;

  // Test ADD Operation
  // ADD just sums all the indices. The result should the sum of the highest and
  // lowest indices multiplied by half the number of sums.
  uint64_t addResult = (maxIdx) * (maxIdx - 1) / 2;
  LogCommentFmt(L"Verifying %d-bit integer atomic add", bitSize);
  // For 32-bit values, the sum exceeds the 16 bit limit, so we can't duplicate
  // That's fine, the duplication is really for 64-bit values.
  if (bitSize < 64)
    VERIFY_IS_TRUE(
        AtomicResultMatches(uResults + stride * ADD_IDX, addResult, byteSize));
  else
    VERIFY_IS_TRUE(AtomicResultMatches(uResults + stride * ADD_IDX,
                                       SHIFT(addResult, shBits), byteSize));

  // Test MIN and MAX Operations

  // The result of a simple min and max of any sequence of indices would be
  // fairly uninteresting and certain erroneous behavior might mistakenly
  // produce the correct results.

  // To make it interesting, the contributing values will change depending on
  // the evenness of the index. On an even index, min and max operate on the
  // bitflipped index. For signed compares, this is interpretted as a negative
  // value and for unsigned, a very high value.

  // For unsigned min/max, index 0 will be bitflipped to ~0, which is
  // interpretted as the maximum Because zero is manipulated, this leaves 1 as
  // the lowest value.
  LogCommentFmt(L"Verifying %d-bit integer atomic umin", bitSize);
  VERIFY_IS_TRUE(AtomicResultMatches(uResults + stride * UMIN_IDX,
                                     SHIFT(1ULL, shBits), byteSize)); // UMin
  LogCommentFmt(L"Verifying %d-bit integer atomic umax", bitSize);
  VERIFY_IS_TRUE(AtomicResultMatches(uResults + stride * UMAX_IDX, ~0ULL,
                                     byteSize)); // UMax

  // For signed min/max, the index just before the last will be bitflipped
  // (maxIndex is always even). This is interpretted as -(maxIndex-1) and will
  // be the lowest The maxIndex will be unaltered and interpretted as the
  // highest.
  LogCommentFmt(L"Verifying %d-bit integer atomic smin", bitSize);
  VERIFY_IS_TRUE(AtomicResultMatches(sResults + stride * SMIN_IDX,
                                     SHIFT(-((int64_t)maxIdx - 1), shBits),
                                     byteSize)); // SMin
  LogCommentFmt(L"Verifying %d-bit integer atomic smax", bitSize);
  VERIFY_IS_TRUE(AtomicResultMatches(sResults + stride * SMAX_IDX,
                                     SHIFT(maxIdx - 1, shBits),
                                     byteSize)); // SMax

  // Test AND and OR operations.

  // For AND operations, all indices are bitflipped and ANDed to the previous
  // result. This means that the highest bits, which are never set by the
  // contributing indices will be set for all the indices, so they will be set
  // in the final result.

  // For OR operations, the indices are ORed to the previous result unaltered
  // This means that any bit that is set in any index will be set in the final
  // OR result.

  // In practice, this means that the cumulative result of the AND and OR
  // operations are bitflipped versions of each other. Finding the most
  // significant set bit by the max index or next power of two (pot) gives us
  // the pivot point for these results
  uint64_t nextPot = 1ULL << (bitSize - 1);
  for (; nextPot && !((maxIdx - 1) & (nextPot)); nextPot >>= 1) {
  }
  nextPot <<= 1;
  LogCommentFmt(L"Verifying %d-bit integer atomic and", bitSize);
  VERIFY_IS_TRUE(AtomicResultMatches(uResults + stride * AND_IDX,
                                     ~SHIFT(nextPot - 1, shBits),
                                     byteSize)); // And
  LogCommentFmt(L"Verifying %d-bit integer atomic or", bitSize);
  VERIFY_IS_TRUE(AtomicResultMatches(
      uResults + stride * OR_IDX, SHIFT(nextPot - 1, shBits), byteSize)); // Or

  // Test XOR operation

  // For XOR operations, a 1 is shifted by the number of spaces equal to the
  // index and XORed to the previous result. Because this would rapidely shift
  // off the end of the value, giving undefined and uninteresting results, the
  // index is moduloed to a value that will fit within the type size.

  // Because many of the tests use total numbers of lanes that can be evenly
  // divisible by 32 or 64, these values aren't used for the modulo since the
  // expected result might be zero, which could be encountered through erroneous
  // behavior.

  // Instead, one less than the type size in bits is used for the modulo.
  // Even though we don't know the actual order these operations are performed,
  // indices that make up a contiguous sequence of 31 or 63 values can be
  // thought of as one of a series of "passes". Each "pass" sets or clears the
  // bits depending on what's already there. if the number of the pass is odd,
  // the bits are being unset and all above the mod position should be set. If
  // even, the bits are in the process of being set and bits below the mod
  // position should be set.
  uint64_t xorResult = ((1ULL << ((maxIdx) % (bitSize - 1))) - 1);

  if (((maxIdx / (bitSize - 1)) & 1)) {
    xorResult ^= ~0ULL;
    // The XOR above may set uninvolved upper bits, messing up the compare. So
    // AND off the uninvolved bits.
    xorResult &= ((1ULL << (bitSize - 1)) - 1);
  }

  LogCommentFmt(L"Verifying %d-bit integer atomic xor", bitSize);
  VERIFY_IS_TRUE(
      AtomicResultMatches(uResults + stride * XOR_IDX, xorResult, byteSize));

  // Test CMP/XCHG Operations
  // This tests CompareStore, CompareExchange, and Exchange operations.

  // Unlike above, every lane isn't contributing to the same resource location
  // Instead, every lane competes with a few others to update the same resource
  // location. The first lane to find the contents of their location
  // uninitialized will update it. To verify that upper bits are considered in
  // the comparison and in the assignment, the value stored in the lowest bits
  // is the location index. This ensures that part will be the same for each of
  // the competing lanes. The uppermost bits are updated with the index of the
  // lane that got there first. Subsequent calls to CompareExchange will verify
  // this value matches and alter the content slightly. Finally, a simple check
  // of the output value to what the current lane would expect and a call to
  // exchange will update the value once more

  // To verify this has gone through properly, the upper portion is converted as
  // if to calculate the location index and compared with the location index.
  // It could be the index of any of several lanes that assign to that location,
  // but this ensures that it is not any lane outside of that group.
  // The lower bits are compared to the location index as well.
  LogCommentFmt(L"Verifying %d-bit integer atomic cmp/xchg results", bitSize);
  for (size_t i = 0; i < 64; i++) {
    uint64_t val = *((const uint64_t *)(pXchg + i * stride));
    // Verify lower bits match location index exactly
    VERIFY_ARE_EQUAL(i, val & ((1ULL << shBits) - 1ULL));
    // Verify that upper bits contain original index that transforms to location
    // index
    VERIFY_ARE_EQUAL(((val >> shBits) / 3) % 64, i);
  }
}

void VerifyAtomicsRawTest(std::shared_ptr<ShaderOpTestResult> test,
                          uint64_t maxIdx, size_t bitSize) {

  size_t stride = 8;
  // struct mirroring that in the shader
  struct AtomicStuff {
    float prepad[2][3];
    UINT uintEl[4];
    int sintEl[4];
    struct useless {
      uint32_t unused[3];
    } postpad;
    float last;
  };

  MappedData uintData, xchgData;

  test->Test->GetReadBackData("U0", &uintData);
  test->Test->GetReadBackData("U1", &xchgData);

  const AtomicStuff *pStruct = (AtomicStuff *)uintData.data();
  const AtomicStuff *pStrXchg = (AtomicStuff *)xchgData.data();

  LogCommentFmt(L"Verifying %d-bit integer atomic operations on "
                L"RWStructuredBuffer resource",
                bitSize);

  VerifyAtomicResults((const BYTE *)&(pStruct[0].uintEl[2]),
                      (const BYTE *)&(pStruct[1].sintEl[2]),
                      (const BYTE *)&(pStrXchg[0].uintEl[2]),
                      sizeof(AtomicStuff), maxIdx, bitSize);

  const BYTE *pUint = nullptr;
  const BYTE *pXchg = nullptr;

  test->Test->GetReadBackData("U2", &uintData);
  test->Test->GetReadBackData("U3", &xchgData);

  pUint = (BYTE *)uintData.data();
  pXchg = (BYTE *)xchgData.data();

  LogCommentFmt(L"Verifying %d-bit integer atomic operations on "
                L"RWByteAddressBuffer resource",
                bitSize);

  VerifyAtomicResults(pUint, pUint + stride * 6, pXchg, stride, maxIdx,
                      bitSize);
}

void VerifyAtomicsTypedTest(std::shared_ptr<ShaderOpTestResult> test,
                            uint64_t maxIdx, size_t bitSize) {

  size_t stride = 8;
  MappedData uintData, sintData, xchgData;
  const BYTE *pUint = nullptr;
  const BYTE *pSint = nullptr;
  const BYTE *pXchg = nullptr;

  // Typed resources can't share between 32 and 64 bits
  if (bitSize == 32) {
    test->Test->GetReadBackData("U6", &uintData);
    test->Test->GetReadBackData("U7", &sintData);
    test->Test->GetReadBackData("U8", &xchgData);
  } else {
    test->Test->GetReadBackData("U12", &uintData);
    test->Test->GetReadBackData("U13", &sintData);
    test->Test->GetReadBackData("U14", &xchgData);
  }

  pUint = (BYTE *)uintData.data();
  pSint = (BYTE *)sintData.data();
  pXchg = (BYTE *)xchgData.data();

  LogCommentFmt(
      L"Verifying %d-bit integer atomic operations on RWBuffer resource",
      bitSize);

  VerifyAtomicResults(pUint, pSint + stride, pXchg, stride, maxIdx, bitSize);

  // Typed resources can't share between 32 and 64 bits
  if (bitSize == 32) {
    test->Test->GetReadBackData("U9", &uintData);
    test->Test->GetReadBackData("U10", &sintData);
    test->Test->GetReadBackData("U11", &xchgData);
  } else {
    test->Test->GetReadBackData("U15", &uintData);
    test->Test->GetReadBackData("U16", &sintData);
    test->Test->GetReadBackData("U17", &xchgData);
  }

  pUint = (BYTE *)uintData.data();
  pSint = (BYTE *)sintData.data();
  pXchg = (BYTE *)xchgData.data();

  LogCommentFmt(
      L"Verifying %d-bit integer atomic operations on RWTexture resource",
      bitSize);

  VerifyAtomicResults(pUint, pSint + stride, pXchg, stride, maxIdx, bitSize);
}

void VerifyAtomicsSharedTest(std::shared_ptr<ShaderOpTestResult> test,
                             uint64_t maxIdx, size_t bitSize) {

  size_t stride = 8;
  MappedData uintData, xchgData;
  const BYTE *pUint = nullptr;
  const BYTE *pXchg = nullptr;

  test->Test->GetReadBackData("U4", &uintData);
  test->Test->GetReadBackData("U5", &xchgData);

  pUint = (BYTE *)uintData.data();
  pXchg = (BYTE *)xchgData.data();

  LogCommentFmt(
      L"Verifying %d-bit integer atomic operations on groupshared variables",
      bitSize);
  VerifyAtomicResults(pUint, pUint + stride * 6, pXchg, stride, maxIdx,
                      bitSize);
}

void VerifyAtomicsTest(std::shared_ptr<ShaderOpTestResult> test,
                       uint64_t maxIdx, size_t bitSize) {
  VerifyAtomicsRawTest(test, maxIdx, bitSize);
  VerifyAtomicsTypedTest(test, maxIdx, bitSize);
}

TEST_F(ExecutionTest, AtomicsTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice))
    return;

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());

  st::ShaderOp *pShaderOp = ShaderOpSet->GetShaderOp("AtomicsHeap");

  // Test compute shader
  LogCommentFmt(
      L"Verifying 32-bit integer atomic operations in compute shader");
  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
      pDevice, m_support, "AtomicsHeap", nullptr, ShaderOpSet);

  VerifyAtomicsTest(test, 32 * 32, 32);
  VerifyAtomicsSharedTest(test, 32 * 32, 32);

  // Test mesh shader if available
  pShaderOp->CS = nullptr;
  if (DoesDeviceSupportMeshShaders(pDevice)) {
    LogCommentFmt(L"Verifying 32-bit integer atomic operations in "
                  L"amp/mesh/pixel shaders");
    test = RunShaderOpTestAfterParse(pDevice, m_support, "AtomicsHeap", nullptr,
                                     ShaderOpSet);
    VerifyAtomicsTest(test, 8 * 8 * 2 + 8 * 8 * 2 + 64 * 64, 32);
    VerifyAtomicsSharedTest(test, 8 * 8 * 2 + 8 * 8 * 2, 32);
  }

  // Test Vertex + Pixel shader
  pShaderOp->MS = nullptr;
  LogCommentFmt(
      L"Verifying 32-bit integer atomic operations in vert/pixel shaders");
  test = RunShaderOpTestAfterParse(pDevice, m_support, "AtomicsHeap", nullptr,
                                   ShaderOpSet);
  VerifyAtomicsTest(test, 64 * 64 + 6, 32);
}

TEST_F(ExecutionTest, Atomics64Test) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL_6_6))
    return;

  if (!DoesDeviceSupportInt64(pDevice)) {
    WEX::Logging::Log::Comment(L"Device does not support int64 operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());

  st::ShaderOp *pShaderOp = ShaderOpSet->GetShaderOp("AtomicsRoot");

  // Reassign shader stages to 64-bit versions
  // Collect 64-bit shaders
  pShaderOp->CS = pShaderOp->GetString("CS");
  pShaderOp->VS = pShaderOp->GetString("VS");
  pShaderOp->PS = pShaderOp->GetString("PS");
  pShaderOp->AS = pShaderOp->GetString("AS");
  pShaderOp->MS = pShaderOp->GetString("MS");

  // Test compute shader
  LogCommentFmt(L"Verifying 64-bit integer atomic operations on raw buffers in "
                L"compute shader");
  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
      pDevice, m_support, "AtomicsRoot", nullptr, ShaderOpSet);
  VerifyAtomicsRawTest(test, 32 * 32, 64);

  // Test mesh shader if available
  pShaderOp->CS = nullptr;
  if (DoesDeviceSupportMeshShaders(pDevice)) {
    LogCommentFmt(L"Verifying 64-bit integer atomic operations on raw buffers "
                  L"in amp/mesh/pixel shader");
    test = RunShaderOpTestAfterParse(pDevice, m_support, "AtomicsRoot", nullptr,
                                     ShaderOpSet);
    VerifyAtomicsRawTest(test, 8 * 8 * 2 + 8 * 8 * 2 + 64 * 64, 64);
  }

  // Test Vertex + Pixel shader
  pShaderOp->MS = nullptr;
  LogCommentFmt(L"Verifying 64-bit integer atomic operations on raw buffers in "
                L"vert/pixel shader");
  test = RunShaderOpTestAfterParse(pDevice, m_support, "AtomicsRoot", nullptr,
                                   ShaderOpSet);
  VerifyAtomicsRawTest(test, 64 * 64 + 6, 64);
}

TEST_F(ExecutionTest, AtomicsRawHeap64Test) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL_6_6))
    return;

  if (!DoesDeviceSupportInt64(pDevice)) {
    WEX::Logging::Log::Comment(L"Device does not support int64 operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  if (!DoesDeviceSupportHeap64Atomics(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support 64-bit atomic operations on heap resources.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());

  st::ShaderOp *pShaderOp = ShaderOpSet->GetShaderOp("AtomicsHeap");

  // Reassign shader stages to 64-bit versions
  // Collect 64-bit shaders
  pShaderOp->CS = pShaderOp->GetString("CS64");
  pShaderOp->VS = pShaderOp->GetString("VS64");
  pShaderOp->PS = pShaderOp->GetString("PS64");
  pShaderOp->AS = pShaderOp->GetString("AS64");
  pShaderOp->MS = pShaderOp->GetString("MS64");

  // Test compute shader
  LogCommentFmt(L"Verifying 64-bit integer atomic operations on heap raw "
                L"buffers in compute shader");
  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
      pDevice, m_support, "AtomicsHeap", nullptr, ShaderOpSet);
  VerifyAtomicsRawTest(test, 32 * 32, 64);

  // Test mesh shader if available
  pShaderOp->CS = nullptr;
  if (DoesDeviceSupportMeshShaders(pDevice)) {
    LogCommentFmt(L"Verifying 64-bit integer atomic operations on heap raw "
                  L"buffers in amp/mesh/pixel shader");
    test = RunShaderOpTestAfterParse(pDevice, m_support, "AtomicsHeap", nullptr,
                                     ShaderOpSet);
    VerifyAtomicsRawTest(test, 8 * 8 * 2 + 8 * 8 * 2 + 64 * 64, 64);
  }

  // Test Vertex + Pixel shader
  pShaderOp->MS = nullptr;
  LogCommentFmt(L"Verifying 64-bit integer atomic operations on heap raw "
                L"buffers in vert/pixel shader");
  test = RunShaderOpTestAfterParse(pDevice, m_support, "AtomicsHeap", nullptr,
                                   ShaderOpSet);
  VerifyAtomicsRawTest(test, 64 * 64 + 6, 64);
}

TEST_F(ExecutionTest, AtomicsTyped64Test) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL_6_6))
    return;

  if (!DoesDeviceSupportInt64(pDevice)) {
    WEX::Logging::Log::Comment(L"Device does not support int64 operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  if (!DoesDeviceSupportTyped64Atomics(pDevice)) {
    WEX::Logging::Log::Comment(
        L"Device does not support int64 atomic operations on typed resources.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());

  st::ShaderOp *pShaderOp = ShaderOpSet->GetShaderOp("AtomicsHeap");

  // Reassign shader stages to 64-bit versions
  // Collect 64-bit shaders
  pShaderOp->CS = pShaderOp->GetString("CSTY64");
  pShaderOp->VS = pShaderOp->GetString("VSTY64");
  pShaderOp->PS = pShaderOp->GetString("PSTY64");
  pShaderOp->AS = pShaderOp->GetString("ASTY64");
  pShaderOp->MS = pShaderOp->GetString("MSTY64");

  // Test compute shader
  LogCommentFmt(L"Verifying 64-bit integer atomic operations on typed "
                L"resources in compute shader");
  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
      pDevice, m_support, "AtomicsHeap", nullptr, ShaderOpSet);
  VerifyAtomicsTypedTest(test, 32 * 32, 64);

  // Test mesh shader if available
  pShaderOp->CS = nullptr;
  if (DoesDeviceSupportMeshShaders(pDevice)) {
    LogCommentFmt(L"Verifying 64-bit integer atomic operations on typed "
                  L"resources in amp/mesh/pixel shader");
    test = RunShaderOpTestAfterParse(pDevice, m_support, "AtomicsHeap", nullptr,
                                     ShaderOpSet);
    VerifyAtomicsTypedTest(test, 8 * 8 * 2 + 8 * 8 * 2 + 64 * 64, 64);
  }

  // Test Vertex + Pixel shader
  pShaderOp->MS = nullptr;
  LogCommentFmt(L"Verifying 64-bit integer atomic operations on typed "
                L"resources in vert/pixel shader");
  test = RunShaderOpTestAfterParse(pDevice, m_support, "AtomicsHeap", nullptr,
                                   ShaderOpSet);
  VerifyAtomicsTypedTest(test, 64 * 64 + 6, 64);
}

TEST_F(ExecutionTest, AtomicsShared64Test) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice, D3D_SHADER_MODEL_6_6))
    return;

  if (!DoesDeviceSupportInt64(pDevice)) {
    WEX::Logging::Log::Comment(L"Device does not support int64 operations.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  if (!DoesDeviceSupportShared64Atomics(pDevice)) {
    WEX::Logging::Log::Comment(L"Device does not support int64 atomic "
                               L"operations on groupshared variables.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
  }

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());

  st::ShaderOp *pShaderOp = ShaderOpSet->GetShaderOp("AtomicsRoot");

  // Reassign shader stages to 64-bit versions
  // Collect 64-bit shaders
  pShaderOp->CS = pShaderOp->GetString("CSSH64");
  pShaderOp->AS = pShaderOp->GetString("ASSH64");
  pShaderOp->MS = pShaderOp->GetString("MSSH64");

  LogCommentFmt(L"Verifying 64-bit integer atomic operations on groupshared "
                L"variables in compute shader");
  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
      pDevice, m_support, "AtomicsRoot", nullptr, ShaderOpSet);
  VerifyAtomicsSharedTest(test, 32 * 32, 64);

  // Test mesh shader if available
  pShaderOp->CS = nullptr;
  if (DoesDeviceSupportMeshShaders(pDevice)) {
    LogCommentFmt(L"Verifying 64-bit integer atomic operations on groupshared "
                  L"variables in amp/mesh/pixel shader");
    test = RunShaderOpTestAfterParse(pDevice, m_support, "AtomicsRoot", nullptr,
                                     ShaderOpSet);
    VerifyAtomicsSharedTest(test, 8 * 8 * 2 + 8 * 8 * 2, 64);
  }
}

// Float Atomics

// These operations are almost the same as for the 32-bit and 64-bit integer
// tests The difference is that there is no need to verify the upper bits. So
// there is no storing of different parts in upper and lower halves.
// Additionally, the only operations that are supported on floats
// are compare and exchange operations. So that's all that is tested here.
// Just as above, a number of lanes are assigned the same output value.
// Unlike above, one location is needed for the result of the special NaN test
// For this reason, the conversion is reduced by one and shifted by one to leave
// the zero-indexed location available.

// Verify results for a particular set of atomics results
void VerifyAtomicFloatResults(const float *results) {
  // The first entry is for NaN to ensure that compares between NaNs succeed
  // The sentinal value is 0.123, for which this compare is sufficient.
  VERIFY_IS_TRUE(results[0] >= 0.120 && results[0] < 0.125);
  // Start at 1 because 0 is just for NaN tests
  for (int i = 1; i < 64; i++) {
    VERIFY_ARE_EQUAL((int(results[i]) / 3) % 63 + 1, i);
  }
}

void VerifyAtomicsFloatSharedTest(std::shared_ptr<ShaderOpTestResult> test) {
  MappedData Data;
  const float *pData = nullptr;

  test->Test->GetReadBackData("U4", &Data);
  pData = (float *)Data.data();

  LogCommentFmt(
      L"Verifying float cmp/xchg atomic operations on groupshared variables");
  VerifyAtomicFloatResults(pData);
}

void VerifyAtomicsFloatTest(std::shared_ptr<ShaderOpTestResult> test) {

  // struct mirroring that in the shader
  struct AtomicStuff {
    float prepad[2][3];
    float fltEl[2];
    struct useless {
      uint32_t unused[3];
    } postpad;
  };

  // Test Compute Shader
  MappedData Data;
  const float *pData = nullptr;

  test->Test->GetReadBackData("U0", &Data);
  const AtomicStuff *pStructData = (AtomicStuff *)Data.data();
  LogCommentFmt(L"Verifying float cmp/xchg atomic operations on "
                L"RWStructuredBuffer resources");
  VERIFY_IS_TRUE(pStructData[0].fltEl[1] >= 0.120 &&
                 pStructData[0].fltEl[1] < 0.125);
  for (int i = 1; i < 64; i++) {
    VERIFY_ARE_EQUAL((int(pStructData[i].fltEl[1]) / 3) % 63 + 1, i);
  }

  test->Test->GetReadBackData("U1", &Data);
  pData = (float *)Data.data();
  LogCommentFmt(L"Verifying float cmp/xchg atomic operations on "
                L"RWByteAddressBuffer resources");
  VerifyAtomicFloatResults(pData);

  test->Test->GetReadBackData("U2", &Data);
  pData = (float *)Data.data();
  LogCommentFmt(
      L"Verifying float cmp/xchg atomic operations on RWBuffer resources");
  VerifyAtomicFloatResults(pData);

  test->Test->GetReadBackData("U3", &Data);
  pData = (float *)Data.data();
  LogCommentFmt(
      L"Verifying float cmp/xchg atomic operations on RWTexture resources");
  VerifyAtomicFloatResults(pData);
}

TEST_F(ExecutionTest, AtomicsFloatTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  CComPtr<ID3D12Device> pDevice;
  if (!CreateDevice(&pDevice))
    return;

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());

  st::ShaderOp *pShaderOp = ShaderOpSet->GetShaderOp("FloatAtomics");

  // Test compute shader
  LogCommentFmt(
      L"Verifying float cmp/xchg atomic operations in compute shader");
  std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
      pDevice, m_support, "FloatAtomics", nullptr, ShaderOpSet);
  VerifyAtomicsFloatTest(test);
  VerifyAtomicsFloatSharedTest(test);

  // Test mesh shader if available
  pShaderOp->CS = nullptr;
  if (DoesDeviceSupportMeshShaders(pDevice)) {
    LogCommentFmt(L"Verifying float cmp/xchg atomic operations in "
                  L"amp/mesh/pixel shaders");
    test = RunShaderOpTestAfterParse(pDevice, m_support, "FloatAtomics",
                                     nullptr, ShaderOpSet);
    VerifyAtomicsFloatTest(test);
    VerifyAtomicsFloatSharedTest(test);
  }

  // Test Vertex + Pixel shader
  pShaderOp->MS = nullptr;
  LogCommentFmt(
      L"Verifying float cmp/xchg atomic operations in vert/pixel shaders");
  test = RunShaderOpTestAfterParse(pDevice, m_support, "FloatAtomics", nullptr,
                                   ShaderOpSet);
  VerifyAtomicsFloatTest(test);
}

// The IsHelperLane test renders 3-pixel triangle into 16x16 render target
// restricted to 2x2 viewport alligned at (0,0) which guarantees it will run in
// a single quad.
//
// Pixels to be rendered*
// (0,0)*  (0,1)*
// (1,0)   (1,1)*
//
// Pixel (1,0) is not rendered and is in helper lane.
//
// Each thread will use ddx_fine and ddy_fine to read the IsHelperLane() values
// from other threads. The bottom right pixel will write the results into the
// UAV buffer.
//
// Then the top level pixel (0,0) is discarded and the process above is
// repeated.
//
// Runs with shader models 6.0 and 6.6 to test both the HLSL built-in
// IsHelperLane fallback function (sm <= 6.5) and the IsHelperLane intrisics (sm
// >= 6.6).
//
TEST_F(ExecutionTest, HelperLaneTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());

  D3D_SHADER_MODEL TestShaderModels[] = {D3D_SHADER_MODEL_6_0,
                                         D3D_SHADER_MODEL_6_6};

  for (unsigned i = 0; i < _countof(TestShaderModels); i++) {
    D3D_SHADER_MODEL sm = TestShaderModels[i];
    LogCommentFmt(L"Verifying IsHelperLane in shader model 6.%1u",
                  ((UINT)sm & 0x0f));

    CComPtr<ID3D12Device> pDevice;
    if (!CreateDevice(&pDevice, sm, false /* skipUnsupported */))
      continue;

    std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
        pDevice, m_support, "HelperLaneTestNoWave",
        // this callbacked is called when the test is creating the resource to
        // run the test
        [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *pShaderOp) {
          VERIFY_IS_TRUE(0 == _stricmp(Name, "UAVBuffer0"));
          std::fill(Data.begin(), Data.end(), (BYTE)0xCC);
          UNREFERENCED_PARAMETER(pShaderOp);
        },
        ShaderOpSet);

    struct HelperLaneTestResult {
      int32_t is_helper_00;
      int32_t is_helper_10;
      int32_t is_helper_01;
      int32_t is_helper_11;
    };

    MappedData uavData;
    test->Test->GetReadBackData("UAVBuffer0", &uavData);
    HelperLaneTestResult *pTestResults = (HelperLaneTestResult *)uavData.data();

    MappedData renderData;
    test->Test->GetReadBackData("RTarget", &renderData);
    const uint32_t *pPixels = (uint32_t *)renderData.data();

    // before discard
    VERIFY_ARE_EQUAL(pTestResults[0].is_helper_00, 0);
    VERIFY_ARE_EQUAL(pTestResults[0].is_helper_10, 0);
    VERIFY_ARE_EQUAL(pTestResults[0].is_helper_01, 1);
    VERIFY_ARE_EQUAL(pTestResults[0].is_helper_11, 0);

    // after discard
    VERIFY_ARE_EQUAL(pTestResults[1].is_helper_00, 1);
    VERIFY_ARE_EQUAL(pTestResults[1].is_helper_10, 0);
    VERIFY_ARE_EQUAL(pTestResults[1].is_helper_01, 1);
    VERIFY_ARE_EQUAL(pTestResults[1].is_helper_11, 0);

    UNREFERENCED_PARAMETER(pPixels);
  }
}

struct HelperLaneWaveTestResult60 {
  // 6.0 wave ops
  int32_t anyTrue;
  int32_t allTrue;
  XMUINT4 ballot;
  int32_t waterfallLoopCount;
  int32_t allEqual;
  int32_t countBits;
  int32_t sum;
  int32_t product;
  int32_t bitAnd;
  int32_t bitOr;
  int32_t bitXor;
  int32_t min;
  int32_t max;
  int32_t prefixCountBits;
  int32_t prefixProduct;
  int32_t prefixSum;
};

struct HelperLaneQuadTestResult {
  int32_t is_helper_this;
  int32_t is_helper_across_X;
  int32_t is_helper_across_Y;
  int32_t is_helper_across_Diag;
};

struct HelperLaneWaveTestResult65 {
  // 6.5 wave ops
  XMUINT4 match;
  int32_t mpCountBits;
  int32_t mpSum;
  int32_t mpProduct;
  int32_t mpBitAnd;
  int32_t mpBitOr;
  int32_t mpBitXor;
};

struct HelperLaneWaveTestResult {
  HelperLaneWaveTestResult60 sm60;
  HelperLaneQuadTestResult sm60_quad;
  HelperLaneWaveTestResult65 sm65;
};

struct foo {
  int32_t a;
  int32_t b;
  int32_t c;
};
struct bar {
  foo f;
  int32_t d;
  XMUINT4 g;
};
foo f = {1, 2, 3};
bar b = {{1, 2, 3}, 0, {1, 2, 3, 4}};

HelperLaneWaveTestResult HelperLane_CS_ExpectedResults = {
    // HelperLaneWaveTestResult60
    {0, 1, {0x7, 0, 0, 0}, 3, 1, 3, 12, 64, 1, 0, 0, 10, 1, 2, 16, 4},
    // HelperLaneQuadTestResult
    {0, 0, 0, 0},
    // HelperLaneWaveTestResult65
    {{0x7, 0, 0, 0}, 2, 4, 16, 1, 0, 0}};

HelperLaneWaveTestResult HelperLane_VS_ExpectedResults =
    HelperLane_CS_ExpectedResults;

HelperLaneWaveTestResult HelperLane_PS_ExpectedResults = {
    // HelperLaneWaveTestResult60
    {0, 1, {0xB, 0, 0, 0}, 3, 1, 3, 12, 64, 1, 0, 0, 10, 1, 2, 16, 4},
    // HelperLaneQuadTestResult
    {0, 1, 0, 0},
    // HelperLaneWaveTestResult65
    {{0xB, 0, 0, 0}, 2, 4, 16, 1, 0, 0}};

HelperLaneWaveTestResult HelperLane_PSAfterDiscard_ExpectedResults = {
    // HelperLaneWaveTestResult60
    {0, 1, {0xA, 0, 0, 0}, 2, 1, 2, 8, 16, 1, 0, 0, 10, 1, 1, 4, 2},
    // HelperLaneQuadTestResult
    {0, 1, 0, 1},
    // HelperLaneWaveTestResult65
    {{0xA, 0, 0, 0}, 1, 2, 4, 1, 0, 0}};

HelperLaneWaveTestResult IncludesHelperLane_PS_ExpectedResults = {
    // HelperLaneWaveTestResult60
    {1, 0, {0xF, 0, 0, 0}, 4, 0, 4, 16, 256, 0, 1, 1, 1, 10, 3, 64, 6},
    // HelperLaneQuadTestResult
    {0, 1, 0, 0},
    // HelperLaneWaveTestResult65
    {{0xF, 0, 0, 0}, 3, 6, 64, 0, 1, 1}};

HelperLaneWaveTestResult IncludesHelperLane_PSAfterDiscard_ExpectedResults = {
    // HelperLaneWaveTestResult60
    {1, 0, {0xF, 0, 0, 0}, 4, 0, 4, 16, 256, 0, 1, 0, 1, 10, 3, 64, 6},
    // HelperLaneQuadTestResult
    {0, 1, 0, 1},
    // HelperLaneWaveTestResult65
    {{0xF, 0, 0, 0}, 3, 6, 64, 0, 1, 0}};

bool HelperLaneResultLogAndVerify(const wchar_t *testDesc,
                                  uint32_t expectedValue,
                                  uint32_t actualValue) {
  bool matches = (expectedValue == actualValue);
  LogCommentFmt(L"%s%s, expected = %u, actual = %u",
                matches ? L" - " : L"FAILED: ", testDesc, expectedValue,
                actualValue);
  return matches;
}

bool HelperLaneResultLogAndVerify(const wchar_t *testDesc,
                                  XMUINT4 expectedValue, XMUINT4 actualValue) {
  bool matches =
      (expectedValue.x == actualValue.x && expectedValue.y == actualValue.y &&
       expectedValue.z == actualValue.z && expectedValue.w == actualValue.w);
  LogCommentFmt(
      L"%s%s, expected = (0x%X,0x%X,0x%X,0x%X), actual = (0x%X,0x%X,0x%X,0x%X)",
      matches ? L" - " : L"FAILED: ", testDesc, expectedValue.x,
      expectedValue.y, expectedValue.z, expectedValue.w, actualValue.x,
      actualValue.y, actualValue.z, actualValue.w);
  return matches;
}

bool VerifyHelperLaneWaveResults(ExecutionTest::D3D_SHADER_MODEL sm,
                                 HelperLaneWaveTestResult &testResults,
                                 HelperLaneWaveTestResult &expectedResults,
                                 bool verifyQuads) {
  bool passed = true;
  {
    HelperLaneWaveTestResult60 &tr60 = testResults.sm60;
    HelperLaneWaveTestResult60 &tr60exp = expectedResults.sm60;

    passed &= HelperLaneResultLogAndVerify(L"WaveActiveAnyTrue(IsHelperLane())",
                                           tr60exp.anyTrue, tr60.anyTrue);
    passed &= HelperLaneResultLogAndVerify(
        L"WaveActiveAllTrue(!IsHelperLane())", tr60exp.allTrue, tr60.allTrue);
    passed &= HelperLaneResultLogAndVerify(
        L"WaveActiveBallot(true) has exactly 3 bits set", tr60exp.ballot,
        tr60.ballot);

    passed &= HelperLaneResultLogAndVerify(
        L"!WaveReadLaneFirst(IsHelperLane()) && WaveIsFirstLane() in a "
        L"waterfall loop",
        tr60exp.waterfallLoopCount, tr60.waterfallLoopCount);
    passed &= HelperLaneResultLogAndVerify(
        L"WaveActiveAllEqual(IsHelperLane())", tr60exp.allEqual, tr60.allEqual);
    passed &= HelperLaneResultLogAndVerify(L"WaveActiveCountBits(true)",
                                           tr60exp.countBits, tr60.countBits);
    passed &= HelperLaneResultLogAndVerify(L"WaveActiveSum(4)", tr60exp.sum,
                                           tr60.sum);
    passed &= HelperLaneResultLogAndVerify(L"WaveActiveProduct(4)",
                                           tr60exp.product, tr60.product);

    passed &= HelperLaneResultLogAndVerify(L"WaveActiveBitAnd(!IsHelperLane())",
                                           tr60exp.bitAnd, tr60.bitAnd);
    passed &= HelperLaneResultLogAndVerify(L"WaveActiveBitOr(IsHelperLane())",
                                           tr60exp.bitOr, tr60.bitOr);
    passed &= HelperLaneResultLogAndVerify(L"WaveActiveBitXor(IsHelperLane())",
                                           tr60exp.bitXor, tr60.bitXor);

    passed &= HelperLaneResultLogAndVerify(
        L"WaveActiveMin(IsHelperLane() ? 1 : 10)", tr60exp.min, tr60.min);
    passed &= HelperLaneResultLogAndVerify(
        L"WaveActiveMax(IsHelperLane() ? 10 : 1)", tr60exp.max, tr60.max);

    passed &= HelperLaneResultLogAndVerify(L"WavePrefixCountBits(1)",
                                           tr60exp.prefixCountBits,
                                           tr60.prefixCountBits);
    passed &= HelperLaneResultLogAndVerify(
        L"WavePrefixProduct(4)", tr60exp.prefixProduct, tr60.prefixProduct);
    passed &= HelperLaneResultLogAndVerify(L"WavePrefixSum(2)",
                                           tr60exp.prefixSum, tr60.prefixSum);
  }

  if (verifyQuads) {
    HelperLaneQuadTestResult &quad_tr = testResults.sm60_quad;
    HelperLaneQuadTestResult &quad_tr_exp = expectedResults.sm60_quad;
    passed &= HelperLaneResultLogAndVerify(
        L"QuadReadAcross* - lane 3 / pixel (1,1) - IsHelperLane()",
        quad_tr_exp.is_helper_this, quad_tr.is_helper_this);
    passed &= HelperLaneResultLogAndVerify(
        L"QuadReadAcross* - lane 2 / pixel (0,1) - IsHelperLane()",
        quad_tr_exp.is_helper_across_X, quad_tr.is_helper_across_X);
    passed &= HelperLaneResultLogAndVerify(
        L"QuadReadAcross* - lane 1 / pixel (1,0) - IsHelperLane()",
        quad_tr_exp.is_helper_across_Y, quad_tr.is_helper_across_Y);
    passed &= HelperLaneResultLogAndVerify(
        L"QuadReadAcross* - lane 0 / pixel (0,0) - IsHelperLane()",
        quad_tr_exp.is_helper_across_Diag, quad_tr.is_helper_across_Diag);
  }

  if (sm >= ExecutionTest::D3D_SHADER_MODEL_6_5) {
    HelperLaneWaveTestResult65 &tr65 = testResults.sm65;
    HelperLaneWaveTestResult65 &tr65exp = expectedResults.sm65;

    passed &= HelperLaneResultLogAndVerify(
        L"WaveMatch(true) has exactly 3 bits set", tr65exp.match, tr65.match);
    passed &= HelperLaneResultLogAndVerify(
        L"WaveMultiPrefixCountBits(1, no_masked_bits)", tr65exp.mpCountBits,
        tr65.mpCountBits);
    passed &= HelperLaneResultLogAndVerify(
        L"WaveMultiPrefixSum(2, no_masked_bits)", tr65exp.mpSum, tr65.mpSum);
    passed &= HelperLaneResultLogAndVerify(
        L"WaveMultiPrefixProduct(4, no_masked_bits)", tr65exp.mpProduct,
        tr65.mpProduct);

    passed &= HelperLaneResultLogAndVerify(
        L"WaveMultiPrefixAnd(IsHelperLane() ? 0 : 1, no_masked_bits)",
        tr65exp.mpBitAnd, tr65.mpBitAnd);
    passed &= HelperLaneResultLogAndVerify(
        L"WaveMultiPrefixOr(IsHelperLane() ? 1 : 0, no_masked_bits)",
        tr65exp.mpBitOr, tr65.mpBitOr);
    passed &= HelperLaneResultLogAndVerify(
        L"verify WaveMultiPrefixXor(IsHelperLane() ? 1 : 0, no_masked_bits)",
        tr65exp.mpBitXor, tr65.mpBitXor);
  }
  return passed;
}
// Contrary to compute or pixel shaders the layout of lanes in vertex shaders is
// not specified. A conforming implementation could, in the extreme case, decide
// to dispatch three waves that each process only a single vertex.
// So instead of compare with fixed expected result, calculate the correct
// result from ballot.
bool VerifyHelperLaneWaveResultsForVS(ExecutionTest::D3D_SHADER_MODEL sm,
                                      HelperLaneWaveTestResult &testResults) {
  bool passed = true;
  XMUINT4 mask = testResults.sm60.ballot;
  unsigned countBits = 0;
  std::bitset<32> x(mask.x);
  std::bitset<32> y(mask.y);
  std::bitset<32> z(mask.z);
  std::bitset<32> w(mask.w);
  countBits += (unsigned)x.count();
  countBits += (unsigned)y.count();
  countBits += (unsigned)z.count();
  countBits += (unsigned)w.count();

  {
    // For VS, IsHelperLane always return false.
    HelperLaneWaveTestResult60 &tr60 = testResults.sm60;
    passed &= HelperLaneResultLogAndVerify(L"WaveActiveAnyTrue(IsHelperLane())",
                                           0, tr60.anyTrue);
    passed &= HelperLaneResultLogAndVerify(
        L"WaveActiveAllTrue(!IsHelperLane())", 1, tr60.allTrue);
    bool ballotMatch = 1 <= countBits && countBits <= 3;

    LogCommentFmt(
        L"%sWaveActiveBallot(true) expected 1~3 bits set, actual = %u",
        ballotMatch ? L" - " : L"FAILED: ", tr60.ballot);

    passed &= HelperLaneResultLogAndVerify(
        L"!WaveReadLaneFirst(IsHelperLane()) && WaveIsFirstLane() in a "
        L"waterfall loop",
        countBits, tr60.waterfallLoopCount);
    passed &= HelperLaneResultLogAndVerify(
        L"WaveActiveAllEqual(IsHelperLane())", 1, tr60.allEqual);
    passed &= HelperLaneResultLogAndVerify(L"WaveActiveCountBits(true)",
                                           countBits, tr60.countBits);
    passed &= HelperLaneResultLogAndVerify(L"WaveActiveSum(4)", 4 * countBits,
                                           tr60.sum);
    passed &= HelperLaneResultLogAndVerify(L"WaveActiveProduct(4)",
                                           (unsigned)std::pow(4, countBits),
                                           tr60.product);

    passed &= HelperLaneResultLogAndVerify(L"WaveActiveBitAnd(!IsHelperLane())",
                                           1, tr60.bitAnd);
    passed &= HelperLaneResultLogAndVerify(L"WaveActiveBitOr(IsHelperLane())",
                                           0, tr60.bitOr);
    passed &= HelperLaneResultLogAndVerify(L"WaveActiveBitXor(IsHelperLane())",
                                           0, tr60.bitXor);

    passed &= HelperLaneResultLogAndVerify(
        L"WaveActiveMin(IsHelperLane() ? 1 : 10)", 10, tr60.min);
    passed &= HelperLaneResultLogAndVerify(
        L"WaveActiveMax(IsHelperLane() ? 10 : 1)", 1, tr60.max);

    passed &= HelperLaneResultLogAndVerify(L"WavePrefixCountBits(1)",
                                           countBits - 1, tr60.prefixCountBits);
    passed &= HelperLaneResultLogAndVerify(L"WavePrefixProduct(4)",
                                           (unsigned)std::pow(4, countBits - 1),
                                           tr60.prefixProduct);
    passed &= HelperLaneResultLogAndVerify(L"WavePrefixSum(2)",
                                           2 * (countBits - 1), tr60.prefixSum);
  }

  if (sm >= ExecutionTest::D3D_SHADER_MODEL_6_5) {
    HelperLaneWaveTestResult65 &tr65 = testResults.sm65;

    passed &= HelperLaneResultLogAndVerify(
        L"WaveMatch(true) has exactly 3 bits set", mask, tr65.match);
    passed &= HelperLaneResultLogAndVerify(
        L"WaveMultiPrefixCountBits(1, no_masked_bits)", countBits - 1,
        tr65.mpCountBits);
    passed &=
        HelperLaneResultLogAndVerify(L"WaveMultiPrefixSum(2, no_masked_bits)",
                                     2 * (countBits - 1), tr65.mpSum);
    passed &= HelperLaneResultLogAndVerify(
        L"WaveMultiPrefixProduct(4, no_masked_bits)",
        (unsigned)std::pow(4, countBits - 1), tr65.mpProduct);

    passed &= HelperLaneResultLogAndVerify(
        L"WaveMultiPrefixAnd(IsHelperLane() ? 0 : 1, no_masked_bits)", 1,
        tr65.mpBitAnd);
    passed &= HelperLaneResultLogAndVerify(
        L"WaveMultiPrefixOr(IsHelperLane() ? 1 : 0, no_masked_bits)", 0,
        tr65.mpBitOr);
    passed &= HelperLaneResultLogAndVerify(
        L"verify WaveMultiPrefixXor(IsHelperLane() ? 1 : 0, no_masked_bits)", 0,
        tr65.mpBitXor);
  }
  return passed;
}

void CleanUAVBuffer0Buffer(LPCSTR BufferName, std::vector<BYTE> &Data,
                           st::ShaderOp *pShaderOp) {
  UNREFERENCED_PARAMETER(pShaderOp);
  VERIFY_IS_TRUE(0 == _stricmp(BufferName, "UAVBuffer0"));
  std::fill(Data.begin(), Data.end(), (BYTE)0xCC);
}

//
// The IsHelperLane test that use Wave intrinsics to verify IsHelperLane() and
// Wave operations on active lanes.
//
// Runs with shader models 6.0, 6.5 and 6.6 to test both the HLSL built-in
// IsHelperLane fallback function (sm <= 6.5) and the IsHelperLane intrisics (sm
// >= 6.6) and the shader model 6.5 wave intrinsics (sm >= 6.5).
//
// For compute and vertex shaders IsHelperLane() always returns false and might
// be optimized away in the front end. However it can be exposed to the driver
// in CS/VS through an exported function in a library so drivers need to be
// prepared to handle it. For this reason the test is compiled with disabled
// optimizations (/Od). The tests are also validating that wave intrinsics
// operate correctly with 3 threads in a CS or 3 vertices in a VS where the rest
// of the lanes in the wave are not active (dead lanes).
//
TEST_F(ExecutionTest, HelperLaneTestWave) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());
  st::ShaderOp *pShaderOp = ShaderOpSet->GetShaderOp("HelperLaneTestWave");

  bool testPassed = true;

  D3D_SHADER_MODEL TestShaderModels[] = {
      D3D_SHADER_MODEL_6_0, D3D_SHADER_MODEL_6_5, D3D_SHADER_MODEL_6_6,
      D3D_SHADER_MODEL_6_7};
  for (unsigned i = 0; i < _countof(TestShaderModels); i++) {
    D3D_SHADER_MODEL sm = TestShaderModels[i];
    LogCommentFmt(L"\r\nVerifying IsHelperLane using Wave intrinsics in shader "
                  L"model 6.%1u",
                  ((UINT)sm & 0x0f));

    bool smPassed = true;

    CComPtr<ID3D12Device> pDevice;
    if (!CreateDevice(&pDevice, sm, false /* skipUnsupported */)) {
      continue;
    }

    if (!DoesDeviceSupportWaveOps(pDevice)) {
      LogCommentFmt(
          L"Device does not support wave operations in shader model 6.%1u",
          ((UINT)sm & 0x0f));
      continue;
    }

    if (sm == D3D_SHADER_MODEL_6_5) {
      // Reassign shader stages to 6.5 versions
      pShaderOp->CS = pShaderOp->GetString("CS65");
      pShaderOp->VS = pShaderOp->GetString("VS65");
      pShaderOp->PS = pShaderOp->GetString("PS65");
    } else if (sm == D3D_SHADER_MODEL_6_6) {
      // Reassign shader stages to 6.6 versions
      pShaderOp->CS = pShaderOp->GetString("CS66");
      pShaderOp->VS = pShaderOp->GetString("VS66");
      pShaderOp->PS = pShaderOp->GetString("PS66");
    } else if (sm == D3D_SHADER_MODEL_6_7) {
      // Reassign shader stages to 6.7 versions
      pShaderOp->CS = pShaderOp->GetString("CS66");
      pShaderOp->VS = pShaderOp->GetString("VS66");
      // Only PS has SM 6.7 version to test new [WaveOpsIncludeHelperLanes]
      // attribute
      pShaderOp->PS = pShaderOp->GetString("PS67");
    }

    const unsigned CS_INDEX = 0, VS_INDEX = 0, PS_INDEX = 1,
                   PS_INDEX_AFTER_DISCARD = 2;

    // Test Compute shader
    {
      std::shared_ptr<ShaderOpTestResult> test =
          RunShaderOpTestAfterParse(pDevice, m_support, "HelperLaneTestWave",
                                    CleanUAVBuffer0Buffer, ShaderOpSet);

      MappedData uavData;
      test->Test->GetReadBackData("UAVBuffer0", &uavData);
      HelperLaneWaveTestResult *pTestResults =
          (HelperLaneWaveTestResult *)uavData.data();
      LogCommentFmt(L"\r\nCompute shader");
      smPassed &= VerifyHelperLaneWaveResults(
          sm, pTestResults[CS_INDEX], HelperLane_CS_ExpectedResults, true);
    }

    HelperLaneWaveTestResult &PS_ExpectedResults =
        (sm >= D3D_SHADER_MODEL_6_7) ? IncludesHelperLane_PS_ExpectedResults
                                     : HelperLane_PS_ExpectedResults;
    HelperLaneWaveTestResult &PSAfterDiscard_ExpectedResults =
        (sm >= D3D_SHADER_MODEL_6_7)
            ? IncludesHelperLane_PSAfterDiscard_ExpectedResults
            : HelperLane_PSAfterDiscard_ExpectedResults;

    // Test Vertex + Pixel shader
    {
      pShaderOp->CS = nullptr;
      std::shared_ptr<ShaderOpTestResult> test =
          RunShaderOpTestAfterParse(pDevice, m_support, "HelperLaneTestWave",
                                    CleanUAVBuffer0Buffer, ShaderOpSet);

      MappedData uavData;
      test->Test->GetReadBackData("UAVBuffer0", &uavData);
      HelperLaneWaveTestResult *pTestResults =
          (HelperLaneWaveTestResult *)uavData.data();
      LogCommentFmt(L"\r\nVertex shader");
      smPassed &= VerifyHelperLaneWaveResultsForVS(sm, pTestResults[VS_INDEX]);
      LogCommentFmt(L"\r\nPixel shader");
      smPassed &= VerifyHelperLaneWaveResults(sm, pTestResults[PS_INDEX],
                                              PS_ExpectedResults, true);
      LogCommentFmt(L"\r\nPixel shader with discarded pixel");
      smPassed &=
          VerifyHelperLaneWaveResults(sm, pTestResults[PS_INDEX_AFTER_DISCARD],
                                      PSAfterDiscard_ExpectedResults, true);

      MappedData renderData;
      test->Test->GetReadBackData("RTarget", &renderData);
      const uint32_t *pPixels = (uint32_t *)renderData.data();

      UNREFERENCED_PARAMETER(pPixels);
    }
    testPassed &= smPassed;
  }
  VERIFY_ARE_EQUAL(testPassed, true);
}

struct int2 {
  int x;
  int y;
};

bool VerifyQuadAnyAllResults(int2 *Res) {
  int Idx = 0;
  for (; Idx < 4; ++Idx) {
    if (Res[Idx].x != 2)
      return false;
    if (Res[Idx].y != 4)
      return false;
  }
  for (; Idx < 60; ++Idx) {
    if (Res[Idx].x != 1)
      return false;
    if (Res[Idx].y != 4)
      return false;
  }
  for (; Idx < 64; ++Idx) {
    if (Res[Idx].x != 1)
      return false;
    if (Res[Idx].y != 3)
      return false;
  }
  return true;
}

TEST_F(ExecutionTest, QuadAnyAll) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());
  st::ShaderOp *pShaderOp = ShaderOpSet->GetShaderOp("QuadAnyAll");

  LPCSTR args = "/Od";

  if (args[0]) {
    for (st::ShaderOpShader &S : pShaderOp->Shaders)
      S.Arguments = args;
  }

  bool Skipped = true;
  D3D_SHADER_MODEL TestShaderModels[] = {
      D3D_SHADER_MODEL_6_0, D3D_SHADER_MODEL_6_5, D3D_SHADER_MODEL_6_7};
  for (unsigned i = 0; i < _countof(TestShaderModels); i++) {
    D3D_SHADER_MODEL sm = TestShaderModels[i];
    LogCommentFmt(L"\r\nVerifying QuadAny/QuadAll using Wave intrinsics in "
                  L"shader model 6.%1u",
                  ((UINT)sm & 0x0f));

    if (sm == D3D_SHADER_MODEL_6_5) {
      pShaderOp->MS = pShaderOp->GetString("MS");
      pShaderOp->AS = pShaderOp->GetString("AS");
    } else if (sm == D3D_SHADER_MODEL_6_7) {
      pShaderOp->AS = pShaderOp->GetString("AS67");
      pShaderOp->MS = pShaderOp->GetString("MS67");
      pShaderOp->CS = pShaderOp->GetString("CS67");
    }

    CComPtr<ID3D12Device> pDevice;
    if (!CreateDevice(&pDevice, sm, false /* skipUnsupported */)) {
      continue;
    }

    if (!DoesDeviceSupportWaveOps(pDevice)) {
      LogCommentFmt(
          L"Device does not support wave operations in shader model 6.%1u",
          ((UINT)sm & 0x0f));
      continue;
    }
    Skipped = false;

    // test compute
    std::shared_ptr<ShaderOpTestResult> test = RunShaderOpTestAfterParse(
        pDevice, m_support, "QuadAnyAll", CleanUAVBuffer0Buffer, ShaderOpSet);

    MappedData uavData;
    test->Test->GetReadBackData("UAVBuffer0", &uavData);
    bool Result = VerifyQuadAnyAllResults((int2 *)uavData.data());
    VERIFY_IS_TRUE(Result);

    if (sm < D3D_SHADER_MODEL_6_5 || !DoesDeviceSupportMeshShaders(pDevice))
      continue;

    pShaderOp->CS = nullptr;
    // test AS/MS
    test = RunShaderOpTestAfterParse(pDevice, m_support, "QuadAnyAll",
                                     CleanUAVBuffer0Buffer, ShaderOpSet);

    test->Test->GetReadBackData("UAVBuffer0", &uavData);
    Result = VerifyQuadAnyAllResults((int2 *)uavData.data());
    VERIFY_IS_TRUE(Result);
    Result = VerifyQuadAnyAllResults(&((int2 *)uavData.data())[64]);
    VERIFY_IS_TRUE(Result);
  }
  if (Skipped)
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
}

// Copies input strings to local storage, so it doesn't rely on lifetime of
// input string pointers.
st::ShaderOpTest::TShaderCallbackFn MakeShaderReplacementCallback(
    std::vector<std::wstring> dxcArgs, std::vector<std::string> lookFors,
    std::vector<std::string> replacements, dxc::DxcDllSupport &dllSupport) {

  auto ShaderInitFn = [dxcArgs, lookFors, replacements, &dllSupport](
                          LPCSTR Name, LPCSTR pText, IDxcBlob **ppShaderBlob,
                          st::ShaderOp *pShaderOp) {
    UNREFERENCED_PARAMETER(pShaderOp);
    UNREFERENCED_PARAMETER(Name);
    // Create pointer vectors from local storage to supply API needs
    std::vector<LPCWSTR> Args(dxcArgs.size());
    for (unsigned i = 0; i < dxcArgs.size(); ++i)
      Args[i] = dxcArgs[i].c_str();

    CComPtr<IDxcUtils> pUtils;
    VERIFY_SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcUtils, &pUtils));

    // Compile original HLSL with op to replace, and disassemble.
    CComPtr<IDxcCompiler3> pCompiler;
    VERIFY_SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
    CComPtr<IDxcBlob> compiledShader;
    {
      CComPtr<IDxcResult> pResult;
      DxcBuffer source = {pText, strlen(pText), DXC_CP_UTF8};
      VERIFY_SUCCEEDED(pCompiler->Compile(&source, Args.data(),
                                          (UINT32)Args.size(), nullptr,
                                          IID_PPV_ARGS(&pResult)));
      HRESULT hrCompile;
      VERIFY_SUCCEEDED(pResult->GetStatus(&hrCompile));
      VERIFY_SUCCEEDED(hrCompile);
      VERIFY_SUCCEEDED(pResult->GetResult(&compiledShader));
    }

    // Disassemble
    std::string disassembly;
    {
      CComPtr<IDxcResult> pDisassemblyResult;
      CComPtr<IDxcBlobUtf8> pDisassembly;
      DxcBuffer compiledBuffer = {compiledShader->GetBufferPointer(),
                                  compiledShader->GetBufferSize(), 0};
      VERIFY_SUCCEEDED(pCompiler->Disassemble(
          &compiledBuffer, IID_PPV_ARGS(&pDisassemblyResult)));
      VERIFY_SUCCEEDED(pDisassemblyResult->GetOutput(
          DXC_OUT_DISASSEMBLY, IID_PPV_ARGS(&pDisassembly), nullptr));
      disassembly.assign(pDisassembly->GetStringPointer(),
                         pDisassembly->GetStringLength());
    }

    // Replace op
    strreplace(lookFors, replacements, disassembly);

    // Wrap text in UTF8 blob
    // No need to copy, disassembly won't be changed again and will live as long
    // as rewrittenDisassembly. c_str() guarantees null termination; passing
    // size + 1 to include it will create an IDxcBlobUtf8 without copying.
    CComPtr<IDxcBlobEncoding> rewrittenDisassembly;
    VERIFY_SUCCEEDED(pUtils->CreateBlobFromPinned(
        disassembly.c_str(), (UINT32)disassembly.size() + 1, DXC_CP_UTF8,
        &rewrittenDisassembly));
    // Assemble to container
    CComPtr<IDxcBlob> assembledShader;
    {
      CComPtr<IDxcAssembler> pAssembler;
      CComPtr<IDxcOperationResult> pResult;
      CComPtr<IDxcOperationResult> pValidationResult;
      CComPtr<IDxcValidator> pValidator;

      HRESULT status;
      HRESULT validationStatus;
      VERIFY_SUCCEEDED(
          dllSupport.CreateInstance(CLSID_DxcAssembler, &pAssembler));
      VERIFY_SUCCEEDED(
          pAssembler->AssembleToContainer(rewrittenDisassembly, &pResult));
      VERIFY_SUCCEEDED(pResult->GetStatus(&status));
      VERIFY_SUCCEEDED(status);
      VERIFY_SUCCEEDED(pResult->GetResult(&assembledShader));

      // now validate the rewritten disassembly and sign the shader
      VERIFY_SUCCEEDED(
          dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));

      VERIFY_SUCCEEDED(pValidator->Validate(
          assembledShader, DxcValidatorFlags_InPlaceEdit, &pValidationResult));
      VERIFY_SUCCEEDED(pValidationResult->GetStatus(&validationStatus));
      VERIFY_SUCCEEDED(validationStatus);
    }

    // Find root signature part in container
    CComPtr<IDxcContainerReflection> pReflection;
    VERIFY_SUCCEEDED(
        dllSupport.CreateInstance(CLSID_DxcContainerReflection, &pReflection));
    VERIFY_SUCCEEDED(pReflection->Load(compiledShader));
    UINT32 iPartIndex;
    if (FAILED(pReflection->FindFirstPartKind(DXC_PART_ROOT_SIGNATURE,
                                              &iPartIndex))) {
      // No root signature to copy, use the assembledShader.
      *ppShaderBlob = assembledShader.Detach();
      return;
    }

    CComPtr<IDxcContainerBuilder> pBuilder;
    VERIFY_SUCCEEDED(
        dllSupport.CreateInstance(CLSID_DxcContainerBuilder, &pBuilder));
    VERIFY_SUCCEEDED(pBuilder->Load(assembledShader));

    // Wrap root signature in blob
    CComPtr<IDxcBlob> pRootSignatureBlob;
    VERIFY_SUCCEEDED(
        pReflection->GetPartContent(iPartIndex, &pRootSignatureBlob));
    // Add root signature to container
    pBuilder->AddPart(DXC_PART_ROOT_SIGNATURE, pRootSignatureBlob);

    CComPtr<IDxcOperationResult> pOpResult;
    VERIFY_SUCCEEDED(pBuilder->SerializeContainer(&pOpResult));
    HRESULT status;
    VERIFY_SUCCEEDED(pOpResult->GetStatus(&status));
    VERIFY_SUCCEEDED(status);
    VERIFY_SUCCEEDED(pOpResult->GetResult(ppShaderBlob));
  };

  return ShaderInitFn;
}

struct FloatInputUintOutput {
  float input;
  unsigned int output;
};

TEST_F(ExecutionTest, IsNormalTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  CComPtr<ID3D12Device> pDevice;
  VERIFY_IS_TRUE(CreateDevice(&pDevice, D3D_SHADER_MODEL_6_0,
                              false /* skipUnsupported */));

  // The input is -Zero, Zero, -Denormal, Denormal, -Infinity, Infinity, -NaN,
  // Nan, and then 4 normal float numbers. Only the last 4 floats are normal, so
  // we expect the first 8 results to be 0, and the last 4 to be 1, as defined
  // by IsNormal.
  std::vector<float> Validation_Input_Vec = {
      -0.0,   0.0, -(FLT_MIN / 2), FLT_MIN / 2, -(INFINITY), INFINITY,
      -(NAN), NAN, 530.99f,        -530.99f,    -122.900f,   .122900f};
  std::vector<float> *Validation_Input = &Validation_Input_Vec;

  std::vector<unsigned int> Validation_Expected_Vec = {0u, 0u, 0u, 0u, 0u, 0u,
                                                       0u, 0u, 1u, 1u, 1u, 1u};
  std::vector<unsigned int> *Validation_Expected = &Validation_Expected_Vec;

  CComPtr<IStream> pStream;
  ReadHlslDataIntoNewStream(L"ShaderOpArith.xml", &pStream);

  std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
      std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());
  st::ShaderOp *pShaderOp = ShaderOpSet->GetShaderOp("IsNormal");

  D3D_SHADER_MODEL sm = D3D_SHADER_MODEL_6_0;
  LogCommentFmt(L"\r\nVerifying isNormal in shader "
                L"model 6.%1u",
                ((UINT)sm & 0x0f));

  size_t count = Validation_Input->size();

  auto ShaderInitFn = MakeShaderReplacementCallback(
      {L"isSpecialFloat.hlsl", L"-Emain", L"-Tcs_6_0"},
      // Replace the above with what's below when IsSpecialFloat supports
      // doubles
      //{ "@dx.op.isSpecialFloat.f32(i32 8,",  "@dx.op.isSpecialFloat.f64(i32
      // 8," }, { "@dx.op.isSpecialFloat.f32(i32 11,",
      //"@dx.op.isSpecialFloat.f64(i32 11," },
      {"@dx.op.isSpecialFloat.f32(i32 8,"},
      {"@dx.op.isSpecialFloat.f32(i32 11,"}, m_support);

  auto ResourceInitFn = [&](LPCSTR Name, std::vector<BYTE> &Data,
                            st::ShaderOp *pShaderOp) {
    UNREFERENCED_PARAMETER(pShaderOp);
    VERIFY_IS_TRUE(0 == _stricmp(Name, "g_TestData"));
    size_t size = sizeof(FloatInputUintOutput) * count;
    Data.resize(size);
    FloatInputUintOutput *pPrimitives = (FloatInputUintOutput *)Data.data();
    for (size_t i = 0; i < count; ++i) {
      FloatInputUintOutput *p = &pPrimitives[i];
      float inputFloat = (*Validation_Input)[i % Validation_Input->size()];
      p->input = inputFloat;
    }
  };

  // Test Compute shader
  {
    pShaderOp->CS = pShaderOp->GetString("CS60");
    std::shared_ptr<ShaderOpTestResult> test =
        RunShaderOpTestAfterParse(pDevice, m_support, "IsNormal",
                                  ResourceInitFn, ShaderInitFn, ShaderOpSet);

    MappedData data;
    test->Test->GetReadBackData("g_TestData", &data);

    FloatInputUintOutput *pPrimitives = (FloatInputUintOutput *)data.data();
    WEX::TestExecution::DisableVerifyExceptions dve;
    for (unsigned i = 0; i < count; ++i) {
      FloatInputUintOutput *p = &pPrimitives[i];
      unsigned int val =
          (*Validation_Expected)[i % Validation_Expected->size()];
      LogCommentFmt(
          L"element #%u, input = %6.8f, output = %6.8f, expected = %d", i,
          p->input, p->output, val);
      VERIFY_ARE_EQUAL(p->output, val);
    }
  }
}

#ifndef _HLK_CONF
static void WriteReadBackDump(st::ShaderOp *pShaderOp, st::ShaderOpTest *pTest,
                              char **pReadBackDump) {
  std::stringstream str;

  unsigned count = 0;
  for (auto &R : pShaderOp->Resources) {
    if (!R.ReadBack)
      continue;
    ++count;
    str << "Resource: " << R.Name << "\r\n";
    // Find a descriptor that can tell us how to dump this resource.
    bool found = false;
    for (auto &Heaps : pShaderOp->DescriptorHeaps) {
      for (auto &D : Heaps.Descriptors) {
        if (_stricmp(D.ResName, R.Name) != 0) {
          continue;
        }
        found = true;
        if (_stricmp(D.Kind, "UAV") != 0) {
          str << "Resource dump for kind " << D.Kind
              << " not implemented yet.\r\n";
          break;
        }
        if (D.UavDesc.ViewDimension != D3D12_UAV_DIMENSION_BUFFER) {
          str << "Resource dump for this kind of view dimension not "
                 "implemented yet.\r\n";
          break;
        }
        // We can map back to the structure if a structured buffer via the
        // shader, but we'll keep this simple and simply dump out 32-bit
        // uint/float representations.
        MappedData data;
        pTest->GetReadBackData(R.Name, &data);
        uint32_t *pData = (uint32_t *)data.data();
        size_t u32_count = ((size_t)R.Desc.Width) / sizeof(uint32_t);
        for (size_t i = 0; i < u32_count; ++i) {
          float f = *(float *)pData;
          str << i << ": 0n" << *pData << "   0x" << std::hex << *pData
              << std::dec << "   " << f << "\r\n";
          ++pData;
        }
        break;
      }
      if (found)
        break;
    }
    if (!found) {
      str << "Unable to find a view for the resource.\r\n";
    }
  }

  str << "Resources read back: " << count << "\r\n";

  std::string s(str.str());
  CComHeapPtr<char> pDump;
  if (!pDump.Allocate(s.size() + 1))
    throw std::bad_alloc();
  memcpy(pDump.m_pData, s.data(), s.size());
  pDump.m_pData[s.size()] = '\0';
  *pReadBackDump = pDump.Detach();
}

// This is the exported interface by use from HLSLHost.exe.
// It's exclusive with the use of the DLL as a TAEF target.
extern "C" {
__declspec(dllexport) HRESULT WINAPI
    InitializeOpTests(void *pStrCtx, st::OutputStringFn pOutputStrFn) {
  HRESULT hr = ExecutionTest::EnableExperimentalShaderModels();
  if (FAILED(hr)) {
    pOutputStrFn(pStrCtx, L"Unable to enable experimental shader models.\r\n.");
  }
  return S_OK;
}

__declspec(dllexport) HRESULT WINAPI
    RunOpTest(void *pStrCtx, st::OutputStringFn pOutputStrFn, LPCSTR pText,
              ID3D12Device *pDevice, ID3D12CommandQueue *pCommandQueue,
              ID3D12Resource *pRenderTarget, char **pReadBackDump) {

  HRESULT hr;
  if (pReadBackDump)
    *pReadBackDump = nullptr;
  st::SetOutputFn(pStrCtx, pOutputStrFn);
  CComPtr<ID3D12InfoQueue> pInfoQueue;
  CComHeapPtr<char> pDump;
  bool FilterCreation = false;
  if (SUCCEEDED(pDevice->QueryInterface(&pInfoQueue))) {
    // Creation is largely driven by inputs, so don't log create/destroy
    // messages.
    pInfoQueue->PushEmptyStorageFilter();
    pInfoQueue->PushEmptyRetrievalFilter();
    if (FilterCreation) {
      D3D12_INFO_QUEUE_FILTER filter;
      D3D12_MESSAGE_CATEGORY denyCategories[] = {
          D3D12_MESSAGE_CATEGORY_STATE_CREATION};
      ZeroMemory(&filter, sizeof(filter));
      filter.DenyList.NumCategories = _countof(denyCategories);
      filter.DenyList.pCategoryList = denyCategories;
      pInfoQueue->PushStorageFilter(&filter);
    }
  } else {
    pOutputStrFn(pStrCtx, L"Unable to enable info queue for D3D.\r\n.");
  }
  try {
    dxc::DxcDllSupport m_support;
    m_support.Initialize();

    const char *pName = nullptr;
    CComPtr<IStream> pStream =
        SHCreateMemStream((const BYTE *)pText, (UINT)strlen(pText));
    std::shared_ptr<st::ShaderOpSet> ShaderOpSet =
        std::make_shared<st::ShaderOpSet>();
    st::ParseShaderOpSetFromStream(pStream, ShaderOpSet.get());
    st::ShaderOp *pShaderOp;
    if (pName == nullptr) {
      if (ShaderOpSet->ShaderOps.size() != 1) {
        pOutputStrFn(pStrCtx, L"Expected a single shader operation.\r\n");
        return E_FAIL;
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
      pOutputStrFn(pStrCtx, msgWide);
      return E_FAIL;
    }

    std::shared_ptr<st::ShaderOpTest> test =
        std::make_shared<st::ShaderOpTest>();
    test->SetupRenderTarget(pShaderOp, pDevice, pCommandQueue, pRenderTarget);
    test->SetDxcSupport(&m_support);
    test->RunShaderOp(pShaderOp);
    test->PresentRenderTarget(pShaderOp, pCommandQueue, pRenderTarget);

    pOutputStrFn(pStrCtx, L"Rendering complete.\r\n");

    if (!pShaderOp->IsCompute()) {
      D3D12_QUERY_DATA_PIPELINE_STATISTICS stats;
      test->GetPipelineStats(&stats);
      wchar_t statsText[400];
      StringCchPrintfW(
          statsText, _countof(statsText),
          L"Vertices/primitives read by input assembler: %I64u/%I64u\r\n"
          L"Vertex shader invocations: %I64u\r\n"
          L"Geometry shader invocations/output primitive: %I64u/%I64u\r\n"
          L"Primitives sent to rasterizer/rendered: %I64u/%I64u\r\n"
          L"PS/HS/DS/CS invocations: %I64u/%I64u/%I64u/%I64u\r\n",
          stats.IAVertices, stats.IAPrimitives, stats.VSInvocations,
          stats.GSInvocations, stats.GSPrimitives, stats.CInvocations,
          stats.CPrimitives, stats.PSInvocations, stats.HSInvocations,
          stats.DSInvocations, stats.CSInvocations);
      pOutputStrFn(pStrCtx, statsText);
    }

    if (pReadBackDump) {
      WriteReadBackDump(pShaderOp, test.get(), &pDump);
    }

    hr = S_OK;
  } catch (const CAtlException &E) {
    hr = E.m_hr;
  } catch (const std::bad_alloc &) {
    hr = E_OUTOFMEMORY;
  } catch (const std::exception &) {
    hr = E_FAIL;
  }

  // Drain the device message queue if available.
  if (pInfoQueue != nullptr) {
    wchar_t buf[200];
    StringCchPrintfW(
        buf, _countof(buf),
        L"NumStoredMessages=%u limit/discarded by limit=%u/%u "
        L"allowed/denied by storage filter=%u/%u "
        L"NumStoredMessagesAllowedByRetrievalFilter=%u\r\n",
        (unsigned)pInfoQueue->GetNumStoredMessages(),
        (unsigned)pInfoQueue->GetMessageCountLimit(),
        (unsigned)pInfoQueue->GetNumMessagesDiscardedByMessageCountLimit(),
        (unsigned)pInfoQueue->GetNumMessagesAllowedByStorageFilter(),
        (unsigned)pInfoQueue->GetNumMessagesDeniedByStorageFilter(),
        (unsigned)pInfoQueue->GetNumStoredMessagesAllowedByRetrievalFilter());
    pOutputStrFn(pStrCtx, buf);

    WriteInfoQueueMessages(pStrCtx, pOutputStrFn, pInfoQueue);

    pInfoQueue->ClearStoredMessages();
    pInfoQueue->PopRetrievalFilter();
    pInfoQueue->PopStorageFilter();
    if (FilterCreation) {
      pInfoQueue->PopStorageFilter();
    }
  }

  if (pReadBackDump)
    *pReadBackDump = pDump.Detach();

  return hr;
}
}
#endif
// MARKER: ExecutionTest/DxilConf Shared Implementation End
// Do not remove the line above - it is used by TranslateExecutionTest.py
