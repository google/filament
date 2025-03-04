///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcisenseimpl.h                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements the DirectX Compiler IntelliSense component.                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef __DXC_ISENSEIMPL__
#define __DXC_ISENSEIMPL__

#include "dxc/Support/DxcLangExtensionsCommonHelper.h"
#include "dxc/Support/DxcLangExtensionsHelper.h"
#include "dxc/Support/microcom.h"
#include "dxc/dxcapi.internal.h"
#include "dxc/dxcisense.h"
#include "clang-c/Index.h"
#include "clang/AST/Decl.h"
#include "clang/Frontend/CompilerInstance.h"

// Forward declarations.
class DxcCursor;
class DxcDiagnostic;
class DxcFile;
class DxcIndex;
class DxcIntelliSense;
class DxcSourceLocation;
class DxcSourceRange;
class DxcTranslationUnit;
class DxcToken;
struct IMalloc;

class DxcCursor : public IDxcCursor {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CXCursor m_cursor;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(DxcCursor)
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcCursor>(this, iid, ppvObject);
  }

  void Initialize(const CXCursor &cursor);
  static HRESULT Create(const CXCursor &cursor, IDxcCursor **pObject);

  HRESULT STDMETHODCALLTYPE GetExtent(IDxcSourceRange **pRange) override;
  HRESULT STDMETHODCALLTYPE GetLocation(IDxcSourceLocation **pResult) override;
  HRESULT STDMETHODCALLTYPE GetKind(DxcCursorKind *pResult) override;
  HRESULT STDMETHODCALLTYPE GetKindFlags(DxcCursorKindFlags *pResult) override;
  HRESULT STDMETHODCALLTYPE GetSemanticParent(IDxcCursor **pResult) override;
  HRESULT STDMETHODCALLTYPE GetLexicalParent(IDxcCursor **pResult) override;
  HRESULT STDMETHODCALLTYPE GetCursorType(IDxcType **pResult) override;
  HRESULT STDMETHODCALLTYPE GetNumArguments(int *pResult) override;
  HRESULT STDMETHODCALLTYPE GetArgumentAt(int index,
                                          IDxcCursor **pResult) override;
  HRESULT STDMETHODCALLTYPE GetReferencedCursor(IDxcCursor **pResult) override;
  HRESULT STDMETHODCALLTYPE GetDefinitionCursor(IDxcCursor **pResult) override;
  HRESULT STDMETHODCALLTYPE
  FindReferencesInFile(IDxcFile *file, unsigned skip, unsigned top,
                       unsigned *pResultLength, IDxcCursor ***pResult) override;
  HRESULT STDMETHODCALLTYPE GetSpelling(LPSTR *pResult) override;
  HRESULT STDMETHODCALLTYPE IsEqualTo(IDxcCursor *other,
                                      BOOL *pResult) override;
  HRESULT STDMETHODCALLTYPE IsNull(BOOL *pResult) override;
  HRESULT STDMETHODCALLTYPE IsDefinition(BOOL *pResult) override;
  /// <summary>Gets the display name for the cursor, including e.g. parameter
  /// types for a function.</summary>
  HRESULT STDMETHODCALLTYPE GetDisplayName(BSTR *pResult) override;
  /// <summary>Gets the qualified name for the symbol the cursor refers
  /// to.</summary>
  HRESULT STDMETHODCALLTYPE GetQualifiedName(BOOL includeTemplateArgs,
                                             BSTR *pResult) override;
  /// <summary>Gets a name for the cursor, applying the specified formatting
  /// flags.</summary>
  HRESULT STDMETHODCALLTYPE GetFormattedName(DxcCursorFormatting formatting,
                                             BSTR *pResult) override;
  /// <summary>Gets children in pResult up to top elements.</summary>
  HRESULT STDMETHODCALLTYPE GetChildren(unsigned skip, unsigned top,
                                        unsigned *pResultLength,
                                        IDxcCursor ***pResult) override;
  /// <summary>Gets the cursor following a location within a compound
  /// cursor.</summary>
  HRESULT STDMETHODCALLTYPE GetSnappedChild(IDxcSourceLocation *location,
                                            IDxcCursor **pResult) override;
};

