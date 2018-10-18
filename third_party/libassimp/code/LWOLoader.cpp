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

/** @file  LWOLoader.cpp
 *  @brief Implementation of the LWO importer class
 */


#ifndef ASSIMP_BUILD_NO_LWO_IMPORTER

// internal headers
#include "LWOLoader.h"
#include <assimp/StringComparison.h>
#include <assimp/SGSpatialSort.h>
#include <assimp/ByteSwapper.h>
#include "ProcessHelper.h"
#include "ConvertToLHProcess.h"
#include <assimp/IOSystem.hpp>
#include <assimp/importerdesc.h>
#include <memory>
#include <sstream>
#include <iomanip>
#include <map>

using namespace Assimp;

static const aiImporterDesc desc = {
    "LightWave/Modo Object Importer",
    "",
    "",
    "https://www.lightwave3d.com/lightwave_sdk/",
    aiImporterFlags_SupportTextFlavour,
    0,
    0,
    0,
    0,
    "lwo lxo"
};

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
LWOImporter::LWOImporter()
    : mIsLWO2(),
    mIsLXOB(),
    mLayers(),
    mCurLayer(),
    mTags(),
    mMapping(),
    mSurfaces(),
    mFileBuffer(),
    fileSize(),
    pScene(),
    configSpeedFlag(),
    configLayerIndex(),
    hasNamedLayer()
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
LWOImporter::~LWOImporter()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool LWOImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    const std::string extension = GetExtension(pFile);
    if (extension == "lwo" || extension == "lxo") {
        return true;
    }

    // if check for extension is not enough, check for the magic tokens
    if (!extension.length() || checkSig) {
        uint32_t tokens[3];
        tokens[0] = AI_LWO_FOURCC_LWOB;
        tokens[1] = AI_LWO_FOURCC_LWO2;
        tokens[2] = AI_LWO_FOURCC_LXOB;
        return CheckMagicToken(pIOHandler,pFile,tokens,3,8);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Setup configuration properties
void LWOImporter::SetupProperties(const Importer* pImp)
{
    configSpeedFlag  = ( 0 != pImp->GetPropertyInteger(AI_CONFIG_FAVOUR_SPEED,0) ? true : false);
    configLayerIndex = pImp->GetPropertyInteger (AI_CONFIG_IMPORT_LWO_ONE_LAYER_ONLY,UINT_MAX);
    configLayerName  = pImp->GetPropertyString  (AI_CONFIG_IMPORT_LWO_ONE_LAYER_ONLY,"");
}

// ------------------------------------------------------------------------------------------------
// Get list of file extensions
const aiImporterDesc* LWOImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void LWOImporter::InternReadFile( const std::string& pFile,
    aiScene* pScene,
    IOSystem* pIOHandler)
{
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile, "rb"));

    // Check whether we can read from the file
    if( file.get() == NULL)
        throw DeadlyImportError( "Failed to open LWO file " + pFile + ".");

    if((this->fileSize = (unsigned int)file->FileSize()) < 12)
        throw DeadlyImportError("LWO: The file is too small to contain the IFF header");

    // Allocate storage and copy the contents of the file to a memory buffer
    std::vector< uint8_t > mBuffer(fileSize);
    file->Read( &mBuffer[0], 1, fileSize);
    this->pScene = pScene;

    // Determine the type of the file
    uint32_t fileType;
    const char* sz = IFF::ReadHeader(&mBuffer[0],fileType);
    if (sz)throw DeadlyImportError(sz);

    mFileBuffer = &mBuffer[0] + 12;
    fileSize -= 12;

    // Initialize some members with their default values
    hasNamedLayer   = false;

    // Create temporary storage on the stack but store pointers to it in the class
    // instance. Therefore everything will be destructed properly if an exception
    // is thrown and we needn't take care of that.
    LayerList       _mLayers;
    SurfaceList     _mSurfaces;
    TagList         _mTags;
    TagMappingTable _mMapping;

    mLayers         = &_mLayers;
    mTags           = &_mTags;
    mMapping        = &_mMapping;
    mSurfaces       = &_mSurfaces;

    // Allocate a default layer (layer indices are 1-based from now)
    mLayers->push_back(Layer());
    mCurLayer = &mLayers->back();
    mCurLayer->mName = "<LWODefault>";
    mCurLayer->mIndex = -1;

    // old lightwave file format (prior to v6)
    if (AI_LWO_FOURCC_LWOB == fileType) {
        ASSIMP_LOG_INFO("LWO file format: LWOB (<= LightWave 5.5)");

        mIsLWO2 = false;
        mIsLXOB = false;
        LoadLWOBFile();
    }
    // New lightwave format
    else if (AI_LWO_FOURCC_LWO2 == fileType)    {
        mIsLXOB = false;
        ASSIMP_LOG_INFO("LWO file format: LWO2 (>= LightWave 6)");
    }
    // MODO file format
    else if (AI_LWO_FOURCC_LXOB == fileType)    {
        mIsLXOB = true;
        ASSIMP_LOG_INFO("LWO file format: LXOB (Modo)");
    }
    // we don't know this format
    else
    {
        char szBuff[5];
        szBuff[0] = (char)(fileType >> 24u);
        szBuff[1] = (char)(fileType >> 16u);
        szBuff[2] = (char)(fileType >> 8u);
        szBuff[3] = (char)(fileType);
        szBuff[4] = '\0';
        throw DeadlyImportError(std::string("Unknown LWO sub format: ") + szBuff);
    }

    if (AI_LWO_FOURCC_LWOB != fileType) {
        mIsLWO2 = true;
        LoadLWO2File();

        // The newer lightwave format allows the user to configure the
        // loader that just one layer is used. If this is the case
        // we need to check now whether the requested layer has been found.
        if (UINT_MAX != configLayerIndex) {
            unsigned int layerCount = 0;
            for(std::list<LWO::Layer>::iterator itLayers=mLayers->begin(); itLayers!=mLayers->end(); ++itLayers)
                if (!itLayers->skip)
                    layerCount++;
            if (layerCount!=2)
                throw DeadlyImportError("LWO2: The requested layer was not found");
        }

        if (configLayerName.length() && !hasNamedLayer) {
            throw DeadlyImportError("LWO2: Unable to find the requested layer: "
                + configLayerName);
        }
    }

    // now, as we have loaded all data, we can resolve cross-referenced tags and clips
    ResolveTags();
    ResolveClips();

    // now process all layers and build meshes and nodes
    std::vector<aiMesh*> apcMeshes;
    std::map<uint16_t, aiNode*> apcNodes;

    apcMeshes.reserve(mLayers->size()*std::min(((unsigned int)mSurfaces->size()/2u), 1u));

    unsigned int iDefaultSurface = UINT_MAX; // index of the default surface
	for (LWO::Layer &layer : *mLayers) {
        if (layer.skip)
            continue;

        // I don't know whether there could be dummy layers, but it would be possible
        const unsigned int meshStart = (unsigned int)apcMeshes.size();
        if (!layer.mFaces.empty() && !layer.mTempPoints.empty())    {

            // now sort all faces by the surfaces assigned to them
            std::vector<SortedRep> pSorted(mSurfaces->size()+1);

            unsigned int i = 0;
            for (FaceList::iterator it = layer.mFaces.begin(), end = layer.mFaces.end();it != end;++it,++i) {
                // Check whether we support this face's type
                if ((*it).type != AI_LWO_FACE && (*it).type != AI_LWO_PTCH &&
                    (*it).type != AI_LWO_BONE && (*it).type != AI_LWO_SUBD) {
                    continue;
                }

                unsigned int idx = (*it).surfaceIndex;
                if (idx >= mTags->size())
                {
                    ASSIMP_LOG_WARN("LWO: Invalid face surface index");
                    idx = UINT_MAX;
                }
                if(UINT_MAX == idx || UINT_MAX == (idx = _mMapping[idx]))   {
                    if (UINT_MAX == iDefaultSurface)    {
                        iDefaultSurface = (unsigned int)mSurfaces->size();
                        mSurfaces->push_back(LWO::Surface());
                        LWO::Surface& surf = mSurfaces->back();
                        surf.mColor.r = surf.mColor.g = surf.mColor.b = 0.6f;
                        surf.mName = "LWODefaultSurface";
                    }
                    idx = iDefaultSurface;
                }
                pSorted[idx].push_back(i);
            }
            if (UINT_MAX == iDefaultSurface) {
                pSorted.erase(pSorted.end()-1);
            }
            for (unsigned int p = 0,i = 0;i < mSurfaces->size();++i)    {
                SortedRep& sorted = pSorted[i];
                if (sorted.empty())
                    continue;

                // generate the mesh
                aiMesh* mesh = new aiMesh();
                apcMeshes.push_back(mesh);
                mesh->mNumFaces = (unsigned int)sorted.size();

                // count the number of vertices
                SortedRep::const_iterator it = sorted.begin(), end = sorted.end();
                for (;it != end;++it)   {
                    mesh->mNumVertices += layer.mFaces[*it].mNumIndices;
                }

                aiVector3D *nrm = NULL, * pv = mesh->mVertices = new aiVector3D[mesh->mNumVertices];
                aiFace* pf = mesh->mFaces = new aiFace[mesh->mNumFaces];
                mesh->mMaterialIndex = i;

                // find out which vertex color channels and which texture coordinate
                // channels are really required by the material attached to this mesh
                unsigned int vUVChannelIndices[AI_MAX_NUMBER_OF_TEXTURECOORDS];
                unsigned int vVColorIndices[AI_MAX_NUMBER_OF_COLOR_SETS];

#ifdef ASSIMP_BUILD_DEBUG
                for (unsigned int mui = 0; mui < AI_MAX_NUMBER_OF_TEXTURECOORDS;++mui ) {
                    vUVChannelIndices[mui] = UINT_MAX;
                }
                for (unsigned int mui = 0; mui < AI_MAX_NUMBER_OF_COLOR_SETS;++mui ) {
                    vVColorIndices[mui] = UINT_MAX;
                }
#endif

                FindUVChannels(_mSurfaces[i],sorted,layer,vUVChannelIndices);
                FindVCChannels(_mSurfaces[i],sorted,layer,vVColorIndices);

                // allocate storage for UV and CV channels
                aiVector3D* pvUV[AI_MAX_NUMBER_OF_TEXTURECOORDS];
                for (unsigned int mui = 0; mui < AI_MAX_NUMBER_OF_TEXTURECOORDS;++mui ) {
                    if (UINT_MAX == vUVChannelIndices[mui]) {
                        break;
                    }

                    pvUV[mui] = mesh->mTextureCoords[mui] = new aiVector3D[mesh->mNumVertices];

                    // LightWave doesn't support more than 2 UV components (?)
                    mesh->mNumUVComponents[0] = 2;
                }

                if (layer.mNormals.name.length())
                    nrm = mesh->mNormals = new aiVector3D[mesh->mNumVertices];

                aiColor4D* pvVC[AI_MAX_NUMBER_OF_COLOR_SETS];
                for (unsigned int mui = 0; mui < AI_MAX_NUMBER_OF_COLOR_SETS;++mui) {
                    if (UINT_MAX == vVColorIndices[mui]) {
                        break;
                    }
                    pvVC[mui] = mesh->mColors[mui] = new aiColor4D[mesh->mNumVertices];
                }

                // we would not need this extra array, but the code is much cleaner if we use it
                std::vector<unsigned int>& smoothingGroups = layer.mPointReferrers;
                smoothingGroups.erase (smoothingGroups.begin(),smoothingGroups.end());
                smoothingGroups.resize(mesh->mNumFaces,0);

                // now convert all faces
                unsigned int vert = 0;
                std::vector<unsigned int>::iterator outIt = smoothingGroups.begin();
                for (it = sorted.begin(); it != end;++it,++outIt)   {
                    const LWO::Face& face = layer.mFaces[*it];
                    *outIt = face.smoothGroup;

                    // copy all vertices
                    for (unsigned int q = 0; q  < face.mNumIndices;++q,++vert)  {
                        unsigned int idx = face.mIndices[q];
                        *pv++ = layer.mTempPoints[idx] /*- layer.mPivot*/;

                        // process UV coordinates
                        for (unsigned int w = 0; w < AI_MAX_NUMBER_OF_TEXTURECOORDS;++w)    {
                            if (UINT_MAX == vUVChannelIndices[w]) {
                                break;
                            }
                            aiVector3D*& pp = pvUV[w];
                            const aiVector2D& src = ((aiVector2D*)&layer.mUVChannels[vUVChannelIndices[w]].rawData[0])[idx];
                            pp->x = src.x;
                            pp->y = src.y;
                            pp++;
                        }

                        // process normals (MODO extension)
                        if (nrm)    {
                            *nrm = ((aiVector3D*)&layer.mNormals.rawData[0])[idx];
                            nrm->z *= -1.f;
                            ++nrm;
                        }

                        // process vertex colors
                        for (unsigned int w = 0; w < AI_MAX_NUMBER_OF_COLOR_SETS;++w)   {
                            if (UINT_MAX == vVColorIndices[w]) {
                                break;
                            }
                            *pvVC[w] = ((aiColor4D*)&layer.mVColorChannels[vVColorIndices[w]].rawData[0])[idx];

                            // If a RGB color map is explicitly requested delete the
                            // alpha channel - it could theoretically be != 1.
                            if(_mSurfaces[i].mVCMapType == AI_LWO_RGB)
                                pvVC[w]->a = 1.f;

                            pvVC[w]++;
                        }

#if 0
                        // process vertex weights. We can't properly reconstruct the whole skeleton for now,
                        // but we can create dummy bones for all weight channels which we have.
                        for (unsigned int w = 0; w < layer.mWeightChannels.size();++w)
                        {
                        }
#endif

                        face.mIndices[q] = vert;
                    }
                    pf->mIndices = face.mIndices;
                    pf->mNumIndices = face.mNumIndices;
                    unsigned int** p = (unsigned int**)&face.mIndices;*p = NULL; // HACK: make sure it won't be deleted
                    pf++;
                }

                if (!mesh->mNormals)    {
                    // Compute normal vectors for the mesh - we can't use our GenSmoothNormal-
                    // Step here since it wouldn't handle smoothing groups correctly for LWO.
                    // So we use a separate implementation.
                    ComputeNormals(mesh,smoothingGroups,_mSurfaces[i]);
                }
                else {
                    ASSIMP_LOG_DEBUG("LWO2: No need to compute normals, they're already there");
                }
                ++p;
            }
        }

        // Generate nodes to render the mesh. Store the source layer in the mParent member of the nodes
        unsigned int num = static_cast<unsigned int>(apcMeshes.size() - meshStart);
        if (layer.mName != "<LWODefault>" || num > 0) {
            aiNode* pcNode = new aiNode();
            pcNode->mName.Set(layer.mName);
            pcNode->mParent = (aiNode*)&layer;
            pcNode->mNumMeshes = num;

            if (pcNode->mNumMeshes) {
                pcNode->mMeshes = new unsigned int[pcNode->mNumMeshes];
                for (unsigned int p = 0; p < pcNode->mNumMeshes;++p)
                    pcNode->mMeshes[p] = p + meshStart;
            }
            apcNodes[layer.mIndex] = pcNode;
        }
    }

    if (apcNodes.empty() || apcMeshes.empty())
        throw DeadlyImportError("LWO: No meshes loaded");

    // The RemoveRedundantMaterials step will clean this up later
    pScene->mMaterials = new aiMaterial*[pScene->mNumMaterials = (unsigned int)mSurfaces->size()];
    for (unsigned int mat = 0; mat < pScene->mNumMaterials;++mat)   {
        aiMaterial* pcMat = new aiMaterial();
        pScene->mMaterials[mat] = pcMat;
        ConvertMaterial((*mSurfaces)[mat],pcMat);
    }

    // copy the meshes to the output structure
    pScene->mMeshes = new aiMesh*[ pScene->mNumMeshes = (unsigned int)apcMeshes.size() ];
    ::memcpy(pScene->mMeshes,&apcMeshes[0],pScene->mNumMeshes*sizeof(void*));

    // generate the final node graph
    GenerateNodeGraph(apcNodes);
}

