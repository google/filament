///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DXIsenseTest.cpp                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides tests for the dxcompiler Intellisense API.                       //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Test/CompilationResult.h"
#include "dxc/Test/HLSLTestData.h"
#include <stdint.h>

#include "dxc/Support/microcom.h"
#include "dxc/Test/HlslTestUtils.h"

#ifdef _WIN32
class DXIntellisenseTest {
#else
class DXIntellisenseTest : public ::testing::Test {
#endif
public:
  BEGIN_TEST_CLASS(DXIntellisenseTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

protected:
  TEST_CLASS_SETUP(DXIntellisenseTestClassSetup)
  TEST_CLASS_CLEANUP(DXIntellisenseTestClassCleanup)

  void GetLocationAt(IDxcTranslationUnit *TU, unsigned line, unsigned col,
                     IDxcSourceLocation **pResult) {
    CComPtr<IDxcFile> file;
    VERIFY_SUCCEEDED(TU->GetFile("filename.hlsl", &file));
    VERIFY_SUCCEEDED(TU->GetLocation(file, line, col, pResult));
  }

  void GetCursorAt(IDxcTranslationUnit *TU, unsigned line, unsigned col,
                   IDxcCursor **pResult) {
    CComPtr<IDxcSourceLocation> location;
    GetLocationAt(TU, line, col, &location);
    VERIFY_SUCCEEDED(TU->GetCursorForLocation(location, pResult));
  }

  void ExpectCursorAt(IDxcTranslationUnit *TU, unsigned line, unsigned col,
                      DxcCursorKind expectedKind,
                      IDxcCursor **pResult = nullptr) {
    CComPtr<IDxcCursor> cursor;
    DxcCursorKind actualKind;
    GetCursorAt(TU, line, col, &cursor);
    VERIFY_SUCCEEDED(cursor->GetKind(&actualKind));
    EXPECT_EQ(expectedKind,
              actualKind); // << " for cursor at " << line << ":" << col;
    if (pResult != nullptr) {
      *pResult = cursor.Detach();
    }
  }

  void ExpectQualifiedName(IDxcTranslationUnit *TU, unsigned line, unsigned col,
                           const wchar_t *expectedName) {
    CComPtr<IDxcCursor> cursor;
    CComBSTR name;
    GetCursorAt(TU, line, col, &cursor);
    ASSERT_HRESULT_SUCCEEDED(cursor->GetQualifiedName(FALSE, &name));
    EXPECT_STREQW(expectedName,
                  name); // << "qualified name at " << line << ":" << col;
  }

  void ExpectDeclarationText(IDxcTranslationUnit *TU, unsigned line,
                             unsigned col, const wchar_t *expectedDecl) {
    CComPtr<IDxcCursor> cursor;
    CComBSTR name;
    GetCursorAt(TU, line, col, &cursor);
    DxcCursorFormatting formatting =
        (DxcCursorFormatting)(DxcCursorFormatting_IncludeNamespaceKeyword);
    ASSERT_HRESULT_SUCCEEDED(cursor->GetFormattedName(formatting, &name));
    EXPECT_STREQW(expectedDecl,
                  name); // << "declaration text at " << line << ":" << col;
  }

  TEST_METHOD(CursorWhenCBufferRefThenFound)
  TEST_METHOD(CursorWhenPresumedLocationDifferentFromSpellingLocation)
  TEST_METHOD(CursorWhenPresumedLocationSameAsSpellingLocation)
  TEST_METHOD(CursorWhenFieldRefThenSimpleNames)
  TEST_METHOD(CursorWhenFindAtBodyCallThenMatch)
  TEST_METHOD(CursorWhenFindAtGlobalThenMatch)
  TEST_METHOD(CursorWhenFindBeforeBodyCallThenMatch)
  TEST_METHOD(CursorWhenFindBeforeGlobalThenMatch)
  TEST_METHOD(CursorWhenFunctionThenParamsAvailable)
  TEST_METHOD(CursorWhenFunctionThenReturnTypeAvailable)
  TEST_METHOD(CursorWhenFunctionThenSignatureAvailable)
  TEST_METHOD(CursorWhenGlobalVariableThenSimpleNames)
  TEST_METHOD(CursorWhenOverloadedIncompleteThenInvisible)
  TEST_METHOD(CursorWhenOverloadedResolvedThenDirectSymbol)
  TEST_METHOD(CursorWhenReferenceThenDefinitionAvailable)
  TEST_METHOD(CursorWhenTypeOfVariableDeclThenNamesHaveType)
  TEST_METHOD(CursorTypeUsedNamespace)
  TEST_METHOD(CursorWhenVariableRefThenSimpleNames)
  TEST_METHOD(CursorWhenVariableUsedThenDeclarationAvailable)

  TEST_METHOD(FileWhenSameThenEqual)
  TEST_METHOD(FileWhenNotSameThenNotEqual)

  TEST_METHOD(InclusionWhenMissingThenError)
  TEST_METHOD(InclusionWhenValidThenAvailable)

  TEST_METHOD(TUWhenGetFileMissingThenFail)
  TEST_METHOD(TUWhenGetFilePresentThenOK)
  TEST_METHOD(TUWhenEmptyStructThenErrorIfISense)
  TEST_METHOD(TUWhenRegionInactiveMissingThenCountIsZero)
  TEST_METHOD(TUWhenRegionInactiveThenEndIsBeforeElseHash)
  TEST_METHOD(TUWhenRegionInactiveThenEndIsBeforeEndifHash)
  TEST_METHOD(TUWhenRegionInactiveThenStartIsAtIfdefEol)
  TEST_METHOD(TUWhenUnsaveFileThenOK)

  TEST_METHOD(QualifiedNameClass)
  TEST_METHOD(QualifiedNameVariable)

  TEST_METHOD(TypeWhenICEThenEval)

  TEST_METHOD(CompletionWhenResultsAvailable)
};

bool DXIntellisenseTest::DXIntellisenseTestClassSetup() {
  std::shared_ptr<HlslIntellisenseSupport> result =
      std::make_shared<HlslIntellisenseSupport>();
  if (FAILED(result->Initialize()))
    return false;
  CompilationResult::DefaultHlslSupport = result;
  return true;
}

bool DXIntellisenseTest::DXIntellisenseTestClassCleanup() {
  CompilationResult::DefaultHlslSupport = nullptr;
  return true;
}

TEST_F(DXIntellisenseTest, CursorWhenCBufferRefThenFound) {
  char program[] = "cbuffer MyBuffer {\r\n"
                   " int a; }\r\n"
                   "int main() { return\r\n"
                   "a; }";

  CComPtr<IDxcCursor> varRefCursor;
  CComPtr<IDxcFile> file;
  CComInterfaceArray<IDxcCursor> refs;
  CComPtr<IDxcSourceLocation> loc;
  unsigned line;

  CompilationResult c(
      CompilationResult::CreateForProgram(program, strlen(program)));
  VERIFY_IS_TRUE(c.ParseSucceeded());
  ExpectCursorAt(c.TU, 4, 1, DxcCursor_DeclRefExpr, &varRefCursor);
  VERIFY_SUCCEEDED(
      c.TU->GetFile(CompilationResult::getDefaultFileName(), &file));
  VERIFY_SUCCEEDED(varRefCursor->FindReferencesInFile(
      file, 0, 4, refs.size_ref(), refs.data_ref()));
  VERIFY_ARE_EQUAL(2U, refs.size());
  VERIFY_SUCCEEDED(refs.begin()[0]->GetLocation(&loc));
  VERIFY_SUCCEEDED(loc->GetSpellingLocation(nullptr, &line, nullptr, nullptr));
  VERIFY_ARE_EQUAL(2U, line);
}

TEST_F(DXIntellisenseTest,
       CursorWhenPresumedLocationDifferentFromSpellingLocation) {
  char program[] = "#line 21 \"something.h\"\r\n"
                   "struct MyStruct { };";

  CComPtr<IDxcCursor> varCursor;
  CComPtr<IDxcSourceLocation> loc;
  CComPtr<IDxcFile> spellingFile;
  unsigned spellingLine, spellingCol, spellingOffset;
  CComHeapPtr<char> presumedFilename;
  unsigned presumedLine, presumedCol;

  CompilationResult c(
      CompilationResult::CreateForProgram(program, strlen(program)));
  VERIFY_IS_TRUE(c.ParseSucceeded());
  ExpectCursorAt(c.TU, 2, 1, DxcCursor_StructDecl, &varCursor);
  VERIFY_SUCCEEDED(varCursor->GetLocation(&loc));

  VERIFY_SUCCEEDED(loc->GetSpellingLocation(&spellingFile, &spellingLine,
                                            &spellingCol, &spellingOffset));
  VERIFY_ARE_EQUAL(2u, spellingLine);
  VERIFY_ARE_EQUAL(8u, spellingCol);
  VERIFY_ARE_EQUAL(31u, spellingOffset);

  VERIFY_SUCCEEDED(
      loc->GetPresumedLocation(&presumedFilename, &presumedLine, &presumedCol));
  VERIFY_ARE_EQUAL_STR("something.h", presumedFilename);
  VERIFY_ARE_EQUAL(21u, presumedLine);
  VERIFY_ARE_EQUAL(8u, presumedCol);
}

TEST_F(DXIntellisenseTest, CursorWhenPresumedLocationSameAsSpellingLocation) {
  char program[] = "struct MyStruct { };";

  CComPtr<IDxcCursor> varCursor;
  CComPtr<IDxcSourceLocation> loc;
  CComPtr<IDxcFile> spellingFile;
  unsigned spellingLine, spellingCol, spellingOffset;
  CComHeapPtr<char> presumedFilename;
  unsigned presumedLine, presumedCol;

  CompilationResult c(
      CompilationResult::CreateForProgram(program, strlen(program)));
  VERIFY_IS_TRUE(c.ParseSucceeded());
  ExpectCursorAt(c.TU, 1, 1, DxcCursor_StructDecl, &varCursor);
  VERIFY_SUCCEEDED(varCursor->GetLocation(&loc));

  VERIFY_SUCCEEDED(loc->GetSpellingLocation(&spellingFile, &spellingLine,
                                            &spellingCol, &spellingOffset));
  VERIFY_ARE_EQUAL(1u, spellingLine);
  VERIFY_ARE_EQUAL(8u, spellingCol);
  VERIFY_ARE_EQUAL(7u, spellingOffset);

  VERIFY_SUCCEEDED(
      loc->GetPresumedLocation(&presumedFilename, &presumedLine, &presumedCol));
  VERIFY_ARE_EQUAL_STR(CompilationResult::getDefaultFileName(),
                       presumedFilename);
  VERIFY_ARE_EQUAL(1u, presumedLine);
  VERIFY_ARE_EQUAL(8u, presumedCol);
}

TEST_F(DXIntellisenseTest, InclusionWhenMissingThenError) {
  CComPtr<IDxcIntelliSense> isense;
  CComPtr<IDxcIndex> index;
  CComPtr<IDxcUnsavedFile> unsaved;
  CComPtr<IDxcTranslationUnit> TU;
  CComPtr<IDxcDiagnostic> pDiag;
  DxcDiagnosticSeverity Severity;
  const char main_text[] =
      "error\r\n#include \"missing.hlsl\"\r\nfloat3 g_global;";
  unsigned diagCount;
  VERIFY_SUCCEEDED(
      CompilationResult::DefaultHlslSupport->CreateIntellisense(&isense));
  VERIFY_SUCCEEDED(isense->CreateIndex(&index));
  VERIFY_SUCCEEDED(isense->CreateUnsavedFile("file.hlsl", main_text,
                                             strlen(main_text), &unsaved));
  VERIFY_SUCCEEDED(index->ParseTranslationUnit(
      "file.hlsl", nullptr, 0, &unsaved.p, 1,
      DxcTranslationUnitFlags_UseCallerThread, &TU));
  VERIFY_SUCCEEDED(TU->GetNumDiagnostics(&diagCount));
  VERIFY_ARE_EQUAL(1U, diagCount);
  VERIFY_SUCCEEDED(TU->GetDiagnostic(0, &pDiag));
  VERIFY_SUCCEEDED(pDiag->GetSeverity(&Severity));
  VERIFY_IS_TRUE(Severity == DxcDiagnosticSeverity::DxcDiagnostic_Error ||
                 Severity == DxcDiagnosticSeverity::DxcDiagnostic_Fatal);
}

TEST_F(DXIntellisenseTest, InclusionWhenValidThenAvailable) {
  CComPtr<IDxcIntelliSense> isense;
  CComPtr<IDxcIndex> index;
  CComPtr<IDxcUnsavedFile> unsaved[2];
  CComPtr<IDxcTranslationUnit> TU;
  CComInterfaceArray<IDxcInclusion> inclusions;
  const char main_text[] =
      "#include \"inc.h\"\r\nfloat4 main() : SV_Target { return FOO; }";
  const char unsaved_text[] = "#define FOO 1";
  unsigned diagCount;
  unsigned expectedIndex = 0;
  const char *expectedNames[2] = {"file.hlsl", "./inc.h"};
  VERIFY_SUCCEEDED(
      CompilationResult::DefaultHlslSupport->CreateIntellisense(&isense));
  VERIFY_SUCCEEDED(isense->CreateIndex(&index));
  VERIFY_SUCCEEDED(isense->CreateUnsavedFile(
      "./inc.h", unsaved_text, strlen(unsaved_text), &unsaved[0]));
  VERIFY_SUCCEEDED(isense->CreateUnsavedFile("file.hlsl", main_text,
                                             strlen(main_text), &unsaved[1]));
  VERIFY_SUCCEEDED(index->ParseTranslationUnit(
      "file.hlsl", nullptr, 0, &unsaved[0].p, 2,
      DxcTranslationUnitFlags_UseCallerThread, &TU));
  VERIFY_SUCCEEDED(TU->GetNumDiagnostics(&diagCount));
  VERIFY_ARE_EQUAL(0U, diagCount);
  VERIFY_SUCCEEDED(
      TU->GetInclusionList(inclusions.size_ref(), inclusions.data_ref()));
  VERIFY_ARE_EQUAL(2U, inclusions.size());
  for (IDxcInclusion *i : inclusions) {
    CComPtr<IDxcFile> file;
    CComHeapPtr<char> fileName;
    VERIFY_SUCCEEDED(i->GetIncludedFile(&file));
    VERIFY_SUCCEEDED(file->GetName(&fileName));
    VERIFY_ARE_EQUAL_STR(expectedNames[expectedIndex], fileName.m_pData);
    expectedIndex++;
  }
}

TEST_F(DXIntellisenseTest, TUWhenGetFileMissingThenFail) {
  const char program[] = "int i;";
  CompilationResult result =
      CompilationResult::CreateForProgram(program, strlen(program), nullptr);
  VERIFY_IS_TRUE(result.ParseSucceeded());
  CComPtr<IDxcFile> file;
  VERIFY_FAILED(result.TU->GetFile("unknonwn.txt", &file));
}

TEST_F(DXIntellisenseTest, TUWhenGetFilePresentThenOK) {
  const char program[] = "int i;";
  CompilationResult result =
      CompilationResult::CreateForProgram(program, strlen(program), nullptr);
  VERIFY_IS_TRUE(result.ParseSucceeded());
  CComPtr<IDxcFile> file;
  VERIFY_SUCCEEDED(
      result.TU->GetFile(CompilationResult::getDefaultFileName(), &file));
  VERIFY_IS_NOT_NULL(file.p);
}

TEST_F(DXIntellisenseTest, TUWhenEmptyStructThenErrorIfISense) {
  // An declaration of the from 'struct S;' is a forward declaration in HLSL
  // 2016, but is invalid in HLSL 2015.
  const char program[] = "struct S;";
  const char programWithDef[] = "struct S { int i; };";
  const char *args2015[] = {"-HV", "2015"};
  const char *args2016[] = {"-HV", "2016"};

  CompilationResult result15WithDef(CompilationResult::CreateForProgramAndArgs(
      programWithDef, _countof(programWithDef), args2015, _countof(args2015),
      nullptr));
  VERIFY_IS_TRUE(result15WithDef.ParseSucceeded());

  CompilationResult result15(CompilationResult::CreateForProgramAndArgs(
      program, _countof(program), args2015, _countof(args2015), nullptr));
  VERIFY_IS_FALSE(result15.ParseSucceeded());

  CompilationResult result16(CompilationResult::CreateForProgramAndArgs(
      program, _countof(program), args2016, _countof(args2016), nullptr));
  VERIFY_IS_TRUE(result16.ParseSucceeded());
}

TEST_F(DXIntellisenseTest, TUWhenRegionInactiveMissingThenCountIsZero) {
  char program[] = "void foo() { }";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));
  CComPtr<IDxcFile> file;
  unsigned resultCount;
  IDxcSourceRange **results;
  VERIFY_SUCCEEDED(result.TU->GetFile("filename.hlsl", &file));
  VERIFY_SUCCEEDED(result.TU->GetSkippedRanges(file.p, &resultCount, &results));
  VERIFY_ARE_EQUAL(0U, resultCount);
  VERIFY_IS_NULL(results);
}

