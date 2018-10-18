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

/** @file  AssbinLoader.cpp
 *  @brief Implementation of the .assbin importer class
 *
 *  see assbin_chunks.h
 */

#ifndef ASSIMP_BUILD_NO_ASSBIN_IMPORTER

// internal headers
#include "AssbinLoader.h"
#include "assbin_chunks.h"
#include <assimp/MemoryIOWrapper.h>
#include <assimp/mesh.h>
#include <assimp/anim.h>
#include <assimp/scene.h>
#include <assimp/importerdesc.h>

#ifdef ASSIMP_BUILD_NO_OWN_ZLIB
#   include <zlib.h>
#else
#   include <contrib/zlib/zlib.h>
#endif

using namespace Assimp;

static const aiImporterDesc desc = {
    ".assbin Importer",
    "Gargaj / Conspiracy",
    "",
    "",
    aiImporterFlags_SupportBinaryFlavour | aiImporterFlags_SupportCompressedFlavour,
    0,
    0,
    0,
    0,
    "assbin"
};

// -----------------------------------------------------------------------------------
const aiImporterDesc* AssbinImporter::GetInfo() const {
    return &desc;
}

// -----------------------------------------------------------------------------------
bool AssbinImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool /*checkSig*/ ) const {
    IOStream * in = pIOHandler->Open(pFile);
    if (nullptr == in) {
        return false;
    }

    char s[32];
    in->Read( s, sizeof(char), 32 );

    pIOHandler->Close(in);

    return strncmp( s, "ASSIMP.binary-dump.", 19 ) == 0;
}

// -----------------------------------------------------------------------------------
template <typename T>
T Read(IOStream * stream) {
    T t;
    stream->Read( &t, sizeof(T), 1 );
    return t;
}

// -----------------------------------------------------------------------------------
template <>
aiVector3D Read<aiVector3D>(IOStream * stream) {
    aiVector3D v;
    v.x = Read<float>(stream);
    v.y = Read<float>(stream);
    v.z = Read<float>(stream);
    return v;
}

// -----------------------------------------------------------------------------------
template <>
aiColor4D Read<aiColor4D>(IOStream * stream) {
    aiColor4D c;
    c.r = Read<float>(stream);
    c.g = Read<float>(stream);
    c.b = Read<float>(stream);
    c.a = Read<float>(stream);
    return c;
}

// -----------------------------------------------------------------------------------
template <>
aiQuaternion Read<aiQuaternion>(IOStream * stream) {
    aiQuaternion v;
    v.w = Read<float>(stream);
    v.x = Read<float>(stream);
    v.y = Read<float>(stream);
    v.z = Read<float>(stream);
    return v;
}

// -----------------------------------------------------------------------------------
template <>
aiString Read<aiString>(IOStream * stream) {
    aiString s;
    stream->Read(&s.length,4,1);
    stream->Read(s.data,s.length,1);
    s.data[s.length] = 0;
    return s;
}

// -----------------------------------------------------------------------------------
template <>
aiVertexWeight Read<aiVertexWeight>(IOStream * stream) {
    aiVertexWeight w;
    w.mVertexId = Read<unsigned int>(stream);
    w.mWeight = Read<float>(stream);
    return w;
}

// -----------------------------------------------------------------------------------
template <>
aiMatrix4x4 Read<aiMatrix4x4>(IOStream * stream) {
    aiMatrix4x4 m;
    for (unsigned int i = 0; i < 4;++i) {
        for (unsigned int i2 = 0; i2 < 4;++i2) {
            m[i][i2] = Read<float>(stream);
        }
    }
    return m;
}

// -----------------------------------------------------------------------------------
template <>
aiVectorKey Read<aiVectorKey>(IOStream * stream) {
    aiVectorKey v;
    v.mTime = Read<double>(stream);
    v.mValue = Read<aiVector3D>(stream);
    return v;
}

// -----------------------------------------------------------------------------------
template <>
aiQuatKey Read<aiQuatKey>(IOStream * stream) {
    aiQuatKey v;
    v.mTime = Read<double>(stream);
    v.mValue = Read<aiQuaternion>(stream);
    return v;
}

// -----------------------------------------------------------------------------------
template <typename T>
void ReadArray( IOStream *stream, T * out, unsigned int size) {
    ai_assert( nullptr != stream );
    ai_assert( nullptr != out );

    for (unsigned int i=0; i<size; i++) {
        out[i] = Read<T>(stream);
    }
}

