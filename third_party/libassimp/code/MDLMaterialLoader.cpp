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

/** @file Implementation of the material part of the MDL importer class */


#ifndef ASSIMP_BUILD_NO_MDL_IMPORTER

// internal headers
#include "MDLLoader.h"
#include "MDLDefaultColorMap.h"
#include <assimp/StringUtils.h>
#include <assimp/texture.h>
#include <assimp/IOSystem.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/scene.h>
#include <assimp/Defines.h>
#include <assimp/qnan.h>

#include <memory>


using namespace Assimp;
static aiTexel* const bad_texel = reinterpret_cast<aiTexel*>(SIZE_MAX);

// ------------------------------------------------------------------------------------------------
// Find a suitable palette file or take the default one
void MDLImporter::SearchPalette(const unsigned char** pszColorMap)
{
    // now try to find the color map in the current directory
    IOStream* pcStream = pIOHandler->Open(configPalette,"rb");

    const unsigned char* szColorMap = (const unsigned char*)::g_aclrDefaultColorMap;
    if(pcStream)
    {
        if (pcStream->FileSize() >= 768)
        {
            size_t len = 256 * 3;
            unsigned char* colorMap = new unsigned char[len];
            szColorMap = colorMap;
            pcStream->Read(colorMap, len,1);
            ASSIMP_LOG_INFO("Found valid colormap.lmp in directory. "
                "It will be used to decode embedded textures in palletized formats.");
        }
        delete pcStream;
        pcStream = NULL;
    }
    *pszColorMap = szColorMap;
}

// ------------------------------------------------------------------------------------------------
// Free the palette again
void MDLImporter::FreePalette(const unsigned char* szColorMap)
{
    if (szColorMap != (const unsigned char*)::g_aclrDefaultColorMap)
        delete[] szColorMap;
}

// ------------------------------------------------------------------------------------------------
// Check whether we can replace a texture with a single color
aiColor4D MDLImporter::ReplaceTextureWithColor(const aiTexture* pcTexture)
{
    ai_assert(NULL != pcTexture);

    aiColor4D clrOut;
    clrOut.r = get_qnan();
    if (!pcTexture->mHeight || !pcTexture->mWidth)
        return clrOut;

    const unsigned int iNumPixels = pcTexture->mHeight*pcTexture->mWidth;
    const aiTexel* pcTexel = pcTexture->pcData+1;
    const aiTexel* const pcTexelEnd = &pcTexture->pcData[iNumPixels];

    while (pcTexel != pcTexelEnd)
    {
        if (*pcTexel != *(pcTexel-1))
        {
            pcTexel = NULL;
            break;
        }
        ++pcTexel;
    }
    if (pcTexel)
    {
        clrOut.r = pcTexture->pcData->r / 255.0f;
        clrOut.g = pcTexture->pcData->g / 255.0f;
        clrOut.b = pcTexture->pcData->b / 255.0f;
        clrOut.a = pcTexture->pcData->a / 255.0f;
    }
    return clrOut;
}

// ------------------------------------------------------------------------------------------------
// Read a texture from a MDL3 file
void MDLImporter::CreateTextureARGB8_3DGS_MDL3(const unsigned char* szData)
{
    const MDL::Header *pcHeader = (const MDL::Header*)mBuffer;  //the endianness is already corrected in the InternReadFile_3DGS_MDL345 function

    VALIDATE_FILE_SIZE(szData + pcHeader->skinwidth *
        pcHeader->skinheight);

    // allocate a new texture object
    aiTexture* pcNew = new aiTexture();
    pcNew->mWidth = pcHeader->skinwidth;
    pcNew->mHeight = pcHeader->skinheight;

    pcNew->pcData = new aiTexel[pcNew->mWidth * pcNew->mHeight];

    const unsigned char* szColorMap;
    this->SearchPalette(&szColorMap);

    // copy texture data
    for (unsigned int i = 0; i < pcNew->mWidth*pcNew->mHeight;++i)
    {
        const unsigned char val = szData[i];
        const unsigned char* sz = &szColorMap[val*3];

        pcNew->pcData[i].a = 0xFF;
        pcNew->pcData[i].r = *sz++;
        pcNew->pcData[i].g = *sz++;
        pcNew->pcData[i].b = *sz;
    }

    FreePalette(szColorMap);

    // store the texture
    aiTexture** pc = this->pScene->mTextures;
    this->pScene->mTextures = new aiTexture*[pScene->mNumTextures+1];
    for (unsigned int i = 0; i <pScene->mNumTextures;++i)
        pScene->mTextures[i] = pc[i];

    pScene->mTextures[this->pScene->mNumTextures] = pcNew;
    pScene->mNumTextures++;
    delete[] pc;
    return;
}

