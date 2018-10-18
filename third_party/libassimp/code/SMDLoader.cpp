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

/** @file  SMDLoader.cpp
 *  @brief Implementation of the SMD importer class
 */


#ifndef ASSIMP_BUILD_NO_SMD_IMPORTER

// internal headers
#include "SMDLoader.h"
#include <assimp/fast_atof.h>
#include <assimp/SkeletonMeshBuilder.h>
#include <assimp/Importer.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/importerdesc.h>
#include <memory>

using namespace Assimp;

static const aiImporterDesc desc = {
    "Valve SMD Importer",
    "",
    "",
    "",
    aiImporterFlags_SupportTextFlavour,
    0,
    0,
    0,
    0,
    "smd vta"
};

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
SMDImporter::SMDImporter()
: configFrameID(),
mBuffer(),
pScene( nullptr ),
iFileSize( 0 ),
iSmallestFrame( -1 ),
dLengthOfAnim( 0.0 ),
bHasUVs(false ),
iLineNumber(-1) {
    // empty
}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
SMDImporter::~SMDImporter() {
    // empty
}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool SMDImporter::CanRead( const std::string& pFile, IOSystem* /*pIOHandler*/, bool) const
{
    // fixme: auto format detection
    return SimpleExtensionCheck(pFile,"smd","vta");
}

