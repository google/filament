///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// HlslTestUtils.h                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides utility functions for HLSL tests.                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// *** THIS FILE CANNOT TAKE ANY LLVM DEPENDENCIES  *** //
#ifndef HLSLTESTUTILS_H
#define HLSLTESTUTILS_H

#include <algorithm>
#include <atomic>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32

// Disable -Wignored-qualifiers for WexTestClass.h.
// For const size_t GetSize() const; in TestData.h.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#endif
#include "WexTestClass.h"
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <dxgiformat.h>
#else
#include "WEXAdapter.h"
#include "dxc/Support/Global.h" // DXASSERT_LOCALVAR
#endif
#include "dxc/DXIL/DxilConstants.h" // DenormMode

#ifdef _HLK_CONF
#define DEFAULT_TEST_DIR L""
#define DEFAULT_EXEC_TEST_DIR DEFAULT_TEST_DIR
#else
#include "dxc/Test/TestConfig.h"
#endif

using namespace std;

#ifndef HLSLDATAFILEPARAM
#define HLSLDATAFILEPARAM L"HlslDataDir"
#endif

#ifndef FILECHECKDUMPDIRPARAM
#define FILECHECKDUMPDIRPARAM L"FileCheckDumpDir"
#endif

// If TAEF verify macros are available, use them to alias other legacy
// comparison macros that don't have a direct translation.
//
// Other common replacements are as follows.
//
// EXPECT_EQ -> VERIFY_ARE_EQUAL
// ASSERT_EQ -> VERIFY_ARE_EQUAL
//
// Note that whether verification throws or continues depends on
// preprocessor settings.

#ifdef VERIFY_ARE_EQUAL
#ifndef EXPECT_STREQ
#define EXPECT_STREQ(a, b) VERIFY_ARE_EQUAL(0, strcmp(a, b))
#endif
#define EXPECT_STREQW(a, b) VERIFY_ARE_EQUAL(0, wcscmp(a, b))
#define VERIFY_ARE_EQUAL_CMP(a, b, ...) VERIFY_IS_TRUE(a == b, __VA_ARGS__)
#define VERIFY_ARE_EQUAL_STR(a, b)                                             \
  {                                                                            \
    const char *pTmpA = (a);                                                   \
    const char *pTmpB = (b);                                                   \
    if (0 != strcmp(pTmpA, pTmpB)) {                                           \
      CA2W conv(pTmpB);                                                        \
      WEX::Logging::Log::Comment(conv);                                        \
      const char *pA = pTmpA;                                                  \
      const char *pB = pTmpB;                                                  \
      while (*pA == *pB) {                                                     \
        pA++;                                                                  \
        pB++;                                                                  \
      }                                                                        \
      wchar_t diffMsg[32];                                                     \
      swprintf_s(diffMsg, _countof(diffMsg), L"diff at %u",                    \
                 (unsigned)(pA - pTmpA));                                      \
      WEX::Logging::Log::Comment(diffMsg);                                     \
    }                                                                          \
    VERIFY_ARE_EQUAL(0, strcmp(pTmpA, pTmpB));                                 \
  }
#define VERIFY_ARE_EQUAL_WSTR(a, b)                                            \
  {                                                                            \
    if (0 != wcscmp(a, b)) {                                                   \
      WEX::Logging::Log::Comment(b);                                           \
    }                                                                          \
    VERIFY_ARE_EQUAL(0, wcscmp(a, b));                                         \
  }
#ifndef ASSERT_EQ
#define ASSERT_EQ(expected, actual) VERIFY_ARE_EQUAL(expected, actual)
#endif
#ifndef ASSERT_NE
#define ASSERT_NE(expected, actual) VERIFY_ARE_NOT_EQUAL(expected, actual)
#endif
#ifndef TEST_F
#define TEST_F(typeName, functionName) void typeName::functionName()
#endif
#define ASSERT_HRESULT_SUCCEEDED VERIFY_SUCCEEDED
#ifndef EXPECT_EQ
#define EXPECT_EQ(expected, actual) VERIFY_ARE_EQUAL(expected, actual)
#endif
#endif // VERIFY_ARE_EQUAL

static constexpr char whitespaceChars[] = " \t\r\n";
static constexpr wchar_t wideWhitespaceChars[] = L" \t\r\n";

