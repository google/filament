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
#ifndef AI_Q3BSP_ZIPARCHIVE_H_INC
#define AI_Q3BSP_ZIPARCHIVE_H_INC

#include <unzip.h>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <vector>
#include <map>
#include <cassert>

namespace Assimp {
namespace Q3BSP {

// ------------------------------------------------------------------------------------------------
/// \class      IOSystem2Unzip
/// \ingroup    Assimp::Q3BSP
///
/// \brief
// ------------------------------------------------------------------------------------------------
class IOSystem2Unzip {
public:
    static voidpf open(voidpf opaque, const char* filename, int mode);
    static uLong read(voidpf opaque, voidpf stream, void* buf, uLong size);
    static uLong write(voidpf opaque, voidpf stream, const void* buf, uLong size);
    static long tell(voidpf opaque, voidpf stream);
    static long seek(voidpf opaque, voidpf stream, uLong offset, int origin);
    static int close(voidpf opaque, voidpf stream);
    static int testerror(voidpf opaque, voidpf stream);
    static zlib_filefunc_def get(IOSystem* pIOHandler);
};

// ------------------------------------------------------------------------------------------------
/// \class      ZipFile
/// \ingroup    Assimp::Q3BSP
///
/// \brief
// ------------------------------------------------------------------------------------------------
class ZipFile : public IOStream {
    friend class Q3BSPZipArchive;

public:
    explicit ZipFile(size_t size);
    ~ZipFile();
    size_t Read(void* pvBuffer, size_t pSize, size_t pCount );
    size_t Write(const void* /*pvBuffer*/, size_t /*pSize*/, size_t /*pCount*/);
    size_t FileSize() const;
    aiReturn Seek(size_t /*pOffset*/, aiOrigin /*pOrigin*/);
    size_t Tell() const;
    void Flush();

private:
    void* m_Buffer;
    size_t m_Size;
};

// ------------------------------------------------------------------------------------------------
/// \class      Q3BSPZipArchive
/// \ingroup    Assimp::Q3BSP
///
/// \brief  IMplements a zip archive like the WinZip archives. Will be also used to import data
/// from a P3K archive ( Quake level format ).
// ------------------------------------------------------------------------------------------------
class Q3BSPZipArchive : public Assimp::IOSystem {
public:
    static const unsigned int FileNameSize = 256;

public:
    Q3BSPZipArchive(IOSystem* pIOHandler, const std::string & rFile);
    ~Q3BSPZipArchive();
    bool Exists(const char* pFile) const;
    char getOsSeparator() const;
    IOStream* Open(const char* pFile, const char* pMode = "rb");
    void Close(IOStream* pFile);
    bool isOpen() const;
    void getFileList(std::vector<std::string> &rFileList);

private:
    bool mapArchive();

private:
    unzFile m_ZipFileHandle;
    std::map<std::string, ZipFile*> m_ArchiveMap;
};

// ------------------------------------------------------------------------------------------------

} // Namespace Q3BSP
} // Namespace Assimp

#endif // AI_Q3BSP_ZIPARCHIVE_H_INC
