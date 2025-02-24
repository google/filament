
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcapi.h                                                                  //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides declarations for the DirectX Compiler API entry point.           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef __DXC_API__
#define __DXC_API__

#ifdef _WIN32
#ifndef DXC_API_IMPORT
#define DXC_API_IMPORT __declspec(dllimport)
#endif
#else
#ifndef DXC_API_IMPORT
#define DXC_API_IMPORT __attribute__((visibility("default")))
#endif
#endif

#ifdef _WIN32

#ifndef CROSS_PLATFORM_UUIDOF
// Warning: This macro exists in WinAdapter.h as well
#define CROSS_PLATFORM_UUIDOF(interface, spec)                                 \
  struct __declspec(uuid(spec)) interface;
#endif

#else

#include "WinAdapter.h"
#include <dlfcn.h>
#endif

struct IMalloc;

struct IDxcIncludeHandler;

/// \brief Typedef for DxcCreateInstance function pointer.
///
/// This can be used with GetProcAddress to get the DxcCreateInstance function.
typedef HRESULT(__stdcall *DxcCreateInstanceProc)(_In_ REFCLSID rclsid,
                                                  _In_ REFIID riid,
                                                  _Out_ LPVOID *ppv);

/// \brief Typedef for DxcCreateInstance2 function pointer.
///
/// This can be used with GetProcAddress to get the DxcCreateInstance2 function.
typedef HRESULT(__stdcall *DxcCreateInstance2Proc)(_In_ IMalloc *pMalloc,
                                                   _In_ REFCLSID rclsid,
                                                   _In_ REFIID riid,
                                                   _Out_ LPVOID *ppv);

/// \brief Creates a single uninitialized object of the class associated with a
/// specified CLSID.
///
/// \param rclsid The CLSID associated with the data and code that will be used
/// to create the object.
///
/// \param riid A reference to the identifier of the interface to be used to
/// communicate with the object.
///
/// \param ppv Address of pointer variable that receives the interface pointer
/// requested in riid.  Upon successful return, *ppv contains the requested
/// interface pointer. Upon failure, *ppv contains NULL.
///
/// While this function is similar to CoCreateInstance, there is no COM
/// involvement.
extern "C" DXC_API_IMPORT
    HRESULT __stdcall DxcCreateInstance(_In_ REFCLSID rclsid, _In_ REFIID riid,
                                        _Out_ LPVOID *ppv);

/// \brief Version of DxcCreateInstance that takes an IMalloc interface.
///
/// This can be used to create an instance of the compiler with a custom memory
/// allocator.
extern "C" DXC_API_IMPORT
    HRESULT __stdcall DxcCreateInstance2(_In_ IMalloc *pMalloc,
                                         _In_ REFCLSID rclsid, _In_ REFIID riid,
                                         _Out_ LPVOID *ppv);

// For convenience, equivalent definitions to CP_UTF8 and CP_UTF16.
#define DXC_CP_UTF8 65001
#define DXC_CP_UTF16 1200
#define DXC_CP_UTF32 12000
// Use DXC_CP_ACP for: Binary;  ANSI Text;  Autodetect UTF with BOM
#define DXC_CP_ACP 0

/// Codepage for "wide" characters - UTF16 on Windows, UTF32 on other platforms.
#ifdef _WIN32
#define DXC_CP_WIDE DXC_CP_UTF16
#else
#define DXC_CP_WIDE DXC_CP_UTF32
#endif

/// Indicates that the shader hash was computed taking into account source
/// information (-Zss).
#define DXC_HASHFLAG_INCLUDES_SOURCE 1

/// Hash digest type for ShaderHash.
typedef struct DxcShaderHash {
  UINT32 Flags;        ///< DXC_HASHFLAG_*
  BYTE HashDigest[16]; ///< The hash digest
} DxcShaderHash;

#define DXC_FOURCC(ch0, ch1, ch2, ch3)                                         \
  ((UINT32)(UINT8)(ch0) | (UINT32)(UINT8)(ch1) << 8 |                          \
   (UINT32)(UINT8)(ch2) << 16 | (UINT32)(UINT8)(ch3) << 24)
#define DXC_PART_PDB DXC_FOURCC('I', 'L', 'D', 'B')
#define DXC_PART_PDB_NAME DXC_FOURCC('I', 'L', 'D', 'N')
#define DXC_PART_PRIVATE_DATA DXC_FOURCC('P', 'R', 'I', 'V')
#define DXC_PART_ROOT_SIGNATURE DXC_FOURCC('R', 'T', 'S', '0')
#define DXC_PART_DXIL DXC_FOURCC('D', 'X', 'I', 'L')
#define DXC_PART_REFLECTION_DATA DXC_FOURCC('S', 'T', 'A', 'T')
#define DXC_PART_SHADER_HASH DXC_FOURCC('H', 'A', 'S', 'H')
#define DXC_PART_INPUT_SIGNATURE DXC_FOURCC('I', 'S', 'G', '1')
#define DXC_PART_OUTPUT_SIGNATURE DXC_FOURCC('O', 'S', 'G', '1')
#define DXC_PART_PATCH_CONSTANT_SIGNATURE DXC_FOURCC('P', 'S', 'G', '1')

// Some option arguments are defined here for continuity with D3DCompile
// interface.
#define DXC_ARG_DEBUG L"-Zi"
#define DXC_ARG_SKIP_VALIDATION L"-Vd"
#define DXC_ARG_SKIP_OPTIMIZATIONS L"-Od"
#define DXC_ARG_PACK_MATRIX_ROW_MAJOR L"-Zpr"
#define DXC_ARG_PACK_MATRIX_COLUMN_MAJOR L"-Zpc"
#define DXC_ARG_AVOID_FLOW_CONTROL L"-Gfa"
#define DXC_ARG_PREFER_FLOW_CONTROL L"-Gfp"
#define DXC_ARG_ENABLE_STRICTNESS L"-Ges"
#define DXC_ARG_ENABLE_BACKWARDS_COMPATIBILITY L"-Gec"
#define DXC_ARG_IEEE_STRICTNESS L"-Gis"
#define DXC_ARG_OPTIMIZATION_LEVEL0 L"-O0"
#define DXC_ARG_OPTIMIZATION_LEVEL1 L"-O1"
#define DXC_ARG_OPTIMIZATION_LEVEL2 L"-O2"
#define DXC_ARG_OPTIMIZATION_LEVEL3 L"-O3"
#define DXC_ARG_WARNINGS_ARE_ERRORS L"-WX"
#define DXC_ARG_RESOURCES_MAY_ALIAS L"-res_may_alias"
#define DXC_ARG_ALL_RESOURCES_BOUND L"-all_resources_bound"
#define DXC_ARG_DEBUG_NAME_FOR_SOURCE L"-Zss"
#define DXC_ARG_DEBUG_NAME_FOR_BINARY L"-Zsb"

CROSS_PLATFORM_UUIDOF(IDxcBlob, "8BA5FB08-5195-40e2-AC58-0D989C3A0102")
/// \brief A sized buffer that can be passed in and out of DXC APIs.
///
/// This is an alias of ID3D10Blob and ID3DBlob.
struct IDxcBlob : public IUnknown {
public:
  /// \brief Retrieves a pointer to the blob's data.
  virtual LPVOID STDMETHODCALLTYPE GetBufferPointer(void) = 0;

  /// \brief Retrieves the size, in bytes, of the blob's data.
  virtual SIZE_T STDMETHODCALLTYPE GetBufferSize(void) = 0;
};

CROSS_PLATFORM_UUIDOF(IDxcBlobEncoding, "7241d424-2646-4191-97c0-98e96e42fc68")
/// \brief A blob that might have a known encoding.
struct IDxcBlobEncoding : public IDxcBlob {
public:
  /// \brief Retrieve the encoding for this blob.
  ///
  /// \param pKnown Pointer to a variable that will be set to TRUE if the
  /// encoding is known.
  ///
  /// \param pCodePage Pointer to variable that will be set to the encoding used
  /// for this blog.
  ///
  /// If the encoding is not known then pCodePage will be set to CP_ACP.
  virtual HRESULT STDMETHODCALLTYPE GetEncoding(_Out_ BOOL *pKnown,
                                                _Out_ UINT32 *pCodePage) = 0;
};