inline std::string strltrim(const std::string &value) {
  size_t first = value.find_first_not_of(whitespaceChars);
  return first == string::npos ? value : value.substr(first);
}

inline std::string strrtrim(const std::string &value) {
  size_t last = value.find_last_not_of(whitespaceChars);
  return last == string::npos ? value : value.substr(0, last + 1);
}

inline std::string strtrim(const std::string &value) {
  return strltrim(strrtrim(value));
}

inline bool strstartswith(const std::string &value, const char *pattern) {
  for (size_t i = 0;; ++i) {
    if (pattern[i] == '\0')
      return true;
    if (i == value.size() || value[i] != pattern[i])
      return false;
  }
}

inline std::vector<std::string>
strtok(const std::string &value, const char *delimiters = whitespaceChars) {
  size_t searchOffset = 0;
  std::vector<std::string> tokens;
  while (searchOffset != value.size()) {
    size_t tokenStartIndex = value.find_first_not_of(delimiters, searchOffset);
    if (tokenStartIndex == std::string::npos)
      break;
    size_t tokenEndIndex = value.find_first_of(delimiters, tokenStartIndex);
    if (tokenEndIndex == std::string::npos)
      tokenEndIndex = value.size();
    tokens.emplace_back(
        value.substr(tokenStartIndex, tokenEndIndex - tokenStartIndex));
    searchOffset = tokenEndIndex;
  }
  return tokens;
}

inline std::vector<std::wstring>
strtok(const std::wstring &value,
       const wchar_t *delimiters = wideWhitespaceChars) {
  size_t searchOffset = 0;
  std::vector<std::wstring> tokens;
  while (searchOffset != value.size()) {
    size_t tokenStartIndex = value.find_first_not_of(delimiters, searchOffset);
    if (tokenStartIndex == std::string::npos)
      break;
    size_t tokenEndIndex = value.find_first_of(delimiters, tokenStartIndex);
    if (tokenEndIndex == std::string::npos)
      tokenEndIndex = value.size();
    tokens.emplace_back(
        value.substr(tokenStartIndex, tokenEndIndex - tokenStartIndex));
    searchOffset = tokenEndIndex;
  }
  return tokens;
}

// strreplace will replace all instances of lookFors with replacements at the
// same index. Will log an error if the string is not found, unless the first
// character is ? marking it optional.
inline void strreplace(const std::vector<std::string> &lookFors,
                       const std::vector<std::string> &replacements,
                       std::string &str) {
  for (unsigned i = 0; i < lookFors.size(); ++i) {
    bool bOptional = false;
    bool found = false;
    size_t pos = 0;
    LPCSTR pLookFor = lookFors[i].data();
    size_t lookForLen = lookFors[i].size();
    if (pLookFor[0] == '?') {
      bOptional = true;
      pLookFor++;
      lookForLen--;
    }
    if (!pLookFor || !*pLookFor) {
      continue;
    }
    for (;;) {
      pos = str.find(pLookFor, pos);
      if (pos == std::string::npos)
        break;
      found = true; // at least once
      str.replace(pos, lookForLen, replacements[i]);
      pos += replacements[i].size();
    }
    if (!bOptional) {
      if (!found) {
        WEX::Logging::Log::Comment(WEX::Common::String().Format(
            L"String not found: '%S' in text:\r\n%.*S", pLookFor,
            (unsigned)str.size(), str.data()));
      }
      VERIFY_IS_TRUE(found);
    }
  }
}

