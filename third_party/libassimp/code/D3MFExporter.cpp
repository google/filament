/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team

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
#ifndef ASSIMP_BUILD_NO_EXPORT
#ifndef ASSIMP_BUILD_NO_3MF_EXPORTER

#include "D3MFExporter.h"

#include <assimp/scene.h>
#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/DefaultLogger.hpp>

#include "Exceptional.h"
#include "3MFXmlTags.h"
#include "D3MFOpcPackage.h"

#include <contrib/zip/src/zip.h>

namespace Assimp {

void ExportScene3MF( const char* pFile, IOSystem* pIOSystem, const aiScene* pScene, const ExportProperties* /*pProperties*/ ) {
    if ( nullptr == pIOSystem ) {
        throw DeadlyExportError( "Could not export 3MP archive: " + std::string( pFile ) );
    }
    D3MF::D3MFExporter myExporter( pFile, pScene );
    if ( myExporter.validate() ) {
        if ( pIOSystem->Exists( pFile ) ) {
            if ( !pIOSystem->DeleteFile( pFile ) ) {
                throw DeadlyExportError( "File exists, cannot override : " + std::string( pFile ) );
            }
        }
        bool ok = myExporter.exportArchive(pFile);
        if ( !ok ) {
            throw DeadlyExportError( "Could not export 3MP archive: " + std::string( pFile ) );
        }
    }
}

namespace D3MF {

D3MFExporter::D3MFExporter( const char* pFile, const aiScene* pScene )
: mArchiveName( pFile )
, m_zipArchive( nullptr )
, mScene( pScene )
, mModelOutput()
, mRelOutput()
, mContentOutput()
, mBuildItems()
, mRelations() {
    // empty
}

D3MFExporter::~D3MFExporter() {
    for ( size_t i = 0; i < mRelations.size(); ++i ) {
        delete mRelations[ i ];
    }
    mRelations.clear();
}

bool D3MFExporter::validate() {
    if ( mArchiveName.empty() ) {
        return false;
    }

    if ( nullptr == mScene ) {
        return false;
    }

    return true;
}

bool D3MFExporter::exportArchive( const char *file ) {
    bool ok( true );

    m_zipArchive = zip_open( file, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w' );
    if ( nullptr == m_zipArchive ) {
        return false;
    }
    ok |= exportContentTypes();
    ok |= export3DModel();
    ok |= exportRelations();

    zip_close( m_zipArchive );
    m_zipArchive = nullptr;

    return ok;
}


bool D3MFExporter::exportContentTypes() {
    mContentOutput.clear();

    mContentOutput << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    mContentOutput << std::endl;
    mContentOutput << "<Types xmlns = \"http://schemas.openxmlformats.org/package/2006/content-types\">";
    mContentOutput << std::endl;
    mContentOutput << "<Default Extension = \"rels\" ContentType = \"application/vnd.openxmlformats-package.relationships+xml\" />";
    mContentOutput << std::endl;
    mContentOutput << "<Default Extension = \"model\" ContentType = \"application/vnd.ms-package.3dmanufacturing-3dmodel+xml\" />";
    mContentOutput << std::endl;
    mContentOutput << "</Types>";
    mContentOutput << std::endl;
    exportContentTyp( XmlTag::CONTENT_TYPES_ARCHIVE );

    return true;
}

bool D3MFExporter::exportRelations() {
    mRelOutput.clear();

    mRelOutput << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    mRelOutput << std::endl;
    mRelOutput << "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">";

    for ( size_t i = 0; i < mRelations.size(); ++i ) {
        mRelOutput << "<Relationship Target=\"/" << mRelations[ i ]->target << "\" ";
        mRelOutput << "Id=\"" << mRelations[i]->id << "\" ";
        mRelOutput << "Type=\"" << mRelations[ i ]->type << "\" />";
        mRelOutput << std::endl;
    }
    mRelOutput << "</Relationships>";
    mRelOutput << std::endl;

    writeRelInfoToFile( "_rels", ".rels" );
    mRelOutput.flush();

    return true;
}

bool D3MFExporter::export3DModel() {
    mModelOutput.clear();

    writeHeader();
    mModelOutput << "<" << XmlTag::model << " " << XmlTag::model_unit << "=\"millimeter\""
            << "xmlns=\"http://schemas.microsoft.com/3dmanufacturing/core/2015/02\">"
            << std::endl;
    mModelOutput << "<" << XmlTag::resources << ">";
    mModelOutput << std::endl;

    writeObjects();


    mModelOutput << "</" << XmlTag::resources << ">";
    mModelOutput << std::endl;
    writeBuild();

    mModelOutput << "</" << XmlTag::model << ">\n";

    OpcPackageRelationship *info = new OpcPackageRelationship;
    info->id = "rel0";
    info->target = "/3D/3DModel.model";
    info->type = XmlTag::PACKAGE_START_PART_RELATIONSHIP_TYPE;
    mRelations.push_back( info );

    writeModelToArchive( "3D", "3DModel.model" );
    mModelOutput.flush();

    return true;
}

void D3MFExporter::writeHeader() {
    mModelOutput << "<?xml version=\"1.0\" encoding=\"UTF - 8\"?>";
    mModelOutput << std::endl;
}

void D3MFExporter::writeObjects() {
    if ( nullptr == mScene->mRootNode ) {
        return;
    }

    aiNode *root = mScene->mRootNode;
    for ( unsigned int i = 0; i < root->mNumChildren; ++i ) {
        aiNode *currentNode( root->mChildren[ i ] );
        if ( nullptr == currentNode ) {
            continue;
        }
        mModelOutput << "<" << XmlTag::object << " id=\"" << currentNode->mName.C_Str() << "\" type=\"model\">";
        mModelOutput << std::endl;
        for ( unsigned int j = 0; j < currentNode->mNumMeshes; ++j ) {
            aiMesh *currentMesh = mScene->mMeshes[ currentNode->mMeshes[ j ] ];
            if ( nullptr == currentMesh ) {
                continue;
            }
            writeMesh( currentMesh );
        }
        mBuildItems.push_back( i );

        mModelOutput << "</" << XmlTag::object << ">";
        mModelOutput << std::endl;
    }
}

void D3MFExporter::writeMesh( aiMesh *mesh ) {
    if ( nullptr == mesh ) {
        return;
    }

    mModelOutput << "<" << XmlTag::mesh << ">" << std::endl;
    mModelOutput << "<" << XmlTag::vertices << ">" << std::endl;
    for ( unsigned int i = 0; i < mesh->mNumVertices; ++i ) {
        writeVertex( mesh->mVertices[ i ] );
    }
    mModelOutput << "</" << XmlTag::vertices << ">" << std::endl;

    writeFaces( mesh );

    mModelOutput << "</" << XmlTag::mesh << ">" << std::endl;
}

void D3MFExporter::writeVertex( const aiVector3D &pos ) {
    mModelOutput << "<" << XmlTag::vertex << " x=\"" << pos.x << "\" y=\"" << pos.y << "\" z=\"" << pos.z << "\" />";
    mModelOutput << std::endl;
}

void D3MFExporter::writeFaces( aiMesh *mesh ) {
    if ( nullptr == mesh ) {
        return;
    }

    if ( !mesh->HasFaces() ) {
        return;
    }
    mModelOutput << "<" << XmlTag::triangles << ">" << std::endl;
    for ( unsigned int i = 0; i < mesh->mNumFaces; ++i ) {
        aiFace &currentFace = mesh->mFaces[ i ];
        mModelOutput << "<" << XmlTag::triangle << " v1=\"" << currentFace.mIndices[ 0 ] << "\" v2=\""
                << currentFace.mIndices[ 1 ] << "\" v3=\"" << currentFace.mIndices[ 2 ] << "\"/>";
        mModelOutput << std::endl;
    }
    mModelOutput << "</" << XmlTag::triangles << ">";
    mModelOutput << std::endl;
}

void D3MFExporter::writeBuild() {
    mModelOutput << "<" << XmlTag::build << ">" << std::endl;

    for ( size_t i = 0; i < mBuildItems.size(); ++i ) {
        mModelOutput << "<" << XmlTag::item << " objectid=\"" << i + 1 << "\"/>";
        mModelOutput << std::endl;
    }
    mModelOutput << "</" << XmlTag::build << ">";
    mModelOutput << std::endl;
}

void D3MFExporter::exportContentTyp( const std::string &filename ) {
    if ( nullptr == m_zipArchive ) {
        throw DeadlyExportError( "3MF-Export: Zip archive not valid, nullptr." );
    }
    const std::string entry = filename;
    zip_entry_open( m_zipArchive, entry.c_str() );

    const std::string &exportTxt( mContentOutput.str() );
    zip_entry_write( m_zipArchive, exportTxt.c_str(), exportTxt.size() );

    zip_entry_close( m_zipArchive );
}

void D3MFExporter::writeModelToArchive( const std::string &folder, const std::string &modelName ) {
    if ( nullptr == m_zipArchive ) {
        throw DeadlyExportError( "3MF-Export: Zip archive not valid, nullptr." );
    }
    const std::string entry = folder + "/" + modelName;
    zip_entry_open( m_zipArchive, entry.c_str() );

    const std::string &exportTxt( mModelOutput.str() );
    zip_entry_write( m_zipArchive, exportTxt.c_str(), exportTxt.size() );

    zip_entry_close( m_zipArchive );
}

void D3MFExporter::writeRelInfoToFile( const std::string &folder, const std::string &relName ) {
    if ( nullptr == m_zipArchive ) {
        throw DeadlyExportError( "3MF-Export: Zip archive not valid, nullptr." );
    }
    const std::string entry = folder + "/" + relName;
    zip_entry_open( m_zipArchive, entry.c_str() );

    const std::string &exportTxt( mRelOutput.str() );
    zip_entry_write( m_zipArchive, exportTxt.c_str(), exportTxt.size() );

    zip_entry_close( m_zipArchive );
}


} // Namespace D3MF
} // Namespace Assimp

#endif // ASSIMP_BUILD_NO3MF_EXPORTER
#endif // ASSIMP_BUILD_NO_EXPORT
