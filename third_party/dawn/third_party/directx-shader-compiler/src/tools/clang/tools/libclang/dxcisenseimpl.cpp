///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcisenseimpl.cpp                                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements the DirectX Compiler IntelliSense component.                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Sema/SemaConsumer.h"
#include "clang/Sema/SemaHLSL.h"
#include "llvm/Support/Host.h"

#include "dxc/Support/Global.h"
#include "dxc/Support/WinFunctions.h"
#include "dxc/Support/WinIncludes.h"
#include "dxcisenseimpl.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MSFileSystem.h"

///////////////////////////////////////////////////////////////////////////////

HRESULT CreateDxcIntelliSense(REFIID riid, LPVOID *ppv) throw() {
  CComPtr<DxcIntelliSense> isense =
      CreateOnMalloc<DxcIntelliSense>(DxcGetThreadMallocNoRef());
  if (isense == nullptr) {
    *ppv = nullptr;
    return E_OUTOFMEMORY;
  }

  return isense.p->QueryInterface(riid, ppv);
}

///////////////////////////////////////////////////////////////////////////////

// This is exposed as a helper class, but the implementation works on
// interfaces; we expect callers should be able to use their own.
class DxcBasicUnsavedFile : public IDxcUnsavedFile {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  LPSTR m_fileName;
  LPSTR m_contents;
  unsigned m_length;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcBasicUnsavedFile)
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcUnsavedFile>(this, iid, ppvObject);
  }

  DxcBasicUnsavedFile(IMalloc *pMalloc);
  ~DxcBasicUnsavedFile();
  HRESULT Initialize(LPCSTR fileName, LPCSTR contents, unsigned length);
  static HRESULT Create(LPCSTR fileName, LPCSTR contents, unsigned length,
                        IDxcUnsavedFile **pObject);

  HRESULT STDMETHODCALLTYPE GetFileName(LPSTR *pFileName) override;
  HRESULT STDMETHODCALLTYPE GetContents(LPSTR *pContents) override;
  HRESULT STDMETHODCALLTYPE GetLength(unsigned *pLength) override;
};

///////////////////////////////////////////////////////////////////////////////

static bool IsCursorKindQualifiedByParent(CXCursorKind kind) throw();

///////////////////////////////////////////////////////////////////////////////

static HRESULT AnsiToBSTR(const char *text, BSTR *pValue) throw() {
  if (pValue == nullptr)
    return E_POINTER;
  *pValue = nullptr;
  if (text == nullptr) {
    return S_OK;
  }

  //
  // charCount will include the null terminating character, because
  // -1 is used as the input length.
  // SysAllocStringLen takes the character count and adds one for the
  // null terminator, so we remove that from charCount for that call.
  //
  int charCount = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
  if (charCount <= 0) {
    return HRESULT_FROM_WIN32(GetLastError());
  }
  *pValue = SysAllocStringLen(nullptr, charCount - 1);
  if (*pValue == nullptr) {
    return E_OUTOFMEMORY;
  }

  MultiByteToWideChar(CP_UTF8, 0, text, -1, *pValue, charCount);

  return S_OK;
}

static LPVOID CoTaskMemAllocZero(SIZE_T cb) throw() {
  LPVOID result = CoTaskMemAlloc(cb);
  if (result != nullptr) {
    ZeroMemory(result, cb);
  }
  return result;
}

/// <summary>Allocates one or more zero-initialized structures in task
/// memory.</summary> <remarks> This does more work than CoTaskMemAlloc, but
/// often simplifies cleanup for error cases.
/// </remarks>
template <typename T>
static void CoTaskMemAllocZeroElems(SIZE_T elementCount, T **buf) throw() {
  *buf = reinterpret_cast<T *>(CoTaskMemAllocZero(elementCount * sizeof(T)));
}

static HRESULT CXStringToAnsiAndDispose(CXString value, LPSTR *pValue) throw() {
  if (pValue == nullptr)
    return E_POINTER;
  *pValue = nullptr;
  const char *text = clang_getCString(value);
  if (text == nullptr) {
    return S_OK;
  }
  size_t len = strlen(text);
  *pValue = (char *)CoTaskMemAlloc(len + 1);
  if (*pValue == nullptr) {
    return E_OUTOFMEMORY;
  }
  memcpy(*pValue, text, len + 1);
  clang_disposeString(value);
  return S_OK;
}

static HRESULT CXStringToBSTRAndDispose(CXString value, BSTR *pValue) throw() {
  HRESULT hr = AnsiToBSTR(clang_getCString(value), pValue);
  clang_disposeString(value);
  return hr;
}

static void CleanupUnsavedFiles(CXUnsavedFile *files,
                                unsigned file_count) throw() {
  for (unsigned i = 0; i < file_count; ++i) {
    CoTaskMemFree((LPVOID)files[i].Filename);
    CoTaskMemFree((LPVOID)files[i].Contents);
  }

  delete[] files;
}

static HRESULT CoTaskMemAllocString(const char *src, LPSTR *pResult) throw() {
  assert(src != nullptr);

  if (pResult == nullptr) {
    return E_POINTER;
  }

  unsigned len = strlen(src);
  *pResult = (char *)CoTaskMemAlloc(len + 1);
  if (*pResult == nullptr) {
    return E_OUTOFMEMORY;
  }
  CopyMemory(*pResult, src, len + 1);
  return S_OK;
}

static HRESULT GetCursorQualifiedName(CXCursor cursor, bool includeTemplateArgs,
                                      BSTR *pResult) throw() {
  *pResult = nullptr;

  if (IsCursorKindQualifiedByParent(clang_getCursorKind(cursor))) {
    CXString text;
    text = clang_getCursorSpellingWithFormatting(
        cursor, CXCursorFormatting_SuppressTagKeyword);
    return CXStringToBSTRAndDispose(text, pResult);
  }

  if (clang_getCursorKind(cursor) == CXCursor_VarDecl ||
      (clang_getCursorKind(cursor) == CXCursor_DeclRefExpr &&
       clang_getCursorKind(clang_getCursorReferenced(cursor)) ==
           CXCursor_VarDecl)) {
    return CXStringToBSTRAndDispose(
        clang_getCursorSpellingWithFormatting(
            cursor, CXCursorFormatting_SuppressTagKeyword |
                        CXCursorFormatting_SuppressSpecifiers),
        pResult);
  }
  return CXStringToBSTRAndDispose(
      clang_getCursorSpellingWithFormatting(
          cursor, CXCursorFormatting_SuppressTagKeyword),
      pResult);
}

static bool IsCursorKindQualifiedByParent(CXCursorKind kind) throw() {
  return kind == CXCursor_TypeRef || kind == CXCursor_TemplateRef ||
         kind == CXCursor_NamespaceRef || kind == CXCursor_MemberRef ||
         kind == CXCursor_OverloadedDeclRef || kind == CXCursor_StructDecl ||
         kind == CXCursor_UnionDecl || kind == CXCursor_ClassDecl ||
         kind == CXCursor_EnumDecl || kind == CXCursor_FieldDecl ||
         kind == CXCursor_EnumConstantDecl || kind == CXCursor_FunctionDecl ||
         kind == CXCursor_CXXMethod || kind == CXCursor_Namespace ||
         kind == CXCursor_Constructor || kind == CXCursor_Destructor ||
         kind == CXCursor_FunctionTemplate || kind == CXCursor_ClassTemplate ||
         kind == CXCursor_ClassTemplatePartialSpecialization;
}

template <typename TIface>
static void SafeReleaseIfaceArray(TIface **arr, unsigned count) throw() {
  if (arr != nullptr) {
    for (unsigned i = 0; i < count; i++) {
      if (arr[i] != nullptr) {
        arr[i]->Release();
        arr[i] = nullptr;
      }
    }
  }
}

static HRESULT SetupUnsavedFiles(IDxcUnsavedFile **unsaved_files,
                                 unsigned num_unsaved_files,
                                 CXUnsavedFile **files) {
  *files = nullptr;
  if (num_unsaved_files == 0) {
    return S_OK;
  }

  HRESULT hr = S_OK;
  CXUnsavedFile *localFiles =
      new (std::nothrow) CXUnsavedFile[num_unsaved_files];
  IFROOM(localFiles);
  ZeroMemory(localFiles, num_unsaved_files * sizeof(localFiles[0]));
  for (unsigned i = 0; i < num_unsaved_files; ++i) {
    if (unsaved_files[i] == nullptr) {
      hr = E_INVALIDARG;
      break;
    }

    LPSTR strPtr;
    hr = unsaved_files[i]->GetFileName(&strPtr);
    if (FAILED(hr))
      break;
    localFiles[i].Filename = strPtr;

    hr = unsaved_files[i]->GetContents(&strPtr);
    if (FAILED(hr))
      break;
    localFiles[i].Contents = strPtr;

    hr = unsaved_files[i]->GetLength((unsigned *)&localFiles[i].Length);
    if (FAILED(hr))
      break;
  }

  if (SUCCEEDED(hr)) {
    *files = localFiles;
  } else {
    CleanupUnsavedFiles(localFiles, num_unsaved_files);
  }

  return hr;
}

struct PagedCursorVisitorContext {
  unsigned skip;               // References to skip at the beginning.
  unsigned top;                // Maximum number of references to get.
  CSimpleArray<CXCursor> refs; // Cursor references found.
};

static CXVisitorResult LIBCLANG_CC PagedCursorFindVisit(void *context,
                                                        CXCursor c,
                                                        CXSourceRange range) {
  PagedCursorVisitorContext *pagedContext =
      (PagedCursorVisitorContext *)context;
  if (pagedContext->skip > 0) {
    --pagedContext->skip;
    return CXVisit_Continue;
  }

  pagedContext->refs.Add(c);

  --pagedContext->top;
  return (pagedContext->top == 0) ? CXVisit_Break : CXVisit_Continue;
}

CXChildVisitResult LIBCLANG_CC PagedCursorTraverseVisit(
    CXCursor cursor, CXCursor parent, CXClientData client_data) {
  PagedCursorVisitorContext *pagedContext =
      (PagedCursorVisitorContext *)client_data;
  if (pagedContext->skip > 0) {
    --pagedContext->skip;
    return CXChildVisit_Continue;
  }

  pagedContext->refs.Add(cursor);

  --pagedContext->top;
  return (pagedContext->top == 0) ? CXChildVisit_Break : CXChildVisit_Continue;
}

