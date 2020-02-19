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

/** @file Implementation of the PLY parser class */


#ifndef ASSIMP_BUILD_NO_PLY_IMPORTER

#include <assimp/fast_atof.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/ByteSwapper.h>
#include "PlyLoader.h"

using namespace Assimp;

// ------------------------------------------------------------------------------------------------
PLY::EDataType PLY::Property::ParseDataType(std::vector<char> &buffer) {
  ai_assert(!buffer.empty());

  PLY::EDataType eOut = PLY::EDT_INVALID;

  if (PLY::DOM::TokenMatch(buffer, "char", 4) ||
    PLY::DOM::TokenMatch(buffer, "int8", 4))
  {
    eOut = PLY::EDT_Char;
  }
  else if (PLY::DOM::TokenMatch(buffer, "uchar", 5) ||
    PLY::DOM::TokenMatch(buffer, "uint8", 5))
  {
    eOut = PLY::EDT_UChar;
  }
  else if (PLY::DOM::TokenMatch(buffer, "short", 5) ||
    PLY::DOM::TokenMatch(buffer, "int16", 5))
  {
    eOut = PLY::EDT_Short;
  }
  else if (PLY::DOM::TokenMatch(buffer, "ushort", 6) ||
    PLY::DOM::TokenMatch(buffer, "uint16", 6))
  {
    eOut = PLY::EDT_UShort;
  }
  else if (PLY::DOM::TokenMatch(buffer, "int32", 5) || PLY::DOM::TokenMatch(buffer, "int", 3))
  {
    eOut = PLY::EDT_Int;
  }
  else if (PLY::DOM::TokenMatch(buffer, "uint32", 6) || PLY::DOM::TokenMatch(buffer, "uint", 4))
  {
    eOut = PLY::EDT_UInt;
  }
  else if (PLY::DOM::TokenMatch(buffer, "float", 5) || PLY::DOM::TokenMatch(buffer, "float32", 7))
  {
    eOut = PLY::EDT_Float;
  }
  else if (PLY::DOM::TokenMatch(buffer, "double64", 8) || PLY::DOM::TokenMatch(buffer, "double", 6) ||
    PLY::DOM::TokenMatch(buffer, "float64", 7))
  {
    eOut = PLY::EDT_Double;
  }
  if (PLY::EDT_INVALID == eOut)
  {
      ASSIMP_LOG_INFO("Found unknown data type in PLY file. This is OK");
  }

  return eOut;
}

