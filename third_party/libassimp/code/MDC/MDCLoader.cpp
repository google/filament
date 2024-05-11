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

/** @file Implementation of the MDC importer class */


#ifndef ASSIMP_BUILD_NO_MDC_IMPORTER

// internal headers
#include "MDC/MDCLoader.h"
#include "MD3/MD3FileData.h"
#include "MDC/MDCNormalTable.h" // shouldn't be included by other units

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>
#include <assimp/importerdesc.h>

#include <memory>

using namespace Assimp;
using namespace Assimp::MDC;

static const aiImporterDesc desc = {
    "Return To Castle Wolfenstein Mesh Importer",
    "",
    "",
    "",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "mdc"
};

// ------------------------------------------------------------------------------------------------
void MDC::BuildVertex(const Frame& frame,
    const BaseVertex& bvert,
    const CompressedVertex& cvert,
    aiVector3D& vXYZOut,
    aiVector3D& vNorOut)
{
    // compute the position
    const float xd = (cvert.xd - AI_MDC_CVERT_BIAS) * AI_MDC_DELTA_SCALING;
    const float yd = (cvert.yd - AI_MDC_CVERT_BIAS) * AI_MDC_DELTA_SCALING;
    const float zd = (cvert.zd - AI_MDC_CVERT_BIAS) * AI_MDC_DELTA_SCALING;
    vXYZOut.x = frame.localOrigin.x + AI_MDC_BASE_SCALING * (bvert.x + xd);
    vXYZOut.y = frame.localOrigin.y + AI_MDC_BASE_SCALING * (bvert.y + yd);
    vXYZOut.z = frame.localOrigin.z + AI_MDC_BASE_SCALING * (bvert.z + zd);

    // compute the normal vector .. ehm ... lookup it in the table :-)
    vNorOut.x = mdcNormals[cvert.nd][0];
    vNorOut.y = mdcNormals[cvert.nd][1];
    vNorOut.z = mdcNormals[cvert.nd][2];
}

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
MDCImporter::MDCImporter()
    : configFrameID(),
    pcHeader(),
    mBuffer(),
    fileSize()
{
}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
MDCImporter::~MDCImporter()
{
}
// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool MDCImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    const std::string extension = GetExtension(pFile);
    if (extension == "mdc")
        return true;

    // if check for extension is not enough, check for the magic tokens
    if (!extension.length() || checkSig) {
        uint32_t tokens[1];
        tokens[0] = AI_MDC_MAGIC_NUMBER_LE;
        return CheckMagicToken(pIOHandler,pFile,tokens,1);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
const aiImporterDesc* MDCImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Validate the header of the given MDC file
void MDCImporter::ValidateHeader()
{
    AI_SWAP4( this->pcHeader->ulVersion );
    AI_SWAP4( this->pcHeader->ulFlags );
    AI_SWAP4( this->pcHeader->ulNumFrames );
    AI_SWAP4( this->pcHeader->ulNumTags );
    AI_SWAP4( this->pcHeader->ulNumSurfaces );
    AI_SWAP4( this->pcHeader->ulNumSkins );
    AI_SWAP4( this->pcHeader->ulOffsetBorderFrames );

    if (pcHeader->ulIdent != AI_MDC_MAGIC_NUMBER_BE &&
        pcHeader->ulIdent != AI_MDC_MAGIC_NUMBER_LE)
    {
        char szBuffer[5];
        szBuffer[0] = ((char*)&pcHeader->ulIdent)[0];
        szBuffer[1] = ((char*)&pcHeader->ulIdent)[1];
        szBuffer[2] = ((char*)&pcHeader->ulIdent)[2];
        szBuffer[3] = ((char*)&pcHeader->ulIdent)[3];
        szBuffer[4] = '\0';

        throw DeadlyImportError("Invalid MDC magic word: should be IDPC, the "
            "magic word found is " + std::string( szBuffer ));
    }

    if (pcHeader->ulVersion != AI_MDC_VERSION) {
        ASSIMP_LOG_WARN("Unsupported MDC file version (2 (AI_MDC_VERSION) was expected)");
    }

    if (pcHeader->ulOffsetBorderFrames + pcHeader->ulNumFrames * sizeof(MDC::Frame) > this->fileSize ||
        pcHeader->ulOffsetSurfaces + pcHeader->ulNumSurfaces * sizeof(MDC::Surface) > this->fileSize)
    {
        throw DeadlyImportError("Some of the offset values in the MDC header are invalid "
            "and point to something behind the file.");
    }

    if (this->configFrameID >= this->pcHeader->ulNumFrames) {
        throw DeadlyImportError("The requested frame is not available");
    }
}

// ------------------------------------------------------------------------------------------------
// Validate the header of a given MDC file surface
void MDCImporter::ValidateSurfaceHeader(BE_NCONST MDC::Surface* pcSurf)
{
    AI_SWAP4(pcSurf->ulFlags);
    AI_SWAP4(pcSurf->ulNumCompFrames);
    AI_SWAP4(pcSurf->ulNumBaseFrames);
    AI_SWAP4(pcSurf->ulNumShaders);
    AI_SWAP4(pcSurf->ulNumVertices);
    AI_SWAP4(pcSurf->ulNumTriangles);
    AI_SWAP4(pcSurf->ulOffsetTriangles);
    AI_SWAP4(pcSurf->ulOffsetTexCoords);
    AI_SWAP4(pcSurf->ulOffsetBaseVerts);
    AI_SWAP4(pcSurf->ulOffsetCompVerts);
    AI_SWAP4(pcSurf->ulOffsetFrameBaseFrames);
    AI_SWAP4(pcSurf->ulOffsetFrameCompFrames);
    AI_SWAP4(pcSurf->ulOffsetEnd);

    const unsigned int iMax = this->fileSize - (unsigned int)((int8_t*)pcSurf-(int8_t*)pcHeader);

    if (pcSurf->ulOffsetBaseVerts + pcSurf->ulNumVertices * sizeof(MDC::BaseVertex)         > iMax ||
        (pcSurf->ulNumCompFrames && pcSurf->ulOffsetCompVerts + pcSurf->ulNumVertices * sizeof(MDC::CompressedVertex)   > iMax) ||
        pcSurf->ulOffsetTriangles + pcSurf->ulNumTriangles * sizeof(MDC::Triangle)          > iMax ||
        pcSurf->ulOffsetTexCoords + pcSurf->ulNumVertices * sizeof(MDC::TexturCoord)        > iMax ||
        pcSurf->ulOffsetShaders + pcSurf->ulNumShaders * sizeof(MDC::Shader)                > iMax ||
        pcSurf->ulOffsetFrameBaseFrames + pcSurf->ulNumBaseFrames * 2                       > iMax ||
        (pcSurf->ulNumCompFrames && pcSurf->ulOffsetFrameCompFrames + pcSurf->ulNumCompFrames * 2   > iMax))
    {
        throw DeadlyImportError("Some of the offset values in the MDC surface header "
            "are invalid and point somewhere behind the file.");
    }
}

// ------------------------------------------------------------------------------------------------
// Setup configuration properties
void MDCImporter::SetupProperties(const Importer* pImp)
{
    // The AI_CONFIG_IMPORT_MDC_KEYFRAME option overrides the
    // AI_CONFIG_IMPORT_GLOBAL_KEYFRAME option.
    if(static_cast<unsigned int>(-1) == (configFrameID = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_MDC_KEYFRAME,-1))){
        configFrameID = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_GLOBAL_KEYFRAME,0);
    }
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void MDCImporter::InternReadFile(
    const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler)
{
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile));

    // Check whether we can read from the file
    if( file.get() == NULL)
        throw DeadlyImportError( "Failed to open MDC file " + pFile + ".");

    // check whether the mdc file is large enough to contain the file header
    fileSize = (unsigned int)file->FileSize();
    if( fileSize < sizeof(MDC::Header))
        throw DeadlyImportError( "MDC File is too small.");

    std::vector<unsigned char> mBuffer2(fileSize);
    file->Read( &mBuffer2[0], 1, fileSize);
    mBuffer = &mBuffer2[0];

    // validate the file header
    this->pcHeader = (BE_NCONST MDC::Header*)this->mBuffer;
    this->ValidateHeader();

    std::vector<std::string> aszShaders;

    // get a pointer to the frame we want to read
    BE_NCONST MDC::Frame* pcFrame = (BE_NCONST MDC::Frame*)(this->mBuffer+
        this->pcHeader->ulOffsetBorderFrames);

    // no need to swap the other members, we won't need them
    pcFrame += configFrameID;
    AI_SWAP4( pcFrame->localOrigin[0] );
    AI_SWAP4( pcFrame->localOrigin[1] );
    AI_SWAP4( pcFrame->localOrigin[2] );

    // get the number of valid surfaces
    BE_NCONST MDC::Surface* pcSurface, *pcSurface2;
    pcSurface = pcSurface2 = new (mBuffer + pcHeader->ulOffsetSurfaces) MDC::Surface;
    unsigned int iNumShaders = 0;
    for (unsigned int i = 0; i < pcHeader->ulNumSurfaces;++i)
    {
        // validate the surface header
        this->ValidateSurfaceHeader(pcSurface2);

        if (pcSurface2->ulNumVertices && pcSurface2->ulNumTriangles)++pScene->mNumMeshes;
        iNumShaders += pcSurface2->ulNumShaders;
        pcSurface2 = new ((int8_t*)pcSurface2 + pcSurface2->ulOffsetEnd) MDC::Surface;
    }
    aszShaders.reserve(iNumShaders);
    pScene->mMeshes = new aiMesh*[pScene->mNumMeshes];

    // necessary that we don't crash if an exception occurs
    for (unsigned int i = 0; i < pScene->mNumMeshes;++i)
        pScene->mMeshes[i] = NULL;

    // now read all surfaces
    unsigned int iDefaultMatIndex = UINT_MAX;
    for (unsigned int i = 0, iNum = 0; i < pcHeader->ulNumSurfaces;++i)
    {
        if (!pcSurface->ulNumVertices || !pcSurface->ulNumTriangles)continue;
        aiMesh* pcMesh = pScene->mMeshes[iNum++] = new aiMesh();

        pcMesh->mNumFaces = pcSurface->ulNumTriangles;
        pcMesh->mNumVertices = pcMesh->mNumFaces * 3;

        // store the name of the surface for use as node name.
        pcMesh->mName.Set(std::string(pcSurface->ucName
                                    , strnlen(pcSurface->ucName, AI_MDC_MAXQPATH - 1)));

        // go to the first shader in the file. ignore the others.
        if (pcSurface->ulNumShaders)
        {
            const MDC::Shader* pcShader = (const MDC::Shader*)((int8_t*)pcSurface + pcSurface->ulOffsetShaders);
            pcMesh->mMaterialIndex = (unsigned int)aszShaders.size();

            // create a new shader
            aszShaders.push_back(std::string( pcShader->ucName, 
                ::strnlen(pcShader->ucName, sizeof(pcShader->ucName)) ));
        }
        // need to create a default material
        else if (UINT_MAX == iDefaultMatIndex)
        {
            pcMesh->mMaterialIndex = iDefaultMatIndex = (unsigned int)aszShaders.size();
            aszShaders.push_back(std::string());
        }
        // otherwise assign a reference to the default material
        else pcMesh->mMaterialIndex = iDefaultMatIndex;

        // allocate output storage for the mesh
        aiVector3D* pcVertCur   = pcMesh->mVertices         = new aiVector3D[pcMesh->mNumVertices];
        aiVector3D* pcNorCur    = pcMesh->mNormals          = new aiVector3D[pcMesh->mNumVertices];
        aiVector3D* pcUVCur     = pcMesh->mTextureCoords[0] = new aiVector3D[pcMesh->mNumVertices];
        aiFace* pcFaceCur       = pcMesh->mFaces            = new aiFace[pcMesh->mNumFaces];

        // create all vertices/faces
        BE_NCONST MDC::Triangle* pcTriangle = (BE_NCONST MDC::Triangle*)
            ((int8_t*)pcSurface+pcSurface->ulOffsetTriangles);

        BE_NCONST MDC::TexturCoord* const pcUVs = (BE_NCONST MDC::TexturCoord*)
            ((int8_t*)pcSurface+pcSurface->ulOffsetTexCoords);

        // get a pointer to the uncompressed vertices
        int16_t iOfs = *((int16_t*) ((int8_t*) pcSurface +
            pcSurface->ulOffsetFrameBaseFrames) +  this->configFrameID);

        AI_SWAP2(iOfs);

        BE_NCONST MDC::BaseVertex* const pcVerts = (BE_NCONST MDC::BaseVertex*)
            ((int8_t*)pcSurface+pcSurface->ulOffsetBaseVerts) +
            ((int)iOfs * pcSurface->ulNumVertices * 4);

        // do the main swapping stuff ...
#if (defined AI_BUILD_BIG_ENDIAN)

        // swap all triangles
        for (unsigned int i = 0; i < pcSurface->ulNumTriangles;++i)
        {
            AI_SWAP4( pcTriangle[i].aiIndices[0] );
            AI_SWAP4( pcTriangle[i].aiIndices[1] );
            AI_SWAP4( pcTriangle[i].aiIndices[2] );
        }

        // swap all vertices
        for (unsigned int i = 0; i < pcSurface->ulNumVertices*pcSurface->ulNumBaseFrames;++i)
        {
            AI_SWAP2( pcVerts->normal );
            AI_SWAP2( pcVerts->x );
            AI_SWAP2( pcVerts->y );
            AI_SWAP2( pcVerts->z );
        }

        // swap all texture coordinates
        for (unsigned int i = 0; i < pcSurface->ulNumVertices;++i)
        {
            AI_SWAP4( pcUVs->v );
            AI_SWAP4( pcUVs->v );
        }

#endif

        const MDC::CompressedVertex* pcCVerts = NULL;
        int16_t* mdcCompVert = NULL;

        // access compressed frames for large frame numbers, but never for the first
        if( this->configFrameID && pcSurface->ulNumCompFrames > 0 )
        {
            mdcCompVert = (int16_t*) ((int8_t*)pcSurface+pcSurface->ulOffsetFrameCompFrames) + this->configFrameID;
            AI_SWAP2P(mdcCompVert);
            if( *mdcCompVert >= 0 )
            {
                pcCVerts = (const MDC::CompressedVertex*)((int8_t*)pcSurface +
                    pcSurface->ulOffsetCompVerts) + *mdcCompVert * pcSurface->ulNumVertices;
            }
            else mdcCompVert = NULL;
        }

        // copy all faces
        for (unsigned int iFace = 0; iFace < pcSurface->ulNumTriangles;++iFace,
            ++pcTriangle,++pcFaceCur)
        {
            const unsigned int iOutIndex = iFace*3;
            pcFaceCur->mNumIndices = 3;
            pcFaceCur->mIndices = new unsigned int[3];

            for (unsigned int iIndex = 0; iIndex < 3;++iIndex,
                ++pcVertCur,++pcUVCur,++pcNorCur)
            {
                uint32_t quak = pcTriangle->aiIndices[iIndex];
                if (quak >= pcSurface->ulNumVertices)
                {
                    ASSIMP_LOG_ERROR("MDC vertex index is out of range");
                    quak = pcSurface->ulNumVertices-1;
                }

                // compressed vertices?
                if (mdcCompVert)
                {
                    MDC::BuildVertex(*pcFrame,pcVerts[quak],pcCVerts[quak],
                        *pcVertCur,*pcNorCur);
                }
                else
                {
                    // copy position
                    pcVertCur->x = pcVerts[quak].x * AI_MDC_BASE_SCALING;
                    pcVertCur->y = pcVerts[quak].y * AI_MDC_BASE_SCALING;
                    pcVertCur->z = pcVerts[quak].z * AI_MDC_BASE_SCALING;

                    // copy normals
                    MD3::LatLngNormalToVec3( pcVerts[quak].normal, &pcNorCur->x );

                    // copy texture coordinates
                    pcUVCur->x = pcUVs[quak].u;
                    pcUVCur->y = ai_real( 1.0 )-pcUVs[quak].v; // DX to OGL
                }
                pcVertCur->x += pcFrame->localOrigin[0] ;
                pcVertCur->y += pcFrame->localOrigin[1] ;
                pcVertCur->z += pcFrame->localOrigin[2] ;
            }

            // swap the face order - DX to OGL
            pcFaceCur->mIndices[0] = iOutIndex + 2;
            pcFaceCur->mIndices[1] = iOutIndex + 1;
            pcFaceCur->mIndices[2] = iOutIndex + 0;
        }

        pcSurface =  new ((int8_t*)pcSurface + pcSurface->ulOffsetEnd) MDC::Surface;
    }

    // create a flat node graph with a root node and one child for each surface
    if (!pScene->mNumMeshes)
        throw DeadlyImportError( "Invalid MDC file: File contains no valid mesh");
    else if (1 == pScene->mNumMeshes)
    {
        pScene->mRootNode = new aiNode();
        if ( nullptr != pScene->mMeshes[0] ) {
            pScene->mRootNode->mName = pScene->mMeshes[0]->mName;
            pScene->mRootNode->mNumMeshes = 1;
            pScene->mRootNode->mMeshes = new unsigned int[1];
            pScene->mRootNode->mMeshes[0] = 0;
        }
    }
    else
    {
        pScene->mRootNode = new aiNode();
        pScene->mRootNode->mNumChildren = pScene->mNumMeshes;
        pScene->mRootNode->mChildren = new aiNode*[pScene->mNumMeshes];
        pScene->mRootNode->mName.Set("<root>");
        for (unsigned int i = 0; i < pScene->mNumMeshes;++i)
        {
            aiNode* pcNode = pScene->mRootNode->mChildren[i] = new aiNode();
            pcNode->mParent = pScene->mRootNode;
            pcNode->mName = pScene->mMeshes[i]->mName;
            pcNode->mNumMeshes = 1;
            pcNode->mMeshes = new unsigned int[1];
            pcNode->mMeshes[0] = i;
        }
    }

    // create materials
    pScene->mNumMaterials = (unsigned int)aszShaders.size();
    pScene->mMaterials = new aiMaterial*[pScene->mNumMaterials];
    for (unsigned int i = 0; i < pScene->mNumMaterials;++i)
    {
        aiMaterial* pcMat = new aiMaterial();
        pScene->mMaterials[i] = pcMat;

        const std::string& name = aszShaders[i];

        int iMode = (int)aiShadingMode_Gouraud;
        pcMat->AddProperty<int>(&iMode, 1, AI_MATKEY_SHADING_MODEL);

        // add a small ambient color value - RtCW seems to have one
        aiColor3D clr;
        clr.b = clr.g = clr.r = 0.05f;
        pcMat->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_AMBIENT);

        if (name.length())clr.b = clr.g = clr.r = 1.0f;
        else clr.b = clr.g = clr.r = 0.6f;

        pcMat->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_DIFFUSE);
        pcMat->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_SPECULAR);

        if (name.length())
        {
            aiString path;
            path.Set(name);
            pcMat->AddProperty(&path,AI_MATKEY_TEXTURE_DIFFUSE(0));
        }
    }
}

#endif // !! ASSIMP_BUILD_NO_MDC_IMPORTER