// ------------------------------------------------------------------------------------------------
// Get a list of all supported file extensions
const aiImporterDesc* SMDImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Setup configuration properties
void SMDImporter::SetupProperties(const Importer* pImp)
{
    // The
    // AI_CONFIG_IMPORT_SMD_KEYFRAME option overrides the
    // AI_CONFIG_IMPORT_GLOBAL_KEYFRAME option.
    configFrameID = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_SMD_KEYFRAME,-1);
    if(static_cast<unsigned int>(-1) == configFrameID)  {
        configFrameID = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_GLOBAL_KEYFRAME,0);
    }
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void SMDImporter::InternReadFile( const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler)
{
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile, "rb"));

    // Check whether we can read from the file
    if( file.get() == NULL) {
        throw DeadlyImportError( "Failed to open SMD/VTA file " + pFile + ".");
    }

    iFileSize = (unsigned int)file->FileSize();

    // Allocate storage and copy the contents of the file to a memory buffer
    this->pScene = pScene;

    mBuffer.resize( iFileSize + 1 );
    TextFileToBuffer(file.get(), mBuffer );

    iSmallestFrame = (1 << 31);
    bHasUVs = true;
    iLineNumber = 1;

    // Reserve enough space for ... hm ... 10 textures
    aszTextures.reserve(10);

    // Reserve enough space for ... hm ... 1000 triangles
    asTriangles.reserve(1000);

    // Reserve enough space for ... hm ... 20 bones
    asBones.reserve(20);


    // parse the file ...
    ParseFile();

    // If there are no triangles it seems to be an animation SMD,
    // containing only the animation skeleton.
    if (asTriangles.empty())
    {
        if (asBones.empty())
        {
            throw DeadlyImportError("SMD: No triangles and no bones have "
                "been found in the file. This file seems to be invalid.");
        }

        // Set the flag in the scene structure which indicates
        // that there is nothing than an animation skeleton
        pScene->mFlags |= AI_SCENE_FLAGS_INCOMPLETE;
    }

    if (!asBones.empty())
    {
        // Check whether all bones have been initialized
        for (std::vector<SMD::Bone>::const_iterator
            i =  asBones.begin();
            i != asBones.end();++i)
        {
            if (!(*i).mName.length())
            {
                ASSIMP_LOG_WARN("SMD: Not all bones have been initialized");
                break;
            }
        }

        // now fix invalid time values and make sure the animation starts at frame 0
        FixTimeValues();

        // compute absolute bone transformation matrices
    //  ComputeAbsoluteBoneTransformations();
    }

    if (!(pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
    {
        // create output meshes
        CreateOutputMeshes();

        // build an output material list
        CreateOutputMaterials();
    }

    // build the output animation
    CreateOutputAnimations();

    // build output nodes (bones are added as empty dummy nodes)
    CreateOutputNodes();

    if (pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        SkeletonMeshBuilder skeleton(pScene);
    }
}
// ------------------------------------------------------------------------------------------------
// Write an error message with line number to the log file
void SMDImporter::LogErrorNoThrow(const char* msg)
{
    char szTemp[1024];
    ai_snprintf(szTemp,1024,"Line %u: %s",iLineNumber,msg);
    DefaultLogger::get()->error(szTemp);
}

// ------------------------------------------------------------------------------------------------
// Write a warning with line number to the log file
void SMDImporter::LogWarning(const char* msg)
{
    char szTemp[1024];
    ai_assert(strlen(msg) < 1000);
    ai_snprintf(szTemp,1024,"Line %u: %s",iLineNumber,msg);
    ASSIMP_LOG_WARN(szTemp);
}

// ------------------------------------------------------------------------------------------------
// Fix invalid time values in the file
void SMDImporter::FixTimeValues()
{
    double dDelta = (double)iSmallestFrame;
    double dMax = 0.0f;
    for (std::vector<SMD::Bone>::iterator
        iBone =  asBones.begin();
        iBone != asBones.end();++iBone)
    {
        for (std::vector<SMD::Bone::Animation::MatrixKey>::iterator
            iKey =  (*iBone).sAnim.asKeys.begin();
            iKey != (*iBone).sAnim.asKeys.end();++iKey)
        {
            (*iKey).dTime -= dDelta;
            dMax = std::max(dMax, (*iKey).dTime);
        }
    }
    dLengthOfAnim = dMax;
}

// ------------------------------------------------------------------------------------------------
// create output meshes
void SMDImporter::CreateOutputMeshes()
{
    if (aszTextures.empty())
        aszTextures.push_back(std::string());

    // we need to sort all faces by their material index
    // in opposition to other loaders we can be sure that each
    // material is at least used once.
    pScene->mNumMeshes = (unsigned int) aszTextures.size();
    pScene->mMeshes = new aiMesh*[pScene->mNumMeshes];

    typedef std::vector<unsigned int> FaceList;
    FaceList* aaiFaces = new FaceList[pScene->mNumMeshes];

    // approximate the space that will be required
    unsigned int iNum = (unsigned int)asTriangles.size() / pScene->mNumMeshes;
    iNum += iNum >> 1;
    for (unsigned int i = 0; i < pScene->mNumMeshes;++i)
        aaiFaces[i].reserve(iNum);


    // collect all faces
    iNum = 0;
    for (std::vector<SMD::Face>::const_iterator
        iFace =  asTriangles.begin();
        iFace != asTriangles.end();++iFace,++iNum)
    {
        if (UINT_MAX == (*iFace).iTexture)aaiFaces[(*iFace).iTexture].push_back( 0 );
        else if ((*iFace).iTexture >= aszTextures.size())
        {
            ASSIMP_LOG_INFO("[SMD/VTA] Material index overflow in face");
            aaiFaces[(*iFace).iTexture].push_back((unsigned int)aszTextures.size()-1);
        }
        else aaiFaces[(*iFace).iTexture].push_back(iNum);
    }

    // now create the output meshes
    for (unsigned int i = 0; i < pScene->mNumMeshes;++i)
    {
        aiMesh*& pcMesh = pScene->mMeshes[i] = new aiMesh();
        ai_assert(!aaiFaces[i].empty()); // should not be empty ...

        pcMesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
        pcMesh->mNumVertices = (unsigned int)aaiFaces[i].size()*3;
        pcMesh->mNumFaces = (unsigned int)aaiFaces[i].size();
        pcMesh->mMaterialIndex = i;

        // storage for bones
        typedef std::pair<unsigned int,float> TempWeightListEntry;
        typedef std::vector< TempWeightListEntry > TempBoneWeightList;

        TempBoneWeightList* aaiBones = new TempBoneWeightList[asBones.size()]();

        // try to reserve enough memory without wasting too much
        for (unsigned int iBone = 0; iBone < asBones.size();++iBone)
        {
            aaiBones[iBone].reserve(pcMesh->mNumVertices/asBones.size());
        }

        // allocate storage
        pcMesh->mFaces = new aiFace[pcMesh->mNumFaces];
        aiVector3D* pcNormals = pcMesh->mNormals = new aiVector3D[pcMesh->mNumVertices];
        aiVector3D* pcVerts = pcMesh->mVertices = new aiVector3D[pcMesh->mNumVertices];

        aiVector3D* pcUVs = NULL;
        if (bHasUVs)
        {
            pcUVs = pcMesh->mTextureCoords[0] = new aiVector3D[pcMesh->mNumVertices];
            pcMesh->mNumUVComponents[0] = 2;
        }

        iNum = 0;
        for (unsigned int iFace = 0; iFace < pcMesh->mNumFaces;++iFace)
        {
            pcMesh->mFaces[iFace].mIndices = new unsigned int[3];
            pcMesh->mFaces[iFace].mNumIndices = 3;

            // fill the vertices
            unsigned int iSrcFace = aaiFaces[i][iFace];
            SMD::Face& face = asTriangles[iSrcFace];

            *pcVerts++ = face.avVertices[0].pos;
            *pcVerts++ = face.avVertices[1].pos;
            *pcVerts++ = face.avVertices[2].pos;

            // fill the normals
            *pcNormals++ = face.avVertices[0].nor;
            *pcNormals++ = face.avVertices[1].nor;
            *pcNormals++ = face.avVertices[2].nor;

            // fill the texture coordinates
            if (pcUVs)
            {
                *pcUVs++ = face.avVertices[0].uv;
                *pcUVs++ = face.avVertices[1].uv;
                *pcUVs++ = face.avVertices[2].uv;
            }

            for (unsigned int iVert = 0; iVert < 3;++iVert)
            {
                float fSum = 0.0f;
                for (unsigned int iBone = 0;iBone < face.avVertices[iVert].aiBoneLinks.size();++iBone)
                {
                    TempWeightListEntry& pairval = face.avVertices[iVert].aiBoneLinks[iBone];

                    // FIX: The second check is here just to make sure we won't
                    // assign more than one weight to a single vertex index
                    if (pairval.first >= asBones.size() ||
                        pairval.first == face.avVertices[iVert].iParentNode)
                    {
                        ASSIMP_LOG_ERROR("[SMD/VTA] Bone index overflow. "
                            "The bone index will be ignored, the weight will be assigned "
                            "to the vertex' parent node");
                        continue;
                    }
                    aaiBones[pairval.first].push_back(TempWeightListEntry(iNum,pairval.second));
                    fSum += pairval.second;
                }
                // ******************************************************************
                // If the sum of all vertex weights is not 1.0 we must assign
                // the rest to the vertex' parent node. Well, at least the doc says
                // we should ...
                // FIX: We use 0.975 as limit, floating-point inaccuracies seem to
                // be very strong in some SMD exporters. Furthermore it is possible
                // that the parent of a vertex is 0xffffffff (if the corresponding
                // entry in the file was unreadable)
                // ******************************************************************
                if (fSum < 0.975f && face.avVertices[iVert].iParentNode != UINT_MAX)
                {
                    if (face.avVertices[iVert].iParentNode >= asBones.size())
                    {
                        ASSIMP_LOG_ERROR("[SMD/VTA] Bone index overflow. "
                            "The index of the vertex parent bone is invalid. "
                            "The remaining weights will be normalized to 1.0");

                        if (fSum)
                        {
                            fSum = 1 / fSum;
                            for (unsigned int iBone = 0;iBone < face.avVertices[iVert].aiBoneLinks.size();++iBone)
                            {
                                TempWeightListEntry& pairval = face.avVertices[iVert].aiBoneLinks[iBone];
                                if (pairval.first >= asBones.size())continue;
                                aaiBones[pairval.first].back().second *= fSum;
                            }
                        }
                    }
                    else
                    {
                        aaiBones[face.avVertices[iVert].iParentNode].push_back(
                            TempWeightListEntry(iNum,1.0f-fSum));
                    }
                }
                pcMesh->mFaces[iFace].mIndices[iVert] = iNum++;
            }
        }

        // now build all bones of the mesh
        iNum = 0;
        for (unsigned int iBone = 0; iBone < asBones.size();++iBone)
            if (!aaiBones[iBone].empty())++iNum;

        if (false && iNum)
        {
            pcMesh->mNumBones = iNum;
            pcMesh->mBones = new aiBone*[pcMesh->mNumBones];
            iNum = 0;
            for (unsigned int iBone = 0; iBone < asBones.size();++iBone)
            {
                if (aaiBones[iBone].empty())continue;
                aiBone*& bone = pcMesh->mBones[iNum] = new aiBone();

                bone->mNumWeights = (unsigned int)aaiBones[iBone].size();
                bone->mWeights = new aiVertexWeight[bone->mNumWeights];
                bone->mOffsetMatrix = asBones[iBone].mOffsetMatrix;
                bone->mName.Set( asBones[iBone].mName );

                asBones[iBone].bIsUsed = true;

                for (unsigned int iWeight = 0; iWeight < bone->mNumWeights;++iWeight)
                {
                    bone->mWeights[iWeight].mVertexId = aaiBones[iBone][iWeight].first;
                    bone->mWeights[iWeight].mWeight = aaiBones[iBone][iWeight].second;
                }
                ++iNum;
            }
        }
        delete[] aaiBones;
    }
    delete[] aaiFaces;
}

// ------------------------------------------------------------------------------------------------
// add bone child nodes
void SMDImporter::AddBoneChildren(aiNode* pcNode, uint32_t iParent)
{
    ai_assert( NULL != pcNode );
    ai_assert( 0 == pcNode->mNumChildren );
    ai_assert( NULL == pcNode->mChildren);

    // first count ...
    for (unsigned int i = 0; i < asBones.size();++i)
    {
        SMD::Bone& bone = asBones[i];
        if (bone.iParent == iParent)++pcNode->mNumChildren;
    }

    // now allocate the output array
    pcNode->mChildren = new aiNode*[pcNode->mNumChildren];

    // and fill all subnodes
    unsigned int qq = 0;
    for (unsigned int i = 0; i < asBones.size();++i)
    {
        SMD::Bone& bone = asBones[i];
        if (bone.iParent != iParent)continue;

        aiNode* pc = pcNode->mChildren[qq++] = new aiNode();
        pc->mName.Set(bone.mName);

        // store the local transformation matrix of the bind pose
        pc->mTransformation = bone.sAnim.asKeys[bone.sAnim.iFirstTimeKey].matrix;
        pc->mParent = pcNode;

        // add children to this node, too
        AddBoneChildren(pc,i);
    }
}

// ------------------------------------------------------------------------------------------------
// create output nodes
void SMDImporter::CreateOutputNodes()
{
    pScene->mRootNode = new aiNode();
    if (!(pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
    {
        // create one root node that renders all meshes
        pScene->mRootNode->mNumMeshes = pScene->mNumMeshes;
        pScene->mRootNode->mMeshes = new unsigned int[pScene->mNumMeshes];
        for (unsigned int i = 0; i < pScene->mNumMeshes;++i)
            pScene->mRootNode->mMeshes[i] = i;
    }

    // now add all bones as dummy sub nodes to the graph
    // AddBoneChildren(pScene->mRootNode,(uint32_t)-1);

    // if we have only one bone we can even remove the root node
    if (pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE &&
        1 == pScene->mRootNode->mNumChildren)
    {
        aiNode* pcOldRoot = pScene->mRootNode;
        pScene->mRootNode = pcOldRoot->mChildren[0];
        pcOldRoot->mChildren[0] = NULL;
        delete pcOldRoot;

        pScene->mRootNode->mParent = NULL;
    }
    else
    {
        ::strcpy(pScene->mRootNode->mName.data, "<SMD_root>");
        pScene->mRootNode->mName.length = 10;
    }
}

// ------------------------------------------------------------------------------------------------
// create output animations
void SMDImporter::CreateOutputAnimations()
{
    unsigned int iNumBones = 0;
    for (std::vector<SMD::Bone>::const_iterator
        i =  asBones.begin();
        i != asBones.end();++i)
    {
        if ((*i).bIsUsed)++iNumBones;
    }
    if (!iNumBones)
    {
        // just make sure this case doesn't occur ... (it could occur
        // if the file was invalid)
        return;
    }

    pScene->mNumAnimations = 1;
    pScene->mAnimations = new aiAnimation*[1];
    aiAnimation*& anim = pScene->mAnimations[0] = new aiAnimation();

    anim->mDuration = dLengthOfAnim;
    anim->mNumChannels = iNumBones;
    anim->mTicksPerSecond = 25.0; // FIXME: is this correct?

    aiNodeAnim** pp = anim->mChannels = new aiNodeAnim*[anim->mNumChannels];

    // now build valid keys
    unsigned int a = 0;
    for (std::vector<SMD::Bone>::const_iterator
        i =  asBones.begin();
        i != asBones.end();++i)
    {
        if (!(*i).bIsUsed)continue;

        aiNodeAnim* p = pp[a] = new aiNodeAnim();

        // copy the name of the bone
        p->mNodeName.Set( i->mName);

        p->mNumRotationKeys = (unsigned int) (*i).sAnim.asKeys.size();
        if (p->mNumRotationKeys)
        {
            p->mNumPositionKeys = p->mNumRotationKeys;
            aiVectorKey* pVecKeys = p->mPositionKeys = new aiVectorKey[p->mNumRotationKeys];
            aiQuatKey* pRotKeys = p->mRotationKeys = new aiQuatKey[p->mNumRotationKeys];

            for (std::vector<SMD::Bone::Animation::MatrixKey>::const_iterator
                qq =  (*i).sAnim.asKeys.begin();
                qq != (*i).sAnim.asKeys.end(); ++qq)
            {
                pRotKeys->mTime = pVecKeys->mTime = (*qq).dTime;

                // compute the rotation quaternion from the euler angles
                pRotKeys->mValue = aiQuaternion( (*qq).vRot.x, (*qq).vRot.y, (*qq).vRot.z );
                pVecKeys->mValue = (*qq).vPos;

                ++pVecKeys; ++pRotKeys;
            }
        }
        ++a;

        // there are no scaling keys ...
    }
}

// ------------------------------------------------------------------------------------------------
void SMDImporter::ComputeAbsoluteBoneTransformations()
{
    // For each bone: determine the key with the lowest time value
    // theoretically the SMD format should have all keyframes
    // in order. However, I've seen a file where this wasn't true.
    for (unsigned int i = 0; i < asBones.size();++i)
    {
        SMD::Bone& bone = asBones[i];

        uint32_t iIndex = 0;
        double dMin = 10e10;
        for (unsigned int i = 0; i < bone.sAnim.asKeys.size();++i)
        {
            double d = std::min(bone.sAnim.asKeys[i].dTime,dMin);
            if (d < dMin)
            {
                dMin = d;
                iIndex = i;
            }
        }
        bone.sAnim.iFirstTimeKey = iIndex;
    }

    unsigned int iParent = 0;
    while (iParent < asBones.size())
    {
        for (unsigned int iBone = 0; iBone < asBones.size();++iBone)
        {
            SMD::Bone& bone = asBones[iBone];

            if (iParent == bone.iParent)
            {
                SMD::Bone& parentBone = asBones[iParent];


                uint32_t iIndex = bone.sAnim.iFirstTimeKey;
                const aiMatrix4x4& mat = bone.sAnim.asKeys[iIndex].matrix;
                aiMatrix4x4& matOut = bone.sAnim.asKeys[iIndex].matrixAbsolute;

                // The same for the parent bone ...
                iIndex = parentBone.sAnim.iFirstTimeKey;
                const aiMatrix4x4& mat2 = parentBone.sAnim.asKeys[iIndex].matrixAbsolute;

                // Compute the absolute transformation matrix
                matOut = mat * mat2;
            }
        }
        ++iParent;
    }

    // Store the inverse of the absolute transformation matrix
    // of the first key as bone offset matrix
    for (iParent = 0; iParent < asBones.size();++iParent)
    {
        SMD::Bone& bone = asBones[iParent];
        bone.mOffsetMatrix = bone.sAnim.asKeys[bone.sAnim.iFirstTimeKey].matrixAbsolute;
        bone.mOffsetMatrix.Inverse();
    }
}
\
// ------------------------------------------------------------------------------------------------
// create output materials
void SMDImporter::CreateOutputMaterials()
{
    ai_assert( nullptr != pScene );

    pScene->mNumMaterials = (unsigned int)aszTextures.size();
    pScene->mMaterials = new aiMaterial*[std::max(1u, pScene->mNumMaterials)];

    for (unsigned int iMat = 0; iMat < pScene->mNumMaterials; ++iMat) {
        aiMaterial* pcMat = new aiMaterial();
        ai_assert( nullptr != pcMat );
        pScene->mMaterials[iMat] = pcMat;

        aiString szName;
        szName.length = (size_t)ai_snprintf(szName.data,MAXLEN,"Texture_%u",iMat);
        pcMat->AddProperty(&szName,AI_MATKEY_NAME);

        if (aszTextures[iMat].length())
        {
            ::strncpy(szName.data, aszTextures[iMat].c_str(),MAXLEN-1);
            szName.length = aszTextures[iMat].length();
            pcMat->AddProperty(&szName,AI_MATKEY_TEXTURE_DIFFUSE(0));
        }
    }

    // create a default material if necessary
    if (0 == pScene->mNumMaterials)
    {
        pScene->mNumMaterials = 1;

        aiMaterial* pcHelper = new aiMaterial();
        pScene->mMaterials[0] = pcHelper;

        int iMode = (int)aiShadingMode_Gouraud;
        pcHelper->AddProperty<int>(&iMode, 1, AI_MATKEY_SHADING_MODEL);

        aiColor3D clr;
        clr.b = clr.g = clr.r = 0.7f;
        pcHelper->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_DIFFUSE);
        pcHelper->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_SPECULAR);

        clr.b = clr.g = clr.r = 0.05f;
        pcHelper->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_AMBIENT);

        aiString szName;
        szName.Set(AI_DEFAULT_MATERIAL_NAME);
        pcHelper->AddProperty(&szName,AI_MATKEY_NAME);
    }
}