// ------------------------------------------------------------------------------------------------
void LWOImporter::ComputeNormals(aiMesh* mesh, const std::vector<unsigned int>& smoothingGroups,
    const LWO::Surface& surface)
{
    // Allocate output storage
    mesh->mNormals = new aiVector3D[mesh->mNumVertices];

    // First generate per-face normals
    aiVector3D* out;
    std::vector<aiVector3D> faceNormals;

    // ... in some cases that's already enough
    if (!surface.mMaximumSmoothAngle)
        out = mesh->mNormals;
    else    {
        faceNormals.resize(mesh->mNumVertices);
        out = &faceNormals[0];
    }

    aiFace* begin = mesh->mFaces, *const end = mesh->mFaces+mesh->mNumFaces;
    for (; begin != end; ++begin)   {
        aiFace& face = *begin;

        if(face.mNumIndices < 3) {
            continue;
        }

        // LWO doc: "the normal is defined as the cross product of the first and last edges"
        aiVector3D* pV1 = mesh->mVertices + face.mIndices[0];
        aiVector3D* pV2 = mesh->mVertices + face.mIndices[1];
        aiVector3D* pV3 = mesh->mVertices + face.mIndices[face.mNumIndices-1];

        aiVector3D vNor = ((*pV2 - *pV1) ^(*pV3 - *pV1)).Normalize();
        for (unsigned int i = 0; i < face.mNumIndices;++i)
            out[face.mIndices[i]] = vNor;
    }
    if (!surface.mMaximumSmoothAngle)return;
    const float posEpsilon = ComputePositionEpsilon(mesh);

    // Now generate the spatial sort tree
    SGSpatialSort sSort;
    std::vector<unsigned int>::const_iterator it = smoothingGroups.begin();
    for( begin =  mesh->mFaces; begin != end; ++begin, ++it)
    {
        aiFace& face = *begin;
        for (unsigned int i = 0; i < face.mNumIndices;++i)
        {
            unsigned int tt = face.mIndices[i];
            sSort.Add(mesh->mVertices[tt],tt,*it);
        }
    }
    // Sort everything - this takes O(nlogn) time
    sSort.Prepare();
    std::vector<unsigned int> poResult;
    poResult.reserve(20);

    // Generate vertex normals. We have O(logn) for the binary lookup, which we need
    // for n elements, thus the EXPECTED complexity is O(nlogn)
    if (surface.mMaximumSmoothAngle < 3.f && !configSpeedFlag)  {
        const float fLimit = std::cos(surface.mMaximumSmoothAngle);

        for( begin =  mesh->mFaces, it = smoothingGroups.begin(); begin != end; ++begin, ++it)  {
            const aiFace& face = *begin;
            unsigned int* beginIdx = face.mIndices, *const endIdx = face.mIndices+face.mNumIndices;
            for (; beginIdx != endIdx; ++beginIdx)
            {
                unsigned int idx = *beginIdx;
                sSort.FindPositions(mesh->mVertices[idx],*it,posEpsilon,poResult,true);
                std::vector<unsigned int>::const_iterator a, end = poResult.end();

                aiVector3D vNormals;
                for (a =  poResult.begin();a != end;++a)    {
                    const aiVector3D& v = faceNormals[*a];
                    if (v * faceNormals[idx] < fLimit)
                        continue;
                    vNormals += v;
                }
                mesh->mNormals[idx] = vNormals.Normalize();
            }
        }
    }
     // faster code path in case there is no smooth angle
    else    {
        std::vector<bool> vertexDone(mesh->mNumVertices,false);
        for( begin =  mesh->mFaces, it = smoothingGroups.begin(); begin != end; ++begin, ++it)  {
            const aiFace& face = *begin;
            unsigned int* beginIdx = face.mIndices, *const endIdx = face.mIndices+face.mNumIndices;
            for (; beginIdx != endIdx; ++beginIdx)
            {
                unsigned int idx = *beginIdx;
                if (vertexDone[idx])
                    continue;
                sSort.FindPositions(mesh->mVertices[idx],*it,posEpsilon,poResult,true);
                std::vector<unsigned int>::const_iterator a, end = poResult.end();

                aiVector3D vNormals;
                for (a =  poResult.begin();a != end;++a)    {
                    const aiVector3D& v = faceNormals[*a];
                    vNormals += v;
                }
                vNormals.Normalize();
                for (a =  poResult.begin();a != end;++a)    {
                    mesh->mNormals[*a] = vNormals;
                    vertexDone[*a] = true;
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
void LWOImporter::GenerateNodeGraph(std::map<uint16_t,aiNode*>& apcNodes)
{
    // now generate the final nodegraph - generate a root node and attach children
    aiNode* root = pScene->mRootNode = new aiNode();
    root->mName.Set("<LWORoot>");

    //Set parent of all children, inserting pivots
    //std::cout << "Set parent of all children" << std::endl;
    std::map<uint16_t, aiNode*> mapPivot;
    for (auto itapcNodes = apcNodes.begin(); itapcNodes != apcNodes.end(); ++itapcNodes) {

        //Get the parent index
        LWO::Layer* nodeLayer = (LWO::Layer*)(itapcNodes->second->mParent);
        uint16_t parentIndex = nodeLayer->mParent;

        //Create pivot node, store it into the pivot map, and set the parent as the pivot
        aiNode* pivotNode = new aiNode();
        pivotNode->mName.Set("Pivot-"+std::string(itapcNodes->second->mName.data));
        itapcNodes->second->mParent = pivotNode;

        //Look for the parent node to attach the pivot to
        if (apcNodes.find(parentIndex) != apcNodes.end()) {
            pivotNode->mParent = apcNodes[parentIndex];
        } else {
            //If not, attach to the root node
            pivotNode->mParent = root;
        }

        //Set the node and the pivot node transformation
        itapcNodes->second->mTransformation.a4 = -nodeLayer->mPivot.x;
        itapcNodes->second->mTransformation.b4 = -nodeLayer->mPivot.y;
        itapcNodes->second->mTransformation.c4 = -nodeLayer->mPivot.z;
        pivotNode->mTransformation.a4 = nodeLayer->mPivot.x;
        pivotNode->mTransformation.b4 = nodeLayer->mPivot.y;
        pivotNode->mTransformation.c4 = nodeLayer->mPivot.z;
        mapPivot[-(itapcNodes->first+2)] = pivotNode;
    }

    //Merge pivot map into node map
    //std::cout << "Merge pivot map into node map" << std::endl;
    for (auto itMapPivot = mapPivot.begin(); itMapPivot != mapPivot.end(); ++itMapPivot) {
        apcNodes[itMapPivot->first] = itMapPivot->second;
    }

    //Set children of all parents
    apcNodes[-1] = root;
    for (auto itMapParentNodes = apcNodes.begin(); itMapParentNodes != apcNodes.end(); ++itMapParentNodes) {
        for (auto itMapChildNodes = apcNodes.begin(); itMapChildNodes != apcNodes.end(); ++itMapChildNodes) {
            if ((itMapParentNodes->first != itMapChildNodes->first) && (itMapParentNodes->second == itMapChildNodes->second->mParent)) {
                ++(itMapParentNodes->second->mNumChildren);
            }
        }
        if (itMapParentNodes->second->mNumChildren) {
            itMapParentNodes->second->mChildren = new aiNode* [ itMapParentNodes->second->mNumChildren ];
            uint16_t p = 0;
            for (auto itMapChildNodes = apcNodes.begin(); itMapChildNodes != apcNodes.end(); ++itMapChildNodes) {
                if ((itMapParentNodes->first != itMapChildNodes->first) && (itMapParentNodes->second == itMapChildNodes->second->mParent)) {
                    itMapParentNodes->second->mChildren[p++] = itMapChildNodes->second;
                }
            }
        }
    }

    if (!pScene->mRootNode->mNumChildren)
        throw DeadlyImportError("LWO: Unable to build a valid node graph");

    // Remove a single root node with no meshes assigned to it ...
    if (1 == pScene->mRootNode->mNumChildren)   {
        aiNode* pc = pScene->mRootNode->mChildren[0];
        pc->mParent = pScene->mRootNode->mChildren[0] = NULL;
        delete pScene->mRootNode;
        pScene->mRootNode = pc;
    }

    // convert the whole stuff to RH with CCW winding
    MakeLeftHandedProcess maker;
    maker.Execute(pScene);

    FlipWindingOrderProcess flipper;
    flipper.Execute(pScene);
}

// ------------------------------------------------------------------------------------------------
void LWOImporter::ResolveTags()
{
    // --- this function is used for both LWO2 and LWOB
    mMapping->resize(mTags->size(), UINT_MAX);
    for (unsigned int a = 0; a  < mTags->size();++a)    {

        const std::string& c = (*mTags)[a];
        for (unsigned int i = 0; i < mSurfaces->size();++i) {

            const std::string& d = (*mSurfaces)[i].mName;
            if (!ASSIMP_stricmp(c,d))   {

                (*mMapping)[a] = i;
                break;
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
void LWOImporter::ResolveClips()
{
    for( unsigned int i = 0; i < mClips.size();++i) {

        Clip& clip = mClips[i];
        if (Clip::REF == clip.type) {

            if (clip.clipRef >= mClips.size())  {
                ASSIMP_LOG_ERROR("LWO2: Clip referrer index is out of range");
                clip.clipRef = 0;
            }

            Clip& dest = mClips[clip.clipRef];
            if (Clip::REF == dest.type) {
                ASSIMP_LOG_ERROR("LWO2: Clip references another clip reference");
                clip.type = Clip::UNSUPPORTED;
            }

            else    {
                clip.path = dest.path;
                clip.type = dest.type;
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
void LWOImporter::AdjustTexturePath(std::string& out)
{
    // --- this function is used for both LWO2 and LWOB
    if (!mIsLWO2 && ::strstr(out.c_str(), "(sequence)"))    {

        // remove the (sequence) and append 000
        ASSIMP_LOG_INFO("LWOB: Sequence of animated texture found. It will be ignored");
        out = out.substr(0,out.length()-10) + "000";
    }

    // format: drive:path/file - we just need to insert a slash after the drive
    std::string::size_type n = out.find_first_of(':');
    if (std::string::npos != n) {
        out.insert(n+1,"/");
    }
}

// ------------------------------------------------------------------------------------------------
void LWOImporter::LoadLWOTags(unsigned int size)
{
    // --- this function is used for both LWO2 and LWOB

    const char* szCur = (const char*)mFileBuffer, *szLast = szCur;
    const char* const szEnd = szLast+size;
    while (szCur < szEnd)
    {
        if (!(*szCur))
        {
            const size_t len = (size_t)(szCur-szLast);
            // FIX: skip empty-sized tags
            if (len)
                mTags->push_back(std::string(szLast,len));
            szCur += (len&0x1 ? 1 : 2);
            szLast = szCur;
        }
        szCur++;
    }
}

// ------------------------------------------------------------------------------------------------
void LWOImporter::LoadLWOPoints(unsigned int length)
{
    // --- this function is used for both LWO2 and LWOB but for
    // LWO2 we need to allocate 25% more storage - it could be we'll
    // need to duplicate some points later.
    const size_t vertexLen = 12;
    if ((length % vertexLen) != 0)
    {
        throw DeadlyImportError( "LWO2: Points chunk length is not multiple of vertexLen (12)");
    }
    unsigned int regularSize = (unsigned int)mCurLayer->mTempPoints.size() + length / 12;
    if (mIsLWO2)
    {
        mCurLayer->mTempPoints.reserve  ( regularSize + (regularSize>>2u) );
        mCurLayer->mTempPoints.resize   ( regularSize );

        // initialize all point referrers with the default values
        mCurLayer->mPointReferrers.reserve  ( regularSize + (regularSize>>2u) );
        mCurLayer->mPointReferrers.resize   ( regularSize, UINT_MAX );
    }
    else mCurLayer->mTempPoints.resize( regularSize );

    // perform endianness conversions
#ifndef AI_BUILD_BIG_ENDIAN
    for (unsigned int i = 0; i < length>>2;++i)
        ByteSwap::Swap4( mFileBuffer + (i << 2));
#endif
    ::memcpy(&mCurLayer->mTempPoints[0],mFileBuffer,length);
}

// ------------------------------------------------------------------------------------------------
void LWOImporter::LoadLWO2Polygons(unsigned int length)
{
    LE_NCONST uint16_t* const end   = (LE_NCONST uint16_t*)(mFileBuffer+length);
    const uint32_t type = GetU4();

    // Determine the type of the polygons
    switch (type)
    {
        // read unsupported stuff too (although we won't process it)
    case  AI_LWO_MBAL:
        ASSIMP_LOG_WARN("LWO2: Encountered unsupported primitive chunk (METABALL)");
        break;
    case  AI_LWO_CURV:
        ASSIMP_LOG_WARN("LWO2: Encountered unsupported primitive chunk (SPLINE)");;
        break;

        // These are ok with no restrictions
    case  AI_LWO_PTCH:
    case  AI_LWO_FACE:
    case  AI_LWO_BONE:
    case  AI_LWO_SUBD:
        break;
    default:

        // hm!? wtf is this? ok ...
        ASSIMP_LOG_ERROR("LWO2: Ignoring unknown polygon type.");
        break;
    }

    // first find out how many faces and vertices we'll finally need
    uint16_t* cursor= (uint16_t*)mFileBuffer;

    unsigned int iNumFaces = 0,iNumVertices = 0;
    CountVertsAndFacesLWO2(iNumVertices,iNumFaces,cursor,end);

    // allocate the output array and copy face indices
    if (iNumFaces)
    {
        cursor = (uint16_t*)mFileBuffer;

        mCurLayer->mFaces.resize(iNumFaces,LWO::Face(type));
        FaceList::iterator it = mCurLayer->mFaces.begin();
        CopyFaceIndicesLWO2(it,cursor,end);
    }
}

// ------------------------------------------------------------------------------------------------
void LWOImporter::CountVertsAndFacesLWO2(unsigned int& verts, unsigned int& faces,
    uint16_t*& cursor, const uint16_t* const end, unsigned int max)
{
    while (cursor < end && max--)
    {
        uint16_t numIndices;
        ::memcpy(&numIndices, cursor++, 2);
        AI_LSWAP2(numIndices);
        numIndices &= 0x03FF;

        verts += numIndices;
        ++faces;

        for(uint16_t i = 0; i < numIndices; i++)
        {
            ReadVSizedIntLWO2((uint8_t*&)cursor);
        }
    }
}

// ------------------------------------------------------------------------------------------------
void LWOImporter::CopyFaceIndicesLWO2(FaceList::iterator& it,
    uint16_t*& cursor,
    const uint16_t* const end)
{
    while (cursor < end)
    {
        LWO::Face& face = *it++;
        uint16_t numIndices;
        ::memcpy(&numIndices, cursor++, 2);
        AI_LSWAP2(numIndices);
        face.mNumIndices = numIndices & 0x03FF;

        if(face.mNumIndices) /* byte swapping has already been done */
        {
            face.mIndices = new unsigned int[face.mNumIndices];
            for(unsigned int i = 0; i < face.mNumIndices; i++)
            {
                face.mIndices[i] = ReadVSizedIntLWO2((uint8_t*&)cursor) + mCurLayer->mPointIDXOfs;
                if(face.mIndices[i] > mCurLayer->mTempPoints.size())
                {
                    ASSIMP_LOG_WARN("LWO2: Failure evaluating face record, index is out of range");
                    face.mIndices[i] = (unsigned int)mCurLayer->mTempPoints.size()-1;
                }
            }
        }
        else throw DeadlyImportError("LWO2: Encountered invalid face record with zero indices");
    }
}


// ------------------------------------------------------------------------------------------------
void LWOImporter::LoadLWO2PolygonTags(unsigned int length)
{
    LE_NCONST uint8_t* const end = mFileBuffer+length;

    AI_LWO_VALIDATE_CHUNK_LENGTH(length,PTAG,4);
    uint32_t type = GetU4();

    if (type != AI_LWO_SURF && type != AI_LWO_SMGP)
        return;

    while (mFileBuffer < end)
    {
        unsigned int i = ReadVSizedIntLWO2(mFileBuffer) + mCurLayer->mFaceIDXOfs;
        unsigned int j = GetU2();

        if (i >= mCurLayer->mFaces.size())  {
            ASSIMP_LOG_WARN("LWO2: face index in PTAG is out of range");
            continue;
        }

        switch (type)   {

        case AI_LWO_SURF:
            mCurLayer->mFaces[i].surfaceIndex = j;
            break;
        case AI_LWO_SMGP: /* is that really used? */
            mCurLayer->mFaces[i].smoothGroup = j;
            break;
        };
    }
}

// ------------------------------------------------------------------------------------------------
template <class T>
VMapEntry* FindEntry(std::vector< T >& list,const std::string& name, bool perPoly)
{
    for (auto & elem : list)   {
        if (elem.name == name) {
            if (!perPoly)   {
                ASSIMP_LOG_WARN("LWO2: Found two VMAP sections with equal names");
            }
            return &elem;
        }
    }
    list.push_back( T() );
    VMapEntry* p = &list.back();
    p->name = name;
    return p;
}

// ------------------------------------------------------------------------------------------------
template <class T>
inline void CreateNewEntry(T& chan, unsigned int srcIdx)
{
    if (!chan.name.length())
        return;

    chan.abAssigned[srcIdx] = true;
    chan.abAssigned.resize(chan.abAssigned.size()+1,false);

    for (unsigned int a = 0; a < chan.dims;++a)
        chan.rawData.push_back(chan.rawData[srcIdx*chan.dims+a]);
}

// ------------------------------------------------------------------------------------------------
template <class T>
inline void CreateNewEntry(std::vector< T >& list, unsigned int srcIdx)
{
    for (auto &elem : list)   {
        CreateNewEntry( elem, srcIdx );
    }
}

// ------------------------------------------------------------------------------------------------
inline void LWOImporter::DoRecursiveVMAPAssignment(VMapEntry* base, unsigned int numRead,
    unsigned int idx, float* data)
{
    ai_assert(NULL != data);
    LWO::ReferrerList& refList  = mCurLayer->mPointReferrers;
    unsigned int i;

    if (idx >= base->abAssigned.size()) {
        throw DeadlyImportError("Bad index");
    }
    base->abAssigned[idx] = true;
    for (i = 0; i < numRead;++i) {
        base->rawData[idx*base->dims+i]= data[i];
    }

    if (UINT_MAX != (i = refList[idx])) {
        DoRecursiveVMAPAssignment(base,numRead,i,data);
    }
}

// ------------------------------------------------------------------------------------------------
inline void AddToSingleLinkedList(ReferrerList& refList, unsigned int srcIdx, unsigned int destIdx)
{
    if(UINT_MAX == refList[srcIdx]) {
        refList[srcIdx] = destIdx;
        return;
    }
    AddToSingleLinkedList(refList,refList[srcIdx],destIdx);
}

// ------------------------------------------------------------------------------------------------
// Load LWO2 vertex map
void LWOImporter::LoadLWO2VertexMap(unsigned int length, bool perPoly)
{
    LE_NCONST uint8_t* const end = mFileBuffer+length;

    AI_LWO_VALIDATE_CHUNK_LENGTH(length,VMAP,6);
    unsigned int type = GetU4();
    unsigned int dims = GetU2();

    VMapEntry* base;

    // read the name of the vertex map
    std::string name;
    GetS0(name,length);

    switch (type)
    {
    case AI_LWO_TXUV:
        if (dims != 2)  {
            ASSIMP_LOG_WARN("LWO2: Skipping UV channel \'"
            + name + "\' with !2 components");
            return;
        }
        base = FindEntry(mCurLayer->mUVChannels,name,perPoly);
        break;
    case AI_LWO_WGHT:
    case AI_LWO_MNVW:
        if (dims != 1)  {
            ASSIMP_LOG_WARN("LWO2: Skipping Weight Channel \'"
            + name + "\' with !1 components");
            return;
        }
        base = FindEntry((type == AI_LWO_WGHT ? mCurLayer->mWeightChannels
            : mCurLayer->mSWeightChannels),name,perPoly);
        break;
    case AI_LWO_RGB:
    case AI_LWO_RGBA:
        if (dims != 3 && dims != 4) {
            ASSIMP_LOG_WARN("LWO2: Skipping Color Map \'"
            + name + "\' with a dimension > 4 or < 3");
            return;
        }
        base = FindEntry(mCurLayer->mVColorChannels,name,perPoly);
        break;

    case AI_LWO_MODO_NORM:
        /*  This is a non-standard extension chunk used by Luxology's MODO.
         *  It stores per-vertex normals. This VMAP exists just once, has
         *  3 dimensions and is btw extremely beautiful.
         */
        if (name != "vert_normals" || dims != 3 || mCurLayer->mNormals.name.length())
            return;

        ASSIMP_LOG_INFO("Processing non-standard extension: MODO VMAP.NORM.vert_normals");

        mCurLayer->mNormals.name = name;
        base = & mCurLayer->mNormals;
        break;

    case AI_LWO_PICK: /* these VMAPs are just silently dropped */
    case AI_LWO_MORF:
    case AI_LWO_SPOT:
        return;

    default:
        if (name == "APS.Level") {
            // XXX handle this (seems to be subdivision-related).
        }
        ASSIMP_LOG_WARN_F("LWO2: Skipping unknown VMAP/VMAD channel \'", name, "\'");
        return;
    };
    base->Allocate((unsigned int)mCurLayer->mTempPoints.size());

    // now read all entries in the map
    type = std::min(dims,base->dims);
    const unsigned int diff = (dims - type)<<2u;

    LWO::FaceList& list = mCurLayer->mFaces;
    LWO::PointList& pointList = mCurLayer->mTempPoints;
    LWO::ReferrerList& refList = mCurLayer->mPointReferrers;

    const unsigned int numPoints = (unsigned int)pointList.size();
    const unsigned int numFaces  = (unsigned int)list.size();

    while (mFileBuffer < end)   {

        unsigned int idx = ReadVSizedIntLWO2(mFileBuffer) + mCurLayer->mPointIDXOfs;
        if (idx >= numPoints)   {
            ASSIMP_LOG_WARN_F("LWO2: Failure evaluating VMAP/VMAD entry \'", name, "\', vertex index is out of range");
            mFileBuffer += base->dims<<2u;
            continue;
        }
        if (perPoly)    {
            unsigned int polyIdx = ReadVSizedIntLWO2(mFileBuffer) + mCurLayer->mFaceIDXOfs;
            if (base->abAssigned[idx])  {
                // we have already a VMAP entry for this vertex - thus
                // we need to duplicate the corresponding polygon.
                if (polyIdx >= numFaces)    {
                    ASSIMP_LOG_WARN_F("LWO2: Failure evaluating VMAD entry \'", name, "\', polygon index is out of range");
                    mFileBuffer += base->dims<<2u;
                    continue;
                }

                LWO::Face& src = list[polyIdx];

                // generate a new unique vertex for the corresponding index - but only
                // if we can find the index in the face
                bool had = false;
                for (unsigned int i = 0; i < src.mNumIndices;++i)   {

                    unsigned int srcIdx = src.mIndices[i], tmp = idx;
                    do {
                        if (tmp == srcIdx)
                            break;
                    }
                    while ((tmp = refList[tmp]) != UINT_MAX);
                    if (tmp == UINT_MAX) {
                        continue;
                    }

                    had = true;
                    refList.resize(refList.size()+1, UINT_MAX);

                    idx = (unsigned int)pointList.size();
                    src.mIndices[i] = (unsigned int)pointList.size();

                    // store the index of the new vertex in the old vertex
                    // so we get a single linked list we can traverse in
                    // only one direction
                    AddToSingleLinkedList(refList,srcIdx,src.mIndices[i]);
                    pointList.push_back(pointList[srcIdx]);

                    CreateNewEntry(mCurLayer->mVColorChannels,  srcIdx );
                    CreateNewEntry(mCurLayer->mUVChannels,      srcIdx );
                    CreateNewEntry(mCurLayer->mWeightChannels,  srcIdx );
                    CreateNewEntry(mCurLayer->mSWeightChannels, srcIdx );
                    CreateNewEntry(mCurLayer->mNormals, srcIdx );
                }
                if (!had) {
                    ASSIMP_LOG_WARN_F("LWO2: Failure evaluating VMAD entry \'", name, "\', vertex index wasn't found in that polygon");
                    ai_assert(had);
                }
            }
        }

        std::unique_ptr<float[]> temp(new float[type]);
        for (unsigned int l = 0; l < type;++l)
            temp[l] = GetF4();

        DoRecursiveVMAPAssignment(base,type,idx, temp.get());
        mFileBuffer += diff;
    }
}

// ------------------------------------------------------------------------------------------------
// Load LWO2 clip
void LWOImporter::LoadLWO2Clip(unsigned int length)
{
    AI_LWO_VALIDATE_CHUNK_LENGTH(length,CLIP,10);

    mClips.push_back(LWO::Clip());
    LWO::Clip& clip = mClips.back();

    // first - get the index of the clip
    clip.idx = GetU4();

    IFF::SubChunkHeader head = IFF::LoadSubChunk(mFileBuffer);
    switch (head.type)
    {
    case AI_LWO_STIL:
        AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,STIL,1);

        // "Normal" texture
        GetS0(clip.path,head.length);
        clip.type = Clip::STILL;
        break;

    case AI_LWO_ISEQ:
        AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,ISEQ,16);
        // Image sequence. We'll later take the first.
        {
            uint8_t digits = GetU1();  mFileBuffer++;
            int16_t offset = GetU2();  mFileBuffer+=4;
            int16_t start  = GetU2();  mFileBuffer+=4;

            std::string s;
            std::ostringstream ss;
            GetS0(s,head.length);

            head.length -= (uint16_t)s.length()+1;
            ss << s;
            ss << std::setw(digits) << offset + start;
            GetS0(s,head.length);
            ss << s;
            clip.path = ss.str();
            clip.type = Clip::SEQ;
        }
        break;

    case AI_LWO_STCC:
        ASSIMP_LOG_WARN("LWO2: Color shifted images are not supported");
        break;

    case AI_LWO_ANIM:
        ASSIMP_LOG_WARN("LWO2: Animated textures are not supported");
        break;

    case AI_LWO_XREF:
        AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,XREF,4);

        // Just a cross-reference to another CLIp
        clip.type = Clip::REF;
        clip.clipRef = GetU4();
        break;

    case AI_LWO_NEGA:
        AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,NEGA,2);
        clip.negate = (0 != GetU2());
        break;

    default:
        ASSIMP_LOG_WARN("LWO2: Encountered unknown CLIP sub-chunk");
    }
}

// ------------------------------------------------------------------------------------------------
// Load envelope description
void LWOImporter::LoadLWO2Envelope(unsigned int length)
{
    LE_NCONST uint8_t* const end = mFileBuffer + length;
    AI_LWO_VALIDATE_CHUNK_LENGTH(length,ENVL,4);

    mEnvelopes.push_back(LWO::Envelope());
    LWO::Envelope& envelope = mEnvelopes.back();

    // Get the index of the envelope
    envelope.index = ReadVSizedIntLWO2(mFileBuffer);

    // It looks like there might be an extra U4 right after the index,
    // at least in modo (LXOB) files: we'll ignore it if it's zero,
    // otherwise it represents the start of a subchunk, so we backtrack.
    if (mIsLXOB)
    {
        uint32_t extra = GetU4();
        if (extra)
        {
            mFileBuffer -= 4;
        }
    }

    // ... and read all subchunks
    while (true)
    {
        if (mFileBuffer + 6 >= end)break;
        LE_NCONST IFF::SubChunkHeader head = IFF::LoadSubChunk(mFileBuffer);

        if (mFileBuffer + head.length > end)
            throw DeadlyImportError("LWO2: Invalid envelope chunk length");

        uint8_t* const next = mFileBuffer+head.length;
        switch (head.type)
        {
            // Type & representation of the envelope
        case AI_LWO_TYPE:
            AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,TYPE,2);
            mFileBuffer++; // skip user format

            // Determine type of envelope
            envelope.type  = (LWO::EnvelopeType)*mFileBuffer;
            ++mFileBuffer;
            break;

            // precondition
        case AI_LWO_PRE:
            AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,PRE,2);
            envelope.pre = (LWO::PrePostBehaviour)GetU2();
            break;

            // postcondition
        case AI_LWO_POST:
            AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,POST,2);
            envelope.post = (LWO::PrePostBehaviour)GetU2();
            break;

            // keyframe
        case AI_LWO_KEY:
            {
            AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,KEY,8);

            envelope.keys.push_back(LWO::Key());
            LWO::Key& key = envelope.keys.back();

            key.time = GetF4();
            key.value = GetF4();
            break;
            }

            // interval interpolation
        case AI_LWO_SPAN:
            {
                AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,SPAN,4);
                if (envelope.keys.size()<2)
                    ASSIMP_LOG_WARN("LWO2: Unexpected SPAN chunk");
                else {
                    LWO::Key& key = envelope.keys.back();
                    switch (GetU4())
                    {
                        case AI_LWO_STEP:
                            key.inter = LWO::IT_STEP;break;
                        case AI_LWO_LINE:
                            key.inter = LWO::IT_LINE;break;
                        case AI_LWO_TCB:
                            key.inter = LWO::IT_TCB;break;
                        case AI_LWO_HERM:
                            key.inter = LWO::IT_HERM;break;
                        case AI_LWO_BEZI:
                            key.inter = LWO::IT_BEZI;break;
                        case AI_LWO_BEZ2:
                            key.inter = LWO::IT_BEZ2;break;
                        default:
                            ASSIMP_LOG_WARN("LWO2: Unknown interval interpolation mode");
                    };

                    // todo ... read params
                }
                break;
            }

        default:
            ASSIMP_LOG_WARN("LWO2: Encountered unknown ENVL subchunk");
            break;
        }
        // regardless how much we did actually read, go to the next chunk
        mFileBuffer = next;
    }
}

// ------------------------------------------------------------------------------------------------
// Load file - master function
void LWOImporter::LoadLWO2File()
{
    bool skip = false;

    LE_NCONST uint8_t* const end = mFileBuffer + fileSize;
    while (true)
    {
        if (mFileBuffer + sizeof(IFF::ChunkHeader) > end)break;
        const IFF::ChunkHeader head = IFF::LoadChunk(mFileBuffer);

        if (mFileBuffer + head.length > end)
        {
            throw DeadlyImportError("LWO2: Chunk length points behind the file");
            break;
        }
        uint8_t* const next = mFileBuffer+head.length;
        unsigned int iUnnamed = 0;

        if(!head.length) {
            mFileBuffer = next;
            continue;
        }

        switch (head.type)
        {
            // new layer
        case AI_LWO_LAYR:
            {
                // add a new layer to the list ....
                mLayers->push_back ( LWO::Layer() );
                LWO::Layer& layer = mLayers->back();
                mCurLayer = &layer;

                AI_LWO_VALIDATE_CHUNK_LENGTH(head.length,LAYR,16);

                // layer index.
                layer.mIndex = GetU2();

                // Continue loading this layer or ignore it? Check the layer index property
                if (UINT_MAX != configLayerIndex && (configLayerIndex-1) != layer.mIndex)   {
                    skip = true;
                }
                else skip = false;

                // pivot point
                mFileBuffer += 2; /* unknown */
                mCurLayer->mPivot.x = GetF4();
                mCurLayer->mPivot.y = GetF4();
                mCurLayer->mPivot.z = GetF4();
                GetS0(layer.mName,head.length-16);

                // if the name is empty, generate a default name
                if (layer.mName.empty())    {
                    char buffer[128]; // should be sufficiently large
                    ::ai_snprintf(buffer, 128, "Layer_%i", iUnnamed++);
                    layer.mName = buffer;
                }

                // load this layer or ignore it? Check the layer name property
                if (configLayerName.length() && configLayerName != layer.mName) {
                    skip = true;
                }
                else hasNamedLayer = true;

                // optional: parent of this layer
                if (mFileBuffer + 2 <= next)
                    layer.mParent = GetU2();
                else layer.mParent = -1;

                // Set layer skip parameter
                layer.skip = skip;

                break;
            }

            // vertex list
        case AI_LWO_PNTS:
            {
                if (skip)
                    break;

                unsigned int old = (unsigned int)mCurLayer->mTempPoints.size();
                LoadLWOPoints(head.length);
                mCurLayer->mPointIDXOfs = old;
                break;
            }
            // vertex tags
        case AI_LWO_VMAD:
            if (mCurLayer->mFaces.empty())
            {
                ASSIMP_LOG_WARN("LWO2: Unexpected VMAD chunk");
                break;
            }
            // --- intentionally no break here
        case AI_LWO_VMAP:
            {
                if (skip)
                    break;

                if (mCurLayer->mTempPoints.empty())
                    ASSIMP_LOG_WARN("LWO2: Unexpected VMAP chunk");
                else LoadLWO2VertexMap(head.length,head.type == AI_LWO_VMAD);
                break;
            }
            // face list
        case AI_LWO_POLS:
            {
                if (skip)
                    break;

                unsigned int old = (unsigned int)mCurLayer->mFaces.size();
                LoadLWO2Polygons(head.length);
                mCurLayer->mFaceIDXOfs = old;
                break;
            }
            // polygon tags
        case AI_LWO_PTAG:
            {
                if (skip)
                    break;

                if (mCurLayer->mFaces.empty()) {
                    ASSIMP_LOG_WARN("LWO2: Unexpected PTAG");
                } else {
                    LoadLWO2PolygonTags(head.length);
                }
                break;
            }
            // list of tags
        case AI_LWO_TAGS:
            {
                if (!mTags->empty()) {
                    ASSIMP_LOG_WARN("LWO2: SRFS chunk encountered twice");
                }   else {
                    LoadLWOTags(head.length);
                }
                break;
            }

            // surface chunk
        case AI_LWO_SURF:
            {
                LoadLWO2Surface(head.length);
                break;
            }

            // clip chunk
        case AI_LWO_CLIP:
            {
                LoadLWO2Clip(head.length);
                break;
            }

            // envelope chunk
        case AI_LWO_ENVL:
            {
                LoadLWO2Envelope(head.length);
                break;
            }
        }
        mFileBuffer = next;
    }
}

#endif // !! ASSIMP_BUILD_NO_LWO_IMPORTER