TEST_F(DXIntellisenseTest, TUWhenRegionInactiveThenEndIsBeforeElseHash) {
  char program[] = "#ifdef NOT // a comment\r\n"
                   "int foo() { }\r\n"
                   "#else  // a comment\r\n"
                   "int bar() { }\r\n"
                   "#endif  // a comment\r\n";
  DxcTranslationUnitFlags options =
      (DxcTranslationUnitFlags)(DxcTranslationUnitFlags_DetailedPreprocessingRecord |
                                DxcTranslationUnitFlags_UseCallerThread);
  CompilationResult result(CompilationResult::CreateForProgram(
      program, _countof(program), &options));
  CComPtr<IDxcFile> file;
  unsigned resultCount;
  IDxcSourceRange **results;
  VERIFY_SUCCEEDED(result.TU->GetFile("filename.hlsl", &file));
  VERIFY_SUCCEEDED(result.TU->GetSkippedRanges(file.p, &resultCount, &results));

  ::WEX::TestExecution::DisableVerifyExceptions disable;
  VERIFY_ARE_EQUAL(1U, resultCount);
  for (unsigned i = 0; i < resultCount; ++i) {
    CComPtr<IDxcSourceLocation> endLoc;
    VERIFY_SUCCEEDED(results[i]->GetEnd(&endLoc));
    unsigned line, col, offset;
    VERIFY_SUCCEEDED(
        endLoc->GetSpellingLocation(nullptr, &line, &col, &offset));
    VERIFY_ARE_EQUAL(3U, line);
    VERIFY_ARE_EQUAL(1U, col);
    results[i]->Release();
  }
  CoTaskMemFree(results);
}