// ------------------------------------------------------------------------------------------------
// Parse the file
void SMDImporter::ParseFile()
{
    const char* szCurrent = &mBuffer[0];

    // read line per line ...
    for ( ;; )
    {
        if(!SkipSpacesAndLineEnd(szCurrent,&szCurrent)) break;

        // "version <n> \n", <n> should be 1 for hl and hl2 SMD files
        if (TokenMatch(szCurrent,"version",7))
        {
            if(!SkipSpaces(szCurrent,&szCurrent)) break;
            if (1 != strtoul10(szCurrent,&szCurrent))
            {
                ASSIMP_LOG_WARN("SMD.version is not 1. This "
                    "file format is not known. Continuing happily ...");
            }
            continue;
        }
        // "nodes\n" - Starts the node section
        if (TokenMatch(szCurrent,"nodes",5))
        {
            ParseNodesSection(szCurrent,&szCurrent);
            continue;
        }
        // "triangles\n" - Starts the triangle section
        if (TokenMatch(szCurrent,"triangles",9))
        {
            ParseTrianglesSection(szCurrent,&szCurrent);
            continue;
        }
        // "vertexanimation\n" - Starts the vertex animation section
        if (TokenMatch(szCurrent,"vertexanimation",15))
        {
            bHasUVs = false;
            ParseVASection(szCurrent,&szCurrent);
            continue;
        }
        // "skeleton\n" - Starts the skeleton section
        if (TokenMatch(szCurrent,"skeleton",8))
        {
            ParseSkeletonSection(szCurrent,&szCurrent);
            continue;
        }
        SkipLine(szCurrent,&szCurrent);
    }
    return;
}

