///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// PixDiaTest.cpp                                                            //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides tests for the PIX-specific components related to dia             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// This whole file is win32-only
#ifdef _WIN32

#include <array>
#include <set>

#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/WinIncludes.h"

#include "dxc/Test/DxcTestUtils.h"
#include "dxc/Test/HLSLTestData.h"
#include "dxc/Test/HlslTestUtils.h"

#include "llvm/Support/raw_os_ostream.h"

#include <../lib/DxilDia/DxilDiaSession.h>

#include "PixTestUtils.h"

using namespace std;
using namespace hlsl;
using namespace hlsl_test;
using namespace pix_test;

// Aligned to SymTagEnum.
const char *SymTagEnumText[] = {
    "Null",               // SymTagNull
    "Exe",                // SymTagExe
    "Compiland",          // SymTagCompiland
    "CompilandDetails",   // SymTagCompilandDetails
    "CompilandEnv",       // SymTagCompilandEnv
    "Function",           // SymTagFunction
    "Block",              // SymTagBlock
    "Data",               // SymTagData
    "Annotation",         // SymTagAnnotation
    "Label",              // SymTagLabel
    "PublicSymbol",       // SymTagPublicSymbol
    "UDT",                // SymTagUDT
    "Enum",               // SymTagEnum
    "FunctionType",       // SymTagFunctionType
    "PointerType",        // SymTagPointerType
    "ArrayType",          // SymTagArrayType
    "BaseType",           // SymTagBaseType
    "Typedef",            // SymTagTypedef
    "BaseClass",          // SymTagBaseClass
    "Friend",             // SymTagFriend
    "FunctionArgType",    // SymTagFunctionArgType
    "FuncDebugStart",     // SymTagFuncDebugStart
    "FuncDebugEnd",       // SymTagFuncDebugEnd
    "UsingNamespace",     // SymTagUsingNamespace
    "VTableShape",        // SymTagVTableShape
    "VTable",             // SymTagVTable
    "Custom",             // SymTagCustom
    "Thunk",              // SymTagThunk
    "CustomType",         // SymTagCustomType
    "ManagedType",        // SymTagManagedType
    "Dimension",          // SymTagDimension
    "CallSite",           // SymTagCallSite
    "InlineSite",         // SymTagInlineSite
    "BaseInterface",      // SymTagBaseInterface
    "VectorType",         // SymTagVectorType
    "MatrixType",         // SymTagMatrixType
    "HLSLType",           // SymTagHLSLType
    "Caller",             // SymTagCaller
    "Callee",             // SymTagCallee
    "Export",             // SymTagExport
    "HeapAllocationSite", // SymTagHeapAllocationSite
    "CoffGroup",          // SymTagCoffGroup
};

// Aligned to DataKind.
const char *DataKindText[] = {
    "Unknown",    "Local",  "StaticLocal", "Param",        "ObjectPtr",
    "FileStatic", "Global", "Member",      "StaticMember", "Constant",
};

static void CompileAndGetDebugPart(dxc::DllLoader &dllSupport,
                                   const char *source, const wchar_t *profile,
                                   IDxcBlob **ppDebugPart) {
  CComPtr<IDxcBlob> pContainer;
  CComPtr<IDxcLibrary> pLib;
  CComPtr<IDxcContainerReflection> pReflection;
  UINT32 index;
  std::vector<LPCWSTR> args;
  args.push_back(L"/Zi");
  args.push_back(L"/Qembed_debug");

  VerifyCompileOK(dllSupport, source, profile, args, &pContainer);
  VERIFY_SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));
  VERIFY_SUCCEEDED(
      dllSupport.CreateInstance(CLSID_DxcContainerReflection, &pReflection));
  VERIFY_SUCCEEDED(pReflection->Load(pContainer));
  VERIFY_SUCCEEDED(
      pReflection->FindFirstPartKind(hlsl::DFCC_ShaderDebugInfoDXIL, &index));
  VERIFY_SUCCEEDED(pReflection->GetPartContent(index, ppDebugPart));
}

static LPCWSTR defaultFilename = L"source.hlsl";

static HRESULT UnAliasType(IDxcPixType *MaybeAlias,
                           IDxcPixType **OriginalType) {
  *OriginalType = nullptr;
  CComPtr<IDxcPixType> Tmp(MaybeAlias);

  do {
    HRESULT hr;

    CComPtr<IDxcPixType> Alias;
    hr = Tmp->UnAlias(&Alias);
    if (FAILED(hr)) {
      return hr;
    }
    if (hr == S_FALSE) {
      break;
    }
    Tmp = Alias;
  } while (true);

  *OriginalType = Tmp.Detach();
  return S_OK;
}

class PixDiaTest {
public:
  BEGIN_TEST_CLASS(PixDiaTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_CLASS_SETUP(InitSupport);

  TEST_METHOD(CompileWhenDebugThenDIPresent)
  TEST_METHOD(CompileDebugPDB)

  TEST_METHOD(DiaLoadBadBitcodeThenFail)
  TEST_METHOD(DiaLoadDebugThenOK)
  TEST_METHOD(DiaTableIndexThenOK)
  TEST_METHOD(DiaLoadDebugSubrangeNegativeThenOK)
  TEST_METHOD(DiaLoadRelocatedBitcode)
  TEST_METHOD(DiaLoadBitcodePlusExtraData)
  TEST_METHOD(DiaCompileArgs)

  TEST_METHOD(PixTypeManager_InheritancePointerStruct)
  TEST_METHOD(PixTypeManager_InheritancePointerTypedef)
  TEST_METHOD(PixTypeManager_MatricesInBase)
  TEST_METHOD(PixTypeManager_SamplersAndResources)
  TEST_METHOD(PixTypeManager_XBoxDiaAssert)

  TEST_METHOD(PixDebugCompileInfo)

  TEST_METHOD(SymbolManager_Embedded2DArray)

  TEST_METHOD(
      DxcPixDxilDebugInfo_GlobalBackedGlobalStaticEmbeddedArrays_NoDbgValue)
  TEST_METHOD(
      DxcPixDxilDebugInfo_GlobalBackedGlobalStaticEmbeddedArrays_WithDbgValue)
  TEST_METHOD(
      DxcPixDxilDebugInfo_GlobalBackedGlobalStaticEmbeddedArrays_ArrayInValues)

  TEST_METHOD(DxcPixDxilDebugInfo_InstructionOffsets)
  TEST_METHOD(DxcPixDxilDebugInfo_InstructionOffsetsInClassMethods)
  TEST_METHOD(DxcPixDxilDebugInfo_DuplicateGlobals)
  TEST_METHOD(DxcPixDxilDebugInfo_StructInheritance)
  TEST_METHOD(DxcPixDxilDebugInfo_StructContainedResource)
  TEST_METHOD(DxcPixDxilDebugInfo_StructStaticInit)
  TEST_METHOD(DxcPixDxilDebugInfo_StructMemberFnFirst)
  TEST_METHOD(DxcPixDxilDebugInfo_UnnamedConstStruct)
  TEST_METHOD(DxcPixDxilDebugInfo_UnnamedStruct)
  TEST_METHOD(DxcPixDxilDebugInfo_UnnamedArray)
  TEST_METHOD(DxcPixDxilDebugInfo_UnnamedField)
  TEST_METHOD(DxcPixDxilDebugInfo_SubProgramsInNamespaces)
  TEST_METHOD(DxcPixDxilDebugInfo_SubPrograms)
  TEST_METHOD(DxcPixDxilDebugInfo_Alignment_ConstInt)
  TEST_METHOD(DxcPixDxilDebugInfo_QIOldFieldInterface)
  TEST_METHOD(DxcPixDxilDebugInfo_BitFields_Simple)
  TEST_METHOD(DxcPixDxilDebugInfo_BitFields_Derived)
  TEST_METHOD(DxcPixDxilDebugInfo_BitFields_Bool)
  TEST_METHOD(DxcPixDxilDebugInfo_BitFields_Overlap)
  TEST_METHOD(DxcPixDxilDebugInfo_BitFields_uint64)
  TEST_METHOD(DxcPixDxilDebugInfo_Min16SizesAndOffsets_Enabled)
  TEST_METHOD(DxcPixDxilDebugInfo_Min16SizesAndOffsets_Disabled)
  TEST_METHOD(DxcPixDxilDebugInfo_Min16VectorOffsets_Enabled)
  TEST_METHOD(DxcPixDxilDebugInfo_Min16VectorOffsets_Disabled)
  TEST_METHOD(
      DxcPixDxilDebugInfo_VariableScopes_InlinedFunctions_TwiceInlinedFunctions)
  TEST_METHOD(
      DxcPixDxilDebugInfo_VariableScopes_InlinedFunctions_CalledTwiceInSameCaller)
  TEST_METHOD(DxcPixDxilDebugInfo_VariableScopes_ForScopes)
  TEST_METHOD(DxcPixDxilDebugInfo_VariableScopes_ScopeBraces)
  TEST_METHOD(DxcPixDxilDebugInfo_VariableScopes_Function)
  TEST_METHOD(DxcPixDxilDebugInfo_VariableScopes_Member)

  dxc::DxCompilerDllLoader m_dllSupport;
  VersionSupportInfo m_ver;

  void RunSubProgramsCase(const char *hlsl);
  void TestUnnamedTypeCase(const char *hlsl, const wchar_t *expectedTypeName);

  template <typename T, typename TDefault, typename TIface>
  void WriteIfValue(TIface *pSymbol, std::wstringstream &o,
                    TDefault defaultValue, LPCWSTR valueLabel,
                    HRESULT (__stdcall TIface::*pFn)(T *)) {
    T value;
    HRESULT hr = (pSymbol->*(pFn))(&value);
    if (SUCCEEDED(hr) && value != (T)defaultValue) {
      o << L", " << valueLabel << L": " << value;
    }
  }

  template <typename TIface>
  void WriteIfValue(TIface *pSymbol, std::wstringstream &o, LPCWSTR valueLabel,
                    HRESULT (__stdcall TIface::*pFn)(BSTR *)) {
    CComBSTR value;
    HRESULT hr = (pSymbol->*(pFn))(&value);
    if (SUCCEEDED(hr) && value.Length()) {
      o << L", " << valueLabel << L": " << (LPCWSTR)value;
    }
  }
  template <typename TIface>
  void WriteIfValue(TIface *pSymbol, std::wstringstream &o, LPCWSTR valueLabel,
                    HRESULT (__stdcall TIface::*pFn)(VARIANT *)) {
    CComVariant value;
    HRESULT hr = (pSymbol->*(pFn))(&value);
    if (SUCCEEDED(hr) && value.vt != VT_NULL && value.vt != VT_EMPTY) {
      if (SUCCEEDED(value.ChangeType(VT_BSTR))) {
        o << L", " << valueLabel << L": " << (LPCWSTR)value.bstrVal;
      }
    }
  }
  template <typename TIface>
  void WriteIfValue(TIface *pSymbol, std::wstringstream &o, LPCWSTR valueLabel,
                    HRESULT (__stdcall TIface::*pFn)(IDiaSymbol **)) {
    CComPtr<IDiaSymbol> value;
    HRESULT hr = (pSymbol->*(pFn))(&value);
    if (SUCCEEDED(hr) && value.p != nullptr) {
      DWORD symId;
      value->get_symIndexId(&symId);
      o << L", " << valueLabel << L": id=" << symId;
    }
  }

  std::wstring GetDebugInfoAsText(IDiaDataSource *pDataSource) {
    CComPtr<IDiaSession> pSession;
    CComPtr<IDiaTable> pTable;
    CComPtr<IDiaEnumTables> pEnumTables;
    std::wstringstream o;

    VERIFY_SUCCEEDED(pDataSource->openSession(&pSession));
    VERIFY_SUCCEEDED(pSession->getEnumTables(&pEnumTables));
    LONG count;
    VERIFY_SUCCEEDED(pEnumTables->get_Count(&count));
    for (LONG i = 0; i < count; ++i) {
      pTable.Release();
      ULONG fetched;
      VERIFY_SUCCEEDED(pEnumTables->Next(1, &pTable, &fetched));
      VERIFY_ARE_EQUAL(fetched, 1u);
      CComBSTR tableName;
      VERIFY_SUCCEEDED(pTable->get_name(&tableName));
      o << L"Table: " << (LPWSTR)tableName << std::endl;
      LONG rowCount;
      IFT(pTable->get_Count(&rowCount));
      o << L" Row count: " << rowCount << std::endl;

      for (LONG rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
        CComPtr<IUnknown> item;
        o << L'#' << rowIndex;
        IFT(pTable->Item(rowIndex, &item));
        CComPtr<IDiaSymbol> pSymbol;
        if (SUCCEEDED(item.QueryInterface(&pSymbol))) {
          DWORD symTag;
          DWORD dataKind;
          DWORD locationType;
          DWORD registerId;
          pSymbol->get_symTag(&symTag);
          pSymbol->get_dataKind(&dataKind);
          pSymbol->get_locationType(&locationType);
          pSymbol->get_registerId(&registerId);
          // pSymbol->get_value(&value);

          WriteIfValue(pSymbol.p, o, 0, L"symIndexId",
                       &IDiaSymbol::get_symIndexId);
          o << L", " << SymTagEnumText[symTag];
          if (dataKind != 0)
            o << L", " << DataKindText[dataKind];
          WriteIfValue(pSymbol.p, o, L"name", &IDiaSymbol::get_name);
          WriteIfValue(pSymbol.p, o, L"lexicalParent",
                       &IDiaSymbol::get_lexicalParent);
          WriteIfValue(pSymbol.p, o, L"type", &IDiaSymbol::get_type);
          WriteIfValue(pSymbol.p, o, 0, L"slot", &IDiaSymbol::get_slot);
          WriteIfValue(pSymbol.p, o, 0, L"platform", &IDiaSymbol::get_platform);
          WriteIfValue(pSymbol.p, o, 0, L"language", &IDiaSymbol::get_language);
          WriteIfValue(pSymbol.p, o, 0, L"frontEndMajor",
                       &IDiaSymbol::get_frontEndMajor);
          WriteIfValue(pSymbol.p, o, 0, L"frontEndMinor",
                       &IDiaSymbol::get_frontEndMinor);
          WriteIfValue(pSymbol.p, o, 0, L"token", &IDiaSymbol::get_token);
          WriteIfValue(pSymbol.p, o, L"value", &IDiaSymbol::get_value);
          WriteIfValue(pSymbol.p, o, 0, L"code", &IDiaSymbol::get_code);
          WriteIfValue(pSymbol.p, o, 0, L"function", &IDiaSymbol::get_function);
          WriteIfValue(pSymbol.p, o, 0, L"udtKind", &IDiaSymbol::get_udtKind);
          WriteIfValue(pSymbol.p, o, 0, L"hasDebugInfo",
                       &IDiaSymbol::get_hasDebugInfo);
          WriteIfValue(pSymbol.p, o, L"compilerName",
                       &IDiaSymbol::get_compilerName);
          WriteIfValue(pSymbol.p, o, 0, L"isLocationControlFlowDependent",
                       &IDiaSymbol::get_isLocationControlFlowDependent);
          WriteIfValue(pSymbol.p, o, 0, L"numberOfRows",
                       &IDiaSymbol::get_numberOfRows);
          WriteIfValue(pSymbol.p, o, 0, L"numberOfColumns",
                       &IDiaSymbol::get_numberOfColumns);
          WriteIfValue(pSymbol.p, o, 0, L"length", &IDiaSymbol::get_length);
          WriteIfValue(pSymbol.p, o, 0, L"isMatrixRowMajor",
                       &IDiaSymbol::get_isMatrixRowMajor);
          WriteIfValue(pSymbol.p, o, 0, L"builtInKind",
                       &IDiaSymbol::get_builtInKind);
          WriteIfValue(pSymbol.p, o, 0, L"textureSlot",
                       &IDiaSymbol::get_textureSlot);
          WriteIfValue(pSymbol.p, o, 0, L"memorySpaceKind",
                       &IDiaSymbol::get_memorySpaceKind);
          WriteIfValue(pSymbol.p, o, 0, L"isHLSLData",
                       &IDiaSymbol::get_isHLSLData);
        }

        CComPtr<IDiaSourceFile> pSourceFile;
        if (SUCCEEDED(item.QueryInterface(&pSourceFile))) {
          WriteIfValue(pSourceFile.p, o, 0, L"uniqueId",
                       &IDiaSourceFile::get_uniqueId);
          WriteIfValue(pSourceFile.p, o, L"fileName",
                       &IDiaSourceFile::get_fileName);
        }

        CComPtr<IDiaLineNumber> pLineNumber;
        if (SUCCEEDED(item.QueryInterface(&pLineNumber))) {
          WriteIfValue(pLineNumber.p, o, L"compiland",
                       &IDiaLineNumber::get_compiland);
          // WriteIfValue(pLineNumber.p, o, L"sourceFile",
          // &IDiaLineNumber::get_sourceFile);
          WriteIfValue(pLineNumber.p, o, 0, L"lineNumber",
                       &IDiaLineNumber::get_lineNumber);
          WriteIfValue(pLineNumber.p, o, 0, L"lineNumberEnd",
                       &IDiaLineNumber::get_lineNumberEnd);
          WriteIfValue(pLineNumber.p, o, 0, L"columnNumber",
                       &IDiaLineNumber::get_columnNumber);
          WriteIfValue(pLineNumber.p, o, 0, L"columnNumberEnd",
                       &IDiaLineNumber::get_columnNumberEnd);
          WriteIfValue(pLineNumber.p, o, 0, L"addressSection",
                       &IDiaLineNumber::get_addressSection);
          WriteIfValue(pLineNumber.p, o, 0, L"addressOffset",
                       &IDiaLineNumber::get_addressOffset);
          WriteIfValue(pLineNumber.p, o, 0, L"relativeVirtualAddress",
                       &IDiaLineNumber::get_relativeVirtualAddress);
          WriteIfValue(pLineNumber.p, o, 0, L"virtualAddress",
                       &IDiaLineNumber::get_virtualAddress);
          WriteIfValue(pLineNumber.p, o, 0, L"length",
                       &IDiaLineNumber::get_length);
          WriteIfValue(pLineNumber.p, o, 0, L"sourceFileId",
                       &IDiaLineNumber::get_sourceFileId);
          WriteIfValue(pLineNumber.p, o, 0, L"statement",
                       &IDiaLineNumber::get_statement);
          WriteIfValue(pLineNumber.p, o, 0, L"compilandId",
                       &IDiaLineNumber::get_compilandId);
        }

        CComPtr<IDiaSectionContrib> pSectionContrib;
        if (SUCCEEDED(item.QueryInterface(&pSectionContrib))) {
          WriteIfValue(pSectionContrib.p, o, L"compiland",
                       &IDiaSectionContrib::get_compiland);
          WriteIfValue(pSectionContrib.p, o, 0, L"addressSection",
                       &IDiaSectionContrib::get_addressSection);
          WriteIfValue(pSectionContrib.p, o, 0, L"addressOffset",
                       &IDiaSectionContrib::get_addressOffset);
          WriteIfValue(pSectionContrib.p, o, 0, L"relativeVirtualAddress",
                       &IDiaSectionContrib::get_relativeVirtualAddress);
          WriteIfValue(pSectionContrib.p, o, 0, L"virtualAddress",
                       &IDiaSectionContrib::get_virtualAddress);
          WriteIfValue(pSectionContrib.p, o, 0, L"length",
                       &IDiaSectionContrib::get_length);
          WriteIfValue(pSectionContrib.p, o, 0, L"notPaged",
                       &IDiaSectionContrib::get_notPaged);
          WriteIfValue(pSectionContrib.p, o, 0, L"code",
                       &IDiaSectionContrib::get_code);
          WriteIfValue(pSectionContrib.p, o, 0, L"initializedData",
                       &IDiaSectionContrib::get_initializedData);
          WriteIfValue(pSectionContrib.p, o, 0, L"uninitializedData",
                       &IDiaSectionContrib::get_uninitializedData);
          WriteIfValue(pSectionContrib.p, o, 0, L"remove",
                       &IDiaSectionContrib::get_remove);
          WriteIfValue(pSectionContrib.p, o, 0, L"comdat",
                       &IDiaSectionContrib::get_comdat);
          WriteIfValue(pSectionContrib.p, o, 0, L"discardable",
                       &IDiaSectionContrib::get_discardable);
          WriteIfValue(pSectionContrib.p, o, 0, L"notCached",
                       &IDiaSectionContrib::get_notCached);
          WriteIfValue(pSectionContrib.p, o, 0, L"share",
                       &IDiaSectionContrib::get_share);
          WriteIfValue(pSectionContrib.p, o, 0, L"execute",
                       &IDiaSectionContrib::get_execute);
          WriteIfValue(pSectionContrib.p, o, 0, L"read",
                       &IDiaSectionContrib::get_read);
          WriteIfValue(pSectionContrib.p, o, 0, L"write",
                       &IDiaSectionContrib::get_write);
          WriteIfValue(pSectionContrib.p, o, 0, L"dataCrc",
                       &IDiaSectionContrib::get_dataCrc);
          WriteIfValue(pSectionContrib.p, o, 0, L"relocationsCrc",
                       &IDiaSectionContrib::get_relocationsCrc);
          WriteIfValue(pSectionContrib.p, o, 0, L"compilandId",
                       &IDiaSectionContrib::get_compilandId);
        }

        CComPtr<IDiaSegment> pSegment;
        if (SUCCEEDED(item.QueryInterface(&pSegment))) {
          WriteIfValue(pSegment.p, o, 0, L"frame", &IDiaSegment::get_frame);
          WriteIfValue(pSegment.p, o, 0, L"offset", &IDiaSegment::get_offset);
          WriteIfValue(pSegment.p, o, 0, L"length", &IDiaSegment::get_length);
          WriteIfValue(pSegment.p, o, 0, L"read", &IDiaSegment::get_read);
          WriteIfValue(pSegment.p, o, 0, L"write", &IDiaSegment::get_write);
          WriteIfValue(pSegment.p, o, 0, L"execute", &IDiaSegment::get_execute);
          WriteIfValue(pSegment.p, o, 0, L"addressSection",
                       &IDiaSegment::get_addressSection);
          WriteIfValue(pSegment.p, o, 0, L"relativeVirtualAddress",
                       &IDiaSegment::get_relativeVirtualAddress);
          WriteIfValue(pSegment.p, o, 0, L"virtualAddress",
                       &IDiaSegment::get_virtualAddress);
        }

        CComPtr<IDiaInjectedSource> pInjectedSource;
        if (SUCCEEDED(item.QueryInterface(&pInjectedSource))) {
          WriteIfValue(pInjectedSource.p, o, 0, L"crc",
                       &IDiaInjectedSource::get_crc);
          WriteIfValue(pInjectedSource.p, o, 0, L"length",
                       &IDiaInjectedSource::get_length);
          WriteIfValue(pInjectedSource.p, o, L"filename",
                       &IDiaInjectedSource::get_filename);
          WriteIfValue(pInjectedSource.p, o, L"objectFilename",
                       &IDiaInjectedSource::get_objectFilename);
          WriteIfValue(pInjectedSource.p, o, L"virtualFilename",
                       &IDiaInjectedSource::get_virtualFilename);
          WriteIfValue(pInjectedSource.p, o, 0, L"sourceCompression",
                       &IDiaInjectedSource::get_sourceCompression);
          // get_source is also available
        }

        CComPtr<IDiaFrameData> pFrameData;
        if (SUCCEEDED(item.QueryInterface(&pFrameData))) {
        }

        o << std::endl;
      }
    }

    return o.str();
  }
  std::wstring GetDebugFileContent(IDiaDataSource *pDataSource) {
    CComPtr<IDiaSession> pSession;
    CComPtr<IDiaTable> pTable;

    CComPtr<IDiaTable> pSourcesTable;

    CComPtr<IDiaEnumTables> pEnumTables;
    std::wstringstream o;

    VERIFY_SUCCEEDED(pDataSource->openSession(&pSession));
    VERIFY_SUCCEEDED(pSession->getEnumTables(&pEnumTables));

    ULONG fetched = 0;
    while (pEnumTables->Next(1, &pTable, &fetched) == S_OK && fetched == 1) {
      CComBSTR name;
      IFT(pTable->get_name(&name));

      if (wcscmp(name, L"SourceFiles") == 0) {
        pSourcesTable = pTable.Detach();
        continue;
      }

      pTable.Release();
    }

    if (!pSourcesTable) {
      return L"cannot find source";
    }

    // Get source file contents.
    // NOTE: "SourceFiles" has the root file first while "InjectedSources" is in
    // alphabetical order.
    //       It is important to keep the root file first for recompilation, so
    //       iterate "SourceFiles" and look up the corresponding injected
    //       source.
    LONG count;
    IFT(pSourcesTable->get_Count(&count));

    CComPtr<IDiaSourceFile> pSourceFile;
    CComBSTR pName;
    CComPtr<IUnknown> pSymbolUnk;
    CComPtr<IDiaEnumInjectedSources> pEnumInjectedSources;
    CComPtr<IDiaInjectedSource> pInjectedSource;
    std::wstring sourceText, sourceFilename;

    while (SUCCEEDED(pSourcesTable->Next(1, &pSymbolUnk, &fetched)) &&
           fetched == 1) {
      sourceText = sourceFilename = L"";

      IFT(pSymbolUnk->QueryInterface(&pSourceFile));
      IFT(pSourceFile->get_fileName(&pName));

      IFT(pSession->findInjectedSource(pName, &pEnumInjectedSources));

      if (SUCCEEDED(pEnumInjectedSources->get_Count(&count)) && count == 1) {
        IFT(pEnumInjectedSources->Item(0, &pInjectedSource));

        DWORD cbData = 0;
        std::string tempString;
        CComBSTR bstr;
        IFT(pInjectedSource->get_filename(&bstr));
        IFT(pInjectedSource->get_source(0, &cbData, nullptr));

        tempString.resize(cbData);
        IFT(pInjectedSource->get_source(
            cbData, &cbData, reinterpret_cast<BYTE *>(&tempString[0])));

        CA2W tempWString(tempString.data());
        o << tempWString.m_psz;
      }
      pSymbolUnk.Release();
    }

    return o.str();
  }

  struct LineNumber {
    DWORD line;
    DWORD rva;
  };
  std::vector<LineNumber>
  ReadLineNumbers(IDiaEnumLineNumbers *pEnumLineNumbers) {
    std::vector<LineNumber> lines;
    CComPtr<IDiaLineNumber> pLineNumber;
    DWORD lineCount;
    while (SUCCEEDED(pEnumLineNumbers->Next(1, &pLineNumber, &lineCount)) &&
           lineCount == 1) {
      DWORD line;
      DWORD rva;
      VERIFY_SUCCEEDED(pLineNumber->get_lineNumber(&line));
      VERIFY_SUCCEEDED(pLineNumber->get_relativeVirtualAddress(&rva));
      lines.push_back({line, rva});
      pLineNumber.Release();
    }
    return lines;
  }

  HRESULT CreateDiaSourceForCompile(const char *hlsl,
                                    IDiaDataSource **ppDiaSource) {
    if (!ppDiaSource)
      return E_POINTER;

    CComPtr<IDxcCompiler> pCompiler;
    CComPtr<IDxcOperationResult> pResult;
    CComPtr<IDxcBlobEncoding> pSource;
    CComPtr<IDxcBlob> pProgram;

    VERIFY_SUCCEEDED(CreateCompiler(m_dllSupport, &pCompiler));
    CreateBlobFromText(m_dllSupport, hlsl, &pSource);
    LPCWSTR args[] = {L"/Zi", L"/Qembed_debug", L"/Od"};
    VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                        L"ps_6_0", args, _countof(args),
                                        nullptr, 0, nullptr, &pResult));

