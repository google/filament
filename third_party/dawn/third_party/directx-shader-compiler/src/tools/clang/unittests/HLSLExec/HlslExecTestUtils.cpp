#include "HlslExecTestUtils.h"

#include "ShaderOpTest.h"
#include "dxc/Support/dxcapi.use.h"

#include "HlslTestUtils.h"

#include <Verify.h>
#include <atlcomcli.h>
#include <cstddef>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <filesystem>
#include <mutex>
#include <optional>

// D3D12_FEATURE_D3D12_OPTIONS_PREVIEW and its data struct are not yet in
// the released Windows SDK. Define locally so the test can query variable
// group shared memory capabilities from the Agility SDK runtime.
// TODO(#8661): Remove me when GroupSharedLimit is available in a released
// Windows SDK.
#if defined(D3D12_PREVIEW_SDK_VERSION) && D3D12_PREVIEW_SDK_VERSION < 720

#ifndef D3D12_FEATURE_D3D12_OPTIONS_PREVIEW
#define D3D12_FEATURE_D3D12_OPTIONS_PREVIEW ((D3D12_FEATURE)72)
#endif

typedef struct D3D12_FEATURE_DATA_D3D12_OPTIONS_PREVIEW {
  UINT MaxGroupSharedMemoryPerGroupCS;
  UINT MaxGroupSharedMemoryPerGroupAS;
  UINT MaxGroupSharedMemoryPerGroupMS;
} D3D12_FEATURE_DATA_D3D12_OPTIONS_PREVIEW;

#endif

constexpr UINT KnownMultiplicationFlags = 0xf;
constexpr UINT MatrixMultiplicationFlags =
    static_cast<UINT>(
        linalg_abi::
            D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED) |
    static_cast<UINT>(
        linalg_abi::
            D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_EMULATED_OUTPUTS);

static bool isValidMultiplicationFlags(UINT Flags, UINT AllowedFlags) {
  if ((Flags & ~AllowedFlags) != 0)
    return false;
  return Flags == 0 ||
         (Flags &
          static_cast<UINT>(
              linalg_abi::
                  D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED)) !=
             0;
}

static bool isFloat8DataType(linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE Type) {
  return Type == linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT8_E4M3FN ||
         Type == linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT8_E5M2;
}

static bool isValidThreadVectorMultiplicationFlags(
    UINT Flags, linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE MatrixInputType) {
  if (!isValidMultiplicationFlags(Flags, KnownMultiplicationFlags))
    return false;
  const UINT EmulatedInputs = static_cast<UINT>(
      linalg_abi::
          D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_EMULATED_INPUTS);
  return (Flags & EmulatedInputs) == 0 || isFloat8DataType(MatrixInputType);
}

static LPCWSTR dataTypeName(linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE Type) {
  switch (Type) {
  case linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_NONE:
    return L"None";
  case linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_SINT16:
    return L"SInt16";
  case linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_UINT16:
    return L"UInt16";
  case linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_SINT32:
    return L"SInt32";
  case linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_UINT32:
    return L"UInt32";
  case linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16:
    return L"Float16";
  case linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32:
    return L"Float32";
  case linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_SINT8:
    return L"SInt8";
  case linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_UINT8:
    return L"UInt8";
  case linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT8_E4M3FN:
    return L"Float8E4M3FN";
  case linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT8_E5M2:
    return L"Float8E5M2";
  }
  return L"Unknown";
}

static void setWaveInputs(
    linalg_abi::D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_INPUTS &RuntimeInputs,
    const linalg_test::WaveMatrixMultiplyInputs &Inputs) {
  RuntimeInputs.WaveSize = Inputs.WaveSize;
  RuntimeInputs.MatrixAComponentType =
      static_cast<UINT>(Inputs.MatrixAComponentType);
  RuntimeInputs.MatrixBComponentType =
      static_cast<UINT>(Inputs.MatrixBComponentType);
  RuntimeInputs.AccumulatorComponentType =
      static_cast<UINT>(Inputs.AccumulatorComponentType);
}

using namespace hlsl_test;

// Disabled by default for HLK runs.
static bool debugLayerOnByDefault() {
#ifdef _HLK_CONF
  return false;
#else
  return true;
#endif
}

// /p:D3D12DebugLayer=true or /p:D3D12DebugLayer=false overrides the default.
static bool useDebugIfaces() {
  static const bool Enabled = [] {
    bool Value = debugLayerOnByDefault();
    WEX::TestExecution::RuntimeParameters::TryGetValue(L"D3D12DebugLayer",
                                                       Value);
    return Value;
  }();
  return Enabled;
}

// Set when the debug layer has been enabled.
static bool DebugLayerEnabled = false;

bool useDxbc() {
#ifdef _HLK_CONF
  return false;
#else
  return GetTestParamBool(L"DXBC");
#endif
}

static bool useWarpByDefault() {
#ifdef _HLK_CONF
  return false;
#else
  return true;
#endif
}

static std::wstring getModuleName() {
  wchar_t ModuleName[MAX_PATH + 1] = {0};
  const DWORD Length = GetModuleFileNameW(NULL, ModuleName, MAX_PATH);

  if (Length == 0 || Length == MAX_PATH)
    return std::wstring(); // Error condition

  return std::wstring(ModuleName, Length);
}

static std::wstring computeSDKFullPath(const std::wstring &SDKPath) {
  if (std::filesystem::path(SDKPath).is_absolute())
    return SDKPath;

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
    LogCommentFmt(L"%s - D3D12SDKVersion is %d", D3DCorePath.c_str(),
                  SDKVersion);
  } else {
    LogCommentFmt(L"%s - unable to load", D3DCorePath.c_str());
  }
  return SDKVersion;
}

// Severities are ordered most-to-least severe (CORRUPTION == 0), so a message
// is reported when its severity is <= this threshold. -1 reports nothing.
static int DebugMessageSeverityThreshold = D3D12_MESSAGE_SEVERITY_ERROR;

// /p:D3D12DebugMessageSeverity accepts "none" or a D3D12_MESSAGE_SEVERITY name:
// "corruption", "error", "warning", "info" or "message".
static int getDebugMessageSeverityThreshold() {
  WEX::Common::String Value;
  if (FAILED(WEX::TestExecution::RuntimeParameters::TryGetValue(
          L"D3D12DebugMessageSeverity", Value)))
    return D3D12_MESSAGE_SEVERITY_ERROR;

  const wchar_t *Name = Value;
  if (_wcsicmp(Name, L"none") == 0)
    return -1;
  if (_wcsicmp(Name, L"corruption") == 0)
    return D3D12_MESSAGE_SEVERITY_CORRUPTION;
  if (_wcsicmp(Name, L"error") == 0)
    return D3D12_MESSAGE_SEVERITY_ERROR;
  if (_wcsicmp(Name, L"warning") == 0)
    return D3D12_MESSAGE_SEVERITY_WARNING;
  if (_wcsicmp(Name, L"info") == 0)
    return D3D12_MESSAGE_SEVERITY_INFO;
  if (_wcsicmp(Name, L"message") == 0)
    return D3D12_MESSAGE_SEVERITY_MESSAGE;

  LogWarningFmt(L"Unrecognized D3D12DebugMessageSeverity '%s'. Using 'error'.",
                Name);
  return D3D12_MESSAGE_SEVERITY_ERROR;
}

// Routes debug layer messages into the test log; they are otherwise emitted via
// OutputDebugString and only visible under a debugger. Logged as comments, so
// they never alter a test outcome.
static void __stdcall logD3D12DebugMessage(D3D12_MESSAGE_CATEGORY Category,
                                           D3D12_MESSAGE_SEVERITY Severity,
                                           D3D12_MESSAGE_ID ID,
                                           LPCSTR Description, void *Context) {
  (void)Category;
  (void)Context;

  if (static_cast<int>(Severity) > DebugMessageSeverityThreshold)
    return;

  const wchar_t *SeverityName;
  switch (Severity) {
  case D3D12_MESSAGE_SEVERITY_CORRUPTION:
    SeverityName = L"CORRUPTION";
    break;
  case D3D12_MESSAGE_SEVERITY_ERROR:
    SeverityName = L"ERROR";
    break;
  case D3D12_MESSAGE_SEVERITY_WARNING:
    SeverityName = L"WARNING";
    break;
  case D3D12_MESSAGE_SEVERITY_INFO:
    SeverityName = L"INFO";
    break;
  default:
    SeverityName = L"MESSAGE";
    break;
  }

  LogCommentFmt(L"D3D12 %s [id %u]: %s", SeverityName, static_cast<UINT>(ID),
                static_cast<const wchar_t *>(CA2W(Description)));
}