// ------------------------------------------------------------------------------------------------
// Read a texture from a MDL4 file
void MDLImporter::CreateTexture_3DGS_MDL4(const unsigned char* szData,
    unsigned int iType,
     unsigned int* piSkip)
{
    ai_assert(NULL != piSkip);

    const MDL::Header *pcHeader = (const MDL::Header*)mBuffer;  //the endianness is already corrected in the InternReadFile_3DGS_MDL345 function

    if (iType == 1 || iType > 3)
    {
        ASSIMP_LOG_ERROR("Unsupported texture file format");
        return;
    }

    const bool bNoRead = *piSkip == UINT_MAX;

    // allocate a new texture object
    aiTexture* pcNew = new aiTexture();
    pcNew->mWidth = pcHeader->skinwidth;
    pcNew->mHeight = pcHeader->skinheight;

    if (bNoRead)pcNew->pcData = bad_texel;
    ParseTextureColorData(szData,iType,piSkip,pcNew);

    // store the texture
    if (!bNoRead)
    {
        if (!this->pScene->mNumTextures)
        {
            pScene->mNumTextures = 1;
            pScene->mTextures = new aiTexture*[1];
            pScene->mTextures[0] = pcNew;
        }
        else
        {
            aiTexture** pc = pScene->mTextures;
            pScene->mTextures = new aiTexture*[pScene->mNumTextures+1];
            for (unsigned int i = 0; i < this->pScene->mNumTextures;++i)
                pScene->mTextures[i] = pc[i];
            pScene->mTextures[pScene->mNumTextures] = pcNew;
            pScene->mNumTextures++;
            delete[] pc;
        }
    }
    else {
        pcNew->pcData = NULL;
        delete pcNew;
    }
    return;
}