TEST_F(DXIntellisenseTest, TUWhenRegionInactiveThenEndIsBeforeEndifHash) {
  char program[] = "#ifdef NOT // a comment\r\n"
                   "int bar() { }\r\n"
                   "#endif  // a comment\r\n";
  DxcTranslationUnitFlags options =
      (DxcTranslationUnitFlags)(DxcTranslationUnitFlags_DetailedPreprocessingRecord |
                                DxcTranslationUnitFlags_UseCallerThread);
  CompilationResult result(CompilationResult::CreateForProgram(
      program, _countof(program), &options));
  CComPtr<IDxcFile> file;
  unsigned resultCount;
  IDxcSourceRange **results;
  VERIFY_SUCCEEDED(result.TU->GetFile("filename.hlsl", &file));
  VERIFY_SUCCEEDED(result.TU->GetSkippedRanges(file.p, &resultCount, &results));

  ::WEX::TestExecution::DisableVerifyExceptions disable;
  VERIFY_ARE_EQUAL(1U, resultCount);
  for (unsigned i = 0; i < resultCount; ++i) {
    CComPtr<IDxcSourceLocation> endLoc;
    VERIFY_SUCCEEDED(results[i]->GetEnd(&endLoc));
    unsigned line, col, offset;
    VERIFY_SUCCEEDED(
        endLoc->GetSpellingLocation(nullptr, &line, &col, &offset));
    VERIFY_ARE_EQUAL(3U, line);
    VERIFY_ARE_EQUAL(1U, col);
    results[i]->Release();
  }
  CoTaskMemFree(results);
}

