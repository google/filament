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

/** @file Implementation of the XGL/ZGL importer class */


#ifndef ASSIMP_BUILD_NO_XGL_IMPORTER

#include "XGLLoader.h"
#include "ParsingUtils.h"
#include "fast_atof.h"

#include "StreamReader.h"
#include "MemoryIOWrapper.h"
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/importerdesc.h>
#include <cctype>
#include <memory>

using namespace Assimp;
using namespace irr;
using namespace irr::io;

// zlib is needed for compressed XGL files
#ifndef ASSIMP_BUILD_NO_COMPRESSED_XGL
#   ifdef ASSIMP_BUILD_NO_OWN_ZLIB
#       include <zlib.h>
#   else
#       include <contrib/zlib/zlib.h>
#   endif
#endif


// scopeguard for a malloc'ed buffer
struct free_it
{
    free_it(void* free) : free(free) {}
    ~free_it() {
        ::free(this->free);
    }

    void* free;
};

namespace Assimp { // this has to be in here because LogFunctions is in ::Assimp
    template<> const char* LogFunctions<XGLImporter>::Prefix()
    {
        static auto prefix = "XGL: ";
        return prefix;
    }
}

static const aiImporterDesc desc = {
    "XGL Importer",
    "",
    "",
    "",
    aiImporterFlags_SupportTextFlavour | aiImporterFlags_SupportCompressedFlavour,
    0,
    0,
    0,
    0,
    "xgl zgl"
};


// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
XGLImporter::XGLImporter()
: m_reader( nullptr )
, m_scene( nullptr ) {
    // empty
}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
XGLImporter::~XGLImporter() {
    // empty
}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool XGLImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    /* NOTE: A simple check for the file extension is not enough
     * here. XGL and ZGL are ok, but xml is too generic
     * and might be collada as well. So open the file and
     * look for typical signal tokens.
     */
    const std::string extension = GetExtension(pFile);

    if (extension == "xgl" || extension == "zgl") {
        return true;
    }
    else if (extension == "xml" || checkSig) {
        ai_assert(pIOHandler != NULL);

        const char* tokens[] = {"<world>","<World>","<WORLD>"};
        return SearchFileHeaderForToken(pIOHandler,pFile,tokens,3);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Get a list of all file extensions which are handled by this class
const aiImporterDesc* XGLImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void XGLImporter::InternReadFile( const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
#ifndef ASSIMP_BUILD_NO_COMPRESSED_XGL
    Bytef* dest = NULL;
    free_it free_it_really(dest);
#endif

    m_scene = pScene;
    std::shared_ptr<IOStream> stream( pIOHandler->Open( pFile, "rb"));

    // check whether we can read from the file
    if( stream.get() == NULL) {
        throw DeadlyImportError( "Failed to open XGL/ZGL file " + pFile + "");
    }

    // see if its compressed, if so uncompress it
    if (GetExtension(pFile) == "zgl") {
#ifdef ASSIMP_BUILD_NO_COMPRESSED_XGL
        ThrowException("Cannot read ZGL file since Assimp was built without compression support");
#else
        std::unique_ptr<StreamReaderLE> raw_reader(new StreamReaderLE(stream));

        // build a zlib stream
        z_stream zstream;
        zstream.opaque = Z_NULL;
        zstream.zalloc = Z_NULL;
        zstream.zfree  = Z_NULL;
        zstream.data_type = Z_BINARY;

        // raw decompression without a zlib or gzip header
        inflateInit2(&zstream, -MAX_WBITS);

        // skip two extra bytes, zgl files do carry a crc16 upfront (I think)
        raw_reader->IncPtr(2);

        zstream.next_in   = reinterpret_cast<Bytef*>( raw_reader->GetPtr() );
        zstream.avail_in  = raw_reader->GetRemainingSize();

        size_t total = 0l;

        // and decompress the data .... do 1k chunks in the hope that we won't kill the stack
    #define MYBLOCK 1024
        Bytef block[MYBLOCK];
        int ret;
        do {
            zstream.avail_out = MYBLOCK;
            zstream.next_out = block;
            ret = inflate(&zstream, Z_NO_FLUSH);

            if (ret != Z_STREAM_END && ret != Z_OK) {
                ThrowException("Failure decompressing this file using gzip, seemingly it is NOT a compressed .XGL file");
            }
            const size_t have = MYBLOCK - zstream.avail_out;
            total += have;
            dest = reinterpret_cast<Bytef*>( realloc(dest,total) );
            memcpy(dest + total - have,block,have);
        }
        while (ret != Z_STREAM_END);

        // terminate zlib
        inflateEnd(&zstream);

        // replace the input stream with a memory stream
        stream.reset(new MemoryIOStream(reinterpret_cast<uint8_t*>(dest),total));
#endif
    }

    // construct the irrXML parser
    CIrrXML_IOStreamReader st(stream.get());
    m_reader.reset( createIrrXMLReader( ( IFileReadCallBack* ) &st ) );

    // parse the XML file
    TempScope scope;

    while (ReadElement())   {
        if (!ASSIMP_stricmp(m_reader->getNodeName(),"world")) {
            ReadWorld(scope);
        }
    }


    std::vector<aiMesh*>& meshes = scope.meshes_linear;
    std::vector<aiMaterial*>& materials = scope.materials_linear;
    if(!meshes.size() || !materials.size()) {
        ThrowException("failed to extract data from XGL file, no meshes loaded");
    }

    // copy meshes
    m_scene->mNumMeshes = static_cast<unsigned int>(meshes.size());
    m_scene->mMeshes = new aiMesh*[m_scene->mNumMeshes]();
    std::copy(meshes.begin(),meshes.end(),m_scene->mMeshes);

    // copy materials
    m_scene->mNumMaterials = static_cast<unsigned int>(materials.size());
    m_scene->mMaterials = new aiMaterial*[m_scene->mNumMaterials]();
    std::copy(materials.begin(),materials.end(),m_scene->mMaterials);

    if (scope.light) {
        m_scene->mNumLights = 1;
        m_scene->mLights = new aiLight*[1];
        m_scene->mLights[0] = scope.light;

        scope.light->mName = m_scene->mRootNode->mName;
    }

    scope.dismiss();
}

// ------------------------------------------------------------------------------------------------
bool XGLImporter::ReadElement()
{
    while(m_reader->read()) {
        if (m_reader->getNodeType() == EXN_ELEMENT) {
            return true;
        }
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
bool XGLImporter::ReadElementUpToClosing(const char* closetag)
{
    while(m_reader->read()) {
        if (m_reader->getNodeType() == EXN_ELEMENT) {
            return true;
        }
        else if (m_reader->getNodeType() == EXN_ELEMENT_END && !ASSIMP_stricmp(m_reader->getNodeName(),closetag)) {
            return false;
        }
    }
    LogError("unexpected EOF, expected closing <" + std::string(closetag) + "> tag");
    return false;
}

// ------------------------------------------------------------------------------------------------
bool XGLImporter::SkipToText()
{
    while(m_reader->read()) {
        if (m_reader->getNodeType() == EXN_TEXT) {
            return true;
        }
        else if (m_reader->getNodeType() == EXN_ELEMENT || m_reader->getNodeType() == EXN_ELEMENT_END) {
            ThrowException("expected text contents but found another element (or element end)");
        }
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
std::string XGLImporter::GetElementName()
{
    const char* s  = m_reader->getNodeName();
    size_t len = strlen(s);

    std::string ret;
    ret.resize(len);

    std::transform(s,s+len,ret.begin(),::tolower);
    return ret;
}

// ------------------------------------------------------------------------------------------------
void XGLImporter::ReadWorld(TempScope& scope)
{
    while (ReadElementUpToClosing("world")) {
        const std::string& s = GetElementName();
        // XXX right now we'd skip <lighting> if it comes after
        // <object> or <mesh>
        if (s == "lighting") {
            ReadLighting(scope);
        }
        else if (s == "object" || s == "mesh" || s == "mat") {
            break;
        }
    }


    aiNode* const nd = ReadObject(scope,true,"world");
    if(!nd) {
        ThrowException("failure reading <world>");
    }
    if(!nd->mName.length) {
        nd->mName.Set("WORLD");
    }

    m_scene->mRootNode = nd;
}

// ------------------------------------------------------------------------------------------------
void XGLImporter::ReadLighting(TempScope& scope)
{
    while (ReadElementUpToClosing("lighting"))  {
        const std::string& s = GetElementName();
        if (s == "directionallight") {
            scope.light = ReadDirectionalLight();
        }
        else if (s == "ambient") {
            LogWarn("ignoring <ambient> tag");
        }
        else if (s == "spheremap") {
            LogWarn("ignoring <spheremap> tag");
        }
    }
}

// ------------------------------------------------------------------------------------------------
aiLight* XGLImporter::ReadDirectionalLight()
{
    std::unique_ptr<aiLight> l(new aiLight());
    l->mType = aiLightSource_DIRECTIONAL;

    while (ReadElementUpToClosing("directionallight"))  {
        const std::string& s = GetElementName();
        if (s == "direction") {
            l->mDirection = ReadVec3();
        }
        else if (s == "diffuse") {
            l->mColorDiffuse = ReadCol3();
        }
        else if (s == "specular") {
            l->mColorSpecular = ReadCol3();
        }
    }
    return l.release();
}

// ------------------------------------------------------------------------------------------------
aiNode* XGLImporter::ReadObject(TempScope& scope, bool skipFirst, const char* closetag)
{
    std::unique_ptr<aiNode> nd(new aiNode());
    std::vector<aiNode*> children;
    std::vector<unsigned int> meshes;

    try {
        while (skipFirst || ReadElementUpToClosing(closetag))   {
            skipFirst = false;

            const std::string& s = GetElementName();
            if (s == "mesh") {
                const size_t prev = scope.meshes_linear.size();
                if(ReadMesh(scope)) {
                    const size_t newc = scope.meshes_linear.size();
                    for(size_t i = 0; i < newc-prev; ++i) {
                        meshes.push_back(static_cast<unsigned int>(i+prev));
                    }
                }
            }
            else if (s == "mat") {
                ReadMaterial(scope);
            }
            else if (s == "object") {
                children.push_back(ReadObject(scope));
            }
            else if (s == "objectref") {
                // XXX
            }
            else if (s == "meshref") {
                const unsigned int id = static_cast<unsigned int>( ReadIndexFromText() );

                std::multimap<unsigned int, aiMesh*>::iterator it = scope.meshes.find(id), end = scope.meshes.end();
                if (it == end) {
                    ThrowException("<meshref> index out of range");
                }

                for(; it != end && (*it).first == id; ++it) {
                    // ok, this is n^2 and should get optimized one day
                    aiMesh* const m = (*it).second;

                    unsigned int i = 0, mcount = static_cast<unsigned int>(scope.meshes_linear.size());
                    for(; i < mcount; ++i) {
                        if (scope.meshes_linear[i] == m) {
                            meshes.push_back(i);
                            break;
                        }
                    }

                    ai_assert(i < mcount);
                }
            }
            else if (s == "transform") {
                nd->mTransformation = ReadTrafo();
            }
        }

    } catch(...) {
        for(aiNode* ch : children) {
            delete ch;
        }
        throw;
    }

    // FIX: since we used std::multimap<> to keep meshes by id, mesh order now depends on the behaviour
    // of the multimap implementation with respect to the ordering of entries with same values.
    // C++11 gives the guarantee that it uses insertion order, before it is implementation-specific.
    // Sort by material id to always guarantee a deterministic result.
    std::sort(meshes.begin(), meshes.end(), SortMeshByMaterialId(scope));

    // link meshes to node
    nd->mNumMeshes = static_cast<unsigned int>(meshes.size());
    if (nd->mNumMeshes) {
        nd->mMeshes = new unsigned int[nd->mNumMeshes]();
        for(unsigned int i = 0; i < nd->mNumMeshes; ++i) {
            nd->mMeshes[i] = meshes[i];
        }
    }

    // link children to parent
    nd->mNumChildren = static_cast<unsigned int>(children.size());
    if (nd->mNumChildren) {
        nd->mChildren = new aiNode*[nd->mNumChildren]();
        for(unsigned int i = 0; i < nd->mNumChildren; ++i) {
            nd->mChildren[i] = children[i];
            children[i]->mParent = nd.get();
        }
    }

    return nd.release();
}

// ------------------------------------------------------------------------------------------------
aiMatrix4x4 XGLImporter::ReadTrafo()
{
    aiVector3D forward, up, right, position;
    float scale = 1.0f;

    while (ReadElementUpToClosing("transform")) {
        const std::string& s = GetElementName();
        if (s == "forward") {
            forward = ReadVec3();
        }
        else if (s == "up") {
            up = ReadVec3();
        }
        else if (s == "position") {
            position = ReadVec3();
        }
        if (s == "scale") {
            scale = ReadFloat();
            if(scale < 0.f) {
                // this is wrong, but we can leave the value and pass it to the caller
                LogError("found negative scaling in <transform>, ignoring");
            }
        }
    }

    aiMatrix4x4 m;
    if(forward.SquareLength() < 1e-4 || up.SquareLength() < 1e-4) {
        LogError("A direction vector in <transform> is zero, ignoring trafo");
        return m;
    }

    forward.Normalize();
    up.Normalize();

    right = forward ^ up;
    if (std::fabs(up * forward) > 1e-4) {
        // this is definitely wrong - a degenerate coordinate space ruins everything
        // so subtitute identity transform.
        LogError("<forward> and <up> vectors in <transform> are skewing, ignoring trafo");
        return m;
    }

    right *= scale;
    up *= scale;
    forward *= scale;

    m.a1 = right.x;
    m.b1 = right.y;
    m.c1 = right.z;

    m.a2 = up.x;
    m.b2 = up.y;
    m.c2 = up.z;

    m.a3 = forward.x;
    m.b3 = forward.y;
    m.c3 = forward.z;

    m.a4 = position.x;
    m.b4 = position.y;
    m.c4 = position.z;

    return m;
}

// ------------------------------------------------------------------------------------------------
aiMesh* XGLImporter::ToOutputMesh(const TempMaterialMesh& m)
{
    std::unique_ptr<aiMesh> mesh(new aiMesh());

    mesh->mNumVertices = static_cast<unsigned int>(m.positions.size());
    mesh->mVertices = new aiVector3D[mesh->mNumVertices];
    std::copy(m.positions.begin(),m.positions.end(),mesh->mVertices);

    if(m.normals.size()) {
        mesh->mNormals = new aiVector3D[mesh->mNumVertices];
        std::copy(m.normals.begin(),m.normals.end(),mesh->mNormals);
    }

    if(m.uvs.size()) {
        mesh->mNumUVComponents[0] = 2;
        mesh->mTextureCoords[0] = new aiVector3D[mesh->mNumVertices];

        for(unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            mesh->mTextureCoords[0][i] = aiVector3D(m.uvs[i].x,m.uvs[i].y,0.f);
        }
    }

    mesh->mNumFaces =  static_cast<unsigned int>(m.vcounts.size());
    mesh->mFaces = new aiFace[m.vcounts.size()];

    unsigned int idx = 0;
    for(unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace& f = mesh->mFaces[i];
        f.mNumIndices = m.vcounts[i];
        f.mIndices = new unsigned int[f.mNumIndices];
        for(unsigned int c = 0; c < f.mNumIndices; ++c) {
            f.mIndices[c] = idx++;
        }
    }

    ai_assert(idx == mesh->mNumVertices);

    mesh->mPrimitiveTypes = m.pflags;
    mesh->mMaterialIndex = m.matid;
    return mesh.release();
}

// ------------------------------------------------------------------------------------------------
bool XGLImporter::ReadMesh(TempScope& scope)
{
    TempMesh t;

    std::map<unsigned int, TempMaterialMesh> bymat;
    const unsigned int mesh_id = ReadIDAttr();

    while (ReadElementUpToClosing("mesh"))  {
        const std::string& s = GetElementName();

        if (s == "mat") {
            ReadMaterial(scope);
        }
        else if (s == "p") {
            if (!m_reader->getAttributeValue("ID")) {
                LogWarn("no ID attribute on <p>, ignoring");
            }
            else {
                int id = m_reader->getAttributeValueAsInt("ID");
                t.points[id] = ReadVec3();
            }
        }
        else if (s == "n") {
            if (!m_reader->getAttributeValue("ID")) {
                LogWarn("no ID attribute on <n>, ignoring");
            }
            else {
                int id = m_reader->getAttributeValueAsInt("ID");
                t.normals[id] = ReadVec3();
            }
        }
        else if (s == "tc") {
            if (!m_reader->getAttributeValue("ID")) {
                LogWarn("no ID attribute on <tc>, ignoring");
            }
            else {
                int id = m_reader->getAttributeValueAsInt("ID");
                t.uvs[id] = ReadVec2();
            }
        }
        else if (s == "f" || s == "l" || s == "p") {
            const unsigned int vcount = s == "f" ? 3 : (s == "l" ? 2 : 1);

            unsigned int mid = ~0u;
            TempFace tf[3];
            bool has[3] = {0};

            while (ReadElementUpToClosing(s.c_str()))   {
                const std::string& s = GetElementName();
                if (s == "fv1" || s == "lv1" || s == "pv1") {
                    ReadFaceVertex(t,tf[0]);
                    has[0] = true;
                }
                else if (s == "fv2" || s == "lv2") {
                    ReadFaceVertex(t,tf[1]);
                    has[1] = true;
                }
                else if (s == "fv3") {
                    ReadFaceVertex(t,tf[2]);
                    has[2] = true;
                }
                else if (s == "mat") {
                    if (mid != ~0u) {
                        LogWarn("only one material tag allowed per <f>");
                    }
                    mid = ResolveMaterialRef(scope);
                }
                else if (s == "matref") {
                    if (mid != ~0u) {
                        LogWarn("only one material tag allowed per <f>");
                    }
                    mid = ResolveMaterialRef(scope);
                }
            }

            if (mid == ~0u) {
                ThrowException("missing material index");
            }

            bool nor = false;
            bool uv = false;
            for(unsigned int i = 0; i < vcount; ++i) {
                if (!has[i]) {
                    ThrowException("missing face vertex data");
                }

                nor = nor || tf[i].has_normal;
                uv = uv || tf[i].has_uv;
            }

            if (mid >= (1<<30)) {
                LogWarn("material indices exhausted, this may cause errors in the output");
            }
            unsigned int meshId = mid | ((nor?1:0)<<31) | ((uv?1:0)<<30);

            TempMaterialMesh& mesh = bymat[meshId];
            mesh.matid = mid;

            for(unsigned int i = 0; i < vcount; ++i) {
                mesh.positions.push_back(tf[i].pos);
                if(nor) {
                    mesh.normals.push_back(tf[i].normal);
                }
                if(uv) {
                    mesh.uvs.push_back(tf[i].uv);
                }

                mesh.pflags |= 1 << (vcount-1);
            }

            mesh.vcounts.push_back(vcount);
        }
    }

    // finally extract output meshes and add them to the scope
    typedef std::pair<unsigned int, TempMaterialMesh> pairt;
    for(const pairt& p : bymat) {
        aiMesh* const m  = ToOutputMesh(p.second);
        scope.meshes_linear.push_back(m);

        // if this is a definition, keep it on the stack
        if(mesh_id != ~0u) {
            scope.meshes.insert(std::pair<unsigned int, aiMesh*>(mesh_id,m));
        }
    }

    // no id == not a reference, insert this mesh right *here*
    return mesh_id == ~0u;
}

// ----------------------------------------------------------------------------------------------
unsigned int XGLImporter::ResolveMaterialRef(TempScope& scope)
{
    const std::string& s = GetElementName();
    if (s == "mat") {
        ReadMaterial(scope);
        return static_cast<unsigned int>(scope.materials_linear.size()-1);
    }

    const int id = ReadIndexFromText();

    std::map<unsigned int, aiMaterial*>::iterator it = scope.materials.find(id), end = scope.materials.end();
    if (it == end) {
        ThrowException("<matref> index out of range");
    }

    // ok, this is n^2 and should get optimized one day
    aiMaterial* const m = (*it).second;

    unsigned int i = 0, mcount = static_cast<unsigned int>(scope.materials_linear.size());
    for(; i < mcount; ++i) {
        if (scope.materials_linear[i] == m) {
            return i;
        }
    }

    ai_assert(false);
    return 0;
}

// ------------------------------------------------------------------------------------------------
void XGLImporter::ReadMaterial(TempScope& scope)
{
    const unsigned int mat_id = ReadIDAttr();

    std::unique_ptr<aiMaterial> mat(new aiMaterial());
    while (ReadElementUpToClosing("mat"))  {
        const std::string& s = GetElementName();
        if (s == "amb") {
            const aiColor3D c = ReadCol3();
            mat->AddProperty(&c,1,AI_MATKEY_COLOR_AMBIENT);
        }
        else if (s == "diff") {
            const aiColor3D c = ReadCol3();
            mat->AddProperty(&c,1,AI_MATKEY_COLOR_DIFFUSE);
        }
        else if (s == "spec") {
            const aiColor3D c = ReadCol3();
            mat->AddProperty(&c,1,AI_MATKEY_COLOR_SPECULAR);
        }
        else if (s == "emiss") {
            const aiColor3D c = ReadCol3();
            mat->AddProperty(&c,1,AI_MATKEY_COLOR_EMISSIVE);
        }
        else if (s == "alpha") {
            const float f = ReadFloat();
            mat->AddProperty(&f,1,AI_MATKEY_OPACITY);
        }
        else if (s == "shine") {
            const float f = ReadFloat();
            mat->AddProperty(&f,1,AI_MATKEY_SHININESS);
        }
    }

    scope.materials[mat_id] = mat.get();
    scope.materials_linear.push_back(mat.release());
}


// ----------------------------------------------------------------------------------------------
void XGLImporter::ReadFaceVertex(const TempMesh& t, TempFace& out)
{
    const std::string& end = GetElementName();

    bool havep = false;
    while (ReadElementUpToClosing(end.c_str()))  {
        const std::string& s = GetElementName();
        if (s == "pref") {
            const unsigned int id = ReadIndexFromText();
            std::map<unsigned int, aiVector3D>::const_iterator it = t.points.find(id);
            if (it == t.points.end()) {
                ThrowException("point index out of range");
            }

            out.pos = (*it).second;
            havep = true;
        }
        else if (s == "nref") {
            const unsigned int id = ReadIndexFromText();
            std::map<unsigned int, aiVector3D>::const_iterator it = t.normals.find(id);
            if (it == t.normals.end()) {
                ThrowException("normal index out of range");
            }

            out.normal = (*it).second;
            out.has_normal = true;
        }
        else if (s == "tcref") {
            const unsigned int id = ReadIndexFromText();
            std::map<unsigned int, aiVector2D>::const_iterator it = t.uvs.find(id);
            if (it == t.uvs.end()) {
                ThrowException("uv index out of range");
            }

            out.uv = (*it).second;
            out.has_uv = true;
        }
        else if (s == "p") {
            out.pos = ReadVec3();
        }
        else if (s == "n") {
            out.normal = ReadVec3();
        }
        else if (s == "tc") {
            out.uv = ReadVec2();
        }
    }

    if (!havep) {
        ThrowException("missing <pref> in <fvN> element");
    }
}

// ------------------------------------------------------------------------------------------------
unsigned int XGLImporter::ReadIDAttr()
{
    for(int i = 0, e = m_reader->getAttributeCount(); i < e; ++i) {

        if(!ASSIMP_stricmp(m_reader->getAttributeName(i),"id")) {
            return m_reader->getAttributeValueAsInt(i);
        }
    }
    return ~0u;
}

// ------------------------------------------------------------------------------------------------
float XGLImporter::ReadFloat()
{
    if(!SkipToText()) {
        LogError("unexpected EOF reading float element contents");
        return 0.f;
    }
    const char* s = m_reader->getNodeData(), *se;

    if(!SkipSpaces(&s)) {
        LogError("unexpected EOL, failed to parse float");
        return 0.f;
    }

    float t;
    se = fast_atoreal_move(s,t);

    if (se == s) {
        LogError("failed to read float text");
        return 0.f;
    }

    return t;
}

// ------------------------------------------------------------------------------------------------
unsigned int XGLImporter::ReadIndexFromText()
{
    if(!SkipToText()) {
        LogError("unexpected EOF reading index element contents");
        return ~0u;
    }
    const char* s = m_reader->getNodeData(), *se;
    if(!SkipSpaces(&s)) {
        LogError("unexpected EOL, failed to parse index element");
        return ~0u;
    }

    const unsigned int t = strtoul10(s,&se);

    if (se == s) {
        LogError("failed to read index");
        return ~0u;
    }

    return t;
}

// ------------------------------------------------------------------------------------------------
aiVector2D XGLImporter::ReadVec2()
{
    aiVector2D vec;

    if(!SkipToText()) {
        LogError("unexpected EOF reading vec2 contents");
        return vec;
    }
    const char* s = m_reader->getNodeData();

    for(int i = 0; i < 2; ++i) {
        if(!SkipSpaces(&s)) {
            LogError("unexpected EOL, failed to parse vec2");
            return vec;
        }
        vec[i] = fast_atof(&s);

        SkipSpaces(&s);
        if (i != 1 && *s != ',') {
            LogError("expected comma, failed to parse vec2");
            return vec;
        }
        ++s;
    }

    return vec;
}

// ------------------------------------------------------------------------------------------------
aiVector3D XGLImporter::ReadVec3()
{
    aiVector3D vec;

    if(!SkipToText()) {
        LogError("unexpected EOF reading vec3 contents");
        return vec;
    }
    const char* s = m_reader->getNodeData();

    for(int i = 0; i < 3; ++i) {
        if(!SkipSpaces(&s)) {
            LogError("unexpected EOL, failed to parse vec3");
            return vec;
        }
        vec[i] = fast_atof(&s);

        SkipSpaces(&s);
        if (i != 2 && *s != ',') {
            LogError("expected comma, failed to parse vec3");
            return vec;
        }
        ++s;
    }

    return vec;
}

// ------------------------------------------------------------------------------------------------
aiColor3D XGLImporter::ReadCol3()
{
    const aiVector3D& v = ReadVec3();
    if (v.x < 0.f || v.x > 1.0f || v.y < 0.f || v.y > 1.0f || v.z < 0.f || v.z > 1.0f) {
        LogWarn("color values out of range, ignoring");
    }
    return aiColor3D(v.x,v.y,v.z);
}

#endif
