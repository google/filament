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

/** @file Implementation of the LWO importer class for the older LWOB
    file formats, including materials */


#ifndef ASSIMP_BUILD_NO_LWO_IMPORTER

// Internal headers
#include "LWOLoader.h"
using namespace Assimp;


// ------------------------------------------------------------------------------------------------
void LWOImporter::LoadLWOBFile()
{
    LE_NCONST uint8_t* const end = mFileBuffer + fileSize;
    bool running = true;
    while (running)
    {
        if (mFileBuffer + sizeof(IFF::ChunkHeader) > end)break;
        const IFF::ChunkHeader head = IFF::LoadChunk(mFileBuffer);

        if (mFileBuffer + head.length > end)
        {
            throw DeadlyImportError("LWOB: Invalid chunk length");
            break;
        }
        uint8_t* const next = mFileBuffer+head.length;
        switch (head.type)
        {
            // vertex list
        case AI_LWO_PNTS:
            {
                if (!mCurLayer->mTempPoints.empty())
                    ASSIMP_LOG_WARN("LWO: PNTS chunk encountered twice");
                else LoadLWOPoints(head.length);
                break;
            }
            // face list
        case AI_LWO_POLS:
            {

                if (!mCurLayer->mFaces.empty())
                    ASSIMP_LOG_WARN("LWO: POLS chunk encountered twice");
                else LoadLWOBPolygons(head.length);
                break;
            }
            // list of tags
        case AI_LWO_SRFS:
            {
                if (!mTags->empty())
                    ASSIMP_LOG_WARN("LWO: SRFS chunk encountered twice");
                else LoadLWOTags(head.length);
                break;
            }

            // surface chunk
        case AI_LWO_SURF:
            {
                LoadLWOBSurface(head.length);
                break;
            }
        }
        mFileBuffer = next;
    }
}

// ------------------------------------------------------------------------------------------------
void LWOImporter::LoadLWOBPolygons(unsigned int length)
{
    // first find out how many faces and vertices we'll finally need
    LE_NCONST uint16_t* const end   = (LE_NCONST uint16_t*)(mFileBuffer+length);
    LE_NCONST uint16_t* cursor      = (LE_NCONST uint16_t*)mFileBuffer;

    // perform endianness conversions
#ifndef AI_BUILD_BIG_ENDIAN
    while (cursor < end)ByteSwap::Swap2(cursor++);
    cursor = (LE_NCONST uint16_t*)mFileBuffer;
#endif

    unsigned int iNumFaces = 0,iNumVertices = 0;
    CountVertsAndFacesLWOB(iNumVertices,iNumFaces,cursor,end);

    // allocate the output array and copy face indices
    if (iNumFaces)
    {
        cursor = (LE_NCONST uint16_t*)mFileBuffer;

        mCurLayer->mFaces.resize(iNumFaces);
        FaceList::iterator it = mCurLayer->mFaces.begin();
        CopyFaceIndicesLWOB(it,cursor,end);
    }
}

// ------------------------------------------------------------------------------------------------
void LWOImporter::CountVertsAndFacesLWOB(unsigned int& verts, unsigned int& faces,
    LE_NCONST uint16_t*& cursor, const uint16_t* const end, unsigned int max)
{
    while (cursor < end && max--)
    {
        uint16_t numIndices;
        // must have 2 shorts left for numIndices and surface
        if (end - cursor < 2) {
            throw DeadlyImportError("LWOB: Unexpected end of file");
        }
        ::memcpy(&numIndices, cursor++, 2);
        // must have enough left for indices and surface
        if (end - cursor < (1 + numIndices)) {
            throw DeadlyImportError("LWOB: Unexpected end of file");
        }
        verts += numIndices;
        faces++;
        cursor += numIndices;
        int16_t surface;
        ::memcpy(&surface, cursor++, 2);
        if (surface < 0)
        {
            // there are detail polygons
            ::memcpy(&numIndices, cursor++, 2);
            CountVertsAndFacesLWOB(verts,faces,cursor,end,numIndices);
        }
    }
}

