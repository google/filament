///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilRDATParts.h                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dxc/DxilContainer/DxilRuntimeReflection.h"
#include "dxc/Support/WinIncludes.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace hlsl {

class RDATPart {
public:
  virtual uint32_t GetPartSize() const { return 0; }
  virtual void Write(void *ptr) {}
  virtual RDAT::RuntimeDataPartType GetType() const {
    return RDAT::RuntimeDataPartType::Invalid;
  }
  virtual ~RDATPart() {}
};

class StringBufferPart : public RDATPart {
private:
  std::unordered_map<std::string, uint32_t> m_Map;
  std::vector<llvm::StringRef> m_List;
  size_t m_Size = 0;

public:
  StringBufferPart() {
    // Always start string table with null so empty/null strings have offset of
    // zero
    Insert("");
  }
  // returns the offset of the name inserted
  uint32_t Insert(llvm::StringRef str);
  RDAT::RuntimeDataPartType GetType() const {
    return RDAT::RuntimeDataPartType::StringBuffer;
  }
  uint32_t GetPartSize() const { return m_Size; }
  void Write(void *ptr);
};

class IndexArraysPart : public RDATPart {
private:
  std::vector<uint32_t> m_IndexBuffer;

  // Use m_IndexSet with CmpIndices to avoid duplicate index arrays
  struct CmpIndices {
    const IndexArraysPart &Table;
    CmpIndices(const IndexArraysPart &table) : Table(table) {}
    bool operator()(uint32_t left, uint32_t right) const {
      const uint32_t *pLeft = Table.m_IndexBuffer.data() + left;
      const uint32_t *pRight = Table.m_IndexBuffer.data() + right;
      if (*pLeft != *pRight)
        return (*pLeft < *pRight);
      uint32_t count = *pLeft;
      for (unsigned i = 0; i < count; i++) {
        ++pLeft;
        ++pRight;
        if (*pLeft != *pRight)
          return (*pLeft < *pRight);
      }
      return false;
    }
  };
  std::set<uint32_t, CmpIndices> m_IndexSet;

public:
  IndexArraysPart() : m_IndexBuffer(), m_IndexSet(*this) {}
  template <class iterator> uint32_t AddIndex(iterator begin, iterator end) {
    uint32_t newOffset = m_IndexBuffer.size();
    m_IndexBuffer.push_back(0); // Size: update after insertion
    m_IndexBuffer.insert(m_IndexBuffer.end(), begin, end);
    m_IndexBuffer[newOffset] = (m_IndexBuffer.size() - newOffset) - 1;
    // Check for duplicate, return new offset if not duplicate
    auto insertResult = m_IndexSet.insert(newOffset);
    if (insertResult.second)
      return newOffset;
    // Otherwise it was a duplicate, so chop off the size and return the
    // original
    m_IndexBuffer.resize(newOffset);
    return *insertResult.first;
  }

  RDAT::RuntimeDataPartType GetType() const {
    return RDAT::RuntimeDataPartType::IndexArrays;
  }
  uint32_t GetPartSize() const {
    return sizeof(uint32_t) * m_IndexBuffer.size();
  }

  void Write(void *ptr) {
    memcpy(ptr, m_IndexBuffer.data(), m_IndexBuffer.size() * sizeof(uint32_t));
  }
};

class RawBytesPart : public RDATPart {
private:
  std::unordered_map<std::string, uint32_t> m_Map;
  std::vector<llvm::StringRef> m_List;
  size_t m_Size = 0;

public:
  RawBytesPart() {}
  uint32_t Insert(const void *pData, size_t dataSize);
  RDAT::BytesRef InsertBytesRef(const void *pData, size_t dataSize) {
    RDAT::BytesRef ret = {};
    ret.Offset = Insert(pData, dataSize);
    ret.Size = dataSize;
    return ret;
  }

  RDAT::RuntimeDataPartType GetType() const {
    return RDAT::RuntimeDataPartType::RawBytes;
  }
  uint32_t GetPartSize() const { return m_Size; }
  void Write(void *ptr);
};

class RDATTable : public RDATPart {
protected:
  // m_map is map of records to their index.
  // Used to alias identical records.
  std::unordered_map<std::string, uint32_t> m_map;
  std::vector<llvm::StringRef> m_rows;
  size_t m_RecordStride = 0;
  bool m_bDeduplicationEnabled = false;
  RDAT::RuntimeDataPartType m_Type = RDAT::RuntimeDataPartType::Invalid;
  uint32_t InsertImpl(const void *ptr, size_t size);

public:
  virtual ~RDATTable() {}

  void SetType(RDAT::RuntimeDataPartType type) { m_Type = type; }
  RDAT::RuntimeDataPartType GetType() const { return m_Type; }
  void SetRecordStride(size_t RecordStride);
  size_t GetRecordStride() const { return m_RecordStride; }
  void SetDeduplication(bool bEnabled = true) {
    m_bDeduplicationEnabled = bEnabled;
  }

  uint32_t Count() {
    size_t count = m_rows.size();
    return (count < UINT32_MAX) ? count : 0;
  }

  template <typename RecordType> uint32_t Insert(const RecordType &data) {
    return InsertImpl(&data, sizeof(RecordType));
  }

  void Write(void *ptr);
  uint32_t GetPartSize() const;
};

} // namespace hlsl
