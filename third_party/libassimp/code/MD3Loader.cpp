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

/** @file MD3Loader.cpp
 *  @brief Implementation of the MD3 importer class
 *
 *  Sources:
 *     http://www.gamers.org/dEngine/quake3/UQ3S
 *     http://linux.ucla.edu/~phaethon/q3/formats/md3format.html
 *     http://www.heppler.com/shader/shader/
 */


#ifndef ASSIMP_BUILD_NO_MD3_IMPORTER

#include "MD3Loader.h"
#include <assimp/SceneCombiner.h>
#include <assimp/GenericProperty.h>
#include <assimp/RemoveComments.h>
#include <assimp/ParsingUtils.h>
#include "Importer.h"
#include <assimp/DefaultLogger.hpp>
#include <memory>
#include <assimp/IOSystem.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/importerdesc.h>
#include <cctype>

using namespace Assimp;

static const aiImporterDesc desc = {
    "Quake III Mesh Importer",
    "",
    "",
    "",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "md3"
};

// ------------------------------------------------------------------------------------------------
// Convert a Q3 shader blend function to the appropriate enum value
Q3Shader::BlendFunc StringToBlendFunc(const std::string& m)
{
    if (m == "GL_ONE") {
        return Q3Shader::BLEND_GL_ONE;
    }
    if (m == "GL_ZERO") {
        return Q3Shader::BLEND_GL_ZERO;
    }
    if (m == "GL_SRC_ALPHA") {
        return Q3Shader::BLEND_GL_SRC_ALPHA;
    }
    if (m == "GL_ONE_MINUS_SRC_ALPHA") {
        return Q3Shader::BLEND_GL_ONE_MINUS_SRC_ALPHA;
    }
    if (m == "GL_ONE_MINUS_DST_COLOR") {
        return Q3Shader::BLEND_GL_ONE_MINUS_DST_COLOR;
    }
    ASSIMP_LOG_ERROR("Q3Shader: Unknown blend function: " + m);
    return Q3Shader::BLEND_NONE;
}

// ------------------------------------------------------------------------------------------------
// Load a Quake 3 shader
bool Q3Shader::LoadShader(ShaderData& fill, const std::string& pFile,IOSystem* io)
{
    std::unique_ptr<IOStream> file( io->Open( pFile, "rt"));
    if (!file.get())
        return false; // if we can't access the file, don't worry and return

    ASSIMP_LOG_INFO_F("Loading Quake3 shader file ", pFile);

    // read file in memory
    const size_t s = file->FileSize();
    std::vector<char> _buff(s+1);
    file->Read(&_buff[0],s,1);
    _buff[s] = 0;

    // remove comments from it (C++ style)
    CommentRemover::RemoveLineComments("//",&_buff[0]);
    const char* buff = &_buff[0];

    Q3Shader::ShaderDataBlock* curData = NULL;
    Q3Shader::ShaderMapBlock*  curMap  = NULL;

    // read line per line
    for (;SkipSpacesAndLineEnd(&buff);SkipLine(&buff)) {

        if (*buff == '{') {
            ++buff;

            // append to last section, if any
            if (!curData) {
                ASSIMP_LOG_ERROR("Q3Shader: Unexpected shader section token \'{\'");
                return true; // still no failure, the file is there
            }

            // read this data section
            for (;SkipSpacesAndLineEnd(&buff);SkipLine(&buff)) {
                if (*buff == '{') {
                    ++buff;
                    // add new map section
                    curData->maps.push_back(Q3Shader::ShaderMapBlock());
                    curMap = &curData->maps.back();

                    for (;SkipSpacesAndLineEnd(&buff);SkipLine(&buff)) {
                        // 'map' - Specifies texture file name
                        if (TokenMatchI(buff,"map",3) || TokenMatchI(buff,"clampmap",8)) {
                            curMap->name = GetNextToken(buff);
                        }
                        // 'blendfunc' - Alpha blending mode
                        else if (TokenMatchI(buff,"blendfunc",9)) {
                            const std::string blend_src = GetNextToken(buff);
                            if (blend_src == "add") {
                                curMap->blend_src  = Q3Shader::BLEND_GL_ONE;
                                curMap->blend_dest = Q3Shader::BLEND_GL_ONE;
                            }
                            else if (blend_src == "filter") {
                                curMap->blend_src  = Q3Shader::BLEND_GL_DST_COLOR;
                                curMap->blend_dest = Q3Shader::BLEND_GL_ZERO;
                            }
                            else if (blend_src == "blend") {
                                curMap->blend_src  = Q3Shader::BLEND_GL_SRC_ALPHA;
                                curMap->blend_dest = Q3Shader::BLEND_GL_ONE_MINUS_SRC_ALPHA;
                            }
                            else {
                                curMap->blend_src  = StringToBlendFunc(blend_src);
                                curMap->blend_dest = StringToBlendFunc(GetNextToken(buff));
                            }
                        }
                        // 'alphafunc' - Alpha testing mode
                        else if (TokenMatchI(buff,"alphafunc",9)) {
                            const std::string at = GetNextToken(buff);
                            if (at == "GT0") {
                                curMap->alpha_test = Q3Shader::AT_GT0;
                            }
                            else if (at == "LT128") {
                                curMap->alpha_test = Q3Shader::AT_LT128;
                            }
                            else if (at == "GE128") {
                                curMap->alpha_test = Q3Shader::AT_GE128;
                            }
                        }
                        else if (*buff == '}') {
                            ++buff;
                            // close this map section
                            curMap = NULL;
                            break;
                        }
                    }

                }
                else if (*buff == '}') {
                    ++buff;
                    curData = NULL;
                    break;
                }

                // 'cull' specifies culling behaviour for the model
                else if (TokenMatchI(buff,"cull",4)) {
                    SkipSpaces(&buff);
                    if (!ASSIMP_strincmp(buff,"back",4)) {
                        curData->cull = Q3Shader::CULL_CCW;
                    } else if (!ASSIMP_strincmp(buff,"front",5)) {
                        curData->cull = Q3Shader::CULL_CW;
                    } else if (!ASSIMP_strincmp(buff,"none",4) || !ASSIMP_strincmp(buff,"disable",7)) {
                        curData->cull = Q3Shader::CULL_NONE;
                    } else {
                        ASSIMP_LOG_ERROR("Q3Shader: Unrecognized cull mode");
                    }
                }
            }
        } else {
            // add new section
            fill.blocks.push_back(Q3Shader::ShaderDataBlock());
            curData = &fill.blocks.back();

            // get the name of this section
            curData->name = GetNextToken(buff);
        }
    }
    return true;
}