    HRESULT compilationStatus;
    VERIFY_SUCCEEDED(pResult->GetStatus(&compilationStatus));
    if (FAILED(compilationStatus)) {
      CComPtr<IDxcBlobEncoding> pErrros;
      VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&pErrros));
      CA2W errorTextW(static_cast<const char *>(pErrros->GetBufferPointer()),
                      CP_UTF8);
      WEX::Logging::Log::Error(errorTextW);
    }

    VERIFY_SUCCEEDED(pResult->GetResult(&pProgram));

    // Disassemble the compiled (stripped) program.
    {
      CComPtr<IDxcBlobEncoding> pDisassembly;
      VERIFY_SUCCEEDED(pCompiler->Disassemble(pProgram, &pDisassembly));
      std::string disText = BlobToUtf8(pDisassembly);
      CA2W disTextW(disText.c_str());
      // WEX::Logging::Log::Comment(disTextW);
    }

    auto annotated = WrapInNewContainer(
        m_dllSupport, RunAnnotationPasses(m_dllSupport, pProgram).blob);

    // CONSIDER: have the dia data source look for the part if passed a whole
    // container.
    CComPtr<IDiaDataSource> pDiaSource;
    CComPtr<IStream> pProgramStream;
    CComPtr<IDxcLibrary> pLib;
    VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));
    const hlsl::DxilContainerHeader *pContainer = hlsl::IsDxilContainerLike(
        annotated->GetBufferPointer(), annotated->GetBufferSize());
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
        annotated, pBitcode - (char *)annotated->GetBufferPointer(),
        bitcodeLength, &pProgramPdb));

    // Disassemble the program with debug information.
    {
      CComPtr<IDxcBlobEncoding> pDbgDisassembly;
      VERIFY_SUCCEEDED(pCompiler->Disassemble(pProgramPdb, &pDbgDisassembly));
      std::string disText = BlobToUtf8(pDbgDisassembly);
      CA2W disTextW(disText.c_str());
      // WEX::Logging::Log::Comment(disTextW);
    }

    // Create a short text dump of debug information.
    VERIFY_SUCCEEDED(
        pLib->CreateStreamFromBlobReadOnly(pProgramPdb, &pProgramStream));
    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcDiaDataSource, &pDiaSource));
    VERIFY_SUCCEEDED(pDiaSource->loadDataFromIStream(pProgramStream));
    *ppDiaSource = pDiaSource.Detach();
    return S_OK;
  }

  void CompileAndRunAnnotationAndGetDebugPart(
      dxc::DllLoader &dllSupport, const char *source, const wchar_t *profile,
      IDxcIncludeHandler *includer, IDxcBlob **ppDebugPart,
      std::vector<const wchar_t *> extraArgs = {L"-Od"});
  void CompileAndRunAnnotationAndLoadDiaSource(
      dxc::DllLoader &dllSupport, const char *source, const wchar_t *profile,
      IDxcIncludeHandler *includer, IDiaDataSource **ppDataSource,
      std::vector<const wchar_t *> extraArgs = {L"-Od"});

  struct VariableComponentInfo {
    std::wstring Name;
    std::wstring Type;
  };

  void TestGlobalStaticCase(
      const char *hlsl, const wchar_t *profile,
      const char *lineAtWhichToExamineVariables,
      std::vector<VariableComponentInfo> const &ExpectedVariables);
  CComPtr<IDxcPixDxilStorage>
  RunSizeAndOffsetTestCase(const char *hlsl,
                           std::array<DWORD, 4> const &memberOffsets,
                           std::array<DWORD, 4> const &memberSizes,
                           std::vector<const wchar_t *> extraArgs = {L"-Od"});
  void RunVectorSizeAndOffsetTestCase(const char *hlsl,
                                      std::array<DWORD, 4> const &memberOffsets,
                                      std::vector<const wchar_t *> extraArgs = {
                                          L"-Od"});
  DebuggerInterfaces
  CompileAndCreateDxcDebug(const char *hlsl, const wchar_t *profile,
                           IDxcIncludeHandler *includer = nullptr,
                           std::vector<const wchar_t *> extraArgs = {L"-Od"});

  CComPtr<IDxcPixDxilLiveVariables>
  GetLiveVariablesAt(const char *hlsl,
                     const char *lineAtWhichToExamineVariables,
                     IDxcPixDxilDebugInfo *dxilDebugger);
};

static bool AddStorageComponents(
    IDxcPixDxilStorage *pStorage, std::wstring Name,
    std::vector<PixDiaTest::VariableComponentInfo> &VariableComponents) {
  CComPtr<IDxcPixType> StorageType;
  if (FAILED(pStorage->GetType(&StorageType))) {
    return false;
  }

  CComPtr<IDxcPixType> UnAliasedType;
  if (FAILED(UnAliasType(StorageType, &UnAliasedType))) {
    return false;
  }

  CComPtr<IDxcPixArrayType> ArrayType;
  CComPtr<IDxcPixScalarType> ScalarType;
  CComPtr<IDxcPixStructType2> StructType;

  if (!FAILED(UnAliasedType->QueryInterface(&ScalarType))) {
    CComBSTR TypeName;
    // StorageType is the type that the storage was defined in HLSL, i.e.,
    // it could be a typedef, const etc.
    if (FAILED(StorageType->GetName(&TypeName))) {
      return false;
    }

    VariableComponents.emplace_back(PixDiaTest::VariableComponentInfo{
        std::move(Name), std::wstring(TypeName)});
    return true;
  } else if (!FAILED(UnAliasedType->QueryInterface(&ArrayType))) {
    DWORD NumElements;
    if (FAILED(ArrayType->GetNumElements(&NumElements))) {
      return false;
    }

    std::wstring BaseName = Name + L'[';
    for (DWORD i = 0; i < NumElements; ++i) {
      CComPtr<IDxcPixDxilStorage> EntryStorage;
      if (FAILED(pStorage->Index(i, &EntryStorage))) {
        return false;
      }

      if (!AddStorageComponents(EntryStorage,
                                BaseName + std::to_wstring(i) + L"]",
                                VariableComponents)) {
        return false;
      }
    }
  } else if (!FAILED(UnAliasedType->QueryInterface(&StructType))) {
    DWORD NumFields;
    if (FAILED(StructType->GetNumFields(&NumFields))) {
      return false;
    }

    std::wstring BaseName = Name + L'.';
    for (DWORD i = 0; i < NumFields; ++i) {
      CComPtr<IDxcPixStructField> Field;
      if (FAILED(StructType->GetFieldByIndex(i, &Field))) {
        return false;
      }

      CComBSTR FieldName;
      if (FAILED(Field->GetName(&FieldName))) {
        return false;
      }

      CComPtr<IDxcPixDxilStorage> FieldStorage;
      if (FAILED(pStorage->AccessField(FieldName, &FieldStorage))) {
        return false;
      }

      if (!AddStorageComponents(FieldStorage,
                                BaseName + std::wstring(FieldName),
                                VariableComponents)) {
        return false;
      }
    }

    CComPtr<IDxcPixType> BaseType;
    if (SUCCEEDED(StructType->GetBaseType(&BaseType))) {
      CComPtr<IDxcPixDxilStorage> BaseStorage;
      if (FAILED(pStorage->AccessField(L"", &BaseStorage))) {
        return false;
      }
      if (!AddStorageComponents(BaseStorage, Name, VariableComponents)) {
        return false;
      }
    }
  }

  return true;
}

bool PixDiaTest::InitSupport() {
  if (!m_dllSupport.IsEnabled()) {
    VERIFY_SUCCEEDED(m_dllSupport.Initialize());
    m_ver.Initialize(m_dllSupport);
  }
  return true;
}

void PixDiaTest::CompileAndRunAnnotationAndGetDebugPart(
    dxc::DllLoader &dllSupport, const char *source, const wchar_t *profile,
    IDxcIncludeHandler *includer, IDxcBlob **ppDebugPart,
    std::vector<const wchar_t *> extraArgs) {

  CComPtr<IDxcBlob> pContainer;
  std::vector<LPCWSTR> args;
  args.push_back(L"/Zi");
  args.push_back(L"/Qembed_debug");
  args.insert(args.end(), extraArgs.begin(), extraArgs.end());

  CompileAndLogErrors(dllSupport, source, profile, args, includer, &pContainer);

  auto annotated = RunAnnotationPasses(m_dllSupport, pContainer);

  CComPtr<IDxcBlob> pNewContainer =
      WrapInNewContainer(m_dllSupport, annotated.blob);

  *ppDebugPart = GetDebugPart(dllSupport, pNewContainer).Detach();
}

