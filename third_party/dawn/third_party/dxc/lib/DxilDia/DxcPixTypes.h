///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxcPixTypes.h                                                             //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Declares the classes implementing DxcPixType and its subinterfaces. These //
// classes are used to interpret llvm::DITypes from the debug metadata.      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dxc/Support/WinIncludes.h"

#include "DxcPixDxilDebugInfo.h"
#include "DxcPixTypes.h"

#include "dxc/Support/Global.h"
#include "dxc/Support/microcom.h"

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"

namespace dxil_debug_info {
HRESULT CreateDxcPixType(DxcPixDxilDebugInfo *ppDxilDebugInfo,
                         llvm::DIType *diType, IDxcPixType **ppResult);

class DxcPixConstType : public IDxcPixConstType {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CComPtr<DxcPixDxilDebugInfo> m_pDxilDebugInfo;
  llvm::DIDerivedType *m_pType;
  llvm::DIType *m_pBaseType;

  DxcPixConstType(IMalloc *pMalloc, DxcPixDxilDebugInfo *pDxilDebugInfo,
                  llvm::DIDerivedType *pType)
      : m_pMalloc(pMalloc), m_pDxilDebugInfo(pDxilDebugInfo), m_pType(pType) {
    const llvm::DITypeIdentifierMap EmptyMap;
    m_pBaseType = m_pType->getBaseType().resolve(EmptyMap);
  }

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcPixConstType)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcPixConstType, IDxcPixType>(this, iid,
                                                                ppvObject);
  }

  STDMETHODIMP GetName(BSTR *Name) override;

  STDMETHODIMP GetSizeInBits(DWORD *pSizeInBits) override;

  STDMETHODIMP UnAlias(IDxcPixType **ppType) override;
};

class DxcPixTypedefType : public IDxcPixTypedefType {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CComPtr<DxcPixDxilDebugInfo> m_pDxilDebugInfo;
  llvm::DIDerivedType *m_pType;
  llvm::DIType *m_pBaseType;

  DxcPixTypedefType(IMalloc *pMalloc, DxcPixDxilDebugInfo *pDxilDebugInfo,
                    llvm::DIDerivedType *pType)
      : m_pMalloc(pMalloc), m_pDxilDebugInfo(pDxilDebugInfo), m_pType(pType) {
    const llvm::DITypeIdentifierMap EmptyMap;
    m_pBaseType = m_pType->getBaseType().resolve(EmptyMap);
  }

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcPixTypedefType)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcPixTypedefType, IDxcPixType>(this, iid,
                                                                  ppvObject);
  }

  STDMETHODIMP GetName(BSTR *Name) override;

  STDMETHODIMP GetSizeInBits(DWORD *pSizeInBits) override;

  STDMETHODIMP UnAlias(IDxcPixType **ppBaseType) override;
};

class DxcPixScalarType : public IDxcPixScalarType {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CComPtr<DxcPixDxilDebugInfo> m_pDxilDebugInfo;
  llvm::DIBasicType *m_pType;

  DxcPixScalarType(IMalloc *pMalloc, DxcPixDxilDebugInfo *pDxilDebugInfo,
                   llvm::DIBasicType *pType)
      : m_pMalloc(pMalloc), m_pDxilDebugInfo(pDxilDebugInfo), m_pType(pType) {}

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcPixScalarType)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcPixScalarType, IDxcPixType>(this, iid,
                                                                 ppvObject);
  }

  STDMETHODIMP GetName(BSTR *Name) override;

  STDMETHODIMP GetSizeInBits(DWORD *pSizeInBits) override;

  STDMETHODIMP UnAlias(IDxcPixType **ppBaseType) override;
};

class DxcPixArrayType : public IDxcPixArrayType {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CComPtr<DxcPixDxilDebugInfo> m_pDxilDebugInfo;
  llvm::DICompositeType *m_pArray;
  llvm::DIType *m_pBaseType;
  unsigned m_DimNum;

