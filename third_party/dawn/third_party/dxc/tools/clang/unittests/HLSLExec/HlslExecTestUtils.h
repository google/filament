#ifndef HLSLEXECTESTUTILS_H
#define HLSLEXECTESTUTILS_H

#include "dxc/Support/dxcapi.use.h"
#include "dxc/Test/HlslTestUtils.h"
#include <Verify.h>
#include <d3d12.h>
#include <dxgi1_4.h>

namespace ExecTestUtils {
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
  D3D_HIGHEST_SHADER_MODEL = D3D_SHADER_MODEL_6_9
} D3D_SHADER_MODEL;
} // namespace ExecTestUtils

static bool useDebugIfaces() { return true; }

static bool useDxbc() {
#ifdef _HLK_CONF
  return false;
#else
  return hlsl_test::GetTestParamBool(L"DXBC");
#endif
}

static bool useWarpByDefualt() {
#ifdef _HLK_CONF
  return false;
#else
  return true;
#endif
}

// A more recent Windows SDK than currently required is needed for these.
typedef HRESULT(WINAPI *D3D12EnableExperimentalFeaturesFn)(
    UINT NumFeatures, __in_ecount(NumFeatures) const IID *IIDs,
    __in_ecount_opt(NumFeatures) void *ConfigurationStructs,
    __in_ecount_opt(NumFeatures) UINT *ConfigurationStructSizes);

static const GUID D3D12ExperimentalShaderModelsID =
    {/* 76f5573e-f13a-40f5-b297-81ce9e18933f */
     0x76f5573e,
     0xf13a,
     0x40f5,
     {0xb2, 0x97, 0x81, 0xce, 0x9e, 0x18, 0x93, 0x3f}};

// Used to create D3D12SDKConfiguration to enable AgilitySDK programmatically.
typedef HRESULT(WINAPI *D3D12GetInterfaceFn)(REFCLSID Rclsid, REFIID Riid,
                                             void **Debug);

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

static std::wstring getModuleName() {
  wchar_t ModuleName[MAX_PATH + 1] = {0};
  const DWORD Length = GetModuleFileNameW(NULL, ModuleName, MAX_PATH);

  if (Length == 0 || Length == MAX_PATH)
    return std::wstring(); // Error condition

  return std::wstring(ModuleName, Length);
}

static std::wstring computeSDKFullPath(std::wstring SDKPath) {
  std::wstring ModulePath = getModuleName();
  const size_t Pos = ModulePath.rfind('\\');

  if (Pos == std::wstring::npos)
    return SDKPath;

  if (SDKPath.substr(0, 2) != L".\\")
    return SDKPath;

  return ModulePath.substr(0, Pos) + SDKPath.substr(1);
}

static UINT getD3D12SDKVersion(std::wstring SDKPath) {
  // Try to automatically get the D3D12SDKVersion from the DLL
  UINT SDKVersion = 0;
  std::wstring D3DCorePath = computeSDKFullPath(SDKPath);
  D3DCorePath.append(L"D3D12Core.dll");
  HMODULE D3DCore = LoadLibraryW(D3DCorePath.c_str());
  if (D3DCore) {
    if (UINT *SDKVersionOut =
            (UINT *)GetProcAddress(D3DCore, "D3D12SDKVersion"))
      SDKVersion = *SDKVersionOut;
    FreeModule(D3DCore);
  }
  return SDKVersion;
}

