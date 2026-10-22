///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxc.h                                                                     //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides wrappers to dxc main function.                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef __DXC_DXCLIB__
#define __DXC_DXCLIB__

namespace llvm {
class raw_ostream;
}

namespace dxc {
class DllLoader;

// Writes compiler version info to stream
void WriteDxCompilerVersionInfo(llvm::raw_ostream &OS, const char *ExternalLib,
                                const char *ExternalFn, DllLoader &DxcSupport);
void WriteDXILVersionInfo(llvm::raw_ostream &OS, DllLoader &DxilSupport);

#ifdef _WIN32
int main(int argc, const wchar_t **argv_);
#else
int main(int argc, const char **argv_);
#endif // _WIN32
} // namespace dxc

#endif // __DXC_DXCLIB__