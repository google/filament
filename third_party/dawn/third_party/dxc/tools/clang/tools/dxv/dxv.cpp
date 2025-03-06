///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxv.cpp                                                                   //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides the entry point for the dxv console program.                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/WinIncludes.h"

#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/dxcapi.use.h"
#include "dxc/dxcapi.h"
#include "dxc/dxcapi.internal.h"

#include "llvm/Support//MSFileSystem.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"

using namespace dxc;
using namespace llvm;
using namespace llvm::opt;
using namespace hlsl::options;

static cl::opt<bool> Help("help", cl::desc("Print help"));
static cl::alias Help_h("h", cl::aliasopt(Help));
static cl::alias Help_q("?", cl::aliasopt(Help));

static cl::opt<std::string> InputFilename(cl::Positional,
                                          cl::desc("<input dxil file>"));

static cl::opt<std::string>
    OutputFilename("o",
                   cl::desc("Override output filename for signed container"),
                   cl::value_desc("filename"));

class DxvContext {
private:
  DxcDllSupport &m_dxcSupport;

public:
  DxvContext(DxcDllSupport &dxcSupport) : m_dxcSupport(dxcSupport) {}

  void Validate();
};

void DxvContext::Validate() {
  {
    CComPtr<IDxcBlobEncoding> pSource;
    ReadFileIntoBlob(m_dxcSupport, StringRefWide(InputFilename), &pSource);

    bool bSourceIsDxilContainer = hlsl::IsValidDxilContainer(
        hlsl::IsDxilContainerLike(pSource->GetBufferPointer(),
                                  pSource->GetBufferSize()),
        pSource->GetBufferSize());

    hlsl::DxilContainerHash origHash = {};
    CComPtr<IDxcBlob> pContainerBlob;
    if (bSourceIsDxilContainer) {
      pContainerBlob = pSource;
      const hlsl::DxilContainerHeader *pHeader = hlsl::IsDxilContainerLike(
          pSource->GetBufferPointer(), pSource->GetBufferSize());
      // Copy hash into origHash
      memcpy(&origHash, &pHeader->Hash, sizeof(hlsl::DxilContainerHash));
    } else {
      // Otherwise assume assembly to container is required.
      CComPtr<IDxcAssembler> pAssembler;
      CComPtr<IDxcOperationResult> pAsmResult;

      HRESULT resultStatus;

      IFT(m_dxcSupport.CreateInstance(CLSID_DxcAssembler, &pAssembler));
      IFT(pAssembler->AssembleToContainer(pSource, &pAsmResult));
      IFT(pAsmResult->GetStatus(&resultStatus));
      if (FAILED(resultStatus)) {
        CComPtr<IDxcBlobEncoding> text;
        IFT(pAsmResult->GetErrorBuffer(&text));
        const char *pStart = (const char *)text->GetBufferPointer();
        std::string msg(pStart);
        IFTMSG(resultStatus, msg);
        return;
      }
      IFT(pAsmResult->GetResult(&pContainerBlob));
    }

    CComPtr<IDxcValidator> pValidator;
    CComPtr<IDxcOperationResult> pResult;

    IFT(m_dxcSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
    IFT(pValidator->Validate(pContainerBlob, DxcValidatorFlags_InPlaceEdit,
                             &pResult));

    HRESULT status;
    IFT(pResult->GetStatus(&status));

    if (FAILED(status)) {
      CComPtr<IDxcBlobEncoding> text;
      IFT(pResult->GetErrorBuffer(&text));
      const char *pStart = (const char *)text->GetBufferPointer();
      std::string msg(pStart);
      IFTMSG(status, msg);
    } else {
      // Source was unsigned DxilContainer, write signed container if it's now
      // signed:
      if (!OutputFilename.empty()) {
        if (bSourceIsDxilContainer) {
          const hlsl::DxilContainerHeader *pHeader = hlsl::IsDxilContainerLike(
              pSource->GetBufferPointer(), pSource->GetBufferSize());
          WriteBlobToFile(pSource, StringRefWide(OutputFilename), CP_ACP);
          if (memcmp(&pHeader->Hash, &origHash,
                     sizeof(hlsl::DxilContainerHash)) != 0) {
            printf("Signed DxilContainer written to \"%s\"\n",
                   OutputFilename.c_str());
          } else {
            printf("Unchanged DxilContainer written to \"%s\"\n",
                   OutputFilename.c_str());
          }
        } else {
          printf("Source was not a DxilContainer, no output file written.\n");
        }
      }
      printf("Validation succeeded.");
    }
  }
}

#ifdef _WIN32
int __cdecl main(int argc, const char **argv) {
#else
int main(int argc, const char **argv) {
#endif
  const char *pStage = "Operation";
  if (llvm::sys::fs::SetupPerThreadFileSystem())
    return 1;
  llvm::sys::fs::AutoCleanupPerThreadFileSystem auto_cleanup_fs;
  if (FAILED(DxcInitThreadMalloc()))
    return 1;
  DxcSetThreadMallocToDefault();
  try {
    llvm::sys::fs::MSFileSystem *msfPtr;
    IFT(CreateMSFileSystemForDisk(&msfPtr));
    std::unique_ptr<::llvm::sys::fs::MSFileSystem> msf(msfPtr);

    ::llvm::sys::fs::AutoPerThreadSystem pts(msf.get());
    IFTLLVM(pts.error_code());

    pStage = "Argument processing";

    // Parse command line options.
    cl::ParseCommandLineOptions(argc, argv, "dxil validator\n");

    if (InputFilename == "" || Help) {
      cl::PrintHelpMessage();
      return 2;
    }

    DxcDllSupport dxcSupport;
    dxc::EnsureEnabled(dxcSupport);

    DxvContext context(dxcSupport);
    pStage = "Validation";
    context.Validate();
  } catch (const ::hlsl::Exception &hlslException) {
    try {
      const char *msg = hlslException.what();
      Unicode::acp_char printBuffer[128]; // printBuffer is safe to treat as
                                          // UTF-8 because we use ASCII only
                                          // errors only
      if (msg == nullptr || *msg == '\0') {
        sprintf_s(printBuffer, _countof(printBuffer),
                  "Validation failed - error code 0x%08x.", hlslException.hr);
        msg = printBuffer;
      }
      printf("%s\n", msg);
    } catch (...) {
      printf("%s failed - unable to retrieve error message.\n", pStage);
    }

    return 1;
  } catch (std::bad_alloc &) {
    printf("%s failed - out of memory.\n", pStage);
    return 1;
  } catch (...) {
    printf("%s failed - unknown error.\n", pStage);
    return 1;
  }

  return 0;
}