// ------------------------------------------------------------------------------------------------
PLY::ESemantic PLY::Property::ParseSemantic(std::vector<char> &buffer) {
  ai_assert(!buffer.empty());

  PLY::ESemantic eOut = PLY::EST_INVALID;
  if (PLY::DOM::TokenMatch(buffer, "red", 3)) {
    eOut = PLY::EST_Red;
  }
  else if (PLY::DOM::TokenMatch(buffer, "green", 5)) {
    eOut = PLY::EST_Green;
  }
  else if (PLY::DOM::TokenMatch(buffer, "blue", 4)) {
    eOut = PLY::EST_Blue;
  }
  else if (PLY::DOM::TokenMatch(buffer, "alpha", 5)) {
    eOut = PLY::EST_Alpha;
  }
  else if (PLY::DOM::TokenMatch(buffer, "vertex_index", 12) || PLY::DOM::TokenMatch(buffer, "vertex_indices", 14)) {
    eOut = PLY::EST_VertexIndex;
  }
  else if (PLY::DOM::TokenMatch(buffer, "texcoord", 8)) // Manage uv coords on faces
  {
    eOut = PLY::EST_TextureCoordinates;
  }
  else if (PLY::DOM::TokenMatch(buffer, "material_index", 14))
  {
    eOut = PLY::EST_MaterialIndex;
  }
  else if (PLY::DOM::TokenMatch(buffer, "ambient_red", 11))
  {
    eOut = PLY::EST_AmbientRed;
  }
  else if (PLY::DOM::TokenMatch(buffer, "ambient_green", 13))
  {
    eOut = PLY::EST_AmbientGreen;
  }
  else if (PLY::DOM::TokenMatch(buffer, "ambient_blue", 12))
  {
    eOut = PLY::EST_AmbientBlue;
  }
  else if (PLY::DOM::TokenMatch(buffer, "ambient_alpha", 13))
  {
    eOut = PLY::EST_AmbientAlpha;
  }
  else if (PLY::DOM::TokenMatch(buffer, "diffuse_red", 11))
  {
    eOut = PLY::EST_DiffuseRed;
  }
  else if (PLY::DOM::TokenMatch(buffer, "diffuse_green", 13))
  {
    eOut = PLY::EST_DiffuseGreen;
  }
  else if (PLY::DOM::TokenMatch(buffer, "diffuse_blue", 12))
  {
    eOut = PLY::EST_DiffuseBlue;
  }
  else if (PLY::DOM::TokenMatch(buffer, "diffuse_alpha", 13))
  {
    eOut = PLY::EST_DiffuseAlpha;
  }
  else if (PLY::DOM::TokenMatch(buffer, "specular_red", 12))
  {
    eOut = PLY::EST_SpecularRed;
  }
  else if (PLY::DOM::TokenMatch(buffer, "specular_green", 14))
  {
    eOut = PLY::EST_SpecularGreen;
  }
  else if (PLY::DOM::TokenMatch(buffer, "specular_blue", 13))
  {
    eOut = PLY::EST_SpecularBlue;
  }
  else if (PLY::DOM::TokenMatch(buffer, "specular_alpha", 14))
  {
    eOut = PLY::EST_SpecularAlpha;
  }
  else if (PLY::DOM::TokenMatch(buffer, "opacity", 7))
  {
    eOut = PLY::EST_Opacity;
  }
  else if (PLY::DOM::TokenMatch(buffer, "specular_power", 14))
  {
    eOut = PLY::EST_PhongPower;
  }
  else if (PLY::DOM::TokenMatch(buffer, "r", 1))
  {
    eOut = PLY::EST_Red;
  }
  else if (PLY::DOM::TokenMatch(buffer, "g", 1))
  {
    eOut = PLY::EST_Green;
  }
  else if (PLY::DOM::TokenMatch(buffer, "b", 1))
  {
    eOut = PLY::EST_Blue;
  }

  // NOTE: Blender3D exports texture coordinates as s,t tuples
  else if (PLY::DOM::TokenMatch(buffer, "u", 1) || PLY::DOM::TokenMatch(buffer, "s", 1) || PLY::DOM::TokenMatch(buffer, "tx", 2) || PLY::DOM::TokenMatch(buffer, "texture_u", 9))
  {
    eOut = PLY::EST_UTextureCoord;
  }
  else if (PLY::DOM::TokenMatch(buffer, "v", 1) || PLY::DOM::TokenMatch(buffer, "t", 1) || PLY::DOM::TokenMatch(buffer, "ty", 2) || PLY::DOM::TokenMatch(buffer, "texture_v", 9))
  {
    eOut = PLY::EST_VTextureCoord;
  }
  else if (PLY::DOM::TokenMatch(buffer, "x", 1))
  {
    eOut = PLY::EST_XCoord;
  }
  else if (PLY::DOM::TokenMatch(buffer, "y", 1)) {
    eOut = PLY::EST_YCoord;
  }
  else if (PLY::DOM::TokenMatch(buffer, "z", 1)) {
    eOut = PLY::EST_ZCoord;
  }
  else if (PLY::DOM::TokenMatch(buffer, "nx", 2)) {
    eOut = PLY::EST_XNormal;
  }
  else if (PLY::DOM::TokenMatch(buffer, "ny", 2)) {
    eOut = PLY::EST_YNormal;
  }
  else if (PLY::DOM::TokenMatch(buffer, "nz", 2)) {
    eOut = PLY::EST_ZNormal;
  }
  else {
      ASSIMP_LOG_INFO("Found unknown property semantic in file. This is ok");
    PLY::DOM::SkipLine(buffer);
  }
  return eOut;
}

// ------------------------------------------------------------------------------------------------
bool PLY::Property::ParseProperty(std::vector<char> &buffer, PLY::Property* pOut)
{
  ai_assert(!buffer.empty());

  // Forms supported:
  // "property float x"
  // "property list uchar int vertex_index"

  // skip leading spaces
  if (!PLY::DOM::SkipSpaces(buffer)) {
    return false;
  }

  // skip the "property" string at the beginning
  if (!PLY::DOM::TokenMatch(buffer, "property", 8))
  {
    // seems not to be a valid property entry
    return false;
  }
  // get next word
  if (!PLY::DOM::SkipSpaces(buffer)) {
    return false;
  }
  if (PLY::DOM::TokenMatch(buffer, "list", 4))
  {
    pOut->bIsList = true;

    // seems to be a list.
    if (EDT_INVALID == (pOut->eFirstType = PLY::Property::ParseDataType(buffer)))
    {
      // unable to parse list size data type
      PLY::DOM::SkipLine(buffer);
      return false;
    }
    if (!PLY::DOM::SkipSpaces(buffer))return false;
    if (EDT_INVALID == (pOut->eType = PLY::Property::ParseDataType(buffer)))
    {
      // unable to parse list data type
      PLY::DOM::SkipLine(buffer);
      return false;
    }
  }
  else
  {
    if (EDT_INVALID == (pOut->eType = PLY::Property::ParseDataType(buffer)))
    {
      // unable to parse data type. Skip the property
      PLY::DOM::SkipLine(buffer);
      return false;
    }
  }

  if (!PLY::DOM::SkipSpaces(buffer))
    return false;

  pOut->Semantic = PLY::Property::ParseSemantic(buffer);

  if (PLY::EST_INVALID == pOut->Semantic)
  {
    ASSIMP_LOG_INFO("Found unknown semantic in PLY file. This is OK");
    std::string(&buffer[0], &buffer[0] + strlen(&buffer[0]));
  }

  PLY::DOM::SkipSpacesAndLineEnd(buffer);
  return true;
}