CROSS_PLATFORM_UUIDOF(IDxcBlobWide, "A3F84EAB-0FAA-497E-A39C-EE6ED60B2D84")
/// \brief A blob containing a null-terminated wide string.
///
/// This uses the native wide character encoding (utf16 on Windows, utf32 on
/// Linux).
///
/// The value returned by GetBufferSize() is the size of the buffer, in bytes,
/// including the null-terminator.
///
/// This interface is used to return output name strings DXC.  Other string
/// output blobs, such as errors/warnings, preprocessed HLSL, or other text are
/// returned using encodings based on the -encoding option passed to the
/// compiler.
struct IDxcBlobWide : public IDxcBlobEncoding {
public:
  /// \brief Retrieves a pointer to the string stored in this blob.
  virtual LPCWSTR STDMETHODCALLTYPE GetStringPointer(void) = 0;

  /// \brief Retrieves the length of the string stored in this blob, in
  /// characters, excluding the null-terminator.
  virtual SIZE_T STDMETHODCALLTYPE GetStringLength(void) = 0;
};

CROSS_PLATFORM_UUIDOF(IDxcBlobUtf8, "3DA636C9-BA71-4024-A301-30CBF125305B")
/// \brief A blob containing a UTF-8 encoded string.
///
/// The value returned by GetBufferSize() is the size of the buffer, in bytes,
/// including the null-terminator.
///
/// Depending on the -encoding option passed to the compiler, this interface is
/// used to return string output blobs, such as errors/warnings, preprocessed
/// HLSL, or other text. Output name strings always use IDxcBlobWide.
struct IDxcBlobUtf8 : public IDxcBlobEncoding {
public:
  /// \brief Retrieves a pointer to the string stored in this blob.
  virtual LPCSTR STDMETHODCALLTYPE GetStringPointer(void) = 0;

  /// \brief Retrieves the length of the string stored in this blob, in
  /// characters, excluding the null-terminator.
  virtual SIZE_T STDMETHODCALLTYPE GetStringLength(void) = 0;
};

#ifdef _WIN32
/// IDxcBlobUtf16 is a legacy alias for IDxcBlobWide on Win32.
typedef IDxcBlobWide IDxcBlobUtf16;
#endif

CROSS_PLATFORM_UUIDOF(IDxcIncludeHandler,
                      "7f61fc7d-950d-467f-b3e3-3c02fb49187c")
/// \brief Interface for handling include directives.
///
/// This interface can be implemented to customize handling of include
/// directives.
///
/// Use IDxcUtils::CreateDefaultIncludeHandler to create a default
/// implementation that reads include files from the filesystem.
///
struct IDxcIncludeHandler : public IUnknown {
  /// \brief Load a source file to be included by the compiler.
  ///
  /// \param pFilename Candidate filename.
  ///
  /// \param ppIncludeSource Resultant source object for included file, nullptr
  /// if not found.
  virtual HRESULT STDMETHODCALLTYPE
  LoadSource(_In_z_ LPCWSTR pFilename,
             _COM_Outptr_result_maybenull_ IDxcBlob **ppIncludeSource) = 0;
};

/// \brief Structure for supplying bytes or text input to Dxc APIs.
typedef struct DxcBuffer {
  /// \brief Pointer to the start of the buffer.
  LPCVOID Ptr;

  /// \brief Size of the buffer in bytes.
  SIZE_T Size;

  /// \brief Encoding of the buffer.
  ///
  /// Use Encoding = 0 for non-text bytes, ANSI text, or unknown with BOM.
  UINT Encoding;
} DxcText;

/// \brief Structure for supplying defines to Dxc APIs.
struct DxcDefine {
  LPCWSTR Name;              ///< The define name.
  _Maybenull_ LPCWSTR Value; ///< Optional value for the define.
};

CROSS_PLATFORM_UUIDOF(IDxcCompilerArgs, "73EFFE2A-70DC-45F8-9690-EFF64C02429D")
/// \brief Interface for managing arguments passed to DXC.
///
/// Use IDxcUtils::BuildArguments to create an instance of this interface.
struct IDxcCompilerArgs : public IUnknown {
  /// \brief Retrieve the array of arguments.
  ///
  /// This can be passed directly to the pArguments parameter of the Compile()
  /// method.
  virtual LPCWSTR *STDMETHODCALLTYPE GetArguments() = 0;

  /// \brief Retrieve the number of arguments.
  ///
  /// This can be passed directly to the argCount parameter of the Compile()
  /// method.
  virtual UINT32 STDMETHODCALLTYPE GetCount() = 0;

  /// \brief Add additional arguments to this list of compiler arguments.
  virtual HRESULT STDMETHODCALLTYPE AddArguments(
      _In_opt_count_(argCount)
          LPCWSTR *pArguments, ///< Array of pointers to arguments to add.
      _In_ UINT32 argCount     ///< Number of arguments to add.
      ) = 0;

  /// \brief Add additional UTF-8 encoded arguments to this list of compiler
  /// arguments.
  virtual HRESULT STDMETHODCALLTYPE AddArgumentsUTF8(
      _In_opt_count_(argCount)
          LPCSTR *pArguments, ///< Array of pointers to UTF-8 arguments to add.
      _In_ UINT32 argCount    ///< Number of arguments to add.
      ) = 0;

  /// \brief Add additional defines to this list of compiler arguments.
  virtual HRESULT STDMETHODCALLTYPE AddDefines(
      _In_count_(defineCount) const DxcDefine *pDefines, ///< Array of defines.
      _In_ UINT32 defineCount                            ///< Number of defines.
      ) = 0;
};

//////////////////////////
// Legacy Interfaces
/////////////////////////

CROSS_PLATFORM_UUIDOF(IDxcLibrary, "e5204dc7-d18c-4c3c-bdfb-851673980fe7")
/// \deprecated IDxcUtils replaces IDxcLibrary; please use IDxcUtils insted.
struct IDxcLibrary : public IUnknown {
  /// \deprecated
  virtual HRESULT STDMETHODCALLTYPE SetMalloc(_In_opt_ IMalloc *pMalloc) = 0;

  /// \deprecated
  virtual HRESULT STDMETHODCALLTYPE
  CreateBlobFromBlob(_In_ IDxcBlob *pBlob, UINT32 offset, UINT32 length,
                     _COM_Outptr_ IDxcBlob **ppResult) = 0;

  /// \deprecated
  virtual HRESULT STDMETHODCALLTYPE
  CreateBlobFromFile(_In_z_ LPCWSTR pFileName, _In_opt_ UINT32 *codePage,
                     _COM_Outptr_ IDxcBlobEncoding **pBlobEncoding) = 0;

  /// \deprecated
  virtual HRESULT STDMETHODCALLTYPE CreateBlobWithEncodingFromPinned(
      _In_bytecount_(size) LPCVOID pText, UINT32 size, UINT32 codePage,
      _COM_Outptr_ IDxcBlobEncoding **pBlobEncoding) = 0;

  /// \deprecated
  virtual HRESULT STDMETHODCALLTYPE CreateBlobWithEncodingOnHeapCopy(
      _In_bytecount_(size) LPCVOID pText, UINT32 size, UINT32 codePage,
      _COM_Outptr_ IDxcBlobEncoding **pBlobEncoding) = 0;

  /// \deprecated
  virtual HRESULT STDMETHODCALLTYPE CreateBlobWithEncodingOnMalloc(
      _In_bytecount_(size) LPCVOID pText, IMalloc *pIMalloc, UINT32 size,
      UINT32 codePage, _COM_Outptr_ IDxcBlobEncoding **pBlobEncoding) = 0;