// ------------------------------------------------------------------------------------------------
unsigned int SMDImporter::GetTextureIndex(const std::string& filename)
{
    unsigned int iIndex = 0;
    for (std::vector<std::string>::const_iterator
        i =  aszTextures.begin();
        i != aszTextures.end();++i,++iIndex)
    {
        // case-insensitive ... it's a path
        if (0 == ASSIMP_stricmp ( filename.c_str(),(*i).c_str()))return iIndex;
    }
    iIndex = (unsigned int)aszTextures.size();
    aszTextures.push_back(filename);
    return iIndex;
}

// ------------------------------------------------------------------------------------------------
// Parse the nodes section of the file
void SMDImporter::ParseNodesSection(const char* szCurrent,
    const char** szCurrentOut)
{
    for ( ;; )
    {
        // "end\n" - Ends the nodes section
        if (0 == ASSIMP_strincmp(szCurrent,"end",3) &&
            IsSpaceOrNewLine(*(szCurrent+3)))
        {
            szCurrent += 4;
            break;
        }
        ParseNodeInfo(szCurrent,&szCurrent);
    }
    SkipSpacesAndLineEnd(szCurrent,&szCurrent);
    *szCurrentOut = szCurrent;
}

// ------------------------------------------------------------------------------------------------
// Parse the triangles section of the file
void SMDImporter::ParseTrianglesSection(const char* szCurrent,
    const char** szCurrentOut)
{
    // Parse a triangle, parse another triangle, parse the next triangle ...
    // and so on until we reach a token that looks quite similar to "end"
    for ( ;; )
    {
        if(!SkipSpacesAndLineEnd(szCurrent,&szCurrent)) break;

        // "end\n" - Ends the triangles section
        if (TokenMatch(szCurrent,"end",3))
            break;
        ParseTriangle(szCurrent,&szCurrent);
    }
    SkipSpacesAndLineEnd(szCurrent,&szCurrent);
    *szCurrentOut = szCurrent;
}
// ------------------------------------------------------------------------------------------------
// Parse the vertex animation section of the file
void SMDImporter::ParseVASection(const char* szCurrent,
    const char** szCurrentOut)
{
    unsigned int iCurIndex = 0;
    for ( ;; )
    {
        if(!SkipSpacesAndLineEnd(szCurrent,&szCurrent)) break;

        // "end\n" - Ends the "vertexanimation" section
        if (TokenMatch(szCurrent,"end",3))
            break;

        // "time <n>\n"
        if (TokenMatch(szCurrent,"time",4))
        {
            // NOTE: The doc says that time values COULD be negative ...
            // NOTE2: this is the shape key -> valve docs
            int iTime = 0;
            if(!ParseSignedInt(szCurrent,&szCurrent,iTime) || configFrameID != (unsigned int)iTime)break;
            SkipLine(szCurrent,&szCurrent);
        }
        else
        {
            if(0 == iCurIndex)
            {
                asTriangles.push_back(SMD::Face());
            }
            if (++iCurIndex == 3)iCurIndex = 0;
            ParseVertex(szCurrent,&szCurrent,asTriangles.back().avVertices[iCurIndex],true);
        }
    }

    if (iCurIndex != 2 && !asTriangles.empty())
    {
        // we want to no degenerates, so throw this triangle away
        asTriangles.pop_back();
    }

    SkipSpacesAndLineEnd(szCurrent,&szCurrent);
    *szCurrentOut = szCurrent;
}
// ------------------------------------------------------------------------------------------------
// Parse the skeleton section of the file
void SMDImporter::ParseSkeletonSection(const char* szCurrent,
    const char** szCurrentOut)
{
    int iTime = 0;
    for ( ;; )
    {
        if(!SkipSpacesAndLineEnd(szCurrent,&szCurrent)) break;

        // "end\n" - Ends the skeleton section
        if (TokenMatch(szCurrent,"end",3))
            break;

        // "time <n>\n" - Specifies the current animation frame
        else if (TokenMatch(szCurrent,"time",4))
        {
            // NOTE: The doc says that time values COULD be negative ...
            if(!ParseSignedInt(szCurrent,&szCurrent,iTime))break;

            iSmallestFrame = std::min(iSmallestFrame,iTime);
            SkipLine(szCurrent,&szCurrent);
        }
        else ParseSkeletonElement(szCurrent,&szCurrent,iTime);
    }
    *szCurrentOut = szCurrent;
}