// ------------------------------------------------------------------------------------------------
PLY::EElementSemantic PLY::Element::ParseSemantic(std::vector<char> &buffer)
{
  ai_assert(!buffer.empty());

  PLY::EElementSemantic eOut = PLY::EEST_INVALID;
  if (PLY::DOM::TokenMatch(buffer, "vertex", 6))
  {
    eOut = PLY::EEST_Vertex;
  }
  else if (PLY::DOM::TokenMatch(buffer, "face", 4))
  {
    eOut = PLY::EEST_Face;
  }
  else if (PLY::DOM::TokenMatch(buffer, "tristrips", 9))
  {
    eOut = PLY::EEST_TriStrip;
  }
#if 0
  // TODO: maybe implement this?
  else if (PLY::DOM::TokenMatch(buffer,"range_grid",10))
  {
    eOut = PLY::EEST_Face;
  }
#endif
  else if (PLY::DOM::TokenMatch(buffer, "edge", 4))
  {
    eOut = PLY::EEST_Edge;
  }
  else if (PLY::DOM::TokenMatch(buffer, "material", 8))
  {
    eOut = PLY::EEST_Material;
  }
  else if (PLY::DOM::TokenMatch(buffer, "TextureFile", 11))
  {
    eOut = PLY::EEST_TextureFile;
  }

  return eOut;
}

// ------------------------------------------------------------------------------------------------
bool PLY::Element::ParseElement(IOStreamBuffer<char> &streamBuffer, std::vector<char> &buffer, PLY::Element* pOut)
{
  ai_assert(NULL != pOut);
  // Example format: "element vertex 8"

  // skip leading spaces
  if (!PLY::DOM::SkipSpaces(buffer))
  {
    return false;
  }

  // skip the "element" string at the beginning
  if (!PLY::DOM::TokenMatch(buffer, "element", 7) && !PLY::DOM::TokenMatch(buffer, "comment", 7))
  {
    // seems not to be a valid property entry
    return false;
  }
  // get next word
  if (!PLY::DOM::SkipSpaces(buffer))
    return false;

  // parse the semantic of the element
  pOut->eSemantic = PLY::Element::ParseSemantic(buffer);
  if (PLY::EEST_INVALID == pOut->eSemantic)
  {
    // if the exact semantic can't be determined, just store
    // the original string identifier
    pOut->szName = std::string(&buffer[0], &buffer[0] + strlen(&buffer[0]));
  }

  if (!PLY::DOM::SkipSpaces(buffer))
    return false;

  if (PLY::EEST_TextureFile == pOut->eSemantic)
  {
    char* endPos = &buffer[0] + (strlen(&buffer[0]) - 1);
    pOut->szName = std::string(&buffer[0], endPos);

    // go to the next line
    PLY::DOM::SkipSpacesAndLineEnd(buffer);

    return true;
  }

  //parse the number of occurrences of this element
  const char* pCur = (char*)&buffer[0];
  pOut->NumOccur = strtoul10(pCur, &pCur);

  // go to the next line
  PLY::DOM::SkipSpacesAndLineEnd(buffer);

  // now parse all properties of the element
  while (true)
  {
    streamBuffer.getNextLine(buffer);
    pCur = (char*)&buffer[0];

    // skip all comments
    PLY::DOM::SkipComments(buffer);

    PLY::Property prop;
    if (!PLY::Property::ParseProperty(buffer, &prop))
      break;

    pOut->alProperties.push_back(prop);
  }

  return true;
}