  /// \deprecated
  virtual HRESULT STDMETHODCALLTYPE
  CreateIncludeHandler(_COM_Outptr_ IDxcIncludeHandler **ppResult) = 0;

  /// \deprecated
  virtual HRESULT STDMETHODCALLTYPE CreateStreamFromBlobReadOnly(
      _In_ IDxcBlob *pBlob, _COM_Outptr_ IStream **ppStream) = 0;

  /// \deprecated
  virtual HRESULT STDMETHODCALLTYPE GetBlobAsUtf8(
      _In_ IDxcBlob *pBlob, _COM_Outptr_ IDxcBlobEncoding **pBlobEncoding) = 0;

  // Renamed from GetBlobAsUtf16 to GetBlobAsWide
  /// \deprecated
  virtual HRESULT STDMETHODCALLTYPE GetBlobAsWide(
      _In_ IDxcBlob *pBlob, _COM_Outptr_ IDxcBlobEncoding **pBlobEncoding) = 0;

#ifdef _WIN32
  // Alias to GetBlobAsWide on Win32
  /// \deprecated
  inline HRESULT GetBlobAsUtf16(_In_ IDxcBlob *pBlob,
                                _COM_Outptr_ IDxcBlobEncoding **pBlobEncoding) {
    return this->GetBlobAsWide(pBlob, pBlobEncoding);
  }
#endif
};

CROSS_PLATFORM_UUIDOF(IDxcOperationResult,
                      "CEDB484A-D4E9-445A-B991-CA21CA157DC2")
/// \brief The results of a DXC operation.
///
/// Note: IDxcResult replaces IDxcOperationResult and should be used wherever
/// possible.
struct IDxcOperationResult : public IUnknown {
  /// \brief Retrieve the overall status of the operation.
  virtual HRESULT STDMETHODCALLTYPE GetStatus(_Out_ HRESULT *pStatus) = 0;

  /// \brief Retrieve the primary output of the operation.
  ///
  /// This corresponds to:
  /// * DXC_OUT_OBJECT - Compile() with shader or library target
  /// * DXC_OUT_DISASSEMBLY - Disassemble()
  /// * DXC_OUT_HLSL - Compile() with -P
  /// * DXC_OUT_ROOT_SIGNATURE - Compile() with rootsig_* target
  virtual HRESULT STDMETHODCALLTYPE
  GetResult(_COM_Outptr_result_maybenull_ IDxcBlob **ppResult) = 0;

  /// \brief Retrieves the error buffer from the operation, if there is one.
  ///
  // This corresponds to calling IDxcResult::GetOutput() with DXC_OUT_ERRORS.
  virtual HRESULT STDMETHODCALLTYPE
  GetErrorBuffer(_COM_Outptr_result_maybenull_ IDxcBlobEncoding **ppErrors) = 0;
};

CROSS_PLATFORM_UUIDOF(IDxcCompiler, "8c210bf3-011f-4422-8d70-6f9acb8db617")
/// \deprecated Please use IDxcCompiler3 instead.
struct IDxcCompiler : public IUnknown {
  /// \brief Compile a single entry point to the target shader model.
  ///
  /// \deprecated Please use IDxcCompiler3::Compile() instead.
  virtual HRESULT STDMETHODCALLTYPE Compile(
      _In_ IDxcBlob *pSource,         // Source text to compile.
      _In_opt_z_ LPCWSTR pSourceName, // Optional file name for pSource. Used in
                                      // errors and include handlers.
      _In_opt_z_ LPCWSTR pEntryPoint, // Entry point name.
      _In_z_ LPCWSTR pTargetProfile,  // Shader profile to compile.
      _In_opt_count_(argCount)
          LPCWSTR *pArguments, // Array of pointers to arguments.
      _In_ UINT32 argCount,    // Number of arguments.
      _In_count_(defineCount) const DxcDefine *pDefines, // Array of defines.
      _In_ UINT32 defineCount,                           // Number of defines.
      _In_opt_ IDxcIncludeHandler
          *pIncludeHandler, // User-provided interface to handle #include
                            // directives (optional).
      _COM_Outptr_ IDxcOperationResult *
          *ppResult // Compiler output status, buffer, and errors.
      ) = 0;

  /// \brief Preprocess source text.
  ///
  /// \deprecated Please use IDxcCompiler3::Compile() with the "-P" argument
  /// instead.
  virtual HRESULT STDMETHODCALLTYPE Preprocess(
      _In_ IDxcBlob *pSource,         // Source text to preprocess.
      _In_opt_z_ LPCWSTR pSourceName, // Optional file name for pSource. Used in
                                      // errors and include handlers.
      _In_opt_count_(argCount)
          LPCWSTR *pArguments, // Array of pointers to arguments.
      _In_ UINT32 argCount,    // Number of arguments.
      _In_count_(defineCount) const DxcDefine *pDefines, // Array of defines.
      _In_ UINT32 defineCount,                           // Number of defines.
      _In_opt_ IDxcIncludeHandler
          *pIncludeHandler, // user-provided interface to handle #include
                            // directives (optional).
      _COM_Outptr_ IDxcOperationResult *
          *ppResult // Preprocessor output status, buffer, and errors.
      ) = 0;

  /// \brief Disassemble a program.
  ///
  /// \deprecated Please use IDxcCompiler3::Disassemble() instead.
  virtual HRESULT STDMETHODCALLTYPE Disassemble(
      _In_ IDxcBlob *pSource,                       // Program to disassemble.
      _COM_Outptr_ IDxcBlobEncoding **ppDisassembly // Disassembly text.
      ) = 0;
};

CROSS_PLATFORM_UUIDOF(IDxcCompiler2, "A005A9D9-B8BB-4594-B5C9-0E633BEC4D37")
/// \deprecated Please use IDxcCompiler3 instead.
struct IDxcCompiler2 : public IDxcCompiler {
  /// \brief Compile a single entry point to the target shader model with debug
  /// information.
  ///
  /// \deprecated Please use IDxcCompiler3::Compile() instead.
  virtual HRESULT STDMETHODCALLTYPE CompileWithDebug(
      _In_ IDxcBlob *pSource,         // Source text to compile.
      _In_opt_z_ LPCWSTR pSourceName, // Optional file name for pSource. Used in
                                      // errors and include handlers.
      _In_opt_z_ LPCWSTR pEntryPoint, // Entry point name.
      _In_z_ LPCWSTR pTargetProfile,  // Shader profile to compile.
      _In_opt_count_(argCount)
          LPCWSTR *pArguments, // Array of pointers to arguments.
      _In_ UINT32 argCount,    // Number of arguments.
      _In_count_(defineCount) const DxcDefine *pDefines, // Array of defines.
      _In_ UINT32 defineCount,                           // Number of defines.
      _In_opt_ IDxcIncludeHandler
          *pIncludeHandler, // user-provided interface to handle #include
                            // directives (optional).
      _COM_Outptr_ IDxcOperationResult *
          *ppResult, // Compiler output status, buffer, and errors.
      _Outptr_opt_result_z_ LPWSTR
          *ppDebugBlobName, // Suggested file name for debug blob. Must be
                            // CoTaskMemFree()'d.
      _COM_Outptr_opt_ IDxcBlob **ppDebugBlob // Debug blob.
      ) = 0;
};

CROSS_PLATFORM_UUIDOF(IDxcLinker, "F1B5BE2A-62DD-4327-A1C2-42AC1E1E78E6")
/// \brief DXC linker interface.
///
/// Use DxcCreateInstance with CLSID_DxcLinker to obtain an instance of this
/// interface.
struct IDxcLinker : public IUnknown {
public:
  /// \brief Register a library with name to reference it later.
  virtual HRESULT
  RegisterLibrary(_In_opt_ LPCWSTR pLibName, ///< Name of the library.
                  _In_ IDxcBlob *pLib        ///< Library blob.
                  ) = 0;

