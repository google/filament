/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2018, assimp team



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

/** @file MDLLoader.cpp
 *  @brief Implementation of the main parts of the MDL importer class
 *  *TODO* Cleanup and further testing of some parts necessary
 */

// internal headers

#ifndef ASSIMP_BUILD_NO_MDL_IMPORTER

#include "MDLLoader.h"
#include <assimp/Macros.h>
#include <assimp/qnan.h>
#include "MDLDefaultColorMap.h"
#include "MD2FileData.h"
#include <assimp/StringUtils.h>
#include <assimp/Importer.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/importerdesc.h>

#include <memory>

using namespace Assimp;

static const aiImporterDesc desc = {
    "Quake Mesh / 3D GameStudio Mesh Importer",
    "",
    "",
    "",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    7,
    0,
    "mdl"
};

// ------------------------------------------------------------------------------------------------
// Ugly stuff ... nevermind
#define _AI_MDL7_ACCESS(_data, _index, _limit, _type)               \
    (*((const _type*)(((const char*)_data) + _index * _limit)))

#define _AI_MDL7_ACCESS_PTR(_data, _index, _limit, _type)           \
    ((BE_NCONST _type*)(((const char*)_data) + _index * _limit))

#define _AI_MDL7_ACCESS_VERT(_data, _index, _limit)                 \
    _AI_MDL7_ACCESS(_data,_index,_limit,MDL::Vertex_MDL7)

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
MDLImporter::MDLImporter()
    : configFrameID(),
    mBuffer(),
    iGSFileVersion(),
    pIOHandler(),
    pScene(),
    iFileSize()
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
MDLImporter::~MDLImporter()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool MDLImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    const std::string extension = GetExtension(pFile);

    // if check for extension is not enough, check for the magic tokens
    if (extension == "mdl"  || !extension.length() || checkSig) {
        uint32_t tokens[8];
        tokens[0] = AI_MDL_MAGIC_NUMBER_LE_HL2a;
        tokens[1] = AI_MDL_MAGIC_NUMBER_LE_HL2b;
        tokens[2] = AI_MDL_MAGIC_NUMBER_LE_GS7;
        tokens[3] = AI_MDL_MAGIC_NUMBER_LE_GS5b;
        tokens[4] = AI_MDL_MAGIC_NUMBER_LE_GS5a;
        tokens[5] = AI_MDL_MAGIC_NUMBER_LE_GS4;
        tokens[6] = AI_MDL_MAGIC_NUMBER_LE_GS3;
        tokens[7] = AI_MDL_MAGIC_NUMBER_LE;
        return CheckMagicToken(pIOHandler,pFile,tokens,8,0);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Setup configuration properties
void MDLImporter::SetupProperties(const Importer* pImp)
{
    configFrameID = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_MDL_KEYFRAME,-1);

    // The
    // AI_CONFIG_IMPORT_MDL_KEYFRAME option overrides the
    // AI_CONFIG_IMPORT_GLOBAL_KEYFRAME option.
    if(static_cast<unsigned int>(-1) == configFrameID)  {
        configFrameID =  pImp->GetPropertyInteger(AI_CONFIG_IMPORT_GLOBAL_KEYFRAME,0);
    }

    // AI_CONFIG_IMPORT_MDL_COLORMAP - palette file
    configPalette =  pImp->GetPropertyString(AI_CONFIG_IMPORT_MDL_COLORMAP,"colormap.lmp");
}

// ------------------------------------------------------------------------------------------------
// Get a list of all supported extensions
const aiImporterDesc* MDLImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void MDLImporter::InternReadFile( const std::string& pFile,
    aiScene* _pScene, IOSystem* _pIOHandler)
{
    pScene     = _pScene;
    pIOHandler = _pIOHandler;
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile));

    // Check whether we can read from the file
    if( file.get() == NULL) {
        throw DeadlyImportError( "Failed to open MDL file " + pFile + ".");
    }

    // This should work for all other types of MDL files, too ...
    // the quake header is one of the smallest, afaik
    iFileSize = (unsigned int)file->FileSize();
    if( iFileSize < sizeof(MDL::Header)) {
        throw DeadlyImportError( "MDL File is too small.");
    }

    // Allocate storage and copy the contents of the file to a memory buffer
    mBuffer =new unsigned char[iFileSize+1];
    file->Read( (void*)mBuffer, 1, iFileSize);

    // Append a binary zero to the end of the buffer.
    // this is just for safety that string parsing routines
    // find the end of the buffer ...
    mBuffer[iFileSize] = '\0';
    const uint32_t iMagicWord = *((uint32_t*)mBuffer);

    // Determine the file subtype and call the appropriate member function

    // Original Quake1 format
    if (AI_MDL_MAGIC_NUMBER_BE == iMagicWord || AI_MDL_MAGIC_NUMBER_LE == iMagicWord)   {
        ASSIMP_LOG_DEBUG("MDL subtype: Quake 1, magic word is IDPO");
        iGSFileVersion = 0;
        InternReadFile_Quake1();
    }
    // GameStudio A<old> MDL2 format - used by some test models that come with 3DGS
    else if (AI_MDL_MAGIC_NUMBER_BE_GS3 == iMagicWord || AI_MDL_MAGIC_NUMBER_LE_GS3 == iMagicWord)  {
        ASSIMP_LOG_DEBUG("MDL subtype: 3D GameStudio A2, magic word is MDL2");
        iGSFileVersion = 2;
        InternReadFile_Quake1();
    }
    // GameStudio A4 MDL3 format
    else if (AI_MDL_MAGIC_NUMBER_BE_GS4 == iMagicWord || AI_MDL_MAGIC_NUMBER_LE_GS4 == iMagicWord)  {
        ASSIMP_LOG_DEBUG("MDL subtype: 3D GameStudio A4, magic word is MDL3");
        iGSFileVersion = 3;
        InternReadFile_3DGS_MDL345();
    }
    // GameStudio A5+ MDL4 format
    else if (AI_MDL_MAGIC_NUMBER_BE_GS5a == iMagicWord || AI_MDL_MAGIC_NUMBER_LE_GS5a == iMagicWord)    {
        ASSIMP_LOG_DEBUG("MDL subtype: 3D GameStudio A4, magic word is MDL4");
        iGSFileVersion = 4;
        InternReadFile_3DGS_MDL345();
    }
    // GameStudio A5+ MDL5 format
    else if (AI_MDL_MAGIC_NUMBER_BE_GS5b == iMagicWord || AI_MDL_MAGIC_NUMBER_LE_GS5b == iMagicWord)    {
        ASSIMP_LOG_DEBUG("MDL subtype: 3D GameStudio A5, magic word is MDL5");
        iGSFileVersion = 5;
        InternReadFile_3DGS_MDL345();
    }
    // GameStudio A7 MDL7 format
    else if (AI_MDL_MAGIC_NUMBER_BE_GS7 == iMagicWord || AI_MDL_MAGIC_NUMBER_LE_GS7 == iMagicWord)  {
        ASSIMP_LOG_DEBUG("MDL subtype: 3D GameStudio A7, magic word is MDL7");
        iGSFileVersion = 7;
        InternReadFile_3DGS_MDL7();
    }
    // IDST/IDSQ Format (CS:S/HL^2, etc ...)
    else if (AI_MDL_MAGIC_NUMBER_BE_HL2a == iMagicWord || AI_MDL_MAGIC_NUMBER_LE_HL2a == iMagicWord ||
        AI_MDL_MAGIC_NUMBER_BE_HL2b == iMagicWord || AI_MDL_MAGIC_NUMBER_LE_HL2b == iMagicWord)
    {
        ASSIMP_LOG_DEBUG("MDL subtype: Source(tm) Engine, magic word is IDST/IDSQ");
        iGSFileVersion = 0;
        InternReadFile_HL2();
    }
    else    {
        // print the magic word to the log file
        throw DeadlyImportError( "Unknown MDL subformat " + pFile +
            ". Magic word (" + std::string((char*)&iMagicWord,4) + ") is not known");
    }

    // Now rotate the whole scene 90 degrees around the x axis to convert to internal coordinate system
    pScene->mRootNode->mTransformation = aiMatrix4x4(1.f,0.f,0.f,0.f,
        0.f,0.f,1.f,0.f,0.f,-1.f,0.f,0.f,0.f,0.f,0.f,1.f);

    // delete the file buffer and cleanup
    delete [] mBuffer;
    mBuffer= nullptr;
    AI_DEBUG_INVALIDATE_PTR(pIOHandler);
    AI_DEBUG_INVALIDATE_PTR(pScene);
}

// ------------------------------------------------------------------------------------------------
// Check whether we're still inside the valid file range
void MDLImporter::SizeCheck(const void* szPos)
{
    if (!szPos || (const unsigned char*)szPos > this->mBuffer + this->iFileSize)
    {
        throw DeadlyImportError("Invalid MDL file. The file is too small "
            "or contains invalid data.");
    }
}

// ------------------------------------------------------------------------------------------------
// Just for debugging purposes
void MDLImporter::SizeCheck(const void* szPos, const char* szFile, unsigned int iLine)
{
    ai_assert(NULL != szFile);
    if (!szPos || (const unsigned char*)szPos > mBuffer + iFileSize)
    {
        // remove a directory if there is one
        const char* szFilePtr = ::strrchr(szFile,'\\');
        if (!szFilePtr) {
            if(!(szFilePtr = ::strrchr(szFile,'/')))
                szFilePtr = szFile;
        }
        if (szFilePtr)++szFilePtr;

        char szBuffer[1024];
        ::sprintf(szBuffer,"Invalid MDL file. The file is too small "
            "or contains invalid data (File: %s Line: %u)",szFilePtr,iLine);

        throw DeadlyImportError(szBuffer);
    }
}

// ------------------------------------------------------------------------------------------------
// Validate a quake file header
void MDLImporter::ValidateHeader_Quake1(const MDL::Header* pcHeader)
{
    // some values may not be NULL
    if (!pcHeader->num_frames)
        throw DeadlyImportError( "[Quake 1 MDL] There are no frames in the file");

    if (!pcHeader->num_verts)
        throw DeadlyImportError( "[Quake 1 MDL] There are no vertices in the file");

    if (!pcHeader->num_tris)
        throw DeadlyImportError( "[Quake 1 MDL] There are no triangles in the file");

    // check whether the maxima are exceeded ...however, this applies for Quake 1 MDLs only
    if (!this->iGSFileVersion)
    {
        if (pcHeader->num_verts > AI_MDL_MAX_VERTS)
            ASSIMP_LOG_WARN("Quake 1 MDL model has more than AI_MDL_MAX_VERTS vertices");

        if (pcHeader->num_tris > AI_MDL_MAX_TRIANGLES)
            ASSIMP_LOG_WARN("Quake 1 MDL model has more than AI_MDL_MAX_TRIANGLES triangles");

        if (pcHeader->num_frames > AI_MDL_MAX_FRAMES)
            ASSIMP_LOG_WARN("Quake 1 MDL model has more than AI_MDL_MAX_FRAMES frames");

        // (this does not apply for 3DGS MDLs)
        if (!this->iGSFileVersion && pcHeader->version != AI_MDL_VERSION)
            ASSIMP_LOG_WARN("Quake 1 MDL model has an unknown version: AI_MDL_VERSION (=6) is "
                "the expected file format version");
        if(pcHeader->num_skins && (!pcHeader->skinwidth || !pcHeader->skinheight))
            ASSIMP_LOG_WARN("Skin width or height are 0");
    }
}