// ------------------------------------------------------------------------------------------------
// Load color data of a texture and convert it to our output format
void MDLImporter::ParseTextureColorData(const unsigned char* szData,
    unsigned int iType,
    unsigned int* piSkip,
    aiTexture* pcNew)
{
    const bool do_read = bad_texel != pcNew->pcData;

    // allocate storage for the texture image
    if (do_read) {
        pcNew->pcData = new aiTexel[pcNew->mWidth * pcNew->mHeight];
    }

    // R5G6B5 format (with or without MIPs)
    // ****************************************************************
    if (2 == iType || 10 == iType)
    {
        VALIDATE_FILE_SIZE(szData + pcNew->mWidth*pcNew->mHeight*2);

        // copy texture data
        unsigned int i;
        if (do_read)
        {
            for (i = 0; i < pcNew->mWidth*pcNew->mHeight;++i)
            {
                MDL::RGB565 val = ((MDL::RGB565*)szData)[i];
                AI_SWAP2(val);

                pcNew->pcData[i].a = 0xFF;
                pcNew->pcData[i].r = (unsigned char)val.b << 3;
                pcNew->pcData[i].g = (unsigned char)val.g << 2;
                pcNew->pcData[i].b = (unsigned char)val.r << 3;
            }
        }
        else i = pcNew->mWidth*pcNew->mHeight;
        *piSkip = i * 2;

        // apply MIP maps
        if (10 == iType)
        {
            *piSkip += ((i >> 2) + (i >> 4) + (i >> 6)) << 1;
            VALIDATE_FILE_SIZE(szData + *piSkip);
        }
    }
    // ARGB4 format (with or without MIPs)
    // ****************************************************************
    else if (3 == iType || 11 == iType)
    {
        VALIDATE_FILE_SIZE(szData + pcNew->mWidth*pcNew->mHeight*4);

        // copy texture data
        unsigned int i;
        if (do_read)
        {
            for (i = 0; i < pcNew->mWidth*pcNew->mHeight;++i)
            {
                MDL::ARGB4 val = ((MDL::ARGB4*)szData)[i];
                AI_SWAP2(val);

                pcNew->pcData[i].a = (unsigned char)val.a << 4;
                pcNew->pcData[i].r = (unsigned char)val.r << 4;
                pcNew->pcData[i].g = (unsigned char)val.g << 4;
                pcNew->pcData[i].b = (unsigned char)val.b << 4;
            }
        }
        else i = pcNew->mWidth*pcNew->mHeight;
        *piSkip = i * 2;

        // apply MIP maps
        if (11 == iType)
        {
            *piSkip += ((i >> 2) + (i >> 4) + (i >> 6)) << 1;
            VALIDATE_FILE_SIZE(szData + *piSkip);
        }
    }
    // RGB8 format (with or without MIPs)
    // ****************************************************************
    else if (4 == iType || 12 == iType)
    {
        VALIDATE_FILE_SIZE(szData + pcNew->mWidth*pcNew->mHeight*3);

        // copy texture data
        unsigned int i;
        if (do_read)
        {
            for (i = 0; i < pcNew->mWidth*pcNew->mHeight;++i)
            {
                const unsigned char* _szData = &szData[i*3];

                pcNew->pcData[i].a = 0xFF;
                pcNew->pcData[i].b = *_szData++;
                pcNew->pcData[i].g = *_szData++;
                pcNew->pcData[i].r = *_szData;
            }
        }
        else i = pcNew->mWidth*pcNew->mHeight;


        // apply MIP maps
        *piSkip = i * 3;
        if (12 == iType)
        {
            *piSkip += ((i >> 2) + (i >> 4) + (i >> 6)) *3;
            VALIDATE_FILE_SIZE(szData + *piSkip);
        }
    }
    // ARGB8 format (with ir without MIPs)
    // ****************************************************************
    else if (5 == iType || 13 == iType)
    {
        VALIDATE_FILE_SIZE(szData + pcNew->mWidth*pcNew->mHeight*4);

        // copy texture data
        unsigned int i;
        if (do_read)
        {
            for (i = 0; i < pcNew->mWidth*pcNew->mHeight;++i)
            {
                const unsigned char* _szData = &szData[i*4];

                pcNew->pcData[i].b = *_szData++;
                pcNew->pcData[i].g = *_szData++;
                pcNew->pcData[i].r = *_szData++;
                pcNew->pcData[i].a = *_szData;
            }
        }
        else i = pcNew->mWidth*pcNew->mHeight;

        // apply MIP maps
        *piSkip = i << 2;
        if (13 == iType)
        {
            *piSkip += ((i >> 2) + (i >> 4) + (i >> 6)) << 2;
        }
    }
    // palletized 8 bit texture. As for Quake 1
    // ****************************************************************
    else if (0 == iType)
    {
        VALIDATE_FILE_SIZE(szData + pcNew->mWidth*pcNew->mHeight);

        // copy texture data
        unsigned int i;
        if (do_read)
        {

            const unsigned char* szColorMap;
            SearchPalette(&szColorMap);

            for (i = 0; i < pcNew->mWidth*pcNew->mHeight;++i)
            {
                const unsigned char val = szData[i];
                const unsigned char* sz = &szColorMap[val*3];

                pcNew->pcData[i].a = 0xFF;
                pcNew->pcData[i].r = *sz++;
                pcNew->pcData[i].g = *sz++;
                pcNew->pcData[i].b = *sz;
            }
            this->FreePalette(szColorMap);

        }
        else i = pcNew->mWidth*pcNew->mHeight;
        *piSkip = i;

        // FIXME: Also support for MIP maps?
    }
}

