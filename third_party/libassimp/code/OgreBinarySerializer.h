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

#ifndef AI_OGREBINARYSERIALIZER_H_INC
#define AI_OGREBINARYSERIALIZER_H_INC

#ifndef ASSIMP_BUILD_NO_OGRE_IMPORTER

#include "OgreStructs.h"
#include <assimp/StreamReader.h>

namespace Assimp
{
namespace Ogre
{

typedef Assimp::StreamReaderLE MemoryStreamReader;
typedef std::shared_ptr<MemoryStreamReader> MemoryStreamReaderPtr;

class OgreBinarySerializer
{
public:
    /// Imports mesh and returns the result.
    /** @note Fatal unrecoverable errors will throw a DeadlyImportError. */
    static Mesh *ImportMesh(MemoryStreamReader *reader);

    /// Imports skeleton to @c mesh into Mesh::skeleton.
    /** If mesh does not have a skeleton reference or the skeleton file
        cannot be found it is not a fatal DeadlyImportError.
        @return If skeleton import was successful. */
    static bool ImportSkeleton(Assimp::IOSystem *pIOHandler, Mesh *mesh);
    static bool ImportSkeleton(Assimp::IOSystem *pIOHandler, MeshXml *mesh);

private:
    enum AssetMode
    {
        AM_Mesh,
        AM_Skeleton
    };

    OgreBinarySerializer(MemoryStreamReader *reader, AssetMode mode) :
        m_currentLen(0),
        m_reader(reader),
        assetMode(mode)
    {
    }

    static MemoryStreamReaderPtr OpenReader(Assimp::IOSystem *pIOHandler, const std::string &filename);

    // Header

    uint16_t ReadHeader(bool readLen = true);
    void RollbackHeader();

    // Mesh

    void ReadMesh(Mesh *mesh);
    void ReadMeshLodInfo(Mesh *mesh);
    void ReadMeshSkeletonLink(Mesh *mesh);
    void ReadMeshBounds(Mesh *mesh);
    void ReadMeshExtremes(Mesh *mesh);

    void ReadSubMesh(Mesh *mesh);
    void ReadSubMeshNames(Mesh *mesh);
    void ReadSubMeshOperation(SubMesh *submesh);
    void ReadSubMeshTextureAlias(SubMesh *submesh);

    void ReadBoneAssignment(VertexData *dest);

    void ReadGeometry(VertexData *dest);
    void ReadGeometryVertexDeclaration(VertexData *dest);
    void ReadGeometryVertexElement(VertexData *dest);
    void ReadGeometryVertexBuffer(VertexData *dest);

    void ReadEdgeList(Mesh *mesh);
    void ReadPoses(Mesh *mesh);
    void ReadPoseVertices(Pose *pose);

    void ReadAnimations(Mesh *mesh);
    void ReadAnimation(Animation *anim);
    void ReadAnimationKeyFrames(Animation *anim, VertexAnimationTrack *track);

    void NormalizeBoneWeights(VertexData *vertexData) const;

    // Skeleton

    void ReadSkeleton(Skeleton *skeleton);

    void ReadBone(Skeleton *skeleton);
    void ReadBoneParent(Skeleton *skeleton);

    void ReadSkeletonAnimation(Skeleton *skeleton);
    void ReadSkeletonAnimationTrack(Skeleton *skeleton, Animation *dest);
    void ReadSkeletonAnimationKeyFrame(VertexAnimationTrack *dest);
    void ReadSkeletonAnimationLink(Skeleton *skeleton);

    // Reader utils
    bool AtEnd() const;

    template<typename T>
    inline T Read();

    void ReadBytes(char *dest, size_t numBytes);
    void ReadBytes(uint8_t *dest, size_t numBytes);
    void ReadBytes(void *dest, size_t numBytes);
    uint8_t *ReadBytes(size_t numBytes);

    void ReadVector(aiVector3D &vec);
    void ReadQuaternion(aiQuaternion &quat);

    std::string ReadString(size_t len);
    std::string ReadLine();

    void SkipBytes(size_t numBytes);

    uint32_t m_currentLen;
    MemoryStreamReader *m_reader;

