///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxc.cpp                                                                   //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides the entry point for the dxc console program.                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
// Some unimplemented flags as compared to fxc:
//
// /compress    - Compress DX10 shader bytecode from files.
// /decompress  - Decompress DX10 shader bytecode from first file.
// /Fx <file>   - Output assembly code and hex listing file.
// /Fl <file>   - Output a library.
// /Gch         - Compile as a child effect for fx_4_x profiles.
// /Gdp         - Disable effect performance mode.
// /Gec         - Enable backwards compatibility mode.
// /Ges         - Enable strict mode.
// /Gpp         - Force partial precision.
// /Lx          - Output hexadecimal literals
// /Op          - Disable preshaders
//
// Unimplemented but on roadmap:
//
// /matchUAVs   - Match template shader UAV slot allocations in the current
// shader /mergeUAVs   - Merge UAV slot allocations of template shader and the
// current shader /Ni          - Output instruction numbers in assembly listings
// /No          - Output instruction byte offset in assembly listings
// /Qstrip_reflect
// /res_may_alias
// /shtemplate
// /verifyrootsignature
//

#include "dxc.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/WinFunctions.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/dxcerrors.h"
#include <sstream>
#include <string>
#include <vector>

#include "dxc/DXIL/DxilShaderModel.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/DxilRootSignature/DxilRootSignature.h"
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/dxcapi.use.h"
#include "dxc/Support/microcom.h"
#include "dxc/dxcapi.h"
#include "dxc/dxcapi.internal.h"
#include "dxc/dxctools.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#ifdef _WIN32
#include <comdef.h>
#include <dia2.h>
#endif
#include <algorithm>
#include <unordered_map>

#ifdef _WIN32
#pragma comment(lib, "version.lib")
#endif

// SPIRV Change Starts
#ifdef ENABLE_SPIRV_CODEGEN
#include "spirv-tools/libspirv.hpp"
#include "clang/SPIRV/FeatureManager.h"

static bool DisassembleSpirv(IDxcBlob *binaryBlob, IDxcLibrary *library,
                             IDxcBlobEncoding **assemblyBlob, bool withColor,
                             bool withByteOffset, spv_target_env target_env) {
  if (!binaryBlob)
    return true;

  size_t num32BitWords = (binaryBlob->GetBufferSize() + 3) / 4;
  std::string binaryStr((char *)binaryBlob->GetBufferPointer(),
                        binaryBlob->GetBufferSize());
  binaryStr.resize(num32BitWords * 4, 0);

  std::vector<uint32_t> words;
  words.resize(num32BitWords, 0);
  memcpy(words.data(), binaryStr.data(), binaryStr.size());

  std::string assembly;
  spvtools::SpirvTools spirvTools(target_env);
  uint32_t options = (SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES |
                      SPV_BINARY_TO_TEXT_OPTION_INDENT);
  if (withColor)
    options |= SPV_BINARY_TO_TEXT_OPTION_COLOR;
  if (withByteOffset)
    options |= SPV_BINARY_TO_TEXT_OPTION_SHOW_BYTE_OFFSET;

  if (!spirvTools.Disassemble(words, &assembly, options))
    return false;

  IFT(library->CreateBlobWithEncodingOnHeapCopy(
      assembly.data(), assembly.size(), CP_UTF8, assemblyBlob));

  return true;
}
#endif
// SPIRV Change Ends

inline bool wcseq(LPCWSTR a, LPCWSTR b) {
  return (a == nullptr && b == nullptr) ||
         (a != nullptr && b != nullptr && wcscmp(a, b) == 0);
}

using namespace dxc;
using namespace llvm::opt;
using namespace hlsl::options;

class DxcContext {

private:
  DxcOpts &m_Opts;
  DxcDllSupport &m_dxcSupport;

  int ActOnBlob(IDxcBlob *pBlob);
  int ActOnBlob(IDxcBlob *pBlob, IDxcBlob *pDebugBlob, LPCWSTR pDebugBlobName);
  void UpdatePart(IDxcBlob *pBlob, IDxcBlob **ppResult);
  bool UpdatePartRequired();
  void WriteHeader(IDxcBlobEncoding *pDisassembly, IDxcBlob *pCode,
                   llvm::Twine &pVariableName, LPCWSTR pPath);
  HRESULT ReadFileIntoPartContent(hlsl::DxilFourCC fourCC, LPCWSTR fileName,
                                  IDxcBlob **ppResult);

// Dia is only supported on Windows.
#ifdef _WIN32
  // TODO : Refactor two functions below. There are duplicate functions in
  // DxcContext in dxa.cpp
  HRESULT GetDxcDiaTable(IDxcLibrary *pLibrary, IDxcBlob *pTargetBlob,
                         IDiaTable **ppTable, LPCWSTR tableName);
#endif // _WIN32

  HRESULT FindModuleBlob(hlsl::DxilFourCC fourCC, IDxcBlob *pSource,
                         IDxcLibrary *pLibrary, IDxcBlob **ppTargetBlob);
  void ExtractRootSignature(IDxcBlob *pBlob, IDxcBlob **ppResult);
  int VerifyRootSignature();

  template <typename TInterface>
  HRESULT CreateInstance(REFCLSID clsid, TInterface **pResult) {
    return m_dxcSupport.CreateInstance(clsid, pResult);
  }

public:
  DxcContext(DxcOpts &Opts, DxcDllSupport &dxcSupport)
      : m_Opts(Opts), m_dxcSupport(dxcSupport) {}

  int Compile();
  void Recompile(IDxcBlob *pSource, IDxcLibrary *pLibrary,
                 IDxcCompiler *pCompiler, std::vector<LPCWSTR> &args,
                 std::wstring &outputPDBPath, CComPtr<IDxcBlob> &pDebugBlob,
                 IDxcOperationResult **pCompileResult);
  int DumpBinary();
  int Link();
  void Preprocess();
  void GetCompilerVersionInfo(llvm::raw_string_ostream &OS);
};

static void WriteBlobToFile(IDxcBlob *pBlob, llvm::StringRef FName,
                            UINT32 defaultTextCodePage) {
  ::dxc::WriteBlobToFile(pBlob, StringRefWide(FName), defaultTextCodePage);
}

static void WritePartToFile(IDxcBlob *pBlob, hlsl::DxilFourCC CC,
                            llvm::StringRef FName) {
  const hlsl::DxilContainerHeader *pContainer = hlsl::IsDxilContainerLike(
      pBlob->GetBufferPointer(), pBlob->GetBufferSize());
  if (!pContainer) {
    throw hlsl::Exception(E_FAIL, "Unable to find required part in blob");
  }
  hlsl::DxilPartIsType pred(CC);
  hlsl::DxilPartIterator it =
      std::find_if(hlsl::begin(pContainer), hlsl::end(pContainer), pred);
  if (it == hlsl::end(pContainer)) {
    throw hlsl::Exception(E_FAIL, "Unable to find required part in blob");
  }

  const char *pData = hlsl::GetDxilPartData(*it);
  DWORD dataLen = (*it)->PartSize;
  StringRefWide WideName(FName);
  CHandle file(CreateFileW(WideName, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr));
  if (file == INVALID_HANDLE_VALUE) {
    IFT_Data(HRESULT_FROM_WIN32(GetLastError()), WideName);
  }
  DWORD written;
  if (FALSE == WriteFile(file, pData, dataLen, &written, nullptr)) {
    IFT_Data(HRESULT_FROM_WIN32(GetLastError()), WideName);
  }
}

static void WriteDxcOutputToFile(DXC_OUT_KIND kind, IDxcResult *pResult,
                                 UINT32 textCodePage) {
  if (pResult->HasOutput(kind)) {
    CComPtr<IDxcBlob> pData;
    CComPtr<IDxcBlobWide> pName;
    IFT(pResult->GetOutput(kind, IID_PPV_ARGS(&pData), &pName));
    if (pName && pName->GetStringLength() > 0)
      WriteBlobToFile(pData, pName->GetStringPointer(), textCodePage);
  }
}

static bool StringBlobEqualWide(IDxcBlobWide *pBlob, const WCHAR *pStr) {
  size_t uSize = wcslen(pStr);
  if (pBlob && pBlob->GetStringLength() == uSize) {
    return 0 == memcmp(pBlob->GetBufferPointer(), pStr, pBlob->GetBufferSize());
  }
  return false;
}

