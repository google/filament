/*
Open Asset Import Library (assimp)
---------------------------------------------------------------------------------------------------

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

---------------------------------------------------------------------------------------------------
*/

#ifndef ASSIMP_BUILD_NO_Q3BSP_IMPORTER

#include "Q3BSPFileImporter.h"
#include "Q3BSPFileParser.h"
#include "Q3BSPFileData.h"

#include <assimp/DefaultLogger.hpp>

#ifdef ASSIMP_BUILD_NO_OWN_ZLIB
#   include <zlib.h>
#else
#   include "../contrib/zlib/zlib.h"
#endif

#include <assimp/types.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/ai_assert.h>
#include <assimp/DefaultIOSystem.h>
#include <assimp/ZipArchiveIOSystem.h>
#include <assimp/importerdesc.h>
#include <vector>
#include <sstream>
#include <assimp/StringComparison.h>

static const aiImporterDesc desc = {
    "Quake III BSP Importer",
    "",
    "",
    "",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "pk3"
};

namespace Assimp {

using namespace Q3BSP;

// ------------------------------------------------------------------------------------------------
//  Local function to create a material key name.
static void createKey( int id1, int id2, std::string &key ) {
    std::ostringstream str;
    str << id1 << "." << id2;
    key = str.str();
}

// ------------------------------------------------------------------------------------------------
//  Local function to extract the texture ids from a material key-name.
static void extractIds( const std::string &key, int &id1, int &id2 ) {
    id1 = -1;
    id2 = -1;
    if (key.empty()) {
        return;
    }

    const std::string::size_type pos = key.find( "." );
    if (std::string::npos == pos) {
        return;
    }

    std::string tmp1 = key.substr( 0, pos );
    std::string tmp2 = key.substr( pos + 1, key.size() - pos - 1 );
    id1 = atoi( tmp1.c_str() );
    id2 = atoi( tmp2.c_str() );
}

// ------------------------------------------------------------------------------------------------
//  Local helper function to normalize filenames.
static void normalizePathName( const std::string &rPath, std::string &normalizedPath ) {
    normalizedPath = "";
    if (rPath.empty()) {
        return;
    }

#ifdef _WIN32
    std::string sep = "\\";
#else
    std::string sep = "/";
#endif

    static const unsigned int numDelimiters = 2;
    const char delimiters[ numDelimiters ] = { '/', '\\' };
    normalizedPath = rPath;
    for (const char delimiter : delimiters) {
        for ( size_t j=0; j<normalizedPath.size(); ++j ) {
            if ( normalizedPath[j] == delimiter ) {
                normalizedPath[ j ] = sep[ 0 ];
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
//  Constructor.
Q3BSPFileImporter::Q3BSPFileImporter()
: m_pCurrentMesh( nullptr )
, m_pCurrentFace(nullptr)
, m_MaterialLookupMap()
, mTextures() {
    // empty
}

// ------------------------------------------------------------------------------------------------
//  Destructor.
Q3BSPFileImporter::~Q3BSPFileImporter() {
    m_pCurrentMesh = nullptr;
    m_pCurrentFace = nullptr;

    // Clear face-to-material map
    for ( FaceMap::iterator it = m_MaterialLookupMap.begin(); it != m_MaterialLookupMap.end(); ++it ) {
        const std::string &matName = it->first;
        if ( !matName.empty() ) {
            delete it->second;
        }
    }
    m_MaterialLookupMap.clear();
}

// ------------------------------------------------------------------------------------------------
//  Returns true, if the loader can read this.
bool Q3BSPFileImporter::CanRead( const std::string& rFile, IOSystem* /*pIOHandler*/, bool checkSig ) const {
    if(!checkSig) {
        return SimpleExtensionCheck( rFile, "pk3", "bsp" );
    }

    return false;
}

// ------------------------------------------------------------------------------------------------
//  Adds extensions.
const aiImporterDesc* Q3BSPFileImporter::GetInfo () const {
    return &desc;
}

// ------------------------------------------------------------------------------------------------
//  Import method.
void Q3BSPFileImporter::InternReadFile(const std::string &rFile, aiScene* scene, IOSystem* ioHandler) {
    ZipArchiveIOSystem Archive( ioHandler, rFile );
    if ( !Archive.isOpen() ) {
        throw DeadlyImportError( "Failed to open file " + rFile + "." );
    }

    std::string archiveName( "" ), mapName( "" );
    separateMapName( rFile, archiveName, mapName );

    if ( mapName.empty() ) {
        if ( !findFirstMapInArchive( Archive, mapName ) ) {
            return;
        }
    }

    Q3BSPFileParser fileParser( mapName, &Archive );
    Q3BSPModel *pBSPModel = fileParser.getModel();
    if ( nullptr != pBSPModel ) {
        CreateDataFromImport( pBSPModel, scene, &Archive );
    }
}

// ------------------------------------------------------------------------------------------------
//  Separates the map name from the import name.
void Q3BSPFileImporter::separateMapName( const std::string &importName, std::string &archiveName, std::string &mapName ) {
    archiveName = "";
    mapName = "";
    if (importName.empty()) {
        return;
    }

    const std::string::size_type pos = importName.rfind( "," );
    if ( std::string::npos == pos ) {
        archiveName = importName;
        return;
    }

    archiveName = importName.substr( 0, pos );
    mapName = importName.substr( pos, importName.size() - pos - 1 );
}

// ------------------------------------------------------------------------------------------------
//  Returns the first map in the map archive.
bool Q3BSPFileImporter::findFirstMapInArchive(ZipArchiveIOSystem &bspArchive, std::string &mapName ) {
    mapName = "";
    std::vector<std::string> fileList;
    bspArchive.getFileListExtension( fileList, "bsp" );
    if (fileList.empty()) {
        return false;
    }

    std::vector<std::string>::iterator it( fileList.begin() );
    for ( ; it != fileList.end(); ++it ) {
        const std::string::size_type pos = (*it).find( "maps/" );
        if ( std::string::npos != pos ) {
            std::string::size_type extPos = (*it).find( ".bsp" );
            if ( std::string::npos != extPos ) {
                mapName = *it;
                return true;
            }
        }
    }

    return false;
}

// ------------------------------------------------------------------------------------------------
//  Creates the assimp specific data.
void Q3BSPFileImporter::CreateDataFromImport( const Q3BSP::Q3BSPModel *pModel, aiScene* pScene,
    ZipArchiveIOSystem *pArchive ) {
    if (nullptr == pModel || nullptr == pScene) {
        return;
    }

    pScene->mRootNode = new aiNode;
    if ( !pModel->m_ModelName.empty() ) {
        pScene->mRootNode->mName.Set( pModel->m_ModelName );
    }

    // Create the face to material relation map
    createMaterialMap( pModel );

    // Create all nodes
    CreateNodes( pModel, pScene, pScene->mRootNode );

    // Create the assigned materials
    createMaterials( pModel, pScene, pArchive );
}

// ------------------------------------------------------------------------------------------------
//  Creates all assimp nodes.
void Q3BSPFileImporter::CreateNodes( const Q3BSP::Q3BSPModel *pModel, aiScene* pScene,
        aiNode *pParent ) {
    if ( nullptr == pModel ) {
        return;
    }

    unsigned int matIdx( 0 );
    std::vector<aiMesh*> MeshArray;
    std::vector<aiNode*> NodeArray;
    for ( FaceMapIt it = m_MaterialLookupMap.begin(); it != m_MaterialLookupMap.end(); ++it ) {
        std::vector<Q3BSP::sQ3BSPFace*> *pArray = (*it).second;
        size_t numVerts = countData( *pArray );
        if ( 0 != numVerts ) {
            aiMesh *pMesh( nullptr );
            aiNode *pNode = CreateTopology( pModel, matIdx, *pArray, &pMesh );
            if ( nullptr != pNode ) {
                NodeArray.push_back( pNode );
                MeshArray.push_back( pMesh );
            }
        }
        matIdx++;
    }

    pScene->mNumMeshes = static_cast<unsigned int>( MeshArray.size() );
    if ( pScene->mNumMeshes > 0 ) {
        pScene->mMeshes = new aiMesh*[ pScene->mNumMeshes ];
        for ( size_t i = 0; i < MeshArray.size(); i++ ) {
            aiMesh *pMesh = MeshArray[ i ];
            if ( nullptr != pMesh ) {
                pScene->mMeshes[ i ] = pMesh;
            }
        }
    }

    pParent->mNumChildren = static_cast<unsigned int>(MeshArray.size());
    pParent->mChildren = new aiNode*[ pScene->mRootNode->mNumChildren ];
    for ( size_t i=0; i<NodeArray.size(); i++ ) {
        aiNode *pNode = NodeArray[ i ];
        pNode->mParent = pParent;
        pParent->mChildren[ i ] = pNode;
        pParent->mChildren[ i ]->mMeshes[ 0 ] = static_cast<unsigned int>(i);
    }
}

// ------------------------------------------------------------------------------------------------
//  Creates the topology.
aiNode *Q3BSPFileImporter::CreateTopology( const Q3BSP::Q3BSPModel *pModel, unsigned int materialIdx,
        std::vector<sQ3BSPFace*> &rArray, aiMesh **pMesh ) {
    size_t numVerts = countData( rArray );
    if ( 0 == numVerts ) {
        return nullptr;
    }

    size_t numFaces = countFaces( rArray );
    if ( 0 == numFaces ) {
        return nullptr;
    }

    aiMesh *mesh = new aiMesh;
    size_t numTriangles = countTriangles( rArray );
    mesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;

    mesh->mFaces = new aiFace[ numTriangles ];
    mesh->mNumFaces = static_cast<unsigned int>(numTriangles);

    mesh->mNumVertices = static_cast<unsigned int>(numVerts);
    mesh->mVertices = new aiVector3D[ numVerts ];
    mesh->mNormals =  new aiVector3D[ numVerts ];
    mesh->mTextureCoords[ 0 ] = new aiVector3D[ numVerts ];
    mesh->mTextureCoords[ 1 ] = new aiVector3D[ numVerts ];
    mesh->mMaterialIndex = materialIdx;

    unsigned int faceIdx = 0;
    unsigned int vertIdx = 0;
    mesh->mNumUVComponents[ 0 ] = 2;
    mesh->mNumUVComponents[ 1 ] = 2;
    for ( std::vector<sQ3BSPFace*>::const_iterator it = rArray.begin(); it != rArray.end(); ++it ) {
        Q3BSP::sQ3BSPFace *pQ3BSPFace = *it;
        ai_assert( NULL != pQ3BSPFace );
        if ( nullptr == pQ3BSPFace ) {
            continue;
        }

        if ( pQ3BSPFace->iNumOfFaceVerts > 0 ) {
            if ( pQ3BSPFace->iType == Polygon || pQ3BSPFace->iType == TriangleMesh ) {
                createTriangleTopology( pModel, pQ3BSPFace, mesh, faceIdx, vertIdx );
            }
        }
    }

    aiNode *pNode = new aiNode;
    pNode->mNumMeshes = 1;
    pNode->mMeshes = new unsigned int[ 1 ];
    *pMesh = mesh;

    return pNode;
}

// ------------------------------------------------------------------------------------------------
//  Creates the triangle topology from a face array.
void Q3BSPFileImporter::createTriangleTopology( const Q3BSP::Q3BSPModel *pModel, sQ3BSPFace *pQ3BSPFace,
        aiMesh* pMesh, unsigned int &faceIdx, unsigned int &vertIdx ) {
    ai_assert( faceIdx < pMesh->mNumFaces );

    m_pCurrentFace = getNextFace( pMesh, faceIdx );
    if ( nullptr == m_pCurrentFace ) {
        return;
    }

    m_pCurrentFace->mNumIndices = 3;
    m_pCurrentFace->mIndices = new unsigned int[ m_pCurrentFace->mNumIndices ];

    size_t idx( 0 );
    for ( size_t i = 0; i < (size_t) pQ3BSPFace->iNumOfFaceVerts; ++i ) {
        const size_t index = pQ3BSPFace->iVertexIndex + pModel->m_Indices[ pQ3BSPFace->iFaceVertexIndex + i ];
        if ( index >= pModel->m_Vertices.size() ) {
            continue;
        }

        sQ3BSPVertex *pVertex = pModel->m_Vertices[ index ];
        if ( nullptr == pVertex ) {
            continue;
        }
        if (idx > 2) {
            idx = 0;
            m_pCurrentFace = getNextFace(pMesh, faceIdx);
            if (nullptr != m_pCurrentFace) {
                m_pCurrentFace->mNumIndices = 3;
                m_pCurrentFace->mIndices = new unsigned int[3];
                m_pCurrentFace->mIndices[ idx ] = vertIdx;
            }
        }

        pMesh->mVertices[ vertIdx ].Set( pVertex->vPosition.x, pVertex->vPosition.y, pVertex->vPosition.z );
        pMesh->mNormals[ vertIdx ].Set( pVertex->vNormal.x, pVertex->vNormal.y, pVertex->vNormal.z );

        pMesh->mTextureCoords[ 0 ][ vertIdx ].Set( pVertex->vTexCoord.x, pVertex->vTexCoord.y, 0.0f );
        pMesh->mTextureCoords[ 1 ][ vertIdx ].Set( pVertex->vLightmap.x, pVertex->vLightmap.y, 0.0f );

        vertIdx++;
        idx++;
    }
}

// ------------------------------------------------------------------------------------------------
//  Creates all referenced materials.
void Q3BSPFileImporter::createMaterials( const Q3BSP::Q3BSPModel *pModel, aiScene* pScene,
    ZipArchiveIOSystem *pArchive ) {
    if ( m_MaterialLookupMap.empty() ) {
        return;
    }

    pScene->mMaterials = new aiMaterial*[ m_MaterialLookupMap.size() ];
    aiString aiMatName;
    int textureId( -1 ), lightmapId( -1 );
    for ( FaceMapIt it = m_MaterialLookupMap.begin(); it != m_MaterialLookupMap.end();
        ++it ) {
        const std::string matName( it->first );
        if ( matName.empty() ) {
            continue;
        }

        aiMatName.Set( matName );
        aiMaterial *pMatHelper = new aiMaterial;
        pMatHelper->AddProperty( &aiMatName, AI_MATKEY_NAME );

        extractIds( matName, textureId, lightmapId );

        // Adding the texture
        if ( -1 != textureId ) {
            sQ3BSPTexture *pTexture = pModel->m_Textures[ textureId ];
            if ( nullptr != pTexture ) {
                std::string tmp( "*" ), texName( "" );
                tmp += pTexture->strName;
                tmp += ".jpg";
                normalizePathName( tmp, texName );

                if ( !importTextureFromArchive( pModel, pArchive, pScene, pMatHelper, textureId ) ) {
                    ASSIMP_LOG_ERROR("Cannot import texture from archive " + texName);
                }
            }

        }
        if ( -1 != lightmapId )
        {
            importLightmap( pModel, pScene, pMatHelper, lightmapId );
        }
        pScene->mMaterials[ pScene->mNumMaterials ] = pMatHelper;
        pScene->mNumMaterials++;
    }
    pScene->mNumTextures = static_cast<unsigned int>(mTextures.size());
    pScene->mTextures = new aiTexture*[ pScene->mNumTextures ];
    std::copy( mTextures.begin(), mTextures.end(), pScene->mTextures );
}

// ------------------------------------------------------------------------------------------------
//  Counts the number of referenced vertices.
size_t Q3BSPFileImporter::countData( const std::vector<sQ3BSPFace*> &faceArray ) const {
    size_t numVerts( 0 );
    for ( std::vector<sQ3BSPFace*>::const_iterator it = faceArray.begin(); it != faceArray.end();
        ++it )
    {
        sQ3BSPFace *pQ3BSPFace = *it;
        if ( pQ3BSPFace->iType == Polygon || pQ3BSPFace->iType == TriangleMesh )
        {
            Q3BSP::sQ3BSPFace *pQ3BSPFace = *it;
            ai_assert( nullptr != pQ3BSPFace );
            numVerts += pQ3BSPFace->iNumOfFaceVerts;
        }
    }

    return numVerts;
}

// ------------------------------------------------------------------------------------------------
//  Counts the faces with vertices.
size_t Q3BSPFileImporter::countFaces( const std::vector<Q3BSP::sQ3BSPFace*> &rArray ) const
{
    size_t numFaces = 0;
    for ( std::vector<sQ3BSPFace*>::const_iterator it = rArray.begin(); it != rArray.end();
        ++it )
    {
        Q3BSP::sQ3BSPFace *pQ3BSPFace = *it;
        if ( pQ3BSPFace->iNumOfFaceVerts > 0 )
        {
            numFaces++;
        }
    }

    return numFaces;
}

// ------------------------------------------------------------------------------------------------
//  Counts the number of triangles in a Q3-face-array.
size_t Q3BSPFileImporter::countTriangles( const std::vector<Q3BSP::sQ3BSPFace*> &rArray ) const
{
    size_t numTriangles = 0;
    for ( std::vector<Q3BSP::sQ3BSPFace*>::const_iterator it = rArray.begin(); it != rArray.end();
        ++it )
    {
        const Q3BSP::sQ3BSPFace *pQ3BSPFace = *it;
        if ( NULL != pQ3BSPFace )
        {
            numTriangles += pQ3BSPFace->iNumOfFaceVerts / 3;
        }
    }

    return numTriangles;
}

// ------------------------------------------------------------------------------------------------
//  Creates the faces-to-material map.
void Q3BSPFileImporter::createMaterialMap( const Q3BSP::Q3BSPModel *pModel ) {
    std::string key( "" );
    std::vector<sQ3BSPFace*> *pCurFaceArray = NULL;
    for ( size_t idx = 0; idx < pModel->m_Faces.size(); idx++ )
    {
        Q3BSP::sQ3BSPFace *pQ3BSPFace = pModel->m_Faces[ idx ];
        const int texId = pQ3BSPFace->iTextureID;
        const int lightMapId = pQ3BSPFace->iLightmapID;
        createKey( texId, lightMapId, key );
        FaceMapIt it = m_MaterialLookupMap.find( key );
        if ( m_MaterialLookupMap.end() == it ) {
            pCurFaceArray = new std::vector<Q3BSP::sQ3BSPFace*>;
            m_MaterialLookupMap[ key ] = pCurFaceArray;
        }
        else
        {
            pCurFaceArray = (*it).second;
        }
        ai_assert( nullptr != pCurFaceArray );
        if (nullptr != pCurFaceArray )
        {
            pCurFaceArray->push_back( pQ3BSPFace );
        }
    }
}

// ------------------------------------------------------------------------------------------------
//  Returns the next face.
aiFace *Q3BSPFileImporter::getNextFace( aiMesh *mesh, unsigned int &faceIdx ) {
    aiFace *face( nullptr );
    if ( faceIdx < mesh->mNumFaces ) {
        face = &mesh->mFaces[ faceIdx ];
        ++faceIdx;
    }

    return face;
}

// ------------------------------------------------------------------------------------------------
//  Imports a texture file.
bool Q3BSPFileImporter::importTextureFromArchive( const Q3BSP::Q3BSPModel *model,
                                                 ZipArchiveIOSystem *archive, aiScene*,
                                                 aiMaterial *pMatHelper, int textureId ) {
    if (nullptr == archive || nullptr == pMatHelper ) {
        return false;
    }

    if ( textureId < 0 || textureId >= static_cast<int>( model->m_Textures.size() ) ) {
        return false;
    }

    bool res = true;
    sQ3BSPTexture *pTexture = model->m_Textures[ textureId ];
    if ( !pTexture ) {
        return false;
    }

    std::vector<std::string> supportedExtensions;
    supportedExtensions.push_back( ".jpg" );
    supportedExtensions.push_back( ".png" );
    supportedExtensions.push_back( ".tga" );
    std::string textureName, ext;
    if ( expandFile( archive, pTexture->strName, supportedExtensions, textureName, ext ) ) {
        IOStream *pTextureStream = archive->Open( textureName.c_str() );
        if ( pTextureStream ) {
            size_t texSize = pTextureStream->FileSize();
            aiTexture *pTexture = new aiTexture;
            pTexture->mHeight = 0;
            pTexture->mWidth = static_cast<unsigned int>(texSize);
            unsigned char *pData = new unsigned char[ pTexture->mWidth ];
            size_t readSize = pTextureStream->Read( pData, sizeof( unsigned char ), pTexture->mWidth );
            (void)readSize;
            ai_assert( readSize == pTexture->mWidth );
            pTexture->pcData = reinterpret_cast<aiTexel*>( pData );
            pTexture->achFormatHint[ 0 ] = ext[ 1 ];
            pTexture->achFormatHint[ 1 ] = ext[ 2 ];
            pTexture->achFormatHint[ 2 ] = ext[ 3 ];
            pTexture->achFormatHint[ 3 ] = '\0';
            res = true;

            aiString name;
            name.data[ 0 ] = '*';
            name.length = 1 + ASSIMP_itoa10( name.data + 1, static_cast<unsigned int>(MAXLEN-1), static_cast<int32_t>(mTextures.size()) );

            archive->Close( pTextureStream );

            pMatHelper->AddProperty( &name, AI_MATKEY_TEXTURE_DIFFUSE( 0 ) );
            mTextures.push_back( pTexture );
        } else {
            // If it doesn't exist in the archive, it is probably just a reference to an external file.
            // We'll leave it up to the user to figure out which extension the file has.
            aiString name;
            strncpy( name.data, pTexture->strName, sizeof name.data );
            name.length = strlen( name.data );
            pMatHelper->AddProperty( &name, AI_MATKEY_TEXTURE_DIFFUSE( 0 ) );
        }
    }

    return res;
}

// ------------------------------------------------------------------------------------------------
//  Imports a light map file.
bool Q3BSPFileImporter::importLightmap( const Q3BSP::Q3BSPModel *pModel, aiScene* pScene,
                                       aiMaterial *pMatHelper, int lightmapId )
{
    if (nullptr == pModel || nullptr == pScene || nullptr == pMatHelper ) {
        return false;
    }

    if ( lightmapId < 0 || lightmapId >= static_cast<int>( pModel->m_Lightmaps.size() ) ) {
        return false;
    }

    sQ3BSPLightmap *pLightMap = pModel->m_Lightmaps[ lightmapId ];
    if (nullptr == pLightMap ) {
        return false;
    }

    aiTexture *pTexture = new aiTexture;

    pTexture->mWidth = CE_BSP_LIGHTMAPWIDTH;
    pTexture->mHeight = CE_BSP_LIGHTMAPHEIGHT;
    pTexture->pcData = new aiTexel[CE_BSP_LIGHTMAPWIDTH * CE_BSP_LIGHTMAPHEIGHT];

    ::memcpy( pTexture->pcData, pLightMap->bLMapData, pTexture->mWidth );
    size_t p = 0;
    for ( size_t i = 0; i < CE_BSP_LIGHTMAPWIDTH * CE_BSP_LIGHTMAPHEIGHT; ++i ) {
        pTexture->pcData[ i ].r = pLightMap->bLMapData[ p++ ];
        pTexture->pcData[ i ].g = pLightMap->bLMapData[ p++ ];
        pTexture->pcData[ i ].b = pLightMap->bLMapData[ p++ ];
        pTexture->pcData[ i ].a = 0xFF;
    }

    aiString name;
    name.data[ 0 ] = '*';
    name.length = 1 + ASSIMP_itoa10( name.data + 1, static_cast<unsigned int>(MAXLEN-1), static_cast<int32_t>(mTextures.size()) );

    pMatHelper->AddProperty( &name,AI_MATKEY_TEXTURE_LIGHTMAP( 1 ) );
    mTextures.push_back( pTexture );

    return true;
}

// ------------------------------------------------------------------------------------------------
//  Will search for a supported extension.
bool Q3BSPFileImporter::expandFile(ZipArchiveIOSystem *pArchive, const std::string &rFilename,
                                   const std::vector<std::string> &rExtList, std::string &rFile,
                                   std::string &rExt )
{
    ai_assert( NULL != pArchive );
    ai_assert( !rFilename.empty() );

    if ( rExtList.empty() )
    {
        rFile =  rFilename;
        rExt = "";
        return true;
    }

    bool found = false;
    for ( std::vector<std::string>::const_iterator it = rExtList.begin(); it != rExtList.end(); ++it )
    {
        const std::string textureName = rFilename + *it;
        if ( pArchive->Exists( textureName.c_str() ) )
        {
            rExt = *it;
            rFile = textureName;
            found = true;
            break;
        }
    }

    return found;
}

// ------------------------------------------------------------------------------------------------

} // Namespace Assimp

#endif // ASSIMP_BUILD_NO_Q3BSP_IMPORTER