  /// \brief Links the shader and produces a shader blob that the Direct3D
  /// runtime can use.
  virtual HRESULT STDMETHODCALLTYPE Link(
      _In_opt_ LPCWSTR pEntryName, ///< Entry point name.
      _In_ LPCWSTR pTargetProfile, ///< shader profile to link.
      _In_count_(libCount)
          const LPCWSTR *pLibNames, ///< Array of library names to link.
      _In_ UINT32 libCount,         ///< Number of libraries to link.
      _In_opt_count_(argCount)
          const LPCWSTR *pArguments, ///< Array of pointers to arguments.
      _In_ UINT32 argCount,          ///< Number of arguments.
      _COM_Outptr_ IDxcOperationResult *
          *ppResult ///< Linker output status, buffer, and errors.
      ) = 0;
};

/////////////////////////
// Latest interfaces. Please use these.
////////////////////////

CROSS_PLATFORM_UUIDOF(IDxcUtils, "4605C4CB-2019-492A-ADA4-65F20BB7D67F")
/// \brief Various utility functions for DXC.
///
/// Use DxcCreateInstance with CLSID_DxcUtils to obtain an instance of this
/// interface.
///
/// IDxcUtils replaces IDxcLibrary.
struct IDxcUtils : public IUnknown {
  /// \brief Create a sub-blob that holds a reference to the outer blob and
  /// points to its memory.
  ///
  /// \param pBlob The outer blob.
  ///
  /// \param offset The offset inside the outer blob.
  ///
  /// \param length The size, in bytes, of the buffer to reference from the
  /// output blob.
  ///
  /// \param ppResult Address of the pointer that receives a pointer to the
  /// newly created blob.
  virtual HRESULT STDMETHODCALLTYPE
  CreateBlobFromBlob(_In_ IDxcBlob *pBlob, UINT32 offset, UINT32 length,
                     _COM_Outptr_ IDxcBlob **ppResult) = 0;

  // For codePage, use 0 (or DXC_CP_ACP) for raw binary or ANSI code page.

  /// \brief Create a blob referencing existing memory, with no copy.
  ///
  /// \param pData Pointer to buffer containing the contents of the new blob.
  ///
  /// \param size The size of the pData buffer, in bytes.
  ///
  /// \param codePage The code page to use if the blob contains text.  Use
  /// DXC_CP_ACP for binary or ANSI code page.
  ///
  /// \param ppBlobEncoding Address of the pointer that receives a pointer to
  /// the newly created blob.
  ///
  /// The user must manage the memory lifetime separately.
  ///
  /// This replaces IDxcLibrary::CreateBlobWithEncodingFromPinned.
  virtual HRESULT STDMETHODCALLTYPE CreateBlobFromPinned(
      _In_bytecount_(size) LPCVOID pData, UINT32 size, UINT32 codePage,
      _COM_Outptr_ IDxcBlobEncoding **ppBlobEncoding) = 0;

  /// \brief Create a blob, taking ownership of memory allocated with the
  /// supplied allocator.
  ///
  /// \param pData Pointer to buffer containing the contents of the new blob.
  ///
  /// \param pIMalloc The memory allocator to use.
  ///
  /// \param size The size of thee pData buffer, in bytes.
  ///
  /// \param codePage The code page to use if the blob contains text. Use
  /// DXC_CP_ACP for binary or ANSI code page.
  ///
  /// \param ppBlobEncoding Address of the pointer that receives a pointer to
  /// the newly created blob.
  ///
  /// This replaces IDxcLibrary::CreateBlobWithEncodingOnMalloc.
  virtual HRESULT STDMETHODCALLTYPE MoveToBlob(
      _In_bytecount_(size) LPCVOID pData, IMalloc *pIMalloc, UINT32 size,
      UINT32 codePage, _COM_Outptr_ IDxcBlobEncoding **ppBlobEncoding) = 0;

  /// \brief Create a blob containing a copy of the existing data.
  ///
  /// \param pData Pointer to buffer containing the contents of the new blob.
  ///
  /// \param size The size of thee pData buffer, in bytes.
  ///
  /// \param codePage The code page to use if the blob contains text.  Use
  /// DXC_CP_ACP for binary or ANSI code page.
  ///
  /// \param ppBlobEncoding Address of the pointer that receives a pointer to
  /// the newly created blob.
  ///
  /// The new blob and its contents are allocated with the current allocator.
  /// This replaces IDxcLibrary::CreateBlobWithEncodingOnHeapCopy.
  virtual HRESULT STDMETHODCALLTYPE
  CreateBlob(_In_bytecount_(size) LPCVOID pData, UINT32 size, UINT32 codePage,
             _COM_Outptr_ IDxcBlobEncoding **ppBlobEncoding) = 0;

  /// \brief Create a blob with data loaded from a file.
  ///
  /// \param pFileName The name of the file to load from.
  ///
  /// \param pCodePage Optional code page to use if the blob contains text. Pass
  /// NULL for binary data.
  ///
  /// \param ppBlobEncoding Address of the pointer that receives a pointer to
  /// the newly created blob.
  ///
  /// The new blob and its contents are allocated with the current allocator.
  /// This replaces IDxcLibrary::CreateBlobFromFile.
  virtual HRESULT STDMETHODCALLTYPE
  LoadFile(_In_z_ LPCWSTR pFileName, _In_opt_ UINT32 *pCodePage,
           _COM_Outptr_ IDxcBlobEncoding **ppBlobEncoding) = 0;

  /// \brief Create a stream that reads data from a blob.
  ///
  /// \param pBlob The blob to read from.
  ///
  /// \param ppStream Address of the pointer that receives a pointer to the
  /// newly created stream.
  virtual HRESULT STDMETHODCALLTYPE CreateReadOnlyStreamFromBlob(
      _In_ IDxcBlob *pBlob, _COM_Outptr_ IStream **ppStream) = 0;

  /// \brief Create default file-based include handler.
  ///
  /// \param ppResult Address of the pointer that receives a pointer to the
  /// newly created include handler.
  virtual HRESULT STDMETHODCALLTYPE
  CreateDefaultIncludeHandler(_COM_Outptr_ IDxcIncludeHandler **ppResult) = 0;

  /// \brief Convert or return matching encoded text blob as UTF-8.
  ///
  /// \param pBlob The blob to convert.
  ///
  /// \param ppBlobEncoding Address of the pointer that receives a pointer to
  /// the newly created blob.
  virtual HRESULT STDMETHODCALLTYPE GetBlobAsUtf8(
      _In_ IDxcBlob *pBlob, _COM_Outptr_ IDxcBlobUtf8 **ppBlobEncoding) = 0;

  /// \brief Convert or return matching encoded text blob as UTF-16.
  ///
  /// \param pBlob The blob to convert.
  ///
  /// \param ppBlobEncoding Address of the pointer that receives a pointer to
  /// the newly created blob.
  virtual HRESULT STDMETHODCALLTYPE GetBlobAsWide(
      _In_ IDxcBlob *pBlob, _COM_Outptr_ IDxcBlobWide **ppBlobEncoding) = 0;

#ifdef _WIN32
  /// \brief Convert or return matching encoded text blob as UTF-16.
  ///
  /// \param pBlob The blob to convert.
  ///
  /// \param ppBlobEncoding Address of the pointer that receives a pointer to
  /// the newly created blob.
  ///
  /// Alias to GetBlobAsWide on Win32.
  inline HRESULT GetBlobAsUtf16(_In_ IDxcBlob *pBlob,
                                _COM_Outptr_ IDxcBlobWide **ppBlobEncoding) {
    return this->GetBlobAsWide(pBlob, ppBlobEncoding);
  }
#endif

