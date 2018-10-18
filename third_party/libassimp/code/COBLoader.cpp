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

/** @file  COBLoader.cpp
 *  @brief Implementation of the TrueSpace COB/SCN importer class.
 */


#ifndef ASSIMP_BUILD_NO_COB_IMPORTER
#include "COBLoader.h"
#include "COBScene.h"
#include "ConvertToLHProcess.h"
#include <assimp/StreamReader.h>
#include <assimp/ParsingUtils.h>
#include <assimp/fast_atof.h>
#include <assimp/LineSplitter.h>
#include <assimp/TinyFormatter.h>
#include <memory>
#include <assimp/IOSystem.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/scene.h>
#include <assimp/importerdesc.h>

using namespace Assimp;
using namespace Assimp::COB;
using namespace Assimp::Formatter;


static const float units[] = {
    1000.f,
    100.f,
    1.f,
    0.001f,
    1.f/0.0254f,
    1.f/0.3048f,
    1.f/0.9144f,
    1.f/1609.344f
};

static const aiImporterDesc desc = {
    "TrueSpace Object Importer",
    "",
    "",
    "little-endian files only",
    aiImporterFlags_SupportTextFlavour | aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "cob scn"
};


// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
COBImporter::COBImporter()
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
COBImporter::~COBImporter()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool COBImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    const std::string& extension = GetExtension(pFile);
    if (extension == "cob" || extension == "scn" || extension == "COB" || extension == "SCN") {
        return true;
    }

    else if ((!extension.length() || checkSig) && pIOHandler)   {
        const char* tokens[] = {"Caligary"};
        return SearchFileHeaderForToken(pIOHandler,pFile,tokens,1);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Loader meta information
const aiImporterDesc* COBImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Setup configuration properties for the loader
void COBImporter::SetupProperties(const Importer* /*pImp*/)
{
    // nothing to be done for the moment
}

// ------------------------------------------------------------------------------------------------
/*static*/ AI_WONT_RETURN void COBImporter::ThrowException(const std::string& msg)
{
    throw DeadlyImportError("COB: "+msg);
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void COBImporter::InternReadFile( const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler) {
    COB::Scene scene;
    std::unique_ptr<StreamReaderLE> stream(new StreamReaderLE( pIOHandler->Open(pFile,"rb")) );

    // check header
    char head[32];
    stream->CopyAndAdvance(head,32);
    if (strncmp(head,"Caligari ",9)) {
        ThrowException("Could not found magic id: `Caligari`");
    }

    ASSIMP_LOG_INFO_F("File format tag: ",std::string(head+9,6));
    if (head[16]!='L') {
        ThrowException("File is big-endian, which is not supported");
    }

    // load data into intermediate structures
    if (head[15]=='A') {
        ReadAsciiFile(scene, stream.get());
    }
    else {
        ReadBinaryFile(scene, stream.get());
    }
    if(scene.nodes.empty()) {
        ThrowException("No nodes loaded");
    }

    // sort faces by material indices
    for(std::shared_ptr< Node >& n : scene.nodes) {
        if (n->type == Node::TYPE_MESH) {
            Mesh& mesh = (Mesh&)(*n.get());
            for(Face& f : mesh.faces) {
                mesh.temp_map[f.material].push_back(&f);
            }
        }
    }

    // count meshes
    for(std::shared_ptr< Node >& n : scene.nodes) {
        if (n->type == Node::TYPE_MESH) {
            Mesh& mesh = (Mesh&)(*n.get());
            if (mesh.vertex_positions.size() && mesh.texture_coords.size()) {
                pScene->mNumMeshes += static_cast<unsigned int>(mesh.temp_map.size());
            }
        }
    }
    pScene->mMeshes = new aiMesh*[pScene->mNumMeshes]();
    pScene->mMaterials = new aiMaterial*[pScene->mNumMeshes]();
    pScene->mNumMeshes = 0;

    // count lights and cameras
    for(std::shared_ptr< Node >& n : scene.nodes) {
        if (n->type == Node::TYPE_LIGHT) {
            ++pScene->mNumLights;
        }
        else if (n->type == Node::TYPE_CAMERA) {
            ++pScene->mNumCameras;
        }
    }

    if (pScene->mNumLights) {
        pScene->mLights  = new aiLight*[pScene->mNumLights]();
    }
    if (pScene->mNumCameras) {
        pScene->mCameras = new aiCamera*[pScene->mNumCameras]();
    }
    pScene->mNumLights = pScene->mNumCameras = 0;

    // resolve parents by their IDs and build the output graph
    std::unique_ptr<Node> root(new Group());
    for(size_t n = 0; n < scene.nodes.size(); ++n) {
        const Node& nn = *scene.nodes[n].get();
        if(nn.parent_id==0) {
            root->temp_children.push_back(&nn);
        }

        for(size_t m = n; m < scene.nodes.size(); ++m) {
            const Node& mm = *scene.nodes[m].get();
            if (mm.parent_id == nn.id) {
                nn.temp_children.push_back(&mm);
            }
        }
    }

    pScene->mRootNode = BuildNodes(*root.get(),scene,pScene);
	//flip normals after import
    FlipWindingOrderProcess flip;
    flip.Execute( pScene );
}

// ------------------------------------------------------------------------------------------------
void ConvertTexture(std::shared_ptr< Texture > tex, aiMaterial* out, aiTextureType type)
{
    const aiString path( tex->path );
    out->AddProperty(&path,AI_MATKEY_TEXTURE(type,0));
    out->AddProperty(&tex->transform,1,AI_MATKEY_UVTRANSFORM(type,0));
}

// ------------------------------------------------------------------------------------------------
aiNode* COBImporter::BuildNodes(const Node& root,const Scene& scin,aiScene* fill)
{
    aiNode* nd = new aiNode();
    nd->mName.Set(root.name);
    nd->mTransformation = root.transform;

    // Note to everybody believing Voodoo is appropriate here:
    // I know polymorphism, run as fast as you can ;-)
    if (Node::TYPE_MESH == root.type) {
        const Mesh& ndmesh = (const Mesh&)(root);
        if (ndmesh.vertex_positions.size() && ndmesh.texture_coords.size()) {

            typedef std::pair<unsigned int,Mesh::FaceRefList> Entry;
            for(const Entry& reflist : ndmesh.temp_map) {
                {   // create mesh
                    size_t n = 0;
                    for(Face* f : reflist.second) {
                        n += f->indices.size();
                    }
                    if (!n) {
                        continue;
                    }
                    aiMesh* outmesh = fill->mMeshes[fill->mNumMeshes++] = new aiMesh();
                    ++nd->mNumMeshes;

                    outmesh->mVertices = new aiVector3D[n];
                    outmesh->mTextureCoords[0] = new aiVector3D[n];

                    outmesh->mFaces = new aiFace[reflist.second.size()]();
                    for(Face* f : reflist.second) {
                        if (f->indices.empty()) {
                            continue;
                        }

                        aiFace& fout = outmesh->mFaces[outmesh->mNumFaces++];
                        fout.mIndices = new unsigned int[f->indices.size()];

                        for(VertexIndex& v : f->indices) {
                            if (v.pos_idx >= ndmesh.vertex_positions.size()) {
                                ThrowException("Position index out of range");
                            }
                            if (v.uv_idx >= ndmesh.texture_coords.size()) {
                                ThrowException("UV index out of range");
                            }
                            outmesh->mVertices[outmesh->mNumVertices] = ndmesh.vertex_positions[ v.pos_idx ];
                            outmesh->mTextureCoords[0][outmesh->mNumVertices] = aiVector3D(
                                ndmesh.texture_coords[ v.uv_idx ].x,
                                ndmesh.texture_coords[ v.uv_idx ].y,
                                0.f
                            );

                            fout.mIndices[fout.mNumIndices++] = outmesh->mNumVertices++;
                        }
                    }
                    outmesh->mMaterialIndex = fill->mNumMaterials;
                }{  // create material
                    const Material* min = NULL;
                    for(const Material& m : scin.materials) {
                        if (m.parent_id == ndmesh.id && m.matnum == reflist.first) {
                            min = &m;
                            break;
                        }
                    }
                    std::unique_ptr<const Material> defmat;
                    if(!min) {
                        ASSIMP_LOG_DEBUG(format()<<"Could not resolve material index "
                            <<reflist.first<<" - creating default material for this slot");

                        defmat.reset(min=new Material());
                    }

                    aiMaterial* mat = new aiMaterial();
                    fill->mMaterials[fill->mNumMaterials++] = mat;

                    const aiString s(format("#mat_")<<fill->mNumMeshes<<"_"<<min->matnum);
                    mat->AddProperty(&s,AI_MATKEY_NAME);

                    if(int tmp = ndmesh.draw_flags & Mesh::WIRED ? 1 : 0) {
                        mat->AddProperty(&tmp,1,AI_MATKEY_ENABLE_WIREFRAME);
                    }

                    {   int shader;
                        switch(min->shader)
                        {
                        case Material::FLAT:
                            shader = aiShadingMode_Gouraud;
                            break;

                        case Material::PHONG:
                            shader = aiShadingMode_Phong;
                            break;

                        case Material::METAL:
                            shader = aiShadingMode_CookTorrance;
                            break;

                        default:
                            ai_assert(false); // shouldn't be here
                        }
                        mat->AddProperty(&shader,1,AI_MATKEY_SHADING_MODEL);
                        if(shader != aiShadingMode_Gouraud) {
                            mat->AddProperty(&min->exp,1,AI_MATKEY_SHININESS);
                        }
                    }

                    mat->AddProperty(&min->ior,1,AI_MATKEY_REFRACTI);
                    mat->AddProperty(&min->rgb,1,AI_MATKEY_COLOR_DIFFUSE);

                    aiColor3D c = aiColor3D(min->rgb)*min->ks;
                    mat->AddProperty(&c,1,AI_MATKEY_COLOR_SPECULAR);

                    c = aiColor3D(min->rgb)*min->ka;
                    mat->AddProperty(&c,1,AI_MATKEY_COLOR_AMBIENT);

                    // convert textures if some exist.
                    if(min->tex_color) {
                        ConvertTexture(min->tex_color,mat,aiTextureType_DIFFUSE);
                    }
                    if(min->tex_env) {
                        ConvertTexture(min->tex_env  ,mat,aiTextureType_UNKNOWN);
                    }
                    if(min->tex_bump) {
                        ConvertTexture(min->tex_bump ,mat,aiTextureType_HEIGHT);
                    }
                }
            }
        }
    }
    else if (Node::TYPE_LIGHT == root.type) {
        const Light& ndlight = (const Light&)(root);
        aiLight* outlight = fill->mLights[fill->mNumLights++] = new aiLight();

        outlight->mName.Set(ndlight.name);
        outlight->mColorDiffuse = outlight->mColorAmbient = outlight->mColorSpecular = ndlight.color;

        outlight->mAngleOuterCone = AI_DEG_TO_RAD(ndlight.angle);
        outlight->mAngleInnerCone = AI_DEG_TO_RAD(ndlight.inner_angle);

        // XXX
        outlight->mType = ndlight.ltype==Light::SPOT ? aiLightSource_SPOT : aiLightSource_DIRECTIONAL;
    }
    else if (Node::TYPE_CAMERA == root.type) {
        const Camera& ndcam = (const Camera&)(root);
        aiCamera* outcam = fill->mCameras[fill->mNumCameras++] = new aiCamera();

        outcam->mName.Set(ndcam.name);
    }

    // add meshes
    if (nd->mNumMeshes) { // mMeshes must be NULL if count is 0
        nd->mMeshes = new unsigned int[nd->mNumMeshes];
        for(unsigned int i = 0; i < nd->mNumMeshes;++i) {
            nd->mMeshes[i] = fill->mNumMeshes-i-1;
        }
    }

    // add children recursively
    nd->mChildren = new aiNode*[root.temp_children.size()]();
    for(const Node* n : root.temp_children) {
        (nd->mChildren[nd->mNumChildren++] = BuildNodes(*n,scin,fill))->mParent = nd;
    }

    return nd;
}

// ------------------------------------------------------------------------------------------------
// Read an ASCII file into the given scene data structure
void COBImporter::ReadAsciiFile(Scene& out, StreamReaderLE* stream)
{
    ChunkInfo ci;
    for(LineSplitter splitter(*stream);splitter;++splitter) {

        // add all chunks to be recognized here. /else ../ omitted intentionally.
        if (splitter.match_start("PolH ")) {
            ReadChunkInfo_Ascii(ci,splitter);
            ReadPolH_Ascii(out,splitter,ci);
        }
        if (splitter.match_start("BitM ")) {
            ReadChunkInfo_Ascii(ci,splitter);
            ReadBitM_Ascii(out,splitter,ci);
        }
        if (splitter.match_start("Mat1 ")) {
            ReadChunkInfo_Ascii(ci,splitter);
            ReadMat1_Ascii(out,splitter,ci);
        }
        if (splitter.match_start("Grou ")) {
            ReadChunkInfo_Ascii(ci,splitter);
            ReadGrou_Ascii(out,splitter,ci);
        }
        if (splitter.match_start("Lght ")) {
            ReadChunkInfo_Ascii(ci,splitter);
            ReadLght_Ascii(out,splitter,ci);
        }
        if (splitter.match_start("Came ")) {
            ReadChunkInfo_Ascii(ci,splitter);
            ReadCame_Ascii(out,splitter,ci);
        }
        if (splitter.match_start("Bone ")) {
            ReadChunkInfo_Ascii(ci,splitter);
            ReadBone_Ascii(out,splitter,ci);
        }
        if (splitter.match_start("Chan ")) {
            ReadChunkInfo_Ascii(ci,splitter);
            ReadChan_Ascii(out,splitter,ci);
        }
        if (splitter.match_start("Unit ")) {
            ReadChunkInfo_Ascii(ci,splitter);
            ReadUnit_Ascii(out,splitter,ci);
        }
        if (splitter.match_start("END ")) {
            // we don't need this, but I guess there is a reason this
            // chunk has been implemented into COB for.
            return;
        }
    }
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadChunkInfo_Ascii(ChunkInfo& out, const LineSplitter& splitter)
{
    const char* all_tokens[8];
    splitter.get_tokens(all_tokens);

    out.version = (all_tokens[1][1]-'0')*100+(all_tokens[1][3]-'0')*10+(all_tokens[1][4]-'0');
    out.id  = strtoul10(all_tokens[3]);
    out.parent_id = strtoul10(all_tokens[5]);
    out.size = strtol10(all_tokens[7]);
}

// ------------------------------------------------------------------------------------------------
void COBImporter::UnsupportedChunk_Ascii(LineSplitter& splitter, const ChunkInfo& nfo, const char* name)
{
    const std::string error = format("Encountered unsupported chunk: ") <<  name <<
        " [version: "<<nfo.version<<", size: "<<nfo.size<<"]";

    // we can recover if the chunk size was specified.
    if(nfo.size != static_cast<unsigned int>(-1)) {
        ASSIMP_LOG_ERROR(error);

        // (HACK) - our current position in the stream is the beginning of the
        // head line of the next chunk. That's fine, but the caller is going
        // to call ++ on `splitter`, which we need to swallow to avoid
        // missing the next line.
        splitter.get_stream().IncPtr(nfo.size);
        splitter.swallow_next_increment();
    }
    else ThrowException(error);
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadBasicNodeInfo_Ascii(Node& msh, LineSplitter& splitter, const ChunkInfo& /*nfo*/)
{
    for(;splitter;++splitter) {
        if (splitter.match_start("Name")) {
            msh.name = std::string(splitter[1]);

            // make nice names by merging the dupe count
            std::replace(msh.name.begin(),msh.name.end(),
                ',','_');
        }
        else if (splitter.match_start("Transform")) {
            for(unsigned int y = 0; y < 4 && ++splitter; ++y) {
                const char* s = splitter->c_str();
                for(unsigned int x = 0; x < 4; ++x) {
                    SkipSpaces(&s);
                    msh.transform[y][x] = fast_atof(&s);
                }
            }
            // we need the transform chunk, so we won't return until we have it.
            return;
        }
    }
}

// ------------------------------------------------------------------------------------------------
template <typename T>
void COBImporter::ReadFloat3Tuple_Ascii(T& fill, const char** in)
{
    const char* rgb = *in;
    for(unsigned int i = 0; i < 3; ++i) {
        SkipSpaces(&rgb);
        if (*rgb == ',')++rgb;
        SkipSpaces(&rgb);

        fill[i] = fast_atof(&rgb);
    }
    *in = rgb;
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadMat1_Ascii(Scene& out, LineSplitter& splitter, const ChunkInfo& nfo)
{
    if(nfo.version > 8) {
        return UnsupportedChunk_Ascii(splitter,nfo,"Mat1");
    }

    ++splitter;
    if (!splitter.match_start("mat# ")) {
        ASSIMP_LOG_WARN_F( "Expected `mat#` line in `Mat1` chunk ", nfo.id );
        return;
    }

    out.materials.push_back(Material());
    Material& mat = out.materials.back();
    mat = nfo;

    mat.matnum = strtoul10(splitter[1]);
    ++splitter;

    if (!splitter.match_start("shader: ")) {
        ASSIMP_LOG_WARN_F( "Expected `mat#` line in `Mat1` chunk ", nfo.id);
        return;
    }
    std::string shader = std::string(splitter[1]);
    shader = shader.substr(0,shader.find_first_of(" \t"));

    if (shader == "metal") {
        mat.shader = Material::METAL;
    }
    else if (shader == "phong") {
        mat.shader = Material::PHONG;
    }
    else if (shader != "flat") {
        ASSIMP_LOG_WARN_F( "Unknown value for `shader` in `Mat1` chunk ", nfo.id );
    }

    ++splitter;
    if (!splitter.match_start("rgb ")) {
        ASSIMP_LOG_WARN_F( "Expected `rgb` line in `Mat1` chunk ", nfo.id);
    }

    const char* rgb = splitter[1];
    ReadFloat3Tuple_Ascii(mat.rgb,&rgb);

    ++splitter;
    if (!splitter.match_start("alpha ")) {
        ASSIMP_LOG_WARN_F( "Expected `alpha` line in `Mat1` chunk ", nfo.id);
    }

    const char* tokens[10];
    splitter.get_tokens(tokens);

    mat.alpha   = fast_atof( tokens[1] );
    mat.ka      = fast_atof( tokens[3] );
    mat.ks      = fast_atof( tokens[5] );
    mat.exp     = fast_atof( tokens[7] );
    mat.ior     = fast_atof( tokens[9] );
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadUnit_Ascii(Scene& out, LineSplitter& splitter, const ChunkInfo& nfo)
{
    if(nfo.version > 1) {
        return UnsupportedChunk_Ascii(splitter,nfo,"Unit");
    }
    ++splitter;
    if (!splitter.match_start("Units ")) {
        ASSIMP_LOG_WARN_F( "Expected `Units` line in `Unit` chunk ", nfo.id);
        return;
    }

    // parent chunks preceede their childs, so we should have the
    // corresponding chunk already.
    for(std::shared_ptr< Node >& nd : out.nodes) {
        if (nd->id == nfo.parent_id) {
            const unsigned int t=strtoul10(splitter[1]);

            nd->unit_scale = t>=sizeof(units)/sizeof(units[0])?(
                ASSIMP_LOG_WARN_F(t, " is not a valid value for `Units` attribute in `Unit chunk` ", nfo.id)
                ,1.f):units[t];
            return;
        }
    }
    ASSIMP_LOG_WARN_F( "`Unit` chunk ", nfo.id, " is a child of ", nfo.parent_id, " which does not exist");
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadChan_Ascii(Scene& /*out*/, LineSplitter& splitter, const ChunkInfo& nfo)
{
    if(nfo.version > 8) {
        return UnsupportedChunk_Ascii(splitter,nfo,"Chan");
    }
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadLght_Ascii(Scene& out, LineSplitter& splitter, const ChunkInfo& nfo)
{
    if(nfo.version > 8) {
        return UnsupportedChunk_Ascii(splitter,nfo,"Lght");
    }

    out.nodes.push_back(std::shared_ptr<Light>(new Light()));
    Light& msh = (Light&)(*out.nodes.back().get());
    msh = nfo;

    ReadBasicNodeInfo_Ascii(msh,++splitter,nfo);

    if (splitter.match_start("Infinite ")) {
        msh.ltype = Light::INFINITE;
    }
    else if (splitter.match_start("Local ")) {
        msh.ltype = Light::LOCAL;
    }
    else if (splitter.match_start("Spot ")) {
        msh.ltype = Light::SPOT;
    }
    else {
        ASSIMP_LOG_WARN_F( "Unknown kind of light source in `Lght` chunk ", nfo.id, " : ", *splitter );
        msh.ltype = Light::SPOT;
    }

    ++splitter;
    if (!splitter.match_start("color ")) {
        ASSIMP_LOG_WARN_F( "Expected `color` line in `Lght` chunk ", nfo.id );
    }

    const char* rgb = splitter[1];
    ReadFloat3Tuple_Ascii(msh.color ,&rgb);

    SkipSpaces(&rgb);
    if (strncmp(rgb,"cone angle",10)) {
        ASSIMP_LOG_WARN_F( "Expected `cone angle` entity in `color` line in `Lght` chunk ", nfo.id );
    }
    SkipSpaces(rgb+10,&rgb);
    msh.angle = fast_atof(&rgb);

    SkipSpaces(&rgb);
    if (strncmp(rgb,"inner angle",11)) {
        ASSIMP_LOG_WARN_F( "Expected `inner angle` entity in `color` line in `Lght` chunk ", nfo.id);
    }
    SkipSpaces(rgb+11,&rgb);
    msh.inner_angle = fast_atof(&rgb);

    // skip the rest for we can't handle this kind of physically-based lighting information.
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadCame_Ascii(Scene& out, LineSplitter& splitter, const ChunkInfo& nfo)
{
    if(nfo.version > 2) {
        return UnsupportedChunk_Ascii(splitter,nfo,"Came");
    }

    out.nodes.push_back(std::shared_ptr<Camera>(new Camera()));
    Camera& msh = (Camera&)(*out.nodes.back().get());
    msh = nfo;

    ReadBasicNodeInfo_Ascii(msh,++splitter,nfo);

    // skip the next line, we don't know this differenciation between a
    // standard camera and a panoramic camera.
    ++splitter;
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadBone_Ascii(Scene& out, LineSplitter& splitter, const ChunkInfo& nfo)
{
    if(nfo.version > 5) {
        return UnsupportedChunk_Ascii(splitter,nfo,"Bone");
    }

    out.nodes.push_back(std::shared_ptr<Bone>(new Bone()));
    Bone& msh = (Bone&)(*out.nodes.back().get());
    msh = nfo;

    ReadBasicNodeInfo_Ascii(msh,++splitter,nfo);

    // TODO
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadGrou_Ascii(Scene& out, LineSplitter& splitter, const ChunkInfo& nfo)
{
    if(nfo.version > 1) {
        return UnsupportedChunk_Ascii(splitter,nfo,"Grou");
    }

    out.nodes.push_back(std::shared_ptr<Group>(new Group()));
    Group& msh = (Group&)(*out.nodes.back().get());
    msh = nfo;

    ReadBasicNodeInfo_Ascii(msh,++splitter,nfo);
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadPolH_Ascii(Scene& out, LineSplitter& splitter, const ChunkInfo& nfo)
{
    if(nfo.version > 8) {
        return UnsupportedChunk_Ascii(splitter,nfo,"PolH");
    }

    out.nodes.push_back(std::shared_ptr<Mesh>(new Mesh()));
    Mesh& msh = (Mesh&)(*out.nodes.back().get());
    msh = nfo;

    ReadBasicNodeInfo_Ascii(msh,++splitter,nfo);

    // the chunk has a fixed order of components, but some are not interesting of us so
    // we're just looking for keywords in arbitrary order. The end of the chunk is
    // either the last `Face` or the `DrawFlags` attribute, depending on the format ver.
    for(;splitter;++splitter) {
        if (splitter.match_start("World Vertices")) {
            const unsigned int cnt = strtoul10(splitter[2]);
            msh.vertex_positions.resize(cnt);

            for(unsigned int cur = 0;cur < cnt && ++splitter;++cur) {
                const char* s = splitter->c_str();

                aiVector3D& v = msh.vertex_positions[cur];

                SkipSpaces(&s);
                v.x = fast_atof(&s);
                SkipSpaces(&s);
                v.y = fast_atof(&s);
                SkipSpaces(&s);
                v.z = fast_atof(&s);
            }
        }
        else if (splitter.match_start("Texture Vertices")) {
            const unsigned int cnt = strtoul10(splitter[2]);
            msh.texture_coords.resize(cnt);

            for(unsigned int cur = 0;cur < cnt && ++splitter;++cur) {
                const char* s = splitter->c_str();

                aiVector2D& v = msh.texture_coords[cur];

                SkipSpaces(&s);
                v.x = fast_atof(&s);
                SkipSpaces(&s);
                v.y = fast_atof(&s);
            }
        }
        else if (splitter.match_start("Faces")) {
            const unsigned int cnt = strtoul10(splitter[1]);
            msh.faces.reserve(cnt);

            for(unsigned int cur = 0; cur < cnt && ++splitter ;++cur) {
                if (splitter.match_start("Hole")) {
                    ASSIMP_LOG_WARN( "Skipping unsupported `Hole` line" );
                    continue;
                }

                if (!splitter.match_start("Face")) {
                    ThrowException("Expected Face line");
                }

                msh.faces.push_back(Face());
                Face& face = msh.faces.back();

                face.indices.resize(strtoul10(splitter[2]));
                face.flags = strtoul10(splitter[4]);
                face.material = strtoul10(splitter[6]);

                const char* s = (++splitter)->c_str();
                for(size_t i = 0; i < face.indices.size(); ++i) {
                    if(!SkipSpaces(&s)) {
                        ThrowException("Expected EOL token in Face entry");
                    }
                    if ('<' != *s++) {
                        ThrowException("Expected < token in Face entry");
                    }
                    face.indices[i].pos_idx = strtoul10(s,&s);
                    if (',' != *s++) {
                        ThrowException("Expected , token in Face entry");
                    }
                    face.indices[i].uv_idx = strtoul10(s,&s);
                    if ('>' != *s++) {
                        ThrowException("Expected < token in Face entry");
                    }
                }
            }
            if (nfo.version <= 4) {
                break;
            }
        }
        else if (splitter.match_start("DrawFlags")) {
            msh.draw_flags = strtoul10(splitter[1]);
            break;
        }
    }
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadBitM_Ascii(Scene& /*out*/, LineSplitter& splitter, const ChunkInfo& nfo)
{
    if(nfo.version > 1) {
        return UnsupportedChunk_Ascii(splitter,nfo,"BitM");
    }
/*
    "\nThumbNailHdrSize %ld"
    "\nThumbHeader: %02hx 02hx %02hx "
    "\nColorBufSize %ld"
    "\nColorBufZipSize %ld"
    "\nZippedThumbnail: %02hx 02hx %02hx "
*/

    const unsigned int head = strtoul10((++splitter)[1]);
    if (head != sizeof(Bitmap::BitmapHeader)) {
        ASSIMP_LOG_WARN("Unexpected ThumbNailHdrSize, skipping this chunk");
        return;
    }

    /*union {
        Bitmap::BitmapHeader data;
        char opaq[sizeof Bitmap::BitmapHeader()];
    };*/
//  ReadHexOctets(opaq,head,(++splitter)[1]);
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadString_Binary(std::string& out, StreamReaderLE& reader)
{
    out.resize( reader.GetI2());
    for(char& c : out) {
        c = reader.GetI1();
    }
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadBasicNodeInfo_Binary(Node& msh, StreamReaderLE& reader, const ChunkInfo& /*nfo*/)
{
    const unsigned int dupes = reader.GetI2();
    ReadString_Binary(msh.name,reader);

    msh.name = format(msh.name)<<'_'<<dupes;

    // skip local axes for the moment
    reader.IncPtr(48);

    msh.transform = aiMatrix4x4();
    for(unsigned int y = 0; y < 3; ++y) {
        for(unsigned int x =0; x < 4; ++x) {
            msh.transform[y][x] = reader.GetF4();
        }
    }
}

// ------------------------------------------------------------------------------------------------
void COBImporter::UnsupportedChunk_Binary( StreamReaderLE& reader, const ChunkInfo& nfo, const char* name)
{
    const std::string error = format("Encountered unsupported chunk: ") <<  name <<
        " [version: "<<nfo.version<<", size: "<<nfo.size<<"]";

    // we can recover if the chunk size was specified.
    if(nfo.size != static_cast<unsigned int>(-1)) {
        ASSIMP_LOG_ERROR(error);
        reader.IncPtr(nfo.size);
    }
    else ThrowException(error);
}

// ------------------------------------------------------------------------------------------------
// tiny utility guard to aid me at staying within chunk boundaries.
class chunk_guard {
public:
    chunk_guard(const COB::ChunkInfo& nfo, StreamReaderLE& reader)
    : nfo(nfo)
    , reader(reader)
    , cur(reader.GetCurrentPos()) {
    }

    ~chunk_guard() {
        // don't do anything if the size is not given
        if(nfo.size != static_cast<unsigned int>(-1)) {
            try {
                reader.IncPtr( static_cast< int >( nfo.size ) - reader.GetCurrentPos() + cur );
            } catch ( DeadlyImportError e ) {
                // out of limit so correct the value
                reader.IncPtr( reader.GetReadLimit() );
            }
        }
    }

private:

    const COB::ChunkInfo& nfo;
    StreamReaderLE& reader;
    long cur;
};

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadBinaryFile(Scene& out, StreamReaderLE* reader)
{
    while(1) {
        std::string type;
         type += reader -> GetI1()
        ,type += reader -> GetI1()
        ,type += reader -> GetI1()
        ,type += reader -> GetI1()
        ;

        ChunkInfo nfo;
        nfo.version  = reader -> GetI2()*10;
        nfo.version += reader -> GetI2();

        nfo.id = reader->GetI4();
        nfo.parent_id = reader->GetI4();
        nfo.size = reader->GetI4();

        if (type == "PolH") {
            ReadPolH_Binary(out,*reader,nfo);
        }
        else if (type == "BitM") {
            ReadBitM_Binary(out,*reader,nfo);
        }
        else if (type == "Grou") {
            ReadGrou_Binary(out,*reader,nfo);
        }
        else if (type == "Lght") {
            ReadLght_Binary(out,*reader,nfo);
        }
        else if (type == "Came") {
            ReadCame_Binary(out,*reader,nfo);
        }
        else if (type == "Mat1") {
            ReadMat1_Binary(out,*reader,nfo);
        }
    /*  else if (type == "Bone") {
            ReadBone_Binary(out,*reader,nfo);
        }
        else if (type == "Chan") {
            ReadChan_Binary(out,*reader,nfo);
        }*/
        else if (type == "Unit") {
            ReadUnit_Binary(out,*reader,nfo);
        }
        else if (type == "OLay") {
            // ignore layer index silently.
            if(nfo.size != static_cast<unsigned int>(-1) ) {
                reader->IncPtr(nfo.size);
            }
            else return UnsupportedChunk_Binary(*reader,nfo,type.c_str());
        }
        else if (type == "END ") {
            return;
        }
        else UnsupportedChunk_Binary(*reader,nfo,type.c_str());
    }
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadPolH_Binary(COB::Scene& out, StreamReaderLE& reader, const ChunkInfo& nfo)
{
    if(nfo.version > 8) {
        return UnsupportedChunk_Binary(reader,nfo,"PolH");
    }
    const chunk_guard cn(nfo,reader);

    out.nodes.push_back(std::shared_ptr<Mesh>(new Mesh()));
    Mesh& msh = (Mesh&)(*out.nodes.back().get());
    msh = nfo;

    ReadBasicNodeInfo_Binary(msh,reader,nfo);

    msh.vertex_positions.resize(reader.GetI4());
    for(aiVector3D& v : msh.vertex_positions) {
        v.x = reader.GetF4();
        v.y = reader.GetF4();
        v.z = reader.GetF4();
    }

    msh.texture_coords.resize(reader.GetI4());
    for(aiVector2D& v : msh.texture_coords) {
        v.x = reader.GetF4();
        v.y = reader.GetF4();
    }

    const size_t numf = reader.GetI4();
    msh.faces.reserve(numf);
    for(size_t i = 0; i < numf; ++i) {
        // XXX backface culling flag is 0x10 in flags

        // hole?
        bool hole;
        if ((hole = (reader.GetI1() & 0x08) != 0)) {
            // XXX Basically this should just work fine - then triangulator
            // should output properly triangulated data even for polygons
            // with holes. Test data specific to COB is needed to confirm it.
            if (msh.faces.empty()) {
                ThrowException(format("A hole is the first entity in the `PolH` chunk with id ") << nfo.id);
            }
        }
        else msh.faces.push_back(Face());
        Face& f = msh.faces.back();

        const size_t num = reader.GetI2();
        f.indices.reserve(f.indices.size() + num);

        if(!hole) {
            f.material = reader.GetI2();
            f.flags = 0;
        }

        for(size_t x = 0; x < num; ++x) {
            f.indices.push_back(VertexIndex());

            VertexIndex& v = f.indices.back();
            v.pos_idx = reader.GetI4();
            v.uv_idx = reader.GetI4();
        }

        if(hole) {
            std::reverse(f.indices.rbegin(),f.indices.rbegin()+num);
        }
    }
    if (nfo.version>4) {
        msh.draw_flags = reader.GetI4();
    }
    nfo.version>5 && nfo.version<8 ? reader.GetI4() : 0;
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadBitM_Binary(COB::Scene& /*out*/, StreamReaderLE& reader, const ChunkInfo& nfo)
{
    if(nfo.version > 1) {
        return UnsupportedChunk_Binary(reader,nfo,"BitM");
    }

    const chunk_guard cn(nfo,reader);

    const uint32_t len = reader.GetI4();
    reader.IncPtr(len);

    reader.GetI4();
    reader.IncPtr(reader.GetI4());
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadMat1_Binary(COB::Scene& out, StreamReaderLE& reader, const ChunkInfo& nfo)
{
    if(nfo.version > 8) {
        return UnsupportedChunk_Binary(reader,nfo,"Mat1");
    }

    const chunk_guard cn(nfo,reader);

    out.materials.push_back(Material());
    Material& mat = out.materials.back();
    mat = nfo;

    mat.matnum = reader.GetI2();
    switch(reader.GetI1()) {
        case 'f':
            mat.type = Material::FLAT;
            break;
        case 'p':
            mat.type = Material::PHONG;
            break;
        case 'm':
            mat.type = Material::METAL;
            break;
        default:
            ASSIMP_LOG_ERROR_F( "Unrecognized shader type in `Mat1` chunk with id ", nfo.id );
            mat.type = Material::FLAT;
    }

    switch(reader.GetI1()) {
        case 'f':
            mat.autofacet = Material::FACETED;
            break;
        case 'a':
            mat.autofacet = Material::AUTOFACETED;
            break;
        case 's':
            mat.autofacet = Material::SMOOTH;
            break;
        default:
            ASSIMP_LOG_ERROR_F( "Unrecognized faceting mode in `Mat1` chunk with id ", nfo.id );
            mat.autofacet = Material::FACETED;
    }
    mat.autofacet_angle = static_cast<float>(reader.GetI1());

    mat.rgb.r = reader.GetF4();
    mat.rgb.g = reader.GetF4();
    mat.rgb.b = reader.GetF4();

    mat.alpha = reader.GetF4();
    mat.ka    = reader.GetF4();
    mat.ks    = reader.GetF4();
    mat.exp   = reader.GetF4();
    mat.ior   = reader.GetF4();

    char id[2];
    id[0] = reader.GetI1(),id[1] = reader.GetI1();

    if (id[0] == 'e' && id[1] == ':') {
        mat.tex_env.reset(new Texture());

        reader.GetI1();
        ReadString_Binary(mat.tex_env->path,reader);

        // advance to next texture-id
        id[0] = reader.GetI1(),id[1] = reader.GetI1();
    }

    if (id[0] == 't' && id[1] == ':') {
        mat.tex_color.reset(new Texture());

        reader.GetI1();
        ReadString_Binary(mat.tex_color->path,reader);

        mat.tex_color->transform.mTranslation.x = reader.GetF4();
        mat.tex_color->transform.mTranslation.y = reader.GetF4();

        mat.tex_color->transform.mScaling.x = reader.GetF4();
        mat.tex_color->transform.mScaling.y = reader.GetF4();

        // advance to next texture-id
        id[0] = reader.GetI1(),id[1] = reader.GetI1();
    }

    if (id[0] == 'b' && id[1] == ':') {
        mat.tex_bump.reset(new Texture());

        reader.GetI1();
        ReadString_Binary(mat.tex_bump->path,reader);

        mat.tex_bump->transform.mTranslation.x = reader.GetF4();
        mat.tex_bump->transform.mTranslation.y = reader.GetF4();

        mat.tex_bump->transform.mScaling.x = reader.GetF4();
        mat.tex_bump->transform.mScaling.y = reader.GetF4();

        // skip amplitude for I don't know its purpose.
        reader.GetF4();
    }
    reader.IncPtr(-2);
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadCame_Binary(COB::Scene& out, StreamReaderLE& reader, const ChunkInfo& nfo)
{
    if(nfo.version > 2) {
        return UnsupportedChunk_Binary(reader,nfo,"Came");
    }

    const chunk_guard cn(nfo,reader);

    out.nodes.push_back(std::shared_ptr<Camera>(new Camera()));
    Camera& msh = (Camera&)(*out.nodes.back().get());
    msh = nfo;

    ReadBasicNodeInfo_Binary(msh,reader,nfo);

    // the rest is not interesting for us, so we skip over it.
    if(nfo.version > 1) {
        if (reader.GetI2()==512) {
            reader.IncPtr(42);
        }
    }
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadLght_Binary(COB::Scene& out, StreamReaderLE& reader, const ChunkInfo& nfo)
{
    if(nfo.version > 2) {
        return UnsupportedChunk_Binary(reader,nfo,"Lght");
    }

    const chunk_guard cn(nfo,reader);

    out.nodes.push_back(std::shared_ptr<Light>(new Light()));
    Light& msh = (Light&)(*out.nodes.back().get());
    msh = nfo;

    ReadBasicNodeInfo_Binary(msh,reader,nfo);
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadGrou_Binary(COB::Scene& out, StreamReaderLE& reader, const ChunkInfo& nfo)
{
    if(nfo.version > 2) {
        return UnsupportedChunk_Binary(reader,nfo,"Grou");
    }

    const chunk_guard cn(nfo,reader);

    out.nodes.push_back(std::shared_ptr<Group>(new Group()));
    Group& msh = (Group&)(*out.nodes.back().get());
    msh = nfo;

    ReadBasicNodeInfo_Binary(msh,reader,nfo);
}

// ------------------------------------------------------------------------------------------------
void COBImporter::ReadUnit_Binary(COB::Scene& out, StreamReaderLE& reader, const ChunkInfo& nfo)
{
     if(nfo.version > 1) {
        return UnsupportedChunk_Binary(reader,nfo,"Unit");
    }

     const chunk_guard cn(nfo,reader);

    // parent chunks preceede their childs, so we should have the
    // corresponding chunk already.
    for(std::shared_ptr< Node >& nd : out.nodes) {
        if (nd->id == nfo.parent_id) {
            const unsigned int t=reader.GetI2();
            nd->unit_scale = t>=sizeof(units)/sizeof(units[0])?(
                ASSIMP_LOG_WARN_F(t," is not a valid value for `Units` attribute in `Unit chunk` ", nfo.id)
                ,1.f):units[t];

            return;
        }
    }
    ASSIMP_LOG_WARN_F( "`Unit` chunk ", nfo.id, " is a child of ", nfo.parent_id, " which does not exist");
}

#endif // ASSIMP_BUILD_NO_COB_IMPORTER