TEST_F(DXIntellisenseTest, TUWhenRegionInactiveThenStartIsAtIfdefEol) {
  char program[] = "#ifdef NOT // a comment\r\n"
                   "int foo() { }\r\n"
                   "#else  // a comment\r\n"
                   "int bar() { }\r\n"
                   "#endif  // a comment\r\n";
  DxcTranslationUnitFlags options =
      (DxcTranslationUnitFlags)(DxcTranslationUnitFlags_DetailedPreprocessingRecord |
                                DxcTranslationUnitFlags_UseCallerThread);
  CompilationResult result(CompilationResult::CreateForProgram(
      program, _countof(program), &options));
  CComPtr<IDxcFile> file;
  unsigned resultCount;
  IDxcSourceRange **results;
  VERIFY_SUCCEEDED(result.TU->GetFile("filename.hlsl", &file));
  VERIFY_SUCCEEDED(result.TU->GetSkippedRanges(file.p, &resultCount, &results));

  ::WEX::TestExecution::DisableVerifyExceptions disable;
  VERIFY_ARE_EQUAL(1U, resultCount);
  for (unsigned i = 0; i < resultCount; ++i) {
    CComPtr<IDxcSourceLocation> startLoc;
    VERIFY_SUCCEEDED(results[i]->GetStart(&startLoc));
    unsigned line, col, offset;
    VERIFY_SUCCEEDED(
        startLoc->GetSpellingLocation(nullptr, &line, &col, &offset));
    VERIFY_ARE_EQUAL(1U, line);
    VERIFY_ARE_EQUAL(24U, col);
    results[i]->Release();
  }
  CoTaskMemFree(results);
}