// Routes debug layer messages for this device into the test log.
static void logDebugLayerMessages(ID3D12Device *Device) {
  CComPtr<ID3D12InfoQueue> InfoQueue;
  if (FAILED(Device->QueryInterface(&InfoQueue))) {
    if (DebugLayerEnabled)
      LogWarningFmt(L"The debug layer was enabled but this device does not "
                    L"expose ID3D12InfoQueue. D3D12 debug layer messages will "
                    L"not be reported.");
    return;
  }

  InfoQueue->SetMuteDebugOutput(FALSE);

  CComPtr<ID3D12InfoQueue1> InfoQueue1;
  if (FAILED(InfoQueue->QueryInterface(&InfoQueue1))) {
    LogWarningFmt(L"Device does not expose ID3D12InfoQueue1. D3D12 debug "
                  L"layer messages will not be reported.");
    return;
  }

  DebugMessageSeverityThreshold = getDebugMessageSeverityThreshold();

  // The callback is unregistered implicitly when the device is destroyed.
  DWORD CallbackCookie = 0;
  HRESULT HR = InfoQueue1->RegisterMessageCallback(
      logD3D12DebugMessage, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr,
      &CallbackCookie);
  if (FAILED(HR))
    LogWarningFmt(L"RegisterMessageCallback failed: 0x%08x. D3D12 debug "
                  L"layer messages will not be reported.",
                  HR);
}

static bool createDevice(
    ID3D12Device **D3DDevice, D3D_SHADER_MODEL TestModel, bool SkipUnsupported,
    std::function<HRESULT(IUnknown *, D3D_FEATURE_LEVEL, REFIID, void **)>
        CreateDeviceFn

) {
  if (*D3DDevice)
    LogErrorFmt(L"createDevice called with non-null *D3DDevice - "
                L"this will likely leak the previous device");
  if (TestModel > DXC_HIGHEST_SHADER_MODEL) {
    const UINT Minor = (UINT)TestModel & 0x0f;
    LogCommentFmt(L"Installed SDK does not support "
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
  if (GetTestParamUseWARP(useWarpByDefault())) {
    // The WARP_DLL runtime parameter can be used to specify a specific DLL to
    // load.  To force this to be used, we make sure that this DLL is loaded
    // before attempting to create the device.

    struct WarpDll {
      HMODULE Module = NULL; // NOLINT

      ~WarpDll() { Close(); }

      void Close() {
        if (Module) {
          FreeLibrary(Module);
          Module = NULL;
        }
      }
    };

    WarpDll ExplicitlyLoadedWarpDll;
    WEX::Common::String WarpDllPath;
    if (SUCCEEDED(WEX::TestExecution::RuntimeParameters::TryGetValue(
            L"WARP_DLL", WarpDllPath))) {
      WEX::Logging::Log::Comment(WEX::Common::String().Format(
          L"WARP_DLL requested: %ls", (const wchar_t *)WarpDllPath));
      ExplicitlyLoadedWarpDll.Module = LoadLibraryExW(WarpDllPath, NULL, 0);
      VERIFY_WIN32_BOOL_SUCCEEDED(!!ExplicitlyLoadedWarpDll.Module);
    }

    // Create the WARP device
    CComPtr<IDXGIAdapter> WarpAdapter;
    VERIFY_SUCCEEDED(DXGIFactory->EnumWarpAdapter(IID_PPV_ARGS(&WarpAdapter)));
    HRESULT CreateHR = CreateDeviceFn(WarpAdapter, D3D_FEATURE_LEVEL_11_0,
                                      IID_PPV_ARGS(&D3DDeviceCom));
    if (FAILED(CreateHR)) {
      LogCommentFmt(L"Failed to create WARP device: 0x%08x", CreateHR);

      if (SkipUnsupported)
        WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);

      return false;
    }

    // Now that the WARP device is created we can release our reference to the
    // warp dll.
    ExplicitlyLoadedWarpDll.Close();

    // Log the actual version of WARP that's loaded so we can be sure that
    // we're using the version we think.
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

    VERIFY_SUCCEEDED(CreateDeviceFn(HardwareAdapter, D3D_FEATURE_LEVEL_11_0,
                                    IID_PPV_ARGS(&D3DDeviceCom)));
  }
  // retrieve adapter information
  const LUID AdapterID = D3DDeviceCom->GetAdapterLuid();
  CComPtr<IDXGIAdapter> DXGIAdapter;
  DXGIFactory->EnumAdapterByLuid(AdapterID, IID_PPV_ARGS(&DXGIAdapter));
  DXGI_ADAPTER_DESC AdapterDesc;
  VERIFY_SUCCEEDED(DXGIAdapter->GetDesc(&AdapterDesc));
  LogCommentFmt(L"Using Adapter:%s", AdapterDesc.Description);

  if (D3DDeviceCom == nullptr)
    return false;

  if (!useDxbc()) {
    D3D12_FEATURE_DATA_SHADER_MODEL SMData;
    SMData.HighestShaderModel = TestModel;
    if (FAILED(D3DDeviceCom->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL,
                                                 &SMData, sizeof(SMData))) ||
        SMData.HighestShaderModel < TestModel) {
      const UINT Minor = (UINT)TestModel & 0x0f;
      LogCommentFmt(L"The selected device does not support "
                    L"shader model 6.%1u (highest is 6.%1u)",
                    Minor, SMData.HighestShaderModel & 0x0f);

      if (SkipUnsupported)
        WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);

      return false;
    }
  }

  if (useDebugIfaces())
    logDebugLayerMessages(D3DDeviceCom);

  *D3DDevice = D3DDeviceCom.Detach();
  return true;
}

// Linker-provided symbol at the base of the module containing this code, which
// is also its HMODULE. Identifies the module without depending on the name of
// the binary these sources are built into.
// See https://devblogs.microsoft.com/oldnewthing/20041025-00/?p=37483.
extern "C" IMAGE_DOS_HEADER __ImageBase;

// Directory of the module containing this code.
static const std::wstring &getContainingModuleDirectory() {
  static const std::wstring Dir = [] {
    HMODULE Module = reinterpret_cast<HMODULE>(&__ImageBase);

    // GetModuleFileNameW truncates rather than failing, so grow until it fits.
    std::wstring Path(MAX_PATH, L'\0');
    for (;;) {
      const DWORD Len =
          GetModuleFileNameW(Module, &Path[0], static_cast<DWORD>(Path.size()));
      if (Len == 0)
        return std::wstring();
      if (Len < Path.size()) {
        Path.resize(Len);
        break;
      }
      Path.resize(Path.size() * 2);
    }
    return std::filesystem::path(Path).parent_path().wstring();
  }();
  return Dir;
}

// Every call site loads from the same directory, so log the first resolution
// only. Tests may run in parallel, so this has to be thread-safe.
static void logResolvedDataDirOnce(const wchar_t *Origin,
                                   const std::filesystem::path &Path) {
  static std::once_flag Logged;
  std::call_once(Logged, [&] {
    LogCommentFmt(L"Loading execution test data from %s: %s", Origin,
                  Path.parent_path().c_str());
  });
}

// Resolves a data file shipped with the execution tests, in this order:
//
//   1. the HlslDataDir runtime parameter, if supplied. Authoritative: we fail
//      rather than fall through, so a typo is reported instead of hidden.
//   2. the directory containing this module, so a binary deployed with its
//      data files finds them.
//   3. the source directory recorded by CMake, so a build tree picks up edits
//      to the data files without rebuilding.
static std::wstring resolveHlslDataFile(LPCWSTR RelativePath) {
  WEX::Common::String ParamValue;
  std::wstring OverrideDir;
  if (SUCCEEDED(WEX::TestExecution::RuntimeParameters::TryGetValue(
          HLSLDATAFILEPARAM, ParamValue)))
    OverrideDir = reinterpret_cast<const wchar_t *>(ParamValue.GetBuffer());

  // Treat an empty value as absent and fall through to the other locations.
  if (!OverrideDir.empty()) {
    std::filesystem::path Candidate =
        std::filesystem::path(OverrideDir) / RelativePath;
    if (!std::filesystem::exists(Candidate))
      LOG_ERROR_FMT_THROW(
          L"Unable to find %s in the directory given by the %s parameter.\n"
          L"  Tried: %s\n"
          L"%s is authoritative, so no other location was searched. Omit it "
          L"to search the test binary's directory and the source directory.",
          RelativePath, HLSLDATAFILEPARAM, Candidate.c_str(),
          HLSLDATAFILEPARAM);
    logResolvedDataDirOnce(
        FormatToWString(L"the %s parameter", HLSLDATAFILEPARAM).c_str(),
        Candidate);
    return Candidate.wstring();
  }

  const std::wstring ModuleDir = getContainingModuleDirectory();
  std::filesystem::path ModuleCandidate;
  if (!ModuleDir.empty()) {
    ModuleCandidate = std::filesystem::path(ModuleDir) / RelativePath;
    if (std::filesystem::exists(ModuleCandidate)) {
      logResolvedDataDirOnce(L"the directory containing the test binary",
                             ModuleCandidate);
      return ModuleCandidate.wstring();
    }
  }

  // Empty in _HLK_CONF builds; probing it would just look in the current
  // directory.
  const std::wstring SourceDir = DEFAULT_EXEC_TEST_DIR;
  std::filesystem::path SourceCandidate;
  if (!SourceDir.empty()) {
    SourceCandidate = std::filesystem::path(SourceDir) / RelativePath;
    if (std::filesystem::exists(SourceCandidate)) {
      logResolvedDataDirOnce(L"the build-configured source directory",
                             SourceCandidate);
      return SourceCandidate.wstring();
    }
  }

  LOG_ERROR_FMT_THROW(
      L"Unable to find the test data file %s in any known location.\n"
      L"  Next to the test binary:      %s\n"
      L"  Build-configured source path: %s\n"
      L"Deploy %s next to the test binary, or pass "
      L"/p:\"%s=<directory containing %s>\" to specify its location.",
      RelativePath,
      ModuleCandidate.empty() ? L"<could not determine the binary's directory>"
                              : ModuleCandidate.c_str(),
      SourceCandidate.empty() ? L"<not configured for this build>"
                              : SourceCandidate.c_str(),
      RelativePath, HLSLDATAFILEPARAM, RelativePath);
  return std::wstring();
}