#ifdef AI_BUILD_BIG_ENDIAN
// ------------------------------------------------------------------------------------------------
void FlipQuakeHeader(BE_NCONST MDL::Header* pcHeader)
{
    AI_SWAP4( pcHeader->ident);
    AI_SWAP4( pcHeader->version);
    AI_SWAP4( pcHeader->boundingradius);
    AI_SWAP4( pcHeader->flags);
    AI_SWAP4( pcHeader->num_frames);
    AI_SWAP4( pcHeader->num_skins);
    AI_SWAP4( pcHeader->num_tris);
    AI_SWAP4( pcHeader->num_verts);
    for (unsigned int i = 0; i < 3;++i)
    {
        AI_SWAP4( pcHeader->scale[i]);
        AI_SWAP4( pcHeader->translate[i]);
    }
    AI_SWAP4( pcHeader->size);
    AI_SWAP4( pcHeader->skinheight);
    AI_SWAP4( pcHeader->skinwidth);
    AI_SWAP4( pcHeader->synctype);
}
#endif

// ------------------------------------------------------------------------------------------------
// Read a Quake 1 file
void MDLImporter::InternReadFile_Quake1() {
    ai_assert(NULL != pScene);

    BE_NCONST MDL::Header *pcHeader = (BE_NCONST MDL::Header*)this->mBuffer;

#ifdef AI_BUILD_BIG_ENDIAN
    FlipQuakeHeader(pcHeader);
#endif

    ValidateHeader_Quake1(pcHeader);

    // current cursor position in the file
    const unsigned char* szCurrent = (const unsigned char*)(pcHeader+1);

    // need to read all textures
    for ( unsigned int i = 0; i < (unsigned int)pcHeader->num_skins; ++i) {
        union {
            BE_NCONST MDL::Skin* pcSkin;
            BE_NCONST MDL::GroupSkin* pcGroupSkin;
        };
        if (szCurrent + sizeof(MDL::Skin) > this->mBuffer + this->iFileSize) {
            throw DeadlyImportError("[Quake 1 MDL] Unexpected EOF");
        }
        pcSkin = (BE_NCONST MDL::Skin*)szCurrent;

        AI_SWAP4( pcSkin->group );

        // Quake 1 group-skins
        if (1 == pcSkin->group) {
            AI_SWAP4( pcGroupSkin->nb );

            // need to skip multiple images
            const unsigned int iNumImages = (unsigned int)pcGroupSkin->nb;
            szCurrent += sizeof(uint32_t) * 2;

            if (0 != iNumImages) {
                if (!i) {
                    // however, create only one output image (the first)
                    this->CreateTextureARGB8_3DGS_MDL3(szCurrent + iNumImages * sizeof(float));
                }
                // go to the end of the skin section / the beginning of the next skin
                szCurrent += pcHeader->skinheight * pcHeader->skinwidth +
                    sizeof(float) * iNumImages;
            }
        } else {
            szCurrent += sizeof(uint32_t);
            unsigned int iSkip = i ? UINT_MAX : 0;
            CreateTexture_3DGS_MDL4(szCurrent,pcSkin->group,&iSkip);
            szCurrent += iSkip;
        }
    }
    // get a pointer to the texture coordinates
    BE_NCONST MDL::TexCoord* pcTexCoords = (BE_NCONST MDL::TexCoord*)szCurrent;
    szCurrent += sizeof(MDL::TexCoord) * pcHeader->num_verts;

    // get a pointer to the triangles
    BE_NCONST MDL::Triangle* pcTriangles = (BE_NCONST MDL::Triangle*)szCurrent;
    szCurrent += sizeof(MDL::Triangle) * pcHeader->num_tris;
    VALIDATE_FILE_SIZE(szCurrent);

    // now get a pointer to the first frame in the file
    BE_NCONST MDL::Frame* pcFrames = (BE_NCONST MDL::Frame*)szCurrent;
    BE_NCONST MDL::SimpleFrame* pcFirstFrame;

    if (0 == pcFrames->type) {
        // get address of single frame
        pcFirstFrame = &pcFrames->frame;
    } else {
        // get the first frame in the group

#if 1
        // FIXME: the cast is wrong and cause a warning on clang 5.0
        // disable thi code for now, fix it later
        ai_assert(false && "Bad pointer cast");
#else
        BE_NCONST MDL::GroupFrame* pcFrames2 = (BE_NCONST MDL::GroupFrame*)pcFrames;
        pcFirstFrame = (BE_NCONST MDL::SimpleFrame*)(&pcFrames2->time + pcFrames->type);
#endif
    }
    BE_NCONST MDL::Vertex* pcVertices = (BE_NCONST MDL::Vertex*) ((pcFirstFrame->name) + sizeof(pcFirstFrame->name));
    VALIDATE_FILE_SIZE((const unsigned char*)(pcVertices + pcHeader->num_verts));

#ifdef AI_BUILD_BIG_ENDIAN
    for (int i = 0; i<pcHeader->num_verts;++i)
    {
        AI_SWAP4( pcTexCoords[i].onseam );
        AI_SWAP4( pcTexCoords[i].s );
        AI_SWAP4( pcTexCoords[i].t );
    }

    for (int i = 0; i<pcHeader->num_tris;++i)
    {
        AI_SWAP4( pcTriangles[i].facesfront);
        AI_SWAP4( pcTriangles[i].vertex[0]);
        AI_SWAP4( pcTriangles[i].vertex[1]);
        AI_SWAP4( pcTriangles[i].vertex[2]);
    }
#endif

    // setup materials
    SetupMaterialProperties_3DGS_MDL5_Quake1();

    // allocate enough storage to hold all vertices and triangles
    aiMesh* pcMesh = new aiMesh();

    pcMesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
    pcMesh->mNumVertices = pcHeader->num_tris * 3;
    pcMesh->mNumFaces = pcHeader->num_tris;
    pcMesh->mVertices = new aiVector3D[pcMesh->mNumVertices];
    pcMesh->mTextureCoords[0] = new aiVector3D[pcMesh->mNumVertices];
    pcMesh->mFaces = new aiFace[pcMesh->mNumFaces];
    pcMesh->mNormals = new aiVector3D[pcMesh->mNumVertices];
    pcMesh->mNumUVComponents[0] = 2;

    // there won't be more than one mesh inside the file
    pScene->mRootNode = new aiNode();
    pScene->mRootNode->mNumMeshes = 1;
    pScene->mRootNode->mMeshes = new unsigned int[1];
    pScene->mRootNode->mMeshes[0] = 0;
    pScene->mNumMeshes = 1;
    pScene->mMeshes = new aiMesh*[1];
    pScene->mMeshes[0] = pcMesh;

    // now iterate through all triangles
    unsigned int iCurrent = 0;
    for (unsigned int i = 0; i < (unsigned int) pcHeader->num_tris;++i)
    {
        pcMesh->mFaces[i].mIndices = new unsigned int[3];
        pcMesh->mFaces[i].mNumIndices = 3;

        unsigned int iTemp = iCurrent;
        for (unsigned int c = 0; c < 3;++c,++iCurrent)
        {
            pcMesh->mFaces[i].mIndices[c] = iCurrent;

            // read vertices
            unsigned int iIndex = pcTriangles->vertex[c];
            if (iIndex >= (unsigned int)pcHeader->num_verts)
            {
                iIndex = pcHeader->num_verts-1;
                ASSIMP_LOG_WARN("Index overflow in Q1-MDL vertex list.");
            }

            aiVector3D& vec = pcMesh->mVertices[iCurrent];
            vec.x = (float)pcVertices[iIndex].v[0] * pcHeader->scale[0];
            vec.x += pcHeader->translate[0];

            vec.y = (float)pcVertices[iIndex].v[1] * pcHeader->scale[1];
            vec.y += pcHeader->translate[1];
            //vec.y *= -1.0f;

            vec.z = (float)pcVertices[iIndex].v[2] * pcHeader->scale[2];
            vec.z += pcHeader->translate[2];

            // read the normal vector from the precalculated normal table
            MD2::LookupNormalIndex(pcVertices[iIndex].normalIndex,pcMesh->mNormals[iCurrent]);
            //pcMesh->mNormals[iCurrent].y *= -1.0f;

            // read texture coordinates
            float s = (float)pcTexCoords[iIndex].s;
            float t = (float)pcTexCoords[iIndex].t;

            // translate texture coordinates
            if (0 == pcTriangles->facesfront && 0 != pcTexCoords[iIndex].onseam)    {
                s += pcHeader->skinwidth * 0.5f;
            }

            // Scale s and t to range from 0.0 to 1.0
            pcMesh->mTextureCoords[0][iCurrent].x = (s + 0.5f) / pcHeader->skinwidth;
            pcMesh->mTextureCoords[0][iCurrent].y = 1.0f-(t + 0.5f) / pcHeader->skinheight;

        }
        pcMesh->mFaces[i].mIndices[0] = iTemp+2;
        pcMesh->mFaces[i].mIndices[1] = iTemp+1;
        pcMesh->mFaces[i].mIndices[2] = iTemp+0;
        pcTriangles++;
    }
    return;
}

// ------------------------------------------------------------------------------------------------
// Setup material properties for Quake and older GameStudio files
void MDLImporter::SetupMaterialProperties_3DGS_MDL5_Quake1( )
{
    const MDL::Header* const pcHeader = (const MDL::Header*)this->mBuffer;

    // allocate ONE material
    pScene->mMaterials    = new aiMaterial*[1];
    pScene->mMaterials[0] = new aiMaterial();
    pScene->mNumMaterials = 1;

    // setup the material's properties
    const int iMode = (int)aiShadingMode_Gouraud;
    aiMaterial* const pcHelper = (aiMaterial*)pScene->mMaterials[0];
    pcHelper->AddProperty<int>(&iMode, 1, AI_MATKEY_SHADING_MODEL);

    aiColor4D clr;
    if (0 != pcHeader->num_skins && pScene->mNumTextures)   {
        // can we replace the texture with a single color?
        clr = this->ReplaceTextureWithColor(pScene->mTextures[0]);
        if (is_not_qnan(clr.r)) {
            delete pScene->mTextures[0];
            delete[] pScene->mTextures;

            pScene->mTextures = NULL;
            pScene->mNumTextures = 0;
        }
        else    {
            clr.b = clr.a = clr.g = clr.r = 1.0f;
            aiString szString;
            ::memcpy(szString.data,AI_MAKE_EMBEDDED_TEXNAME(0),3);
            szString.length = 2;
            pcHelper->AddProperty(&szString,AI_MATKEY_TEXTURE_DIFFUSE(0));
        }
    }

    pcHelper->AddProperty<aiColor4D>(&clr, 1,AI_MATKEY_COLOR_DIFFUSE);
    pcHelper->AddProperty<aiColor4D>(&clr, 1,AI_MATKEY_COLOR_SPECULAR);

    clr.r *= 0.05f;clr.g *= 0.05f;
    clr.b *= 0.05f;clr.a  = 1.0f;
    pcHelper->AddProperty<aiColor4D>(&clr, 1,AI_MATKEY_COLOR_AMBIENT);
}

