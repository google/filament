// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

// These #defines prevent the idl-generated headers from trying to include
// Windows.h from the SDK rather than this one.
#define RPC_NO_WINDOWS_H
#define COM_NO_WINDOWS_H

// Allcaps type definitions
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

// Note: using fixed-width here to match Windows widths
// Specifically this is different for 'long' vs 'LONG'
typedef uint8_t UINT8;
typedef int8_t INT8;
typedef uint16_t UINT16;
typedef int16_t INT16;
typedef uint32_t UINT32, UINT, ULONG, DWORD, BOOL;
typedef int32_t INT32, INT, LONG;
typedef uint64_t UINT64, ULONG_PTR;
typedef int64_t INT64, LONG_PTR;
typedef void VOID, *HANDLE, *RPC_IF_HANDLE, *LPVOID;
typedef const void *LPCVOID;
typedef size_t SIZE_T;
typedef float FLOAT;
typedef double DOUBLE;
typedef unsigned char BYTE;
typedef int HWND;
typedef int PALETTEENTRY;
typedef int HDC;
typedef uint16_t WORD;
typedef void* PVOID;
typedef char BOOLEAN;
typedef uint64_t ULONGLONG;
typedef uint16_t USHORT, *PUSHORT;
typedef int64_t LONGLONG, *PLONGLONG;
typedef int64_t LONG_PTR, *PLONG_PTR;
typedef int64_t LONG64, *PLONG64;
typedef uint64_t ULONG64, *PULONG64;
typedef wchar_t WCHAR, *PWSTR;
typedef uint8_t UCHAR, *PUCHAR;
typedef uint64_t ULONG_PTR, *PULONG_PTR;
typedef uint64_t UINT_PTR, *PUINT_PTR;
typedef int64_t INT_PTR, *PINT_PTR;

// Note: WCHAR is not the same between Windows and Linux, to enable
// string manipulation APIs to work with resulting strings.
// APIs to D3D/DXCore will work on Linux wchars, but beware with
// interactions directly with the Windows kernel.
typedef char CHAR, *PSTR, *LPSTR, TCHAR, *PTSTR;
typedef const char *LPCSTR, *PCSTR, *LPCTSTR, *PCTSTR;
typedef wchar_t WCHAR, *PWSTR, *LPWSTR, *PWCHAR;
typedef const wchar_t *LPCWSTR, *PCWSTR;

#undef LONG_MAX
#define LONG_MAX INT_MAX
#undef ULONG_MAX
#define ULONG_MAX UINT_MAX

// Misc defines
#define interface struct
#define MIDL_INTERFACE(x) interface
#define __analysis_assume(x)
#define TRUE 1u
#define FALSE 0u
#define DECLARE_INTERFACE(iface)                interface iface
#define PURE = 0
#define THIS_
#define DECLSPEC_UUID(x)
#define DECLSPEC_NOVTABLE
#define DECLSPEC_SELECTANY
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif
#define APIENTRY
#define OUT
#define IN
#define CONST const
#define MAX_PATH 260
#define GENERIC_ALL 0x10000000L
#define C_ASSERT(expr) static_assert((expr))
#define _countof(a) (sizeof(a) / sizeof(*(a)))

typedef struct tagRECTL
{
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECTL;

typedef struct tagPOINT
{
    int x;
    int y;
} POINT;

typedef struct _GUID {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[ 8 ];
} GUID;

#ifdef __cplusplus
#ifdef INITGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) extern "C" const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#else
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) extern "C" const GUID name
#endif

template <typename T> GUID uuidof() = delete;
template <typename T> GUID uuidof(T*) { return uuidof<T>(); }
template <typename T> GUID uuidof(T**) { return uuidof<T>(); }
template <typename T> GUID uuidof(T&) { return uuidof<T>(); }
#define __uuidof(x) uuidof(x)
#else
#ifdef INITGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#else
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) extern const GUID name
#endif
#endif

typedef GUID IID;
typedef GUID UUID;
typedef GUID CLSID;
#ifdef __cplusplus
#define REFGUID const GUID &
#define REFIID const IID &
#define REFCLSID const IID &

__inline int InlineIsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
    return (
        ((uint32_t *)&rguid1)[0] == ((uint32_t *)&rguid2)[0] &&
        ((uint32_t *)&rguid1)[1] == ((uint32_t *)&rguid2)[1] &&
        ((uint32_t *)&rguid1)[2] == ((uint32_t *)&rguid2)[2] &&
        ((uint32_t *)&rguid1)[3] == ((uint32_t *)&rguid2)[3]);
}

