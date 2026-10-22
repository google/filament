/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team


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


/** @file Defines the helper data structures for importing XFiles */
#ifndef AI_XFILEHELPER_H_INC
#define AI_XFILEHELPER_H_INC

#include <string>
#include <vector>
#include <stdint.h>

#include <assimp/types.h>
#include <assimp/quaternion.h>
#include <assimp/mesh.h>
#include <assimp/anim.h>
#include <assimp/Defines.h>

namespace Assimp {
namespace XFile {

/** Helper structure representing a XFile mesh face */
struct Face {
    std::vector<unsigned int> mIndices;
};

/** Helper structure representing a texture filename inside a material and its potential source */
struct TexEntry {
    std::string mName;
    bool mIsNormalMap; // true if the texname was specified in a NormalmapFilename tag

    TexEntry() AI_NO_EXCEPT
    : mName()
    , mIsNormalMap(false) {
        // empty
    }
    TexEntry(const std::string& pName, bool pIsNormalMap = false)
    : mName(pName)
    , mIsNormalMap(pIsNormalMap) {
        // empty
    }
};

/** Helper structure representing a XFile material */
struct Material {
    std::string mName;
    bool mIsReference; // if true, mName holds a name by which the actual material can be found in the material list
    aiColor4D mDiffuse;
    ai_real mSpecularExponent;
    aiColor3D mSpecular;
    aiColor3D mEmissive;
    std::vector<TexEntry> mTextures;
    size_t sceneIndex; ///< the index under which it was stored in the scene's material list

    Material() AI_NO_EXCEPT
    : mIsReference(false)
    , mSpecularExponent()
    , sceneIndex(SIZE_MAX) {
        // empty
    }
};

/** Helper structure to represent a bone weight */
struct BoneWeight {
    unsigned int mVertex;
    ai_real mWeight;
};

/** Helper structure to represent a bone in a mesh */
struct Bone
{
    std::string mName;
    std::vector<BoneWeight> mWeights;
    aiMatrix4x4 mOffsetMatrix;
};

/** Helper structure to represent an XFile mesh */
struct Mesh {
    std::string mName;
    std::vector<aiVector3D> mPositions;
    std::vector<Face> mPosFaces;
    std::vector<aiVector3D> mNormals;
    std::vector<Face> mNormFaces;
    unsigned int mNumTextures;
    std::vector<aiVector2D> mTexCoords[AI_MAX_NUMBER_OF_TEXTURECOORDS];
    unsigned int mNumColorSets;
    std::vector<aiColor4D> mColors[AI_MAX_NUMBER_OF_COLOR_SETS];

    std::vector<unsigned int> mFaceMaterials;
    std::vector<Material> mMaterials;

    std::vector<Bone> mBones;

    explicit Mesh(const std::string &pName = "") AI_NO_EXCEPT
    : mName( pName )
    , mPositions()
    , mPosFaces()
    , mNormals()
    , mNormFaces()
    , mNumTextures(0)
    , mTexCoords{}
    , mNumColorSets(0)
    , mColors{}
    , mFaceMaterials()
    , mMaterials()
    , mBones() {
        // empty
    }
};

/** Helper structure to represent a XFile frame */
struct Node {
    std::string mName;
    aiMatrix4x4 mTrafoMatrix;
    Node* mParent;
    std::vector<Node*> mChildren;
    std::vector<Mesh*> mMeshes;

    Node() AI_NO_EXCEPT
    : mName()
    , mTrafoMatrix()
    , mParent(nullptr)
    , mChildren()
    , mMeshes() {
        // empty
    }
    explicit Node( Node* pParent)
    : mName()
    , mTrafoMatrix()
    , mParent(pParent)
    , mChildren()
    , mMeshes() {
        // empty
    }

    ~Node() {
        for (unsigned int a = 0; a < mChildren.size(); ++a ) {
            delete mChildren[a];
        }
        for (unsigned int a = 0; a < mMeshes.size(); ++a) {
            delete mMeshes[a];
        }
    }
};

struct MatrixKey {
    double mTime;
    aiMatrix4x4 mMatrix;
};

/** Helper structure representing a single animated bone in a XFile */
struct AnimBone {
    std::string mBoneName;
    std::vector<aiVectorKey> mPosKeys;  // either three separate key sequences for position, rotation, scaling
    std::vector<aiQuatKey> mRotKeys;
    std::vector<aiVectorKey> mScaleKeys;
    std::vector<MatrixKey> mTrafoKeys; // or a combined key sequence of transformation matrices.
};

/** Helper structure to represent an animation set in a XFile */
struct Animation
{
    std::string mName;
    std::vector<AnimBone*> mAnims;

    ~Animation()
    {
        for( unsigned int a = 0; a < mAnims.size(); a++)
            delete mAnims[a];
    }
};

/** Helper structure analogue to aiScene */
struct Scene
{
    Node* mRootNode;

    std::vector<Mesh*> mGlobalMeshes; // global meshes found outside of any frames
    std::vector<Material> mGlobalMaterials; // global materials found outside of any meshes.

    std::vector<Animation*> mAnims;
    unsigned int mAnimTicksPerSecond;

    Scene() AI_NO_EXCEPT
    : mRootNode(nullptr)
    , mGlobalMeshes()
    , mGlobalMaterials()
    , mAnimTicksPerSecond(0) {
        // empty
    }
    ~Scene() {
        delete mRootNode;
        mRootNode = nullptr;
        for (unsigned int a = 0; a < mGlobalMeshes.size(); ++a ) {
            delete mGlobalMeshes[a];
        }
        for (unsigned int a = 0; a < mAnims.size(); ++a ) {
            delete mAnims[a];
        }
    }
};

} // end of namespace XFile
} // end of namespace Assimp

#endif // AI_XFILEHELPER_H_INC
