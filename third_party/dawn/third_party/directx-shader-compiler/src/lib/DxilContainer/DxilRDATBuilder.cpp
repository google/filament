#include "dxc/DxilContainer/DxilRDATBuilder.h"
#include "dxc/DxilContainer/DxilPipelineStateValidation.h"
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/Global.h"

using namespace llvm;
using namespace hlsl;
using namespace RDAT;

void RDATTable::SetRecordStride(size_t RecordStride) {
  DXASSERT(m_rows.empty(), "record stride is fixed for the entire table");
  m_RecordStride = RecordStride;
}

uint32_t RDATTable::InsertImpl(const void *ptr, size_t size) {
  IFTBOOL(m_RecordStride <= size, DXC_E_GENERAL_INTERNAL_ERROR);
  size_t count = m_rows.size();
  if (count < (UINT32_MAX - 1)) {
    const char *pData = (const char *)ptr;
    auto result = m_map.insert(
        std::make_pair(std::string(pData, pData + m_RecordStride), count));
    if (!m_bDeduplicationEnabled || result.second) {
      m_rows.emplace_back(result.first->first);
      return count;
    } else {
      return result.first->second;
    }
  }
  return RDAT_NULL_REF;
}

void RDATTable::Write(void *ptr) {
  char *pCur = (char *)ptr;
  RuntimeDataTableHeader &header =
      *reinterpret_cast<RuntimeDataTableHeader *>(pCur);
  header.RecordCount = m_rows.size();
  header.RecordStride = m_RecordStride;
  pCur += sizeof(RuntimeDataTableHeader);
  for (auto &record : m_rows) {
    DXASSERT_NOMSG(record.size() == m_RecordStride);
    memcpy(pCur, record.data(), m_RecordStride);
    pCur += m_RecordStride;
  }
};

uint32_t RDATTable::GetPartSize() const {
  if (m_rows.empty())
    return 0;
  return sizeof(RuntimeDataTableHeader) + m_rows.size() * m_RecordStride;
}

uint32_t RawBytesPart::Insert(const void *pData, size_t dataSize) {
  auto result = m_Map.insert(std::make_pair(
      std::string((const char *)pData, (const char *)pData + dataSize),
      m_Size));
  auto iterator = result.first;
  if (result.second) {
    const std::string &key = iterator->first;
    m_List.push_back(llvm::StringRef(key.data(), key.size()));
    m_Size += key.size();
  }
  return iterator->second;
}

void RawBytesPart::Write(void *ptr) {
  for (llvm::StringRef &entry : m_List) {
    memcpy(ptr, entry.data(), entry.size());
    ptr = (char *)ptr + entry.size();
  }
}

DxilRDATBuilder::DxilRDATBuilder(bool allowRecordDuplication)
    : m_bRecordDeduplicationEnabled(allowRecordDuplication) {}

DxilRDATBuilder::SizeInfo DxilRDATBuilder::ComputeSize() const {
  uint32_t totalSizeOfNonEmptyParts = 0;
  uint32_t numNonEmptyParts = 0;
  for (auto &part : m_Parts) {
    if (part->GetPartSize() == 0)
      continue;
    numNonEmptyParts++;
    totalSizeOfNonEmptyParts +=
        sizeof(RuntimeDataPartHeader) + PSVALIGN4(part->GetPartSize());
  }

  uint32_t total = sizeof(RuntimeDataHeader) +           // Header
                   numNonEmptyParts * sizeof(uint32_t) + // Offset array
                   totalSizeOfNonEmptyParts;             // Parts contents

  SizeInfo ret = {};
  ret.numParts = numNonEmptyParts;
  ret.sizeInBytes = total;
  return ret;
}

namespace {
// Size-checked writer
//  on overrun: throw buffer_overrun{};
//  on overlap: throw buffer_overlap{};
class CheckedWriter {
  char *Ptr;
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

  CheckedWriter(void *ptr, size_t size)
      : Ptr(reinterpret_cast<char *>(ptr)), Size(size), Offset(0) {}

  size_t GetOffset() const { return Offset; }
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
  template <typename T> T *Cast(size_t size = 0) {
    if (0 == size)
      size = sizeof(T);
    CheckBounds(size);
    return reinterpret_cast<T *>(Ptr + Offset);
  }

  // Map and Write advance Offset:
  template <typename T> T &Map() {
    const size_t size = sizeof(T);
    T *p = Cast<T>(size);
    Offset += size;
    return *p;
  }
  template <typename T> T *MapArray(size_t count = 1) {
    const size_t size = sizeof(T) * count;
    T *p = Cast<T>(size);
    Offset += size;
    return p;
  }
  template <typename T> void Write(const T &obj) {
    const size_t size = sizeof(T);
    *Cast<T>(size) = obj;
    Offset += size;
  }
  template <typename T> void WriteArray(const T *pArray, size_t count = 1) {
    const size_t size = sizeof(T) * count;
    memcpy(Cast<T>(size), pArray, size);
    Offset += size;
  }
};
} // namespace

// returns the offset of the name inserted
uint32_t StringBufferPart::Insert(llvm::StringRef str) {
  auto result = m_Map.insert(
      std::make_pair(std::string(str.data(), str.data() + str.size()), m_Size));

  auto iterator = result.first;
  if (result.second) {
    const std::string &key = iterator->first;
    m_List.push_back(llvm::StringRef(key.data(), key.size()));
    m_Size += key.size() + 1 /*null terminator*/;
  }
  return iterator->second;
}

void StringBufferPart::Write(void *ptr) {
  for (llvm::StringRef &entry : m_List) {
    memcpy(ptr, entry.data(), entry.size() + 1 /*null terminator*/);
    ptr = (char *)ptr + entry.size() + 1;
  }
}

StringRef DxilRDATBuilder::FinalizeAndGetData() {
  try {
    m_RDATBuffer.resize(size(), 0);
    CheckedWriter W(m_RDATBuffer.data(), m_RDATBuffer.size());
    // write RDAT header
    RuntimeDataHeader &header = W.Map<RuntimeDataHeader>();
    header.Version = RDAT_Version_10;
    header.PartCount = ComputeSize().numParts;
    // map offsets
    uint32_t *offsets = W.MapArray<uint32_t>(header.PartCount);
    // write parts
    unsigned i = 0;
    for (auto &part : m_Parts) {
      if (part->GetPartSize() == 0)
        continue;
      offsets[i++] = W.GetOffset();
      RuntimeDataPartHeader &partHeader = W.Map<RuntimeDataPartHeader>();
      partHeader.Type = part->GetType();
      partHeader.Size = PSVALIGN4(part->GetPartSize());
      DXASSERT(partHeader.Size, "otherwise, failed to remove empty part");
      char *bytes = W.MapArray<char>(partHeader.Size);
      part->Write(bytes);
    }
  } catch (CheckedWriter::exception e) {
    throw hlsl::Exception(DXC_E_GENERAL_INTERNAL_ERROR, e.what());
  }
  return llvm::StringRef(m_RDATBuffer.data(), m_RDATBuffer.size());
}