static bool createDevice(ID3D12Device **D3DDevice,
                         ExecTestUtils::D3D_SHADER_MODEL TestModel =
                             ExecTestUtils::D3D_SHADER_MODEL_6_0,
                         bool SkipUnsupported = true) {
  if (TestModel > ExecTestUtils::D3D_HIGHEST_SHADER_MODEL) {
    const UINT Minor = (UINT)TestModel & 0x0f;
    hlsl_test::LogCommentFmt(L"Installed SDK does not support "
                             L"shader model 6.%1u",
                             Minor);

    if (SkipUnsupported)
      WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);

    return false;
  }
  CComPtr<IDXGIFactory4> DXGIFactory;
  CComPtr<ID3D12Device> D3DDeviceCom;

  *D3DDevice = nullptr;

  VERIFY_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&DXGIFactory)));
  if (hlsl_test::GetTestParamUseWARP(useWarpByDefualt())) {
    CComPtr<IDXGIAdapter> WarpAdapter;
    VERIFY_SUCCEEDED(DXGIFactory->EnumWarpAdapter(IID_PPV_ARGS(&WarpAdapter)));
    HRESULT CreateHR = D3D12CreateDevice(WarpAdapter, D3D_FEATURE_LEVEL_11_0,
                                         IID_PPV_ARGS(&D3DDeviceCom));
    if (FAILED(CreateHR)) {
      hlsl_test::LogCommentFmt(
          L"The available version of WARP does not support d3d12.");

      if (SkipUnsupported)
        WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);

      return false;
    }

    if (GetModuleHandleW(L"d3d10warp.dll") != NULL) {
      WCHAR FullModuleFilePath[MAX_PATH] = L"";
      GetModuleFileNameW(GetModuleHandleW(L"d3d10warp.dll"), FullModuleFilePath,
                         sizeof(FullModuleFilePath));
      WEX::Logging::Log::Comment(WEX::Common::String().Format(
          L"WARP driver loaded from: %ls", FullModuleFilePath));
    }

  } else {
    CComPtr<IDXGIAdapter1> HardwareAdapter;
    WEX::Common::String AdapterValue;
    HRESULT HR = WEX::TestExecution::RuntimeParameters::TryGetValue(
        L"Adapter", AdapterValue);
    if (SUCCEEDED(HR))
      st::GetHardwareAdapter(DXGIFactory, AdapterValue, &HardwareAdapter);
    else
      WEX::Logging::Log::Comment(
          L"Using default hardware adapter with D3D12 support.");

    VERIFY_SUCCEEDED(D3D12CreateDevice(HardwareAdapter, D3D_FEATURE_LEVEL_11_0,
                                       IID_PPV_ARGS(&D3DDeviceCom)));
  }
  // retrieve adapter information
  const LUID AdapterID = D3DDeviceCom->GetAdapterLuid();
  CComPtr<IDXGIAdapter> DXGIAdapter;
  DXGIFactory->EnumAdapterByLuid(AdapterID, IID_PPV_ARGS(&DXGIAdapter));
  DXGI_ADAPTER_DESC AdapterDesc;
  VERIFY_SUCCEEDED(DXGIAdapter->GetDesc(&AdapterDesc));
  hlsl_test::LogCommentFmt(L"Using Adapter:%s", AdapterDesc.Description);

  if (D3DDeviceCom == nullptr)
    return false;

  if (!useDxbc()) {
    // Check for DXIL support.
    typedef struct D3D12_FEATURE_DATA_SHADER_MODEL {
      ExecTestUtils::D3D_SHADER_MODEL HighestShaderModel;
    } D3D12_FEATURE_DATA_SHADER_MODEL;
    const UINT D3D12_FEATURE_SHADER_MODEL = 7;
    D3D12_FEATURE_DATA_SHADER_MODEL SMData;
    SMData.HighestShaderModel = TestModel;
    if (FAILED(D3DDeviceCom->CheckFeatureSupport(
            (D3D12_FEATURE)D3D12_FEATURE_SHADER_MODEL, &SMData,
            sizeof(SMData))) ||
        SMData.HighestShaderModel < TestModel) {
      const UINT Minor = (UINT)TestModel & 0x0f;
      hlsl_test::LogCommentFmt(L"The selected device does not support "
                               L"shader model 6.%1u",
                               Minor);

      if (SkipUnsupported)
        WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);

      return false;
    }
  }

  if (useDebugIfaces()) {
    CComPtr<ID3D12InfoQueue> InfoQueue;
    if (SUCCEEDED(D3DDeviceCom->QueryInterface(&InfoQueue)))
      InfoQueue->SetMuteDebugOutput(FALSE);
  }

  *D3DDevice = D3DDeviceCom.Detach();
  return true;
}