namespace hlsl_test {

inline std::wstring vFormatToWString(const wchar_t *fmt, va_list argptr) {
  std::wstring result;
#ifdef _WIN32
  int len = _vscwprintf(fmt, argptr);
  result.resize(len + 1);
  vswprintf_s((wchar_t *)result.data(), len + 1, fmt, argptr);
#else
  wchar_t fmtOut[1000];
  int len = vswprintf(fmtOut, 1000, fmt, argptr);
  DXASSERT_LOCALVAR(len, len >= 0,
                    "Too long formatted string in vFormatToWstring");
  result = fmtOut;
#endif
  return result;
}

inline std::wstring FormatToWString(const wchar_t *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::wstring result(vFormatToWString(fmt, args));
  va_end(args);
  return result;
}

inline void LogCommentFmt(const wchar_t *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::wstring buf(vFormatToWString(fmt, args));
  va_end(args);
  WEX::Logging::Log::Comment(buf.data());
}

inline void LogErrorFmt(const wchar_t *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::wstring buf(vFormatToWString(fmt, args));
  va_end(args);
  WEX::Logging::Log::Error(buf.data());
}

inline void LogErrorFmtThrow(const char *fileName, int line, const wchar_t *fmt,
                             ...) {
  va_list args;
  va_start(args, fmt);
  std::wstring buf(vFormatToWString(fmt, args));
  va_end(args);

  std::wstringstream wss;
  wss << L"Error in file: " << fileName << L" at line: " << line << L"\n"
      << buf.data() << L"\n"
      << buf;

  WEX::Logging::Log::Error(wss.str().c_str());

  // Throws an exception to abort the test.
  VERIFY_FAIL(L"Test error");
}

// Macro to pass the file name and line number. Otherwise TAEF prints this file
// and line number.
#define LOG_ERROR_FMT_THROW(fmt, ...)                                          \
  hlsl_test::LogErrorFmtThrow(__FILE__, __LINE__, fmt, __VA_ARGS__)

inline std::wstring
GetPathToHlslDataFile(const wchar_t *relative,
                      LPCWSTR paramName = HLSLDATAFILEPARAM,
                      LPCWSTR defaultDataDir = DEFAULT_TEST_DIR) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  WEX::Common::String HlslDataDirValue;
  if (std::wstring(paramName).compare(HLSLDATAFILEPARAM) != 0) {
    // Not fatal, for instance, FILECHECKDUMPDIRPARAM will dump files before
    // running FileCheck, so they can be compared run to run
    if (FAILED(WEX::TestExecution::RuntimeParameters::TryGetValue(
            paramName, HlslDataDirValue)))
      return std::wstring();
  } else {
    if (FAILED(WEX::TestExecution::RuntimeParameters::TryGetValue(
            HLSLDATAFILEPARAM, HlslDataDirValue)))
      HlslDataDirValue = defaultDataDir;
  }

  wchar_t envPath[MAX_PATH];
  wchar_t expanded[MAX_PATH];
  swprintf_s(envPath, _countof(envPath), L"%ls\\%ls",
             reinterpret_cast<const wchar_t *>(HlslDataDirValue.GetBuffer()),
             relative);
  VERIFY_WIN32_BOOL_SUCCEEDED(
      ExpandEnvironmentStringsW(envPath, expanded, _countof(expanded)));
  return std::wstring(expanded);
}

inline bool PathLooksAbsolute(LPCWSTR name) {
  // Very simplified, only for the cases we care about in the test suite.
#ifdef _WIN32
  return name && *name && ((*name == L'\\') || (name[1] == L':'));
#else
  return name && *name && (*name == L'/');
#endif
}

static bool HasRunLine(std::string &line) {
  const char *delimiters = " ;/";
  auto lineelems = strtok(line, delimiters);
  return !lineelems.empty() && lineelems.front().compare("RUN:") == 0;
}

inline std::vector<std::string> GetRunLines(const LPCWSTR name) {
  const std::wstring path = PathLooksAbsolute(name)
                                ? std::wstring(name)
                                : hlsl_test::GetPathToHlslDataFile(name);
#ifdef _WIN32
  std::ifstream infile(path);
#else
  std::ifstream infile((CW2A(path.c_str())));
#endif
  if (infile.fail() || infile.bad()) {
    std::wstring errMsg(L"Unable to read file ");
    errMsg += path;
    WEX::Logging::Log::Error(errMsg.c_str());
    VERIFY_FAIL();
  }

  std::vector<std::string> runlines;
  std::string line;
  constexpr size_t runlinesize = 300;
  while (std::getline(infile, line)) {
    if (!HasRunLine(line))
      continue;
    char runline[runlinesize];
    memset(runline, 0, runlinesize);
    memcpy(runline, line.c_str(), min(runlinesize, line.size()));
    runlines.emplace_back(runline);
  }
  return runlines;
}