class DxcDiagnostic : public IDxcDiagnostic {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CXDiagnostic m_diagnostic;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcDiagnostic)
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcDiagnostic>(this, iid, ppvObject);
  }

  DxcDiagnostic(IMalloc *pMalloc);
  ~DxcDiagnostic();
  void Initialize(const CXDiagnostic &diagnostic);
  static HRESULT Create(const CXDiagnostic &diagnostic,
                        IDxcDiagnostic **pObject);

  HRESULT STDMETHODCALLTYPE FormatDiagnostic(
      DxcDiagnosticDisplayOptions options, LPSTR *pResult) override;
  HRESULT STDMETHODCALLTYPE
  GetSeverity(DxcDiagnosticSeverity *pResult) override;
  HRESULT STDMETHODCALLTYPE GetLocation(IDxcSourceLocation **pResult) override;
  HRESULT STDMETHODCALLTYPE GetSpelling(LPSTR *pResult) override;
  HRESULT STDMETHODCALLTYPE GetCategoryText(LPSTR *pResult) override;
  HRESULT STDMETHODCALLTYPE GetNumRanges(unsigned *pResult) override;
  HRESULT STDMETHODCALLTYPE GetRangeAt(unsigned index,
                                       IDxcSourceRange **pResult) override;
  HRESULT STDMETHODCALLTYPE GetNumFixIts(unsigned *pResult) override;
  HRESULT STDMETHODCALLTYPE GetFixItAt(unsigned index,
                                       IDxcSourceRange **pReplacementRange,
                                       LPSTR *pText) override;
};

class DxcFile : public IDxcFile {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CXFile m_file;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(DxcFile)
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcFile>(this, iid, ppvObject);
  }

  void Initialize(const CXFile &file);
  static HRESULT Create(const CXFile &file, IDxcFile **pObject);

  const CXFile &GetFile() const { return m_file; }
  HRESULT STDMETHODCALLTYPE GetName(LPSTR *pResult) override;
  HRESULT STDMETHODCALLTYPE IsEqualTo(IDxcFile *other, BOOL *pResult) override;
};

class DxcInclusion : public IDxcInclusion {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CXFile m_file;
  CXSourceLocation *m_locations;
  unsigned m_locationLength;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcInclusion)
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcInclusion>(this, iid, ppvObject);
  }

  DxcInclusion(IMalloc *pMalloc);
  ~DxcInclusion();
  HRESULT Initialize(CXFile file, unsigned locations,
                     CXSourceLocation *pLocation);
  static HRESULT Create(CXFile file, unsigned locations,
                        CXSourceLocation *pLocation, IDxcInclusion **pResult);

  HRESULT STDMETHODCALLTYPE GetIncludedFile(IDxcFile **pResult) override;
  HRESULT STDMETHODCALLTYPE GetStackLength(unsigned *pResult) override;
  HRESULT STDMETHODCALLTYPE GetStackItem(unsigned index,
                                         IDxcSourceLocation **pResult) override;
};

class DxcIndex : public IDxcIndex {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CXIndex m_index;
  DxcGlobalOptions m_options;
  hlsl::DxcLangExtensionsHelper m_langHelper;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcIndex)
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcIndex>(this, iid, ppvObject);
  }

  DxcIndex(IMalloc *pMalloc);
  ~DxcIndex();
  HRESULT Initialize(hlsl::DxcLangExtensionsHelper &langHelper);
  static HRESULT Create(hlsl::DxcLangExtensionsHelper &langHelper,
                        DxcIndex **index);

  HRESULT STDMETHODCALLTYPE SetGlobalOptions(DxcGlobalOptions options) override;
  HRESULT STDMETHODCALLTYPE
  GetGlobalOptions(DxcGlobalOptions *options) override;
  HRESULT STDMETHODCALLTYPE ParseTranslationUnit(
      const char *source_filename, const char *const *command_line_args,
      int num_command_line_args, IDxcUnsavedFile **unsaved_files,
      unsigned num_unsaved_files, DxcTranslationUnitFlags options,
      IDxcTranslationUnit **pTranslationUnit) override;
};