DebuggerInterfaces
PixDiaTest::CompileAndCreateDxcDebug(const char *hlsl, const wchar_t *profile,
                                     IDxcIncludeHandler *includer,
                                     std::vector<const wchar_t *> extraArgs) {

  CComPtr<IDiaDataSource> pDiaDataSource;
  CompileAndRunAnnotationAndLoadDiaSource(m_dllSupport, hlsl, profile, includer,
                                          &pDiaDataSource, extraArgs);

  CComPtr<IDiaSession> session;
  VERIFY_SUCCEEDED(pDiaDataSource->openSession(&session));

  CComPtr<IDxcPixDxilDebugInfoFactory> Factory;
  VERIFY_SUCCEEDED(session->QueryInterface(IID_PPV_ARGS(&Factory)));

  DebuggerInterfaces ret;
  VERIFY_SUCCEEDED(Factory->NewDxcPixDxilDebugInfo(&ret.debugInfo));
  VERIFY_SUCCEEDED(Factory->NewDxcPixCompilationInfo(&ret.compilationInfo));
  return ret;
}

CComPtr<IDxcPixDxilLiveVariables>
PixDiaTest::GetLiveVariablesAt(const char *hlsl,
                               const char *lineAtWhichToExamineVariables,
                               IDxcPixDxilDebugInfo *dxilDebugger) {

  auto lines = SplitAndPreserveEmptyLines(std::string(hlsl), '\n');
  auto FindInterestingLine = std::find_if(
      lines.begin(), lines.end(),
      [&lineAtWhichToExamineVariables](std::string const &line) {
        return line.find(lineAtWhichToExamineVariables) != std::string::npos;
      });
  auto InterestingLine =
      static_cast<DWORD>(FindInterestingLine - lines.begin()) + 1;

  CComPtr<IDxcPixDxilLiveVariables> liveVariables;
  for (; InterestingLine <= static_cast<DWORD>(lines.size());
       ++InterestingLine) {
    CComPtr<IDxcPixDxilInstructionOffsets> instructionOffsets;
    if (SUCCEEDED(dxilDebugger->InstructionOffsetsFromSourceLocation(
            defaultFilename, InterestingLine, 0, &instructionOffsets))) {
      if (instructionOffsets->GetCount() > 0) {
        auto instructionOffset = instructionOffsets->GetOffsetByIndex(0);
        if (SUCCEEDED(dxilDebugger->GetLiveVariablesAt(instructionOffset,
                                                       &liveVariables))) {
          break;
        }
      }
    }
  }
  VERIFY_IS_TRUE(liveVariables != nullptr);

  return liveVariables;
}

static bool
ContainedBy(std::vector<PixDiaTest::VariableComponentInfo> const &v1,
            std::vector<PixDiaTest::VariableComponentInfo> const &v2) {
  for (auto const &c1 : v1) {
    bool FoundThis = false;
    for (auto const &c2 : v2) {
      if (c1.Name == c2.Name && c1.Type == c2.Type) {
        FoundThis = true;
        break;
      }
    }
    if (!FoundThis) {
      return false;
    }
  }
  return true;
}

void PixDiaTest::TestGlobalStaticCase(
    const char *hlsl, const wchar_t *profile,
    const char *lineAtWhichToExamineVariables,
    std::vector<VariableComponentInfo> const &ExpectedVariables) {

  auto dxilDebugger = CompileAndCreateDxcDebug(hlsl, profile).debugInfo;

  auto liveVariables =
      GetLiveVariablesAt(hlsl, lineAtWhichToExamineVariables, dxilDebugger);

  DWORD count;
  VERIFY_SUCCEEDED(liveVariables->GetCount(&count));
  bool FoundGlobal = false;
  for (DWORD i = 0; i < count; ++i) {
    CComPtr<IDxcPixVariable> variable;
    VERIFY_SUCCEEDED(liveVariables->GetVariableByIndex(i, &variable));
    CComBSTR name;
    variable->GetName(&name);
    if (0 == wcscmp(name, L"global.globalStruct")) {
      FoundGlobal = true;
      CComPtr<IDxcPixDxilStorage> storage;
      VERIFY_SUCCEEDED(variable->GetStorage(&storage));
      std::vector<VariableComponentInfo> ActualVariableComponents;
      VERIFY_IS_TRUE(AddStorageComponents(storage, L"global.globalStruct",
                                          ActualVariableComponents));
      VERIFY_IS_TRUE(ContainedBy(ActualVariableComponents, ExpectedVariables));
      break;
    }
  }
  VERIFY_IS_TRUE(FoundGlobal);
}

TEST_F(PixDiaTest, CompileWhenDebugThenDIPresent) {
  // BUG: the first test written was of this form:
  // float4 local = 0; return local;
  //
  // However we get no numbers because of the _wrapper form
  // that exports the zero initialization from main into
  // a global can't be attributed to any particular location
  // within main, and everything in main is eventually folded away.
  //
  // Making the function do a bit more work by calling an intrinsic
  // helps this case.
  CComPtr<IDiaDataSource> pDiaSource;
  VERIFY_SUCCEEDED(CreateDiaSourceForCompile(
      "float4 main(float4 pos : SV_Position) : SV_Target {\r\n"
      "  float4 local = abs(pos);\r\n"
      "  return local;\r\n"
      "}",
      &pDiaSource));
  std::wstring diaDump = GetDebugInfoAsText(pDiaSource).c_str();
  // WEX::Logging::Log::Comment(GetDebugInfoAsText(pDiaSource).c_str());

  // Very basic tests - we have basic symbols, line numbers, and files with
  // sources.
  VERIFY_IS_NOT_NULL(wcsstr(diaDump.c_str(),
                            L"symIndexId: 5, CompilandEnv, name: hlslTarget, "
                            L"lexicalParent: id=2, value: ps_6_0"));
  VERIFY_IS_NOT_NULL(wcsstr(diaDump.c_str(), L"lineNumber: 2"));
  VERIFY_IS_NOT_NULL(
      wcsstr(diaDump.c_str(), L"length: 99, filename: source.hlsl"));
  std::wstring diaFileContent = GetDebugFileContent(pDiaSource).c_str();
  VERIFY_IS_NOT_NULL(
      wcsstr(diaFileContent.c_str(),
             L"loat4 main(float4 pos : SV_Position) : SV_Target"));
#if SUPPORT_FXC_PDB
  // Now, fake it by loading from a .pdb!
  VERIFY_SUCCEEDED(CoInitializeEx(0, COINITBASE_MULTITHREADED));
  const wchar_t path[] = L"path-to-fxc-blob.bin";
  pDiaSource.Release();
  pProgramStream.Release();
  CComPtr<IDxcBlobEncoding> fxcBlob;
  CComPtr<IDxcBlob> pdbBlob;
  VERIFY_SUCCEEDED(pLib->CreateBlobFromFile(path, nullptr, &fxcBlob));
  std::string s = DumpParts(fxcBlob);
  CA2W sW(s.c_str());
  WEX::Logging::Log::Comment(sW);
  VERIFY_SUCCEEDED(CreateDiaSourceFromDxbcBlob(pLib, fxcBlob, &pDiaSource));
  WEX::Logging::Log::Comment(GetDebugInfoAsText(pDiaSource).c_str());
#endif
}

// Test that the new PDB format still works with Dia
TEST_F(PixDiaTest, CompileDebugPDB) {
  const char *hlsl = R"(
    [RootSignature("")]
    float main(float pos : A) : SV_Target {
      float x = abs(pos);
      float y = sin(pos);
      float z = x + y;
      return z;
    }
  )";
  CComPtr<IDxcLibrary> pLib;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));

  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcCompiler2> pCompiler2;

  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlob> pProgram;
  CComPtr<IDxcBlob> pPdbBlob;
  CComHeapPtr<WCHAR> pDebugName;

  VERIFY_SUCCEEDED(CreateCompiler(m_dllSupport, &pCompiler));
  VERIFY_SUCCEEDED(pCompiler.QueryInterface(&pCompiler2));
  CreateBlobFromText(m_dllSupport, hlsl, &pSource);
  LPCWSTR args[] = {L"/Zi", L"/Qembed_debug"};
  VERIFY_SUCCEEDED(pCompiler2->CompileWithDebug(
      pSource, L"source.hlsl", L"main", L"ps_6_0", args, _countof(args),
      nullptr, 0, nullptr, &pResult, &pDebugName, &pPdbBlob));
  VERIFY_SUCCEEDED(pResult->GetResult(&pProgram));

  CComPtr<IDiaDataSource> pDiaSource;
  CComPtr<IStream> pProgramStream;

  VERIFY_SUCCEEDED(
      pLib->CreateStreamFromBlobReadOnly(pPdbBlob, &pProgramStream));
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcDiaDataSource, &pDiaSource));
  VERIFY_SUCCEEDED(pDiaSource->loadDataFromIStream(pProgramStream));

  // Test that IDxcContainerReflection can consume a PDB container
  CComPtr<IDxcContainerReflection> pReflection;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcContainerReflection, &pReflection));
  VERIFY_SUCCEEDED(pReflection->Load(pPdbBlob));

  UINT32 uDebugInfoIndex = 0;
  VERIFY_SUCCEEDED(pReflection->FindFirstPartKind(
      hlsl::DFCC_ShaderDebugInfoDXIL, &uDebugInfoIndex));
}

TEST_F(PixDiaTest, DiaLoadBadBitcodeThenFail) {
  CComPtr<IDxcBlob> pBadBitcode;
  CComPtr<IDiaDataSource> pDiaSource;
  CComPtr<IStream> pStream;
  CComPtr<IDxcLibrary> pLib;

  Utf8ToBlob(m_dllSupport, "badcode", &pBadBitcode);
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));
  VERIFY_SUCCEEDED(pLib->CreateStreamFromBlobReadOnly(pBadBitcode, &pStream));
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcDiaDataSource, &pDiaSource));
  VERIFY_FAILED(pDiaSource->loadDataFromIStream(pStream));
}

static void CompileTestAndLoadDiaSource(dxc::DllLoader &dllSupport,
                                        const char *source,
                                        const wchar_t *profile,
                                        IDiaDataSource **ppDataSource) {
  CComPtr<IDxcBlob> pDebugContent;
  CComPtr<IStream> pStream;
  CComPtr<IDiaDataSource> pDiaSource;
  CComPtr<IDxcLibrary> pLib;
  VERIFY_SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));

  CompileAndGetDebugPart(dllSupport, source, profile, &pDebugContent);
  VERIFY_SUCCEEDED(pLib->CreateStreamFromBlobReadOnly(pDebugContent, &pStream));
  VERIFY_SUCCEEDED(
      dllSupport.CreateInstance(CLSID_DxcDiaDataSource, &pDiaSource));
  VERIFY_SUCCEEDED(pDiaSource->loadDataFromIStream(pStream));
  if (ppDataSource) {
    *ppDataSource = pDiaSource.Detach();
  }
}

static void CompileTestAndLoadDia(dxc::DllLoader &dllSupport,
                                  IDiaDataSource **ppDataSource) {
  CompileTestAndLoadDiaSource(dllSupport, "[numthreads(8,8,1)] void main() { }",
                              L"cs_6_0", ppDataSource);
}

TEST_F(PixDiaTest, DiaLoadDebugSubrangeNegativeThenOK) {
  static const char source[] = R"(
    SamplerState  samp0 : register(s0);
    Texture2DArray tex0 : register(t0);

    float4 foo(Texture2DArray textures[], int idx, SamplerState samplerState, float3 uvw) {
      return textures[NonUniformResourceIndex(idx)].Sample(samplerState, uvw);
    }

    [RootSignature( "DescriptorTable(SRV(t0)), DescriptorTable(Sampler(s0)) " )]
    float4 main(int index : INDEX, float3 uvw : TEXCOORD) : SV_Target {
      Texture2DArray textures[] = {
        tex0,
      };
      return foo(textures, index, samp0, uvw);
    }
  )";

  CComPtr<IDiaDataSource> pDiaDataSource;
  CComPtr<IDiaSession> pDiaSession;
  CompileTestAndLoadDiaSource(m_dllSupport, source, L"ps_6_0", &pDiaDataSource);

  VERIFY_SUCCEEDED(pDiaDataSource->openSession(&pDiaSession));
}

TEST_F(PixDiaTest, DiaLoadRelocatedBitcode) {

  static const char source[] = R"(
    SamplerState  samp0 : register(s0);
    Texture2DArray tex0 : register(t0);

    float4 foo(Texture2DArray textures[], int idx, SamplerState samplerState, float3 uvw) {
      return textures[NonUniformResourceIndex(idx)].Sample(samplerState, uvw);
    }

    [RootSignature( "DescriptorTable(SRV(t0)), DescriptorTable(Sampler(s0)) " )]
    float4 main(int index : INDEX, float3 uvw : TEXCOORD) : SV_Target {
      Texture2DArray textures[] = {
        tex0,
      };
      return foo(textures, index, samp0, uvw);
    }
  )";

  CComPtr<IDxcBlob> pPart;
  CComPtr<IDiaDataSource> pDiaSource;
  CComPtr<IStream> pStream;

  CComPtr<IDxcLibrary> pLib;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));

  CompileAndGetDebugPart(m_dllSupport, source, L"ps_6_0", &pPart);
  const char *pPartData = (char *)pPart->GetBufferPointer();

  // Get program header
  const hlsl::DxilProgramHeader *programHeader =
      (const hlsl::DxilProgramHeader *)pPartData;

  const char *pBitcode = nullptr;
  uint32_t uBitcodeSize = 0;
  hlsl::GetDxilProgramBitcode(programHeader, &pBitcode, &uBitcodeSize);
  VERIFY_IS_TRUE(uBitcodeSize % sizeof(UINT32) == 0);

  size_t uNewGapSize =
      4 * 10; // Size of some bytes between program header and bitcode
  size_t uNewSuffixeBytes =
      4 * 10; // Size of some random bytes after the program

  hlsl::DxilProgramHeader newProgramHeader = {};
  memcpy(&newProgramHeader, programHeader, sizeof(newProgramHeader));
  newProgramHeader.BitcodeHeader.BitcodeOffset =
      uNewGapSize + sizeof(newProgramHeader.BitcodeHeader);

  unsigned uNewSizeInBytes =
      sizeof(newProgramHeader) + uNewGapSize + uBitcodeSize + uNewSuffixeBytes;
  VERIFY_IS_TRUE(uNewSizeInBytes % sizeof(UINT32) == 0);
  newProgramHeader.SizeInUint32 = uNewSizeInBytes / sizeof(UINT32);

  llvm::SmallVector<char, 0> buffer;
  llvm::raw_svector_ostream OS(buffer);

  // Write the header
  OS.write((char *)&newProgramHeader, sizeof(newProgramHeader));

  // Write some garbage between the header and the bitcode
  for (unsigned i = 0; i < uNewGapSize; i++) {
    OS.write(0xFF);
  }

  // Write the actual bitcode
  OS.write(pBitcode, uBitcodeSize);

  // Write some garbage after the bitcode
  for (unsigned i = 0; i < uNewSuffixeBytes; i++) {
    OS.write(0xFF);
  }
  OS.flush();

  // Try to load this new program, make sure dia is still okay.
  CComPtr<IDxcBlobEncoding> pNewProgramBlob;
  VERIFY_SUCCEEDED(pLib->CreateBlobWithEncodingFromPinned(
      buffer.data(), buffer.size(), CP_ACP, &pNewProgramBlob));

  CComPtr<IStream> pNewProgramStream;
  VERIFY_SUCCEEDED(
      pLib->CreateStreamFromBlobReadOnly(pNewProgramBlob, &pNewProgramStream));

  CComPtr<IDiaDataSource> pDiaDataSource;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcDiaDataSource, &pDiaDataSource));

  VERIFY_SUCCEEDED(pDiaDataSource->loadDataFromIStream(pNewProgramStream));
}

