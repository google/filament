///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilRuntimeReflection.inl                                                 //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Defines shader reflection for runtime usage.                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DxilContainer/DxilRuntimeReflection.h"
#include <cwchar>
#include <memory>
#include <unordered_map>
#include <vector>

namespace hlsl {
namespace RDAT {

#define DEF_RDAT_TYPES DEF_RDAT_READER_IMPL
#include "dxc/DxilContainer/RDAT_Macros.inl"

struct ResourceKey {
  uint32_t Class, ID;
  ResourceKey(uint32_t Class, uint32_t ID) : Class(Class), ID(ID) {}
  bool operator==(const ResourceKey &other) const {
    return other.Class == Class && other.ID == ID;
  }
};

// Size-checked reader
//  on overrun: throw buffer_overrun{};
//  on overlap: throw buffer_overlap{};
class CheckedReader {
  const char *Ptr;
  size_t Size;
  size_t Offset;

public:
  class exception : public std::exception {};
  class buffer_overrun : public exception {
  public:
    buffer_overrun() noexcept {}
    virtual const char *what() const noexcept override {
      return ("buffer_overrun");
    }
  };
  class buffer_overlap : public exception {
  public:
    buffer_overlap() noexcept {}
    virtual const char *what() const noexcept override {
      return ("buffer_overlap");
    }
  };