static void WriteDxcExtraOuputs(IDxcResult *pResult) {
  DXC_OUT_KIND kind = DXC_OUT_EXTRA_OUTPUTS;
  if (!pResult->HasOutput(kind)) {
    return;
  }

  CComPtr<IDxcExtraOutputs> pOutputs;
  CComPtr<IDxcBlobWide> pName;
  IFT(pResult->GetOutput(kind, IID_PPV_ARGS(&pOutputs), &pName));

  UINT32 uOutputCount = pOutputs->GetOutputCount();
  for (UINT32 i = 0; i < uOutputCount; i++) {
    CComPtr<IDxcBlobWide> pFileName;
    CComPtr<IDxcBlobWide> pType;
    CComPtr<IDxcBlob> pBlob;
    HRESULT hr =
        pOutputs->GetOutput(i, IID_PPV_ARGS(&pBlob), &pType, &pFileName);

    // Not a blob
    if (FAILED(hr))
      continue;

    UINT32 uCodePage = CP_ACP;
    CComPtr<IDxcBlobEncoding> pBlobEncoding;
    if (SUCCEEDED(pBlob.QueryInterface(&pBlobEncoding))) {
      BOOL bKnown = FALSE;
      UINT32 uKnownCodePage = CP_ACP;
      IFT(pBlobEncoding->GetEncoding(&bKnown, &uKnownCodePage));
      if (bKnown) {
        uCodePage = uKnownCodePage;
      }
    }

    if (pFileName && pFileName->GetStringLength() > 0) {
      if (StringBlobEqualWide(pFileName, DXC_EXTRA_OUTPUT_NAME_STDOUT)) {
        if (uCodePage != CP_ACP) {
          WriteBlobToConsole(pBlob, STD_OUTPUT_HANDLE);
        }
      } else if (StringBlobEqualWide(pFileName, DXC_EXTRA_OUTPUT_NAME_STDERR)) {
        if (uCodePage != CP_ACP) {
          WriteBlobToConsole(pBlob, STD_ERROR_HANDLE);
        }
      } else {
        WriteBlobToFile(pBlob, pFileName->GetStringPointer(), uCodePage);
      }
    }
  }
}

static void WriteDxcOutputToConsole(IDxcResult *pResult, DXC_OUT_KIND kind) {
  if (!pResult->HasOutput(kind))
    return;

  CComPtr<IDxcBlob> pBlob;
  IFT(pResult->GetOutput(kind, IID_PPV_ARGS(&pBlob), nullptr));
  llvm::StringRef outputString((LPSTR)pBlob->GetBufferPointer(),
                               pBlob->GetBufferSize());
  llvm::SmallVector<llvm::StringRef, 20> lines;
  outputString.split(lines, "\n");

  std::string outputStr;
  llvm::raw_string_ostream SS(outputStr);
  for (auto line : lines) {
    SS << "; " << line << "\n";
  }

  WriteUtf8ToConsole(outputStr.data(), outputStr.size());
}

std::string getDependencyOutputFileName(llvm::StringRef inputFileName) {
  return inputFileName.substr(0, inputFileName.rfind('.')).str() + ".d";
}

// This function is called either after the compilation is done or /dumpbin
// option is provided Performing options that are used to process dxil
// container.
int DxcContext::ActOnBlob(IDxcBlob *pBlob) {
  return ActOnBlob(pBlob, nullptr, nullptr);
}