// ------------------------------------------------------------------------------------------------
bool PLY::DOM::SkipSpaces(std::vector<char> &buffer)
{
  const char* pCur = buffer.empty() ? NULL : (char*)&buffer[0];
  bool ret = false;
  if (pCur)
  {
    const char* szCur = pCur;
    ret = Assimp::SkipSpaces(pCur, &pCur);

    uintptr_t iDiff = (uintptr_t)pCur - (uintptr_t)szCur;
    buffer.erase(buffer.begin(), buffer.begin() + iDiff);
    return ret;
  }

  return ret;
}

bool PLY::DOM::SkipLine(std::vector<char> &buffer)
{
  const char* pCur = buffer.empty() ? NULL : (char*)&buffer[0];
  bool ret = false;
  if (pCur)
  {
    const char* szCur = pCur;
    ret = Assimp::SkipLine(pCur, &pCur);

    uintptr_t iDiff = (uintptr_t)pCur - (uintptr_t)szCur;
    buffer.erase(buffer.begin(), buffer.begin() + iDiff);
    return ret;
  }

  return ret;
}

bool PLY::DOM::TokenMatch(std::vector<char> &buffer, const char* token, unsigned int len)
{
  const char* pCur = buffer.empty() ? NULL : (char*)&buffer[0];
  bool ret = false;
  if (pCur)
  {
    const char* szCur = pCur;
    ret = Assimp::TokenMatch(pCur, token, len);

    uintptr_t iDiff = (uintptr_t)pCur - (uintptr_t)szCur;
    buffer.erase(buffer.begin(), buffer.begin() + iDiff);
    return ret;
  }

  return ret;
}

bool PLY::DOM::SkipSpacesAndLineEnd(std::vector<char> &buffer)
{
  const char* pCur = buffer.empty() ? NULL : (char*)&buffer[0];
  bool ret = false;
  if (pCur)
  {
    const char* szCur = pCur;
    ret = Assimp::SkipSpacesAndLineEnd(pCur, &pCur);

    uintptr_t iDiff = (uintptr_t)pCur - (uintptr_t)szCur;
    buffer.erase(buffer.begin(), buffer.begin() + iDiff);
    return ret;
  }

  return ret;
}

bool PLY::DOM::SkipComments(std::vector<char> &buffer)
{
  ai_assert(!buffer.empty());

  std::vector<char> nbuffer = buffer;
  // skip spaces
  if (!SkipSpaces(nbuffer)) {
    return false;
  }

  if (TokenMatch(nbuffer, "comment", 7))
  {
    if (!SkipSpaces(nbuffer))
      SkipLine(nbuffer);

    if (!TokenMatch(nbuffer, "TextureFile", 11))
    {
      SkipLine(nbuffer);
      buffer = nbuffer;
      return true;
    }

    return true;
  }

  return false;
}

// ------------------------------------------------------------------------------------------------
bool PLY::DOM::ParseHeader(IOStreamBuffer<char> &streamBuffer, std::vector<char> &buffer, bool isBinary) {
    ASSIMP_LOG_DEBUG("PLY::DOM::ParseHeader() begin");

  // parse all elements
  while (!buffer.empty())
  {
    // skip all comments
    PLY::DOM::SkipComments(buffer);

    PLY::Element out;
    if (PLY::Element::ParseElement(streamBuffer, buffer, &out))
    {
      // add the element to the list of elements
      alElements.push_back(out);
    }
    else if (TokenMatch(buffer, "end_header", 10))
    {
      // we have reached the end of the header
      break;
    }
    else
    {
      // ignore unknown header elements
      streamBuffer.getNextLine(buffer);
    }
  }

  if (!isBinary) // it would occur an error, if binary data start with values as space or line end.
    SkipSpacesAndLineEnd(buffer);

  ASSIMP_LOG_DEBUG("PLY::DOM::ParseHeader() succeeded");
  return true;
}

// ------------------------------------------------------------------------------------------------
bool PLY::DOM::ParseElementInstanceLists(IOStreamBuffer<char> &streamBuffer, std::vector<char> &buffer, PLYImporter* loader)
{
    ASSIMP_LOG_DEBUG("PLY::DOM::ParseElementInstanceLists() begin");
  alElementData.resize(alElements.size());

  std::vector<PLY::Element>::const_iterator i = alElements.begin();
  std::vector<PLY::ElementInstanceList>::iterator a = alElementData.begin();

  // parse all element instances
  //construct vertices and faces
  for (; i != alElements.end(); ++i, ++a)
  {
    if ((*i).eSemantic == EEST_Vertex || (*i).eSemantic == EEST_Face || (*i).eSemantic == EEST_TriStrip)
    {
      PLY::ElementInstanceList::ParseInstanceList(streamBuffer, buffer, &(*i), NULL, loader);
    }
    else
    {
      (*a).alInstances.resize((*i).NumOccur);
      PLY::ElementInstanceList::ParseInstanceList(streamBuffer, buffer, &(*i), &(*a), NULL);
    }
  }

  ASSIMP_LOG_DEBUG("PLY::DOM::ParseElementInstanceLists() succeeded");
  return true;
}

