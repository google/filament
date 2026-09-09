///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// PixTestUtils.h                                                            //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides utility functions for PIX tests.                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "PixTestUtils.h"

#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/dxcapi.use.h"
#include "dxc/Test/DxcTestUtils.h"
#include "dxc/Test/HlslTestUtils.h"

static std::vector<std::string> Tokenize(const std::string &str,
                                         const char *delimiters) {
  std::vector<std::string> tokens;
  std::string copy = str;

  for (auto i = strtok(&copy[0], delimiters); i != nullptr;
       i = strtok(nullptr, delimiters)) {
    tokens.push_back(i);
  }

  return tokens;
}

// For RunAnnotationPasses
namespace {
std::string ToString(std::wstring from) {
  std::string ret;
  ret.assign(from.data(), from.data() + from.size());
  return ret;
}
std::string ExtractBracedSubstring(std::string const &line) {
  auto open = line.find('{');
  auto close = line.find('}');
  if (open != std::string::npos && close != std::string::npos &&
      open + 1 < close) {
    return line.substr(open + 1, close - open - 1);
  }
  return "";
}

int ExtractMetaInt32Value(std::string const &token) {
  if (token.substr(0, 5) == " i32 ") {
    return atoi(token.c_str() + 5);
  }
  return -1;
}
std::map<int, std::pair<int, int>>
FindAllocaRelatedMetadata(std::vector<std::string> const &lines) {
  // Find lines of the exemplary form
  // "!249 = !{i32 0, i32 20}"  (in the case of a DXIL value)
  // "!196 = !{i32 1, i32 5, i32 1}" (in the case of a DXIL alloca reg)
  // The first i32 is a tag indicating what type of metadata this is.
  // It doesn't matter if we parse poorly and find some data that don't match
  // this pattern, as long as we do find all the data that do match (we won't
  // be looking up the non-matchers in the resultant map anyway).

  const char *valueMetaDataAssignment = "= !{i32 0, ";
  const char *allocaMetaDataAssignment = "= !{i32 1, ";

  std::map<int, std::pair<int, int>> ret;
  for (auto const &line : lines) {
    if (line[0] == '!') {
      if (line.find(valueMetaDataAssignment) != std::string::npos ||
          line.find(allocaMetaDataAssignment) != std::string::npos) {
        int key = atoi(line.c_str() + 1);
        if (key != 0) {
          std::string bitInBraces = ExtractBracedSubstring(line);
          if (bitInBraces != "") {
            auto tokens = Tokenize(bitInBraces.c_str(), ",");
            if (tokens.size() == 2) {
              auto value = ExtractMetaInt32Value(tokens[1]);
              if (value != -1) {
                ret[key] = {value, 1};
              }
            }
            if (tokens.size() == 3) {
              auto value0 = ExtractMetaInt32Value(tokens[1]);
              if (value0 != -1) {
                auto value1 = ExtractMetaInt32Value(tokens[2]);
                if (value1 != -1) {
                  ret[key] = {value0, value1};
                }
              }
            }
          }
        }
      }
    }
  }
  return ret;
}

std::string ExtractValueName(std::string const &line) {
  auto foundEquals = line.find('=');
  if (foundEquals != std::string::npos && foundEquals > 4) {
    return line.substr(2, foundEquals - 3);
  }
  return "";
}
using DxilRegisterToNameMap = std::map<std::pair<int, int>, std::string>;

void CheckForAndInsertMapEntryIfFound(
    DxilRegisterToNameMap &registerToNameMap,
    std::map<int, std::pair<int, int>> const &metaDataKeyToValue,
    std::string const &line, char const *tag, size_t tagLength) {
  auto foundAlloca = line.find(tag);
  if (foundAlloca != std::string::npos) {
    auto valueName = ExtractValueName(line);
    if (valueName != "") {
      int key = atoi(line.c_str() + foundAlloca + tagLength);
      auto foundKey = metaDataKeyToValue.find(key);
      if (foundKey != metaDataKeyToValue.end()) {
        registerToNameMap[foundKey->second] = valueName;
      }
    }
  }
}

// Here's some exemplary DXIL to help understand what's going on:
//
//  %5 = alloca [1 x float], i32 0, !pix-alloca-reg !196
//  %25 = call float @dx.op.loadInput.f32(...), !pix-dxil-reg !255
//
// The %5 is an alloca name, and the %25 is a regular llvm value.
// The meta-data tags !pix-alloca-reg and !pix-dxil-reg denote this,
// and provide keys !196 and !255 respectively.
// Those keys are then given values later on in the DXIL like this:
//
// !196 = !{i32 1, i32 5, i32 1}  (5 is the base alloca, 1 is the offset into
// it) !255 = !{i32 0, i32 23}
//
// So the task is first to find all of those key/value pairs and make a map
// from e.g. !196 to, e.g., (5,1), and then to find all of the alloca and reg
// tags and look up the keys in said map to build the map we're actually
// looking for, with key->values like e.g. "%5"->(5,1) and "%25"->(23)

DxilRegisterToNameMap BuildDxilRegisterToNameMap(char const *disassembly) {
  DxilRegisterToNameMap ret;

  auto lines = Tokenize(disassembly, "\n");

  auto metaDataKeyToValue = FindAllocaRelatedMetadata(lines);

  for (auto const &line : lines) {
    if (line[0] == '!') {
      // Stop searching for values when we've run into the metadata region of
      // the disassembly
      break;
    }
    const char allocaTag[] = "!pix-alloca-reg !";
    CheckForAndInsertMapEntryIfFound(ret, metaDataKeyToValue, line, allocaTag,
                                     _countof(allocaTag) - 1);
    const char valueTag[] = "!pix-dxil-reg !";
    CheckForAndInsertMapEntryIfFound(ret, metaDataKeyToValue, line, valueTag,
                                     _countof(valueTag) - 1);
  }
  return ret;
}

std::wstring Disassemble(dxc::DllLoader &dllSupport, IDxcBlob *pProgram) {
  CComPtr<IDxcCompiler> pCompiler;
  VERIFY_SUCCEEDED(pix_test::CreateCompiler(dllSupport, &pCompiler));

  CComPtr<IDxcBlobEncoding> pDbgDisassembly;
  VERIFY_SUCCEEDED(pCompiler->Disassemble(pProgram, &pDbgDisassembly));
  std::string disText = BlobToUtf8(pDbgDisassembly);
  CA2W disTextW(disText.c_str());
  return std::wstring(disTextW);
}

} // namespace

