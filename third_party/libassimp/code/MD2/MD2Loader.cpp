/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team



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


#ifndef ASSIMP_BUILD_NO_MD2_IMPORTER

/** @file Implementation of the MD2 importer class */
#include "MD2Loader.h"
#include <assimp/ByteSwapper.h>
#include "MD2NormalTable.h" // shouldn't be included by other units
#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>
#include <assimp/importerdesc.h>

#include <memory>

using namespace Assimp;
using namespace Assimp::MD2;

// helper macro to determine the size of an array
#if (!defined ARRAYSIZE)
#   define ARRAYSIZE(_array) (int(sizeof(_array) / sizeof(_array[0])))
#endif

static const aiImporterDesc desc = {
    "Quake II Mesh Importer",
    "",
    "",
    "",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "md2"
};

// ------------------------------------------------------------------------------------------------
// Helper function to lookup a normal in Quake 2's precalculated table
void MD2::LookupNormalIndex(uint8_t iNormalIndex,aiVector3D& vOut)
{
    // make sure the normal index has a valid value
    if (iNormalIndex >= ARRAYSIZE(g_avNormals)) {
        ASSIMP_LOG_WARN("Index overflow in Quake II normal vector list");
        iNormalIndex = ARRAYSIZE(g_avNormals) - 1;
    }
    vOut = *((const aiVector3D*)(&g_avNormals[iNormalIndex]));
}


// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
MD2Importer::MD2Importer()
    : configFrameID(),
    m_pcHeader(),
    mBuffer(),
    fileSize()
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
MD2Importer::~MD2Importer()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool MD2Importer::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    const std::string extension = GetExtension(pFile);
    if (extension == "md2")
        return true;

    // if check for extension is not enough, check for the magic tokens
    if (!extension.length() || checkSig) {
        uint32_t tokens[1];
        tokens[0] = AI_MD2_MAGIC_NUMBER_LE;
        return CheckMagicToken(pIOHandler,pFile,tokens,1);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Get a list of all extensions supported by this loader
const aiImporterDesc* MD2Importer::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Setup configuration properties
void MD2Importer::SetupProperties(const Importer* pImp)
{
    // The
    // AI_CONFIG_IMPORT_MD2_KEYFRAME option overrides the
    // AI_CONFIG_IMPORT_GLOBAL_KEYFRAME option.
    configFrameID = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_MD2_KEYFRAME,-1);
    if(static_cast<unsigned int>(-1) == configFrameID){
        configFrameID = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_GLOBAL_KEYFRAME,0);
    }
}
// ------------------------------------------------------------------------------------------------
// Validate the file header
void MD2Importer::ValidateHeader( )
{
    // check magic number
    if (m_pcHeader->magic != AI_MD2_MAGIC_NUMBER_BE &&
        m_pcHeader->magic != AI_MD2_MAGIC_NUMBER_LE)
    {
        char szBuffer[5];
        szBuffer[0] = ((char*)&m_pcHeader->magic)[0];
        szBuffer[1] = ((char*)&m_pcHeader->magic)[1];
        szBuffer[2] = ((char*)&m_pcHeader->magic)[2];
        szBuffer[3] = ((char*)&m_pcHeader->magic)[3];
        szBuffer[4] = '\0';

        throw DeadlyImportError("Invalid MD2 magic word: should be IDP2, the "
            "magic word found is " + std::string(szBuffer));
    }

    // check file format version
    if (m_pcHeader->version != 8)
        ASSIMP_LOG_WARN( "Unsupported md2 file version. Continuing happily ...");

    // check some values whether they are valid
    if (0 == m_pcHeader->numFrames)
        throw DeadlyImportError( "Invalid md2 file: NUM_FRAMES is 0");

    if (m_pcHeader->offsetEnd > (uint32_t)fileSize)
        throw DeadlyImportError( "Invalid md2 file: File is too small");

    if (m_pcHeader->numSkins > AI_MAX_ALLOC(MD2::Skin)) {
        throw DeadlyImportError("Invalid MD2 header: too many skins, would overflow");
    }

    if (m_pcHeader->numVertices > AI_MAX_ALLOC(MD2::Vertex)) {
        throw DeadlyImportError("Invalid MD2 header: too many vertices, would overflow");
    }

    if (m_pcHeader->numTexCoords > AI_MAX_ALLOC(MD2::TexCoord)) {
        throw DeadlyImportError("Invalid MD2 header: too many texcoords, would overflow");
    }

    if (m_pcHeader->numTriangles > AI_MAX_ALLOC(MD2::Triangle)) {
        throw DeadlyImportError("Invalid MD2 header: too many triangles, would overflow");
    }

    if (m_pcHeader->numFrames > AI_MAX_ALLOC(MD2::Frame)) {
        throw DeadlyImportError("Invalid MD2 header: too many frames, would overflow");
    }

    // -1 because Frame already contains one
    unsigned int frameSize = sizeof (MD2::Frame) + (m_pcHeader->numVertices - 1) * sizeof(MD2::Vertex);

    if (m_pcHeader->offsetSkins     + m_pcHeader->numSkins * sizeof (MD2::Skin)         >= fileSize ||
        m_pcHeader->offsetTexCoords + m_pcHeader->numTexCoords * sizeof (MD2::TexCoord) >= fileSize ||
        m_pcHeader->offsetTriangles + m_pcHeader->numTriangles * sizeof (MD2::Triangle) >= fileSize ||
        m_pcHeader->offsetFrames    + m_pcHeader->numFrames * frameSize                 >= fileSize ||
        m_pcHeader->offsetEnd           > fileSize)
    {
        throw DeadlyImportError("Invalid MD2 header: some offsets are outside the file");
    }

    if (m_pcHeader->numSkins > AI_MD2_MAX_SKINS)
        ASSIMP_LOG_WARN("The model contains more skins than Quake 2 supports");
    if ( m_pcHeader->numFrames > AI_MD2_MAX_FRAMES)
        ASSIMP_LOG_WARN("The model contains more frames than Quake 2 supports");
    if (m_pcHeader->numVertices > AI_MD2_MAX_VERTS)
        ASSIMP_LOG_WARN("The model contains more vertices than Quake 2 supports");

    if (m_pcHeader->numFrames <= configFrameID )
        throw DeadlyImportError("The requested frame is not existing the file");
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void MD2Importer::InternReadFile( const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile));

    // Check whether we can read from the file
    if( file.get() == NULL)
        throw DeadlyImportError( "Failed to open MD2 file " + pFile + "");

    // check whether the md3 file is large enough to contain
    // at least the file header
    fileSize = (unsigned int)file->FileSize();
    if( fileSize < sizeof(MD2::Header))
        throw DeadlyImportError( "MD2 File is too small");

    std::vector<uint8_t> mBuffer2(fileSize);
    file->Read(&mBuffer2[0], 1, fileSize);
    mBuffer = &mBuffer2[0];


    m_pcHeader = (BE_NCONST MD2::Header*)mBuffer;

