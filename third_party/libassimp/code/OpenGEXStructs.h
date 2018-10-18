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
#ifndef AI_OPENGEXSTRUCTS_H_INC
#define AI_OPENGEXSTRUCTS_H_INC

#include <string>
#include <map>

namespace Assimp {
namespace OpenGEX {

struct Skin;
struct Object;
struct LightObject;
struct CameraObject;
struct Material;
struct BoneNode;
struct BoneCountArray;
struct BoneIndexArray;
struct BoneWeightArray;

struct Metric {
    float m_distance;
    float m_angle;
    float m_time;
    float m_up;
};

struct VertexArray {
    std::string arrayAttrib;
    unsigned int morphIndex;
};

struct IndexArray {
    unsigned int materialIndex;
    unsigned int restartIndex;
    std::string frontFace;
};

struct Mesh {
    unsigned int meshLevel;
    std::string meshPrimitive;
    Skin *skinStructure;
};

struct Node {
    std::string nodeName;
};

struct GeometryNode {
    bool visibleFlag[ 2 ];
    bool shadowFlag[ 2 ];
    bool motionBlurFlag[ 2 ];
};

struct LightNode {
    bool shadowFlag[ 2 ];
    const LightObject *lightObjectStructure;
};

struct CameraNode {
    const CameraObject *cameraObjectStructure;
};

struct GeometryObject {
    Object *object;
    bool visibleFlag;
    bool shadowFlag;
    bool motionBlurFlag;
    std::map<std::string, Mesh*> meshMap;
};

struct LightObject {
    Object *object;
    std::string typeString;
    bool shadowFlag;
};

struct CameraObject {
    float focalLength;
    float nearDepth;
    float farDepth;
};

struct Matrix {
    bool objectFlag;
};

struct Transform {
    Matrix *matrix;
    int transformCount;
    const float *transformArray;
};

struct Translation {
    std::string translationKind;
};

struct Rotation {
    std::string rotationKind;
};

struct Scale {
    std::string scaleKind;
};

struct Name {
    std::string name;
};

struct ObjectRef {
    Object *targetStructure;
};

struct MaterialRef {
    unsigned int materialIndex;
    const Material *targetStructure;
};

struct BoneRefArray {
    int boneCount;
    const BoneNode **boneNodeArray;
};

struct BoneCount {
    int vertexCount;
    const unsigned short *boneCountArray;
    unsigned short *arrayStorage;
};

struct BoneIndex {
    int boneIndexCount;
    const unsigned short *boneIndexArray;
    unsigned short *arrayStorage;
};

struct BoneWeight {
    int boneWeightCount;
    const float *boneWeightArray;
};

struct Skeleton {
    const BoneRefArray *boneRefArrayStructure;
    const Transform *transformStructure;
};

struct Skin {
    const Skeleton *skeletonStructure;
    const BoneCountArray *boneCountArrayStructure;
    const BoneIndexArray *boneIndexArrayStructure;
    const BoneWeightArray *boneWeightArrayStructure;
};

struct Material {
    bool twoSidedFlag;
    const char *materialName;
};

struct Attrib {
    std::string attribString;
};

struct Param {
    float param;
};

struct Color {
    float color[ 4 ];
};

struct Texture {
    std::string textureName;
    unsigned int texcoordIndex;
};

struct Atten {
    std::string attenKind;
    std::string curveType;

    float beginParam;
    float endParam;

    float scaleParam;
    float offsetParam;

    float constantParam;
    float linearParam;
    float quadraticParam;

    float powerParam;
};

struct Key {
    std::string keyKind;
    bool scalarFlag;
};

struct Curve {
    std::string curveType;
    const Key *keyValueStructure;
    const Key *keyControlStructure[ 2 ];
    const Key *keyTensionStructure;
    const Key *keyContinuityStructure;
    const Key *keyBiasStructure;
};

struct Animation {
    int clipIndex;
    bool beginFlag;
    bool endFlag;
    float beginTime;
    float endTime;
};

struct OpenGexDataDescription {
    float distanceScale;
    float angleScale;
    float timeScale;
    int upDirection;
};

} // Namespace OpenGEX
} // Namespace Assimp

#endif // AI_OPENGEXSTRUCTS_H_INC