inline std::string GetFirstLine(LPCWSTR name) {
  char firstLine[300];
  memset(firstLine, 0, sizeof(firstLine));

  const std::wstring path = PathLooksAbsolute(name)
                                ? std::wstring(name)
                                : hlsl_test::GetPathToHlslDataFile(name);
#ifdef _WIN32
  std::ifstream infile(path);
#else
  std::ifstream infile((CW2A(path.c_str())));
#endif
  if (infile.bad()) {
    std::wstring errMsg(L"Unable to read file ");
    errMsg += path;
    WEX::Logging::Log::Error(errMsg.c_str());
    VERIFY_FAIL();
  }

  infile.getline(firstLine, _countof(firstLine));
  return firstLine;
}

inline HANDLE CreateFileForReading(LPCWSTR path) {
  HANDLE sourceHandle =
      CreateFileW(path, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
  if (sourceHandle == INVALID_HANDLE_VALUE) {
    DWORD err = GetLastError();
    std::wstring errorMessage(
        FormatToWString(L"Unable to open file '%s', err=%u", path, err)
            .c_str());
    VERIFY_SUCCEEDED(HRESULT_FROM_WIN32(err), errorMessage.c_str());
  }
  return sourceHandle;
}

inline HANDLE CreateNewFileForReadWrite(LPCWSTR path) {
  HANDLE sourceHandle = CreateFileW(path, GENERIC_READ | GENERIC_WRITE, 0, 0,
                                    CREATE_ALWAYS, 0, 0);
  if (sourceHandle == INVALID_HANDLE_VALUE) {
    DWORD err = GetLastError();
    std::wstring errorMessage(
        FormatToWString(L"Unable to create file '%s', err=%u", path, err)
            .c_str());
    VERIFY_SUCCEEDED(HRESULT_FROM_WIN32(err), errorMessage.c_str());
  }
  return sourceHandle;
}

// Copy of Unicode::IsStarMatchT/IsStarMatchWide is included here to avoid the
// dependency on DXC support libraries.
template <typename TChar>
inline static bool IsStarMatchT(const TChar *pMask, size_t maskLen,
                                const TChar *pName, size_t nameLen,
                                TChar star) {
  if (maskLen == 0 && nameLen == 0) {
    return true;
  }
  if (maskLen == 0 || nameLen == 0) {
    return false;
  }

  if (pMask[maskLen - 1] == star) {
    // Prefix match.
    if (maskLen == 1) { // For just '*', everything is a match.
      return true;
    }
    --maskLen;
    if (maskLen > nameLen) { // Mask is longer than name, can't be a match.
      return false;
    }
    return 0 == memcmp(pMask, pName, sizeof(TChar) * maskLen);
  } else {
    // Exact match.
    if (nameLen != maskLen) {
      return false;
    }
    return 0 == memcmp(pMask, pName, sizeof(TChar) * nameLen);
  }
}

inline bool IsStarMatchWide(const wchar_t *pMask, size_t maskLen,
                            const wchar_t *pName, size_t nameLen) {
  return IsStarMatchT<wchar_t>(pMask, maskLen, pName, nameLen, L'*');
}

inline bool GetTestParamBool(LPCWSTR name) {
  WEX::Common::String ParamValue;
  WEX::Common::String NameValue;
  if (FAILED(WEX::TestExecution::RuntimeParameters::TryGetValue(name,
                                                                ParamValue))) {
    return false;
  }
  if (ParamValue.IsEmpty()) {
    return false;
  }
  if (0 == wcscmp(ParamValue, L"*")) {
    return true;
  }
  VERIFY_SUCCEEDED(WEX::TestExecution::RuntimeParameters::TryGetValue(
      L"TestName", NameValue));
  if (NameValue.IsEmpty()) {
    return false;
  }

  return hlsl_test::IsStarMatchWide(ParamValue, ParamValue.GetLength(),
                                    NameValue, NameValue.GetLength());
}

inline bool GetTestParamUseWARP(bool defaultVal) {
  WEX::Common::String AdapterValue;
  if (FAILED(WEX::TestExecution::RuntimeParameters::TryGetValue(
          L"Adapter", AdapterValue))) {
    return defaultVal;
  }
  if ((defaultVal && AdapterValue.IsEmpty()) ||
      AdapterValue.CompareNoCase(L"WARP") == 0 ||
      AdapterValue.CompareNoCase(L"Microsoft Basic Render Driver") == 0) {
    return true;
  }
  return false;
}

} // namespace hlsl_test