int DxcContext::ActOnBlob(IDxcBlob *pBlob, IDxcBlob *pDebugBlob,
                          LPCWSTR pDebugBlobName) {
  int retVal = 0;
  if (m_Opts.DumpDependencies) {
    if (!m_Opts.OutputFileForDependencies.empty()) {
      CComPtr<IDxcBlob> pResult;
      UpdatePart(pBlob, &pResult);
      WriteBlobToFile(pResult, m_Opts.OutputFileForDependencies,
                      m_Opts.DefaultTextCodePage);
    } else if (m_Opts.WriteDependencies) {
      CComPtr<IDxcBlob> pResult;
      UpdatePart(pBlob, &pResult);
      WriteBlobToFile(pResult, getDependencyOutputFileName(m_Opts.InputFile),
                      m_Opts.DefaultTextCodePage);
    } else {
      WriteBlobToConsole(pBlob);
    }
    return retVal;
  }

  // Text output.
  if (m_Opts.AstDump || m_Opts.OptDump || m_Opts.VerifyDiagnostics) {
    WriteBlobToConsole(pBlob);
    return retVal;
  }

  // Write the output blob.
  if (!m_Opts.OutputObject.empty()) {
    // For backward compatability: fxc requires /Fo for /extractrootsignature
    if (!m_Opts.ExtractRootSignature) {
      CComPtr<IDxcBlob> pResult;
      UpdatePart(pBlob, &pResult);
      WriteBlobToFile(pResult, m_Opts.OutputObject, m_Opts.DefaultTextCodePage);
    }
  }

  // Verify Root Signature
  if (!m_Opts.VerifyRootSignatureSource.empty()) {
    return VerifyRootSignature();
  }

  // Extract and write the PDB/debug information.
  if (!m_Opts.DebugFile.empty()) {
    IFTBOOLMSG(m_Opts.GeneratePDB(), E_INVALIDARG,
               "/Fd specified, but no Debug Info was "
               "found in the shader, please use the "
               "/Zi or /Zs switch to generate debug "
               "information compiling this shader.");
    if (pDebugBlob != nullptr) {
      IFTBOOLMSG(pDebugBlobName && *pDebugBlobName, E_INVALIDARG,
                 "/Fd was specified but no debug name was produced");
      WriteBlobToFile(pDebugBlob, pDebugBlobName, m_Opts.DefaultTextCodePage);
    } else {
      // Note: This is for load from binary case
      WritePartToFile(pBlob, hlsl::DFCC_ShaderDebugInfoDXIL, m_Opts.DebugFile);
    }
  }

  // Extract and write root signature information.
  if (m_Opts.ExtractRootSignature) {
    CComPtr<IDxcBlob> pRootSignatureContainer;
    ExtractRootSignature(pBlob, &pRootSignatureContainer);
    WriteBlobToFile(pRootSignatureContainer, m_Opts.OutputObject,
                    m_Opts.DefaultTextCodePage);
  }

  // Extract and write private data.
  if (!m_Opts.ExtractPrivateFile.empty()) {
    WritePartToFile(pBlob, hlsl::DFCC_PrivateData, m_Opts.ExtractPrivateFile);
  }

  // OutputObject suppresses console dump.
  bool needDisassembly =
      !m_Opts.OutputHeader.empty() || !m_Opts.AssemblyCode.empty() ||
      (m_Opts.OutputObject.empty() && m_Opts.DebugFile.empty() &&
       m_Opts.ExtractPrivateFile.empty() &&
       m_Opts.VerifyRootSignatureSource.empty() &&
       !m_Opts.ExtractRootSignature);

  if (!needDisassembly)
    return retVal;

  CComPtr<IDxcBlobEncoding> pDisassembleResult;

  // SPIRV Change Starts
#ifdef ENABLE_SPIRV_CODEGEN
  if (m_Opts.GenSPIRV) {
    CComPtr<IDxcLibrary> pLibrary;
    IFT(m_dxcSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
    llvm::Optional<spv_target_env> target_env =
        clang::spirv::FeatureManager::stringToSpvEnvironment(
            m_Opts.SpirvOptions.targetEnv);
    IFTBOOLMSG(target_env, E_INVALIDARG, "Cannot parse SPIR-V target env.");
    IFTBOOLMSG(DisassembleSpirv(pBlob, pLibrary, &pDisassembleResult,
                                m_Opts.ColorCodeAssembly,
                                m_Opts.DisassembleByteOffset, *target_env),
               E_FAIL,
               "dxc failed : Internal Compiler Error - "
               "unable to disassemble generated SPIR-V.");
  } else {
#endif // ENABLE_SPIRV_CODEGEN
    // SPIRV Change Ends

    if (m_Opts.IsRootSignatureProfile()) {
      // keep the same behavior as fxc, people may want to embed the root
      // signatures in their code bases.
      CComPtr<IDxcLibrary> pLibrary;
      IFT(m_dxcSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
      std::string Message = "Disassembly failed";
      IFT(pLibrary->CreateBlobWithEncodingOnHeapCopy(
          (LPBYTE)&Message[0], Message.size(), CP_ACP, &pDisassembleResult));
    } else {
      CComPtr<IDxcCompiler> pCompiler;
      IFT(CreateInstance(CLSID_DxcCompiler, &pCompiler));
      IFT(pCompiler->Disassemble(pBlob, &pDisassembleResult));
    }

    // SPIRV Change Starts
#ifdef ENABLE_SPIRV_CODEGEN
  }
#endif // ENABLE_SPIRV_CODEGEN
  // SPIRV Change Ends

  bool disassemblyWritten = false;
  if (!m_Opts.OutputHeader.empty()) {
    llvm::Twine varName = m_Opts.VariableName.empty()
                              ? llvm::Twine("g_", m_Opts.EntryPoint)
                              : m_Opts.VariableName;
    WriteHeader(pDisassembleResult, pBlob, varName,
                StringRefWide(m_Opts.OutputHeader));
    disassemblyWritten = true;
  }

  if (!m_Opts.AssemblyCode.empty()) {
    WriteBlobToFile(pDisassembleResult, m_Opts.AssemblyCode,
                    m_Opts.DefaultTextCodePage);
    disassemblyWritten = true;
  }

  if (!disassemblyWritten) {
    WriteBlobToConsole(pDisassembleResult);
  }
  return retVal;
}

// Given a dxil container, update the dxil container by processing container
// specific options.
void DxcContext::UpdatePart(IDxcBlob *pSource, IDxcBlob **ppResult) {
  DXASSERT(pSource && ppResult, "otherwise blob cannot be updated");
  if (!UpdatePartRequired()) {
    *ppResult = pSource;
    pSource->AddRef();
    return;
  }

  CComPtr<IDxcContainerBuilder> pContainerBuilder;
  CComPtr<IDxcBlob> pResult;
  IFT(CreateInstance(CLSID_DxcContainerBuilder, &pContainerBuilder));

  // Load original container and update blob for each given option
  IFT(pContainerBuilder->Load(pSource));

  // Update parts based on dxc options
  if (m_Opts.StripDebug) {
    IFT(pContainerBuilder->RemovePart(
        hlsl::DxilFourCC::DFCC_ShaderDebugInfoDXIL));
  }
  if (m_Opts.StripPrivate) {
    IFT(pContainerBuilder->RemovePart(hlsl::DxilFourCC::DFCC_PrivateData));
  }
  if (m_Opts.StripRootSignature) {
    IFT(pContainerBuilder->RemovePart(hlsl::DxilFourCC::DFCC_RootSignature));
  }
  if (!m_Opts.PrivateSource.empty()) {
    CComPtr<IDxcBlob> privateBlob;
    IFT(ReadFileIntoPartContent(hlsl::DxilFourCC::DFCC_PrivateData,
                                StringRefWide(m_Opts.PrivateSource),
                                &privateBlob));

    // setprivate option can replace existing private part.
    // Try removing the private data if exists
    pContainerBuilder->RemovePart(hlsl::DxilFourCC::DFCC_PrivateData);
    IFT(pContainerBuilder->AddPart(hlsl::DxilFourCC::DFCC_PrivateData,
                                   privateBlob));
  }
  if (!m_Opts.RootSignatureSource.empty()) {
    // set rootsignature assumes that the given input is a dxil container.
    // We only want to add RTS0 part to the container builder.
    CComPtr<IDxcBlob> RootSignatureBlob;
    IFT(ReadFileIntoPartContent(hlsl::DxilFourCC::DFCC_RootSignature,
                                StringRefWide(m_Opts.RootSignatureSource),
                                &RootSignatureBlob));

    // setrootsignature option can replace existing rootsignature part
    // Try removing rootsignature if exists
    pContainerBuilder->RemovePart(hlsl::DxilFourCC::DFCC_RootSignature);
    IFT(pContainerBuilder->AddPart(hlsl::DxilFourCC::DFCC_RootSignature,
                                   RootSignatureBlob));
  }

  // Get the final blob from container builder
  CComPtr<IDxcOperationResult> pBuilderResult;
  IFT(pContainerBuilder->SerializeContainer(&pBuilderResult));
  if (!m_Opts.OutputWarningsFile.empty()) {
    CComPtr<IDxcBlobEncoding> pErrors;
    IFT(pBuilderResult->GetErrorBuffer(&pErrors));
    if (pErrors != nullptr) {
      WriteBlobToFile(pErrors, m_Opts.OutputWarningsFile,
                      m_Opts.DefaultTextCodePage);
    }
  } else {
    WriteOperationErrorsToConsole(pBuilderResult, m_Opts.OutputWarnings);
  }
  HRESULT status;
  IFT(pBuilderResult->GetStatus(&status));
  IFT(status);
  IFT(pBuilderResult->GetResult(ppResult));
}

bool DxcContext::UpdatePartRequired() {
  return (m_Opts.StripDebug || m_Opts.StripPrivate ||
          m_Opts.StripRootSignature || !m_Opts.PrivateSource.empty() ||
          !m_Opts.RootSignatureSource.empty()) &&
         (m_Opts.Link || m_Opts.DumpBin || !m_Opts.Preprocess.empty());
}

// This function reads the file from input file and constructs a blob with
// fourCC parts Used for setprivate and setrootsignature option
HRESULT DxcContext::ReadFileIntoPartContent(hlsl::DxilFourCC fourCC,
                                            LPCWSTR fileName,
                                            IDxcBlob **ppResult) {
  DXASSERT(fourCC == hlsl::DxilFourCC::DFCC_PrivateData ||
               fourCC == hlsl::DxilFourCC::DFCC_RootSignature,
           "Otherwise we provided wrong part to read for updating part.");

  // Read result, if it's private data, then return the blob
  if (fourCC == hlsl::DxilFourCC::DFCC_PrivateData) {
    CComPtr<IDxcBlobEncoding> pResult;
    ReadFileIntoBlob(m_dxcSupport, fileName, &pResult);
    *ppResult = pResult.Detach();
  }

  // If root signature, check if it's a dxil container that contains
  // rootsignature part, then construct a blob of root signature part
  if (fourCC == hlsl::DxilFourCC::DFCC_RootSignature) {
    CComPtr<IDxcBlob> pResult;
    CComHeapPtr<BYTE> pData;
    DWORD dataSize;
    IFT(hlsl::ReadBinaryFile(fileName, (void **)&pData, &dataSize));
    DXASSERT(pData != nullptr,
             "otherwise ReadBinaryFile should throw an exception");
    hlsl::DxilContainerHeader *pHeader =
        hlsl::IsDxilContainerLike(pData.m_pData, dataSize);
    IFRBOOL(hlsl::IsValidDxilContainer(pHeader, dataSize), E_INVALIDARG);
    hlsl::DxilPartHeader *pPartHeader =
        hlsl::GetDxilPartByType(pHeader, hlsl::DxilFourCC::DFCC_RootSignature);
    IFRBOOL(pPartHeader != nullptr, E_INVALIDARG);
    hlsl::DxcCreateBlobOnHeapCopy(hlsl::GetDxilPartData(pPartHeader),
                                  pPartHeader->PartSize, &pResult);
    *ppResult = pResult.Detach();
  }
  return S_OK;
}

// Constructs a dxil container builder with only root signature part.
// Right now IDxcContainerBuilder assumes that we are building a full dxil
// container, but we are building a container with only rootsignature part
void DxcContext::ExtractRootSignature(IDxcBlob *pBlob, IDxcBlob **ppResult) {
  DXASSERT_NOMSG(pBlob != nullptr && ppResult != nullptr);
  const hlsl::DxilContainerHeader *pHeader =
      (hlsl::DxilContainerHeader *)(pBlob->GetBufferPointer());
  IFTBOOL(hlsl::IsValidDxilContainer(pHeader, pHeader->ContainerSizeInBytes),
          DXC_E_CONTAINER_INVALID);
  const hlsl::DxilPartHeader *pPartHeader =
      hlsl::GetDxilPartByType(pHeader, hlsl::DxilFourCC::DFCC_RootSignature);
  IFTBOOL(pPartHeader != nullptr, DXC_E_MISSING_PART);

  // Get new header and allocate memory for new container
  hlsl::DxilContainerHeader newHeader;
  uint32_t containerSize =
      hlsl::GetDxilContainerSizeFromParts(1, pPartHeader->PartSize);
  hlsl::InitDxilContainer(&newHeader, 1, containerSize);
  CComPtr<hlsl::AbstractMemoryStream> pMemoryStream;
  IFT(hlsl::CreateMemoryStream(DxcGetThreadMallocNoRef(), &pMemoryStream));
  ULONG cbWritten;

  // Write Container Header
  IFT(pMemoryStream->Write(&newHeader, sizeof(hlsl::DxilContainerHeader),
                           &cbWritten));
  IFTBOOL(cbWritten == sizeof(hlsl::DxilContainerHeader), E_OUTOFMEMORY);

  // Write Part Offset
  uint32_t offset =
      sizeof(hlsl::DxilContainerHeader) + hlsl::GetOffsetTableSize(1);
  IFT(pMemoryStream->Write(&offset, sizeof(uint32_t), &cbWritten));
  IFTBOOL(cbWritten == sizeof(uint32_t), E_OUTOFMEMORY);

  // Write Root Signature Header
  IFT(pMemoryStream->Write(pPartHeader, sizeof(hlsl::DxilPartHeader),
                           &cbWritten));
  IFTBOOL(cbWritten == sizeof(hlsl::DxilPartHeader), E_OUTOFMEMORY);
  const char *partContent = hlsl::GetDxilPartData(pPartHeader);

  // Write Root Signature Content
  IFT(pMemoryStream->Write(partContent, pPartHeader->PartSize, &cbWritten));
  IFTBOOL(cbWritten == pPartHeader->PartSize, E_OUTOFMEMORY);

  // Return Result
  CComPtr<IDxcBlob> pResult;
  IFT(pMemoryStream->QueryInterface(&pResult));
  *ppResult = pResult.Detach();
}

int DxcContext::VerifyRootSignature() {
  // Get dxil container from file
  CComPtr<IDxcBlobEncoding> pSource;
  ReadFileIntoBlob(m_dxcSupport, StringRefWide(m_Opts.InputFile), &pSource);
  hlsl::DxilContainerHeader *pSourceHeader =
      (hlsl::DxilContainerHeader *)pSource->GetBufferPointer();
  IFTBOOLMSG(hlsl::IsValidDxilContainer(pSourceHeader,
                                        pSourceHeader->ContainerSizeInBytes),
             E_INVALIDARG, "invalid DXIL container to verify.");

  // Get rootsignature from file
  CComPtr<IDxcBlob> pRootSignature;

  IFTMSG(ReadFileIntoPartContent(
             hlsl::DxilFourCC::DFCC_RootSignature,
             StringRefWide(m_Opts.VerifyRootSignatureSource), &pRootSignature),
         "invalid root signature to verify.");

  // TODO : Right now we are just going to bild a new blob with updated root
  // signature to verify root signature Since dxil container builder will verify
  // on its behalf. This does unnecessary memory allocation. We can improve this
  // later.
  CComPtr<IDxcContainerBuilder> pContainerBuilder;
  IFT(CreateInstance(CLSID_DxcContainerBuilder, &pContainerBuilder));
  IFT(pContainerBuilder->Load(pSource));
  // Try removing root signature if it already exists
  pContainerBuilder->RemovePart(hlsl::DxilFourCC::DFCC_RootSignature);
  IFT(pContainerBuilder->AddPart(hlsl::DxilFourCC::DFCC_RootSignature,
                                 pRootSignature));
  CComPtr<IDxcOperationResult> pOperationResult;
  IFT(pContainerBuilder->SerializeContainer(&pOperationResult));
  HRESULT status = E_FAIL;
  CComPtr<IDxcBlob> pResult;
  IFT(pOperationResult->GetStatus(&status));
  if (FAILED(status)) {
    if (!m_Opts.OutputWarningsFile.empty()) {
      CComPtr<IDxcBlobEncoding> pErrors;
      IFT(pOperationResult->GetErrorBuffer(&pErrors));
      WriteBlobToFile(pErrors, m_Opts.OutputWarningsFile,
                      m_Opts.DefaultTextCodePage);
    } else {
      WriteOperationErrorsToConsole(pOperationResult, m_Opts.OutputWarnings);
    }
    return 1;
  } else {
    printf("root signature verification succeeded.");
    return 0;
  }
}

class DxcIncludeHandlerForInjectedSources : public IDxcIncludeHandler {
private:
  DXC_MICROCOM_REF_FIELD(m_dwRef)

public:
  DXC_MICROCOM_ADDREF_RELEASE_IMPL(m_dwRef)
  DxcIncludeHandlerForInjectedSources() : m_dwRef(0){};
  std::unordered_map<std::wstring, CComPtr<IDxcBlob>> includeFiles;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcIncludeHandler>(this, iid, ppvObject);
  }

  HRESULT insertIncludeFile(LPCWSTR pFilename, IDxcBlobEncoding *pBlob,
                            UINT32 dataLen) {
    try {
#ifdef _WIN32
      includeFiles.try_emplace(std::wstring(pFilename), pBlob);
#else
      // Note: try_emplace is only available in C++17 on Linux.
      // try_emplace does nothing if the key already exists in the map.
      if (includeFiles.find(std::wstring(pFilename)) == includeFiles.end())
        includeFiles.emplace(std::wstring(pFilename), pBlob);
#endif // _WIN32
    }
    CATCH_CPP_RETURN_HRESULT()
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR pFilename,
                                       IDxcBlob **ppIncludeSource) override {
    try {
      // Convert pFilename into native form for indexing as is done when the MD
      // is created
      std::string FilenameStr8 = Unicode::WideToUTF8StringOrThrow(pFilename);
      llvm::SmallString<128> NormalizedPath;
      llvm::sys::path::native(FilenameStr8, NormalizedPath);
      std::wstring FilenameStr16 =
          Unicode::UTF8ToWideStringOrThrow(NormalizedPath.c_str());
      *ppIncludeSource = includeFiles.at(FilenameStr16);
      (*ppIncludeSource)->AddRef();
    }
    CATCH_CPP_RETURN_HRESULT()
    return S_OK;
  }
};

void DxcContext::Recompile(IDxcBlob *pSource, IDxcLibrary *pLibrary,
                           IDxcCompiler *pCompiler, std::vector<LPCWSTR> &args,
                           std::wstring &outputPDBPath,
                           CComPtr<IDxcBlob> &pDebugBlob,
                           IDxcOperationResult **ppCompileResult) {

  CComPtr<IDxcPdbUtils> pPdbUtils;
  IFT(CreateInstance(CLSID_DxcPdbUtils, &pPdbUtils));
  IFT(pPdbUtils->Load(pSource));

  UINT32 uNumFlags = 0;
  IFT(pPdbUtils->GetFlagCount(&uNumFlags));
  std::vector<const WCHAR *> NewArgs;
  std::vector<std::wstring> NewArgsStorage;
  for (UINT32 i = 0; i < uNumFlags; i++) {
    CComBSTR pFlag;
    IFT(pPdbUtils->GetFlag(i, &pFlag));
    NewArgsStorage.push_back(std::wstring(pFlag));
  }
  for (const std::wstring &flag : NewArgsStorage)
    NewArgs.push_back(flag.c_str());

  UINT32 uNumDefines = 0;
  IFT(pPdbUtils->GetDefineCount(&uNumDefines));
  std::vector<std::wstring> NewDefinesStorage;
  std::vector<DxcDefine> NewDefines;
  for (UINT32 i = 0; i < uNumDefines; i++) {
    CComBSTR pDefine;
    IFT(pPdbUtils->GetDefine(i, &pDefine));
    NewDefinesStorage.push_back(std::wstring(pDefine));
  }
  for (std::wstring &Define : NewDefinesStorage) {
    wchar_t *pDefineStart = &Define[0];
    wchar_t *pDefineEnd = pDefineStart + Define.size();

    DxcDefine D = {};
    D.Name = pDefineStart;
    D.Value = nullptr;
    for (wchar_t *pCursor = pDefineStart; pCursor < pDefineEnd; ++pCursor) {
      if (*pCursor == L'=') {
        *pCursor = L'\0';
        D.Value = (pCursor + 1);
        break;
      }
    }
    NewDefines.push_back(D);
  }

  CComBSTR pMainFileName;
  CComBSTR pTargetProfile;
  CComBSTR pEntryPoint;
  IFT(pPdbUtils->GetMainFileName(&pMainFileName));
  IFT(pPdbUtils->GetTargetProfile(&pTargetProfile));
  IFT(pPdbUtils->GetEntryPoint(&pEntryPoint));

  CComPtr<IDxcBlobEncoding> pCompileSource;
  CComPtr<DxcIncludeHandlerForInjectedSources> pIncludeHandler =
      new DxcIncludeHandlerForInjectedSources();
  UINT32 uSourceCount = 0;
  IFT(pPdbUtils->GetSourceCount(&uSourceCount));
  for (UINT32 i = 0; i < uSourceCount; i++) {
    CComPtr<IDxcBlobEncoding> pSourceFile;
    CComBSTR pFileName;
    IFT(pPdbUtils->GetSource(i, &pSourceFile));
    IFT(pPdbUtils->GetSourceName(i, &pFileName));
    IFT(pIncludeHandler->insertIncludeFile(pFileName, pSourceFile, 0));
    if (pMainFileName == pFileName) {
      // Transfer pSourceFile to avoid extra AddRef+Release.
      pCompileSource.Attach(pSourceFile.Detach());
    }
  }

  CComPtr<IDxcOperationResult> pResult;

  if (!m_Opts.DebugFile.empty()) {
    CComPtr<IDxcCompiler2> pCompiler2;
    CComHeapPtr<WCHAR> pDebugName;
    Unicode::UTF8ToWideString(m_Opts.DebugFile.str().c_str(), &outputPDBPath);
    IFT(pCompiler->QueryInterface(&pCompiler2));
    IFT(pCompiler2->CompileWithDebug(
        pCompileSource, pMainFileName, pEntryPoint, pTargetProfile,
        NewArgs.data(), NewArgs.size(), NewDefines.data(), NewDefines.size(),
        pIncludeHandler, &pResult, &pDebugName, &pDebugBlob));
    if (pDebugName.m_pData && m_Opts.DebugFileIsDirectory()) {
      outputPDBPath += pDebugName.m_pData;
    }
  } else {
    IFT(pCompiler->Compile(pCompileSource, pMainFileName, pEntryPoint,
                           pTargetProfile, NewArgs.data(), NewArgs.size(),
                           NewDefines.data(), NewDefines.size(),
                           pIncludeHandler, &pResult));
  }

  *ppCompileResult = pResult.Detach();
}

int DxcContext::Compile() {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pCompileResult;
  CComPtr<IDxcBlob> pDebugBlob;
  std::wstring outputPDBPath;
  {
    CComPtr<IDxcBlobEncoding> pSource;

    std::vector<std::wstring> argStrings;
    CopyArgsToWStrings(m_Opts.Args, CoreOption, argStrings);

    std::vector<LPCWSTR> args;
    args.reserve(argStrings.size());
    for (const std::wstring &a : argStrings)
      args.push_back(a.data());

    if (m_Opts.AstDump)
      args.push_back(L"-ast-dump");

    CComPtr<IDxcLibrary> pLibrary;
    IFT(CreateInstance(CLSID_DxcLibrary, &pLibrary));
    IFT(CreateInstance(CLSID_DxcCompiler, &pCompiler));
    ReadFileIntoBlob(m_dxcSupport, StringRefWide(m_Opts.InputFile), &pSource);
    IFTARG(pSource->GetBufferSize() >= 4);

    if (m_Opts.RecompileFromBinary) {
      Recompile(pSource, pLibrary, pCompiler, args, outputPDBPath, pDebugBlob,
                &pCompileResult);
    } else {
      CComPtr<IDxcIncludeHandler> pIncludeHandler;
      IFT(pLibrary->CreateIncludeHandler(&pIncludeHandler));

      // Upgrade profile to 6.0 version from minimum recognized shader model
      llvm::StringRef TargetProfile = m_Opts.TargetProfile;
      const hlsl::ShaderModel *SM =
          hlsl::ShaderModel::GetByName(m_Opts.TargetProfile.str().c_str());
      if (SM->IsValid() && SM->GetMajor() < 6) {
        TargetProfile = hlsl::ShaderModel::Get(SM->GetKind(), 6, 0)->GetName();
        std::string versionWarningString =
            "warning: Promoting older shader model profile to 6.0 version.";
        fprintf(stderr, "%s\n", versionWarningString.data());
        if (!SM->IsSM51Plus()) {
          // Add flag for backcompat with SM 5.0 resource reservation
          args.push_back(L"-flegacy-resource-reservation");
        }
      }

      if (!m_Opts.DebugFile.empty()) {
        CComPtr<IDxcCompiler2> pCompiler2;
        CComHeapPtr<WCHAR> pDebugName;
        Unicode::UTF8ToWideString(m_Opts.DebugFile.str().c_str(),
                                  &outputPDBPath);
        IFT(pCompiler.QueryInterface(&pCompiler2));
        IFT(pCompiler2->CompileWithDebug(
            pSource, StringRefWide(m_Opts.InputFile),
            StringRefWide(m_Opts.EntryPoint), StringRefWide(TargetProfile),
            args.data(), args.size(), m_Opts.Defines.data(),
            m_Opts.Defines.size(), pIncludeHandler, &pCompileResult,
            &pDebugName, &pDebugBlob));
        if (pDebugName.m_pData && m_Opts.DebugFileIsDirectory()) {
          outputPDBPath += pDebugName.m_pData;
        }
      } else {
        IFT(pCompiler->Compile(
            pSource, StringRefWide(m_Opts.InputFile),
            StringRefWide(m_Opts.EntryPoint), StringRefWide(TargetProfile),
            args.data(), args.size(), m_Opts.Defines.data(),
            m_Opts.Defines.size(), pIncludeHandler, &pCompileResult));
      }
    }

    // When compiling we don't embed debug info if options don't ask for it.
    // If user specified /Qstrip_debug, remove from m_Opts now so we don't
    // try to modify the container to strip debug info that isn't there.
    if (!m_Opts.EmbedDebugInfo()) {
      m_Opts.StripDebug = false;
    }
  }

  if (!m_Opts.OutputWarningsFile.empty()) {
    CComPtr<IDxcBlobEncoding> pErrors;
    IFT(pCompileResult->GetErrorBuffer(&pErrors));
    WriteBlobToFile(pErrors, m_Opts.OutputWarningsFile,
                    m_Opts.DefaultTextCodePage);
  } else {
    WriteOperationErrorsToConsole(pCompileResult, m_Opts.OutputWarnings);
  }

  HRESULT status;
  IFT(pCompileResult->GetStatus(&status));
  if (SUCCEEDED(status) || m_Opts.AstDump || m_Opts.OptDump ||
      m_Opts.DumpDependencies || m_Opts.VerifyDiagnostics) {
    CComPtr<IDxcBlob> pProgram;
    IFT(pCompileResult->GetResult(&pProgram));
    if (pProgram.p != nullptr) {
      ActOnBlob(pProgram.p, pDebugBlob, outputPDBPath.c_str());

      // Now write out extra parts
      CComPtr<IDxcResult> pResult;
      if (SUCCEEDED(pCompileResult->QueryInterface(&pResult))) {
        WriteDxcOutputToConsole(pResult, DXC_OUT_REMARKS);
        WriteDxcOutputToConsole(pResult, DXC_OUT_TIME_REPORT);

        if (m_Opts.TimeTrace == "-")
          WriteDxcOutputToConsole(pResult, DXC_OUT_TIME_TRACE);
        else if (!m_Opts.TimeTrace.empty()) {
          CComPtr<IDxcBlob> pData;
          CComPtr<IDxcBlobWide> pName;
          IFT(pResult->GetOutput(DXC_OUT_TIME_TRACE, IID_PPV_ARGS(&pData),
                                 &pName));
          WriteBlobToFile(pData, m_Opts.TimeTrace, m_Opts.DefaultTextCodePage);
        }

        WriteDxcOutputToFile(DXC_OUT_ROOT_SIGNATURE, pResult,
                             m_Opts.DefaultTextCodePage);
        WriteDxcOutputToFile(DXC_OUT_SHADER_HASH, pResult,
                             m_Opts.DefaultTextCodePage);
        WriteDxcOutputToFile(DXC_OUT_REFLECTION, pResult,
                             m_Opts.DefaultTextCodePage);
        WriteDxcExtraOuputs(pResult);
      }
    }
  }
  return status;
}

int DxcContext::Link() {
  CComPtr<IDxcLinker> pLinker;
  IFT(CreateInstance(CLSID_DxcLinker, &pLinker));
  llvm::StringRef InputFiles = m_Opts.InputFile;
  llvm::StringRef InputFilesRef(InputFiles);
  llvm::SmallVector<llvm::StringRef, 2> InputFileList;
  InputFilesRef.split(InputFileList, ";");

  std::vector<std::wstring> wInputFiles;
  wInputFiles.reserve(InputFileList.size());
  std::vector<LPCWSTR> wpInputFiles;
  wpInputFiles.reserve(InputFileList.size());
  for (auto &file : InputFileList) {
    wInputFiles.emplace_back(StringRefWide(file.str()));
    wpInputFiles.emplace_back(wInputFiles.back().c_str());
    CComPtr<IDxcBlobEncoding> pLib;
    ReadFileIntoBlob(m_dxcSupport, wInputFiles.back().c_str(), &pLib);
    IFT(pLinker->RegisterLibrary(wInputFiles.back().c_str(), pLib));
  }

  CComPtr<IDxcOperationResult> pLinkResult;

  std::vector<std::wstring> argStrings;
  CopyArgsToWStrings(m_Opts.Args, CoreOption, argStrings);

  std::vector<LPCWSTR> args;
  args.reserve(argStrings.size());
  for (const std::wstring &a : argStrings)
    args.push_back(a.data());

  IFT(pLinker->Link(StringRefWide(m_Opts.EntryPoint),
                    StringRefWide(m_Opts.TargetProfile), wpInputFiles.data(),
                    wpInputFiles.size(), args.data(), args.size(),
                    &pLinkResult));

  HRESULT status;
  IFT(pLinkResult->GetStatus(&status));
  if (SUCCEEDED(status)) {
    CComPtr<IDxcBlob> pContainer;
    IFT(pLinkResult->GetResult(&pContainer));
    if (pContainer.p != nullptr) {
      ActOnBlob(pContainer.p);
    }
  } else {
    CComPtr<IDxcBlobEncoding> pErrors;
    IFT(pLinkResult->GetErrorBuffer(&pErrors));
    if (pErrors != nullptr) {
      printf("Link failed:\n%s",
             static_cast<char *>(pErrors->GetBufferPointer()));
    }
    return 1;
  }
  return 0;
}

int DxcContext::DumpBinary() {
  CComPtr<IDxcBlobEncoding> pSource;
  ReadFileIntoBlob(m_dxcSupport, StringRefWide(m_Opts.InputFile), &pSource);
  return ActOnBlob(pSource.p);
}

void DxcContext::Preprocess() {
  DXASSERT(!m_Opts.Preprocess.empty(),
           "else option reading should have failed");
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pPreprocessResult;
  CComPtr<IDxcBlobEncoding> pSource;

  std::vector<std::wstring> argStrings;
  CopyArgsToWStrings(m_Opts.Args, CoreOption, argStrings);

  std::vector<LPCWSTR> args;
  args.reserve(argStrings.size());
  for (const std::wstring &a : argStrings)
    args.push_back(a.data());

  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcIncludeHandler> pIncludeHandler;
  IFT(CreateInstance(CLSID_DxcLibrary, &pLibrary));
  IFT(pLibrary->CreateIncludeHandler(&pIncludeHandler));

  ReadFileIntoBlob(m_dxcSupport, StringRefWide(m_Opts.InputFile), &pSource);
  IFT(CreateInstance(CLSID_DxcCompiler, &pCompiler));
  IFT(pCompiler->Preprocess(pSource, StringRefWide(m_Opts.InputFile),
                            args.data(), args.size(), m_Opts.Defines.data(),
                            m_Opts.Defines.size(), pIncludeHandler,
                            &pPreprocessResult));
  WriteOperationErrorsToConsole(pPreprocessResult, m_Opts.OutputWarnings);

  HRESULT status;
  IFT(pPreprocessResult->GetStatus(&status));
  if (SUCCEEDED(status)) {
    CComPtr<IDxcBlob> pProgram;
    IFT(pPreprocessResult->GetResult(&pProgram));
    WriteBlobToFile(pProgram, m_Opts.Preprocess, m_Opts.DefaultTextCodePage);
  }
}

void DxcContext::WriteHeader(IDxcBlobEncoding *pDisassembly, IDxcBlob *pCode,
                             llvm::Twine &pVariableName, LPCWSTR pFileName) {
  // Use older interface for compatibility with older DLL.
  CComPtr<IDxcLibrary> pLibrary;
  IFT(CreateInstance(CLSID_DxcLibrary, &pLibrary));

  std::string s;
  llvm::raw_string_ostream OS(s);

  {
    // Not safe to assume pDisassembly is utf8, must GetBlobAsUtf8 first.
    CComPtr<IDxcBlobEncoding> pDisasmEncoding;
    IFT(pLibrary->GetBlobAsUtf8(pDisassembly, &pDisasmEncoding));

    // Don't fail if this QI doesn't succeed (older dll, perhaps)
    CComPtr<IDxcBlobUtf8> pDisasmUtf8;
    pDisasmEncoding->QueryInterface(&pDisasmUtf8);

    LPCSTR pBytes = pDisasmUtf8 ? pDisasmUtf8->GetStringPointer()
                                : (LPCSTR)pDisasmEncoding->GetBufferPointer();
    // IDxcBlobUtf8's GetStringLength will return length without null character
    size_t len = pDisasmUtf8 ? pDisasmUtf8->GetStringLength()
                             : pDisasmEncoding->GetBufferSize();
    // Just in case there are still any null characters at the end, get rid of
    // them.
    while (len && pBytes[len - 1] == '\0')
      len -= 1;

    // Note: with \r\n line endings, writing the disassembly could be a simple
    // WriteBlobToHandle with a prior and following WriteString for #ifs
    OS << "#if 0\r\n";
    s.reserve(len + len * 0.1f); // rough estimate
    for (size_t i = 0; i < len; ++i) {
      if (pBytes[i] == '\n')
        OS << '\r';
      OS << pBytes[i];
    }
    OS << "\r\n#endif\r\n";
  }

  {
    OS << "\r\nconst unsigned char " << pVariableName << "[] = {";
    const uint8_t *pBytes = (const uint8_t *)pCode->GetBufferPointer();
    size_t len = pCode->GetBufferSize();
    s.reserve(100 + len * 6 + (len / 12) * 3); // rough estimate
    for (size_t i = 0; i < len; ++i) {
      if (i != 0)
        OS << ',';
      if ((i % 12) == 0)
        OS << "\r\n ";
      OS << " 0x";
      if (pBytes[i] < 0x10)
        OS << '0';
      OS.write_hex(pBytes[i]);
    }
    OS << "\r\n};\r\n";
  }

  OS.flush();

  // Respect user's -encoding option
  CComPtr<IDxcBlobEncoding> pOutBlob;
  pLibrary->CreateBlobWithEncodingFromPinned(s.data(), s.length(), DXC_CP_UTF8,
                                             &pOutBlob);
  WriteBlobToFile(pOutBlob, pFileName, m_Opts.DefaultTextCodePage);
}

// Finds DXIL module from the blob assuming blob is either DxilContainer,
// DxilPartHeader, or DXIL module
HRESULT DxcContext::FindModuleBlob(hlsl::DxilFourCC fourCC, IDxcBlob *pSource,
                                   IDxcLibrary *pLibrary,
                                   IDxcBlob **ppTargetBlob) {
  if (!pSource || !pLibrary || !ppTargetBlob)
    return E_INVALIDARG;
  const UINT32 BC_C0DE = ((INT32)(INT8)'B' | (INT32)(INT8)'C' << 8 |
                          (INT32)0xDEC0 << 16); // BC0xc0de in big endian
  if (BC_C0DE == *(UINT32 *)pSource->GetBufferPointer()) {
    *ppTargetBlob = pSource;
    pSource->AddRef();
    return S_OK;
  }
  const char *pBitcode = nullptr;
  const hlsl::DxilPartHeader *pDxilPartHeader =
      (hlsl::DxilPartHeader *)
          pSource->GetBufferPointer(); // Initialize assuming that source is
                                       // starting with DXIL part

  if (hlsl::IsValidDxilContainer(
          (hlsl::DxilContainerHeader *)pSource->GetBufferPointer(),
          pSource->GetBufferSize())) {
    hlsl::DxilContainerHeader *pDxilContainerHeader =
        (hlsl::DxilContainerHeader *)pSource->GetBufferPointer();
    pDxilPartHeader = hlsl::GetDxilPartByType(pDxilContainerHeader, fourCC);
    IFTBOOL(pDxilPartHeader != nullptr, DXC_E_CONTAINER_MISSING_DEBUG);
  }
  if (fourCC == pDxilPartHeader->PartFourCC) {
    UINT32 pBlobSize;
    const hlsl::DxilProgramHeader *pDxilProgramHeader =
        (const hlsl::DxilProgramHeader *)(pDxilPartHeader + 1);
    hlsl::GetDxilProgramBitcode(pDxilProgramHeader, &pBitcode, &pBlobSize);
    UINT32 offset =
        (UINT32)(pBitcode - (const char *)pSource->GetBufferPointer());
    pLibrary->CreateBlobFromBlob(pSource, offset, pBlobSize, ppTargetBlob);
    return S_OK;
  }
  return E_INVALIDARG;
}

// This function is currently only supported on Windows due to usage of
// IDiaTable.
#ifdef _WIN32
// TODO : There is an identical code in DxaContext in Dxa.cpp. Refactor this
// function.
HRESULT DxcContext::GetDxcDiaTable(IDxcLibrary *pLibrary, IDxcBlob *pTargetBlob,
                                   IDiaTable **ppTable, LPCWSTR tableName) {
  if (!pLibrary || !pTargetBlob || !ppTable)
    return E_INVALIDARG;
  CComPtr<IDiaDataSource> pDataSource;
  CComPtr<IStream> pSourceStream;
  CComPtr<IDiaSession> pSession;
  CComPtr<IDiaEnumTables> pEnumTables;
  IFT(CreateInstance(CLSID_DxcDiaDataSource, &pDataSource));
  IFT(pLibrary->CreateStreamFromBlobReadOnly(pTargetBlob, &pSourceStream));
  IFT(pDataSource->loadDataFromIStream(pSourceStream));
  IFT(pDataSource->openSession(&pSession));
  IFT(pSession->getEnumTables(&pEnumTables));
  CComPtr<IDiaTable> pTable;
  for (;;) {
    ULONG fetched;
    pTable.Release();
    IFT(pEnumTables->Next(1, &pTable, &fetched));
    if (fetched == 0) {
      pTable.Release();
      break;
    }
    CComBSTR name;
    IFT(pTable->get_name(&name));
    if (wcscmp(name, tableName) == 0) {
      break;
    }
  }
  *ppTable = pTable.Detach();
  return S_OK;
}
#endif // _WIN32

bool GetDLLFileVersionInfo(const char *dllPath, unsigned int *version) {
  // This function is used to get version information from the DLL file.
  // This information in is not available through a Unix interface.
#ifdef _WIN32
  DWORD dwVerHnd = 0;
  DWORD size = GetFileVersionInfoSize(dllPath, &dwVerHnd);
  if (size == 0)
    return false;
  std::unique_ptr<BYTE[]> VfInfo(new BYTE[size]);
  if (GetFileVersionInfo(dllPath, NULL, size, VfInfo.get())) {
    LPVOID versionInfo;
    UINT size;
    if (VerQueryValue(VfInfo.get(), "\\", &versionInfo, &size)) {
      if (size >= sizeof(VS_FIXEDFILEINFO)) {
        VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)versionInfo;
        version[0] = (verInfo->dwFileVersionMS >> 16) & 0xffff;
        version[1] = (verInfo->dwFileVersionMS >> 0) & 0xffff;
        version[2] = (verInfo->dwFileVersionLS >> 16) & 0xffff;
        version[3] = (verInfo->dwFileVersionLS >> 0) & 0xffff;
        return true;
      }
    }
  }
#endif // _WIN32
  return false;
}

