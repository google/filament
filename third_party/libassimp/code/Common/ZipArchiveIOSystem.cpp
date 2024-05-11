/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team


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

/** @file  ZipArchiveIOSystem.cpp
 *  @brief Zip File I/O implementation for #Importer
 */

#include <assimp/ZipArchiveIOSystem.h>
#include <assimp/BaseImporter.h>

#include <assimp/ai_assert.h>

#include <map>
#include <memory>

#ifdef ASSIMP_USE_HUNTER
#  include <minizip/unzip.h>
#else
#  include <unzip.h>
#endif

namespace Assimp {
    // ----------------------------------------------------------------
    // Wraps an existing Assimp::IOSystem for unzip
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

    voidpf IOSystem2Unzip::open(voidpf opaque, const char* filename, int mode) {
        IOSystem* io_system = reinterpret_cast<IOSystem*>(opaque);

        const char* mode_fopen = nullptr;
        if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER) == ZLIB_FILEFUNC_MODE_READ) {
            mode_fopen = "rb";
        }
        else {
            if (mode & ZLIB_FILEFUNC_MODE_EXISTING) {
                mode_fopen = "r+b";
            }
            else {
                if (mode & ZLIB_FILEFUNC_MODE_CREATE) {
                    mode_fopen = "wb";
                }
            }
        }

        return (voidpf)io_system->Open(filename, mode_fopen);
    }

    uLong IOSystem2Unzip::read(voidpf /*opaque*/, voidpf stream, void* buf, uLong size) {
        IOStream* io_stream = (IOStream*)stream;

        return static_cast<uLong>(io_stream->Read(buf, 1, size));
    }

    uLong IOSystem2Unzip::write(voidpf /*opaque*/, voidpf stream, const void* buf, uLong size) {
        IOStream* io_stream = (IOStream*)stream;

        return static_cast<uLong>(io_stream->Write(buf, 1, size));
    }

    long IOSystem2Unzip::tell(voidpf /*opaque*/, voidpf stream) {
        IOStream* io_stream = (IOStream*)stream;

        return static_cast<long>(io_stream->Tell());
    }

    long IOSystem2Unzip::seek(voidpf /*opaque*/, voidpf stream, uLong offset, int origin) {
        IOStream* io_stream = (IOStream*)stream;

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
        IOSystem* io_system = (IOSystem*)opaque;
        IOStream* io_stream = (IOStream*)stream;

        io_system->Close(io_stream);

        return 0;
    }

    int IOSystem2Unzip::testerror(voidpf /*opaque*/, voidpf /*stream*/) {
        return 0;
    }

    zlib_filefunc_def IOSystem2Unzip::get(IOSystem* pIOHandler) {
        zlib_filefunc_def mapping;

#ifdef ASSIMP_USE_HUNTER
        mapping.zopen_file = (open_file_func)open;
        mapping.zread_file = (read_file_func)read;
        mapping.zwrite_file = (write_file_func)write;
        mapping.ztell_file = (tell_file_func)tell;
        mapping.zseek_file = (seek_file_func)seek;
        mapping.zclose_file = (close_file_func)close;
        mapping.zerror_file = (error_file_func)testerror;
#else
        mapping.zopen_file = open;
        mapping.zread_file = read;
        mapping.zwrite_file = write;
        mapping.ztell_file = tell;
        mapping.zseek_file = seek;
        mapping.zclose_file = close;
        mapping.zerror_file = testerror;
#endif
        mapping.opaque = reinterpret_cast<voidpf>(pIOHandler);

        return mapping;
    }

    // ----------------------------------------------------------------
    // A read-only file inside a ZIP

    class ZipFile : public IOStream {
        friend class ZipFileInfo;
        explicit ZipFile(size_t size);
    public:
        virtual ~ZipFile();

        // IOStream interface
        size_t Read(void* pvBuffer, size_t pSize, size_t pCount) override;
        size_t Write(const void* /*pvBuffer*/, size_t /*pSize*/, size_t /*pCount*/) override { return 0; }
        size_t FileSize() const override;
        aiReturn Seek(size_t pOffset, aiOrigin pOrigin) override;
        size_t Tell() const override;
        void Flush() override {}

    private:
        size_t m_Size = 0;
        size_t m_SeekPtr = 0;
        std::unique_ptr<uint8_t[]> m_Buffer;
    };


    // ----------------------------------------------------------------
    // Info about a read-only file inside a ZIP
    class ZipFileInfo
    {
    public:
        explicit ZipFileInfo(unzFile zip_handle, size_t size);

        // Allocate and Extract data from the ZIP
        ZipFile * Extract(unzFile zip_handle) const;

    private:
        size_t m_Size = 0;
        unz_file_pos_s m_ZipFilePos;
    };

    ZipFileInfo::ZipFileInfo(unzFile zip_handle, size_t size)
        : m_Size(size) {
        ai_assert(m_Size != 0);
        // Workaround for MSVC 2013 - C2797
        m_ZipFilePos.num_of_file = 0;
        m_ZipFilePos.pos_in_zip_directory = 0;
        unzGetFilePos(zip_handle, &(m_ZipFilePos));
    }

    ZipFile * ZipFileInfo::Extract(unzFile zip_handle) const {
        // Find in the ZIP. This cannot fail
        unz_file_pos_s *filepos = const_cast<unz_file_pos_s*>(&(m_ZipFilePos));
        if (unzGoToFilePos(zip_handle, filepos) != UNZ_OK)
            return nullptr;

        if (unzOpenCurrentFile(zip_handle) != UNZ_OK)
            return nullptr;

        ZipFile *zip_file = new ZipFile(m_Size);

        if (unzReadCurrentFile(zip_handle, zip_file->m_Buffer.get(), static_cast<unsigned int>(m_Size)) != static_cast<int>(m_Size))
        {
            // Failed, release the memory
            delete zip_file;
            zip_file = nullptr;
        }

        ai_assert(unzCloseCurrentFile(zip_handle) == UNZ_OK);
        return zip_file;
    }

    ZipFile::ZipFile(size_t size)
        : m_Size(size) {
        ai_assert(m_Size != 0);
        m_Buffer = std::unique_ptr<uint8_t[]>(new uint8_t[m_Size]);
    }

    ZipFile::~ZipFile() {
    }

    size_t ZipFile::Read(void* pvBuffer, size_t pSize, size_t pCount) {
        // Should be impossible
        ai_assert(m_Buffer != nullptr);
        ai_assert(NULL != pvBuffer && 0 != pSize && 0 != pCount);

        // Clip down to file size
        size_t byteSize = pSize * pCount;
        if ((byteSize + m_SeekPtr) > m_Size)
        {
            pCount = (m_Size - m_SeekPtr) / pSize;
            byteSize = pSize * pCount;
            if (byteSize == 0)
                return 0;
        }

        std::memcpy(pvBuffer, m_Buffer.get() + m_SeekPtr, byteSize);

        m_SeekPtr += byteSize;

        return pCount;
    }

    size_t ZipFile::FileSize() const {
        return m_Size;
    }

    aiReturn ZipFile::Seek(size_t pOffset, aiOrigin pOrigin) {
        switch (pOrigin)
        {
        case aiOrigin_SET: {
            if (pOffset > m_Size) return aiReturn_FAILURE;
            m_SeekPtr = pOffset;
            return aiReturn_SUCCESS;
        }

        case aiOrigin_CUR: {
            if ((pOffset + m_SeekPtr) > m_Size) return aiReturn_FAILURE;
            m_SeekPtr += pOffset;
            return aiReturn_SUCCESS;
        }

        case aiOrigin_END: {
            if (pOffset > m_Size) return aiReturn_FAILURE;
            m_SeekPtr = m_Size - pOffset;
            return aiReturn_SUCCESS;
        }
        default:;
        }

        return aiReturn_FAILURE;
    }

    size_t ZipFile::Tell() const {
        return m_SeekPtr;
    }

    // ----------------------------------------------------------------
    // pImpl of the Zip Archive IO
    class ZipArchiveIOSystem::Implement {
    public:
        static const unsigned int FileNameSize = 256;

        Implement(IOSystem* pIOHandler, const char* pFilename, const char* pMode);
        ~Implement();

        bool isOpen() const;
        void getFileList(std::vector<std::string>& rFileList);
        void getFileListExtension(std::vector<std::string>& rFileList, const std::string& extension);
        bool Exists(std::string& filename);
        IOStream* OpenFile(std::string& filename);

        static void SimplifyFilename(std::string& filename);

    private:
        void MapArchive();

    private:
        typedef std::map<std::string, ZipFileInfo> ZipFileInfoMap;

        unzFile m_ZipFileHandle = nullptr;
        ZipFileInfoMap m_ArchiveMap;
    };

    ZipArchiveIOSystem::Implement::Implement(IOSystem* pIOHandler, const char* pFilename, const char* pMode) {
        ai_assert(strcmp(pMode, "r") == 0);
        ai_assert(pFilename != nullptr);
        if (pFilename[0] == 0)
            return;

        zlib_filefunc_def mapping = IOSystem2Unzip::get(pIOHandler);
        m_ZipFileHandle = unzOpen2(pFilename, &mapping);
    }

    ZipArchiveIOSystem::Implement::~Implement() {
        m_ArchiveMap.clear();

        if (m_ZipFileHandle != nullptr) {
            unzClose(m_ZipFileHandle);
            m_ZipFileHandle = nullptr;
        }
    }

    void ZipArchiveIOSystem::Implement::MapArchive() {
        if (m_ZipFileHandle == nullptr)
            return;

        if (!m_ArchiveMap.empty())
            return;

        //  At first ensure file is already open
        if (unzGoToFirstFile(m_ZipFileHandle) != UNZ_OK)
            return;

        // Loop over all files
        do {
            char filename[FileNameSize];
            unz_file_info fileInfo;

            if (unzGetCurrentFileInfo(m_ZipFileHandle, &fileInfo, filename, FileNameSize, nullptr, 0, nullptr, 0) == UNZ_OK) {
                if (fileInfo.uncompressed_size != 0) {
                    std::string filename_string(filename, fileInfo.size_filename);
                    SimplifyFilename(filename_string);
                    m_ArchiveMap.emplace(filename_string, ZipFileInfo(m_ZipFileHandle, fileInfo.uncompressed_size));
                }
            }
        } while (unzGoToNextFile(m_ZipFileHandle) != UNZ_END_OF_LIST_OF_FILE);
    }

    bool ZipArchiveIOSystem::Implement::isOpen() const {
        return (m_ZipFileHandle != nullptr);
    }

    void ZipArchiveIOSystem::Implement::getFileList(std::vector<std::string>& rFileList) {
        MapArchive();
        rFileList.clear();

        for (const auto &file : m_ArchiveMap) {
            rFileList.push_back(file.first);
        }
    }

    void ZipArchiveIOSystem::Implement::getFileListExtension(std::vector<std::string>& rFileList, const std::string& extension) {
        MapArchive();
        rFileList.clear();

        for (const auto &file : m_ArchiveMap) {
            if (extension == BaseImporter::GetExtension(file.first))
                rFileList.push_back(file.first);
        }
    }

    bool ZipArchiveIOSystem::Implement::Exists(std::string& filename) {
        MapArchive();

        ZipFileInfoMap::const_iterator it = m_ArchiveMap.find(filename);
        return (it != m_ArchiveMap.end());
    }

    IOStream * ZipArchiveIOSystem::Implement::OpenFile(std::string& filename) {
        MapArchive();

        SimplifyFilename(filename);

        // Find in the map
        ZipFileInfoMap::const_iterator zip_it = m_ArchiveMap.find(filename);
        if (zip_it == m_ArchiveMap.cend())
            return nullptr;

        const ZipFileInfo &zip_file = (*zip_it).second;
        return zip_file.Extract(m_ZipFileHandle);
    }

    inline void ReplaceAll(std::string& data, const std::string& before, const std::string& after) {
        size_t pos = data.find(before);
        while (pos != std::string::npos)
        {
            data.replace(pos, before.size(), after);
            pos = data.find(before, pos + after.size());
        }
    }

    inline void ReplaceAllChar(std::string& data, const char before, const char after) {
        size_t pos = data.find(before);
        while (pos != std::string::npos)
        {
            data[pos] = after;
            pos = data.find(before, pos + 1);
        }
    }

    void ZipArchiveIOSystem::Implement::SimplifyFilename(std::string& filename)
    {
        ReplaceAllChar(filename, '\\', '/');

        // Remove all . and / from the beginning of the path
        size_t pos = filename.find_first_not_of("./");
        if (pos != 0)
            filename.erase(0, pos);

        // Simplify "my/folder/../file.png" constructions, if any
        static const std::string relative("/../");
        const size_t relsize = relative.size() - 1;
        pos = filename.find(relative);
        while (pos != std::string::npos)
        {
            // Previous slash
            size_t prevpos = filename.rfind('/', pos - 1);
            if (prevpos == pos)
                filename.erase(0, pos + relative.size());
            else
                filename.erase(prevpos, pos + relsize - prevpos);

            pos = filename.find(relative);
        }
    }

    ZipArchiveIOSystem::ZipArchiveIOSystem(IOSystem* pIOHandler, const char* pFilename, const char* pMode)
        : pImpl(new Implement(pIOHandler, pFilename, pMode)) {
    }

    // ----------------------------------------------------------------
    // The ZipArchiveIO
    ZipArchiveIOSystem::ZipArchiveIOSystem(IOSystem* pIOHandler, const std::string& rFilename, const char* pMode)
        : pImpl(new Implement(pIOHandler, rFilename.c_str(), pMode))
    {
    }

    ZipArchiveIOSystem::~ZipArchiveIOSystem() {
        delete pImpl;
    }

    bool ZipArchiveIOSystem::Exists(const char* pFilename) const {
        ai_assert(pFilename != nullptr);

        if (pFilename == nullptr) {
            return false;
        }

        std::string filename(pFilename);
        return pImpl->Exists(filename);
    }

    // This is always '/' in a ZIP
    char ZipArchiveIOSystem::getOsSeparator() const {
        return '/';
    }

    // Only supports Reading
    IOStream * ZipArchiveIOSystem::Open(const char* pFilename, const char* pMode) {
        ai_assert(pFilename != nullptr);

        for (size_t i = 0; pMode[i] != 0; ++i)
        {
            ai_assert(pMode[i] != 'w');
            if (pMode[i] == 'w')
                return nullptr;
        }

        std::string filename(pFilename);
        return pImpl->OpenFile(filename);
    }

    void ZipArchiveIOSystem::Close(IOStream* pFile) {
        delete pFile;
    }

    bool ZipArchiveIOSystem::isOpen() const {
        return (pImpl->isOpen());
    }

    void ZipArchiveIOSystem::getFileList(std::vector<std::string>& rFileList) const {
        return pImpl->getFileList(rFileList);
    }

    void ZipArchiveIOSystem::getFileListExtension(std::vector<std::string>& rFileList, const std::string& extension) const {
        return pImpl->getFileListExtension(rFileList, extension);
    }

    bool ZipArchiveIOSystem::isZipArchive(IOSystem* pIOHandler, const char* pFilename) {
        Implement tmp(pIOHandler, pFilename, "r");
        return tmp.isOpen();
    }

    bool ZipArchiveIOSystem::isZipArchive(IOSystem* pIOHandler, const std::string& rFilename) {
        return isZipArchive(pIOHandler, rFilename.c_str());
    }

}