std::ostream &operator<<(std::ostream &os, CComPtr<IDxcSourceLocation> &loc) {
  CComPtr<IDxcFile> locFile;
  unsigned locLine, locCol, locOffset;
  loc->GetSpellingLocation(&locFile, &locLine, &locCol, &locOffset);
  os << locLine << ':' << locCol << '@' << locOffset;
  return os;
}

std::wostream &operator<<(std::wostream &os, CComPtr<IDxcSourceLocation> &loc) {
  CComPtr<IDxcFile> locFile;
  unsigned locLine, locCol, locOffset;
  loc->GetSpellingLocation(&locFile, &locLine, &locCol, &locOffset);
  os << locLine << L':' << locCol << L'@' << locOffset;
  return os;
}

TEST_F(DXIntellisenseTest, TUWhenUnsaveFileThenOK) {
  // Verify that an unsaved file using the library-provided implementation still
  // works.
  const char fileName[] = "filename.hlsl";
  char program[] = "[numthreads(1, 1, 1)]\r\n"
                   "[shader(\"compute\")]\r\n"
                   "void main( uint3 DTid : SV_DispatchThreadID )\r\n"
                   "{\r\n"
                   "}";
  bool useBuiltInValues[] = {false, true};

  HlslIntellisenseSupport support;
  VERIFY_SUCCEEDED(support.Initialize());
  for (bool useBuiltIn : useBuiltInValues) {
    CComPtr<IDxcIntelliSense> isense;
    CComPtr<IDxcIndex> tuIndex;
    CComPtr<IDxcTranslationUnit> tu;
    CComPtr<IDxcUnsavedFile> unsavedFile;
    DxcTranslationUnitFlags localOptions;
    const char **commandLineArgs = nullptr;
    int commandLineArgsCount = 0;

    VERIFY_SUCCEEDED(support.CreateIntellisense(&isense));
    VERIFY_SUCCEEDED(isense->CreateIndex(&tuIndex));
    VERIFY_SUCCEEDED(isense->GetDefaultEditingTUOptions(&localOptions));

    if (useBuiltIn)
      VERIFY_SUCCEEDED(isense->CreateUnsavedFile(
          fileName, program, strlen(program), &unsavedFile));
    else
      VERIFY_SUCCEEDED(
          TrivialDxcUnsavedFile::Create(fileName, program, &unsavedFile));

    VERIFY_SUCCEEDED(tuIndex->ParseTranslationUnit(
        fileName, commandLineArgs, commandLineArgsCount, &(unsavedFile.p), 1,
        localOptions, &tu));

    // No errors expected.
    unsigned numDiagnostics;
    VERIFY_SUCCEEDED(tu->GetNumDiagnostics(&numDiagnostics));
    VERIFY_ARE_EQUAL(0U, numDiagnostics);

    CComPtr<IDxcCursor> tuCursor;
    CComInterfaceArray<IDxcCursor> cursors;

    VERIFY_SUCCEEDED(tu->GetCursor(&tuCursor));
    VERIFY_SUCCEEDED(
        tuCursor->GetChildren(0, 20, cursors.size_ref(), cursors.data_ref()));
    std::wstringstream offsetStream;
    for (IDxcCursor *pCursor : cursors) {
      CComPtr<IDxcSourceRange> range;
      CComPtr<IDxcSourceLocation> location;
      CComPtr<IDxcSourceLocation> rangeStart, rangeEnd;
      CComBSTR name;
      VERIFY_SUCCEEDED(pCursor->GetExtent(&range));
      VERIFY_SUCCEEDED(range->GetStart(&rangeStart));
      VERIFY_SUCCEEDED(range->GetEnd(&rangeEnd));
      VERIFY_SUCCEEDED(pCursor->GetDisplayName(&name));
      VERIFY_SUCCEEDED(pCursor->GetLocation(&location));
      offsetStream << (LPWSTR)name << " - spelling " << location << " - extent "
                   << rangeStart << " .. " << rangeEnd << std::endl;
    }
    // Format for a location is line:col@offset
    VERIFY_ARE_EQUAL_WSTR(
        L"main(uint3) - spelling 3:6@49 - extent 3:1@44 .. 5:2@95\n",
        offsetStream.str().c_str());
  }
}

TEST_F(DXIntellisenseTest, QualifiedNameClass) {
  char program[] = "class TheClass {\r\n"
                   "};\r\n"
                   "TheClass C;";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));
  ExpectQualifiedName(result.TU, 3, 1, L"TheClass");
}

TEST_F(DXIntellisenseTest, CursorWhenGlobalVariableThenSimpleNames) {
  char program[] = "namespace Ns { class TheClass {\r\n"
                   "}; }\r\n"
                   "Ns::TheClass C;";

  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));

  // Qualified name does not include type.
  ExpectQualifiedName(result.TU, 3, 14, L"C");
  // Decalaration name includes type.
  ExpectDeclarationText(result.TU, 3, 14, L"Ns::TheClass C");

  // Semicolon is empty.
  ExpectQualifiedName(result.TU, 3, 15, L"");
}