void readHlslDataIntoNewStream(LPCWSTR RelativePath, IStream **Stream,
                               dxc::SpecificDllLoader &Support) {
  VERIFY_SUCCEEDED(
      Support.InitializeForDll(dxc::kDxCompilerLib, "DxcCreateInstance"));
  CComPtr<IDxcLibrary> Library;
  CComPtr<IDxcBlobEncoding> Blob;
  CComPtr<IStream> StreamCom;
  std::wstring Path = resolveHlslDataFile(RelativePath);
  VERIFY_SUCCEEDED(Support.CreateInstance(CLSID_DxcLibrary, &Library));
  VERIFY_SUCCEEDED(Library->CreateBlobFromFile(Path.c_str(), nullptr, &Blob));
  VERIFY_SUCCEEDED(Library->CreateStreamFromBlobReadOnly(Blob, &StreamCom));
  *Stream = StreamCom.Detach();
}

// Enables the debug layer on the process-global D3D12Core, so it must be called
// after the global Agility SDK version has been selected. Devices created
// through an ID3D12DeviceFactory need enableDebugLayerOnFactory() instead.
static bool enableGlobalDebugLayer() {
  CComPtr<ID3D12Debug> DebugController;
  HRESULT HR;
  if (FAILED(HR = D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController)))) {
    LogWarningFmt(L"Failed to get ID3D12Debug: 0x%08x", HR);
    return false;
  }

  DebugController->EnableDebugLayer();
  DebugLayerEnabled = true;
  return true;
}

struct AgilitySDKConfiguration {
  WEX::Common::String SDKPath;
  UINT SDKVersion = 0;
  bool MustFind = false;
};

static std::optional<AgilitySDKConfiguration> getAgilitySDKConfiguration() {
  using WEX::TestExecution::RuntimeParameters;

  AgilitySDKConfiguration C;

  // For global configuration, D3D12SDKPath must be a relative path from the
  // .exe, meaning it should be relative to the TE.exe location and must start
  // with ".\", such as with the default: ".\D3D12\".
  //
  // For ID3D12DeviceFactory-style configuration, D3D12SDKPath can be an
  // absolute path.
  if (SUCCEEDED(RuntimeParameters::TryGetValue(L"D3D12SDKPath", C.SDKPath))) {
    // Make sure path ends in backslash
    if (!C.SDKPath.IsEmpty() && C.SDKPath.Right(1) != "\\")
      C.SDKPath.Append("\\");
  }

  if (C.SDKPath.IsEmpty())
    C.SDKPath = L".\\D3D12\\";

  // D3D12SDKVersion > 1 will use provided version, otherwise, auto-detect.
  // D3D12SDKVersion == 1 means fail if we can't auto-detect.
  RuntimeParameters::TryGetValue(L"D3D12SDKVersion", C.SDKVersion);

  C.MustFind = C.SDKVersion >= 1;

  if (C.SDKVersion <= 1) {
    // Use the version supported by the SDK in the path.
    C.SDKVersion = getD3D12SDKVersion(std::wstring(C.SDKPath));
    if (C.SDKVersion == 0) {
      if (C.MustFind) {
        LogErrorFmt(L"Agility SDK not found in path: %s",
                    static_cast<const wchar_t *>(C.SDKPath));
        return std::nullopt;
      }

      // No AgilitySDK found, caller indicated that they just want to use the
      // inbox D3D12 in this case.
      return AgilitySDKConfiguration{};
    }
  }

  return C;
}

static bool
enableGlobalAgilitySDK(const std::optional<AgilitySDKConfiguration> &C) {
  if (!C)
    return false;

  if (C->SDKVersion == 0)
    return false;

  CComPtr<ID3D12SDKConfiguration> SDKConfig;
  HRESULT HR;
  if (FAILED(HR = D3D12GetInterface(CLSID_D3D12SDKConfiguration,
                                    IID_PPV_ARGS(&SDKConfig)))) {
    LogWarningFmt(L"Failed to get ID3D12SDKConfiguration instance: 0x%08x", HR);
    return !C->MustFind;
  }

  if (FAILED(HR = SDKConfig->SetSDKVersion(C->SDKVersion, CW2A(C->SDKPath)))) {
    LogWarningFmt(L"SetSDKVersion(%d, %s) failed: 0x%08x", C->SDKVersion,
                  static_cast<const wchar_t *>(C->SDKPath), HR);
    return !C->MustFind;
  }

  // Currently, it appears that the SetSDKVersion will succeed even when
  // D3D12Core is not found, or its version doesn't match.  When that's the
  // case, will cause a failure in the very next thing that actually requires
  // D3D12Core.dll to be loaded instead.  So, we attempt to clear experimental
  // features next, which is a valid use case and a no-op at this point.  This
  // requires D3D12Core to be loaded.  If this fails, we know the AgilitySDK
  // setting actually failed.
  if (FAILED(
          HR = D3D12EnableExperimentalFeatures(0, nullptr, nullptr, nullptr))) {
    LogWarningFmt(L"D3D12EnableExperimentalFeatures(0...) failed: 0x%08x", HR);
    return !C->MustFind;
  }

  return true;
}

static bool isExperimentalShadersEnabled() {
  return GetTestParamBool(L"ExperimentalShaders");
}

static bool enableGlobalExperimentalMode() {
  if (!isExperimentalShadersEnabled())
    return false;

  HRESULT HR;
  if (FAILED(HR = D3D12EnableExperimentalFeatures(
                 1, &D3D12ExperimentalShaderModels, nullptr, nullptr))) {
    LogWarningFmt(L"D3D12EnableExperimentalFeatures("
                  L"D3D12ExperimentalShaderModels) failed: 0x%08x",
                  HR);
    return false;
  }

  return true;
}

static void
setGlobalConfiguration(const std::optional<AgilitySDKConfiguration> &C) {
  if (enableGlobalAgilitySDK(C))
    LogCommentFmt(L"Agility SDK enabled.");
  else
    LogCommentFmt(L"Agility SDK not enabled.");

  if (!useDebugIfaces())
    LogCommentFmt(L"Debug layer disabled.");
  else if (enableGlobalDebugLayer())
    LogCommentFmt(L"Debug layer enabled.");
  else
    LogCommentFmt(L"Debug layer not enabled.");

  if (enableGlobalExperimentalMode())
    LogCommentFmt(L"Experimental mode enabled.");
  else
    LogCommentFmt(L"Experimental mode not enabled.");
}

static bool enableExperimentalMode(ID3D12DeviceFactory *DeviceFactory) {
  if (!isExperimentalShadersEnabled())
    return false;

  HRESULT HR;
  if (FAILED(HR = DeviceFactory->EnableExperimentalFeatures(
                 1, &D3D12ExperimentalShaderModels, nullptr, nullptr))) {
    LogWarningFmt(L"EnableExperimentalFeature(D3D12ExperimentalShaderModels) "
                  L"failed: 0x%08x",
                  HR);
    return false;
  }

  return true;
}

// An ID3D12DeviceFactory hosts its own D3D12Core, so the debug layer must be
// enabled through the factory's configuration interface.
static bool enableDebugLayerOnFactory(ID3D12DeviceFactory *DeviceFactory) {
  CComPtr<ID3D12Debug> DebugController;
  HRESULT HR;
  if (FAILED(HR = DeviceFactory->GetConfigurationInterface(
                 CLSID_D3D12Debug, IID_PPV_ARGS(&DebugController)))) {
    LogWarningFmt(L"Failed to get ID3D12Debug from device factory: 0x%08x", HR);
    return false;
  }

  DebugController->EnableDebugLayer();
  DebugLayerEnabled = true;
  return true;
}