inline void readHlslDataIntoNewStream(LPCWSTR RelativePath, IStream **Stream,
                                      dxc::DxcDllSupport &Support) {
  VERIFY_SUCCEEDED(Support.Initialize());
  CComPtr<IDxcLibrary> Library;
  CComPtr<IDxcBlobEncoding> Blob;
  CComPtr<IStream> StreamCom;
  std::wstring Path = hlsl_test::GetPathToHlslDataFile(
      RelativePath, HLSLDATAFILEPARAM, DEFAULT_EXEC_TEST_DIR);
  VERIFY_SUCCEEDED(Support.CreateInstance(CLSID_DxcLibrary, &Library));
  VERIFY_SUCCEEDED(Library->CreateBlobFromFile(Path.c_str(), nullptr, &Blob));
  VERIFY_SUCCEEDED(Library->CreateStreamFromBlobReadOnly(Blob, &StreamCom));
  *Stream = StreamCom.Detach();
}

static HRESULT enableAgilitySDK(HMODULE Runtime, UINT SDKVersion,
                                LPCWSTR SDKPath) {
  D3D12GetInterfaceFn GetInterfaceFunc =
      (D3D12GetInterfaceFn)GetProcAddress(Runtime, "D3D12GetInterface");
  CComPtr<ID3D12SDKConfiguration> D3D12SDKConfiguration;
  IFR(GetInterfaceFunc(CLSID_D3D12SDKConfiguration,
                       IID_PPV_ARGS(&D3D12SDKConfiguration)));
  IFR(D3D12SDKConfiguration->SetSDKVersion(SDKVersion, CW2A(SDKPath)));

  // Currently, it appears that the SetSDKVersion will succeed even when
  // D3D12Core is not found, or its version doesn't match.  When that's the
  // case, will cause a failure in the very next thing that actually requires
  // D3D12Core.dll to be loaded instead.  So, we attempt to clear experimental
  // features next, which is a valid use case and a no-op at this point.  This
  // requires D3D12Core to be loaded.  If this fails, we know the AgilitySDK
  // setting actually failed.
  D3D12EnableExperimentalFeaturesFn ExperimentalFeaturesFunc =
      (D3D12EnableExperimentalFeaturesFn)GetProcAddress(
          Runtime, "D3D12EnableExperimentalFeatures");
  if (ExperimentalFeaturesFunc == nullptr)
    // If this failed, D3D12 must be too old for AgilitySDK.  But if that's
    // the case, creating D3D12SDKConfiguration should have failed.  So while
    // this case shouldn't be hit, fail if it is.
    return HRESULT_FROM_WIN32(GetLastError());

  return ExperimentalFeaturesFunc(0, nullptr, nullptr, nullptr);
}

static HRESULT
enableExperimentalShaderModels(HMODULE hRuntime,
                               UUID AdditionalFeatures[] = nullptr,
                               size_t NumAdditionalFeatures = 0) {
  D3D12EnableExperimentalFeaturesFn ExperimentalFeaturesFunc =
      (D3D12EnableExperimentalFeaturesFn)GetProcAddress(
          hRuntime, "D3D12EnableExperimentalFeatures");
  if (ExperimentalFeaturesFunc == nullptr)
    return HRESULT_FROM_WIN32(GetLastError());

  std::vector<UUID> Features;

  Features.push_back(D3D12ExperimentalShaderModels);

  if (AdditionalFeatures != nullptr && NumAdditionalFeatures > 0)
    Features.insert(Features.end(), AdditionalFeatures,
                    AdditionalFeatures + NumAdditionalFeatures);

  return ExperimentalFeaturesFunc((UINT)Features.size(), Features.data(),
                                  nullptr, nullptr);
}