// ------------------------------------------------------------------------------------------------
// Load a Quake 3 skin
bool Q3Shader::LoadSkin(SkinData& fill, const std::string& pFile,IOSystem* io)
{
    std::unique_ptr<IOStream> file( io->Open( pFile, "rt"));
    if (!file.get())
        return false; // if we can't access the file, don't worry and return

    ASSIMP_LOG_INFO("Loading Quake3 skin file " + pFile);

    // read file in memory
    const size_t s = file->FileSize();
    std::vector<char> _buff(s+1);const char* buff = &_buff[0];
    file->Read(&_buff[0],s,1);
    _buff[s] = 0;

    // remove commas
    std::replace(_buff.begin(),_buff.end(),',',' ');

    // read token by token and fill output table
    for (;*buff;) {
        SkipSpacesAndLineEnd(&buff);

        // get first identifier
        std::string ss = GetNextToken(buff);

        // ignore tokens starting with tag_
        if (!::strncmp(&ss[0],"tag_",std::min((size_t)4, ss.length())))
            continue;

        fill.textures.push_back(SkinData::TextureEntry());
        SkinData::TextureEntry& s = fill.textures.back();

        s.first  = ss;
        s.second = GetNextToken(buff);
    }
    return true;
}

// ------------------------------------------------------------------------------------------------
// Convert Q3Shader to material
void Q3Shader::ConvertShaderToMaterial(aiMaterial* out, const ShaderDataBlock& shader)
{
    ai_assert(NULL != out);

    /*  IMPORTANT: This is not a real conversion. Actually we're just guessing and
     *  hacking around to build an aiMaterial that looks nearly equal to the
     *  original Quake 3 shader. We're missing some important features like
     *  animatable material properties in our material system, but at least
     *  multiple textures should be handled correctly.
     */

    // Two-sided material?
    if (shader.cull == Q3Shader::CULL_NONE) {
        const int twosided = 1;
        out->AddProperty(&twosided,1,AI_MATKEY_TWOSIDED);
    }

    unsigned int cur_emissive = 0, cur_diffuse = 0, cur_lm =0;

    // Iterate through all textures
    for (std::list< Q3Shader::ShaderMapBlock >::const_iterator it = shader.maps.begin(); it != shader.maps.end();++it) {

        // CONVERSION BEHAVIOUR:
        //
        //
        // If the texture is additive
        //  - if it is the first texture, assume additive blending for the whole material
        //  - otherwise register it as emissive texture.
        //
        // If the texture is using standard blend (or if the blend mode is unknown)
        //  - if first texture: assume default blending for material
        //  - in any case: set it as diffuse texture
        //
        // If the texture is using 'filter' blending
        //  - take as lightmap
        //
        // Textures with alpha funcs
        //  - aiTextureFlags_UseAlpha is set (otherwise aiTextureFlags_NoAlpha is explicitly set)
        aiString s((*it).name);
        aiTextureType type; unsigned int index;

        if ((*it).blend_src == Q3Shader::BLEND_GL_ONE && (*it).blend_dest == Q3Shader::BLEND_GL_ONE) {
            if (it == shader.maps.begin()) {
                const int additive = aiBlendMode_Additive;
                out->AddProperty(&additive,1,AI_MATKEY_BLEND_FUNC);

                index = cur_diffuse++;
                type  = aiTextureType_DIFFUSE;
            }
            else {
                index = cur_emissive++;
                type  = aiTextureType_EMISSIVE;
            }
        }
        else if ((*it).blend_src == Q3Shader::BLEND_GL_DST_COLOR && (*it).blend_dest == Q3Shader::BLEND_GL_ZERO) {
            index = cur_lm++;
            type  = aiTextureType_LIGHTMAP;
        }
        else {
            const int blend = aiBlendMode_Default;
            out->AddProperty(&blend,1,AI_MATKEY_BLEND_FUNC);

            index = cur_diffuse++;
            type  = aiTextureType_DIFFUSE;
        }

        // setup texture
        out->AddProperty(&s,AI_MATKEY_TEXTURE(type,index));

        // setup texture flags
        const int use_alpha = ((*it).alpha_test != Q3Shader::AT_NONE ? aiTextureFlags_UseAlpha : aiTextureFlags_IgnoreAlpha);
        out->AddProperty(&use_alpha,1,AI_MATKEY_TEXFLAGS(type,index));
    }
    // If at least one emissive texture was set, set the emissive base color to 1 to ensure
    // the texture is actually displayed.
    if (0 != cur_emissive) {
        aiColor3D one(1.f,1.f,1.f);
        out->AddProperty(&one,1,AI_MATKEY_COLOR_EMISSIVE);
    }
}

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
MD3Importer::MD3Importer()
    : configFrameID  (0)
    , configHandleMP (true)
    , configSpeedFlag()
    , pcHeader()
    , mBuffer()
    , fileSize()
    , mScene()
    , mIOHandler()
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
MD3Importer::~MD3Importer()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool MD3Importer::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    const std::string extension = GetExtension(pFile);
    if (extension == "md3")
        return true;

    // if check for extension is not enough, check for the magic tokens
    if (!extension.length() || checkSig) {
        uint32_t tokens[1];
        tokens[0] = AI_MD3_MAGIC_NUMBER_LE;
        return CheckMagicToken(pIOHandler,pFile,tokens,1);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
void MD3Importer::ValidateHeaderOffsets()
{
    // Check magic number
    if (pcHeader->IDENT != AI_MD3_MAGIC_NUMBER_BE &&
        pcHeader->IDENT != AI_MD3_MAGIC_NUMBER_LE)
            throw DeadlyImportError( "Invalid MD3 file: Magic bytes not found");

    // Check file format version
    if (pcHeader->VERSION > 15)
        ASSIMP_LOG_WARN( "Unsupported MD3 file version. Continuing happily ...");

    // Check some offset values whether they are valid
    if (!pcHeader->NUM_SURFACES)
        throw DeadlyImportError( "Invalid md3 file: NUM_SURFACES is 0");

    if (pcHeader->OFS_FRAMES >= fileSize || pcHeader->OFS_SURFACES >= fileSize ||
        pcHeader->OFS_EOF > fileSize) {
        throw DeadlyImportError("Invalid MD3 header: some offsets are outside the file");
    }

	if (pcHeader->NUM_SURFACES > AI_MAX_ALLOC(MD3::Surface)) {
        throw DeadlyImportError("Invalid MD3 header: too many surfaces, would overflow");
	}

    if (pcHeader->OFS_SURFACES + pcHeader->NUM_SURFACES * sizeof(MD3::Surface) >= fileSize) {
        throw DeadlyImportError("Invalid MD3 header: some surfaces are outside the file");
    }

    if (pcHeader->NUM_FRAMES <= configFrameID )
        throw DeadlyImportError("The requested frame is not existing the file");
}

// ------------------------------------------------------------------------------------------------
void MD3Importer::ValidateSurfaceHeaderOffsets(const MD3::Surface* pcSurf)
{
    // Calculate the relative offset of the surface
    const int32_t ofs = int32_t((const unsigned char*)pcSurf-this->mBuffer);

    // Check whether all data chunks are inside the valid range
    if (pcSurf->OFS_TRIANGLES + ofs + pcSurf->NUM_TRIANGLES * sizeof(MD3::Triangle) > fileSize  ||
        pcSurf->OFS_SHADERS + ofs + pcSurf->NUM_SHADER * sizeof(MD3::Shader) > fileSize         ||
        pcSurf->OFS_ST + ofs + pcSurf->NUM_VERTICES * sizeof(MD3::TexCoord) > fileSize          ||
        pcSurf->OFS_XYZNORMAL + ofs + pcSurf->NUM_VERTICES * sizeof(MD3::Vertex) > fileSize)    {

        throw DeadlyImportError("Invalid MD3 surface header: some offsets are outside the file");
    }

    // Check whether all requirements for Q3 files are met. We don't
    // care, but probably someone does.
    if (pcSurf->NUM_TRIANGLES > AI_MD3_MAX_TRIANGLES) {
        ASSIMP_LOG_WARN("MD3: Quake III triangle limit exceeded");
    }

    if (pcSurf->NUM_SHADER > AI_MD3_MAX_SHADERS) {
        ASSIMP_LOG_WARN("MD3: Quake III shader limit exceeded");
    }

    if (pcSurf->NUM_VERTICES > AI_MD3_MAX_VERTS) {
        ASSIMP_LOG_WARN("MD3: Quake III vertex limit exceeded");
    }

    if (pcSurf->NUM_FRAMES > AI_MD3_MAX_FRAMES) {
        ASSIMP_LOG_WARN("MD3: Quake III frame limit exceeded");
    }
}

// ------------------------------------------------------------------------------------------------
const aiImporterDesc* MD3Importer::GetInfo () const {
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Setup configuration properties
void MD3Importer::SetupProperties(const Importer* pImp)
{
    // The
    // AI_CONFIG_IMPORT_MD3_KEYFRAME option overrides the
    // AI_CONFIG_IMPORT_GLOBAL_KEYFRAME option.
    configFrameID = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_MD3_KEYFRAME,-1);
    if(static_cast<unsigned int>(-1) == configFrameID) {
        configFrameID = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_GLOBAL_KEYFRAME,0);
    }

    // AI_CONFIG_IMPORT_MD3_HANDLE_MULTIPART
    configHandleMP = (0 != pImp->GetPropertyInteger(AI_CONFIG_IMPORT_MD3_HANDLE_MULTIPART,1));

    // AI_CONFIG_IMPORT_MD3_SKIN_NAME
    configSkinFile = (pImp->GetPropertyString(AI_CONFIG_IMPORT_MD3_SKIN_NAME,"default"));

    // AI_CONFIG_IMPORT_MD3_SHADER_SRC
    configShaderFile = (pImp->GetPropertyString(AI_CONFIG_IMPORT_MD3_SHADER_SRC,""));

    // AI_CONFIG_FAVOUR_SPEED
    configSpeedFlag = (0 != pImp->GetPropertyInteger(AI_CONFIG_FAVOUR_SPEED,0));
}

