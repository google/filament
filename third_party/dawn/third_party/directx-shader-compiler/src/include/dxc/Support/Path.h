///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Path.h                                                                    //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Helper for HLSL related file paths.                                       //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "llvm/ADT/StringRef.h"
#include <string>

namespace hlsl {

template <typename CharTy>
bool IsAbsoluteOrCurDirRelativeImpl(const CharTy *Path, size_t Len) {
  if (Len == 1 && Path[0] == '.')
    return true;
  // Current dir-relative path.
  if (Len >= 2 && Path[0] == '.' && (Path[1] == '/' || Path[1] == '\\')) {
    return true;
  }
  // Disk designator, then absolute path.
  if (Len >= 3 && Path[1] && Path[1] == ':' &&
      (Path[2] == '\\' || Path[2] == '/')) {
    return true;
  }
  // UNC name
  if (Len >= 2 && Path[0] == '\\') {
    return Path[1] == '\\';
  }

#ifndef _WIN32
  // Absolute paths on unix systems start with '/'
  if (Len >= 1 && Path[0] == '/') {
    return true;
  }
#endif

  //
  // NOTE: there are a number of cases we don't handle, as they don't play well
  // with the simple file system abstraction we use:
  // - current directory on disk designator (eg, D:file.ext), requires per-disk
  // current dir
  // - parent paths relative to current directory (eg, ..\\file.ext)
  //
  // The current-directory support is available to help in-memory handlers.
  // On-disk handlers will typically have absolute paths to begin with.
  //
  return false;
}

inline bool IsAbsoluteOrCurDirRelativeW(const wchar_t *Path) {
  if (!Path)
    return false;
  return IsAbsoluteOrCurDirRelativeImpl<wchar_t>(Path, wcslen(Path));
}
inline bool IsAbsoluteOrCurDirRelative(const char *Path) {
  if (!Path)
    return false;
  return IsAbsoluteOrCurDirRelativeImpl<char>(Path, strlen(Path));
}

template <typename CharT, typename StringTy>
void RemoveDoubleSlashes(StringTy &Path, CharT Slash) {
  // Remove double slashes.
  bool SeenNonSlash = false;
  for (unsigned i = 0; i < Path.size();) {
    // Remove this slash if:
    // 1. It is preceded by another slash.
    // 2. It is NOT part of a series of leading slashes. (E.G. \\, which on
    // windows is a network path).
    if (Path[i] == Slash && i > 0 && Path[i - 1] == Slash && SeenNonSlash) {
      Path.erase(Path.begin() + i);
      continue;
    }
    SeenNonSlash |= Path[i] != Slash;
    i++;
  }
}

// This is the new ground truth of how paths are normalized. There had been
// many inconsistent path normalization littered all over the code base.
// 1. All slashes are changed to system native: `\` for windows and `/` for all
// others.
// 2. All repeated slashes are removed (except for leading slashes, so windows
// UNC paths are not broken)
// 3. All relative paths (including ones that begin with ..) are prepended with
// ./ or .\ if not already
//
// Examples:
//   F:\\\my_path////\\/my_shader.hlsl   ->  F:\my_path\my_shader.hlsl
//   my_path/my_shader.hlsl              ->  .\my_path\my_shader.hlsl
//   ..\\.//.\\\my_path/my_shader.hlsl   ->  .\..\.\.\my_path\my_shader.hlsl
//   \\my_network_path/my_shader.hlsl    ->  \\my_network_path\my_shader.hlsl
//
template <typename CharT, typename StringTy>
StringTy NormalizePathImpl(const CharT *Path, size_t Length) {
  StringTy PathCopy(Path, Length);

#ifdef _WIN32
  constexpr CharT SlashFrom = '/';
  constexpr CharT SlashTo = '\\';
#else
  constexpr CharT SlashFrom = '\\';
  constexpr CharT SlashTo = '/';
#endif

  for (unsigned i = 0; i < PathCopy.size(); i++) {
    if (PathCopy[i] == SlashFrom)
      PathCopy[i] = SlashTo;
  }

  RemoveDoubleSlashes<CharT, StringTy>(PathCopy, SlashTo);

  // If relative path, prefix with dot.
  if (IsAbsoluteOrCurDirRelativeImpl<CharT>(PathCopy.c_str(),
                                            PathCopy.size())) {
    return PathCopy;
  } else {
    PathCopy = StringTy(1, CharT('.')) + StringTy(1, SlashTo) + PathCopy;
    RemoveDoubleSlashes<CharT, StringTy>(PathCopy, SlashTo);
    return PathCopy;
  }
}

inline std::string NormalizePath(const char *Path) {
  return NormalizePathImpl<char, std::string>(Path, ::strlen(Path));
}
inline std::wstring NormalizePathW(const wchar_t *Path) {
  return NormalizePathImpl<wchar_t, std::wstring>(Path, ::wcslen(Path));
}
inline std::string NormalizePath(llvm::StringRef Path) {
  return NormalizePathImpl<char, std::string>(Path.data(), Path.size());
}

} // namespace hlsl