// -----------------------------------------------------------------------------------
template <typename T>
void ReadBounds( IOStream * stream, T* /*p*/, unsigned int n ) {
    // not sure what to do here, the data isn't really useful.
    stream->Seek( sizeof(T) * n, aiOrigin_CUR );
}

// -----------------------------------------------------------------------------------
void AssbinImporter::ReadBinaryNode( IOStream * stream, aiNode** node, aiNode* parent ) {
    uint32_t chunkID = Read<uint32_t>(stream);
    (void)(chunkID);
    ai_assert(chunkID == ASSBIN_CHUNK_AINODE);
    /*uint32_t size =*/ Read<uint32_t>(stream);

    *node = new aiNode();

    (*node)->mName = Read<aiString>(stream);
    (*node)->mTransformation = Read<aiMatrix4x4>(stream);
    (*node)->mNumChildren = Read<unsigned int>(stream);
    (*node)->mNumMeshes = Read<unsigned int>(stream);
	unsigned int nb_metadata = Read<unsigned int>(stream);

    if(parent) {
        (*node)->mParent = parent;
    }

    if ((*node)->mNumMeshes) {
        (*node)->mMeshes = new unsigned int[(*node)->mNumMeshes];
        for (unsigned int i = 0; i < (*node)->mNumMeshes; ++i) {
            (*node)->mMeshes[i] = Read<unsigned int>(stream);
        }
    }

    if ((*node)->mNumChildren) {
        (*node)->mChildren = new aiNode*[(*node)->mNumChildren];
        for (unsigned int i = 0; i < (*node)->mNumChildren; ++i) {
            ReadBinaryNode( stream, &(*node)->mChildren[i], *node );
        }
    }

    if ( nb_metadata > 0 ) {
        (*node)->mMetaData = aiMetadata::Alloc(nb_metadata);
        for (unsigned int i = 0; i < nb_metadata; ++i) {
            (*node)->mMetaData->mKeys[i] = Read<aiString>(stream);
            (*node)->mMetaData->mValues[i].mType = (aiMetadataType) Read<uint16_t>(stream);
            void* data( nullptr );

            switch ((*node)->mMetaData->mValues[i].mType) {
                case AI_BOOL:
                    data = new bool(Read<bool>(stream));
                    break;
                case AI_INT32:
                    data = new int32_t(Read<int32_t>(stream));
                    break;
                case AI_UINT64:
                    data = new uint64_t(Read<uint64_t>(stream));
                    break;
                case AI_FLOAT:
                    data = new float(Read<float>(stream));
                    break;
                case AI_DOUBLE:
                    data = new double(Read<double>(stream));
                    break;
                case AI_AISTRING:
                    data = new aiString(Read<aiString>(stream));
                    break;
                case AI_AIVECTOR3D:
                    data = new aiVector3D(Read<aiVector3D>(stream));
                    break;
#ifndef SWIG
                case FORCE_32BIT:
#endif // SWIG
                default:
                    break;
            }

			(*node)->mMetaData->mValues[i].mData = data;
		}
	}
}

// -----------------------------------------------------------------------------------
void AssbinImporter::ReadBinaryBone( IOStream * stream, aiBone* b ) {
    uint32_t chunkID = Read<uint32_t>(stream);
    (void)(chunkID);
    ai_assert(chunkID == ASSBIN_CHUNK_AIBONE);
    /*uint32_t size =*/ Read<uint32_t>(stream);

    b->mName = Read<aiString>(stream);
    b->mNumWeights = Read<unsigned int>(stream);
    b->mOffsetMatrix = Read<aiMatrix4x4>(stream);

    // for the moment we write dumb min/max values for the bones, too.
    // maybe I'll add a better, hash-like solution later
    if (shortened) {
        ReadBounds(stream,b->mWeights,b->mNumWeights);
    } else {
        // else write as usual
        b->mWeights = new aiVertexWeight[b->mNumWeights];
        ReadArray<aiVertexWeight>(stream,b->mWeights,b->mNumWeights);
    }
}

// -----------------------------------------------------------------------------------
static bool fitsIntoUI16(unsigned int mNumVertices) {
    return ( mNumVertices < (1u<<16) );
}

