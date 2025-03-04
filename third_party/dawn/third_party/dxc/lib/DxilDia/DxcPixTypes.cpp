///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxcPixTypes.cpp                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Defines the implementation for the DxcPixType -- and its subinterfaces.   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "DxcPixTypes.h"
#include "DxcPixBase.h"
#include "DxilDiaSession.h"

static const char *GetTypeNameOrDefault(llvm::DIType *diType) {
  auto stringRef = diType->getName();
  if (stringRef.empty())
    return "<unnamed>";
  return stringRef.data();
}

HRESULT dxil_debug_info::CreateDxcPixType(DxcPixDxilDebugInfo *pDxilDebugInfo,
                                          llvm::DIType *diType,
                                          IDxcPixType **ppResult) {
  if (auto *BT = llvm::dyn_cast<llvm::DIBasicType>(diType)) {
    return NewDxcPixDxilDebugInfoObjectOrThrow<DxcPixScalarType>(
        ppResult, pDxilDebugInfo->GetMallocNoRef(), pDxilDebugInfo, BT);
  } else if (auto *CT = llvm::dyn_cast<llvm::DICompositeType>(diType)) {
    switch (CT->getTag()) {
    default:
      break;

    case llvm::dwarf::DW_TAG_array_type: {
      const unsigned FirstDim = 0;
      return NewDxcPixDxilDebugInfoObjectOrThrow<DxcPixArrayType>(
          ppResult, pDxilDebugInfo->GetMallocNoRef(), pDxilDebugInfo, CT,
          FirstDim);
    }

    case llvm::dwarf::DW_TAG_class_type:
    case llvm::dwarf::DW_TAG_structure_type:
      return NewDxcPixDxilDebugInfoObjectOrThrow<DxcPixStructType>(
          ppResult, pDxilDebugInfo->GetMallocNoRef(), pDxilDebugInfo, CT);
    }
  } else if (auto *DT = llvm::dyn_cast<llvm::DIDerivedType>(diType)) {
    switch (DT->getTag()) {
    default:
      break;

    case llvm::dwarf::DW_TAG_const_type:
      return NewDxcPixDxilDebugInfoObjectOrThrow<DxcPixConstType>(
          ppResult, pDxilDebugInfo->GetMallocNoRef(), pDxilDebugInfo, DT);

    case llvm::dwarf::DW_TAG_typedef:
      return NewDxcPixDxilDebugInfoObjectOrThrow<DxcPixTypedefType>(
          ppResult, pDxilDebugInfo->GetMallocNoRef(), pDxilDebugInfo, DT);
    }
  }

  return E_UNEXPECTED;
}

STDMETHODIMP dxil_debug_info::DxcPixConstType::GetName(BSTR *Name) {
  CComPtr<IDxcPixType> BaseType;
  IFR(UnAlias(&BaseType));

  CComBSTR BaseName;
  IFR(BaseType->GetName(&BaseName));

  *Name = CComBSTR((L"const " + std::wstring(BaseName)).c_str()).Detach();
  return S_OK;
}

STDMETHODIMP dxil_debug_info::DxcPixConstType::GetSizeInBits(DWORD *pSize) {
  CComPtr<IDxcPixType> BaseType;
  IFR(UnAlias(&BaseType));

  return BaseType->GetSizeInBits(pSize);
}

STDMETHODIMP dxil_debug_info::DxcPixConstType::UnAlias(IDxcPixType **ppType) {
  return CreateDxcPixType(m_pDxilDebugInfo, m_pBaseType, ppType);
}

STDMETHODIMP dxil_debug_info::DxcPixTypedefType::GetName(BSTR *Name) {
  *Name = CComBSTR(CA2W(GetTypeNameOrDefault(m_pType))).Detach();
  return S_OK;
}

STDMETHODIMP dxil_debug_info::DxcPixTypedefType::GetSizeInBits(DWORD *pSize) {
  CComPtr<IDxcPixType> BaseType;
  IFR(UnAlias(&BaseType));

  return BaseType->GetSizeInBits(pSize);
}