#ifdef FP_SUBNORMAL

template <typename T> inline bool isdenorm(T f) {
  return FP_SUBNORMAL == std::fpclassify(f);
}

#else

template <typename T> inline bool isdenorm(T f) {
  return (std::numeric_limits<T>::denorm_min() <= f &&
          f < std::numeric_limits<T>::min()) ||
         (-std::numeric_limits<T>::min() < f &&
          f <= -std::numeric_limits<T>::denorm_min());
}

#endif // FP_SUBNORMAL

inline float ifdenorm_flushf(float a) {
  return isdenorm(a) ? copysign(0.0f, a) : a;
}

inline bool ifdenorm_flushf_eq(float a, float b) {
  return ifdenorm_flushf(a) == ifdenorm_flushf(b);
}

static const uint16_t Float16NaN = 0xff80;
static const uint16_t Float16PosInf = 0x7c00;
static const uint16_t Float16NegInf = 0xfc00;
static const uint16_t Float16PosDenorm = 0x0008;
static const uint16_t Float16NegDenorm = 0x8008;
static const uint16_t Float16PosZero = 0x0000;
static const uint16_t Float16NegZero = 0x8000;

inline bool GetSign(float x) { return std::signbit(x); }

inline int GetMantissa(float x) {
  int bits = reinterpret_cast<int &>(x);
  return bits & 0x7fffff;
}

inline int GetExponent(float x) {
  int bits = reinterpret_cast<int &>(x);
  return (bits >> 23) & 0xff;
}

#define FLOAT16_BIT_SIGN 0x8000
#define FLOAT16_BIT_EXP 0x7c00
#define FLOAT16_BIT_MANTISSA 0x03ff
#define FLOAT16_BIGGEST_DENORM FLOAT16_BIT_MANTISSA
#define FLOAT16_BIGGEST_NORMAL 0x7bff

inline bool isnanFloat16(uint16_t val) {
  return (val & FLOAT16_BIT_EXP) == FLOAT16_BIT_EXP &&
         (val & FLOAT16_BIT_MANTISSA) != 0;
}

// These are defined in ShaderOpTest.cpp using DirectXPackedVector functions.
uint16_t ConvertFloat32ToFloat16(float val) throw();
float ConvertFloat16ToFloat32(uint16_t val) throw();

inline bool CompareDoubleULP(
    const double &Src, const double &Ref, int64_t ULPTolerance,
    hlsl::DXIL::Float32DenormMode Mode = hlsl::DXIL::Float32DenormMode::Any) {
  if (Src == Ref) {
    return true;
  }
  if (std::isnan(Src)) {
    return std::isnan(Ref);
  }

  if (Mode == hlsl::DXIL::Float32DenormMode::Any) {
    // If denorm expected, output can be sign preserved zero. Otherwise output
    // should pass the regular ulp testing.
    if (isdenorm(Ref) && Src == 0 && std::signbit(Src) == std::signbit(Ref))
      return true;
  }

  // For FTZ or Preserve mode, we should get the expected number within
  // ULPTolerance for any operations.
  int64_t Diff = *((const uint64_t *)&Src) - *((const uint64_t *)&Ref);

  uint64_t AbsoluteDiff = Diff < 0 ? -Diff : Diff;
  return AbsoluteDiff <= (uint64_t)ULPTolerance;
}

inline bool CompareDoubleEpsilon(const double &Src, const double &Ref,
                                 float Epsilon) {
  if (Src == Ref) {
    return true;
  }
  if (std::isnan(Src)) {
    return std::isnan(Ref);
  }
  // For FTZ or Preserve mode, we should get the expected number within
  // epsilon for any operations.
  return fabs(Src - Ref) < Epsilon;
}

inline bool CompareFloatULP(
    const float &fsrc, const float &fref, int ULPTolerance,
    hlsl::DXIL::Float32DenormMode mode = hlsl::DXIL::Float32DenormMode::Any) {
  if (fsrc == fref) {
    return true;
  }
  if (std::isnan(fsrc)) {
    return std::isnan(fref);
  }
  if (mode == hlsl::DXIL::Float32DenormMode::Any) {
    // If denorm expected, output can be sign preserved zero. Otherwise output
    // should pass the regular ulp testing.
    if (isdenorm(fref) && fsrc == 0 && std::signbit(fsrc) == std::signbit(fref))
      return true;
  }
  // For FTZ or Preserve mode, we should get the expected number within
  // ULPTolerance for any operations.
  int diff = *((const DWORD *)&fsrc) - *((const DWORD *)&fref);
  unsigned int uDiff = diff < 0 ? -diff : diff;
  return uDiff <= (unsigned int)ULPTolerance;
}