TEST_F(PixDiaTest, DiaCompileArgs) {
  static const char source[] = R"(
    SamplerState  samp0 : register(s0);
    Texture2DArray tex0 : register(t0);

    float4 foo(Texture2DArray textures[], int idx, SamplerState samplerState, float3 uvw) {
      return textures[NonUniformResourceIndex(idx)].Sample(samplerState, uvw);
    }

    [RootSignature( "DescriptorTable(SRV(t0)), DescriptorTable(Sampler(s0)) " )]
    float4 main(int index : INDEX, float3 uvw : TEXCOORD) : SV_Target {
      Texture2DArray textures[] = {
        tex0,
      };
      return foo(textures, index, samp0, uvw);
    }
  )";

  CComPtr<IDxcBlob> pPart;
  CComPtr<IDiaDataSource> pDiaSource;
  CComPtr<IStream> pStream;

  CComPtr<IDxcLibrary> pLib;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));

  const WCHAR *FlagList[] = {
      L"/Zi",          L"-Zpr", L"/Qembed_debug",        L"/Fd",
      L"F:\\my dir\\", L"-Fo",  L"F:\\my dir\\file.dxc",
  };
  const WCHAR *DefineList[] = {
      L"MY_SPECIAL_DEFINE",
      L"MY_OTHER_SPECIAL_DEFINE=\"MY_STRING\"",
  };

  std::vector<LPCWSTR> args;
  for (unsigned i = 0; i < _countof(FlagList); i++) {
    args.push_back(FlagList[i]);
  }
  for (unsigned i = 0; i < _countof(DefineList); i++) {
    args.push_back(L"/D");
    args.push_back(DefineList[i]);
  }
  auto CompileAndGetDebugPart = [&args](dxc::DllLoader &dllSupport,
                                        const char *source,
                                        const wchar_t *profile,
                                        IDxcBlob **ppDebugPart) {
    CComPtr<IDxcBlob> pContainer;
    CComPtr<IDxcLibrary> pLib;
    CComPtr<IDxcContainerReflection> pReflection;
    UINT32 index;

    VerifyCompileOK(dllSupport, source, profile, args, &pContainer);
    VERIFY_SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));
    VERIFY_SUCCEEDED(
        dllSupport.CreateInstance(CLSID_DxcContainerReflection, &pReflection));
    VERIFY_SUCCEEDED(pReflection->Load(pContainer));
    VERIFY_SUCCEEDED(
        pReflection->FindFirstPartKind(hlsl::DFCC_ShaderDebugInfoDXIL, &index));
    VERIFY_SUCCEEDED(pReflection->GetPartContent(index, ppDebugPart));
  };
  CompileAndGetDebugPart(m_dllSupport, source, L"ps_6_0", &pPart);

  CComPtr<IStream> pNewProgramStream;
  VERIFY_SUCCEEDED(
      pLib->CreateStreamFromBlobReadOnly(pPart, &pNewProgramStream));

  CComPtr<IDiaDataSource> pDiaDataSource;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcDiaDataSource, &pDiaDataSource));

  VERIFY_SUCCEEDED(pDiaDataSource->loadDataFromIStream(pNewProgramStream));

  CComPtr<IDiaSession> pSession;
  VERIFY_SUCCEEDED(pDiaDataSource->openSession(&pSession));

  CComPtr<IDiaEnumTables> pEnumTables;
  VERIFY_SUCCEEDED(pSession->getEnumTables(&pEnumTables));

  CComPtr<IDiaTable> pSymbolTable;

  LONG uCount = 0;
  VERIFY_SUCCEEDED(pEnumTables->get_Count(&uCount));
  for (int i = 0; i < uCount; i++) {
    CComPtr<IDiaTable> pTable;
    VARIANT index = {};
    index.vt = VT_I4;
    index.intVal = i;
    VERIFY_SUCCEEDED(pEnumTables->Item(index, &pTable));

    CComBSTR pName;
    VERIFY_SUCCEEDED(pTable->get_name(&pName));

    if (pName == "Symbols") {
      pSymbolTable = pTable;
      break;
    }
  }

  std::wstring Args;
  std::wstring Entry;
  std::wstring Target;
  std::vector<std::wstring> Defines;
  std::vector<std::wstring> Flags;

  auto ReadNullSeparatedTokens = [](BSTR Str) -> std::vector<std::wstring> {
    std::vector<std::wstring> Result;
    while (*Str) {
      Result.push_back(std::wstring(Str));
      Str += wcslen(Str) + 1;
    }
    return Result;
  };

  VERIFY_SUCCEEDED(pSymbolTable->get_Count(&uCount));
  for (int i = 0; i < uCount; i++) {
    CComPtr<IUnknown> pSymbolUnk;
    CComPtr<IDiaSymbol> pSymbol;
    CComVariant pValue;
    CComBSTR pName;
    VERIFY_SUCCEEDED(pSymbolTable->Item(i, &pSymbolUnk));
    VERIFY_SUCCEEDED(pSymbolUnk->QueryInterface(&pSymbol));
    VERIFY_SUCCEEDED(pSymbol->get_name(&pName));
    VERIFY_SUCCEEDED(pSymbol->get_value(&pValue));
    if (pName == "hlslTarget") {
      if (pValue.vt == VT_BSTR)
        Target = pValue.bstrVal;
    } else if (pName == "hlslEntry") {
      if (pValue.vt == VT_BSTR)
        Entry = pValue.bstrVal;
    } else if (pName == "hlslFlags") {
      if (pValue.vt == VT_BSTR)
        Flags = ReadNullSeparatedTokens(pValue.bstrVal);
    } else if (pName == "hlslArguments") {
      if (pValue.vt == VT_BSTR)
        Args = pValue.bstrVal;
    } else if (pName == "hlslDefines") {
      if (pValue.vt == VT_BSTR)
        Defines = ReadNullSeparatedTokens(pValue.bstrVal);
    }
  }

  auto VectorContains = [](std::vector<std::wstring> &Tokens,
                           std::wstring Sub) {
    for (unsigned i = 0; i < Tokens.size(); i++) {
      if (Tokens[i].find(Sub) != std::wstring::npos)
        return true;
    }
    return false;
  };

  VERIFY_IS_TRUE(Target == L"ps_6_0");
  VERIFY_IS_TRUE(Entry == L"main");

  VERIFY_IS_TRUE(_countof(FlagList) == Flags.size());
  for (unsigned i = 0; i < _countof(FlagList); i++) {
    VERIFY_IS_TRUE(Flags[i] == FlagList[i]);
  }
  for (unsigned i = 0; i < _countof(DefineList); i++) {
    VERIFY_IS_TRUE(VectorContains(Defines, DefineList[i]));
  }
}

TEST_F(PixDiaTest, DiaLoadBitcodePlusExtraData) {
  // Test that dia doesn't crash when bitcode has unused extra data at the end

  static const char source[] = R"(
    SamplerState  samp0 : register(s0);
    Texture2DArray tex0 : register(t0);

    float4 foo(Texture2DArray textures[], int idx, SamplerState samplerState, float3 uvw) {
      return textures[NonUniformResourceIndex(idx)].Sample(samplerState, uvw);
    }

    [RootSignature( "DescriptorTable(SRV(t0)), DescriptorTable(Sampler(s0)) " )]
    float4 main(int index : INDEX, float3 uvw : TEXCOORD) : SV_Target {
      Texture2DArray textures[] = {
        tex0,
      };
      return foo(textures, index, samp0, uvw);
    }
  )";

  CComPtr<IDxcBlob> pPart;
  CComPtr<IDiaDataSource> pDiaSource;
  CComPtr<IStream> pStream;

  CComPtr<IDxcLibrary> pLib;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));

  CompileAndGetDebugPart(m_dllSupport, source, L"ps_6_0", &pPart);
  const char *pPartData = (char *)pPart->GetBufferPointer();

  // Get program header
  const hlsl::DxilProgramHeader *programHeader =
      (const hlsl::DxilProgramHeader *)pPartData;

  const char *pBitcode = nullptr;
  uint32_t uBitcodeSize = 0;
  hlsl::GetDxilProgramBitcode(programHeader, &pBitcode, &uBitcodeSize);

  llvm::SmallVector<char, 0> buffer;
  llvm::raw_svector_ostream OS(buffer);

  // Write the bitcode
  OS.write(pBitcode, uBitcodeSize);
  for (unsigned i = 0; i < 12; i++) {
    OS.write(0xFF);
  }
  OS.flush();

  // Try to load this new program, make sure dia is still okay.
  CComPtr<IDxcBlobEncoding> pNewProgramBlob;
  VERIFY_SUCCEEDED(pLib->CreateBlobWithEncodingFromPinned(
      buffer.data(), buffer.size(), CP_ACP, &pNewProgramBlob));

  CComPtr<IStream> pNewProgramStream;
  VERIFY_SUCCEEDED(
      pLib->CreateStreamFromBlobReadOnly(pNewProgramBlob, &pNewProgramStream));

  CComPtr<IDiaDataSource> pDiaDataSource;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcDiaDataSource, &pDiaDataSource));

  VERIFY_SUCCEEDED(pDiaDataSource->loadDataFromIStream(pNewProgramStream));
}

TEST_F(PixDiaTest, DiaLoadDebugThenOK) {
  CompileTestAndLoadDia(m_dllSupport, nullptr);
}

TEST_F(PixDiaTest, DiaTableIndexThenOK) {
  CComPtr<IDiaDataSource> pDiaSource;
  CComPtr<IDiaSession> pDiaSession;
  CComPtr<IDiaEnumTables> pEnumTables;
  CComPtr<IDiaTable> pTable;
  VARIANT vtIndex;
  CompileTestAndLoadDia(m_dllSupport, &pDiaSource);
  VERIFY_SUCCEEDED(pDiaSource->openSession(&pDiaSession));
  VERIFY_SUCCEEDED(pDiaSession->getEnumTables(&pEnumTables));

  vtIndex.vt = VT_EMPTY;
  VERIFY_FAILED(pEnumTables->Item(vtIndex, &pTable));

  vtIndex.vt = VT_I4;
  vtIndex.intVal = 1;
  VERIFY_SUCCEEDED(pEnumTables->Item(vtIndex, &pTable));
  VERIFY_IS_NOT_NULL(pTable.p);
  pTable.Release();

  vtIndex.vt = VT_UI4;
  vtIndex.uintVal = 1;
  VERIFY_SUCCEEDED(pEnumTables->Item(vtIndex, &pTable));
  VERIFY_IS_NOT_NULL(pTable.p);
  pTable.Release();

  vtIndex.uintVal = 100;
  VERIFY_FAILED(pEnumTables->Item(vtIndex, &pTable));
}

TEST_F(PixDiaTest, PixDebugCompileInfo) {
  static const char source[] = R"(
    SamplerState  samp0 : register(s0);
    Texture2DArray tex0 : register(t0);

    float4 foo(Texture2DArray textures[], int idx, SamplerState samplerState, float3 uvw) {
      return textures[NonUniformResourceIndex(idx)].Sample(samplerState, uvw);
    }

    [RootSignature( "DescriptorTable(SRV(t0)), DescriptorTable(Sampler(s0)) " )]
    float4 main(int index : INDEX, float3 uvw : TEXCOORD) : SV_Target {
      Texture2DArray textures[] = {
        tex0,
      };
      return foo(textures, index, samp0, uvw);
    }
  )";

  CComPtr<IDxcBlob> pPart;
  CComPtr<IDiaDataSource> pDiaSource;
  CComPtr<IStream> pStream;

  CComPtr<IDxcLibrary> pLib;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));

  const WCHAR *FlagList[] = {
      L"/Zi",          L"-Zpr", L"/Qembed_debug",        L"/Fd",
      L"F:\\my dir\\", L"-Fo",  L"F:\\my dir\\file.dxc",
  };
  const WCHAR *DefineList[] = {
      L"MY_SPECIAL_DEFINE",
      L"MY_OTHER_SPECIAL_DEFINE=\"MY_STRING\"",
  };

  std::vector<LPCWSTR> args;
  for (unsigned i = 0; i < _countof(FlagList); i++) {
    args.push_back(FlagList[i]);
  }
  for (unsigned i = 0; i < _countof(DefineList); i++) {
    args.push_back(L"/D");
    args.push_back(DefineList[i]);
  }

  auto CompileAndGetDebugPart = [&args](dxc::DllLoader &dllSupport,
                                        const char *source,
                                        const wchar_t *profile,
                                        IDxcBlob **ppDebugPart) {
    CComPtr<IDxcBlob> pContainer;
    CComPtr<IDxcLibrary> pLib;
    CComPtr<IDxcContainerReflection> pReflection;
    UINT32 index;

    VerifyCompileOK(dllSupport, source, profile, args, &pContainer);
    VERIFY_SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));
    VERIFY_SUCCEEDED(
        dllSupport.CreateInstance(CLSID_DxcContainerReflection, &pReflection));
    VERIFY_SUCCEEDED(pReflection->Load(pContainer));
    VERIFY_SUCCEEDED(
        pReflection->FindFirstPartKind(hlsl::DFCC_ShaderDebugInfoDXIL, &index));
    VERIFY_SUCCEEDED(pReflection->GetPartContent(index, ppDebugPart));
  };

  const wchar_t *profile = L"ps_6_0";
  CompileAndGetDebugPart(m_dllSupport, source, profile, &pPart);

  CComPtr<IStream> pNewProgramStream;
  VERIFY_SUCCEEDED(
      pLib->CreateStreamFromBlobReadOnly(pPart, &pNewProgramStream));

  CComPtr<IDiaDataSource> pDiaDataSource;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcDiaDataSource, &pDiaDataSource));

  VERIFY_SUCCEEDED(pDiaDataSource->loadDataFromIStream(pNewProgramStream));

  CComPtr<IDiaSession> pSession;
  VERIFY_SUCCEEDED(pDiaDataSource->openSession(&pSession));

  CComPtr<IDxcPixDxilDebugInfoFactory> factory;
  VERIFY_SUCCEEDED(pSession->QueryInterface(IID_PPV_ARGS(&factory)));

  CComPtr<IDxcPixCompilationInfo> compilationInfo;
  VERIFY_SUCCEEDED(factory->NewDxcPixCompilationInfo(&compilationInfo));

  CComBSTR arguments;
  VERIFY_SUCCEEDED(compilationInfo->GetArguments(&arguments));
  for (unsigned i = 0; i < _countof(FlagList); i++) {
    VERIFY_IS_TRUE(nullptr != wcsstr(arguments, FlagList[i]));
  }

  CComBSTR macros;
  VERIFY_SUCCEEDED(compilationInfo->GetMacroDefinitions(&macros));
  for (unsigned i = 0; i < _countof(DefineList); i++) {
    std::wstring MacroDef = std::wstring(L"-D") + DefineList[i];
    VERIFY_IS_TRUE(nullptr != wcsstr(macros, MacroDef.c_str()));
  }

  CComBSTR entryPointFile;
  VERIFY_SUCCEEDED(compilationInfo->GetEntryPointFile(&entryPointFile));
  VERIFY_ARE_EQUAL(std::wstring(L"source.hlsl"), std::wstring(entryPointFile));

  CComBSTR entryPointFunction;
  VERIFY_SUCCEEDED(compilationInfo->GetEntryPoint(&entryPointFunction));
  VERIFY_ARE_EQUAL(std::wstring(L"main"), std::wstring(entryPointFunction));

  CComBSTR hlslTarget;
  VERIFY_SUCCEEDED(compilationInfo->GetHlslTarget(&hlslTarget));
  VERIFY_ARE_EQUAL(std::wstring(profile), std::wstring(hlslTarget));
}

void PixDiaTest::CompileAndRunAnnotationAndLoadDiaSource(
    dxc::DllLoader &dllSupport, const char *source, const wchar_t *profile,
    IDxcIncludeHandler *includer, IDiaDataSource **ppDataSource,
    std::vector<const wchar_t *> extraArgs) {
  CComPtr<IDxcBlob> pDebugContent;
  CComPtr<IStream> pStream;
  CComPtr<IDiaDataSource> pDiaSource;
  CComPtr<IDxcLibrary> pLib;
  VERIFY_SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));

  CompileAndRunAnnotationAndGetDebugPart(dllSupport, source, profile, includer,
                                         &pDebugContent, extraArgs);
  VERIFY_SUCCEEDED(pLib->CreateStreamFromBlobReadOnly(pDebugContent, &pStream));
  VERIFY_SUCCEEDED(
      dllSupport.CreateInstance(CLSID_DxcDiaDataSource, &pDiaSource));
  VERIFY_SUCCEEDED(pDiaSource->loadDataFromIStream(pStream));
  if (ppDataSource) {
    *ppDataSource = pDiaSource.Detach();
  }
}

TEST_F(PixDiaTest, PixTypeManager_InheritancePointerStruct) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
struct Base
{
    float floatValue;
};

struct Derived : Base
{
	int intValue;
};

RaytracingAccelerationStructure Scene : register(t0, space0);

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0,0,0);
    ray.Direction = float3(0,0,1);
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    Derived payload;
    payload.floatValue = 1;
    payload.intValue = 2;
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);}

)";

  CComPtr<IDiaDataSource> pDiaDataSource;
  CComPtr<IDiaSession> pDiaSession;
  CompileAndRunAnnotationAndLoadDiaSource(m_dllSupport, hlsl, L"lib_6_6",
                                          nullptr, &pDiaDataSource);

  VERIFY_SUCCEEDED(pDiaDataSource->openSession(&pDiaSession));
}

TEST_F(PixDiaTest, PixTypeManager_MatricesInBase) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
struct Base
{
    float4x4 mat;
};
typedef Base BaseTypedef;

struct Derived : BaseTypedef
{
	int intValue;
};

RaytracingAccelerationStructure Scene : register(t0, space0);

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0,0,0);
    ray.Direction = float3(0,0,1);
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    Derived payload;
    payload.mat[0][0] = 1;
    payload.intValue = 2;
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);}

)";

  CComPtr<IDiaDataSource> pDiaDataSource;
  CComPtr<IDiaSession> pDiaSession;
  CompileAndRunAnnotationAndLoadDiaSource(m_dllSupport, hlsl, L"lib_6_6",
                                          nullptr, &pDiaDataSource);

  VERIFY_SUCCEEDED(pDiaDataSource->openSession(&pDiaSession));
}

TEST_F(PixDiaTest, PixTypeManager_SamplersAndResources) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(

static const SamplerState SamplerRef = SamplerDescriptorHeap[1];

Texture3D<uint4> Tex3DTemplated ;
Texture3D Tex3d ;
Texture2D Tex2D ;
Texture2D<uint> Tex2DTemplated ;
StructuredBuffer<float4> StructBuf ;
Texture2DArray Tex2DArray ;
Buffer<float4> Buff ;

static const struct
{
	float  AFloat;
	SamplerState Samp1;
	Texture3D<uint4> Tex1;
	Texture3D Tex2;
	Texture2D Tex3;
	Texture2D<uint> Tex4;
	StructuredBuffer<float4> Buff1;
	Texture2DArray Tex5;
	Buffer<float4> Buff2;
	float  AnotherFloat;
} View = {
1,
SamplerRef,
Tex3DTemplated,
Tex3d,
Tex2D,
Tex2DTemplated,
StructBuf,
Tex2DArray,
Buff,
2
};

struct Payload
{
	int intValue;
};

RaytracingAccelerationStructure Scene : register(t0, space0);

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0,0,0);
    ray.Direction = float3(0,0,1);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    Payload payload;
    payload.intValue = View.AFloat;
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);}
)";

  CComPtr<IDiaDataSource> pDiaDataSource;
  CComPtr<IDiaSession> pDiaSession;
  CompileAndRunAnnotationAndLoadDiaSource(m_dllSupport, hlsl, L"lib_6_6",
                                          nullptr, &pDiaDataSource);

  VERIFY_SUCCEEDED(pDiaDataSource->openSession(&pDiaSession));
}

TEST_F(PixDiaTest, PixTypeManager_XBoxDiaAssert) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
struct VSOut
{
    float4 vPosition : SV_POSITION;
    float4 vLightAndFog : COLOR0_center;
    float4 vTexCoords : TEXCOORD1;
};

struct HSPatchData
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