// -----------------------------------------------------------------------------------
void AssbinImporter::ReadBinaryMesh( IOStream * stream, aiMesh* mesh ) {
    uint32_t chunkID = Read<uint32_t>(stream);
    (void)(chunkID);
    ai_assert(chunkID == ASSBIN_CHUNK_AIMESH);
    /*uint32_t size =*/ Read<uint32_t>(stream);

    mesh->mPrimitiveTypes = Read<unsigned int>(stream);
    mesh->mNumVertices = Read<unsigned int>(stream);
    mesh->mNumFaces = Read<unsigned int>(stream);
    mesh->mNumBones = Read<unsigned int>(stream);
    mesh->mMaterialIndex = Read<unsigned int>(stream);

    // first of all, write bits for all existent vertex components
    unsigned int c = Read<unsigned int>(stream);

    if (c & ASSBIN_MESH_HAS_POSITIONS) {
        if (shortened) {
            ReadBounds(stream,mesh->mVertices,mesh->mNumVertices);
        }  else {
            // else write as usual
            mesh->mVertices = new aiVector3D[mesh->mNumVertices];
            ReadArray<aiVector3D>(stream,mesh->mVertices,mesh->mNumVertices);
        }
    }
    if (c & ASSBIN_MESH_HAS_NORMALS) {
        if (shortened) {
            ReadBounds(stream,mesh->mNormals,mesh->mNumVertices);
        }  else {
            // else write as usual
            mesh->mNormals = new aiVector3D[mesh->mNumVertices];
            ReadArray<aiVector3D>(stream,mesh->mNormals,mesh->mNumVertices);
        }
    }
    if (c & ASSBIN_MESH_HAS_TANGENTS_AND_BITANGENTS) {
        if (shortened) {
            ReadBounds(stream,mesh->mTangents,mesh->mNumVertices);
            ReadBounds(stream,mesh->mBitangents,mesh->mNumVertices);
        }  else {
            // else write as usual
            mesh->mTangents = new aiVector3D[mesh->mNumVertices];
            ReadArray<aiVector3D>(stream,mesh->mTangents,mesh->mNumVertices);
            mesh->mBitangents = new aiVector3D[mesh->mNumVertices];
            ReadArray<aiVector3D>(stream,mesh->mBitangents,mesh->mNumVertices);
        }
    }
    for (unsigned int n = 0; n < AI_MAX_NUMBER_OF_COLOR_SETS;++n) {
        if (!(c & ASSBIN_MESH_HAS_COLOR(n))) {
            break;
        }

        if (shortened) {
            ReadBounds(stream,mesh->mColors[n],mesh->mNumVertices);
        }  else {
            // else write as usual
            mesh->mColors[n] = new aiColor4D[mesh->mNumVertices];
            ReadArray<aiColor4D>(stream,mesh->mColors[n],mesh->mNumVertices);
        }
    }
    for (unsigned int n = 0; n < AI_MAX_NUMBER_OF_TEXTURECOORDS;++n) {
        if (!(c & ASSBIN_MESH_HAS_TEXCOORD(n))) {
            break;
        }

        // write number of UV components
        mesh->mNumUVComponents[n] = Read<unsigned int>(stream);

        if (shortened) {
            ReadBounds(stream,mesh->mTextureCoords[n],mesh->mNumVertices);
        }  else {
            // else write as usual
            mesh->mTextureCoords[n] = new aiVector3D[mesh->mNumVertices];
            ReadArray<aiVector3D>(stream,mesh->mTextureCoords[n],mesh->mNumVertices);
        }
    }

    // write faces. There are no floating-point calculations involved
    // in these, so we can write a simple hash over the face data
    // to the dump file. We generate a single 32 Bit hash for 512 faces
    // using Assimp's standard hashing function.
    if (shortened) {
        Read<unsigned int>(stream);
    } else  {
        // else write as usual
        // if there are less than 2^16 vertices, we can simply use 16 bit integers ...
        mesh->mFaces = new aiFace[mesh->mNumFaces];
        for (unsigned int i = 0; i < mesh->mNumFaces;++i) {
            aiFace& f = mesh->mFaces[i];

            static_assert(AI_MAX_FACE_INDICES <= 0xffff, "AI_MAX_FACE_INDICES <= 0xffff");
            f.mNumIndices = Read<uint16_t>(stream);
            f.mIndices = new unsigned int[f.mNumIndices];

            for (unsigned int a = 0; a < f.mNumIndices;++a) {
                // Check if unsigned  short ( 16 bit  ) are big enought for the indices
                if ( fitsIntoUI16( mesh->mNumVertices ) ) {
                    f.mIndices[a] = Read<uint16_t>(stream);
                } else {
                    f.mIndices[a] = Read<unsigned int>(stream);
                }
            }
        }
    }

    // write bones
    if (mesh->mNumBones) {
        mesh->mBones = new C_STRUCT aiBone*[mesh->mNumBones];
        for (unsigned int a = 0; a < mesh->mNumBones;++a) {
            mesh->mBones[a] = new aiBone();
            ReadBinaryBone(stream,mesh->mBones[a]);
        }
    }
}