static HRESULT PagedCursorVisitorCopyResults(PagedCursorVisitorContext *context,
                                             unsigned *pResultLength,
                                             IDxcCursor ***pResult) {
  *pResultLength = 0;
  *pResult = nullptr;

  unsigned resultLength = context->refs.GetSize();
  CoTaskMemAllocZeroElems(resultLength, pResult);
  if (*pResult == nullptr) {
    return E_OUTOFMEMORY;
  }

  *pResultLength = resultLength;
  HRESULT hr = S_OK;
  for (unsigned i = 0; i < resultLength; ++i) {
    IDxcCursor *newCursor;
    hr = DxcCursor::Create(context->refs[i], &newCursor);
    if (FAILED(hr)) {
      break;
    }
    (*pResult)[i] = newCursor;
  }

  // Clean up any progress on failure.
  if (FAILED(hr)) {
    SafeReleaseIfaceArray(*pResult, resultLength);
    CoTaskMemFree(*pResult);
    *pResult = nullptr;
    *pResultLength = 0;
  }

  return hr;
}

struct SourceCursorVisitorContext {
  const CXSourceLocation &loc;
  CXFile file;
  unsigned offset;

  bool found;
  CXCursor result;
  unsigned resultOffset;
  SourceCursorVisitorContext(const CXSourceLocation &l) : loc(l), found(false) {
    clang_getSpellingLocation(loc, &file, nullptr, nullptr, &offset);
  }
};

static CXChildVisitResult LIBCLANG_CC
SourceCursorVisit(CXCursor cursor, CXCursor parent, CXClientData client_data) {
  SourceCursorVisitorContext *context =
      (SourceCursorVisitorContext *)client_data;

  CXSourceRange range = clang_getCursorExtent(cursor);

  // If the range ends before our location of interest, simply continue; no need
  // to recurse.
  CXFile cursorFile;
  unsigned cursorEndOffset;
  clang_getSpellingLocation(clang_getRangeEnd(range), &cursorFile, nullptr,
                            nullptr, &cursorEndOffset);
  if (cursorFile != context->file || cursorEndOffset < context->offset) {
    return CXChildVisit_Continue;
  }

  // If the range start is before or equal to our position, we cover the
  // location, and we have a result but might need to recurse. If the range
  // start is after, this is where snapping behavior kicks in, and we'll
  // consider it just as good as overlap (for the closest match we have). So we
  // don't in fact need to consider the value, other than to snap to the closest
  // cursor.
  unsigned cursorStartOffset;
  clang_getSpellingLocation(clang_getRangeStart(range), &cursorFile, nullptr,
                            nullptr, &cursorStartOffset);

  bool isKindResult = cursor.kind != CXCursor_CompoundStmt &&
                      cursor.kind != CXCursor_TranslationUnit &&
                      !clang_isInvalid(cursor.kind);
  if (isKindResult) {
    bool cursorIsBetter = context->found == false ||
                          (cursorStartOffset < context->resultOffset) ||
                          (cursorStartOffset == context->resultOffset &&
                           (int)cursor.kind == (int)DxcCursor_DeclRefExpr);
    if (cursorIsBetter) {
      context->found = true;
      context->result = cursor;
      context->resultOffset = cursorStartOffset;
    }
  }

  return CXChildVisit_Recurse;
}

///////////////////////////////////////////////////////////////////////////////

DxcBasicUnsavedFile::DxcBasicUnsavedFile(IMalloc *pMalloc)
    : m_dwRef(0), m_pMalloc(pMalloc), m_fileName(nullptr), m_contents(nullptr) {
}

DxcBasicUnsavedFile::~DxcBasicUnsavedFile() {
  free(m_fileName);
  delete[] m_contents;
}

HRESULT DxcBasicUnsavedFile::Initialize(LPCSTR fileName, LPCSTR contents,
                                        unsigned contentLength) {
  if (fileName == nullptr)
    return E_INVALIDARG;
  if (contents == nullptr)
    return E_INVALIDARG;

  m_fileName = _strdup(fileName);
  if (m_fileName == nullptr)
    return E_OUTOFMEMORY;

  unsigned bufferLength = strlen(contents);
  if (contentLength > bufferLength) {
    contentLength = bufferLength;
  }

  m_contents = new (std::nothrow) char[contentLength + 1];
  if (m_contents == nullptr) {
    free(m_fileName);
    m_fileName = nullptr;
    return E_OUTOFMEMORY;
  }
  CopyMemory(m_contents, contents, contentLength);
  m_contents[contentLength] = '\0';
  m_length = contentLength;
  return S_OK;
}

HRESULT DxcBasicUnsavedFile::Create(LPCSTR fileName, LPCSTR contents,
                                    unsigned contentLength,
                                    IDxcUnsavedFile **pObject) {
  if (pObject == nullptr)
    return E_POINTER;
  *pObject = nullptr;
  DxcBasicUnsavedFile *newValue =
      DxcBasicUnsavedFile::Alloc(DxcGetThreadMallocNoRef());
  if (newValue == nullptr)
    return E_OUTOFMEMORY;
  HRESULT hr = newValue->Initialize(fileName, contents, contentLength);
  if (FAILED(hr)) {
    CComPtr<IMalloc> pTmp(newValue->m_pMalloc);
    newValue->DxcBasicUnsavedFile::~DxcBasicUnsavedFile();
    pTmp->Free(newValue);
    return hr;
  }
  newValue->AddRef();
  *pObject = newValue;
  return S_OK;
}

HRESULT DxcBasicUnsavedFile::GetFileName(LPSTR *pFileName) {
  return CoTaskMemAllocString(m_fileName, pFileName);
}

HRESULT DxcBasicUnsavedFile::GetContents(LPSTR *pContents) {
  return CoTaskMemAllocString(m_contents, pContents);
}

