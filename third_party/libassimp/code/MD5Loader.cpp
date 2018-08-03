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

/** @file  MD5Loader.cpp
 *  @brief Implementation of the MD5 importer class
 */


#ifndef ASSIMP_BUILD_NO_MD5_IMPORTER

// internal headers
#include "RemoveComments.h"
#include "MD5Loader.h"
#include "StringComparison.h"
#include "fast_atof.h"
#include "SkeletonMeshBuilder.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/IOSystem.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/importerdesc.h>
#include <memory>

using namespace Assimp;

// Minimum weight value. Weights inside [-n ... n] are ignored
#define AI_MD5_WEIGHT_EPSILON 1e-5f


static const aiImporterDesc desc = {
    "Doom 3 / MD5 Mesh Importer",
    "",
    "",
    "",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "md5mesh md5camera md5anim"
};

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
MD5Importer::MD5Importer()
    : mIOHandler()
    , mBuffer()
    , fileSize()
    , iLineNumber()
    , pScene()
    , pIOHandler()
    , bHadMD5Mesh()
    , bHadMD5Anim()
    , bHadMD5Camera()
    , configNoAutoLoad (false)
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
MD5Importer::~MD5Importer()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool MD5Importer::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    const std::string extension = GetExtension(pFile);

    if (extension == "md5anim" || extension == "md5mesh" || extension == "md5camera")
        return true;
    else if (!extension.length() || checkSig)   {
        if (!pIOHandler) {
            return true;
        }
        const char* tokens[] = {"MD5Version"};
        return SearchFileHeaderForToken(pIOHandler,pFile,tokens,1);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Get list of all supported extensions
const aiImporterDesc* MD5Importer::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Setup import properties
void MD5Importer::SetupProperties(const Importer* pImp)
{
    // AI_CONFIG_IMPORT_MD5_NO_ANIM_AUTOLOAD
    configNoAutoLoad = (0 !=  pImp->GetPropertyInteger(AI_CONFIG_IMPORT_MD5_NO_ANIM_AUTOLOAD,0));
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void MD5Importer::InternReadFile( const std::string& pFile,
                                 aiScene* _pScene, IOSystem* _pIOHandler)
{
    pIOHandler = _pIOHandler;
    pScene     = _pScene;
    bHadMD5Mesh = bHadMD5Anim = bHadMD5Camera = false;

    // remove the file extension
    const std::string::size_type pos = pFile.find_last_of('.');
    mFile = (std::string::npos == pos ? pFile : pFile.substr(0,pos+1));

    const std::string extension = GetExtension(pFile);
    try {
        if (extension == "md5camera") {
            LoadMD5CameraFile();
        }
        else if (configNoAutoLoad || extension == "md5anim") {
            // determine file extension and process just *one* file
            if (extension.length() == 0) {
                throw DeadlyImportError("Failure, need file extension to determine MD5 part type");
            }
            if (extension == "md5anim") {
                LoadMD5AnimFile();
            }
            else if (extension == "md5mesh") {
                LoadMD5MeshFile();
            }
        }
        else {
            LoadMD5MeshFile();
            LoadMD5AnimFile();
        }
    }
    catch ( ... ) { // std::exception, Assimp::DeadlyImportError
        UnloadFileFromMemory();
        throw;
    }

    // make sure we have at least one file
    if (!bHadMD5Mesh && !bHadMD5Anim && !bHadMD5Camera) {
        throw DeadlyImportError("Failed to read valid contents out of this MD5* file");
    }

    // Now rotate the whole scene 90 degrees around the x axis to match our internal coordinate system
    pScene->mRootNode->mTransformation = aiMatrix4x4(1.f,0.f,0.f,0.f,
        0.f,0.f,1.f,0.f,0.f,-1.f,0.f,0.f,0.f,0.f,0.f,1.f);

    // the output scene wouldn't pass the validation without this flag
    if (!bHadMD5Mesh) {
        pScene->mFlags |= AI_SCENE_FLAGS_INCOMPLETE;
    }

    // clean the instance -- the BaseImporter instance may be reused later.
    UnloadFileFromMemory();
}

// ------------------------------------------------------------------------------------------------
// Load a file into a memory buffer
void MD5Importer::LoadFileIntoMemory (IOStream* file)
{
    // unload the previous buffer, if any
    UnloadFileFromMemory();

    ai_assert(NULL != file);
    fileSize = (unsigned int)file->FileSize();
    ai_assert(fileSize);

    // allocate storage and copy the contents of the file to a memory buffer
    mBuffer = new char[fileSize+1];
    file->Read( (void*)mBuffer, 1, fileSize);
    iLineNumber = 1;

    // append a terminal 0
    mBuffer[fileSize] = '\0';

    // now remove all line comments from the file
    CommentRemover::RemoveLineComments("//",mBuffer,' ');
}

// ------------------------------------------------------------------------------------------------
// Unload the current memory buffer
void MD5Importer::UnloadFileFromMemory ()
{
    // delete the file buffer
    delete[] mBuffer;
    mBuffer = NULL;
    fileSize = 0;
}

// ------------------------------------------------------------------------------------------------
// Build unique vertices
void MD5Importer::MakeDataUnique (MD5::MeshDesc& meshSrc)
{
    std::vector<bool> abHad(meshSrc.mVertices.size(),false);

    // allocate enough storage to keep the output structures
    const unsigned int iNewNum = static_cast<unsigned int>(meshSrc.mFaces.size()*3);
    unsigned int iNewIndex = static_cast<unsigned int>(meshSrc.mVertices.size());
    meshSrc.mVertices.resize(iNewNum);

    // try to guess how much storage we'll need for new weights
    const float fWeightsPerVert = meshSrc.mWeights.size() / (float)iNewIndex;
    const unsigned int guess = (unsigned int)(fWeightsPerVert*iNewNum);
    meshSrc.mWeights.reserve(guess + (guess >> 3)); // + 12.5% as buffer

    for (FaceList::const_iterator iter = meshSrc.mFaces.begin(),iterEnd = meshSrc.mFaces.end();iter != iterEnd;++iter){
        const aiFace& face = *iter;
        for (unsigned int i = 0; i < 3;++i) {
            if (face.mIndices[0] >= meshSrc.mVertices.size()) {
                throw DeadlyImportError("MD5MESH: Invalid vertex index");
            }

            if (abHad[face.mIndices[i]])    {
                // generate a new vertex
                meshSrc.mVertices[iNewIndex] = meshSrc.mVertices[face.mIndices[i]];
                face.mIndices[i] = iNewIndex++;
            }
            else abHad[face.mIndices[i]] = true;
        }
        // swap face order
        std::swap(face.mIndices[0],face.mIndices[2]);
    }
}

// ------------------------------------------------------------------------------------------------
// Recursive node graph construction from a MD5MESH
void MD5Importer::AttachChilds_Mesh(int iParentID,aiNode* piParent, BoneList& bones)
{
    ai_assert(NULL != piParent && !piParent->mNumChildren);

    // First find out how many children we'll have
    for (int i = 0; i < (int)bones.size();++i)  {
        if (iParentID != i && bones[i].mParentIndex == iParentID)   {
            ++piParent->mNumChildren;
        }
    }
    if (piParent->mNumChildren) {
        piParent->mChildren = new aiNode*[piParent->mNumChildren];
        for (int i = 0; i < (int)bones.size();++i)  {
            // (avoid infinite recursion)
            if (iParentID != i && bones[i].mParentIndex == iParentID)   {
                aiNode* pc;
                // setup a new node
                *piParent->mChildren++ = pc = new aiNode();
                pc->mName = aiString(bones[i].mName);
                pc->mParent = piParent;

                // get the transformation matrix from rotation and translational components
                aiQuaternion quat;
                MD5::ConvertQuaternion ( bones[i].mRotationQuat, quat );

                bones[i].mTransform = aiMatrix4x4 ( quat.GetMatrix());
                bones[i].mTransform.a4 = bones[i].mPositionXYZ.x;
                bones[i].mTransform.b4 = bones[i].mPositionXYZ.y;
                bones[i].mTransform.c4 = bones[i].mPositionXYZ.z;

                // store it for later use
                pc->mTransformation = bones[i].mInvTransform = bones[i].mTransform;
                bones[i].mInvTransform.Inverse();

                // the transformations for each bone are absolute, so we need to multiply them
                // with the inverse of the absolute matrix of the parent joint
                if (-1 != iParentID)    {
                    pc->mTransformation = bones[iParentID].mInvTransform * pc->mTransformation;
                }

                // add children to this node, too
                AttachChilds_Mesh( i, pc, bones);
            }
        }
        // undo offset computations
        piParent->mChildren -= piParent->mNumChildren;
    }
}

// ------------------------------------------------------------------------------------------------
// Recursive node graph construction from a MD5ANIM
void MD5Importer::AttachChilds_Anim(int iParentID,aiNode* piParent, AnimBoneList& bones,const aiNodeAnim** node_anims)
{
    ai_assert(NULL != piParent && !piParent->mNumChildren);

    // First find out how many children we'll have
    for (int i = 0; i < (int)bones.size();++i)  {
        if (iParentID != i && bones[i].mParentIndex == iParentID)   {
            ++piParent->mNumChildren;
        }
    }
    if (piParent->mNumChildren) {
        piParent->mChildren = new aiNode*[piParent->mNumChildren];
        for (int i = 0; i < (int)bones.size();++i)  {
            // (avoid infinite recursion)
            if (iParentID != i && bones[i].mParentIndex == iParentID)
            {
                aiNode* pc;
                // setup a new node
                *piParent->mChildren++ = pc = new aiNode();
                pc->mName = aiString(bones[i].mName);
                pc->mParent = piParent;

                // get the corresponding animation channel and its first frame
                const aiNodeAnim** cur = node_anims;
                while ((**cur).mNodeName != pc->mName)++cur;

                aiMatrix4x4::Translation((**cur).mPositionKeys[0].mValue,pc->mTransformation);
                pc->mTransformation = pc->mTransformation * aiMatrix4x4((**cur).mRotationKeys[0].mValue.GetMatrix()) ;

                // add children to this node, too
                AttachChilds_Anim( i, pc, bones,node_anims);
            }
        }
        // undo offset computations
        piParent->mChildren -= piParent->mNumChildren;
    }
}

// ------------------------------------------------------------------------------------------------
// Load a MD5MESH file
void MD5Importer::LoadMD5MeshFile ()
{
    std::string pFile = mFile + "md5mesh";
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile, "rb"));

    // Check whether we can read from the file
    if( file.get() == NULL || !file->FileSize())    {
        DefaultLogger::get()->warn("Failed to access MD5MESH file: " + pFile);
        return;
    }
    bHadMD5Mesh = true;
    LoadFileIntoMemory(file.get());

    // now construct a parser and parse the file
    MD5::MD5Parser parser(mBuffer,fileSize);

    // load the mesh information from it
    MD5::MD5MeshParser meshParser(parser.mSections);

    // create the bone hierarchy - first the root node and dummy nodes for all meshes
    pScene->mRootNode = new aiNode("<MD5_Root>");
    pScene->mRootNode->mNumChildren = 2;
    pScene->mRootNode->mChildren = new aiNode*[2];

    // build the hierarchy from the MD5MESH file
    aiNode* pcNode = pScene->mRootNode->mChildren[1] = new aiNode();
    pcNode->mName.Set("<MD5_Hierarchy>");
    pcNode->mParent = pScene->mRootNode;
    AttachChilds_Mesh(-1,pcNode,meshParser.mJoints);

    pcNode = pScene->mRootNode->mChildren[0] = new aiNode();
    pcNode->mName.Set("<MD5_Mesh>");
    pcNode->mParent = pScene->mRootNode;

#if 0
    if (pScene->mRootNode->mChildren[1]->mNumChildren) /* start at the right hierarchy level */
        SkeletonMeshBuilder skeleton_maker(pScene,pScene->mRootNode->mChildren[1]->mChildren[0]);
#else

    // FIX: MD5 files exported from Blender can have empty meshes
    for (std::vector<MD5::MeshDesc>::const_iterator it  = meshParser.mMeshes.begin(),end = meshParser.mMeshes.end(); it != end;++it) {
        if (!(*it).mFaces.empty() && !(*it).mVertices.empty())
            ++pScene->mNumMaterials;
    }

    // generate all meshes
    pScene->mNumMeshes = pScene->mNumMaterials;
    pScene->mMeshes = new aiMesh*[pScene->mNumMeshes];
    pScene->mMaterials = new aiMaterial*[pScene->mNumMeshes];

    //  storage for node mesh indices
    pcNode->mNumMeshes = pScene->mNumMeshes;
    pcNode->mMeshes = new unsigned int[pcNode->mNumMeshes];
    for (unsigned int m = 0; m < pcNode->mNumMeshes;++m)
        pcNode->mMeshes[m] = m;

    unsigned int n = 0;
    for (std::vector<MD5::MeshDesc>::iterator it  = meshParser.mMeshes.begin(),end = meshParser.mMeshes.end(); it != end;++it) {
        MD5::MeshDesc& meshSrc = *it;
        if (meshSrc.mFaces.empty() || meshSrc.mVertices.empty())
            continue;

        aiMesh* mesh = pScene->mMeshes[n] = new aiMesh();
        mesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;

        // generate unique vertices in our internal verbose format
        MakeDataUnique(meshSrc);

        mesh->mNumVertices = (unsigned int) meshSrc.mVertices.size();
        mesh->mVertices = new aiVector3D[mesh->mNumVertices];
        mesh->mTextureCoords[0] = new aiVector3D[mesh->mNumVertices];
        mesh->mNumUVComponents[0] = 2;

        // copy texture coordinates
        aiVector3D* pv = mesh->mTextureCoords[0];
        for (MD5::VertexList::const_iterator iter =  meshSrc.mVertices.begin();iter != meshSrc.mVertices.end();++iter,++pv) {
            pv->x = (*iter).mUV.x;
            pv->y = 1.0f-(*iter).mUV.y; // D3D to OpenGL
            pv->z = 0.0f;
        }

        // sort all bone weights - per bone
        unsigned int* piCount = new unsigned int[meshParser.mJoints.size()];
        ::memset(piCount,0,sizeof(unsigned int)*meshParser.mJoints.size());

        for (MD5::VertexList::const_iterator iter =  meshSrc.mVertices.begin();iter != meshSrc.mVertices.end();++iter,++pv) {
            for (unsigned int jub = (*iter).mFirstWeight, w = jub; w < jub + (*iter).mNumWeights;++w)
            {
                MD5::WeightDesc& desc = meshSrc.mWeights[w];
                /* FIX for some invalid exporters */
                if (!(desc.mWeight < AI_MD5_WEIGHT_EPSILON && desc.mWeight >= -AI_MD5_WEIGHT_EPSILON ))
                    ++piCount[desc.mBone];
            }
        }

        // check how many we will need
        for (unsigned int p = 0; p < meshParser.mJoints.size();++p)
            if (piCount[p])mesh->mNumBones++;

        if (mesh->mNumBones) // just for safety
        {
            mesh->mBones = new aiBone*[mesh->mNumBones];
            for (unsigned int q = 0,h = 0; q < meshParser.mJoints.size();++q)
            {
                if (!piCount[q])continue;
                aiBone* p = mesh->mBones[h] = new aiBone();
                p->mNumWeights = piCount[q];
                p->mWeights = new aiVertexWeight[p->mNumWeights];
                p->mName = aiString(meshParser.mJoints[q].mName);
                p->mOffsetMatrix = meshParser.mJoints[q].mInvTransform;

                // store the index for later use
                MD5::BoneDesc& boneSrc = meshParser.mJoints[q];
                boneSrc.mMap = h++;

                // compute w-component of quaternion
                MD5::ConvertQuaternion( boneSrc.mRotationQuat, boneSrc.mRotationQuatConverted );
            }

            //unsigned int g = 0;
            pv = mesh->mVertices;
            for (MD5::VertexList::const_iterator iter =  meshSrc.mVertices.begin();iter != meshSrc.mVertices.end();++iter,++pv) {
                // compute the final vertex position from all single weights
                *pv = aiVector3D();

                // there are models which have weights which don't sum to 1 ...
                ai_real fSum = 0.0;
                for (unsigned int jub = (*iter).mFirstWeight, w = jub; w < jub + (*iter).mNumWeights;++w)
                    fSum += meshSrc.mWeights[w].mWeight;
                if (!fSum) {
                    DefaultLogger::get()->error("MD5MESH: The sum of all vertex bone weights is 0");
                    continue;
                }

                // process bone weights
                for (unsigned int jub = (*iter).mFirstWeight, w = jub; w < jub + (*iter).mNumWeights;++w)   {
                    if (w >= meshSrc.mWeights.size())
                        throw DeadlyImportError("MD5MESH: Invalid weight index");

                    MD5::WeightDesc& desc = meshSrc.mWeights[w];
                    if ( desc.mWeight < AI_MD5_WEIGHT_EPSILON && desc.mWeight >= -AI_MD5_WEIGHT_EPSILON) {
                        continue;
                    }

                    const ai_real fNewWeight = desc.mWeight / fSum;

                    // transform the local position into worldspace
                    MD5::BoneDesc& boneSrc = meshParser.mJoints[desc.mBone];
                    const aiVector3D v = boneSrc.mRotationQuatConverted.Rotate (desc.vOffsetPosition);

                    // use the original weight to compute the vertex position
                    // (some MD5s seem to depend on the invalid weight values ...)
                    *pv += ((boneSrc.mPositionXYZ+v)* (ai_real)desc.mWeight);

                    aiBone* bone = mesh->mBones[boneSrc.mMap];
                    *bone->mWeights++ = aiVertexWeight((unsigned int)(pv-mesh->mVertices),fNewWeight);
                }
            }

            // undo our nice offset tricks ...
            for (unsigned int p = 0; p < mesh->mNumBones;++p) {
                mesh->mBones[p]->mWeights -= mesh->mBones[p]->mNumWeights;
            }
        }

        delete[] piCount;

        // now setup all faces - we can directly copy the list
        // (however, take care that the aiFace destructor doesn't delete the mIndices array)
        mesh->mNumFaces = (unsigned int)meshSrc.mFaces.size();
        mesh->mFaces = new aiFace[mesh->mNumFaces];
        for (unsigned int c = 0; c < mesh->mNumFaces;++c)   {
            mesh->mFaces[c].mNumIndices = 3;
            mesh->mFaces[c].mIndices = meshSrc.mFaces[c].mIndices;
            meshSrc.mFaces[c].mIndices = NULL;
        }

        // generate a material for the mesh
        aiMaterial* mat = new aiMaterial();
        pScene->mMaterials[n] = mat;

        // insert the typical doom3 textures:
        // nnn_local.tga  - normal map
        // nnn_h.tga      - height map
        // nnn_s.tga      - specular map
        // nnn_d.tga      - diffuse map
        if (meshSrc.mShader.length && !strchr(meshSrc.mShader.data,'.')) {

            aiString temp(meshSrc.mShader);
            temp.Append("_local.tga");
            mat->AddProperty(&temp,AI_MATKEY_TEXTURE_NORMALS(0));

            temp =  aiString(meshSrc.mShader);
            temp.Append("_s.tga");
            mat->AddProperty(&temp,AI_MATKEY_TEXTURE_SPECULAR(0));

            temp =  aiString(meshSrc.mShader);
            temp.Append("_d.tga");
            mat->AddProperty(&temp,AI_MATKEY_TEXTURE_DIFFUSE(0));

            temp =  aiString(meshSrc.mShader);
            temp.Append("_h.tga");
            mat->AddProperty(&temp,AI_MATKEY_TEXTURE_HEIGHT(0));

            // set this also as material name
            mat->AddProperty(&meshSrc.mShader,AI_MATKEY_NAME);
        }
        else mat->AddProperty(&meshSrc.mShader,AI_MATKEY_TEXTURE_DIFFUSE(0));
        mesh->mMaterialIndex = n++;
    }
#endif
}