bool GetDLLProductVersionInfo(const char *dllPath,
                              std::string &productVersion) {
  // This function is used to get product version information from the DLL file.
  // This information in is not available through a Unix interface.
#ifdef _WIN32
  DWORD dwVerHnd = 0;
  DWORD size = GetFileVersionInfoSize(dllPath, &dwVerHnd);
  if (size == 0)
    return false;
  std::unique_ptr<BYTE[]> VfInfo(new BYTE[size]);
  if (GetFileVersionInfo(dllPath, NULL, size, VfInfo.get())) {
    LPVOID pvProductVersion = NULL;
    unsigned int iProductVersionLen = 0;
    // 040904b0 == code page US English, Unicode
    if (VerQueryValue(VfInfo.get(),
                      "\\StringFileInfo\\040904b0\\ProductVersion",
                      &pvProductVersion, &iProductVersionLen)) {
      productVersion = (LPCSTR)pvProductVersion;
      return true;
    }
  }
#endif // _WIN32
  return false;
}

namespace dxc {

// Writes compiler version info to stream
void WriteDxCompilerVersionInfo(llvm::raw_ostream &OS, const char *ExternalLib,
                                const char *ExternalFn,
                                DxcDllSupport &DxcSupport) {
  if (DxcSupport.IsEnabled()) {
    UINT32 compilerMajor = 1;
    UINT32 compilerMinor = 0;
    CComPtr<IDxcVersionInfo> VerInfo;

#ifdef SUPPORT_QUERY_GIT_COMMIT_INFO
    UINT32 commitCount = 0;
    CComHeapPtr<char> commitHash;
    CComPtr<IDxcVersionInfo2> VerInfo2;
#endif // SUPPORT_QUERY_GIT_COMMIT_INFO

    const char *dllName = !ExternalLib ? kDxCompilerLib : ExternalLib;
    std::string compilerName(dllName);
    if (ExternalFn)
      compilerName = compilerName + "!" + ExternalFn;

    if (SUCCEEDED(DxcSupport.CreateInstance(CLSID_DxcCompiler, &VerInfo))) {
      VerInfo->GetVersion(&compilerMajor, &compilerMinor);
#ifdef SUPPORT_QUERY_GIT_COMMIT_INFO
      if (SUCCEEDED(VerInfo->QueryInterface(&VerInfo2)))
        VerInfo2->GetCommitInfo(&commitCount, &commitHash);
#endif // SUPPORT_QUERY_GIT_COMMIT_INFO
      OS << compilerName << ": " << compilerMajor << "." << compilerMinor;
    }
    // compiler.dll 1.0 did not support IdxcVersionInfo
    else if (!ExternalLib) {
      OS << compilerName << ": " << 1 << "." << 0;
    } else {
      // ExternalLib/ExternalFn, no version info:
      OS << compilerName;
    }

#ifdef _WIN32
    unsigned int version[4];
    if (GetDLLFileVersionInfo(dllName, version)) {
      // back-compat - old dev buidls had version 3.7.0.0
      if (version[0] == 3 && version[1] == 7 && version[2] == 0 &&
          version[3] == 0) {
#endif
        OS << "(dev"
#ifdef SUPPORT_QUERY_GIT_COMMIT_INFO
           << ";" << commitCount << "-"
           << (commitHash.m_pData ? commitHash.m_pData : "<unknown-git-hash>")
#endif // SUPPORT_QUERY_GIT_COMMIT_I#else
           << ")";
#ifdef _WIN32
      } else {
        std::string productVersion;
        if (GetDLLProductVersionInfo(dllName, productVersion)) {
          OS << " - " << productVersion;
        }
      }
    }
#endif
  }
}

// Writes compiler version info to stream
void WriteDXILVersionInfo(llvm::raw_ostream &OS, DxcDllSupport &DxilSupport) {
  if (DxilSupport.IsEnabled()) {
    CComPtr<IDxcVersionInfo> VerInfo;
    if (SUCCEEDED(DxilSupport.CreateInstance(CLSID_DxcValidator, &VerInfo))) {
      UINT32 validatorMajor, validatorMinor = 0;
      VerInfo->GetVersion(&validatorMajor, &validatorMinor);
      OS << "; " << kDxilLib << ": " << validatorMajor << "." << validatorMinor;

    }
    // dxil.dll 1.0 did not support IdxcVersionInfo
    else {
      OS << "; " << kDxilLib << ": " << 1 << "." << 0;
    }
    unsigned int version[4];
    if (GetDLLFileVersionInfo(kDxilLib, version)) {
      OS << "(" << version[0] << "." << version[1] << "." << version[2] << "."
         << version[3] << ")";
    }
  }
}

} // namespace dxc