  /// \brief Retrieve a single part from a DXIL container.
  ///
  /// \param pShader The shader to retrieve the part from.
  ///
  /// \param DxcPart The part to retrieve (eg DXC_PART_ROOT_SIGNATURE).
  ///
  /// \param ppPartData Address of the pointer that receives a pointer to the
  /// part.
  ///
  /// \param pPartSizeInBytes Address of the pointer that receives the size of
  /// the part.
  ///
  /// The returned pointer points inside the buffer passed in pShader.
  virtual HRESULT STDMETHODCALLTYPE
  GetDxilContainerPart(_In_ const DxcBuffer *pShader, _In_ UINT32 DxcPart,
                       _Outptr_result_nullonfailure_ void **ppPartData,
                       _Out_ UINT32 *pPartSizeInBytes) = 0;

  /// \brief Create reflection interface from serialized DXIL container or the
  /// DXC_OUT_REFLECTION blob contents.
  ///
  /// \param pData The source data.
  ///
  /// \param iid The interface ID of the reflection interface to create.
  ///
  /// \param ppvReflection Address of the pointer that receives a pointer to the
  /// newly created reflection interface.
  ///
  /// Use this with interfaces such as ID3D12ShaderReflection.
  virtual HRESULT STDMETHODCALLTYPE CreateReflection(
      _In_ const DxcBuffer *pData, REFIID iid, void **ppvReflection) = 0;

  /// \brief Build arguments that can be passed to the Compile method.
  virtual HRESULT STDMETHODCALLTYPE BuildArguments(
      _In_opt_z_ LPCWSTR pSourceName, ///< Optional file name for pSource. Used
                                      ///< in errors and include handlers.
      _In_opt_z_ LPCWSTR pEntryPoint, ///< Entry point name (-E).
      _In_z_ LPCWSTR pTargetProfile,  ///< Shader profile to compile (-T).
      _In_opt_count_(argCount)
          LPCWSTR *pArguments, ///< Array of pointers to arguments.
      _In_ UINT32 argCount,    ///< Number of arguments.
      _In_count_(defineCount) const DxcDefine *pDefines, ///< Array of defines.
      _In_ UINT32 defineCount,                           ///< Number of defines.
      _COM_Outptr_ IDxcCompilerArgs *
          *ppArgs ///< Arguments you can use with Compile() method.
      ) = 0;

  /// \brief Retrieve the hash and contents of a shader PDB.
  ///
  /// \param pPDBBlob The blob containing the PDB.
  ///
  /// \param ppHash Address of the pointer that receives a pointer to the hash
  /// blob.
  ///
  /// \param ppContainer Address of the pointer that receives a pointer to the
  /// bloc containing the contents of the PDB.
  ///
  virtual HRESULT STDMETHODCALLTYPE
  GetPDBContents(_In_ IDxcBlob *pPDBBlob, _COM_Outptr_ IDxcBlob **ppHash,
                 _COM_Outptr_ IDxcBlob **ppContainer) = 0;
};

/// \brief Specifies the kind of output to retrieve from a IDxcResult.
///
/// Note: text outputs returned from version 2 APIs are UTF-8 or UTF-16 based on
/// the -encoding option passed to the compiler.
typedef enum DXC_OUT_KIND {
  DXC_OUT_NONE = 0,        ///< No output.
  DXC_OUT_OBJECT = 1,      ///< IDxcBlob - Shader or library object.
  DXC_OUT_ERRORS = 2,      ///< IDxcBlobUtf8 or IDxcBlobWide.
  DXC_OUT_PDB = 3,         ///< IDxcBlob.
  DXC_OUT_SHADER_HASH = 4, ///< IDxcBlob - DxcShaderHash of shader or shader
                           ///< with source info (-Zsb/-Zss).
  DXC_OUT_DISASSEMBLY = 5, ///< IDxcBlobUtf8 or IDxcBlobWide - from Disassemble.
  DXC_OUT_HLSL =
      6, ///< IDxcBlobUtf8 or IDxcBlobWide - from Preprocessor or Rewriter.
  DXC_OUT_TEXT = 7, ///< IDxcBlobUtf8 or IDxcBlobWide - other text, such as
                    ///< -ast-dump or -Odump.
  DXC_OUT_REFLECTION = 8,     ///< IDxcBlob - RDAT part with reflection data.
  DXC_OUT_ROOT_SIGNATURE = 9, ///< IDxcBlob - Serialized root signature output.
  DXC_OUT_EXTRA_OUTPUTS = 10, ///< IDxcExtraOutputs - Extra outputs.
  DXC_OUT_REMARKS =
      11, ///< IDxcBlobUtf8 or IDxcBlobWide - text directed at stdout.
  DXC_OUT_TIME_REPORT =
      12, ///< IDxcBlobUtf8 or IDxcBlobWide - text directed at stdout.
  DXC_OUT_TIME_TRACE =
      13, ///< IDxcBlobUtf8 or IDxcBlobWide - text directed at stdout.

  DXC_OUT_LAST = DXC_OUT_TIME_TRACE, ///< Last value for a counter.

  DXC_OUT_NUM_ENUMS,
  DXC_OUT_FORCE_DWORD = 0xFFFFFFFF
} DXC_OUT_KIND;

static_assert(DXC_OUT_NUM_ENUMS == DXC_OUT_LAST + 1,
              "DXC_OUT_* Enum added and last value not updated.");

CROSS_PLATFORM_UUIDOF(IDxcResult, "58346CDA-DDE7-4497-9461-6F87AF5E0659")
/// \brief Result of a DXC operation.
///
/// DXC operations may have multiple outputs, such as a shader object and
/// errors. This interface provides access to the outputs.
struct IDxcResult : public IDxcOperationResult {
  /// \brief Determines whether or not this result has the specified output.
  ///
  /// \param dxcOutKind The kind of output to check for.
  virtual BOOL STDMETHODCALLTYPE HasOutput(_In_ DXC_OUT_KIND dxcOutKind) = 0;

  /// \brief Retrieves the specified output.
  ///
  /// \param dxcOutKind The kind of output to retrieve.
  ///
  /// \param iid The interface ID of the output interface.
  ///
  /// \param ppvObject Address of the pointer that receives a pointer to the
  /// output.
  ///
  /// \param ppOutputName Optional address of a pointer to receive the name
  /// blob, if there is one.
  virtual HRESULT STDMETHODCALLTYPE
  GetOutput(_In_ DXC_OUT_KIND dxcOutKind, _In_ REFIID iid,
            _COM_Outptr_opt_result_maybenull_ void **ppvObject,
            _COM_Outptr_opt_result_maybenull_ IDxcBlobWide **ppOutputName) = 0;

  /// \brief Retrieves the number of outputs available in this result.
  virtual UINT32 GetNumOutputs() = 0;

  /// \brief Retrieves the output kind at the specified index.
  virtual DXC_OUT_KIND GetOutputByIndex(UINT32 Index) = 0;

  /// \brief Retrieves the primary output kind for this result.
  ///
  /// See IDxcOperationResult::GetResult() for more information on the primary
  /// output kinds.
  virtual DXC_OUT_KIND PrimaryOutput() = 0;
};

// Special names for extra output that should get written to specific streams.
#define DXC_EXTRA_OUTPUT_NAME_STDOUT L"*stdout*"
#define DXC_EXTRA_OUTPUT_NAME_STDERR L"*stderr*"

CROSS_PLATFORM_UUIDOF(IDxcExtraOutputs, "319b37a2-a5c2-494a-a5de-4801b2faf989")
/// \brief Additional outputs from a DXC operation.
///
/// This can be used to obtain outputs that don't have an explicit DXC_OUT_KIND.
/// Use DXC_OUT_EXTRA_OUTPUTS to obtain instances of this.
struct IDxcExtraOutputs : public IUnknown {
  /// \brief Retrieves the number of outputs available
  virtual UINT32 STDMETHODCALLTYPE GetOutputCount() = 0;

