///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxcPixDxilStorage.cpp                                                     //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Defines the DxcPixDxilStorage API.                                        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "DxcPixDxilStorage.h"

#include "dxc/DxilPIXPasses/DxilPIXVirtualRegisters.h"
#include "dxc/Support/WinIncludes.h"
#include "llvm/IR/Instructions.h"

#include "DxcPixBase.h"
#include "DxcPixLiveVariables.h"
#include "DxcPixTypes.h"
#include "DxilDiaSession.h"

static HRESULT UnAliasType(IDxcPixType *MaybeAlias,
                           IDxcPixType **OriginalType) {
  CComPtr<IDxcPixType> Tmp(MaybeAlias);
  HRESULT hr = E_FAIL;

  *OriginalType = nullptr;
  do {
    CComPtr<IDxcPixType> Other;

    hr = Tmp->UnAlias(&Other);
    IFR(hr);
    if (hr == S_FALSE) {
      break;
    }

    Tmp = Other;
  } while (true);

  *OriginalType = Tmp.Detach();
  return S_OK;
}

HRESULT CreateDxcPixStorageImpl(
    dxil_debug_info::DxcPixDxilDebugInfo *pDxilDebugInfo,
    IDxcPixType *OriginalType, const dxil_debug_info::VariableInfo *VarInfo,
    unsigned CurrentOffsetInBits, IDxcPixDxilStorage **ppStorage) {
  CComPtr<IDxcPixArrayType> ArrayTy;
  CComPtr<IDxcPixStructType2> StructTy;
  CComPtr<IDxcPixScalarType> ScalarTy;

  CComPtr<IDxcPixType> UnalisedType;
  IFR(UnAliasType(OriginalType, &UnalisedType));

  if (UnalisedType->QueryInterface(&ArrayTy) == S_OK) {
    return dxil_debug_info::NewDxcPixDxilDebugInfoObjectOrThrow<
        dxil_debug_info::DxcPixDxilArrayStorage>(
        ppStorage, pDxilDebugInfo->GetMallocNoRef(), pDxilDebugInfo,
        OriginalType, ArrayTy, VarInfo, CurrentOffsetInBits);
  } else if (UnalisedType->QueryInterface(&StructTy) == S_OK) {
    return dxil_debug_info::NewDxcPixDxilDebugInfoObjectOrThrow<
        dxil_debug_info::DxcPixDxilStructStorage>(
        ppStorage, pDxilDebugInfo->GetMallocNoRef(), pDxilDebugInfo,
        OriginalType, StructTy, VarInfo, CurrentOffsetInBits);
  } else if (UnalisedType->QueryInterface(&ScalarTy) == S_OK) {
    return dxil_debug_info::NewDxcPixDxilDebugInfoObjectOrThrow<
        dxil_debug_info::DxcPixDxilScalarStorage>(
        ppStorage, pDxilDebugInfo->GetMallocNoRef(), pDxilDebugInfo,
        OriginalType, ScalarTy, VarInfo, CurrentOffsetInBits);
  }

  return E_UNEXPECTED;
}

STDMETHODIMP dxil_debug_info::DxcPixDxilArrayStorage::AccessField(
    LPCWSTR Name, IDxcPixDxilStorage **ppResult) {
  return E_FAIL;
}

STDMETHODIMP
dxil_debug_info::DxcPixDxilArrayStorage::Index(DWORD Index,
                                               IDxcPixDxilStorage **ppResult) {
  CComPtr<IDxcPixType> IndexedType;
  IFR(m_pType->GetIndexedType(&IndexedType));

  DWORD NumElements;
  IFR(m_pType->GetNumElements(&NumElements));

  if (Index >= NumElements) {
    return E_BOUNDS;
  }

  DWORD IndexedTypeSizeInBits;
  IFR(IndexedType->GetSizeInBits(&IndexedTypeSizeInBits));

  const unsigned NewOffsetInBits =
      m_OffsetFromStorageStartInBits + Index * IndexedTypeSizeInBits;

  return CreateDxcPixStorageImpl(m_pDxilDebugInfo, IndexedType, m_pVarInfo,
                                 NewOffsetInBits, ppResult);
}

STDMETHODIMP dxil_debug_info::DxcPixDxilArrayStorage::GetRegisterNumber(
    DWORD *pRegisterNumber) {
  return E_FAIL;
}

STDMETHODIMP dxil_debug_info::DxcPixDxilArrayStorage::GetIsAlive() {
  for (auto OffsetAndRegister : m_pVarInfo->m_ValueLocationMap) {
    if (OffsetAndRegister.second.m_V != nullptr) {
      return S_OK;
    }
  }
  return E_FAIL;
}