static CComPtr<ID3D12DeviceFactory>
createDeviceFactorySDK(const AgilitySDKConfiguration &C) {
  HRESULT HR;

  CComPtr<ID3D12SDKConfiguration1> SDKConfig;
  if (FAILED(HR = D3D12GetInterface(CLSID_D3D12SDKConfiguration,
                                    IID_PPV_ARGS(&SDKConfig)))) {
    LogCommentFmt(L"Failed to get ID3D12SDKConfiguration1 interface: 0x%08x",
                  HR);
    return nullptr;
  }

  CComPtr<ID3D12DeviceFactory> DeviceFactory;
  if (FAILED(
          HR = SDKConfig->CreateDeviceFactory(C.SDKVersion, CW2A(C.SDKPath),
                                              IID_PPV_ARGS(&DeviceFactory)))) {
    LogCommentFmt(L"CreateDeviceFactory(%d, '%s', ...) failed: 0x%08x",
                  C.SDKVersion, static_cast<const wchar_t *>(C.SDKPath), HR);
    return nullptr;
  }

  LogCommentFmt(L"Using DeviceFactory for SDKVersion %d, SDKPath %s",
                C.SDKVersion, static_cast<const wchar_t *>(C.SDKPath));

  if (!useDebugIfaces())
    LogCommentFmt(L"Debug layer disabled.");
  else if (enableDebugLayerOnFactory(DeviceFactory))
    LogCommentFmt(L"Debug layer enabled on device factory.");
  else
    LogCommentFmt(L"Debug layer not enabled on device factory.");

  if (enableExperimentalMode(DeviceFactory))
    LogCommentFmt(L"Experimental mode enabled.");
  else
    LogCommentFmt(L"Experimental mode not enabled.");

  return DeviceFactory;
}

D3D12SDKSelector::D3D12SDKSelector() {
  std::optional<AgilitySDKConfiguration> C = getAgilitySDKConfiguration();

  if (C && C->SDKVersion > 0) {
    // createDeviceFactorySDK() enables the debug layer on the factory itself.
    CComPtr<ID3D12DeviceFactory> DeviceFactory = createDeviceFactorySDK(*C);
    if (DeviceFactory) {
      this->DeviceFactory = DeviceFactory;
      return;
    }
  }

  setGlobalConfiguration(C);
}

D3D12SDKSelector::~D3D12SDKSelector() {
  if (DeviceFactory) {
    DeviceFactory.Release();

    HRESULT HR;
    CComPtr<ID3D12SDKConfiguration1> SDKConfig;
    if (FAILED(HR = D3D12GetInterface(CLSID_D3D12SDKConfiguration,
                                      IID_PPV_ARGS(&SDKConfig)))) {
      LogCommentFmt(L"Failed to get ID3D12SDKConfiguration1 interface: 0x%08x",
                    HR);
      return;
    }

    // Workaround internal bug #55347376 by not calling FreeUnusedSDKs
    // SDKConfig->FreeUnusedSDKs();
  }
}

bool D3D12SDKSelector::createDevice(ID3D12Device **D3DDevice,
                                    D3D_SHADER_MODEL TestModel,
                                    bool SkipUnsupported) {

  if (DeviceFactory) {
    LogCommentFmt(L"Creating device using DeviceFactory");
    return ::createDevice(
        D3DDevice, TestModel, SkipUnsupported,
        [&](IUnknown *A, D3D_FEATURE_LEVEL FL, REFIID R, void **P) {
          LogCommentFmt(L"Calling DeviceFactory->CreateDevice");
          HRESULT HR = DeviceFactory->CreateDevice(A, FL, R, P);
          LogCommentFmt(L" Result: 0x%x", HR);
          return HR;
        });
  }

  return ::createDevice(D3DDevice, TestModel, SkipUnsupported,
                        D3D12CreateDevice);
}

bool isWarp(ID3D12Device *D3DDevice) {
  CComPtr<IDXGIFactory4> DXGIFactory;
  VERIFY_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&DXGIFactory)));

  CComPtr<IDXGIAdapter1> DXGIAdapter;
  VERIFY_SUCCEEDED(DXGIFactory->EnumAdapterByLuid(D3DDevice->GetAdapterLuid(),
                                                  IID_PPV_ARGS(&DXGIAdapter)));

  DXGI_ADAPTER_DESC1 AdapterDesc;
  VERIFY_SUCCEEDED(DXGIAdapter->GetDesc1(&AdapterDesc));
  return (AdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0;
}

bool doesDeviceSupportInt64(ID3D12Device *pDevice) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS1 O;
  if (FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS1, &O, sizeof(O))))
    return false;
  return O.Int64ShaderOps != FALSE;
}

bool doesDeviceSupportDouble(ID3D12Device *pDevice) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS O;
  if (FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS, &O, sizeof(O))))
    return false;
  return O.DoublePrecisionFloatShaderOps != FALSE;
}

bool doesDeviceSupportWaveOps(ID3D12Device *pDevice) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS1 O;
  if (FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS1, &O, sizeof(O))))
    return false;
  return O.WaveOps != FALSE;
}

bool doesDeviceSupportBarycentrics(ID3D12Device *pDevice) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS3 O;
  if (FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS3, &O, sizeof(O))))
    return false;
  return O.BarycentricsSupported != FALSE;
}

bool doesDeviceSupportNative16bitOps(ID3D12Device *pDevice) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS4 O;
  if (FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS4, &O, sizeof(O))))
    return false;
  return O.Native16BitShaderOpsSupported != FALSE;
}

bool doesDeviceSupportMeshShaders(ID3D12Device *pDevice) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS7 O7;
  if (FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS7, &O7, sizeof(O7))))
    return false;
  return O7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;
}

bool doesDeviceSupportRayTracing(ID3D12Device *pDevice) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS5 O5;
  if (FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS5, &O5, sizeof(O5))))
    return false;
  return O5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
}

bool doesDeviceSupportMeshAmpDerivatives(ID3D12Device *pDevice) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS7 O7;
  D3D12_FEATURE_DATA_D3D12_OPTIONS9 O9;
  if (FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS7, &O7, sizeof(O7))) ||
      FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS9, &O9, sizeof(O9))))
    return false;
  return O7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED &&
         O9.DerivativesInMeshAndAmplificationShadersSupported != FALSE;
}

bool doesDeviceSupportTyped64Atomics(ID3D12Device *pDevice) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS9 O9;
  if (FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS9, &O9, sizeof(O9))))
    return false;
  return O9.AtomicInt64OnTypedResourceSupported != FALSE;
}

bool doesDeviceSupportHeap64Atomics(ID3D12Device *pDevice) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS11 O11;
  if (FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS11, &O11, sizeof(O11))))
    return false;
  return O11.AtomicInt64OnDescriptorHeapResourceSupported != FALSE;
}

bool doesDeviceSupportShared64Atomics(ID3D12Device *pDevice) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS9 O9;
  if (FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS9, &O9, sizeof(O9))))
    return false;
  return O9.AtomicInt64OnGroupSharedSupported != FALSE;
}

bool doesDeviceSupportAdvancedTexOps(ID3D12Device *pDevice) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS14 O14;
  if (FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS14, &O14, sizeof(O14))))
    return false;
  return O14.AdvancedTextureOpsSupported != FALSE;
}

bool doesDeviceSupportWritableMSAA(ID3D12Device *pDevice) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS14 O14;
  if (FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS14, &O14, sizeof(O14))))
    return false;
  return O14.WriteableMSAATexturesSupported != FALSE;
}

bool doesDeviceSupportEnhancedBarriers(ID3D12Device *pDevice) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS12 O12;
  if (FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS12, &O12, sizeof(O12))))
    return false;
  return O12.EnhancedBarriersSupported != FALSE;
}

bool doesDeviceSupportRelaxedFormatCasting(ID3D12Device *pDevice) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS12 O12;
  if (!doesDeviceSupportEnhancedBarriers(pDevice))
    return false;

  if (FAILED(pDevice->CheckFeatureSupport(
          (D3D12_FEATURE)D3D12_FEATURE_D3D12_OPTIONS12, &O12, sizeof(O12))))
    return false;
  return O12.RelaxedFormatCastingSupported != FALSE;
}

bool isFallbackPathEnabled() {
  // Enable fallback paths with: /p:"EnableFallback=1"
  UINT EnableFallbackValue = 0;
  WEX::TestExecution::RuntimeParameters::TryGetValue(L"EnableFallback",
                                                     EnableFallbackValue);
  return EnableFallbackValue != 0;
}

