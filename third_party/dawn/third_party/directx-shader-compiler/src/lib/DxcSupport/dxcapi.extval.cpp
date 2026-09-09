#ifndef _WIN32
#include "dxc/WinAdapter.h"
#endif

#include "dxc/Support/Global.h" // for hresult handling
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/WinIncludes.h"
#include <filesystem> // C++17 and later
#include <sstream>
// WinIncludes must come before dxcapi.extval.h
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/dxcapi.extval.h"
#include "dxc/Support/dxcapi.impl.h"
#include "dxc/Support/microcom.h"
#include "dxc/dxcapi.internal.h"
#include <iostream>

#include "llvm/ADT/SmallVector.h"

namespace {

// The ExtValidationArgHelper class helps manage arguments for external
// validation, as well as perform the validation step if needed.
class ExtValidationArgHelper {
public:
  ExtValidationArgHelper() = default;
  ExtValidationArgHelper(const ExtValidationArgHelper &) = delete;
  HRESULT initialize(IDxcValidator *NewValidator, LPCWSTR **Arguments,
                     UINT32 *ArgCount, LPCWSTR TargetProfile = nullptr) {
    IFR(setValidator(NewValidator));
    IFR(processArgs(Arguments, ArgCount, TargetProfile));
    return S_OK;
  }

  HRESULT doValidation(IDxcOperationResult *CompileResult, REFIID Riid,
                       void **ValResult) {

    if (!CompileResult)
      return E_INVALIDARG;

    // No validation needed; just set the result and return.
    if (!NeedsValidation)
      return CompileResult->QueryInterface(Riid, ValResult);

    // Get the compiled shader.
    CComPtr<IDxcBlob> CompiledBlob;
    IFR(CompileResult->GetResult(&CompiledBlob));

    // If no compiled blob; just place the compile result
    // into ValResult and return.
    if (!CompiledBlob)
      return CompileResult->QueryInterface(Riid, ValResult);

    // Validate the compiled shader.
    CComPtr<IDxcOperationResult> TempValidationResult;
    UINT32 DxcValidatorFlags =
        DxcValidatorFlags_InPlaceEdit |
        (RootSignatureOnly ? DxcValidatorFlags_RootSignatureOnly : 0);
    IFR(Validator->Validate(CompiledBlob, DxcValidatorFlags,
                            &TempValidationResult));

    // Return the validation result if it failed.
    HRESULT HR;
    IFR(TempValidationResult->GetStatus(&HR));
    if (FAILED(HR)) {
      // prefix the stderr output
      fprintf(stderr, "error: validation errors\r\n");
      return TempValidationResult->QueryInterface(Riid, ValResult);
    }

    // Validation succeeded. Return the original compile result.
    return CompileResult->QueryInterface(Riid, ValResult);
  }

private:
  std::vector<std::wstring> ArgStorage;
  llvm::SmallVector<LPCWSTR, 16> NewArgs;
  hlsl::options::DxcOpts Opts;
  CComPtr<IDxcValidator> Validator;
  UINT32 ValidatorVersionMajor = 0;
  UINT32 ValidatorVersionMinor = 0;
  bool NeedsValidation = false;
  bool RootSignatureOnly = false;

  /// Add argument to ArgStorage to be referenced by NewArgs.
  void addArgument(const std::wstring &Arg) { ArgStorage.push_back(Arg); }

  HRESULT setValidator(IDxcValidator *NewValidator) {
    DXASSERT(NewValidator, "Invalid Validator argument");
    Validator = NewValidator;

    // 1.0 had no version interface.
    ValidatorVersionMajor = 1;
    ValidatorVersionMinor = 0;
    CComPtr<IDxcVersionInfo> ValidatorVersionInfo;
    if (SUCCEEDED(
            Validator->QueryInterface(IID_PPV_ARGS(&ValidatorVersionInfo)))) {
      if (FAILED(ValidatorVersionInfo->GetVersion(&ValidatorVersionMajor,
                                                  &ValidatorVersionMinor))) {
        DXASSERT(false, "Failed to get validator version");
        return E_FAIL;
      }
    }
    return S_OK;
  }