inline bool CompareFloatEpsilon(
    const float &fsrc, const float &fref, float epsilon,
    hlsl::DXIL::Float32DenormMode mode = hlsl::DXIL::Float32DenormMode::Any) {
  if (fsrc == fref) {
    return true;
  }
  if (std::isnan(fsrc)) {
    return std::isnan(fref);
  }
  if (mode == hlsl::DXIL::Float32DenormMode::Any) {
    // If denorm expected, output can be sign preserved zero. Otherwise output
    // should pass the regular epsilon testing.
    if (isdenorm(fref) && fsrc == 0 && std::signbit(fsrc) == std::signbit(fref))
      return true;
  }
  // For FTZ or Preserve mode, we should get the expected number within
  // epsilon for any operations.
  return fabsf(fsrc - fref) < epsilon;
}

// Compare using relative error (relative error < 2^{nRelativeExp})
inline bool CompareFloatRelativeEpsilon(
    const float &fsrc, const float &fref, int nRelativeExp,
    hlsl::DXIL::Float32DenormMode mode = hlsl::DXIL::Float32DenormMode::Any) {
  return CompareFloatULP(fsrc, fref, 23 - nRelativeExp, mode);
}

inline bool CompareHalfULP(const uint16_t &fsrc, const uint16_t &fref,
                           float ULPTolerance) {
  // Treat +0 and -0 as equal
  if ((fsrc & ~FLOAT16_BIT_SIGN) == 0 && (fref & ~FLOAT16_BIT_SIGN) == 0)
    return true;
  if (fsrc == fref)
    return true;

  const bool nanRef = isnanFloat16(fref);
  const bool nanSrc = isnanFloat16(fsrc);
  if (nanRef || nanSrc)
    return nanRef && nanSrc;

  // Map to monotonic ordering for correct ULP diff
  auto toOrdered = [](uint16_t h) -> int {
    return (h & FLOAT16_BIT_SIGN) ? (~h & 0xFFFF) : (h | 0x8000);
  };

  // 16-bit floating point numbers must preserve denorms
  int i_fsrc = toOrdered(fsrc);
  int i_fref = toOrdered(fref);
  int diff = i_fsrc - i_fref;
  unsigned int uDiff = diff < 0 ? -diff : diff;
  return uDiff <= (unsigned int)ULPTolerance;
}

inline bool CompareHalfEpsilon(const uint16_t &fsrc, const uint16_t &fref,
                               float epsilon) {
  if (fsrc == fref)
    return true;
  if (isnanFloat16(fsrc))
    return isnanFloat16(fref);
  float src_f32 = ConvertFloat16ToFloat32(fsrc);
  float ref_f32 = ConvertFloat16ToFloat32(fref);
  return std::abs(src_f32 - ref_f32) < epsilon;
}

inline bool CompareHalfRelativeEpsilon(const uint16_t &fsrc,
                                       const uint16_t &fref, int nRelativeExp) {
  return CompareHalfULP(fsrc, fref, (float)(10 - nRelativeExp));
}

