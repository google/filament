///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilRDATBuilder.h                                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Helper type to build the RDAT data format.                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DxilRDATParts.h"

#include <vector>

namespace hlsl {

// Like DXIL container, RDAT itself is a mini container that contains multiple
// RDAT parts
class DxilRDATBuilder {
  llvm::SmallVector<char, 1024> m_RDATBuffer;
  std::vector<std::unique_ptr<RDATPart>> m_Parts;

  StringBufferPart *m_pStringBufferPart = nullptr;
  IndexArraysPart *m_pIndexArraysPart = nullptr;
  RawBytesPart *m_pRawBytesPart = nullptr;
  RDATTable *m_pTables[(size_t)RDAT::RecordTableIndex::RecordTableCount] = {};

  bool m_bRecordDeduplicationEnabled = true;

  template <typename T> T *GetOrAddPart(T **ptrStorage) {
    if (!*ptrStorage) {
      m_Parts.emplace_back(llvm::make_unique<T>());
      *ptrStorage = reinterpret_cast<T *>(m_Parts.back().get());
    }
    return *ptrStorage;
  }

public:
  DxilRDATBuilder(bool allowRecordDuplication);

  // Add all supported parts first to control table ordering and prevent use of
  // unsupported parts.

  StringBufferPart *AddStringBufferPart() {
    assert(nullptr == m_pStringBufferPart &&
           "otherwise, string buffer part already exists");
    return GetOrAddPart(&m_pStringBufferPart);
  }
  IndexArraysPart *AddIndexArraysPart() {
    assert(nullptr == m_pIndexArraysPart &&
           "otherwise, index arrays part already exists");
    return GetOrAddPart(&m_pIndexArraysPart);
  }
  RawBytesPart *AddRawBytesPart() {
    assert(nullptr == m_pRawBytesPart &&
           "otherwise, raw bytes part already exists");
    return GetOrAddPart(&m_pRawBytesPart);
  }

  template <typename T> RDATTable *AddTable() {
    RDATTable **TablePtr =
        &m_pTables[(size_t)RDAT::RecordTraits<T>::TableIndex()];
    assert(nullptr == *TablePtr && "otherwise, table already exists");
    RDATTable *Table = GetOrAddPart(TablePtr);
    Table->SetRecordStride(sizeof(T));
    Table->SetType(RDAT::RecordTraits<T>::PartType());
    Table->SetDeduplication(m_bRecordDeduplicationEnabled);
    return Table;
  }

  StringBufferPart &GetStringBufferPart() {
    assert(m_pStringBufferPart);
    return *m_pStringBufferPart;
  }
  IndexArraysPart &GetIndexArraysPart() {
    assert(m_pIndexArraysPart);
    return *m_pIndexArraysPart;
  }
  RawBytesPart &GetRawBytesPart() {
    assert(m_pRawBytesPart);
    return *m_pRawBytesPart;
  }

  template <typename T> RDATTable *GetTable() {
    return m_pTables[(size_t)RDAT::RecordTraits<T>::TableIndex()];
  }

  template <typename T> uint32_t InsertRecord(const T &record) {
    RDATTable *Table = GetTable<T>();
    assert(Table && "otherwise, missing version check");
    return Table->Insert(record);
  }
  uint32_t InsertString(llvm::StringRef str) {
    return GetStringBufferPart().Insert(str);
  }
  hlsl::RDAT::BytesRef InsertBytesRef(const void *ptr, size_t size) {
    return GetRawBytesPart().InsertBytesRef(ptr, size);
  }
  hlsl::RDAT::BytesRef InsertBytesRef(llvm::StringRef data) {
    return GetRawBytesPart().InsertBytesRef(data.data(), data.size());
  }
  template <typename T> uint32_t InsertArray(T begin, T end) {
    return GetIndexArraysPart().AddIndex(begin, end);
  }
  template <typename T> uint32_t InsertArray(T arr) {
    return InsertArray(arr.begin(), arr.end());
  }

  struct SizeInfo {
    uint32_t sizeInBytes;
    uint32_t numParts;
  };
  SizeInfo ComputeSize() const;

  uint32_t size() const { return ComputeSize().sizeInBytes; }

  llvm::StringRef FinalizeAndGetData();
};

} // namespace hlsl
