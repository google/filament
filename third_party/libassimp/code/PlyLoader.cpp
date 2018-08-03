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

/** @file  PlyLoader.cpp
 *  @brief Implementation of the PLY importer class
 */

#ifndef ASSIMP_BUILD_NO_PLY_IMPORTER

// internal headers
#include "PlyLoader.h"
#include "IOStreamBuffer.h"
#include "Macros.h"
#include <memory>
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>
#include <assimp/importerdesc.h>

using namespace Assimp;

static const aiImporterDesc desc = {
  "Stanford Polygon Library (PLY) Importer",
  "",
  "",
  "",
  aiImporterFlags_SupportBinaryFlavour | aiImporterFlags_SupportTextFlavour,
  0,
  0,
  0,
  0,
  "ply"
};


// ------------------------------------------------------------------------------------------------
// Internal stuff
namespace
{
  // ------------------------------------------------------------------------------------------------
  // Checks that property index is within range
  template <class T>
  const T &GetProperty(const std::vector<T> &props, int idx)
  {
    if (static_cast<size_t>(idx) >= props.size()) {
      throw DeadlyImportError("Invalid .ply file: Property index is out of range.");
    }

    return props[idx];
  }
}


// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
PLYImporter::PLYImporter()
  : mBuffer()
  , pcDOM()
  , mGeneratedMesh(NULL){
  // empty
}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
PLYImporter::~PLYImporter() {
  // empty
}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool PLYImporter::CanRead(const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
  const std::string extension = GetExtension(pFile);

  if (extension == "ply")
    return true;
  else if (!extension.length() || checkSig)
  {
    if (!pIOHandler)return true;
    const char* tokens[] = { "ply" };
    return SearchFileHeaderForToken(pIOHandler, pFile, tokens, 1);
  }
  return false;
}

// ------------------------------------------------------------------------------------------------
const aiImporterDesc* PLYImporter::GetInfo() const
{
  return &desc;
}