STDMETHODIMP dxil_debug_info::DxcPixTypedefType::UnAlias(IDxcPixType **ppType) {
  return CreateDxcPixType(m_pDxilDebugInfo, m_pBaseType, ppType);
}

STDMETHODIMP dxil_debug_info::DxcPixScalarType::GetName(BSTR *Name) {
  *Name = CComBSTR(CA2W(GetTypeNameOrDefault(m_pType))).Detach();
  return S_OK;
}

STDMETHODIMP
dxil_debug_info::DxcPixScalarType::GetSizeInBits(DWORD *pSizeInBits) {
  *pSizeInBits = m_pType->getSizeInBits();
  return S_OK;
}

STDMETHODIMP dxil_debug_info::DxcPixScalarType::UnAlias(IDxcPixType **ppType) {
  *ppType = this;
  this->AddRef();
  return S_FALSE;
}

STDMETHODIMP dxil_debug_info::DxcPixArrayType::GetName(BSTR *Name) {
  CComBSTR name(CA2W(GetTypeNameOrDefault(m_pBaseType)));
  name.Append(L"[]");
  *Name = name.Detach();
  return S_OK;
}

STDMETHODIMP
dxil_debug_info::DxcPixArrayType::GetSizeInBits(DWORD *pSizeInBits) {
  *pSizeInBits = m_pArray->getSizeInBits();
  for (unsigned ContainerDims = 0; ContainerDims < m_DimNum; ++ContainerDims) {
    auto *SR = llvm::dyn_cast<llvm::DISubrange>(
        m_pArray->getElements()[ContainerDims]);
    auto count = SR->getCount();
    if (count == 0) {
      return E_FAIL;
    }
    *pSizeInBits /= count;
  }
  return S_OK;
}

STDMETHODIMP dxil_debug_info::DxcPixArrayType::UnAlias(IDxcPixType **ppType) {
  *ppType = this;
  this->AddRef();
  return S_FALSE;
}

STDMETHODIMP
dxil_debug_info::DxcPixArrayType::GetNumElements(DWORD *ppNumElements) {
  auto *SR =
      llvm::dyn_cast<llvm::DISubrange>(m_pArray->getElements()[m_DimNum]);
  if (SR == nullptr) {
    return E_FAIL;
  }

  *ppNumElements = SR->getCount();
  return S_OK;
}

STDMETHODIMP dxil_debug_info::DxcPixArrayType::GetIndexedType(
    IDxcPixType **ppIndexedElement) {
  assert(1 + m_DimNum <= m_pArray->getElements().size());
  if (1 + m_DimNum == m_pArray->getElements().size()) {
    return CreateDxcPixType(m_pDxilDebugInfo, m_pBaseType, ppIndexedElement);
  }

  return NewDxcPixDxilDebugInfoObjectOrThrow<DxcPixArrayType>(
      ppIndexedElement, m_pMalloc, m_pDxilDebugInfo, m_pArray, 1 + m_DimNum);
}

STDMETHODIMP
dxil_debug_info::DxcPixArrayType::GetElementType(IDxcPixType **ppElementType) {
  return CreateDxcPixType(m_pDxilDebugInfo, m_pBaseType, ppElementType);
}

STDMETHODIMP dxil_debug_info::DxcPixStructType::GetName(BSTR *Name) {
  *Name = CComBSTR(CA2W(GetTypeNameOrDefault(m_pStruct))).Detach();
  return S_OK;
}

STDMETHODIMP
dxil_debug_info::DxcPixStructType::GetSizeInBits(DWORD *pSizeInBits) {
  *pSizeInBits = m_pStruct->getSizeInBits();
  return S_OK;
}

STDMETHODIMP dxil_debug_info::DxcPixStructType::UnAlias(IDxcPixType **ppType) {
  *ppType = this;
  this->AddRef();
  return S_FALSE;
}

STDMETHODIMP
dxil_debug_info::DxcPixStructType::GetNumFields(DWORD *ppNumFields) {
  *ppNumFields = 0;
  // DWARF lists the ancestor class, if any, and member fns
  // as a member element. Don't count those as data members:
  for (auto *Node : m_pStruct->getElements()) {
    if (Node->getTag() != llvm::dwarf::DW_TAG_inheritance &&
        Node->getTag() != llvm::dwarf::DW_TAG_subprogram) {
      (*ppNumFields)++;
    }
  }
  return S_OK;
}