// ------------------------------------------------------------------------------------------------
// Get a texture from a MDL5 file
void MDLImporter::CreateTexture_3DGS_MDL5(const unsigned char* szData,
    unsigned int iType,
    unsigned int* piSkip)
{
    ai_assert(NULL != piSkip);
    bool bNoRead = *piSkip == UINT_MAX;

    // allocate a new texture object
    aiTexture* pcNew = new aiTexture();

    VALIDATE_FILE_SIZE(szData+8);

    // first read the size of the texture
    pcNew->mWidth = *((uint32_t*)szData);
    AI_SWAP4(pcNew->mWidth);
    szData += sizeof(uint32_t);

    pcNew->mHeight = *((uint32_t*)szData);
    AI_SWAP4(pcNew->mHeight);
    szData += sizeof(uint32_t);

    if (bNoRead) {
        pcNew->pcData = bad_texel;
    }

    // this should not occur - at least the docs say it shouldn't.
    // however, one can easily try out what MED does if you have
    // a model with a DDS texture and export it to MDL5 ...
    // yeah, it embedds the DDS file.
    if (6 == iType)
    {
        // this is a compressed texture in DDS format
        *piSkip = pcNew->mWidth;
        VALIDATE_FILE_SIZE(szData + *piSkip);

        if (!bNoRead)
        {
            // place a hint and let the application know that this is a DDS file
            pcNew->mHeight = 0;
            pcNew->achFormatHint[0] = 'd';
            pcNew->achFormatHint[1] = 'd';
            pcNew->achFormatHint[2] = 's';
            pcNew->achFormatHint[3] = '\0';

            pcNew->pcData = (aiTexel*) new unsigned char[pcNew->mWidth];
            ::memcpy(pcNew->pcData,szData,pcNew->mWidth);
        }
    }
    else
    {
        // parse the color data of the texture
        ParseTextureColorData(szData,iType,piSkip,pcNew);
    }
    *piSkip += sizeof(uint32_t) * 2;

    if (!bNoRead)
    {
        // store the texture
        if (!this->pScene->mNumTextures)
        {
            pScene->mNumTextures = 1;
            pScene->mTextures = new aiTexture*[1];
            pScene->mTextures[0] = pcNew;
        }
        else
        {
            aiTexture** pc = pScene->mTextures;
            pScene->mTextures = new aiTexture*[pScene->mNumTextures+1];
            for (unsigned int i = 0; i < pScene->mNumTextures;++i)
                this->pScene->mTextures[i] = pc[i];

            pScene->mTextures[pScene->mNumTextures] = pcNew;
            pScene->mNumTextures++;
            delete[] pc;
        }
    }
    else {
        pcNew->pcData = NULL;
        delete pcNew;
    }
    return;
}