// ------------------------------------------------------------------------------------------------
#define SMDI_PARSE_RETURN { \
    SkipLine(szCurrent,&szCurrent); \
    *szCurrentOut = szCurrent; \
    return; \
}
// ------------------------------------------------------------------------------------------------
// Parse a node line
void SMDImporter::ParseNodeInfo(const char* szCurrent,
    const char** szCurrentOut)
{
    unsigned int iBone  = 0;
    SkipSpacesAndLineEnd(szCurrent,&szCurrent);
    if(!ParseUnsignedInt(szCurrent,&szCurrent,iBone) || !SkipSpaces(szCurrent,&szCurrent))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing bone index");
        SMDI_PARSE_RETURN;
    }
    // add our bone to the list
    if (iBone >= asBones.size())asBones.resize(iBone+1);
    SMD::Bone& bone = asBones[iBone];

    bool bQuota = true;
    if ('\"' != *szCurrent)
    {
        LogWarning("Bone name is expcted to be enclosed in "
            "double quotation marks. ");
        bQuota = false;
    }
    else ++szCurrent;

    const char* szEnd = szCurrent;
    for ( ;; )
    {
        if (bQuota && '\"' == *szEnd)
        {
            iBone = (unsigned int)(szEnd - szCurrent);
            ++szEnd;
            break;
        }
        else if (IsSpaceOrNewLine(*szEnd))
        {
            iBone = (unsigned int)(szEnd - szCurrent);
            break;
        }
        else if (!(*szEnd))
        {
            LogErrorNoThrow("Unexpected EOF/EOL while parsing bone name");
            SMDI_PARSE_RETURN;
        }
        ++szEnd;
    }
    bone.mName = std::string(szCurrent,iBone);
    szCurrent = szEnd;

    // the only negative bone parent index that could occur is -1 AFAIK
    if(!ParseSignedInt(szCurrent,&szCurrent,(int&)bone.iParent))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing bone parent index. Assuming -1");
        SMDI_PARSE_RETURN;
    }

    // go to the beginning of the next line
    SMDI_PARSE_RETURN;
}