// For CreateBlobFromText
namespace {

void CreateBlobPinned(dxc::DllLoader &dllSupport,
                      _In_bytecount_(size) LPCVOID data, SIZE_T size,
                      UINT32 codePage, IDxcBlobEncoding **ppBlob) {
  CComPtr<IDxcLibrary> library;
  IFT(dllSupport.CreateInstance(CLSID_DxcLibrary, &library));
  IFT(library->CreateBlobWithEncodingFromPinned(data, size, codePage, ppBlob));
}
} // namespace

namespace pix_test {
std::vector<std::string> SplitAndPreserveEmptyLines(std::string const &str,
                                                    char delimeter) {
  std::vector<std::string> lines;

  auto const *p = str.data();
  auto const *justPastPreviousDelimiter = p;
  while (p < str.data() + str.length()) {
    if (*p == delimeter) {
      lines.emplace_back(justPastPreviousDelimiter,
                         p - justPastPreviousDelimiter);
      justPastPreviousDelimiter = p + 1;
      p = justPastPreviousDelimiter;
    } else {
      p++;
    }
  }

  lines.emplace_back(
      std::string(justPastPreviousDelimiter, p - justPastPreviousDelimiter));

  return lines;
}

void CompileAndLogErrors(dxc::DllLoader &dllSupport, LPCSTR pText,
                         LPCWSTR pTargetProfile, std::vector<LPCWSTR> &args,
                         IDxcIncludeHandler *includer,
                         _Outptr_ IDxcBlob **ppResult) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcOperationResult> pResult;
  HRESULT hrCompile;
  *ppResult = nullptr;
  VERIFY_SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
  Utf8ToBlob(dllSupport, pText, &pSource);
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      pTargetProfile, args.data(), args.size(),
                                      nullptr, 0, includer, &pResult));

  VERIFY_SUCCEEDED(pResult->GetStatus(&hrCompile));
  if (FAILED(hrCompile)) {
    CComPtr<IDxcBlobEncoding> textBlob;
    VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&textBlob));
    std::wstring text = BlobToWide(textBlob);
    WEX::Logging::Log::Comment(text.c_str());
  }
  VERIFY_SUCCEEDED(hrCompile);
  VERIFY_SUCCEEDED(pResult->GetResult(ppResult));
}