// ------------------------------------------------------------------------------------------------
void LWOImporter::CopyFaceIndicesLWOB(FaceList::iterator& it,
    LE_NCONST uint16_t*& cursor,
    const uint16_t* const end,
    unsigned int max)
{
    while (cursor < end && max--)
    {
        LWO::Face& face = *it;++it;
        uint16_t numIndices;
        ::memcpy(&numIndices, cursor++, 2);
        face.mNumIndices = numIndices;
        if(face.mNumIndices)
        {
            if (cursor + face.mNumIndices >= end)
            {
                break;
            }
            face.mIndices = new unsigned int[face.mNumIndices];
            for (unsigned int i = 0; i < face.mNumIndices;++i) {
                unsigned int & mi = face.mIndices[i];
                uint16_t index;
                ::memcpy(&index, cursor++, 2);
                mi = index;
                if (mi > mCurLayer->mTempPoints.size())
                {
                    ASSIMP_LOG_WARN("LWOB: face index is out of range");
                    mi = (unsigned int)mCurLayer->mTempPoints.size()-1;
                }
            }
        } else {
            ASSIMP_LOG_WARN("LWOB: Face has 0 indices");
        }
        int16_t surface;
        ::memcpy(&surface, cursor++, 2);
        if (surface < 0)
        {
            surface = -surface;

            // there are detail polygons.
            uint16_t numPolygons;
            ::memcpy(&numPolygons, cursor++, 2);
            if (cursor < end)
            {
                CopyFaceIndicesLWOB(it,cursor,end,numPolygons);
            }
        }
        face.surfaceIndex = surface-1;
    }
}

// ------------------------------------------------------------------------------------------------
LWO::Texture* LWOImporter::SetupNewTextureLWOB(LWO::TextureList& list,unsigned int size)
{
    list.push_back(LWO::Texture());
    LWO::Texture* tex = &list.back();

    std::string type;
    GetS0(type,size);
    const char* s = type.c_str();

    if(strstr(s, "Image Map"))
    {
        // Determine mapping type
        if(strstr(s, "Planar"))
            tex->mapMode = LWO::Texture::Planar;
        else if(strstr(s, "Cylindrical"))
            tex->mapMode = LWO::Texture::Cylindrical;
        else if(strstr(s, "Spherical"))
            tex->mapMode = LWO::Texture::Spherical;
        else if(strstr(s, "Cubic"))
            tex->mapMode = LWO::Texture::Cubic;
        else if(strstr(s, "Front"))
            tex->mapMode = LWO::Texture::FrontProjection;
    }
    else
    {
        // procedural or gradient, not supported
        ASSIMP_LOG_ERROR_F("LWOB: Unsupported legacy texture: ", type);
    }

    return tex;
}