// Collects compiler/validator version info
void DxcContext::GetCompilerVersionInfo(llvm::raw_string_ostream &OS) {
  WriteDxCompilerVersionInfo(
      OS, m_Opts.ExternalLib.empty() ? nullptr : m_Opts.ExternalLib.data(),
      m_Opts.ExternalFn.empty() ? nullptr : m_Opts.ExternalFn.data(),
      m_dxcSupport);

  // Print validator if exists
  DxcDllSupport DxilSupport;
  DxilSupport.InitializeForDll(kDxilLib, "DxcCreateInstance");
  WriteDXILVersionInfo(OS, DxilSupport);
}

#ifndef VERSION_STRING_SUFFIX
#define VERSION_STRING_SUFFIX ""
#endif

#ifdef _WIN32
// Unhandled exception filter called when an unhandled exception occurs
// to at least print an generic error message instead of crashing silently.
// passes exception along to allow crash dumps to be generated
static LONG CALLBACK ExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo) {
  static char scratch[32];

  fputs("Internal compiler error: ", stderr);

  if (!pExceptionInfo || !pExceptionInfo->ExceptionRecord) {
    // No information at all, it's not much, but it's the best we can do
    fputs("Unknown", stderr);
    return EXCEPTION_CONTINUE_SEARCH;
  }

  switch (pExceptionInfo->ExceptionRecord->ExceptionCode) {
    // native exceptions
  case EXCEPTION_ACCESS_VIOLATION: {
    fputs("access violation. Attempted to ", stderr);
    if (pExceptionInfo->ExceptionRecord->ExceptionInformation[0])
      fputs("write", stderr);
    else
      fputs("read", stderr);
    fputs(" from address ", stderr);
    sprintf_s(scratch, _countof(scratch), "0x%p\n",
              (void *)pExceptionInfo->ExceptionRecord->ExceptionInformation[1]);
    fputs(scratch, stderr);
  } break;
  case EXCEPTION_STACK_OVERFLOW:
    fputs("stack overflow\n", stderr);
    break;
    // LLVM exceptions
  case STATUS_LLVM_ASSERT:
    fputs("LLVM Assert\n", stderr);
    break;
  case STATUS_LLVM_UNREACHABLE:
    fputs("LLVM Unreachable\n", stderr);
    break;
  case STATUS_LLVM_FATAL:
    fputs("LLVM Fatal Error\n", stderr);
    break;
  case EXCEPTION_LOAD_LIBRARY_FAILED:
    if (pExceptionInfo->ExceptionRecord->ExceptionInformation[0]) {
      fputs("cannot not load ", stderr);
      fputws((const wchar_t *)
                 pExceptionInfo->ExceptionRecord->ExceptionInformation[0],
             stderr);
      fputs(" library.\n", stderr);
    } else {
      fputs("cannot not load library.\n", stderr);
    }
    break;
  default:
    fputs("Terminal Error ", stderr);
    sprintf_s(scratch, _countof(scratch), "0x%08x\n",
              pExceptionInfo->ExceptionRecord->ExceptionCode);
    fputs(scratch, stderr);
  }

  // Continue search to pass along the exception
  return EXCEPTION_CONTINUE_SEARCH;
}
#endif