HSPatchData HSPatchFunc(const InputPatch<VSOut, 3> tri)
{

    float dist = (tri[0].vPosition.w + tri[1].vPosition.w + tri[2].vPosition.w) / 3;


    float tf = max(1, dist / 100.f);

    HSPatchData pd;
    pd.edges[0] = pd.edges[1] = pd.edges[2] = tf;
    pd.inside = tf;

    return pd;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HSPatchFunc")]
[outputcontrolpoints(3)]
[RootSignature("RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " "DescriptorTable(SRV(t0, numDescriptors=2), visibility=SHADER_VISIBILITY_ALL)," "DescriptorTable(Sampler(s0, numDescriptors=2), visibility=SHADER_VISIBILITY_PIXEL)," "DescriptorTable(CBV(b0, numDescriptors=1), visibility=SHADER_VISIBILITY_ALL)," "DescriptorTable(CBV(b1, numDescriptors=1), visibility=SHADER_VISIBILITY_ALL)," "DescriptorTable(CBV(b2, numDescriptors=1), visibility=SHADER_VISIBILITY_ALL)," "DescriptorTable(SRV(t3, numDescriptors=1), visibility=SHADER_VISIBILITY_ALL)," "DescriptorTable(UAV(u9, numDescriptors=2), visibility=SHADER_VISIBILITY_ALL),")]
VSOut main( const uint id : SV_OutputControlPointID,
              const InputPatch< VSOut, 3 > triIn )
{
    return triIn[id];
}
)";

  CComPtr<IDiaDataSource> pDiaDataSource;
  CComPtr<IDiaSession> pDiaSession;
  CompileAndRunAnnotationAndLoadDiaSource(m_dllSupport, hlsl, L"hs_6_0",
                                          nullptr, &pDiaDataSource);

  VERIFY_SUCCEEDED(pDiaDataSource->openSession(&pDiaSession));
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_InstructionOffsets) {

  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl =
      R"(RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);

struct SceneConstantBuffer
{
    float4x4 projectionToWorld;
    float4 cameraPosition;
    float4 lightPosition;
    float4 lightAmbientColor;
    float4 lightDiffuseColor;
};

ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);

struct RayPayload
{
    float4 color;
};

inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy;// / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a ray.
    float4 world = /*mul(*/float4(screenPos, 0, 1)/*, g_sceneCB.projectionToWorld)*/;

    //world.xyz /= world.w;
    origin = world.xyz; //g_sceneCB.cameraPosition.xyz;
    direction = float3(1,0,0);//normalize(world.xyz - origin);
}

void RaygenCommon()
{
    float3 rayDir;
    float3 origin;

    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    GenerateCameraRay(DispatchRaysIndex().xy, origin, rayDir);

    // Trace the ray.
    // Set the ray's extents.
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) };
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);

    // Write the raytraced color to the output texture.
   // RenderTarget[DispatchRaysIndex().xy] = payload.color;
}

[shader("raygeneration")]
void Raygen()
{
    RaygenCommon();
}

typedef BuiltInTriangleIntersectionAttributes MyAttributes;

namespace ANameSpace
{
    namespace AContainedNamespace
    {
        float4 RoundaboutWayToReturnAmbientColor()
        {
            return g_sceneCB.lightAmbientColor;
        }
    }
}


[shader("closesthit")]
void InnerClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    payload.color = ANameSpace::AContainedNamespace::RoundaboutWayToReturnAmbientColor();
}


[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    payload.color = float4(1, 0, 0, 0);
})";

  auto lines = SplitAndPreserveEmptyLines(std::string(hlsl), '\n');
  DWORD countOfSourceLines = static_cast<DWORD>(lines.size());

  CComPtr<IDiaDataSource> pDiaDataSource;
  CompileAndRunAnnotationAndLoadDiaSource(m_dllSupport, hlsl, L"lib_6_6",
                                          nullptr, &pDiaDataSource);

  CComPtr<IDiaSession> session;
  VERIFY_SUCCEEDED(pDiaDataSource->openSession(&session));

  CComPtr<IDxcPixDxilDebugInfoFactory> Factory;
  VERIFY_SUCCEEDED(session->QueryInterface(IID_PPV_ARGS(&Factory)));

  CComPtr<IDxcPixDxilDebugInfo> dxilDebugger;
  VERIFY_SUCCEEDED(Factory->NewDxcPixDxilDebugInfo(&dxilDebugger));

  // Quick crash test for wrong filename:
  CComPtr<IDxcPixDxilInstructionOffsets> garbageOffsets;
  dxilDebugger->InstructionOffsetsFromSourceLocation(L"garbage", 0, 0,
                                                     &garbageOffsets);

  // Since the API offers both source-from-instruction and
  // instruction-from-source, we'll compare them against each other:
  for (size_t line = 0; line < lines.size(); ++line) {

    auto lineNumber = static_cast<DWORD>(line);

    constexpr DWORD sourceLocationReaderOnlySupportsColumnZero = 0;
    CComPtr<IDxcPixDxilInstructionOffsets> offsets;
    dxilDebugger->InstructionOffsetsFromSourceLocation(
        defaultFilename, lineNumber, sourceLocationReaderOnlySupportsColumnZero,
        &offsets);

    auto offsetCount = offsets->GetCount();
    for (DWORD offsetOrdinal = 0; offsetOrdinal < offsetCount;
         ++offsetOrdinal) {

      DWORD instructionOffsetFromSource =
          offsets->GetOffsetByIndex(offsetOrdinal);

      CComPtr<IDxcPixDxilSourceLocations> sourceLocations;
      VERIFY_SUCCEEDED(dxilDebugger->SourceLocationsFromInstructionOffset(
          instructionOffsetFromSource, &sourceLocations));

      auto count = sourceLocations->GetCount();
      for (DWORD sourceLocationOrdinal = 0; sourceLocationOrdinal < count;
           ++sourceLocationOrdinal) {
        DWORD lineNumber =
            sourceLocations->GetLineNumberByIndex(sourceLocationOrdinal);
        DWORD column = sourceLocations->GetColumnByIndex(sourceLocationOrdinal);
        CComBSTR filename;
        VERIFY_SUCCEEDED(sourceLocations->GetFileNameByIndex(
            sourceLocationOrdinal, &filename));

        VERIFY_IS_TRUE(lineNumber < countOfSourceLines);

        constexpr DWORD lineNumbersAndColumnsStartAtOne = 1;
        VERIFY_IS_TRUE(
            column - lineNumbersAndColumnsStartAtOne <=
            static_cast<DWORD>(
                lines.at(lineNumber - lineNumbersAndColumnsStartAtOne).size()));
        VERIFY_IS_TRUE(0 == wcscmp(filename, defaultFilename));
      }
    }
  }
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_InstructionOffsetsInClassMethods) {

  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
RWByteAddressBuffer RawUAV: register(u1);

class AClass
{
  float Saturate(float f) // StartClassMethod
  {
    float l = RawUAV.Load(0);
    return saturate(f * l);
  } //EndClassMethod
};

[numthreads(1, 1, 1)]
void main()
{
    uint orig;
    AClass aClass;
    float i = orig;
    float f = aClass.Saturate(i);
    uint fi = (uint)f;
    RawUAV.InterlockedAdd(0, 42, fi);
}

)";

  auto lines = SplitAndPreserveEmptyLines(std::string(hlsl), '\n');

  CComPtr<IDiaDataSource> pDiaDataSource;
  CompileAndRunAnnotationAndLoadDiaSource(m_dllSupport, hlsl, L"cs_6_6",
                                          nullptr, &pDiaDataSource);

  CComPtr<IDiaSession> session;
  VERIFY_SUCCEEDED(pDiaDataSource->openSession(&session));

  CComPtr<IDxcPixDxilDebugInfoFactory> Factory;
  VERIFY_SUCCEEDED(session->QueryInterface(IID_PPV_ARGS(&Factory)));

  CComPtr<IDxcPixDxilDebugInfo> dxilDebugger;
  VERIFY_SUCCEEDED(Factory->NewDxcPixDxilDebugInfo(&dxilDebugger));

  size_t lineAfterMethod = 0;
  size_t lineBeforeMethod = static_cast<size_t>(-1);
  for (size_t line = 0; line < lines.size(); ++line) {
    if (lines[line].find("StartClassMethod") != std::string::npos)
      lineBeforeMethod = line;
    if (lines[line].find("EndClassMethod") != std::string::npos)
      lineAfterMethod = line;
  }

  VERIFY_IS_TRUE(lineAfterMethod > lineBeforeMethod);

  // For each source line, get the instruction numbers.
  // For each instruction number, map back to source line.
  // Some of them better be in the class method

  bool foundClassMethodLines = false;

  for (size_t line = 0; line < lines.size(); ++line) {

    auto lineNumber = static_cast<DWORD>(line);

    constexpr DWORD sourceLocationReaderOnlySupportsColumnZero = 0;
    CComPtr<IDxcPixDxilInstructionOffsets> offsets;
    dxilDebugger->InstructionOffsetsFromSourceLocation(
        defaultFilename, lineNumber, sourceLocationReaderOnlySupportsColumnZero,
        &offsets);

    auto offsetCount = offsets->GetCount();
    for (DWORD offsetOrdinal = 0; offsetOrdinal < offsetCount;
         ++offsetOrdinal) {

      DWORD instructionOffsetFromSource =
          offsets->GetOffsetByIndex(offsetOrdinal);

      CComPtr<IDxcPixDxilSourceLocations> sourceLocations;
      VERIFY_SUCCEEDED(dxilDebugger->SourceLocationsFromInstructionOffset(
          instructionOffsetFromSource, &sourceLocations));

      auto count = sourceLocations->GetCount();
      for (DWORD sourceLocationOrdinal = 0; sourceLocationOrdinal < count;
           ++sourceLocationOrdinal) {
        DWORD lineNumber =
            sourceLocations->GetLineNumberByIndex(sourceLocationOrdinal);

        if (lineNumber >= lineBeforeMethod && lineNumber <= lineAfterMethod) {
          foundClassMethodLines = true;
        }
      }
    }
  }

  VERIFY_IS_TRUE(foundClassMethodLines);
}

TEST_F(PixDiaTest, PixTypeManager_InheritancePointerTypedef) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
struct Base
{
    float floatValue;
};
typedef Base BaseTypedef;

struct Derived : BaseTypedef
{
	int intValue;
};

RaytracingAccelerationStructure Scene : register(t0, space0);

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0,0,0);
    ray.Direction = float3(0,0,1);
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    Derived payload;
    payload.floatValue = 1;
    payload.intValue = 2;
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);}

)";

  CComPtr<IDiaDataSource> pDiaDataSource;
  CComPtr<IDiaSession> pDiaSession;
  CompileAndRunAnnotationAndLoadDiaSource(m_dllSupport, hlsl, L"lib_6_6",
                                          nullptr, &pDiaDataSource);

  VERIFY_SUCCEEDED(pDiaDataSource->openSession(&pDiaSession));
}

TEST_F(PixDiaTest, SymbolManager_Embedded2DArray) {
  const char *code = R"x(
struct EmbeddedStruct
{
    uint32_t TwoDArray[2][2];
};

struct smallPayload
{
    uint32_t OneInt;
    EmbeddedStruct embeddedStruct;
    uint64_t bigOne;
};

[numthreads(1, 1, 1)]
void ASMain()
{
    smallPayload p;
    p.OneInt = -137;
    p.embeddedStruct.TwoDArray[0][0] = 252;
    p.embeddedStruct.TwoDArray[0][1] = 253;
    p.embeddedStruct.TwoDArray[1][0] = 254;
    p.embeddedStruct.TwoDArray[1][1] = 255;
    p.bigOne = 123456789;

    DispatchMesh(2, 1, 1, p);
}

)x";

  auto compiled = Compile(m_dllSupport, code, L"as_6_5", {}, L"ASMain");

  auto debugPart = GetDebugPart(
      m_dllSupport,
      WrapInNewContainer(m_dllSupport,
                         RunAnnotationPasses(m_dllSupport, compiled).blob));

  CComPtr<IDxcLibrary> library;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &library));

  CComPtr<IStream> programStream;
  VERIFY_SUCCEEDED(
      library->CreateStreamFromBlobReadOnly(debugPart, &programStream));

  CComPtr<IDiaDataSource> diaDataSource;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcDiaDataSource, &diaDataSource));

  VERIFY_SUCCEEDED(diaDataSource->loadDataFromIStream(programStream));

  CComPtr<IDiaSession> session;
  VERIFY_SUCCEEDED(diaDataSource->openSession(&session));

  CComPtr<IDxcPixDxilDebugInfoFactory> Factory;
  VERIFY_SUCCEEDED(session->QueryInterface(&Factory));
  CComPtr<IDxcPixDxilDebugInfo> dxilDebugger;
  VERIFY_SUCCEEDED(Factory->NewDxcPixDxilDebugInfo(&dxilDebugger));

  auto lines = SplitAndPreserveEmptyLines(code, '\n');
  auto DispatchMeshLineFind =
      std::find_if(lines.begin(), lines.end(), [](std::string const &line) {
        return line.find("DispatchMesh") != std::string::npos;
      });
  auto DispatchMeshLine =
      static_cast<DWORD>(DispatchMeshLineFind - lines.begin()) + 2;

  CComPtr<IDxcPixDxilInstructionOffsets> instructionOffsets;
  VERIFY_SUCCEEDED(dxilDebugger->InstructionOffsetsFromSourceLocation(
      L"source.hlsl", DispatchMeshLine, 0, &instructionOffsets));
  VERIFY_IS_TRUE(instructionOffsets->GetCount() > 0);
  DWORD InstructionOrdinal = instructionOffsets->GetOffsetByIndex(0);
  CComPtr<IDxcPixDxilLiveVariables> liveVariables;
  VERIFY_SUCCEEDED(
      dxilDebugger->GetLiveVariablesAt(InstructionOrdinal, &liveVariables));
  CComPtr<IDxcPixVariable> variable;
  VERIFY_SUCCEEDED(liveVariables->GetVariableByIndex(0, &variable));
  CComBSTR name;
  variable->GetName(&name);
  VERIFY_ARE_EQUAL_WSTR(name, L"p");
  CComPtr<IDxcPixType> type;
  VERIFY_SUCCEEDED(variable->GetType(&type));
  CComPtr<IDxcPixStructType> structType;
  VERIFY_SUCCEEDED(type->QueryInterface(IID_PPV_ARGS(&structType)));
  auto ValidateStructMember = [&structType](DWORD index, const wchar_t *name,
                                            uint64_t offset) {
    CComPtr<IDxcPixStructField> member;
    VERIFY_SUCCEEDED(structType->GetFieldByIndex(index, &member));
    CComBSTR actualName;
    VERIFY_SUCCEEDED(member->GetName(&actualName));
    VERIFY_ARE_EQUAL_WSTR(actualName, name);
    DWORD actualOffset = 0;
    VERIFY_SUCCEEDED(member->GetOffsetInBits(&actualOffset));
    VERIFY_ARE_EQUAL(actualOffset, offset);
  };

  ValidateStructMember(0, L"OneInt", 0);
  ValidateStructMember(1, L"embeddedStruct", 4 * 8);
  ValidateStructMember(2, L"bigOne", 24 * 8);
}

TEST_F(PixDiaTest,
       DxcPixDxilDebugInfo_GlobalBackedGlobalStaticEmbeddedArrays_NoDbgValue) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
RWStructuredBuffer<float> floatRWUAV: register(u0);

struct GlobalStruct
{
    int IntArray[2];
    float FloatArray[2];
};

static GlobalStruct globalStruct;
[noinline]
void fn()
{
    float Accumulator;
    globalStruct.IntArray[0] = floatRWUAV[0];
    globalStruct.IntArray[1] = floatRWUAV[1];
    globalStruct.FloatArray[0] = floatRWUAV[4];
    globalStruct.FloatArray[1] = floatRWUAV[5];
    Accumulator = 0;

    uint killSwitch = 0;

    [loop] // do not unroll this
    while (true)
    {
        Accumulator += globalStruct.FloatArray[killSwitch % 2];

        if (killSwitch++ == 4) break;
    }

    floatRWUAV[0] = Accumulator + globalStruct.IntArray[0] + globalStruct.IntArray[1];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
  fn();
}

)";
  // The above HLSL should generate a module that represents the FloatArray
  // member as a global, and the IntArray as an alloca. Since only embedded
  // arrays are present in GlobalStruct, no dbg.value will be present for
  // globalStruct. We expect the value-to-declare pass to generate its own
  // dbg.value for stores into FloatArray. We will observe those dbg.value here
  // via the PIX-specific debug data API.

  std::vector<VariableComponentInfo> Expected;
  Expected.push_back({L"global.globalStruct.IntArray[0]", L"int"});
  Expected.push_back({L"global.globalStruct.IntArray[1]", L"int"});
  Expected.push_back({L"global.globalStruct.FloatArray[0]", L"float"});
  Expected.push_back({L"global.globalStruct.FloatArray[1]", L"float"});
  TestGlobalStaticCase(hlsl, L"lib_6_6", "float Accumulator", Expected);
}

TEST_F(
    PixDiaTest,
    DxcPixDxilDebugInfo_GlobalBackedGlobalStaticEmbeddedArrays_WithDbgValue) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
RWStructuredBuffer<float> floatRWUAV: register(u0);

struct GlobalStruct
{
    float Accumulator;
    int IntArray[2];
    float FloatArray[2];
};

static GlobalStruct globalStruct;
[numthreads(1, 1, 1)]
void main()
{
    globalStruct.IntArray[0] = floatRWUAV[0];
    globalStruct.IntArray[1] = floatRWUAV[1];
    globalStruct.FloatArray[0] = floatRWUAV[4];
    globalStruct.FloatArray[1] = floatRWUAV[5];
    globalStruct.Accumulator = 0;

    uint killSwitch = 0;

    [loop] // do not unroll this
    while (true)
    {
        globalStruct.Accumulator += globalStruct.FloatArray[killSwitch % 2];

        if (killSwitch++ == 4) break;
    }

    floatRWUAV[0] = globalStruct.Accumulator + globalStruct.IntArray[0] + globalStruct.IntArray[1];
}

)";
  // The above HLSL should generate a module that represents the FloatArray
  // member as a global, and the IntArray as an alloca. The presence of
  // Accumulator in the GlobalStruct will force a dbg.value to be present for
  // globalStruct. We expect the value-to-declare pass to find that dbg.value.

  std::vector<VariableComponentInfo> Expected;
  Expected.push_back({L"global.globalStruct.Accumulator", L"float"});
  Expected.push_back({L"global.globalStruct.IntArray[0]", L"int"});
  Expected.push_back({L"global.globalStruct.IntArray[1]", L"int"});
  Expected.push_back({L"global.globalStruct.FloatArray[0]", L"float"});
  Expected.push_back({L"global.globalStruct.FloatArray[1]", L"float"});
  TestGlobalStaticCase(hlsl, L"cs_6_6", "float Accumulator", Expected);
}