inline bool operator==(REFGUID guidOne, REFGUID guidOther)
{
    return !!InlineIsEqualGUID(guidOne, guidOther);
}

inline bool operator!=(REFGUID guidOne, REFGUID guidOther)
{
    return !(guidOne == guidOther);
}

#else
#define REFGUID const GUID *
#define REFIID const IID *
#define REFCLSID const IID *
#endif

// SAL annotations
#define _In_
#define _In_z_
#define _In_opt_
#define _In_opt_z_
#define _In_reads_(x)
#define _In_reads_opt_(x)
#define _In_reads_bytes_(x)
#define _In_reads_bytes_opt_(x)
#define _In_range_(x, y)
#define _In_bytecount_(x)
#define _Out_
#define _Out_opt_
#define _Outptr_
#define _Outptr_opt_result_z_
#define _Outptr_opt_result_bytebuffer_(x)
#define _COM_Outptr_
#define _COM_Outptr_result_maybenull_
#define _COM_Outptr_opt_
#define _COM_Outptr_opt_result_maybenull_
#define _Out_writes_(x)
#define _Out_writes_z_(x)
#define _Out_writes_opt_(x)
#define _Out_writes_all_(x)
#define _Out_writes_all_opt_(x)
#define _Out_writes_to_opt_(x, y)
#define _Out_writes_bytes_(x)
#define _Out_writes_bytes_all_(x)
#define _Out_writes_bytes_all_opt_(x)
#define _Out_writes_bytes_opt_(x)
#define _Inout_
#define _Inout_opt_
#define _Inout_updates_(x)
#define _Inout_updates_bytes_(x)
#define _Field_size_(x)
#define _Field_size_opt_(x)
#define _Field_size_bytes_(x)
#define _Field_size_full_(x)
#define _Field_size_full_opt_(x)
#define _Field_size_bytes_full_(x)
#define _Field_size_bytes_full_opt_(x)
#define _Field_size_bytes_part_(x, y)
#define _Field_range_(x, y)
#define _Field_z_
#define _Check_return_
#define _IRQL_requires_(x)
#define _IRQL_requires_min_(x)
#define _IRQL_requires_max_(x)
#define _At_(x, y)
#define _Always_(x)
#define _Return_type_success_(x)
#define _Translates_Win32_to_HRESULT_(x)
#define _Maybenull_
#define _Outptr_result_maybenull_
#define _Outptr_result_nullonfailure_
#define _Analysis_assume_(x)
#define _Success_(x)
#define _In_count_(x)
#define _In_opt_count_(x)
#define _Use_decl_annotations_

// Calling conventions
#define __cdecl
#define __stdcall
#define STDMETHODCALLTYPE
#define STDAPICALLTYPE
#define STDAPI extern "C" HRESULT STDAPICALLTYPE
#define WINAPI
#define STDMETHOD(name) virtual HRESULT name
#define STDMETHOD_(type,name) virtual type name
#define IFACEMETHOD(method) /*__override*/ STDMETHOD(method)
#define IFACEMETHOD_(type, method) /*__override*/ STDMETHOD_(type, method)

// Error codes
typedef LONG HRESULT;
#define SUCCEEDED(hr)  (((HRESULT)(hr)) >= 0)
#define FAILED(hr)     (((HRESULT)(hr)) < 0)
#define S_OK           ((HRESULT)0L)
#define S_FALSE        ((HRESULT)1L)
#define E_NOTIMPL      ((HRESULT)0x80004001L)
#define E_OUTOFMEMORY  ((HRESULT)0x8007000EL)
#define E_INVALIDARG   ((HRESULT)0x80070057L)
#define E_NOINTERFACE  ((HRESULT)0x80004002L)
#define E_POINTER      ((HRESULT)0x80004003L)
#define E_HANDLE       ((HRESULT)0x80070006L)
#define E_ABORT        ((HRESULT)0x80004004L)
#define E_FAIL         ((HRESULT)0x80004005L)
#define E_ACCESSDENIED ((HRESULT)0x80070005L)
#define E_UNEXPECTED   ((HRESULT)0x8000FFFFL)
#define DXGI_ERROR_INVALID_CALL ((HRESULT)0x887A0001L)
#define DXGI_ERROR_NOT_FOUND ((HRESULT)0x887A0002L)
#define DXGI_ERROR_MORE_DATA ((HRESULT)0x887A0003L)
#define DXGI_ERROR_UNSUPPORTED ((HRESULT)0x887A0004L)
#define DXGI_ERROR_DEVICE_REMOVED ((HRESULT)0x887A0005L)
#define DXGI_ERROR_DEVICE_HUNG ((HRESULT)0x887A0006L)
#define DXGI_ERROR_DEVICE_RESET ((HRESULT)0x887A0007L)
#define DXGI_ERROR_DRIVER_INTERNAL_ERROR ((HRESULT)0x887A0020L)