#ifdef _WIN32
int dxc::main(int argc, const wchar_t **argv_) {
#else
int dxc::main(int argc, const char **argv_) {
#endif // _WIN32
  const char *pStage = "Operation";
  int retVal = 0;
  if (FAILED(DxcInitThreadMalloc()))
    return 1;
  DxcSetThreadMallocToDefault();
  try {
    pStage = "Argument processing";
    if (initHlslOptTable())
      throw std::bad_alloc();

    // Parse command line options.
    const OptTable *optionTable = getHlslOptTable();
    MainArgs argStrings(argc, argv_);
    DxcOpts dxcOpts;
    DxcDllSupport dxcSupport;

    // Read options and check errors.
    {
      std::string errorString;
      llvm::raw_string_ostream errorStream(errorString);
      int optResult =
          ReadDxcOpts(optionTable, DxcFlags, argStrings, dxcOpts, errorStream);
      errorStream.flush();
      if (errorString.size()) {
        if (optResult)
          fprintf(stderr, "dxc failed : %s\n", errorString.data());
        else
          fprintf(stderr, "dxc warning : %s\n", errorString.data());
      }
      if (optResult != 0) {
        return optResult;
      }
    }

    // Apply defaults.
    if (dxcOpts.EntryPoint.empty() && !dxcOpts.RecompileFromBinary) {
      dxcOpts.EntryPoint = "main";
    }

#ifdef _WIN32
    // Set exception handler if enabled
    if (dxcOpts.HandleExceptions)
      SetUnhandledExceptionFilter(ExceptionFilter);
#endif

    // Setup a helper DLL.
    {
      std::string dllErrorString;
      llvm::raw_string_ostream dllErrorStream(dllErrorString);
      int dllResult = SetupDxcDllSupport(dxcOpts, dxcSupport, dllErrorStream);
      dllErrorStream.flush();
      if (dllErrorString.size()) {
        fprintf(stderr, "%s\n", dllErrorString.data());
      }
      if (dllResult)
        return dllResult;
    }

    EnsureEnabled(dxcSupport);
    DxcContext context(dxcOpts, dxcSupport);
    // Handle help request, which overrides any other processing.
    if (dxcOpts.ShowHelp) {
      std::string helpString;
      llvm::raw_string_ostream helpStream(helpString);
      std::string version;
      llvm::raw_string_ostream versionStream(version);
      context.GetCompilerVersionInfo(versionStream);
      optionTable->PrintHelp(
          helpStream, "dxc.exe", "HLSL Compiler" VERSION_STRING_SUFFIX,
          versionStream.str().c_str(), dxcOpts.ShowHelpHidden);
      helpStream.flush();
      WriteUtf8ToConsoleSizeT(helpString.data(), helpString.size());
      return 0;
    }

    if (dxcOpts.ShowVersion) {
      std::string version;
      llvm::raw_string_ostream versionStream(version);
      context.GetCompilerVersionInfo(versionStream);
      versionStream.flush();
      WriteUtf8ToConsoleSizeT(version.data(), version.size());
      return 0;
    }

    // TODO: implement all other actions.
    if (!dxcOpts.Preprocess.empty()) {
      pStage = "Preprocessing";
      context.Preprocess();
    } else if (dxcOpts.DumpBin) {
      pStage = "Dumping existing binary";
      retVal = context.DumpBinary();
    } else if (dxcOpts.Link) {
      pStage = "Linking";
      retVal = context.Link();
    } else {
      pStage = "Compilation";
      retVal = context.Compile();
    }
  } catch (const ::hlsl::Exception &hlslException) {
    try {
      const char *msg = hlslException.what();
      Unicode::acp_char
          printBuffer[128]; // printBuffer is safe to treat as
                            // UTF-8 because we use ASCII only errors
      if (msg == nullptr || *msg == '\0') {
        switch (hlslException.hr) {
        case DXC_E_DUPLICATE_PART:
          sprintf_s(
              printBuffer, _countof(printBuffer),
              "dxc failed : DXIL container already contains the given part.");
          break;
        case DXC_E_MISSING_PART:
          sprintf_s(
              printBuffer, _countof(printBuffer),
              "dxc failed : DXIL container does not contain the given part.");
          break;
        case DXC_E_CONTAINER_INVALID:
          sprintf_s(printBuffer, _countof(printBuffer),
                    "dxc failed : Invalid DXIL container.");
          break;
        case DXC_E_CONTAINER_MISSING_DXIL:
          sprintf_s(printBuffer, _countof(printBuffer),
                    "dxc failed : DXIL container is missing DXIL part.");
          break;
        case DXC_E_CONTAINER_MISSING_DEBUG:
          sprintf_s(printBuffer, _countof(printBuffer),
                    "dxc failed : DXIL container is missing Debug Info part.");
          break;
        case DXC_E_LLVM_FATAL_ERROR:
          sprintf_s(printBuffer, _countof(printBuffer),
                    "dxc failed : Internal Compiler Error - LLVM Fatal Error!");
          break;
        case DXC_E_LLVM_UNREACHABLE:
          sprintf_s(
              printBuffer, _countof(printBuffer),
              "dxc failed : Internal Compiler Error - UNREACHABLE executed!");
          break;
        case DXC_E_LLVM_CAST_ERROR:
          sprintf_s(printBuffer, _countof(printBuffer),
                    "dxc failed : Internal Compiler Error - Cast of "
                    "incompatible type!");
          break;
        case E_OUTOFMEMORY:
          sprintf_s(printBuffer, _countof(printBuffer),
                    "dxc failed : Out of Memory.");
          break;
        case E_INVALIDARG:
          sprintf_s(printBuffer, _countof(printBuffer),
                    "dxc failed : Invalid argument.");
          break;
        default:
          sprintf_s(printBuffer, _countof(printBuffer),
                    "dxc failed : error code 0x%08x.\n", hlslException.hr);
        }
        msg = printBuffer;
      }

      WriteUtf8ToConsoleSizeT(msg, strlen(msg), STD_ERROR_HANDLE);
      printf("\n");
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

  return retVal;
}