namespace linalg_test {

// The runtime answers with a BOOL, so the only thing left to police is that it
// is a canonical TRUE or FALSE rather than an arbitrary non-zero value. Every
// BOOL-valued response goes through here so that no category silently accepts
// a malformed value by normalizing it to true.
static bool isCanonicalBool(BOOL Value) {
  return Value == TRUE || Value == FALSE;
}

bool MatrixConstructionSupport::valid() const {
  return isCanonicalBool(Supported);
}

bool MatrixConstructionSupport::supported() const {
  return valid() && Supported != FALSE;
}

bool hasFlag(
    linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS Value,
    linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS Flag) {
  return (static_cast<UINT>(Value) & static_cast<UINT>(Flag)) != 0;
}

bool WaveMatrixMultiplySupport::valid() const {
  return isValidMultiplicationFlags(static_cast<UINT>(SupportFlags),
                                    MatrixMultiplicationFlags);
}

bool WaveMatrixMultiplySupport::supported() const {
  return valid() &&
         hasFlag(
             SupportFlags,
             linalg_abi::
                 D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED);
}

bool ThreadGroupMatrixMultiplySupport::supported() const {
  return valid() &&
         hasFlag(
             SupportFlags,
             linalg_abi::
                 D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED);
}

bool ThreadGroupMatrixMultiplySupport::valid() const {
  if (!isValidMultiplicationFlags(static_cast<UINT>(SupportFlags),
                                  MatrixMultiplicationFlags))
    return false;
  if (!hasFlag(SupportFlags,
               linalg_abi::
                   D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED))
    return true;
  return MinThreadGroupSize != 0 && MaxThreadGroupSize >= MinThreadGroupSize &&
         MaxThreadGroupSize % MinThreadGroupSize == 0 &&
         (PreferredThreadGroupSize == 0 ||
          (PreferredThreadGroupSize >= MinThreadGroupSize &&
           PreferredThreadGroupSize <= MaxThreadGroupSize &&
           PreferredThreadGroupSize % MinThreadGroupSize == 0));
}

bool ThreadGroupMatrixMultiplySupport::supportsThreadGroupSize(
    UINT ThreadGroupSize) const {
  return supported() && MinThreadGroupSize != 0 &&
         ThreadGroupSize >= MinThreadGroupSize &&
         ThreadGroupSize <= MaxThreadGroupSize &&
         ThreadGroupSize % MinThreadGroupSize == 0;
}

bool ThreadVectorMatrixMultiplySupport::supported() const {
  return valid() &&
         hasFlag(
             SupportFlags,
             linalg_abi::
                 D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED);
}

bool ThreadVectorMatrixMultiplySupport::valid() const {
  return isValidThreadVectorMultiplicationFlags(static_cast<UINT>(SupportFlags),
                                                MatrixInputType);
}

bool AtomicAccumulateStoreSupport::supports(
    AtomicDestination Destination) const {
  switch (Destination) {
  case AtomicDestination::RWByteAddressBuffer:
    return RWByteAddressBufferSupported;
  case AtomicDestination::GroupShared:
    return GroupSharedSupported;
  }
  return false;
}

ScopeFlags
legalScopes(linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE Operation) {
  using hlsl::DXIL::MatrixScope;
  const ScopeFlags Thread = scopeBit(MatrixScope::Thread);
  const ScopeFlags Wave = scopeBit(MatrixScope::Wave);
  const ScopeFlags ThreadGroup = scopeBit(MatrixScope::ThreadGroup);
  switch (Operation) {
  case linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_MATRIX_CONSTRUCTION:
    // Per D3D12LinearAlgebraRuntimeFeatureSupport.md, this query "indicates a
    // driver's level of support for general operations on wave-scope and
    // group-scope matrices". Thread scope is deliberately excluded: the spec
    // states there is no requirement around thread-scope vector-matrix
    // multiplication dimensions, and neither the support struct nor the
    // enumeration entry for THREAD_VECTOR_MATRIX_MULTIPLY carries a shape.
    return Wave | ThreadGroup;
  case linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_WAVE_MATRIX_MULTIPLY:
    return Wave;
  case linalg_abi::
      D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREADGROUP_MATRIX_MULTIPLY:
    return ThreadGroup;
  case linalg_abi::
      D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREAD_VECTOR_MATRIX_MULTIPLY:
  case linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREAD_OUTER_PRODUCT:
    return Thread;
  case linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_ATOMIC_ACCUMULATE_STORE:
    // The query category spans thread vector/outer-product accumulation and
    // Wave/ThreadGroup matrix forms. Individual operations narrow this mask.
    return Thread | Wave | ThreadGroup;
  }
  return 0;
}

bool isLegalScope(linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE Operation,
                  hlsl::DXIL::MatrixScope Scope) {
  return (legalScopes(Operation) & scopeBit(Scope)) != 0;
}

Applicability classifyApplicability(HRESULT QueryResult, bool Supported,
                                    CapabilityRequirement Requirement) {
  if (FAILED(QueryResult))
    return Applicability::Fail;
  if (Supported)
    return Applicability::Execute;
  if (Requirement == CapabilityRequirement::CapabilityGated)
    return Applicability::NotApplicable;
  return Applicability::Fail;
}

HRESULT queryTierSupport(ID3D12Device *Device, TierSupport &Support) {
  Support = {};
  if (!Device)
    return E_INVALIDARG;

  linalg_abi::D3D12_FEATURE_DATA_LINEAR_ALGEBRA_SUPPORT RuntimeSupport = {};
  const HRESULT HR = Device->CheckFeatureSupport(
      linalg_abi::D3D12_FEATURE_LINEAR_ALGEBRA_SUPPORT, &RuntimeSupport,
      sizeof(RuntimeSupport));
  if (FAILED(HR)) {
    LogCommentFmt(L"Linear algebra tier query failed: 0x%08x", HR);
    return HR;
  }

  if (RuntimeSupport.LinearAlgebraTier !=
          static_cast<UINT>(
              linalg_abi::D3D12_LINEAR_ALGEBRA_TIER_NOT_SUPPORTED) &&
      RuntimeSupport.LinearAlgebraTier !=
          static_cast<UINT>(linalg_abi::D3D12_LINEAR_ALGEBRA_TIER_1_0)) {
    LogCommentFmt(L"Linear algebra tier query returned invalid tier: 0x%x",
                  RuntimeSupport.LinearAlgebraTier);
    return E_UNEXPECTED;
  }

  Support.LinearAlgebraTier =
      static_cast<linalg_abi::D3D12_LINEAR_ALGEBRA_TIER>(
          RuntimeSupport.LinearAlgebraTier);
  LogCommentFmt(L"Linear algebra tier: 0x%x",
                static_cast<UINT>(Support.LinearAlgebraTier));
  return S_OK;
}

HRESULT queryMatrixConstruction(ID3D12Device *Device,
                                const MatrixConstructionQuery &Query,
                                MatrixConstructionSupport &Support) {
  Support = {};
  if (!Device)
    return E_INVALIDARG;

  linalg_abi::D3D12_FEATURE_DATA_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT
      RuntimeSupport = {};
  RuntimeSupport.OperationType = static_cast<UINT>(
      linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_MATRIX_CONSTRUCTION);
  RuntimeSupport.MatrixConstruction.ComponentType =
      static_cast<UINT>(Query.ComponentType);
  RuntimeSupport.MatrixConstruction.WaveSize = Query.WaveSize;
  RuntimeSupport.MatrixConstruction.Shape = {
      Query.Shape.M,
      Query.Shape.K,
      Query.Shape.N,
  };

  const HRESULT HR = Device->CheckFeatureSupport(
      linalg_abi::D3D12_FEATURE_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT,
      &RuntimeSupport, sizeof(RuntimeSupport));
  if (FAILED(HR)) {
    LogCommentFmt(L"MatrixConstruction query failed: type=%s, wave=%u, "
                  L"shape=(%u,%u,%u), hr=0x%08x",
                  dataTypeName(Query.ComponentType), Query.WaveSize,
                  Query.Shape.M, Query.Shape.K, Query.Shape.N, HR);
    return HR;
  }

  Support.Supported = RuntimeSupport.MatrixConstruction.Supported;
  if (!Support.valid()) {
    LogCommentFmt(L"MatrixConstruction query returned a non-boolean result: "
                  L"0x%x",
                  Support.Supported);
    Support = {};
    return E_UNEXPECTED;
  }

  LogCommentFmt(L"MatrixConstruction query: type=%s, wave=%u, "
                L"shape=(%u,%u,%u), supported=%u",
                dataTypeName(Query.ComponentType), Query.WaveSize,
                Query.Shape.M, Query.Shape.K, Query.Shape.N,
                Support.supported());
  return S_OK;
}

HRESULT queryWaveMatrixMultiply(ID3D12Device *Device,
                                const WaveMatrixMultiplyQuery &Query,
                                WaveMatrixMultiplySupport &Support) {
  Support = {};
  if (!Device)
    return E_INVALIDARG;

  linalg_abi::D3D12_FEATURE_DATA_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT
      RuntimeSupport = {};
  RuntimeSupport.OperationType = static_cast<UINT>(
      linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_WAVE_MATRIX_MULTIPLY);
  setWaveInputs(RuntimeSupport.WaveMatrixMultiply.Inputs, Query.Inputs);
  RuntimeSupport.WaveMatrixMultiply.Shape = {
      Query.Shape.M,
      Query.Shape.K,
      Query.Shape.N,
  };

  const HRESULT HR = Device->CheckFeatureSupport(
      linalg_abi::D3D12_FEATURE_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT,
      &RuntimeSupport, sizeof(RuntimeSupport));
  if (FAILED(HR)) {
    LogCommentFmt(L"WaveMatrixMultiply query failed: wave=%u, A=%s, B=%s, "
                  L"Acc=%s, shape=(%u,%u,%u), hr=0x%08x",
                  Query.Inputs.WaveSize,
                  dataTypeName(Query.Inputs.MatrixAComponentType),
                  dataTypeName(Query.Inputs.MatrixBComponentType),
                  dataTypeName(Query.Inputs.AccumulatorComponentType),
                  Query.Shape.M, Query.Shape.K, Query.Shape.N, HR);
    return HR;
  }

  const UINT Flags = RuntimeSupport.WaveMatrixMultiply.SupportFlags;
  if (!isValidMultiplicationFlags(Flags, MatrixMultiplicationFlags)) {
    LogCommentFmt(L"WaveMatrixMultiply query returned invalid flags: 0x%x",
                  Flags);
    return E_UNEXPECTED;
  }
  Support.SupportFlags = static_cast<
      linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS>(Flags);

  LogCommentFmt(L"WaveMatrixMultiply query: wave=%u, A=%s, B=%s, Acc=%s, "
                L"shape=(%u,%u,%u), flags=0x%x",
                Query.Inputs.WaveSize,
                dataTypeName(Query.Inputs.MatrixAComponentType),
                dataTypeName(Query.Inputs.MatrixBComponentType),
                dataTypeName(Query.Inputs.AccumulatorComponentType),
                Query.Shape.M, Query.Shape.K, Query.Shape.N, Flags);
  return S_OK;
}

HRESULT
queryThreadGroupMatrixMultiply(ID3D12Device *Device,
                               const ThreadGroupMatrixMultiplyQuery &Query,
                               ThreadGroupMatrixMultiplySupport &Support) {
  Support = {};
  if (!Device)
    return E_INVALIDARG;

  linalg_abi::D3D12_FEATURE_DATA_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT
      RuntimeSupport = {};
  RuntimeSupport.OperationType = static_cast<UINT>(
      linalg_abi::
          D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREADGROUP_MATRIX_MULTIPLY);
  setWaveInputs(RuntimeSupport.ThreadGroupMatrixMultiply.WaveInputs,
                Query.WaveInputs);
  RuntimeSupport.ThreadGroupMatrixMultiply.Shape = {
      Query.Shape.M,
      Query.Shape.K,
      Query.Shape.N,
  };

  const HRESULT HR = Device->CheckFeatureSupport(
      linalg_abi::D3D12_FEATURE_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT,
      &RuntimeSupport, sizeof(RuntimeSupport));
  if (FAILED(HR)) {
    LogCommentFmt(
        L"ThreadGroupMatrixMultiply query failed: wave=%u, A=%s, B=%s, "
        L"Acc=%s, shape=(%u,%u,%u), hr=0x%08x",
        Query.WaveInputs.WaveSize,
        dataTypeName(Query.WaveInputs.MatrixAComponentType),
        dataTypeName(Query.WaveInputs.MatrixBComponentType),
        dataTypeName(Query.WaveInputs.AccumulatorComponentType), Query.Shape.M,
        Query.Shape.K, Query.Shape.N, HR);
    return HR;
  }

  const UINT Flags = RuntimeSupport.ThreadGroupMatrixMultiply.SupportFlags;
  if (!isValidMultiplicationFlags(Flags, MatrixMultiplicationFlags)) {
    LogCommentFmt(
        L"ThreadGroupMatrixMultiply query returned invalid flags: 0x%x", Flags);
    return E_UNEXPECTED;
  }

  const bool Supported =
      (Flags &
       static_cast<UINT>(
           linalg_abi::
               D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED)) !=
      0;
  const UINT MinSize =
      RuntimeSupport.ThreadGroupMatrixMultiply.MinThreadGroupSize;
  const UINT MaxSize =
      RuntimeSupport.ThreadGroupMatrixMultiply.MaxThreadGroupSize;
  const UINT PreferredSize =
      RuntimeSupport.ThreadGroupMatrixMultiply.PreferredThreadGroupSize;
  if (Supported &&
      (MinSize == 0 || MaxSize < MinSize || MaxSize % MinSize != 0 ||
       (PreferredSize != 0 &&
        (PreferredSize < MinSize || PreferredSize > MaxSize ||
         PreferredSize % MinSize != 0)))) {
    LogCommentFmt(
        L"ThreadGroupMatrixMultiply query returned malformed group sizes: "
        L"min=%u, max=%u, preferred=%u",
        MinSize, MaxSize, PreferredSize);
    return E_UNEXPECTED;
  }

  Support.SupportFlags = static_cast<
      linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS>(Flags);
  Support.MinThreadGroupSize = MinSize;
  Support.MaxThreadGroupSize = MaxSize;
  Support.PreferredThreadGroupSize = PreferredSize;
  LogCommentFmt(
      L"ThreadGroupMatrixMultiply query: wave=%u, A=%s, B=%s, Acc=%s, "
      L"shape=(%u,%u,%u), flags=0x%x, minGroup=%u, maxGroup=%u, "
      L"preferredGroup=%u",
      Query.WaveInputs.WaveSize,
      dataTypeName(Query.WaveInputs.MatrixAComponentType),
      dataTypeName(Query.WaveInputs.MatrixBComponentType),
      dataTypeName(Query.WaveInputs.AccumulatorComponentType), Query.Shape.M,
      Query.Shape.K, Query.Shape.N, Flags, MinSize, MaxSize, PreferredSize);
  return S_OK;
}

HRESULT
queryThreadVectorMatrixMultiply(ID3D12Device *Device,
                                const ThreadVectorMatrixMultiplyQuery &Query,
                                ThreadVectorMatrixMultiplySupport &Support) {
  Support = {};
  if (!Device)
    return E_INVALIDARG;

  linalg_abi::D3D12_FEATURE_DATA_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT
      RuntimeSupport = {};
  RuntimeSupport.OperationType = static_cast<UINT>(
      linalg_abi::
          D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREAD_VECTOR_MATRIX_MULTIPLY);
  RuntimeSupport.ThreadVectorMatrixMultiply.VectorInputType =
      static_cast<UINT>(Query.VectorInputType);
  RuntimeSupport.ThreadVectorMatrixMultiply.MatrixInputType =
      static_cast<UINT>(Query.MatrixInputType);
  RuntimeSupport.ThreadVectorMatrixMultiply.BiasInputType =
      static_cast<UINT>(Query.BiasInputType);
  RuntimeSupport.ThreadVectorMatrixMultiply.VectorResultType =
      static_cast<UINT>(Query.VectorResultType);

  const HRESULT HR = Device->CheckFeatureSupport(
      linalg_abi::D3D12_FEATURE_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT,
      &RuntimeSupport, sizeof(RuntimeSupport));
  if (FAILED(HR)) {
    LogCommentFmt(
        L"ThreadVectorMatrixMultiply query failed: vector=%s, matrix=%s, "
        L"bias=%s, result=%s, hr=0x%08x",
        dataTypeName(Query.VectorInputType),
        dataTypeName(Query.MatrixInputType), dataTypeName(Query.BiasInputType),
        dataTypeName(Query.VectorResultType), HR);
    return HR;
  }

  const UINT Flags = RuntimeSupport.ThreadVectorMatrixMultiply.SupportFlags;
  if (!isValidThreadVectorMultiplicationFlags(Flags, Query.MatrixInputType)) {
    LogCommentFmt(
        L"ThreadVectorMatrixMultiply query returned invalid flags: 0x%x",
        Flags);
    return E_UNEXPECTED;
  }

  Support.SupportFlags = static_cast<
      linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS>(Flags);
  Support.MatrixInputType = Query.MatrixInputType;
  LogCommentFmt(
      L"ThreadVectorMatrixMultiply query: vector=%s, matrix=%s, bias=%s, "
      L"result=%s, flags=0x%x",
      dataTypeName(Query.VectorInputType), dataTypeName(Query.MatrixInputType),
      dataTypeName(Query.BiasInputType), dataTypeName(Query.VectorResultType),
      Flags);
  return S_OK;
}

HRESULT queryThreadOuterProduct(ID3D12Device *Device,
                                const ThreadOuterProductQuery &Query,
                                ThreadOuterProductSupport &Support) {
  Support = {};
  if (!Device)
    return E_INVALIDARG;

  linalg_abi::D3D12_FEATURE_DATA_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT
      RuntimeSupport = {};
  RuntimeSupport.OperationType = static_cast<UINT>(
      linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREAD_OUTER_PRODUCT);
  RuntimeSupport.ThreadOuterProductSupport.InputComponentType =
      static_cast<UINT>(Query.InputComponentType);
  RuntimeSupport.ThreadOuterProductSupport.ResultComponentType =
      static_cast<UINT>(Query.ResultComponentType);

  const HRESULT HR = Device->CheckFeatureSupport(
      linalg_abi::D3D12_FEATURE_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT,
      &RuntimeSupport, sizeof(RuntimeSupport));
  if (FAILED(HR)) {
    LogCommentFmt(
        L"ThreadOuterProduct query failed: input=%s, result=%s, hr=0x%08x",
        dataTypeName(Query.InputComponentType),
        dataTypeName(Query.ResultComponentType), HR);
    return HR;
  }

  const BOOL Supported = RuntimeSupport.ThreadOuterProductSupport.Supported;
  if (!isCanonicalBool(Supported)) {
    LogCommentFmt(
        L"ThreadOuterProduct query returned a non-boolean result: 0x%x",
        Supported);
    return E_UNEXPECTED;
  }

  Support.Supported = Supported != FALSE;
  LogCommentFmt(L"ThreadOuterProduct query: input=%s, result=%s, supported=%u",
                dataTypeName(Query.InputComponentType),
                dataTypeName(Query.ResultComponentType), Support.Supported);
  return S_OK;
}

HRESULT queryAtomicAccumulateStore(ID3D12Device *Device,
                                   const AtomicAccumulateStoreQuery &Query,
                                   AtomicAccumulateStoreSupport &Support) {
  Support = {};
  if (!Device)
    return E_INVALIDARG;

  linalg_abi::D3D12_FEATURE_DATA_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT
      RuntimeSupport = {};
  RuntimeSupport.OperationType = static_cast<UINT>(
      linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_ATOMIC_ACCUMULATE_STORE);
  RuntimeSupport.AccumulateStore.ComponentType =
      static_cast<UINT>(Query.ComponentType);

  const HRESULT HR = Device->CheckFeatureSupport(
      linalg_abi::D3D12_FEATURE_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT,
      &RuntimeSupport, sizeof(RuntimeSupport));
  if (FAILED(HR)) {
    LogCommentFmt(L"AtomicAccumulateStore query failed: type=%s, hr=0x%08x",
                  dataTypeName(Query.ComponentType), HR);
    return HR;
  }

  const BOOL BufferSupported =
      RuntimeSupport.AccumulateStore.RWByteAddressBufferSupported;
  const BOOL GroupSharedSupported =
      RuntimeSupport.AccumulateStore.GroupSharedSupported;
  if (!isCanonicalBool(BufferSupported) ||
      !isCanonicalBool(GroupSharedSupported)) {
    LogCommentFmt(L"AtomicAccumulateStore query returned a non-boolean result: "
                  L"UAV=0x%x, groupshared=0x%x",
                  BufferSupported, GroupSharedSupported);
    return E_UNEXPECTED;
  }

  Support.RWByteAddressBufferSupported = BufferSupported != FALSE;
  Support.GroupSharedSupported = GroupSharedSupported != FALSE;
  LogCommentFmt(L"AtomicAccumulateStore query: type=%s, UAV=%u, groupshared=%u",
                dataTypeName(Query.ComponentType),
                Support.RWByteAddressBufferSupported,
                Support.GroupSharedSupported);
  return S_OK;
}

} // namespace linalg_test