PassOutput RunAnnotationPasses(dxc::DllLoader &dllSupport, IDxcBlob *dxil,
                               int startingLineNumber) {
  CComPtr<IDxcOptimizer> pOptimizer;
  VERIFY_SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcOptimizer, &pOptimizer));
  std::vector<LPCWSTR> Options;
  Options.push_back(L"-opt-mod-passes");
  Options.push_back(L"-dxil-dbg-value-to-dbg-declare");
  std::wstring annotationCommandLine =
      L"-dxil-annotate-with-virtual-regs,startInstruction=" +
      std::to_wstring(startingLineNumber);
  Options.push_back(annotationCommandLine.c_str());

  CComPtr<IDxcBlob> pOptimizedModule;
  CComPtr<IDxcBlobEncoding> pText;
  VERIFY_SUCCEEDED(pOptimizer->RunOptimizer(
      dxil, Options.data(), Options.size(), &pOptimizedModule, &pText));

  std::string outputText = BlobToUtf8(pText);

  auto disasm = ToString(Disassemble(dllSupport, pOptimizedModule));

  auto registerToName = BuildDxilRegisterToNameMap(disasm.c_str());

  std::vector<ValueLocation> valueLocations;

  for (auto const &r2n : registerToName) {
    valueLocations.push_back({r2n.first.first, r2n.first.second});
  }

  return {std::move(pOptimizedModule), std::move(valueLocations),
          Tokenize(outputText.c_str(), "\n")};
}

class InstructionOffsetSeekerImpl : public InstructionOffsetSeeker {

  std::map<std::wstring, DWORD> m_labelToInstructionOffset;

public:
  InstructionOffsetSeekerImpl(DebuggerInterfaces &debuggerInterfaces) {
    DWORD SourceFileOrdinal = 0;
    CComBSTR fileName;
    CComBSTR fileContent;
    while (SUCCEEDED(debuggerInterfaces.compilationInfo->GetSourceFile(
        SourceFileOrdinal, &fileName, &fileContent))) {
      auto lines = strtok(std::wstring(fileContent), L"\n");
      for (size_t line = 0; line < lines.size(); ++line) {
        const wchar_t DebugLocLabel[] = L"debug-loc(";
        auto DebugLocPos = lines[line].find(DebugLocLabel);
        if (DebugLocPos != std::wstring::npos) {
          auto StartLabelPos = DebugLocPos + (_countof(DebugLocLabel) - 1);
          auto CloseDebugLocPos = lines[line].find(L")", StartLabelPos);
          if (CloseDebugLocPos == std::string::npos) {
            VERIFY_ARE_NOT_EQUAL(CloseDebugLocPos, std::string::npos);
          }
          auto Label = lines[line].substr(StartLabelPos,
                                          CloseDebugLocPos - StartLabelPos);
          CComPtr<IDxcPixDxilInstructionOffsets> InstructionOffsets;
          if (FAILED((debuggerInterfaces.debugInfo
                          ->InstructionOffsetsFromSourceLocation(
                              fileName, line + 1, 0, &InstructionOffsets)))) {
            VERIFY_FAIL(L"InstructionOffsetsFromSourceLocation failed");
          }
          auto InstructionOffsetCount = InstructionOffsets->GetCount();
          if (InstructionOffsetCount == 0) {
            VERIFY_FAIL(L"Instruction offset count was zero");
          }
          if (m_labelToInstructionOffset.find(Label) !=
              m_labelToInstructionOffset.end()) {
            VERIFY_FAIL(L"Duplicate label found!");
          }
          // Just the last offset is sufficient:
          m_labelToInstructionOffset[Label] =
              InstructionOffsets->GetOffsetByIndex(InstructionOffsetCount - 1);
        }
      }
      SourceFileOrdinal++;
      fileName.Empty();
      fileContent.Empty();
    }
  }

  virtual DWORD FindInstructionOffsetForLabel(const wchar_t *label) override {
    VERIFY_ARE_NOT_EQUAL(m_labelToInstructionOffset.find(label),
                         m_labelToInstructionOffset.end());
    return m_labelToInstructionOffset[label];
  }
};

std::unique_ptr<pix_test::InstructionOffsetSeeker>
GatherDebugLocLabelsFromDxcUtils(DebuggerInterfaces &debuggerInterfaces) {
  return std::make_unique<InstructionOffsetSeekerImpl>(debuggerInterfaces);
}

CComPtr<IDxcBlob> GetDebugPart(dxc::DllLoader &dllSupport,
                               IDxcBlob *container) {

  CComPtr<IDxcLibrary> pLib;
  VERIFY_SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));
  CComPtr<IDxcContainerReflection> pReflection;

  VERIFY_SUCCEEDED(
      dllSupport.CreateInstance(CLSID_DxcContainerReflection, &pReflection));
  VERIFY_SUCCEEDED(pReflection->Load(container));

  UINT32 index;
  VERIFY_SUCCEEDED(
      pReflection->FindFirstPartKind(hlsl::DFCC_ShaderDebugInfoDXIL, &index));

  CComPtr<IDxcBlob> debugPart;
  VERIFY_SUCCEEDED(pReflection->GetPartContent(index, &debugPart));

  return debugPart;
}