// ------------------------------------------------------------------------------------------------
// Read a MDL 3,4,5 file
void MDLImporter::InternReadFile_3DGS_MDL345( )
{
    ai_assert(NULL != pScene);

    // the header of MDL 3/4/5 is nearly identical to the original Quake1 header
    BE_NCONST MDL::Header *pcHeader = (BE_NCONST MDL::Header*)this->mBuffer;
#ifdef AI_BUILD_BIG_ENDIAN
    FlipQuakeHeader(pcHeader);
#endif
    ValidateHeader_Quake1(pcHeader);

    // current cursor position in the file
    const unsigned char* szCurrent = (const unsigned char*)(pcHeader+1);
    const unsigned char* szEnd = mBuffer + iFileSize;

    // need to read all textures
    for (unsigned int i = 0; i < (unsigned int)pcHeader->num_skins;++i) {
        if (szCurrent >= szEnd) {
            throw DeadlyImportError( "Texture data past end of file.");
        }
        BE_NCONST MDL::Skin* pcSkin;
        pcSkin = (BE_NCONST  MDL::Skin*)szCurrent;
        AI_SWAP4( pcSkin->group);
        // create one output image
        unsigned int iSkip = i ? UINT_MAX : 0;
        if (5 <= iGSFileVersion)
        {
            // MDL5 format could contain MIPmaps
            CreateTexture_3DGS_MDL5((unsigned char*)pcSkin + sizeof(uint32_t),
                pcSkin->group,&iSkip);
        }
        else    {
            CreateTexture_3DGS_MDL4((unsigned char*)pcSkin + sizeof(uint32_t),
                pcSkin->group,&iSkip);
        }
        // need to skip one image
        szCurrent += iSkip + sizeof(uint32_t);

    }
    // get a pointer to the texture coordinates
    BE_NCONST MDL::TexCoord_MDL3* pcTexCoords = (BE_NCONST MDL::TexCoord_MDL3*)szCurrent;
    szCurrent += sizeof(MDL::TexCoord_MDL3) * pcHeader->synctype;

    // NOTE: for MDLn formats "synctype" corresponds to the number of UV coords

    // get a pointer to the triangles
    BE_NCONST MDL::Triangle_MDL3* pcTriangles = (BE_NCONST MDL::Triangle_MDL3*)szCurrent;
    szCurrent += sizeof(MDL::Triangle_MDL3) * pcHeader->num_tris;

#ifdef AI_BUILD_BIG_ENDIAN

    for (int i = 0; i<pcHeader->synctype;++i)   {
        AI_SWAP2( pcTexCoords[i].u );
        AI_SWAP2( pcTexCoords[i].v );
    }

    for (int i = 0; i<pcHeader->num_tris;++i)   {
        AI_SWAP2( pcTriangles[i].index_xyz[0]);
        AI_SWAP2( pcTriangles[i].index_xyz[1]);
        AI_SWAP2( pcTriangles[i].index_xyz[2]);
        AI_SWAP2( pcTriangles[i].index_uv[0]);
        AI_SWAP2( pcTriangles[i].index_uv[1]);
        AI_SWAP2( pcTriangles[i].index_uv[2]);
    }

#endif

    VALIDATE_FILE_SIZE(szCurrent);

    // setup materials
    SetupMaterialProperties_3DGS_MDL5_Quake1();

    // allocate enough storage to hold all vertices and triangles
    aiMesh* pcMesh = new aiMesh();
    pcMesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;

    pcMesh->mNumVertices = pcHeader->num_tris * 3;
    pcMesh->mNumFaces = pcHeader->num_tris;
    pcMesh->mFaces = new aiFace[pcMesh->mNumFaces];

    // there won't be more than one mesh inside the file
    pScene->mRootNode = new aiNode();
    pScene->mRootNode->mNumMeshes = 1;
    pScene->mRootNode->mMeshes = new unsigned int[1];
    pScene->mRootNode->mMeshes[0] = 0;
    pScene->mNumMeshes = 1;
    pScene->mMeshes = new aiMesh*[1];
    pScene->mMeshes[0] = pcMesh;

    // allocate output storage
    pcMesh->mNumVertices = (unsigned int)pcHeader->num_tris*3;
    pcMesh->mVertices    = new aiVector3D[pcMesh->mNumVertices];
    pcMesh->mNormals     = new aiVector3D[pcMesh->mNumVertices];

    if (pcHeader->synctype) {
        pcMesh->mTextureCoords[0] = new aiVector3D[pcMesh->mNumVertices];
        pcMesh->mNumUVComponents[0] = 2;
    }

    // now get a pointer to the first frame in the file
    BE_NCONST MDL::Frame* pcFrames = (BE_NCONST MDL::Frame*)szCurrent;
    AI_SWAP4(pcFrames->type);

    // byte packed vertices
    // FIXME: these two snippets below are almost identical ... join them?
    /////////////////////////////////////////////////////////////////////////////////////
    if (0 == pcFrames->type || 3 >= this->iGSFileVersion)   {

        const MDL::SimpleFrame* pcFirstFrame = (const MDL::SimpleFrame*)(szCurrent + sizeof(uint32_t));
        const MDL::Vertex* pcVertices = (const MDL::Vertex*) ((pcFirstFrame->name) + sizeof(pcFirstFrame->name));

        VALIDATE_FILE_SIZE(pcVertices + pcHeader->num_verts);

        // now iterate through all triangles
        unsigned int iCurrent = 0;
        for (unsigned int i = 0; i < (unsigned int) pcHeader->num_tris;++i) {
            pcMesh->mFaces[i].mIndices = new unsigned int[3];
            pcMesh->mFaces[i].mNumIndices = 3;

            unsigned int iTemp = iCurrent;
            for (unsigned int c = 0; c < 3;++c,++iCurrent)  {
                // read vertices
                unsigned int iIndex = pcTriangles->index_xyz[c];
                if (iIndex >= (unsigned int)pcHeader->num_verts)    {
                    iIndex = pcHeader->num_verts-1;
                    ASSIMP_LOG_WARN("Index overflow in MDLn vertex list");
                }

                aiVector3D& vec = pcMesh->mVertices[iCurrent];
                vec.x = (float)pcVertices[iIndex].v[0] * pcHeader->scale[0];
                vec.x += pcHeader->translate[0];

                vec.y = (float)pcVertices[iIndex].v[1] * pcHeader->scale[1];
                vec.y += pcHeader->translate[1];
                // vec.y *= -1.0f;

                vec.z = (float)pcVertices[iIndex].v[2] * pcHeader->scale[2];
                vec.z += pcHeader->translate[2];

                // read the normal vector from the precalculated normal table
                MD2::LookupNormalIndex(pcVertices[iIndex].normalIndex,pcMesh->mNormals[iCurrent]);
                // pcMesh->mNormals[iCurrent].y *= -1.0f;

                // read texture coordinates
                if (pcHeader->synctype) {
                    ImportUVCoordinate_3DGS_MDL345(pcMesh->mTextureCoords[0][iCurrent],
                        pcTexCoords,pcTriangles->index_uv[c]);
                }
            }
            pcMesh->mFaces[i].mIndices[0] = iTemp+2;
            pcMesh->mFaces[i].mIndices[1] = iTemp+1;
            pcMesh->mFaces[i].mIndices[2] = iTemp+0;
            pcTriangles++;
        }

    }
    // short packed vertices
    /////////////////////////////////////////////////////////////////////////////////////
    else    {
        // now get a pointer to the first frame in the file
        const MDL::SimpleFrame_MDLn_SP* pcFirstFrame = (const MDL::SimpleFrame_MDLn_SP*) (szCurrent + sizeof(uint32_t));

        // get a pointer to the vertices
        const MDL::Vertex_MDL4* pcVertices = (const MDL::Vertex_MDL4*) ((pcFirstFrame->name) +
            sizeof(pcFirstFrame->name));

        VALIDATE_FILE_SIZE(pcVertices + pcHeader->num_verts);

        // now iterate through all triangles
        unsigned int iCurrent = 0;
        for (unsigned int i = 0; i < (unsigned int) pcHeader->num_tris;++i) {
            pcMesh->mFaces[i].mIndices = new unsigned int[3];
            pcMesh->mFaces[i].mNumIndices = 3;

            unsigned int iTemp = iCurrent;
            for (unsigned int c = 0; c < 3;++c,++iCurrent)  {
                // read vertices
                unsigned int iIndex = pcTriangles->index_xyz[c];
                if (iIndex >= (unsigned int)pcHeader->num_verts)    {
                    iIndex = pcHeader->num_verts-1;
                    ASSIMP_LOG_WARN("Index overflow in MDLn vertex list");
                }

                aiVector3D& vec = pcMesh->mVertices[iCurrent];
                vec.x = (float)pcVertices[iIndex].v[0] * pcHeader->scale[0];
                vec.x += pcHeader->translate[0];

                vec.y = (float)pcVertices[iIndex].v[1] * pcHeader->scale[1];
                vec.y += pcHeader->translate[1];
                // vec.y *= -1.0f;

                vec.z = (float)pcVertices[iIndex].v[2] * pcHeader->scale[2];
                vec.z += pcHeader->translate[2];

                // read the normal vector from the precalculated normal table
                MD2::LookupNormalIndex(pcVertices[iIndex].normalIndex,pcMesh->mNormals[iCurrent]);
                // pcMesh->mNormals[iCurrent].y *= -1.0f;

                // read texture coordinates
                if (pcHeader->synctype) {
                    ImportUVCoordinate_3DGS_MDL345(pcMesh->mTextureCoords[0][iCurrent],
                        pcTexCoords,pcTriangles->index_uv[c]);
                }
            }
            pcMesh->mFaces[i].mIndices[0] = iTemp+2;
            pcMesh->mFaces[i].mIndices[1] = iTemp+1;
            pcMesh->mFaces[i].mIndices[2] = iTemp+0;
            pcTriangles++;
        }
    }

    // For MDL5 we will need to build valid texture coordinates
    // basing upon the file loaded (only support one file as skin)
    if (0x5 == iGSFileVersion)
        CalculateUVCoordinates_MDL5();
    return;
}

// ------------------------------------------------------------------------------------------------
// Get a single UV coordinate for Quake and older GameStudio files
void MDLImporter::ImportUVCoordinate_3DGS_MDL345(
    aiVector3D& vOut,
    const MDL::TexCoord_MDL3* pcSrc,
    unsigned int iIndex)
{
    ai_assert(NULL != pcSrc);
    const MDL::Header* const pcHeader = (const MDL::Header*)this->mBuffer;

    // validate UV indices
    if (iIndex >= (unsigned int) pcHeader->synctype)    {
        iIndex = pcHeader->synctype-1;
        ASSIMP_LOG_WARN("Index overflow in MDLn UV coord list");
    }

    float s = (float)pcSrc[iIndex].u;
    float t = (float)pcSrc[iIndex].v;

    // Scale s and t to range from 0.0 to 1.0
    if (0x5 != iGSFileVersion)  {
        s = (s + 0.5f)      / pcHeader->skinwidth;
        t = 1.0f-(t + 0.5f) / pcHeader->skinheight;
    }

    vOut.x = s;
    vOut.y = t;
    vOut.z = 0.0f;
}