// ------------------------------------------------------------------------------------------------
// Parse a skeleton element
void SMDImporter::ParseSkeletonElement(const char* szCurrent,
    const char** szCurrentOut,int iTime)
{
    aiVector3D vPos;
    aiVector3D vRot;

    unsigned int iBone  = 0;
    if(!ParseUnsignedInt(szCurrent,&szCurrent,iBone))
    {
        ASSIMP_LOG_ERROR("Unexpected EOF/EOL while parsing bone index");
        SMDI_PARSE_RETURN;
    }
    if (iBone >= asBones.size())
    {
        LogErrorNoThrow("Bone index in skeleton section is out of range");
        SMDI_PARSE_RETURN;
    }
    SMD::Bone& bone = asBones[iBone];

    bone.sAnim.asKeys.push_back(SMD::Bone::Animation::MatrixKey());
    SMD::Bone::Animation::MatrixKey& key = bone.sAnim.asKeys.back();

    key.dTime = (double)iTime;
    if(!ParseFloat(szCurrent,&szCurrent,(float&)vPos.x))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing bone.pos.x");
        SMDI_PARSE_RETURN;
    }
    if(!ParseFloat(szCurrent,&szCurrent,(float&)vPos.y))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing bone.pos.y");
        SMDI_PARSE_RETURN;
    }
    if(!ParseFloat(szCurrent,&szCurrent,(float&)vPos.z))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing bone.pos.z");
        SMDI_PARSE_RETURN;
    }
    if(!ParseFloat(szCurrent,&szCurrent,(float&)vRot.x))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing bone.rot.x");
        SMDI_PARSE_RETURN;
    }
    if(!ParseFloat(szCurrent,&szCurrent,(float&)vRot.y))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing bone.rot.y");
        SMDI_PARSE_RETURN;
    }
    if(!ParseFloat(szCurrent,&szCurrent,(float&)vRot.z))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing bone.rot.z");
        SMDI_PARSE_RETURN;
    }
    // build the transformation matrix of the key
    key.matrix.FromEulerAnglesXYZ(vRot.x,vRot.y,vRot.z);
    {
        aiMatrix4x4 mTemp;
        mTemp.a4 = vPos.x;
        mTemp.b4 = vPos.y;
        mTemp.c4 = vPos.z;
        key.matrix = key.matrix * mTemp;
    }

    // go to the beginning of the next line
    SMDI_PARSE_RETURN;
}

