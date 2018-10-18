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

#ifndef ASSIMP_BUILD_NO_3MF_IMPORTER

#include "D3MFOpcPackage.h"
#include <assimp/Exceptional.h>

#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/ai_assert.h>

#include <cstdlib>
#include <memory>
#include <vector>
#include <map>
#include <algorithm>
#include <cassert>
#include <unzip.h>
#include "3MFXmlTags.h"

namespace Assimp {

namespace D3MF {

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
    mapping.opaque = reinterpret_cast<voidpf>(pIOHandler);

    return mapping;
}

class ZipFile : public IOStream {
    friend class D3MFZipArchive;

public:
    explicit ZipFile(size_t size);
    virtual ~ZipFile();
    size_t Read(void* pvBuffer, size_t pSize, size_t pCount );
    size_t Write(const void* /*pvBuffer*/, size_t /*pSize*/, size_t /*pCount*/);
    size_t FileSize() const;
    aiReturn Seek(size_t /*pOffset*/, aiOrigin /*pOrigin*/);
    size_t Tell() const;
    void Flush();

private:
    void *m_Buffer;
    size_t m_Size;
};

ZipFile::ZipFile(size_t size)
: m_Buffer( nullptr )
, m_Size(size) {
    ai_assert(m_Size != 0);
    m_Buffer = ::malloc(m_Size);
}

ZipFile::~ZipFile() {
    ::free(m_Buffer);
    m_Buffer = NULL;
}

size_t ZipFile::Read(void* pvBuffer, size_t pSize, size_t pCount) {
    const size_t size = pSize * pCount;
    ai_assert(size <= m_Size);

    std::memcpy(pvBuffer, m_Buffer, size);

    return size;
}

size_t ZipFile::Write(const void* pvBuffer, size_t size, size_t pCount ) {
    const size_t size_to_write( size * pCount );
    if ( 0 == size_to_write ) {
        return 0U;
    }
    return 0U;
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

class D3MFZipArchive : public IOSystem {
public:
    static const unsigned int FileNameSize = 256;

    D3MFZipArchive(IOSystem* pIOHandler, const std::string & rFile);
    ~D3MFZipArchive();
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
//  Constructor.
D3MFZipArchive::D3MFZipArchive(IOSystem* pIOHandler, const std::string& rFile)
: m_ZipFileHandle( nullptr )
, m_ArchiveMap() {
    if (! rFile.empty()) {                
        zlib_filefunc_def mapping = IOSystem2Unzip::get(pIOHandler);            

        m_ZipFileHandle = unzOpen2(rFile.c_str(), &mapping);
        if(m_ZipFileHandle != nullptr ) {
            mapArchive();
        }
    }
}

// ------------------------------------------------------------------------------------------------
//  Destructor.
D3MFZipArchive::~D3MFZipArchive() {
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
bool D3MFZipArchive::isOpen() const {
    return (m_ZipFileHandle != nullptr );
}

// ------------------------------------------------------------------------------------------------
//  Returns true, if the filename is part of the archive.
bool D3MFZipArchive::Exists(const char* pFile) const {
    ai_assert(pFile != nullptr );

    if ( pFile == nullptr ) {
        return false;
    }

    std::string filename(pFile);
    std::map<std::string, ZipFile*>::const_iterator it = m_ArchiveMap.find(filename);
    bool exist( false );
    if(it != m_ArchiveMap.end()) {
        exist = true;
    }

    return exist;
}

// ------------------------------------------------------------------------------------------------
//  Returns the separator delimiter.
char D3MFZipArchive::getOsSeparator() const {
#ifndef _WIN32
    return '/';
#else
    return '\\';
#endif
}

// ------------------------------------------------------------------------------------------------
//  Opens a file, which is part of the archive.
IOStream *D3MFZipArchive::Open(const char* pFile, const char* /*pMode*/) {
    ai_assert(pFile != NULL);

    IOStream* result = NULL;

    std::map<std::string, ZipFile*>::iterator it = m_ArchiveMap.find(pFile);

    if(it != m_ArchiveMap.end()) {
        result = static_cast<IOStream*>(it->second);
    }

    return result;
}

// ------------------------------------------------------------------------------------------------
//  Close a filestream.
void D3MFZipArchive::Close(IOStream *pFile) {
    (void)(pFile);
    ai_assert(pFile != NULL);

    // We don't do anything in case the file would be opened again in the future
}
// ------------------------------------------------------------------------------------------------
//  Returns the file-list of the archive.
void D3MFZipArchive::getFileList(std::vector<std::string> &rFileList) {
    rFileList.clear();

    for(const auto &file : m_ArchiveMap) {
        rFileList.push_back(file.first);
    }
}

// ------------------------------------------------------------------------------------------------
//  Maps the archive content.
bool D3MFZipArchive::mapArchive() {
    bool success = false;

    if(m_ZipFileHandle != NULL) {
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

typedef std::shared_ptr<OpcPackageRelationship> OpcPackageRelationshipPtr;

class OpcPackageRelationshipReader {
public:
    OpcPackageRelationshipReader(XmlReader* xmlReader) {        
        while(xmlReader->read()) {
            if(xmlReader->getNodeType() == irr::io::EXN_ELEMENT &&
               xmlReader->getNodeName() == XmlTag::RELS_RELATIONSHIP_CONTAINER)
            {
                ParseRootNode(xmlReader);
            }
        }
    }

    void ParseRootNode(XmlReader* xmlReader)
    {       
        ParseAttributes(xmlReader);

        while(xmlReader->read())
        {
            if(xmlReader->getNodeType() == irr::io::EXN_ELEMENT &&
               xmlReader->getNodeName() == XmlTag::RELS_RELATIONSHIP_NODE)
            {
                ParseChildNode(xmlReader);
            }
        }
    }

    void ParseAttributes(XmlReader*) {
        // empty
    }

    bool validateRels( OpcPackageRelationshipPtr &relPtr ) {
        if ( relPtr->id.empty() || relPtr->type.empty() || relPtr->target.empty() ) {
            return false;
        }
        return true;
    }

    void ParseChildNode(XmlReader* xmlReader) {        
        OpcPackageRelationshipPtr relPtr(new OpcPackageRelationship());

        relPtr->id = xmlReader->getAttributeValueSafe(XmlTag::RELS_ATTRIB_ID.c_str());
        relPtr->type = xmlReader->getAttributeValueSafe(XmlTag::RELS_ATTRIB_TYPE.c_str());
        relPtr->target = xmlReader->getAttributeValueSafe(XmlTag::RELS_ATTRIB_TARGET.c_str());
        if ( validateRels( relPtr ) ) {
            m_relationShips.push_back( relPtr );
        }
    }

    std::vector<OpcPackageRelationshipPtr> m_relationShips;
};

// ------------------------------------------------------------------------------------------------
D3MFOpcPackage::D3MFOpcPackage(IOSystem* pIOHandler, const std::string& rFile)
: mRootStream(nullptr)
, mZipArchive() {    
    mZipArchive.reset( new D3MF::D3MFZipArchive( pIOHandler, rFile ) );    
    if(!mZipArchive->isOpen()) {
        throw DeadlyImportError("Failed to open file " + rFile+ ".");
    }

    std::vector<std::string> fileList;
    mZipArchive->getFileList(fileList);

    for (auto& file: fileList) {
        if(file == D3MF::XmlTag::ROOT_RELATIONSHIPS_ARCHIVE) {
            //PkgRelationshipReader pkgRelReader(file, archive);
            ai_assert(mZipArchive->Exists(file.c_str()));

            IOStream *fileStream = mZipArchive->Open(file.c_str());

            ai_assert(fileStream != nullptr);

            std::string rootFile = ReadPackageRootRelationship(fileStream);
            if ( rootFile.size() > 0 && rootFile[ 0 ] == '/' ) {
                rootFile = rootFile.substr( 1 );
                if ( rootFile[ 0 ] == '/' ) {
                    // deal with zip-bug
                    rootFile = rootFile.substr( 1 );
                }
            }

            ASSIMP_LOG_DEBUG(rootFile);

            mRootStream = mZipArchive->Open(rootFile.c_str());
            ai_assert( mRootStream != nullptr );
            if ( nullptr == mRootStream ) {
                throw DeadlyExportError( "Cannot open root-file in archive : " + rootFile );
            }

            mZipArchive->Close( fileStream );

        } else if( file == D3MF::XmlTag::CONTENT_TYPES_ARCHIVE) {

        }
    }
}

D3MFOpcPackage::~D3MFOpcPackage() {
    // empty
}

IOStream* D3MFOpcPackage::RootStream() const {
    return mRootStream;
}

static const std::string ModelRef = "3D/3dmodel.model";

bool D3MFOpcPackage::validate() {
    if ( nullptr == mRootStream || nullptr == mZipArchive ) {
        return false;
    }

    return mZipArchive->Exists( ModelRef.c_str() );
}

bool D3MFOpcPackage::isZipArchive( IOSystem* pIOHandler, const std::string& rFile ) {
    D3MF::D3MFZipArchive ar( pIOHandler, rFile );
    if ( !ar.isOpen() ) {
        return false;
    }

    return true;
}

std::string D3MFOpcPackage::ReadPackageRootRelationship(IOStream* stream) {
    std::unique_ptr<CIrrXML_IOStreamReader> xmlStream(new CIrrXML_IOStreamReader(stream));
    std::unique_ptr<XmlReader> xml(irr::io::createIrrXMLReader(xmlStream.get()));

    OpcPackageRelationshipReader reader(xml.get());

    auto itr = std::find_if(reader.m_relationShips.begin(), reader.m_relationShips.end(), [](const OpcPackageRelationshipPtr& rel){
        return rel->type == XmlTag::PACKAGE_START_PART_RELATIONSHIP_TYPE;
    });

    if ( itr == reader.m_relationShips.end() ) {
        throw DeadlyImportError( "Cannot find " + XmlTag::PACKAGE_START_PART_RELATIONSHIP_TYPE );
    }

    return (*itr)->target;
}

} // Namespace D3MF
} // Namespace Assimp

#endif //ASSIMP_BUILD_NO_3MF_IMPORTER
