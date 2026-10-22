///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// PixTestUtils.h                                                            //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides utility functions for PIX tests.                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dxc/Support/WinIncludes.h"
#include "dxc/dxcpix.h"

#include <memory>
#include <string>
#include <vector>

namespace dxc {
class DllLoader;
}

namespace pix_test {

std::vector<std::string> SplitAndPreserveEmptyLines(std::string const &str,
                                                    char delimeter);

CComPtr<IDxcBlob> GetDebugPart(dxc::DllLoader &dllSupport, IDxcBlob *container);
void CreateBlobFromText(dxc::DllLoader &dllSupport, const char *pText,
                        IDxcBlobEncoding **ppBlob);

HRESULT CreateCompiler(dxc::DllLoader &dllSupport, IDxcCompiler **ppResult);
CComPtr<IDxcBlob> Compile(dxc::DllLoader &dllSupport, const char *hlsl,
                          const wchar_t *target,
                          std::vector<const wchar_t *> extraArgs = {},
                          const wchar_t *entry = L"main");

void CompileAndLogErrors(dxc::DllLoader &dllSupport, LPCSTR pText,
                         LPCWSTR pTargetProfile, std::vector<LPCWSTR> &args,
                         IDxcIncludeHandler *includer,
                         _Outptr_ IDxcBlob **ppResult);

CComPtr<IDxcBlob> WrapInNewContainer(dxc::DllLoader &dllSupport,
                                     IDxcBlob *part);

struct ValueLocation {
  int base;
  int count;
};
struct PassOutput {
  CComPtr<IDxcBlob> blob;
  std::vector<ValueLocation> valueLocations;
  std::vector<std::string> lines;
};

PassOutput RunAnnotationPasses(dxc::DllLoader &dllSupport, IDxcBlob *dxil,
                               int startingLineNumber = 0);

struct DebuggerInterfaces {
  CComPtr<IDxcPixDxilDebugInfo> debugInfo;
  CComPtr<IDxcPixCompilationInfo> compilationInfo;
};

class InstructionOffsetSeeker {
public:
  virtual ~InstructionOffsetSeeker() {}
  virtual DWORD FindInstructionOffsetForLabel(const wchar_t *label) = 0;
};

std::unique_ptr<pix_test::InstructionOffsetSeeker>
GatherDebugLocLabelsFromDxcUtils(DebuggerInterfaces &debuggerInterfaces);

} // namespace pix_test