HRESULT DxcBasicUnsavedFile::GetLength(unsigned *pLength) {
  if (pLength == nullptr)
    return E_POINTER;
  *pLength = m_length;
  return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT DxcCursor::Create(const CXCursor &cursor, IDxcCursor **pObject) {
  if (pObject == nullptr)
    return E_POINTER;
  *pObject = nullptr;
  DxcCursor *newValue = DxcCursor::Alloc(DxcGetThreadMallocNoRef());
  if (newValue == nullptr)
    return E_OUTOFMEMORY;
  newValue->Initialize(cursor);
  newValue->AddRef();
  *pObject = newValue;
  return S_OK;
}

void DxcCursor::Initialize(const CXCursor &cursor) { m_cursor = cursor; }

HRESULT DxcCursor::GetExtent(IDxcSourceRange **pValue) {
  DxcThreadMalloc TM(m_pMalloc);
  CXSourceRange range = clang_getCursorExtent(m_cursor);
  return DxcSourceRange::Create(range, pValue);
}

HRESULT DxcCursor::GetLocation(IDxcSourceLocation **pResult) {
  DxcThreadMalloc TM(m_pMalloc);
  return DxcSourceLocation::Create(clang_getCursorLocation(m_cursor), pResult);
}

HRESULT DxcCursor::GetKind(DxcCursorKind *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  *pResult = (DxcCursorKind)clang_getCursorKind(m_cursor);
  return S_OK;
}

HRESULT DxcCursor::GetKindFlags(DxcCursorKindFlags *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  DxcCursorKindFlags f = DxcCursorKind_None;
  CXCursorKind kind = clang_getCursorKind(m_cursor);
  if (0 != clang_isDeclaration(kind))
    f = (DxcCursorKindFlags)(f | DxcCursorKind_Declaration);
  if (0 != clang_isReference(kind))
    f = (DxcCursorKindFlags)(f | DxcCursorKind_Reference);
  if (0 != clang_isExpression(kind))
    f = (DxcCursorKindFlags)(f | DxcCursorKind_Expression);
  if (0 != clang_isStatement(kind))
    f = (DxcCursorKindFlags)(f | DxcCursorKind_Statement);
  if (0 != clang_isAttribute(kind))
    f = (DxcCursorKindFlags)(f | DxcCursorKind_Attribute);
  if (0 != clang_isInvalid(kind))
    f = (DxcCursorKindFlags)(f | DxcCursorKind_Invalid);
  if (0 != clang_isTranslationUnit(kind))
    f = (DxcCursorKindFlags)(f | DxcCursorKind_TranslationUnit);
  if (0 != clang_isPreprocessing(kind))
    f = (DxcCursorKindFlags)(f | DxcCursorKind_Preprocessing);
  if (0 != clang_isUnexposed(kind))
    f = (DxcCursorKindFlags)(f | DxcCursorKind_Unexposed);
  *pResult = f;
  return S_OK;
}

HRESULT DxcCursor::GetSemanticParent(IDxcCursor **pResult) {
  DxcThreadMalloc TM(m_pMalloc);
  return DxcCursor::Create(clang_getCursorSemanticParent(m_cursor), pResult);
}

HRESULT DxcCursor::GetLexicalParent(IDxcCursor **pResult) {
  DxcThreadMalloc TM(m_pMalloc);
  return DxcCursor::Create(clang_getCursorLexicalParent(m_cursor), pResult);
}

HRESULT DxcCursor::GetCursorType(IDxcType **pResult) {
  DxcThreadMalloc TM(m_pMalloc);
  return DxcType::Create(clang_getCursorType(m_cursor), pResult);
}

HRESULT DxcCursor::GetNumArguments(int *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  *pResult = clang_Cursor_getNumArguments(m_cursor);
  return S_OK;
}

HRESULT DxcCursor::GetArgumentAt(int index, IDxcCursor **pResult) {
  DxcThreadMalloc TM(m_pMalloc);
  return DxcCursor::Create(clang_Cursor_getArgument(m_cursor, index), pResult);
}

HRESULT DxcCursor::GetReferencedCursor(IDxcCursor **pResult) {
  DxcThreadMalloc TM(m_pMalloc);
  return DxcCursor::Create(clang_getCursorReferenced(m_cursor), pResult);
}

HRESULT DxcCursor::GetDefinitionCursor(IDxcCursor **pResult) {
  DxcThreadMalloc TM(m_pMalloc);
  return DxcCursor::Create(clang_getCursorDefinition(m_cursor), pResult);
}

HRESULT DxcCursor::FindReferencesInFile(IDxcFile *file, unsigned skip,
                                        unsigned top, unsigned *pResultLength,
                                        IDxcCursor ***pResult) {
  if (pResultLength == nullptr)
    return E_POINTER;
  if (pResult == nullptr)
    return E_POINTER;
  if (file == nullptr)
    return E_INVALIDARG;

  *pResult = nullptr;
  *pResultLength = 0;
  if (top == 0) {
    return S_OK;
  }

  DxcThreadMalloc TM(m_pMalloc);
  CXCursorAndRangeVisitor visitor;
  PagedCursorVisitorContext findReferencesInFileContext;
  findReferencesInFileContext.skip = skip;
  findReferencesInFileContext.top = top;
  visitor.context = &findReferencesInFileContext;
  visitor.visit = PagedCursorFindVisit;

  DxcFile *fileImpl = reinterpret_cast<DxcFile *>(file);
  clang_findReferencesInFile(m_cursor, fileImpl->GetFile(),
                             visitor); // known visitor, so ignore result

  return PagedCursorVisitorCopyResults(&findReferencesInFileContext,
                                       pResultLength, pResult);
}

HRESULT DxcCursor::GetSpelling(LPSTR *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  DxcThreadMalloc TM(m_pMalloc);
  return CXStringToAnsiAndDispose(clang_getCursorSpelling(m_cursor), pResult);
}

HRESULT DxcCursor::IsEqualTo(IDxcCursor *other, BOOL *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  if (other == nullptr) {
    *pResult = FALSE;
  } else {
    DxcCursor *otherImpl = reinterpret_cast<DxcCursor *>(other);
    *pResult = 0 != clang_equalCursors(m_cursor, otherImpl->m_cursor);
  }
  return S_OK;
}

HRESULT DxcCursor::IsNull(BOOL *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  *pResult = 0 != clang_Cursor_isNull(m_cursor);
  return S_OK;
}

HRESULT DxcCursor::IsDefinition(BOOL *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  *pResult = 0 != clang_isCursorDefinition(m_cursor);
  return S_OK;
}

HRESULT DxcCursor::GetDisplayName(BSTR *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  DxcThreadMalloc TM(m_pMalloc);
  return CXStringToBSTRAndDispose(clang_getCursorDisplayName(m_cursor),
                                  pResult);
}

HRESULT DxcCursor::GetQualifiedName(BOOL includeTemplateArgs, BSTR *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  DxcThreadMalloc TM(m_pMalloc);
  return GetCursorQualifiedName(m_cursor, includeTemplateArgs, pResult);
}

HRESULT DxcCursor::GetFormattedName(DxcCursorFormatting formatting,
                                    BSTR *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  DxcThreadMalloc TM(m_pMalloc);
  return CXStringToBSTRAndDispose(
      clang_getCursorSpellingWithFormatting(m_cursor, formatting), pResult);
}

HRESULT DxcCursor::GetChildren(unsigned skip, unsigned top,
                               unsigned *pResultLength, IDxcCursor ***pResult) {
  if (pResultLength == nullptr)
    return E_POINTER;
  if (pResult == nullptr)
    return E_POINTER;

  *pResult = nullptr;
  *pResultLength = 0;
  if (top == 0) {
    return S_OK;
  }

  DxcThreadMalloc TM(m_pMalloc);
  PagedCursorVisitorContext visitorContext;
  visitorContext.skip = skip;
  visitorContext.top = top;
  clang_visitChildren(m_cursor, PagedCursorTraverseVisit,
                      &visitorContext); // known visitor, so ignore result
  return PagedCursorVisitorCopyResults(&visitorContext, pResultLength, pResult);
}

HRESULT DxcCursor::GetSnappedChild(IDxcSourceLocation *location,
                                   IDxcCursor **pResult) {
  if (location == nullptr)
    return E_POINTER;
  if (pResult == nullptr)
    return E_POINTER;

  *pResult = nullptr;

  DxcThreadMalloc TM(m_pMalloc);
  DxcSourceLocation *locationImpl =
      reinterpret_cast<DxcSourceLocation *>(location);
  const CXSourceLocation &snapLocation = locationImpl->GetLocation();
  SourceCursorVisitorContext visitorContext(snapLocation);
  clang_visitChildren(m_cursor, SourceCursorVisit, &visitorContext);
  if (visitorContext.found) {
    return DxcCursor::Create(visitorContext.result, pResult);
  }

  return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

DxcDiagnostic::DxcDiagnostic(IMalloc *pMalloc)
    : m_dwRef(0), m_pMalloc(pMalloc), m_diagnostic(nullptr) {}

DxcDiagnostic::~DxcDiagnostic() {
  if (m_diagnostic) {
    clang_disposeDiagnostic(m_diagnostic);
  }
}

void DxcDiagnostic::Initialize(const CXDiagnostic &diagnostic) {
  m_diagnostic = diagnostic;
}

HRESULT DxcDiagnostic::Create(const CXDiagnostic &diagnostic,
                              IDxcDiagnostic **pObject) {
  if (pObject == nullptr)
    return E_POINTER;
  *pObject = nullptr;
  DxcDiagnostic *newValue = DxcDiagnostic::Alloc(DxcGetThreadMallocNoRef());
  if (newValue == nullptr)
    return E_OUTOFMEMORY;
  newValue->Initialize(diagnostic);
  newValue->AddRef();
  *pObject = newValue;
  return S_OK;
}

HRESULT DxcDiagnostic::FormatDiagnostic(DxcDiagnosticDisplayOptions options,
                                        LPSTR *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  DxcThreadMalloc TM(m_pMalloc);
  return CXStringToAnsiAndDispose(clang_formatDiagnostic(m_diagnostic, options),
                                  pResult);
}

HRESULT DxcDiagnostic::GetSeverity(DxcDiagnosticSeverity *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  *pResult = (DxcDiagnosticSeverity)clang_getDiagnosticSeverity(m_diagnostic);
  return S_OK;
}

HRESULT DxcDiagnostic::GetLocation(IDxcSourceLocation **pResult) {
  return DxcSourceLocation::Create(clang_getDiagnosticLocation(m_diagnostic),
                                   pResult);
}

HRESULT DxcDiagnostic::GetSpelling(LPSTR *pResult) {
  return CXStringToAnsiAndDispose(clang_getDiagnosticSpelling(m_diagnostic),
                                  pResult);
}

HRESULT DxcDiagnostic::GetCategoryText(LPSTR *pResult) {
  return CXStringToAnsiAndDispose(clang_getDiagnosticCategoryText(m_diagnostic),
                                  pResult);
}

HRESULT DxcDiagnostic::GetNumRanges(unsigned *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  *pResult = clang_getDiagnosticNumRanges(m_diagnostic);
  return S_OK;
}

HRESULT DxcDiagnostic::GetRangeAt(unsigned index, IDxcSourceRange **pResult) {
  return DxcSourceRange::Create(clang_getDiagnosticRange(m_diagnostic, index),
                                pResult);
}

HRESULT DxcDiagnostic::GetNumFixIts(unsigned *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  *pResult = clang_getDiagnosticNumFixIts(m_diagnostic);
  return S_OK;
}

HRESULT DxcDiagnostic::GetFixItAt(unsigned index,
                                  IDxcSourceRange **pReplacementRange,
                                  LPSTR *pText) {
  if (pReplacementRange == nullptr)
    return E_POINTER;
  if (pText == nullptr)
    return E_POINTER;
  *pReplacementRange = nullptr;
  *pText = nullptr;

  DxcThreadMalloc TM(m_pMalloc);
  CXSourceRange range;
  CXString text = clang_getDiagnosticFixIt(m_diagnostic, index, &range);
  HRESULT hr = DxcSourceRange::Create(range, pReplacementRange);
  if (SUCCEEDED(hr)) {
    hr = CXStringToAnsiAndDispose(text, pText);
    if (FAILED(hr)) {
      (*pReplacementRange)->Release();
      *pReplacementRange = nullptr;
    }
  }

  return hr;
}

///////////////////////////////////////////////////////////////////////////////

void DxcFile::Initialize(const CXFile &file) { m_file = file; }

HRESULT DxcFile::Create(const CXFile &file, IDxcFile **pObject) {
  if (pObject == nullptr)
    return E_POINTER;
  *pObject = nullptr;
  DxcFile *newValue = DxcFile::Alloc(DxcGetThreadMallocNoRef());
  if (newValue == nullptr)
    return E_OUTOFMEMORY;
  newValue->Initialize(file);
  newValue->AddRef();
  *pObject = newValue;
  return S_OK;
}

HRESULT DxcFile::GetName(LPSTR *pResult) {
  DxcThreadMalloc TM(m_pMalloc);
  return CXStringToAnsiAndDispose(clang_getFileName(m_file), pResult);
}

HRESULT DxcFile::IsEqualTo(IDxcFile *other, BOOL *pResult) {
  if (!pResult)
    return E_POINTER;
  if (other == nullptr) {
    *pResult = FALSE;
  } else {
    // CXFile is an internal pointer into the source manager, and so
    // should be equal for the same file.
    DxcFile *otherImpl = reinterpret_cast<DxcFile *>(other);
    *pResult = (m_file == otherImpl->m_file) ? TRUE : FALSE;
  }
  return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

DxcInclusion::DxcInclusion(IMalloc *pMalloc)
    : m_dwRef(0), m_pMalloc(pMalloc), m_file(nullptr), m_locations(nullptr),
      m_locationLength(0) {}

DxcInclusion::~DxcInclusion() { delete[] m_locations; }

HRESULT DxcInclusion::Initialize(CXFile file, unsigned locations,
                                 CXSourceLocation *pLocations) {
  if (locations) {
    m_locations = new (std::nothrow) CXSourceLocation[locations];
    if (m_locations == nullptr)
      return E_OUTOFMEMORY;
    std::copy(pLocations, pLocations + locations, m_locations);
    m_locationLength = locations;
  }
  m_file = file;
  return S_OK;
}

HRESULT DxcInclusion::Create(CXFile file, unsigned locations,
                             CXSourceLocation *pLocations,
                             IDxcInclusion **pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  *pResult = nullptr;

  CComPtr<DxcInclusion> local;
  local = DxcInclusion::Alloc(DxcGetThreadMallocNoRef());
  if (local == nullptr)
    return E_OUTOFMEMORY;
  HRESULT hr = local->Initialize(file, locations, pLocations);
  if (FAILED(hr))
    return hr;
  *pResult = local.Detach();
  return S_OK;
}

HRESULT DxcInclusion::GetIncludedFile(IDxcFile **pResult) {
  DxcThreadMalloc TM(m_pMalloc);
  return DxcFile::Create(m_file, pResult);
}

HRESULT DxcInclusion::GetStackLength(unsigned *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  *pResult = m_locationLength;
  return S_OK;
}

HRESULT DxcInclusion::GetStackItem(unsigned index,
                                   IDxcSourceLocation **pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  if (index >= m_locationLength)
    return E_INVALIDARG;
  DxcThreadMalloc TM(m_pMalloc);
  return DxcSourceLocation::Create(m_locations[index], pResult);
}

///////////////////////////////////////////////////////////////////////////////

DxcIndex::DxcIndex(IMalloc *pMalloc)
    : m_dwRef(0), m_pMalloc(pMalloc), m_index(0), m_options(DxcGlobalOpt_None) {
}

DxcIndex::~DxcIndex() {
  if (m_index) {
    clang_disposeIndex(m_index);
    m_index = 0;
  }
}

HRESULT DxcIndex::Initialize(hlsl::DxcLangExtensionsHelper &langHelper) {
  try {
    m_langHelper = langHelper; // Clone the object.
    m_index = clang_createIndex(1, 0);
    if (m_index == 0) {
      return E_FAIL;
    }

    hlsl::DxcLangExtensionsHelperApply *apply = &m_langHelper;
    clang_index_setLangHelper(m_index, apply);
  }
  CATCH_CPP_RETURN_HRESULT();
  return S_OK;
}

HRESULT DxcIndex::Create(hlsl::DxcLangExtensionsHelper &langHelper,
                         DxcIndex **index) {
  if (index == nullptr)
    return E_POINTER;
  *index = nullptr;

  CComPtr<DxcIndex> local;
  local = DxcIndex::Alloc(DxcGetThreadMallocNoRef());
  if (local == nullptr)
    return E_OUTOFMEMORY;
  HRESULT hr = local->Initialize(langHelper);
  if (FAILED(hr))
    return hr;
  *index = local.Detach();
  return S_OK;
}

HRESULT DxcIndex::SetGlobalOptions(DxcGlobalOptions options) {
  m_options = options;
  return S_OK;
}

HRESULT DxcIndex::GetGlobalOptions(DxcGlobalOptions *options) {
  if (options == nullptr)
    return E_POINTER;
  *options = m_options;
  return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT DxcIndex::ParseTranslationUnit(const char *source_filename,
                                       const char *const *command_line_args,
                                       int num_command_line_args,
                                       IDxcUnsavedFile **unsaved_files,
                                       unsigned num_unsaved_files,
                                       DxcTranslationUnitFlags options,
                                       IDxcTranslationUnit **pTranslationUnit) {
  if (pTranslationUnit == nullptr)
    return E_POINTER;
  *pTranslationUnit = nullptr;

  if (m_index == 0)
    return E_FAIL;

  DxcThreadMalloc TM(m_pMalloc);

  CXUnsavedFile *files;
  HRESULT hr = SetupUnsavedFiles(unsaved_files, num_unsaved_files, &files);
  if (FAILED(hr))
    return hr;

  try {
    // TODO: until an interface to file access is defined and implemented,
    // simply fall back to pure Win32/CRT calls.
    ::llvm::sys::fs::MSFileSystem *msfPtr;
    IFT(CreateMSFileSystemForDisk(&msfPtr));
    std::unique_ptr<::llvm::sys::fs::MSFileSystem> msf(msfPtr);

    ::llvm::sys::fs::AutoPerThreadSystem pts(msf.get());
    IFTLLVM(pts.error_code());
    CXTranslationUnit tu = clang_parseTranslationUnit(
        m_index, source_filename, command_line_args, num_command_line_args,
        files, num_unsaved_files, options);
    CleanupUnsavedFiles(files, num_unsaved_files);
    if (tu == nullptr) {
      return E_FAIL;
    }

    CComPtr<DxcTranslationUnit> localTU =
        DxcTranslationUnit::Alloc(DxcGetThreadMallocNoRef());
    if (localTU == nullptr) {
      clang_disposeTranslationUnit(tu);
      return E_OUTOFMEMORY;
    }
    localTU->Initialize(tu);
    *pTranslationUnit = localTU.Detach();

    return S_OK;
  }
  CATCH_CPP_RETURN_HRESULT();
}

///////////////////////////////////////////////////////////////////////////////

HRESULT DxcIntelliSense::CreateIndex(IDxcIndex **index) {
  DxcThreadMalloc TM(m_pMalloc);
  CComPtr<DxcIndex> local;
  HRESULT hr = DxcIndex::Create(m_langHelper, &local);
  *index = local.Detach();
  return hr;
}

HRESULT DxcIntelliSense::GetNullLocation(IDxcSourceLocation **location) {
  DxcThreadMalloc TM(m_pMalloc);
  return DxcSourceLocation::Create(clang_getNullLocation(), location);
}

HRESULT DxcIntelliSense::GetNullRange(IDxcSourceRange **location) {
  DxcThreadMalloc TM(m_pMalloc);
  return DxcSourceRange::Create(clang_getNullRange(), location);
}

HRESULT DxcIntelliSense::GetRange(IDxcSourceLocation *start,
                                  IDxcSourceLocation *end,
                                  IDxcSourceRange **pResult) {
  if (start == nullptr || end == nullptr)
    return E_INVALIDARG;
  if (pResult == nullptr)
    return E_POINTER;
  DxcSourceLocation *startImpl = reinterpret_cast<DxcSourceLocation *>(start);
  DxcSourceLocation *endImpl = reinterpret_cast<DxcSourceLocation *>(end);
  DxcThreadMalloc TM(m_pMalloc);
  return DxcSourceRange::Create(
      clang_getRange(startImpl->GetLocation(), endImpl->GetLocation()),
      pResult);
}

HRESULT DxcIntelliSense::GetDefaultDiagnosticDisplayOptions(
    DxcDiagnosticDisplayOptions *pValue) {
  if (pValue == nullptr)
    return E_POINTER;
  *pValue =
      (DxcDiagnosticDisplayOptions)clang_defaultDiagnosticDisplayOptions();
  return S_OK;
}

HRESULT
DxcIntelliSense::GetDefaultEditingTUOptions(DxcTranslationUnitFlags *pValue) {
  if (pValue == nullptr)
    return E_POINTER;
  *pValue =
      (DxcTranslationUnitFlags)(clang_defaultEditingTranslationUnitOptions() |
                                (unsigned)
                                    DxcTranslationUnitFlags_UseCallerThread);
  return S_OK;
}

HRESULT DxcIntelliSense::CreateUnsavedFile(LPCSTR fileName, LPCSTR contents,
                                           unsigned contentLength,
                                           IDxcUnsavedFile **pResult) {
  DxcThreadMalloc TM(m_pMalloc);
  return DxcBasicUnsavedFile::Create(fileName, contents, contentLength,
                                     pResult);
}

///////////////////////////////////////////////////////////////////////////////

void DxcSourceLocation::Initialize(const CXSourceLocation &location) {
  m_location = location;
}

HRESULT DxcSourceLocation::Create(const CXSourceLocation &location,
                                  IDxcSourceLocation **pObject) {
  if (pObject == nullptr)
    return E_POINTER;
  *pObject = nullptr;
  DxcSourceLocation *local =
      DxcSourceLocation::Alloc(DxcGetThreadMallocNoRef());
  if (local == nullptr)
    return E_OUTOFMEMORY;
  local->Initialize(location);
  local->AddRef();
  *pObject = local;
  return S_OK;
}

HRESULT DxcSourceLocation::IsEqualTo(IDxcSourceLocation *other, BOOL *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  if (other == nullptr) {
    *pResult = FALSE;
  } else {
    DxcSourceLocation *otherImpl = reinterpret_cast<DxcSourceLocation *>(other);
    *pResult = clang_equalLocations(m_location, otherImpl->m_location) != 0;
  }

  return S_OK;
}

HRESULT DxcSourceLocation::GetSpellingLocation(IDxcFile **pFile,
                                               unsigned *pLine, unsigned *pCol,
                                               unsigned *pOffset) {
  CXFile file;
  unsigned line, col, offset;
  DxcThreadMalloc TM(m_pMalloc);
  clang_getSpellingLocation(m_location, &file, &line, &col, &offset);
  if (pFile != nullptr) {
    HRESULT hr = DxcFile::Create(file, pFile);
    if (FAILED(hr))
      return hr;
  }
  if (pLine)
    *pLine = line;
  if (pCol)
    *pCol = col;
  if (pOffset)
    *pOffset = offset;
  return S_OK;
}

HRESULT DxcSourceLocation::IsNull(BOOL *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  CXSourceLocation nullLocation = clang_getNullLocation();
  *pResult = 0 != clang_equalLocations(nullLocation, m_location);
  return S_OK;
}

HRESULT DxcSourceLocation::GetPresumedLocation(LPSTR *pFilename,
                                               unsigned *pLine,
                                               unsigned *pCol) {
  DxcThreadMalloc TM(m_pMalloc);

  CXString filename;
  unsigned line, col;
  clang_getPresumedLocation(m_location, &filename, &line, &col);
  if (pFilename != nullptr) {
    HRESULT hr = CXStringToAnsiAndDispose(filename, pFilename);
    if (FAILED(hr))
      return hr;
  }
  if (pLine)
    *pLine = line;
  if (pCol)
    *pCol = col;
  return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

void DxcSourceRange::Initialize(const CXSourceRange &range) { m_range = range; }

HRESULT DxcSourceRange::Create(const CXSourceRange &range,
                               IDxcSourceRange **pObject) {
  if (pObject == nullptr)
    return E_POINTER;
  *pObject = nullptr;
  DxcSourceRange *local = DxcSourceRange::Alloc(DxcGetThreadMallocNoRef());
  if (local == nullptr)
    return E_OUTOFMEMORY;
  local->Initialize(range);
  local->AddRef();
  *pObject = local;
  return S_OK;
}

HRESULT DxcSourceRange::IsNull(BOOL *pValue) {
  if (pValue == nullptr)
    return E_POINTER;
  *pValue = clang_Range_isNull(m_range) != 0;
  return S_OK;
}

HRESULT DxcSourceRange::GetStart(IDxcSourceLocation **pValue) {
  CXSourceLocation location = clang_getRangeStart(m_range);
  DxcThreadMalloc TM(m_pMalloc);
  return DxcSourceLocation::Create(location, pValue);
}

HRESULT DxcSourceRange::GetEnd(IDxcSourceLocation **pValue) {
  CXSourceLocation location = clang_getRangeEnd(m_range);
  DxcThreadMalloc TM(m_pMalloc);
  return DxcSourceLocation::Create(location, pValue);
}

HRESULT DxcSourceRange::GetOffsets(unsigned *startOffset, unsigned *endOffset) {
  if (startOffset == nullptr)
    return E_POINTER;
  if (endOffset == nullptr)
    return E_POINTER;
  CXSourceLocation startLocation = clang_getRangeStart(m_range);
  CXSourceLocation endLocation = clang_getRangeEnd(m_range);

  CXFile file;
  unsigned line, col;
  clang_getSpellingLocation(startLocation, &file, &line, &col, startOffset);
  clang_getSpellingLocation(endLocation, &file, &line, &col, endOffset);

  return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

void DxcToken::Initialize(const CXTranslationUnit &tu, const CXToken &token) {
  m_tu = tu;
  m_token = token;
}

HRESULT DxcToken::Create(const CXTranslationUnit &tu, const CXToken &token,
                         IDxcToken **pObject) {
  if (pObject == nullptr)
    return E_POINTER;
  *pObject = nullptr;
  DxcToken *local = DxcToken::Alloc(DxcGetThreadMallocNoRef());
  if (local == nullptr)
    return E_OUTOFMEMORY;
  local->Initialize(tu, token);
  local->AddRef();
  *pObject = local;
  return S_OK;
}

HRESULT DxcToken::GetKind(DxcTokenKind *pValue) {
  if (pValue == nullptr)
    return E_POINTER;
  switch (clang_getTokenKind(m_token)) {
  case CXToken_Punctuation:
    *pValue = DxcTokenKind_Punctuation;
    break;
  case CXToken_Keyword:
    *pValue = DxcTokenKind_Keyword;
    break;
  case CXToken_Identifier:
    *pValue = DxcTokenKind_Identifier;
    break;
  case CXToken_Literal:
    *pValue = DxcTokenKind_Literal;
    break;
  case CXToken_Comment:
    *pValue = DxcTokenKind_Comment;
    break;
  case CXToken_BuiltInType:
    *pValue = DxcTokenKind_BuiltInType;
    break;
  default:
    *pValue = DxcTokenKind_Unknown;
    break;
  }
  return S_OK;
}

HRESULT DxcToken::GetLocation(IDxcSourceLocation **pValue) {
  if (pValue == nullptr)
    return E_POINTER;
  DxcThreadMalloc TM(m_pMalloc);
  return DxcSourceLocation::Create(clang_getTokenLocation(m_tu, m_token),
                                   pValue);
}

HRESULT DxcToken::GetExtent(IDxcSourceRange **pValue) {
  if (pValue == nullptr)
    return E_POINTER;
  DxcThreadMalloc TM(m_pMalloc);
  return DxcSourceRange::Create(clang_getTokenExtent(m_tu, m_token), pValue);
}

HRESULT DxcToken::GetSpelling(LPSTR *pValue) {
  if (pValue == nullptr)
    return E_POINTER;
  DxcThreadMalloc TM(m_pMalloc);
  return CXStringToAnsiAndDispose(clang_getTokenSpelling(m_tu, m_token),
                                  pValue);
}

///////////////////////////////////////////////////////////////////////////////

DxcTranslationUnit::DxcTranslationUnit(IMalloc *pMalloc)
    : m_dwRef(0), m_pMalloc(pMalloc), m_tu(nullptr) {}

DxcTranslationUnit::~DxcTranslationUnit() {
  if (m_tu != nullptr) {
    // TODO: until an interface to file access is defined and implemented,
    // simply fall back to pure Win32/CRT calls. Also, note that this can throw
    // / fail in a destructor, which is a big no-no.
    ::llvm::sys::fs::MSFileSystem *msfPtr;
    CreateMSFileSystemForDisk(&msfPtr);
    assert(msfPtr != nullptr);
    std::unique_ptr<::llvm::sys::fs::MSFileSystem> msf(msfPtr);

    ::llvm::sys::fs::AutoPerThreadSystem pts(msf.get());
    assert(!pts.error_code());

    clang_disposeTranslationUnit(m_tu);
    m_tu = nullptr;
  }
}

void DxcTranslationUnit::Initialize(CXTranslationUnit tu) { m_tu = tu; }

HRESULT DxcTranslationUnit::GetCursor(IDxcCursor **pCursor) {
  DxcThreadMalloc TM(m_pMalloc);
  if (m_tu == nullptr)
    return E_FAIL;
  return DxcCursor::Create(clang_getTranslationUnitCursor(m_tu), pCursor);
}

HRESULT DxcTranslationUnit::Tokenize(IDxcSourceRange *range,
                                     IDxcToken ***pTokens,
                                     unsigned *pTokenCount) {
  if (range == nullptr)
    return E_INVALIDARG;
  if (pTokens == nullptr)
    return E_POINTER;
  if (pTokenCount == nullptr)
    return E_POINTER;

  *pTokens = nullptr;
  *pTokenCount = 0;

  // Only accept our own source range.
  DxcThreadMalloc TM(m_pMalloc);
  HRESULT hr = S_OK;
  DxcSourceRange *rangeImpl = reinterpret_cast<DxcSourceRange *>(range);
  IDxcToken **localTokens = nullptr;
  CXToken *tokens = nullptr;
  unsigned numTokens = 0;
  clang_tokenize(m_tu, rangeImpl->GetRange(), &tokens, &numTokens);
  if (numTokens != 0) {
    CoTaskMemAllocZeroElems(numTokens, &localTokens);
    if (localTokens == nullptr) {
      hr = E_OUTOFMEMORY;
    } else {
      for (unsigned i = 0; i < numTokens; ++i) {
        hr = DxcToken::Create(m_tu, tokens[i], &localTokens[i]);
        if (FAILED(hr))
          break;
      }
      *pTokens = localTokens;
      *pTokenCount = numTokens;
    }
  }

  // Cleanup partial progress on failures.
  if (FAILED(hr)) {
    SafeReleaseIfaceArray(localTokens, numTokens);
    delete[] localTokens;
  }

  if (tokens != nullptr) {
    clang_disposeTokens(m_tu, tokens, numTokens);
  }

  return hr;
}

HRESULT DxcTranslationUnit::GetLocation(IDxcFile *file, unsigned line,
                                        unsigned column,
                                        IDxcSourceLocation **pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  *pResult = nullptr;

  if (file == nullptr)
    return E_INVALIDARG;
  DxcThreadMalloc TM(m_pMalloc);
  DxcFile *fileImpl = reinterpret_cast<DxcFile *>(file);
  return DxcSourceLocation::Create(
      clang_getLocation(m_tu, fileImpl->GetFile(), line, column), pResult);
}

HRESULT DxcTranslationUnit::GetNumDiagnostics(unsigned *pValue) {
  if (pValue == nullptr)
    return E_POINTER;
  DxcThreadMalloc TM(m_pMalloc);
  *pValue = clang_getNumDiagnostics(m_tu);
  return S_OK;
}

HRESULT DxcTranslationUnit::GetDiagnostic(unsigned index,
                                          IDxcDiagnostic **pValue) {
  if (pValue == nullptr)
    return E_POINTER;
  DxcThreadMalloc TM(m_pMalloc);
  return DxcDiagnostic::Create(clang_getDiagnostic(m_tu, index), pValue);
}

HRESULT DxcTranslationUnit::GetFile(LPCSTR name, IDxcFile **pResult) {
  if (name == nullptr)
    return E_INVALIDARG;
  if (pResult == nullptr)
    return E_POINTER;
  *pResult = nullptr;

  // TODO: until an interface to file access is defined and implemented, simply
  // fall back to pure Win32/CRT calls.
  DxcThreadMalloc TM(m_pMalloc);
  ::llvm::sys::fs::MSFileSystem *msfPtr;
  IFR(CreateMSFileSystemForDisk(&msfPtr));
  std::unique_ptr<::llvm::sys::fs::MSFileSystem> msf(msfPtr);
  ::llvm::sys::fs::AutoPerThreadSystem pts(msf.get());

  CXFile localFile = clang_getFile(m_tu, name);
  return localFile == nullptr ? DISP_E_BADINDEX
                              : DxcFile::Create(localFile, pResult);
}

HRESULT DxcTranslationUnit::GetFileName(LPSTR *pResult) {
  DxcThreadMalloc TM(m_pMalloc);
  return CXStringToAnsiAndDispose(clang_getTranslationUnitSpelling(m_tu),
                                  pResult);
}

HRESULT DxcTranslationUnit::Reparse(IDxcUnsavedFile **unsaved_files,
                                    unsigned num_unsaved_files) {
  HRESULT hr;
  CXUnsavedFile *local_unsaved_files;
  DxcThreadMalloc TM(m_pMalloc);
  hr =
      SetupUnsavedFiles(unsaved_files, num_unsaved_files, &local_unsaved_files);
  if (FAILED(hr))
    return hr;
  int reparseResult =
      clang_reparseTranslationUnit(m_tu, num_unsaved_files, local_unsaved_files,
                                   clang_defaultReparseOptions(m_tu));
  CleanupUnsavedFiles(local_unsaved_files, num_unsaved_files);
  return reparseResult == 0 ? S_OK : E_FAIL;
}

HRESULT DxcTranslationUnit::GetCursorForLocation(IDxcSourceLocation *location,
                                                 IDxcCursor **pResult) {
  if (location == nullptr)
    return E_INVALIDARG;
  if (pResult == nullptr)
    return E_POINTER;
  DxcSourceLocation *locationImpl =
      reinterpret_cast<DxcSourceLocation *>(location);
  DxcThreadMalloc TM(m_pMalloc);
  return DxcCursor::Create(clang_getCursor(m_tu, locationImpl->GetLocation()),
                           pResult);
}

HRESULT DxcTranslationUnit::GetLocationForOffset(IDxcFile *file,
                                                 unsigned offset,
                                                 IDxcSourceLocation **pResult) {
  if (file == nullptr)
    return E_INVALIDARG;
  if (pResult == nullptr)
    return E_POINTER;
  DxcFile *fileImpl = reinterpret_cast<DxcFile *>(file);
  DxcThreadMalloc TM(m_pMalloc);
  return DxcSourceLocation::Create(
      clang_getLocationForOffset(m_tu, fileImpl->GetFile(), offset), pResult);
}

HRESULT DxcTranslationUnit::GetSkippedRanges(IDxcFile *file,
                                             unsigned *pResultCount,
                                             IDxcSourceRange ***pResult) {
  if (file == nullptr)
    return E_INVALIDARG;
  if (pResultCount == nullptr)
    return E_POINTER;
  if (pResult == nullptr)
    return E_POINTER;

  *pResultCount = 0;
  *pResult = nullptr;

  DxcThreadMalloc TM(m_pMalloc);
  DxcFile *fileImpl = reinterpret_cast<DxcFile *>(file);

  unsigned len = clang_ms_countSkippedRanges(m_tu, fileImpl->GetFile());
  if (len == 0) {
    return S_OK;
  }

  CoTaskMemAllocZeroElems(len, pResult);
  if (*pResult == nullptr) {
    return E_OUTOFMEMORY;
  }

  HRESULT hr = S_OK;
  CXSourceRange *ranges = new CXSourceRange[len];
  clang_ms_getSkippedRanges(m_tu, fileImpl->GetFile(), ranges, len);
  for (unsigned i = 0; i < len; ++i) {
    hr = DxcSourceRange::Create(ranges[i], &(*pResult)[i]);
    if (FAILED(hr))
      break;
  }

  // Cleanup partial progress.
  if (FAILED(hr)) {
    SafeReleaseIfaceArray(*pResult, len);
    CoTaskMemFree(*pResult);
    *pResult = nullptr;
  } else {
    *pResultCount = len;
  }

  delete[] ranges;

  return hr;
}

HRESULT DxcTranslationUnit::GetDiagnosticDetails(
    unsigned index, DxcDiagnosticDisplayOptions options, unsigned *errorCode,
    unsigned *errorLine, unsigned *errorColumn, BSTR *errorFile,
    unsigned *errorOffset, unsigned *errorLength, BSTR *errorMessage) {
  if (errorCode == nullptr || errorLine == nullptr || errorColumn == nullptr ||
      errorFile == nullptr || errorOffset == nullptr ||
      errorLength == nullptr || errorMessage == nullptr) {
    return E_POINTER;
  }

  *errorCode = *errorLine = *errorColumn = *errorOffset = *errorLength = 0;
  *errorFile = *errorMessage = nullptr;

  HRESULT hr = S_OK;
  DxcThreadMalloc TM(m_pMalloc);
  CXDiagnostic diag = clang_getDiagnostic(m_tu, index);
  hr = CXStringToBSTRAndDispose(clang_formatDiagnostic(diag, options),
                                errorMessage);
  if (FAILED(hr)) {
    return hr;
  }

  CXSourceLocation diagLoc = clang_getDiagnosticLocation(diag);
  CXFile diagFile;
  clang_getSpellingLocation(diagLoc, &diagFile, errorLine, errorColumn,
                            errorOffset);
  hr = CXStringToBSTRAndDispose(clang_getFileName(diagFile), errorFile);
  if (FAILED(hr)) {
    SysFreeString(*errorMessage);
    *errorMessage = nullptr;
    return hr;
  }

  return S_OK;
}

struct InclusionData {
  HRESULT result;
  CSimpleArray<CComPtr<IDxcInclusion>> inclusions;
};

static void VisitInclusion(CXFile included_file,
                           CXSourceLocation *inclusion_stack,
                           unsigned include_len, CXClientData client_data) {
  InclusionData *D = (InclusionData *)client_data;
  if (SUCCEEDED(D->result)) {
    CComPtr<IDxcInclusion> pInclusion;
    HRESULT hr = DxcInclusion::Create(included_file, include_len,
                                      inclusion_stack, &pInclusion);
    if (FAILED(hr)) {
      D->result = E_FAIL;
    } else if (!D->inclusions.Add(pInclusion)) {
      D->result = E_OUTOFMEMORY;
    }
  }
}

HRESULT DxcTranslationUnit::GetInclusionList(unsigned *pResultCount,
                                             IDxcInclusion ***pResult) {
  if (pResultCount == nullptr || pResult == nullptr) {
    return E_POINTER;
  }

  *pResultCount = 0;
  *pResult = nullptr;
  DxcThreadMalloc TM(m_pMalloc);
  InclusionData D;
  D.result = S_OK;
  clang_getInclusions(m_tu, VisitInclusion, &D);
  if (FAILED(D.result)) {
    return D.result;
  }
  int inclusionCount = D.inclusions.GetSize();
  if (inclusionCount > 0) {
    CoTaskMemAllocZeroElems<IDxcInclusion *>(inclusionCount, pResult);
    if (*pResult == nullptr) {
      return E_OUTOFMEMORY;
    }
    for (int i = 0; i < inclusionCount; ++i) {
      (*pResult)[i] = D.inclusions[i].Detach();
    }
    *pResultCount = inclusionCount;
  }
  return S_OK;
}

HRESULT DxcTranslationUnit::CodeCompleteAt(const char *fileName, unsigned line,
                                           unsigned column,
                                           IDxcUnsavedFile **pUnsavedFiles,
                                           unsigned numUnsavedFiles,
                                           DxcCodeCompleteFlags options,
                                           IDxcCodeCompleteResults **pResult) {
  if (pResult == nullptr)
    return E_POINTER;

  DxcThreadMalloc TM(m_pMalloc);

  CXUnsavedFile *files;
  HRESULT hr = SetupUnsavedFiles(pUnsavedFiles, numUnsavedFiles, &files);
  if (FAILED(hr))
    return hr;

  CXCodeCompleteResults *results = clang_codeCompleteAt(
      m_tu, fileName, line, column, files, numUnsavedFiles, options);

  CleanupUnsavedFiles(files, numUnsavedFiles);

  if (results == nullptr)
    return E_FAIL;
  *pResult = nullptr;
  DxcCodeCompleteResults *newValue =
      DxcCodeCompleteResults::Alloc(DxcGetThreadMallocNoRef());
  if (newValue == nullptr) {
    clang_disposeCodeCompleteResults(results);
    return E_OUTOFMEMORY;
  }
  newValue->Initialize(results);
  newValue->AddRef();
  *pResult = newValue;
  return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT DxcType::Create(const CXType &type, IDxcType **pObject) {
  if (pObject == nullptr)
    return E_POINTER;
  *pObject = nullptr;
  DxcType *newValue = DxcType::Alloc(DxcGetThreadMallocNoRef());
  if (newValue == nullptr)
    return E_OUTOFMEMORY;
  newValue->Initialize(type);
  newValue->AddRef();
  *pObject = newValue;
  return S_OK;
}

void DxcType::Initialize(const CXType &type) { m_type = type; }

HRESULT DxcType::GetSpelling(LPSTR *pResult) {
  DxcThreadMalloc TM(m_pMalloc);
  return CXStringToAnsiAndDispose(clang_getTypeSpelling(m_type), pResult);
}

HRESULT DxcType::IsEqualTo(IDxcType *other, BOOL *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  if (other == nullptr) {
    *pResult = FALSE;
    return S_OK;
  }
  DxcType *otherImpl = reinterpret_cast<DxcType *>(other);
  *pResult = 0 != clang_equalTypes(m_type, otherImpl->m_type);
  return S_OK;
}

HRESULT DxcType::GetKind(DxcTypeKind *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  *pResult = (DxcTypeKind)m_type.kind;
  return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

DxcCodeCompleteResults::~DxcCodeCompleteResults() {
  clang_disposeCodeCompleteResults(m_ccr);
}

void DxcCodeCompleteResults::Initialize(CXCodeCompleteResults *ccr) {
  m_ccr = ccr;
}

HRESULT DxcCodeCompleteResults::GetNumResults(unsigned *pResult) {
  if (pResult == nullptr)
    return E_POINTER;

  DxcThreadMalloc TM(m_pMalloc);

  *pResult = m_ccr->NumResults;
  return S_OK;
}

HRESULT DxcCodeCompleteResults::GetResultAt(unsigned index,
                                            IDxcCompletionResult **pResult) {
  if (pResult == nullptr)
    return E_POINTER;

  DxcThreadMalloc TM(m_pMalloc);

  CXCompletionResult result = m_ccr->Results[index];

  *pResult = nullptr;
  DxcCompletionResult *newValue =
      DxcCompletionResult::Alloc(DxcGetThreadMallocNoRef());
  if (newValue == nullptr)
    return E_OUTOFMEMORY;
  newValue->Initialize(result);
  newValue->AddRef();
  *pResult = newValue;

  return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

void DxcCompletionResult::Initialize(const CXCompletionResult &cr) {
  m_cr = cr;
}

HRESULT DxcCompletionResult::GetCursorKind(DxcCursorKind *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  *pResult = (DxcCursorKind)m_cr.CursorKind;
  return S_OK;
}

HRESULT
DxcCompletionResult::GetCompletionString(IDxcCompletionString **pResult) {
  if (pResult == nullptr)
    return E_POINTER;

  DxcThreadMalloc TM(m_pMalloc);

  *pResult = nullptr;
  DxcCompletionString *newValue =
      DxcCompletionString::Alloc(DxcGetThreadMallocNoRef());
  if (newValue == nullptr)
    return E_OUTOFMEMORY;
  newValue->Initialize(m_cr.CompletionString);
  newValue->AddRef();
  *pResult = newValue;

  return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

void DxcCompletionString::Initialize(const CXCompletionString &cs) {
  m_cs = cs;
}

HRESULT DxcCompletionString::GetNumCompletionChunks(unsigned *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  *pResult = clang_getNumCompletionChunks(m_cs);
  return S_OK;
}

HRESULT
DxcCompletionString::GetCompletionChunkKind(unsigned chunkNumber,
                                            DxcCompletionChunkKind *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  *pResult =
      (DxcCompletionChunkKind)clang_getCompletionChunkKind(m_cs, chunkNumber);
  return S_OK;
}

HRESULT DxcCompletionString::GetCompletionChunkText(unsigned chunkNumber,
                                                    LPSTR *pResult) {
  if (pResult == nullptr)
    return E_POINTER;
  DxcThreadMalloc TM(m_pMalloc);
  return CXStringToAnsiAndDispose(
      clang_getCompletionChunkText(m_cs, chunkNumber), pResult);
}

///////////////////////////////////////////////////////////////////////////////

C_ASSERT((int)DxcCursor_UnexposedDecl == (int)CXCursor_UnexposedDecl);
C_ASSERT((int)DxcCursor_StructDecl == (int)CXCursor_StructDecl);
C_ASSERT((int)DxcCursor_UnionDecl == (int)CXCursor_UnionDecl);
C_ASSERT((int)DxcCursor_ClassDecl == (int)CXCursor_ClassDecl);
C_ASSERT((int)DxcCursor_EnumDecl == (int)CXCursor_EnumDecl);
C_ASSERT((int)DxcCursor_FieldDecl == (int)CXCursor_FieldDecl);
C_ASSERT((int)DxcCursor_EnumConstantDecl == (int)CXCursor_EnumConstantDecl);
C_ASSERT((int)DxcCursor_FunctionDecl == (int)CXCursor_FunctionDecl);
C_ASSERT((int)DxcCursor_VarDecl == (int)CXCursor_VarDecl);
C_ASSERT((int)DxcCursor_ParmDecl == (int)CXCursor_ParmDecl);
C_ASSERT((int)DxcCursor_ObjCInterfaceDecl == (int)CXCursor_ObjCInterfaceDecl);
C_ASSERT((int)DxcCursor_ObjCCategoryDecl == (int)CXCursor_ObjCCategoryDecl);
C_ASSERT((int)DxcCursor_ObjCProtocolDecl == (int)CXCursor_ObjCProtocolDecl);
C_ASSERT((int)DxcCursor_ObjCPropertyDecl == (int)CXCursor_ObjCPropertyDecl);
C_ASSERT((int)DxcCursor_ObjCIvarDecl == (int)CXCursor_ObjCIvarDecl);
C_ASSERT((int)DxcCursor_ObjCInstanceMethodDecl ==
         (int)CXCursor_ObjCInstanceMethodDecl);
C_ASSERT((int)DxcCursor_ObjCClassMethodDecl ==
         (int)CXCursor_ObjCClassMethodDecl);
C_ASSERT((int)DxcCursor_ObjCImplementationDecl ==
         (int)CXCursor_ObjCImplementationDecl);
C_ASSERT((int)DxcCursor_ObjCCategoryImplDecl ==
         (int)CXCursor_ObjCCategoryImplDecl);
C_ASSERT((int)DxcCursor_TypedefDecl == (int)CXCursor_TypedefDecl);
C_ASSERT((int)DxcCursor_CXXMethod == (int)CXCursor_CXXMethod);
C_ASSERT((int)DxcCursor_Namespace == (int)CXCursor_Namespace);
C_ASSERT((int)DxcCursor_LinkageSpec == (int)CXCursor_LinkageSpec);
C_ASSERT((int)DxcCursor_Constructor == (int)CXCursor_Constructor);
C_ASSERT((int)DxcCursor_Destructor == (int)CXCursor_Destructor);
C_ASSERT((int)DxcCursor_ConversionFunction == (int)CXCursor_ConversionFunction);
C_ASSERT((int)DxcCursor_TemplateTypeParameter ==
         (int)CXCursor_TemplateTypeParameter);
C_ASSERT((int)DxcCursor_NonTypeTemplateParameter ==
         (int)CXCursor_NonTypeTemplateParameter);
C_ASSERT((int)DxcCursor_TemplateTemplateParameter ==
         (int)CXCursor_TemplateTemplateParameter);
C_ASSERT((int)DxcCursor_FunctionTemplate == (int)CXCursor_FunctionTemplate);
C_ASSERT((int)DxcCursor_ClassTemplate == (int)CXCursor_ClassTemplate);
C_ASSERT((int)DxcCursor_ClassTemplatePartialSpecialization ==
         (int)CXCursor_ClassTemplatePartialSpecialization);
C_ASSERT((int)DxcCursor_NamespaceAlias == (int)CXCursor_NamespaceAlias);
C_ASSERT((int)DxcCursor_UsingDirective == (int)CXCursor_UsingDirective);
C_ASSERT((int)DxcCursor_UsingDeclaration == (int)CXCursor_UsingDeclaration);
C_ASSERT((int)DxcCursor_TypeAliasDecl == (int)CXCursor_TypeAliasDecl);
C_ASSERT((int)DxcCursor_ObjCSynthesizeDecl == (int)CXCursor_ObjCSynthesizeDecl);
C_ASSERT((int)DxcCursor_ObjCDynamicDecl == (int)CXCursor_ObjCDynamicDecl);
C_ASSERT((int)DxcCursor_CXXAccessSpecifier == (int)CXCursor_CXXAccessSpecifier);
C_ASSERT((int)DxcCursor_FirstDecl == (int)CXCursor_FirstDecl);
C_ASSERT((int)DxcCursor_LastDecl == (int)CXCursor_LastDecl);
C_ASSERT((int)DxcCursor_FirstRef == (int)CXCursor_FirstRef);
C_ASSERT((int)DxcCursor_ObjCSuperClassRef == (int)CXCursor_ObjCSuperClassRef);
C_ASSERT((int)DxcCursor_ObjCProtocolRef == (int)CXCursor_ObjCProtocolRef);
C_ASSERT((int)DxcCursor_ObjCClassRef == (int)CXCursor_ObjCClassRef);
C_ASSERT((int)DxcCursor_TypeRef == (int)CXCursor_TypeRef);
C_ASSERT((int)DxcCursor_CXXBaseSpecifier == (int)CXCursor_CXXBaseSpecifier);
C_ASSERT((int)DxcCursor_TemplateRef == (int)CXCursor_TemplateRef);
C_ASSERT((int)DxcCursor_NamespaceRef == (int)CXCursor_NamespaceRef);
C_ASSERT((int)DxcCursor_MemberRef == (int)CXCursor_MemberRef);
C_ASSERT((int)DxcCursor_LabelRef == (int)CXCursor_LabelRef);
C_ASSERT((int)DxcCursor_OverloadedDeclRef == (int)CXCursor_OverloadedDeclRef);
C_ASSERT((int)DxcCursor_VariableRef == (int)CXCursor_VariableRef);
C_ASSERT((int)DxcCursor_LastRef == (int)CXCursor_LastRef);
C_ASSERT((int)DxcCursor_FirstInvalid == (int)CXCursor_FirstInvalid);
C_ASSERT((int)DxcCursor_InvalidFile == (int)CXCursor_InvalidFile);
C_ASSERT((int)DxcCursor_NoDeclFound == (int)CXCursor_NoDeclFound);
C_ASSERT((int)DxcCursor_NotImplemented == (int)CXCursor_NotImplemented);
C_ASSERT((int)DxcCursor_InvalidCode == (int)CXCursor_InvalidCode);
C_ASSERT((int)DxcCursor_LastInvalid == (int)CXCursor_LastInvalid);
C_ASSERT((int)DxcCursor_FirstExpr == (int)CXCursor_FirstExpr);
C_ASSERT((int)DxcCursor_UnexposedExpr == (int)CXCursor_UnexposedExpr);
C_ASSERT((int)DxcCursor_DeclRefExpr == (int)CXCursor_DeclRefExpr);
C_ASSERT((int)DxcCursor_MemberRefExpr == (int)CXCursor_MemberRefExpr);
C_ASSERT((int)DxcCursor_CallExpr == (int)CXCursor_CallExpr);
C_ASSERT((int)DxcCursor_ObjCMessageExpr == (int)CXCursor_ObjCMessageExpr);
C_ASSERT((int)DxcCursor_BlockExpr == (int)CXCursor_BlockExpr);
C_ASSERT((int)DxcCursor_IntegerLiteral == (int)CXCursor_IntegerLiteral);
C_ASSERT((int)DxcCursor_FloatingLiteral == (int)CXCursor_FloatingLiteral);
C_ASSERT((int)DxcCursor_ImaginaryLiteral == (int)CXCursor_ImaginaryLiteral);
C_ASSERT((int)DxcCursor_StringLiteral == (int)CXCursor_StringLiteral);
C_ASSERT((int)DxcCursor_CharacterLiteral == (int)CXCursor_CharacterLiteral);
C_ASSERT((int)DxcCursor_ParenExpr == (int)CXCursor_ParenExpr);
C_ASSERT((int)DxcCursor_UnaryOperator == (int)CXCursor_UnaryOperator);
C_ASSERT((int)DxcCursor_ArraySubscriptExpr == (int)CXCursor_ArraySubscriptExpr);
C_ASSERT((int)DxcCursor_BinaryOperator == (int)CXCursor_BinaryOperator);
C_ASSERT((int)DxcCursor_CompoundAssignOperator ==
         (int)CXCursor_CompoundAssignOperator);
C_ASSERT((int)DxcCursor_ConditionalOperator ==
         (int)CXCursor_ConditionalOperator);
C_ASSERT((int)DxcCursor_CStyleCastExpr == (int)CXCursor_CStyleCastExpr);
C_ASSERT((int)DxcCursor_CompoundLiteralExpr ==
         (int)CXCursor_CompoundLiteralExpr);
C_ASSERT((int)DxcCursor_InitListExpr == (int)CXCursor_InitListExpr);
C_ASSERT((int)DxcCursor_AddrLabelExpr == (int)CXCursor_AddrLabelExpr);
C_ASSERT((int)DxcCursor_StmtExpr == (int)CXCursor_StmtExpr);
C_ASSERT((int)DxcCursor_GenericSelectionExpr ==
         (int)CXCursor_GenericSelectionExpr);
C_ASSERT((int)DxcCursor_GNUNullExpr == (int)CXCursor_GNUNullExpr);
C_ASSERT((int)DxcCursor_CXXStaticCastExpr == (int)CXCursor_CXXStaticCastExpr);
C_ASSERT((int)DxcCursor_CXXDynamicCastExpr == (int)CXCursor_CXXDynamicCastExpr);
C_ASSERT((int)DxcCursor_CXXReinterpretCastExpr ==
         (int)CXCursor_CXXReinterpretCastExpr);
C_ASSERT((int)DxcCursor_CXXConstCastExpr == (int)CXCursor_CXXConstCastExpr);
C_ASSERT((int)DxcCursor_CXXFunctionalCastExpr ==
         (int)CXCursor_CXXFunctionalCastExpr);
C_ASSERT((int)DxcCursor_CXXTypeidExpr == (int)CXCursor_CXXTypeidExpr);
C_ASSERT((int)DxcCursor_CXXBoolLiteralExpr == (int)CXCursor_CXXBoolLiteralExpr);
C_ASSERT((int)DxcCursor_CXXNullPtrLiteralExpr ==
         (int)CXCursor_CXXNullPtrLiteralExpr);
C_ASSERT((int)DxcCursor_CXXThisExpr == (int)CXCursor_CXXThisExpr);
C_ASSERT((int)DxcCursor_CXXThrowExpr == (int)CXCursor_CXXThrowExpr);
C_ASSERT((int)DxcCursor_CXXNewExpr == (int)CXCursor_CXXNewExpr);
C_ASSERT((int)DxcCursor_CXXDeleteExpr == (int)CXCursor_CXXDeleteExpr);
C_ASSERT((int)DxcCursor_UnaryExpr == (int)CXCursor_UnaryExpr);
C_ASSERT((int)DxcCursor_ObjCStringLiteral == (int)CXCursor_ObjCStringLiteral);
C_ASSERT((int)DxcCursor_ObjCEncodeExpr == (int)CXCursor_ObjCEncodeExpr);
C_ASSERT((int)DxcCursor_ObjCSelectorExpr == (int)CXCursor_ObjCSelectorExpr);
C_ASSERT((int)DxcCursor_ObjCProtocolExpr == (int)CXCursor_ObjCProtocolExpr);
C_ASSERT((int)DxcCursor_ObjCBridgedCastExpr ==
         (int)CXCursor_ObjCBridgedCastExpr);
C_ASSERT((int)DxcCursor_PackExpansionExpr == (int)CXCursor_PackExpansionExpr);
C_ASSERT((int)DxcCursor_SizeOfPackExpr == (int)CXCursor_SizeOfPackExpr);
C_ASSERT((int)DxcCursor_LambdaExpr == (int)CXCursor_LambdaExpr);
C_ASSERT((int)DxcCursor_ObjCBoolLiteralExpr ==
         (int)CXCursor_ObjCBoolLiteralExpr);
C_ASSERT((int)DxcCursor_ObjCSelfExpr == (int)CXCursor_ObjCSelfExpr);
C_ASSERT((int)DxcCursor_LastExpr == (int)CXCursor_LastExpr);
C_ASSERT((int)DxcCursor_FirstStmt == (int)CXCursor_FirstStmt);
C_ASSERT((int)DxcCursor_UnexposedStmt == (int)CXCursor_UnexposedStmt);
C_ASSERT((int)DxcCursor_LabelStmt == (int)CXCursor_LabelStmt);
C_ASSERT((int)DxcCursor_CompoundStmt == (int)CXCursor_CompoundStmt);
C_ASSERT((int)DxcCursor_CaseStmt == (int)CXCursor_CaseStmt);
C_ASSERT((int)DxcCursor_DefaultStmt == (int)CXCursor_DefaultStmt);
C_ASSERT((int)DxcCursor_IfStmt == (int)CXCursor_IfStmt);
C_ASSERT((int)DxcCursor_SwitchStmt == (int)CXCursor_SwitchStmt);
C_ASSERT((int)DxcCursor_WhileStmt == (int)CXCursor_WhileStmt);
C_ASSERT((int)DxcCursor_DoStmt == (int)CXCursor_DoStmt);
C_ASSERT((int)DxcCursor_ForStmt == (int)CXCursor_ForStmt);
C_ASSERT((int)DxcCursor_GotoStmt == (int)CXCursor_GotoStmt);
C_ASSERT((int)DxcCursor_IndirectGotoStmt == (int)CXCursor_IndirectGotoStmt);
C_ASSERT((int)DxcCursor_ContinueStmt == (int)CXCursor_ContinueStmt);
C_ASSERT((int)DxcCursor_BreakStmt == (int)CXCursor_BreakStmt);
C_ASSERT((int)DxcCursor_ReturnStmt == (int)CXCursor_ReturnStmt);
C_ASSERT((int)DxcCursor_GCCAsmStmt == (int)CXCursor_GCCAsmStmt);
C_ASSERT((int)DxcCursor_AsmStmt == (int)CXCursor_AsmStmt);
C_ASSERT((int)DxcCursor_ObjCAtTryStmt == (int)CXCursor_ObjCAtTryStmt);
C_ASSERT((int)DxcCursor_ObjCAtCatchStmt == (int)CXCursor_ObjCAtCatchStmt);
C_ASSERT((int)DxcCursor_ObjCAtFinallyStmt == (int)CXCursor_ObjCAtFinallyStmt);
C_ASSERT((int)DxcCursor_ObjCAtThrowStmt == (int)CXCursor_ObjCAtThrowStmt);
C_ASSERT((int)DxcCursor_ObjCAtSynchronizedStmt ==
         (int)CXCursor_ObjCAtSynchronizedStmt);
C_ASSERT((int)DxcCursor_ObjCAutoreleasePoolStmt ==
         (int)CXCursor_ObjCAutoreleasePoolStmt);
C_ASSERT((int)DxcCursor_ObjCForCollectionStmt ==
         (int)CXCursor_ObjCForCollectionStmt);
C_ASSERT((int)DxcCursor_CXXCatchStmt == (int)CXCursor_CXXCatchStmt);
C_ASSERT((int)DxcCursor_CXXTryStmt == (int)CXCursor_CXXTryStmt);
C_ASSERT((int)DxcCursor_CXXForRangeStmt == (int)CXCursor_CXXForRangeStmt);
C_ASSERT((int)DxcCursor_SEHTryStmt == (int)CXCursor_SEHTryStmt);
C_ASSERT((int)DxcCursor_SEHExceptStmt == (int)CXCursor_SEHExceptStmt);
C_ASSERT((int)DxcCursor_SEHFinallyStmt == (int)CXCursor_SEHFinallyStmt);
C_ASSERT((int)DxcCursor_MSAsmStmt == (int)CXCursor_MSAsmStmt);
C_ASSERT((int)DxcCursor_NullStmt == (int)CXCursor_NullStmt);
C_ASSERT((int)DxcCursor_DeclStmt == (int)CXCursor_DeclStmt);
C_ASSERT((int)DxcCursor_OMPParallelDirective ==
         (int)CXCursor_OMPParallelDirective);
C_ASSERT((int)DxcCursor_LastStmt == (int)CXCursor_LastStmt);
C_ASSERT((int)DxcCursor_TranslationUnit == (int)CXCursor_TranslationUnit);
C_ASSERT((int)DxcCursor_FirstAttr == (int)CXCursor_FirstAttr);
C_ASSERT((int)DxcCursor_UnexposedAttr == (int)CXCursor_UnexposedAttr);
C_ASSERT((int)DxcCursor_IBActionAttr == (int)CXCursor_IBActionAttr);
C_ASSERT((int)DxcCursor_IBOutletAttr == (int)CXCursor_IBOutletAttr);
C_ASSERT((int)DxcCursor_IBOutletCollectionAttr ==
         (int)CXCursor_IBOutletCollectionAttr);
C_ASSERT((int)DxcCursor_CXXFinalAttr == (int)CXCursor_CXXFinalAttr);
C_ASSERT((int)DxcCursor_CXXOverrideAttr == (int)CXCursor_CXXOverrideAttr);
C_ASSERT((int)DxcCursor_AnnotateAttr == (int)CXCursor_AnnotateAttr);
C_ASSERT((int)DxcCursor_AsmLabelAttr == (int)CXCursor_AsmLabelAttr);
C_ASSERT((int)DxcCursor_PackedAttr == (int)CXCursor_PackedAttr);
C_ASSERT((int)DxcCursor_LastAttr == (int)CXCursor_LastAttr);
C_ASSERT((int)DxcCursor_PreprocessingDirective ==
         (int)CXCursor_PreprocessingDirective);
C_ASSERT((int)DxcCursor_MacroDefinition == (int)CXCursor_MacroDefinition);
C_ASSERT((int)DxcCursor_MacroExpansion == (int)CXCursor_MacroExpansion);
C_ASSERT((int)DxcCursor_MacroInstantiation == (int)CXCursor_MacroInstantiation);
C_ASSERT((int)DxcCursor_InclusionDirective == (int)CXCursor_InclusionDirective);
C_ASSERT((int)DxcCursor_FirstPreprocessing == (int)CXCursor_FirstPreprocessing);
C_ASSERT((int)DxcCursor_LastPreprocessing == (int)CXCursor_LastPreprocessing);
C_ASSERT((int)DxcCursor_ModuleImportDecl == (int)CXCursor_ModuleImportDecl);
C_ASSERT((int)DxcCursor_FirstExtraDecl == (int)CXCursor_FirstExtraDecl);
C_ASSERT((int)DxcCursor_LastExtraDecl == (int)CXCursor_LastExtraDecl);

C_ASSERT((int)DxcTranslationUnitFlags_UseCallerThread ==
         (int)CXTranslationUnit_UseCallerThread);

C_ASSERT((int)DxcCodeCompleteFlags_IncludeMacros ==
         (int)CXCodeComplete_IncludeMacros);
C_ASSERT((int)DxcCodeCompleteFlags_IncludeCodePatterns ==
         (int)CXCodeComplete_IncludeCodePatterns);
C_ASSERT((int)DxcCodeCompleteFlags_IncludeBriefComments ==
         (int)CXCodeComplete_IncludeBriefComments);

C_ASSERT((int)DxcCompletionChunk_Optional == (int)CXCompletionChunk_Optional);
C_ASSERT((int)DxcCompletionChunk_TypedText == (int)CXCompletionChunk_TypedText);
C_ASSERT((int)DxcCompletionChunk_Text == (int)CXCompletionChunk_Text);
C_ASSERT((int)DxcCompletionChunk_Placeholder ==
         (int)CXCompletionChunk_Placeholder);
C_ASSERT((int)DxcCompletionChunk_Informative ==
         (int)CXCompletionChunk_Informative);
C_ASSERT((int)DxcCompletionChunk_CurrentParameter ==
         (int)CXCompletionChunk_CurrentParameter);
C_ASSERT((int)DxcCompletionChunk_LeftParen == (int)CXCompletionChunk_LeftParen);
C_ASSERT((int)DxcCompletionChunk_RightParen ==
         (int)CXCompletionChunk_RightParen);
C_ASSERT((int)DxcCompletionChunk_LeftBracket ==
         (int)CXCompletionChunk_LeftBracket);
C_ASSERT((int)DxcCompletionChunk_RightBracket ==
         (int)CXCompletionChunk_RightBracket);
C_ASSERT((int)DxcCompletionChunk_LeftBrace == (int)CXCompletionChunk_LeftBrace);
C_ASSERT((int)DxcCompletionChunk_RightBrace ==
         (int)CXCompletionChunk_RightBrace);
C_ASSERT((int)DxcCompletionChunk_Comma == (int)CXCompletionChunk_Comma);
C_ASSERT((int)DxcCompletionChunk_ResultType ==
         (int)CXCompletionChunk_ResultType);
C_ASSERT((int)DxcCompletionChunk_Colon == (int)CXCompletionChunk_Colon);
C_ASSERT((int)DxcCompletionChunk_SemiColon == (int)CXCompletionChunk_SemiColon);
C_ASSERT((int)DxcCompletionChunk_Equal == (int)CXCompletionChunk_Equal);
C_ASSERT((int)DxcCompletionChunk_HorizontalSpace ==
         (int)CXCompletionChunk_HorizontalSpace);
C_ASSERT((int)DxcCompletionChunk_VerticalSpace ==
         (int)CXCompletionChunk_VerticalSpace);