    AssetMode assetMode;
};

enum MeshChunkId
{
    M_HEADER = 0x1000,
        // char*          version          : Version number check
    M_MESH   = 0x3000,
        // bool skeletallyAnimated   // important flag which affects h/w buffer policies
        // Optional M_GEOMETRY chunk
        M_SUBMESH            = 0x4000,
            // char* materialName
            // bool useSharedVertices
            // unsigned int indexCount
            // bool indexes32Bit
            // unsigned int* faceVertexIndices (indexCount)
            // OR
            // unsigned short* faceVertexIndices (indexCount)
            // M_GEOMETRY chunk (Optional: present only if useSharedVertices = false)
            M_SUBMESH_OPERATION = 0x4010, // optional, trilist assumed if missing
                // unsigned short operationType
            M_SUBMESH_BONE_ASSIGNMENT = 0x4100,
                // Optional bone weights (repeating section)
                // unsigned int vertexIndex;
                // unsigned short boneIndex;
                // float weight;
            // Optional chunk that matches a texture name to an alias
            // a texture alias is sent to the submesh material to use this texture name
            // instead of the one in the texture unit with a matching alias name
            M_SUBMESH_TEXTURE_ALIAS = 0x4200, // Repeating section
                // char* aliasName;
                // char* textureName;

        M_GEOMETRY        = 0x5000, // NB this chunk is embedded within M_MESH and M_SUBMESH
            // unsigned int vertexCount
            M_GEOMETRY_VERTEX_DECLARATION = 0x5100,
                M_GEOMETRY_VERTEX_ELEMENT = 0x5110, // Repeating section
                    // unsigned short source;   // buffer bind source
                    // unsigned short type;     // VertexElementType
                    // unsigned short semantic; // VertexElementSemantic
                    // unsigned short offset;   // start offset in buffer in bytes
                    // unsigned short index;    // index of the semantic (for colours and texture coords)
            M_GEOMETRY_VERTEX_BUFFER = 0x5200, // Repeating section
                // unsigned short bindIndex;    // Index to bind this buffer to
                // unsigned short vertexSize;   // Per-vertex size, must agree with declaration at this index
                M_GEOMETRY_VERTEX_BUFFER_DATA = 0x5210,
                    // raw buffer data
        M_MESH_SKELETON_LINK = 0x6000,
            // Optional link to skeleton
            // char* skeletonName          : name of .skeleton to use
        M_MESH_BONE_ASSIGNMENT = 0x7000,
            // Optional bone weights (repeating section)
            // unsigned int vertexIndex;
            // unsigned short boneIndex;
            // float weight;
        M_MESH_LOD = 0x8000,
            // Optional LOD information
            // string strategyName;
            // unsigned short numLevels;
            // bool manual;  (true for manual alternate meshes, false for generated)
            M_MESH_LOD_USAGE = 0x8100,
            // Repeating section, ordered in increasing depth
            // NB LOD 0 (full detail from 0 depth) is omitted
            // LOD value - this is a distance, a pixel count etc, based on strategy
            // float lodValue;
                M_MESH_LOD_MANUAL = 0x8110,
                // Required if M_MESH_LOD section manual = true
                // String manualMeshName;
                M_MESH_LOD_GENERATED = 0x8120,
                // Required if M_MESH_LOD section manual = false
                // Repeating section (1 per submesh)
                // unsigned int indexCount;
                // bool indexes32Bit
                // unsigned short* faceIndexes;  (indexCount)
                // OR
                // unsigned int* faceIndexes;  (indexCount)
        M_MESH_BOUNDS = 0x9000,
            // float minx, miny, minz
            // float maxx, maxy, maxz
            // float radius

        // Added By DrEvil
        // optional chunk that contains a table of submesh indexes and the names of
        // the sub-meshes.
        M_SUBMESH_NAME_TABLE = 0xA000,
            // Subchunks of the name table. Each chunk contains an index & string
            M_SUBMESH_NAME_TABLE_ELEMENT = 0xA100,
                // short index
                // char* name
        // Optional chunk which stores precomputed edge data
        M_EDGE_LISTS = 0xB000,
            // Each LOD has a separate edge list
            M_EDGE_LIST_LOD = 0xB100,
                // unsigned short lodIndex
                // bool isManual            // If manual, no edge data here, loaded from manual mesh
                    // bool isClosed
                    // unsigned long numTriangles
                    // unsigned long numEdgeGroups
                    // Triangle* triangleList
                        // unsigned long indexSet
                        // unsigned long vertexSet
                        // unsigned long vertIndex[3]
                        // unsigned long sharedVertIndex[3]
                        // float normal[4]

