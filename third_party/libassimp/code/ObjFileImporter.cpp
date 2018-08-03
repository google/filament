/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team


All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

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
---------------------------------------------------------------------------
*/

#ifndef ASSIMP_BUILD_NO_OBJ_IMPORTER

#include "ObjFileImporter.h"
#include "ObjFileParser.h"
#include "ObjFileData.h"
#include "IOStreamBuffer.h"
#include <memory>
#include <assimp/DefaultIOSystem.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/ai_assert.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/importerdesc.h>

static const aiImporterDesc desc = {
    "Wavefront Object Importer",
    "",
    "",
    "surfaces not supported",
    aiImporterFlags_SupportTextFlavour,
    0,
    0,
    0,
    0,
    "obj"
};

static const unsigned int ObjMinSize = 16;

namespace Assimp {

using namespace std;

// ------------------------------------------------------------------------------------------------
//  Default constructor
ObjFileImporter::ObjFileImporter() :
    m_Buffer(),
    m_pRootObject( NULL ),
    m_strAbsPath( "" )
{
    DefaultIOSystem io;
    m_strAbsPath = io.getOsSeparator();
}

// ------------------------------------------------------------------------------------------------
//  Destructor.
ObjFileImporter::~ObjFileImporter()
{
    delete m_pRootObject;
    m_pRootObject = NULL;
}

// ------------------------------------------------------------------------------------------------
//  Returns true, if file is an obj file.
bool ObjFileImporter::CanRead( const std::string& pFile, IOSystem*  pIOHandler , bool checkSig ) const
{
    if(!checkSig) //Check File Extension
    {
        return SimpleExtensionCheck(pFile,"obj");
    }
    else //Check file Header
    {
        static const char *pTokens[] = { "mtllib", "usemtl", "v ", "vt ", "vn ", "o ", "g ", "s ", "f " };
        return BaseImporter::SearchFileHeaderForToken(pIOHandler, pFile, pTokens, 9 );
    }
}

// ------------------------------------------------------------------------------------------------
const aiImporterDesc* ObjFileImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
//  Obj-file import implementation
void ObjFileImporter::InternReadFile( const std::string &file, aiScene* pScene, IOSystem* pIOHandler) {
    // Read file into memory
    static const std::string mode = "rb";
    std::unique_ptr<IOStream> fileStream( pIOHandler->Open( file, mode));
    if( !fileStream.get() ) {
        throw DeadlyImportError( "Failed to open file " + file + "." );
    }

    // Get the file-size and validate it, throwing an exception when fails
    size_t fileSize = fileStream->FileSize();
    if( fileSize < ObjMinSize ) {
        throw DeadlyImportError( "OBJ-file is too small.");
    }

    IOStreamBuffer<char> streamedBuffer;
    streamedBuffer.open( fileStream.get() );

    // Allocate buffer and read file into it
    //TextFileToBuffer( fileStream.get(),m_Buffer);

    // Get the model name
    std::string  modelName, folderName;
    std::string::size_type pos = file.find_last_of( "\\/" );
    if ( pos != std::string::npos ) {
        modelName = file.substr(pos+1, file.size() - pos - 1);
        folderName = file.substr( 0, pos );
        if ( !folderName.empty() ) {
            pIOHandler->PushDirectory( folderName );
        }
    } else {
        modelName = file;
    }

    // This next stage takes ~ 1/3th of the total readFile task
    // so should amount for 1/3th of the progress
    // only update every 100KB or it'll be too slow
    /*unsigned int progress = 0;
    unsigned int progressCounter = 0;
    const unsigned int updateProgressEveryBytes = 100 * 1024;
    const unsigned int progressTotal = static_cast<unsigned int>(3*m_Buffer.size()/updateProgressEveryBytes);*/
    // process all '\'
    /*std::vector<char> ::iterator iter = m_Buffer.begin();
    while (iter != m_Buffer.end())
    {
        if (*iter == '\\')
        {
            // remove '\'
            iter = m_Buffer.erase(iter);
            // remove next character
            while (*iter == '\r' || *iter == '\n')
                iter = m_Buffer.erase(iter);
        }
        else
            ++iter;

        if (++progressCounter >= updateProgressEveryBytes)
        {
            m_progress->UpdateFileRead(++progress, progressTotal);
            progressCounter = 0;
        }
    }*/

    // 1/3rd progress
    m_progress->UpdateFileRead(1, 3);

    // parse the file into a temporary representation
    ObjFileParser parser( streamedBuffer, modelName, pIOHandler, m_progress, file);

    // And create the proper return structures out of it
    CreateDataFromImport(parser.GetModel(), pScene);

    streamedBuffer.close();

    // Clean up allocated storage for the next import
    m_Buffer.clear();

    // Pop directory stack
    if ( pIOHandler->StackSize() > 0 ) {
        pIOHandler->PopDirectory();
    }
}

// ------------------------------------------------------------------------------------------------
//  Create the data from parsed obj-file
void ObjFileImporter::CreateDataFromImport(const ObjFile::Model* pModel, aiScene* pScene) {
    if( 0L == pModel ) {
        return;
    }

    // Create the root node of the scene
    pScene->mRootNode = new aiNode;
    if ( !pModel->m_ModelName.empty() )
    {
        // Set the name of the scene
        pScene->mRootNode->mName.Set(pModel->m_ModelName);
    }
    else
    {
        // This is a fatal error, so break down the application
        ai_assert(false);
    }

    // Create nodes for the whole scene
    std::vector<aiMesh*> MeshArray;
    for (size_t index = 0; index < pModel->m_Objects.size(); index++)
    {
        createNodes(pModel, pModel->m_Objects[ index ], pScene->mRootNode, pScene, MeshArray);
    }

    // Create mesh pointer buffer for this scene
    if (pScene->mNumMeshes > 0)
    {
        pScene->mMeshes = new aiMesh*[ MeshArray.size() ];
        for (size_t index =0; index < MeshArray.size(); index++)
        {
            pScene->mMeshes[ index ] = MeshArray[ index ];
        }
    }

    // Create all materials
    createMaterials( pModel, pScene );
}

// ------------------------------------------------------------------------------------------------
//  Creates all nodes of the model
aiNode *ObjFileImporter::createNodes(const ObjFile::Model* pModel, const ObjFile::Object* pObject,
                                     aiNode *pParent, aiScene* pScene,
                                     std::vector<aiMesh*> &MeshArray )
{
    ai_assert( NULL != pModel );
    if( NULL == pObject ) {
        return NULL;
    }

    // Store older mesh size to be able to computes mesh offsets for new mesh instances
    const size_t oldMeshSize = MeshArray.size();
    aiNode *pNode = new aiNode;

    pNode->mName = pObject->m_strObjName;

    // If we have a parent node, store it
    if( pParent != NULL ) {
        appendChildToParentNode( pParent, pNode );
    }

    for ( size_t i=0; i< pObject->m_Meshes.size(); i++ )
    {
        unsigned int meshId = pObject->m_Meshes[ i ];
        aiMesh *pMesh = createTopology( pModel, pObject, meshId );
        if( pMesh && pMesh->mNumFaces > 0 ) {
            MeshArray.push_back( pMesh );
        }
    }

    // Create all nodes from the sub-objects stored in the current object
    if ( !pObject->m_SubObjects.empty() )
    {
        size_t numChilds = pObject->m_SubObjects.size();
        pNode->mNumChildren = static_cast<unsigned int>( numChilds );
        pNode->mChildren = new aiNode*[ numChilds ];
        pNode->mNumMeshes = 1;
        pNode->mMeshes = new unsigned int[ 1 ];
    }

    // Set mesh instances into scene- and node-instances
    const size_t meshSizeDiff = MeshArray.size()- oldMeshSize;
    if ( meshSizeDiff > 0 )
    {
        pNode->mMeshes = new unsigned int[ meshSizeDiff ];
        pNode->mNumMeshes = static_cast<unsigned int>( meshSizeDiff );
        size_t index = 0;
        for (size_t i = oldMeshSize; i < MeshArray.size(); i++)
        {
            pNode->mMeshes[ index ] = pScene->mNumMeshes;
            pScene->mNumMeshes++;
            index++;
        }
    }

    return pNode;
}

// ------------------------------------------------------------------------------------------------
//  Create topology data
aiMesh *ObjFileImporter::createTopology( const ObjFile::Model* pModel, const ObjFile::Object* pData, unsigned int meshIndex ) {
    // Checking preconditions
    ai_assert( NULL != pModel );

    if( NULL == pData ) {
        return NULL;
    }

    // Create faces
    ObjFile::Mesh *pObjMesh = pModel->m_Meshes[ meshIndex ];
    if( !pObjMesh ) {
        return NULL;
    }

    if( pObjMesh->m_Faces.empty() ) {
        return NULL;
    }

    aiMesh* pMesh = new aiMesh;
    if( !pObjMesh->m_name.empty() ) {
        pMesh->mName.Set( pObjMesh->m_name );
    }

    for (size_t index = 0; index < pObjMesh->m_Faces.size(); index++)
    {
        ObjFile::Face *const inp = pObjMesh->m_Faces[ index ];
        ai_assert( NULL != inp  );

        if (inp->m_PrimitiveType == aiPrimitiveType_LINE) {
            pMesh->mNumFaces += static_cast<unsigned int>(inp->m_vertices.size() - 1);
            pMesh->mPrimitiveTypes |= aiPrimitiveType_LINE;
        } else if (inp->m_PrimitiveType == aiPrimitiveType_POINT) {
            pMesh->mNumFaces += static_cast<unsigned int>(inp->m_vertices.size());
            pMesh->mPrimitiveTypes |= aiPrimitiveType_POINT;
        } else {
            ++pMesh->mNumFaces;
            if (inp->m_vertices.size() > 3) {
                pMesh->mPrimitiveTypes |= aiPrimitiveType_POLYGON;
            } else {
                pMesh->mPrimitiveTypes |= aiPrimitiveType_TRIANGLE;
            }
        }
    }

    unsigned int uiIdxCount( 0u );
    if ( pMesh->mNumFaces > 0 ) {
        pMesh->mFaces = new aiFace[ pMesh->mNumFaces ];
        if ( pObjMesh->m_uiMaterialIndex != ObjFile::Mesh::NoMaterial ) {
            pMesh->mMaterialIndex = pObjMesh->m_uiMaterialIndex;
        }

        unsigned int outIndex( 0 );

        // Copy all data from all stored meshes
        for (size_t index = 0; index < pObjMesh->m_Faces.size(); index++) {
            ObjFile::Face* const inp = pObjMesh->m_Faces[ index ];
            if (inp->m_PrimitiveType == aiPrimitiveType_LINE) {
                for(size_t i = 0; i < inp->m_vertices.size() - 1; ++i) {
                    aiFace& f = pMesh->mFaces[ outIndex++ ];
                    uiIdxCount += f.mNumIndices = 2;
                    f.mIndices = new unsigned int[2];
                }
                continue;
            }
            else if (inp->m_PrimitiveType == aiPrimitiveType_POINT) {
                for(size_t i = 0; i < inp->m_vertices.size(); ++i) {
                    aiFace& f = pMesh->mFaces[ outIndex++ ];
                    uiIdxCount += f.mNumIndices = 1;
                    f.mIndices = new unsigned int[1];
                }
                continue;
            }

            aiFace *pFace = &pMesh->mFaces[ outIndex++ ];
            const unsigned int uiNumIndices = (unsigned int) pObjMesh->m_Faces[ index ]->m_vertices.size();
            uiIdxCount += pFace->mNumIndices = (unsigned int) uiNumIndices;
            if (pFace->mNumIndices > 0) {
                pFace->mIndices = new unsigned int[ uiNumIndices ];
            }
        }
    }

    // Create mesh vertices
    createVertexArray(pModel, pData, meshIndex, pMesh, uiIdxCount);

    return pMesh;
}

// ------------------------------------------------------------------------------------------------
//  Creates a vertex array
void ObjFileImporter::createVertexArray(const ObjFile::Model* pModel,
                                        const ObjFile::Object* pCurrentObject,
                                        unsigned int uiMeshIndex,
                                        aiMesh* pMesh,
                                        unsigned int numIndices) {
    // Checking preconditions
    ai_assert( NULL != pCurrentObject );

    // Break, if no faces are stored in object
    if ( pCurrentObject->m_Meshes.empty() )
        return;

    // Get current mesh
    ObjFile::Mesh *pObjMesh = pModel->m_Meshes[ uiMeshIndex ];
    if ( NULL == pObjMesh || pObjMesh->m_uiNumIndices < 1 ) {
        return;
    }

    // Copy vertices of this mesh instance
    pMesh->mNumVertices = numIndices;
    if (pMesh->mNumVertices == 0) {
        throw DeadlyImportError( "OBJ: no vertices" );
    } else if (pMesh->mNumVertices > AI_MAX_ALLOC(aiVector3D)) {
        throw DeadlyImportError( "OBJ: Too many vertices, would run out of memory" );
    }
    pMesh->mVertices = new aiVector3D[ pMesh->mNumVertices ];

    // Allocate buffer for normal vectors
    if ( !pModel->m_Normals.empty() && pObjMesh->m_hasNormals )
        pMesh->mNormals = new aiVector3D[ pMesh->mNumVertices ];

    // Allocate buffer for vertex-color vectors
    if ( !pModel->m_VertexColors.empty() )
        pMesh->mColors[0] = new aiColor4D[ pMesh->mNumVertices ];

    // Allocate buffer for texture coordinates
    if ( !pModel->m_TextureCoord.empty() && pObjMesh->m_uiUVCoordinates[0] )
    {
        pMesh->mNumUVComponents[ 0 ] = 2;
        pMesh->mTextureCoords[ 0 ] = new aiVector3D[ pMesh->mNumVertices ];
    }

    // Copy vertices, normals and textures into aiMesh instance
    unsigned int newIndex = 0, outIndex = 0;
    for ( size_t index=0; index < pObjMesh->m_Faces.size(); index++ ) {
        // Get source face
        ObjFile::Face *pSourceFace = pObjMesh->m_Faces[ index ];

        // Copy all index arrays
        for ( size_t vertexIndex = 0, outVertexIndex = 0; vertexIndex < pSourceFace->m_vertices.size(); vertexIndex++ ) {
            const unsigned int vertex = pSourceFace->m_vertices.at( vertexIndex );
            if ( vertex >= pModel->m_Vertices.size() ) {
                throw DeadlyImportError( "OBJ: vertex index out of range" );
            }

            pMesh->mVertices[ newIndex ] = pModel->m_Vertices[ vertex ];

            // Copy all normals
            if ( !pModel->m_Normals.empty() && vertexIndex < pSourceFace->m_normals.size()) {
                const unsigned int normal = pSourceFace->m_normals.at( vertexIndex );
                if ( normal >= pModel->m_Normals.size() ) {
                    throw DeadlyImportError( "OBJ: vertex normal index out of range" );
                }
                pMesh->mNormals[ newIndex ] = pModel->m_Normals[ normal ];
            }

            // Copy all vertex colors
            if ( !pModel->m_VertexColors.empty())
            {
                const aiVector3D color = pModel->m_VertexColors[ vertex ];
                pMesh->mColors[0][ newIndex ] = aiColor4D(color.x, color.y, color.z, 1.0);
            }

            // Copy all texture coordinates
            if ( !pModel->m_TextureCoord.empty() && vertexIndex < pSourceFace->m_texturCoords.size())
            {
                const unsigned int tex = pSourceFace->m_texturCoords.at( vertexIndex );
                ai_assert( tex < pModel->m_TextureCoord.size() );

                if ( tex >= pModel->m_TextureCoord.size() )
                    throw DeadlyImportError("OBJ: texture coordinate index out of range");

                const aiVector3D &coord3d = pModel->m_TextureCoord[ tex ];
                pMesh->mTextureCoords[ 0 ][ newIndex ] = aiVector3D( coord3d.x, coord3d.y, coord3d.z );
            }

            if ( pMesh->mNumVertices <= newIndex ) {
                throw DeadlyImportError("OBJ: bad vertex index");
            }

            // Get destination face
            aiFace *pDestFace = &pMesh->mFaces[ outIndex ];

            const bool last = ( vertexIndex == pSourceFace->m_vertices.size() - 1 );
            if (pSourceFace->m_PrimitiveType != aiPrimitiveType_LINE || !last) {
                pDestFace->mIndices[ outVertexIndex ] = newIndex;
                outVertexIndex++;
            }

            if (pSourceFace->m_PrimitiveType == aiPrimitiveType_POINT) {
                outIndex++;
                outVertexIndex = 0;
            } else if (pSourceFace->m_PrimitiveType == aiPrimitiveType_LINE) {
                outVertexIndex = 0;

                if(!last)
                    outIndex++;

                if (vertexIndex) {
                    if(!last) {
                        pMesh->mVertices[ newIndex+1 ] = pMesh->mVertices[ newIndex ];
                        if ( !pSourceFace->m_normals.empty() && !pModel->m_Normals.empty()) {
                            pMesh->mNormals[ newIndex+1 ] = pMesh->mNormals[newIndex ];
                        }
                        if ( !pModel->m_TextureCoord.empty() ) {
                            for ( size_t i=0; i < pMesh->GetNumUVChannels(); i++ ) {
                                pMesh->mTextureCoords[ i ][ newIndex+1 ] = pMesh->mTextureCoords[ i ][ newIndex ];
                            }
                        }
                        ++newIndex;
                    }

                    pDestFace[-1].mIndices[1] = newIndex;
                }
            }
            else if (last) {
                outIndex++;
            }
            ++newIndex;
        }
    }
}

// ------------------------------------------------------------------------------------------------
//  Counts all stored meshes
void ObjFileImporter::countObjects(const std::vector<ObjFile::Object*> &rObjects, int &iNumMeshes)
{
    iNumMeshes = 0;
    if ( rObjects.empty() )
        return;

    iNumMeshes += static_cast<unsigned int>( rObjects.size() );
    for (std::vector<ObjFile::Object*>::const_iterator it = rObjects.begin();
        it != rObjects.end();
        ++it)
    {
        if (!(*it)->m_SubObjects.empty())
        {
            countObjects((*it)->m_SubObjects, iNumMeshes);
        }
    }
}

// ------------------------------------------------------------------------------------------------
//   Add clamp mode property to material if necessary
void ObjFileImporter::addTextureMappingModeProperty( aiMaterial* mat, aiTextureType type, int clampMode, int index) {
    if ( nullptr == mat ) {
        return;
    }

    mat->AddProperty<int>( &clampMode, 1, AI_MATKEY_MAPPINGMODE_U( type, index ) );
    mat->AddProperty<int>( &clampMode, 1, AI_MATKEY_MAPPINGMODE_V( type, index ) );
}

// ------------------------------------------------------------------------------------------------
//  Creates the material
void ObjFileImporter::createMaterials(const ObjFile::Model* pModel, aiScene* pScene ) {
    if ( NULL == pScene ) {
        return;
    }

    const unsigned int numMaterials = (unsigned int) pModel->m_MaterialLib.size();
    pScene->mNumMaterials = 0;
    if ( pModel->m_MaterialLib.empty() ) {
        DefaultLogger::get()->debug("OBJ: no materials specified");
        return;
    }

    pScene->mMaterials = new aiMaterial*[ numMaterials ];
    for ( unsigned int matIndex = 0; matIndex < numMaterials; matIndex++ )
    {
        // Store material name
        std::map<std::string, ObjFile::Material*>::const_iterator it;
        it = pModel->m_MaterialMap.find( pModel->m_MaterialLib[ matIndex ] );

        // No material found, use the default material
        if ( pModel->m_MaterialMap.end() == it )
            continue;

        aiMaterial* mat = new aiMaterial;
        ObjFile::Material *pCurrentMaterial = (*it).second;
        mat->AddProperty( &pCurrentMaterial->MaterialName, AI_MATKEY_NAME );

        // convert illumination model
        int sm = 0;
        switch (pCurrentMaterial->illumination_model)
        {
        case 0:
            sm = aiShadingMode_NoShading;
            break;
        case 1:
            sm = aiShadingMode_Gouraud;
            break;
        case 2:
            sm = aiShadingMode_Phong;
            break;
        default:
            sm = aiShadingMode_Gouraud;
            DefaultLogger::get()->error("OBJ: unexpected illumination model (0-2 recognized)");
        }

        mat->AddProperty<int>( &sm, 1, AI_MATKEY_SHADING_MODEL);

        // Adding material colors
        mat->AddProperty( &pCurrentMaterial->ambient, 1, AI_MATKEY_COLOR_AMBIENT );
        mat->AddProperty( &pCurrentMaterial->diffuse, 1, AI_MATKEY_COLOR_DIFFUSE );
        mat->AddProperty( &pCurrentMaterial->specular, 1, AI_MATKEY_COLOR_SPECULAR );
        mat->AddProperty( &pCurrentMaterial->emissive, 1, AI_MATKEY_COLOR_EMISSIVE );
        mat->AddProperty( &pCurrentMaterial->shineness, 1, AI_MATKEY_SHININESS );
        mat->AddProperty( &pCurrentMaterial->alpha, 1, AI_MATKEY_OPACITY );
        mat->AddProperty( &pCurrentMaterial->transparent,1,AI_MATKEY_COLOR_TRANSPARENT);

        // Adding refraction index
        mat->AddProperty( &pCurrentMaterial->ior, 1, AI_MATKEY_REFRACTI );

        // Adding textures
        const int uvwIndex = 0;

        if ( 0 != pCurrentMaterial->texture.length )
        {
            mat->AddProperty( &pCurrentMaterial->texture, AI_MATKEY_TEXTURE_DIFFUSE(0));
            mat->AddProperty( &uvwIndex, 1, AI_MATKEY_UVWSRC_DIFFUSE(0) );
            if (pCurrentMaterial->clamp[ObjFile::Material::TextureDiffuseType])
            {
                addTextureMappingModeProperty(mat, aiTextureType_DIFFUSE);
            }
        }

        if ( 0 != pCurrentMaterial->textureAmbient.length )
        {
            mat->AddProperty( &pCurrentMaterial->textureAmbient, AI_MATKEY_TEXTURE_AMBIENT(0));
            mat->AddProperty( &uvwIndex, 1, AI_MATKEY_UVWSRC_AMBIENT(0) );
            if (pCurrentMaterial->clamp[ObjFile::Material::TextureAmbientType])
            {
                addTextureMappingModeProperty(mat, aiTextureType_AMBIENT);
            }
        }

        if ( 0 != pCurrentMaterial->textureEmissive.length )
        {
            mat->AddProperty( &pCurrentMaterial->textureEmissive, AI_MATKEY_TEXTURE_EMISSIVE(0));
            mat->AddProperty( &uvwIndex, 1, AI_MATKEY_UVWSRC_EMISSIVE(0) );
        }

        if ( 0 != pCurrentMaterial->textureSpecular.length )
        {
            mat->AddProperty( &pCurrentMaterial->textureSpecular, AI_MATKEY_TEXTURE_SPECULAR(0));
            mat->AddProperty( &uvwIndex, 1, AI_MATKEY_UVWSRC_SPECULAR(0) );
            if (pCurrentMaterial->clamp[ObjFile::Material::TextureSpecularType])
            {
                addTextureMappingModeProperty(mat, aiTextureType_SPECULAR);
            }
        }

        if ( 0 != pCurrentMaterial->textureBump.length )
        {
            mat->AddProperty( &pCurrentMaterial->textureBump, AI_MATKEY_TEXTURE_HEIGHT(0));
            mat->AddProperty( &uvwIndex, 1, AI_MATKEY_UVWSRC_HEIGHT(0) );
            if (pCurrentMaterial->clamp[ObjFile::Material::TextureBumpType])
            {
                addTextureMappingModeProperty(mat, aiTextureType_HEIGHT);
            }
        }

        if ( 0 != pCurrentMaterial->textureNormal.length )
        {
            mat->AddProperty( &pCurrentMaterial->textureNormal, AI_MATKEY_TEXTURE_NORMALS(0));
            mat->AddProperty( &uvwIndex, 1, AI_MATKEY_UVWSRC_NORMALS(0) );
            if (pCurrentMaterial->clamp[ObjFile::Material::TextureNormalType])
            {
                addTextureMappingModeProperty(mat, aiTextureType_NORMALS);
            }
        }

        if( 0 != pCurrentMaterial->textureReflection[0].length )
        {
            ObjFile::Material::TextureType type = 0 != pCurrentMaterial->textureReflection[1].length ?
                ObjFile::Material::TextureReflectionCubeTopType :
                ObjFile::Material::TextureReflectionSphereType;

            unsigned count = type == ObjFile::Material::TextureReflectionSphereType ? 1 : 6;
            for( unsigned i = 0; i < count; i++ )
            {
                mat->AddProperty(&pCurrentMaterial->textureReflection[i], AI_MATKEY_TEXTURE_REFLECTION(i));
                mat->AddProperty( &uvwIndex, 1, AI_MATKEY_UVWSRC_REFLECTION(i) );

                if(pCurrentMaterial->clamp[type])
                    addTextureMappingModeProperty(mat, aiTextureType_REFLECTION, 1, i);
            }
        }

        if ( 0 != pCurrentMaterial->textureDisp.length )
        {
            mat->AddProperty( &pCurrentMaterial->textureDisp, AI_MATKEY_TEXTURE_DISPLACEMENT(0) );
            mat->AddProperty( &uvwIndex, 1, AI_MATKEY_UVWSRC_DISPLACEMENT(0) );
            if (pCurrentMaterial->clamp[ObjFile::Material::TextureDispType])
            {
                addTextureMappingModeProperty(mat, aiTextureType_DISPLACEMENT);
            }
        }

        if ( 0 != pCurrentMaterial->textureOpacity.length )
        {
            mat->AddProperty( &pCurrentMaterial->textureOpacity, AI_MATKEY_TEXTURE_OPACITY(0));
            mat->AddProperty( &uvwIndex, 1, AI_MATKEY_UVWSRC_OPACITY(0) );
            if (pCurrentMaterial->clamp[ObjFile::Material::TextureOpacityType])
            {
                addTextureMappingModeProperty(mat, aiTextureType_OPACITY);
            }
        }

        if ( 0 != pCurrentMaterial->textureSpecularity.length )
        {
            mat->AddProperty( &pCurrentMaterial->textureSpecularity, AI_MATKEY_TEXTURE_SHININESS(0));
            mat->AddProperty( &uvwIndex, 1, AI_MATKEY_UVWSRC_SHININESS(0) );
            if (pCurrentMaterial->clamp[ObjFile::Material::TextureSpecularityType])
            {
                addTextureMappingModeProperty(mat, aiTextureType_SHININESS);
            }
        }

        // Store material property info in material array in scene
        pScene->mMaterials[ pScene->mNumMaterials ] = mat;
        pScene->mNumMaterials++;
    }

    // Test number of created materials.
    ai_assert( pScene->mNumMaterials == numMaterials );
}

// ------------------------------------------------------------------------------------------------
//  Appends this node to the parent node
void ObjFileImporter::appendChildToParentNode(aiNode *pParent, aiNode *pChild)
{
    // Checking preconditions
    ai_assert( NULL != pParent );
    ai_assert( NULL != pChild );

    // Assign parent to child
    pChild->mParent = pParent;

    // If already children was assigned to the parent node, store them in a
    std::vector<aiNode*> temp;
    if (pParent->mChildren != NULL)
    {
        ai_assert( 0 != pParent->mNumChildren );
        for (size_t index = 0; index < pParent->mNumChildren; index++)
        {
            temp.push_back(pParent->mChildren [ index ] );
        }
        delete [] pParent->mChildren;
    }

    // Copy node instances into parent node
    pParent->mNumChildren++;
    pParent->mChildren = new aiNode*[ pParent->mNumChildren ];
    for (size_t index = 0; index < pParent->mNumChildren-1; index++)
    {
        pParent->mChildren[ index ] = temp [ index ];
    }
    pParent->mChildren[ pParent->mNumChildren-1 ] = pChild;
}

// ------------------------------------------------------------------------------------------------

}   // Namespace Assimp

#endif // !! ASSIMP_BUILD_NO_OBJ_IMPORTER
