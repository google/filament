//===- unittests/HLSL/HLSLTestOptions.cpp ----- Test Options Init -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines and initializes command line options that can be passed to
// HLSL gtests.
//
//===----------------------------------------------------------------------===//

#include "HLSLTestOptions.h"
#include "dxc/Test/WEXAdapter.h"
#include "dxc/WinAdapter.h"

namespace clang {
namespace hlsl {
namespace testOptions {

#define ARG_DEFINE(argname) std::string argname = "";
ARG_LIST(ARG_DEFINE)

} // namespace testOptions
} // namespace hlsl
} // namespace clang

namespace WEX {
namespace TestExecution {
namespace RuntimeParameters {
HRESULT TryGetValue(const wchar_t *param, WEX::Common::String &retStr) {
#define RETURN_ARG(argname)                                                    \
  if (wcscmp(param, L## #argname) == 0) {                                      \
    if (!clang::hlsl::testOptions::argname.empty()) {                          \
      retStr.assign(CA2W(clang::hlsl::testOptions::argname.c_str()).m_psz);    \
      return S_OK;                                                             \
    } else                                                                     \
      return E_FAIL;                                                           \
  }
  ARG_LIST(RETURN_ARG)
  return E_NOTIMPL;
}
} // namespace RuntimeParameters
} // namespace TestExecution
} // namespace WEX
