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

  template <typename T> RDATTable *GetOrAddTable() {
    RDATTable **tablePtr =
        &m_pTables[(size_t)RDAT::RecordTraits<T>::TableIndex()];
    if (!*tablePtr) {
      m_Parts.emplace_back(llvm::make_unique<RDATTable>());
      *tablePtr = reinterpret_cast<RDATTable *>(m_Parts.back().get());
      (*tablePtr)->SetRecordStride(sizeof(T));
      (*tablePtr)->SetType(RDAT::RecordTraits<T>::PartType());
      (*tablePtr)->SetDeduplication(m_bRecordDeduplicationEnabled);
    }
    return *tablePtr;
  }

  template <typename T> uint32_t InsertRecord(const T &record) {
    return GetOrAddTable<T>()->Insert(record);
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

  StringBufferPart &GetStringBufferPart() {
    return *GetOrAddPart(&m_pStringBufferPart);
  }
  IndexArraysPart &GetIndexArraysPart() {
    return *GetOrAddPart(&m_pIndexArraysPart);
  }
  RawBytesPart &GetRawBytesPart() { return *GetOrAddPart(&m_pRawBytesPart); }

  struct SizeInfo {
    uint32_t sizeInBytes;
    uint32_t numParts;
  };
  SizeInfo ComputeSize() const;

  uint32_t size() const { return ComputeSize().sizeInBytes; }

  llvm::StringRef FinalizeAndGetData();
};

} // namespace hlsl