// ------------------------------------------------------------------------------------------------
// Get a skin from a MDL7 file - more complex than all other subformats
void MDLImporter::ParseSkinLump_3DGS_MDL7(
    const unsigned char* szCurrent,
    const unsigned char** szCurrentOut,
    aiMaterial* pcMatOut,
    unsigned int iType,
    unsigned int iWidth,
    unsigned int iHeight)
{
    std::unique_ptr<aiTexture> pcNew;

    // get the type of the skin
    unsigned int iMasked = (unsigned int)(iType & 0xF);

    if (0x1 ==  iMasked)
    {
        // ***** REFERENCE TO ANOTHER SKIN INDEX *****
        int referrer = (int)iWidth;
        pcMatOut->AddProperty<int>(&referrer,1,AI_MDL7_REFERRER_MATERIAL);
    }
    else if (0x6 == iMasked)
    {
        // ***** EMBEDDED DDS FILE *****
        if (1 != iHeight)
        {
            ASSIMP_LOG_WARN("Found a reference to an embedded DDS texture, "
                "but texture height is not equal to 1, which is not supported by MED");
        }

        pcNew.reset(new aiTexture());
        pcNew->mHeight = 0;
        pcNew->mWidth = iWidth;

        // place a proper format hint
        pcNew->achFormatHint[0] = 'd';
        pcNew->achFormatHint[1] = 'd';
        pcNew->achFormatHint[2] = 's';
        pcNew->achFormatHint[3] = '\0';

        pcNew->pcData = (aiTexel*) new unsigned char[pcNew->mWidth];
        memcpy(pcNew->pcData,szCurrent,pcNew->mWidth);
        szCurrent += iWidth;
    }
    else if (0x7 == iMasked)
    {
        // ***** REFERENCE TO EXTERNAL FILE *****
        if (1 != iHeight)
        {
            ASSIMP_LOG_WARN("Found a reference to an external texture, "
                "but texture height is not equal to 1, which is not supported by MED");
        }

        aiString szFile;
        const size_t iLen = strlen((const char*)szCurrent);
        size_t iLen2 = iLen+1;
        iLen2 = iLen2 > MAXLEN ? MAXLEN : iLen2;
        memcpy(szFile.data,(const char*)szCurrent,iLen2);
        szFile.length = iLen;

        szCurrent += iLen2;

        // place this as diffuse texture
        pcMatOut->AddProperty(&szFile,AI_MATKEY_TEXTURE_DIFFUSE(0));
    }
    else if (iMasked || !iType || (iType && iWidth && iHeight))
    {
        pcNew.reset(new aiTexture());
        if (!iHeight || !iWidth)
        {
            ASSIMP_LOG_WARN("Found embedded texture, but its width "
                "an height are both 0. Is this a joke?");

            // generate an empty chess pattern
            pcNew->mWidth = pcNew->mHeight = 8;
            pcNew->pcData = new aiTexel[64];
            for (unsigned int x = 0; x < 8;++x)
            {
                for (unsigned int y = 0; y < 8;++y)
                {
                    const bool bSet = ((0 == x % 2 && 0 != y % 2) ||
                        (0 != x % 2 && 0 == y % 2));

                    aiTexel* pc = &pcNew->pcData[y * 8 + x];
                    pc->r = pc->b = pc->g = (bSet?0xFF:0);
                    pc->a = 0xFF;
                }
            }
        }
        else
        {
            // it is a standard color texture. Fill in width and height
            // and call the same function we used for loading MDL5 files

            pcNew->mWidth = iWidth;
            pcNew->mHeight = iHeight;

            unsigned int iSkip = 0;
            ParseTextureColorData(szCurrent,iMasked,&iSkip,pcNew.get());

            // skip length of texture data
            szCurrent += iSkip;
        }
    }

    // sometimes there are MDL7 files which have a monochrome
    // texture instead of material colors ... posssible they have
    // been converted to MDL7 from other formats, such as MDL5
    aiColor4D clrTexture;
    if (pcNew)clrTexture = ReplaceTextureWithColor(pcNew.get());
    else clrTexture.r = get_qnan();

    // check whether a material definition is contained in the skin
    if (iType & AI_MDL7_SKINTYPE_MATERIAL)
    {
        BE_NCONST MDL::Material_MDL7* pcMatIn = (BE_NCONST MDL::Material_MDL7*)szCurrent;
        szCurrent = (unsigned char*)(pcMatIn+1);
        VALIDATE_FILE_SIZE(szCurrent);

        aiColor3D clrTemp;

#define COLOR_MULTIPLY_RGB() \
    if (is_not_qnan(clrTexture.r)) \
        { \
        clrTemp.r *= clrTexture.r; \
        clrTemp.g *= clrTexture.g; \
        clrTemp.b *= clrTexture.b; \
        }

        // read diffuse color
        clrTemp.r = pcMatIn->Diffuse.r;
        AI_SWAP4(clrTemp.r);
        clrTemp.g = pcMatIn->Diffuse.g;
        AI_SWAP4(clrTemp.g);
        clrTemp.b = pcMatIn->Diffuse.b;
        AI_SWAP4(clrTemp.b);
        COLOR_MULTIPLY_RGB();
        pcMatOut->AddProperty<aiColor3D>(&clrTemp,1,AI_MATKEY_COLOR_DIFFUSE);

        // read specular color
        clrTemp.r = pcMatIn->Specular.r;
        AI_SWAP4(clrTemp.r);
        clrTemp.g = pcMatIn->Specular.g;
        AI_SWAP4(clrTemp.g);
        clrTemp.b = pcMatIn->Specular.b;
        AI_SWAP4(clrTemp.b);
        COLOR_MULTIPLY_RGB();
        pcMatOut->AddProperty<aiColor3D>(&clrTemp,1,AI_MATKEY_COLOR_SPECULAR);

        // read ambient color
        clrTemp.r = pcMatIn->Ambient.r;
        AI_SWAP4(clrTemp.r);
        clrTemp.g = pcMatIn->Ambient.g;
        AI_SWAP4(clrTemp.g);
        clrTemp.b = pcMatIn->Ambient.b;
        AI_SWAP4(clrTemp.b);
        COLOR_MULTIPLY_RGB();
        pcMatOut->AddProperty<aiColor3D>(&clrTemp,1,AI_MATKEY_COLOR_AMBIENT);

        // read emissive color
        clrTemp.r = pcMatIn->Emissive.r;
        AI_SWAP4(clrTemp.r);
        clrTemp.g = pcMatIn->Emissive.g;
        AI_SWAP4(clrTemp.g);
        clrTemp.b = pcMatIn->Emissive.b;
        AI_SWAP4(clrTemp.b);
        pcMatOut->AddProperty<aiColor3D>(&clrTemp,1,AI_MATKEY_COLOR_EMISSIVE);

#undef COLOR_MULITPLY_RGB

        // FIX: Take the opacity from the ambient color.
        // The doc say something else, but it is fact that MED exports the
        // opacity like this .... oh well.
        clrTemp.r = pcMatIn->Ambient.a;
        AI_SWAP4(clrTemp.r);
        if (is_not_qnan(clrTexture.r)) {
            clrTemp.r *= clrTexture.a;
        }
        pcMatOut->AddProperty<ai_real>(&clrTemp.r,1,AI_MATKEY_OPACITY);

        // read phong power
        int iShadingMode = (int)aiShadingMode_Gouraud;
        AI_SWAP4(pcMatIn->Power);
        if (0.0f != pcMatIn->Power)
        {
            iShadingMode = (int)aiShadingMode_Phong;
            // pcMatIn is packed, we can't form pointers to its members
            float power = pcMatIn->Power;
            pcMatOut->AddProperty<float>(&power,1,AI_MATKEY_SHININESS);
        }
        pcMatOut->AddProperty<int>(&iShadingMode,1,AI_MATKEY_SHADING_MODEL);
    }
    else if (is_not_qnan(clrTexture.r))
    {
        pcMatOut->AddProperty<aiColor4D>(&clrTexture,1,AI_MATKEY_COLOR_DIFFUSE);
        pcMatOut->AddProperty<aiColor4D>(&clrTexture,1,AI_MATKEY_COLOR_SPECULAR);
    }
    // if the texture could be replaced by a single material color
    // we don't need the texture anymore
    if (is_not_qnan(clrTexture.r))
    {
        pcNew.reset();
    }

    // If an ASCII effect description (HLSL?) is contained in the file,
    // we can simply ignore it ...
    if (iType & AI_MDL7_SKINTYPE_MATERIAL_ASCDEF)
    {
        VALIDATE_FILE_SIZE(szCurrent);
        int32_t iMe = *((int32_t*)szCurrent);
        AI_SWAP4(iMe);
        szCurrent += sizeof(char) * iMe + sizeof(int32_t);
        VALIDATE_FILE_SIZE(szCurrent);
    }

    // If an embedded texture has been loaded setup the corresponding
    // data structures in the aiScene instance
    if (pcNew && pScene->mNumTextures <= 999)
    {

        // place this as diffuse texture
        char szCurrent[5];
        ai_snprintf(szCurrent,5,"*%i",this->pScene->mNumTextures);

        aiString szFile;
        const size_t iLen = strlen((const char*)szCurrent);
        ::memcpy(szFile.data,(const char*)szCurrent,iLen+1);
        szFile.length = iLen;

        pcMatOut->AddProperty(&szFile,AI_MATKEY_TEXTURE_DIFFUSE(0));

        // store the texture
        if (!pScene->mNumTextures)
        {
            pScene->mNumTextures = 1;
            pScene->mTextures = new aiTexture*[1];
            pScene->mTextures[0] = pcNew.release();
        }
        else
        {
            aiTexture** pc = pScene->mTextures;
            pScene->mTextures = new aiTexture*[pScene->mNumTextures+1];
            for (unsigned int i = 0; i < pScene->mNumTextures;++i) {
                pScene->mTextures[i] = pc[i];
            }

            pScene->mTextures[pScene->mNumTextures] = pcNew.release();
            pScene->mNumTextures++;
            delete[] pc;
        }
    }
    VALIDATE_FILE_SIZE(szCurrent);
    *szCurrentOut = szCurrent;
}