// TODO(#8661): Remove me when GroupSharedLimit is available in a released
// Windows SDK.
#if defined(D3D12_PREVIEW_SDK_VERSION)
UINT getMaxGroupSharedMemoryCS(ID3D12Device *Device) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS_PREVIEW O = {};
  VERIFY_SUCCEEDED(Device->CheckFeatureSupport(
      D3D12_FEATURE_D3D12_OPTIONS_PREVIEW, &O, sizeof(O)));
  return O.MaxGroupSharedMemoryPerGroupCS;
}

UINT getMaxGroupSharedMemoryAS(ID3D12Device *Device) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS_PREVIEW O = {};
  VERIFY_SUCCEEDED(Device->CheckFeatureSupport(
      D3D12_FEATURE_D3D12_OPTIONS_PREVIEW, &O, sizeof(O)));
  return O.MaxGroupSharedMemoryPerGroupAS;
}

UINT getMaxGroupSharedMemoryMS(ID3D12Device *Device) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS_PREVIEW O = {};
  VERIFY_SUCCEEDED(Device->CheckFeatureSupport(
      D3D12_FEATURE_D3D12_OPTIONS_PREVIEW, &O, sizeof(O)));
  return O.MaxGroupSharedMemoryPerGroupMS;
}
#endif // defined(D3D12_PREVIEW_SDK_VERSION)

