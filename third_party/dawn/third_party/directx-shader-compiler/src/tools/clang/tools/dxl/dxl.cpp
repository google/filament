///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxl.cpp                                                                   //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides the entry point for the dxl console program.                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxclib/dxc.h"
#include <vector>

#ifdef _WIN32
int __cdecl wmain(int argc, const wchar_t **argv_) {
  std::vector<const wchar_t *> args;
  for (int i = 0; i < argc; i++)
    args.emplace_back(argv_[i]);
  args.emplace_back(L"-link");

  return dxc::main(args.size(), args.data());
#else
int main(int argc, const char **argv_) {
  std::vector<const char *> args;
  for (int i = 0; i < argc; i++)
    args.emplace_back(argv_[i]);
  args.emplace_back("-link");
  return dxc::main(args.size(), args.data());
#endif // _WIN32
}
