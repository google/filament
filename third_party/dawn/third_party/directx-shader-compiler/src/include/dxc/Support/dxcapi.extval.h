#include "dxc/Support/dxcapi.use.h"
#include "dxc/WinAdapter.h"
#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <string>

namespace dxc {

class DxcDllExtValidationLoader : public DllLoader {
  // DxCompilerSupport manages the
  // lifetime of dxcompiler.dll, while DxilExtValSupport
  // manages the lifetime of dxil.dll
  dxc::SpecificDllLoader DxCompilerSupport;
  dxc::SpecificDllLoader DxilExtValSupport;
  std::string DxilDllPath;

public:
  std::string getDxilDllPath() { return DxilDllPath; }
  enum InitializationFailures {
    FailedNone = 0,
    FailedCompilerLoad,
    FailedDxilPath,
    FailedDxilLoad,
  } FailureReason = FailedNone;
  InitializationFailures getFailureReason() { return FailureReason; }
  bool dxilDllFailedToLoad() {
    return !DxilDllPath.empty() && !DxilExtValSupport.IsEnabled();
  }

  HRESULT CreateInstanceImpl(REFCLSID clsid, REFIID riid,
                             IUnknown **pResult) override;
  HRESULT CreateInstance2Impl(IMalloc *pMalloc, REFCLSID clsid, REFIID riid,
                              IUnknown **pResult) override;

  HRESULT initialize();
  HRESULT InitializeForDll(LPCSTR dllName, LPCSTR fnName);

  bool IsEnabled() const override { return DxCompilerSupport.IsEnabled(); }
};
} // namespace dxc