// ------------------------------------------------------------------------------------------------
static bool isBigEndian(const char* szMe) {
  ai_assert(NULL != szMe);

  // binary_little_endian
  // binary_big_endian
  bool isBigEndian(false);
#if (defined AI_BUILD_BIG_ENDIAN)
  if ( 'l' == *szMe || 'L' == *szMe ) {
    isBigEndian = true;
  }
#else
  if ('b' == *szMe || 'B' == *szMe) {
    isBigEndian = true;
  }
#endif // ! AI_BUILD_BIG_ENDIAN

  return isBigEndian;
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void PLYImporter::InternReadFile(const std::string& pFile,
  aiScene* pScene, IOSystem* pIOHandler)
{
  static const std::string mode = "rb";
  std::unique_ptr<IOStream> fileStream(pIOHandler->Open(pFile, mode));
  if (!fileStream.get()) {
    throw DeadlyImportError("Failed to open file " + pFile + ".");
  }

  // Get the file-size
  size_t fileSize = fileStream->FileSize();
  if ( 0 == fileSize ) {
      throw DeadlyImportError("File " + pFile + " is empty.");
  }

  IOStreamBuffer<char> streamedBuffer(1024 * 1024);
  streamedBuffer.open(fileStream.get());

  // the beginning of the file must be PLY - magic, magic
  std::vector<char> headerCheck;
  streamedBuffer.getNextLine(headerCheck);

  if ((headerCheck.size() < 3) ||
      (headerCheck[0] != 'P' && headerCheck[0] != 'p') ||
      (headerCheck[1] != 'L' && headerCheck[1] != 'l') ||
      (headerCheck[2] != 'Y' && headerCheck[2] != 'y') )
  {
    streamedBuffer.close();
    throw DeadlyImportError("Invalid .ply file: Magic number \'ply\' is no there");
  }

  std::vector<char> mBuffer2;
  streamedBuffer.getNextLine(mBuffer2);
  mBuffer = (unsigned char*)&mBuffer2[0];

  char* szMe = (char*)&this->mBuffer[0];
  SkipSpacesAndLineEnd(szMe, (const char**)&szMe);

  // determine the format of the file data and construct the aimesh
  PLY::DOM sPlyDom;
  this->pcDOM = &sPlyDom;

  if (TokenMatch(szMe, "format", 6)) {
    if (TokenMatch(szMe, "ascii", 5)) {
      SkipLine(szMe, (const char**)&szMe);
      if (!PLY::DOM::ParseInstance(streamedBuffer, &sPlyDom, this))
      {
        if (mGeneratedMesh != NULL)
          delete(mGeneratedMesh);

        streamedBuffer.close();
        throw DeadlyImportError("Invalid .ply file: Unable to build DOM (#1)");
      }
    }
    else if (!::strncmp(szMe, "binary_", 7))
    {
      szMe += 7;
      const bool bIsBE(isBigEndian(szMe));

      // skip the line, parse the rest of the header and build the DOM
      if (!PLY::DOM::ParseInstanceBinary(streamedBuffer, &sPlyDom, this, bIsBE))
      {
        if (mGeneratedMesh != NULL)
          delete(mGeneratedMesh);

        streamedBuffer.close();
        throw DeadlyImportError("Invalid .ply file: Unable to build DOM (#2)");
      }
    }
    else
    {
      if (mGeneratedMesh != NULL)
        delete(mGeneratedMesh);

      streamedBuffer.close();
      throw DeadlyImportError("Invalid .ply file: Unknown file format");
    }
  }
  else
  {
    AI_DEBUG_INVALIDATE_PTR(this->mBuffer);
    if (mGeneratedMesh != NULL)
      delete(mGeneratedMesh);

    streamedBuffer.close();
    throw DeadlyImportError("Invalid .ply file: Missing format specification");
  }

  //free the file buffer
  streamedBuffer.close();

  if (mGeneratedMesh == NULL)
  {
    throw DeadlyImportError("Invalid .ply file: Unable to extract mesh data ");
  }

  // if no face list is existing we assume that the vertex
  // list is containing a list of points
  bool pointsOnly = mGeneratedMesh->mFaces == NULL ? true : false;
  if (pointsOnly)
  {
    if (mGeneratedMesh->mNumVertices < 3)
    {
      if (mGeneratedMesh != NULL)
        delete(mGeneratedMesh);

      streamedBuffer.close();
      throw DeadlyImportError("Invalid .ply file: Not enough "
        "vertices to build a proper face list. ");
    }

    const unsigned int iNum = (unsigned int)mGeneratedMesh->mNumVertices / 3;
    mGeneratedMesh->mNumFaces = iNum;
    mGeneratedMesh->mFaces = new aiFace[mGeneratedMesh->mNumFaces];

    for (unsigned int i = 0; i < iNum; ++i)
    {
      mGeneratedMesh->mFaces[i].mNumIndices = 3;
      mGeneratedMesh->mFaces[i].mIndices = new unsigned int[3];
      mGeneratedMesh->mFaces[i].mIndices[0] = (i * 3);
      mGeneratedMesh->mFaces[i].mIndices[1] = (i * 3) + 1;
      mGeneratedMesh->mFaces[i].mIndices[2] = (i * 3) + 2;
    }
  }

  // now load a list of all materials
  std::vector<aiMaterial*> avMaterials;
  std::string defaultTexture;
  LoadMaterial(&avMaterials, defaultTexture, pointsOnly);

  // now generate the output scene object. Fill the material list
  pScene->mNumMaterials = (unsigned int)avMaterials.size();
  pScene->mMaterials = new aiMaterial*[pScene->mNumMaterials];
  for (unsigned int i = 0; i < pScene->mNumMaterials; ++i) {
    pScene->mMaterials[i] = avMaterials[i];
  }

  // fill the mesh list
  pScene->mNumMeshes = 1;
  pScene->mMeshes = new aiMesh*[pScene->mNumMeshes];
  pScene->mMeshes[0] = mGeneratedMesh;

  // generate a simple node structure
  pScene->mRootNode = new aiNode();
  pScene->mRootNode->mNumMeshes = pScene->mNumMeshes;
  pScene->mRootNode->mMeshes = new unsigned int[pScene->mNumMeshes];

  for (unsigned int i = 0; i < pScene->mRootNode->mNumMeshes; ++i) {
    pScene->mRootNode->mMeshes[i] = i;
  }
}

void PLYImporter::LoadVertex(const PLY::Element* pcElement, const PLY::ElementInstance* instElement, unsigned int pos) {
    ai_assert(NULL != pcElement);
    ai_assert(NULL != instElement);

    ai_uint aiPositions[3] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
    PLY::EDataType aiTypes[3] = { EDT_Char, EDT_Char, EDT_Char };

    ai_uint aiNormal[3] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
    PLY::EDataType aiNormalTypes[3] = { EDT_Char, EDT_Char, EDT_Char };

    unsigned int aiColors[4] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
    PLY::EDataType aiColorsTypes[4] = { EDT_Char, EDT_Char, EDT_Char, EDT_Char };

    unsigned int aiTexcoord[2] = { 0xFFFFFFFF, 0xFFFFFFFF };
    PLY::EDataType aiTexcoordTypes[2] = { EDT_Char, EDT_Char };

    // now check whether which normal components are available
    unsigned int _a( 0 ), cnt( 0 );
    for ( std::vector<PLY::Property>::const_iterator a = pcElement->alProperties.begin();
            a != pcElement->alProperties.end(); ++a, ++_a) {
        if ((*a).bIsList) {
            continue;
        }

        // Positions
        if (PLY::EST_XCoord == (*a).Semantic) {
            ++cnt;
            aiPositions[0] = _a;
            aiTypes[0] = (*a).eType;
        } else if (PLY::EST_YCoord == (*a).Semantic) {
            ++cnt;
            aiPositions[1] = _a;
            aiTypes[1] = (*a).eType;
        } else if (PLY::EST_ZCoord == (*a).Semantic) {
            ++cnt;
            aiPositions[2] = _a;
            aiTypes[2] = (*a).eType;
        } else if (PLY::EST_XNormal == (*a).Semantic) {
            // Normals
            ++cnt;
            aiNormal[0] = _a;
            aiNormalTypes[0] = (*a).eType;
        } else if (PLY::EST_YNormal == (*a).Semantic) {
            ++cnt;
            aiNormal[1] = _a;
            aiNormalTypes[1] = (*a).eType;
        } else if (PLY::EST_ZNormal == (*a).Semantic) {
            ++cnt;
            aiNormal[2] = _a;
            aiNormalTypes[2] = (*a).eType;
        } else if (PLY::EST_Red == (*a).Semantic) {
            // Colors
            ++cnt;
            aiColors[0] = _a;
            aiColorsTypes[0] = (*a).eType;
        } else if (PLY::EST_Green == (*a).Semantic) {
            ++cnt;
            aiColors[1] = _a;
            aiColorsTypes[1] = (*a).eType;
        } else if (PLY::EST_Blue == (*a).Semantic) {
            ++cnt;
            aiColors[2] = _a;
            aiColorsTypes[2] = (*a).eType;
        } else if (PLY::EST_Alpha == (*a).Semantic) {
            ++cnt;
            aiColors[3] = _a;
            aiColorsTypes[3] = (*a).eType;
        } else if (PLY::EST_UTextureCoord == (*a).Semantic) {
            // Texture coordinates
            ++cnt;
            aiTexcoord[0] = _a;
            aiTexcoordTypes[0] = (*a).eType;
        } else if (PLY::EST_VTextureCoord == (*a).Semantic) {
            ++cnt;
            aiTexcoord[1] = _a;
            aiTexcoordTypes[1] = (*a).eType;
        }
    }

    // check whether we have a valid source for the vertex data
    if (0 != cnt) {
        // Position
        aiVector3D vOut;
        if (0xFFFFFFFF != aiPositions[0]) {
            vOut.x = PLY::PropertyInstance::ConvertTo<ai_real>(
                GetProperty(instElement->alProperties, aiPositions[0]).avList.front(), aiTypes[0]);
        }

        if (0xFFFFFFFF != aiPositions[1]) {
            vOut.y = PLY::PropertyInstance::ConvertTo<ai_real>(
                GetProperty(instElement->alProperties, aiPositions[1]).avList.front(), aiTypes[1]);
        }

        if (0xFFFFFFFF != aiPositions[2]) {
            vOut.z = PLY::PropertyInstance::ConvertTo<ai_real>(
                GetProperty(instElement->alProperties, aiPositions[2]).avList.front(), aiTypes[2]);
        }

        // Normals
        aiVector3D nOut;
        bool haveNormal = false;
        if (0xFFFFFFFF != aiNormal[0]) {
            nOut.x = PLY::PropertyInstance::ConvertTo<ai_real>(
                GetProperty(instElement->alProperties, aiNormal[0]).avList.front(), aiNormalTypes[0]);
            haveNormal = true;
        }

        if (0xFFFFFFFF != aiNormal[1]) {
            nOut.y = PLY::PropertyInstance::ConvertTo<ai_real>(
                GetProperty(instElement->alProperties, aiNormal[1]).avList.front(), aiNormalTypes[1]);
            haveNormal = true;
        }

        if (0xFFFFFFFF != aiNormal[2]) {
            nOut.z = PLY::PropertyInstance::ConvertTo<ai_real>(
                GetProperty(instElement->alProperties, aiNormal[2]).avList.front(), aiNormalTypes[2]);
            haveNormal = true;
        }

        //Colors
        aiColor4D cOut;
        bool haveColor = false;
        if (0xFFFFFFFF != aiColors[0]) {
            cOut.r = NormalizeColorValue(GetProperty(instElement->alProperties,
                aiColors[0]).avList.front(), aiColorsTypes[0]);
            haveColor = true;
        }

        if (0xFFFFFFFF != aiColors[1]) {
            cOut.g = NormalizeColorValue(GetProperty(instElement->alProperties,
                aiColors[1]).avList.front(), aiColorsTypes[1]);
            haveColor = true;
        }

        if (0xFFFFFFFF != aiColors[2]) {
            cOut.b = NormalizeColorValue(GetProperty(instElement->alProperties,
                aiColors[2]).avList.front(), aiColorsTypes[2]);
            haveColor = true;
        }

        // assume 1.0 for the alpha channel ifit is not set
        if (0xFFFFFFFF == aiColors[3]) {
            cOut.a = 1.0;
        } else {
            cOut.a = NormalizeColorValue(GetProperty(instElement->alProperties,
                aiColors[3]).avList.front(), aiColorsTypes[3]);

            haveColor = true;
        }

        //Texture coordinates
        aiVector3D tOut;
        tOut.z = 0;
        bool haveTextureCoords = false;
        if (0xFFFFFFFF != aiTexcoord[0]) {
            tOut.x = PLY::PropertyInstance::ConvertTo<ai_real>(
                GetProperty(instElement->alProperties, aiTexcoord[0]).avList.front(), aiTexcoordTypes[0]);
            haveTextureCoords = true;
        }

        if (0xFFFFFFFF != aiTexcoord[1]) {
            tOut.y = PLY::PropertyInstance::ConvertTo<ai_real>(
                GetProperty(instElement->alProperties, aiTexcoord[1]).avList.front(), aiTexcoordTypes[1]);
            haveTextureCoords = true;
        }

        //create aiMesh if needed
        if ( nullptr == mGeneratedMesh ) {
            mGeneratedMesh = new aiMesh();
            mGeneratedMesh->mMaterialIndex = 0;
        }

        if (nullptr == mGeneratedMesh->mVertices) {
            mGeneratedMesh->mNumVertices = pcElement->NumOccur;
            mGeneratedMesh->mVertices = new aiVector3D[mGeneratedMesh->mNumVertices];
        }

        mGeneratedMesh->mVertices[pos] = vOut;

        if (haveNormal) {
            if (nullptr == mGeneratedMesh->mNormals)
                mGeneratedMesh->mNormals = new aiVector3D[mGeneratedMesh->mNumVertices];
            mGeneratedMesh->mNormals[pos] = nOut;
        }

        if (haveColor) {
            if (nullptr == mGeneratedMesh->mColors[0])
                mGeneratedMesh->mColors[0] = new aiColor4D[mGeneratedMesh->mNumVertices];
            mGeneratedMesh->mColors[0][pos] = cOut;
        }

        if (haveTextureCoords) {
            if (nullptr == mGeneratedMesh->mTextureCoords[0]) {
                mGeneratedMesh->mNumUVComponents[0] = 2;
                mGeneratedMesh->mTextureCoords[0] = new aiVector3D[mGeneratedMesh->mNumVertices];
            }
            mGeneratedMesh->mTextureCoords[0][pos] = tOut;
        }
    }
}


// ------------------------------------------------------------------------------------------------
// Convert a color component to [0...1]
ai_real PLYImporter::NormalizeColorValue(PLY::PropertyInstance::ValueUnion val,
  PLY::EDataType eType)
{
  switch (eType)
  {
  case EDT_Float:
    return val.fFloat;
  case EDT_Double:
    return (ai_real)val.fDouble;

  case EDT_UChar:
    return (ai_real)val.iUInt / (ai_real)0xFF;
  case EDT_Char:
    return (ai_real)(val.iInt + (0xFF / 2)) / (ai_real)0xFF;
  case EDT_UShort:
    return (ai_real)val.iUInt / (ai_real)0xFFFF;
  case EDT_Short:
    return (ai_real)(val.iInt + (0xFFFF / 2)) / (ai_real)0xFFFF;
  case EDT_UInt:
    return (ai_real)val.iUInt / (ai_real)0xFFFF;
  case EDT_Int:
    return ((ai_real)val.iInt / (ai_real)0xFF) + 0.5f;
  default:;
  };
  return 0.0f;
}

// ------------------------------------------------------------------------------------------------
// Try to extract proper faces from the PLY DOM
void PLYImporter::LoadFace(const PLY::Element* pcElement, const PLY::ElementInstance* instElement, unsigned int pos)
{
  ai_assert(NULL != pcElement);
  ai_assert(NULL != instElement);

  if (mGeneratedMesh == NULL)
    throw DeadlyImportError("Invalid .ply file: Vertices should be declared before faces");

  bool bOne = false;

  // index of the vertex index list
  unsigned int iProperty = 0xFFFFFFFF;
  PLY::EDataType eType = EDT_Char;
  bool bIsTriStrip = false;

  // index of the material index property
  //unsigned int iMaterialIndex = 0xFFFFFFFF;
  //PLY::EDataType eType2 = EDT_Char;

  // texture coordinates
  unsigned int iTextureCoord = 0xFFFFFFFF;
  PLY::EDataType eType3 = EDT_Char;

  // face = unique number of vertex indices
  if (PLY::EEST_Face == pcElement->eSemantic)
  {
    unsigned int _a = 0;
    for (std::vector<PLY::Property>::const_iterator a = pcElement->alProperties.begin();
      a != pcElement->alProperties.end(); ++a, ++_a)
    {
      if (PLY::EST_VertexIndex == (*a).Semantic)
      {
        // must be a dynamic list!
        if (!(*a).bIsList)
          continue;

        iProperty = _a;
        bOne = true;
        eType = (*a).eType;
      }
      /*else if (PLY::EST_MaterialIndex == (*a).Semantic)
      {
      if ((*a).bIsList)
      continue;
      iMaterialIndex = _a;
      bOne = true;
      eType2 = (*a).eType;
      }*/
      else if (PLY::EST_TextureCoordinates == (*a).Semantic)
      {
        // must be a dynamic list!
        if (!(*a).bIsList)
          continue;
        iTextureCoord = _a;
        bOne = true;
        eType3 = (*a).eType;
      }
    }
  }
  // triangle strip
  // TODO: triangle strip and material index support???
  else if (PLY::EEST_TriStrip == pcElement->eSemantic)
  {
    unsigned int _a = 0;
    for (std::vector<PLY::Property>::const_iterator a = pcElement->alProperties.begin();
      a != pcElement->alProperties.end(); ++a, ++_a)
    {
      // must be a dynamic list!
      if (!(*a).bIsList)
        continue;
      iProperty = _a;
      bOne = true;
      bIsTriStrip = true;
      eType = (*a).eType;
      break;
    }
  }

  // check whether we have at least one per-face information set
  if (bOne)
  {
    if (mGeneratedMesh->mFaces == NULL)
    {
      mGeneratedMesh->mNumFaces = pcElement->NumOccur;
      mGeneratedMesh->mFaces = new aiFace[mGeneratedMesh->mNumFaces];
    }

    if (!bIsTriStrip)
    {
      // parse the list of vertex indices
      if (0xFFFFFFFF != iProperty)
      {
        const unsigned int iNum = (unsigned int)GetProperty(instElement->alProperties, iProperty).avList.size();
        mGeneratedMesh->mFaces[pos].mNumIndices = iNum;
        mGeneratedMesh->mFaces[pos].mIndices = new unsigned int[iNum];

        std::vector<PLY::PropertyInstance::ValueUnion>::const_iterator p =
          GetProperty(instElement->alProperties, iProperty).avList.begin();

        for (unsigned int a = 0; a < iNum; ++a, ++p)
        {
          mGeneratedMesh->mFaces[pos].mIndices[a] = PLY::PropertyInstance::ConvertTo<unsigned int>(*p, eType);
        }
      }

      // parse the material index
      // cannot be handled without processing the whole file first
      /*if (0xFFFFFFFF != iMaterialIndex)
      {
      mGeneratedMesh->mFaces[pos]. = PLY::PropertyInstance::ConvertTo<unsigned int>(
      GetProperty(instElement->alProperties, iMaterialIndex).avList.front(), eType2);
      }*/

      if (0xFFFFFFFF != iTextureCoord)
      {
        const unsigned int iNum = (unsigned int)GetProperty(instElement->alProperties, iTextureCoord).avList.size();

        //should be 6 coords
        std::vector<PLY::PropertyInstance::ValueUnion>::const_iterator p =
          GetProperty(instElement->alProperties, iTextureCoord).avList.begin();

        if ((iNum / 3) == 2) // X Y coord
        {
          for (unsigned int a = 0; a < iNum; ++a, ++p)
          {
            unsigned int vindex = mGeneratedMesh->mFaces[pos].mIndices[a / 2];
            if (vindex < mGeneratedMesh->mNumVertices)
            {
              if (mGeneratedMesh->mTextureCoords[0] == NULL)
              {
                mGeneratedMesh->mNumUVComponents[0] = 2;
                mGeneratedMesh->mTextureCoords[0] = new aiVector3D[mGeneratedMesh->mNumVertices];
              }

              if (a % 2 == 0)
                mGeneratedMesh->mTextureCoords[0][vindex].x = PLY::PropertyInstance::ConvertTo<ai_real>(*p, eType3);
              else
                mGeneratedMesh->mTextureCoords[0][vindex].y = PLY::PropertyInstance::ConvertTo<ai_real>(*p, eType3);

              mGeneratedMesh->mTextureCoords[0][vindex].z = 0;
            }
          }
        }
      }
    }
    else // triangle strips
    {
      // normally we have only one triangle strip instance where
      // a value of -1 indicates a restart of the strip
      bool flip = false;
      const std::vector<PLY::PropertyInstance::ValueUnion>& quak = GetProperty(instElement->alProperties, iProperty).avList;
      //pvOut->reserve(pvOut->size() + quak.size() + (quak.size()>>2u)); //Limits memory consumption

      int aiTable[2] = { -1, -1 };
      for (std::vector<PLY::PropertyInstance::ValueUnion>::const_iterator a = quak.begin(); a != quak.end(); ++a)  {
        const int p = PLY::PropertyInstance::ConvertTo<int>(*a, eType);

        if (-1 == p)    {
          // restart the strip ...
          aiTable[0] = aiTable[1] = -1;
          flip = false;
          continue;
        }
        if (-1 == aiTable[0]) {
          aiTable[0] = p;
          continue;
        }
        if (-1 == aiTable[1]) {
          aiTable[1] = p;
          continue;
        }

        if (mGeneratedMesh->mFaces == NULL)
        {
          mGeneratedMesh->mNumFaces = pcElement->NumOccur;
          mGeneratedMesh->mFaces = new aiFace[mGeneratedMesh->mNumFaces];
        }

        mGeneratedMesh->mFaces[pos].mNumIndices = 3;
        mGeneratedMesh->mFaces[pos].mIndices = new unsigned int[3];
        mGeneratedMesh->mFaces[pos].mIndices[0] = aiTable[0];
        mGeneratedMesh->mFaces[pos].mIndices[1] = aiTable[1];
        mGeneratedMesh->mFaces[pos].mIndices[2] = p;

        if ((flip = !flip)) {
          std::swap(mGeneratedMesh->mFaces[pos].mIndices[0], mGeneratedMesh->mFaces[pos].mIndices[1]);
        }

        aiTable[0] = aiTable[1];
        aiTable[1] = p;
      }
    }
  }
}

// ------------------------------------------------------------------------------------------------
// Get a RGBA color in [0...1] range
void PLYImporter::GetMaterialColor(const std::vector<PLY::PropertyInstance>& avList,
  unsigned int aiPositions[4],
  PLY::EDataType aiTypes[4],
  aiColor4D* clrOut)
{
  ai_assert(NULL != clrOut);

  if (0xFFFFFFFF == aiPositions[0])clrOut->r = 0.0f;
  else
  {
    clrOut->r = NormalizeColorValue(GetProperty(avList,
      aiPositions[0]).avList.front(), aiTypes[0]);
  }

  if (0xFFFFFFFF == aiPositions[1])clrOut->g = 0.0f;
  else
  {
    clrOut->g = NormalizeColorValue(GetProperty(avList,
      aiPositions[1]).avList.front(), aiTypes[1]);
  }

  if (0xFFFFFFFF == aiPositions[2])clrOut->b = 0.0f;
  else
  {
    clrOut->b = NormalizeColorValue(GetProperty(avList,
      aiPositions[2]).avList.front(), aiTypes[2]);
  }

  // assume 1.0 for the alpha channel ifit is not set
  if (0xFFFFFFFF == aiPositions[3])clrOut->a = 1.0f;
  else
  {
    clrOut->a = NormalizeColorValue(GetProperty(avList,
      aiPositions[3]).avList.front(), aiTypes[3]);
  }
}

// ------------------------------------------------------------------------------------------------
// Extract a material from the PLY DOM
void PLYImporter::LoadMaterial(std::vector<aiMaterial*>* pvOut, std::string &defaultTexture, const bool pointsOnly)
{
  ai_assert(NULL != pvOut);

  // diffuse[4], specular[4], ambient[4]
  // rgba order
  unsigned int aaiPositions[3][4] = {

    { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF },
    { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF },
    { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF },
  };

  PLY::EDataType aaiTypes[3][4] = {
    { EDT_Char, EDT_Char, EDT_Char, EDT_Char },
    { EDT_Char, EDT_Char, EDT_Char, EDT_Char },
    { EDT_Char, EDT_Char, EDT_Char, EDT_Char }
  };
  PLY::ElementInstanceList* pcList = NULL;

  unsigned int iPhong = 0xFFFFFFFF;
  PLY::EDataType ePhong = EDT_Char;

  unsigned int iOpacity = 0xFFFFFFFF;
  PLY::EDataType eOpacity = EDT_Char;

  // search in the DOM for a vertex entry
  unsigned int _i = 0;
  for (std::vector<PLY::Element>::const_iterator i = this->pcDOM->alElements.begin();
    i != this->pcDOM->alElements.end(); ++i, ++_i)
  {
    if (PLY::EEST_Material == (*i).eSemantic)
    {
      pcList = &this->pcDOM->alElementData[_i];

      // now check whether which coordinate sets are available
      unsigned int _a = 0;
      for (std::vector<PLY::Property>::const_iterator
        a = (*i).alProperties.begin();
        a != (*i).alProperties.end(); ++a, ++_a)
      {
        if ((*a).bIsList)continue;

        // pohng specularity      -----------------------------------
        if (PLY::EST_PhongPower == (*a).Semantic)
        {
          iPhong = _a;
          ePhong = (*a).eType;
        }

        // general opacity        -----------------------------------
        if (PLY::EST_Opacity == (*a).Semantic)
        {
          iOpacity = _a;
          eOpacity = (*a).eType;
        }

        // diffuse color channels -----------------------------------
        if (PLY::EST_DiffuseRed == (*a).Semantic)
        {
          aaiPositions[0][0] = _a;
          aaiTypes[0][0] = (*a).eType;
        }
        else if (PLY::EST_DiffuseGreen == (*a).Semantic)
        {
          aaiPositions[0][1] = _a;
          aaiTypes[0][1] = (*a).eType;
        }
        else if (PLY::EST_DiffuseBlue == (*a).Semantic)
        {
          aaiPositions[0][2] = _a;
          aaiTypes[0][2] = (*a).eType;
        }
        else if (PLY::EST_DiffuseAlpha == (*a).Semantic)
        {
          aaiPositions[0][3] = _a;
          aaiTypes[0][3] = (*a).eType;
        }
        // specular color channels -----------------------------------
        else if (PLY::EST_SpecularRed == (*a).Semantic)
        {
          aaiPositions[1][0] = _a;
          aaiTypes[1][0] = (*a).eType;
        }
        else if (PLY::EST_SpecularGreen == (*a).Semantic)
        {
          aaiPositions[1][1] = _a;
          aaiTypes[1][1] = (*a).eType;
        }
        else if (PLY::EST_SpecularBlue == (*a).Semantic)
        {
          aaiPositions[1][2] = _a;
          aaiTypes[1][2] = (*a).eType;
        }
        else if (PLY::EST_SpecularAlpha == (*a).Semantic)
        {
          aaiPositions[1][3] = _a;
          aaiTypes[1][3] = (*a).eType;
        }
        // ambient color channels -----------------------------------
        else if (PLY::EST_AmbientRed == (*a).Semantic)
        {
          aaiPositions[2][0] = _a;
          aaiTypes[2][0] = (*a).eType;
        }
        else if (PLY::EST_AmbientGreen == (*a).Semantic)
        {
          aaiPositions[2][1] = _a;
          aaiTypes[2][1] = (*a).eType;
        }
        else if (PLY::EST_AmbientBlue == (*a).Semantic)
        {
          aaiPositions[2][2] = _a;
          aaiTypes[2][2] = (*a).eType;
        }
        else if (PLY::EST_AmbientAlpha == (*a).Semantic)
        {
          aaiPositions[2][3] = _a;
          aaiTypes[2][3] = (*a).eType;
        }
      }
      break;
    }
    else if (PLY::EEST_TextureFile == (*i).eSemantic)
    {
      defaultTexture = (*i).szName;
    }
  }
  // check whether we have a valid source for the material data
  if (NULL != pcList) {
    for (std::vector<ElementInstance>::const_iterator i = pcList->alInstances.begin(); i != pcList->alInstances.end(); ++i)  {
      aiColor4D clrOut;
      aiMaterial* pcHelper = new aiMaterial();

      // build the diffuse material color
      GetMaterialColor((*i).alProperties, aaiPositions[0], aaiTypes[0], &clrOut);
      pcHelper->AddProperty<aiColor4D>(&clrOut, 1, AI_MATKEY_COLOR_DIFFUSE);

      // build the specular material color
      GetMaterialColor((*i).alProperties, aaiPositions[1], aaiTypes[1], &clrOut);
      pcHelper->AddProperty<aiColor4D>(&clrOut, 1, AI_MATKEY_COLOR_SPECULAR);

      // build the ambient material color
      GetMaterialColor((*i).alProperties, aaiPositions[2], aaiTypes[2], &clrOut);
      pcHelper->AddProperty<aiColor4D>(&clrOut, 1, AI_MATKEY_COLOR_AMBIENT);

      // handle phong power and shading mode
      int iMode = (int)aiShadingMode_Gouraud;
      if (0xFFFFFFFF != iPhong)   {
        ai_real fSpec = PLY::PropertyInstance::ConvertTo<ai_real>(GetProperty((*i).alProperties, iPhong).avList.front(), ePhong);

        // if shininess is 0 (and the pow() calculation would therefore always
        // become 1, not depending on the angle), use gouraud lighting
        if (fSpec)  {
          // scale this with 15 ... hopefully this is correct
          fSpec *= 15;
          pcHelper->AddProperty<ai_real>(&fSpec, 1, AI_MATKEY_SHININESS);

          iMode = (int)aiShadingMode_Phong;
        }
      }
      pcHelper->AddProperty<int>(&iMode, 1, AI_MATKEY_SHADING_MODEL);

      // handle opacity
      if (0xFFFFFFFF != iOpacity) {
        ai_real fOpacity = PLY::PropertyInstance::ConvertTo<ai_real>(GetProperty((*i).alProperties, iPhong).avList.front(), eOpacity);
        pcHelper->AddProperty<ai_real>(&fOpacity, 1, AI_MATKEY_OPACITY);
      }

      // The face order is absolutely undefined for PLY, so we have to
      // use two-sided rendering to be sure it's ok.
      const int two_sided = 1;
      pcHelper->AddProperty(&two_sided, 1, AI_MATKEY_TWOSIDED);

      //default texture
      if (!defaultTexture.empty())
      {
        const aiString name(defaultTexture.c_str());
        pcHelper->AddProperty(&name, _AI_MATKEY_TEXTURE_BASE, aiTextureType_DIFFUSE, 0);
      }

      if (!pointsOnly)
      {
        const int two_sided = 1;
        pcHelper->AddProperty(&two_sided, 1, AI_MATKEY_TWOSIDED);
      }

      //set to wireframe, so when using this material info we can switch to points rendering
      if (pointsOnly)
      {
        const int wireframe = 1;
        pcHelper->AddProperty(&wireframe, 1, AI_MATKEY_ENABLE_WIREFRAME);
      }

      // add the newly created material instance to the list
      pvOut->push_back(pcHelper);
    }
  }
  else
  {
    // generate a default material
    aiMaterial* pcHelper = new aiMaterial();

    // fill in a default material
    int iMode = (int)aiShadingMode_Gouraud;
    pcHelper->AddProperty<int>(&iMode, 1, AI_MATKEY_SHADING_MODEL);

    //generate white material most 3D engine just multiply ambient / diffuse color with actual ambient / light color
    aiColor3D clr;
    clr.b = clr.g = clr.r = 1.0f;
    pcHelper->AddProperty<aiColor3D>(&clr, 1, AI_MATKEY_COLOR_DIFFUSE);
    pcHelper->AddProperty<aiColor3D>(&clr, 1, AI_MATKEY_COLOR_SPECULAR);

    clr.b = clr.g = clr.r = 1.0f;
    pcHelper->AddProperty<aiColor3D>(&clr, 1, AI_MATKEY_COLOR_AMBIENT);

    // The face order is absolutely undefined for PLY, so we have to
    // use two-sided rendering to be sure it's ok.
    if (!pointsOnly)
    {
      const int two_sided = 1;
      pcHelper->AddProperty(&two_sided, 1, AI_MATKEY_TWOSIDED);
    }

    //default texture
    if (!defaultTexture.empty())
    {
      const aiString name(defaultTexture.c_str());
      pcHelper->AddProperty(&name, _AI_MATKEY_TEXTURE_BASE, aiTextureType_DIFFUSE, 0);
    }

    //set to wireframe, so when using this material info we can switch to points rendering
    if (pointsOnly)
    {
      const int wireframe = 1;
      pcHelper->AddProperty(&wireframe, 1, AI_MATKEY_ENABLE_WIREFRAME);
    }

    pvOut->push_back(pcHelper);
  }
}

#endif // !! ASSIMP_BUILD_NO_PLY_IMPORTER