TEST_F(DXIntellisenseTest, CursorWhenTypeOfVariableDeclThenNamesHaveType) {
  char program[] = "namespace Ns { class TheClass {\r\n"
                   "}; }\r\n"
                   "Ns::TheClass C;";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));
  ExpectQualifiedName(result.TU, 3, 1, L"Ns");
  ExpectDeclarationText(result.TU, 3, 1, L"namespace Ns");

  ExpectQualifiedName(result.TU, 3, 6, L"Ns::TheClass");
  ExpectDeclarationText(result.TU, 3, 6, L"class Ns::TheClass");
}

TEST_F(DXIntellisenseTest, CursorTypeUsedNamespace) {
  char program[] = "namespace Ns { class TheClass {\n"
                   "}; }\n"
                   "using namespace Ns;\n"
                   "TheClass C;";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));
  ExpectQualifiedName(result.TU, 4, 4, L"Ns::TheClass");
}

TEST_F(DXIntellisenseTest, CursorWhenVariableRefThenSimpleNames) {
  char program[] = "namespace Ns { class TheClass {\r\n"
                   "public: int f;\r\n"
                   "}; }\r\n"
                   "void fn() {\r\n"
                   "Ns::TheClass C;\r\n"
                   "C.f = 1;\r\n"
                   "}";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));
  ExpectCursorAt(result.TU, 6, 1, DxcCursor_DeclRefExpr);
  ExpectQualifiedName(result.TU, 6, 1, L"C");
}

TEST_F(DXIntellisenseTest, CursorWhenFieldRefThenSimpleNames) {
  char program[] = "namespace Ns { class TheClass {\r\n"
                   "public: int f;\r\n"
                   "}; }\r\n"
                   "void fn() {\r\n"
                   "Ns::TheClass C;\r\n"
                   "C.f = 1;\r\n"
                   "}";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));
  ExpectQualifiedName(result.TU, 6, 3, L"int f");
}

TEST_F(DXIntellisenseTest, QualifiedNameVariable) {
  char program[] = "namespace Ns { class TheClass {\r\n"
                   "}; }\r\n"
                   "Ns::TheClass C;";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));
  ExpectQualifiedName(result.TU, 3, 14, L"C");
}

TEST_F(DXIntellisenseTest, CursorWhenOverloadedResolvedThenDirectSymbol) {
  char program[] = "int abc(int);\r\n"
                   "float abc(float);\r\n"
                   "void foo() {\r\n"
                   "int i = abc(123);\r\n"
                   "}\r\n";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));
  CComPtr<IDxcCursor> cursor;
  GetCursorAt(result.TU, 4, 10, &cursor);
  DxcCursorKind kind;

  // 'abc' in 'abc(123)' is an expression that refers to a declaration.
  ASSERT_HRESULT_SUCCEEDED(cursor->GetKind(&kind));
  EXPECT_EQ(DxcCursor_DeclRefExpr, kind);

  // The referenced declaration is a function declaration.
  CComPtr<IDxcCursor> referenced;
  ASSERT_HRESULT_SUCCEEDED(cursor->GetReferencedCursor(&referenced));
  ASSERT_HRESULT_SUCCEEDED(referenced->GetKind(&kind));
  EXPECT_EQ(DxcCursor_FunctionDecl, kind);
}

TEST_F(DXIntellisenseTest, CursorWhenOverloadedIncompleteThenInvisible) {
  char program[] = "int abc(int);\r\n"
                   "float abc(float);\r\n"
                   "void foo() {\r\n"
                   "int i = abc();\r\n"
                   "}";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));
  CComPtr<IDxcCursor> cursor;
  GetCursorAt(result.TU, 4, 10, &cursor);
  DxcCursorKind kind;

  // 'abc' in 'abc()' is just part of the declaration - it's not a valid
  // standalone entity
  ASSERT_HRESULT_SUCCEEDED(cursor->GetKind(&kind));
  EXPECT_EQ(DxcCursor_DeclStmt, kind);

  // The child of the declaration statement is the declaration.
  CComPtr<IDxcCursor> decl;
  ASSERT_HRESULT_SUCCEEDED(GetFirstChildFromCursor(cursor, &decl));
  ASSERT_HRESULT_SUCCEEDED(decl->GetKind(&kind));
  EXPECT_EQ(DxcCursor_VarDecl, kind);
}

TEST_F(DXIntellisenseTest, CursorWhenVariableUsedThenDeclarationAvailable) {
  char program[] = "int foo() {\r\n"
                   "int i = 1;\r\n"
                   "return i;\r\n"
                   "}";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));
  CComPtr<IDxcCursor> cursor;

  // 'abc' in 'abc()' is just part of the declaration - it's not a valid
  // standalone entity
  ExpectCursorAt(result.TU, 3, 8, DxcCursor_DeclRefExpr, &cursor);

  // The referenced declaration is a variable declaration.
  CComPtr<IDxcCursor> referenced;
  DxcCursorKind kind;
  ASSERT_HRESULT_SUCCEEDED(cursor->GetReferencedCursor(&referenced));
  ASSERT_HRESULT_SUCCEEDED(referenced->GetKind(&kind));
  EXPECT_EQ(DxcCursor_VarDecl, kind);

  CComBSTR name;
  ASSERT_HRESULT_SUCCEEDED(
      referenced->GetFormattedName(DxcCursorFormatting_Default, &name));
  EXPECT_STREQW(L"i", (LPWSTR)name);
}

// TODO: get a referenced local variable as 'int localVar';
// TODO: get a referenced type as 'class Something';
// TODO: having the caret on a function name in definition, highlights calls to
// it
// TODO: get a class name without the template arguments
// TODO: get a class name with ftemplate arguments
// TODO: code completion for a built-in function
// TODO: code completion for a built-in method

