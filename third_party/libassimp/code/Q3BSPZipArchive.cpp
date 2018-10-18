/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2018, assimp team


All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------
*/

#ifndef ASSIMP_BUILD_NO_Q3BSP_IMPORTER

#include "Q3BSPZipArchive.h"
#include <cassert>
#include <cstdlib>
#include <assimp/ai_assert.h>

namespace Assimp {
namespace Q3BSP {

voidpf IOSystem2Unzip::open(voidpf opaque, const char* filename, int mode) {
    IOSystem* io_system = (IOSystem*) opaque;

    const char* mode_fopen = NULL;
    if((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ) {
        mode_fopen = "rb";
    } else {
        if(mode & ZLIB_FILEFUNC_MODE_EXISTING) {
            mode_fopen = "r+b";
        } else {
            if(mode & ZLIB_FILEFUNC_MODE_CREATE) {
                mode_fopen = "wb";
            }
        }
    }

    return (voidpf) io_system->Open(filename, mode_fopen);
}

uLong IOSystem2Unzip::read(voidpf /*opaque*/, voidpf stream, void* buf, uLong size) {
    IOStream* io_stream = (IOStream*) stream;

    return static_cast<uLong>(io_stream->Read(buf, 1, size));
}

uLong IOSystem2Unzip::write(voidpf /*opaque*/, voidpf stream, const void* buf, uLong size) {
    IOStream* io_stream = (IOStream*) stream;

    return static_cast<uLong>(io_stream->Write(buf, 1, size));
}

long IOSystem2Unzip::tell(voidpf /*opaque*/, voidpf stream) {
    IOStream* io_stream = (IOStream*) stream;

    return static_cast<long>(io_stream->Tell());
}

long IOSystem2Unzip::seek(voidpf /*opaque*/, voidpf stream, uLong offset, int origin) {
    IOStream* io_stream = (IOStream*) stream;

    aiOrigin assimp_origin;
    switch (origin) {
        default:
        case ZLIB_FILEFUNC_SEEK_CUR:
            assimp_origin = aiOrigin_CUR;
            break;
        case ZLIB_FILEFUNC_SEEK_END:
            assimp_origin = aiOrigin_END;
            break;
        case ZLIB_FILEFUNC_SEEK_SET:
            assimp_origin = aiOrigin_SET;
            break;
    }

    return (io_stream->Seek(offset, assimp_origin) == aiReturn_SUCCESS ? 0 : -1);
}

int IOSystem2Unzip::close(voidpf opaque, voidpf stream) {
    IOSystem* io_system = (IOSystem*) opaque;
    IOStream* io_stream = (IOStream*) stream;

    io_system->Close(io_stream);

    return 0;
}

int IOSystem2Unzip::testerror(voidpf /*opaque*/, voidpf /*stream*/) {
    return 0;
}

zlib_filefunc_def IOSystem2Unzip::get(IOSystem* pIOHandler) {
    zlib_filefunc_def mapping;

    mapping.zopen_file = open;
    mapping.zread_file = read;
    mapping.zwrite_file = write;
    mapping.ztell_file = tell;
    mapping.zseek_file = seek;
    mapping.zclose_file = close;
    mapping.zerror_file = testerror;
    mapping.opaque = (voidpf) pIOHandler;

    return mapping;
}

ZipFile::ZipFile(size_t size) : m_Size(size) {
    ai_assert(m_Size != 0);

    m_Buffer = malloc(m_Size);
}

ZipFile::~ZipFile() {
    free(m_Buffer);
    m_Buffer = NULL;
}

size_t ZipFile::Read(void* pvBuffer, size_t pSize, size_t pCount) {
    const size_t size = pSize * pCount;
    assert(size <= m_Size);

    std::memcpy(pvBuffer, m_Buffer, size);

    return size;
}

size_t ZipFile::Write(const void* /*pvBuffer*/, size_t /*pSize*/, size_t /*pCount*/) {
    return 0;
}

size_t ZipFile::FileSize() const {
    return m_Size;
}

aiReturn ZipFile::Seek(size_t /*pOffset*/, aiOrigin /*pOrigin*/) {
    return aiReturn_FAILURE;
}

size_t ZipFile::Tell() const {
    return 0;
}

void ZipFile::Flush() {
    // empty
}

// ------------------------------------------------------------------------------------------------
//  Constructor.
Q3BSPZipArchive::Q3BSPZipArchive(IOSystem* pIOHandler, const std::string& rFile) : m_ZipFileHandle(NULL), m_ArchiveMap() {
    if (! rFile.empty()) {
        zlib_filefunc_def mapping = IOSystem2Unzip::get(pIOHandler);

        m_ZipFileHandle = unzOpen2(rFile.c_str(), &mapping);

        if(m_ZipFileHandle != nullptr) {
            mapArchive();
        }
    }
}

// ------------------------------------------------------------------------------------------------
//  Destructor.
Q3BSPZipArchive::~Q3BSPZipArchive() {
    for(auto &file : m_ArchiveMap) {
        delete file.second;
    }
    m_ArchiveMap.clear();

    if(m_ZipFileHandle != nullptr) {
        unzClose(m_ZipFileHandle);
        m_ZipFileHandle = nullptr;
    }
}

// ------------------------------------------------------------------------------------------------
//  Returns true, if the archive is already open.
bool Q3BSPZipArchive::isOpen() const {
    return (m_ZipFileHandle != nullptr);
}

// ------------------------------------------------------------------------------------------------
//  Returns true, if the filename is part of the archive.
bool Q3BSPZipArchive::Exists(const char* pFile) const {
    bool exist = false;
    if (pFile != nullptr) {
        std::string rFile(pFile);
        std::map<std::string, ZipFile*>::const_iterator it = m_ArchiveMap.find(rFile);

        if(it != m_ArchiveMap.end()) {
            exist = true;
        }
    }

    return exist;
}

// ------------------------------------------------------------------------------------------------
//  Returns the separator delimiter.
char Q3BSPZipArchive::getOsSeparator() const {
#ifndef _WIN32
    return '/';
#else
    return '\\';
#endif
}

// ------------------------------------------------------------------------------------------------
//  Opens a file, which is part of the archive.
IOStream *Q3BSPZipArchive::Open(const char* pFile, const char* /*pMode*/) {
    ai_assert(pFile != nullptr);

    IOStream* result = nullptr;

    std::map<std::string, ZipFile*>::iterator it = m_ArchiveMap.find(pFile);

    if(it != m_ArchiveMap.end()) {
        result = (IOStream*) it->second;
    }

    return result;
}

// ------------------------------------------------------------------------------------------------
//  Close a filestream.
void Q3BSPZipArchive::Close(IOStream *pFile) {
    (void)(pFile);
    ai_assert(pFile != nullptr);

    // We don't do anything in case the file would be opened again in the future
}
// ------------------------------------------------------------------------------------------------
//  Returns the file-list of the archive.
void Q3BSPZipArchive::getFileList(std::vector<std::string> &rFileList) {
    rFileList.clear();

    for(auto &file : m_ArchiveMap) {
        rFileList.push_back(file.first);
    }
}

// ------------------------------------------------------------------------------------------------
//  Maps the archive content.
bool Q3BSPZipArchive::mapArchive() {
    bool success = false;

    if(m_ZipFileHandle != nullptr) {
        if(m_ArchiveMap.empty()) {
            //  At first ensure file is already open
            if(unzGoToFirstFile(m_ZipFileHandle) == UNZ_OK) {
                // Loop over all files
                do {
                    char filename[FileNameSize];
                    unz_file_info fileInfo;

                    if(unzGetCurrentFileInfo(m_ZipFileHandle, &fileInfo, filename, FileNameSize, NULL, 0, NULL, 0) == UNZ_OK) {
                        // The file has EXACTLY the size of uncompressed_size. In C
                        // you need to mark the last character with '\0', so add
                        // another character
                        if(fileInfo.uncompressed_size != 0 && unzOpenCurrentFile(m_ZipFileHandle) == UNZ_OK) {
                            std::pair<std::map<std::string, ZipFile*>::iterator, bool> result = m_ArchiveMap.insert(std::make_pair(filename, new ZipFile(fileInfo.uncompressed_size)));

                            if(unzReadCurrentFile(m_ZipFileHandle, result.first->second->m_Buffer, fileInfo.uncompressed_size) == (long int) fileInfo.uncompressed_size) {
                                if(unzCloseCurrentFile(m_ZipFileHandle) == UNZ_OK) {
                                    // Nothing to do anymore...
                                }
                            }
                        }
                    }
                } while(unzGoToNextFile(m_ZipFileHandle) != UNZ_END_OF_LIST_OF_FILE);
            }
        }

        success = true;
    }

    return success;
}

// ------------------------------------------------------------------------------------------------

} // Namespace Q3BSP
} // Namespace Assimp

#endif // ASSIMP_BUILD_NO_Q3BSP_IMPORTER