  CheckedReader(const void *ptr, size_t size)
      : Ptr(reinterpret_cast<const char *>(ptr)), Size(size), Offset(0) {}
  void Reset(size_t offset = 0) {
    if (offset >= Size)
      throw buffer_overrun{};
    Offset = offset;
  }
  // offset is absolute, ensure offset is >= current offset
  void Advance(size_t offset = 0) {
    if (offset < Offset)
      throw buffer_overlap{};
    if (offset >= Size)
      throw buffer_overrun{};
    Offset = offset;
  }
  void CheckBounds(size_t size) const {
    assert(Offset <= Size && "otherwise, offset larger than size");
    if (size > Size - Offset)
      throw buffer_overrun{};
  }
  template <typename T> const T *Cast(size_t size = 0) {
    if (0 == size)
      size = sizeof(T);
    CheckBounds(size);
    return reinterpret_cast<const T *>(Ptr + Offset);
  }
  template <typename T> const T &Read() {
    const size_t size = sizeof(T);
    const T *p = Cast<T>(size);
    Offset += size;
    return *p;
  }
  template <typename T> const T *ReadArray(size_t count = 1) {
    const size_t size = sizeof(T) * count;
    const T *p = Cast<T>(size);
    Offset += size;
    return p;
  }
};

DxilRuntimeData::DxilRuntimeData() : DxilRuntimeData(nullptr, 0) {}

DxilRuntimeData::DxilRuntimeData(const void *ptr, size_t size) {
  InitFromRDAT(ptr, size);
}

static void InitTable(RDATContext &ctx, CheckedReader &PR,
                      RecordTableIndex tableIndex) {
  RuntimeDataTableHeader table = PR.Read<RuntimeDataTableHeader>();
  size_t tableSize = table.RecordCount * table.RecordStride;
  ctx.Table(tableIndex)
      .Init(PR.ReadArray<char>(tableSize), table.RecordCount,
            table.RecordStride);
}

// initializing reader from RDAT. return true if no error has occured.
bool DxilRuntimeData::InitFromRDAT(const void *pRDAT, size_t size) {
  if (pRDAT) {
    m_DataSize = size;
    try {
      CheckedReader Reader(pRDAT, size);
      RuntimeDataHeader RDATHeader = Reader.Read<RuntimeDataHeader>();
      if (RDATHeader.Version < RDAT_Version_10) {
        return false;
      }
      const uint32_t *offsets =
          Reader.ReadArray<uint32_t>(RDATHeader.PartCount);
      for (uint32_t i = 0; i < RDATHeader.PartCount; ++i) {
        Reader.Advance(offsets[i]);
        RuntimeDataPartHeader part = Reader.Read<RuntimeDataPartHeader>();
        CheckedReader PR(Reader.ReadArray<char>(part.Size), part.Size);
        switch (part.Type) {
        case RuntimeDataPartType::StringBuffer: {
          m_Context.StringBuffer.Init(PR.ReadArray<char>(part.Size), part.Size);
          break;
        }
        case RuntimeDataPartType::IndexArrays: {
          uint32_t count = part.Size / sizeof(uint32_t);
          m_Context.IndexTable.Init(PR.ReadArray<uint32_t>(count), count);
          break;
        }
        case RuntimeDataPartType::RawBytes: {
          m_Context.RawBytes.Init(PR.ReadArray<char>(part.Size), part.Size);
          break;
        }

// Once per table.
#define RDAT_STRUCT_TABLE(type, table)                                         \
  case RuntimeDataPartType::table:                                             \
    InitTable(m_Context, PR, RecordTableIndex::table);                         \
    break;
#define DEF_RDAT_TYPES DEF_RDAT_DEFAULTS
#include "dxc/DxilContainer/RDAT_Macros.inl"

        default:
          continue; // Skip unrecognized parts
        }
      }
#ifndef NDEBUG
      return Validate();
#else  // NDEBUG
      return true;
#endif // NDEBUG
    } catch (CheckedReader::exception e) {
      // TODO: error handling
      // throw hlsl::Exception(DXC_E_MALFORMED_CONTAINER, e.what());
      return false;
    }
  }
  m_DataSize = 0;
  return false;
}

// TODO: Incorporate field names and report errors in error stream

// TODO: Low-pri: Check other things like that all the index, string,
// and binary buffer space is actually used.

template <typename _RecordType>
static bool ValidateRecordRef(const RDATContext &ctx, uint32_t id) {
  if (id == RDAT_NULL_REF)
    return true;
  // id should be a valid index into the appropriate table
  auto &table = ctx.Table(RecordTraits<_RecordType>::TableIndex());
  if (id >= table.Count())
    return false;
  return true;
}

static bool ValidateIndexArrayRef(const RDATContext &ctx, uint32_t id) {
  if (id == RDAT_NULL_REF)
    return true;
  uint32_t size = ctx.IndexTable.Count();
  // check that id < size of index array
  if (id >= size)
    return false;
  // check that array size fits in remaining index space
  if (id + ctx.IndexTable.Data()[id] >= size)
    return false;
  return true;
}

template <typename _RecordType>
static bool ValidateRecordArrayRef(const RDATContext &ctx, uint32_t id) {
  // Make sure index array is well-formed
  if (!ValidateIndexArrayRef(ctx, id))
    return false;
  // Make sure each record id is a valid record ref in the table
  auto ids = ctx.IndexTable.getRow(id);
  for (unsigned i = 0; i < ids.Count(); i++) {
    if (!ValidateRecordRef<_RecordType>(ctx, ids.At(i)))
      return false;
  }
  return true;
}
static bool ValidateStringRef(const RDATContext &ctx, uint32_t id) {
  if (id == RDAT_NULL_REF)
    return true;
  uint32_t size = ctx.StringBuffer.Size();
  if (id >= size)
    return false;
  return true;
}
static bool ValidateStringArrayRef(const RDATContext &ctx, uint32_t id) {
  if (id == RDAT_NULL_REF)
    return true;
  // Make sure index array is well-formed
  if (!ValidateIndexArrayRef(ctx, id))
    return false;
  // Make sure each index is valid in string buffer
  auto ids = ctx.IndexTable.getRow(id);
  for (unsigned i = 0; i < ids.Count(); i++) {
    if (!ValidateStringRef(ctx, ids.At(i)))
      return false;
  }
  return true;
}

// Specialized for each record type
template <typename _RecordType>
bool ValidateRecord(const RDATContext &ctx, const _RecordType *pRecord) {
  return false;
}

#define DEF_RDAT_TYPES DEF_RDAT_STRUCT_VALIDATION
#include "dxc/DxilContainer/RDAT_Macros.inl"

// This class ensures that all versions of record to latest one supported by
// table stride are validated
class RecursiveRecordValidator {
  const hlsl::RDAT::RDATContext &m_Context;
  uint32_t m_RecordStride;

public:
  RecursiveRecordValidator(const hlsl::RDAT::RDATContext &ctx,
                           uint32_t recordStride)
      : m_Context(ctx), m_RecordStride(recordStride) {}
  template <typename _RecordType>
  bool Validate(const _RecordType *pRecord) const {
    if (pRecord && sizeof(_RecordType) <= m_RecordStride) {
      if (!ValidateRecord(m_Context, pRecord))
        return false;
      return ValidateDerived<_RecordType>(pRecord);
    }
    return true;
  }
  // Specialized for base type to recurse into derived
  template <typename _RecordType>
  bool ValidateDerived(const _RecordType *) const {
    return true;
  }
};

template <typename _RecordType>
static bool ValidateRecordTable(RDATContext &ctx, RecordTableIndex tableIndex) {
  // iterate through records, bounds-checking all refs and index arrays
  auto &table = ctx.Table(tableIndex);
  for (unsigned i = 0; i < table.Count(); i++) {
    RecursiveRecordValidator(ctx, table.Stride())
        .Validate<_RecordType>(table.Row<_RecordType>(i));
  }
  return true;
}

bool DxilRuntimeData::Validate() {
  if (m_Context.StringBuffer.Size()) {
    if (m_Context.StringBuffer.Data()[m_Context.StringBuffer.Size() - 1] != 0)
      return false;
  }

  // Once per table.
#define RDAT_STRUCT_TABLE(type, table)                                         \
  ValidateRecordTable<type>(m_Context, RecordTableIndex::table);
  // As an assumption of the way record types are versioned, derived record
  // types must always be larger than base record types.
#define RDAT_STRUCT_DERIVED(type, base)                                        \
  static_assert(sizeof(type) > sizeof(base),                                   \
                "otherwise, derived record type " #type                        \
                " is not larger than base record type " #base ".");
#define DEF_RDAT_TYPES DEF_RDAT_DEFAULTS
#include "dxc/DxilContainer/RDAT_Macros.inl"
  return true;
}

} // namespace RDAT
} // namespace hlsl
