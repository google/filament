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

/** @file  3DSLoader.cpp
 *  @brief Implementation of the 3ds importer class
 *
 *  http://www.the-labs.com/Blender/3DS-details.html
 */


#ifndef ASSIMP_BUILD_NO_3DS_IMPORTER

// internal headers
#include "3DSLoader.h"
#include <assimp/Macros.h>
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/importerdesc.h>
#include <assimp/StringComparison.h>

using namespace Assimp;

static const aiImporterDesc desc = {
    "Discreet 3DS Importer",
    "",
    "",
    "Limited animation support",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
	"3ds prj"
};


// ------------------------------------------------------------------------------------------------
// Begins a new parsing block
// - Reads the current chunk and validates it
// - computes its length
#define ASSIMP_3DS_BEGIN_CHUNK()                                         \
    while (true) {                                                       \
    if (stream->GetRemainingSizeToLimit() < sizeof(Discreet3DS::Chunk)){ \
        return;                                                          \
    }                                                                    \
    Discreet3DS::Chunk chunk;                                            \
    ReadChunk(&chunk);                                                   \
    int chunkSize = chunk.Size-sizeof(Discreet3DS::Chunk);               \
    if(chunkSize <= 0)                                                   \
        continue;                                                        \
    const unsigned int oldReadLimit = stream->SetReadLimit(              \
        stream->GetCurrentPos() + chunkSize);                            \


// ------------------------------------------------------------------------------------------------
// End a parsing block
// Must follow at the end of each parsing block, reset chunk end marker to previous value
#define ASSIMP_3DS_END_CHUNK()                  \
    stream->SkipToReadLimit();                  \
    stream->SetReadLimit(oldReadLimit);         \
    if (stream->GetRemainingSizeToLimit() == 0) \
        return;                                 \
    }

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
Discreet3DSImporter::Discreet3DSImporter()
: stream()
, mLastNodeIndex()
, mCurrentNode()
, mRootNode()
, mScene()
, mMasterScale()
, bHasBG()
, bIsPrj() {
    // empty
}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
Discreet3DSImporter::~Discreet3DSImporter() {
    // empty
}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool Discreet3DSImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const {
    std::string extension = GetExtension(pFile);
	if(extension == "3ds" || extension == "prj") {
        return true;
    }

    if (!extension.length() || checkSig) {
        uint16_t token[3];
        token[0] = 0x4d4d;
        token[1] = 0x3dc2;
        //token[2] = 0x3daa;
        return CheckMagicToken(pIOHandler,pFile,token,2,0,2);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Loader registry entry
const aiImporterDesc* Discreet3DSImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Setup configuration properties
void Discreet3DSImporter::SetupProperties(const Importer* /*pImp*/)
{
    // nothing to be done for the moment
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void Discreet3DSImporter::InternReadFile( const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
    StreamReaderLE stream(pIOHandler->Open(pFile,"rb"));
    this->stream = &stream;

    // We should have at least one chunk
    if (stream.GetRemainingSize() < 16) {
        throw DeadlyImportError("3DS file is either empty or corrupt: " + pFile);
    }

    // Allocate our temporary 3DS representation
    mScene = new D3DS::Scene();

    // Initialize members
    mLastNodeIndex             = -1;
    mCurrentNode               = new D3DS::Node("UNNAMED");
    mRootNode                  = mCurrentNode;
    mRootNode->mHierarchyPos   = -1;
    mRootNode->mHierarchyIndex = -1;
    mRootNode->mParent         = NULL;
    mMasterScale               = 1.0f;
    mBackgroundImage           = "";
    bHasBG                     = false;
    bIsPrj                     = false;

    // Parse the file
    ParseMainChunk();

    // Process all meshes in the file. First check whether all
    // face indices have valid values. The generate our
    // internal verbose representation. Finally compute normal
    // vectors from the smoothing groups we read from the
    // file.
    for (auto &mesh : mScene->mMeshes) {
        if (mesh.mFaces.size() > 0 && mesh.mPositions.size() == 0)  {
            delete mScene;
            throw DeadlyImportError("3DS file contains faces but no vertices: " + pFile);
        }
        CheckIndices(mesh);
        MakeUnique  (mesh);
        ComputeNormalsWithSmoothingsGroups<D3DS::Face>(mesh);
    }

    // Replace all occurrences of the default material with a
    // valid material. Generate it if no material containing
    // DEFAULT in its name has been found in the file
    ReplaceDefaultMaterial();

    // Convert the scene from our internal representation to an
    // aiScene object. This involves copying all meshes, lights
    // and cameras to the scene
    ConvertScene(pScene);

    // Generate the node graph for the scene. This is a little bit
    // tricky since we'll need to split some meshes into sub-meshes
    GenerateNodeGraph(pScene);

    // Now apply the master scaling factor to the scene
    ApplyMasterScale(pScene);

    // Delete our internal scene representation and the root
    // node, so the whole hierarchy will follow
    delete mRootNode;
    delete mScene;

    AI_DEBUG_INVALIDATE_PTR(mRootNode);
    AI_DEBUG_INVALIDATE_PTR(mScene);
    AI_DEBUG_INVALIDATE_PTR(this->stream);
}

// ------------------------------------------------------------------------------------------------
// Applies a master-scaling factor to the imported scene
void Discreet3DSImporter::ApplyMasterScale(aiScene* pScene)
{
    // There are some 3DS files with a zero scaling factor
    if (!mMasterScale)mMasterScale = 1.0f;
    else mMasterScale = 1.0f / mMasterScale;

    // Construct an uniform scaling matrix and multiply with it
    pScene->mRootNode->mTransformation *= aiMatrix4x4(
        mMasterScale,0.0f, 0.0f, 0.0f,
        0.0f, mMasterScale,0.0f, 0.0f,
        0.0f, 0.0f, mMasterScale,0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    // Check whether a scaling track is assigned to the root node.
}

// ------------------------------------------------------------------------------------------------
// Reads a new chunk from the file
void Discreet3DSImporter::ReadChunk(Discreet3DS::Chunk* pcOut)
{
    ai_assert(pcOut != NULL);

    pcOut->Flag = stream->GetI2();
    pcOut->Size = stream->GetI4();

    if (pcOut->Size - sizeof(Discreet3DS::Chunk) > stream->GetRemainingSize())
        throw DeadlyImportError("Chunk is too large");

    if (pcOut->Size - sizeof(Discreet3DS::Chunk) > stream->GetRemainingSizeToLimit()) {
        ASSIMP_LOG_ERROR("3DS: Chunk overflow");
    }
}

// ------------------------------------------------------------------------------------------------
// Skip a chunk
void Discreet3DSImporter::SkipChunk()
{
    Discreet3DS::Chunk psChunk;
    ReadChunk(&psChunk);

    stream->IncPtr(psChunk.Size-sizeof(Discreet3DS::Chunk));
    return;
}

// ------------------------------------------------------------------------------------------------
// Process the primary chunk of the file
void Discreet3DSImporter::ParseMainChunk()
{
    ASSIMP_3DS_BEGIN_CHUNK();

    // get chunk type
    switch (chunk.Flag)
    {

    case Discreet3DS::CHUNK_PRJ:
        bIsPrj = true;
    case Discreet3DS::CHUNK_MAIN:
        ParseEditorChunk();
        break;
    };

    ASSIMP_3DS_END_CHUNK();
    // recursively continue processing this hierarchy level
    return ParseMainChunk();
}

// ------------------------------------------------------------------------------------------------
void Discreet3DSImporter::ParseEditorChunk()
{
    ASSIMP_3DS_BEGIN_CHUNK();

    // get chunk type
    switch (chunk.Flag)
    {
    case Discreet3DS::CHUNK_OBJMESH:

        ParseObjectChunk();
        break;

    // NOTE: In several documentations in the internet this
    // chunk appears at different locations
    case Discreet3DS::CHUNK_KEYFRAMER:

        ParseKeyframeChunk();
        break;

    case Discreet3DS::CHUNK_VERSION:
        {
        // print the version number
        char buff[10];
        ASSIMP_itoa10(buff,stream->GetI2());
        ASSIMP_LOG_INFO_F(std::string("3DS file format version: "), buff);
        }
        break;
    };
    ASSIMP_3DS_END_CHUNK();
}

// ------------------------------------------------------------------------------------------------
void Discreet3DSImporter::ParseObjectChunk()
{
    ASSIMP_3DS_BEGIN_CHUNK();

    // get chunk type
    switch (chunk.Flag)
    {
    case Discreet3DS::CHUNK_OBJBLOCK:
        {
        unsigned int cnt = 0;
        const char* sz = (const char*)stream->GetPtr();

        // Get the name of the geometry object
        while (stream->GetI1())++cnt;
        ParseChunk(sz,cnt);
        }
        break;

    case Discreet3DS::CHUNK_MAT_MATERIAL:

        // Add a new material to the list
        mScene->mMaterials.push_back(D3DS::Material(std::string("UNNAMED_" + to_string(mScene->mMaterials.size()))));
        ParseMaterialChunk();
        break;

    case Discreet3DS::CHUNK_AMBCOLOR:

        // This is the ambient base color of the scene.
        // We add it to the ambient color of all materials
        ParseColorChunk(&mClrAmbient,true);
        if (is_qnan(mClrAmbient.r))
        {
            // We failed to read the ambient base color.
            ASSIMP_LOG_ERROR("3DS: Failed to read ambient base color");
            mClrAmbient.r = mClrAmbient.g = mClrAmbient.b = 0.0f;
        }
        break;

    case Discreet3DS::CHUNK_BIT_MAP:
        {
        // Specifies the background image. The string should already be
        // properly 0 terminated but we need to be sure
        unsigned int cnt = 0;
        const char* sz = (const char*)stream->GetPtr();
        while (stream->GetI1())++cnt;
        mBackgroundImage = std::string(sz,cnt);
        }
        break;

    case Discreet3DS::CHUNK_BIT_MAP_EXISTS:
        bHasBG = true;
        break;

    case Discreet3DS::CHUNK_MASTER_SCALE:
        // Scene master scaling factor
        mMasterScale = stream->GetF4();
        break;
    };
    ASSIMP_3DS_END_CHUNK();
}

// ------------------------------------------------------------------------------------------------
void Discreet3DSImporter::ParseChunk(const char* name, unsigned int num)
{
    ASSIMP_3DS_BEGIN_CHUNK();

    // IMPLEMENTATION NOTE;
    // Cameras or lights define their transformation in their parent node and in the
    // corresponding light or camera chunks. However, we read and process the latter
    // to to be able to return valid cameras/lights even if no scenegraph is given.

    // get chunk type
    switch (chunk.Flag)
    {
    case Discreet3DS::CHUNK_TRIMESH:
        {
        // this starts a new triangle mesh
        mScene->mMeshes.push_back(D3DS::Mesh(std::string(name, num)));

        // Read mesh chunks
        ParseMeshChunk();
        }
        break;

    case Discreet3DS::CHUNK_LIGHT:
        {
        // This starts a new light
        aiLight* light = new aiLight();
        mScene->mLights.push_back(light);

        light->mName.Set(std::string(name, num));

        // First read the position of the light
        light->mPosition.x = stream->GetF4();
        light->mPosition.y = stream->GetF4();
        light->mPosition.z = stream->GetF4();

        light->mColorDiffuse = aiColor3D(1.f,1.f,1.f);

        // Now check for further subchunks
        if (!bIsPrj) /* fixme */
            ParseLightChunk();

        // The specular light color is identical the the diffuse light color. The ambient light color
        // is equal to the ambient base color of the whole scene.
        light->mColorSpecular = light->mColorDiffuse;
        light->mColorAmbient  = mClrAmbient;

        if (light->mType == aiLightSource_UNDEFINED)
        {
            // It must be a point light
            light->mType = aiLightSource_POINT;
        }}
        break;

    case Discreet3DS::CHUNK_CAMERA:
        {
        // This starts a new camera
        aiCamera* camera = new aiCamera();
        mScene->mCameras.push_back(camera);
        camera->mName.Set(std::string(name, num));

        // First read the position of the camera
        camera->mPosition.x = stream->GetF4();
        camera->mPosition.y = stream->GetF4();
        camera->mPosition.z = stream->GetF4();

        // Then the camera target
        camera->mLookAt.x = stream->GetF4() - camera->mPosition.x;
        camera->mLookAt.y = stream->GetF4() - camera->mPosition.y;
        camera->mLookAt.z = stream->GetF4() - camera->mPosition.z;
        ai_real len = camera->mLookAt.Length();
        if (len < 1e-5) {

            // There are some files with lookat == position. Don't know why or whether it's ok or not.
            ASSIMP_LOG_ERROR("3DS: Unable to read proper camera look-at vector");
            camera->mLookAt = aiVector3D(0.0,1.0,0.0);

        }
        else camera->mLookAt /= len;

        // And finally - the camera rotation angle, in counter clockwise direction
        const ai_real angle =  AI_DEG_TO_RAD( stream->GetF4() );
        aiQuaternion quat(camera->mLookAt,angle);
        camera->mUp = quat.GetMatrix() * aiVector3D(0.0,1.0,0.0);

        // Read the lense angle
        camera->mHorizontalFOV = AI_DEG_TO_RAD ( stream->GetF4() );
        if (camera->mHorizontalFOV < 0.001f)  {
            camera->mHorizontalFOV = AI_DEG_TO_RAD(45.f);
        }

        // Now check for further subchunks
        if (!bIsPrj) /* fixme */ {
            ParseCameraChunk();
        }}
        break;
    };
    ASSIMP_3DS_END_CHUNK();
}

// ------------------------------------------------------------------------------------------------
void Discreet3DSImporter::ParseLightChunk()
{
    ASSIMP_3DS_BEGIN_CHUNK();
    aiLight* light = mScene->mLights.back();

    // get chunk type
    switch (chunk.Flag)
    {
    case Discreet3DS::CHUNK_DL_SPOTLIGHT:
        // Now we can be sure that the light is a spot light
        light->mType = aiLightSource_SPOT;

        // We wouldn't need to normalize here, but we do it
        light->mDirection.x = stream->GetF4() - light->mPosition.x;
        light->mDirection.y = stream->GetF4() - light->mPosition.y;
        light->mDirection.z = stream->GetF4() - light->mPosition.z;
        light->mDirection.Normalize();

        // Now the hotspot and falloff angles - in degrees
        light->mAngleInnerCone = AI_DEG_TO_RAD( stream->GetF4() );

        // FIX: the falloff angle is just an offset
        light->mAngleOuterCone = light->mAngleInnerCone+AI_DEG_TO_RAD( stream->GetF4() );
        break;

        // intensity multiplier
    case Discreet3DS::CHUNK_DL_MULTIPLIER:
        light->mColorDiffuse = light->mColorDiffuse * stream->GetF4();
        break;

        // light color
    case Discreet3DS::CHUNK_RGBF:
    case Discreet3DS::CHUNK_LINRGBF:
        light->mColorDiffuse.r *= stream->GetF4();
        light->mColorDiffuse.g *= stream->GetF4();
        light->mColorDiffuse.b *= stream->GetF4();
        break;

        // light attenuation
    case Discreet3DS::CHUNK_DL_ATTENUATE:
        light->mAttenuationLinear = stream->GetF4();
        break;
    };

    ASSIMP_3DS_END_CHUNK();
}

// ------------------------------------------------------------------------------------------------
void Discreet3DSImporter::ParseCameraChunk()
{
    ASSIMP_3DS_BEGIN_CHUNK();
    aiCamera* camera = mScene->mCameras.back();

    // get chunk type
    switch (chunk.Flag)
    {
        // near and far clip plane
    case Discreet3DS::CHUNK_CAM_RANGES:
        camera->mClipPlaneNear = stream->GetF4();
        camera->mClipPlaneFar  = stream->GetF4();
        break;
    }

    ASSIMP_3DS_END_CHUNK();
}

// ------------------------------------------------------------------------------------------------
void Discreet3DSImporter::ParseKeyframeChunk()
{
    ASSIMP_3DS_BEGIN_CHUNK();

    // get chunk type
    switch (chunk.Flag)
    {
    case Discreet3DS::CHUNK_TRACKCAMTGT:
    case Discreet3DS::CHUNK_TRACKSPOTL:
    case Discreet3DS::CHUNK_TRACKCAMERA:
    case Discreet3DS::CHUNK_TRACKINFO:
    case Discreet3DS::CHUNK_TRACKLIGHT:
    case Discreet3DS::CHUNK_TRACKLIGTGT:

        // this starts a new mesh hierarchy chunk
        ParseHierarchyChunk(chunk.Flag);
        break;
    };

    ASSIMP_3DS_END_CHUNK();
}

// ------------------------------------------------------------------------------------------------
// Little helper function for ParseHierarchyChunk
void Discreet3DSImporter::InverseNodeSearch(D3DS::Node* pcNode,D3DS::Node* pcCurrent)
{
    if (!pcCurrent) {
        mRootNode->push_back(pcNode);
        return;
    }

    if (pcCurrent->mHierarchyPos == pcNode->mHierarchyPos)  {
        if(pcCurrent->mParent) {
            pcCurrent->mParent->push_back(pcNode);
        }
        else pcCurrent->push_back(pcNode);
        return;
    }
    return InverseNodeSearch(pcNode,pcCurrent->mParent);
}

// ------------------------------------------------------------------------------------------------
// Find a node with a specific name in the import hierarchy
D3DS::Node* FindNode(D3DS::Node* root, const std::string& name)
{
    if (root->mName == name)
        return root;
    for (std::vector<D3DS::Node*>::iterator it = root->mChildren.begin();it != root->mChildren.end(); ++it) {
        D3DS::Node* nd;
        if (( nd = FindNode(*it,name)))
            return nd;
    }
    return NULL;
}

// ------------------------------------------------------------------------------------------------
// Binary predicate for std::unique()
template <class T>
bool KeyUniqueCompare(const T& first, const T& second)
{
    return first.mTime == second.mTime;
}

// ------------------------------------------------------------------------------------------------
// Skip some additional import data.
void Discreet3DSImporter::SkipTCBInfo()
{
    unsigned int flags = stream->GetI2();

    if (!flags) {
        // Currently we can't do anything with these values. They occur
        // quite rare, so it wouldn't be worth the effort implementing
        // them. 3DS is not really suitable for complex animations,
        // so full support is not required.
        ASSIMP_LOG_WARN("3DS: Skipping TCB animation info");
    }

    if (flags & Discreet3DS::KEY_USE_TENS) {
        stream->IncPtr(4);
    }
    if (flags & Discreet3DS::KEY_USE_BIAS) {
        stream->IncPtr(4);
    }
    if (flags & Discreet3DS::KEY_USE_CONT) {
        stream->IncPtr(4);
    }
    if (flags & Discreet3DS::KEY_USE_EASE_FROM) {
        stream->IncPtr(4);
    }
    if (flags & Discreet3DS::KEY_USE_EASE_TO) {
        stream->IncPtr(4);
    }
}

// ------------------------------------------------------------------------------------------------
// Read hierarchy and keyframe info
void Discreet3DSImporter::ParseHierarchyChunk(uint16_t parent)
{
    ASSIMP_3DS_BEGIN_CHUNK();

    // get chunk type
    switch (chunk.Flag)
    {
    case Discreet3DS::CHUNK_TRACKOBJNAME:

        // This is the name of the object to which the track applies. The chunk also
        // defines the position of this object in the hierarchy.
        {

        // First of all: get the name of the object
        unsigned int cnt = 0;
        const char* sz = (const char*)stream->GetPtr();

        while (stream->GetI1())++cnt;
        std::string name = std::string(sz,cnt);

        // Now find out whether we have this node already (target animation channels
        // are stored with a separate object ID)
        D3DS::Node* pcNode = FindNode(mRootNode,name);
        int instanceNumber = 1;

        if ( pcNode)
        {
            // if the source is not a CHUNK_TRACKINFO block it won't be an object instance
            if (parent != Discreet3DS::CHUNK_TRACKINFO)
            {
                mCurrentNode = pcNode;
                break;
            }
            pcNode->mInstanceCount++;
            instanceNumber = pcNode->mInstanceCount;
        }
        pcNode = new D3DS::Node(name);
        pcNode->mInstanceNumber = instanceNumber;

        // There are two unknown values which we can safely ignore
        stream->IncPtr(4);

        // Now read the hierarchy position of the object
        uint16_t hierarchy = stream->GetI2() + 1;
        pcNode->mHierarchyPos   = hierarchy;
        pcNode->mHierarchyIndex = mLastNodeIndex;

        // And find a proper position in the graph for it
        if (mCurrentNode && mCurrentNode->mHierarchyPos == hierarchy)   {

            // add to the parent of the last touched node
            mCurrentNode->mParent->push_back(pcNode);
            mLastNodeIndex++;
        }
        else if(hierarchy >= mLastNodeIndex)    {

            // place it at the current position in the hierarchy
            mCurrentNode->push_back(pcNode);
            mLastNodeIndex = hierarchy;
        }
        else    {
            // need to go back to the specified position in the hierarchy.
            InverseNodeSearch(pcNode,mCurrentNode);
            mLastNodeIndex++;
        }
        // Make this node the current node
        mCurrentNode = pcNode;
        }
        break;

    case Discreet3DS::CHUNK_TRACKDUMMYOBJNAME:

        // This is the "real" name of a $$$DUMMY object
        {
            const char* sz = (const char*) stream->GetPtr();
            while (stream->GetI1());

            // If object name is DUMMY, take this one instead
            if (mCurrentNode->mName == "$$$DUMMY")  {
                mCurrentNode->mName = std::string(sz);
                break;
            }
        }
        break;

    case Discreet3DS::CHUNK_TRACKPIVOT:

        if ( Discreet3DS::CHUNK_TRACKINFO != parent)
        {
            ASSIMP_LOG_WARN("3DS: Skipping pivot subchunk for non usual object");
            break;
        }

        // Pivot = origin of rotation and scaling
        mCurrentNode->vPivot.x = stream->GetF4();
        mCurrentNode->vPivot.y = stream->GetF4();
        mCurrentNode->vPivot.z = stream->GetF4();
        break;


        // ////////////////////////////////////////////////////////////////////
        // POSITION KEYFRAME
    case Discreet3DS::CHUNK_TRACKPOS:
        {
        stream->IncPtr(10);
        const unsigned int numFrames = stream->GetI4();
        bool sortKeys = false;

        // This could also be meant as the target position for
        // (targeted) lights and cameras
        std::vector<aiVectorKey>* l;
        if ( Discreet3DS::CHUNK_TRACKCAMTGT == parent || Discreet3DS::CHUNK_TRACKLIGTGT == parent)  {
            l = & mCurrentNode->aTargetPositionKeys;
        }
        else l = & mCurrentNode->aPositionKeys;

        l->reserve(numFrames);
        for (unsigned int i = 0; i < numFrames;++i) {
            const unsigned int fidx = stream->GetI4();

            // Setup a new position key
            aiVectorKey v;
            v.mTime = (double)fidx;

            SkipTCBInfo();
            v.mValue.x = stream->GetF4();
            v.mValue.y = stream->GetF4();
            v.mValue.z = stream->GetF4();

            // check whether we'll need to sort the keys
            if (!l->empty() && v.mTime <= l->back().mTime)
                sortKeys = true;

            // Add the new keyframe to the list
            l->push_back(v);
        }

        // Sort all keys with ascending time values and remove duplicates?
        if (sortKeys)   {
            std::stable_sort(l->begin(),l->end());
            l->erase ( std::unique (l->begin(),l->end(),&KeyUniqueCompare<aiVectorKey>), l->end() );
        }}

        break;

        // ////////////////////////////////////////////////////////////////////
        // CAMERA ROLL KEYFRAME
    case Discreet3DS::CHUNK_TRACKROLL:
        {
        // roll keys are accepted for cameras only
        if (parent != Discreet3DS::CHUNK_TRACKCAMERA)   {
            ASSIMP_LOG_WARN("3DS: Ignoring roll track for non-camera object");
            break;
        }
        bool sortKeys = false;
        std::vector<aiFloatKey>* l = &mCurrentNode->aCameraRollKeys;

        stream->IncPtr(10);
        const unsigned int numFrames = stream->GetI4();
        l->reserve(numFrames);
        for (unsigned int i = 0; i < numFrames;++i) {
            const unsigned int fidx = stream->GetI4();

            // Setup a new position key
            aiFloatKey v;
            v.mTime = (double)fidx;

            // This is just a single float
            SkipTCBInfo();
            v.mValue = stream->GetF4();

            // Check whether we'll need to sort the keys
            if (!l->empty() && v.mTime <= l->back().mTime)
                sortKeys = true;

            // Add the new keyframe to the list
            l->push_back(v);
        }

        // Sort all keys with ascending time values and remove duplicates?
        if (sortKeys)   {
            std::stable_sort(l->begin(),l->end());
            l->erase ( std::unique (l->begin(),l->end(),&KeyUniqueCompare<aiFloatKey>), l->end() );
        }}
        break;


        // ////////////////////////////////////////////////////////////////////
        // CAMERA FOV KEYFRAME
    case Discreet3DS::CHUNK_TRACKFOV:
        {
            ASSIMP_LOG_ERROR("3DS: Skipping FOV animation track. "
                "This is not supported");
        }
        break;


        // ////////////////////////////////////////////////////////////////////
        // ROTATION KEYFRAME
    case Discreet3DS::CHUNK_TRACKROTATE:
        {
        stream->IncPtr(10);
        const unsigned int numFrames = stream->GetI4();

        bool sortKeys = false;
        std::vector<aiQuatKey>* l = &mCurrentNode->aRotationKeys;
        l->reserve(numFrames);

        for (unsigned int i = 0; i < numFrames;++i) {
            const unsigned int fidx = stream->GetI4();
            SkipTCBInfo();

            aiQuatKey v;
            v.mTime = (double)fidx;

            // The rotation keyframe is given as an axis-angle pair
            const float rad = stream->GetF4();
            aiVector3D axis;
            axis.x = stream->GetF4();
            axis.y = stream->GetF4();
            axis.z = stream->GetF4();

            if (!axis.x && !axis.y && !axis.z)
                axis.y = 1.f;

            // Construct a rotation quaternion from the axis-angle pair
            v.mValue = aiQuaternion(axis,rad);

            // Check whether we'll need to sort the keys
            if (!l->empty() && v.mTime <= l->back().mTime)
                sortKeys = true;

            // add the new keyframe to the list
            l->push_back(v);
        }
        // Sort all keys with ascending time values and remove duplicates?
        if (sortKeys)   {
            std::stable_sort(l->begin(),l->end());
            l->erase ( std::unique (l->begin(),l->end(),&KeyUniqueCompare<aiQuatKey>), l->end() );
        }}
        break;

        // ////////////////////////////////////////////////////////////////////
        // SCALING KEYFRAME
    case Discreet3DS::CHUNK_TRACKSCALE:
        {
        stream->IncPtr(10);
        const unsigned int numFrames = stream->GetI2();
        stream->IncPtr(2);

        bool sortKeys = false;
        std::vector<aiVectorKey>* l = &mCurrentNode->aScalingKeys;
        l->reserve(numFrames);

        for (unsigned int i = 0; i < numFrames;++i) {
            const unsigned int fidx = stream->GetI4();
            SkipTCBInfo();

            // Setup a new key
            aiVectorKey v;
            v.mTime = (double)fidx;

            // ... and read its value
            v.mValue.x = stream->GetF4();
            v.mValue.y = stream->GetF4();
            v.mValue.z = stream->GetF4();

            // check whether we'll need to sort the keys
            if (!l->empty() && v.mTime <= l->back().mTime)
                sortKeys = true;

            // Remove zero-scalings on singular axes - they've been reported to be there erroneously in some strange files
            if (!v.mValue.x) v.mValue.x = 1.f;
            if (!v.mValue.y) v.mValue.y = 1.f;
            if (!v.mValue.z) v.mValue.z = 1.f;

            l->push_back(v);
        }
        // Sort all keys with ascending time values and remove duplicates?
        if (sortKeys)   {
            std::stable_sort(l->begin(),l->end());
            l->erase ( std::unique (l->begin(),l->end(),&KeyUniqueCompare<aiVectorKey>), l->end() );
        }}
        break;
    };

    ASSIMP_3DS_END_CHUNK();
}

// ------------------------------------------------------------------------------------------------
// Read a face chunk - it contains smoothing groups and material assignments
void Discreet3DSImporter::ParseFaceChunk()
{
    ASSIMP_3DS_BEGIN_CHUNK();

    // Get the mesh we're currently working on
    D3DS::Mesh& mMesh = mScene->mMeshes.back();

    // Get chunk type
    switch (chunk.Flag)
    {
    case Discreet3DS::CHUNK_SMOOLIST:
        {
        // This is the list of smoothing groups - a bitfield for every face.
        // Up to 32 smoothing groups assigned to a single face.
        unsigned int num = chunkSize/4, m = 0;
        if (num > mMesh.mFaces.size())  {
            throw DeadlyImportError("3DS: More smoothing groups than faces");
        }
        for (std::vector<D3DS::Face>::iterator i =  mMesh.mFaces.begin(); m != num;++i, ++m)    {
            // nth bit is set for nth smoothing group
            (*i).iSmoothGroup = stream->GetI4();
        }}
        break;

    case Discreet3DS::CHUNK_FACEMAT:
        {
        // at fist an asciiz with the material name
        const char* sz = (const char*)stream->GetPtr();
        while (stream->GetI1());

        // find the index of the material
        unsigned int idx = 0xcdcdcdcd, cnt = 0;
        for (std::vector<D3DS::Material>::const_iterator i =  mScene->mMaterials.begin();i != mScene->mMaterials.end();++i,++cnt)   {
            // use case independent comparisons. hopefully it will work.
            if ((*i).mName.length() && !ASSIMP_stricmp(sz, (*i).mName.c_str())) {
                idx = cnt;
                break;
            }
        }
        if (0xcdcdcdcd == idx)  {
            ASSIMP_LOG_ERROR_F( "3DS: Unknown material: ", sz);
        }

        // Now continue and read all material indices
        cnt = (uint16_t)stream->GetI2();
        for (unsigned int i = 0; i < cnt;++i)   {
            unsigned int fidx = (uint16_t)stream->GetI2();

            // check range
            if (fidx >= mMesh.mFaceMaterials.size())    {
                ASSIMP_LOG_ERROR("3DS: Invalid face index in face material list");
            }
            else mMesh.mFaceMaterials[fidx] = idx;
        }}
        break;
    };
    ASSIMP_3DS_END_CHUNK();
}

// ------------------------------------------------------------------------------------------------
// Read a mesh chunk. Here's the actual mesh data
void Discreet3DSImporter::ParseMeshChunk()
{
    ASSIMP_3DS_BEGIN_CHUNK();

    // Get the mesh we're currently working on
    D3DS::Mesh& mMesh = mScene->mMeshes.back();

    // get chunk type
    switch (chunk.Flag)
    {
    case Discreet3DS::CHUNK_VERTLIST:
        {
        // This is the list of all vertices in the current mesh
        int num = (int)(uint16_t)stream->GetI2();
        mMesh.mPositions.reserve(num);
        while (num-- > 0)   {
            aiVector3D v;
            v.x = stream->GetF4();
            v.y = stream->GetF4();
            v.z = stream->GetF4();
            mMesh.mPositions.push_back(v);
        }}
        break;
    case Discreet3DS::CHUNK_TRMATRIX:
        {
        // This is the RLEATIVE transformation matrix of the current mesh. Vertices are
        // pretransformed by this matrix wonder.
        mMesh.mMat.a1 = stream->GetF4();
        mMesh.mMat.b1 = stream->GetF4();
        mMesh.mMat.c1 = stream->GetF4();
        mMesh.mMat.a2 = stream->GetF4();
        mMesh.mMat.b2 = stream->GetF4();
        mMesh.mMat.c2 = stream->GetF4();
        mMesh.mMat.a3 = stream->GetF4();
        mMesh.mMat.b3 = stream->GetF4();
        mMesh.mMat.c3 = stream->GetF4();
        mMesh.mMat.a4 = stream->GetF4();
        mMesh.mMat.b4 = stream->GetF4();
        mMesh.mMat.c4 = stream->GetF4();
        }
        break;

    case Discreet3DS::CHUNK_MAPLIST:
        {
        // This is the list of all UV coords in the current mesh
        int num = (int)(uint16_t)stream->GetI2();
        mMesh.mTexCoords.reserve(num);
        while (num-- > 0)   {
            aiVector3D v;
            v.x = stream->GetF4();
            v.y = stream->GetF4();
            mMesh.mTexCoords.push_back(v);
        }}
        break;

    case Discreet3DS::CHUNK_FACELIST:
        {
        // This is the list of all faces in the current mesh
        int num = (int)(uint16_t)stream->GetI2();
        mMesh.mFaces.reserve(num);
        while (num-- > 0)   {
            // 3DS faces are ALWAYS triangles
            mMesh.mFaces.push_back(D3DS::Face());
            D3DS::Face& sFace = mMesh.mFaces.back();

            sFace.mIndices[0] = (uint16_t)stream->GetI2();
            sFace.mIndices[1] = (uint16_t)stream->GetI2();
            sFace.mIndices[2] = (uint16_t)stream->GetI2();

            stream->IncPtr(2); // skip edge visibility flag
        }

        // Resize the material array (0xcdcdcdcd marks the default material; so if a face is
        // not referenced by a material, $$DEFAULT will be assigned to it)
        mMesh.mFaceMaterials.resize(mMesh.mFaces.size(),0xcdcdcdcd);

        // Larger 3DS files could have multiple FACE chunks here
        chunkSize = stream->GetRemainingSizeToLimit();
        if ( chunkSize > (int) sizeof(Discreet3DS::Chunk ) )
            ParseFaceChunk();
        }
        break;
    };
    ASSIMP_3DS_END_CHUNK();
}

// ------------------------------------------------------------------------------------------------
// Read a 3DS material chunk
void Discreet3DSImporter::ParseMaterialChunk()
{
    ASSIMP_3DS_BEGIN_CHUNK();
    switch (chunk.Flag)
    {
    case Discreet3DS::CHUNK_MAT_MATNAME:

        {
        // The material name string is already zero-terminated, but we need to be sure ...
        const char* sz = (const char*)stream->GetPtr();
        unsigned int cnt = 0;
        while (stream->GetI1())
            ++cnt;

        if (!cnt)   {
            // This may not be, we use the default name instead
            ASSIMP_LOG_ERROR("3DS: Empty material name");
        }
        else mScene->mMaterials.back().mName = std::string(sz,cnt);
        }
        break;

    case Discreet3DS::CHUNK_MAT_DIFFUSE:
        {
        // This is the diffuse material color
        aiColor3D* pc = &mScene->mMaterials.back().mDiffuse;
        ParseColorChunk(pc);
        if (is_qnan(pc->r)) {
            // color chunk is invalid. Simply ignore it
            ASSIMP_LOG_ERROR("3DS: Unable to read DIFFUSE chunk");
            pc->r = pc->g = pc->b = 1.0f;
        }}
        break;

    case Discreet3DS::CHUNK_MAT_SPECULAR:
        {
        // This is the specular material color
        aiColor3D* pc = &mScene->mMaterials.back().mSpecular;
        ParseColorChunk(pc);
        if (is_qnan(pc->r)) {
            // color chunk is invalid. Simply ignore it
            ASSIMP_LOG_ERROR("3DS: Unable to read SPECULAR chunk");
            pc->r = pc->g = pc->b = 1.0f;
        }}
        break;

    case Discreet3DS::CHUNK_MAT_AMBIENT:
        {
        // This is the ambient material color
        aiColor3D* pc = &mScene->mMaterials.back().mAmbient;
        ParseColorChunk(pc);
        if (is_qnan(pc->r)) {
            // color chunk is invalid. Simply ignore it
            ASSIMP_LOG_ERROR("3DS: Unable to read AMBIENT chunk");
            pc->r = pc->g = pc->b = 0.0f;
        }}
        break;

    case Discreet3DS::CHUNK_MAT_SELF_ILLUM:
        {
        // This is the emissive material color
        aiColor3D* pc = &mScene->mMaterials.back().mEmissive;
        ParseColorChunk(pc);
        if (is_qnan(pc->r)) {
            // color chunk is invalid. Simply ignore it
            ASSIMP_LOG_ERROR("3DS: Unable to read EMISSIVE chunk");
            pc->r = pc->g = pc->b = 0.0f;
        }}
        break;

    case Discreet3DS::CHUNK_MAT_TRANSPARENCY:
        {
            // This is the material's transparency
            ai_real* pcf = &mScene->mMaterials.back().mTransparency;
            *pcf = ParsePercentageChunk();

            // NOTE: transparency, not opacity
            if (is_qnan(*pcf))
                *pcf = ai_real( 1.0 );
            else 
                *pcf = ai_real( 1.0 ) - *pcf * (ai_real)0xFFFF / ai_real( 100.0 );
        }
        break;

    case Discreet3DS::CHUNK_MAT_SHADING:
        // This is the material shading mode
        mScene->mMaterials.back().mShading = (D3DS::Discreet3DS::shadetype3ds)stream->GetI2();
        break;

    case Discreet3DS::CHUNK_MAT_TWO_SIDE:
        // This is the two-sided flag
        mScene->mMaterials.back().mTwoSided = true;
        break;

    case Discreet3DS::CHUNK_MAT_SHININESS:
        { // This is the shininess of the material
        ai_real* pcf = &mScene->mMaterials.back().mSpecularExponent;
        *pcf = ParsePercentageChunk();
        if (is_qnan(*pcf))
            *pcf = 0.0;
        else *pcf *= (ai_real)0xFFFF;
        }
        break;

    case Discreet3DS::CHUNK_MAT_SHININESS_PERCENT:
        { // This is the shininess strength of the material
            ai_real* pcf = &mScene->mMaterials.back().mShininessStrength;
            *pcf = ParsePercentageChunk();
            if (is_qnan(*pcf))
                *pcf = ai_real( 0.0 );
            else 
                *pcf *= (ai_real)0xffff / ai_real( 100.0 );
        }
        break;

    case Discreet3DS::CHUNK_MAT_SELF_ILPCT:
        { // This is the self illumination strength of the material
            ai_real f = ParsePercentageChunk();
            if (is_qnan(f))
                f = ai_real( 0.0 );
            else 
                f *= (ai_real)0xFFFF / ai_real( 100.0 );
            mScene->mMaterials.back().mEmissive = aiColor3D(f,f,f);
        }
        break;

    // Parse texture chunks
    case Discreet3DS::CHUNK_MAT_TEXTURE:
        // Diffuse texture
        ParseTextureChunk(&mScene->mMaterials.back().sTexDiffuse);
        break;
    case Discreet3DS::CHUNK_MAT_BUMPMAP:
        // Height map
        ParseTextureChunk(&mScene->mMaterials.back().sTexBump);
        break;
    case Discreet3DS::CHUNK_MAT_OPACMAP:
        // Opacity texture
        ParseTextureChunk(&mScene->mMaterials.back().sTexOpacity);
        break;
    case Discreet3DS::CHUNK_MAT_MAT_SHINMAP:
        // Shininess map
        ParseTextureChunk(&mScene->mMaterials.back().sTexShininess);
        break;
    case Discreet3DS::CHUNK_MAT_SPECMAP:
        // Specular map
        ParseTextureChunk(&mScene->mMaterials.back().sTexSpecular);
        break;
    case Discreet3DS::CHUNK_MAT_SELFIMAP:
        // Self-illumination (emissive) map
        ParseTextureChunk(&mScene->mMaterials.back().sTexEmissive);
        break;
    case Discreet3DS::CHUNK_MAT_REFLMAP:
        // Reflection map
        ParseTextureChunk(&mScene->mMaterials.back().sTexReflective);
        break;
    };
    ASSIMP_3DS_END_CHUNK();
}

// ------------------------------------------------------------------------------------------------
void Discreet3DSImporter::ParseTextureChunk(D3DS::Texture* pcOut)
{
    ASSIMP_3DS_BEGIN_CHUNK();

    // get chunk type
    switch (chunk.Flag)
    {
    case Discreet3DS::CHUNK_MAPFILE:
        {
        // The material name string is already zero-terminated, but we need to be sure ...
        const char* sz = (const char*)stream->GetPtr();
        unsigned int cnt = 0;
        while (stream->GetI1())
            ++cnt;
        pcOut->mMapName = std::string(sz,cnt);
        }
        break;


    case Discreet3DS::CHUNK_PERCENTD:
        // Manually parse the blend factor
        pcOut->mTextureBlend = ai_real( stream->GetF8() );
        break;

    case Discreet3DS::CHUNK_PERCENTF:
        // Manually parse the blend factor
        pcOut->mTextureBlend = stream->GetF4();
        break;

    case Discreet3DS::CHUNK_PERCENTW:
        // Manually parse the blend factor
        pcOut->mTextureBlend = (ai_real)((uint16_t)stream->GetI2()) / ai_real( 100.0 );
        break;

    case Discreet3DS::CHUNK_MAT_MAP_USCALE:
        // Texture coordinate scaling in the U direction
        pcOut->mScaleU = stream->GetF4();
        if (0.0f == pcOut->mScaleU)
        {
            ASSIMP_LOG_WARN("Texture coordinate scaling in the x direction is zero. Assuming 1.");
            pcOut->mScaleU = 1.0f;
        }
        break;
    case Discreet3DS::CHUNK_MAT_MAP_VSCALE:
        // Texture coordinate scaling in the V direction
        pcOut->mScaleV = stream->GetF4();
        if (0.0f == pcOut->mScaleV)
        {
            ASSIMP_LOG_WARN("Texture coordinate scaling in the y direction is zero. Assuming 1.");
            pcOut->mScaleV = 1.0f;
        }
        break;

    case Discreet3DS::CHUNK_MAT_MAP_UOFFSET:
        // Texture coordinate offset in the U direction
        pcOut->mOffsetU = -stream->GetF4();
        break;

    case Discreet3DS::CHUNK_MAT_MAP_VOFFSET:
        // Texture coordinate offset in the V direction
        pcOut->mOffsetV = stream->GetF4();
        break;

    case Discreet3DS::CHUNK_MAT_MAP_ANG:
        // Texture coordinate rotation, CCW in DEGREES
        pcOut->mRotation = -AI_DEG_TO_RAD( stream->GetF4() );
        break;

    case Discreet3DS::CHUNK_MAT_MAP_TILING:
        {
        const uint16_t iFlags = stream->GetI2();

        // Get the mapping mode (for both axes)
        if (iFlags & 0x2u)
            pcOut->mMapMode = aiTextureMapMode_Mirror;

        else if (iFlags & 0x10u)
            pcOut->mMapMode = aiTextureMapMode_Decal;

        // wrapping in all remaining cases
        else pcOut->mMapMode = aiTextureMapMode_Wrap;
        }
        break;
    };

    ASSIMP_3DS_END_CHUNK();
}

// ------------------------------------------------------------------------------------------------
// Read a percentage chunk
ai_real Discreet3DSImporter::ParsePercentageChunk()
{
    Discreet3DS::Chunk chunk;
    ReadChunk(&chunk);

    if (Discreet3DS::CHUNK_PERCENTF == chunk.Flag)
        return stream->GetF4();
    else if (Discreet3DS::CHUNK_PERCENTW == chunk.Flag)
        return (ai_real)((uint16_t)stream->GetI2()) / (ai_real)0xFFFF;
    return get_qnan();
}

// ------------------------------------------------------------------------------------------------
// Read a color chunk. If a percentage chunk is found instead it is read as a grayscale color
void Discreet3DSImporter::ParseColorChunk( aiColor3D* out, bool acceptPercent )
{
    ai_assert(out != NULL);

    // error return value
    const ai_real qnan = get_qnan();
    static const aiColor3D clrError = aiColor3D(qnan,qnan,qnan);

    Discreet3DS::Chunk chunk;
    ReadChunk(&chunk);
    const unsigned int diff = chunk.Size - sizeof(Discreet3DS::Chunk);

    bool bGamma = false;

    // Get the type of the chunk
    switch(chunk.Flag)
    {
    case Discreet3DS::CHUNK_LINRGBF:
        bGamma = true;

    case Discreet3DS::CHUNK_RGBF:
        if (sizeof(float) * 3 > diff)   {
            *out = clrError;
            return;
        }
        out->r = stream->GetF4();
        out->g = stream->GetF4();
        out->b = stream->GetF4();
        break;

    case Discreet3DS::CHUNK_LINRGBB:
        bGamma = true;
    case Discreet3DS::CHUNK_RGBB:
        {
            if ( sizeof( char ) * 3 > diff ) {
                *out = clrError;
                return;
            }
            const ai_real invVal = ai_real( 1.0 ) / ai_real( 255.0 );
            out->r = ( ai_real ) ( uint8_t ) stream->GetI1() * invVal;
            out->g = ( ai_real ) ( uint8_t ) stream->GetI1() * invVal;
            out->b = ( ai_real ) ( uint8_t ) stream->GetI1() * invVal;
        }
        break;

    // Percentage chunks are accepted, too.
    case Discreet3DS::CHUNK_PERCENTF:
        if (acceptPercent && 4 <= diff) {
            out->g = out->b = out->r = stream->GetF4();
            break;
        }
        *out = clrError;
        return;

    case Discreet3DS::CHUNK_PERCENTW:
        if (acceptPercent && 1 <= diff) {
            out->g = out->b = out->r = (ai_real)(uint8_t)stream->GetI1() / ai_real( 255.0 );
            break;
        }
        *out = clrError;
        return;

    default:
        stream->IncPtr(diff);
        // Skip unknown chunks, hope this won't cause any problems.
        return ParseColorChunk(out,acceptPercent);
    };
    (void)bGamma;
}

#endif // !! ASSIMP_BUILD_NO_3DS_IMPORTER
