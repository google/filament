/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team



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

/** @file  LWSLoader.cpp
 *  @brief Implementation of the LWS importer class
 */


#ifndef ASSIMP_BUILD_NO_LWS_IMPORTER

#include "LWS/LWSLoader.h"
#include "PostProcessing/ConvertToLHProcess.h"
#include "Common/Importer.h"

#include <assimp/ParsingUtils.h>
#include <assimp/fast_atof.h>
#include <assimp/SceneCombiner.h>
#include <assimp/GenericProperty.h>
#include <assimp/SkeletonMeshBuilder.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/scene.h>
#include <assimp/IOSystem.hpp>
#include <assimp/importerdesc.h>

#include <memory>

using namespace Assimp;

static const aiImporterDesc desc = {
    "LightWave Scene Importer",
    "",
    "",
    "http://www.newtek.com/lightwave.html=",
    aiImporterFlags_SupportTextFlavour,
    0,
    0,
    0,
    0,
    "lws mot"
};

// ------------------------------------------------------------------------------------------------
// Recursive parsing of LWS files
void LWS::Element::Parse (const char*& buffer)
{
    for (;SkipSpacesAndLineEnd(&buffer);SkipLine(&buffer)) {

        // begin of a new element with children
        bool sub = false;
        if (*buffer == '{') {
            ++buffer;
            SkipSpaces(&buffer);
            sub = true;
        }
        else if (*buffer == '}')
            return;

        children.push_back(Element());

        // copy data line - read token per token

        const char* cur = buffer;
        while (!IsSpaceOrNewLine(*buffer)) ++buffer;
        children.back().tokens[0] = std::string(cur,(size_t) (buffer-cur));
        SkipSpaces(&buffer);

        if (children.back().tokens[0] == "Plugin")
        {
            ASSIMP_LOG_DEBUG("LWS: Skipping over plugin-specific data");

            // strange stuff inside Plugin/Endplugin blocks. Needn't
            // follow LWS syntax, so we skip over it
            for (;SkipSpacesAndLineEnd(&buffer);SkipLine(&buffer)) {
                if (!::strncmp(buffer,"EndPlugin",9)) {
                    //SkipLine(&buffer);
                    break;
                }
            }
            continue;
        }

        cur = buffer;
        while (!IsLineEnd(*buffer)) ++buffer;
        children.back().tokens[1] = std::string(cur,(size_t) (buffer-cur));

        // parse more elements recursively
        if (sub)
            children.back().Parse(buffer);
    }
}

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
LWSImporter::LWSImporter()
    : configSpeedFlag(),
    io(),
    first(),
    last(),
    fps(),
    noSkeletonMesh()
{
    // nothing to do here
}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
LWSImporter::~LWSImporter()
{
    // nothing to do here
}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool LWSImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler,bool checkSig) const
{
    const std::string extension = GetExtension(pFile);
    if (extension == "lws" || extension == "mot")
        return true;

    // if check for extension is not enough, check for the magic tokens LWSC and LWMO
    if (!extension.length() || checkSig) {
        uint32_t tokens[2];
        tokens[0] = AI_MAKE_MAGIC("LWSC");
        tokens[1] = AI_MAKE_MAGIC("LWMO");
        return CheckMagicToken(pIOHandler,pFile,tokens,2);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Get list of file extensions
const aiImporterDesc* LWSImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Setup configuration properties
void LWSImporter::SetupProperties(const Importer* pImp)
{
    // AI_CONFIG_FAVOUR_SPEED
    configSpeedFlag = (0 != pImp->GetPropertyInteger(AI_CONFIG_FAVOUR_SPEED,0));

    // AI_CONFIG_IMPORT_LWS_ANIM_START
    first = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_LWS_ANIM_START,
        150392 /* magic hack */);

    // AI_CONFIG_IMPORT_LWS_ANIM_END
    last = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_LWS_ANIM_END,
        150392 /* magic hack */);

    if (last < first) {
        std::swap(last,first);
    }

    noSkeletonMesh = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_NO_SKELETON_MESHES,0) != 0;
}