void CreateBlobFromText(dxc::DllLoader &dllSupport, const char *pText,
                        IDxcBlobEncoding **ppBlob) {
  CreateBlobPinned(dllSupport, pText, strlen(pText) + 1, CP_UTF8, ppBlob);
}

HRESULT CreateCompiler(dxc::DllLoader &dllSupport, IDxcCompiler **ppResult) {
  return dllSupport.CreateInstance(CLSID_DxcCompiler, ppResult);
}

CComPtr<IDxcBlob> Compile(dxc::DllLoader &dllSupport, const char *hlsl,
                          const wchar_t *target,
                          std::vector<const wchar_t *> extraArgs,
                          const wchar_t *entry) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;

  VERIFY_SUCCEEDED(CreateCompiler(dllSupport, &pCompiler));
  CreateBlobFromText(dllSupport, hlsl, &pSource);
  std::vector<const wchar_t *> args = {L"/Zi", L"/Qembed_debug"};
  args.insert(args.end(), extraArgs.begin(), extraArgs.end());
  VERIFY_SUCCEEDED(pCompiler->Compile(
      pSource, L"source.hlsl", entry, target, args.data(),
      static_cast<UINT32>(args.size()), nullptr, 0, nullptr, &pResult));

  HRESULT compilationStatus;
  VERIFY_SUCCEEDED(pResult->GetStatus(&compilationStatus));
  if (FAILED(compilationStatus)) {
    CComPtr<IDxcBlobEncoding> pErrros;
    VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&pErrros));
    CA2W errorTextW(static_cast<const char *>(pErrros->GetBufferPointer()));
    WEX::Logging::Log::Error(errorTextW);
    return {};
  }

#if 0 // handy for debugging
    {
      CComPtr<IDxcBlob> pProgram;
      CheckOperationSucceeded(pResult, &pProgram);

      CComPtr<IDxcLibrary> pLib;
      VERIFY_SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));
      const hlsl::DxilContainerHeader *pContainer = hlsl::IsDxilContainerLike(
          pProgram->GetBufferPointer(), pProgram->GetBufferSize());
      VERIFY_IS_NOT_NULL(pContainer);
      hlsl::DxilPartIterator partIter =
          std::find_if(hlsl::begin(pContainer), hlsl::end(pContainer),
                       hlsl::DxilPartIsType(hlsl::DFCC_ShaderDebugInfoDXIL));
      const hlsl::DxilProgramHeader *pProgramHeader =
          (const hlsl::DxilProgramHeader *)hlsl::GetDxilPartData(*partIter);
      uint32_t bitcodeLength;
      const char *pBitcode;
      CComPtr<IDxcBlob> pProgramPdb;
      hlsl::GetDxilProgramBitcode(pProgramHeader, &pBitcode, &bitcodeLength);
      VERIFY_SUCCEEDED(pLib->CreateBlobFromBlob(
          pProgram, pBitcode - (char *)pProgram->GetBufferPointer(),
          bitcodeLength, &pProgramPdb));

      CComPtr<IDxcBlobEncoding> pDbgDisassembly;
      VERIFY_SUCCEEDED(pCompiler->Disassemble(pProgramPdb, &pDbgDisassembly));
      std::string disText = BlobToUtf8(pDbgDisassembly);
      CA2W disTextW(disText.c_str());
      WEX::Logging::Log::Comment(disTextW);
    }
#endif

  CComPtr<IDxcBlob> pProgram;
  VERIFY_SUCCEEDED(pResult->GetResult(&pProgram));

  return pProgram;
}

CComPtr<IDxcBlob> WrapInNewContainer(dxc::DllLoader &dllSupport,
                                     IDxcBlob *part) {
  CComPtr<IDxcAssembler> pAssembler;
  IFT(dllSupport.CreateInstance(CLSID_DxcAssembler, &pAssembler));

  CComPtr<IDxcOperationResult> pAssembleResult;
  VERIFY_SUCCEEDED(pAssembler->AssembleToContainer(part, &pAssembleResult));

  CComPtr<IDxcBlobEncoding> pAssembleErrors;
  VERIFY_SUCCEEDED(pAssembleResult->GetErrorBuffer(&pAssembleErrors));

  if (pAssembleErrors && pAssembleErrors->GetBufferSize() != 0) {
    OutputDebugStringA(
        static_cast<LPCSTR>(pAssembleErrors->GetBufferPointer()));
    VERIFY_SUCCEEDED(E_FAIL);
  }

  CComPtr<IDxcBlob> pNewContainer;
  VERIFY_SUCCEEDED(pAssembleResult->GetResult(&pNewContainer));

  return pNewContainer;
}

} // namespace pix_test