#ifdef AI_BUILD_BIG_ENDIAN

    ByteSwap::Swap4(&m_pcHeader->frameSize);
    ByteSwap::Swap4(&m_pcHeader->magic);
    ByteSwap::Swap4(&m_pcHeader->numFrames);
    ByteSwap::Swap4(&m_pcHeader->numGlCommands);
    ByteSwap::Swap4(&m_pcHeader->numSkins);
    ByteSwap::Swap4(&m_pcHeader->numTexCoords);
    ByteSwap::Swap4(&m_pcHeader->numTriangles);
    ByteSwap::Swap4(&m_pcHeader->numVertices);
    ByteSwap::Swap4(&m_pcHeader->offsetEnd);
    ByteSwap::Swap4(&m_pcHeader->offsetFrames);
    ByteSwap::Swap4(&m_pcHeader->offsetGlCommands);
    ByteSwap::Swap4(&m_pcHeader->offsetSkins);
    ByteSwap::Swap4(&m_pcHeader->offsetTexCoords);
    ByteSwap::Swap4(&m_pcHeader->offsetTriangles);
    ByteSwap::Swap4(&m_pcHeader->skinHeight);
    ByteSwap::Swap4(&m_pcHeader->skinWidth);
    ByteSwap::Swap4(&m_pcHeader->version);