// ------------------------------------------------------------------------------------------------
// Try to read the skin for a MD3 file
void MD3Importer::ReadSkin(Q3Shader::SkinData& fill) const
{
    // skip any postfixes (e.g. lower_1.md3)
    std::string::size_type s = filename.find_last_of('_');
    if (s == std::string::npos) {
        s = filename.find_last_of('.');
        if (s == std::string::npos) {
            s = filename.size();
        }
    }
    ai_assert(s != std::string::npos);

    const std::string skin_file = path + filename.substr(0,s) + "_" + configSkinFile + ".skin";
    Q3Shader::LoadSkin(fill,skin_file,mIOHandler);
}

// ------------------------------------------------------------------------------------------------
// Try to read the shader for a MD3 file
void MD3Importer::ReadShader(Q3Shader::ShaderData& fill) const
{
    // Determine Q3 model name from given path
    const std::string::size_type s = path.find_last_of("\\/",path.length()-2);
    const std::string model_file = path.substr(s+1,path.length()-(s+2));

    // If no specific dir or file is given, use our default search behaviour
    if (!configShaderFile.length()) {
        if(!Q3Shader::LoadShader(fill,path + "..\\..\\..\\scripts\\" + model_file + ".shader",mIOHandler)) {
            Q3Shader::LoadShader(fill,path + "..\\..\\..\\scripts\\" + filename + ".shader",mIOHandler);
        }
    }
    else {
        // If the given string specifies a file, load this file.
        // Otherwise it's a directory.
        const std::string::size_type st = configShaderFile.find_last_of('.');
        if (st == std::string::npos) {

            if(!Q3Shader::LoadShader(fill,configShaderFile + model_file + ".shader",mIOHandler)) {
                Q3Shader::LoadShader(fill,configShaderFile + filename + ".shader",mIOHandler);
            }
        }
        else {
            Q3Shader::LoadShader(fill,configShaderFile,mIOHandler);
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Tiny helper to remove a single node from its parent' list
void RemoveSingleNodeFromList(aiNode* nd)
{
    if (!nd || nd->mNumChildren || !nd->mParent)return;
    aiNode* par = nd->mParent;
    for (unsigned int i = 0; i < par->mNumChildren;++i) {
        if (par->mChildren[i] == nd) {
            --par->mNumChildren;
            for (;i < par->mNumChildren;++i) {
                par->mChildren[i] = par->mChildren[i+1];
            }
            delete nd;
            break;
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Read a multi-part Q3 player model
bool MD3Importer::ReadMultipartFile()
{
    // check whether the file name contains a common postfix, e.g lower_2.md3
    std::string::size_type s = filename.find_last_of('_'), t = filename.find_last_of('.');

    if (t == std::string::npos)
        t = filename.size();
    if (s == std::string::npos)
        s = t;

    const std::string mod_filename = filename.substr(0,s);
    const std::string suffix = filename.substr(s,t-s);

    if (mod_filename == "lower" || mod_filename == "upper" || mod_filename == "head"){
        const std::string lower = path + "lower" + suffix + ".md3";
        const std::string upper = path + "upper" + suffix + ".md3";
        const std::string head  = path + "head"  + suffix + ".md3";

        aiScene* scene_upper = NULL;
        aiScene* scene_lower = NULL;
        aiScene* scene_head = NULL;
        std::string failure;

        aiNode* tag_torso, *tag_head;
        std::vector<AttachmentInfo> attach;

        ASSIMP_LOG_INFO("Multi part MD3 player model: lower, upper and head parts are joined");

        // ensure we won't try to load ourselves recursively
        BatchLoader::PropertyMap props;
        SetGenericProperty( props.ints, AI_CONFIG_IMPORT_MD3_HANDLE_MULTIPART, 0);

        // now read these three files
        BatchLoader batch(mIOHandler);
        const unsigned int _lower = batch.AddLoadRequest(lower,0,&props);
        const unsigned int _upper = batch.AddLoadRequest(upper,0,&props);
        const unsigned int _head  = batch.AddLoadRequest(head,0,&props);
        batch.LoadAll();

        // now construct a dummy scene to place these three parts in
        aiScene* master   = new aiScene();
        aiNode* nd = master->mRootNode = new aiNode();
        nd->mName.Set("<MD3_Player>");

        // ... and get them. We need all of them.
        scene_lower = batch.GetImport(_lower);
        if (!scene_lower) {
            ASSIMP_LOG_ERROR("M3D: Failed to read multi part model, lower.md3 fails to load");
            failure = "lower";
            goto error_cleanup;
        }

        scene_upper = batch.GetImport(_upper);
        if (!scene_upper) {
            ASSIMP_LOG_ERROR("M3D: Failed to read multi part model, upper.md3 fails to load");
            failure = "upper";
            goto error_cleanup;
        }

        scene_head  = batch.GetImport(_head);
        if (!scene_head) {
            ASSIMP_LOG_ERROR("M3D: Failed to read multi part model, head.md3 fails to load");
            failure = "head";
            goto error_cleanup;
        }

        // build attachment infos. search for typical Q3 tags

        // original root
        scene_lower->mRootNode->mName.Set("lower");
        attach.push_back(AttachmentInfo(scene_lower, nd));

        // tag_torso
        tag_torso = scene_lower->mRootNode->FindNode("tag_torso");
        if (!tag_torso) {
            ASSIMP_LOG_ERROR("M3D: Failed to find attachment tag for multi part model: tag_torso expected");
            goto error_cleanup;
        }
        scene_upper->mRootNode->mName.Set("upper");
        attach.push_back(AttachmentInfo(scene_upper,tag_torso));

        // tag_head
        tag_head = scene_upper->mRootNode->FindNode("tag_head");
        if (!tag_head) {
            ASSIMP_LOG_ERROR( "M3D: Failed to find attachment tag for multi part model: tag_head expected");
            goto error_cleanup;
        }
        scene_head->mRootNode->mName.Set("head");
        attach.push_back(AttachmentInfo(scene_head,tag_head));

        // Remove tag_head and tag_torso from all other model parts ...
        // this ensures (together with AI_INT_MERGE_SCENE_GEN_UNIQUE_NAMES_IF_NECESSARY)
        // that tag_torso/tag_head is also the name of the (unique) output node
        RemoveSingleNodeFromList (scene_upper->mRootNode->FindNode("tag_torso"));
        RemoveSingleNodeFromList (scene_head-> mRootNode->FindNode("tag_head" ));

        // Undo the rotations which we applied to the coordinate systems. We're
        // working in global Quake space here
        scene_head->mRootNode->mTransformation  = aiMatrix4x4();
        scene_lower->mRootNode->mTransformation = aiMatrix4x4();
        scene_upper->mRootNode->mTransformation = aiMatrix4x4();

        // and merge the scenes
        SceneCombiner::MergeScenes(&mScene,master, attach,
            AI_INT_MERGE_SCENE_GEN_UNIQUE_NAMES          |
            AI_INT_MERGE_SCENE_GEN_UNIQUE_MATNAMES       |
            AI_INT_MERGE_SCENE_RESOLVE_CROSS_ATTACHMENTS |
            (!configSpeedFlag ? AI_INT_MERGE_SCENE_GEN_UNIQUE_NAMES_IF_NECESSARY : 0));

        // Now rotate the whole scene 90 degrees around the x axis to convert to internal coordinate system
        mScene->mRootNode->mTransformation = aiMatrix4x4(1.f,0.f,0.f,0.f,
            0.f,0.f,1.f,0.f,0.f,-1.f,0.f,0.f,0.f,0.f,0.f,1.f);

        return true;

error_cleanup:
        delete scene_upper;
        delete scene_lower;
        delete scene_head;
        delete master;

        if (failure == mod_filename) {
            throw DeadlyImportError("MD3: failure to read multipart host file");
        }
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Convert a MD3 path to a proper value
void MD3Importer::ConvertPath(const char* texture_name, const char* header_name, std::string& out) const
{
    // If the MD3's internal path itself and the given path are using
    // the same directory, remove it completely to get right output paths.
    const char* end1 = ::strrchr(header_name,'\\');
    if (!end1)end1   = ::strrchr(header_name,'/');

    const char* end2 = ::strrchr(texture_name,'\\');
    if (!end2)end2   = ::strrchr(texture_name,'/');

    // HACK: If the paths starts with "models", ignore the
    // next two hierarchy levels, it specifies just the model name.
    // Ignored by Q3, it might be not equal to the real model location.
    if (end2)   {

        size_t len2;
        const size_t len1 = (size_t)(end1 - header_name);
        if (!ASSIMP_strincmp(texture_name,"models",6) && (texture_name[6] == '/' || texture_name[6] == '\\')) {
            len2 = 6; // ignore the seventh - could be slash or backslash

            if (!header_name[0]) {
                // Use the file name only
                out = end2+1;
                return;
            }
        }
        else len2 = std::min (len1, (size_t)(end2 - texture_name ));
        if (!ASSIMP_strincmp(texture_name,header_name,static_cast<unsigned int>(len2))) {
            // Use the file name only
            out = end2+1;
            return;
        }
    }
    // Use the full path
    out = texture_name;
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void MD3Importer::InternReadFile( const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
    mFile = pFile;
    mScene = pScene;
    mIOHandler = pIOHandler;

    // get base path and file name
    // todo ... move to PathConverter
    std::string::size_type s = mFile.find_last_of("/\\");
    if (s == std::string::npos) {
        s = 0;
    }
    else ++s;
    filename = mFile.substr(s), path = mFile.substr(0,s);
    for( std::string::iterator it = filename .begin(); it != filename.end(); ++it)
        *it = tolower( *it);

    // Load multi-part model file, if necessary
    if (configHandleMP) {
        if (ReadMultipartFile())
            return;
    }

    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile));

    // Check whether we can read from the file
    if( file.get() == NULL)
        throw DeadlyImportError( "Failed to open MD3 file " + pFile + ".");

    // Check whether the md3 file is large enough to contain the header
    fileSize = (unsigned int)file->FileSize();
    if( fileSize < sizeof(MD3::Header))
        throw DeadlyImportError( "MD3 File is too small.");

    // Allocate storage and copy the contents of the file to a memory buffer
    std::vector<unsigned char> mBuffer2 (fileSize);
    file->Read( &mBuffer2[0], 1, fileSize);
    mBuffer = &mBuffer2[0];

    pcHeader = (BE_NCONST MD3::Header*)mBuffer;

    // Ensure correct endianness
#ifdef AI_BUILD_BIG_ENDIAN

    AI_SWAP4(pcHeader->VERSION);
    AI_SWAP4(pcHeader->FLAGS);
    AI_SWAP4(pcHeader->IDENT);
    AI_SWAP4(pcHeader->NUM_FRAMES);
    AI_SWAP4(pcHeader->NUM_SKINS);
    AI_SWAP4(pcHeader->NUM_SURFACES);
    AI_SWAP4(pcHeader->NUM_TAGS);
    AI_SWAP4(pcHeader->OFS_EOF);
    AI_SWAP4(pcHeader->OFS_FRAMES);
    AI_SWAP4(pcHeader->OFS_SURFACES);
    AI_SWAP4(pcHeader->OFS_TAGS);

#endif

    // Validate the file header
    ValidateHeaderOffsets();

    // Navigate to the list of surfaces
    BE_NCONST MD3::Surface* pcSurfaces = (BE_NCONST MD3::Surface*)(mBuffer + pcHeader->OFS_SURFACES);

    // Navigate to the list of tags
    BE_NCONST MD3::Tag* pcTags = (BE_NCONST MD3::Tag*)(mBuffer + pcHeader->OFS_TAGS);

    // Allocate output storage
    pScene->mNumMeshes = pcHeader->NUM_SURFACES;
    if (pcHeader->NUM_SURFACES == 0) {
        throw DeadlyImportError("MD3: No surfaces");
    } else if (pcHeader->NUM_SURFACES > AI_MAX_ALLOC(aiMesh)) {
        // We allocate pointers but check against the size of aiMesh
        // since those pointers will eventually have to point to real objects
        throw DeadlyImportError("MD3: Too many surfaces, would run out of memory");
    }
    pScene->mMeshes = new aiMesh*[pScene->mNumMeshes];

    pScene->mNumMaterials = pcHeader->NUM_SURFACES;
    pScene->mMaterials = new aiMaterial*[pScene->mNumMeshes];

    // Set arrays to zero to ensue proper destruction if an exception is raised
    ::memset(pScene->mMeshes,0,pScene->mNumMeshes*sizeof(aiMesh*));
    ::memset(pScene->mMaterials,0,pScene->mNumMaterials*sizeof(aiMaterial*));

    // Now read possible skins from .skin file
    Q3Shader::SkinData skins;
    ReadSkin(skins);

    // And check whether we can locate a shader file for this model
    Q3Shader::ShaderData shaders;
    ReadShader(shaders);

    // Adjust all texture paths in the shader
    const char* header_name = pcHeader->NAME;
    if (!shaders.blocks.empty()) {
        for (std::list< Q3Shader::ShaderDataBlock >::iterator dit = shaders.blocks.begin(); dit != shaders.blocks.end(); ++dit) {
            ConvertPath((*dit).name.c_str(),header_name,(*dit).name);

            for (std::list< Q3Shader::ShaderMapBlock >::iterator mit = (*dit).maps.begin(); mit != (*dit).maps.end(); ++mit) {
                ConvertPath((*mit).name.c_str(),header_name,(*mit).name);
            }
        }
    }

    // Read all surfaces from the file
    unsigned int iNum = pcHeader->NUM_SURFACES;
    unsigned int iNumMaterials = 0;
    while (iNum-- > 0)  {

        // Ensure correct endianness
#ifdef AI_BUILD_BIG_ENDIAN

        AI_SWAP4(pcSurfaces->FLAGS);
        AI_SWAP4(pcSurfaces->IDENT);
        AI_SWAP4(pcSurfaces->NUM_FRAMES);
        AI_SWAP4(pcSurfaces->NUM_SHADER);
        AI_SWAP4(pcSurfaces->NUM_TRIANGLES);
        AI_SWAP4(pcSurfaces->NUM_VERTICES);
        AI_SWAP4(pcSurfaces->OFS_END);
        AI_SWAP4(pcSurfaces->OFS_SHADERS);
        AI_SWAP4(pcSurfaces->OFS_ST);
        AI_SWAP4(pcSurfaces->OFS_TRIANGLES);
        AI_SWAP4(pcSurfaces->OFS_XYZNORMAL);

#endif

        // Validate the surface header
        ValidateSurfaceHeaderOffsets(pcSurfaces);

        // Navigate to the vertex list of the surface
        BE_NCONST MD3::Vertex* pcVertices = (BE_NCONST MD3::Vertex*)
            (((uint8_t*)pcSurfaces) + pcSurfaces->OFS_XYZNORMAL);

        // Navigate to the triangle list of the surface
        BE_NCONST MD3::Triangle* pcTriangles = (BE_NCONST MD3::Triangle*)
            (((uint8_t*)pcSurfaces) + pcSurfaces->OFS_TRIANGLES);

        // Navigate to the texture coordinate list of the surface
        BE_NCONST MD3::TexCoord* pcUVs = (BE_NCONST MD3::TexCoord*)
            (((uint8_t*)pcSurfaces) + pcSurfaces->OFS_ST);

        // Navigate to the shader list of the surface
        BE_NCONST MD3::Shader* pcShaders = (BE_NCONST MD3::Shader*)
            (((uint8_t*)pcSurfaces) + pcSurfaces->OFS_SHADERS);

        // If the submesh is empty ignore it
        if (0 == pcSurfaces->NUM_VERTICES || 0 == pcSurfaces->NUM_TRIANGLES)
        {
            pcSurfaces = (BE_NCONST MD3::Surface*)(((uint8_t*)pcSurfaces) + pcSurfaces->OFS_END);
            pScene->mNumMeshes--;
            continue;
        }

        // Allocate output mesh
        pScene->mMeshes[iNum] = new aiMesh();
        aiMesh* pcMesh = pScene->mMeshes[iNum];

        std::string _texture_name;
        const char* texture_name = NULL;

        // Check whether we have a texture record for this surface in the .skin file
        std::list< Q3Shader::SkinData::TextureEntry >::iterator it = std::find(
            skins.textures.begin(), skins.textures.end(), pcSurfaces->NAME );

        if (it != skins.textures.end()) {
            texture_name = &*( _texture_name = (*it).second).begin();
            ASSIMP_LOG_DEBUG_F("MD3: Assigning skin texture ", (*it).second, " to surface ", pcSurfaces->NAME);
            (*it).resolved = true; // mark entry as resolved
        }

        // Get the first shader (= texture?) assigned to the surface
        if (!texture_name && pcSurfaces->NUM_SHADER)    {
            texture_name = pcShaders->NAME;
        }

        std::string convertedPath;
        if (texture_name) {
            ConvertPath(texture_name,header_name,convertedPath);
        }

        const Q3Shader::ShaderDataBlock* shader = NULL;

        // Now search the current shader for a record with this name (
        // excluding texture file extension)
        if (!shaders.blocks.empty()) {

            std::string::size_type s = convertedPath.find_last_of('.');
            if (s == std::string::npos)
                s = convertedPath.length();

            const std::string without_ext = convertedPath.substr(0,s);
            std::list< Q3Shader::ShaderDataBlock >::const_iterator dit = std::find(shaders.blocks.begin(),shaders.blocks.end(),without_ext);
            if (dit != shaders.blocks.end()) {
                // Hurra, wir haben einen. Tolle Sache.
                shader = &*dit;
                ASSIMP_LOG_INFO("Found shader record for " +without_ext );
            } else {
                ASSIMP_LOG_WARN("Unable to find shader record for " + without_ext);
            }
        }

        aiMaterial* pcHelper = new aiMaterial();

        const int iMode = (int)aiShadingMode_Gouraud;
        pcHelper->AddProperty<int>(&iMode, 1, AI_MATKEY_SHADING_MODEL);

        // Add a small ambient color value - Quake 3 seems to have one
        aiColor3D clr;
        clr.b = clr.g = clr.r = 0.05f;
        pcHelper->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_AMBIENT);

        clr.b = clr.g = clr.r = 1.0f;
        pcHelper->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_DIFFUSE);
        pcHelper->AddProperty<aiColor3D>(&clr, 1,AI_MATKEY_COLOR_SPECULAR);

        // use surface name + skin_name as material name
        aiString name;
        name.Set("MD3_[" + configSkinFile + "][" + pcSurfaces->NAME + "]");
        pcHelper->AddProperty(&name,AI_MATKEY_NAME);

        if (!shader) {
            // Setup dummy texture file name to ensure UV coordinates are kept during postprocessing
            aiString szString;
            if (convertedPath.length()) {
                szString.Set(convertedPath);
            }
            else    {
                ASSIMP_LOG_WARN("Texture file name has zero length. Using default name");
                szString.Set("dummy_texture.bmp");
            }
            pcHelper->AddProperty(&szString,AI_MATKEY_TEXTURE_DIFFUSE(0));

            // prevent transparency by default
            int no_alpha = aiTextureFlags_IgnoreAlpha;
            pcHelper->AddProperty(&no_alpha,1,AI_MATKEY_TEXFLAGS_DIFFUSE(0));
        }
        else {
            Q3Shader::ConvertShaderToMaterial(pcHelper,*shader);
        }

        pScene->mMaterials[iNumMaterials] = (aiMaterial*)pcHelper;
        pcMesh->mMaterialIndex = iNumMaterials++;

            // Ensure correct endianness
#ifdef AI_BUILD_BIG_ENDIAN

        for (uint32_t i = 0; i < pcSurfaces->NUM_VERTICES;++i)  {
            AI_SWAP2( pcVertices[i].NORMAL );
            AI_SWAP2( pcVertices[i].X );
            AI_SWAP2( pcVertices[i].Y );
            AI_SWAP2( pcVertices[i].Z );

            AI_SWAP4( pcUVs[i].U );
            AI_SWAP4( pcUVs[i].U );
        }
        for (uint32_t i = 0; i < pcSurfaces->NUM_TRIANGLES;++i) {
            AI_SWAP4(pcTriangles[i].INDEXES[0]);
            AI_SWAP4(pcTriangles[i].INDEXES[1]);
            AI_SWAP4(pcTriangles[i].INDEXES[2]);
        }

#endif

        // Fill mesh information
        pcMesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;

        pcMesh->mNumVertices        = pcSurfaces->NUM_TRIANGLES*3;
        pcMesh->mNumFaces           = pcSurfaces->NUM_TRIANGLES;
        pcMesh->mFaces              = new aiFace[pcSurfaces->NUM_TRIANGLES];
        pcMesh->mNormals            = new aiVector3D[pcMesh->mNumVertices];
        pcMesh->mVertices           = new aiVector3D[pcMesh->mNumVertices];
        pcMesh->mTextureCoords[0]   = new aiVector3D[pcMesh->mNumVertices];
        pcMesh->mNumUVComponents[0] = 2;

        // Fill in all triangles
        unsigned int iCurrent = 0;
        for (unsigned int i = 0; i < (unsigned int)pcSurfaces->NUM_TRIANGLES;++i)   {
            pcMesh->mFaces[i].mIndices = new unsigned int[3];
            pcMesh->mFaces[i].mNumIndices = 3;

            //unsigned int iTemp = iCurrent;
            for (unsigned int c = 0; c < 3;++c,++iCurrent)  {
                pcMesh->mFaces[i].mIndices[c] = iCurrent;

                // Read vertices
                aiVector3D& vec = pcMesh->mVertices[iCurrent];
                uint32_t index = pcTriangles->INDEXES[c];
                if (index >= pcSurfaces->NUM_VERTICES) {
                    throw DeadlyImportError( "MD3: Invalid vertex index");
                }
                vec.x = pcVertices[index].X*AI_MD3_XYZ_SCALE;
                vec.y = pcVertices[index].Y*AI_MD3_XYZ_SCALE;
                vec.z = pcVertices[index].Z*AI_MD3_XYZ_SCALE;

                // Convert the normal vector to uncompressed float3 format
                aiVector3D& nor = pcMesh->mNormals[iCurrent];
                LatLngNormalToVec3(pcVertices[index].NORMAL,(ai_real*)&nor);

                // Read texture coordinates
                pcMesh->mTextureCoords[0][iCurrent].x = pcUVs[index].U;
                pcMesh->mTextureCoords[0][iCurrent].y = 1.0f-pcUVs[index].V;
            }
            // Flip face order if necessary
            if (!shader || shader->cull == Q3Shader::CULL_CW) {
                std::swap(pcMesh->mFaces[i].mIndices[2],pcMesh->mFaces[i].mIndices[1]);
            }
            pcTriangles++;
        }

        // Go to the next surface
        pcSurfaces = (BE_NCONST MD3::Surface*)(((unsigned char*)pcSurfaces) + pcSurfaces->OFS_END);
    }

    // For debugging purposes: check whether we found matches for all entries in the skins file
    if (!DefaultLogger::isNullLogger()) {
        for (std::list< Q3Shader::SkinData::TextureEntry>::const_iterator it = skins.textures.begin();it != skins.textures.end(); ++it) {
            if (!(*it).resolved) {
                ASSIMP_LOG_ERROR_F("MD3: Failed to match skin ", (*it).first, " to surface ", (*it).second);
            }
        }
    }

    if (!pScene->mNumMeshes)
        throw DeadlyImportError( "MD3: File contains no valid mesh");
    pScene->mNumMaterials = iNumMaterials;

    // Now we need to generate an empty node graph
    pScene->mRootNode = new aiNode("<MD3Root>");
    pScene->mRootNode->mNumMeshes = pScene->mNumMeshes;
    pScene->mRootNode->mMeshes = new unsigned int[pScene->mNumMeshes];

    // Attach tiny children for all tags
    if (pcHeader->NUM_TAGS) {
        pScene->mRootNode->mNumChildren = pcHeader->NUM_TAGS;
        pScene->mRootNode->mChildren = new aiNode*[pcHeader->NUM_TAGS];

        for (unsigned int i = 0; i < pcHeader->NUM_TAGS; ++i, ++pcTags) {

            aiNode* nd = pScene->mRootNode->mChildren[i] = new aiNode();
            nd->mName.Set((const char*)pcTags->NAME);
            nd->mParent = pScene->mRootNode;

            AI_SWAP4(pcTags->origin.x);
            AI_SWAP4(pcTags->origin.y);
            AI_SWAP4(pcTags->origin.z);

            // Copy local origin, again flip z,y
            nd->mTransformation.a4 = pcTags->origin.x;
            nd->mTransformation.b4 = pcTags->origin.y;
            nd->mTransformation.c4 = pcTags->origin.z;

            // Copy rest of transformation (need to transpose to match row-order matrix)
            for (unsigned int a = 0; a < 3;++a) {
                for (unsigned int m = 0; m < 3;++m) {
                    nd->mTransformation[m][a] = pcTags->orientation[a][m];
                    AI_SWAP4(nd->mTransformation[m][a]);
                }
            }
        }
    }

    for (unsigned int i = 0; i < pScene->mNumMeshes;++i)
        pScene->mRootNode->mMeshes[i] = i;

    // Now rotate the whole scene 90 degrees around the x axis to convert to internal coordinate system
    pScene->mRootNode->mTransformation = aiMatrix4x4(1.f,0.f,0.f,0.f,
        0.f,0.f,1.f,0.f,0.f,-1.f,0.f,0.f,0.f,0.f,0.f,1.f);
}

#endif // !! ASSIMP_BUILD_NO_MD3_IMPORTER