  /// Process arguments, adding validator version and disable validation if
  /// needed. If TargetProfile provided, use this instead of -T argument.
  HRESULT processArgs(LPCWSTR **Arguments, UINT32 *ArgCount,
                      LPCWSTR TargetProfile = nullptr) {
    DXASSERT((Arguments && ArgCount) && (*Arguments || *ArgCount == 0) &&
                 *ArgCount < INT_MAX - 3,
             "Invalid Arguments and ArgCount arguments");
    NeedsValidation = false;

    // Must not call twice.
    DXASSERT(NewArgs.empty(), "Must not call twice");

    // Parse compiler arguments to Opts.
    std::string Errors;
    llvm::raw_string_ostream OS(Errors);
    hlsl::options::MainArgs MainArgs(static_cast<int>(*ArgCount), *Arguments,
                                     0);
    if (hlsl::options::ReadDxcOpts(hlsl::options::getHlslOptTable(),
                                   hlsl::options::CompilerFlags, MainArgs, Opts,
                                   OS))
      return E_FAIL;

    // Determine important conditions from arguments.
    const bool ProduceDxModule = Opts.ProduceDxModule();
    NeedsValidation = Opts.NeedsValidation();
    RootSignatureOnly = Opts.IsRootSignatureProfile();

    // Add extra arguments as needed
    if (ProduceDxModule) {
      if (Opts.ValVerMajor == UINT_MAX) {
        // If not supplied, add validator version arguments.
        addArgument(L"-validator-version");
        std::wstring VerStr = std::to_wstring(ValidatorVersionMajor) + L"." +
                              std::to_wstring(ValidatorVersionMinor);
        addArgument(VerStr);
      }

      // Add argument to disable validation, so we can call it ourselves
      // later.
      if (NeedsValidation)
        addArgument(L"-Vd");
    }

    if (ArgStorage.empty())
      return S_OK;

    // Reference added arguments from storage.
    NewArgs.reserve(*ArgCount + ArgStorage.size());

    // Copied arguments refer to original strings.
    for (UINT32 I = 0; I < *ArgCount; ++I)
      NewArgs.push_back((*Arguments)[I]);

    for (const auto &Arg : ArgStorage)
      NewArgs.push_back(Arg.c_str());

    *Arguments = NewArgs.data();
    *ArgCount = (UINT32)NewArgs.size();
    return S_OK;
  }
};

// ExternalValidationCompiler wraps a DxCompiler to use an external validator
// instead of the internal one. It disables internal validation by adding
// '-Vd' and sets the default validator version with '-validator-version'.
// After a successful compilation, it uses the provided IDxcValidator to
// perform validation when it would normally be performed.
class ExternalValidationCompiler : public IDxcCompiler2,
                                   public IDxcCompiler3,
                                   public IDxcVersionInfo2,
                                   public IDxcVersionInfo3 {
public:
  ExternalValidationCompiler(IMalloc *Malloc, IDxcValidator *OtherValidator,
                             IUnknown *OtherCompiler)
      : Validator(OtherValidator), Compiler(OtherCompiler), m_pMalloc(Malloc) {}

  // IUnknown implementation
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(ExternalValidationCompiler)

  // QueryInterface creates a new wrapper for the correct compiler interface
  // retrieved by calling QueryInterface on the compiler. The new wrapper
  // keeps the IID and returned compiler interface pointer for use in
  // associated interface methods. Those methods can then use
  // castCompilerUnsafe to upcast the pointer to the appropriate interface
  // for the method.
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID Iid,
                                           void **ResultObject) override {
    if (!ResultObject)
      return E_POINTER;

    *ResultObject = nullptr;

    // Get pointer for an interface the compiler object supports.
    CComPtr<IUnknown> TempCompiler;
    IFR(Compiler->QueryInterface(Iid, (void **)&TempCompiler));

    try {
      // This can throw; do not leak C++ exceptions through COM method
      // calls.
      CComPtr<ExternalValidationCompiler> NewWrapper(
          Alloc(m_pMalloc, Validator, TempCompiler));
      return DoBasicQueryInterface<IDxcCompiler, IDxcCompiler2, IDxcCompiler3,
                                   IDxcVersionInfo, IDxcVersionInfo2,
                                   IDxcVersionInfo3>(NewWrapper.p, Iid,
                                                     ResultObject);
    } catch (...) {
      return E_FAIL;
    }
  }