class DxcIntelliSense : public IDxcIntelliSense, public IDxcLangExtensions3 {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  hlsl::DxcLangExtensionsHelper m_langHelper;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL();
  DXC_MICROCOM_TM_CTOR(DxcIntelliSense)
  DXC_LANGEXTENSIONS_HELPER_IMPL(m_langHelper);

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcIntelliSense, IDxcLangExtensions,
                                 IDxcLangExtensions2, IDxcLangExtensions3>(
        this, iid, ppvObject);
  }

  HRESULT STDMETHODCALLTYPE CreateIndex(IDxcIndex **index) override;
  HRESULT STDMETHODCALLTYPE
  GetNullLocation(IDxcSourceLocation **location) override;
  HRESULT STDMETHODCALLTYPE GetNullRange(IDxcSourceRange **location) override;
  HRESULT STDMETHODCALLTYPE GetRange(IDxcSourceLocation *start,
                                     IDxcSourceLocation *end,
                                     IDxcSourceRange **location) override;
  HRESULT STDMETHODCALLTYPE GetDefaultDiagnosticDisplayOptions(
      DxcDiagnosticDisplayOptions *pValue) override;
  HRESULT STDMETHODCALLTYPE
  GetDefaultEditingTUOptions(DxcTranslationUnitFlags *pValue) override;
  HRESULT STDMETHODCALLTYPE
  CreateUnsavedFile(LPCSTR fileName, LPCSTR contents, unsigned contentLength,
                    IDxcUnsavedFile **pResult) override;
};

class DxcSourceLocation : public IDxcSourceLocation {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CXSourceLocation m_location;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(DxcSourceLocation)
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcSourceLocation>(this, iid, ppvObject);
  }

  void Initialize(const CXSourceLocation &location);
  static HRESULT Create(const CXSourceLocation &location,
                        IDxcSourceLocation **pObject);

  const CXSourceLocation &GetLocation() const { return m_location; }
  HRESULT STDMETHODCALLTYPE IsEqualTo(IDxcSourceLocation *other,
                                      BOOL *pValue) override;
  HRESULT STDMETHODCALLTYPE GetSpellingLocation(IDxcFile **pFile,
                                                unsigned *pLine, unsigned *pCol,
                                                unsigned *pOffset) override;
  HRESULT STDMETHODCALLTYPE IsNull(BOOL *pResult) override;
  HRESULT STDMETHODCALLTYPE GetPresumedLocation(LPSTR *pFilename,
                                                unsigned *pLine,
                                                unsigned *pCol) override;
};

class DxcSourceRange : public IDxcSourceRange {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CXSourceRange m_range;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(DxcSourceRange)
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcSourceRange>(this, iid, ppvObject);
  }

  void Initialize(const CXSourceRange &range);
  static HRESULT Create(const CXSourceRange &range, IDxcSourceRange **pObject);

  const CXSourceRange &GetRange() const { return m_range; }
  HRESULT STDMETHODCALLTYPE IsNull(BOOL *pValue) override;
  HRESULT STDMETHODCALLTYPE GetStart(IDxcSourceLocation **pValue) override;
  HRESULT STDMETHODCALLTYPE GetEnd(IDxcSourceLocation **pValue) override;
  HRESULT STDMETHODCALLTYPE GetOffsets(unsigned *startOffset,
                                       unsigned *endOffset) override;
};

class DxcToken : public IDxcToken {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CXToken m_token;
  CXTranslationUnit m_tu;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(DxcToken)
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcToken>(this, iid, ppvObject);
  }

  void Initialize(const CXTranslationUnit &tu, const CXToken &token);
  static HRESULT Create(const CXTranslationUnit &tu, const CXToken &token,
                        IDxcToken **pObject);

  HRESULT STDMETHODCALLTYPE GetKind(DxcTokenKind *pValue) override;
  HRESULT STDMETHODCALLTYPE GetLocation(IDxcSourceLocation **pValue) override;
  HRESULT STDMETHODCALLTYPE GetExtent(IDxcSourceRange **pValue) override;
  HRESULT STDMETHODCALLTYPE GetSpelling(LPSTR *pValue) override;
};