// ------------------------------------------------------------------------------------------------
// Read an envelope description
void LWSImporter::ReadEnvelope(const LWS::Element& dad, LWO::Envelope& fill )
{
    if (dad.children.empty()) {
        ASSIMP_LOG_ERROR("LWS: Envelope descriptions must not be empty");
        return;
    }

    // reserve enough storage
    std::list< LWS::Element >::const_iterator it = dad.children.begin();;
    fill.keys.reserve(strtoul10(it->tokens[1].c_str()));

    for (++it; it != dad.children.end(); ++it) {
        const char* c = (*it).tokens[1].c_str();

        if ((*it).tokens[0] == "Key") {
            fill.keys.push_back(LWO::Key());
            LWO::Key& key = fill.keys.back();

            float f;
            SkipSpaces(&c);
            c = fast_atoreal_move<float>(c,key.value);
            SkipSpaces(&c);
            c = fast_atoreal_move<float>(c,f);

            key.time = f;

            unsigned int span = strtoul10(c,&c), num = 0;
            switch (span) {

                case 0:
                    key.inter = LWO::IT_TCB;
                    num = 5;
                    break;
                case 1:
                case 2:
                    key.inter = LWO::IT_HERM;
                    num = 5;
                    break;
                case 3:
                    key.inter = LWO::IT_LINE;
                    num = 0;
                    break;
                case 4:
                    key.inter = LWO::IT_STEP;
                    num = 0;
                    break;
                case 5:
                    key.inter = LWO::IT_BEZ2;
                    num = 4;
                    break;
                default:
                    ASSIMP_LOG_ERROR("LWS: Unknown span type");
            }
            for (unsigned int i = 0; i < num;++i) {
                SkipSpaces(&c);
                c = fast_atoreal_move<float>(c,key.params[i]);
            }
        }
        else if ((*it).tokens[0] == "Behaviors") {
            SkipSpaces(&c);
            fill.pre = (LWO::PrePostBehaviour) strtoul10(c,&c);
            SkipSpaces(&c);
            fill.post = (LWO::PrePostBehaviour) strtoul10(c,&c);
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Read animation channels in the old LightWave animation format
void LWSImporter::ReadEnvelope_Old(
    std::list< LWS::Element >::const_iterator& it,
    const std::list< LWS::Element >::const_iterator& end,
    LWS::NodeDesc& nodes,
    unsigned int /*version*/)
{
    unsigned int num,sub_num;
    if (++it == end)goto unexpected_end;

    num = strtoul10((*it).tokens[0].c_str());
    for (unsigned int i = 0; i < num; ++i) {

        nodes.channels.push_back(LWO::Envelope());
        LWO::Envelope& envl = nodes.channels.back();

        envl.index = i;
        envl.type  = (LWO::EnvelopeType)(i+1);

        if (++it == end)goto unexpected_end;
        sub_num = strtoul10((*it).tokens[0].c_str());

        for (unsigned int n = 0; n < sub_num;++n) {

            if (++it == end)goto unexpected_end;

            // parse value and time, skip the rest for the moment.
            LWO::Key key;
            const char* c = fast_atoreal_move<float>((*it).tokens[0].c_str(),key.value);
            SkipSpaces(&c);
            float f;
            fast_atoreal_move<float>((*it).tokens[0].c_str(),f);
            key.time = f;

            envl.keys.push_back(key);
        }
    }
    return;

unexpected_end:
    ASSIMP_LOG_ERROR("LWS: Encountered unexpected end of file while parsing object motion");
}

// ------------------------------------------------------------------------------------------------
// Setup a nice name for a node
void LWSImporter::SetupNodeName(aiNode* nd, LWS::NodeDesc& src)
{
    const unsigned int combined = src.number | ((unsigned int)src.type) << 28u;

    // the name depends on the type. We break LWS's strange naming convention
    // and return human-readable, but still machine-parsable and unique, strings.
    if (src.type == LWS::NodeDesc::OBJECT)  {

        if (src.path.length()) {
            std::string::size_type s = src.path.find_last_of("\\/");
            if (s == std::string::npos)
                s = 0;
            else ++s;
            std::string::size_type t = src.path.substr(s).find_last_of(".");

            nd->mName.length = ::ai_snprintf(nd->mName.data, MAXLEN, "%s_(%08X)",src.path.substr(s).substr(0,t).c_str(),combined);
            return;
        }
    }
    nd->mName.length = ::ai_snprintf(nd->mName.data, MAXLEN, "%s_(%08X)",src.name,combined);
}

// ------------------------------------------------------------------------------------------------
// Recursively build the scenegraph
void LWSImporter::BuildGraph(aiNode* nd, LWS::NodeDesc& src, std::vector<AttachmentInfo>& attach,
    BatchLoader& batch,
    aiCamera**& camOut,
    aiLight**& lightOut,
    std::vector<aiNodeAnim*>& animOut)
{
    // Setup a very cryptic name for the node, we want the user to be happy
    SetupNodeName(nd,src);
    aiNode* ndAnim = nd;

    // If the node is an object
    if (src.type == LWS::NodeDesc::OBJECT) {

        // If the object is from an external file, get it
        aiScene* obj = NULL;
        if (src.path.length() ) {
            obj = batch.GetImport(src.id);
            if (!obj) {
                ASSIMP_LOG_ERROR("LWS: Failed to read external file " + src.path);
            }
            else {
                if (obj->mRootNode->mNumChildren == 1) {

                    //If the pivot is not set for this layer, get it from the external object
                    if (!src.isPivotSet) {
                        src.pivotPos.x = +obj->mRootNode->mTransformation.a4;
                        src.pivotPos.y = +obj->mRootNode->mTransformation.b4;
                        src.pivotPos.z = -obj->mRootNode->mTransformation.c4; //The sign is the RH to LH back conversion
                    }

                    //Remove first node from obj (the old pivot), reset transform of second node (the mesh node)
                    aiNode* newRootNode = obj->mRootNode->mChildren[0];
                    obj->mRootNode->mChildren[0] = NULL;
                    delete obj->mRootNode;

                    obj->mRootNode = newRootNode;
                    obj->mRootNode->mTransformation.a4 = 0.0;
                    obj->mRootNode->mTransformation.b4 = 0.0;
                    obj->mRootNode->mTransformation.c4 = 0.0;
                }
            }
        }

        //Setup the pivot node (also the animation node), the one we received
        nd->mName = std::string("Pivot:") + nd->mName.data;
        ndAnim = nd;

        //Add the attachment node to it
        nd->mNumChildren = 1;
        nd->mChildren = new aiNode*[1];
        nd->mChildren[0] = new aiNode();
        nd->mChildren[0]->mParent = nd;
        nd->mChildren[0]->mTransformation.a4 = -src.pivotPos.x;
        nd->mChildren[0]->mTransformation.b4 = -src.pivotPos.y;
        nd->mChildren[0]->mTransformation.c4 = -src.pivotPos.z;
        SetupNodeName(nd->mChildren[0], src);

        //Update the attachment node
        nd = nd->mChildren[0];

        //Push attachment, if the object came from an external file
        if (obj) {
            attach.push_back(AttachmentInfo(obj,nd));
        }
    }

    // If object is a light source - setup a corresponding ai structure
    else if (src.type == LWS::NodeDesc::LIGHT) {
        aiLight* lit = *lightOut++ = new aiLight();

        // compute final light color
        lit->mColorDiffuse = lit->mColorSpecular = src.lightColor*src.lightIntensity;

        // name to attach light to node -> unique due to LWs indexing system
        lit->mName = nd->mName;

        // determine light type and setup additional members
        if (src.lightType == 2) { /* spot light */

            lit->mType = aiLightSource_SPOT;
            lit->mAngleInnerCone = (float)AI_DEG_TO_RAD( src.lightConeAngle );
            lit->mAngleOuterCone = lit->mAngleInnerCone+(float)AI_DEG_TO_RAD( src.lightEdgeAngle );

        }
        else if (src.lightType == 1) { /* directional light source */
            lit->mType = aiLightSource_DIRECTIONAL;
        }
        else lit->mType = aiLightSource_POINT;

        // fixme: no proper handling of light falloffs yet
        if (src.lightFalloffType == 1)
            lit->mAttenuationConstant = 1.f;
        else if (src.lightFalloffType == 1)
            lit->mAttenuationLinear = 1.f;
        else
            lit->mAttenuationQuadratic = 1.f;
    }

    // If object is a camera - setup a corresponding ai structure
    else if (src.type == LWS::NodeDesc::CAMERA) {
        aiCamera* cam = *camOut++ = new aiCamera();

        // name to attach cam to node -> unique due to LWs indexing system
        cam->mName = nd->mName;
    }

    // Get the node transformation from the LWO key
    LWO::AnimResolver resolver(src.channels,fps);
    resolver.ExtractBindPose(ndAnim->mTransformation);

    // .. and construct animation channels
    aiNodeAnim* anim = NULL;

    if (first != last) {
        resolver.SetAnimationRange(first,last);
        resolver.ExtractAnimChannel(&anim,AI_LWO_ANIM_FLAG_SAMPLE_ANIMS|AI_LWO_ANIM_FLAG_START_AT_ZERO);
        if (anim) {
            anim->mNodeName = ndAnim->mName;
            animOut.push_back(anim);
        }
    }

    // Add children
    if (!src.children.empty()) {
        nd->mChildren = new aiNode*[src.children.size()];
        for (std::list<LWS::NodeDesc*>::iterator it = src.children.begin(); it != src.children.end(); ++it) {
            aiNode* ndd = nd->mChildren[nd->mNumChildren++] = new aiNode();
            ndd->mParent = nd;

            BuildGraph(ndd,**it,attach,batch,camOut,lightOut,animOut);
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Determine the exact location of a LWO file
std::string LWSImporter::FindLWOFile(const std::string& in)
{
    // insert missing directory separator if necessary
    std::string tmp;
    if (in.length() > 3 && in[1] == ':'&& in[2] != '\\' && in[2] != '/')
    {
        tmp = in[0] + (std::string(":\\") + in.substr(2));
    }
    else tmp = in;

    if (io->Exists(tmp)) {
        return in;
    }

    // file is not accessible for us ... maybe it's packed by
    // LightWave's 'Package Scene' command?

    // Relevant for us are the following two directories:
    // <folder>\Objects\<hh>\<*>.lwo
    // <folder>\Scenes\<hh>\<*>.lws
    // where <hh> is optional.

    std::string test = std::string("..") + (io->getOsSeparator() + tmp);
    if (io->Exists(test)) {
        return test;
    }

    test = std::string("..") + (io->getOsSeparator() + test);
    if (io->Exists(test)) {
        return test;
    }


    // return original path, maybe the IOsystem knows better
    return tmp;
}

// ------------------------------------------------------------------------------------------------
// Read file into given scene data structure
void LWSImporter::InternReadFile( const std::string& pFile, aiScene* pScene,
    IOSystem* pIOHandler)
{
    io = pIOHandler;
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile, "rb"));

    // Check whether we can read from the file
    if( file.get() == NULL) {
        throw DeadlyImportError( "Failed to open LWS file " + pFile + ".");
    }

    // Allocate storage and copy the contents of the file to a memory buffer
    std::vector< char > mBuffer;
    TextFileToBuffer(file.get(),mBuffer);

    // Parse the file structure
    LWS::Element root; const char* dummy = &mBuffer[0];
    root.Parse(dummy);

    // Construct a Batchimporter to read more files recursively
    BatchLoader batch(pIOHandler);
//  batch.SetBasePath(pFile);

    // Construct an array to receive the flat output graph
    std::list<LWS::NodeDesc> nodes;

    unsigned int cur_light = 0, cur_camera = 0, cur_object = 0;
    unsigned int num_light = 0, num_camera = 0, num_object = 0;

    // check magic identifier, 'LWSC'
    bool motion_file = false;
    std::list< LWS::Element >::const_iterator it = root.children.begin();

    if ((*it).tokens[0] == "LWMO")
        motion_file = true;

    if ((*it).tokens[0] != "LWSC" && !motion_file)
        throw DeadlyImportError("LWS: Not a LightWave scene, magic tag LWSC not found");

    // get file format version and print to log
    ++it;
    unsigned int version = strtoul10((*it).tokens[0].c_str());
    ASSIMP_LOG_INFO("LWS file format version is " + (*it).tokens[0]);
    first = 0.;
    last  = 60.;
    fps   = 25.; /* seems to be a good default frame rate */

    // Now read all elements in a very straghtforward manner
    for (; it != root.children.end(); ++it) {
        const char* c = (*it).tokens[1].c_str();

        // 'FirstFrame': begin of animation slice
        if ((*it).tokens[0] == "FirstFrame") {
            if (150392. != first           /* see SetupProperties() */)
                first = strtoul10(c,&c)-1.; /* we're zero-based */
        }

        // 'LastFrame': end of animation slice
        else if ((*it).tokens[0] == "LastFrame") {
            if (150392. != last      /* see SetupProperties() */)
                last = strtoul10(c,&c)-1.; /* we're zero-based */
        }

        // 'FramesPerSecond': frames per second
        else if ((*it).tokens[0] == "FramesPerSecond") {
            fps = strtoul10(c,&c);
        }

        // 'LoadObjectLayer': load a layer of a specific LWO file
        else if ((*it).tokens[0] == "LoadObjectLayer") {

            // get layer index
            const int layer = strtoul10(c,&c);

            // setup the layer to be loaded
            BatchLoader::PropertyMap props;
            SetGenericProperty(props.ints,AI_CONFIG_IMPORT_LWO_ONE_LAYER_ONLY,layer);

            // add node to list
            LWS::NodeDesc d;
            d.type = LWS::NodeDesc::OBJECT;
            if (version >= 4) { // handle LWSC 4 explicit ID
                SkipSpaces(&c);
                d.number = strtoul16(c,&c) & AI_LWS_MASK;
            }
            else d.number = cur_object++;

            // and add the file to the import list
            SkipSpaces(&c);
            std::string path = FindLWOFile( c );
            d.path = path;
            d.id = batch.AddLoadRequest(path,0,&props);

            nodes.push_back(d);
            num_object++;
        }
        // 'LoadObject': load a LWO file into the scenegraph
        else if ((*it).tokens[0] == "LoadObject") {

            // add node to list
            LWS::NodeDesc d;
            d.type = LWS::NodeDesc::OBJECT;

            if (version >= 4) { // handle LWSC 4 explicit ID
                d.number = strtoul16(c,&c) & AI_LWS_MASK;
                SkipSpaces(&c);
            }
            else d.number = cur_object++;
            std::string path = FindLWOFile( c );
            d.id = batch.AddLoadRequest(path,0,NULL);

            d.path = path;
            nodes.push_back(d);
            num_object++;
        }
        // 'AddNullObject': add a dummy node to the hierarchy
        else if ((*it).tokens[0] == "AddNullObject") {

            // add node to list
            LWS::NodeDesc d;
            d.type = LWS::NodeDesc::OBJECT;
            if (version >= 4) { // handle LWSC 4 explicit ID
                d.number = strtoul16(c,&c) & AI_LWS_MASK;
                SkipSpaces(&c);
            }
            else d.number = cur_object++;
            d.name = c;
            nodes.push_back(d);

            num_object++;
        }
        // 'NumChannels': Number of envelope channels assigned to last layer
        else if ((*it).tokens[0] == "NumChannels") {
            // ignore for now
        }
        // 'Channel': preceedes any envelope description
        else if ((*it).tokens[0] == "Channel") {
            if (nodes.empty()) {
                if (motion_file) {

                    // LightWave motion file. Add dummy node
                    LWS::NodeDesc d;
                    d.type = LWS::NodeDesc::OBJECT;
                    d.name = c;
                    d.number = cur_object++;
                    nodes.push_back(d);
                }
                ASSIMP_LOG_ERROR("LWS: Unexpected keyword: \'Channel\'");
            }

            // important: index of channel
            nodes.back().channels.push_back(LWO::Envelope());
            LWO::Envelope& env = nodes.back().channels.back();

            env.index = strtoul10(c);

            // currently we can just interpret the standard channels 0...9
            // (hack) assume that index-i yields the binary channel type from LWO
            env.type = (LWO::EnvelopeType)(env.index+1);

        }
        // 'Envelope': a single animation channel
        else if ((*it).tokens[0] == "Envelope") {
            if (nodes.empty() || nodes.back().channels.empty())
                ASSIMP_LOG_ERROR("LWS: Unexpected keyword: \'Envelope\'");
            else {
                ReadEnvelope((*it),nodes.back().channels.back());
            }
        }
        // 'ObjectMotion': animation information for older lightwave formats
        else if (version < 3  && ((*it).tokens[0] == "ObjectMotion" ||
            (*it).tokens[0] == "CameraMotion" ||
            (*it).tokens[0] == "LightMotion")) {

            if (nodes.empty())
                ASSIMP_LOG_ERROR("LWS: Unexpected keyword: \'<Light|Object|Camera>Motion\'");
            else {
                ReadEnvelope_Old(it,root.children.end(),nodes.back(),version);
            }
        }
        // 'Pre/PostBehavior': pre/post animation behaviour for LWSC 2
        else if (version == 2 && (*it).tokens[0] == "Pre/PostBehavior") {
            if (nodes.empty())
                ASSIMP_LOG_ERROR("LWS: Unexpected keyword: \'Pre/PostBehavior'");
            else {
                for (std::list<LWO::Envelope>::iterator it = nodes.back().channels.begin(); it != nodes.back().channels.end(); ++it) {
                    // two ints per envelope
                    LWO::Envelope& env = *it;
                    env.pre  = (LWO::PrePostBehaviour) strtoul10(c,&c); SkipSpaces(&c);
                    env.post = (LWO::PrePostBehaviour) strtoul10(c,&c); SkipSpaces(&c);
                }
            }
        }
        // 'ParentItem': specifies the parent of the current element
        else if ((*it).tokens[0] == "ParentItem") {
            if (nodes.empty())
                ASSIMP_LOG_ERROR("LWS: Unexpected keyword: \'ParentItem\'");

            else nodes.back().parent = strtoul16(c,&c);
        }
        // 'ParentObject': deprecated one for older formats
        else if (version < 3 && (*it).tokens[0] == "ParentObject") {
            if (nodes.empty())
                ASSIMP_LOG_ERROR("LWS: Unexpected keyword: \'ParentObject\'");

            else {
                nodes.back().parent = strtoul10(c,&c) | (1u << 28u);
            }
        }
        // 'AddCamera': add a camera to the scenegraph
        else if ((*it).tokens[0] == "AddCamera") {

            // add node to list
            LWS::NodeDesc d;
            d.type = LWS::NodeDesc::CAMERA;

            if (version >= 4) { // handle LWSC 4 explicit ID
                d.number = strtoul16(c,&c) & AI_LWS_MASK;
            }
            else d.number = cur_camera++;
            nodes.push_back(d);

            num_camera++;
        }
        // 'CameraName': set name of currently active camera
        else if ((*it).tokens[0] == "CameraName") {
            if (nodes.empty() || nodes.back().type != LWS::NodeDesc::CAMERA)
                ASSIMP_LOG_ERROR("LWS: Unexpected keyword: \'CameraName\'");

            else nodes.back().name = c;
        }
        // 'AddLight': add a light to the scenegraph
        else if ((*it).tokens[0] == "AddLight") {

            // add node to list
            LWS::NodeDesc d;
            d.type = LWS::NodeDesc::LIGHT;

            if (version >= 4) { // handle LWSC 4 explicit ID
                d.number = strtoul16(c,&c) & AI_LWS_MASK;
            }
            else d.number = cur_light++;
            nodes.push_back(d);

            num_light++;
        }
        // 'LightName': set name of currently active light
        else if ((*it).tokens[0] == "LightName") {
            if (nodes.empty() || nodes.back().type != LWS::NodeDesc::LIGHT)
                ASSIMP_LOG_ERROR("LWS: Unexpected keyword: \'LightName\'");

            else nodes.back().name = c;
        }
        // 'LightIntensity': set intensity of currently active light
        else if ((*it).tokens[0] == "LightIntensity" || (*it).tokens[0] == "LgtIntensity" ) {
            if (nodes.empty() || nodes.back().type != LWS::NodeDesc::LIGHT)
                ASSIMP_LOG_ERROR("LWS: Unexpected keyword: \'LightIntensity\'");

            else fast_atoreal_move<float>(c, nodes.back().lightIntensity );

        }
        // 'LightType': set type of currently active light
        else if ((*it).tokens[0] == "LightType") {
            if (nodes.empty() || nodes.back().type != LWS::NodeDesc::LIGHT)
                ASSIMP_LOG_ERROR("LWS: Unexpected keyword: \'LightType\'");

            else nodes.back().lightType = strtoul10(c);

        }
        // 'LightFalloffType': set falloff type of currently active light
        else if ((*it).tokens[0] == "LightFalloffType") {
            if (nodes.empty() || nodes.back().type != LWS::NodeDesc::LIGHT)
                ASSIMP_LOG_ERROR("LWS: Unexpected keyword: \'LightFalloffType\'");

            else nodes.back().lightFalloffType = strtoul10(c);

        }
        // 'LightConeAngle': set cone angle of currently active light
        else if ((*it).tokens[0] == "LightConeAngle") {
            if (nodes.empty() || nodes.back().type != LWS::NodeDesc::LIGHT)
                ASSIMP_LOG_ERROR("LWS: Unexpected keyword: \'LightConeAngle\'");

            else nodes.back().lightConeAngle = fast_atof(c);

        }
        // 'LightEdgeAngle': set area where we're smoothing from min to max intensity
        else if ((*it).tokens[0] == "LightEdgeAngle") {
            if (nodes.empty() || nodes.back().type != LWS::NodeDesc::LIGHT)
                ASSIMP_LOG_ERROR("LWS: Unexpected keyword: \'LightEdgeAngle\'");

            else nodes.back().lightEdgeAngle = fast_atof(c);

        }
        // 'LightColor': set color of currently active light
        else if ((*it).tokens[0] == "LightColor") {
            if (nodes.empty() || nodes.back().type != LWS::NodeDesc::LIGHT)
                ASSIMP_LOG_ERROR("LWS: Unexpected keyword: \'LightColor\'");

            else {
                c = fast_atoreal_move<float>(c, (float&) nodes.back().lightColor.r );
                SkipSpaces(&c);
                c = fast_atoreal_move<float>(c, (float&) nodes.back().lightColor.g );
                SkipSpaces(&c);
                c = fast_atoreal_move<float>(c, (float&) nodes.back().lightColor.b );
            }
        }

        // 'PivotPosition': position of local transformation origin
        else if ((*it).tokens[0] == "PivotPosition" || (*it).tokens[0] == "PivotPoint") {
            if (nodes.empty())
                ASSIMP_LOG_ERROR("LWS: Unexpected keyword: \'PivotPosition\'");
            else {
                c = fast_atoreal_move<float>(c, (float&) nodes.back().pivotPos.x );
                SkipSpaces(&c);
                c = fast_atoreal_move<float>(c, (float&) nodes.back().pivotPos.y );
                SkipSpaces(&c);
                c = fast_atoreal_move<float>(c, (float&) nodes.back().pivotPos.z );
                // Mark pivotPos as set
                nodes.back().isPivotSet = true;
            }
        }
    }

    // resolve parenting
    for (std::list<LWS::NodeDesc>::iterator it = nodes.begin(); it != nodes.end(); ++it) {

        // check whether there is another node which calls us a parent
        for (std::list<LWS::NodeDesc>::iterator dit = nodes.begin(); dit != nodes.end(); ++dit) {
            if (dit != it && *it == (*dit).parent) {
                if ((*dit).parent_resolved) {
                    // fixme: it's still possible to produce an overflow due to cross references ..
                    ASSIMP_LOG_ERROR("LWS: Found cross reference in scene-graph");
                    continue;
                }

                (*it).children.push_back(&*dit);
                (*dit).parent_resolved = &*it;
            }
        }
    }

    // find out how many nodes have no parent yet
    unsigned int no_parent = 0;
    for (std::list<LWS::NodeDesc>::iterator it = nodes.begin(); it != nodes.end(); ++it) {
        if (!(*it).parent_resolved)
            ++ no_parent;
    }
    if (!no_parent)
        throw DeadlyImportError("LWS: Unable to find scene root node");


    // Load all subsequent files
    batch.LoadAll();

    // and build the final output graph by attaching the loaded external
    // files to ourselves. first build a master graph
    aiScene* master = new aiScene();
    aiNode* nd = master->mRootNode = new aiNode();

    // allocate storage for cameras&lights
    if (num_camera) {
        master->mCameras = new aiCamera*[master->mNumCameras = num_camera];
    }
    aiCamera** cams = master->mCameras;
    if (num_light) {
        master->mLights = new aiLight*[master->mNumLights = num_light];
    }
    aiLight** lights = master->mLights;

    std::vector<AttachmentInfo> attach;
    std::vector<aiNodeAnim*> anims;

    nd->mName.Set("<LWSRoot>");
    nd->mChildren = new aiNode*[no_parent];
    for (std::list<LWS::NodeDesc>::iterator it = nodes.begin(); it != nodes.end(); ++it) {
        if (!(*it).parent_resolved) {
            aiNode* ro = nd->mChildren[ nd->mNumChildren++ ] = new aiNode();
            ro->mParent = nd;

            // ... and build the scene graph. If we encounter object nodes,
            // add then to our attachment table.
            BuildGraph(ro,*it, attach, batch, cams, lights, anims);
        }
    }

    // create a master animation channel for us
    if (anims.size()) {
        master->mAnimations = new aiAnimation*[master->mNumAnimations = 1];
        aiAnimation* anim = master->mAnimations[0] = new aiAnimation();
        anim->mName.Set("LWSMasterAnim");

        // LWS uses seconds as time units, but we convert to frames
        anim->mTicksPerSecond = fps;
        anim->mDuration = last-(first-1); /* fixme ... zero or one-based?*/

        anim->mChannels = new aiNodeAnim*[anim->mNumChannels = static_cast<unsigned int>(anims.size())];
        std::copy(anims.begin(),anims.end(),anim->mChannels);
    }

    // convert the master scene to RH
    MakeLeftHandedProcess monster_cheat;
    monster_cheat.Execute(master);

    // .. ccw
    FlipWindingOrderProcess flipper;
    flipper.Execute(master);

    // OK ... finally build the output graph
    SceneCombiner::MergeScenes(&pScene,master,attach,
        AI_INT_MERGE_SCENE_GEN_UNIQUE_NAMES    | (!configSpeedFlag ? (
        AI_INT_MERGE_SCENE_GEN_UNIQUE_NAMES_IF_NECESSARY | AI_INT_MERGE_SCENE_GEN_UNIQUE_MATNAMES) : 0));

    // Check flags
    if (!pScene->mNumMeshes || !pScene->mNumMaterials) {
        pScene->mFlags |= AI_SCENE_FLAGS_INCOMPLETE;

        if (pScene->mNumAnimations && !noSkeletonMesh) {
            // construct skeleton mesh
            SkeletonMeshBuilder builder(pScene);
        }
    }

}

#endif // !! ASSIMP_BUILD_NO_LWS_IMPORTER
