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

#ifndef ASSIMP_BUILD_NO_OGRE_IMPORTER

#include "OgreStructs.h"
#include "TinyFormatter.h"
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include "Exceptional.h"


namespace Assimp
{
namespace Ogre
{

// VertexElement

VertexElement::VertexElement() :
    index(0),
    source(0),
    offset(0),
    type(VET_FLOAT1),
    semantic(VES_POSITION)
{
}

size_t VertexElement::Size() const
{
    return TypeSize(type);
}

size_t VertexElement::ComponentCount() const
{
    return ComponentCount(type);
}

size_t VertexElement::ComponentCount(Type type)
{
    switch(type)
    {
        case VET_COLOUR:
        case VET_COLOUR_ABGR:
        case VET_COLOUR_ARGB:
        case VET_FLOAT1:
        case VET_DOUBLE1:
        case VET_SHORT1:
        case VET_USHORT1:
        case VET_INT1:
        case VET_UINT1:
            return 1;
        case VET_FLOAT2:
        case VET_DOUBLE2:
        case VET_SHORT2:
        case VET_USHORT2:
        case VET_INT2:
        case VET_UINT2:
            return 2;
        case VET_FLOAT3:
        case VET_DOUBLE3:
        case VET_SHORT3:
        case VET_USHORT3:
        case VET_INT3:
        case VET_UINT3:
            return 3;
        case VET_FLOAT4:
        case VET_DOUBLE4:
        case VET_SHORT4:
        case VET_USHORT4:
        case VET_INT4:
        case VET_UINT4:
        case VET_UBYTE4:
            return 4;
    }
    return 0;
}

size_t VertexElement::TypeSize(Type type)
{
    switch(type)
    {
        case VET_COLOUR:
        case VET_COLOUR_ABGR:
        case VET_COLOUR_ARGB:
            return sizeof(unsigned int);
        case VET_FLOAT1:
            return sizeof(float);
        case VET_FLOAT2:
            return sizeof(float)*2;
        case VET_FLOAT3:
            return sizeof(float)*3;
        case VET_FLOAT4:
            return sizeof(float)*4;
        case VET_DOUBLE1:
            return sizeof(double);
        case VET_DOUBLE2:
            return sizeof(double)*2;
        case VET_DOUBLE3:
            return sizeof(double)*3;
        case VET_DOUBLE4:
            return sizeof(double)*4;
        case VET_SHORT1:
            return sizeof(short);
        case VET_SHORT2:
            return sizeof(short)*2;
        case VET_SHORT3:
            return sizeof(short)*3;
        case VET_SHORT4:
            return sizeof(short)*4;
        case VET_USHORT1:
            return sizeof(unsigned short);
        case VET_USHORT2:
            return sizeof(unsigned short)*2;
        case VET_USHORT3:
            return sizeof(unsigned short)*3;
        case VET_USHORT4:
            return sizeof(unsigned short)*4;
        case VET_INT1:
            return sizeof(int);
        case VET_INT2:
            return sizeof(int)*2;
        case VET_INT3:
            return sizeof(int)*3;
        case VET_INT4:
            return sizeof(int)*4;
        case VET_UINT1:
            return sizeof(unsigned int);
        case VET_UINT2:
            return sizeof(unsigned int)*2;
        case VET_UINT3:
            return sizeof(unsigned int)*3;
        case VET_UINT4:
            return sizeof(unsigned int)*4;
        case VET_UBYTE4:
            return sizeof(unsigned char)*4;
    }
    return 0;
}

std::string VertexElement::TypeToString()
{
    return TypeToString(type);
}

std::string VertexElement::TypeToString(Type type)
{
    switch(type)
    {
        case VET_COLOUR:        return "COLOUR";
        case VET_COLOUR_ABGR:   return "COLOUR_ABGR";
        case VET_COLOUR_ARGB:   return "COLOUR_ARGB";
        case VET_FLOAT1:        return "FLOAT1";
        case VET_FLOAT2:        return "FLOAT2";
        case VET_FLOAT3:        return "FLOAT3";
        case VET_FLOAT4:        return "FLOAT4";
        case VET_DOUBLE1:       return "DOUBLE1";
        case VET_DOUBLE2:       return "DOUBLE2";
        case VET_DOUBLE3:       return "DOUBLE3";
        case VET_DOUBLE4:       return "DOUBLE4";
        case VET_SHORT1:        return "SHORT1";
        case VET_SHORT2:        return "SHORT2";
        case VET_SHORT3:        return "SHORT3";
        case VET_SHORT4:        return "SHORT4";
        case VET_USHORT1:       return "USHORT1";
        case VET_USHORT2:       return "USHORT2";
        case VET_USHORT3:       return "USHORT3";
        case VET_USHORT4:       return "USHORT4";
        case VET_INT1:          return "INT1";
        case VET_INT2:          return "INT2";
        case VET_INT3:          return "INT3";
        case VET_INT4:          return "INT4";
        case VET_UINT1:         return "UINT1";
        case VET_UINT2:         return "UINT2";
        case VET_UINT3:         return "UINT3";
        case VET_UINT4:         return "UINT4";
        case VET_UBYTE4:        return "UBYTE4";
    }
    return "Uknown_VertexElement::Type";
}

std::string VertexElement::SemanticToString()
{
    return SemanticToString(semantic);
}

std::string VertexElement::SemanticToString(Semantic semantic)
{
    switch(semantic)
    {
        case VES_POSITION:              return "POSITION";
        case VES_BLEND_WEIGHTS:         return "BLEND_WEIGHTS";
        case VES_BLEND_INDICES:         return "BLEND_INDICES";
        case VES_NORMAL:                return "NORMAL";
        case VES_DIFFUSE:               return "DIFFUSE";
        case VES_SPECULAR:              return "SPECULAR";
        case VES_TEXTURE_COORDINATES:   return "TEXTURE_COORDINATES";
        case VES_BINORMAL:              return "BINORMAL";
        case VES_TANGENT:               return "TANGENT";
    }
    return "Uknown_VertexElement::Semantic";
}

// IVertexData

IVertexData::IVertexData() :
    count(0)
{
}

bool IVertexData::HasBoneAssignments() const
{
    return !boneAssignments.empty();
}

void IVertexData::AddVertexMapping(uint32_t oldIndex, uint32_t newIndex)
{
    BoneAssignmentsForVertex(oldIndex, newIndex, boneAssignmentsMap[newIndex]);
    vertexIndexMapping[oldIndex].push_back(newIndex);
}

void IVertexData::BoneAssignmentsForVertex(uint32_t currentIndex, uint32_t newIndex, VertexBoneAssignmentList &dest) const
{
    for (const auto &boneAssign : boneAssignments)
    {
        if (boneAssign.vertexIndex == currentIndex)
        {
            VertexBoneAssignment a = boneAssign;
            a.vertexIndex = newIndex;
            dest.push_back(a);
        }
    }
}

AssimpVertexBoneWeightList IVertexData::AssimpBoneWeights(size_t vertices)
{
    AssimpVertexBoneWeightList weights;
    for(size_t vi=0; vi<vertices; ++vi)
    {
        VertexBoneAssignmentList &vertexWeights = boneAssignmentsMap[static_cast<unsigned int>(vi)];
        for (VertexBoneAssignmentList::const_iterator iter=vertexWeights.begin(), end=vertexWeights.end();
            iter!=end; ++iter)
        {
            std::vector<aiVertexWeight> &boneWeights = weights[iter->boneIndex];
            boneWeights.push_back(aiVertexWeight(static_cast<unsigned int>(vi), iter->weight));
        }
    }
    return weights;
}

std::set<uint16_t> IVertexData::ReferencedBonesByWeights() const
{
    std::set<uint16_t> referenced;
    for (const auto &boneAssign : boneAssignments)
    {
        referenced.insert(boneAssign.boneIndex);
    }
    return referenced;
}

// VertexData

VertexData::VertexData()
{
}

VertexData::~VertexData()
{
    Reset();
}

void VertexData::Reset()
{
    // Releases shared ptr memory streams.
    vertexBindings.clear();
    vertexElements.clear();
}

uint32_t VertexData::VertexSize(uint16_t source) const
{
    uint32_t size = 0;
    for(const auto &element : vertexElements)
    {
        if (element.source == source)
            size += static_cast<uint32_t>(element.Size());
    }
    return size;
}

MemoryStream *VertexData::VertexBuffer(uint16_t source)
{
    if (vertexBindings.find(source) != vertexBindings.end())
        return vertexBindings[source].get();
    return 0;
}

VertexElement *VertexData::GetVertexElement(VertexElement::Semantic semantic, uint16_t index)
{
    for(auto & element : vertexElements)
    {
        if (element.semantic == semantic && element.index == index)
            return &element;
    }
    return 0;
}

// VertexDataXml

VertexDataXml::VertexDataXml()
{
}

bool VertexDataXml::HasPositions() const
{
    return !positions.empty();
}

bool VertexDataXml::HasNormals() const
{
    return !normals.empty();
}

bool VertexDataXml::HasTangents() const
{
    return !tangents.empty();
}

bool VertexDataXml::HasUvs() const
{
    return !uvs.empty();
}

size_t VertexDataXml::NumUvs() const
{
    return uvs.size();
}

// IndexData

IndexData::IndexData() :
    count(0),
    faceCount(0),
    is32bit(false)
{
}

IndexData::~IndexData()
{
    Reset();
}

void IndexData::Reset()
{
    // Release shared ptr memory stream.
    buffer.reset();
}

size_t IndexData::IndexSize() const
{
    return (is32bit ? sizeof(uint32_t) : sizeof(uint16_t));
}

size_t IndexData::FaceSize() const
{
    return IndexSize() * 3;
}

// Mesh

Mesh::Mesh() 
    : hasSkeletalAnimations(false)
    , skeleton(NULL)
    , sharedVertexData(NULL)
    , subMeshes()
    , animations()
    , poses()
{
}

Mesh::~Mesh()
{
    Reset();
}

void Mesh::Reset()
{
    OGRE_SAFE_DELETE(skeleton)
    OGRE_SAFE_DELETE(sharedVertexData)

    for(auto &mesh : subMeshes) {
        OGRE_SAFE_DELETE(mesh)
    }
    subMeshes.clear();
    for(auto &anim : animations) {
        OGRE_SAFE_DELETE(anim)
    }
    animations.clear();
    for(auto &pose : poses) {
        OGRE_SAFE_DELETE(pose)
    }
    poses.clear();
}

size_t Mesh::NumSubMeshes() const
{
    return subMeshes.size();
}

SubMesh *Mesh::GetSubMesh( size_t index ) const
{
    for ( size_t i = 0; i < subMeshes.size(); ++i ) {
        if ( subMeshes[ i ]->index == index ) {
            return subMeshes[ i ];
        }
    }
    return 0;
}

void Mesh::ConvertToAssimpScene(aiScene* dest)
{
    if ( nullptr == dest ) {
        return;
    }

    // Setup
    dest->mNumMeshes = static_cast<unsigned int>(NumSubMeshes());
    dest->mMeshes = new aiMesh*[dest->mNumMeshes];

    // Create root node
    dest->mRootNode = new aiNode();
    dest->mRootNode->mNumMeshes = dest->mNumMeshes;
    dest->mRootNode->mMeshes = new unsigned int[dest->mRootNode->mNumMeshes];

    // Export meshes
    for(size_t i=0; i<dest->mNumMeshes; ++i) {
        dest->mMeshes[i] = subMeshes[i]->ConvertToAssimpMesh(this);
        dest->mRootNode->mMeshes[i] = static_cast<unsigned int>(i);
    }

    // Export skeleton
    if (skeleton)
    {
        // Bones
        if (!skeleton->bones.empty())
        {
            BoneList rootBones = skeleton->RootBones();
            dest->mRootNode->mNumChildren = static_cast<unsigned int>(rootBones.size());
            dest->mRootNode->mChildren = new aiNode*[dest->mRootNode->mNumChildren];

            for(size_t i=0, len=rootBones.size(); i<len; ++i)
            {
                dest->mRootNode->mChildren[i] = rootBones[i]->ConvertToAssimpNode(skeleton, dest->mRootNode);
            }
        }

        // Animations
        if (!skeleton->animations.empty())
        {
            dest->mNumAnimations = static_cast<unsigned int>(skeleton->animations.size());
            dest->mAnimations = new aiAnimation*[dest->mNumAnimations];

            for(size_t i=0, len=skeleton->animations.size(); i<len; ++i)
            {
                dest->mAnimations[i] = skeleton->animations[i]->ConvertToAssimpAnimation();
            }
        }
    }
}

// ISubMesh

ISubMesh::ISubMesh() :
    index(0),
    materialIndex(-1),
    usesSharedVertexData(false),
    operationType(OT_POINT_LIST)
{
}

// SubMesh

SubMesh::SubMesh() :
    vertexData(0),
    indexData(new IndexData())
{
}

SubMesh::~SubMesh()
{
    Reset();
}

void SubMesh::Reset()
{
    OGRE_SAFE_DELETE(vertexData)
    OGRE_SAFE_DELETE(indexData)
}

aiMesh *SubMesh::ConvertToAssimpMesh(Mesh *parent)
{
    if (operationType != OT_TRIANGLE_LIST) {
        throw DeadlyImportError(Formatter::format() << "Only mesh operation type OT_TRIANGLE_LIST is supported. Found " << operationType);
    }

    aiMesh *dest = new aiMesh();
    dest->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;

    if (!name.empty())
        dest->mName = name;

    // Material index
    if (materialIndex != -1)
        dest->mMaterialIndex = materialIndex;

    // Pick source vertex data from shader geometry or from internal geometry.
    VertexData *src = (!usesSharedVertexData ? vertexData : parent->sharedVertexData);

    VertexElement *positionsElement = src->GetVertexElement(VertexElement::VES_POSITION);
    VertexElement *normalsElement   = src->GetVertexElement(VertexElement::VES_NORMAL);
    VertexElement *uv1Element       = src->GetVertexElement(VertexElement::VES_TEXTURE_COORDINATES, 0);
    VertexElement *uv2Element       = src->GetVertexElement(VertexElement::VES_TEXTURE_COORDINATES, 1);

    // Sanity checks
    if (!positionsElement) {
        throw DeadlyImportError("Failed to import Ogre VertexElement::VES_POSITION. Mesh does not have vertex positions!");
    } else if (positionsElement->type != VertexElement::VET_FLOAT3) {
        throw DeadlyImportError("Ogre Mesh position vertex element type != VertexElement::VET_FLOAT3. This is not supported.");
    } else if (normalsElement && normalsElement->type != VertexElement::VET_FLOAT3) {
        throw DeadlyImportError("Ogre Mesh normal vertex element type != VertexElement::VET_FLOAT3. This is not supported.");
    }

    // Faces
    dest->mNumFaces = indexData->faceCount;
    dest->mFaces = new aiFace[dest->mNumFaces];

    // Assimp required unique vertices, we need to convert from Ogres shared indexing.
    size_t uniqueVertexCount = dest->mNumFaces * 3;
    dest->mNumVertices = static_cast<unsigned int>(uniqueVertexCount);
    dest->mVertices = new aiVector3D[dest->mNumVertices];

    // Source streams
    MemoryStream *positions      = src->VertexBuffer(positionsElement->source);
    MemoryStream *normals        = (normalsElement ? src->VertexBuffer(normalsElement->source) : 0);
    MemoryStream *uv1            = (uv1Element ? src->VertexBuffer(uv1Element->source) : 0);
    MemoryStream *uv2            = (uv2Element ? src->VertexBuffer(uv2Element->source) : 0);

    // Element size
    const size_t sizePosition    = positionsElement->Size();
    const size_t sizeNormal      = (normalsElement ? normalsElement->Size() : 0);
    const size_t sizeUv1         = (uv1Element ? uv1Element->Size() : 0);
    const size_t sizeUv2         = (uv2Element ? uv2Element->Size() : 0);

    // Vertex width
    const size_t vWidthPosition  = src->VertexSize(positionsElement->source);
    const size_t vWidthNormal    = (normalsElement ? src->VertexSize(normalsElement->source) : 0);
    const size_t vWidthUv1       = (uv1Element ? src->VertexSize(uv1Element->source) : 0);
    const size_t vWidthUv2       = (uv2Element ? src->VertexSize(uv2Element->source) : 0);

    bool boneAssignments = src->HasBoneAssignments();

    // Prepare normals
    if (normals)
        dest->mNormals = new aiVector3D[dest->mNumVertices];

    // Prepare UVs, ignoring incompatible UVs.
    if (uv1)
    {
        if (uv1Element->type == VertexElement::VET_FLOAT2 || uv1Element->type == VertexElement::VET_FLOAT3)
        {
            dest->mNumUVComponents[0] = static_cast<unsigned int>(uv1Element->ComponentCount());
            dest->mTextureCoords[0] = new aiVector3D[dest->mNumVertices];
        }
        else
        {
            DefaultLogger::get()->warn(Formatter::format() << "Ogre imported UV0 type " << uv1Element->TypeToString() << " is not compatible with Assimp. Ignoring UV.");
            uv1 = 0;
        }
    }
    if (uv2)
    {
        if (uv2Element->type == VertexElement::VET_FLOAT2 || uv2Element->type == VertexElement::VET_FLOAT3)
        {
            dest->mNumUVComponents[1] = static_cast<unsigned int>(uv2Element->ComponentCount());
            dest->mTextureCoords[1] = new aiVector3D[dest->mNumVertices];
        }
        else
        {
            DefaultLogger::get()->warn(Formatter::format() << "Ogre imported UV0 type " << uv2Element->TypeToString() << " is not compatible with Assimp. Ignoring UV.");
            uv2 = 0;
        }
    }

    aiVector3D *uv1Dest = (uv1 ? dest->mTextureCoords[0] : 0);
    aiVector3D *uv2Dest = (uv2 ? dest->mTextureCoords[1] : 0);

    MemoryStream *faces = indexData->buffer.get();
    for (size_t fi=0, isize=indexData->IndexSize(), fsize=indexData->FaceSize();
         fi<dest->mNumFaces; ++fi)
    {
        // Source Ogre face
        aiFace ogreFace;
        ogreFace.mNumIndices = 3;
        ogreFace.mIndices = new unsigned int[3];

        faces->Seek(fi * fsize, aiOrigin_SET);
        if (indexData->is32bit)
        {
            faces->Read(&ogreFace.mIndices[0], isize, 3);
        }
        else
        {
            uint16_t iout = 0;
            for (size_t ii=0; ii<3; ++ii)
            {
                faces->Read(&iout, isize, 1);
                ogreFace.mIndices[ii] = static_cast<unsigned int>(iout);
            }
        }

        // Destination Assimp face
        aiFace &face = dest->mFaces[fi];
        face.mNumIndices = 3;
        face.mIndices = new unsigned int[3];

        const size_t pos = fi * 3;
        for (size_t v=0; v<3; ++v)
        {
            const size_t newIndex = pos + v;

            // Write face index
            face.mIndices[v] = static_cast<unsigned int>(newIndex);

            // Ogres vertex index to ref into the source buffers.
            const size_t ogreVertexIndex = ogreFace.mIndices[v];
            src->AddVertexMapping(static_cast<uint32_t>(ogreVertexIndex), static_cast<uint32_t>(newIndex));

            // Position
            positions->Seek((vWidthPosition * ogreVertexIndex) + positionsElement->offset, aiOrigin_SET);
            positions->Read(&dest->mVertices[newIndex], sizePosition, 1);

            // Normal
            if (normals)
            {
                normals->Seek((vWidthNormal * ogreVertexIndex) + normalsElement->offset, aiOrigin_SET);
                normals->Read(&dest->mNormals[newIndex], sizeNormal, 1);
            }
            // UV0
            if (uv1 && uv1Dest)
            {
                uv1->Seek((vWidthUv1 * ogreVertexIndex) + uv1Element->offset, aiOrigin_SET);
                uv1->Read(&uv1Dest[newIndex], sizeUv1, 1);
                uv1Dest[newIndex].y = (uv1Dest[newIndex].y * -1) + 1; // Flip UV from Ogre to Assimp form
            }
            // UV1
            if (uv2 && uv2Dest)
            {
                uv2->Seek((vWidthUv2 * ogreVertexIndex) + uv2Element->offset, aiOrigin_SET);
                uv2->Read(&uv2Dest[newIndex], sizeUv2, 1);
                uv2Dest[newIndex].y = (uv2Dest[newIndex].y * -1) + 1; // Flip UV from Ogre to Assimp form
            }
        }
    }

    // Bones and bone weights
    if (parent->skeleton && boneAssignments)
    {
        AssimpVertexBoneWeightList weights = src->AssimpBoneWeights(dest->mNumVertices);
        std::set<uint16_t> referencedBones = src->ReferencedBonesByWeights();

        dest->mNumBones = static_cast<unsigned int>(referencedBones.size());
        dest->mBones = new aiBone*[dest->mNumBones];

        size_t assimpBoneIndex = 0;
        for(std::set<uint16_t>::const_iterator rbIter=referencedBones.begin(), rbEnd=referencedBones.end(); rbIter != rbEnd; ++rbIter, ++assimpBoneIndex)
        {
            Bone *bone = parent->skeleton->BoneById((*rbIter));
            dest->mBones[assimpBoneIndex] = bone->ConvertToAssimpBone(parent->skeleton, weights[bone->id]);
        }
    }

    return dest;
}

// MeshXml

MeshXml::MeshXml() :
    skeleton(0),
    sharedVertexData(0)
{
}

MeshXml::~MeshXml()
{
    Reset();
}

void MeshXml::Reset()
{
    OGRE_SAFE_DELETE(skeleton)
    OGRE_SAFE_DELETE(sharedVertexData)

    for(auto &mesh : subMeshes) {
        OGRE_SAFE_DELETE(mesh)
    }
    subMeshes.clear();
}

size_t MeshXml::NumSubMeshes() const
{
    return subMeshes.size();
}

SubMeshXml *MeshXml::GetSubMesh(uint16_t index) const
{
    for(size_t i=0; i<subMeshes.size(); ++i)
        if (subMeshes[i]->index == index)
            return subMeshes[i];
    return 0;
}

void MeshXml::ConvertToAssimpScene(aiScene* dest)
{
    // Setup
    dest->mNumMeshes = static_cast<unsigned int>(NumSubMeshes());
    dest->mMeshes = new aiMesh*[dest->mNumMeshes];

    // Create root node
    dest->mRootNode = new aiNode();
    dest->mRootNode->mNumMeshes = dest->mNumMeshes;
    dest->mRootNode->mMeshes = new unsigned int[dest->mRootNode->mNumMeshes];

    // Export meshes
    for(size_t i=0; i<dest->mNumMeshes; ++i)
    {
        dest->mMeshes[i] = subMeshes[i]->ConvertToAssimpMesh(this);
        dest->mRootNode->mMeshes[i] = static_cast<unsigned int>(i);
    }

    // Export skeleton
    if (skeleton)
    {
        // Bones
        if (!skeleton->bones.empty())
        {
            BoneList rootBones = skeleton->RootBones();
            dest->mRootNode->mNumChildren = static_cast<unsigned int>(rootBones.size());
            dest->mRootNode->mChildren = new aiNode*[dest->mRootNode->mNumChildren];

            for(size_t i=0, len=rootBones.size(); i<len; ++i)
            {
                dest->mRootNode->mChildren[i] = rootBones[i]->ConvertToAssimpNode(skeleton, dest->mRootNode);
            }
        }

        // Animations
        if (!skeleton->animations.empty())
        {
            dest->mNumAnimations = static_cast<unsigned int>(skeleton->animations.size());
            dest->mAnimations = new aiAnimation*[dest->mNumAnimations];

            for(size_t i=0, len=skeleton->animations.size(); i<len; ++i)
            {
                dest->mAnimations[i] = skeleton->animations[i]->ConvertToAssimpAnimation();
            }
        }
    }
}

// SubMeshXml

SubMeshXml::SubMeshXml() :
    indexData(new IndexDataXml()),
    vertexData(0)
{
}

SubMeshXml::~SubMeshXml()
{
    Reset();
}

void SubMeshXml::Reset()
{
    OGRE_SAFE_DELETE(indexData)
    OGRE_SAFE_DELETE(vertexData)
}

aiMesh *SubMeshXml::ConvertToAssimpMesh(MeshXml *parent)
{
    aiMesh *dest = new aiMesh();
    dest->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;

    if (!name.empty())
        dest->mName = name;

    // Material index
    if (materialIndex != -1)
        dest->mMaterialIndex = materialIndex;

    // Faces
    dest->mNumFaces = indexData->faceCount;
    dest->mFaces = new aiFace[dest->mNumFaces];

    // Assimp required unique vertices, we need to convert from Ogres shared indexing.
    size_t uniqueVertexCount = dest->mNumFaces * 3;
    dest->mNumVertices = static_cast<unsigned int>(uniqueVertexCount);
    dest->mVertices = new aiVector3D[dest->mNumVertices];

    VertexDataXml *src = (!usesSharedVertexData ? vertexData : parent->sharedVertexData);
    bool boneAssignments = src->HasBoneAssignments();
    bool normals = src->HasNormals();
    size_t uvs = src->NumUvs();

    // Prepare normals
    if (normals)
        dest->mNormals = new aiVector3D[dest->mNumVertices];

    // Prepare UVs
    for(size_t uvi=0; uvi<uvs; ++uvi)
    {
        dest->mNumUVComponents[uvi] = 2;
        dest->mTextureCoords[uvi] = new aiVector3D[dest->mNumVertices];
    }

    for (size_t fi=0; fi<dest->mNumFaces; ++fi)
    {
        // Source Ogre face
        aiFace &ogreFace = indexData->faces[fi];

        // Destination Assimp face
        aiFace &face = dest->mFaces[fi];
        face.mNumIndices = 3;
        face.mIndices = new unsigned int[3];

        const size_t pos = fi * 3;
        for (size_t v=0; v<3; ++v)
        {
            const size_t newIndex = pos + v;

            // Write face index
            face.mIndices[v] = static_cast<unsigned int>(newIndex);

            // Ogres vertex index to ref into the source buffers.
            const size_t ogreVertexIndex = ogreFace.mIndices[v];
            src->AddVertexMapping(static_cast<uint32_t>(ogreVertexIndex), static_cast<uint32_t>(newIndex));

            // Position
            dest->mVertices[newIndex] = src->positions[ogreVertexIndex];

            // Normal
            if (normals)
                dest->mNormals[newIndex] = src->normals[ogreVertexIndex];

            // UVs
            for(size_t uvi=0; uvi<uvs; ++uvi)
            {
                aiVector3D *uvDest = dest->mTextureCoords[uvi];
                std::vector<aiVector3D> &uvSrc = src->uvs[uvi];
                uvDest[newIndex] = uvSrc[ogreVertexIndex];
            }
        }
    }

    // Bones and bone weights
    if (parent->skeleton && boneAssignments)
    {
        AssimpVertexBoneWeightList weights = src->AssimpBoneWeights(dest->mNumVertices);
        std::set<uint16_t> referencedBones = src->ReferencedBonesByWeights();

        dest->mNumBones = static_cast<unsigned int>(referencedBones.size());
        dest->mBones = new aiBone*[dest->mNumBones];

        size_t assimpBoneIndex = 0;
        for(std::set<uint16_t>::const_iterator rbIter=referencedBones.begin(), rbEnd=referencedBones.end(); rbIter != rbEnd; ++rbIter, ++assimpBoneIndex)
        {
            Bone *bone = parent->skeleton->BoneById((*rbIter));
            dest->mBones[assimpBoneIndex] = bone->ConvertToAssimpBone(parent->skeleton, weights[bone->id]);
        }
    }

    return dest;
}

// Animation

Animation::Animation(Skeleton *parent) :
    parentMesh(NULL),
    parentSkeleton(parent),
    length(0.0f),
    baseTime(-1.0f)
{
}

Animation::Animation(Mesh *parent) :
    parentMesh(parent),
    parentSkeleton(0),
    length(0.0f),
    baseTime(-1.0f)
{
}

VertexData *Animation::AssociatedVertexData(VertexAnimationTrack *track) const
{
    if (!parentMesh)
        return 0;

    bool sharedGeom = (track->target == 0);
    if (sharedGeom)
        return parentMesh->sharedVertexData;
    else
        return parentMesh->GetSubMesh(track->target-1)->vertexData;
}

aiAnimation *Animation::ConvertToAssimpAnimation()
{
    aiAnimation *anim = new aiAnimation();
    anim->mName = name;
    anim->mDuration = static_cast<double>(length);
    anim->mTicksPerSecond = 1.0;

    // Tracks
    if (!tracks.empty())
    {
        anim->mNumChannels = static_cast<unsigned int>(tracks.size());
        anim->mChannels = new aiNodeAnim*[anim->mNumChannels];

        for(size_t i=0, len=tracks.size(); i<len; ++i)
        {
            anim->mChannels[i] = tracks[i].ConvertToAssimpAnimationNode(parentSkeleton);
        }
    }
    return anim;
}

// Skeleton

Skeleton::Skeleton() :
    bones(),
    animations(),
    blendMode(ANIMBLEND_AVERAGE)
{
}

Skeleton::~Skeleton()
{
    Reset();
}

void Skeleton::Reset()
{
    for(auto &bone : bones) {
        OGRE_SAFE_DELETE(bone)
    }
    bones.clear();
    for(auto &anim : animations) {
        OGRE_SAFE_DELETE(anim)
    }
    animations.clear();
}

BoneList Skeleton::RootBones() const
{
    BoneList rootBones;
    for(BoneList::const_iterator iter = bones.begin(); iter != bones.end(); ++iter)
    {
        if (!(*iter)->IsParented())
            rootBones.push_back((*iter));
    }
    return rootBones;
}

size_t Skeleton::NumRootBones() const
{
    size_t num = 0;
    for(BoneList::const_iterator iter = bones.begin(); iter != bones.end(); ++iter)
    {
        if (!(*iter)->IsParented())
            num++;
    }
    return num;
}

Bone *Skeleton::BoneByName(const std::string &name) const
{
    for(BoneList::const_iterator iter = bones.begin(); iter != bones.end(); ++iter)
    {
        if ((*iter)->name == name)
            return (*iter);
    }
    return 0;
}

Bone *Skeleton::BoneById(uint16_t id) const
{
    for(BoneList::const_iterator iter = bones.begin(); iter != bones.end(); ++iter)
    {
        if ((*iter)->id == id)
            return (*iter);
    }
    return 0;
}

// Bone

Bone::Bone() :
    id(0),
    parent(0),
    parentId(-1),
    scale(1.0f, 1.0f, 1.0f)
{
}

bool Bone::IsParented() const
{
    return (parentId != -1 && parent != 0);
}

uint16_t Bone::ParentId() const
{
    return static_cast<uint16_t>(parentId);
}

void Bone::AddChild(Bone *bone)
{
    if (!bone)
        return;
    if (bone->IsParented())
        throw DeadlyImportError("Attaching child Bone that is already parented: " + bone->name);

    bone->parent = this;
    bone->parentId = id;
    children.push_back(bone->id);
}

void Bone::CalculateWorldMatrixAndDefaultPose(Skeleton *skeleton)
{
    if (!IsParented())
        worldMatrix = aiMatrix4x4(scale, rotation, position).Inverse();
    else
        worldMatrix = aiMatrix4x4(scale, rotation, position).Inverse() * parent->worldMatrix;

    defaultPose = aiMatrix4x4(scale, rotation, position);

    // Recursively for all children now that the parent matrix has been calculated.
    for (auto boneId : children)
    {
        Bone *child = skeleton->BoneById(boneId);
        if (!child) {
            throw DeadlyImportError(Formatter::format() << "CalculateWorldMatrixAndDefaultPose: Failed to find child bone " << boneId << " for parent " << id << " " << name);
        }
        child->CalculateWorldMatrixAndDefaultPose(skeleton);
    }
}

aiNode *Bone::ConvertToAssimpNode(Skeleton *skeleton, aiNode *parentNode)
{
    // Bone node
    aiNode* node = new aiNode(name);
    node->mParent = parentNode;
    node->mTransformation = defaultPose;

    // Children
    if (!children.empty())
    {
        node->mNumChildren = static_cast<unsigned int>(children.size());
        node->mChildren = new aiNode*[node->mNumChildren];

        for(size_t i=0, len=children.size(); i<len; ++i)
        {
            Bone *child = skeleton->BoneById(children[i]);
            if (!child) {
                throw DeadlyImportError(Formatter::format() << "ConvertToAssimpNode: Failed to find child bone " << children[i] << " for parent " << id << " " << name);
            }
            node->mChildren[i] = child->ConvertToAssimpNode(skeleton, node);
        }
    }
    return node;
}

aiBone *Bone::ConvertToAssimpBone(Skeleton * /*parent*/, const std::vector<aiVertexWeight> &boneWeights)
{
    aiBone *bone = new aiBone();
    bone->mName = name;
    bone->mOffsetMatrix = worldMatrix;

    if (!boneWeights.empty())
    {
        bone->mNumWeights = static_cast<unsigned int>(boneWeights.size());
        bone->mWeights = new aiVertexWeight[boneWeights.size()];
        memcpy(bone->mWeights, &boneWeights[0], boneWeights.size() * sizeof(aiVertexWeight));
    }

    return bone;
}

// VertexAnimationTrack

VertexAnimationTrack::VertexAnimationTrack() :
    type(VAT_NONE),
    target(0)
{
}

aiNodeAnim *VertexAnimationTrack::ConvertToAssimpAnimationNode(Skeleton *skeleton)
{
    if (boneName.empty() || type != VAT_TRANSFORM) {
        throw DeadlyImportError("VertexAnimationTrack::ConvertToAssimpAnimationNode: Cannot convert track that has no target bone name or is not type of VAT_TRANSFORM");
    }

    aiNodeAnim *nodeAnim = new aiNodeAnim();
    nodeAnim->mNodeName = boneName;

    Bone *bone = skeleton->BoneByName(boneName);
    if (!bone) {
        throw DeadlyImportError("VertexAnimationTrack::ConvertToAssimpAnimationNode: Failed to find bone " + boneName + " from parent Skeleton");
    }

    // Keyframes
    size_t numKeyframes = transformKeyFrames.size();

    nodeAnim->mPositionKeys = new aiVectorKey[numKeyframes];
    nodeAnim->mRotationKeys = new aiQuatKey[numKeyframes];
    nodeAnim->mScalingKeys = new aiVectorKey[numKeyframes];
    nodeAnim->mNumPositionKeys = static_cast<unsigned int>(numKeyframes);
    nodeAnim->mNumRotationKeys = static_cast<unsigned int>(numKeyframes);
    nodeAnim->mNumScalingKeys  = static_cast<unsigned int>(numKeyframes);

    for(size_t kfi=0; kfi<numKeyframes; ++kfi)
    {
        TransformKeyFrame &kfSource = transformKeyFrames[kfi];

        // Calculate the complete transformation from world space to bone space
        aiVector3D pos; aiQuaternion rot; aiVector3D scale;

        aiMatrix4x4 finalTransform = bone->defaultPose * kfSource.Transform();
        finalTransform.Decompose(scale, rot, pos);

        double t = static_cast<double>(kfSource.timePos);
        nodeAnim->mPositionKeys[kfi].mTime = t;
        nodeAnim->mRotationKeys[kfi].mTime = t;
        nodeAnim->mScalingKeys[kfi].mTime = t;

        nodeAnim->mPositionKeys[kfi].mValue = pos;
        nodeAnim->mRotationKeys[kfi].mValue = rot;
        nodeAnim->mScalingKeys[kfi].mValue = scale;
    }

    return nodeAnim;
}

// TransformKeyFrame

TransformKeyFrame::TransformKeyFrame() :
    timePos(0.0f),
    scale(1.0f, 1.0f, 1.0f)
{
}

aiMatrix4x4 TransformKeyFrame::Transform()
{
    return aiMatrix4x4(scale, rotation, position);
}

} // Ogre
} // Assimp

#endif // ASSIMP_BUILD_NO_OGRE_IMPORTER