                    M_EDGE_GROUP = 0xB110,
                        // unsigned long vertexSet
                        // unsigned long triStart
                        // unsigned long triCount
                        // unsigned long numEdges
                        // Edge* edgeList
                            // unsigned long  triIndex[2]
                            // unsigned long  vertIndex[2]
                            // unsigned long  sharedVertIndex[2]
                            // bool degenerate
        // Optional poses section, referred to by pose keyframes
        M_POSES = 0xC000,
            M_POSE = 0xC100,
                // char* name (may be blank)
                // unsigned short target    // 0 for shared geometry,
                                            // 1+ for submesh index + 1
                // bool includesNormals [1.8+]
                M_POSE_VERTEX = 0xC111,
                    // unsigned long vertexIndex
                    // float xoffset, yoffset, zoffset
                    // float xnormal, ynormal, znormal (optional, 1.8+)
        // Optional vertex animation chunk
        M_ANIMATIONS = 0xD000,
            M_ANIMATION = 0xD100,
            // char* name
            // float length
            M_ANIMATION_BASEINFO = 0xD105,
            // [Optional] base keyframe information (pose animation only)
            // char* baseAnimationName (blank for self)
            // float baseKeyFrameTime
            M_ANIMATION_TRACK = 0xD110,
                // unsigned short type          // 1 == morph, 2 == pose
                // unsigned short target        // 0 for shared geometry,
                                                // 1+ for submesh index + 1
                M_ANIMATION_MORPH_KEYFRAME = 0xD111,
                    // float time
                    // bool includesNormals [1.8+]
                    // float x,y,z          // repeat by number of vertices in original geometry
                M_ANIMATION_POSE_KEYFRAME = 0xD112,
                    // float time
                    M_ANIMATION_POSE_REF = 0xD113, // repeat for number of referenced poses
                        // unsigned short poseIndex
                        // float influence
        // Optional submesh extreme vertex list chink
        M_TABLE_EXTREMES = 0xE000
        // unsigned short submesh_index;
        // float extremes [n_extremes][3];
};

/*
static std::string MeshHeaderToString(MeshChunkId id)
{
    switch(id)
    {
        case M_HEADER:                      return "HEADER";
        case M_MESH:                        return "MESH";
        case M_SUBMESH:                     return "SUBMESH";
        case M_SUBMESH_OPERATION:           return "SUBMESH_OPERATION";
        case M_SUBMESH_BONE_ASSIGNMENT:     return "SUBMESH_BONE_ASSIGNMENT";
        case M_SUBMESH_TEXTURE_ALIAS:       return "SUBMESH_TEXTURE_ALIAS";
        case M_GEOMETRY:                    return "GEOMETRY";
        case M_GEOMETRY_VERTEX_DECLARATION: return "GEOMETRY_VERTEX_DECLARATION";
        case M_GEOMETRY_VERTEX_ELEMENT:     return "GEOMETRY_VERTEX_ELEMENT";
        case M_GEOMETRY_VERTEX_BUFFER:      return "GEOMETRY_VERTEX_BUFFER";
        case M_GEOMETRY_VERTEX_BUFFER_DATA: return "GEOMETRY_VERTEX_BUFFER_DATA";
        case M_MESH_SKELETON_LINK:          return "MESH_SKELETON_LINK";
        case M_MESH_BONE_ASSIGNMENT:        return "MESH_BONE_ASSIGNMENT";
        case M_MESH_LOD:                    return "MESH_LOD";
        case M_MESH_LOD_USAGE:              return "MESH_LOD_USAGE";
        case M_MESH_LOD_MANUAL:             return "MESH_LOD_MANUAL";
        case M_MESH_LOD_GENERATED:          return "MESH_LOD_GENERATED";
        case M_MESH_BOUNDS:                 return "MESH_BOUNDS";
        case M_SUBMESH_NAME_TABLE:          return "SUBMESH_NAME_TABLE";
        case M_SUBMESH_NAME_TABLE_ELEMENT:  return "SUBMESH_NAME_TABLE_ELEMENT";
        case M_EDGE_LISTS:                  return "EDGE_LISTS";
        case M_EDGE_LIST_LOD:               return "EDGE_LIST_LOD";
        case M_EDGE_GROUP:                  return "EDGE_GROUP";
        case M_POSES:                       return "POSES";
        case M_POSE:                        return "POSE";
        case M_POSE_VERTEX:                 return "POSE_VERTEX";
        case M_ANIMATIONS:                  return "ANIMATIONS";
        case M_ANIMATION:                   return "ANIMATION";
        case M_ANIMATION_BASEINFO:          return "ANIMATION_BASEINFO";
        case M_ANIMATION_TRACK:             return "ANIMATION_TRACK";
        case M_ANIMATION_MORPH_KEYFRAME:    return "ANIMATION_MORPH_KEYFRAME";
        case M_ANIMATION_POSE_KEYFRAME:     return "ANIMATION_POSE_KEYFRAME";
        case M_ANIMATION_POSE_REF:          return "ANIMATION_POSE_REF";
        case M_TABLE_EXTREMES:              return "TABLE_EXTREMES";
    }
    return "Unknown_MeshChunkId";
}
*/

enum SkeletonChunkId
{
    SKELETON_HEADER             = 0x1000,
        // char* version           : Version number check
        SKELETON_BLENDMODE      = 0x1010, // optional
            // unsigned short blendmode     : SkeletonAnimationBlendMode
    SKELETON_BONE               = 0x2000,
    // Repeating section defining each bone in the system.
    // Bones are assigned indexes automatically based on their order of declaration
    // starting with 0.
        // char* name                      : name of the bone
        // unsigned short handle            : handle of the bone, should be contiguous & start at 0
        // Vector3 position              : position of this bone relative to parent
        // Quaternion orientation          : orientation of this bone relative to parent
        // Vector3 scale                    : scale of this bone relative to parent
    SKELETON_BONE_PARENT        = 0x3000,
    // Record of the parent of a single bone, used to build the node tree
    // Repeating section, listed in Bone Index order, one per Bone
        // unsigned short handle             : child bone
        // unsigned short parentHandle   : parent bone
    SKELETON_ANIMATION          = 0x4000,
    // A single animation for this skeleton
        // char* name                      : Name of the animation
        // float length                   : Length of the animation in seconds
        SKELETON_ANIMATION_BASEINFO = 0x4010,
        // [Optional] base keyframe information
        // char* baseAnimationName (blank for self)
        // float baseKeyFrameTime
        SKELETON_ANIMATION_TRACK    = 0x4100,
        // A single animation track (relates to a single bone)
        // Repeating section (within SKELETON_ANIMATION)
            // unsigned short boneIndex  : Index of bone to apply to
            SKELETON_ANIMATION_TRACK_KEYFRAME = 0x4110,
            // A single keyframe within the track
            // Repeating section
                // float time                   : The time position (seconds)
                // Quaternion rotate            : Rotation to apply at this keyframe
                // Vector3 translate            : Translation to apply at this keyframe
                // Vector3 scale                : Scale to apply at this keyframe
    SKELETON_ANIMATION_LINK     = 0x5000
    // Link to another skeleton, to re-use its animations
        // char* skeletonName                   : name of skeleton to get animations from
        // float scale                          : scale to apply to trans/scale keys
};

/*
static std::string SkeletonHeaderToString(SkeletonChunkId id)
{
    switch(id)
    {
        case SKELETON_HEADER:                   return "HEADER";
        case SKELETON_BLENDMODE:                return "BLENDMODE";
        case SKELETON_BONE:                     return "BONE";
        case SKELETON_BONE_PARENT:              return "BONE_PARENT";
        case SKELETON_ANIMATION:                return "ANIMATION";
        case SKELETON_ANIMATION_BASEINFO:       return "ANIMATION_BASEINFO";
        case SKELETON_ANIMATION_TRACK:          return "ANIMATION_TRACK";
        case SKELETON_ANIMATION_TRACK_KEYFRAME: return "ANIMATION_TRACK_KEYFRAME";
        case SKELETON_ANIMATION_LINK:           return "ANIMATION_LINK";
    }
    return "Unknown_SkeletonChunkId";
}
*/
} // Ogre
} // Assimp

#endif // ASSIMP_BUILD_NO_OGRE_IMPORTER
#endif // AI_OGREBINARYSERIALIZER_H_INC