// ------------------------------------------------------------------------------------------------
// Parse a triangle
void SMDImporter::ParseTriangle(const char* szCurrent,
    const char** szCurrentOut)
{
    asTriangles.push_back(SMD::Face());
    SMD::Face& face = asTriangles.back();

    if(!SkipSpaces(szCurrent,&szCurrent))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing a triangle");
        return;
    }

    // read the texture file name
    const char* szLast = szCurrent;
    while (!IsSpaceOrNewLine(*++szCurrent));

    // ... and get the index that belongs to this file name
    face.iTexture = GetTextureIndex(std::string(szLast,(uintptr_t)szCurrent-(uintptr_t)szLast));

    SkipSpacesAndLineEnd(szCurrent,&szCurrent);

    // load three vertices
    for (unsigned int iVert = 0; iVert < 3;++iVert)
    {
        ParseVertex(szCurrent,&szCurrent,
            face.avVertices[iVert]);
    }
    *szCurrentOut = szCurrent;
}

// ------------------------------------------------------------------------------------------------
// Parse a float
bool SMDImporter::ParseFloat(const char* szCurrent,
    const char** szCurrentOut, float& out)
{
    if(!SkipSpaces(&szCurrent))
        return false;

    *szCurrentOut = fast_atoreal_move<float>(szCurrent,out);
    return true;
}

