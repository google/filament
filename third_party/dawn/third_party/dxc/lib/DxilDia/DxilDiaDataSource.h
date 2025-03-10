///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilDiaDataSource.cpp                                                     //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// DIA API implementation for DXIL modules.                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dxc/Support/WinIncludes.h"

#include <memory>

#include "dia2.h"

#include "dxc/DXIL/DxilModule.h"
#include "dxc/Support/Global.h"

#include "DxilDia.h"
#include "DxilDiaTable.h"

namespace dxil_dia {
class Session;

class DataSource : public IDiaDataSource {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  std::shared_ptr<llvm::Module> m_module;
  std::shared_ptr<llvm::LLVMContext> m_context;
  std::shared_ptr<llvm::DebugInfoFinder> m_finder;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()

  STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) override {
    return DoBasicQueryInterface<IDiaDataSource>(this, iid, ppvObject);
  }

  DataSource(IMalloc *pMalloc);

  ~DataSource();

  STDMETHODIMP get_lastError(BSTR *pRetVal) override;

  STDMETHODIMP loadDataFromPdb(LPCOLESTR pdbPath) override {
    return ENotImpl();
  }

  STDMETHODIMP loadAndValidateDataFromPdb(LPCOLESTR pdbPath, GUID *pcsig70,
                                          DWORD sig, DWORD age) override {
    return ENotImpl();
  }

  STDMETHODIMP loadDataForExe(LPCOLESTR executable, LPCOLESTR searchPath,
                              IUnknown *pCallback) override {
    return ENotImpl();
  }

  STDMETHODIMP loadDataFromIStream(IStream *pIStream) override;

  STDMETHODIMP openSession(IDiaSession **ppSession) override;

  HRESULT STDMETHODCALLTYPE loadDataFromCodeViewInfo(
      LPCOLESTR executable, LPCOLESTR searchPath, DWORD cbCvInfo,
      BYTE *pbCvInfo, IUnknown *pCallback) override {
    return ENotImpl();
  }

  HRESULT STDMETHODCALLTYPE loadDataFromMiscInfo(
      LPCOLESTR executable, LPCOLESTR searchPath, DWORD timeStampExe,
      DWORD timeStampDbg, DWORD sizeOfExe, DWORD cbMiscInfo, BYTE *pbMiscInfo,
      IUnknown *pCallback) override {
    return ENotImpl();
  }
};

} // namespace dxil_dia