std::unique_ptr<st::ShaderOp> createComputeOp(const char *Source,
                                              const char *Target,
                                              const char *RootSig,
                                              const char *Args, UINT DispatchX,
                                              UINT DispatchY, UINT DispatchZ) {
  auto Op = std::make_unique<st::ShaderOp>();
  LPCSTR CSName = Op->Strings.insert("CS");
  Op->Name = CSName;
  Op->CS = CSName;
  Op->RootSignature = Op->Strings.insert(RootSig);
  Op->DispatchX = DispatchX;
  Op->DispatchY = DispatchY;
  Op->DispatchZ = DispatchZ;
  Op->UseWarpDevice = true;

  st::ShaderOpShader Shader = {};
  Shader.Name = CSName;
  Shader.Target = Op->Strings.insert(Target);
  Shader.EntryPoint = Op->Strings.insert("main");
  Shader.Text = Op->Strings.insert(Source);
  Shader.Arguments = Args ? Op->Strings.insert(Args) : nullptr;
  Shader.Compiled = FALSE;
  Shader.Callback = FALSE;
  Op->Shaders.push_back(Shader);

  return Op;
}

void addUAVBuffer(st::ShaderOp *Op, const char *Name, UINT64 Width,
                  bool ReadBack, const char *Init) {
  st::ShaderOpResource Res = {};
  Res.Name = Op->Strings.insert(Name);
  Res.Init = Op->Strings.insert(Init);
  Res.ReadBack = ReadBack ? TRUE : FALSE;

  Res.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
  Res.HeapFlags = D3D12_HEAP_FLAG_NONE;
  Res.Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  Res.Desc.Width = Width;
  Res.Desc.Height = 1;
  Res.Desc.DepthOrArraySize = 1;
  Res.Desc.MipLevels = 1;
  Res.Desc.SampleDesc.Count = 1;
  Res.Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  Res.Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  Res.InitialResourceState = D3D12_RESOURCE_STATE_COPY_DEST;
  Res.TransitionTo = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

  Op->Resources.push_back(Res);
}

void addSRVBuffer(st::ShaderOp *Op, const char *Name, UINT64 Width,
                  const char *Init) {
  st::ShaderOpResource Res = {};
  Res.Name = Op->Strings.insert(Name);
  Res.Init = Op->Strings.insert(Init);
  Res.ReadBack = FALSE;

  Res.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
  Res.HeapFlags = D3D12_HEAP_FLAG_NONE;
  Res.Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  Res.Desc.Width = Width;
  Res.Desc.Height = 1;
  Res.Desc.DepthOrArraySize = 1;
  Res.Desc.MipLevels = 1;
  Res.Desc.SampleDesc.Count = 1;
  Res.Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  Res.Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  Res.InitialResourceState = D3D12_RESOURCE_STATE_COPY_DEST;
  Res.TransitionTo = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

  Op->Resources.push_back(Res);
}

void addRootView(st::ShaderOp *Op, UINT Index, const char *ResName) {
  st::ShaderOpRootValue RV = {};
  RV.ResName = Op->Strings.insert(ResName);
  RV.HeapName = nullptr;
  RV.Index = Index;
  Op->RootValues.push_back(RV);
}

static st::ShaderOpDescriptorHeap *getOrAddCbvSrvUavHeap(st::ShaderOp *Op,
                                                         const char *HeapName) {
  if (st::ShaderOpDescriptorHeap *Existing =
          Op->GetDescriptorHeapByName(HeapName))
    return Existing;

  st::ShaderOpDescriptorHeap H = {};
  H.Name = Op->Strings.insert(HeapName);
  H.Desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  // SetDescriptorHeaps skips heaps that are not shader visible, which would
  // leave the table bound to nothing.
  H.Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  // Left at zero so CreateDescriptorHeaps sizes it from the descriptors added.
  H.Desc.NumDescriptors = 0;
  Op->DescriptorHeaps.push_back(H);
  return &Op->DescriptorHeaps.back();
}