static HRESULT
enableExperimentalShaderModels(UUID AdditionalFeatures[] = nullptr,
                               size_t NumAdditionalFeatures = 0) {
  HMODULE Runtime = LoadLibraryW(L"d3d12.dll");
  if (Runtime == NULL)
    return E_FAIL;
  return enableExperimentalShaderModels(Runtime, AdditionalFeatures,
                                        NumAdditionalFeatures);
}

static HRESULT disableExperimentalShaderModels() {
  HMODULE Runtime = LoadLibraryW(L"d3d12.dll");
  if (Runtime == NULL)
    return E_FAIL;

  D3D12EnableExperimentalFeaturesFn ExperimentalFeaturesFunc =
      (D3D12EnableExperimentalFeaturesFn)GetProcAddress(
          Runtime, "D3D12EnableExperimentalFeatures");
  if (ExperimentalFeaturesFunc == nullptr)
    return HRESULT_FROM_WIN32(GetLastError());

  return ExperimentalFeaturesFunc(0, nullptr, nullptr, nullptr);
}

static HRESULT enableAgilitySDK(HMODULE Runtime) {
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
    if (!SDKPath.IsEmpty() && SDKPath.Right(1) != "\\")
      SDKPath.Append("\\");
  }

  if (SDKPath.IsEmpty())
    SDKPath = L".\\D3D12\\";

  const bool MustFind = SDKVersion > 0;
  if (SDKVersion <= 1) {
    // lookup version from D3D12Core.dll
    SDKVersion = getD3D12SDKVersion((LPCWSTR)SDKPath);
    if (MustFind && SDKVersion == 0) {
      hlsl_test::LogErrorFmt(L"Agility SDK not found in relative path: %s",
                             (LPCWSTR)SDKPath);
      return E_FAIL;
    }
  }

  // Not found, not asked for.
  if (SDKVersion == 0)
    return S_FALSE;

  HRESULT HR = enableAgilitySDK(Runtime, SDKVersion, (LPCWSTR)SDKPath);
  if (FAILED(HR)) {
    // If SDKVersion provided, fail if not successful.
    // 1 means we should find it, and fill in the version automatically.
    if (MustFind) {
      hlsl_test::LogErrorFmt(
          L"Failed to set Agility SDK version %d at path: %s", SDKVersion,
          (LPCWSTR)SDKPath);
      return HR;
    }
    return S_FALSE;
  }
  if (HR == S_OK)
    hlsl_test::LogCommentFmt(L"Agility SDK version set to: %d", SDKVersion);

  return HR;
}

static HRESULT enableExperimentalMode(HMODULE Runtime) {
#ifdef _FORCE_EXPERIMENTAL_SHADERS
  bool ExperimentalShaderModels = true;
#else
  bool ExperimentalShaderModels =
      hlsl_test::GetTestParamBool(L"ExperimentalShaders");
#endif // _FORCE_EXPERIMENTAL_SHADERS

  HRESULT HR = S_FALSE;
  if (ExperimentalShaderModels) {
    HR = enableExperimentalShaderModels(Runtime);
    if (SUCCEEDED(HR))
      WEX::Logging::Log::Comment(L"Experimental shader models enabled.");
  }

  return HR;
}

static HRESULT enableDebugLayer() {
  // The debug layer does net yet validate DXIL programs that require
  // rewriting, but basic logging should work properly.
  HRESULT HR = S_FALSE;
  if (useDebugIfaces()) {
    CComPtr<ID3D12Debug> DebugController;
    HR = D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController));
    if (SUCCEEDED(HR)) {
      DebugController->EnableDebugLayer();
      HR = S_OK;
    }
  }
  return HR;
}

#endif // HLSLEXECTESTUTILS_H