// -----------------------------------------------------------------------------------
void AssbinImporter::ReadBinaryMaterialProperty(IOStream * stream, aiMaterialProperty* prop) {
    uint32_t chunkID = Read<uint32_t>(stream);
    (void)(chunkID);
    ai_assert(chunkID == ASSBIN_CHUNK_AIMATERIALPROPERTY);
    /*uint32_t size =*/ Read<uint32_t>(stream);

    prop->mKey = Read<aiString>(stream);
    prop->mSemantic = Read<unsigned int>(stream);
    prop->mIndex = Read<unsigned int>(stream);

    prop->mDataLength = Read<unsigned int>(stream);
    prop->mType = (aiPropertyTypeInfo)Read<unsigned int>(stream);
    prop->mData = new char [ prop->mDataLength ];
    stream->Read(prop->mData,1,prop->mDataLength);
}

// -----------------------------------------------------------------------------------
void AssbinImporter::ReadBinaryMaterial(IOStream * stream, aiMaterial* mat) {
    uint32_t chunkID = Read<uint32_t>(stream);
    (void)(chunkID);
    ai_assert(chunkID == ASSBIN_CHUNK_AIMATERIAL);
    /*uint32_t size =*/ Read<uint32_t>(stream);

    mat->mNumAllocated = mat->mNumProperties = Read<unsigned int>(stream);
    if (mat->mNumProperties)
    {
        if (mat->mProperties)
        {
            delete[] mat->mProperties;
        }
        mat->mProperties = new aiMaterialProperty*[mat->mNumProperties];
        for (unsigned int i = 0; i < mat->mNumProperties;++i) {
            mat->mProperties[i] = new aiMaterialProperty();
            ReadBinaryMaterialProperty( stream, mat->mProperties[i]);
        }
    }
}

// -----------------------------------------------------------------------------------
void AssbinImporter::ReadBinaryNodeAnim(IOStream * stream, aiNodeAnim* nd) {
    uint32_t chunkID = Read<uint32_t>(stream);
    (void)(chunkID);
    ai_assert(chunkID == ASSBIN_CHUNK_AINODEANIM);
    /*uint32_t size =*/ Read<uint32_t>(stream);

    nd->mNodeName = Read<aiString>(stream);
    nd->mNumPositionKeys = Read<unsigned int>(stream);
    nd->mNumRotationKeys = Read<unsigned int>(stream);
    nd->mNumScalingKeys = Read<unsigned int>(stream);
    nd->mPreState = (aiAnimBehaviour)Read<unsigned int>(stream);
    nd->mPostState = (aiAnimBehaviour)Read<unsigned int>(stream);

    if (nd->mNumPositionKeys) {
        if (shortened) {
            ReadBounds(stream,nd->mPositionKeys,nd->mNumPositionKeys);

        } // else write as usual
        else {
            nd->mPositionKeys = new aiVectorKey[nd->mNumPositionKeys];
            ReadArray<aiVectorKey>(stream,nd->mPositionKeys,nd->mNumPositionKeys);
        }
    }
    if (nd->mNumRotationKeys) {
        if (shortened) {
            ReadBounds(stream,nd->mRotationKeys,nd->mNumRotationKeys);

        }  else {
            // else write as usual
            nd->mRotationKeys = new aiQuatKey[nd->mNumRotationKeys];
            ReadArray<aiQuatKey>(stream,nd->mRotationKeys,nd->mNumRotationKeys);
        }
    }
    if (nd->mNumScalingKeys) {
        if (shortened) {
            ReadBounds(stream,nd->mScalingKeys,nd->mNumScalingKeys);

        }  else {
            // else write as usual
            nd->mScalingKeys = new aiVectorKey[nd->mNumScalingKeys];
            ReadArray<aiVectorKey>(stream,nd->mScalingKeys,nd->mNumScalingKeys);
        }
    }
}