TEST_F(DXIntellisenseTest, CursorWhenFunctionThenSignatureAvailable) {
  char program[] = "int myfunc(int a, float b) { return a + b; }\r\n"
                   "int foo() {\r\n"
                   "myfunc(1, 2.0f);\r\n"
                   "}";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));
  CComPtr<IDxcCursor> cursor;

  ExpectCursorAt(result.TU, 3, 1, DxcCursor_DeclRefExpr, &cursor);
  // TODO - how to get signature?
}

TEST_F(DXIntellisenseTest, CursorWhenFunctionThenParamsAvailable) {
  char program[] = "int myfunc(int a, float b) { return a + b; }\r\n"
                   "int foo() {\r\n"
                   "myfunc(1, 2.0f);\r\n"
                   "}";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));
  CComPtr<IDxcCursor> cursor;

  ExpectCursorAt(result.TU, 3, 1, DxcCursor_DeclRefExpr, &cursor);

  int argCount;
  VERIFY_SUCCEEDED(cursor->GetNumArguments(&argCount));
  VERIFY_ARE_EQUAL(-1, argCount); // The reference doesn't have the argument
                                  // count - we need to resolve to func decl

  // TODO - how to get signature?
}

TEST_F(DXIntellisenseTest, CursorWhenFunctionThenReturnTypeAvailable) {
  char program[] = "int myfunc(int a, float b) { return a + b; }\r\n"
                   "int foo() {\r\n"
                   "myfunc(1, 2.0f);\r\n"
                   "}";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));
  CComPtr<IDxcCursor> cursor;

  ExpectCursorAt(result.TU, 3, 1, DxcCursor_DeclRefExpr, &cursor);
  // TODO - how to get signature?
}

TEST_F(DXIntellisenseTest, CursorWhenReferenceThenDefinitionAvailable) {
  char program[] = "int myfunc(int a, float b) { return a + b; }\r\n"
                   "int foo() {\r\n"
                   "myfunc(1, 2.0f);\r\n"
                   "}";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));
  CComPtr<IDxcCursor> cursor;
  CComPtr<IDxcCursor> defCursor;
  CComPtr<IDxcSourceLocation> defLocation;
  CComPtr<IDxcFile> defFile;
  unsigned line, col, offset;

  ExpectCursorAt(result.TU, 3, 1, DxcCursor_DeclRefExpr, &cursor);
  VERIFY_SUCCEEDED(cursor->GetDefinitionCursor(&defCursor));
  VERIFY_IS_NOT_NULL(defCursor.p);

  DxcCursorKind kind;
  VERIFY_SUCCEEDED(defCursor->GetKind(&kind));
  VERIFY_ARE_EQUAL(DxcCursor_FunctionDecl, kind);

  VERIFY_SUCCEEDED(defCursor->GetLocation(&defLocation));
  VERIFY_SUCCEEDED(
      defLocation->GetSpellingLocation(&defFile, &line, &col, &offset));
  VERIFY_ARE_EQUAL(1U, line);
  VERIFY_ARE_EQUAL(5U, col);    // Points to 'myfunc'
  VERIFY_ARE_EQUAL(4U, offset); // Offset is zero-based
}

TEST_F(DXIntellisenseTest, CursorWhenFindAtBodyCallThenMatch) {
  char program[] = "int f();\r\n"
                   "int main() {\r\n"
                   "  f(); }";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));

  CComPtr<IDxcCursor> cursor;
  ExpectCursorAt(result.TU, 3, 3, DxcCursor_DeclRefExpr, &cursor);
}

TEST_F(DXIntellisenseTest, CursorWhenFindAtGlobalThenMatch) {
  char program[] = "int a;";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));

  CComPtr<IDxcCursor> cursor;
  ExpectCursorAt(result.TU, 1, 4, DxcCursor_VarDecl, &cursor);
}

TEST_F(DXIntellisenseTest, CursorWhenFindBeforeBodyCallThenMatch) {
  char program[] = "int f();\r\n"
                   "int main() {\r\n"
                   "  f(); }";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));

  CComPtr<IDxcCursor> cursor;
  ExpectCursorAt(result.TU, 3, 1, DxcCursor_CompoundStmt, &cursor);

  CComPtr<IDxcCursor> snappedCursor;
  CComPtr<IDxcSourceLocation> location;
  DxcCursorKind cursorKind;
  GetLocationAt(result.TU, 3, 1, &location);
  VERIFY_SUCCEEDED(cursor->GetSnappedChild(location, &snappedCursor));
  VERIFY_IS_NOT_NULL(snappedCursor.p);
  VERIFY_SUCCEEDED(snappedCursor->GetKind(&cursorKind));
  VERIFY_ARE_EQUAL(DxcCursor_DeclRefExpr, cursorKind);
}

TEST_F(DXIntellisenseTest, CursorWhenFindBeforeGlobalThenMatch) {
  char program[] = "    int a;";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));

  CComPtr<IDxcCursor> cursor;
  ExpectCursorAt(result.TU, 1, 1, DxcCursor_NoDeclFound, &cursor);

  cursor.Release();

  CComPtr<IDxcCursor> snappedCursor;
  CComPtr<IDxcSourceLocation> location;
  DxcCursorKind cursorKind;
  GetLocationAt(result.TU, 1, 1, &location);
  VERIFY_SUCCEEDED(result.TU->GetCursor(&cursor));
  VERIFY_SUCCEEDED(cursor->GetSnappedChild(location, &snappedCursor));
  VERIFY_IS_NOT_NULL(snappedCursor.p);
  VERIFY_SUCCEEDED(snappedCursor->GetKind(&cursorKind));
  VERIFY_ARE_EQUAL(DxcCursor_VarDecl, cursorKind);
}