STDMETHODIMP
dxil_debug_info::DxcPixDxilArrayStorage::GetType(IDxcPixType **ppType) {
  *ppType = m_pOriginalType;
  (*ppType)->AddRef();
  return S_OK;
}

STDMETHODIMP dxil_debug_info::DxcPixDxilStructStorage::AccessField(
    LPCWSTR Name, IDxcPixDxilStorage **ppResult) {
  DWORD FieldOffsetInBits = 0;
  CComPtr<IDxcPixType> FieldType;
  if (*Name != 0) {
    CComPtr<IDxcPixStructField> Field;
    IFR(m_pType->GetFieldByName(Name, &Field));

    IFR(Field->GetType(&FieldType));

    IFR(Field->GetOffsetInBits(&FieldOffsetInBits));
  } else {
    IFR(m_pType->GetBaseType(&FieldType));
  }

  const unsigned NewOffsetInBits =
      m_OffsetFromStorageStartInBits + FieldOffsetInBits;

  return CreateDxcPixStorageImpl(m_pDxilDebugInfo, FieldType, m_pVarInfo,
                                 NewOffsetInBits, ppResult);
}

STDMETHODIMP
dxil_debug_info::DxcPixDxilStructStorage::Index(DWORD Index,
                                                IDxcPixDxilStorage **ppResult) {
  return E_FAIL;
}

STDMETHODIMP dxil_debug_info::DxcPixDxilStructStorage::GetRegisterNumber(
    DWORD *pRegisterNumber) {
  return E_FAIL;
}

STDMETHODIMP dxil_debug_info::DxcPixDxilStructStorage::GetIsAlive() {
  for (auto OffsetAndRegister : m_pVarInfo->m_ValueLocationMap) {
    if (OffsetAndRegister.second.m_V != nullptr) {
      return S_OK;
    }
  }
  return E_FAIL;
}

STDMETHODIMP
dxil_debug_info::DxcPixDxilStructStorage::GetType(IDxcPixType **ppType) {
  *ppType = m_pOriginalType;
  (*ppType)->AddRef();
  return S_OK;
}

STDMETHODIMP dxil_debug_info::DxcPixDxilScalarStorage::AccessField(
    LPCWSTR Name, IDxcPixDxilStorage **ppResult) {
  return E_FAIL;
}

STDMETHODIMP
dxil_debug_info::DxcPixDxilScalarStorage::Index(DWORD Index,
                                                IDxcPixDxilStorage **ppResult) {
  return E_FAIL;
}

STDMETHODIMP dxil_debug_info::DxcPixDxilScalarStorage::GetRegisterNumber(
    DWORD *pRegisterNumber) {
  const auto &ValueLocationMap = m_pVarInfo->m_ValueLocationMap;
  // Bitfields will have been packed into their containing integer type:
  DWORD size;
  m_pOriginalType->GetSizeInBits(&size);
  auto RegIt =
      ValueLocationMap.find(m_OffsetFromStorageStartInBits & ~(size - 1));

  if (RegIt == ValueLocationMap.end()) {
    return E_FAIL;
  }

  if (auto *AllocaReg = llvm::dyn_cast<llvm::AllocaInst>(RegIt->second.m_V)) {
    uint32_t RegNum;
    uint32_t RegSize;
    if (!pix_dxil::PixAllocaReg::FromInst(AllocaReg, &RegNum, &RegSize)) {
      return E_FAIL;
    }

    *pRegisterNumber = RegNum + RegIt->second.m_FragmentIndex;
  } else {
    return E_FAIL;
  }

  return S_OK;
}

STDMETHODIMP dxil_debug_info::DxcPixDxilScalarStorage::GetIsAlive() {
  const auto &ValueLocationMap = m_pVarInfo->m_ValueLocationMap;
  auto RegIt = ValueLocationMap.find(m_OffsetFromStorageStartInBits);

  return RegIt == ValueLocationMap.end() ? E_FAIL : S_OK;
}

STDMETHODIMP
dxil_debug_info::DxcPixDxilScalarStorage::GetType(IDxcPixType **ppType) {
  *ppType = m_pOriginalType;
  (*ppType)->AddRef();
  return S_OK;
}

HRESULT dxil_debug_info::CreateDxcPixStorage(
    DxcPixDxilDebugInfo *pDxilDebugInfo, llvm::DIType *diType,
    const VariableInfo *VarInfo, unsigned CurrentOffsetInBits,
    IDxcPixDxilStorage **ppStorage) {
  CComPtr<IDxcPixType> OriginalType;
  IFR(dxil_debug_info::CreateDxcPixType(pDxilDebugInfo, diType, &OriginalType));

  return CreateDxcPixStorageImpl(pDxilDebugInfo, OriginalType, VarInfo,
                                 CurrentOffsetInBits, ppStorage);
}