  /// \brief Retrieves the specified output.
  ///
  /// \param uIndex The index of the output to retrieve.
  ///
  /// \param iid The interface ID of the output interface.
  ///
  /// \param ppvObject Optional address of the pointer that receives a pointer
  /// to the output if there is one.
  ///
  /// \param ppOutputType Optional address of the pointer that receives the
  /// output type name blob if there is one.
  ///
  /// \param ppOutputName Optional address of the pointer that receives the
  /// output name blob if there is one.
  virtual HRESULT STDMETHODCALLTYPE
  GetOutput(_In_ UINT32 uIndex, _In_ REFIID iid,
            _COM_Outptr_opt_result_maybenull_ void **ppvObject,
            _COM_Outptr_opt_result_maybenull_ IDxcBlobWide **ppOutputType,
            _COM_Outptr_opt_result_maybenull_ IDxcBlobWide **ppOutputName) = 0;
};

CROSS_PLATFORM_UUIDOF(IDxcCompiler3, "228B4687-5A6A-4730-900C-9702B2203F54")
/// \brief Interface to the DirectX Shader Compiler.
///
/// Use DxcCreateInstance with CLSID_DxcCompiler to obtain an instance of this
/// interface.
struct IDxcCompiler3 : public IUnknown {
  /// \brief Compile a shader.
  ///
  /// IDxcUtils::BuildArguments can be used to assist building the pArguments
  /// and argCount parameters.
  ///
  /// Depending on the arguments, this method can be used to:
  ///
  /// * Compile a single entry point to the target shader model,
  /// * Compile a library to a library target (-T lib_*)
  /// * Compile a root signature (-T rootsig_*),
  /// * Preprocess HLSL source (-P).
  virtual HRESULT STDMETHODCALLTYPE Compile(
      _In_ const DxcBuffer *pSource, ///< Source text to compile.
      _In_opt_count_(argCount)
          LPCWSTR *pArguments, ///< Array of pointers to arguments.
      _In_ UINT32 argCount,    ///< Number of arguments.
      _In_opt_ IDxcIncludeHandler
          *pIncludeHandler,  ///< user-provided interface to handle include
                             ///< directives (optional).
      _In_ REFIID riid,      ///< Interface ID for the result.
      _Out_ LPVOID *ppResult ///< IDxcResult: status, buffer, and errors.
      ) = 0;

  /// \brief Disassemble a program.
  virtual HRESULT STDMETHODCALLTYPE Disassemble(
      _In_ const DxcBuffer
          *pObject,     ///< Program to disassemble: dxil container or bitcode.
      _In_ REFIID riid, ///< Interface ID for the result.
      _Out_ LPVOID
          *ppResult ///< IDxcResult: status, disassembly text, and errors.
      ) = 0;
};

static const UINT32 DxcValidatorFlags_Default = 0;
static const UINT32 DxcValidatorFlags_InPlaceEdit =
    1; // Validator is allowed to update shader blob in-place.
static const UINT32 DxcValidatorFlags_RootSignatureOnly = 2;
static const UINT32 DxcValidatorFlags_ModuleOnly = 4;
static const UINT32 DxcValidatorFlags_ValidMask = 0x7;

CROSS_PLATFORM_UUIDOF(IDxcValidator, "A6E82BD2-1FD7-4826-9811-2857E797F49A")
/// \brief Interface to DXC shader validator.
///
/// Use DxcCreateInstance with CLSID_DxcValidator to obtain an instance of this.
struct IDxcValidator : public IUnknown {
  /// \brief Validate a shader.
  virtual HRESULT STDMETHODCALLTYPE Validate(
      _In_ IDxcBlob *pShader, ///< Shader to validate.
      _In_ UINT32 Flags,      ///< Validation flags.
      _COM_Outptr_ IDxcOperationResult *
          *ppResult ///< Validation output status, buffer, and errors.
      ) = 0;
};

CROSS_PLATFORM_UUIDOF(IDxcValidator2, "458e1fd1-b1b2-4750-a6e1-9c10f03bed92")
/// \brief Interface to DXC shader validator.
///
/// Use DxcCreateInstance with CLSID_DxcValidator to obtain an instance of this.
struct IDxcValidator2 : public IDxcValidator {
  /// \brief Validate a shader with optional debug bitcode.
  virtual HRESULT STDMETHODCALLTYPE ValidateWithDebug(
      _In_ IDxcBlob *pShader,               ///< Shader to validate.
      _In_ UINT32 Flags,                    ///< Validation flags.
      _In_opt_ DxcBuffer *pOptDebugBitcode, ///< Optional debug module bitcode
                                            ///< to provide line numbers.
      _COM_Outptr_ IDxcOperationResult *
          *ppResult ///< Validation output status, buffer, and errors.
      ) = 0;
};

CROSS_PLATFORM_UUIDOF(IDxcContainerBuilder,
                      "334b1f50-2292-4b35-99a1-25588d8c17fe")
/// \brief Interface to DXC container builder.
///
/// Use DxcCreateInstance with CLSID_DxcContainerBuilder to obtain an instance
/// of this.
struct IDxcContainerBuilder : public IUnknown {
  /// \brief Load a DxilContainer to the builder.
  virtual HRESULT STDMETHODCALLTYPE
  Load(_In_ IDxcBlob *pDxilContainerHeader) = 0;

  /// \brief Add a part to the container.
  ///
  /// \param fourCC The part identifier (eg DXC_PART_PDB).
  ///
  /// \param pSource The source blob.
  virtual HRESULT STDMETHODCALLTYPE AddPart(_In_ UINT32 fourCC,
                                            _In_ IDxcBlob *pSource) = 0;

  /// \brief Remove a part from the container.
  ///
  /// \param fourCC The part identifier (eg DXC_PART_PDB).
  ///
  /// \return S_OK on success, DXC_E_MISSING_PART if the part was not found, or
  /// other standard HRESULT error code.
  virtual HRESULT STDMETHODCALLTYPE RemovePart(_In_ UINT32 fourCC) = 0;

  /// \brief Build the container.
  ///
  /// \param ppResult Pointer to variable to receive the result.
  virtual HRESULT STDMETHODCALLTYPE
  SerializeContainer(_Out_ IDxcOperationResult **ppResult) = 0;
};

CROSS_PLATFORM_UUIDOF(IDxcAssembler, "091f7a26-1c1f-4948-904b-e6e3a8a771d5")
/// \brief Interface to DxcAssembler.
///
/// Use DxcCreateInstance with CLSID_DxcAssembler to obtain an instance of this.
struct IDxcAssembler : public IUnknown {
  /// \brief Assemble DXIL in LL or LLVM bitcode to DXIL container.
  virtual HRESULT STDMETHODCALLTYPE AssembleToContainer(
      _In_ IDxcBlob *pShader, ///< Shader to assemble.
      _COM_Outptr_ IDxcOperationResult *
          *ppResult ///< Assembly output status, buffer, and errors.
      ) = 0;
};

CROSS_PLATFORM_UUIDOF(IDxcContainerReflection,
                      "d2c21b26-8350-4bdc-976a-331ce6f4c54c")
/// \brief Interface to DxcContainerReflection.
///
/// Use DxcCreateInstance with CLSID_DxcContainerReflection to obtain an
/// instance of this.
struct IDxcContainerReflection : public IUnknown {
  /// \brief Choose the container to perform reflection on
  ///
  /// \param pContainer The container to load.  If null is passed then this
  /// instance will release any held resources.
  virtual HRESULT STDMETHODCALLTYPE Load(_In_ IDxcBlob *pContainer) = 0;

  /// \brief Retrieves the number of parts in the container.
  ///
  /// \param pResult Pointer to variable to receive the result.
  ///
  /// \return S_OK on success, E_NOT_VALID_STATE if a container has not been
  /// loaded using Load(), or other standard HRESULT error codes.
  virtual HRESULT STDMETHODCALLTYPE GetPartCount(_Out_ UINT32 *pResult) = 0;

  /// \brief Retrieve the kind of a specified part.
  ///
  /// \param idx The index of the part to retrieve the kind of.
  ///
  /// \param pResult Pointer to variable to receive the result.
  ///
  /// \return S_OK on success, E_NOT_VALID_STATE if a container has not been
  /// loaded using Load(), E_BOUND if idx is out of bounds, or other standard
  /// HRESULT error codes.
  virtual HRESULT STDMETHODCALLTYPE GetPartKind(UINT32 idx,
                                                _Out_ UINT32 *pResult) = 0;

