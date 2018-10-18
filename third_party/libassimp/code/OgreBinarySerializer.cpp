/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2018, assimp team


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

#include "OgreBinarySerializer.h"
#include "OgreXmlSerializer.h"
#include "OgreParsingUtils.h"

#include <assimp/TinyFormatter.h>
#include <assimp/DefaultLogger.hpp>


#ifndef ASSIMP_BUILD_NO_OGRE_IMPORTER

// Define as 1 to get verbose logging.
#define OGRE_BINARY_SERIALIZER_DEBUG 0

namespace Assimp
{
namespace Ogre
{

const std::string       MESH_VERSION_1_8        = "[MeshSerializer_v1.8]";
const std::string       SKELETON_VERSION_1_8    = "[Serializer_v1.80]";
const std::string       SKELETON_VERSION_1_1    = "[Serializer_v1.10]";

const unsigned short    HEADER_CHUNK_ID         = 0x1000;

const long              MSTREAM_OVERHEAD_SIZE               = sizeof(uint16_t) + sizeof(uint32_t);
const long              MSTREAM_BONE_SIZE_WITHOUT_SCALE     = MSTREAM_OVERHEAD_SIZE + sizeof(unsigned short) + (sizeof(float) * 7);
const long              MSTREAM_KEYFRAME_SIZE_WITHOUT_SCALE = MSTREAM_OVERHEAD_SIZE + (sizeof(float) * 8);

template<>
inline bool OgreBinarySerializer::Read<bool>()
{
    return (m_reader->GetU1() > 0);
}

template<>
inline char OgreBinarySerializer::Read<char>()
{
    return static_cast<char>(m_reader->GetU1());
}

template<>
inline uint8_t OgreBinarySerializer::Read<uint8_t>()
{
    return m_reader->GetU1();
}

template<>
inline uint16_t OgreBinarySerializer::Read<uint16_t>()
{
    return m_reader->GetU2();
}

template<>
inline uint32_t OgreBinarySerializer::Read<uint32_t>()
{
    return m_reader->GetU4();
}

template<>
inline float OgreBinarySerializer::Read<float>()
{
    return m_reader->GetF4();
}

void OgreBinarySerializer::ReadBytes(char *dest, size_t numBytes)
{
    ReadBytes(static_cast<void*>(dest), numBytes);
}

void OgreBinarySerializer::ReadBytes(uint8_t *dest, size_t numBytes)
{
    ReadBytes(static_cast<void*>(dest), numBytes);
}

void OgreBinarySerializer::ReadBytes(void *dest, size_t numBytes)
{
    m_reader->CopyAndAdvance(dest, numBytes);
}

uint8_t *OgreBinarySerializer::ReadBytes(size_t numBytes)
{
    uint8_t *bytes = new uint8_t[numBytes];
    ReadBytes(bytes, numBytes);
    return bytes;
}

void OgreBinarySerializer::ReadVector(aiVector3D &vec)
{
    m_reader->CopyAndAdvance(&vec.x, sizeof(float)*3);
}

void OgreBinarySerializer::ReadQuaternion(aiQuaternion &quat)
{
    float temp[4];
    m_reader->CopyAndAdvance(temp, sizeof(float)*4);
    quat.x = temp[0];
    quat.y = temp[1];
    quat.z = temp[2];
    quat.w = temp[3];
}

bool OgreBinarySerializer::AtEnd() const
{
    return (m_reader->GetRemainingSize() == 0);
}

std::string OgreBinarySerializer::ReadString(size_t len)
{
    std::string str;
    str.resize(len);
    ReadBytes(&str[0], len);
    return str;
}

std::string OgreBinarySerializer::ReadLine()
{
    std::string str;
    while(!AtEnd())
    {
        char c = Read<char>();
        if (c == '\n')
            break;
        str += c;
    }
    return str;
}

uint16_t OgreBinarySerializer::ReadHeader(bool readLen)
{
    uint16_t id = Read<uint16_t>();
    if (readLen)
        m_currentLen = Read<uint32_t>();

#if (OGRE_BINARY_SERIALIZER_DEBUG == 1)
    if (id != HEADER_CHUNK_ID)
    {
        ASSIMP_LOG_DEBUG(Formatter::format() << (assetMode == AM_Mesh
            ? MeshHeaderToString(static_cast<MeshChunkId>(id)) : SkeletonHeaderToString(static_cast<SkeletonChunkId>(id))));
    }
#endif

    return id;
}

void OgreBinarySerializer::RollbackHeader()
{
    m_reader->IncPtr(-MSTREAM_OVERHEAD_SIZE);
}

void OgreBinarySerializer::SkipBytes(size_t numBytes)
{
#if (OGRE_BINARY_SERIALIZER_DEBUG == 1)
    ASSIMP_LOG_DEBUG_F( "Skipping ", numBytes, " bytes");
#endif

    m_reader->IncPtr(numBytes);
}

// Mesh

Mesh *OgreBinarySerializer::ImportMesh(MemoryStreamReader *stream)
{
    OgreBinarySerializer serializer(stream, OgreBinarySerializer::AM_Mesh);

    uint16_t id = serializer.ReadHeader(false);
    if (id != HEADER_CHUNK_ID) {
        throw DeadlyExportError("Invalid Ogre Mesh file header.");
    }

    /// @todo Check what we can actually support.
    std::string version = serializer.ReadLine();
    if (version != MESH_VERSION_1_8)
    {
        throw DeadlyExportError(Formatter::format() << "Mesh version " << version << " not supported by this importer. Run OgreMeshUpgrader tool on the file and try again."
            << " Supported versions: " << MESH_VERSION_1_8);
    }

    Mesh *mesh = new Mesh();
    while (!serializer.AtEnd())
    {
        id = serializer.ReadHeader();
        switch(id)
        {
            case M_MESH:
            {
                serializer.ReadMesh(mesh);
                break;
            }
        }
    }
    return mesh;
}

void OgreBinarySerializer::ReadMesh(Mesh *mesh)
{
    mesh->hasSkeletalAnimations = Read<bool>();

    ASSIMP_LOG_DEBUG("Reading Mesh");
    ASSIMP_LOG_DEBUG_F( "  - Skeletal animations: ", mesh->hasSkeletalAnimations ? "true" : "false" );

    if (!AtEnd())
    {
        uint16_t id = ReadHeader();
        while (!AtEnd() &&
            (id == M_GEOMETRY ||
             id == M_SUBMESH ||
             id == M_MESH_SKELETON_LINK ||
             id == M_MESH_BONE_ASSIGNMENT ||
             id == M_MESH_LOD ||
             id == M_MESH_BOUNDS ||
             id == M_SUBMESH_NAME_TABLE ||
             id == M_EDGE_LISTS ||
             id == M_POSES ||
             id == M_ANIMATIONS ||
             id == M_TABLE_EXTREMES))
        {
            switch(id)
            {
                case M_GEOMETRY:
                {
                    mesh->sharedVertexData = new VertexData();
                    ReadGeometry(mesh->sharedVertexData);
                    break;
                }
                case M_SUBMESH:
                {
                    ReadSubMesh(mesh);
                    break;
                }
                case M_MESH_SKELETON_LINK:
                {
                    ReadMeshSkeletonLink(mesh);
                    break;
                }
                case M_MESH_BONE_ASSIGNMENT:
                {
                    ReadBoneAssignment(mesh->sharedVertexData);
                    break;
                }
                case M_MESH_LOD:
                {
                    ReadMeshLodInfo(mesh);
                    break;
                }
                case M_MESH_BOUNDS:
                {
                    ReadMeshBounds(mesh);
                    break;
                }
                case M_SUBMESH_NAME_TABLE:
                {
                    ReadSubMeshNames(mesh);
                    break;
                }
                case M_EDGE_LISTS:
                {
                    ReadEdgeList(mesh);
                    break;
                }
                case M_POSES:
                {
                    ReadPoses(mesh);
                    break;
                }
                case M_ANIMATIONS:
                {
                    ReadAnimations(mesh);
                    break;
                }
                case M_TABLE_EXTREMES:
                {
                    ReadMeshExtremes(mesh);
                    break;
                }
            }

            if (!AtEnd())
                id = ReadHeader();
        }
        if (!AtEnd())
            RollbackHeader();
    }

    NormalizeBoneWeights(mesh->sharedVertexData);
}

void OgreBinarySerializer::ReadMeshLodInfo(Mesh *mesh)
{
    // Assimp does not acknowledge LOD levels as far as I can see it. This info is just skipped.
    // @todo Put this stuff to scene/mesh custom properties. If manual mesh the app can use the information.
    ReadLine(); // strategy name
    uint16_t numLods = Read<uint16_t>();
    bool manual = Read<bool>();

    /// @note Main mesh is considered as LOD 0, start from index 1.
    for (size_t i=1; i<numLods; ++i)
    {
        uint16_t id = ReadHeader();
        if (id != M_MESH_LOD_USAGE) {
            throw DeadlyImportError("M_MESH_LOD does not contain a M_MESH_LOD_USAGE for each LOD level");
        }

        m_reader->IncPtr(sizeof(float)); // user value

        if (manual)
        {
            id = ReadHeader();
            if (id != M_MESH_LOD_MANUAL) {
                throw DeadlyImportError("Manual M_MESH_LOD_USAGE does not contain M_MESH_LOD_MANUAL");
            }

            ReadLine(); // manual mesh name (ref to another mesh)
        }
        else
        {
            for(size_t si=0, silen=mesh->NumSubMeshes(); si<silen; ++si)
            {
                id = ReadHeader();
                if (id != M_MESH_LOD_GENERATED) {
                    throw DeadlyImportError("Generated M_MESH_LOD_USAGE does not contain M_MESH_LOD_GENERATED");
                }

                uint32_t indexCount = Read<uint32_t>();
                bool is32bit = Read<bool>();

                if (indexCount > 0)
                {
                    uint32_t len = indexCount * (is32bit ? sizeof(uint32_t) : sizeof(uint16_t));
                    m_reader->IncPtr(len);
                }
            }
        }
    }
}

void OgreBinarySerializer::ReadMeshSkeletonLink(Mesh *mesh)
{
    mesh->skeletonRef = ReadLine();
}

void OgreBinarySerializer::ReadMeshBounds(Mesh * /*mesh*/)
{
    // Skip bounds, not compatible with Assimp.
    // 2x float vec3 + 1x float sphere radius
    SkipBytes(sizeof(float) * 7);
}

void OgreBinarySerializer::ReadMeshExtremes(Mesh * /*mesh*/)
{
    // Skip extremes, not compatible with Assimp.
    size_t numBytes = m_currentLen - MSTREAM_OVERHEAD_SIZE;
    SkipBytes(numBytes);
}

void OgreBinarySerializer::ReadBoneAssignment(VertexData *dest)
{
    if (!dest) {
        throw DeadlyImportError("Cannot read bone assignments, vertex data is null.");
    }

    VertexBoneAssignment ba;
    ba.vertexIndex = Read<uint32_t>();
    ba.boneIndex = Read<uint16_t>();
    ba.weight = Read<float>();

    dest->boneAssignments.push_back(ba);
}

void OgreBinarySerializer::ReadSubMesh(Mesh *mesh)
{
    uint16_t id = 0;

    SubMesh *submesh = new SubMesh();
    submesh->materialRef = ReadLine();
    submesh->usesSharedVertexData = Read<bool>();

    submesh->indexData->count = Read<uint32_t>();
    submesh->indexData->faceCount = static_cast<uint32_t>(submesh->indexData->count / 3);
    submesh->indexData->is32bit = Read<bool>();

    ASSIMP_LOG_DEBUG_F( "Reading SubMesh ", mesh->subMeshes.size());
    ASSIMP_LOG_DEBUG_F( "  - Material: '", submesh->materialRef, "'");
    ASSIMP_LOG_DEBUG_F( "  - Uses shared geometry: ", submesh->usesSharedVertexData ? "true" : "false" );

    // Index buffer
    if (submesh->indexData->count > 0)
    {
        uint32_t numBytes = submesh->indexData->count * (submesh->indexData->is32bit ? sizeof(uint32_t) : sizeof(uint16_t));
        uint8_t *indexBuffer = ReadBytes(numBytes);
        submesh->indexData->buffer = MemoryStreamPtr(new Assimp::MemoryIOStream(indexBuffer, numBytes, true));

        ASSIMP_LOG_DEBUG_F( "  - ", submesh->indexData->faceCount,
            " faces from ", submesh->indexData->count, (submesh->indexData->is32bit ? " 32bit" : " 16bit"),
            " indexes of ", numBytes, " bytes");
    }

    // Vertex buffer if not referencing the shared geometry
    if (!submesh->usesSharedVertexData)
    {
        id = ReadHeader();
        if (id != M_GEOMETRY) {
            throw DeadlyImportError("M_SUBMESH does not contain M_GEOMETRY, but shader geometry is set to false");
        }

        submesh->vertexData = new VertexData();
        ReadGeometry(submesh->vertexData);
    }

    // Bone assignment, submesh operation and texture aliases
    if (!AtEnd())
    {
        id = ReadHeader();
        while (!AtEnd() &&
            (id == M_SUBMESH_OPERATION ||
             id == M_SUBMESH_BONE_ASSIGNMENT ||
             id == M_SUBMESH_TEXTURE_ALIAS))
        {
            switch(id)
            {
                case M_SUBMESH_OPERATION:
                {
                    ReadSubMeshOperation(submesh);
                    break;
                }
                case M_SUBMESH_BONE_ASSIGNMENT:
                {
                    ReadBoneAssignment(submesh->vertexData);
                    break;
                }
                case M_SUBMESH_TEXTURE_ALIAS:
                {
                    ReadSubMeshTextureAlias(submesh);
                    break;
                }
            }

            if (!AtEnd())
                id = ReadHeader();
        }
        if (!AtEnd())
            RollbackHeader();
    }

    NormalizeBoneWeights(submesh->vertexData);

    submesh->index = static_cast<unsigned int>(mesh->subMeshes.size());
    mesh->subMeshes.push_back(submesh);
}

void OgreBinarySerializer::NormalizeBoneWeights(VertexData *vertexData) const
{
    if (!vertexData || vertexData->boneAssignments.empty())
        return;

    std::set<uint32_t> influencedVertices;
    for (VertexBoneAssignmentList::const_iterator baIter=vertexData->boneAssignments.begin(), baEnd=vertexData->boneAssignments.end(); baIter != baEnd; ++baIter) {
        influencedVertices.insert(baIter->vertexIndex);
    }

    /** Normalize bone weights.
        Some exporters won't care if the sum of all bone weights
        for a single vertex equals 1 or not, so validate here. */
    const float epsilon = 0.05f;
    for (const uint32_t vertexIndex : influencedVertices)
    {
        float sum = 0.0f;
        for (VertexBoneAssignmentList::const_iterator baIter=vertexData->boneAssignments.begin(), baEnd=vertexData->boneAssignments.end(); baIter != baEnd; ++baIter)
        {
            if (baIter->vertexIndex == vertexIndex)
                sum += baIter->weight;
        }
        if ((sum < (1.0f - epsilon)) || (sum > (1.0f + epsilon)))
        {
            for (auto &boneAssign : vertexData->boneAssignments)
            {
                if (boneAssign.vertexIndex == vertexIndex)
                    boneAssign.weight /= sum;
            }
        }
    }
}

void OgreBinarySerializer::ReadSubMeshOperation(SubMesh *submesh)
{
    submesh->operationType = static_cast<SubMesh::OperationType>(Read<uint16_t>());
}

void OgreBinarySerializer::ReadSubMeshTextureAlias(SubMesh *submesh)
{
    submesh->textureAliasName = ReadLine();
    submesh->textureAliasRef = ReadLine();
}

void OgreBinarySerializer::ReadSubMeshNames(Mesh *mesh)
{
    uint16_t id = 0;

    if (!AtEnd())
    {
        id = ReadHeader();
        while (!AtEnd() && id == M_SUBMESH_NAME_TABLE_ELEMENT)
        {
            uint16_t submeshIndex = Read<uint16_t>();
            SubMesh *submesh = mesh->GetSubMesh(submeshIndex);
            if (!submesh) {
                throw DeadlyImportError(Formatter::format() << "Ogre Mesh does not include submesh " << submeshIndex << " referenced in M_SUBMESH_NAME_TABLE_ELEMENT. Invalid mesh file.");
            }

            submesh->name = ReadLine();
            ASSIMP_LOG_DEBUG_F( "  - SubMesh ", submesh->index, " name '", submesh->name, "'");

            if (!AtEnd())
                id = ReadHeader();
        }
        if (!AtEnd())
            RollbackHeader();
    }
}

void OgreBinarySerializer::ReadGeometry(VertexData *dest)
{
    dest->count = Read<uint32_t>();

    ASSIMP_LOG_DEBUG_F( "  - Reading geometry of ", dest->count, " vertices");

    if (!AtEnd())
    {
        uint16_t id = ReadHeader();
        while (!AtEnd() &&
            (id == M_GEOMETRY_VERTEX_DECLARATION ||
             id == M_GEOMETRY_VERTEX_BUFFER))
        {
            switch(id)
            {
                case M_GEOMETRY_VERTEX_DECLARATION:
                {
                    ReadGeometryVertexDeclaration(dest);
                    break;
                }
                case M_GEOMETRY_VERTEX_BUFFER:
                {
                    ReadGeometryVertexBuffer(dest);
                    break;
                }
            }

            if (!AtEnd())
                id = ReadHeader();
        }
        if (!AtEnd())
            RollbackHeader();
    }
}

void OgreBinarySerializer::ReadGeometryVertexDeclaration(VertexData *dest)
{
    if (!AtEnd())
    {
        uint16_t id = ReadHeader();
        while (!AtEnd() && id == M_GEOMETRY_VERTEX_ELEMENT)
        {
            ReadGeometryVertexElement(dest);

            if (!AtEnd())
                id = ReadHeader();
        }
        if (!AtEnd())
            RollbackHeader();
    }
}

void OgreBinarySerializer::ReadGeometryVertexElement(VertexData *dest)
{
    VertexElement element;
    element.source = Read<uint16_t>();
    element.type = static_cast<VertexElement::Type>(Read<uint16_t>());
    element.semantic = static_cast<VertexElement::Semantic>(Read<uint16_t>());
    element.offset = Read<uint16_t>();
    element.index = Read<uint16_t>();

    ASSIMP_LOG_DEBUG_F( "    - Vertex element ", element.SemanticToString(), " of type ",
        element.TypeToString(), " index=", element.index, " source=", element.source);

    dest->vertexElements.push_back(element);
}

void OgreBinarySerializer::ReadGeometryVertexBuffer(VertexData *dest)
{
    uint16_t bindIndex = Read<uint16_t>();
    uint16_t vertexSize = Read<uint16_t>();

    uint16_t id = ReadHeader();
    if (id != M_GEOMETRY_VERTEX_BUFFER_DATA)
        throw DeadlyImportError("M_GEOMETRY_VERTEX_BUFFER_DATA not found in M_GEOMETRY_VERTEX_BUFFER");

    if (dest->VertexSize(bindIndex) != vertexSize)
        throw DeadlyImportError("Vertex buffer size does not agree with vertex declaration in M_GEOMETRY_VERTEX_BUFFER");

    size_t numBytes = dest->count * vertexSize;
    uint8_t *vertexBuffer = ReadBytes(numBytes);
    dest->vertexBindings[bindIndex] = MemoryStreamPtr(new Assimp::MemoryIOStream(vertexBuffer, numBytes, true));

    ASSIMP_LOG_DEBUG_F( "    - Read vertex buffer for source ", bindIndex, " of ", numBytes, " bytes");
}

void OgreBinarySerializer::ReadEdgeList(Mesh * /*mesh*/)
{
    // Assimp does not acknowledge LOD levels as far as I can see it. This info is just skipped.

    if (!AtEnd())
    {
        uint16_t id = ReadHeader();
        while (!AtEnd() && id == M_EDGE_LIST_LOD)
        {
            m_reader->IncPtr(sizeof(uint16_t)); // lod index
            bool manual = Read<bool>();

            if (!manual)
            {
                m_reader->IncPtr(sizeof(uint8_t));
                uint32_t numTriangles = Read<uint32_t>();
                uint32_t numEdgeGroups = Read<uint32_t>();

                size_t skipBytes = (sizeof(uint32_t) * 8 + sizeof(float) * 4) * numTriangles;
                m_reader->IncPtr(skipBytes);

                for (size_t i=0; i<numEdgeGroups; ++i)
                {
                    uint16_t id = ReadHeader();
                    if (id != M_EDGE_GROUP)
                        throw DeadlyImportError("M_EDGE_GROUP not found in M_EDGE_LIST_LOD");

                    m_reader->IncPtr(sizeof(uint32_t) * 3);
                    uint32_t numEdges = Read<uint32_t>();
                    for (size_t j=0; j<numEdges; ++j)
                    {
                        m_reader->IncPtr(sizeof(uint32_t) * 6 + sizeof(uint8_t));
                    }
                }
            }

            if (!AtEnd())
                id = ReadHeader();
        }
        if (!AtEnd())
            RollbackHeader();
    }
}

void OgreBinarySerializer::ReadPoses(Mesh *mesh)
{
    if (!AtEnd())
    {
        uint16_t id = ReadHeader();
        while (!AtEnd() && id == M_POSE)
        {
            Pose *pose = new Pose();
            pose->name = ReadLine();
            pose->target = Read<uint16_t>();
            pose->hasNormals = Read<bool>();

            ReadPoseVertices(pose);

            mesh->poses.push_back(pose);

            if (!AtEnd())
                id = ReadHeader();
        }
        if (!AtEnd())
            RollbackHeader();
    }
}

void OgreBinarySerializer::ReadPoseVertices(Pose *pose)
{
    if (!AtEnd())
    {
        uint16_t id = ReadHeader();
        while (!AtEnd() && id == M_POSE_VERTEX)
        {
            Pose::Vertex v;
            v.index = Read<uint32_t>();
            ReadVector(v.offset);
            if (pose->hasNormals)
                ReadVector(v.normal);

            pose->vertices[v.index] = v;

            if (!AtEnd())
                id = ReadHeader();
        }
        if (!AtEnd())
            RollbackHeader();
    }
}

void OgreBinarySerializer::ReadAnimations(Mesh *mesh)
{
    if (!AtEnd())
    {
        uint16_t id = ReadHeader();
        while (!AtEnd() && id == M_ANIMATION)
        {
            Animation *anim = new Animation(mesh);
            anim->name = ReadLine();
            anim->length = Read<float>();

            ReadAnimation(anim);

            mesh->animations.push_back(anim);

            if (!AtEnd())
                id = ReadHeader();
        }
        if (!AtEnd())
            RollbackHeader();
    }
}

void OgreBinarySerializer::ReadAnimation(Animation *anim)
{
    if (!AtEnd())
    {
        uint16_t id = ReadHeader();
        if (id == M_ANIMATION_BASEINFO)
        {
            anim->baseName = ReadLine();
            anim->baseTime = Read<float>();

            // Advance to first track
            id = ReadHeader();
        }

        while (!AtEnd() && id == M_ANIMATION_TRACK)
        {
            VertexAnimationTrack track;
            track.type = static_cast<VertexAnimationTrack::Type>(Read<uint16_t>());
            track.target = Read<uint16_t>();

            ReadAnimationKeyFrames(anim, &track);

            anim->tracks.push_back(track);

            if (!AtEnd())
                id = ReadHeader();
        }
        if (!AtEnd())
            RollbackHeader();
    }
}

void OgreBinarySerializer::ReadAnimationKeyFrames(Animation *anim, VertexAnimationTrack *track)
{
    if (!AtEnd())
    {
        uint16_t id = ReadHeader();
        while (!AtEnd() &&
            (id == M_ANIMATION_MORPH_KEYFRAME ||
             id == M_ANIMATION_POSE_KEYFRAME))
        {
            if (id == M_ANIMATION_MORPH_KEYFRAME)
            {
                MorphKeyFrame kf;
                kf.timePos = Read<float>();
                bool hasNormals = Read<bool>();

                size_t vertexCount = anim->AssociatedVertexData(track)->count;
                size_t vertexSize = sizeof(float) * (hasNormals ? 6 : 3);
                size_t numBytes = vertexCount * vertexSize;

                uint8_t *morphBuffer = ReadBytes(numBytes);
                kf.buffer = MemoryStreamPtr(new Assimp::MemoryIOStream(morphBuffer, numBytes, true));

                track->morphKeyFrames.push_back(kf);
            }
            else if (id == M_ANIMATION_POSE_KEYFRAME)
            {
                PoseKeyFrame kf;
                kf.timePos = Read<float>();

                if (!AtEnd())
                {
                    id = ReadHeader();
                    while (!AtEnd() && id == M_ANIMATION_POSE_REF)
                    {
                        PoseRef pr;
                        pr.index = Read<uint16_t>();
                        pr.influence = Read<float>();
                        kf.references.push_back(pr);

                        if (!AtEnd())
                            id = ReadHeader();
                    }
                    if (!AtEnd())
                        RollbackHeader();
                }

                track->poseKeyFrames.push_back(kf);
            }

            if (!AtEnd())
                id = ReadHeader();
        }
        if (!AtEnd())
            RollbackHeader();
    }
}

// Skeleton

bool OgreBinarySerializer::ImportSkeleton(Assimp::IOSystem *pIOHandler, Mesh *mesh)
{
    if (!mesh || mesh->skeletonRef.empty())
        return false;

    // Highly unusual to see in read world cases but support
    // binary mesh referencing a XML skeleton file.
    if (EndsWith(mesh->skeletonRef, ".skeleton.xml", false))
    {
        OgreXmlSerializer::ImportSkeleton(pIOHandler, mesh);
        return false;
    }

    MemoryStreamReaderPtr reader = OpenReader(pIOHandler, mesh->skeletonRef);

    Skeleton *skeleton = new Skeleton();
    OgreBinarySerializer serializer(reader.get(), OgreBinarySerializer::AM_Skeleton);
    serializer.ReadSkeleton(skeleton);
    mesh->skeleton = skeleton;
    return true;
}

bool OgreBinarySerializer::ImportSkeleton(Assimp::IOSystem *pIOHandler, MeshXml *mesh)
{
    if (!mesh || mesh->skeletonRef.empty())
        return false;

    MemoryStreamReaderPtr reader = OpenReader(pIOHandler, mesh->skeletonRef);
    if (!reader.get())
        return false;

    Skeleton *skeleton = new Skeleton();
    OgreBinarySerializer serializer(reader.get(), OgreBinarySerializer::AM_Skeleton);
    serializer.ReadSkeleton(skeleton);
    mesh->skeleton = skeleton;
    return true;
}

MemoryStreamReaderPtr OgreBinarySerializer::OpenReader(Assimp::IOSystem *pIOHandler, const std::string &filename)
{
    if (!EndsWith(filename, ".skeleton", false))
    {
        ASSIMP_LOG_ERROR_F("Imported Mesh is referencing to unsupported '", filename, "' skeleton file.");
        return MemoryStreamReaderPtr();
    }

    if (!pIOHandler->Exists(filename))
    {
        ASSIMP_LOG_ERROR_F("Failed to find skeleton file '", filename, "' that is referenced by imported Mesh.");
        return MemoryStreamReaderPtr();
    }

    IOStream *f = pIOHandler->Open(filename, "rb");
    if (!f) {
        throw DeadlyImportError("Failed to open skeleton file " + filename);
    }

    return MemoryStreamReaderPtr(new MemoryStreamReader(f));
}

void OgreBinarySerializer::ReadSkeleton(Skeleton *skeleton)
{
    uint16_t id = ReadHeader(false);
    if (id != HEADER_CHUNK_ID) {
        throw DeadlyExportError("Invalid Ogre Skeleton file header.");
    }

    // This deserialization supports both versions of the skeleton spec
    std::string version = ReadLine();
    if (version != SKELETON_VERSION_1_8 && version != SKELETON_VERSION_1_1)
    {
        throw DeadlyExportError(Formatter::format() << "Skeleton version " << version << " not supported by this importer."
            << " Supported versions: " << SKELETON_VERSION_1_8 << " and " << SKELETON_VERSION_1_1);
    }

    ASSIMP_LOG_DEBUG("Reading Skeleton");

    bool firstBone = true;
    bool firstAnim = true;

    while (!AtEnd())
    {
        id = ReadHeader();
        switch(id)
        {
            case SKELETON_BLENDMODE:
            {
                skeleton->blendMode = static_cast<Skeleton::BlendMode>(Read<uint16_t>());
                break;
            }
            case SKELETON_BONE:
            {
                if (firstBone)
                {
                    ASSIMP_LOG_DEBUG("  - Bones");
                    firstBone = false;
                }

                ReadBone(skeleton);
                break;
            }
            case SKELETON_BONE_PARENT:
            {
                ReadBoneParent(skeleton);
                break;
            }
            case SKELETON_ANIMATION:
            {
                if (firstAnim)
                {
                    ASSIMP_LOG_DEBUG("  - Animations");
                    firstAnim = false;
                }

                ReadSkeletonAnimation(skeleton);
                break;
            }
            case SKELETON_ANIMATION_LINK:
            {
                ReadSkeletonAnimationLink(skeleton);
                break;
            }
        }
    }

    // Calculate bone matrices for root bones. Recursively calculates their children.
    for (size_t i=0, len=skeleton->bones.size(); i<len; ++i)
    {
        Bone *bone = skeleton->bones[i];
        if (!bone->IsParented())
            bone->CalculateWorldMatrixAndDefaultPose(skeleton);
    }
}

void OgreBinarySerializer::ReadBone(Skeleton *skeleton)
{
    Bone *bone = new Bone();
    bone->name = ReadLine();
    bone->id = Read<uint16_t>();

    // Pos and rot
    ReadVector(bone->position);
    ReadQuaternion(bone->rotation);

    // Scale (optional)
    if (m_currentLen > MSTREAM_BONE_SIZE_WITHOUT_SCALE)
        ReadVector(bone->scale);

    // Bone indexes need to start from 0 and be contiguous
    if (bone->id != skeleton->bones.size()) {
        throw DeadlyImportError(Formatter::format() << "Ogre Skeleton bone indexes not contiguous. Error at bone index " << bone->id);
    }

    ASSIMP_LOG_DEBUG_F( "    ", bone->id, " ", bone->name);

    skeleton->bones.push_back(bone);
}

void OgreBinarySerializer::ReadBoneParent(Skeleton *skeleton)
{
    uint16_t childId = Read<uint16_t>();
    uint16_t parentId = Read<uint16_t>();

    Bone *child = skeleton->BoneById(childId);
    Bone *parent = skeleton->BoneById(parentId);

    if (child && parent)
        parent->AddChild(child);
    else
        throw DeadlyImportError(Formatter::format() << "Failed to find bones for parenting: Child id " << childId << " for parent id " << parentId);
}

void OgreBinarySerializer::ReadSkeletonAnimation(Skeleton *skeleton)
{
    Animation *anim = new Animation(skeleton);
    anim->name = ReadLine();
    anim->length = Read<float>();

    if (!AtEnd())
    {
        uint16_t id = ReadHeader();
        if (id == SKELETON_ANIMATION_BASEINFO)
        {
            anim->baseName = ReadLine();
            anim->baseTime = Read<float>();

            // Advance to first track
            id = ReadHeader();
        }

        while (!AtEnd() && id == SKELETON_ANIMATION_TRACK)
        {
            ReadSkeletonAnimationTrack(skeleton, anim);

            if (!AtEnd())
                id = ReadHeader();
        }
        if (!AtEnd())
            RollbackHeader();
    }

    skeleton->animations.push_back(anim);

    ASSIMP_LOG_DEBUG_F( "    ", anim->name, " (", anim->length, " sec, ", anim->tracks.size(), " tracks)");
}

void OgreBinarySerializer::ReadSkeletonAnimationTrack(Skeleton * /*skeleton*/, Animation *dest)
{
    uint16_t boneId = Read<uint16_t>();
    Bone *bone = dest->parentSkeleton->BoneById(boneId);
    if (!bone) {
        throw DeadlyImportError(Formatter::format() << "Cannot read animation track, target bone " << boneId << " not in target Skeleton");
    }

    VertexAnimationTrack track;
    track.type = VertexAnimationTrack::VAT_TRANSFORM;
    track.boneName = bone->name;

    uint16_t id = ReadHeader();
    while (!AtEnd() && id == SKELETON_ANIMATION_TRACK_KEYFRAME)
    {
        ReadSkeletonAnimationKeyFrame(&track);

        if (!AtEnd())
            id = ReadHeader();
    }
    if (!AtEnd())
        RollbackHeader();

    dest->tracks.push_back(track);
}

void OgreBinarySerializer::ReadSkeletonAnimationKeyFrame(VertexAnimationTrack *dest)
{
    TransformKeyFrame keyframe;
    keyframe.timePos = Read<float>();

    // Rot and pos
    ReadQuaternion(keyframe.rotation);
    ReadVector(keyframe.position);

    // Scale (optional)
    if (m_currentLen > MSTREAM_KEYFRAME_SIZE_WITHOUT_SCALE)
        ReadVector(keyframe.scale);

    dest->transformKeyFrames.push_back(keyframe);
}

void OgreBinarySerializer::ReadSkeletonAnimationLink(Skeleton * /*skeleton*/)
{
    // Skip bounds, not compatible with Assimp.
    ReadLine(); // skeleton name
    SkipBytes(sizeof(float) * 3); // scale
}

} // Ogre
} // Assimp

#endif // ASSIMP_BUILD_NO_OGRE_IMPORTER