// ------------------------------------------------------------------------------------------------
// Compute UV coordinates for a MDL5 file
void MDLImporter::CalculateUVCoordinates_MDL5()
{
    const MDL::Header* const pcHeader = (const MDL::Header*)this->mBuffer;
    if (pcHeader->num_skins && this->pScene->mNumTextures)  {
        const aiTexture* pcTex = this->pScene->mTextures[0];

        // if the file is loaded in DDS format: get the size of the
        // texture from the header of the DDS file
        // skip three DWORDs and read first height, then the width
        unsigned int iWidth, iHeight;
        if (!pcTex->mHeight)    {
            const uint32_t* piPtr = (uint32_t*)pcTex->pcData;

            piPtr += 3;
            iHeight = (unsigned int)*piPtr++;
            iWidth  = (unsigned int)*piPtr;
            if (!iHeight || !iWidth)
            {
                ASSIMP_LOG_WARN("Either the width or the height of the "
                    "embedded DDS texture is zero. Unable to compute final texture "
                    "coordinates. The texture coordinates remain in their original "
                    "0-x/0-y (x,y = texture size) range.");
                iWidth  = 1;
                iHeight = 1;
            }
        }
        else    {
            iWidth  = pcTex->mWidth;
            iHeight = pcTex->mHeight;
        }

        if (1 != iWidth || 1 != iHeight)    {
            const float fWidth = (float)iWidth;
            const float fHeight = (float)iHeight;
            aiMesh* pcMesh = this->pScene->mMeshes[0];
            for (unsigned int i = 0; i < pcMesh->mNumVertices;++i)
            {
                pcMesh->mTextureCoords[0][i].x /= fWidth;
                pcMesh->mTextureCoords[0][i].y /= fHeight;
                pcMesh->mTextureCoords[0][i].y = 1.0f - pcMesh->mTextureCoords[0][i].y; // DX to OGL
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Validate the header of a MDL7 file
void MDLImporter::ValidateHeader_3DGS_MDL7(const MDL::Header_MDL7* pcHeader)
{
    ai_assert(NULL != pcHeader);

    // There are some fixed sizes ...
    if (sizeof(MDL::ColorValue_MDL7) != pcHeader->colorvalue_stc_size)  {
        throw DeadlyImportError(
            "[3DGS MDL7] sizeof(MDL::ColorValue_MDL7) != pcHeader->colorvalue_stc_size");
    }
    if (sizeof(MDL::TexCoord_MDL7) != pcHeader->skinpoint_stc_size) {
        throw DeadlyImportError(
            "[3DGS MDL7] sizeof(MDL::TexCoord_MDL7) != pcHeader->skinpoint_stc_size");
    }
    if (sizeof(MDL::Skin_MDL7) != pcHeader->skin_stc_size)  {
        throw DeadlyImportError(
            "sizeof(MDL::Skin_MDL7) != pcHeader->skin_stc_size");
    }

    // if there are no groups ... how should we load such a file?
    if(!pcHeader->groups_num)   {
        throw DeadlyImportError( "[3DGS MDL7] No frames found");
    }
}

// ------------------------------------------------------------------------------------------------
// resolve bone animation matrices
void MDLImporter::CalcAbsBoneMatrices_3DGS_MDL7(MDL::IntBone_MDL7** apcOutBones)
{
    const MDL::Header_MDL7 *pcHeader = (const MDL::Header_MDL7*)this->mBuffer;
    const MDL::Bone_MDL7* pcBones = (const MDL::Bone_MDL7*)(pcHeader+1);
    ai_assert(NULL != apcOutBones);

    // first find the bone that has NO parent, calculate the
    // animation matrix for it, then go on and search for the next parent
    // index (0) and so on until we can't find a new node.
    uint16_t iParent = 0xffff;
    uint32_t iIterations = 0;
    while (iIterations++ < pcHeader->bones_num) {
        for (uint32_t iBone = 0; iBone < pcHeader->bones_num;++iBone)   {
            BE_NCONST MDL::Bone_MDL7* pcBone = _AI_MDL7_ACCESS_PTR(pcBones,iBone,
                pcHeader->bone_stc_size,MDL::Bone_MDL7);

            AI_SWAP2(pcBone->parent_index);
            AI_SWAP4(pcBone->x);
            AI_SWAP4(pcBone->y);
            AI_SWAP4(pcBone->z);

            if (iParent == pcBone->parent_index)    {
                // MDL7 readme
                ////////////////////////////////////////////////////////////////
                /*
                The animation matrix is then calculated the following way:

                vector3 bPos = <absolute bone position>
                matrix44 laM;   // local animation matrix
                sphrvector key_rotate = <bone rotation>

                matrix44 m1,m2;
                create_trans_matrix(m1, -bPos.x, -bPos.y, -bPos.z);
                create_trans_matrix(m2, -bPos.x, -bPos.y, -bPos.z);

                create_rotation_matrix(laM,key_rotate);

                laM = sm1 * laM;
                laM = laM * sm2;
                */
                /////////////////////////////////////////////////////////////////

                MDL::IntBone_MDL7* const pcOutBone = apcOutBones[iBone];

                // store the parent index of the bone
                pcOutBone->iParent = pcBone->parent_index;
                if (0xffff != iParent)  {
                    const MDL::IntBone_MDL7* pcParentBone = apcOutBones[iParent];
                    pcOutBone->mOffsetMatrix.a4 = -pcParentBone->vPosition.x;
                    pcOutBone->mOffsetMatrix.b4 = -pcParentBone->vPosition.y;
                    pcOutBone->mOffsetMatrix.c4 = -pcParentBone->vPosition.z;
                }
                pcOutBone->vPosition.x = pcBone->x;
                pcOutBone->vPosition.y = pcBone->y;
                pcOutBone->vPosition.z = pcBone->z;
                pcOutBone->mOffsetMatrix.a4 -= pcBone->x;
                pcOutBone->mOffsetMatrix.b4 -= pcBone->y;
                pcOutBone->mOffsetMatrix.c4 -= pcBone->z;

                if (AI_MDL7_BONE_STRUCT_SIZE__NAME_IS_NOT_THERE == pcHeader->bone_stc_size) {
                    // no real name for our poor bone is specified :-(
                    pcOutBone->mName.length = ai_snprintf(pcOutBone->mName.data, MAXLEN,
                        "UnnamedBone_%i",iBone);
                }
                else    {
                    // Make sure we won't run over the buffer's end if there is no
                    // terminal 0 character (however the documentation says there
                    // should be one)
                    uint32_t iMaxLen = pcHeader->bone_stc_size-16;
                    for (uint32_t qq = 0; qq < iMaxLen;++qq)    {
                        if (!pcBone->name[qq])  {
                            iMaxLen = qq;
                            break;
                        }
                    }

                    // store the name of the bone
                    pcOutBone->mName.length = (size_t)iMaxLen;
                    ::memcpy(pcOutBone->mName.data,pcBone->name,pcOutBone->mName.length);
                    pcOutBone->mName.data[pcOutBone->mName.length] = '\0';
                }
            }
        }
        ++iParent;
    }
}

// ------------------------------------------------------------------------------------------------
// read bones from a MDL7 file
MDL::IntBone_MDL7** MDLImporter::LoadBones_3DGS_MDL7()
{
  const MDL::Header_MDL7 *pcHeader = (const MDL::Header_MDL7*)this->mBuffer;
    if (pcHeader->bones_num)    {
        // validate the size of the bone data structure in the file
        if (AI_MDL7_BONE_STRUCT_SIZE__NAME_IS_20_CHARS  != pcHeader->bone_stc_size &&
            AI_MDL7_BONE_STRUCT_SIZE__NAME_IS_32_CHARS  != pcHeader->bone_stc_size &&
            AI_MDL7_BONE_STRUCT_SIZE__NAME_IS_NOT_THERE != pcHeader->bone_stc_size)
        {
            ASSIMP_LOG_WARN("Unknown size of bone data structure");
            return NULL;
        }

        MDL::IntBone_MDL7** apcBonesOut = new MDL::IntBone_MDL7*[pcHeader->bones_num];
        for (uint32_t crank = 0; crank < pcHeader->bones_num;++crank)
            apcBonesOut[crank] = new MDL::IntBone_MDL7();

        // and calculate absolute bone offset matrices ...
        CalcAbsBoneMatrices_3DGS_MDL7(apcBonesOut);
        return apcBonesOut;
    }
    return NULL;
}

// ------------------------------------------------------------------------------------------------
// read faces from a MDL7 file
void MDLImporter::ReadFaces_3DGS_MDL7(const MDL::IntGroupInfo_MDL7& groupInfo,
    MDL::IntGroupData_MDL7& groupData)
{
    const MDL::Header_MDL7 *pcHeader = (const MDL::Header_MDL7*)this->mBuffer;
    MDL::Triangle_MDL7* pcGroupTris = groupInfo.pcGroupTris;

    // iterate through all triangles and build valid display lists
    unsigned int iOutIndex = 0;
    for (unsigned int iTriangle = 0; iTriangle < (unsigned int)groupInfo.pcGroup->numtris; ++iTriangle) {
        AI_SWAP2(pcGroupTris->v_index[0]);
        AI_SWAP2(pcGroupTris->v_index[1]);
        AI_SWAP2(pcGroupTris->v_index[2]);

        // iterate through all indices of the current triangle
        for (unsigned int c = 0; c < 3;++c,++iOutIndex) {

            // validate the vertex index
            unsigned int iIndex = pcGroupTris->v_index[c];
            if(iIndex > (unsigned int)groupInfo.pcGroup->numverts)  {
                // (we might need to read this section a second time - to process frame vertices correctly)
                pcGroupTris->v_index[c] = iIndex = groupInfo.pcGroup->numverts-1;
                ASSIMP_LOG_WARN("Index overflow in MDL7 vertex list");
            }

            // write the output face index
            groupData.pcFaces[iTriangle].mIndices[2-c] = iOutIndex;

            aiVector3D& vPosition = groupData.vPositions[ iOutIndex ];
            vPosition.x = _AI_MDL7_ACCESS_VERT(groupInfo.pcGroupVerts,iIndex, pcHeader->mainvertex_stc_size) .x;
            vPosition.y = _AI_MDL7_ACCESS_VERT(groupInfo.pcGroupVerts,iIndex,pcHeader->mainvertex_stc_size) .y;
            vPosition.z = _AI_MDL7_ACCESS_VERT(groupInfo.pcGroupVerts,iIndex,pcHeader->mainvertex_stc_size) .z;

            // if we have bones, save the index
            if (!groupData.aiBones.empty()) {
                groupData.aiBones[iOutIndex] = _AI_MDL7_ACCESS_VERT(groupInfo.pcGroupVerts,
                    iIndex,pcHeader->mainvertex_stc_size).vertindex;
            }

            // now read the normal vector
            if (AI_MDL7_FRAMEVERTEX030305_STCSIZE <= pcHeader->mainvertex_stc_size) {
                // read the full normal vector
                aiVector3D& vNormal = groupData.vNormals[ iOutIndex ];
                vNormal.x = _AI_MDL7_ACCESS_VERT(groupInfo.pcGroupVerts,iIndex,pcHeader->mainvertex_stc_size) .norm[0];
                AI_SWAP4(vNormal.x);
                vNormal.y = _AI_MDL7_ACCESS_VERT(groupInfo.pcGroupVerts,iIndex,pcHeader->mainvertex_stc_size) .norm[1];
                AI_SWAP4(vNormal.y);
                vNormal.z = _AI_MDL7_ACCESS_VERT(groupInfo.pcGroupVerts,iIndex,pcHeader->mainvertex_stc_size) .norm[2];
                AI_SWAP4(vNormal.z);
            }
            else if (AI_MDL7_FRAMEVERTEX120503_STCSIZE <= pcHeader->mainvertex_stc_size)    {
                // read the normal vector from Quake2's smart table
                aiVector3D& vNormal = groupData.vNormals[ iOutIndex ];
                MD2::LookupNormalIndex(_AI_MDL7_ACCESS_VERT(groupInfo.pcGroupVerts,iIndex,
                    pcHeader->mainvertex_stc_size) .norm162index,vNormal);
            }
            // validate and process the first uv coordinate set
            if (pcHeader->triangle_stc_size >= AI_MDL7_TRIANGLE_STD_SIZE_ONE_UV)    {

                if (groupInfo.pcGroup->num_stpts)   {
                    AI_SWAP2(pcGroupTris->skinsets[0].st_index[0]);
                    AI_SWAP2(pcGroupTris->skinsets[0].st_index[1]);
                    AI_SWAP2(pcGroupTris->skinsets[0].st_index[2]);

                    iIndex = pcGroupTris->skinsets[0].st_index[c];
                    if(iIndex > (unsigned int)groupInfo.pcGroup->num_stpts) {
                        iIndex = groupInfo.pcGroup->num_stpts-1;
                        ASSIMP_LOG_WARN("Index overflow in MDL7 UV coordinate list (#1)");
                    }

                    float u = groupInfo.pcGroupUVs[iIndex].u;
                    float v = 1.0f-groupInfo.pcGroupUVs[iIndex].v; // DX to OGL

                    groupData.vTextureCoords1[iOutIndex].x = u;
                    groupData.vTextureCoords1[iOutIndex].y = v;
                }
                // assign the material index, but only if it is existing
                if (pcHeader->triangle_stc_size >= AI_MDL7_TRIANGLE_STD_SIZE_ONE_UV_WITH_MATINDEX){
                    AI_SWAP4(pcGroupTris->skinsets[0].material);
                    groupData.pcFaces[iTriangle].iMatIndex[0] = pcGroupTris->skinsets[0].material;
                }
            }
            // validate and process the second uv coordinate set
            if (pcHeader->triangle_stc_size >= AI_MDL7_TRIANGLE_STD_SIZE_TWO_UV)    {

                if (groupInfo.pcGroup->num_stpts)   {
                    AI_SWAP2(pcGroupTris->skinsets[1].st_index[0]);
                    AI_SWAP2(pcGroupTris->skinsets[1].st_index[1]);
                    AI_SWAP2(pcGroupTris->skinsets[1].st_index[2]);
                    AI_SWAP4(pcGroupTris->skinsets[1].material);

                    iIndex = pcGroupTris->skinsets[1].st_index[c];
                    if(iIndex > (unsigned int)groupInfo.pcGroup->num_stpts) {
                        iIndex = groupInfo.pcGroup->num_stpts-1;
                        ASSIMP_LOG_WARN("Index overflow in MDL7 UV coordinate list (#2)");
                    }

                    float u = groupInfo.pcGroupUVs[ iIndex ].u;
                    float v = 1.0f-groupInfo.pcGroupUVs[ iIndex ].v;

                    groupData.vTextureCoords2[ iOutIndex ].x = u;
                    groupData.vTextureCoords2[ iOutIndex ].y = v; // DX to OGL

                    // check whether we do really need the second texture
                    // coordinate set ... wastes memory and loading time
                    if (0 != iIndex && (u != groupData.vTextureCoords1[ iOutIndex ].x ||
                        v != groupData.vTextureCoords1[ iOutIndex ].y ) )
                        groupData.bNeed2UV = true;

                    // if the material differs, we need a second skin, too
                    if (pcGroupTris->skinsets[ 1 ].material != pcGroupTris->skinsets[ 0 ].material)
                        groupData.bNeed2UV = true;
                }
                // assign the material index
                groupData.pcFaces[ iTriangle ].iMatIndex[ 1 ] = pcGroupTris->skinsets[ 1 ].material;
            }
        }
        // get the next triangle in the list
        pcGroupTris = (MDL::Triangle_MDL7*)((const char*)pcGroupTris + pcHeader->triangle_stc_size);
    }
}

// ------------------------------------------------------------------------------------------------
// handle frames in a MDL7 file
bool MDLImporter::ProcessFrames_3DGS_MDL7(const MDL::IntGroupInfo_MDL7& groupInfo,
    MDL::IntGroupData_MDL7&  groupData,
    MDL::IntSharedData_MDL7& shared,
    const unsigned char*     szCurrent,
    const unsigned char**    szCurrentOut)
{
    ai_assert( nullptr != szCurrent );
    ai_assert( nullptr != szCurrentOut);

    const MDL::Header_MDL7 *pcHeader = (const MDL::Header_MDL7*)mBuffer;

    // if we have no bones we can simply skip all frames,
    // otherwise we'll need to process them.
    // FIX: If we need another frame than the first we must apply frame vertex replacements ...
    for(unsigned int iFrame = 0; iFrame < (unsigned int)groupInfo.pcGroup->numframes;++iFrame)  {
        MDL::IntFrameInfo_MDL7 frame ((BE_NCONST MDL::Frame_MDL7*)szCurrent,iFrame);

        AI_SWAP4(frame.pcFrame->vertices_count);
        AI_SWAP4(frame.pcFrame->transmatrix_count);

        const unsigned int iAdd = pcHeader->frame_stc_size +
            frame.pcFrame->vertices_count * pcHeader->framevertex_stc_size +
            frame.pcFrame->transmatrix_count * pcHeader->bonetrans_stc_size;

        if (((const char*)szCurrent - (const char*)pcHeader) + iAdd > (unsigned int)pcHeader->data_size)    {
            ASSIMP_LOG_WARN("Index overflow in frame area. "
                "Ignoring all frames and all further mesh groups, too.");

            // don't parse more groups if we can't even read one
            // FIXME: sometimes this seems to occur even for valid files ...
            *szCurrentOut = szCurrent;
            return false;
        }
        // our output frame?
        if (configFrameID == iFrame)    {
            BE_NCONST MDL::Vertex_MDL7* pcFrameVertices = (BE_NCONST MDL::Vertex_MDL7*)(szCurrent+pcHeader->frame_stc_size);

            for (unsigned int qq = 0; qq < frame.pcFrame->vertices_count;++qq)  {
                // I assume this are simple replacements for normal vertices, the bone index serving
                // as the index of the vertex to be replaced.
                uint16_t iIndex = _AI_MDL7_ACCESS(pcFrameVertices,qq,pcHeader->framevertex_stc_size,MDL::Vertex_MDL7).vertindex;
                AI_SWAP2(iIndex);
                if (iIndex >= groupInfo.pcGroup->numverts)  {
                    ASSIMP_LOG_WARN("Invalid vertex index in frame vertex section");
                    continue;
                }

                aiVector3D vPosition,vNormal;

                vPosition.x = _AI_MDL7_ACCESS_VERT(pcFrameVertices,qq,pcHeader->framevertex_stc_size) .x;
                AI_SWAP4(vPosition.x);
                vPosition.y = _AI_MDL7_ACCESS_VERT(pcFrameVertices,qq,pcHeader->framevertex_stc_size) .y;
                AI_SWAP4(vPosition.y);
                vPosition.z = _AI_MDL7_ACCESS_VERT(pcFrameVertices,qq,pcHeader->framevertex_stc_size) .z;
                AI_SWAP4(vPosition.z);

                // now read the normal vector
                if (AI_MDL7_FRAMEVERTEX030305_STCSIZE <= pcHeader->mainvertex_stc_size) {
                    // read the full normal vector
                    vNormal.x = _AI_MDL7_ACCESS_VERT(pcFrameVertices,qq,pcHeader->framevertex_stc_size) .norm[0];
                    AI_SWAP4(vNormal.x);
                    vNormal.y = _AI_MDL7_ACCESS_VERT(pcFrameVertices,qq,pcHeader->framevertex_stc_size) .norm[1];
                    AI_SWAP4(vNormal.y);
                    vNormal.z = _AI_MDL7_ACCESS_VERT(pcFrameVertices,qq,pcHeader->framevertex_stc_size) .norm[2];
                    AI_SWAP4(vNormal.z);
                }
                else if (AI_MDL7_FRAMEVERTEX120503_STCSIZE <= pcHeader->mainvertex_stc_size)    {
                    // read the normal vector from Quake2's smart table
                    MD2::LookupNormalIndex(_AI_MDL7_ACCESS_VERT(pcFrameVertices,qq,
                        pcHeader->framevertex_stc_size) .norm162index,vNormal);
                }

                // FIXME: O(n^2) at the moment ...
                BE_NCONST MDL::Triangle_MDL7* pcGroupTris = groupInfo.pcGroupTris;
                unsigned int iOutIndex = 0;
                for (unsigned int iTriangle = 0; iTriangle < (unsigned int)groupInfo.pcGroup->numtris; ++iTriangle) {
                    // iterate through all indices of the current triangle
                    for (unsigned int c = 0; c < 3;++c,++iOutIndex) {
                        // replace the vertex with the new data
                        const unsigned int iCurIndex = pcGroupTris->v_index[c];
                        if (iCurIndex == iIndex)    {
                            groupData.vPositions[iOutIndex] = vPosition;
                            groupData.vNormals[iOutIndex] = vNormal;
                        }
                    }
                    // get the next triangle in the list
                    pcGroupTris = (BE_NCONST MDL::Triangle_MDL7*)((const char*)
                        pcGroupTris + pcHeader->triangle_stc_size);
                }
            }
        }
        // parse bone trafo matrix keys (only if there are bones ...)
        if (shared.apcOutBones) {
            ParseBoneTrafoKeys_3DGS_MDL7(groupInfo,frame,shared);
        }
        szCurrent += iAdd;
    }
    *szCurrentOut = szCurrent;
    return true;
}

// ------------------------------------------------------------------------------------------------
// Sort faces by material, handle multiple UVs correctly
void MDLImporter::SortByMaterials_3DGS_MDL7(
    const MDL::IntGroupInfo_MDL7&   groupInfo,
    MDL::IntGroupData_MDL7&         groupData,
    MDL::IntSplitGroupData_MDL7& splitGroupData)
{
    const unsigned int iNumMaterials = (unsigned int)splitGroupData.shared.pcMats.size();
    if (!groupData.bNeed2UV)    {
        // if we don't need a second set of texture coordinates there is no reason to keep it in memory ...
        groupData.vTextureCoords2.clear();

        // allocate the array
        splitGroupData.aiSplit = new std::vector<unsigned int>*[iNumMaterials];

        for (unsigned int m = 0; m < iNumMaterials;++m)
            splitGroupData.aiSplit[m] = new std::vector<unsigned int>();

        // iterate through all faces and sort by material
        for (unsigned int iFace = 0; iFace < (unsigned int)groupInfo.pcGroup->numtris;++iFace)  {
            // check range
            if (groupData.pcFaces[iFace].iMatIndex[0] >= iNumMaterials) {
                // use the last material instead
                splitGroupData.aiSplit[iNumMaterials-1]->push_back(iFace);

                // sometimes MED writes -1, but normally only if there is only
                // one skin assigned. No warning in this case
                if(0xFFFFFFFF != groupData.pcFaces[iFace].iMatIndex[0])
                    ASSIMP_LOG_WARN("Index overflow in MDL7 material list [#0]");
            }
            else splitGroupData.aiSplit[groupData.pcFaces[iFace].
                iMatIndex[0]]->push_back(iFace);
        }
    }
    else
    {
        // we need to build combined materials for each combination of
        std::vector<MDL::IntMaterial_MDL7> avMats;
        avMats.reserve(iNumMaterials*2);

        // fixme: why on the heap?
        std::vector<std::vector<unsigned int>* > aiTempSplit(iNumMaterials*2);
        for (unsigned int m = 0; m < iNumMaterials;++m)
            aiTempSplit[m] = new std::vector<unsigned int>();

        // iterate through all faces and sort by material
        for (unsigned int iFace = 0; iFace < (unsigned int)groupInfo.pcGroup->numtris;++iFace)  {
            // check range
            unsigned int iMatIndex = groupData.pcFaces[iFace].iMatIndex[0];
            if (iMatIndex >= iNumMaterials) {
                // sometimes MED writes -1, but normally only if there is only
                // one skin assigned. No warning in this case
                if(UINT_MAX != iMatIndex)
                    ASSIMP_LOG_WARN("Index overflow in MDL7 material list [#1]");
                iMatIndex = iNumMaterials-1;
            }
            unsigned int iMatIndex2 = groupData.pcFaces[iFace].iMatIndex[1];

            unsigned int iNum = iMatIndex;
            if (UINT_MAX != iMatIndex2 && iMatIndex != iMatIndex2)  {
                if (iMatIndex2 >= iNumMaterials)    {
                    // sometimes MED writes -1, but normally only if there is only
                    // one skin assigned. No warning in this case
                    ASSIMP_LOG_WARN("Index overflow in MDL7 material list [#2]");
                    iMatIndex2 = iNumMaterials-1;
                }

                // do a slow search in the list ...
                iNum = 0;
                bool bFound = false;
                for (std::vector<MDL::IntMaterial_MDL7>::iterator i =  avMats.begin();i != avMats.end();++i,++iNum){
                    if ((*i).iOldMatIndices[0] == iMatIndex && (*i).iOldMatIndices[1] == iMatIndex2)    {
                        // reuse this material
                        bFound = true;
                        break;
                    }
                }
                if (!bFound)    {
                    //  build a new material ...
                    MDL::IntMaterial_MDL7 sHelper;
                    sHelper.pcMat = new aiMaterial();
                    sHelper.iOldMatIndices[0] = iMatIndex;
                    sHelper.iOldMatIndices[1] = iMatIndex2;
                    JoinSkins_3DGS_MDL7(splitGroupData.shared.pcMats[iMatIndex],
                        splitGroupData.shared.pcMats[iMatIndex2],sHelper.pcMat);

                    // and add it to the list
                    avMats.push_back(sHelper);
                    iNum = (unsigned int)avMats.size()-1;
                }
                // adjust the size of the file array
                if (iNum == aiTempSplit.size()) {
                    aiTempSplit.push_back(new std::vector<unsigned int>());
                }
            }
            aiTempSplit[iNum]->push_back(iFace);
        }

        // now add the newly created materials to the old list
        if (0 == groupInfo.iIndex)  {
            splitGroupData.shared.pcMats.resize(avMats.size());
            for (unsigned int o = 0; o < avMats.size();++o)
                splitGroupData.shared.pcMats[o] = avMats[o].pcMat;
        }
        else    {
            // This might result in redundant materials ...
            splitGroupData.shared.pcMats.resize(iNumMaterials + avMats.size());
            for (unsigned int o = iNumMaterials; o < avMats.size();++o)
                splitGroupData.shared.pcMats[o] = avMats[o].pcMat;
        }

        // and build the final face-to-material array
        splitGroupData.aiSplit = new std::vector<unsigned int>*[aiTempSplit.size()];
        for (unsigned int m = 0; m < iNumMaterials;++m)
            splitGroupData.aiSplit[m] = aiTempSplit[m];
    }
}

// ------------------------------------------------------------------------------------------------
// Read a MDL7 file
void MDLImporter::InternReadFile_3DGS_MDL7( )
{
    ai_assert(NULL != pScene);

    MDL::IntSharedData_MDL7 sharedData;

    // current cursor position in the file
    BE_NCONST MDL::Header_MDL7 *pcHeader = (BE_NCONST MDL::Header_MDL7*)this->mBuffer;
    const unsigned char* szCurrent = (const unsigned char*)(pcHeader+1);

    AI_SWAP4(pcHeader->version);
    AI_SWAP4(pcHeader->bones_num);
    AI_SWAP4(pcHeader->groups_num);
    AI_SWAP4(pcHeader->data_size);
    AI_SWAP4(pcHeader->entlump_size);
    AI_SWAP4(pcHeader->medlump_size);
    AI_SWAP2(pcHeader->bone_stc_size);
    AI_SWAP2(pcHeader->skin_stc_size);
    AI_SWAP2(pcHeader->colorvalue_stc_size);
    AI_SWAP2(pcHeader->material_stc_size);
    AI_SWAP2(pcHeader->skinpoint_stc_size);
    AI_SWAP2(pcHeader->triangle_stc_size);
    AI_SWAP2(pcHeader->mainvertex_stc_size);
    AI_SWAP2(pcHeader->framevertex_stc_size);
    AI_SWAP2(pcHeader->bonetrans_stc_size);
    AI_SWAP2(pcHeader->frame_stc_size);

    // validate the header of the file. There are some structure
    // sizes that are expected by the loader to be constant
    this->ValidateHeader_3DGS_MDL7(pcHeader);

    // load all bones (they are shared by all groups, so
    // we'll need to add them to all groups/meshes later)
    // apcBonesOut is a list of all bones or NULL if they could not been loaded
    szCurrent += pcHeader->bones_num * pcHeader->bone_stc_size;
    sharedData.apcOutBones = this->LoadBones_3DGS_MDL7();

    // vector to held all created meshes
    std::vector<aiMesh*>* avOutList;

    // 3 meshes per group - that should be OK for most models
    avOutList = new std::vector<aiMesh*>[pcHeader->groups_num];
    for (uint32_t i = 0; i < pcHeader->groups_num;++i)
        avOutList[i].reserve(3);

    // buffer to held the names of all groups in the file
	const size_t buffersize( AI_MDL7_MAX_GROUPNAMESIZE*pcHeader->groups_num );
	char* aszGroupNameBuffer = new char[ buffersize ];

    // read all groups
    for (unsigned int iGroup = 0; iGroup < (unsigned int)pcHeader->groups_num;++iGroup) {
        MDL::IntGroupInfo_MDL7 groupInfo((BE_NCONST MDL::Group_MDL7*)szCurrent,iGroup);
        szCurrent = (const unsigned char*)(groupInfo.pcGroup+1);

        VALIDATE_FILE_SIZE(szCurrent);

        AI_SWAP4(groupInfo.pcGroup->groupdata_size);
        AI_SWAP4(groupInfo.pcGroup->numskins);
        AI_SWAP4(groupInfo.pcGroup->num_stpts);
        AI_SWAP4(groupInfo.pcGroup->numtris);
        AI_SWAP4(groupInfo.pcGroup->numverts);
        AI_SWAP4(groupInfo.pcGroup->numframes);

        if (1 != groupInfo.pcGroup->typ)    {
            // Not a triangle-based mesh
            ASSIMP_LOG_WARN("[3DGS MDL7] Not a triangle mesh group. Continuing happily");
        }

        // store the name of the group
        const unsigned int ofs = iGroup*AI_MDL7_MAX_GROUPNAMESIZE;
        ::memcpy(&aszGroupNameBuffer[ofs],
            groupInfo.pcGroup->name,AI_MDL7_MAX_GROUPNAMESIZE);

        // make sure '\0' is at the end
        aszGroupNameBuffer[ofs+AI_MDL7_MAX_GROUPNAMESIZE-1] = '\0';

        // read all skins
        sharedData.pcMats.reserve(sharedData.pcMats.size() + groupInfo.pcGroup->numskins);
        sharedData.abNeedMaterials.resize(sharedData.abNeedMaterials.size() +
            groupInfo.pcGroup->numskins,false);

        for (unsigned int iSkin = 0; iSkin < (unsigned int)groupInfo.pcGroup->numskins;++iSkin) {
            ParseSkinLump_3DGS_MDL7(szCurrent,&szCurrent,sharedData.pcMats);
        }
        // if we have absolutely no skin loaded we need to generate a default material
        if (sharedData.pcMats.empty())  {
            const int iMode = (int)aiShadingMode_Gouraud;
            sharedData.pcMats.push_back(new aiMaterial());
            aiMaterial* pcHelper = (aiMaterial*)sharedData.pcMats[0];
            pcHelper->AddProperty<int>(&iMode, 1, AI_MATKEY_SHADING_MODEL);

            aiColor3D clr;
            clr.b = clr.g = clr.r = 0.6f;
            pcHelper->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_DIFFUSE);
            pcHelper->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_SPECULAR);

            clr.b = clr.g = clr.r = 0.05f;
            pcHelper->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_AMBIENT);

            aiString szName;
            szName.Set(AI_DEFAULT_MATERIAL_NAME);
            pcHelper->AddProperty(&szName,AI_MATKEY_NAME);

            sharedData.abNeedMaterials.resize(1,false);
        }

        // now get a pointer to all texture coords in the group
        groupInfo.pcGroupUVs = (BE_NCONST MDL::TexCoord_MDL7*)szCurrent;
        for(int i = 0; i < groupInfo.pcGroup->num_stpts; ++i){
            AI_SWAP4(groupInfo.pcGroupUVs[i].u);
            AI_SWAP4(groupInfo.pcGroupUVs[i].v);
        }
        szCurrent += pcHeader->skinpoint_stc_size * groupInfo.pcGroup->num_stpts;

        // now get a pointer to all triangle in the group
        groupInfo.pcGroupTris = (Triangle_MDL7*)szCurrent;
        szCurrent += pcHeader->triangle_stc_size * groupInfo.pcGroup->numtris;

        // now get a pointer to all vertices in the group
        groupInfo.pcGroupVerts = (BE_NCONST MDL::Vertex_MDL7*)szCurrent;
        for(int i = 0; i < groupInfo.pcGroup->numverts; ++i){
            AI_SWAP4(groupInfo.pcGroupVerts[i].x);
            AI_SWAP4(groupInfo.pcGroupVerts[i].y);
            AI_SWAP4(groupInfo.pcGroupVerts[i].z);

            AI_SWAP2(groupInfo.pcGroupVerts[i].vertindex);
            //We can not swap the normal information now as we don't know which of the two kinds it is
        }
        szCurrent += pcHeader->mainvertex_stc_size * groupInfo.pcGroup->numverts;
        VALIDATE_FILE_SIZE(szCurrent);

        MDL::IntSplitGroupData_MDL7 splitGroupData(sharedData,avOutList[iGroup]);
        MDL::IntGroupData_MDL7 groupData;
        if (groupInfo.pcGroup->numtris && groupInfo.pcGroup->numverts)
        {
            // build output vectors
            const unsigned int iNumVertices = groupInfo.pcGroup->numtris*3;
            groupData.vPositions.resize(iNumVertices);
            groupData.vNormals.resize(iNumVertices);

            if (sharedData.apcOutBones)groupData.aiBones.resize(iNumVertices,UINT_MAX);

            // it is also possible that there are 0 UV coordinate sets
            if (groupInfo.pcGroup->num_stpts){
                groupData.vTextureCoords1.resize(iNumVertices,aiVector3D());

                // check whether the triangle data structure is large enough
                // to contain a second UV coordinate set
                if (pcHeader->triangle_stc_size >= AI_MDL7_TRIANGLE_STD_SIZE_TWO_UV)    {
                    groupData.vTextureCoords2.resize(iNumVertices,aiVector3D());
                    groupData.bNeed2UV = true;
                }
            }
            groupData.pcFaces.resize(groupInfo.pcGroup->numtris);

            // read all faces into the preallocated arrays
            ReadFaces_3DGS_MDL7(groupInfo, groupData);

            // sort by materials
            SortByMaterials_3DGS_MDL7(groupInfo, groupData,
                splitGroupData);

            for (unsigned int qq = 0; qq < sharedData.pcMats.size();++qq)   {
                if (!splitGroupData.aiSplit[qq]->empty())
                    sharedData.abNeedMaterials[qq] = true;
            }
        }
        else ASSIMP_LOG_WARN("[3DGS MDL7] Mesh group consists of 0 "
            "vertices or faces. It will be skipped.");

        // process all frames and generate output meshes
        ProcessFrames_3DGS_MDL7(groupInfo,groupData, sharedData,szCurrent,&szCurrent);
        GenerateOutputMeshes_3DGS_MDL7(groupData,splitGroupData);
    }

    // generate a nodegraph and subnodes for each group
    pScene->mRootNode = new aiNode();

    // now we need to build a final mesh list
    for (uint32_t i = 0; i < pcHeader->groups_num;++i)
        pScene->mNumMeshes += (unsigned int)avOutList[i].size();

    pScene->mMeshes = new aiMesh*[pScene->mNumMeshes];  {
        unsigned int p = 0,q = 0;
        for (uint32_t i = 0; i < pcHeader->groups_num;++i)  {
            for (unsigned int a = 0; a < avOutList[i].size();++a)   {
                pScene->mMeshes[p++] = avOutList[i][a];
            }
            if (!avOutList[i].empty())++pScene->mRootNode->mNumChildren;
        }
        // we will later need an extra node to serve as parent for all bones
        if (sharedData.apcOutBones)++pScene->mRootNode->mNumChildren;
        this->pScene->mRootNode->mChildren = new aiNode*[pScene->mRootNode->mNumChildren];
        p = 0;
        for (uint32_t i = 0; i < pcHeader->groups_num;++i)  {
            if (avOutList[i].empty())continue;

            aiNode* const pcNode = pScene->mRootNode->mChildren[p] = new aiNode();
            pcNode->mNumMeshes = (unsigned int)avOutList[i].size();
            pcNode->mMeshes = new unsigned int[pcNode->mNumMeshes];
            pcNode->mParent = this->pScene->mRootNode;
            for (unsigned int a = 0; a < pcNode->mNumMeshes;++a)
                pcNode->mMeshes[a] = q + a;
            q += (unsigned int)avOutList[i].size();

            // setup the name of the node
            char* const szBuffer = &aszGroupNameBuffer[i*AI_MDL7_MAX_GROUPNAMESIZE];
			if ('\0' == *szBuffer) {
				const size_t maxSize(buffersize - (i*AI_MDL7_MAX_GROUPNAMESIZE));
				pcNode->mName.length = ai_snprintf(szBuffer, maxSize, "Group_%u", p);
			} else {
				pcNode->mName.length = ::strlen(szBuffer);
			}
            ::strncpy(pcNode->mName.data,szBuffer,MAXLEN-1);
            ++p;
        }
    }

    // if there is only one root node with a single child we can optimize it a bit ...
    if (1 == pScene->mRootNode->mNumChildren && !sharedData.apcOutBones)    {
        aiNode* pcOldRoot = this->pScene->mRootNode;
        pScene->mRootNode = pcOldRoot->mChildren[0];
        pcOldRoot->mChildren[0] = NULL;
        delete pcOldRoot;
        pScene->mRootNode->mParent = NULL;
    }
    else pScene->mRootNode->mName.Set("<mesh_root>");

    delete[] avOutList;
    delete[] aszGroupNameBuffer;
    AI_DEBUG_INVALIDATE_PTR(avOutList);
    AI_DEBUG_INVALIDATE_PTR(aszGroupNameBuffer);

    // build a final material list.
    CopyMaterials_3DGS_MDL7(sharedData);
    HandleMaterialReferences_3DGS_MDL7();

    // generate output bone animations and add all bones to the scenegraph
    if (sharedData.apcOutBones) {
        // this step adds empty dummy bones to the nodegraph
        // insert another dummy node to avoid name conflicts
        aiNode* const pc = pScene->mRootNode->mChildren[pScene->mRootNode->mNumChildren-1] = new aiNode();

        pc->mName.Set("<skeleton_root>");

        // add bones to the nodegraph
        AddBonesToNodeGraph_3DGS_MDL7((const Assimp::MDL::IntBone_MDL7 **)
            sharedData.apcOutBones,pc,0xffff);

        // this steps build a valid output animation
        BuildOutputAnims_3DGS_MDL7((const Assimp::MDL::IntBone_MDL7 **)
            sharedData.apcOutBones);
    }
}

// ------------------------------------------------------------------------------------------------
// Copy materials
void MDLImporter::CopyMaterials_3DGS_MDL7(MDL::IntSharedData_MDL7 &shared)
{
    pScene->mNumMaterials = (unsigned int)shared.pcMats.size();
    pScene->mMaterials = new aiMaterial*[pScene->mNumMaterials];
    for (unsigned int i = 0; i < pScene->mNumMaterials;++i)
        pScene->mMaterials[i] = shared.pcMats[i];
}


// ------------------------------------------------------------------------------------------------
// Process material references
void MDLImporter::HandleMaterialReferences_3DGS_MDL7()
{
    // search for referrer materials
    for (unsigned int i = 0; i < pScene->mNumMaterials;++i) {
        int iIndex = 0;
        if (AI_SUCCESS == aiGetMaterialInteger(pScene->mMaterials[i],AI_MDL7_REFERRER_MATERIAL, &iIndex) )  {
            for (unsigned int a = 0; a < pScene->mNumMeshes;++a)    {
                aiMesh* const pcMesh = pScene->mMeshes[a];
                if (i == pcMesh->mMaterialIndex) {
                    pcMesh->mMaterialIndex = iIndex;
                }
            }
            // collapse the rest of the array
            delete pScene->mMaterials[i];
            for (unsigned int pp = i; pp < pScene->mNumMaterials-1;++pp)    {

                pScene->mMaterials[pp] = pScene->mMaterials[pp+1];
                for (unsigned int a = 0; a < pScene->mNumMeshes;++a)    {
                    aiMesh* const pcMesh = pScene->mMeshes[a];
                    if (pcMesh->mMaterialIndex > i)--pcMesh->mMaterialIndex;
                }
            }
            --pScene->mNumMaterials;
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Read bone transformation keys
void MDLImporter::ParseBoneTrafoKeys_3DGS_MDL7(
    const MDL::IntGroupInfo_MDL7& groupInfo,
    IntFrameInfo_MDL7&            frame,
    MDL::IntSharedData_MDL7&      shared)
{
    const MDL::Header_MDL7* const pcHeader = (const MDL::Header_MDL7*)this->mBuffer;

    // only the first group contains bone animation keys
    if (frame.pcFrame->transmatrix_count)   {
        if (!groupInfo.iIndex)  {
            // skip all frames vertices. We can't support them
            const MDL::BoneTransform_MDL7* pcBoneTransforms = (const MDL::BoneTransform_MDL7*)
                (((const char*)frame.pcFrame) + pcHeader->frame_stc_size +
                frame.pcFrame->vertices_count * pcHeader->framevertex_stc_size);

            // read all transformation matrices
            for (unsigned int iTrafo = 0; iTrafo < frame.pcFrame->transmatrix_count;++iTrafo)   {
                if(pcBoneTransforms->bone_index >= pcHeader->bones_num) {
                    ASSIMP_LOG_WARN("Index overflow in frame area. "
                        "Unable to parse this bone transformation");
                }
                else    {
                    AddAnimationBoneTrafoKey_3DGS_MDL7(frame.iIndex,
                        pcBoneTransforms,shared.apcOutBones);
                }
                pcBoneTransforms = (const MDL::BoneTransform_MDL7*)(
                    (const char*)pcBoneTransforms + pcHeader->bonetrans_stc_size);
            }
        }
        else    {
            ASSIMP_LOG_WARN("Ignoring animation keyframes in groups != 0");
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Attach bones to the output nodegraph
void MDLImporter::AddBonesToNodeGraph_3DGS_MDL7(const MDL::IntBone_MDL7** apcBones,
    aiNode* pcParent,uint16_t iParentIndex)
{
    ai_assert(NULL != apcBones && NULL != pcParent);

    // get a pointer to the header ...
    const MDL::Header_MDL7* const pcHeader = (const MDL::Header_MDL7*)this->mBuffer;

    const MDL::IntBone_MDL7** apcBones2 = apcBones;
    for (uint32_t i = 0; i <  pcHeader->bones_num;++i)  {

        const MDL::IntBone_MDL7* const pcBone = *apcBones2++;
        if (pcBone->iParent == iParentIndex) {
            ++pcParent->mNumChildren;
        }
    }
    pcParent->mChildren = new aiNode*[pcParent->mNumChildren];
    unsigned int qq = 0;
    for (uint32_t i = 0; i <  pcHeader->bones_num;++i)  {

        const MDL::IntBone_MDL7* const pcBone = *apcBones++;
        if (pcBone->iParent != iParentIndex)continue;

        aiNode* pcNode = pcParent->mChildren[qq++] = new aiNode();
        pcNode->mName = aiString( pcBone->mName );

        AddBonesToNodeGraph_3DGS_MDL7(apcBones,pcNode,(uint16_t)i);
    }
}

// ------------------------------------------------------------------------------------------------
// Build output animations
void MDLImporter::BuildOutputAnims_3DGS_MDL7(
    const MDL::IntBone_MDL7** apcBonesOut)
{
    ai_assert(NULL != apcBonesOut);
    const MDL::Header_MDL7* const pcHeader = (const MDL::Header_MDL7*)mBuffer;

    // one animation ...
    aiAnimation* pcAnim = new aiAnimation();
    for (uint32_t i = 0; i < pcHeader->bones_num;++i)   {
        if (!apcBonesOut[i]->pkeyPositions.empty()) {

            // get the last frame ... (needn't be equal to pcHeader->frames_num)
            for (size_t qq = 0; qq < apcBonesOut[i]->pkeyPositions.size();++qq) {
                pcAnim->mDuration = std::max(pcAnim->mDuration, (double)
                    apcBonesOut[i]->pkeyPositions[qq].mTime);
            }
            ++pcAnim->mNumChannels;
        }
    }
    if (pcAnim->mDuration)  {
        pcAnim->mChannels = new aiNodeAnim*[pcAnim->mNumChannels];

        unsigned int iCnt = 0;
        for (uint32_t i = 0; i < pcHeader->bones_num;++i)   {
            if (!apcBonesOut[i]->pkeyPositions.empty()) {
                const MDL::IntBone_MDL7* const intBone = apcBonesOut[i];

                aiNodeAnim* const pcNodeAnim = pcAnim->mChannels[iCnt++] = new aiNodeAnim();
                pcNodeAnim->mNodeName = aiString( intBone->mName );

                // allocate enough storage for all keys
                pcNodeAnim->mNumPositionKeys = (unsigned int)intBone->pkeyPositions.size();
                pcNodeAnim->mNumScalingKeys  = (unsigned int)intBone->pkeyPositions.size();
                pcNodeAnim->mNumRotationKeys = (unsigned int)intBone->pkeyPositions.size();

                pcNodeAnim->mPositionKeys = new aiVectorKey[pcNodeAnim->mNumPositionKeys];
                pcNodeAnim->mScalingKeys  = new aiVectorKey[pcNodeAnim->mNumPositionKeys];
                pcNodeAnim->mRotationKeys = new aiQuatKey[pcNodeAnim->mNumPositionKeys];

                // copy all keys
                for (unsigned int qq = 0; qq < pcNodeAnim->mNumPositionKeys;++qq)   {
                    pcNodeAnim->mPositionKeys[qq] = intBone->pkeyPositions[qq];
                    pcNodeAnim->mScalingKeys[qq] = intBone->pkeyScalings[qq];
                    pcNodeAnim->mRotationKeys[qq] = intBone->pkeyRotations[qq];
                }
            }
        }

        // store the output animation
        pScene->mNumAnimations = 1;
        pScene->mAnimations = new aiAnimation*[1];
        pScene->mAnimations[0] = pcAnim;
    }
    else delete pcAnim;
}

// ------------------------------------------------------------------------------------------------
void MDLImporter::AddAnimationBoneTrafoKey_3DGS_MDL7(unsigned int iTrafo,
    const MDL::BoneTransform_MDL7* pcBoneTransforms,
    MDL::IntBone_MDL7** apcBonesOut)
{
    ai_assert(NULL != pcBoneTransforms);
    ai_assert(NULL != apcBonesOut);

    // first .. get the transformation matrix
    aiMatrix4x4 mTransform;
    mTransform.a1 = pcBoneTransforms->m[0];
    mTransform.b1 = pcBoneTransforms->m[1];
    mTransform.c1 = pcBoneTransforms->m[2];
    mTransform.d1 = pcBoneTransforms->m[3];

    mTransform.a2 = pcBoneTransforms->m[4];
    mTransform.b2 = pcBoneTransforms->m[5];
    mTransform.c2 = pcBoneTransforms->m[6];
    mTransform.d2 = pcBoneTransforms->m[7];

    mTransform.a3 = pcBoneTransforms->m[8];
    mTransform.b3 = pcBoneTransforms->m[9];
    mTransform.c3 = pcBoneTransforms->m[10];
    mTransform.d3 = pcBoneTransforms->m[11];

    // now decompose the transformation matrix into separate
    // scaling, rotation and translation
    aiVectorKey vScaling,vPosition;
    aiQuatKey qRotation;

    // FIXME: Decompose will assert in debug builds if the matrix is invalid ...
    mTransform.Decompose(vScaling.mValue,qRotation.mValue,vPosition.mValue);

    // now generate keys
    vScaling.mTime = qRotation.mTime = vPosition.mTime = (double)iTrafo;

    // add the keys to the bone
    MDL::IntBone_MDL7* const pcBoneOut = apcBonesOut[pcBoneTransforms->bone_index];
    pcBoneOut->pkeyPositions.push_back  ( vPosition );
    pcBoneOut->pkeyScalings.push_back   ( vScaling  );
    pcBoneOut->pkeyRotations.push_back  ( qRotation );
}

// ------------------------------------------------------------------------------------------------
// Construct output meshes
void MDLImporter::GenerateOutputMeshes_3DGS_MDL7(
    MDL::IntGroupData_MDL7& groupData,
    MDL::IntSplitGroupData_MDL7& splitGroupData)
{
    const MDL::IntSharedData_MDL7& shared = splitGroupData.shared;

    // get a pointer to the header ...
    const MDL::Header_MDL7* const pcHeader = (const MDL::Header_MDL7*)this->mBuffer;
    const unsigned int iNumOutBones = pcHeader->bones_num;

    for (std::vector<aiMaterial*>::size_type i = 0; i < shared.pcMats.size();++i)   {
        if (!splitGroupData.aiSplit[i]->empty())    {

            // allocate the output mesh
            aiMesh* pcMesh = new aiMesh();

            pcMesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
            pcMesh->mMaterialIndex = (unsigned int)i;

            // allocate output storage
            pcMesh->mNumFaces = (unsigned int)splitGroupData.aiSplit[i]->size();
            pcMesh->mFaces = new aiFace[pcMesh->mNumFaces];

            pcMesh->mNumVertices = pcMesh->mNumFaces*3;
            pcMesh->mVertices = new aiVector3D[pcMesh->mNumVertices];
            pcMesh->mNormals = new aiVector3D[pcMesh->mNumVertices];

            if (!groupData.vTextureCoords1.empty()) {
                pcMesh->mNumUVComponents[0] = 2;
                pcMesh->mTextureCoords[0] = new aiVector3D[pcMesh->mNumVertices];
                if (!groupData.vTextureCoords2.empty()) {
                    pcMesh->mNumUVComponents[1] = 2;
                    pcMesh->mTextureCoords[1] = new aiVector3D[pcMesh->mNumVertices];
                }
            }

            // iterate through all faces and build an unique set of vertices
            unsigned int iCurrent = 0;
            for (unsigned int iFace = 0; iFace < pcMesh->mNumFaces;++iFace) {
                pcMesh->mFaces[iFace].mNumIndices = 3;
                pcMesh->mFaces[iFace].mIndices = new unsigned int[3];

                unsigned int iSrcFace = splitGroupData.aiSplit[i]->operator[](iFace);
                const MDL::IntFace_MDL7& oldFace = groupData.pcFaces[iSrcFace];

                // iterate through all face indices
                for (unsigned int c = 0; c < 3;++c) {
                    const uint32_t iIndex = oldFace.mIndices[c];
                    pcMesh->mVertices[iCurrent] = groupData.vPositions[iIndex];
                    pcMesh->mNormals[iCurrent] = groupData.vNormals[iIndex];

                    if (!groupData.vTextureCoords1.empty()) {

                        pcMesh->mTextureCoords[0][iCurrent] = groupData.vTextureCoords1[iIndex];
                        if (!groupData.vTextureCoords2.empty()) {
                            pcMesh->mTextureCoords[1][iCurrent] = groupData.vTextureCoords2[iIndex];
                        }
                    }
                    pcMesh->mFaces[iFace].mIndices[c] = iCurrent++;
                }
            }

            // if we have bones in the mesh we'll need to generate
            // proper vertex weights for them
            if (!groupData.aiBones.empty()) {
                std::vector<std::vector<unsigned int> > aaiVWeightList;
                aaiVWeightList.resize(iNumOutBones);

                int iCurrent = 0;
                for (unsigned int iFace = 0; iFace < pcMesh->mNumFaces;++iFace) {
                    unsigned int iSrcFace = splitGroupData.aiSplit[i]->operator[](iFace);
                    const MDL::IntFace_MDL7& oldFace = groupData.pcFaces[iSrcFace];

                    // iterate through all face indices
                    for (unsigned int c = 0; c < 3;++c) {
                        unsigned int iBone = groupData.aiBones[ oldFace.mIndices[c] ];
                        if (UINT_MAX != iBone)  {
                            if (iBone >= iNumOutBones)  {
                                ASSIMP_LOG_ERROR("Bone index overflow. "
                                    "The bone index of a vertex exceeds the allowed range. ");
                                iBone = iNumOutBones-1;
                            }
                            aaiVWeightList[ iBone ].push_back ( iCurrent );
                        }
                        ++iCurrent;
                    }
                }
                // now check which bones are required ...
                for (std::vector<std::vector<unsigned int> >::const_iterator k =  aaiVWeightList.begin();k != aaiVWeightList.end();++k) {
                    if (!(*k).empty()) {
                        ++pcMesh->mNumBones;
                    }
                }
                pcMesh->mBones = new aiBone*[pcMesh->mNumBones];
                iCurrent = 0;
                for (std::vector<std::vector<unsigned int> >::const_iterator k = aaiVWeightList.begin();k!= aaiVWeightList.end();++k,++iCurrent)
                {
                    if ((*k).empty())
                        continue;

                    // seems we'll need this node
                    aiBone* pcBone = pcMesh->mBones[ iCurrent ] = new aiBone();
                    pcBone->mName = aiString(shared.apcOutBones[ iCurrent ]->mName);
                    pcBone->mOffsetMatrix = shared.apcOutBones[ iCurrent ]->mOffsetMatrix;

                    // setup vertex weights
                    pcBone->mNumWeights = (unsigned int)(*k).size();
                    pcBone->mWeights = new aiVertexWeight[pcBone->mNumWeights];

                    for (unsigned int weight = 0; weight < pcBone->mNumWeights;++weight)    {
                        pcBone->mWeights[weight].mVertexId = (*k)[weight];
                        pcBone->mWeights[weight].mWeight = 1.0f;
                    }
                }
            }
            // add the mesh to the list of output meshes
            splitGroupData.avOutList.push_back(pcMesh);
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Join to materials
void MDLImporter::JoinSkins_3DGS_MDL7(
    aiMaterial* pcMat1,
    aiMaterial* pcMat2,
    aiMaterial* pcMatOut)
{
    ai_assert(NULL != pcMat1 && NULL != pcMat2 && NULL != pcMatOut);

    // first create a full copy of the first skin property set
    // and assign it to the output material
    aiMaterial::CopyPropertyList(pcMatOut,pcMat1);

    int iVal = 0;
    pcMatOut->AddProperty<int>(&iVal,1,AI_MATKEY_UVWSRC_DIFFUSE(0));

    // then extract the diffuse texture from the second skin,
    // setup 1 as UV source and we have it
    aiString sString;
    if(AI_SUCCESS == aiGetMaterialString ( pcMat2, AI_MATKEY_TEXTURE_DIFFUSE(0),&sString )) {
        iVal = 1;
        pcMatOut->AddProperty<int>(&iVal,1,AI_MATKEY_UVWSRC_DIFFUSE(1));
        pcMatOut->AddProperty(&sString,AI_MATKEY_TEXTURE_DIFFUSE(1));
    }
}

// ------------------------------------------------------------------------------------------------
// Read a half-life 2 MDL
void MDLImporter::InternReadFile_HL2( )
{
    //const MDL::Header_HL2* pcHeader = (const MDL::Header_HL2*)this->mBuffer;
    throw DeadlyImportError("HL2 MDLs are not implemented");
}

#endif // !! ASSIMP_BUILD_NO_MDL_IMPORTER