TEST_F(
    PixDiaTest,
    DxcPixDxilDebugInfo_GlobalBackedGlobalStaticEmbeddedArrays_ArrayInValues) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
RWStructuredBuffer<float> floatRWUAV: register(u0);

struct GlobalStruct
{
    float Accumulator;
    int IntArray[2];
    float FloatArray[2];
};

static GlobalStruct globalStruct;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    globalStruct.IntArray[0] =0;
    globalStruct.IntArray[1] =1;
    globalStruct.FloatArray[0] = floatRWUAV[4];
    globalStruct.FloatArray[1] = floatRWUAV[5];
    globalStruct.Accumulator = 0;

    uint killSwitch = 0;

    [loop] // do not unroll this
    while (true)
    {
        globalStruct.Accumulator += globalStruct.FloatArray[killSwitch % 2];

        if (killSwitch++ == 4) break;
    }

    floatRWUAV[0] = globalStruct.Accumulator + globalStruct.IntArray[0] + globalStruct.IntArray[1];
}

)";
  // The above HLSL should generate a module that represents the FloatArray
  // member as a global, and the IntArray as individual values.

  std::vector<VariableComponentInfo> Expected;
  Expected.push_back({L"global.globalStruct.Accumulator", L"float"});
  Expected.push_back({L"global.globalStruct.IntArray[0]", L"int"});
  Expected.push_back({L"global.globalStruct.IntArray[1]", L"int"});
  Expected.push_back({L"global.globalStruct.FloatArray[0]", L"float"});
  Expected.push_back({L"global.globalStruct.FloatArray[1]", L"float"});
  TestGlobalStaticCase(hlsl, L"lib_6_6", "float Accumulator", Expected);
}

int CountLiveGlobals(IDxcPixDxilLiveVariables *liveVariables) {
  int globalCount = 0;
  DWORD varCount;
  VERIFY_SUCCEEDED(liveVariables->GetCount(&varCount));
  for (DWORD i = 0; i < varCount; ++i) {
    CComPtr<IDxcPixVariable> var;
    VERIFY_SUCCEEDED(liveVariables->GetVariableByIndex(i, &var));
    CComBSTR name;
    VERIFY_SUCCEEDED(var->GetName(&name));
    if (wcsstr(name, L"global.") != nullptr)
      globalCount++;
  }
  return globalCount;
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_DuplicateGlobals) {
  if (m_ver.SkipDxilVersion(1, 6))
    return;

  const char *hlsl = R"(
static float global = 1.0;
struct RayPayload
{
    float4 color;
};
typedef BuiltInTriangleIntersectionAttributes MyAttributes;

[shader("closesthit")]
void InnerClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    payload.color = float4(global, 0, 0, 0); // CHLine
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    payload.color = float4(0, 1, 0, 0); // MSLine
})";

  auto dxilDebugger = CompileAndCreateDxcDebug(hlsl, L"lib_6_6").debugInfo;

  auto CHVars = GetLiveVariablesAt(hlsl, "CHLine", dxilDebugger);
  VERIFY_ARE_EQUAL(1, CountLiveGlobals(CHVars));
  auto MSVars = GetLiveVariablesAt(hlsl, "MSLine", dxilDebugger);
  VERIFY_ARE_EQUAL(0, CountLiveGlobals(MSVars));
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_StructInheritance) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
RWStructuredBuffer<float> floatRWUAV: register(u0);

struct AStruct
{
    float f;
    int i[2];
};

struct ADerivedStruct : AStruct
{
  bool b[2];
};

int AFunction(ADerivedStruct theStruct)
{
  return theStruct.i[0] + theStruct.i[1] + theStruct.f + (theStruct.b[0] ? 1 : 0) + (theStruct.b[1] ? 2 : 3); // InterestingLine
}

[numthreads(1, 1, 1)]
void main()
{
  ADerivedStruct aStruct;
  aStruct.f = floatRWUAV[1];
  aStruct.i[0] = floatRWUAV[2];
  aStruct.i[1] = floatRWUAV[3];
  aStruct.b[0] = floatRWUAV[4] != 0.0;
  aStruct.b[1] = floatRWUAV[5] != 0.0;
  floatRWUAV[0] = AFunction(aStruct);
}

)";
  auto dxilDebugger = CompileAndCreateDxcDebug(hlsl, L"cs_6_6").debugInfo;

  auto liveVariables =
      GetLiveVariablesAt(hlsl, "InterestingLine", dxilDebugger);

  DWORD count;
  VERIFY_SUCCEEDED(liveVariables->GetCount(&count));
  bool FoundTheStruct = false;
  for (DWORD i = 0; i < count; ++i) {
    CComPtr<IDxcPixVariable> variable;
    VERIFY_SUCCEEDED(liveVariables->GetVariableByIndex(i, &variable));
    CComBSTR name;
    variable->GetName(&name);
    if (0 == wcscmp(name, L"theStruct")) {
      FoundTheStruct = true;
      CComPtr<IDxcPixDxilStorage> storage;
      VERIFY_SUCCEEDED(variable->GetStorage(&storage));
      std::vector<VariableComponentInfo> ActualVariableComponents;
      VERIFY_IS_TRUE(AddStorageComponents(storage, L"theStruct",
                                          ActualVariableComponents));
      std::vector<VariableComponentInfo> Expected;
      Expected.push_back({L"theStruct.b[0]", L"bool"});
      Expected.push_back({L"theStruct.b[1]", L"bool"});
      Expected.push_back({L"theStruct.f", L"float"});
      Expected.push_back({L"theStruct.i[0]", L"int"});
      Expected.push_back({L"theStruct.i[1]", L"int"});
      VERIFY_IS_TRUE(ContainedBy(ActualVariableComponents, Expected));
      break;
    }
  }
  VERIFY_IS_TRUE(FoundTheStruct);
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_StructContainedResource) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
RWStructuredBuffer<float> floatRWUAV: register(u0);
Texture2D srv2DTexture : register(t0, space1);
struct AStruct
{
    float f;
    Texture2D tex;
};

float4 AFunction(AStruct theStruct)
{
  return theStruct.tex.Load(int3(0, 0, 0)) + theStruct.f.xxxx; // InterestingLine
}

[numthreads(1, 1, 1)]
void main()
{
  AStruct aStruct;
  aStruct.f = floatRWUAV[1];
  aStruct.tex = srv2DTexture;
  floatRWUAV[0] = AFunction(aStruct).x;
}

)";
  auto dxilDebugger = CompileAndCreateDxcDebug(hlsl, L"cs_6_6").debugInfo;

  auto liveVariables =
      GetLiveVariablesAt(hlsl, "InterestingLine", dxilDebugger);

  DWORD count;
  VERIFY_SUCCEEDED(liveVariables->GetCount(&count));
  bool FoundTheStruct = false;
  for (DWORD i = 0; i < count; ++i) {
    CComPtr<IDxcPixVariable> variable;
    VERIFY_SUCCEEDED(liveVariables->GetVariableByIndex(i, &variable));
    CComBSTR name;
    variable->GetName(&name);
    if (0 == wcscmp(name, L"theStruct")) {
      FoundTheStruct = true;
      CComPtr<IDxcPixDxilStorage> storage;
      VERIFY_SUCCEEDED(variable->GetStorage(&storage));
      std::vector<VariableComponentInfo> ActualVariableComponents;
      VERIFY_IS_TRUE(AddStorageComponents(storage, L"theStruct",
                                          ActualVariableComponents));
      std::vector<VariableComponentInfo> Expected;
      Expected.push_back({L"theStruct.f", L"float"});
      VERIFY_IS_TRUE(ContainedBy(ActualVariableComponents, Expected));
      break;
    }
  }
  VERIFY_IS_TRUE(FoundTheStruct);
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_StructStaticInit) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
RWStructuredBuffer<float> floatRWUAV: register(u0);
struct AStruct
{
    float f;
    static AStruct Init(float fi)
    {
        AStruct ret;
        ret.f = fi;
        for(int i =0; i < 4; ++i)
        {
          ret.f += floatRWUAV[i+2];
        }
        return ret;
    }
};

[numthreads(1, 1, 1)]
void main()
{
  AStruct aStruct = AStruct::Init(floatRWUAV[1]);
  floatRWUAV[0] = aStruct.f; // InterestingLine
}

)";
  auto dxilDebugger = CompileAndCreateDxcDebug(hlsl, L"cs_6_6").debugInfo;

  auto liveVariables =
      GetLiveVariablesAt(hlsl, "InterestingLine", dxilDebugger);

  DWORD count;
  VERIFY_SUCCEEDED(liveVariables->GetCount(&count));
  bool FoundTheStruct = false;
  for (DWORD i = 0; i < count; ++i) {
    CComPtr<IDxcPixVariable> variable;
    VERIFY_SUCCEEDED(liveVariables->GetVariableByIndex(i, &variable));
    CComBSTR name;
    variable->GetName(&name);
    if (0 == wcscmp(name, L"aStruct")) {
      FoundTheStruct = true;
      CComPtr<IDxcPixDxilStorage> storage;
      VERIFY_SUCCEEDED(variable->GetStorage(&storage));
      std::vector<VariableComponentInfo> ActualVariableComponents;
      VERIFY_IS_TRUE(
          AddStorageComponents(storage, L"aStruct", ActualVariableComponents));
      std::vector<VariableComponentInfo> Expected;
      Expected.push_back({L"aStruct.f", L"float"});
      VERIFY_IS_TRUE(ContainedBy(ActualVariableComponents, Expected));
      break;
    }
  }
  VERIFY_IS_TRUE(FoundTheStruct);
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_StructMemberFnFirst) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
RWStructuredBuffer<float> floatRWUAV: register(u0);
struct AStruct
{
    void Init(float fi);
    float f;
};

void AStruct::Init(float fi)
{
    AStruct ret;
    f = fi;
    for(int i =0; i < 4; ++i)
    {
      f += floatRWUAV[i+2];
    }
}

[numthreads(1, 1, 1)]
void main()
{
  AStruct aStruct;
  aStruct.Init(floatRWUAV[1]);
  floatRWUAV[0] = aStruct.f; // InterestingLine
}

)";
  auto dxilDebugger = CompileAndCreateDxcDebug(hlsl, L"cs_6_6").debugInfo;

  auto liveVariables =
      GetLiveVariablesAt(hlsl, "InterestingLine", dxilDebugger);

  DWORD count;
  VERIFY_SUCCEEDED(liveVariables->GetCount(&count));
  bool FoundTheStruct = false;
  for (DWORD i = 0; i < count; ++i) {
    CComPtr<IDxcPixVariable> variable;
    VERIFY_SUCCEEDED(liveVariables->GetVariableByIndex(i, &variable));
    CComBSTR name;
    variable->GetName(&name);
    if (0 == wcscmp(name, L"aStruct")) {
      FoundTheStruct = true;
      CComPtr<IDxcPixDxilStorage> storage;
      VERIFY_SUCCEEDED(variable->GetStorage(&storage));
      std::vector<VariableComponentInfo> ActualVariableComponents;
      VERIFY_IS_TRUE(
          AddStorageComponents(storage, L"aStruct", ActualVariableComponents));
      std::vector<VariableComponentInfo> Expected;
      Expected.push_back({L"aStruct.f", L"float"});
      VERIFY_IS_TRUE(ContainedBy(ActualVariableComponents, Expected));
      break;
    }
  }
  VERIFY_IS_TRUE(FoundTheStruct);
}

void PixDiaTest::TestUnnamedTypeCase(const char *hlsl,
                                     const wchar_t *expectedTypeName) {
  if (m_ver.SkipDxilVersion(1, 2))
    return;
  auto dxilDebugger = CompileAndCreateDxcDebug(hlsl, L"cs_6_0").debugInfo;
  auto liveVariables =
      GetLiveVariablesAt(hlsl, "InterestingLine", dxilDebugger);
  DWORD count;
  VERIFY_SUCCEEDED(liveVariables->GetCount(&count));
  bool FoundTheVariable = false;
  for (DWORD i = 0; i < count; ++i) {
    CComPtr<IDxcPixVariable> variable;
    VERIFY_SUCCEEDED(liveVariables->GetVariableByIndex(i, &variable));
    CComBSTR name;
    variable->GetName(&name);
    if (0 == wcscmp(name, L"glbl")) {
      FoundTheVariable = true;
      CComPtr<IDxcPixType> type;
      VERIFY_SUCCEEDED(variable->GetType(&type));
      CComBSTR typeName;
      VERIFY_SUCCEEDED(type->GetName(&typeName));
      VERIFY_ARE_EQUAL(typeName, expectedTypeName);
      break;
    }
  }
  VERIFY_IS_TRUE(FoundTheVariable);
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_UnnamedConstStruct) {
  const char *hlsl = R"(
RWStructuredBuffer<float> floatRWUAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  const struct
  {
    float fg;
    RWStructuredBuffer<float> buf;
  } glbl = {42.f, floatRWUAV};

  float f = glbl.fg + glbl.buf[1]; // InterestingLine
  floatRWUAV[0] = f;
}

)";

  TestUnnamedTypeCase(hlsl, L"const <unnamed>");
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_UnnamedStruct) {
  const char *hlsl = R"(
RWStructuredBuffer<float> floatRWUAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  struct
  {
    float fg;
    RWStructuredBuffer<float> buf;
  } glbl = {42.f, floatRWUAV};
  glbl.fg = 41.f;
  float f = glbl.fg + glbl.buf[1]; // InterestingLine
  floatRWUAV[0] = f;
}

)";

  TestUnnamedTypeCase(hlsl, L"<unnamed>");
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_UnnamedArray) {
  const char *hlsl = R"(
RWStructuredBuffer<float> floatRWUAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  struct
  {
    float fg;
    RWStructuredBuffer<float> buf;
  } glbl[2] = {{42.f, floatRWUAV},{43.f, floatRWUAV}};
  float f = glbl[0].fg + glbl[1].buf[1]; // InterestingLine
  floatRWUAV[0] = f;
}

)";

  TestUnnamedTypeCase(hlsl, L"<unnamed>[]");
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_UnnamedField) {
  const char *hlsl = R"(
RWStructuredBuffer<float> floatRWUAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  struct
  {
    struct {
      float fg;
      RWStructuredBuffer<float> buf;
    } contained;
  } glbl = { {42.f, floatRWUAV} };
  float f = glbl.contained.fg + glbl.contained.buf[1]; // InterestingLine
  floatRWUAV[0] = f;
}

)";

  if (m_ver.SkipDxilVersion(1, 2))
    return;
  auto dxilDebugger = CompileAndCreateDxcDebug(hlsl, L"cs_6_0").debugInfo;
  auto liveVariables =
      GetLiveVariablesAt(hlsl, "InterestingLine", dxilDebugger);
  DWORD count;
  VERIFY_SUCCEEDED(liveVariables->GetCount(&count));
  bool FoundTheVariable = false;
  for (DWORD i = 0; i < count; ++i) {
    CComPtr<IDxcPixVariable> variable;
    VERIFY_SUCCEEDED(liveVariables->GetVariableByIndex(i, &variable));
    CComBSTR name;
    variable->GetName(&name);
    if (0 == wcscmp(name, L"glbl")) {
      CComPtr<IDxcPixType> type;
      VERIFY_SUCCEEDED(variable->GetType(&type));
      CComPtr<IDxcPixStructType> structType;
      VERIFY_SUCCEEDED(type->QueryInterface(IID_PPV_ARGS(&structType)));
      DWORD fieldCount = 0;
      VERIFY_SUCCEEDED(structType->GetNumFields(&fieldCount));
      VERIFY_ARE_EQUAL(fieldCount, 1u);
      // Just a crash test:
      CComPtr<IDxcPixStructField> structField;
      structType->GetFieldByName(L"", &structField);
      VERIFY_SUCCEEDED(structType->GetFieldByIndex(0, &structField));
      FoundTheVariable = true;
      CComPtr<IDxcPixType> fieldType;
      VERIFY_SUCCEEDED(structField->GetType(&fieldType));
      CComBSTR typeName;
      VERIFY_SUCCEEDED(fieldType->GetName(&typeName));
      VERIFY_ARE_EQUAL(typeName, L"<unnamed>");
      break;
    }
  }
  VERIFY_IS_TRUE(FoundTheVariable);
}

class DxcIncludeHandlerForInjectedSourcesForPix : public IDxcIncludeHandler {
private:
  DXC_MICROCOM_REF_FIELD(m_dwRef)

  std::vector<std::pair<std::wstring, std::string>> m_files;
  PixDiaTest *m_pixTest;

public:
  DXC_MICROCOM_ADDREF_RELEASE_IMPL(m_dwRef)
  DxcIncludeHandlerForInjectedSourcesForPix(
      PixDiaTest *pixTest,
      std::vector<std::pair<std::wstring, std::string>> files)
      : m_dwRef(0), m_files(files), m_pixTest(pixTest){};

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcIncludeHandler>(this, iid, ppvObject);
  }

  HRESULT insertIncludeFile(LPCWSTR pFilename, IDxcBlobEncoding *pBlob,
                            UINT32 dataLen) {
    return E_FAIL;
  }

  HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR pFilename,
                                       IDxcBlob **ppIncludeSource) override {
    for (auto const &file : m_files) {
      std::wstring prependedWithDotHack = L".\\" + file.first;
      if (prependedWithDotHack == std::wstring(pFilename)) {
        CComPtr<IDxcBlobEncoding> blob;
        CreateBlobFromText(m_pixTest->m_dllSupport, file.second.c_str(), &blob);
        *ppIncludeSource = blob.Detach();
        return S_OK;
      }
    }
    return E_FAIL;
  }
};