// -----------------------------------------------------------------------------------
void AssbinImporter::ReadBinaryAnim( IOStream * stream, aiAnimation* anim ) {
    uint32_t chunkID = Read<uint32_t>(stream);
    (void)(chunkID);
    ai_assert(chunkID == ASSBIN_CHUNK_AIANIMATION);
    /*uint32_t size =*/ Read<uint32_t>(stream);

    anim->mName = Read<aiString> (stream);
    anim->mDuration = Read<double> (stream);
    anim->mTicksPerSecond = Read<double> (stream);
    anim->mNumChannels = Read<unsigned int>(stream);

    if (anim->mNumChannels) {
        anim->mChannels = new aiNodeAnim*[ anim->mNumChannels ];
        for (unsigned int a = 0; a < anim->mNumChannels;++a) {
            anim->mChannels[a] = new aiNodeAnim();
            ReadBinaryNodeAnim(stream,anim->mChannels[a]);
        }
    }
}

// -----------------------------------------------------------------------------------
void AssbinImporter::ReadBinaryTexture(IOStream * stream, aiTexture* tex) {
    uint32_t chunkID = Read<uint32_t>(stream);
    (void)(chunkID);
    ai_assert(chunkID == ASSBIN_CHUNK_AITEXTURE);
    /*uint32_t size =*/ Read<uint32_t>(stream);

    tex->mWidth = Read<unsigned int>(stream);
    tex->mHeight = Read<unsigned int>(stream);
    stream->Read( tex->achFormatHint, sizeof(char), 4 );

    if(!shortened) {
        if (!tex->mHeight) {
            tex->pcData = new aiTexel[ tex->mWidth ];
            stream->Read(tex->pcData,1,tex->mWidth);
        } else {
            tex->pcData = new aiTexel[ tex->mWidth*tex->mHeight ];
            stream->Read(tex->pcData,1,tex->mWidth*tex->mHeight*4);
        }
    }
}

// -----------------------------------------------------------------------------------
void AssbinImporter::ReadBinaryLight( IOStream * stream, aiLight* l ) {
    uint32_t chunkID = Read<uint32_t>(stream);
    (void)(chunkID);
    ai_assert(chunkID == ASSBIN_CHUNK_AILIGHT);
    /*uint32_t size =*/ Read<uint32_t>(stream);

    l->mName = Read<aiString>(stream);
    l->mType = (aiLightSourceType)Read<unsigned int>(stream);

    if (l->mType != aiLightSource_DIRECTIONAL) {
        l->mAttenuationConstant = Read<float>(stream);
        l->mAttenuationLinear = Read<float>(stream);
        l->mAttenuationQuadratic = Read<float>(stream);
    }

    l->mColorDiffuse = Read<aiColor3D>(stream);
    l->mColorSpecular = Read<aiColor3D>(stream);
    l->mColorAmbient = Read<aiColor3D>(stream);

    if (l->mType == aiLightSource_SPOT) {
        l->mAngleInnerCone = Read<float>(stream);
        l->mAngleOuterCone = Read<float>(stream);
    }
}

// -----------------------------------------------------------------------------------
void AssbinImporter::ReadBinaryCamera( IOStream * stream, aiCamera* cam ) {
    uint32_t chunkID = Read<uint32_t>(stream);
    (void)(chunkID);
    ai_assert(chunkID == ASSBIN_CHUNK_AICAMERA);
    /*uint32_t size =*/ Read<uint32_t>(stream);

    cam->mName = Read<aiString>(stream);
    cam->mPosition = Read<aiVector3D>(stream);
    cam->mLookAt = Read<aiVector3D>(stream);
    cam->mUp = Read<aiVector3D>(stream);
    cam->mHorizontalFOV = Read<float>(stream);
    cam->mClipPlaneNear = Read<float>(stream);
    cam->mClipPlaneFar = Read<float>(stream);
    cam->mAspect = Read<float>(stream);
}