STDMETHODIMP dxil_debug_info::DxcPixStructType::GetFieldByIndex(
    DWORD dwIndex, IDxcPixStructField **ppField) {
  *ppField = nullptr;

  // DWARF lists the ancestor class, if any, and member fns
  // as a member element. Skip such fields when enumerating
  // by index.

  DWORD ElementIndex = 0;
  DWORD ElementSkipCount = 0;
  for (auto *Node : m_pStruct->getElements()) {
    if (Node->getTag() == llvm::dwarf::DW_TAG_inheritance ||
        Node->getTag() == llvm::dwarf::DW_TAG_subprogram) {
      ElementSkipCount++;
    } else {
      if (dwIndex + ElementSkipCount == ElementIndex) {
        auto *pDIField = llvm::dyn_cast<llvm::DIDerivedType>(
            m_pStruct->getElements()[ElementIndex]);
        if (pDIField == nullptr) {
          return E_FAIL;
        }

        return NewDxcPixDxilDebugInfoObjectOrThrow<DxcPixStructField>(
            ppField, m_pMalloc, m_pDxilDebugInfo, pDIField);
      }
    }
    ElementIndex++;
  }
  return E_BOUNDS;
}

STDMETHODIMP dxil_debug_info::DxcPixStructType::GetFieldByName(
    LPCWSTR lpName, IDxcPixStructField **ppField) {
  std::string name = std::string(CW2A(lpName));
  for (auto *Node : m_pStruct->getElements()) {
    auto *pDIField = llvm::dyn_cast<llvm::DIDerivedType>(Node);
    if (pDIField == nullptr) {
      return E_FAIL;
    }
    if (pDIField->getTag() == llvm::dwarf::DW_TAG_inheritance) {
      continue;
    } else {
      if (name == pDIField->getName()) {
        return NewDxcPixDxilDebugInfoObjectOrThrow<DxcPixStructField>(
            ppField, m_pMalloc, m_pDxilDebugInfo, pDIField);
      }
    }
  }

  return E_BOUNDS;
}

STDMETHODIMP
dxil_debug_info::DxcPixStructType::GetBaseType(IDxcPixType **ppType) {
  for (auto *Node : m_pStruct->getElements()) {
    auto *pDIField = llvm::dyn_cast<llvm::DIDerivedType>(Node);
    if (pDIField != nullptr) {
      if (pDIField->getTag() == llvm::dwarf::DW_TAG_inheritance) {
        const llvm::DITypeIdentifierMap EmptyMap;
        auto baseType = pDIField->getBaseType().resolve(EmptyMap);
        if (auto *CompositeType =
                llvm::dyn_cast<llvm::DICompositeType>(baseType)) {
          return NewDxcPixDxilDebugInfoObjectOrThrow<DxcPixStructType>(
              ppType, m_pMalloc, m_pDxilDebugInfo, CompositeType);
        } else {
          return E_NOINTERFACE;
        }
      }
    }
  }

  return E_NOINTERFACE;
}

STDMETHODIMP dxil_debug_info::DxcPixStructField::GetName(BSTR *Name) {
  *Name = CComBSTR(CA2W(GetTypeNameOrDefault(m_pField))).Detach();
  return S_OK;
}

STDMETHODIMP dxil_debug_info::DxcPixStructField::GetType(IDxcPixType **ppType) {
  return CreateDxcPixType(m_pDxilDebugInfo, m_pType, ppType);
}

STDMETHODIMP
dxil_debug_info::DxcPixStructField::GetOffsetInBits(DWORD *pOffsetInBits) {
  *pOffsetInBits = m_pField->getOffsetInBits();
  return S_OK;
}

STDMETHODIMP
dxil_debug_info::DxcPixStructField::GetFieldSizeInBits(
    DWORD *pFieldSizeInBits) {
  *pFieldSizeInBits = m_pField->getSizeInBits();
  return S_OK;
}