// ------------------------------------------------------------------------------------------------
// Skip a skin lump
void MDLImporter::SkipSkinLump_3DGS_MDL7(
    const unsigned char* szCurrent,
    const unsigned char** szCurrentOut,
    unsigned int iType,
    unsigned int iWidth,
    unsigned int iHeight)
{
    // get the type of the skin
    const unsigned int iMasked = (unsigned int)(iType & 0xF);

    if (0x6 == iMasked)
    {
        szCurrent += iWidth;
    }
    if (0x7 == iMasked)
    {
        const size_t iLen = ::strlen((const char*)szCurrent);
        szCurrent += iLen+1;
    }
    else if (iMasked || !iType)
    {
        if (iMasked || !iType || (iType && iWidth && iHeight))
        {
            // ParseTextureColorData(..., aiTexture::pcData == bad_texel) will simply
            // return the size of the color data in bytes in iSkip
            unsigned int iSkip = 0;

            aiTexture tex;
            tex.pcData = bad_texel;
            tex.mHeight = iHeight;
            tex.mWidth = iWidth;
            ParseTextureColorData(szCurrent,iMasked,&iSkip,&tex);

            // FIX: Important, otherwise the destructor will crash
            tex.pcData = NULL;

            // skip length of texture data
            szCurrent += iSkip;
        }
    }

    // check whether a material definition is contained in the skin
    if (iType & AI_MDL7_SKINTYPE_MATERIAL)
    {
        BE_NCONST MDL::Material_MDL7* pcMatIn = (BE_NCONST MDL::Material_MDL7*)szCurrent;
        szCurrent = (unsigned char*)(pcMatIn+1);
    }

    // if an ASCII effect description (HLSL?) is contained in the file,
    // we can simply ignore it ...
    if (iType & AI_MDL7_SKINTYPE_MATERIAL_ASCDEF)
    {
        int32_t iMe = *((int32_t*)szCurrent);
        AI_SWAP4(iMe);
        szCurrent += sizeof(char) * iMe + sizeof(int32_t);
    }
    *szCurrentOut = szCurrent;
}