  /// \brief Retrieve the content of a specified part.
  ///
  /// \param idx The index of the part to retrieve.
  ///
  /// \param ppResult Pointer to variable to receive the result.
  ///
  /// \return S_OK on success, E_NOT_VALID_STATE if a container has not been
  /// loaded using Load(), E_BOUND if idx is out of bounds, or other standard
  /// HRESULT error codes.
  virtual HRESULT STDMETHODCALLTYPE
  GetPartContent(UINT32 idx, _COM_Outptr_ IDxcBlob **ppResult) = 0;

  /// \brief Retrieve the index of the first part with the specified kind.
  ///
  /// \param kind The kind to search for.
  ///
  /// \param pResult Pointer to variable to receive the index of the matching
  /// part.
  ///
  /// \return S_OK on success, E_NOT_VALID_STATE if a container has not been
  /// loaded using Load(), HRESULT_FROM_WIN32(ERROR_NOT_FOUND) if there is no
  /// part with the specified kind, or other standard HRESULT error codes.
  virtual HRESULT STDMETHODCALLTYPE
  FindFirstPartKind(UINT32 kind, _Out_ UINT32 *pResult) = 0;

  /// \brief Retrieve the reflection interface for a specified part.
  ///
  /// \param idx The index of the part to retrieve the reflection interface of.
  ///
  /// \param iid The IID of the interface to retrieve.
  ///
  /// \param ppvObject Pointer to variable to receive the result.
  ///
  /// Use this with interfaces such as ID3D12ShaderReflection.
  ///
  /// \return S_OK on success, E_NOT_VALID_STATE if a container has not been
  /// loaded using Load(), E_BOUND if idx is out of bounds, or other standard
  /// HRESULT error codes.
  virtual HRESULT STDMETHODCALLTYPE GetPartReflection(UINT32 idx, REFIID iid,
                                                      void **ppvObject) = 0;
};

CROSS_PLATFORM_UUIDOF(IDxcOptimizerPass, "AE2CD79F-CC22-453F-9B6B-B124E7A5204C")
/// \brief An optimizer pass.
///
/// Instances of this can be obtained via IDxcOptimizer::GetAvailablePass.
struct IDxcOptimizerPass : public IUnknown {
  virtual HRESULT STDMETHODCALLTYPE
  GetOptionName(_COM_Outptr_ LPWSTR *ppResult) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetDescription(_COM_Outptr_ LPWSTR *ppResult) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetOptionArgCount(_Out_ UINT32 *pCount) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetOptionArgName(UINT32 argIndex, _COM_Outptr_ LPWSTR *ppResult) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetOptionArgDescription(UINT32 argIndex, _COM_Outptr_ LPWSTR *ppResult) = 0;
};

CROSS_PLATFORM_UUIDOF(IDxcOptimizer, "25740E2E-9CBA-401B-9119-4FB42F39F270")
/// \brief Interface to DxcOptimizer.
///
/// Use DxcCreateInstance with CLSID_DxcOptimizer to obtain an instance of this.
struct IDxcOptimizer : public IUnknown {
  virtual HRESULT STDMETHODCALLTYPE
  GetAvailablePassCount(_Out_ UINT32 *pCount) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetAvailablePass(UINT32 index, _COM_Outptr_ IDxcOptimizerPass **ppResult) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  RunOptimizer(IDxcBlob *pBlob, _In_count_(optionCount) LPCWSTR *ppOptions,
               UINT32 optionCount, _COM_Outptr_ IDxcBlob **pOutputModule,
               _COM_Outptr_opt_ IDxcBlobEncoding **ppOutputText) = 0;
};

static const UINT32 DxcVersionInfoFlags_None = 0;
static const UINT32 DxcVersionInfoFlags_Debug = 1; // Matches VS_FF_DEBUG
static const UINT32 DxcVersionInfoFlags_Internal =
    2; // Internal Validator (non-signing)

CROSS_PLATFORM_UUIDOF(IDxcVersionInfo, "b04f5b50-2059-4f12-a8ff-a1e0cde1cc7e")
/// \brief PDB Version information.
///
/// Use IDxcPdbUtils2::GetVersionInfo to obtain an instance of this.
struct IDxcVersionInfo : public IUnknown {
  virtual HRESULT STDMETHODCALLTYPE GetVersion(_Out_ UINT32 *pMajor,
                                               _Out_ UINT32 *pMinor) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetFlags(_Out_ UINT32 *pFlags) = 0;
};

CROSS_PLATFORM_UUIDOF(IDxcVersionInfo2, "fb6904c4-42f0-4b62-9c46-983af7da7c83")
/// \brief PDB Version Information.
///
/// Use IDxcPdbUtils2::GetVersionInfo to obtain a IDxcVersionInfo interface, and
/// then use QueryInterface to obtain an instance of this interface from it.
struct IDxcVersionInfo2 : public IDxcVersionInfo {
  virtual HRESULT STDMETHODCALLTYPE GetCommitInfo(
      _Out_ UINT32 *pCommitCount,          ///< The total number commits.
      _Outptr_result_z_ char **pCommitHash ///< The SHA of the latest commit.
                                           ///< Must be CoTaskMemFree()'d.
      ) = 0;
};

CROSS_PLATFORM_UUIDOF(IDxcVersionInfo3, "5e13e843-9d25-473c-9ad2-03b2d0b44b1e")
/// \brief PDB Version Information.
///
/// Use IDxcPdbUtils2::GetVersionInfo to obtain a IDxcVersionInfo interface, and
/// then use QueryInterface to obtain an instance of this interface from it.
struct IDxcVersionInfo3 : public IUnknown {
  virtual HRESULT STDMETHODCALLTYPE GetCustomVersionString(
      _Outptr_result_z_ char *
          *pVersionString ///< Custom version string for compiler. Must be
                          ///< CoTaskMemFree()'d.
      ) = 0;
};

struct DxcArgPair {
  const WCHAR *pName;
  const WCHAR *pValue;
};

CROSS_PLATFORM_UUIDOF(IDxcPdbUtils, "E6C9647E-9D6A-4C3B-B94C-524B5A6C343D")
/// \deprecated Please use IDxcPdbUtils2 instead.
struct IDxcPdbUtils : public IUnknown {
  virtual HRESULT STDMETHODCALLTYPE Load(_In_ IDxcBlob *pPdbOrDxil) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetSourceCount(_Out_ UINT32 *pCount) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetSource(_In_ UINT32 uIndex, _COM_Outptr_ IDxcBlobEncoding **ppResult) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetSourceName(_In_ UINT32 uIndex, _Outptr_result_z_ BSTR *pResult) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetFlagCount(_Out_ UINT32 *pCount) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetFlag(_In_ UINT32 uIndex, _Outptr_result_z_ BSTR *pResult) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetArgCount(_Out_ UINT32 *pCount) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetArg(_In_ UINT32 uIndex,
                                           _Outptr_result_z_ BSTR *pResult) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetArgPairCount(_Out_ UINT32 *pCount) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetArgPair(_In_ UINT32 uIndex, _Outptr_result_z_ BSTR *pName,
             _Outptr_result_z_ BSTR *pValue) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetDefineCount(_Out_ UINT32 *pCount) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetDefine(_In_ UINT32 uIndex, _Outptr_result_z_ BSTR *pResult) = 0;

  virtual HRESULT STDMETHODCALLTYPE
  GetTargetProfile(_Outptr_result_z_ BSTR *pResult) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetEntryPoint(_Outptr_result_z_ BSTR *pResult) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetMainFileName(_Outptr_result_z_ BSTR *pResult) = 0;

  virtual HRESULT STDMETHODCALLTYPE
  GetHash(_COM_Outptr_ IDxcBlob **ppResult) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetName(_Outptr_result_z_ BSTR *pResult) = 0;