#endif

    ValidateHeader();

    // there won't be more than one mesh inside the file
    pScene->mNumMaterials = 1;
    pScene->mRootNode = new aiNode();
    pScene->mRootNode->mNumMeshes = 1;
    pScene->mRootNode->mMeshes = new unsigned int[1];
    pScene->mRootNode->mMeshes[0] = 0;
    pScene->mMaterials = new aiMaterial*[1];
    pScene->mMaterials[0] = new aiMaterial();
    pScene->mNumMeshes = 1;
    pScene->mMeshes = new aiMesh*[1];

    aiMesh* pcMesh = pScene->mMeshes[0] = new aiMesh();
    pcMesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;

    // navigate to the begin of the current frame data
	BE_NCONST MD2::Frame* pcFrame = (BE_NCONST MD2::Frame*) ((uint8_t*)
		m_pcHeader + m_pcHeader->offsetFrames + (m_pcHeader->frameSize * configFrameID));

    // navigate to the begin of the triangle data
    MD2::Triangle* pcTriangles = (MD2::Triangle*) ((uint8_t*)
        m_pcHeader + m_pcHeader->offsetTriangles);

    // navigate to the begin of the tex coords data
    BE_NCONST MD2::TexCoord* pcTexCoords = (BE_NCONST MD2::TexCoord*) ((uint8_t*)
        m_pcHeader + m_pcHeader->offsetTexCoords);

    // navigate to the begin of the vertex data
    BE_NCONST MD2::Vertex* pcVerts = (BE_NCONST MD2::Vertex*) (pcFrame->vertices);

#ifdef AI_BUILD_BIG_ENDIAN
    for (uint32_t i = 0; i< m_pcHeader->numTriangles; ++i)
    {
        for (unsigned int p = 0; p < 3;++p)
        {
            ByteSwap::Swap2(& pcTriangles[i].textureIndices[p]);
            ByteSwap::Swap2(& pcTriangles[i].vertexIndices[p]);
        }
    }
    for (uint32_t i = 0; i < m_pcHeader->offsetTexCoords;++i)
    {
        ByteSwap::Swap2(& pcTexCoords[i].s);
        ByteSwap::Swap2(& pcTexCoords[i].t);
    }
    ByteSwap::Swap4( & pcFrame->scale[0] );
    ByteSwap::Swap4( & pcFrame->scale[1] );
    ByteSwap::Swap4( & pcFrame->scale[2] );
    ByteSwap::Swap4( & pcFrame->translate[0] );
    ByteSwap::Swap4( & pcFrame->translate[1] );
    ByteSwap::Swap4( & pcFrame->translate[2] );
