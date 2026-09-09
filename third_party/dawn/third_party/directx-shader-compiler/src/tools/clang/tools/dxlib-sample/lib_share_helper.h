///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// lib_share_helper.h                                                        //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Helper header for lib_shaer.                                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "llvm/ADT/StringRef.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace llvm {
class Twine;
}

namespace dxcutil {
bool IsAbsoluteOrCurDirRelative(const llvm::Twine &T);
}

namespace libshare {

struct CompileInput {
  std::vector<DxcDefine> &defines;
  std::vector<LPCWSTR> &arguments;
};

class LibCacheManager {
public:
  virtual ~LibCacheManager() {}
  virtual HRESULT
  AddLibBlob(std::string &header, const std::string &snippet,
             CompileInput &compiler, size_t &hash, IDxcBlob **pResultLib,
             std::function<void(IDxcBlob *pSource)> compileFn) = 0;
  virtual bool GetLibBlob(std::string &processedHeader,
                          const std::string &snippet, CompileInput &compiler,
                          size_t &hash, IDxcBlob **pResultLib) = 0;
  static LibCacheManager &GetLibCacheManager();
  static void ReleaseLibCacheManager();
};

class IncludeToLibPreprocessor {
public:
  virtual ~IncludeToLibPreprocessor() {}

  virtual void AddIncPath(llvm::StringRef path) = 0;
  virtual HRESULT Preprocess(IDxcBlob *pSource, LPCWSTR pFilename,
                             LPCWSTR *pArguments, UINT32 argCount,
                             const DxcDefine *pDefines,
                             unsigned defineCount) = 0;

  virtual const std::vector<std::string> &GetSnippets() const = 0;
  static std::unique_ptr<IncludeToLibPreprocessor>
  CreateIncludeToLibPreprocessor(IDxcIncludeHandler *handler);
};

} // namespace libshare