// ------------------------------------------------------------------------------------------------
// Parse an unsigned int
bool SMDImporter::ParseUnsignedInt(const char* szCurrent,
    const char** szCurrentOut, unsigned int& out)
{
    if(!SkipSpaces(&szCurrent))
        return false;

    out = strtoul10(szCurrent,szCurrentOut);
    return true;
}

// ------------------------------------------------------------------------------------------------
// Parse a signed int
bool SMDImporter::ParseSignedInt(const char* szCurrent,
    const char** szCurrentOut, int& out)
{
    if(!SkipSpaces(&szCurrent))
        return false;

    out = strtol10(szCurrent,szCurrentOut);
    return true;
}

// ------------------------------------------------------------------------------------------------
// Parse a vertex
void SMDImporter::ParseVertex(const char* szCurrent,
    const char** szCurrentOut, SMD::Vertex& vertex,
    bool bVASection /*= false*/)
{
    if (SkipSpaces(&szCurrent) && IsLineEnd(*szCurrent))
    {
        SkipSpacesAndLineEnd(szCurrent,&szCurrent);
        return ParseVertex(szCurrent,szCurrentOut,vertex,bVASection);
    }
    if(!ParseSignedInt(szCurrent,&szCurrent,(int&)vertex.iParentNode))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing vertex.parent");
        SMDI_PARSE_RETURN;
    }
    if(!ParseFloat(szCurrent,&szCurrent,(float&)vertex.pos.x))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing vertex.pos.x");
        SMDI_PARSE_RETURN;
    }
    if(!ParseFloat(szCurrent,&szCurrent,(float&)vertex.pos.y))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing vertex.pos.y");
        SMDI_PARSE_RETURN;
    }
    if(!ParseFloat(szCurrent,&szCurrent,(float&)vertex.pos.z))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing vertex.pos.z");
        SMDI_PARSE_RETURN;
    }
    if(!ParseFloat(szCurrent,&szCurrent,(float&)vertex.nor.x))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing vertex.nor.x");
        SMDI_PARSE_RETURN;
    }
    if(!ParseFloat(szCurrent,&szCurrent,(float&)vertex.nor.y))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing vertex.nor.y");
        SMDI_PARSE_RETURN;
    }
    if(!ParseFloat(szCurrent,&szCurrent,(float&)vertex.nor.z))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing vertex.nor.z");
        SMDI_PARSE_RETURN;
    }

    if (bVASection)SMDI_PARSE_RETURN;

    if(!ParseFloat(szCurrent,&szCurrent,(float&)vertex.uv.x))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing vertex.uv.x");
        SMDI_PARSE_RETURN;
    }
    if(!ParseFloat(szCurrent,&szCurrent,(float&)vertex.uv.y))
    {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing vertex.uv.y");
        SMDI_PARSE_RETURN;
    }

    // now read the number of bones affecting this vertex
    // all elements from now are fully optional, we don't need them
    unsigned int iSize = 0;
    if(!ParseUnsignedInt(szCurrent,&szCurrent,iSize))SMDI_PARSE_RETURN;
    vertex.aiBoneLinks.resize(iSize,std::pair<unsigned int, float>(0,0.0f));

    for (std::vector<std::pair<unsigned int, float> >::iterator
        i =  vertex.aiBoneLinks.begin();
        i != vertex.aiBoneLinks.end();++i)
    {
        if(!ParseUnsignedInt(szCurrent,&szCurrent,(*i).first))
            SMDI_PARSE_RETURN;
        if(!ParseFloat(szCurrent,&szCurrent,(*i).second))
            SMDI_PARSE_RETURN;
    }

    // go to the beginning of the next line
    SMDI_PARSE_RETURN;
}

#endif // !! ASSIMP_BUILD_NO_SMD_IMPORTER
