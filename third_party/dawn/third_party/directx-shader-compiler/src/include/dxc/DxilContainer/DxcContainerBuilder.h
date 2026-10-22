///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxcContainerBuilder.h                                                     //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements the Dxil Container Builder                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// Include Windows header early for DxilHash.h.
#include "dxc/Support/Global.h"
#include "dxc/Support/WinIncludes.h"

#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/DxilHash/DxilHash.h"
#include "dxc/Support/microcom.h"
#include "dxc/dxcapi.h"
#include "llvm/ADT/SmallVector.h"

using namespace hlsl;
namespace hlsl {
class AbstractMemoryStream;
}

class DxcContainerBuilder : public IDxcContainerBuilder {
public:
  // Loads DxilContainer to the builder
  HRESULT STDMETHODCALLTYPE Load(IDxcBlob *pDxilContainerHeader) override;
  // Add the given part with fourCC
  HRESULT STDMETHODCALLTYPE AddPart(UINT32 fourCC, IDxcBlob *pSource) override;
  // Remove the part with fourCC
  HRESULT STDMETHODCALLTYPE RemovePart(UINT32 fourCC) override;
  // Builds a container of the given container builder state
  HRESULT STDMETHODCALLTYPE
  SerializeContainer(IDxcOperationResult **ppResult) override;

  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(DxcContainerBuilder)
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcContainerBuilder>(this, riid, ppvObject);
  }

  void Init() {
    m_RequireValidation = false;
    m_HasPrivateData = false;
    m_HashFunction = nullptr;
  }

protected:
  DXC_MICROCOM_TM_REF_FIELDS()

private:
  class DxilPart {
  public:
    UINT32 m_fourCC;
    CComPtr<IDxcBlob> m_Blob;
    DxilPart(UINT32 fourCC, IDxcBlob *pSource)
        : m_fourCC(fourCC), m_Blob(pSource) {}
  };
  typedef llvm::SmallVector<DxilPart, 8> PartList;

  PartList m_parts;
  CComPtr<IDxcBlob> m_pContainer;
  bool m_RequireValidation;
  bool m_HasPrivateData;
  // Function to compute hash when valid dxil container is built
  // This is nullptr if loaded container has invalid hash
  HASH_FUNCTION_PROTO *m_HashFunction;

  void DetermineHashFunctionFromContainerContents(
      const DxilContainerHeader *ContainerHeader);
  void HashAndUpdate(DxilContainerHeader *ContainerHeader);

  UINT32 ComputeContainerSize();
  HRESULT UpdateContainerHeader(AbstractMemoryStream *pStream,
                                uint32_t containerSize);
  HRESULT UpdateOffsetTable(AbstractMemoryStream *pStream);
  HRESULT UpdateParts(AbstractMemoryStream *pStream);
  void AddPart(DxilPart &&part);
};