// ------------------------------------------------------------------------------------------------
void MDLImporter::ParseSkinLump_3DGS_MDL7(
    const unsigned char* szCurrent,
    const unsigned char** szCurrentOut,
    std::vector<aiMaterial*>& pcMats)
{
    ai_assert(NULL != szCurrent);
    ai_assert(NULL != szCurrentOut);

    *szCurrentOut = szCurrent;
    BE_NCONST MDL::Skin_MDL7* pcSkin = (BE_NCONST MDL::Skin_MDL7*)szCurrent;
    AI_SWAP4(pcSkin->width);
    AI_SWAP4(pcSkin->height);
    szCurrent += 12;

    // allocate an output material
    aiMaterial* pcMatOut = new aiMaterial();
    pcMats.push_back(pcMatOut);

    // skip length of file name
    szCurrent += AI_MDL7_MAX_TEXNAMESIZE;

    ParseSkinLump_3DGS_MDL7(szCurrent,szCurrentOut,pcMatOut,
        pcSkin->typ,pcSkin->width,pcSkin->height);

    // place the name of the skin in the material
    if (pcSkin->texture_name[0])
    {
        // the 0 termination could be there or not - we can't know
        aiString szFile;
        ::memcpy(szFile.data,pcSkin->texture_name,sizeof(pcSkin->texture_name));
        szFile.data[sizeof(pcSkin->texture_name)] = '\0';
        szFile.length = ::strlen(szFile.data);

        pcMatOut->AddProperty(&szFile,AI_MATKEY_NAME);
    }
}

#endif // !! ASSIMP_BUILD_NO_MDL_IMPORTER
