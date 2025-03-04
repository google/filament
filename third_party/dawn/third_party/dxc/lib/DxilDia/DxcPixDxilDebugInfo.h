///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxcPixDxilDebugInfo.h                                                     //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Declares the main class for dxcompiler's API for PIX support.             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dxc/Support/WinIncludes.h"
#include "dxc/dxcapi.h"
#include "dxc/dxcpix.h"

#include "dxc/Support/Global.h"
#include "dxc/Support/microcom.h"

#include <memory>
#include <vector>

namespace dxil_dia {
class Session;
} // namespace dxil_dia

namespace llvm {
class Instruction;
class Module;
} // namespace llvm

namespace dxil_debug_info {
class LiveVariables;

class DxcPixDxilDebugInfo : public IDxcPixDxilDebugInfo {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CComPtr<dxil_dia::Session> m_pSession;
  std::unique_ptr<LiveVariables> m_LiveVars;

  DxcPixDxilDebugInfo(IMalloc *pMalloc, dxil_dia::Session *pSession);

  llvm::Instruction *FindInstruction(DWORD InstructionOffset) const;

public:
  ~DxcPixDxilDebugInfo();

  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcPixDxilDebugInfo)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcPixDxilDebugInfo>(this, iid, ppvObject);
  }

  STDMETHODIMP
  GetLiveVariablesAt(DWORD InstructionOffset,
                     IDxcPixDxilLiveVariables **ppLiveVariables) override;

  STDMETHODIMP IsVariableInRegister(DWORD InstructionOffset,
                                    const wchar_t *VariableName) override;

  STDMETHODIMP GetFunctionName(DWORD InstructionOffset,
                               BSTR *ppFunctionName) override;

  STDMETHODIMP GetStackDepth(DWORD InstructionOffset,
                             DWORD *StackDepth) override;

  STDMETHODIMP InstructionOffsetsFromSourceLocation(
      const wchar_t *FileName, DWORD SourceLine, DWORD SourceColumn,
      IDxcPixDxilInstructionOffsets **ppOffsets) override;

  STDMETHODIMP SourceLocationsFromInstructionOffset(
      DWORD InstructionOffset,
      IDxcPixDxilSourceLocations **ppSourceLocations) override;

  llvm::Module *GetModuleRef();

  IMalloc *GetMallocNoRef() { return m_pMalloc; }
};

class DxcPixDxilInstructionOffsets : public IDxcPixDxilInstructionOffsets {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CComPtr<dxil_dia::Session> m_pSession;

  DxcPixDxilInstructionOffsets(IMalloc *pMalloc, dxil_dia::Session *pSession,
                               const wchar_t *FileName, DWORD SourceLine,
                               DWORD SourceColumn);

  std::vector<DWORD> m_offsets;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcPixDxilInstructionOffsets)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcPixDxilInstructionOffsets>(this, iid,
                                                                ppvObject);
  }

  virtual STDMETHODIMP_(DWORD) GetCount() override;
  virtual STDMETHODIMP_(DWORD) GetOffsetByIndex(DWORD Index) override;
};

class DxcPixDxilSourceLocations : public IDxcPixDxilSourceLocations {
private:
  DXC_MICROCOM_TM_REF_FIELDS()

  DxcPixDxilSourceLocations(IMalloc *pMalloc, dxil_dia::Session *pSession,
                            llvm::Instruction *IP);

  struct Location {
    CComBSTR Filename;
    DWORD Line;
    DWORD Column;
  };
  std::vector<Location> m_locations;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcPixDxilSourceLocations)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcPixDxilSourceLocations>(this, iid,
                                                             ppvObject);
  }

  virtual STDMETHODIMP_(DWORD) GetCount() override;
  virtual STDMETHODIMP_(DWORD) GetLineNumberByIndex(DWORD Index) override;
  virtual STDMETHODIMP_(DWORD) GetColumnByIndex(DWORD Index) override;
  virtual STDMETHODIMP GetFileNameByIndex(DWORD Index, BSTR *Name) override;
};

} // namespace dxil_debug_info