  // IDxcCompiler implementation
  HRESULT STDMETHODCALLTYPE
  Compile(IDxcBlob *Source, LPCWSTR SourceName, LPCWSTR EntryPoint,
          LPCWSTR TargetProfile, LPCWSTR *Arguments, UINT32 ArgCount,
          const DxcDefine *Defines, UINT32 DefineCount,
          IDxcIncludeHandler *IncludeHandler,
          IDxcOperationResult **ResultObject) override {
    if (ResultObject == nullptr)
      return E_INVALIDARG;

    DxcThreadMalloc TM(m_pMalloc);
    // initialize will update Arguments and ArgCount if needed.
    ExtValidationArgHelper Helper;
    IFR(Helper.initialize(Validator, &Arguments, &ArgCount, TargetProfile));

    CComPtr<IDxcOperationResult> CompileResult;
    IFR(cast<IDxcCompiler>()->Compile(
        Source, SourceName, EntryPoint, TargetProfile, Arguments, ArgCount,
        Defines, DefineCount, IncludeHandler, &CompileResult));
    HRESULT CompileHR;
    CompileResult->GetStatus(&CompileHR);
    if (SUCCEEDED(CompileHR))
      return Helper.doValidation(CompileResult, IID_PPV_ARGS(ResultObject));
    CompileResult->QueryInterface(ResultObject);
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE
  Preprocess(IDxcBlob *Source, LPCWSTR SourceName, LPCWSTR *Arguments,
             UINT32 ArgCount, const DxcDefine *Defines, UINT32 DefineCount,
             IDxcIncludeHandler *IncludeHandler,
             IDxcOperationResult **ResultObject) override {
    return cast<IDxcCompiler>()->Preprocess(Source, SourceName, Arguments,
                                            ArgCount, Defines, DefineCount,
                                            IncludeHandler, ResultObject);
  }

  HRESULT STDMETHODCALLTYPE
  Disassemble(IDxcBlob *Source, IDxcBlobEncoding **Disassembly) override {
    return cast<IDxcCompiler>()->Disassemble(Source, Disassembly);
  }

  // IDxcCompiler2 implementation
  HRESULT STDMETHODCALLTYPE CompileWithDebug(
      IDxcBlob *Source, LPCWSTR SourceName, LPCWSTR EntryPoint,
      LPCWSTR TargetProfile, LPCWSTR *Arguments, UINT32 ArgCount,
      const DxcDefine *pDefines, UINT32 DefineCount,
      IDxcIncludeHandler *IncludeHandler, IDxcOperationResult **ResultObject,
      LPWSTR *DebugBlobName, IDxcBlob **DebugBlob) override {
    if (ResultObject == nullptr)
      return E_INVALIDARG;

    DxcThreadMalloc TM(m_pMalloc);
    // ProcessArgs will update Arguments and ArgCount if needed.
    ExtValidationArgHelper Helper;

    IFR(Helper.initialize(Validator, &Arguments, &ArgCount, TargetProfile));

    CComPtr<IDxcOperationResult> CompileResult;
    IFR(cast<IDxcCompiler2>()->CompileWithDebug(
        Source, SourceName, EntryPoint, TargetProfile, Arguments, ArgCount,
        pDefines, DefineCount, IncludeHandler, &CompileResult, DebugBlobName,
        DebugBlob));

    return Helper.doValidation(CompileResult, IID_PPV_ARGS(ResultObject));
  }

  // IDxcCompiler3 implementation
  HRESULT STDMETHODCALLTYPE Compile(const DxcBuffer *Source, LPCWSTR *Arguments,
                                    UINT32 ArgCount,
                                    IDxcIncludeHandler *IncludeHandler,
                                    REFIID Riid,
                                    LPVOID *ResultObject) override {
    if (ResultObject == nullptr)
      return E_INVALIDARG;

    DxcThreadMalloc TM(m_pMalloc);
    // ProcessArgs will update Arguments and ArgCount if needed.
    ExtValidationArgHelper Helper;

    Helper.initialize(Validator, &Arguments, &ArgCount);

    CComPtr<IDxcResult> CompileResult;
    IFR(cast<IDxcCompiler3>()->Compile(Source, Arguments, ArgCount,
                                       IncludeHandler,
                                       IID_PPV_ARGS(&CompileResult)));

    return Helper.doValidation(CompileResult, Riid, ResultObject);
  }

  HRESULT STDMETHODCALLTYPE Disassemble(const DxcBuffer *Object, REFIID Riid,
                                        LPVOID *ResultObject) override {
    return cast<IDxcCompiler3>()->Disassemble(Object, Riid, ResultObject);
  }

  // IDxcVersionInfo implementation
  HRESULT STDMETHODCALLTYPE GetVersion(_Out_ UINT32 *pMajor,
                                       _Out_ UINT32 *pMinor) override {
    return cast<IDxcVersionInfo>()->GetVersion(pMajor, pMinor);
  }
  HRESULT STDMETHODCALLTYPE GetFlags(_Out_ UINT32 *pFlags) override {
    return cast<IDxcVersionInfo>()->GetFlags(pFlags);
  }

  // IDxcVersionInfo2 implementation
  HRESULT STDMETHODCALLTYPE
  GetCommitInfo(_Out_ UINT32 *pCommitCount,
                _Outptr_result_z_ char **pCommitHash) override {
    return cast<IDxcVersionInfo2>()->GetCommitInfo(pCommitCount, pCommitHash);
  }

  // IDxcVersionInfo3 implementation
  HRESULT STDMETHODCALLTYPE
  GetCustomVersionString(_Outptr_result_z_ char **pVersionString) override {
    return cast<IDxcVersionInfo3>()->GetCustomVersionString(pVersionString);
  }

private:
  CComPtr<IDxcValidator> Validator;

  // This wrapper wraps one particular compiler interface.
  // When QueryInterface is called, we create a new wrapper
  // for the requested interface, which wraps the result of QueryInterface
  // on the compiler object. Compiler pointer is held as IUnknown and must
  // be upcast to the appropriate interface on use.
  CComPtr<IUnknown> Compiler;
  DXC_MICROCOM_TM_REF_FIELDS()

  template <typename T> CComPtr<T> cast() const {
    CComPtr<T> Result;
    if (Compiler)
      Compiler.QueryInterface(&Result);
    assert(Result);
    return Result;
  }
};
} // namespace