// -----------------------------------------------------------------------------------
void AssbinImporter::ReadBinaryScene( IOStream * stream, aiScene* scene ) {
    uint32_t chunkID = Read<uint32_t>(stream);
    (void)(chunkID);
    ai_assert(chunkID == ASSBIN_CHUNK_AISCENE);
    /*uint32_t size =*/ Read<uint32_t>(stream);

    scene->mFlags         = Read<unsigned int>(stream);
    scene->mNumMeshes     = Read<unsigned int>(stream);
    scene->mNumMaterials  = Read<unsigned int>(stream);
    scene->mNumAnimations = Read<unsigned int>(stream);
    scene->mNumTextures   = Read<unsigned int>(stream);
    scene->mNumLights     = Read<unsigned int>(stream);
    scene->mNumCameras    = Read<unsigned int>(stream);

    // Read node graph
    //scene->mRootNode = new aiNode[1];
    ReadBinaryNode( stream, &scene->mRootNode, (aiNode*)NULL );

    // Read all meshes
    if (scene->mNumMeshes) {
        scene->mMeshes = new aiMesh*[scene->mNumMeshes];
        for (unsigned int i = 0; i < scene->mNumMeshes;++i) {
            scene->mMeshes[i] = new aiMesh();
            ReadBinaryMesh( stream,scene->mMeshes[i]);
        }
    }

    // Read materials
    if (scene->mNumMaterials) {
        scene->mMaterials = new aiMaterial*[scene->mNumMaterials];
        for (unsigned int i = 0; i< scene->mNumMaterials; ++i) {
            scene->mMaterials[i] = new aiMaterial();
            ReadBinaryMaterial(stream,scene->mMaterials[i]);
        }
    }

    // Read all animations
    if (scene->mNumAnimations) {
        scene->mAnimations = new aiAnimation*[scene->mNumAnimations];
        for (unsigned int i = 0; i < scene->mNumAnimations;++i) {
            scene->mAnimations[i] = new aiAnimation();
            ReadBinaryAnim(stream,scene->mAnimations[i]);
        }
    }

    // Read all textures
    if (scene->mNumTextures) {
        scene->mTextures = new aiTexture*[scene->mNumTextures];
        for (unsigned int i = 0; i < scene->mNumTextures;++i) {
            scene->mTextures[i] = new aiTexture();
            ReadBinaryTexture(stream,scene->mTextures[i]);
        }
    }

    // Read lights
    if (scene->mNumLights) {
        scene->mLights = new aiLight*[scene->mNumLights];
        for (unsigned int i = 0; i < scene->mNumLights;++i) {
            scene->mLights[i] = new aiLight();
            ReadBinaryLight(stream,scene->mLights[i]);
        }
    }

    // Read cameras
    if (scene->mNumCameras) {
        scene->mCameras = new aiCamera*[scene->mNumCameras];
        for (unsigned int i = 0; i < scene->mNumCameras;++i) {
            scene->mCameras[i] = new aiCamera();
            ReadBinaryCamera(stream,scene->mCameras[i]);
        }
    }

}

// -----------------------------------------------------------------------------------
void AssbinImporter::InternReadFile( const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler ) {
    IOStream * stream = pIOHandler->Open(pFile,"rb");
    if (nullptr == stream) {
        return;
    }

    // signature
    stream->Seek( 44, aiOrigin_CUR ); 

    unsigned int versionMajor = Read<unsigned int>(stream);
    unsigned int versionMinor = Read<unsigned int>(stream);
    if (versionMinor != ASSBIN_VERSION_MINOR || versionMajor != ASSBIN_VERSION_MAJOR) {
        throw DeadlyImportError( "Invalid version, data format not compatible!" );
    }

    /*unsigned int versionRevision =*/ Read<unsigned int>(stream);
    /*unsigned int compileFlags =*/ Read<unsigned int>(stream);

    shortened = Read<uint16_t>(stream) > 0;
    compressed = Read<uint16_t>(stream) > 0;

    if (shortened)
        throw DeadlyImportError( "Shortened binaries are not supported!" );

    stream->Seek( 256, aiOrigin_CUR ); // original filename
    stream->Seek( 128, aiOrigin_CUR ); // options
    stream->Seek( 64, aiOrigin_CUR ); // padding

    if (compressed) {
        uLongf uncompressedSize = Read<uint32_t>(stream);
        uLongf compressedSize = static_cast<uLongf>(stream->FileSize() - stream->Tell());

        unsigned char * compressedData = new unsigned char[ compressedSize ];
        stream->Read( compressedData, 1, compressedSize );

        unsigned char * uncompressedData = new unsigned char[ uncompressedSize ];

        uncompress( uncompressedData, &uncompressedSize, compressedData, compressedSize );

        MemoryIOStream io( uncompressedData, uncompressedSize );

        ReadBinaryScene(&io,pScene);

        delete[] uncompressedData;
        delete[] compressedData;
    } else {
        ReadBinaryScene(stream,pScene);
    }

    pIOHandler->Close(stream);
}

#endif // !! ASSIMP_BUILD_NO_ASSBIN_IMPORTER