#ifdef _WIN32
// returns the number of bytes per pixel for a given dxgi format
// add more cases if different format needed to copy back resources
inline UINT GetByteSizeForFormat(DXGI_FORMAT value) {
  switch (value) {
  case DXGI_FORMAT_R32G32B32A32_TYPELESS:
    return 16;
  case DXGI_FORMAT_R32G32B32A32_FLOAT:
    return 16;
  case DXGI_FORMAT_R32G32B32A32_UINT:
    return 16;
  case DXGI_FORMAT_R32G32B32A32_SINT:
    return 16;
  case DXGI_FORMAT_R32G32B32_TYPELESS:
    return 12;
  case DXGI_FORMAT_R32G32B32_FLOAT:
    return 12;
  case DXGI_FORMAT_R32G32B32_UINT:
    return 12;
  case DXGI_FORMAT_R32G32B32_SINT:
    return 12;
  case DXGI_FORMAT_R16G16B16A16_TYPELESS:
    return 8;
  case DXGI_FORMAT_R16G16B16A16_FLOAT:
    return 8;
  case DXGI_FORMAT_R16G16B16A16_UNORM:
    return 8;
  case DXGI_FORMAT_R16G16B16A16_UINT:
    return 8;
  case DXGI_FORMAT_R16G16B16A16_SNORM:
    return 8;
  case DXGI_FORMAT_R16G16B16A16_SINT:
    return 8;
  case DXGI_FORMAT_R32G32_TYPELESS:
    return 8;
  case DXGI_FORMAT_R32G32_FLOAT:
    return 8;
  case DXGI_FORMAT_R32G32_UINT:
    return 8;
  case DXGI_FORMAT_R32G32_SINT:
    return 8;
  case DXGI_FORMAT_R32G8X24_TYPELESS:
    return 8;
  case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    return 4;
  case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    return 4;
  case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
    return 4;
  case DXGI_FORMAT_R10G10B10A2_TYPELESS:
    return 4;
  case DXGI_FORMAT_R10G10B10A2_UNORM:
    return 4;
  case DXGI_FORMAT_R10G10B10A2_UINT:
    return 4;
  case DXGI_FORMAT_R11G11B10_FLOAT:
    return 4;
  case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    return 4;
  case DXGI_FORMAT_R8G8B8A8_UNORM:
    return 4;
  case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    return 4;
  case DXGI_FORMAT_R8G8B8A8_UINT:
    return 4;
  case DXGI_FORMAT_R8G8B8A8_SNORM:
    return 4;
  case DXGI_FORMAT_R8G8B8A8_SINT:
    return 4;
  case DXGI_FORMAT_R16G16_TYPELESS:
    return 4;
  case DXGI_FORMAT_R16G16_FLOAT:
    return 4;
  case DXGI_FORMAT_R16G16_UNORM:
    return 4;
  case DXGI_FORMAT_R16G16_UINT:
    return 4;
  case DXGI_FORMAT_R16G16_SNORM:
    return 4;
  case DXGI_FORMAT_R16G16_SINT:
    return 4;
  case DXGI_FORMAT_R32_TYPELESS:
    return 4;
  case DXGI_FORMAT_D32_FLOAT:
    return 4;
  case DXGI_FORMAT_R32_FLOAT:
    return 4;
  case DXGI_FORMAT_R32_UINT:
    return 4;
  case DXGI_FORMAT_R32_SINT:
    return 4;
  case DXGI_FORMAT_R24G8_TYPELESS:
    return 4;
  case DXGI_FORMAT_D24_UNORM_S8_UINT:
    return 4;
  case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    return 4;
  case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    return 4;
  case DXGI_FORMAT_R8G8_TYPELESS:
    return 2;
  case DXGI_FORMAT_R8G8_UNORM:
    return 2;
  case DXGI_FORMAT_R8G8_UINT:
    return 2;
  case DXGI_FORMAT_R8G8_SNORM:
    return 2;
  case DXGI_FORMAT_R8G8_SINT:
    return 2;
  case DXGI_FORMAT_R16_TYPELESS:
    return 2;
  case DXGI_FORMAT_R16_FLOAT:
    return 2;
  case DXGI_FORMAT_D16_UNORM:
    return 2;
  case DXGI_FORMAT_R16_UNORM:
    return 2;
  case DXGI_FORMAT_R16_UINT:
    return 2;
  case DXGI_FORMAT_R16_SNORM:
    return 2;
  case DXGI_FORMAT_R16_SINT:
    return 2;
  case DXGI_FORMAT_R8_TYPELESS:
    return 1;
  case DXGI_FORMAT_R8_UNORM:
    return 1;
  case DXGI_FORMAT_R8_UINT:
    return 1;
  case DXGI_FORMAT_R8_SNORM:
    return 1;
  case DXGI_FORMAT_R8_SINT:
    return 1;
  case DXGI_FORMAT_A8_UNORM:
    return 1;
  case DXGI_FORMAT_R1_UNORM:
    return 1;
  default:
    VERIFY_FAILED(E_INVALIDARG);
    return 0;
  }
}
#endif

#endif // HLSLTESTUTILS_H