  DxcPixArrayType(IMalloc *pMalloc, DxcPixDxilDebugInfo *pDxilDebugInfo,
                  llvm::DICompositeType *pArray, unsigned DimNum)
      : m_pMalloc(pMalloc), m_pDxilDebugInfo(pDxilDebugInfo), m_pArray(pArray),
        m_DimNum(DimNum) {
    const llvm::DITypeIdentifierMap EmptyMap;
    m_pBaseType = m_pArray->getBaseType().resolve(EmptyMap);

#ifndef NDEBUG
    assert(m_DimNum < m_pArray->getElements().size());

    for (auto *Dims : m_pArray->getElements()) {
      assert(llvm::isa<llvm::DISubrange>(Dims));
    }
#endif // !NDEBUG
  }

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcPixArrayType)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcPixArrayType, IDxcPixType>(this, iid,
                                                                ppvObject);
  }

  STDMETHODIMP GetName(BSTR *Name) override;

  STDMETHODIMP GetSizeInBits(DWORD *pSizeInBits) override;

  STDMETHODIMP UnAlias(IDxcPixType **ppBaseType) override;

  STDMETHODIMP GetNumElements(DWORD *ppNumElements) override;

  STDMETHODIMP GetIndexedType(IDxcPixType **ppElementType) override;

  STDMETHODIMP GetElementType(IDxcPixType **ppElementType) override;
};

class DxcPixStructType : public IDxcPixStructType2 {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CComPtr<DxcPixDxilDebugInfo> m_pDxilDebugInfo;
  llvm::DICompositeType *m_pStruct;

  DxcPixStructType(IMalloc *pMalloc, DxcPixDxilDebugInfo *pDxilDebugInfo,
                   llvm::DICompositeType *pStruct)
      : m_pMalloc(pMalloc), m_pDxilDebugInfo(pDxilDebugInfo),
        m_pStruct(pStruct) {
#ifndef NDEBUG
    for (auto *Node : m_pStruct->getElements()) {
      assert(llvm::isa<llvm::DIDerivedType>(Node) ||
             llvm::isa<llvm::DISubprogram>(Node));
    }
#endif // !NDEBUG
  }

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcPixStructType)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcPixStructType2, IDxcPixStructType,
                                 IDxcPixType>(this, iid, ppvObject);
  }

  STDMETHODIMP GetName(BSTR *Name) override;

  STDMETHODIMP GetSizeInBits(DWORD *pSizeInBits) override;

  STDMETHODIMP UnAlias(IDxcPixType **ppBaseType) override;

  STDMETHODIMP GetNumFields(DWORD *ppNumFields) override;

  STDMETHODIMP GetFieldByIndex(DWORD dwIndex,
                               IDxcPixStructField **ppField) override;

  STDMETHODIMP GetFieldByName(LPCWSTR lpName,
                              IDxcPixStructField **ppField) override;

  STDMETHODIMP GetBaseType(IDxcPixType **ppType) override;
};

struct __declspec(uuid("6c707d08-7995-4a84-bae5-e6d8291f3b78"))
    PreviousDxcPixStructField {};

class DxcPixStructField : public IDxcPixStructField {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  CComPtr<DxcPixDxilDebugInfo> m_pDxilDebugInfo;
  llvm::DIDerivedType *m_pField;
  llvm::DIType *m_pType;

  DxcPixStructField(IMalloc *pMalloc, DxcPixDxilDebugInfo *pDxilDebugInfo,
                    llvm::DIDerivedType *pField)
      : m_pMalloc(pMalloc), m_pDxilDebugInfo(pDxilDebugInfo), m_pField(pField) {
    const llvm::DITypeIdentifierMap EmptyMap;
    m_pType = m_pField->getBaseType().resolve(EmptyMap);
  }

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcPixStructField)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    if (IsEqualIID(iid, __uuidof(PreviousDxcPixStructField))) {
      *(IDxcPixStructField **)ppvObject = this;
      this->AddRef();
      return S_OK;
    }

    return DoBasicQueryInterface<IDxcPixStructField>(this, iid, ppvObject);
  }

  STDMETHODIMP GetName(BSTR *Name) override;

  STDMETHODIMP GetType(IDxcPixType **ppType) override;

  STDMETHODIMP GetOffsetInBits(DWORD *pOffsetInBits) override;

  STDMETHODIMP GetFieldSizeInBits(DWORD *pFieldSizeInBits) override;
};
} // namespace dxil_debug_info