namespace dxc {

static HRESULT createCompilerWrapper(IMalloc *Malloc,
                                     SpecificDllLoader &DxCompilerSupport,
                                     SpecificDllLoader &DxilExtValSupport,
                                     REFIID Riid, IUnknown **ResultObject) {
  DXASSERT(ResultObject, "Invalid ResultObject");
  *ResultObject = nullptr;

  CComPtr<IUnknown> Compiler;
  CComPtr<IDxcValidator> Validator;

  // Create compiler and validator
  if (Malloc) {
    IFR(DxCompilerSupport.CreateInstance2(Malloc, CLSID_DxcCompiler, Riid,
                                          &Compiler));
    IFR(DxilExtValSupport.CreateInstance2<IDxcValidator>(
        Malloc, CLSID_DxcValidator, &Validator));
  } else {
    IFR(DxCompilerSupport.CreateInstance(CLSID_DxcCompiler, Riid, &Compiler));
    IFR(DxilExtValSupport.CreateInstance<IDxcValidator>(CLSID_DxcValidator,
                                                        &Validator));
  }

  // Wrap compiler
  CComPtr<ExternalValidationCompiler> CompilerWrapper =
      ExternalValidationCompiler::Alloc(
          Malloc ? Malloc : DxcGetThreadMallocNoRef(), Validator, Compiler);
  return CompilerWrapper->QueryInterface(Riid, (void **)ResultObject);
}

HRESULT DxcDllExtValidationLoader::CreateInstanceImpl(REFCLSID Clsid,
                                                      REFIID Riid,
                                                      IUnknown **ResultObject) {
  if (!ResultObject)
    return E_POINTER;

  *ResultObject = nullptr;

  // If there is intent to use an external dxil.dll
  if (!DxilDllPath.empty() && !dxilDllFailedToLoad()) {
    if (Clsid == CLSID_DxcValidator) {
      return DxilExtValSupport.CreateInstance(Clsid, Riid, ResultObject);
    }
    if (Clsid == CLSID_DxcCompiler) {
      return createCompilerWrapper(nullptr, DxCompilerSupport,
                                   DxilExtValSupport, Riid, ResultObject);
    }
  }

  // Fallback: let DxCompiler handle it
  return DxCompilerSupport.CreateInstance(Clsid, Riid, ResultObject);
}

HRESULT DxcDllExtValidationLoader::CreateInstance2Impl(
    IMalloc *Malloc, REFCLSID Clsid, REFIID Riid, IUnknown **ResultObject) {
  if (!ResultObject)
    return E_POINTER;

  *ResultObject = nullptr;
  // If there is intent to use an external dxil.dll
  if (!DxilDllPath.empty() && !dxilDllFailedToLoad()) {
    if (Clsid == CLSID_DxcValidator) {
      return DxilExtValSupport.CreateInstance2(Malloc, Clsid, Riid,
                                               ResultObject);
    }
    if (Clsid == CLSID_DxcCompiler) {
      return createCompilerWrapper(Malloc, DxCompilerSupport, DxilExtValSupport,
                                   Riid, ResultObject);
    }
  }

  // Fallback: let DxCompiler handle it
  return DxCompilerSupport.CreateInstance2(Malloc, Clsid, Riid, ResultObject);
}

HRESULT
DxcDllExtValidationLoader::InitializeForDll(LPCSTR dllName, LPCSTR fnName) {
  // Load dxcompiler.dll
  HRESULT Result = DxCompilerSupport.InitializeForDll(dllName, fnName);
  // if dxcompiler.dll fails to load, return the failed HRESULT
  if (FAILED(Result)) {
    FailureReason = FailedCompilerLoad;
    return Result;
  }

  // now handle external dxil.dll
  const char *EnvVarVal = std::getenv("DXC_DXIL_DLL_PATH");
  if (!EnvVarVal || std::string(EnvVarVal).empty()) {
    // no need to emit anything if external validation isn't wanted
    return S_OK;
  }

  DxilDllPath = std::string(EnvVarVal);
  std::filesystem::path DllPath(DxilDllPath);

  // Check if path is absolute and exists
  if (!DllPath.is_absolute() || !std::filesystem::exists(DllPath)) {
    FailureReason = FailedDxilPath;
    return E_INVALIDARG;
  }

  Result = DxilExtValSupport.InitializeForDll(DxilDllPath.c_str(),
                                              "DxcCreateInstance");
  if (FAILED(Result)) {
    FailureReason = FailedDxilLoad;
    return Result;
  }

  return Result;
}

HRESULT DxcDllExtValidationLoader::initialize() {
  return InitializeForDll(kDxCompilerLib, "DxcCreateInstance");
}

} // namespace dxc