void addHeapRawUAV(st::ShaderOp *Op, const char *HeapName, const char *ResName,
                   UINT64 ViewBytes) {
  // A raw view counts 32-bit words, so a byte size that is not a whole number
  // of words cannot be represented and would silently round down.
  VERIFY_IS_TRUE(ViewBytes % 4 == 0,
                 "raw UAV view size must be a multiple of 4 bytes");
  // NumElements is 32 bits, so a larger view cannot be described at all and
  // narrowing would silently bind a shorter one.
  VERIFY_IS_TRUE(ViewBytes / 4 <= UINT_MAX,
                 "raw UAV view size exceeds what NumElements can describe");

  st::ShaderOpDescriptor D = {};
  D.Name = Op->Strings.insert(ResName);
  D.ResName = Op->Strings.insert(ResName);
  D.Kind = Op->Strings.insert("UAV");
  D.UavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
  D.UavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
  D.UavDesc.Buffer.FirstElement = 0;
  D.UavDesc.Buffer.NumElements = static_cast<UINT>(ViewBytes / 4);
  D.UavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

  getOrAddCbvSrvUavHeap(Op, HeapName)->Descriptors.push_back(D);
}

void addRootTable(st::ShaderOp *Op, UINT Index, const char *HeapName) {
  st::ShaderOpRootValue RV = {};
  RV.ResName = nullptr;
  RV.HeapName = Op->Strings.insert(HeapName);
  RV.Index = Index;
  Op->RootValues.push_back(RV);
}

std::shared_ptr<st::ShaderOpTestResult>
runShaderOp(ID3D12Device *Device, dxc::SpecificDllLoader &DxcSupport,
            std::unique_ptr<st::ShaderOp> Op,
            st::ShaderOpTest::TInitCallbackFn InitCallback,
            st::ShaderOpTest::TCommandCallbackFn PostDispatchCallback) {
  auto OpSet = std::make_shared<st::ShaderOpSet>();
  OpSet->ShaderOps.push_back(std::move(Op));

  return st::RunShaderOpTestAfterParse(
      Device, DxcSupport, nullptr, std::move(InitCallback),
      /*pShaderCallback=*/nullptr, std::move(PostDispatchCallback),
      std::move(OpSet));
}

void compileShader(dxc::SpecificDllLoader &DxcSupport, const char *Source,
                   const char *Target, const std::string &Args,
                   bool VerboseLogging) {
  CComPtr<IDxcCompiler3> Compiler;
  VERIFY_SUCCEEDED(DxcSupport.CreateInstance(CLSID_DxcCompiler, &Compiler));

  CComPtr<IDxcUtils> Utils;
  VERIFY_SUCCEEDED(DxcSupport.CreateInstance(CLSID_DxcUtils, &Utils));

  CComPtr<IDxcBlobEncoding> SourceBlob;
  VERIFY_SUCCEEDED(Utils->CreateBlobFromPinned(
      Source, static_cast<UINT32>(strlen(Source)), DXC_CP_UTF8, &SourceBlob));

  // Build wide-string argument list: -T <target> -E main <extra args>.
  std::vector<std::wstring> WArgStorage;
  WArgStorage.push_back(L"-T");
  WArgStorage.push_back(std::wstring(Target, Target + strlen(Target)));
  WArgStorage.push_back(L"-E");
  WArgStorage.push_back(L"main");

  // Tokenize the additional arguments string.
  std::istringstream SS(Args);
  std::string Tok;
  while (SS >> Tok)
    WArgStorage.push_back(std::wstring(Tok.begin(), Tok.end()));

  std::vector<LPCWSTR> WArgPtrs;
  std::wstringstream LogFlags;
  LogFlags << L"Compiling with flags:";
  for (const auto &A : WArgStorage) {
    WArgPtrs.push_back(A.c_str());
    LogFlags << L" " << A;
  }

  DxcBuffer Buf = {};
  Buf.Ptr = SourceBlob->GetBufferPointer();
  Buf.Size = SourceBlob->GetBufferSize();
  Buf.Encoding = DXC_CP_UTF8;

  if (VerboseLogging) {
    hlsl_test::LogCommentFmt(L"Shader Source:");
    hlsl_test::LogCommentFmt(L"%S", Source);
  }

  hlsl_test::LogCommentFmt(LogFlags.str().c_str());

  CComPtr<IDxcResult> Result;
  VERIFY_SUCCEEDED(Compiler->Compile(&Buf, WArgPtrs.data(),
                                     static_cast<UINT32>(WArgPtrs.size()),
                                     nullptr, IID_PPV_ARGS(&Result)));

  HRESULT HR;
  VERIFY_SUCCEEDED(Result->GetStatus(&HR));

  if (FAILED(HR)) {
    CComPtr<IDxcBlobUtf8> Errors;
    Result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&Errors), nullptr);
    if (Errors && Errors->GetStringLength() > 0)
      hlsl_test::LogErrorFmt(L"Shader compilation failed:\n%S",
                             Errors->GetStringPointer());
    VERIFY_SUCCEEDED(HR);
  }
}

#if defined(DIRECT3D_LINEAR_ALGEBRA)
UINT getLinAlgMatrixByteSize(ID3D12Device *Device, UINT NumRows,
                             UINT NumColumns,
                             D3D12_LINEAR_ALGEBRA_DATATYPE DataType,
                             D3D12_LINEAR_ALGEBRA_MATRIX_LAYOUT Layout,
                             UINT Stride) {
  CComPtr<ID3D12DevicePreview> DevicePreview;
  VERIFY_SUCCEEDED(Device->QueryInterface(IID_PPV_ARGS(&DevicePreview)));

  D3D12_LINEAR_ALGEBRA_MATRIX_CONVERSION_DEST_INFO Info = {};
  Info.DestSize = 0;
  Info.DestLayout = Layout;
  Info.DestStride = Stride;
  Info.NumRows = NumRows;
  Info.NumColumns = NumColumns;
  Info.DestDataType = DataType;
  DevicePreview->GetLinearAlgebraMatrixConversionDestinationInfo(&Info);
  return Info.DestSize;
}

void recordLinAlgMatrixConversion(
    ID3D12GraphicsCommandList *List, ID3D12Resource *SrcBuffer, UINT SrcSize,
    ID3D12Resource *DestBuffer, UINT DestSize, UINT NumRows, UINT NumColumns,
    D3D12_LINEAR_ALGEBRA_DATATYPE DataType,
    D3D12_LINEAR_ALGEBRA_MATRIX_LAYOUT SrcLayout, UINT SrcStride,
    D3D12_LINEAR_ALGEBRA_MATRIX_LAYOUT DestLayout, UINT DestStride) {
  CComPtr<ID3D12GraphicsCommandListPreview> PreviewList;
  VERIFY_SUCCEEDED(List->QueryInterface(IID_PPV_ARGS(&PreviewList)));

  // Per the linear-algebra spec, ConvertLinearAlgebraMatrix (legacy barriers)
  // requires the source buffer in NON_PIXEL_SHADER_RESOURCE and the destination
  // in UNORDERED_ACCESS. The caller passes both in UNORDERED_ACCESS (the
  // ShaderOp default UAV state), so transition the source to the required read
  // state; the destination is already in UNORDERED_ACCESS.
  D3D12_RESOURCE_BARRIER Barrier = {};
  Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  Barrier.Transition.pResource = SrcBuffer;
  Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
  Barrier.Transition.StateAfter =
      D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
  List->ResourceBarrier(1, &Barrier);

  D3D12_LINEAR_ALGEBRA_MATRIX_CONVERSION_INFO Info = {};
  Info.DestInfo.DestSize = DestSize;
  Info.DestInfo.DestLayout = DestLayout;
  Info.DestInfo.DestStride = DestStride;
  Info.DestInfo.NumRows = NumRows;
  Info.DestInfo.NumColumns = NumColumns;
  Info.DestInfo.DestDataType = DataType;
  Info.SrcInfo.SrcSize = SrcSize;
  Info.SrcInfo.SrcDataType = DataType;
  Info.SrcInfo.SrcLayout = SrcLayout;
  Info.SrcInfo.SrcStride = SrcStride;
  Info.DataDesc.DestVA = DestBuffer->GetGPUVirtualAddress();
  Info.DataDesc.SrcVA = SrcBuffer->GetGPUVirtualAddress();
  PreviewList->ConvertLinearAlgebraMatrix(&Info, 1);
}
#endif // defined(DIRECT3D_LINEAR_ALGEBRA)