TEST_F(DXIntellisenseTest, FileWhenSameThenEqual) {
  char program[] = "int a;\r\nint b;";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));

  CComPtr<IDxcSourceLocation> location0, location1;
  CComPtr<IDxcFile> file0, file1;
  unsigned line, col, offset;
  BOOL isEqual;
  GetLocationAt(result.TU, 1, 1, &location0);
  GetLocationAt(result.TU, 2, 1, &location1);
  VERIFY_SUCCEEDED(
      location0->GetSpellingLocation(&file0, &line, &col, &offset));
  VERIFY_SUCCEEDED(
      location1->GetSpellingLocation(&file1, &line, &col, &offset));
  VERIFY_SUCCEEDED(file0->IsEqualTo(file1, &isEqual));
  VERIFY_ARE_EQUAL(TRUE, isEqual);
}

TEST_F(DXIntellisenseTest, FileWhenNotSameThenNotEqual) {
  char program[] = "int a;\r\nint b;";
  CompilationResult result0(
      CompilationResult::CreateForProgram(program, _countof(program)));
  CompilationResult result1(
      CompilationResult::CreateForProgram(program, _countof(program)));

  CComPtr<IDxcSourceLocation> location0, location1;
  CComPtr<IDxcFile> file0, file1;
  unsigned line, col, offset;
  BOOL isEqual;
  GetLocationAt(result0.TU, 1, 1, &location0);
  GetLocationAt(result1.TU, 1, 1, &location1);
  VERIFY_SUCCEEDED(
      location0->GetSpellingLocation(&file0, &line, &col, &offset));
  VERIFY_SUCCEEDED(
      location1->GetSpellingLocation(&file1, &line, &col, &offset));
  VERIFY_SUCCEEDED(file0->IsEqualTo(file1, &isEqual));
  VERIFY_ARE_EQUAL(FALSE, isEqual);
}

TEST_F(DXIntellisenseTest, TypeWhenICEThenEval) {
  // When an ICE is present in a declaration, it appears in the name.
  char program[] = "float c[(1, 2)];\r\n"
                   "float main() : SV_Target\r\n"
                   "{ return c[0]; }";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));
  VERIFY_IS_TRUE(result.ParseSucceeded());
  CComPtr<IDxcCursor> cCursor;
  ExpectCursorAt(result.TU, 1, 7, DxcCursor_VarDecl, &cCursor);
  CComPtr<IDxcType> typeCursor;
  VERIFY_SUCCEEDED(cCursor->GetCursorType(&typeCursor));
  CComHeapPtr<char> name;
  VERIFY_SUCCEEDED(typeCursor->GetSpelling(&name));
  VERIFY_ARE_EQUAL_STR("const float [2]",
                       name); // global variables converted to const by default
}

TEST_F(DXIntellisenseTest, CompletionWhenResultsAvailable) {
  char program[] = "struct MyStruct {};"
                   "MyStr";
  CompilationResult result(
      CompilationResult::CreateForProgram(program, _countof(program)));
  VERIFY_IS_FALSE(result.ParseSucceeded());
  const char *fileName = "filename.hlsl";
  CComPtr<IDxcUnsavedFile> unsavedFile;
  VERIFY_SUCCEEDED(
      TrivialDxcUnsavedFile::Create(fileName, program, &unsavedFile));
  CComPtr<IDxcCodeCompleteResults> codeCompleteResults;
  VERIFY_SUCCEEDED(result.TU->CodeCompleteAt(fileName, 2, 1, &unsavedFile.p, 1,
                                             DxcCodeCompleteFlags_None,
                                             &codeCompleteResults));
  unsigned numResults;
  VERIFY_SUCCEEDED(codeCompleteResults->GetNumResults(&numResults));
  VERIFY_IS_GREATER_THAN_OR_EQUAL(numResults, 1u);
  CComPtr<IDxcCompletionResult> completionResult;
  VERIFY_SUCCEEDED(codeCompleteResults->GetResultAt(0, &completionResult));
  DxcCursorKind completionResultCursorKind;
  VERIFY_SUCCEEDED(
      completionResult->GetCursorKind(&completionResultCursorKind));
  VERIFY_ARE_EQUAL(DxcCursor_StructDecl, completionResultCursorKind);
  CComPtr<IDxcCompletionString> completionString;
  VERIFY_SUCCEEDED(completionResult->GetCompletionString(&completionString));
  unsigned numCompletionChunks;
  VERIFY_SUCCEEDED(
      completionString->GetNumCompletionChunks(&numCompletionChunks));
  VERIFY_ARE_EQUAL(1u, numCompletionChunks);
  DxcCompletionChunkKind completionChunkKind;
  VERIFY_SUCCEEDED(
      completionString->GetCompletionChunkKind(0, &completionChunkKind));
  VERIFY_ARE_EQUAL(DxcCompletionChunk_TypedText, completionChunkKind);
  CComHeapPtr<char> completionChunkText;
  VERIFY_SUCCEEDED(
      completionString->GetCompletionChunkText(0, &completionChunkText));
  VERIFY_ARE_EQUAL_STR("MyStruct", completionChunkText);
}