void PixDiaTest::RunSubProgramsCase(const char *hlsl) {
  CComPtr<DxcIncludeHandlerForInjectedSourcesForPix> pIncludeHandler =
      new DxcIncludeHandlerForInjectedSourcesForPix(
          this, {{L"..\\include1\\samefilename.h",
                  "float fn1(int c, float v) { for(int i = 0; i< c; ++ i) v += "
                  "sqrt(v); return v; } "},
                 {L"..\\include2\\samefilename.h",
                  R"(
float4 fn2( float3 f3, float d, bool sanitize = true )
{
  if (sanitize)
  {
    f3 = any(isnan(f3) | isinf(f3)) ? 0 : clamp(f3, 0, 1034.f);
    d = (isnan(d) | isinf(d)) ? 0 : clamp(d, 0, 1024.f); 
  }
  if( d != 0 ) d = max( d, -1024.f);
  return float4( f3, d );}
)"}});

  auto dxilDebugger =
      CompileAndCreateDxcDebug(hlsl, L"cs_6_0", pIncludeHandler).debugInfo;

  struct SourceLocations {
    CComBSTR Filename;
    DWORD Column;
    DWORD Line;
  };

  std::vector<SourceLocations> sourceLocations;

  DWORD instructionOffset = 0;
  CComPtr<IDxcPixDxilSourceLocations> DxcSourceLocations;
  while (SUCCEEDED(dxilDebugger->SourceLocationsFromInstructionOffset(
      instructionOffset++, &DxcSourceLocations))) {
    auto count = DxcSourceLocations->GetCount();
    for (DWORD i = 0; i < count; ++i) {
      sourceLocations.push_back({});
      DxcSourceLocations->GetFileNameByIndex(i,
                                             &sourceLocations.back().Filename);
      sourceLocations.back().Line = DxcSourceLocations->GetLineNumberByIndex(i);
      sourceLocations.back().Column = DxcSourceLocations->GetColumnByIndex(i);
    }
    DxcSourceLocations = nullptr;
  }

  auto it = sourceLocations.begin();
  VERIFY_IS_FALSE(it == sourceLocations.end());

  const WCHAR *mainFileName = L"source.hlsl";
  // The list of source locations should start with the containing file:
  while (it != sourceLocations.end() && it->Filename == mainFileName)
    it++;
  VERIFY_IS_FALSE(it == sourceLocations.end());

  // Then have a bunch of "../include2/samefilename.h"
  VERIFY_ARE_EQUAL_WSTR(L"./../include2/samefilename.h", it->Filename);
  while (it != sourceLocations.end() &&
         it->Filename == L"./../include2/samefilename.h")
    it++;
  VERIFY_IS_FALSE(it == sourceLocations.end());

  // Then some more main file:
  VERIFY_ARE_EQUAL_WSTR(mainFileName, it->Filename);
  while (it != sourceLocations.end() && it->Filename == mainFileName)
    it++;

  // And that should be the end:
  VERIFY_IS_TRUE(it == sourceLocations.end());
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_SubPrograms) {
  if (m_ver.SkipDxilVersion(1, 2))
    return;

  const char *hlsl = R"(

#include "../include1/samefilename.h"
namespace n1 {
#include "../include2/samefilename.h"
}
RWStructuredBuffer<float> floatRWUAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  float4 result = n1::fn2(
    float3(floatRWUAV[0], floatRWUAV[1], floatRWUAV[2]),
    floatRWUAV[3]);
  floatRWUAV[0]  = result.x;
  floatRWUAV[1]  = result.y;
  floatRWUAV[2]  = result.z;
  floatRWUAV[3]  = result.w;
}

)";
  RunSubProgramsCase(hlsl);
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_QIOldFieldInterface) {
  const char *hlsl = R"(
struct Struct
{
    unsigned int first;
    unsigned int second;
};

RWStructuredBuffer<int> UAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  Struct s;
  s.second = UAV[0];
  UAV[16] = s.second; //STOP_HERE
}
)";

  auto debugInfo = CompileAndCreateDxcDebug(hlsl, L"cs_6_5", nullptr).debugInfo;
  auto live = GetLiveVariablesAt(hlsl, "STOP_HERE", debugInfo);
  CComPtr<IDxcPixVariable> bf;
  VERIFY_SUCCEEDED(live->GetVariableByName(L"s", &bf));
  CComPtr<IDxcPixType> bfType;
  VERIFY_SUCCEEDED(bf->GetType(&bfType));
  CComPtr<IDxcPixStructType> bfStructType;
  VERIFY_SUCCEEDED(bfType->QueryInterface(IID_PPV_ARGS(&bfStructType)));
  CComPtr<IDxcPixStructField> field;
  VERIFY_SUCCEEDED(bfStructType->GetFieldByIndex(1, &field));
  CComPtr<IDxcPixStructField0> mike;
  VERIFY_SUCCEEDED(field->QueryInterface(IID_PPV_ARGS(&mike)));
  DWORD secondFieldOffset = 0;
  VERIFY_SUCCEEDED(mike->GetOffsetInBits(&secondFieldOffset));
  VERIFY_ARE_EQUAL(32u, secondFieldOffset);
}

CComPtr<IDxcPixDxilStorage>
PixDiaTest::RunSizeAndOffsetTestCase(const char *hlsl,
                                     std::array<DWORD, 4> const &memberOffsets,
                                     std::array<DWORD, 4> const &memberSizes,
                                     std::vector<const wchar_t *> extraArgs) {
  auto debugInfo =
      CompileAndCreateDxcDebug(hlsl, L"cs_6_5", nullptr, extraArgs).debugInfo;
  auto live = GetLiveVariablesAt(hlsl, "STOP_HERE", debugInfo);
  CComPtr<IDxcPixVariable> bf;
  VERIFY_SUCCEEDED(live->GetVariableByName(L"bf", &bf));
  CComPtr<IDxcPixType> bfType;
  VERIFY_SUCCEEDED(bf->GetType(&bfType));
  CComPtr<IDxcPixStructType> bfStructType;
  VERIFY_SUCCEEDED(bfType->QueryInterface(IID_PPV_ARGS(&bfStructType)));
  for (size_t i = 0; i < memberOffsets.size(); ++i) {
    CComPtr<IDxcPixStructField> field;
    VERIFY_SUCCEEDED(
        bfStructType->GetFieldByIndex(static_cast<DWORD>(i), &field));
    DWORD offsetInBits = 0;
    VERIFY_SUCCEEDED(field->GetOffsetInBits(&offsetInBits));
    VERIFY_ARE_EQUAL(memberOffsets[i], offsetInBits);
    DWORD sizeInBits = 0;
    VERIFY_SUCCEEDED(field->GetFieldSizeInBits(&sizeInBits));
    VERIFY_ARE_EQUAL(memberSizes[i], sizeInBits);
  }
  // Check that first and second and third are reported as residing in the same
  // register (cuz they do!), and that the third does not

  CComPtr<IDxcPixDxilStorage> bfStorage;
  VERIFY_SUCCEEDED(bf->GetStorage(&bfStorage));
  return bfStorage;
}

void RunBitfieldAdjacencyTest(
    IDxcPixDxilStorage *bfStorage,
    std::vector<std::vector<wchar_t const *>> const &adjacentRuns) {
  std::vector<std::set<DWORD>> registersByRun;
  registersByRun.resize(adjacentRuns.size());
  for (size_t run = 0; run < adjacentRuns.size(); ++run) {
    for (auto const &field : adjacentRuns[run]) {
      CComPtr<IDxcPixDxilStorage> fieldStorage;
      VERIFY_SUCCEEDED(bfStorage->AccessField(field, &fieldStorage));
      DWORD reg;
      VERIFY_SUCCEEDED(fieldStorage->GetRegisterNumber(&reg));
      registersByRun[run].insert(reg);
    }
  }
  for (size_t run = 0; run < registersByRun.size(); ++run) {
    {
      // Every field in this run should have the same register number, so this
      // set should be of size 1:
      VERIFY_ARE_EQUAL(1, registersByRun[run].size());
      // Every adjacent run should have different register numbers:
      if (run != 0) {
        VERIFY_ARE_NOT_EQUAL(*registersByRun[run - 1].begin(),
                             *registersByRun[run].begin());
      }
    }
  }
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_BitFields_Simple) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
struct Bitfields
{
    unsigned int first : 17;
    unsigned int second : 15; // consume all 32 bits of first dword
    unsigned int third : 3; // should be at bit offset 32
    unsigned int fourth; // should be at bit offset 64
};

RWStructuredBuffer<int> UAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  Bitfields bf;
  bf.first = UAV[0];
  bf.second = UAV[1];
  bf.third = UAV[2];
  bf.fourth = UAV[3];
  UAV[16] = bf.first + bf.second + bf.third + bf.fourth; //STOP_HERE
}

)";
  auto bfStorage =
      RunSizeAndOffsetTestCase(hlsl, {0, 17, 32, 64}, {17, 15, 3, 32});
  RunBitfieldAdjacencyTest(bfStorage,
                           {{L"first", L"second"}, {L"third"}, {L"fourth"}});
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_BitFields_Derived) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
struct Bitfields
{
    uint first : 17;
    uint second : 15; // consume all 32 bits of first dword
    uint third : 3; // should be at bit offset 32
    uint fourth; // should be at bit offset 64
};

RWStructuredBuffer<int> UAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  Bitfields bf;
  bf.first = UAV[0];
  bf.second = UAV[1];
  bf.third = UAV[2];
  bf.fourth = UAV[3];
  UAV[16] = bf.first + bf.second + bf.third + bf.fourth; //STOP_HERE
}

)";
  auto bfStorage =
      RunSizeAndOffsetTestCase(hlsl, {0, 17, 32, 64}, {17, 15, 3, 32});
  RunBitfieldAdjacencyTest(bfStorage,
                           {{L"first", L"second"}, {L"third"}, {L"fourth"}});
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_BitFields_Bool) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
struct Bitfields
{
    bool first : 1;
    bool second : 1;
    bool third : 3; // just to be weird
    uint fourth; // should be at bit offset 64
};

RWStructuredBuffer<int> UAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  Bitfields bf;
  bf.first = UAV[0];
  bf.second = UAV[1];
  bf.third = UAV[2];
  bf.fourth = UAV[3];
  UAV[16] = bf.first + bf.second + bf.third + bf.fourth; //STOP_HERE
}

)";
  auto bfStorage = RunSizeAndOffsetTestCase(hlsl, {0, 1, 2, 32}, {1, 1, 3, 32});
  RunBitfieldAdjacencyTest(bfStorage,
                           {{L"first", L"second", L"third"}, {L"fourth"}});
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_BitFields_Overlap) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
struct Bitfields
{
    uint32_t first : 20;
    uint32_t second : 20; // should end up in second DWORD
    uint32_t third : 3; // should shader second DWORD
    uint32_t fourth; // should be in third DWORD
};

RWStructuredBuffer<int> UAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  Bitfields bf;
  bf.first = UAV[0];
  bf.second = UAV[1];
  bf.third = UAV[2];
  bf.fourth = UAV[3];
  UAV[16] = bf.first + bf.second + bf.third + bf.fourth; //STOP_HERE
}

)";
  auto bfStorage =
      RunSizeAndOffsetTestCase(hlsl, {0, 32, 52, 64}, {20, 20, 3, 32});
  // (PIX #58022343): fields that overlap their storage type are not yet
  // reflected properly in terms of their packed offsets as maintained via
  // these PixDxc interfaces based on the dbg.declare data
  // RunBitfieldAdjacencyTest(bfStorage,
  //                         {{L"first"}, {L"second", L"third"}, {L"fourth"}});
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_BitFields_uint64) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
struct Bitfields
{
    uint64_t first : 20;
    uint64_t second : 20; // should end up in first uint64 also
    uint64_t third : 24; // in first
    uint64_t fourth; // should be in second
};

RWStructuredBuffer<int> UAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  Bitfields bf;
  bf.first = UAV[0];
  bf.second = UAV[1];
  bf.third = UAV[2];
  bf.fourth = UAV[3];
  UAV[16] = bf.first + bf.second + bf.third + bf.fourth; //STOP_HERE
}

)";
  auto bfStorage =
      RunSizeAndOffsetTestCase(hlsl, {0, 20, 40, 64}, {20, 20, 24, 64});
  RunBitfieldAdjacencyTest(bfStorage,
                           {{L"first", L"second", L"third"}, {L"fourth"}});
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_Alignment_ConstInt) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(

RWStructuredBuffer<int> UAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  const uint c = UAV[0];
  UAV[16] = c;
}

)";
  CComPtr<IDiaDataSource> pDiaDataSource;
  CompileAndRunAnnotationAndLoadDiaSource(m_dllSupport, hlsl, L"cs_6_5",
                                          nullptr, &pDiaDataSource, {L"-Od"});
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_Min16SizesAndOffsets_Enabled) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
struct Mins
{
    min16uint first;
    min16int second;
    min12int third;
    min16float fourth;
};

RWStructuredBuffer<int> UAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  Mins bf;
  bf.first = UAV[0];
  bf.second = UAV[1];
  bf.third = UAV[2];
  bf.fourth = UAV[3];
  UAV[16] = bf.first + bf.second + bf.third + bf.fourth; //STOP_HERE
}


)";
  RunSizeAndOffsetTestCase(hlsl, {0, 16, 32, 48}, {16, 16, 16, 16},
                           {L"-Od", L"-enable-16bit-types"});
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_Min16SizesAndOffsets_Disabled) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
struct Mins
{
    min16uint first;
    min16int second;
    min12int third;
    min16float fourth;
};

RWStructuredBuffer<int> UAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  Mins bf;
  bf.first = UAV[0];
  bf.second = UAV[1];
  bf.third = UAV[2];
  bf.fourth = UAV[3];
  UAV[16] = bf.first + bf.second + bf.third + bf.fourth; //STOP_HERE
}


)";
  RunSizeAndOffsetTestCase(hlsl, {0, 32, 64, 96}, {16, 16, 16, 16}, {L"-Od"});
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_Min16VectorOffsets_Enabled) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
RWStructuredBuffer<int> UAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  min16float4 vector;
  vector.x = UAV[0];
  vector.y = UAV[1];
  vector.z = UAV[2];
  vector.w = UAV[3];
  UAV[16] = vector.x + vector.y + vector.z + vector.w; //STOP_HERE
}


)";
  RunVectorSizeAndOffsetTestCase(hlsl, {0, 16, 32, 48},
                                 {L"-Od", L"-enable-16bit-types"});
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_Min16VectorOffsets_Disabled) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  const char *hlsl = R"(
RWStructuredBuffer<int> UAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  min16float4 vector;
  vector.x = UAV[0];
  vector.y = UAV[1];
  vector.z = UAV[2];
  vector.w = UAV[3];
  UAV[16] = vector.x + vector.y + vector.z + vector.w; //STOP_HERE
}


)";
  RunVectorSizeAndOffsetTestCase(hlsl, {0, 32, 64, 96});
}
void PixDiaTest::RunVectorSizeAndOffsetTestCase(
    const char *hlsl, std::array<DWORD, 4> const &memberOffsets,
    std::vector<const wchar_t *> extraArgs) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;
  auto debugInfo =
      CompileAndCreateDxcDebug(hlsl, L"cs_6_5", nullptr, extraArgs).debugInfo;
  auto live = GetLiveVariablesAt(hlsl, "STOP_HERE", debugInfo);
  CComPtr<IDxcPixVariable> variable;
  VERIFY_SUCCEEDED(live->GetVariableByName(L"vector", &variable));
  CComPtr<IDxcPixType> type;
  VERIFY_SUCCEEDED(variable->GetType(&type));

  CComPtr<IDxcPixType> unAliasedType;
  VERIFY_SUCCEEDED(UnAliasType(type, &unAliasedType));
  CComPtr<IDxcPixStructType> structType;
  VERIFY_SUCCEEDED(unAliasedType->QueryInterface(IID_PPV_ARGS(&structType)));

  DWORD fieldCount = 0;
  VERIFY_SUCCEEDED(structType->GetNumFields(&fieldCount));
  VERIFY_ARE_EQUAL(fieldCount, 4u);

  for (size_t i = 0; i < memberOffsets.size(); i++) {
    CComPtr<IDxcPixStructField> field;
    VERIFY_SUCCEEDED(structType->GetFieldByIndex(i, &field));
    DWORD offsetInBits = 0;
    VERIFY_SUCCEEDED(field->GetOffsetInBits(&offsetInBits));
    VERIFY_ARE_EQUAL(memberOffsets[i], offsetInBits);
  }
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_SubProgramsInNamespaces) {
  if (m_ver.SkipDxilVersion(1, 2))
    return;

  const char *hlsl = R"(

#include "../include1/samefilename.h"
#include "../include2/samefilename.h"

RWStructuredBuffer<float> floatRWUAV: register(u0);

[numthreads(1, 1, 1)]
void main()
{
  float4 result = fn2(
    float3(floatRWUAV[0], floatRWUAV[1], floatRWUAV[2]),
    floatRWUAV[3]);
  floatRWUAV[0]  = result.x;
  floatRWUAV[1]  = result.y;
  floatRWUAV[2]  = result.z;
  floatRWUAV[3]  = result.w;
}

)";
  RunSubProgramsCase(hlsl);
}

static DWORD AdvanceUntilFunctionEntered(IDxcPixDxilDebugInfo *dxilDebugger,
                                         DWORD instructionOffset,
                                         wchar_t const *fnName) {
  for (;;) {
    CComBSTR FunctioName;
    if (FAILED(
            dxilDebugger->GetFunctionName(instructionOffset, &FunctioName))) {
      VERIFY_FAIL(L"Didn't find function");
      return -1;
    }
    if (FunctioName == fnName)
      break;
    instructionOffset++;
  }
  return instructionOffset;
}