// ------------------------------------------------------------------------------------------------
void LWOImporter::LoadLWOBSurface(unsigned int size)
{
    LE_NCONST uint8_t* const end = mFileBuffer + size;

    mSurfaces->push_back( LWO::Surface () );
    LWO::Surface& surf = mSurfaces->back();
    LWO::Texture* pTex = NULL;

    GetS0(surf.mName,size);
    bool running = true;
    while (running)    {
        if (mFileBuffer + 6 >= end)
            break;

        IFF::SubChunkHeader head = IFF::LoadSubChunk(mFileBuffer);

        /*  A single test file (sonycam.lwo) seems to have invalid surface chunks.
         *  I'm assuming it's the fault of a single, unknown exporter so there are
         *  probably THOUSANDS of them. Here's a dirty workaround:
         *
         *  We don't break if the chunk limit is exceeded. Instead, we're computing
         *  how much storage is actually left and work with this value from now on.
         */
        if (mFileBuffer + head.length > end) {
            ASSIMP_LOG_ERROR("LWOB: Invalid surface chunk length. Trying to continue.");
            head.length = (uint16_t) (end - mFileBuffer);
        }

        uint8_t* const next = mFileBuffer+head.length;
        switch (head.type)
        {
        // diffuse color
        case AI_LWO_COLR:
            {
                AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,COLR,3);
                surf.mColor.r = GetU1() / 255.0f;
                surf.mColor.g = GetU1() / 255.0f;
                surf.mColor.b = GetU1() / 255.0f;
                break;
            }
        // diffuse strength ...
        case AI_LWO_DIFF:
            {
                AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,DIFF,2);
                surf.mDiffuseValue = GetU2() / 255.0f;
                break;
            }
        // specular strength ...
        case AI_LWO_SPEC:
            {
                AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,SPEC,2);
                surf.mSpecularValue = GetU2() / 255.0f;
                break;
            }
        // luminosity ...
        case AI_LWO_LUMI:
            {
                AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,LUMI,2);
                surf.mLuminosity = GetU2() / 255.0f;
                break;
            }
        // transparency
        case AI_LWO_TRAN:
            {
                AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,TRAN,2);
                surf.mTransparency = GetU2() / 255.0f;
                break;
            }
        // surface flags
        case AI_LWO_FLAG:
            {
                AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,FLAG,2);
                uint16_t flag = GetU2();
                if (flag & 0x4 )   surf.mMaximumSmoothAngle = 1.56207f;
                if (flag & 0x8 )   surf.mColorHighlights = 1.f;
                if (flag & 0x100)  surf.bDoubleSided = true;
                break;
            }
        // maximum smoothing angle
        case AI_LWO_SMAN:
            {
                AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,SMAN,4);
                surf.mMaximumSmoothAngle = std::fabs( GetF4() );
                break;
            }
        // glossiness
        case AI_LWO_GLOS:
            {
                AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,GLOS,2);
                surf.mGlossiness = (float)GetU2();
                break;
            }
        // color texture
        case AI_LWO_CTEX:
            {
                pTex = SetupNewTextureLWOB(surf.mColorTextures,
                    head.length);
                break;
            }
        // diffuse texture
        case AI_LWO_DTEX:
            {
                pTex = SetupNewTextureLWOB(surf.mDiffuseTextures,
                    head.length);
                break;
            }
        // specular texture
        case AI_LWO_STEX:
            {
                pTex = SetupNewTextureLWOB(surf.mSpecularTextures,
                    head.length);
                break;
            }
        // bump texture
        case AI_LWO_BTEX:
            {
                pTex = SetupNewTextureLWOB(surf.mBumpTextures,
                    head.length);
                break;
            }
        // transparency texture
        case AI_LWO_TTEX:
            {
                pTex = SetupNewTextureLWOB(surf.mOpacityTextures,
                    head.length);
                break;
            }
        // texture path
        case AI_LWO_TIMG:
            {
                if (pTex)   {
                    GetS0(pTex->mFileName,head.length);
                } else {
                    ASSIMP_LOG_WARN("LWOB: Unexpected TIMG chunk");
                }
                break;
            }
        // texture strength
        case AI_LWO_TVAL:
            {
                AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,TVAL,1);
                if (pTex)   {
                    pTex->mStrength = (float)GetU1()/ 255.f;
                } else {
                    ASSIMP_LOG_ERROR("LWOB: Unexpected TVAL chunk");
                }
                break;
            }
        // texture flags
        case AI_LWO_TFLG:
            {
                AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,TFLG,2);

                if (nullptr != pTex) {
                    const uint16_t s = GetU2();
                    if (s & 1)
                        pTex->majorAxis = LWO::Texture::AXIS_X;
                    else if (s & 2)
                        pTex->majorAxis = LWO::Texture::AXIS_Y;
                    else if (s & 4)
                        pTex->majorAxis = LWO::Texture::AXIS_Z;

                    if (s & 16) {
                        ASSIMP_LOG_WARN("LWOB: Ignoring \'negate\' flag on texture");
                    }
                }
                else {
                    ASSIMP_LOG_WARN("LWOB: Unexpected TFLG chunk");
                }
                break;
            }
        }
        mFileBuffer = next;
    }
}

#endif // !! ASSIMP_BUILD_NO_LWO_IMPORTER