class DxcTranslationUnit : public IDxcTranslationUnit {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CXTranslationUnit m_tu;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcTranslationUnit)
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcTranslationUnit>(this, iid, ppvObject);
  }

  DxcTranslationUnit(IMalloc *pMalloc);
  ~DxcTranslationUnit();
  void Initialize(CXTranslationUnit tu);

  HRESULT STDMETHODCALLTYPE GetCursor(IDxcCursor **pCursor) override;
  HRESULT STDMETHODCALLTYPE Tokenize(IDxcSourceRange *range,
                                     IDxcToken ***pTokens,
                                     unsigned *pTokenCount) override;
  HRESULT STDMETHODCALLTYPE GetLocation(IDxcFile *file, unsigned line,
                                        unsigned column,
                                        IDxcSourceLocation **pResult) override;
  HRESULT STDMETHODCALLTYPE GetNumDiagnostics(unsigned *pValue) override;
  HRESULT STDMETHODCALLTYPE GetDiagnostic(unsigned index,
                                          IDxcDiagnostic **pValue) override;
  HRESULT STDMETHODCALLTYPE GetFile(const char *name,
                                    IDxcFile **pResult) override;
  HRESULT STDMETHODCALLTYPE GetFileName(LPSTR *pResult) override;
  HRESULT STDMETHODCALLTYPE Reparse(IDxcUnsavedFile **unsaved_files,
                                    unsigned num_unsaved_files) override;
  HRESULT STDMETHODCALLTYPE GetCursorForLocation(IDxcSourceLocation *location,
                                                 IDxcCursor **pResult) override;
  HRESULT STDMETHODCALLTYPE GetLocationForOffset(
      IDxcFile *file, unsigned offset, IDxcSourceLocation **pResult) override;
  HRESULT STDMETHODCALLTYPE
  GetSkippedRanges(IDxcFile *file, unsigned *pResultCount,
                   IDxcSourceRange ***pResult) override;
  HRESULT STDMETHODCALLTYPE GetDiagnosticDetails(
      unsigned index, DxcDiagnosticDisplayOptions options, unsigned *errorCode,
      unsigned *errorLine, unsigned *errorColumn, BSTR *errorFile,
      unsigned *errorOffset, unsigned *errorLength,
      BSTR *errorMessage) override;
  HRESULT STDMETHODCALLTYPE GetInclusionList(unsigned *pResultCount,
                                             IDxcInclusion ***pResult) override;
  HRESULT STDMETHODCALLTYPE CodeCompleteAt(
      const char *fileName, unsigned line, unsigned column,
      IDxcUnsavedFile **pUnsavedFiles, unsigned numUnsavedFiles,
      DxcCodeCompleteFlags options, IDxcCodeCompleteResults **pResult) override;
};

class DxcType : public IDxcType {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CXType m_type;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(DxcType)
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcType>(this, iid, ppvObject);
  }

  void Initialize(const CXType &type);
  static HRESULT Create(const CXType &type, IDxcType **pObject);

  HRESULT STDMETHODCALLTYPE GetSpelling(LPSTR *pResult) override;
  HRESULT STDMETHODCALLTYPE IsEqualTo(IDxcType *other, BOOL *pResult) override;
  HRESULT STDMETHODCALLTYPE GetKind(DxcTypeKind *pResult) override;
};

class DxcCodeCompleteResults : public IDxcCodeCompleteResults {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CXCodeCompleteResults *m_ccr;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(DxcCodeCompleteResults)
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcCodeCompleteResults>(this, iid, ppvObject);
  }

  ~DxcCodeCompleteResults();
  void Initialize(CXCodeCompleteResults *ccr);

  HRESULT STDMETHODCALLTYPE GetNumResults(unsigned *pResult) override;
  HRESULT STDMETHODCALLTYPE
  GetResultAt(unsigned index, IDxcCompletionResult **pResult) override;
};

class DxcCompletionResult : public IDxcCompletionResult {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CXCompletionResult m_cr;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(DxcCompletionResult)
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcCompletionResult>(this, iid, ppvObject);
  }

  void Initialize(const CXCompletionResult &cr);

  HRESULT STDMETHODCALLTYPE GetCursorKind(DxcCursorKind *pResult) override;
  HRESULT STDMETHODCALLTYPE
  GetCompletionString(IDxcCompletionString **pResult) override;
};

class DxcCompletionString : public IDxcCompletionString {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CXCompletionString m_cs;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(DxcCompletionString)
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcCompletionString>(this, iid, ppvObject);
  }

  void Initialize(const CXCompletionString &cs);

  HRESULT STDMETHODCALLTYPE GetNumCompletionChunks(unsigned *pResult) override;
  HRESULT STDMETHODCALLTYPE GetCompletionChunkKind(
      unsigned chunkNumber, DxcCompletionChunkKind *pResult) override;
  HRESULT STDMETHODCALLTYPE GetCompletionChunkText(unsigned chunkNumber,
                                                   LPSTR *pResult) override;
};

HRESULT CreateDxcIntelliSense(REFIID riid, LPVOID *ppv) throw();

#endif