// ------------------------------------------------------------------------------------------------
bool PLY::DOM::ParseElementInstanceListsBinary(IOStreamBuffer<char> &streamBuffer, std::vector<char> &buffer,
    const char* &pCur,
    unsigned int &bufferSize,
    PLYImporter* loader,
    bool p_bBE)
{
    ASSIMP_LOG_DEBUG("PLY::DOM::ParseElementInstanceListsBinary() begin");
  alElementData.resize(alElements.size());

  std::vector<PLY::Element>::const_iterator i = alElements.begin();
  std::vector<PLY::ElementInstanceList>::iterator a = alElementData.begin();

  // parse all element instances
  for (; i != alElements.end(); ++i, ++a)
  {
    if ((*i).eSemantic == EEST_Vertex || (*i).eSemantic == EEST_Face || (*i).eSemantic == EEST_TriStrip)
    {
      PLY::ElementInstanceList::ParseInstanceListBinary(streamBuffer, buffer, pCur, bufferSize, &(*i), NULL, loader, p_bBE);
    }
    else
    {
      (*a).alInstances.resize((*i).NumOccur);
      PLY::ElementInstanceList::ParseInstanceListBinary(streamBuffer, buffer, pCur, bufferSize, &(*i), &(*a), NULL, p_bBE);
    }
  }

  ASSIMP_LOG_DEBUG("PLY::DOM::ParseElementInstanceListsBinary() succeeded");
  return true;
}

// ------------------------------------------------------------------------------------------------
bool PLY::DOM::ParseInstanceBinary(IOStreamBuffer<char> &streamBuffer, DOM* p_pcOut, PLYImporter* loader, bool p_bBE)
{
  ai_assert(NULL != p_pcOut);
  ai_assert(NULL != loader);

  std::vector<char> buffer;
  streamBuffer.getNextLine(buffer);

  ASSIMP_LOG_DEBUG("PLY::DOM::ParseInstanceBinary() begin");

  if (!p_pcOut->ParseHeader(streamBuffer, buffer, true))
  {
      ASSIMP_LOG_DEBUG("PLY::DOM::ParseInstanceBinary() failure");
    return false;
  }

  streamBuffer.getNextBlock(buffer);
  unsigned int bufferSize = static_cast<unsigned int>(buffer.size());
  const char* pCur = (char*)&buffer[0];
  if (!p_pcOut->ParseElementInstanceListsBinary(streamBuffer, buffer, pCur, bufferSize, loader, p_bBE))
  {
      ASSIMP_LOG_DEBUG("PLY::DOM::ParseInstanceBinary() failure");
    return false;
  }
  ASSIMP_LOG_DEBUG("PLY::DOM::ParseInstanceBinary() succeeded");
  return true;
}

// ------------------------------------------------------------------------------------------------
bool PLY::DOM::ParseInstance(IOStreamBuffer<char> &streamBuffer, DOM* p_pcOut, PLYImporter* loader)
{
  ai_assert(NULL != p_pcOut);
  ai_assert(NULL != loader);

  std::vector<char> buffer;
  streamBuffer.getNextLine(buffer);

  ASSIMP_LOG_DEBUG("PLY::DOM::ParseInstance() begin");

  if (!p_pcOut->ParseHeader(streamBuffer, buffer, false))
  {
      ASSIMP_LOG_DEBUG("PLY::DOM::ParseInstance() failure");
    return false;
  }

  //get next line after header
  streamBuffer.getNextLine(buffer);
  if (!p_pcOut->ParseElementInstanceLists(streamBuffer, buffer, loader))
  {
      ASSIMP_LOG_DEBUG("PLY::DOM::ParseInstance() failure");
    return false;
  }
  ASSIMP_LOG_DEBUG("PLY::DOM::ParseInstance() succeeded");
  return true;
}