static DWORD GetRegisterNumberForVariable(IDxcPixDxilDebugInfo *dxilDebugger,
                                          DWORD instructionOffset,
                                          wchar_t const *variableName,
                                          wchar_t const *memberName = nullptr) {
  CComPtr<IDxcPixDxilLiveVariables> DxcPixDxilLiveVariables;
  if (SUCCEEDED(dxilDebugger->GetLiveVariablesAt(instructionOffset,
                                                 &DxcPixDxilLiveVariables))) {
    DWORD count = 42;
    VERIFY_SUCCEEDED(DxcPixDxilLiveVariables->GetCount(&count));
    for (DWORD i = 0; i < count; ++i) {
      CComPtr<IDxcPixVariable> DxcPixVariable;
      VERIFY_SUCCEEDED(
          DxcPixDxilLiveVariables->GetVariableByIndex(i, &DxcPixVariable));
      CComBSTR Name;
      VERIFY_SUCCEEDED(DxcPixVariable->GetName(&Name));
      if (Name == variableName) {
        CComPtr<IDxcPixDxilStorage> DxcPixDxilStorage;
        VERIFY_SUCCEEDED(DxcPixVariable->GetStorage(&DxcPixDxilStorage));
        if (memberName != nullptr) {
          CComPtr<IDxcPixDxilStorage> DxcPixDxilMemberStorage;
          VERIFY_SUCCEEDED(DxcPixDxilStorage->AccessField(
              memberName, &DxcPixDxilMemberStorage));
          DxcPixDxilStorage = DxcPixDxilMemberStorage;
        }
        DWORD RegisterNumber = 42;
        VERIFY_SUCCEEDED(DxcPixDxilStorage->GetRegisterNumber(&RegisterNumber));
        return RegisterNumber;
      }
    }
  }
  VERIFY_FAIL(L"Couldn't find register number");
  return -1;
}

static void
CheckVariableExistsAtThisInstruction(IDxcPixDxilDebugInfo *dxilDebugger,
                                     DWORD instructionOffset,
                                     wchar_t const *variableName) {
  // It's sufficient to know that there _is_ a register number the var:
  (void)GetRegisterNumberForVariable(dxilDebugger, instructionOffset,
                                     variableName);
}

static void
CheckVariableDoesNOTExistsAtThisInstruction(IDxcPixDxilDebugInfo *dxilDebugger,
                                            DWORD instructionOffset,
                                            wchar_t const *variableName) {
  CComPtr<IDxcPixDxilLiveVariables> DxcPixDxilLiveVariables;
  VERIFY_SUCCEEDED(dxilDebugger->GetLiveVariablesAt(instructionOffset,
                                                    &DxcPixDxilLiveVariables));
  DWORD count = 42;
  VERIFY_SUCCEEDED(DxcPixDxilLiveVariables->GetCount(&count));
  for (DWORD i = 0; i < count; ++i) {
    CComPtr<IDxcPixVariable> DxcPixVariable;
    VERIFY_SUCCEEDED(
        DxcPixDxilLiveVariables->GetVariableByIndex(i, &DxcPixVariable));
    CComBSTR Name;
    VERIFY_SUCCEEDED(DxcPixVariable->GetName(&Name));
    VERIFY_ARE_NOT_EQUAL(Name, variableName);
  }
}

TEST_F(
    PixDiaTest,
    DxcPixDxilDebugInfo_VariableScopes_InlinedFunctions_TwiceInlinedFunctions) {
  if (m_ver.SkipDxilVersion(1, 6))
    return;

  const char *hlsl = R"(
struct RayPayload
{
    float4 color;
};

RWStructuredBuffer<float4> floatRWUAV: register(u0);

namespace StressScopesABit
{
#include "included.h"
}

namespace StressScopesMore
{
float4 InlinedFunction(in BuiltInTriangleIntersectionAttributes attr, int offset)
{
  float4 ret = floatRWUAV.Load(offset + attr.barycentrics.x + 42);
  float4 color2 = StressScopesABit::StressScopesEvenMore::DeeperInlinedFunction(attr, offset) + ret;
  float4 color3 = StressScopesABit::StressScopesEvenMore::DeeperInlinedFunction(attr, offset+1);
  return color2 + color3;
}
}

[shader("closesthit")]
void ClosestHitShader0(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.color = StressScopesMore::InlinedFunction(attr, 0);
}

[shader("closesthit")]
void ClosestHitShader1(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.color = StressScopesMore::InlinedFunction(attr, 1);
}

[shader("closesthit")]
void ClosestHitShader2(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    float4 generateSomeLocalInstrucitons = floatRWUAV.Load(0);
    float4 c0 = StressScopesMore::InlinedFunction(attr, 0);
    float4 c1 = StressScopesABit::StressScopesEvenMore::DeeperInlinedFunction(attr, 42);
    payload.color = c0 + c1 + generateSomeLocalInstrucitons;
}
)";

  CComPtr<DxcIncludeHandlerForInjectedSourcesForPix> pIncludeHandler =
      new DxcIncludeHandlerForInjectedSourcesForPix(this, {{L"included.h",
                                                            R"(

namespace StressScopesEvenMore
{
float4 DeeperInlinedFunction(in BuiltInTriangleIntersectionAttributes attr, int offset)
{
  float4 ret = float4(0,0,0,0);
  for(int i =0; i < offset; ++i)
  {
    float4 color0 = floatRWUAV.Load(offset + attr.barycentrics.x);
    float4 color1 = floatRWUAV.Load(offset + attr.barycentrics.y);
    ret += color0 + color1;
  }
  return ret;
}
}
)"}});

  auto dxilDebugger =
      CompileAndCreateDxcDebug(hlsl, L"lib_6_6", pIncludeHandler).debugInfo;

  // Case: same functions called from two different top-level callers
  DWORD instructionOffset =
      AdvanceUntilFunctionEntered(dxilDebugger, 0, L"ClosestHitShader0");
  instructionOffset = AdvanceUntilFunctionEntered(
      dxilDebugger, instructionOffset, L"DeeperInlinedFunction");
  DWORD RegisterNumber0 = GetRegisterNumberForVariable(
      dxilDebugger, instructionOffset, L"ret", L"x");
  instructionOffset = AdvanceUntilFunctionEntered(
      dxilDebugger, instructionOffset, L"InlinedFunction");
  DWORD RegisterNumber2 = GetRegisterNumberForVariable(
      dxilDebugger, instructionOffset, L"color2", L"x");
  instructionOffset = AdvanceUntilFunctionEntered(
      dxilDebugger, instructionOffset, L"ClosestHitShader1");
  instructionOffset = AdvanceUntilFunctionEntered(
      dxilDebugger, instructionOffset, L"DeeperInlinedFunction");
  DWORD RegisterNumber1 = GetRegisterNumberForVariable(
      dxilDebugger, instructionOffset, L"ret", L"x");
  instructionOffset = AdvanceUntilFunctionEntered(
      dxilDebugger, instructionOffset, L"InlinedFunction");
  DWORD RegisterNumber3 = GetRegisterNumberForVariable(
      dxilDebugger, instructionOffset, L"color2", L"x");
  VERIFY_ARE_NOT_EQUAL(RegisterNumber0, RegisterNumber1);
  VERIFY_ARE_NOT_EQUAL(RegisterNumber2, RegisterNumber3);

  // Case: two different functions called from same top-level function
  instructionOffset =
      AdvanceUntilFunctionEntered(dxilDebugger, 0, L"ClosestHitShader2");
  instructionOffset = AdvanceUntilFunctionEntered(
      dxilDebugger, instructionOffset, L"InlinedFunction");
  DWORD ColorRegisterNumberWhenCalledFromOuterForInlined =
      GetRegisterNumberForVariable(dxilDebugger, instructionOffset, L"ret",
                                   L"x");
  instructionOffset = AdvanceUntilFunctionEntered(
      dxilDebugger, instructionOffset, L"DeeperInlinedFunction");
  DWORD ColorRegisterNumberWhenCalledFromOuterForDeeper =
      GetRegisterNumberForVariable(dxilDebugger, instructionOffset, L"ret",
                                   L"x");
  VERIFY_ARE_NOT_EQUAL(ColorRegisterNumberWhenCalledFromOuterForInlined,
                       ColorRegisterNumberWhenCalledFromOuterForDeeper);
}

TEST_F(
    PixDiaTest,
    DxcPixDxilDebugInfo_VariableScopes_InlinedFunctions_CalledTwiceInSameCaller) {
  if (m_ver.SkipDxilVersion(1, 6))
    return;

  const char *hlsl = R"(
struct RayPayload
{
    float4 color;
};

RWStructuredBuffer<float4> floatRWUAV: register(u0);

float4 InlinedFunction(in BuiltInTriangleIntersectionAttributes attr, int offset)
{
  float4 ret = floatRWUAV.Load(offset + attr.barycentrics.x);
  return ret;
}

[shader("closesthit")]
void ClosestHitShader3(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    float4 generateSomeLocalInstrucitons = floatRWUAV.Load(0);
    float4 c0 = InlinedFunction(attr, 2);
    float4 generateSomeMoreLocalInstrucitons = floatRWUAV.Load(1);
    float4 c1 = InlinedFunction(attr, 3);
    payload.color = c0 + c1 + generateSomeLocalInstrucitons + generateSomeMoreLocalInstrucitons;
}
)";

  auto dxilDebugger = CompileAndCreateDxcDebug(hlsl, L"lib_6_6").debugInfo;

  // Case: same function called from two places in same top-level function.
  // In this case, we expect the storage for the variable to be in the same
  // place for both "instances" of the function: as a thread proceeds
  // through the caller, it will write new values into the variable's
  // storage during the second or subsequent invocations of the inlined
  // function.
  DWORD instructionOffset =
      AdvanceUntilFunctionEntered(dxilDebugger, 0, L"ClosestHitShader3");
  instructionOffset = AdvanceUntilFunctionEntered(
      dxilDebugger, instructionOffset, L"InlinedFunction");
  DWORD callsite0 = GetRegisterNumberForVariable(
      dxilDebugger, instructionOffset, L"ret", L"x");
  // advance until we're out of InlinedFunction before we call it a second
  // time
  instructionOffset = AdvanceUntilFunctionEntered(
      dxilDebugger, instructionOffset, L"ClosestHitShader3");
  instructionOffset = AdvanceUntilFunctionEntered(
      dxilDebugger, instructionOffset, L"InlinedFunction");
  DWORD callsite1 = GetRegisterNumberForVariable(
      dxilDebugger, instructionOffset++, L"ret", L"x");
  VERIFY_ARE_EQUAL(callsite0, callsite1);
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_VariableScopes_ForScopes) {
  if (m_ver.SkipDxilVersion(1, 6))
    return;

  const char *hlsl =
      R"(/*01*/RWStructuredBuffer<int> intRWUAV: register(u0);
/*02*/[shader("compute")]
/*03*/[numthreads(1,1,1)]
/*04*/void CSMain()
/*05*/{
/*06*/    int zero = intRWUAV.Load(0);
/*07*/    int two = zero * 2; // debug-loc(CheckVariableExistsHere)
/*08*/    int three = zero * 3;
/*09*/    for(int i =0; i < two; ++ i)
/*10*/    {
/*11*/        int one = intRWUAV.Load(i);
/*12*/        three += one; // debug-loc(Stop inside loop)
/*13*/    }
/*14*/    intRWUAV[0] = three; // debug-loc(Stop here)
/*15*/}
)";

  auto debugInterfaces = CompileAndCreateDxcDebug(hlsl, L"lib_6_6");
  auto dxilDebugger = debugInterfaces.debugInfo;
  auto Labels = GatherDebugLocLabelsFromDxcUtils(debugInterfaces);

  // Case: same function called from two places in same top-level function.
  // In this case, we expect the storage for the variable to be in the same
  // place for both "instances" of the function: as a thread proceeds
  // through the caller, it will write new values into the variable's
  // storage during the second or subsequent invocations of the inlined
  // function.
  DWORD instructionOffset =
      AdvanceUntilFunctionEntered(dxilDebugger, 0, L"CSMain");

  instructionOffset =
      Labels->FindInstructionOffsetForLabel(L"CheckVariableExistsHere");
  CheckVariableExistsAtThisInstruction(dxilDebugger, instructionOffset,
                                       L"zero");

  instructionOffset =
      Labels->FindInstructionOffsetForLabel(L"Stop inside loop");
  CheckVariableExistsAtThisInstruction(dxilDebugger, instructionOffset,
                                       L"zero");

  instructionOffset = Labels->FindInstructionOffsetForLabel(L"Stop here");
  CheckVariableDoesNOTExistsAtThisInstruction(dxilDebugger, instructionOffset,
                                              L"one");
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_VariableScopes_ScopeBraces) {
  if (m_ver.SkipDxilVersion(1, 6))
    return;

  const char *hlsl =
      R"(/*01*/RWStructuredBuffer<int> intRWUAV: register(u0);
/*02*/[shader("compute")]
/*03*/[numthreads(1,1,1)]
/*04*/void CSMain()
/*05*/{
/*06*/    int zero = intRWUAV.Load(0);
/*07*/    int two = zero * 2; // debug-loc(CheckVariableExistsHere)
/*08*/    { 
/*09*/        int one = intRWUAV.Load(1);
/*10*/        two += one; // debug-loc(Stop inside loop)
/*11*/    }
/*12*/    intRWUAV[0] = two; // debug-loc(Stop here)
/*13*/}
)";

  auto debugInterfaces = CompileAndCreateDxcDebug(hlsl, L"lib_6_6");
  auto Labels = GatherDebugLocLabelsFromDxcUtils(debugInterfaces);
  auto dxilDebugger = debugInterfaces.debugInfo;

  // Case: same function called from two places in same top-level function.
  // In this case, we expect the storage for the variable to be in the same
  // place for both "instances" of the function: as a thread proceeds
  // through the caller, it will write new values into the variable's
  // storage during the second or subsequent invocations of the inlined
  // function.
  DWORD instructionOffset =
      AdvanceUntilFunctionEntered(dxilDebugger, 0, L"CSMain");

  instructionOffset =
      Labels->FindInstructionOffsetForLabel(L"CheckVariableExistsHere");
  CheckVariableExistsAtThisInstruction(dxilDebugger, instructionOffset,
                                       L"zero");

  instructionOffset =
      Labels->FindInstructionOffsetForLabel(L"Stop inside loop");
  CheckVariableExistsAtThisInstruction(dxilDebugger, instructionOffset,
                                       L"zero");

  instructionOffset = Labels->FindInstructionOffsetForLabel(L"Stop here");
  CheckVariableDoesNOTExistsAtThisInstruction(dxilDebugger, instructionOffset,
                                              L"one");
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_VariableScopes_Function) {
  if (m_ver.SkipDxilVersion(1, 6))
    return;

  const char *hlsl =
      R"(/*01*/RWStructuredBuffer<int> intRWUAV: register(u0);
/*02*/int Square(int i) {
/*03*/  int i2 = i * i; // debug-loc(Stop in subroutine)
/*04*/  return i2;
/*05*/}
/*06*/[shader("compute")]
/*07*/[numthreads(1,1,1)]
/*08*/void CSMain()
/*09*/{
/*10*/    int zero = intRWUAV.Load(0);
/*11*/    int two = Square(zero);
/*12*/    intRWUAV[0] = two; // debug-loc(Stop here)
/*13*/}
)";

  auto debugInterfaces = CompileAndCreateDxcDebug(hlsl, L"lib_6_6");
  auto Labels = GatherDebugLocLabelsFromDxcUtils(debugInterfaces);
  auto dxilDebugger = debugInterfaces.debugInfo;

  // Case: same function called from two places in same top-level function.
  // In this case, we expect the storage for the variable to be in the same
  // place for both "instances" of the function: as a thread proceeds
  // through the caller, it will write new values into the variable's
  // storage during the second or subsequent invocations of the inlined
  // function.
  DWORD instructionOffset =
      AdvanceUntilFunctionEntered(dxilDebugger, 0, L"CSMain");

  instructionOffset =
      Labels->FindInstructionOffsetForLabel(L"Stop in subroutine");
  CheckVariableDoesNOTExistsAtThisInstruction(dxilDebugger, instructionOffset,
                                              L"zero");

  instructionOffset = Labels->FindInstructionOffsetForLabel(L"Stop here");
  CheckVariableDoesNOTExistsAtThisInstruction(dxilDebugger, instructionOffset,
                                              L"i2");
  CheckVariableExistsAtThisInstruction(dxilDebugger, instructionOffset,
                                       L"zero");
}

TEST_F(PixDiaTest, DxcPixDxilDebugInfo_VariableScopes_Member) {
  if (m_ver.SkipDxilVersion(1, 6))
    return;

  const char *hlsl =
      R"(/*01*/RWStructuredBuffer<int> intRWUAV: register(u0);
struct Struct {
  int i;
  int Getter() {
      int q = i;
      return q; //debug-loc(inside member fn)
  }
};
[shader("compute")]
[numthreads(1,1,1)]
void CSMain()
{
    Struct s;
    s.i = intRWUAV.Load(0);
    int j = s.Getter();
    intRWUAV[0] = j; // debug-loc(Stop here)
}
)";

  auto debugInterfaces = CompileAndCreateDxcDebug(hlsl, L"lib_6_6");
  auto Labels = GatherDebugLocLabelsFromDxcUtils(debugInterfaces);
  auto dxilDebugger = debugInterfaces.debugInfo;

  // Case: same function called from two places in same top-level function.
  // In this case, we expect the storage for the variable to be in the same
  // place for both "instances" of the function: as a thread proceeds
  // through the caller, it will write new values into the variable's
  // storage during the second or subsequent invocations of the inlined
  // function.
  DWORD instructionOffset =
      AdvanceUntilFunctionEntered(dxilDebugger, 0, L"CSMain");

  instructionOffset =
      Labels->FindInstructionOffsetForLabel(L"inside member fn");
  CheckVariableDoesNOTExistsAtThisInstruction(dxilDebugger, instructionOffset,
                                              L"s");
  CheckVariableExistsAtThisInstruction(dxilDebugger, instructionOffset, L"q");

  instructionOffset = Labels->FindInstructionOffsetForLabel(L"Stop here");
  CheckVariableDoesNOTExistsAtThisInstruction(dxilDebugger, instructionOffset,
                                              L"i");
  CheckVariableExistsAtThisInstruction(dxilDebugger, instructionOffset, L"j");
}

#endif