typedef struct _LUID
{
    ULONG LowPart;
    LONG HighPart;
} LUID;

typedef struct _RECT
{
    int left;
    int top;
    int right;
    int bottom;
} RECT;

typedef union _LARGE_INTEGER {
  struct {
    uint32_t LowPart;
    uint32_t HighPart;
  } u;
  int64_t QuadPart;
} LARGE_INTEGER;
typedef LARGE_INTEGER *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER {
  struct {
    uint32_t LowPart;
    uint32_t HighPart;
  } u;
  uint64_t QuadPart;
} ULARGE_INTEGER;
typedef ULARGE_INTEGER *PULARGE_INTEGER;

#define DECLARE_HANDLE(name)                                                   \
  struct name##__ {                                                            \
    int unused;                                                                \
  };                                                                           \
  typedef struct name##__ *name

struct SECURITY_ATTRIBUTES;
struct STATSTG;

#ifdef __cplusplus
// ENUM_FLAG_OPERATORS
// Define operator overloads to enable bit operations on enum values that are
// used to define flags. Use DEFINE_ENUM_FLAG_OPERATORS(YOUR_TYPE) to enable these
// operators on YOUR_TYPE.
extern "C++" {
    template <size_t S>
    struct _ENUM_FLAG_INTEGER_FOR_SIZE;

    template <>
    struct _ENUM_FLAG_INTEGER_FOR_SIZE<1>
    {
        typedef int8_t type;
    };

    template <>
    struct _ENUM_FLAG_INTEGER_FOR_SIZE<2>
    {
        typedef int16_t type;
    };

    template <>
    struct _ENUM_FLAG_INTEGER_FOR_SIZE<4>
    {
        typedef int32_t type;
    };

    template <>
    struct _ENUM_FLAG_INTEGER_FOR_SIZE<8>
    {
        typedef int64_t type;
    };

    // used as an approximation of std::underlying_type<T>
    template <class T>
    struct _ENUM_FLAG_SIZED_INTEGER
    {
        typedef typename _ENUM_FLAG_INTEGER_FOR_SIZE<sizeof(T)>::type type;
    };

}
#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE) \
extern "C++" { \
inline constexpr ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) | ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) |= ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline constexpr ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) & ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) &= ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline constexpr ENUMTYPE operator ~ (ENUMTYPE a) { return ENUMTYPE(~((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a)); } \
inline constexpr ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) ^ ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) ^= ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
}
#endif

// D3DX12 uses these
#include <stdlib.h>
#define HeapAlloc(heap, flags, size) malloc(size)
#define HeapFree(heap, flags, ptr) free(ptr)

#ifdef __cplusplus
// IUnknown

interface DECLSPEC_UUID("00000000-0000-0000-C000-000000000046") DECLSPEC_NOVTABLE IUnknown
{
   virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) = 0;
   virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
   virtual ULONG STDMETHODCALLTYPE Release() = 0;

   template <class Q> HRESULT STDMETHODCALLTYPE QueryInterface(Q** pp) {
       return QueryInterface(uuidof<Q>(), (void **)pp);
   }
};

template <> constexpr GUID uuidof<IUnknown>()
{
    return { 0x00000000, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
}

extern "C++"
{
    template<typename T> void** IID_PPV_ARGS_Helper(T** pp)
    {
        static_cast<IUnknown*>(*pp);
        return reinterpret_cast<void**>(pp);
    }
}

#define IID_PPV_ARGS(ppType) __uuidof(**(ppType)), IID_PPV_ARGS_Helper(ppType)
#endif

#if defined(lint)
// Note: lint -e530 says don't complain about uninitialized variables for
// this variable.  Error 527 has to do with unreachable code.
// -restore restores checking to the -save state
#define UNREFERENCED_PARAMETER(P) \
    /*lint -save -e527 -e530 */ \
    { \
        (P) = (P); \
    } \
    /*lint -restore */
#else
#define UNREFERENCED_PARAMETER(P) (P)
#endif