// ------------------------------------------------------------------------------------------------
bool PLY::ElementInstanceList::ParseInstanceList(
  IOStreamBuffer<char> &streamBuffer,
  std::vector<char> &buffer,
  const PLY::Element* pcElement,
  PLY::ElementInstanceList* p_pcOut,
  PLYImporter* loader)
{
  ai_assert(NULL != pcElement);

  // parse all elements
  if (EEST_INVALID == pcElement->eSemantic || pcElement->alProperties.empty())
  {
    // if the element has an unknown semantic we can skip all lines
    // However, there could be comments
    for (unsigned int i = 0; i < pcElement->NumOccur; ++i)
    {
      PLY::DOM::SkipComments(buffer);
      PLY::DOM::SkipLine(buffer);
      streamBuffer.getNextLine(buffer);
    }
  }
  else
  {
    const char* pCur = (const char*)&buffer[0];
    // be sure to have enough storage
    for (unsigned int i = 0; i < pcElement->NumOccur; ++i)
    {
      if (p_pcOut)
        PLY::ElementInstance::ParseInstance(pCur, pcElement, &p_pcOut->alInstances[i]);
      else
      {
        ElementInstance elt;
        PLY::ElementInstance::ParseInstance(pCur, pcElement, &elt);

        // Create vertex or face
        if (pcElement->eSemantic == EEST_Vertex)
        {
          //call loader instance from here
          loader->LoadVertex(pcElement, &elt, i);
        }
        else if (pcElement->eSemantic == EEST_Face)
        {
          //call loader instance from here
          loader->LoadFace(pcElement, &elt, i);
        }
        else if (pcElement->eSemantic == EEST_TriStrip)
        {
          //call loader instance from here
          loader->LoadFace(pcElement, &elt, i);
        }
      }

      streamBuffer.getNextLine(buffer);
      pCur = (buffer.empty()) ? NULL : (const char*)&buffer[0];
    }
  }
  return true;
}

// ------------------------------------------------------------------------------------------------
bool PLY::ElementInstanceList::ParseInstanceListBinary(
  IOStreamBuffer<char> &streamBuffer,
  std::vector<char> &buffer,
  const char* &pCur,
  unsigned int &bufferSize,
  const PLY::Element* pcElement,
  PLY::ElementInstanceList* p_pcOut,
  PLYImporter* loader,
  bool p_bBE /* = false */)
{
  ai_assert(NULL != pcElement);

  // we can add special handling code for unknown element semantics since
  // we can't skip it as a whole block (we don't know its exact size
  // due to the fact that lists could be contained in the property list
  // of the unknown element)
  for (unsigned int i = 0; i < pcElement->NumOccur; ++i)
  {
    if (p_pcOut)
      PLY::ElementInstance::ParseInstanceBinary(streamBuffer, buffer, pCur, bufferSize, pcElement, &p_pcOut->alInstances[i], p_bBE);
    else
    {
      ElementInstance elt;
      PLY::ElementInstance::ParseInstanceBinary(streamBuffer, buffer, pCur, bufferSize, pcElement, &elt, p_bBE);

      // Create vertex or face
      if (pcElement->eSemantic == EEST_Vertex)
      {
        //call loader instance from here
        loader->LoadVertex(pcElement, &elt, i);
      }
      else if (pcElement->eSemantic == EEST_Face)
      {
        //call loader instance from here
        loader->LoadFace(pcElement, &elt, i);
      }
      else if (pcElement->eSemantic == EEST_TriStrip)
      {
        //call loader instance from here
        loader->LoadFace(pcElement, &elt, i);
      }
    }
  }
  return true;
}

// ------------------------------------------------------------------------------------------------
bool PLY::ElementInstance::ParseInstance(const char* &pCur,
  const PLY::Element* pcElement,
  PLY::ElementInstance* p_pcOut)
{
  ai_assert(NULL != pcElement);
  ai_assert(NULL != p_pcOut);

  // allocate enough storage
  p_pcOut->alProperties.resize(pcElement->alProperties.size());

  std::vector<PLY::PropertyInstance>::iterator i = p_pcOut->alProperties.begin();
  std::vector<PLY::Property>::const_iterator  a = pcElement->alProperties.begin();
  for (; i != p_pcOut->alProperties.end(); ++i, ++a)
  {
    if (!(PLY::PropertyInstance::ParseInstance(pCur, &(*a), &(*i))))
    {
        ASSIMP_LOG_WARN("Unable to parse property instance. "
        "Skipping this element instance");

      PLY::PropertyInstance::ValueUnion v = PLY::PropertyInstance::DefaultValue((*a).eType);
      (*i).avList.push_back(v);
    }
  }
  return true;
}