// ------------------------------------------------------------------------------------------------
// Load an MD5ANIM file
void MD5Importer::LoadMD5AnimFile ()
{
    std::string pFile = mFile + "md5anim";
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile, "rb"));

    // Check whether we can read from the file
    if( !file.get() || !file->FileSize())   {
        DefaultLogger::get()->warn("Failed to read MD5ANIM file: " + pFile);
        return;
    }
    LoadFileIntoMemory(file.get());

    // parse the basic file structure
    MD5::MD5Parser parser(mBuffer,fileSize);

    // load the animation information from the parse tree
    MD5::MD5AnimParser animParser(parser.mSections);

    // generate and fill the output animation
    if (animParser.mAnimatedBones.empty() || animParser.mFrames.empty() ||
        animParser.mBaseFrames.size() != animParser.mAnimatedBones.size())  {

        DefaultLogger::get()->error("MD5ANIM: No frames or animated bones loaded");
    }
    else {
        bHadMD5Anim = true;

        pScene->mAnimations = new aiAnimation*[pScene->mNumAnimations = 1];
        aiAnimation* anim = pScene->mAnimations[0] = new aiAnimation();
        anim->mNumChannels = (unsigned int)animParser.mAnimatedBones.size();
        anim->mChannels = new aiNodeAnim*[anim->mNumChannels];
        for (unsigned int i = 0; i < anim->mNumChannels;++i)    {
            aiNodeAnim* node = anim->mChannels[i] = new aiNodeAnim();
            node->mNodeName = aiString( animParser.mAnimatedBones[i].mName );

            // allocate storage for the keyframes
            node->mPositionKeys = new aiVectorKey[animParser.mFrames.size()];
            node->mRotationKeys = new aiQuatKey[animParser.mFrames.size()];
        }

        // 1 tick == 1 frame
        anim->mTicksPerSecond = animParser.fFrameRate;

        for (FrameList::const_iterator iter = animParser.mFrames.begin(), iterEnd = animParser.mFrames.end();iter != iterEnd;++iter){
            double dTime = (double)(*iter).iIndex;
            aiNodeAnim** pcAnimNode = anim->mChannels;
            if (!(*iter).mValues.empty() || iter == animParser.mFrames.begin()) /* be sure we have at least one frame */
            {
                // now process all values in there ... read all joints
                MD5::BaseFrameDesc* pcBaseFrame = &animParser.mBaseFrames[0];
                for (AnimBoneList::const_iterator iter2 = animParser.mAnimatedBones.begin(); iter2 != animParser.mAnimatedBones.end();++iter2,
                    ++pcAnimNode,++pcBaseFrame)
                {
                    if((*iter2).iFirstKeyIndex >= (*iter).mValues.size()) {

                        // Allow for empty frames
                        if ((*iter2).iFlags != 0) {
                            throw DeadlyImportError("MD5: Keyframe index is out of range");

                        }
                        continue;
                    }
                    const float* fpCur = &(*iter).mValues[(*iter2).iFirstKeyIndex];
                    aiNodeAnim* pcCurAnimBone = *pcAnimNode;

                    aiVectorKey* vKey = &pcCurAnimBone->mPositionKeys[pcCurAnimBone->mNumPositionKeys++];
                    aiQuatKey* qKey = &pcCurAnimBone->mRotationKeys  [pcCurAnimBone->mNumRotationKeys++];
                    aiVector3D vTemp;

                    // translational component
                    for (unsigned int i = 0; i < 3; ++i) {
                        if ((*iter2).iFlags & (1u << i)) {
                            vKey->mValue[i] =  *fpCur++;
                        }
                        else vKey->mValue[i] = pcBaseFrame->vPositionXYZ[i];
                    }

                    // orientation component
                    for (unsigned int i = 0; i < 3; ++i) {
                        if ((*iter2).iFlags & (8u << i)) {
                            vTemp[i] =  *fpCur++;
                        }
                        else vTemp[i] = pcBaseFrame->vRotationQuat[i];
                    }

                    MD5::ConvertQuaternion(vTemp, qKey->mValue);
                    qKey->mTime = vKey->mTime = dTime;
                }
            }

            // compute the duration of the animation
            anim->mDuration = std::max(dTime,anim->mDuration);
        }

        // If we didn't build the hierarchy yet (== we didn't load a MD5MESH),
        // construct it now from the data given in the MD5ANIM.
        if (!pScene->mRootNode) {
            pScene->mRootNode = new aiNode();
            pScene->mRootNode->mName.Set("<MD5_Hierarchy>");

            AttachChilds_Anim(-1,pScene->mRootNode,animParser.mAnimatedBones,(const aiNodeAnim**)anim->mChannels);

            // Call SkeletonMeshBuilder to construct a mesh to represent the shape
            if (pScene->mRootNode->mNumChildren) {
                SkeletonMeshBuilder skeleton_maker(pScene,pScene->mRootNode->mChildren[0]);
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Load an MD5CAMERA file
void MD5Importer::LoadMD5CameraFile ()
{
    std::string pFile = mFile + "md5camera";
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile, "rb"));

    // Check whether we can read from the file
    if( !file.get() || !file->FileSize())   {
        throw DeadlyImportError("Failed to read MD5CAMERA file: " + pFile);
    }
    bHadMD5Camera = true;
    LoadFileIntoMemory(file.get());

    // parse the basic file structure
    MD5::MD5Parser parser(mBuffer,fileSize);

    // load the camera animation data from the parse tree
    MD5::MD5CameraParser cameraParser(parser.mSections);

    if (cameraParser.frames.empty()) {
        throw DeadlyImportError("MD5CAMERA: No frames parsed");
    }

    std::vector<unsigned int>& cuts = cameraParser.cuts;
    std::vector<MD5::CameraAnimFrameDesc>& frames = cameraParser.frames;

    // Construct output graph - a simple root with a dummy child.
    // The root node performs the coordinate system conversion
    aiNode* root = pScene->mRootNode = new aiNode("<MD5CameraRoot>");
    root->mChildren = new aiNode*[root->mNumChildren = 1];
    root->mChildren[0] = new aiNode("<MD5Camera>");
    root->mChildren[0]->mParent = root;

    // ... but with one camera assigned to it
    pScene->mCameras = new aiCamera*[pScene->mNumCameras = 1];
    aiCamera* cam = pScene->mCameras[0] = new aiCamera();
    cam->mName = "<MD5Camera>";

    // FIXME: Fov is currently set to the first frame's value
    cam->mHorizontalFOV = AI_DEG_TO_RAD( frames.front().fFOV );

    // every cut is written to a separate aiAnimation
    if (!cuts.size()) {
        cuts.push_back(0);
        cuts.push_back(static_cast<unsigned int>(frames.size()-1));
    }
    else {
        cuts.insert(cuts.begin(),0);

        if (cuts.back() < frames.size()-1)
            cuts.push_back(static_cast<unsigned int>(frames.size()-1));
    }

    pScene->mNumAnimations = static_cast<unsigned int>(cuts.size()-1);
    aiAnimation** tmp = pScene->mAnimations = new aiAnimation*[pScene->mNumAnimations];
    for (std::vector<unsigned int>::const_iterator it = cuts.begin(); it != cuts.end()-1; ++it) {

        aiAnimation* anim = *tmp++ = new aiAnimation();
        anim->mName.length = ::ai_snprintf(anim->mName.data, MAXLEN, "anim%u_from_%u_to_%u",(unsigned int)(it-cuts.begin()),(*it),*(it+1));

        anim->mTicksPerSecond = cameraParser.fFrameRate;
        anim->mChannels = new aiNodeAnim*[anim->mNumChannels = 1];
        aiNodeAnim* nd  = anim->mChannels[0] = new aiNodeAnim();
        nd->mNodeName.Set("<MD5Camera>");

        nd->mNumPositionKeys = nd->mNumRotationKeys = *(it+1) - (*it);
        nd->mPositionKeys = new aiVectorKey[nd->mNumPositionKeys];
        nd->mRotationKeys = new aiQuatKey  [nd->mNumRotationKeys];
        for (unsigned int i = 0; i < nd->mNumPositionKeys; ++i) {

            nd->mPositionKeys[i].mValue = frames[*it+i].vPositionXYZ;
            MD5::ConvertQuaternion(frames[*it+i].vRotationQuat,nd->mRotationKeys[i].mValue);
            nd->mRotationKeys[i].mTime = nd->mPositionKeys[i].mTime = *it+i;
        }
    }
}

#endif // !! ASSIMP_BUILD_NO_MD5_IMPORTER