  virtual BOOL STDMETHODCALLTYPE IsFullPDB() = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetFullPDB(_COM_Outptr_ IDxcBlob **ppFullPDB) = 0;

  virtual HRESULT STDMETHODCALLTYPE
  GetVersionInfo(_COM_Outptr_ IDxcVersionInfo **ppVersionInfo) = 0;

  virtual HRESULT STDMETHODCALLTYPE
  SetCompiler(_In_ IDxcCompiler3 *pCompiler) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  CompileForFullPDB(_COM_Outptr_ IDxcResult **ppResult) = 0;
  virtual HRESULT STDMETHODCALLTYPE OverrideArgs(_In_ DxcArgPair *pArgPairs,
                                                 UINT32 uNumArgPairs) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  OverrideRootSignature(_In_ const WCHAR *pRootSignature) = 0;
};

CROSS_PLATFORM_UUIDOF(IDxcPdbUtils2, "4315D938-F369-4F93-95A2-252017CC3807")
/// \brief DxcPdbUtils interface.
///
/// Use DxcCreateInstance with CLSID_DxcPdbUtils to create an instance of this.
struct IDxcPdbUtils2 : public IUnknown {
  virtual HRESULT STDMETHODCALLTYPE Load(_In_ IDxcBlob *pPdbOrDxil) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetSourceCount(_Out_ UINT32 *pCount) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetSource(_In_ UINT32 uIndex, _COM_Outptr_ IDxcBlobEncoding **ppResult) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetSourceName(_In_ UINT32 uIndex, _COM_Outptr_ IDxcBlobWide **ppResult) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetLibraryPDBCount(UINT32 *pCount) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetLibraryPDB(
      _In_ UINT32 uIndex, _COM_Outptr_ IDxcPdbUtils2 **ppOutPdbUtils,
      _COM_Outptr_opt_result_maybenull_ IDxcBlobWide **ppLibraryName) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetFlagCount(_Out_ UINT32 *pCount) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetFlag(_In_ UINT32 uIndex, _COM_Outptr_ IDxcBlobWide **ppResult) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetArgCount(_Out_ UINT32 *pCount) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetArg(_In_ UINT32 uIndex, _COM_Outptr_ IDxcBlobWide **ppResult) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetArgPairCount(_Out_ UINT32 *pCount) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetArgPair(
      _In_ UINT32 uIndex, _COM_Outptr_result_maybenull_ IDxcBlobWide **ppName,
      _COM_Outptr_result_maybenull_ IDxcBlobWide **ppValue) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetDefineCount(_Out_ UINT32 *pCount) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetDefine(_In_ UINT32 uIndex, _COM_Outptr_ IDxcBlobWide **ppResult) = 0;

  virtual HRESULT STDMETHODCALLTYPE
  GetTargetProfile(_COM_Outptr_result_maybenull_ IDxcBlobWide **ppResult) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetEntryPoint(_COM_Outptr_result_maybenull_ IDxcBlobWide **ppResult) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetMainFileName(_COM_Outptr_result_maybenull_ IDxcBlobWide **ppResult) = 0;

  virtual HRESULT STDMETHODCALLTYPE
  GetHash(_COM_Outptr_result_maybenull_ IDxcBlob **ppResult) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetName(_COM_Outptr_result_maybenull_ IDxcBlobWide **ppResult) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetVersionInfo(
      _COM_Outptr_result_maybenull_ IDxcVersionInfo **ppVersionInfo) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetCustomToolchainID(_Out_ UINT32 *pID) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetCustomToolchainData(_COM_Outptr_result_maybenull_ IDxcBlob **ppBlob) = 0;

  virtual HRESULT STDMETHODCALLTYPE
  GetWholeDxil(_COM_Outptr_result_maybenull_ IDxcBlob **ppResult) = 0;

  virtual BOOL STDMETHODCALLTYPE IsFullPDB() = 0;
  virtual BOOL STDMETHODCALLTYPE IsPDBRef() = 0;
};

// Note: __declspec(selectany) requires 'extern'
// On Linux __declspec(selectany) is removed and using 'extern' results in link
// error.
#ifdef _MSC_VER
#define CLSID_SCOPE __declspec(selectany) extern
#else
#define CLSID_SCOPE
#endif

CLSID_SCOPE const CLSID CLSID_DxcCompiler = {
    0x73e22d93,
    0xe6ce,
    0x47f3,
    {0xb5, 0xbf, 0xf0, 0x66, 0x4f, 0x39, 0xc1, 0xb0}};

// {EF6A8087-B0EA-4D56-9E45-D07E1A8B7806}
CLSID_SCOPE const GUID CLSID_DxcLinker = {
    0xef6a8087,
    0xb0ea,
    0x4d56,
    {0x9e, 0x45, 0xd0, 0x7e, 0x1a, 0x8b, 0x78, 0x6}};

// {CD1F6B73-2AB0-484D-8EDC-EBE7A43CA09F}
CLSID_SCOPE const CLSID CLSID_DxcDiaDataSource = {
    0xcd1f6b73,
    0x2ab0,
    0x484d,
    {0x8e, 0xdc, 0xeb, 0xe7, 0xa4, 0x3c, 0xa0, 0x9f}};

// {3E56AE82-224D-470F-A1A1-FE3016EE9F9D}
CLSID_SCOPE const CLSID CLSID_DxcCompilerArgs = {
    0x3e56ae82,
    0x224d,
    0x470f,
    {0xa1, 0xa1, 0xfe, 0x30, 0x16, 0xee, 0x9f, 0x9d}};

// {6245D6AF-66E0-48FD-80B4-4D271796748C}
CLSID_SCOPE const GUID CLSID_DxcLibrary = {
    0x6245d6af,
    0x66e0,
    0x48fd,
    {0x80, 0xb4, 0x4d, 0x27, 0x17, 0x96, 0x74, 0x8c}};

CLSID_SCOPE const GUID CLSID_DxcUtils = CLSID_DxcLibrary;

// {8CA3E215-F728-4CF3-8CDD-88AF917587A1}
CLSID_SCOPE const GUID CLSID_DxcValidator = {
    0x8ca3e215,
    0xf728,
    0x4cf3,
    {0x8c, 0xdd, 0x88, 0xaf, 0x91, 0x75, 0x87, 0xa1}};

// {D728DB68-F903-4F80-94CD-DCCF76EC7151}
CLSID_SCOPE const GUID CLSID_DxcAssembler = {
    0xd728db68,
    0xf903,
    0x4f80,
    {0x94, 0xcd, 0xdc, 0xcf, 0x76, 0xec, 0x71, 0x51}};

// {b9f54489-55b8-400c-ba3a-1675e4728b91}
CLSID_SCOPE const GUID CLSID_DxcContainerReflection = {
    0xb9f54489,
    0x55b8,
    0x400c,
    {0xba, 0x3a, 0x16, 0x75, 0xe4, 0x72, 0x8b, 0x91}};

// {AE2CD79F-CC22-453F-9B6B-B124E7A5204C}
CLSID_SCOPE const GUID CLSID_DxcOptimizer = {
    0xae2cd79f,
    0xcc22,
    0x453f,
    {0x9b, 0x6b, 0xb1, 0x24, 0xe7, 0xa5, 0x20, 0x4c}};

// {94134294-411f-4574-b4d0-8741e25240d2}
CLSID_SCOPE const GUID CLSID_DxcContainerBuilder = {
    0x94134294,
    0x411f,
    0x4574,
    {0xb4, 0xd0, 0x87, 0x41, 0xe2, 0x52, 0x40, 0xd2}};

// {54621dfb-f2ce-457e-ae8c-ec355faeec7c}
CLSID_SCOPE const GUID CLSID_DxcPdbUtils = {
    0x54621dfb,
    0xf2ce,
    0x457e,
    {0xae, 0x8c, 0xec, 0x35, 0x5f, 0xae, 0xec, 0x7c}};

#endif