// ------------------------------------------------------------------------------------------------
bool PLY::ElementInstance::ParseInstanceBinary(
  IOStreamBuffer<char> &streamBuffer,
  std::vector<char> &buffer,
  const char* &pCur,
  unsigned int &bufferSize,
  const PLY::Element* pcElement,
  PLY::ElementInstance* p_pcOut,
  bool p_bBE /* = false */)
{
  ai_assert(NULL != pcElement);
  ai_assert(NULL != p_pcOut);

  // allocate enough storage
  p_pcOut->alProperties.resize(pcElement->alProperties.size());

  std::vector<PLY::PropertyInstance>::iterator i = p_pcOut->alProperties.begin();
  std::vector<PLY::Property>::const_iterator   a = pcElement->alProperties.begin();
  for (; i != p_pcOut->alProperties.end(); ++i, ++a)
  {
    if (!(PLY::PropertyInstance::ParseInstanceBinary(streamBuffer, buffer, pCur, bufferSize, &(*a), &(*i), p_bBE)))
    {
        ASSIMP_LOG_WARN("Unable to parse binary property instance. "
        "Skipping this element instance");

      (*i).avList.push_back(PLY::PropertyInstance::DefaultValue((*a).eType));
    }
  }
  return true;
}

// ------------------------------------------------------------------------------------------------
bool PLY::PropertyInstance::ParseInstance(const char* &pCur,
  const PLY::Property* prop, PLY::PropertyInstance* p_pcOut)
{
  ai_assert(NULL != prop);
  ai_assert(NULL != p_pcOut);

  // skip spaces at the beginning
  if (!SkipSpaces(&pCur))
  {
    return false;
  }

  if (prop->bIsList)
  {
    // parse the number of elements in the list
    PLY::PropertyInstance::ValueUnion v;
    PLY::PropertyInstance::ParseValue(pCur, prop->eFirstType, &v);

    // convert to unsigned int
    unsigned int iNum = PLY::PropertyInstance::ConvertTo<unsigned int>(v, prop->eFirstType);

    // parse all list elements
    p_pcOut->avList.resize(iNum);
    for (unsigned int i = 0; i < iNum; ++i)
    {
      if (!SkipSpaces(&pCur))
        return false;

      PLY::PropertyInstance::ParseValue(pCur, prop->eType, &p_pcOut->avList[i]);
    }
  }
  else
  {
    // parse the property
    PLY::PropertyInstance::ValueUnion v;

    PLY::PropertyInstance::ParseValue(pCur, prop->eType, &v);
    p_pcOut->avList.push_back(v);
  }
  SkipSpacesAndLineEnd(&pCur);
  return true;
}

// ------------------------------------------------------------------------------------------------
bool PLY::PropertyInstance::ParseInstanceBinary(IOStreamBuffer<char> &streamBuffer, std::vector<char> &buffer,
  const char* &pCur,
  unsigned int &bufferSize,
  const PLY::Property* prop,
  PLY::PropertyInstance* p_pcOut,
  bool p_bBE)
{
  ai_assert(NULL != prop);
  ai_assert(NULL != p_pcOut);

  // parse all elements
  if (prop->bIsList)
  {
    // parse the number of elements in the list
    PLY::PropertyInstance::ValueUnion v;
    PLY::PropertyInstance::ParseValueBinary(streamBuffer, buffer, pCur, bufferSize, prop->eFirstType, &v, p_bBE);

    // convert to unsigned int
    unsigned int iNum = PLY::PropertyInstance::ConvertTo<unsigned int>(v, prop->eFirstType);

    // parse all list elements
    p_pcOut->avList.resize(iNum);
    for (unsigned int i = 0; i < iNum; ++i)
    {
      PLY::PropertyInstance::ParseValueBinary(streamBuffer, buffer, pCur, bufferSize, prop->eType, &p_pcOut->avList[i], p_bBE);
    }
  }
  else
  {
    // parse the property
    PLY::PropertyInstance::ValueUnion v;
    PLY::PropertyInstance::ParseValueBinary(streamBuffer, buffer, pCur, bufferSize, prop->eType, &v, p_bBE);
    p_pcOut->avList.push_back(v);
  }
  return true;
}

// ------------------------------------------------------------------------------------------------
PLY::PropertyInstance::ValueUnion PLY::PropertyInstance::DefaultValue(PLY::EDataType eType)
{
  PLY::PropertyInstance::ValueUnion out;

  switch (eType)
  {
  case EDT_Float:
    out.fFloat = 0.f;
    return out;

  case EDT_Double:
    out.fDouble = 0.;
    return out;

  default:;
  };
  out.iUInt = 0;
  return out;
}

