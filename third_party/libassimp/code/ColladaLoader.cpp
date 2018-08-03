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

/** @file Implementation of the Collada loader */


#ifndef ASSIMP_BUILD_NO_COLLADA_IMPORTER

#include "ColladaLoader.h"
#include <assimp/anim.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/importerdesc.h>

#include "ColladaParser.h"
#include "fast_atof.h"
#include "ParsingUtils.h"
#include "SkeletonMeshBuilder.h"
#include "CreateAnimMesh.h"

#include "time.h"
#include "math.h"
#include <algorithm>
#include <numeric>
#include <assimp/Defines.h>

using namespace Assimp;
using namespace Assimp::Formatter;

static const aiImporterDesc desc = {
    "Collada Importer",
    "",
    "",
    "http://collada.org",
    aiImporterFlags_SupportTextFlavour,
    1,
    3,
    1,
    5,
    "dae"
};

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
ColladaLoader::ColladaLoader()
    : mFileName()
	, mMeshIndexByID()
	, mMaterialIndexByName()
	, mMeshes()
	, newMats()
	, mCameras()
	, mLights()
	, mTextures()
	, mAnims()
	, noSkeletonMesh( false )
    , ignoreUpDirection(false)
    , mNodeNameCounter( 0 )
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
ColladaLoader::~ColladaLoader()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool ColladaLoader::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    // check file extension
    std::string extension = GetExtension(pFile);

    if( extension == "dae")
        return true;

    // XML - too generic, we need to open the file and search for typical keywords
    if( extension == "xml" || !extension.length() || checkSig)  {
        /*  If CanRead() is called in order to check whether we
         *  support a specific file extension in general pIOHandler
         *  might be NULL and it's our duty to return true here.
         */
        if (!pIOHandler)return true;
        const char* tokens[] = {"<collada"};
        return SearchFileHeaderForToken(pIOHandler,pFile,tokens,1);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
void ColladaLoader::SetupProperties(const Importer* pImp)
{
    noSkeletonMesh = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_NO_SKELETON_MESHES,0) != 0;
    ignoreUpDirection = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_COLLADA_IGNORE_UP_DIRECTION,0) != 0;
}

// ------------------------------------------------------------------------------------------------
// Get file extension list
const aiImporterDesc* ColladaLoader::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void ColladaLoader::InternReadFile( const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler)
{
    mFileName = pFile;

    // clean all member arrays - just for safety, it should work even if we did not
    mMeshIndexByID.clear();
    mMaterialIndexByName.clear();
    mMeshes.clear();
    mTargetMeshes.clear();
    newMats.clear();
    mLights.clear();
    mCameras.clear();
    mTextures.clear();
    mAnims.clear();

    // parse the input file
    ColladaParser parser( pIOHandler, pFile);

    if( !parser.mRootNode)
        throw DeadlyImportError( "Collada: File came out empty. Something is wrong here.");

    // reserve some storage to avoid unnecessary reallocs
    newMats.reserve(parser.mMaterialLibrary.size()*2);
    mMeshes.reserve(parser.mMeshLibrary.size()*2);

    mCameras.reserve(parser.mCameraLibrary.size());
    mLights.reserve(parser.mLightLibrary.size());

    // create the materials first, for the meshes to find
    BuildMaterials( parser, pScene);

    // build the node hierarchy from it
    pScene->mRootNode = BuildHierarchy( parser, parser.mRootNode);

    // ... then fill the materials with the now adjusted settings
    FillMaterials(parser, pScene);

        // Apply unitsize scale calculation
        pScene->mRootNode->mTransformation *= aiMatrix4x4(parser.mUnitSize, 0,  0,  0,
                                                          0,  parser.mUnitSize,  0,  0,
                                                          0,  0,  parser.mUnitSize,  0,
                                                          0,  0,  0,  1);
        if( !ignoreUpDirection ) {
        // Convert to Y_UP, if different orientation
        if( parser.mUpDirection == ColladaParser::UP_X)
            pScene->mRootNode->mTransformation *= aiMatrix4x4(
                 0, -1,  0,  0,
                 1,  0,  0,  0,
                 0,  0,  1,  0,
                 0,  0,  0,  1);
        else if( parser.mUpDirection == ColladaParser::UP_Z)
            pScene->mRootNode->mTransformation *= aiMatrix4x4(
                 1,  0,  0,  0,
                 0,  0,  1,  0,
                 0, -1,  0,  0,
                 0,  0,  0,  1);
        }
    // store all meshes
    StoreSceneMeshes( pScene);

    // store all materials
    StoreSceneMaterials( pScene);

    // store all lights
    StoreSceneLights( pScene);

    // store all cameras
    StoreSceneCameras( pScene);

    // store all animations
    StoreAnimations( pScene, parser);


    // If no meshes have been loaded, it's probably just an animated skeleton.
    if (!pScene->mNumMeshes) {

        if (!noSkeletonMesh) {
            SkeletonMeshBuilder hero(pScene);
        }
        pScene->mFlags |= AI_SCENE_FLAGS_INCOMPLETE;
    }
}

// ------------------------------------------------------------------------------------------------
// Recursively constructs a scene node for the given parser node and returns it.
aiNode* ColladaLoader::BuildHierarchy( const ColladaParser& pParser, const Collada::Node* pNode)
{
    // create a node for it
    aiNode* node = new aiNode();

    // find a name for the new node. It's more complicated than you might think
    node->mName.Set( FindNameForNode( pNode));

    // calculate the transformation matrix for it
    node->mTransformation = pParser.CalculateResultTransform( pNode->mTransforms);

    // now resolve node instances
    std::vector<const Collada::Node*> instances;
    ResolveNodeInstances(pParser,pNode,instances);

    // add children. first the *real* ones
    node->mNumChildren = static_cast<unsigned int>(pNode->mChildren.size()+instances.size());
    node->mChildren = new aiNode*[node->mNumChildren];

    for( size_t a = 0; a < pNode->mChildren.size(); a++)
    {
        node->mChildren[a] = BuildHierarchy( pParser, pNode->mChildren[a]);
        node->mChildren[a]->mParent = node;
    }

    // ... and finally the resolved node instances
    for( size_t a = 0; a < instances.size(); a++)
    {
        node->mChildren[pNode->mChildren.size() + a] = BuildHierarchy( pParser, instances[a]);
        node->mChildren[pNode->mChildren.size() + a]->mParent = node;
    }

    // construct meshes
    BuildMeshesForNode( pParser, pNode, node);

    // construct cameras
    BuildCamerasForNode(pParser, pNode, node);

    // construct lights
    BuildLightsForNode(pParser, pNode, node);
    return node;
}

