///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcshadersourceinfo.cpp                                                   //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Utility helpers for dealing with DXIL part related to shader sources      //
// and options.                                                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxcshadersourceinfo.h"
#include "dxc/DxilCompression/DxilCompressionHelpers.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CodeGenOptions.h"
#include "llvm/Support/Path.h"

#include "dxc/Support/Global.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/Path.h"
#include "dxc/Support/WinIncludes.h"

using namespace hlsl;
using Buffer = SourceInfoWriter::Buffer;

///////////////////////////////////////////////////////////////////////////////
// Reader
///////////////////////////////////////////////////////////////////////////////

static size_t PointerByteOffset(const void *a, const void *b) {
  return (const uint8_t *)a - (const uint8_t *)b;
}

bool SourceInfoReader::Init(const hlsl::DxilSourceInfo *SourceInfo,
                            unsigned sourceInfoSize) {
  if (sizeof(*SourceInfo) > sourceInfoSize)
    return false;
  if (SourceInfo->AlignedSizeInBytes > sourceInfoSize)
    return false;

  const hlsl::DxilSourceInfo *mainHeader = SourceInfo;
  const size_t totalSizeInBytes = mainHeader->AlignedSizeInBytes;

  const hlsl::DxilSourceInfoSection *section =
      (const hlsl::DxilSourceInfoSection *)(SourceInfo + 1);
  for (unsigned i = 0; i < SourceInfo->SectionCount; i++) {

    if (PointerByteOffset(section + 1, mainHeader) > totalSizeInBytes)
      return false;
    if (PointerByteOffset(section, mainHeader) + section->AlignedSizeInBytes >
        totalSizeInBytes)
      return false;

    const size_t sectionSizeInBytes = section->AlignedSizeInBytes;

    switch (section->Type) {
    case hlsl::DxilSourceInfoSectionType::Args: {
      const hlsl::DxilSourceInfo_Args *header =
          (const hlsl::DxilSourceInfo_Args *)(section + 1);

      if (PointerByteOffset(header + 1, section) > sectionSizeInBytes)
        return false;
      if (PointerByteOffset(header + 1, section) + header->SizeInBytes >
          sectionSizeInBytes)
        return false;

      const char *ptr = (const char *)(header + 1);
      unsigned i = 0;
      while (i < header->SizeInBytes) {

        // To learn more about the format of this data, check struct
        // DxilSourceInfo_Args in DxilContainer.h Read the argument name
        const char *argName = ptr + i;
        unsigned argNameLength = 0;
        for (; i < header->SizeInBytes; i++) {
          if (ptr[i] == '\0') {
            i++;
            break;
          }
          argNameLength++;
        }

        // Read the argument value
        const char *argValue = ptr + i;
        unsigned argValueLength = 0;
        for (; i < header->SizeInBytes; i++) {
          if (ptr[i] == '\0') {
            i++;
            break;
          }
          argValueLength++;
        }

        ArgPair pair = {};
        assert(argNameLength || argValueLength);
        if (argNameLength || argValueLength) {
          if (argNameLength)
            pair.Name.assign(argName, argNameLength);
          if (argValueLength)
            pair.Value.assign(argValue, argValueLength);
          m_ArgPairs.push_back(std::move(pair));
        }
      }
    } break;

    case hlsl::DxilSourceInfoSectionType::SourceNames: {
      const hlsl::DxilSourceInfo_SourceNames *header =
          (const hlsl::DxilSourceInfo_SourceNames *)(section + 1);
      if (PointerByteOffset(header + 1, section) > sectionSizeInBytes)
        return false;
      if (PointerByteOffset(header + 1, section) + header->EntriesSizeInBytes >
          sectionSizeInBytes)
        return false;

      assert(m_Sources.size() == 0 || m_Sources.size() == header->Count);
      m_Sources.resize(header->Count);

      const hlsl::DxilSourceInfo_SourceNamesEntry *firstEntry =
          (const hlsl::DxilSourceInfo_SourceNamesEntry *)(header + 1);
      const hlsl::DxilSourceInfo_SourceNamesEntry *entry = firstEntry;

      for (unsigned i = 0; i < header->Count; i++) {
        if (PointerByteOffset(entry + 1, firstEntry) >
            header->EntriesSizeInBytes)
          return false;
        if (PointerByteOffset(entry + 1, firstEntry) + entry->NameSizeInBytes >
            header->EntriesSizeInBytes)
          return false;
        if (PointerByteOffset(entry, firstEntry) + entry->AlignedSizeInBytes >
            header->EntriesSizeInBytes)
          return false;

        const void *ptr = entry + 1;
        if (entry->NameSizeInBytes > 0) {
          // Fail if not null terminated
          if (((const char *)ptr)[entry->NameSizeInBytes - 1] != '\0')
            return false;
          llvm::StringRef name = {
              (const char *)ptr,
              entry->NameSizeInBytes - 1,
          };
          m_Sources[i].Name = name;
        }

        entry = (const hlsl::DxilSourceInfo_SourceNamesEntry
                     *)((const uint8_t *)entry + entry->AlignedSizeInBytes);
      }

    } break;
    case hlsl::DxilSourceInfoSectionType::SourceContents: {
      const hlsl::DxilSourceInfo_SourceContents *header =
          (const hlsl::DxilSourceInfo_SourceContents *)(section + 1);
      if (PointerByteOffset(header + 1, section) > sectionSizeInBytes)
        return false;
      if (PointerByteOffset(header + 1, section) + header->EntriesSizeInBytes >
          sectionSizeInBytes)
        return false;

      const hlsl::DxilSourceInfo_SourceContentsEntry *firstEntry = nullptr;
      if (header->CompressType ==
          hlsl::DxilSourceInfo_SourceContentsCompressType::Zlib) {
        m_UncompressedSources.resize(header->UncompressedEntriesSizeInBytes);
        {
          bool bDecompressSucc =
              hlsl::ZlibResult::Success ==
              ZlibDecompress(DxcGetThreadMallocNoRef(), header + 1,
                             header->EntriesSizeInBytes,
                             m_UncompressedSources.data(),
                             m_UncompressedSources.size());
          assert(bDecompressSucc);
          if (!bDecompressSucc)
            return false;
        }
        if (m_UncompressedSources.size() !=
            header->UncompressedEntriesSizeInBytes)
          return false;
        firstEntry = (const hlsl::DxilSourceInfo_SourceContentsEntry *)
                         m_UncompressedSources.data();
      } else {
        if (header->EntriesSizeInBytes !=
            header->UncompressedEntriesSizeInBytes)
          return false;
        if (PointerByteOffset(header + 1, section) +
                header->UncompressedEntriesSizeInBytes >
            sectionSizeInBytes)
          return false;
        firstEntry =
            (const hlsl::DxilSourceInfo_SourceContentsEntry *)(header + 1);
      }

      assert(m_Sources.size() == 0 || m_Sources.size() == header->Count);
      m_Sources.resize(header->Count);

      const hlsl::DxilSourceInfo_SourceContentsEntry *entry = firstEntry;
      for (unsigned i = 0; i < header->Count; i++) {
        if (PointerByteOffset(entry + 1, firstEntry) >
            header->UncompressedEntriesSizeInBytes)
          return false;
        if (PointerByteOffset(entry + 1, firstEntry) +
                entry->ContentSizeInBytes >
            header->UncompressedEntriesSizeInBytes)
          return false;
        if (PointerByteOffset(entry, firstEntry) + entry->AlignedSizeInBytes >
            header->UncompressedEntriesSizeInBytes)
          return false;

        const void *ptr = entry + 1;
        if (entry->ContentSizeInBytes > 0) {
          // Fail if not null terminated
          if (((const char *)ptr)[entry->ContentSizeInBytes - 1] != '\0')
            return false;
          llvm::StringRef content = {
              (const char *)ptr,
              entry->ContentSizeInBytes - 1,
          };
          m_Sources[i].Content = content;
        }

        entry = (const hlsl::DxilSourceInfo_SourceContentsEntry
                     *)((const uint8_t *)entry + entry->AlignedSizeInBytes);
      }
    } break;
    }
    section =
        (const hlsl::DxilSourceInfoSection *)((const uint8_t *)section +
                                              section->AlignedSizeInBytes);
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Writer
///////////////////////////////////////////////////////////////////////////////

static void Append(Buffer *buf, uint8_t c) { buf->push_back(c); }
static void Append(Buffer *buf, const void *ptr, size_t size) {
  size_t OldSize = buf->size();
  buf->resize(OldSize + size);
  memcpy(buf->data() + OldSize, ptr, size);
}

static uint32_t PadToFourBytes(uint32_t size) {
  uint32_t rem = size % 4;
  if (rem)
    return size + (4 - rem);
  return size;
}

static uint32_t PadBufferToFourBytes(Buffer *buf, uint32_t unpaddedSize) {
  const uint32_t paddedSize = PadToFourBytes(unpaddedSize);
  // Padding.
  for (uint32_t i = unpaddedSize; i < paddedSize; i++) {
    Append(buf, 0);
  }
  return paddedSize;
}

static void AppendFileContentEntry(Buffer *buf, llvm::StringRef content) {
  hlsl::DxilSourceInfo_SourceContentsEntry header = {};
  header.AlignedSizeInBytes =
      PadToFourBytes(sizeof(header) + content.size() + 1);
  header.ContentSizeInBytes = content.size() + 1;

  const size_t offset = buf->size();
  Append(buf, &header, sizeof(header));
  Append(buf, content.data(), content.size());
  Append(buf, 0); // Null term

  const size_t paddedOffset = PadBufferToFourBytes(buf, buf->size() - offset);
  (void)paddedOffset;
  assert(paddedOffset == header.AlignedSizeInBytes);
}

static size_t BeginSection(Buffer *buf) {
  const size_t sectionOffset = buf->size();

  hlsl::DxilSourceInfoSection sectionHeader = {};
  Append(buf, &sectionHeader, sizeof(sectionHeader)); // Write an empty header

  return sectionOffset;
}

static void FinishSection(Buffer *buf, const size_t sectionOffset,
                          hlsl::DxilSourceInfoSectionType type) {
  hlsl::DxilSourceInfoSection sectionHeader = {};

  // Calculate and pad the size of the section.
  const size_t sectionSize = buf->size() - sectionOffset;
  const size_t paddedSectionSize = PadBufferToFourBytes(buf, sectionSize);

  // Go back and rewrite the section header
  assert(paddedSectionSize % sizeof(uint32_t) == 0);
  sectionHeader.AlignedSizeInBytes = paddedSectionSize;
  sectionHeader.Type = type;
  memcpy(buf->data() + sectionOffset, &sectionHeader, sizeof(sectionHeader));
}

struct SourceFile {
  std::string Name;
  llvm::StringRef Content;
};

static std::vector<SourceFile> ComputeFileList(clang::CodeGenOptions &cgOpts,
                                               clang::SourceManager &srcMgr) {
  std::vector<SourceFile> ret;
  std::map<std::string, llvm::StringRef> filesMap;
  {
    bool bFoundMainFile = false;
    for (auto it = srcMgr.fileinfo_begin(), end = srcMgr.fileinfo_end();
         it != end; ++it) {
      if (it->first->isValid() && !it->second->IsSystemFile) {
        llvm::SmallString<256> Path = llvm::StringRef(it->first->getName());
        llvm::sys::path::native(Path);
        // If main file, write that to metadata first.
        // Add the rest to filesMap to sort by name.
        if (cgOpts.MainFileName.compare(it->first->getName()) == 0) {
          SourceFile file;
          file.Name = Path.str();
          file.Content = it->second->getRawBuffer()->getBuffer();
          ret.push_back(file);
          assert(!bFoundMainFile &&
                 "otherwise, more than one file matches main filename");
          bFoundMainFile = true;
        } else {
          filesMap[Path.str()] = it->second->getRawBuffer()->getBuffer();
        }
      }
    }
    assert(bFoundMainFile && "otherwise, no file found matches main filename");
    // Emit the rest of the files in sorted order.
    for (auto it : filesMap) {
      SourceFile file;
      file.Name = it.first;
      file.Content = it.second;
      ret.push_back(file);
    }
  }
  return ret;
}

void SourceInfoWriter::Write(llvm::StringRef targetProfile,
                             llvm::StringRef entryPoint,
                             clang::CodeGenOptions &cgOpts,
                             clang::SourceManager &srcMgr) {
  m_Buffer.clear();

  // Write an empty header first.
  hlsl::DxilSourceInfo mainHeader = {};
  Append(&m_Buffer, &mainHeader, sizeof(mainHeader));

  std::vector<SourceFile> sourceFileList = ComputeFileList(cgOpts, srcMgr);

  ////////////////////////////////////////////////////////////////////
  // Add all file names in a list.
  ////////////////////////////////////////////////////////////////////
  {
    const size_t sectionOffset = BeginSection(&m_Buffer);

    // Write an empty header
    const size_t headerOffset = m_Buffer.size();
    hlsl::DxilSourceInfo_SourceNames header = {};
    Append(&m_Buffer, &header, sizeof(header));

    // Write the entries data
    const size_t dataOffset = m_Buffer.size();
    for (unsigned i = 0; i < sourceFileList.size(); i++) {
      SourceFile &file = sourceFileList[i];
      const size_t entryOffset = m_Buffer.size();

      // Write the header
      hlsl::DxilSourceInfo_SourceNamesEntry entryHeader = {};
      entryHeader.NameSizeInBytes = file.Name.size() + 1;
      entryHeader.ContentSizeInBytes = file.Content.size() + 1;
      entryHeader.AlignedSizeInBytes =
          PadToFourBytes(sizeof(entryHeader) + file.Name.size() + 1);

      Append(&m_Buffer, &entryHeader, sizeof(entryHeader));
      Append(&m_Buffer, file.Name.data(), file.Name.size());
      Append(&m_Buffer, 0); // Null terminator

      const size_t paddedOffset =
          PadBufferToFourBytes(&m_Buffer, m_Buffer.size() - entryOffset);
      (void)paddedOffset;
      assert(paddedOffset == entryHeader.AlignedSizeInBytes);
    }

    // Go back and write the header.
    header.Count = sourceFileList.size();
    header.EntriesSizeInBytes = m_Buffer.size() - dataOffset;
    memcpy(m_Buffer.data() + headerOffset, &header, sizeof(header));

    FinishSection(&m_Buffer, sectionOffset,
                  hlsl::DxilSourceInfoSectionType::SourceNames);
    mainHeader.SectionCount++;
  }

  ////////////////////////////////////////////////////////////////////
  // Add all file contents in a list.
  ////////////////////////////////////////////////////////////////////
  {
    const size_t sectionOffset = BeginSection(&m_Buffer);

    // Put all the contents in a buffer
    Buffer uncompressedBuffer;
    for (unsigned i = 0; i < sourceFileList.size(); i++) {
      SourceFile &file = sourceFileList[i];
      AppendFileContentEntry(&uncompressedBuffer, file.Content);
    }

    const size_t headerOffset = m_Buffer.size();

    // Write the header
    hlsl::DxilSourceInfo_SourceContents header = {};
    header.EntriesSizeInBytes = uncompressedBuffer.size();
    header.UncompressedEntriesSizeInBytes = uncompressedBuffer.size();
    header.Count = sourceFileList.size();
    Append(&m_Buffer, &header, sizeof(header));

    const size_t sizeBeforeCompress = m_Buffer.size();
    bool bCompressed =
        hlsl::ZlibResult::Success ==
        ZlibCompressAppend(DxcGetThreadMallocNoRef(), uncompressedBuffer.data(),
                           uncompressedBuffer.size(), m_Buffer);

    // If we compressed the content, go back to rewrite the header to write the
    // correct size in bytes.
    if (bCompressed) {
      header.EntriesSizeInBytes = m_Buffer.size() - sizeBeforeCompress;
      header.CompressType =
          hlsl::DxilSourceInfo_SourceContentsCompressType::Zlib;
      memcpy(m_Buffer.data() + headerOffset, &header, sizeof(header));
    }
    // Otherwise, just write the whole uncompressed
    else {
      Append(&m_Buffer, uncompressedBuffer.data(), uncompressedBuffer.size());
    }

    FinishSection(&m_Buffer, sectionOffset,
                  hlsl::DxilSourceInfoSectionType::SourceContents);
    mainHeader.SectionCount++;
  }

  ////////////////////////////////////////////////////////////////////
  // Args
  ////////////////////////////////////////////////////////////////////
  {
    // Use the opt table to render the arguments and separate them into
    // argName and argValue.
    const size_t sectionOffset = BeginSection(&m_Buffer);

    hlsl::DxilSourceInfo_Args header = {};
    const size_t headerOffset = m_Buffer.size();
    Append(&m_Buffer, &header, sizeof(header)); // Write an empty header first

    llvm::SmallVector<const char *, 32> optPointers;
    for (const std::string &arg : cgOpts.HLSLArguments) {
      optPointers.push_back(arg.c_str());
    }
    unsigned missingIndex = 0;
    unsigned missingCount = 0;
    const llvm::opt::OptTable *optTable = hlsl::options::getHlslOptTable();
    llvm::opt::InputArgList argList =
        optTable->ParseArgs(optPointers, missingIndex, missingCount);

    llvm::SmallString<64> argumentStorage;
    const size_t argumentsOffset = m_Buffer.size();
    for (llvm::opt::Arg *arg : argList) {
      llvm::StringRef name = arg->getOption().getName();
      const char *value = nullptr;
      if (arg->getNumValues() > 0) {
        assert(arg->getNumValues() == 1);
        value = arg->getValue();
      }

      // If this is a positional argument, set the name to ""
      // explicitly.
      if (arg->getOption().getKind() == llvm::opt::Option::InputClass) {
        name = "";
      }
      // If the argument must be merged (eg. -Wx, where W is the option and x is
      // the value), merge them right now.
      else if (arg->getOption().getKind() == llvm::opt::Option::JoinedClass) {
        argumentStorage.clear();
        argumentStorage.append(name);
        argumentStorage.append(value);

        name = argumentStorage;
        value = nullptr;
      }

      // Name
      Append(&m_Buffer, name.data(), name.size());
      Append(&m_Buffer, 0); // Null term

      // Value
      if (value)
        Append(&m_Buffer, value, strlen(value));
      Append(&m_Buffer, 0); // Null term

      header.Count++;
    } // For each arg

    // Go back and rewrite the header now that we know the size
    header.SizeInBytes = m_Buffer.size() - argumentsOffset;
    memcpy(m_Buffer.data() + headerOffset, &header, sizeof(header));

    FinishSection(&m_Buffer, sectionOffset,
                  hlsl::DxilSourceInfoSectionType::Args);
    mainHeader.SectionCount++;
  }

  // Go back and rewrite the main header.
  assert(m_Buffer.size() >= sizeof(mainHeader));

  size_t mainPartSize = m_Buffer.size();
  mainPartSize = PadBufferToFourBytes(&m_Buffer, mainPartSize);
  assert(mainPartSize % sizeof(uint32_t) == 0);
  mainHeader.AlignedSizeInBytes = mainPartSize;

  memcpy(m_Buffer.data(), &mainHeader, sizeof(mainHeader));
}

const hlsl::DxilSourceInfo *hlsl::SourceInfoWriter::GetPart() const {
  if (!m_Buffer.size())
    return nullptr;
  assert(m_Buffer.size() >= sizeof(hlsl::DxilSourceInfo));
  const hlsl::DxilSourceInfo *ret =
      (const hlsl::DxilSourceInfo *)m_Buffer.data();
  assert(ret->AlignedSizeInBytes == m_Buffer.size());
  return ret;
}