// ------------------------------------------------------------------------------------------------
bool PLY::PropertyInstance::ParseValue(const char* &pCur,
  PLY::EDataType eType,
  PLY::PropertyInstance::ValueUnion* out)
{
  ai_assert(NULL != pCur);
  ai_assert(NULL != out);

  //calc element size
  bool ret = true;
  switch (eType)
  {
  case EDT_UInt:
  case EDT_UShort:
  case EDT_UChar:

    out->iUInt = (uint32_t)strtoul10(pCur, &pCur);
    break;

  case EDT_Int:
  case EDT_Short:
  case EDT_Char:

    out->iInt = (int32_t)strtol10(pCur, &pCur);
    break;

  case EDT_Float:
    // technically this should cast to float, but people tend to use float descriptors for double data
    // this is the best way to not risk losing precision on import and it doesn't hurt to do this
    ai_real f;
    pCur = fast_atoreal_move<ai_real>(pCur, f);
    out->fFloat = (ai_real)f;
    break;

  case EDT_Double:
    double d;
    pCur = fast_atoreal_move<double>(pCur, d);
    out->fDouble = (double)d;
    break;

  case EDT_INVALID:
  default:
    ret = false;
    break;
  }

  return ret;
}

// ------------------------------------------------------------------------------------------------
bool PLY::PropertyInstance::ParseValueBinary(IOStreamBuffer<char> &streamBuffer,
  std::vector<char> &buffer,
  const char* &pCur,
  unsigned int &bufferSize,
  PLY::EDataType eType,
  PLY::PropertyInstance::ValueUnion* out,
  bool p_bBE)
{
  ai_assert(NULL != out);

  //calc element size
  unsigned int lsize = 0;
  switch (eType)
  {
  case EDT_Char:
  case EDT_UChar:
    lsize = 1;
    break;

  case EDT_UShort:
  case EDT_Short:
    lsize = 2;
    break;

  case EDT_UInt:
  case EDT_Int:
  case EDT_Float:
    lsize = 4;
    break;

  case EDT_Double:
    lsize = 8;
    break;

  case EDT_INVALID:
  default:
      break;
  }

  //read the next file block if needed
  if (bufferSize < lsize)
  {
    std::vector<char> nbuffer;
    if (streamBuffer.getNextBlock(nbuffer))
    {
      //concat buffer contents
      buffer = std::vector<char>(buffer.end() - bufferSize, buffer.end());
      buffer.insert(buffer.end(), nbuffer.begin(), nbuffer.end());
      nbuffer.clear();
      bufferSize = static_cast<unsigned int>(buffer.size());
      pCur = (char*)&buffer[0];
    }
    else
    {
      throw DeadlyImportError("Invalid .ply file: File corrupted");
    }
  }

  bool ret = true;
  switch (eType)
  {
  case EDT_UInt:
  {
    uint32_t t;
    memcpy(&t, pCur, sizeof(uint32_t));
    pCur += sizeof(uint32_t);

    // Swap endianness
    if (p_bBE)ByteSwap::Swap(&t);
    out->iUInt = t;
    break;
  }

  case EDT_UShort:
  {
    uint16_t t;
    memcpy(&t, pCur, sizeof(uint16_t));
    pCur += sizeof(uint16_t);

    // Swap endianness
    if (p_bBE)ByteSwap::Swap(&t);
    out->iUInt = t;
    break;
  }

  case EDT_UChar:
  {
    uint8_t t;
    memcpy(&t, pCur, sizeof(uint8_t));
    pCur += sizeof(uint8_t);
    out->iUInt = t;
    break;
  }

  case EDT_Int:
  {
    int32_t t;
    memcpy(&t, pCur, sizeof(int32_t));
    pCur += sizeof(int32_t);

    // Swap endianness
    if (p_bBE)ByteSwap::Swap(&t);
    out->iInt = t;
    break;
  }

  case EDT_Short:
  {
    int16_t t;
    memcpy(&t, pCur, sizeof(int16_t));
    pCur += sizeof(int16_t);

    // Swap endianness
    if (p_bBE)ByteSwap::Swap(&t);
    out->iInt = t;
    break;
  }

  case EDT_Char:
  {
    int8_t t;
    memcpy(&t, pCur, sizeof(int8_t));
    pCur += sizeof(int8_t);
    out->iInt = t;
    break;
  }

  case EDT_Float:
  {
    float t;
    memcpy(&t, pCur, sizeof(float));
    pCur += sizeof(float);

    // Swap endianness
    if (p_bBE)ByteSwap::Swap(&t);
    out->fFloat = t;
    break;
  }
  case EDT_Double:
  {
    double t;
    memcpy(&t, pCur, sizeof(double));
    pCur += sizeof(double);

    // Swap endianness
    if (p_bBE)ByteSwap::Swap(&t);
    out->fDouble = t;
    break;
  }
  default:
    ret = false;
  }

  bufferSize -= lsize;

  return ret;
}

#endif // !! ASSIMP_BUILD_NO_PLY_IMPORTER
