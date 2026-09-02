///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Unicode.h                                                                 //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides utitlity functions to work with Unicode and other encodings.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

#ifdef _WIN32
#include <specstrings.h>
#else
// MultiByteToWideChar which is a Windows-specific method.
// This is a very simplistic implementation for non-Windows platforms. This
// implementation completely ignores CodePage and dwFlags.
int MultiByteToWideChar(uint32_t CodePage, uint32_t dwFlags,
                        const char *lpMultiByteStr, int cbMultiByte,
                        wchar_t *lpWideCharStr, int cchWideChar);

// WideCharToMultiByte is a Windows-specific method.
// This is a very simplistic implementation for non-Windows platforms. This
// implementation completely ignores CodePage and dwFlags.
int WideCharToMultiByte(uint32_t CodePage, uint32_t dwFlags,
                        const wchar_t *lpWideCharStr, int cchWideChar,
                        char *lpMultiByteStr, int cbMultiByte,
                        const char *lpDefaultChar = nullptr,
                        bool *lpUsedDefaultChar = nullptr);
#endif // _WIN32

namespace Unicode {

// Based on
// http://msdn.microsoft.com/en-us/library/windows/desktop/dd374101(v=vs.85).aspx.
enum class Encoding {
  ASCII = 0,
  UTF8,
  UTF8_BOM,
  UTF16_LE,
  UTF16_BE,
  UTF32_LE,
  UTF32_BE
};

// An acp_char is a character encoded in the current Windows ANSI code page.
typedef char acp_char;

// A ccp_char is a character encoded in the console code page.
typedef char ccp_char;

bool UTF8ToConsoleString(const char *text, size_t textLen, std::string *pValue,
                         bool *lossy);

bool UTF8ToConsoleString(const char *text, std::string *pValue, bool *lossy);

bool WideToConsoleString(const wchar_t *text, size_t textLen,
                         std::string *pValue, bool *lossy);

bool WideToConsoleString(const wchar_t *text, std::string *pValue, bool *lossy);

bool UTF8ToWideString(const char *pUTF8, std::wstring *pWide);

bool UTF8ToWideString(const char *pUTF8, size_t cbUTF8, std::wstring *pWide);

std::wstring UTF8ToWideStringOrThrow(const char *pUTF8);

bool WideToUTF8String(const wchar_t *pWide, size_t cWide, std::string *pUTF8);
bool WideToUTF8String(const wchar_t *pWide, std::string *pUTF8);

std::string WideToUTF8StringOrThrow(const wchar_t *pWide);

bool IsStarMatchUTF8(const char *pMask, size_t maskLen, const char *pName,
                     size_t nameLen);
bool IsStarMatchWide(const wchar_t *pMask, size_t maskLen, const wchar_t *pName,
                     size_t nameLen);

bool UTF8BufferToWideComHeap(const char *pUTF8, wchar_t **ppWide) throw();

bool UTF8BufferToWideBuffer(const char *pUTF8, int cbUTF8, wchar_t **ppWide,
                            size_t *pcchWide) throw();

bool WideBufferToUTF8Buffer(const wchar_t *pWide, int cchWide, char **ppUTF8,
                            size_t *pcbUTF8) throw();

} // namespace Unicode