#endif

    pcMesh->mNumFaces = m_pcHeader->numTriangles;
    pcMesh->mFaces = new aiFace[m_pcHeader->numTriangles];

    // allocate output storage
    pcMesh->mNumVertices = (unsigned int)pcMesh->mNumFaces*3;
    pcMesh->mVertices = new aiVector3D[pcMesh->mNumVertices];
    pcMesh->mNormals = new aiVector3D[pcMesh->mNumVertices];

    // Not sure whether there are MD2 files without texture coordinates
    // NOTE: texture coordinates can be there without a texture,
    // but a texture can't be there without a valid UV channel
    aiMaterial* pcHelper = (aiMaterial*)pScene->mMaterials[0];
    const int iMode = (int)aiShadingMode_Gouraud;
    pcHelper->AddProperty<int>(&iMode, 1, AI_MATKEY_SHADING_MODEL);

    if (m_pcHeader->numTexCoords && m_pcHeader->numSkins)
    {
        // navigate to the first texture associated with the mesh
        const MD2::Skin* pcSkins = (const MD2::Skin*) ((unsigned char*)m_pcHeader +
            m_pcHeader->offsetSkins);

        aiColor3D clr;
        clr.b = clr.g = clr.r = 1.0f;
        pcHelper->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_DIFFUSE);
        pcHelper->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_SPECULAR);

        clr.b = clr.g = clr.r = 0.05f;
        pcHelper->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_AMBIENT);

        if (pcSkins->name[0])
        {
            aiString szString;
            const ai_uint32 iLen = (ai_uint32) ::strlen(pcSkins->name);
            ::memcpy(szString.data,pcSkins->name,iLen);
            szString.data[iLen] = '\0';
            szString.length = iLen;

            pcHelper->AddProperty(&szString,AI_MATKEY_TEXTURE_DIFFUSE(0));
        }
        else{
            ASSIMP_LOG_WARN("Texture file name has zero length. It will be skipped.");
        }
    }
    else    {
        // apply a default material
        aiColor3D clr;
        clr.b = clr.g = clr.r = 0.6f;
        pcHelper->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_DIFFUSE);
        pcHelper->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_SPECULAR);

        clr.b = clr.g = clr.r = 0.05f;
        pcHelper->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_AMBIENT);

        aiString szName;
        szName.Set(AI_DEFAULT_MATERIAL_NAME);
        pcHelper->AddProperty(&szName,AI_MATKEY_NAME);

        aiString sz;

        // TODO: Try to guess the name of the texture file from the model file name

        sz.Set("$texture_dummy.bmp");
        pcHelper->AddProperty(&sz,AI_MATKEY_TEXTURE_DIFFUSE(0));
    }


    // now read all triangles of the first frame, apply scaling and translation
    unsigned int iCurrent = 0;

    float fDivisorU = 1.0f,fDivisorV = 1.0f;
    if (m_pcHeader->numTexCoords)   {
        // allocate storage for texture coordinates, too
        pcMesh->mTextureCoords[0] = new aiVector3D[pcMesh->mNumVertices];
        pcMesh->mNumUVComponents[0] = 2;

        // check whether the skin width or height are zero (this would
        // cause a division through zero)
        if (!m_pcHeader->skinWidth) {
            ASSIMP_LOG_ERROR("MD2: No valid skin width given");
        }
        else fDivisorU = (float)m_pcHeader->skinWidth;
        if (!m_pcHeader->skinHeight){
            ASSIMP_LOG_ERROR("MD2: No valid skin height given");
        }
        else fDivisorV = (float)m_pcHeader->skinHeight;
    }

    for (unsigned int i = 0; i < (unsigned int)m_pcHeader->numTriangles;++i)    {
        // Allocate the face
        pScene->mMeshes[0]->mFaces[i].mIndices = new unsigned int[3];
        pScene->mMeshes[0]->mFaces[i].mNumIndices = 3;

        // copy texture coordinates
        // check whether they are different from the previous value at this index.
        // In this case, create a full separate set of vertices/normals/texcoords
        for (unsigned int c = 0; c < 3;++c,++iCurrent)  {

            // validate vertex indices
            unsigned int iIndex = (unsigned int)pcTriangles[i].vertexIndices[c];
            if (iIndex >= m_pcHeader->numVertices)  {
                ASSIMP_LOG_ERROR("MD2: Vertex index is outside the allowed range");
                iIndex = m_pcHeader->numVertices-1;
            }

            // read x,y, and z component of the vertex
            aiVector3D& vec = pcMesh->mVertices[iCurrent];

            vec.x = (float)pcVerts[iIndex].vertex[0] * pcFrame->scale[0];
            vec.x += pcFrame->translate[0];

            vec.y = (float)pcVerts[iIndex].vertex[1] * pcFrame->scale[1];
            vec.y += pcFrame->translate[1];

            vec.z = (float)pcVerts[iIndex].vertex[2] * pcFrame->scale[2];
            vec.z += pcFrame->translate[2];

            // read the normal vector from the precalculated normal table
            aiVector3D& vNormal = pcMesh->mNormals[iCurrent];
            LookupNormalIndex(pcVerts[iIndex].lightNormalIndex,vNormal);

            // flip z and y to become right-handed
            std::swap((float&)vNormal.z,(float&)vNormal.y);
            std::swap((float&)vec.z,(float&)vec.y);

            if (m_pcHeader->numTexCoords)   {
                // validate texture coordinates
                iIndex = pcTriangles[i].textureIndices[c];
                if (iIndex >= m_pcHeader->numTexCoords) {
                    ASSIMP_LOG_ERROR("MD2: UV index is outside the allowed range");
                    iIndex = m_pcHeader->numTexCoords-1;
                }

                aiVector3D& pcOut = pcMesh->mTextureCoords[0][iCurrent];

                // the texture coordinates are absolute values but we
                // need relative values between 0 and 1
                pcOut.x = pcTexCoords[iIndex].s / fDivisorU;
                pcOut.y = 1.f-pcTexCoords[iIndex].t / fDivisorV;
            }
            pScene->mMeshes[0]->mFaces[i].mIndices[c] = iCurrent;
        }
    }
}

#endif // !! ASSIMP_BUILD_NO_MD2_IMPORTER