// ------------------------------------------------------------------------------------------------
// Resolve node instances
void ColladaLoader::ResolveNodeInstances( const ColladaParser& pParser, const Collada::Node* pNode,
    std::vector<const Collada::Node*>& resolved)
{
    // reserve enough storage
    resolved.reserve(pNode->mNodeInstances.size());

    // ... and iterate through all nodes to be instanced as children of pNode
    for (const auto &nodeInst: pNode->mNodeInstances)
    {
        // find the corresponding node in the library
        const ColladaParser::NodeLibrary::const_iterator itt = pParser.mNodeLibrary.find(nodeInst.mNode);
        const Collada::Node* nd = itt == pParser.mNodeLibrary.end() ? NULL : (*itt).second;

        // FIX for http://sourceforge.net/tracker/?func=detail&aid=3054873&group_id=226462&atid=1067632
        // need to check for both name and ID to catch all. To avoid breaking valid files,
        // the workaround is only enabled when the first attempt to resolve the node has failed.
        if (!nd) {
            nd = FindNode(pParser.mRootNode, nodeInst.mNode);
        }
        if (!nd)
            DefaultLogger::get()->error("Collada: Unable to resolve reference to instanced node " + nodeInst.mNode);

        else {
            //  attach this node to the list of children
            resolved.push_back(nd);
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Resolve UV channels
void ColladaLoader::ApplyVertexToEffectSemanticMapping(Collada::Sampler& sampler,
     const Collada::SemanticMappingTable& table)
{
    std::map<std::string, Collada::InputSemanticMapEntry>::const_iterator it = table.mMap.find(sampler.mUVChannel);
    if (it != table.mMap.end()) {
        if (it->second.mType != Collada::IT_Texcoord)
            DefaultLogger::get()->error("Collada: Unexpected effect input mapping");

        sampler.mUVId = it->second.mSet;
    }
}

// ------------------------------------------------------------------------------------------------
// Builds lights for the given node and references them
void ColladaLoader::BuildLightsForNode( const ColladaParser& pParser, const Collada::Node* pNode, aiNode* pTarget)
{
    for( const Collada::LightInstance& lid : pNode->mLights)
    {
        // find the referred light
        ColladaParser::LightLibrary::const_iterator srcLightIt = pParser.mLightLibrary.find( lid.mLight);
        if( srcLightIt == pParser.mLightLibrary.end())
        {
            DefaultLogger::get()->warn("Collada: Unable to find light for ID \"" + lid.mLight + "\". Skipping.");
            continue;
        }
        const Collada::Light* srcLight = &srcLightIt->second;

        // now fill our ai data structure
        aiLight* out = new aiLight();
        out->mName = pTarget->mName;
        out->mType = (aiLightSourceType)srcLight->mType;

        // collada lights point in -Z by default, rest is specified in node transform
        out->mDirection = aiVector3D(0.f,0.f,-1.f);

        out->mAttenuationConstant = srcLight->mAttConstant;
        out->mAttenuationLinear = srcLight->mAttLinear;
        out->mAttenuationQuadratic = srcLight->mAttQuadratic;

        out->mColorDiffuse = out->mColorSpecular = out->mColorAmbient = srcLight->mColor*srcLight->mIntensity;
        if (out->mType == aiLightSource_AMBIENT) {
            out->mColorDiffuse = out->mColorSpecular = aiColor3D(0, 0, 0);
            out->mColorAmbient = srcLight->mColor*srcLight->mIntensity;
        }
        else {
            // collada doesn't differentiate between these color types
            out->mColorDiffuse = out->mColorSpecular = srcLight->mColor*srcLight->mIntensity;
            out->mColorAmbient = aiColor3D(0, 0, 0);
        }

        // convert falloff angle and falloff exponent in our representation, if given
        if (out->mType == aiLightSource_SPOT) {

            out->mAngleInnerCone = AI_DEG_TO_RAD( srcLight->mFalloffAngle );

            // ... some extension magic.
            if (srcLight->mOuterAngle >= ASSIMP_COLLADA_LIGHT_ANGLE_NOT_SET*(1-1e-6f))
            {
                // ... some deprecation magic.
                if (srcLight->mPenumbraAngle >= ASSIMP_COLLADA_LIGHT_ANGLE_NOT_SET*(1-1e-6f))
                {
                    // Need to rely on falloff_exponent. I don't know how to interpret it, so I need to guess ....
                    // epsilon chosen to be 0.1
                    out->mAngleOuterCone = std::acos(std::pow(0.1f,1.f/srcLight->mFalloffExponent))+
                            out->mAngleInnerCone;
                }
                else {
                    out->mAngleOuterCone = out->mAngleInnerCone + AI_DEG_TO_RAD(  srcLight->mPenumbraAngle );
                    if (out->mAngleOuterCone < out->mAngleInnerCone)
                        std::swap(out->mAngleInnerCone,out->mAngleOuterCone);
                }
            }
            else out->mAngleOuterCone = AI_DEG_TO_RAD(  srcLight->mOuterAngle );
        }

        // add to light list
        mLights.push_back(out);
    }
}

// ------------------------------------------------------------------------------------------------
// Builds cameras for the given node and references them
void ColladaLoader::BuildCamerasForNode( const ColladaParser& pParser, const Collada::Node* pNode, aiNode* pTarget)
{
    for( const Collada::CameraInstance& cid : pNode->mCameras)
    {
        // find the referred light
        ColladaParser::CameraLibrary::const_iterator srcCameraIt = pParser.mCameraLibrary.find( cid.mCamera);
        if( srcCameraIt == pParser.mCameraLibrary.end())
        {
            DefaultLogger::get()->warn("Collada: Unable to find camera for ID \"" + cid.mCamera + "\". Skipping.");
            continue;
        }
        const Collada::Camera* srcCamera = &srcCameraIt->second;

        // orthographic cameras not yet supported in Assimp
        if (srcCamera->mOrtho) {
            DefaultLogger::get()->warn("Collada: Orthographic cameras are not supported.");
        }

        // now fill our ai data structure
        aiCamera* out = new aiCamera();
        out->mName = pTarget->mName;

        // collada cameras point in -Z by default, rest is specified in node transform
        out->mLookAt = aiVector3D(0.f,0.f,-1.f);

        // near/far z is already ok
        out->mClipPlaneFar = srcCamera->mZFar;
        out->mClipPlaneNear = srcCamera->mZNear;

        // ... but for the rest some values are optional
        // and we need to compute the others in any combination.
         if (srcCamera->mAspect != 10e10f)
            out->mAspect = srcCamera->mAspect;

        if (srcCamera->mHorFov != 10e10f) {
            out->mHorizontalFOV = srcCamera->mHorFov;

            if (srcCamera->mVerFov != 10e10f && srcCamera->mAspect == 10e10f) {
                out->mAspect = std::tan(AI_DEG_TO_RAD(srcCamera->mHorFov)) /
                    std::tan(AI_DEG_TO_RAD(srcCamera->mVerFov));
            }
        }
        else if (srcCamera->mAspect != 10e10f && srcCamera->mVerFov != 10e10f)  {
            out->mHorizontalFOV = 2.0f * AI_RAD_TO_DEG(std::atan(srcCamera->mAspect *
                std::tan(AI_DEG_TO_RAD(srcCamera->mVerFov) * 0.5f)));
        }

        // Collada uses degrees, we use radians
        out->mHorizontalFOV = AI_DEG_TO_RAD(out->mHorizontalFOV);

        // add to camera list
        mCameras.push_back(out);
    }
}

// ------------------------------------------------------------------------------------------------
// Builds meshes for the given node and references them
void ColladaLoader::BuildMeshesForNode( const ColladaParser& pParser, const Collada::Node* pNode, aiNode* pTarget)
{
    // accumulated mesh references by this node
    std::vector<size_t> newMeshRefs;
    newMeshRefs.reserve(pNode->mMeshes.size());

    // add a mesh for each subgroup in each collada mesh
    for( const Collada::MeshInstance& mid : pNode->mMeshes)
    {
        const Collada::Mesh* srcMesh = NULL;
        const Collada::Controller* srcController = NULL;

        // find the referred mesh
        ColladaParser::MeshLibrary::const_iterator srcMeshIt = pParser.mMeshLibrary.find( mid.mMeshOrController);
        if( srcMeshIt == pParser.mMeshLibrary.end())
        {
            // if not found in the mesh-library, it might also be a controller referring to a mesh
            ColladaParser::ControllerLibrary::const_iterator srcContrIt = pParser.mControllerLibrary.find( mid.mMeshOrController);
            if( srcContrIt != pParser.mControllerLibrary.end())
            {
                srcController = &srcContrIt->second;
                srcMeshIt = pParser.mMeshLibrary.find( srcController->mMeshId);
                if( srcMeshIt != pParser.mMeshLibrary.end())
                    srcMesh = srcMeshIt->second;
            }

            if( !srcMesh)
            {
                DefaultLogger::get()->warn( format() << "Collada: Unable to find geometry for ID \"" << mid.mMeshOrController << "\". Skipping." );
                continue;
            }
        } else
        {
            // ID found in the mesh library -> direct reference to an unskinned mesh
            srcMesh = srcMeshIt->second;
        }

        // build a mesh for each of its subgroups
        size_t vertexStart = 0, faceStart = 0;
        for( size_t sm = 0; sm < srcMesh->mSubMeshes.size(); ++sm)
        {
            const Collada::SubMesh& submesh = srcMesh->mSubMeshes[sm];
            if( submesh.mNumFaces == 0)
                continue;

            // find material assigned to this submesh
            std::string meshMaterial;
            std::map<std::string, Collada::SemanticMappingTable >::const_iterator meshMatIt = mid.mMaterials.find( submesh.mMaterial);

            const Collada::SemanticMappingTable* table = NULL;
            if( meshMatIt != mid.mMaterials.end())
            {
                table = &meshMatIt->second;
                meshMaterial = table->mMatName;
            }
            else
            {
                DefaultLogger::get()->warn( format() << "Collada: No material specified for subgroup <" << submesh.mMaterial << "> in geometry <" << mid.mMeshOrController << ">." );
                if( !mid.mMaterials.empty() )
                    meshMaterial = mid.mMaterials.begin()->second.mMatName;
            }

            // OK ... here the *real* fun starts ... we have the vertex-input-to-effect-semantic-table
            // given. The only mapping stuff which we do actually support is the UV channel.
            std::map<std::string, size_t>::const_iterator matIt = mMaterialIndexByName.find( meshMaterial);
            unsigned int matIdx;
            if( matIt != mMaterialIndexByName.end())
                matIdx = static_cast<unsigned int>(matIt->second);
            else
                matIdx = 0;

            if (table && !table->mMap.empty() ) {
                std::pair<Collada::Effect*, aiMaterial*>&  mat = newMats[matIdx];

                // Iterate through all texture channels assigned to the effect and
                // check whether we have mapping information for it.
                ApplyVertexToEffectSemanticMapping(mat.first->mTexDiffuse,    *table);
                ApplyVertexToEffectSemanticMapping(mat.first->mTexAmbient,    *table);
                ApplyVertexToEffectSemanticMapping(mat.first->mTexSpecular,   *table);
                ApplyVertexToEffectSemanticMapping(mat.first->mTexEmissive,   *table);
                ApplyVertexToEffectSemanticMapping(mat.first->mTexTransparent,*table);
                ApplyVertexToEffectSemanticMapping(mat.first->mTexBump,       *table);
            }

            // built lookup index of the Mesh-Submesh-Material combination
            ColladaMeshIndex index( mid.mMeshOrController, sm, meshMaterial);

            // if we already have the mesh at the library, just add its index to the node's array
            std::map<ColladaMeshIndex, size_t>::const_iterator dstMeshIt = mMeshIndexByID.find( index);
            if( dstMeshIt != mMeshIndexByID.end())  {
                newMeshRefs.push_back( dstMeshIt->second);
            }
            else
            {
                // else we have to add the mesh to the collection and store its newly assigned index at the node
                aiMesh* dstMesh = CreateMesh( pParser, srcMesh, submesh, srcController, vertexStart, faceStart);

                // store the mesh, and store its new index in the node
                newMeshRefs.push_back( mMeshes.size());
                mMeshIndexByID[index] = mMeshes.size();
                mMeshes.push_back( dstMesh);
                vertexStart += dstMesh->mNumVertices; faceStart += submesh.mNumFaces;

                // assign the material index
                dstMesh->mMaterialIndex = matIdx;
                if(dstMesh->mName.length == 0)
                {
                    dstMesh->mName = mid.mMeshOrController;
                }
      }
        }
    }

    // now place all mesh references we gathered in the target node
    pTarget->mNumMeshes = static_cast<unsigned int>(newMeshRefs.size());
    if( newMeshRefs.size())
    {
        struct UIntTypeConverter
        {
            unsigned int operator()(const size_t& v) const
            {
                return static_cast<unsigned int>(v);
            }
        };

        pTarget->mMeshes = new unsigned int[pTarget->mNumMeshes];
        std::transform( newMeshRefs.begin(), newMeshRefs.end(), pTarget->mMeshes, UIntTypeConverter());
    }
}

// ------------------------------------------------------------------------------------------------
// Find mesh from either meshes or morph target meshes
aiMesh *ColladaLoader::findMesh(std::string meshid)
{
    for (unsigned int i = 0; i < mMeshes.size(); i++)
        if (std::string(mMeshes[i]->mName.data) == meshid)
            return mMeshes[i];

    for (unsigned int i = 0; i < mTargetMeshes.size(); i++)
        if (std::string(mTargetMeshes[i]->mName.data) == meshid)
            return mTargetMeshes[i];

    return NULL;
}

// ------------------------------------------------------------------------------------------------
// Creates a mesh for the given ColladaMesh face subset and returns the newly created mesh
aiMesh* ColladaLoader::CreateMesh( const ColladaParser& pParser, const Collada::Mesh* pSrcMesh, const Collada::SubMesh& pSubMesh,
    const Collada::Controller* pSrcController, size_t pStartVertex, size_t pStartFace)
{
    aiMesh* dstMesh = new aiMesh;

    dstMesh->mName = pSrcMesh->mName;

    // count the vertices addressed by its faces
    const size_t numVertices = std::accumulate( pSrcMesh->mFaceSize.begin() + pStartFace,
        pSrcMesh->mFaceSize.begin() + pStartFace + pSubMesh.mNumFaces, size_t(0));

    // copy positions
    dstMesh->mNumVertices = static_cast<unsigned int>(numVertices);
    dstMesh->mVertices = new aiVector3D[numVertices];
    std::copy( pSrcMesh->mPositions.begin() + pStartVertex, pSrcMesh->mPositions.begin() +
        pStartVertex + numVertices, dstMesh->mVertices);

    // normals, if given. HACK: (thom) Due to the glorious Collada spec we never
    // know if we have the same number of normals as there are positions. So we
    // also ignore any vertex attribute if it has a different count
    if( pSrcMesh->mNormals.size() >= pStartVertex + numVertices)
    {
        dstMesh->mNormals = new aiVector3D[numVertices];
        std::copy( pSrcMesh->mNormals.begin() + pStartVertex, pSrcMesh->mNormals.begin() +
            pStartVertex + numVertices, dstMesh->mNormals);
    }

    // tangents, if given.
    if( pSrcMesh->mTangents.size() >= pStartVertex + numVertices)
    {
        dstMesh->mTangents = new aiVector3D[numVertices];
        std::copy( pSrcMesh->mTangents.begin() + pStartVertex, pSrcMesh->mTangents.begin() +
            pStartVertex + numVertices, dstMesh->mTangents);
    }

    // bitangents, if given.
    if( pSrcMesh->mBitangents.size() >= pStartVertex + numVertices)
    {
        dstMesh->mBitangents = new aiVector3D[numVertices];
        std::copy( pSrcMesh->mBitangents.begin() + pStartVertex, pSrcMesh->mBitangents.begin() +
            pStartVertex + numVertices, dstMesh->mBitangents);
    }

    // same for texturecoords, as many as we have
    // empty slots are not allowed, need to pack and adjust UV indexes accordingly
    for( size_t a = 0, real = 0; a < AI_MAX_NUMBER_OF_TEXTURECOORDS; a++)
    {
        if( pSrcMesh->mTexCoords[a].size() >= pStartVertex + numVertices)
        {
            dstMesh->mTextureCoords[real] = new aiVector3D[numVertices];
            for( size_t b = 0; b < numVertices; ++b)
                dstMesh->mTextureCoords[real][b] = pSrcMesh->mTexCoords[a][pStartVertex+b];

            dstMesh->mNumUVComponents[real] = pSrcMesh->mNumUVComponents[a];
            ++real;
        }
    }

    // same for vertex colors, as many as we have. again the same packing to avoid empty slots
    for( size_t a = 0, real = 0; a < AI_MAX_NUMBER_OF_COLOR_SETS; a++)
    {
        if( pSrcMesh->mColors[a].size() >= pStartVertex + numVertices)
        {
            dstMesh->mColors[real] = new aiColor4D[numVertices];
            std::copy( pSrcMesh->mColors[a].begin() + pStartVertex, pSrcMesh->mColors[a].begin() + pStartVertex + numVertices,dstMesh->mColors[real]);
            ++real;
        }
    }

    // create faces. Due to the fact that each face uses unique vertices, we can simply count up on each vertex
    size_t vertex = 0;
    dstMesh->mNumFaces = static_cast<unsigned int>(pSubMesh.mNumFaces);
    dstMesh->mFaces = new aiFace[dstMesh->mNumFaces];
    for( size_t a = 0; a < dstMesh->mNumFaces; ++a)
    {
        size_t s = pSrcMesh->mFaceSize[ pStartFace + a];
        aiFace& face = dstMesh->mFaces[a];
        face.mNumIndices = static_cast<unsigned int>(s);
        face.mIndices = new unsigned int[s];
        for( size_t b = 0; b < s; ++b)
            face.mIndices[b] = static_cast<unsigned int>(vertex++);
    }

    // create morph target meshes if any
    std::vector<aiMesh*> targetMeshes;
    std::vector<float> targetWeights;
    Collada::MorphMethod method = Collada::Normalized;

    for(std::map<std::string, Collada::Controller>::const_iterator it = pParser.mControllerLibrary.begin();
        it != pParser.mControllerLibrary.end(); it++)
    {
        const Collada::Controller &c = it->second;
        const Collada::Mesh* baseMesh = pParser.ResolveLibraryReference( pParser.mMeshLibrary, c.mMeshId);

        if (c.mType == Collada::Morph && baseMesh->mName == pSrcMesh->mName)
        {
            const Collada::Accessor& targetAccessor = pParser.ResolveLibraryReference( pParser.mAccessorLibrary, c.mMorphTarget);
            const Collada::Accessor& weightAccessor = pParser.ResolveLibraryReference( pParser.mAccessorLibrary, c.mMorphWeight);
            const Collada::Data& targetData = pParser.ResolveLibraryReference( pParser.mDataLibrary, targetAccessor.mSource);
            const Collada::Data& weightData = pParser.ResolveLibraryReference( pParser.mDataLibrary, weightAccessor.mSource);

            // take method
            method = c.mMethod;

            if (!targetData.mIsStringArray)
                throw DeadlyImportError( "target data must contain id. ");
            if (weightData.mIsStringArray)
                throw DeadlyImportError( "target weight data must not be textual ");

            for (unsigned int i = 0; i < targetData.mStrings.size(); ++i)
            {
                const Collada::Mesh* targetMesh = pParser.ResolveLibraryReference(pParser.mMeshLibrary, targetData.mStrings.at(i));

                aiMesh *aimesh = findMesh(targetMesh->mName);
                if (!aimesh)
                {
                    if (targetMesh->mSubMeshes.size() > 1)
                        throw DeadlyImportError( "Morhing target mesh must be a single");
                    aimesh = CreateMesh(pParser, targetMesh, targetMesh->mSubMeshes.at(0), NULL, 0, 0);
                    mTargetMeshes.push_back(aimesh);
                }
                targetMeshes.push_back(aimesh);
            }
            for (unsigned int i = 0; i < weightData.mValues.size(); ++i)
                targetWeights.push_back(weightData.mValues.at(i));
        }
    }
    if (targetMeshes.size() > 0 && targetWeights.size() == targetMeshes.size())
    {
        std::vector<aiAnimMesh*> animMeshes;
        for (unsigned int i = 0; i < targetMeshes.size(); i++)
        {
            aiAnimMesh *animMesh = aiCreateAnimMesh(targetMeshes.at(i));
            animMesh->mWeight = targetWeights[i];
            animMeshes.push_back(animMesh);
        }
        dstMesh->mMethod = (method == Collada::Relative)
                                ? aiMorphingMethod_MORPH_RELATIVE
                                : aiMorphingMethod_MORPH_NORMALIZED;
        dstMesh->mAnimMeshes = new aiAnimMesh*[animMeshes.size()];
        dstMesh->mNumAnimMeshes = static_cast<unsigned int>(animMeshes.size());
        for (unsigned int i = 0; i < animMeshes.size(); i++)
            dstMesh->mAnimMeshes[i] = animMeshes.at(i);
    }

    // create bones if given
    if( pSrcController && pSrcController->mType == Collada::Skin)
    {
        // refuse if the vertex count does not match
//      if( pSrcController->mWeightCounts.size() != dstMesh->mNumVertices)
//          throw DeadlyImportError( "Joint Controller vertex count does not match mesh vertex count");

        // resolve references - joint names
        const Collada::Accessor& jointNamesAcc = pParser.ResolveLibraryReference( pParser.mAccessorLibrary, pSrcController->mJointNameSource);
        const Collada::Data& jointNames = pParser.ResolveLibraryReference( pParser.mDataLibrary, jointNamesAcc.mSource);
        // joint offset matrices
        const Collada::Accessor& jointMatrixAcc = pParser.ResolveLibraryReference( pParser.mAccessorLibrary, pSrcController->mJointOffsetMatrixSource);
        const Collada::Data& jointMatrices = pParser.ResolveLibraryReference( pParser.mDataLibrary, jointMatrixAcc.mSource);
        // joint vertex_weight name list - should refer to the same list as the joint names above. If not, report and reconsider
        const Collada::Accessor& weightNamesAcc = pParser.ResolveLibraryReference( pParser.mAccessorLibrary, pSrcController->mWeightInputJoints.mAccessor);
        if( &weightNamesAcc != &jointNamesAcc)
            throw DeadlyImportError( "Temporary implementational laziness. If you read this, please report to the author.");
        // vertex weights
        const Collada::Accessor& weightsAcc = pParser.ResolveLibraryReference( pParser.mAccessorLibrary, pSrcController->mWeightInputWeights.mAccessor);
        const Collada::Data& weights = pParser.ResolveLibraryReference( pParser.mDataLibrary, weightsAcc.mSource);

        if( !jointNames.mIsStringArray || jointMatrices.mIsStringArray || weights.mIsStringArray)
            throw DeadlyImportError( "Data type mismatch while resolving mesh joints");
        // sanity check: we rely on the vertex weights always coming as pairs of BoneIndex-WeightIndex
        if( pSrcController->mWeightInputJoints.mOffset != 0 || pSrcController->mWeightInputWeights.mOffset != 1)
            throw DeadlyImportError( "Unsupported vertex_weight addressing scheme. ");

        // create containers to collect the weights for each bone
        size_t numBones = jointNames.mStrings.size();
        std::vector<std::vector<aiVertexWeight> > dstBones( numBones);

        // build a temporary array of pointers to the start of each vertex's weights
        typedef std::vector< std::pair<size_t, size_t> > IndexPairVector;
        std::vector<IndexPairVector::const_iterator> weightStartPerVertex;
        weightStartPerVertex.resize(pSrcController->mWeightCounts.size(),pSrcController->mWeights.end());

        IndexPairVector::const_iterator pit = pSrcController->mWeights.begin();
        for( size_t a = 0; a < pSrcController->mWeightCounts.size(); ++a)
        {
            weightStartPerVertex[a] = pit;
            pit += pSrcController->mWeightCounts[a];
        }

        // now for each vertex put the corresponding vertex weights into each bone's weight collection
        for( size_t a = pStartVertex; a < pStartVertex + numVertices; ++a)
        {
            // which position index was responsible for this vertex? that's also the index by which
            // the controller assigns the vertex weights
            size_t orgIndex = pSrcMesh->mFacePosIndices[a];
            // find the vertex weights for this vertex
            IndexPairVector::const_iterator iit = weightStartPerVertex[orgIndex];
            size_t pairCount = pSrcController->mWeightCounts[orgIndex];

            for( size_t b = 0; b < pairCount; ++b, ++iit)
            {
                size_t jointIndex = iit->first;
                size_t vertexIndex = iit->second;

                ai_real weight = ReadFloat( weightsAcc, weights, vertexIndex, 0);

                // one day I gonna kill that XSI Collada exporter
                if( weight > 0.0f)
                {
                    aiVertexWeight w;
                    w.mVertexId = static_cast<unsigned int>(a - pStartVertex);
                    w.mWeight = weight;
                    dstBones[jointIndex].push_back( w);
                }
            }
        }

        // count the number of bones which influence vertices of the current submesh
        size_t numRemainingBones = 0;
        for( std::vector<std::vector<aiVertexWeight> >::const_iterator it = dstBones.begin(); it != dstBones.end(); ++it)
            if( it->size() > 0)
                numRemainingBones++;

        // create bone array and copy bone weights one by one
        dstMesh->mNumBones = static_cast<unsigned int>(numRemainingBones);
        dstMesh->mBones = new aiBone*[numRemainingBones];
        size_t boneCount = 0;
        for( size_t a = 0; a < numBones; ++a)
        {
            // omit bones without weights
            if( dstBones[a].size() == 0)
                continue;

            // create bone with its weights
            aiBone* bone = new aiBone;
            bone->mName = ReadString( jointNamesAcc, jointNames, a);
            bone->mOffsetMatrix.a1 = ReadFloat( jointMatrixAcc, jointMatrices, a, 0);
            bone->mOffsetMatrix.a2 = ReadFloat( jointMatrixAcc, jointMatrices, a, 1);
            bone->mOffsetMatrix.a3 = ReadFloat( jointMatrixAcc, jointMatrices, a, 2);
            bone->mOffsetMatrix.a4 = ReadFloat( jointMatrixAcc, jointMatrices, a, 3);
            bone->mOffsetMatrix.b1 = ReadFloat( jointMatrixAcc, jointMatrices, a, 4);
            bone->mOffsetMatrix.b2 = ReadFloat( jointMatrixAcc, jointMatrices, a, 5);
            bone->mOffsetMatrix.b3 = ReadFloat( jointMatrixAcc, jointMatrices, a, 6);
            bone->mOffsetMatrix.b4 = ReadFloat( jointMatrixAcc, jointMatrices, a, 7);
            bone->mOffsetMatrix.c1 = ReadFloat( jointMatrixAcc, jointMatrices, a, 8);
            bone->mOffsetMatrix.c2 = ReadFloat( jointMatrixAcc, jointMatrices, a, 9);
            bone->mOffsetMatrix.c3 = ReadFloat( jointMatrixAcc, jointMatrices, a, 10);
            bone->mOffsetMatrix.c4 = ReadFloat( jointMatrixAcc, jointMatrices, a, 11);
            bone->mNumWeights = static_cast<unsigned int>(dstBones[a].size());
            bone->mWeights = new aiVertexWeight[bone->mNumWeights];
            std::copy( dstBones[a].begin(), dstBones[a].end(), bone->mWeights);

            // apply bind shape matrix to offset matrix
            aiMatrix4x4 bindShapeMatrix;
            bindShapeMatrix.a1 = pSrcController->mBindShapeMatrix[0];
            bindShapeMatrix.a2 = pSrcController->mBindShapeMatrix[1];
            bindShapeMatrix.a3 = pSrcController->mBindShapeMatrix[2];
            bindShapeMatrix.a4 = pSrcController->mBindShapeMatrix[3];
            bindShapeMatrix.b1 = pSrcController->mBindShapeMatrix[4];
            bindShapeMatrix.b2 = pSrcController->mBindShapeMatrix[5];
            bindShapeMatrix.b3 = pSrcController->mBindShapeMatrix[6];
            bindShapeMatrix.b4 = pSrcController->mBindShapeMatrix[7];
            bindShapeMatrix.c1 = pSrcController->mBindShapeMatrix[8];
            bindShapeMatrix.c2 = pSrcController->mBindShapeMatrix[9];
            bindShapeMatrix.c3 = pSrcController->mBindShapeMatrix[10];
            bindShapeMatrix.c4 = pSrcController->mBindShapeMatrix[11];
            bindShapeMatrix.d1 = pSrcController->mBindShapeMatrix[12];
            bindShapeMatrix.d2 = pSrcController->mBindShapeMatrix[13];
            bindShapeMatrix.d3 = pSrcController->mBindShapeMatrix[14];
            bindShapeMatrix.d4 = pSrcController->mBindShapeMatrix[15];
            bone->mOffsetMatrix *= bindShapeMatrix;

            // HACK: (thom) Some exporters address the bone nodes by SID, others address them by ID or even name.
            // Therefore I added a little name replacement here: I search for the bone's node by either name, ID or SID,
            // and replace the bone's name by the node's name so that the user can use the standard
            // find-by-name method to associate nodes with bones.
            const Collada::Node* bnode = FindNode( pParser.mRootNode, bone->mName.data);
            if( !bnode)
                bnode = FindNodeBySID( pParser.mRootNode, bone->mName.data);

            // assign the name that we would have assigned for the source node
            if( bnode)
                bone->mName.Set( FindNameForNode( bnode));
            else
                DefaultLogger::get()->warn( format() << "ColladaLoader::CreateMesh(): could not find corresponding node for joint \"" << bone->mName.data << "\"." );

            // and insert bone
            dstMesh->mBones[boneCount++] = bone;
        }
    }

    return dstMesh;
}

// ------------------------------------------------------------------------------------------------
// Stores all meshes in the given scene
void ColladaLoader::StoreSceneMeshes( aiScene* pScene)
{
    pScene->mNumMeshes = static_cast<unsigned int>(mMeshes.size());
    if( mMeshes.size() > 0)
    {
        pScene->mMeshes = new aiMesh*[mMeshes.size()];
        std::copy( mMeshes.begin(), mMeshes.end(), pScene->mMeshes);
        mMeshes.clear();
    }
}

// ------------------------------------------------------------------------------------------------
// Stores all cameras in the given scene
void ColladaLoader::StoreSceneCameras( aiScene* pScene)
{
    pScene->mNumCameras = static_cast<unsigned int>(mCameras.size());
    if( mCameras.size() > 0)
    {
        pScene->mCameras = new aiCamera*[mCameras.size()];
        std::copy( mCameras.begin(), mCameras.end(), pScene->mCameras);
        mCameras.clear();
    }
}

// ------------------------------------------------------------------------------------------------
// Stores all lights in the given scene
void ColladaLoader::StoreSceneLights( aiScene* pScene)
{
    pScene->mNumLights = static_cast<unsigned int>(mLights.size());
    if( mLights.size() > 0)
    {
        pScene->mLights = new aiLight*[mLights.size()];
        std::copy( mLights.begin(), mLights.end(), pScene->mLights);
        mLights.clear();
    }
}

// ------------------------------------------------------------------------------------------------
// Stores all textures in the given scene
void ColladaLoader::StoreSceneTextures( aiScene* pScene)
{
    pScene->mNumTextures = static_cast<unsigned int>(mTextures.size());
    if( mTextures.size() > 0)
    {
        pScene->mTextures = new aiTexture*[mTextures.size()];
        std::copy( mTextures.begin(), mTextures.end(), pScene->mTextures);
        mTextures.clear();
    }
}

// ------------------------------------------------------------------------------------------------
// Stores all materials in the given scene
void ColladaLoader::StoreSceneMaterials( aiScene* pScene)
{
    pScene->mNumMaterials = static_cast<unsigned int>(newMats.size());

    if (newMats.size() > 0) {
        pScene->mMaterials = new aiMaterial*[newMats.size()];
        for (unsigned int i = 0; i < newMats.size();++i)
            pScene->mMaterials[i] = newMats[i].second;

        newMats.clear();
    }
}

// ------------------------------------------------------------------------------------------------
// Stores all animations
void ColladaLoader::StoreAnimations( aiScene* pScene, const ColladaParser& pParser)
{
    // recursivly collect all animations from the collada scene
    StoreAnimations( pScene, pParser, &pParser.mAnims, "");

    // catch special case: many animations with the same length, each affecting only a single node.
    // we need to unite all those single-node-anims to a proper combined animation
    for( size_t a = 0; a < mAnims.size(); ++a)
    {
        aiAnimation* templateAnim = mAnims[a];
        if( templateAnim->mNumChannels == 1)
        {
            // search for other single-channel-anims with the same duration
            std::vector<size_t> collectedAnimIndices;
            for( size_t b = a+1; b < mAnims.size(); ++b)
            {
                aiAnimation* other = mAnims[b];
                if( other->mNumChannels == 1 && other->mDuration == templateAnim->mDuration && other->mTicksPerSecond == templateAnim->mTicksPerSecond )
                    collectedAnimIndices.push_back( b);
            }

            // if there are other animations which fit the template anim, combine all channels into a single anim
            if( !collectedAnimIndices.empty() )
            {
                aiAnimation* combinedAnim = new aiAnimation();
                combinedAnim->mName = aiString( std::string( "combinedAnim_") + char( '0' + a));
                combinedAnim->mDuration = templateAnim->mDuration;
                combinedAnim->mTicksPerSecond = templateAnim->mTicksPerSecond;
                combinedAnim->mNumChannels = static_cast<unsigned int>(collectedAnimIndices.size() + 1);
                combinedAnim->mChannels = new aiNodeAnim*[combinedAnim->mNumChannels];
                // add the template anim as first channel by moving its aiNodeAnim to the combined animation
                combinedAnim->mChannels[0] = templateAnim->mChannels[0];
                templateAnim->mChannels[0] = NULL;
                delete templateAnim;
                // combined animation replaces template animation in the anim array
                mAnims[a] = combinedAnim;

                // move the memory of all other anims to the combined anim and erase them from the source anims
                for( size_t b = 0; b < collectedAnimIndices.size(); ++b)
                {
                    aiAnimation* srcAnimation = mAnims[collectedAnimIndices[b]];
                    combinedAnim->mChannels[1 + b] = srcAnimation->mChannels[0];
                    srcAnimation->mChannels[0] = NULL;
                    delete srcAnimation;
                }

                // in a second go, delete all the single-channel-anims that we've stripped from their channels
                // back to front to preserve indices - you know, removing an element from a vector moves all elements behind the removed one
                while( !collectedAnimIndices.empty() )
                {
                    mAnims.erase( mAnims.begin() + collectedAnimIndices.back());
                    collectedAnimIndices.pop_back();
                }
            }
        }
    }

    // now store all anims in the scene
    if( !mAnims.empty())
    {
        pScene->mNumAnimations = static_cast<unsigned int>(mAnims.size());
        pScene->mAnimations = new aiAnimation*[mAnims.size()];
        std::copy( mAnims.begin(), mAnims.end(), pScene->mAnimations);
    }

    mAnims.clear();
}

// ------------------------------------------------------------------------------------------------
// Constructs the animations for the given source anim
void ColladaLoader::StoreAnimations( aiScene* pScene, const ColladaParser& pParser, const Collada::Animation* pSrcAnim, const std::string &pPrefix)
{
    std::string animName = pPrefix.empty() ? pSrcAnim->mName : pPrefix + "_" + pSrcAnim->mName;

    // create nested animations, if given
    for( std::vector<Collada::Animation*>::const_iterator it = pSrcAnim->mSubAnims.begin(); it != pSrcAnim->mSubAnims.end(); ++it)
        StoreAnimations( pScene, pParser, *it, animName);

    // create animation channels, if any
    if( !pSrcAnim->mChannels.empty())
        CreateAnimation( pScene, pParser, pSrcAnim, animName);
}

struct MorphTimeValues
{
    float mTime;
    struct key
    {
        float mWeight;
        unsigned int mValue;
    };
    std::vector<key> mKeys;
};

void insertMorphTimeValue(std::vector<MorphTimeValues> &values, float time, float weight, unsigned int value)
{
    MorphTimeValues::key k;
    k.mValue = value;
    k.mWeight = weight;
    if (values.size() == 0 || time < values[0].mTime)
    {
        MorphTimeValues val;
        val.mTime = time;
        val.mKeys.push_back(k);
        values.insert(values.begin(), val);
        return;
    }
    if (time > values.back().mTime)
    {
        MorphTimeValues val;
        val.mTime = time;
        val.mKeys.push_back(k);
        values.insert(values.end(), val);
        return;
    }
    for (unsigned int i = 0; i < values.size(); i++)
    {
        if (std::abs(time - values[i].mTime) < 1e-6f)
        {
            values[i].mKeys.push_back(k);
            return;
        } else if (time > values[i].mTime && time < values[i+1].mTime)
        {
            MorphTimeValues val;
            val.mTime = time;
            val.mKeys.push_back(k);
            values.insert(values.begin() + i, val);
            return;
        }
    }
    // should not get here
}

float getWeightAtKey(const std::vector<MorphTimeValues> &values, int key, unsigned int value)
{
    for (unsigned int i = 0; i < values[key].mKeys.size(); i++)
    {
        if (values[key].mKeys[i].mValue == value)
            return values[key].mKeys[i].mWeight;
    }
    // no value at key found, try to interpolate if present at other keys. if not, return zero
    // TODO: interpolation
    return 0.0f;
}

// ------------------------------------------------------------------------------------------------
// Constructs the animation for the given source anim
void ColladaLoader::CreateAnimation( aiScene* pScene, const ColladaParser& pParser, const Collada::Animation* pSrcAnim, const std::string& pName)
{
    // collect a list of animatable nodes
    std::vector<const aiNode*> nodes;
    CollectNodes( pScene->mRootNode, nodes);

    std::vector<aiNodeAnim*> anims;
    std::vector<aiMeshMorphAnim*> morphAnims;

    for( std::vector<const aiNode*>::const_iterator nit = nodes.begin(); nit != nodes.end(); ++nit)
    {
        // find all the collada anim channels which refer to the current node
        std::vector<Collada::ChannelEntry> entries;
        std::string nodeName = (*nit)->mName.data;

        // find the collada node corresponding to the aiNode
        const Collada::Node* srcNode = FindNode( pParser.mRootNode, nodeName);
//      ai_assert( srcNode != NULL);
        if( !srcNode)
            continue;

        // now check all channels if they affect the current node
        for( std::vector<Collada::AnimationChannel>::const_iterator cit = pSrcAnim->mChannels.begin();
            cit != pSrcAnim->mChannels.end(); ++cit)
        {
            const Collada::AnimationChannel& srcChannel = *cit;
            Collada::ChannelEntry entry;

            // we expect the animation target to be of type "nodeName/transformID.subElement". Ignore all others
            // find the slash that separates the node name - there should be only one
            std::string::size_type slashPos = srcChannel.mTarget.find( '/');
            if( slashPos == std::string::npos)
            {
                std::string::size_type targetPos = srcChannel.mTarget.find(srcNode->mID);
                if (targetPos == std::string::npos)
                    continue;

                // not node transform, but something else. store as unknown animation channel for now
                entry.mChannel = &(*cit);
                entry.mTargetId = srcChannel.mTarget.substr(targetPos + pSrcAnim->mName.length(),
                                        srcChannel.mTarget.length() - targetPos - pSrcAnim->mName.length());
                if (entry.mTargetId.front() == '-')
                    entry.mTargetId = entry.mTargetId.substr(1);
                entries.push_back(entry);
                continue;
            }
            if( srcChannel.mTarget.find( '/', slashPos+1) != std::string::npos)
                continue;
            std::string targetID = srcChannel.mTarget.substr( 0, slashPos);
            if( targetID != srcNode->mID)
                continue;

            // find the dot that separates the transformID - there should be only one or zero
            std::string::size_type dotPos = srcChannel.mTarget.find( '.');
            if( dotPos != std::string::npos)
            {
                if( srcChannel.mTarget.find( '.', dotPos+1) != std::string::npos)
                    continue;

                entry.mTransformId = srcChannel.mTarget.substr( slashPos+1, dotPos - slashPos - 1);

                std::string subElement = srcChannel.mTarget.substr( dotPos+1);
                if( subElement == "ANGLE")
                    entry.mSubElement = 3; // last number in an Axis-Angle-Transform is the angle
                else if( subElement == "X")
                    entry.mSubElement = 0;
                else if( subElement == "Y")
                    entry.mSubElement = 1;
                else if( subElement == "Z")
                    entry.mSubElement = 2;
                else
                    DefaultLogger::get()->warn( format() << "Unknown anim subelement <" << subElement << ">. Ignoring" );
            } else
            {
                // no subelement following, transformId is remaining string
                entry.mTransformId = srcChannel.mTarget.substr( slashPos+1);
            }

            std::string::size_type bracketPos = srcChannel.mTarget.find('(');
            if (bracketPos != std::string::npos)
            {
                entry.mTransformId = srcChannel.mTarget.substr(slashPos + 1, bracketPos - slashPos - 1);
                std::string subElement = srcChannel.mTarget.substr(bracketPos);

                if (subElement == "(0)(0)")
                    entry.mSubElement = 0;
                else if (subElement == "(1)(0)")
                    entry.mSubElement = 1;
                else if (subElement == "(2)(0)")
                    entry.mSubElement = 2;
                else if (subElement == "(3)(0)")
                    entry.mSubElement = 3;
                else if (subElement == "(0)(1)")
                    entry.mSubElement = 4;
                else if (subElement == "(1)(1)")
                    entry.mSubElement = 5;
                else if (subElement == "(2)(1)")
                    entry.mSubElement = 6;
                else if (subElement == "(3)(1)")
                    entry.mSubElement = 7;
                else if (subElement == "(0)(2)")
                    entry.mSubElement = 8;
                else if (subElement == "(1)(2)")
                    entry.mSubElement = 9;
                else if (subElement == "(2)(2)")
                    entry.mSubElement = 10;
                else if (subElement == "(3)(2)")
                    entry.mSubElement = 11;
                else if (subElement == "(0)(3)")
                    entry.mSubElement = 12;
                else if (subElement == "(1)(3)")
                    entry.mSubElement = 13;
                else if (subElement == "(2)(3)")
                    entry.mSubElement = 14;
                else if (subElement == "(3)(3)")
                    entry.mSubElement = 15;

            }

            // determine which transform step is affected by this channel
            entry.mTransformIndex = SIZE_MAX;
            for( size_t a = 0; a < srcNode->mTransforms.size(); ++a)
                if( srcNode->mTransforms[a].mID == entry.mTransformId)
                    entry.mTransformIndex = a;

            if( entry.mTransformIndex == SIZE_MAX)
            {
                if (entry.mTransformId.find("morph-weights") != std::string::npos)
                {
                    entry.mTargetId = entry.mTransformId;
                    entry.mTransformId = "";
                } else
                    continue;
            }

            entry.mChannel = &(*cit);
            entries.push_back( entry);
        }

        // if there's no channel affecting the current node, we skip it
        if( entries.empty())
            continue;

        // resolve the data pointers for all anim channels. Find the minimum time while we're at it
        ai_real startTime = ai_real( 1e20 ), endTime = ai_real( -1e20 );
        for( std::vector<Collada::ChannelEntry>::iterator it = entries.begin(); it != entries.end(); ++it)
        {
            Collada::ChannelEntry& e = *it;
            e.mTimeAccessor = &pParser.ResolveLibraryReference( pParser.mAccessorLibrary, e.mChannel->mSourceTimes);
            e.mTimeData = &pParser.ResolveLibraryReference( pParser.mDataLibrary, e.mTimeAccessor->mSource);
            e.mValueAccessor = &pParser.ResolveLibraryReference( pParser.mAccessorLibrary, e.mChannel->mSourceValues);
            e.mValueData = &pParser.ResolveLibraryReference( pParser.mDataLibrary, e.mValueAccessor->mSource);

            // time count and value count must match
            if( e.mTimeAccessor->mCount != e.mValueAccessor->mCount)
                throw DeadlyImportError( format() << "Time count / value count mismatch in animation channel \"" << e.mChannel->mTarget << "\"." );

      if( e.mTimeAccessor->mCount > 0 )
      {
              // find bounding times
              startTime = std::min( startTime, ReadFloat( *e.mTimeAccessor, *e.mTimeData, 0, 0));
            endTime = std::max( endTime, ReadFloat( *e.mTimeAccessor, *e.mTimeData, e.mTimeAccessor->mCount-1, 0));
      }
        }

    std::vector<aiMatrix4x4> resultTrafos;
    if( !entries.empty() && entries.front().mTimeAccessor->mCount > 0 )
    {
          // create a local transformation chain of the node's transforms
          std::vector<Collada::Transform> transforms = srcNode->mTransforms;

          // now for every unique point in time, find or interpolate the key values for that time
          // and apply them to the transform chain. Then the node's present transformation can be calculated.
          ai_real time = startTime;
          while( 1)
          {
              for( std::vector<Collada::ChannelEntry>::iterator it = entries.begin(); it != entries.end(); ++it)
              {
                  Collada::ChannelEntry& e = *it;

                  // find the keyframe behind the current point in time
                  size_t pos = 0;
                  ai_real postTime = 0.0;
                  while( 1)
                  {
                      if( pos >= e.mTimeAccessor->mCount)
                          break;
                      postTime = ReadFloat( *e.mTimeAccessor, *e.mTimeData, pos, 0);
                      if( postTime >= time)
                          break;
                      ++pos;
                  }

                  pos = std::min( pos, e.mTimeAccessor->mCount-1);

                  // read values from there
                  ai_real temp[16];
                  for( size_t c = 0; c < e.mValueAccessor->mSize; ++c)
                      temp[c] = ReadFloat( *e.mValueAccessor, *e.mValueData, pos, c);

                  // if not exactly at the key time, interpolate with previous value set
                  if( postTime > time && pos > 0)
                  {
                      ai_real preTime = ReadFloat( *e.mTimeAccessor, *e.mTimeData, pos-1, 0);
                      ai_real factor = (time - postTime) / (preTime - postTime);

                      for( size_t c = 0; c < e.mValueAccessor->mSize; ++c)
                      {
                          ai_real v = ReadFloat( *e.mValueAccessor, *e.mValueData, pos-1, c);
                          temp[c] += (v - temp[c]) * factor;
                      }
                  }

                  // Apply values to current transformation
                  std::copy( temp, temp + e.mValueAccessor->mSize, transforms[e.mTransformIndex].f + e.mSubElement);
              }

              // Calculate resulting transformation
              aiMatrix4x4 mat = pParser.CalculateResultTransform( transforms);

              // out of laziness: we store the time in matrix.d4
              mat.d4 = time;
              resultTrafos.push_back( mat);

              // find next point in time to evaluate. That's the closest frame larger than the current in any channel
              ai_real nextTime = ai_real( 1e20 );
              for( std::vector<Collada::ChannelEntry>::iterator it = entries.begin(); it != entries.end(); ++it)
              {
                  Collada::ChannelEntry& channelElement = *it;

                  // find the next time value larger than the current
                  size_t pos = 0;
                  while( pos < channelElement.mTimeAccessor->mCount)
                  {
                      const ai_real t = ReadFloat( *channelElement.mTimeAccessor, *channelElement.mTimeData, pos, 0);
                      if( t > time)
                      {
                          nextTime = std::min( nextTime, t);
                          break;
                      }
                      ++pos;
                  }

				  // https://github.com/assimp/assimp/issues/458
			  	  // Sub-sample axis-angle channels if the delta between two consecutive
                  // key-frame angles is >= 180 degrees.
				  if (transforms[channelElement.mTransformIndex].mType == Collada::TF_ROTATE && channelElement.mSubElement == 3 && pos > 0 && pos < channelElement.mTimeAccessor->mCount) {
					  const ai_real cur_key_angle = ReadFloat(*channelElement.mValueAccessor, *channelElement.mValueData, pos, 0);
                      const ai_real last_key_angle = ReadFloat(*channelElement.mValueAccessor, *channelElement.mValueData, pos - 1, 0);
                      const ai_real cur_key_time = ReadFloat(*channelElement.mTimeAccessor, *channelElement.mTimeData, pos, 0);
                      const ai_real last_key_time = ReadFloat(*channelElement.mTimeAccessor, *channelElement.mTimeData, pos - 1, 0);
                      const ai_real last_eval_angle = last_key_angle + (cur_key_angle - last_key_angle) * (time - last_key_time) / (cur_key_time - last_key_time);
                      const ai_real delta = std::abs(cur_key_angle - last_eval_angle);
				      if (delta >= 180.0) {
						const int subSampleCount = static_cast<int>(std::floor(delta / 90.0));
						if (cur_key_time != time) {
							const ai_real nextSampleTime = time + (cur_key_time - time) / subSampleCount;
							nextTime = std::min(nextTime, nextSampleTime);
						  }
					  }
				  }
              }

              // no more keys on any channel after the current time -> we're done
              if( nextTime > 1e19)
                  break;

              // else construct next keyframe at this following time point
              time = nextTime;
          }
    }

        // there should be some keyframes, but we aren't that fixated on valid input data
//      ai_assert( resultTrafos.size() > 0);

        // build an animation channel for the given node out of these trafo keys
        if( !resultTrafos.empty() )
        {
              aiNodeAnim* dstAnim = new aiNodeAnim;
              dstAnim->mNodeName = nodeName;
              dstAnim->mNumPositionKeys = static_cast<unsigned int>(resultTrafos.size());
              dstAnim->mNumRotationKeys = static_cast<unsigned int>(resultTrafos.size());
              dstAnim->mNumScalingKeys = static_cast<unsigned int>(resultTrafos.size());
              dstAnim->mPositionKeys = new aiVectorKey[resultTrafos.size()];
              dstAnim->mRotationKeys = new aiQuatKey[resultTrafos.size()];
              dstAnim->mScalingKeys = new aiVectorKey[resultTrafos.size()];

              for( size_t a = 0; a < resultTrafos.size(); ++a)
              {
                  aiMatrix4x4 mat = resultTrafos[a];
                  double time = double( mat.d4); // remember? time is stored in mat.d4
                    mat.d4 = 1.0f;

                  dstAnim->mPositionKeys[a].mTime = time;
                  dstAnim->mRotationKeys[a].mTime = time;
                  dstAnim->mScalingKeys[a].mTime = time;
                  mat.Decompose( dstAnim->mScalingKeys[a].mValue, dstAnim->mRotationKeys[a].mValue, dstAnim->mPositionKeys[a].mValue);
              }

              anims.push_back( dstAnim);
        } else
        {
          DefaultLogger::get()->warn( "Collada loader: found empty animation channel, ignored. Please check your exporter.");
        }

        if( !entries.empty() && entries.front().mTimeAccessor->mCount > 0 )
        {
            std::vector<Collada::ChannelEntry> morphChannels;
            for( std::vector<Collada::ChannelEntry>::iterator it = entries.begin(); it != entries.end(); ++it)
            {
                Collada::ChannelEntry& e = *it;

                // skip non-transform types
                if (e.mTargetId.empty())
                    continue;

                if (e.mTargetId.find("morph-weights") != std::string::npos)
                    morphChannels.push_back(e);
            }
            if (morphChannels.size() > 0)
            {
                // either 1) morph weight animation count should contain morph target count channels
                // or     2) one channel with morph target count arrays
                // assume first

                aiMeshMorphAnim *morphAnim = new aiMeshMorphAnim;
                morphAnim->mName.Set(nodeName);

                std::vector<MorphTimeValues> morphTimeValues;

                int morphAnimChannelIndex = 0;
                for( std::vector<Collada::ChannelEntry>::iterator it = morphChannels.begin(); it != morphChannels.end(); ++it)
                {
                    Collada::ChannelEntry& e = *it;
                    std::string::size_type apos = e.mTargetId.find('(');
                    std::string::size_type bpos = e.mTargetId.find(')');
                    if (apos == std::string::npos || bpos == std::string::npos)
                        // unknown way to specify weight -> ignore this animation
                        continue;

                    // weight target can be in format Weight_M_N, Weight_N, WeightN, or some other way
                    // we ignore the name and just assume the channels are in the right order
                    for (unsigned int i = 0; i < e.mTimeData->mValues.size(); i++)
                        insertMorphTimeValue(morphTimeValues, e.mTimeData->mValues.at(i), e.mValueData->mValues.at(i), morphAnimChannelIndex);

                    ++morphAnimChannelIndex;
                }

                morphAnim->mNumKeys = static_cast<unsigned int>(morphTimeValues.size());
                morphAnim->mKeys = new aiMeshMorphKey[morphAnim->mNumKeys];
                for (unsigned int key = 0; key < morphAnim->mNumKeys; key++)
                {
                    morphAnim->mKeys[key].mNumValuesAndWeights = static_cast<unsigned int>(morphChannels.size());
                    morphAnim->mKeys[key].mValues = new unsigned int [morphChannels.size()];
                    morphAnim->mKeys[key].mWeights = new double [morphChannels.size()];

                    morphAnim->mKeys[key].mTime = morphTimeValues[key].mTime;
                    for (unsigned int valueIndex = 0; valueIndex < morphChannels.size(); valueIndex++)
                    {
                        morphAnim->mKeys[key].mValues[valueIndex] = valueIndex;
                        morphAnim->mKeys[key].mWeights[valueIndex] = getWeightAtKey(morphTimeValues, key, valueIndex);
                    }
                }

                morphAnims.push_back(morphAnim);
            }
        }
    }

    if( !anims.empty() || !morphAnims.empty())
    {
        aiAnimation* anim = new aiAnimation;
        anim->mName.Set( pName);
        anim->mNumChannels = static_cast<unsigned int>(anims.size());
        if (anim->mNumChannels > 0)
        {
            anim->mChannels = new aiNodeAnim*[anims.size()];
            std::copy( anims.begin(), anims.end(), anim->mChannels);
        }
        anim->mNumMorphMeshChannels = static_cast<unsigned int>(morphAnims.size());
        if (anim->mNumMorphMeshChannels > 0)
        {
            anim->mMorphMeshChannels = new aiMeshMorphAnim*[anim->mNumMorphMeshChannels];
            std::copy( morphAnims.begin(), morphAnims.end(), anim->mMorphMeshChannels);
        }
        anim->mDuration = 0.0f;
        for( size_t a = 0; a < anims.size(); ++a)
        {
                anim->mDuration = std::max( anim->mDuration, anims[a]->mPositionKeys[anims[a]->mNumPositionKeys-1].mTime);
                anim->mDuration = std::max( anim->mDuration, anims[a]->mRotationKeys[anims[a]->mNumRotationKeys-1].mTime);
                anim->mDuration = std::max( anim->mDuration, anims[a]->mScalingKeys[anims[a]->mNumScalingKeys-1].mTime);
        }
        for (size_t a = 0; a < morphAnims.size(); ++a)
        {
            anim->mDuration = std::max(anim->mDuration, morphAnims[a]->mKeys[morphAnims[a]->mNumKeys-1].mTime);
        }
        anim->mTicksPerSecond = 1;
        mAnims.push_back( anim);
    }
}

// ------------------------------------------------------------------------------------------------
// Add a texture to a material structure
void ColladaLoader::AddTexture ( aiMaterial& mat, const ColladaParser& pParser,
    const Collada::Effect& effect,
    const Collada::Sampler& sampler,
    aiTextureType type, unsigned int idx)
{
    // first of all, basic file name
    const aiString name = FindFilenameForEffectTexture( pParser, effect, sampler.mName );
    mat.AddProperty( &name, _AI_MATKEY_TEXTURE_BASE, type, idx );

    // mapping mode
    int map = aiTextureMapMode_Clamp;
    if (sampler.mWrapU)
        map = aiTextureMapMode_Wrap;
    if (sampler.mWrapU && sampler.mMirrorU)
        map = aiTextureMapMode_Mirror;

    mat.AddProperty( &map, 1, _AI_MATKEY_MAPPINGMODE_U_BASE, type, idx);

    map = aiTextureMapMode_Clamp;
    if (sampler.mWrapV)
        map = aiTextureMapMode_Wrap;
    if (sampler.mWrapV && sampler.mMirrorV)
        map = aiTextureMapMode_Mirror;

    mat.AddProperty( &map, 1, _AI_MATKEY_MAPPINGMODE_V_BASE, type, idx);

    // UV transformation
    mat.AddProperty(&sampler.mTransform, 1,
        _AI_MATKEY_UVTRANSFORM_BASE, type, idx);

    // Blend mode
    mat.AddProperty((int*)&sampler.mOp , 1,
        _AI_MATKEY_TEXBLEND_BASE, type, idx);

    // Blend factor
    mat.AddProperty((ai_real*)&sampler.mWeighting , 1,
        _AI_MATKEY_TEXBLEND_BASE, type, idx);

    // UV source index ... if we didn't resolve the mapping, it is actually just
    // a guess but it works in most cases. We search for the frst occurrence of a
    // number in the channel name. We assume it is the zero-based index into the
    // UV channel array of all corresponding meshes. It could also be one-based
    // for some exporters, but we won't care of it unless someone complains about.
    if (sampler.mUVId != UINT_MAX)
        map = sampler.mUVId;
    else {
        map = -1;
        for (std::string::const_iterator it = sampler.mUVChannel.begin();it != sampler.mUVChannel.end(); ++it){
            if (IsNumeric(*it)) {
                map = strtoul10(&(*it));
                break;
            }
        }
        if (-1 == map) {
            DefaultLogger::get()->warn("Collada: unable to determine UV channel for texture");
            map = 0;
        }
    }
    mat.AddProperty(&map,1,_AI_MATKEY_UVWSRC_BASE,type,idx);
}

// ------------------------------------------------------------------------------------------------
// Fills materials from the collada material definitions
void ColladaLoader::FillMaterials( const ColladaParser& pParser, aiScene* /*pScene*/)
{
    for (auto &elem : newMats)
    {
        aiMaterial&  mat = (aiMaterial&)*elem.second;
        Collada::Effect& effect = *elem.first;

        // resolve shading mode
        int shadeMode;
        if (effect.mFaceted) /* fixme */
            shadeMode = aiShadingMode_Flat;
        else {
            switch( effect.mShadeType)
            {
            case Collada::Shade_Constant:
                shadeMode = aiShadingMode_NoShading;
                break;
            case Collada::Shade_Lambert:
                shadeMode = aiShadingMode_Gouraud;
                break;
            case Collada::Shade_Blinn:
                shadeMode = aiShadingMode_Blinn;
                break;
            case Collada::Shade_Phong:
                shadeMode = aiShadingMode_Phong;
                break;

            default:
                DefaultLogger::get()->warn("Collada: Unrecognized shading mode, using gouraud shading");
                shadeMode = aiShadingMode_Gouraud;
                break;
            }
        }
        mat.AddProperty<int>( &shadeMode, 1, AI_MATKEY_SHADING_MODEL);

        // double-sided?
        shadeMode = effect.mDoubleSided;
        mat.AddProperty<int>( &shadeMode, 1, AI_MATKEY_TWOSIDED);

        // wireframe?
        shadeMode = effect.mWireframe;
        mat.AddProperty<int>( &shadeMode, 1, AI_MATKEY_ENABLE_WIREFRAME);

        // add material colors
        mat.AddProperty( &effect.mAmbient, 1,AI_MATKEY_COLOR_AMBIENT);
        mat.AddProperty( &effect.mDiffuse, 1, AI_MATKEY_COLOR_DIFFUSE);
        mat.AddProperty( &effect.mSpecular, 1,AI_MATKEY_COLOR_SPECULAR);
        mat.AddProperty( &effect.mEmissive, 1,  AI_MATKEY_COLOR_EMISSIVE);
        mat.AddProperty( &effect.mReflective, 1, AI_MATKEY_COLOR_REFLECTIVE);

        // scalar properties
        mat.AddProperty( &effect.mShininess, 1, AI_MATKEY_SHININESS);
        mat.AddProperty( &effect.mReflectivity, 1, AI_MATKEY_REFLECTIVITY);
        mat.AddProperty( &effect.mRefractIndex, 1, AI_MATKEY_REFRACTI);

        // transparency, a very hard one. seemingly not all files are following the
        // specification here (1.0 transparency => completely opaque)...
        // therefore, we let the opportunity for the user to manually invert
        // the transparency if necessary and we add preliminary support for RGB_ZERO mode
        if(effect.mTransparency >= 0.f && effect.mTransparency <= 1.f) {
            // handle RGB transparency completely, cf Collada specs 1.5.0 pages 249 and 304
            if(effect.mRGBTransparency) {
				// use luminance as defined by ISO/CIE color standards (see ITU-R Recommendation BT.709-4)
                effect.mTransparency *= (
                    0.212671f * effect.mTransparent.r +
                    0.715160f * effect.mTransparent.g +
                    0.072169f * effect.mTransparent.b
                );

                effect.mTransparent.a = 1.f;

                mat.AddProperty( &effect.mTransparent, 1, AI_MATKEY_COLOR_TRANSPARENT );
            } else {
                effect.mTransparency *=  effect.mTransparent.a;
            }

            if(effect.mInvertTransparency) {
                effect.mTransparency = 1.f - effect.mTransparency;
            }

            // Is the material finally transparent ?
            if (effect.mHasTransparency || effect.mTransparency < 1.f) {
                mat.AddProperty( &effect.mTransparency, 1, AI_MATKEY_OPACITY );
            }
        }

        // add textures, if given
        if( !effect.mTexAmbient.mName.empty())
             /* It is merely a lightmap */
            AddTexture( mat, pParser, effect, effect.mTexAmbient, aiTextureType_LIGHTMAP);

        if( !effect.mTexEmissive.mName.empty())
            AddTexture( mat, pParser, effect, effect.mTexEmissive, aiTextureType_EMISSIVE);

        if( !effect.mTexSpecular.mName.empty())
            AddTexture( mat, pParser, effect, effect.mTexSpecular, aiTextureType_SPECULAR);

        if( !effect.mTexDiffuse.mName.empty())
            AddTexture( mat, pParser, effect, effect.mTexDiffuse, aiTextureType_DIFFUSE);

        if( !effect.mTexBump.mName.empty())
            AddTexture( mat, pParser, effect, effect.mTexBump, aiTextureType_NORMALS);

        if( !effect.mTexTransparent.mName.empty())
            AddTexture( mat, pParser, effect, effect.mTexTransparent, aiTextureType_OPACITY);

        if( !effect.mTexReflective.mName.empty())
            AddTexture( mat, pParser, effect, effect.mTexReflective, aiTextureType_REFLECTION);
    }
}

// ------------------------------------------------------------------------------------------------
// Constructs materials from the collada material definitions
void ColladaLoader::BuildMaterials( ColladaParser& pParser, aiScene* /*pScene*/)
{
    newMats.reserve(pParser.mMaterialLibrary.size());

    for( ColladaParser::MaterialLibrary::const_iterator matIt = pParser.mMaterialLibrary.begin(); matIt != pParser.mMaterialLibrary.end(); ++matIt)
    {
        const Collada::Material& material = matIt->second;
        // a material is only a reference to an effect
        ColladaParser::EffectLibrary::iterator effIt = pParser.mEffectLibrary.find( material.mEffect);
        if( effIt == pParser.mEffectLibrary.end())
            continue;
        Collada::Effect& effect = effIt->second;

        // create material
        aiMaterial* mat = new aiMaterial;
        aiString name( material.mName.empty() ? matIt->first : material.mName );
        mat->AddProperty(&name,AI_MATKEY_NAME);

        // store the material
        mMaterialIndexByName[matIt->first] = newMats.size();
        newMats.push_back( std::pair<Collada::Effect*, aiMaterial*>( &effect,mat) );
    }
    // ScenePreprocessor generates a default material automatically if none is there.
    // All further code here in this loader works well without a valid material so
    // we can safely let it to ScenePreprocessor.
#if 0
    if( newMats.size() == 0)
    {
        aiMaterial* mat = new aiMaterial;
        aiString name( AI_DEFAULT_MATERIAL_NAME );
        mat->AddProperty( &name, AI_MATKEY_NAME);

        const int shadeMode = aiShadingMode_Phong;
        mat->AddProperty<int>( &shadeMode, 1, AI_MATKEY_SHADING_MODEL);
        aiColor4D colAmbient( 0.2, 0.2, 0.2, 1.0), colDiffuse( 0.8, 0.8, 0.8, 1.0), colSpecular( 0.5, 0.5, 0.5, 0.5);
        mat->AddProperty( &colAmbient, 1, AI_MATKEY_COLOR_AMBIENT);
        mat->AddProperty( &colDiffuse, 1, AI_MATKEY_COLOR_DIFFUSE);
        mat->AddProperty( &colSpecular, 1, AI_MATKEY_COLOR_SPECULAR);
        const ai_real specExp = 5.0;
        mat->AddProperty( &specExp, 1, AI_MATKEY_SHININESS);
    }
#endif
}

// ------------------------------------------------------------------------------------------------
// Resolves the texture name for the given effect texture entry
aiString ColladaLoader::FindFilenameForEffectTexture( const ColladaParser& pParser,
    const Collada::Effect& pEffect, const std::string& pName)
{
    aiString result;

    // recurse through the param references until we end up at an image
    std::string name = pName;
    while( 1)
    {
        // the given string is a param entry. Find it
        Collada::Effect::ParamLibrary::const_iterator it = pEffect.mParams.find( name);
        // if not found, we're at the end of the recursion. The resulting string should be the image ID
        if( it == pEffect.mParams.end())
            break;

        // else recurse on
        name = it->second.mReference;
    }

    // find the image referred by this name in the image library of the scene
    ColladaParser::ImageLibrary::const_iterator imIt = pParser.mImageLibrary.find( name);
    if( imIt == pParser.mImageLibrary.end())
    {
        //missing texture should not stop the conversion
        //throw DeadlyImportError( format() <<
        //    "Collada: Unable to resolve effect texture entry \"" << pName << "\", ended up at ID \"" << name << "\"." );

        DefaultLogger::get()->warn("Collada: Unable to resolve effect texture entry \"" + pName + "\", ended up at ID \"" + name + "\".");

        //set default texture file name
        result.Set(name + ".jpg");
        ConvertPath(result);
        return result;
    }

    // if this is an embedded texture image setup an aiTexture for it
    if (imIt->second.mFileName.empty())
    {
        if (imIt->second.mImageData.empty())  {
            throw DeadlyImportError("Collada: Invalid texture, no data or file reference given");
        }

        aiTexture* tex = new aiTexture();

        // setup format hint
        if (imIt->second.mEmbeddedFormat.length() > 3) {
            DefaultLogger::get()->warn("Collada: texture format hint is too long, truncating to 3 characters");
        }
        strncpy(tex->achFormatHint,imIt->second.mEmbeddedFormat.c_str(),3);

        // and copy texture data
        tex->mHeight = 0;
        tex->mWidth = static_cast<unsigned int>(imIt->second.mImageData.size());
        tex->pcData = (aiTexel*)new char[tex->mWidth];
        memcpy(tex->pcData,&imIt->second.mImageData[0],tex->mWidth);

        // setup texture reference string
        result.data[0] = '*';
        result.length = 1 + ASSIMP_itoa10(result.data+1,static_cast<unsigned int>(MAXLEN-1),static_cast<int32_t>(mTextures.size()));

        // and add this texture to the list
        mTextures.push_back(tex);
    }
    else
    {
        result.Set( imIt->second.mFileName );
        ConvertPath(result);
    }
    return result;
}

// ------------------------------------------------------------------------------------------------
// Convert a path read from a collada file to the usual representation
void ColladaLoader::ConvertPath (aiString& ss)
{
    // TODO: collada spec, p 22. Handle URI correctly.
    // For the moment we're just stripping the file:// away to make it work.
    // Windoes doesn't seem to be able to find stuff like
    // 'file://..\LWO\LWO2\MappingModes\earthSpherical.jpg'
    if (0 == strncmp(ss.data,"file://",7))
    {
        ss.length -= 7;
        memmove(ss.data,ss.data+7,ss.length);
        ss.data[ss.length] = '\0';
    }

  // Maxon Cinema Collada Export writes "file:///C:\andsoon" with three slashes...
  // I need to filter it without destroying linux paths starting with "/somewhere"
  if( ss.data[0] == '/' && isalpha( ss.data[1]) && ss.data[2] == ':' )
  {
    ss.length--;
    memmove( ss.data, ss.data+1, ss.length);
    ss.data[ss.length] = 0;
  }

  // find and convert all %xy special chars
  char* out = ss.data;
  for( const char* it = ss.data; it != ss.data + ss.length; /**/ )
  {
    if( *it == '%' && (it + 3) < ss.data + ss.length )
    {
      // separate the number to avoid dragging in chars from behind into the parsing
      char mychar[3] = { it[1], it[2], 0 };
      size_t nbr = strtoul16( mychar);
      it += 3;
      *out++ = (char)(nbr & 0xFF);
    } else
    {
      *out++ = *it++;
    }
  }

  // adjust length and terminator of the shortened string
  *out = 0;
  ss.length = (ptrdiff_t) (out - ss.data);
}

// ------------------------------------------------------------------------------------------------
// Reads a float value from an accessor and its data array.
ai_real ColladaLoader::ReadFloat( const Collada::Accessor& pAccessor, const Collada::Data& pData, size_t pIndex, size_t pOffset) const
{
    // FIXME: (thom) Test for data type here in every access? For the moment, I leave this to the caller
    size_t pos = pAccessor.mStride * pIndex + pAccessor.mOffset + pOffset;
    ai_assert( pos < pData.mValues.size());
    return pData.mValues[pos];
}

// ------------------------------------------------------------------------------------------------
// Reads a string value from an accessor and its data array.
const std::string& ColladaLoader::ReadString( const Collada::Accessor& pAccessor, const Collada::Data& pData, size_t pIndex) const
{
    size_t pos = pAccessor.mStride * pIndex + pAccessor.mOffset;
    ai_assert( pos < pData.mStrings.size());
    return pData.mStrings[pos];
}

// ------------------------------------------------------------------------------------------------
// Collects all nodes into the given array
void ColladaLoader::CollectNodes( const aiNode* pNode, std::vector<const aiNode*>& poNodes) const
{
    poNodes.push_back( pNode);

    for( size_t a = 0; a < pNode->mNumChildren; ++a)
        CollectNodes( pNode->mChildren[a], poNodes);
}

// ------------------------------------------------------------------------------------------------
// Finds a node in the collada scene by the given name
const Collada::Node* ColladaLoader::FindNode( const Collada::Node* pNode, const std::string& pName) const
{
    if( pNode->mName == pName || pNode->mID == pName)
        return pNode;

    for( size_t a = 0; a < pNode->mChildren.size(); ++a)
    {
        const Collada::Node* node = FindNode( pNode->mChildren[a], pName);
        if( node)
            return node;
    }

    return NULL;
}

// ------------------------------------------------------------------------------------------------
// Finds a node in the collada scene by the given SID
const Collada::Node* ColladaLoader::FindNodeBySID( const Collada::Node* pNode, const std::string& pSID) const
{
  if( pNode->mSID == pSID)
    return pNode;

  for( size_t a = 0; a < pNode->mChildren.size(); ++a)
  {
    const Collada::Node* node = FindNodeBySID( pNode->mChildren[a], pSID);
    if( node)
      return node;
  }

  return NULL;
}

// ------------------------------------------------------------------------------------------------
// Finds a proper unique name for a node derived from the collada-node's properties.
// The name must be unique for proper node-bone association.
std::string ColladaLoader::FindNameForNode( const Collada::Node* pNode)
{
    // Now setup the name of the assimp node. The collada name might not be
    // unique, so we use the collada ID.
    if (!pNode->mID.empty())
        return pNode->mID;
    else if (!pNode->mSID.empty())
    return pNode->mSID;
  else
    {
        // No need to worry. Unnamed nodes are no problem at all, except
        // if cameras or lights need to be assigned to them.
    return format() << "$ColladaAutoName$_" << mNodeNameCounter++;
    }
}

#endif // !! ASSIMP_BUILD_NO_DAE_IMPORTER
