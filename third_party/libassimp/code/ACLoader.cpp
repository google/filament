
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

/** @file Implementation of the AC3D importer class */



#ifndef ASSIMP_BUILD_NO_AC_IMPORTER

// internal headers
#include "ACLoader.h"
#include "ParsingUtils.h"
#include "fast_atof.h"
#include "Subdivision.h"
#include "Importer.h"
#include "BaseImporter.h"
#include <assimp/Importer.hpp>
#include <assimp/light.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/config.h>
#include <assimp/IOSystem.hpp>
#include <assimp/importerdesc.h>
#include <memory>

using namespace Assimp;

static const aiImporterDesc desc = {
    "AC3D Importer",
    "",
    "",
    "",
    aiImporterFlags_SupportTextFlavour,
    0,
    0,
    0,
    0,
    "ac acc ac3d"
};

// ------------------------------------------------------------------------------------------------
// skip to the next token
#define AI_AC_SKIP_TO_NEXT_TOKEN() \
    if (!SkipSpaces(&buffer)) \
    { \
        DefaultLogger::get()->error("AC3D: Unexpected EOF/EOL"); \
        continue; \
    }

// ------------------------------------------------------------------------------------------------
// read a string (may be enclosed in double quotation marks). buffer must point to "
#define AI_AC_GET_STRING(out) \
    if (*buffer == '\0') { \
        throw DeadlyImportError("AC3D: Unexpected EOF in string"); \
    } \
    ++buffer; \
    const char* sz = buffer; \
    while ('\"' != *buffer) \
    { \
        if (IsLineEnd( *buffer )) \
        { \
            DefaultLogger::get()->error("AC3D: Unexpected EOF/EOL in string"); \
            out = "ERROR"; \
            break; \
        } \
        ++buffer; \
    } \
    if (IsLineEnd( *buffer ))continue; \
    out = std::string(sz,(unsigned int)(buffer-sz)); \
    ++buffer;


// ------------------------------------------------------------------------------------------------
// read 1 to n floats prefixed with an optional predefined identifier
#define AI_AC_CHECKED_LOAD_FLOAT_ARRAY(name,name_length,num,out) \
    AI_AC_SKIP_TO_NEXT_TOKEN(); \
    if (name_length) \
    { \
        if (strncmp(buffer,name,name_length) || !IsSpace(buffer[name_length])) \
        { \
            DefaultLogger::get()->error("AC3D: Unexpexted token. " name " was expected."); \
            continue; \
        } \
        buffer += name_length+1; \
    } \
    for (unsigned int i = 0; i < num;++i) \
    { \
        AI_AC_SKIP_TO_NEXT_TOKEN(); \
        buffer = fast_atoreal_move<float>(buffer,((float*)out)[i]); \
    }


// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
AC3DImporter::AC3DImporter()
    : buffer(),
    configSplitBFCull(),
    configEvalSubdivision(),
    mNumMeshes(),
    mLights(),
    lights(),
    groups(),
    polys(),
    worlds()
{
    // nothing to be done here
}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
AC3DImporter::~AC3DImporter()
{
    // nothing to be done here
}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool AC3DImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    std::string extension = GetExtension(pFile);

    // fixme: are acc and ac3d *really* used? Some sources say they are
    if(extension == "ac" || extension == "ac3d" || extension == "acc") {
        return true;
    }
    if (!extension.length() || checkSig) {
        uint32_t token = AI_MAKE_MAGIC("AC3D");
        return CheckMagicToken(pIOHandler,pFile,&token,1,0);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Loader meta information
const aiImporterDesc* AC3DImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Get a pointer to the next line from the file
bool AC3DImporter::GetNextLine( )
{
    SkipLine(&buffer);
    return SkipSpaces(&buffer);
}

// ------------------------------------------------------------------------------------------------
// Parse an object section in an AC file
void AC3DImporter::LoadObjectSection(std::vector<Object>& objects)
{
    if (!TokenMatch(buffer,"OBJECT",6))
        return;

    SkipSpaces(&buffer);

    ++mNumMeshes;

    objects.push_back(Object());
    Object& obj = objects.back();

    aiLight* light = NULL;
    if (!ASSIMP_strincmp(buffer,"light",5))
    {
        // This is a light source. Add it to the list
        mLights->push_back(light = new aiLight());

        // Return a point light with no attenuation
        light->mType = aiLightSource_POINT;
        light->mColorDiffuse = light->mColorSpecular = aiColor3D(1.f,1.f,1.f);
        light->mAttenuationConstant = 1.f;

        // Generate a default name for both the light source and the node
        // FIXME - what's the right way to print a size_t? Is 'zu' universally available? stick with the safe version.
        light->mName.length = ::ai_snprintf(light->mName.data, MAXLEN, "ACLight_%i",static_cast<unsigned int>(mLights->size())-1);
        obj.name = std::string( light->mName.data );

        DefaultLogger::get()->debug("AC3D: Light source encountered");
        obj.type = Object::Light;
    }
    else if (!ASSIMP_strincmp(buffer,"group",5))
    {
        obj.type = Object::Group;
    }
    else if (!ASSIMP_strincmp(buffer,"world",5))
    {
        obj.type = Object::World;
    }
    else obj.type = Object::Poly;
    while (GetNextLine())
    {
        if (TokenMatch(buffer,"kids",4))
        {
            SkipSpaces(&buffer);
            unsigned int num = strtoul10(buffer,&buffer);
            GetNextLine();
            if (num)
            {
                // load the children of this object recursively
                obj.children.reserve(num);
                for (unsigned int i = 0; i < num; ++i)
                    LoadObjectSection(obj.children);
            }
            return;
        }
        else if (TokenMatch(buffer,"name",4))
        {
            SkipSpaces(&buffer);
            AI_AC_GET_STRING(obj.name);

            // If this is a light source, we'll also need to store
            // the name of the node in it.
            if (light)
            {
                light->mName.Set(obj.name);
            }
        }
        else if (TokenMatch(buffer,"texture",7))
        {
            SkipSpaces(&buffer);
            AI_AC_GET_STRING(obj.texture);
        }
        else if (TokenMatch(buffer,"texrep",6))
        {
            SkipSpaces(&buffer);
            AI_AC_CHECKED_LOAD_FLOAT_ARRAY("",0,2,&obj.texRepeat);
            if (!obj.texRepeat.x || !obj.texRepeat.y)
                obj.texRepeat = aiVector2D (1.f,1.f);
        }
        else if (TokenMatch(buffer,"texoff",6))
        {
            SkipSpaces(&buffer);
            AI_AC_CHECKED_LOAD_FLOAT_ARRAY("",0,2,&obj.texOffset);
        }
        else if (TokenMatch(buffer,"rot",3))
        {
            SkipSpaces(&buffer);
            AI_AC_CHECKED_LOAD_FLOAT_ARRAY("",0,9,&obj.rotation);
        }
        else if (TokenMatch(buffer,"loc",3))
        {
            SkipSpaces(&buffer);
            AI_AC_CHECKED_LOAD_FLOAT_ARRAY("",0,3,&obj.translation);
        }
        else if (TokenMatch(buffer,"subdiv",6))
        {
            SkipSpaces(&buffer);
            obj.subDiv = strtoul10(buffer,&buffer);
        }
        else if (TokenMatch(buffer,"crease",6))
        {
            SkipSpaces(&buffer);
            obj.crease = fast_atof(buffer);
        }
        else if (TokenMatch(buffer,"numvert",7))
        {
            SkipSpaces(&buffer);

            unsigned int t = strtoul10(buffer,&buffer);
            if (t >= AI_MAX_ALLOC(aiVector3D)) {
                throw DeadlyImportError("AC3D: Too many vertices, would run out of memory");
            }
            obj.vertices.reserve(t);
            for (unsigned int i = 0; i < t;++i)
            {
                if (!GetNextLine())
                {
                    DefaultLogger::get()->error("AC3D: Unexpected EOF: not all vertices have been parsed yet");
                    break;
                }
                else if (!IsNumeric(*buffer))
                {
                    DefaultLogger::get()->error("AC3D: Unexpected token: not all vertices have been parsed yet");
                    --buffer; // make sure the line is processed a second time
                    break;
                }
                obj.vertices.push_back(aiVector3D());
                aiVector3D& v = obj.vertices.back();
                AI_AC_CHECKED_LOAD_FLOAT_ARRAY("",0,3,&v.x);
            }
        }
        else if (TokenMatch(buffer,"numsurf",7))
        {
            SkipSpaces(&buffer);

            bool Q3DWorkAround = false;

            const unsigned int t = strtoul10(buffer,&buffer);
            obj.surfaces.reserve(t);
            for (unsigned int i = 0; i < t;++i)
            {
                GetNextLine();
                if (!TokenMatch(buffer,"SURF",4))
                {
                    // FIX: this can occur for some files - Quick 3D for
                    // example writes no surf chunks
                    if (!Q3DWorkAround)
                    {
                        DefaultLogger::get()->warn("AC3D: SURF token was expected");
                        DefaultLogger::get()->debug("Continuing with Quick3D Workaround enabled");
                    }
                    --buffer; // make sure the line is processed a second time
                    // break; --- see fix notes above

                    Q3DWorkAround = true;
                }
                SkipSpaces(&buffer);
                obj.surfaces.push_back(Surface());
                Surface& surf = obj.surfaces.back();
                surf.flags = strtoul_cppstyle(buffer);

                while (1)
                {
                    if(!GetNextLine())
                    {
                        throw DeadlyImportError("AC3D: Unexpected EOF: surface is incomplete");
                    }
                    if (TokenMatch(buffer,"mat",3))
                    {
                        SkipSpaces(&buffer);
                        surf.mat = strtoul10(buffer);
                    }
                    else if (TokenMatch(buffer,"refs",4))
                    {
                        // --- see fix notes above
                        if (Q3DWorkAround)
                        {
                            if (!surf.entries.empty())
                            {
                                buffer -= 6;
                                break;
                            }
                        }

                        SkipSpaces(&buffer);
                        const unsigned int m = strtoul10(buffer);
                        surf.entries.reserve(m);

                        obj.numRefs += m;

                        for (unsigned int k = 0; k < m; ++k)
                        {
                            if(!GetNextLine())
                            {
                                DefaultLogger::get()->error("AC3D: Unexpected EOF: surface references are incomplete");
                                break;
                            }
                            surf.entries.push_back(Surface::SurfaceEntry());
                            Surface::SurfaceEntry& entry = surf.entries.back();

                            entry.first = strtoul10(buffer,&buffer);
                            SkipSpaces(&buffer);
                            AI_AC_CHECKED_LOAD_FLOAT_ARRAY("",0,2,&entry.second);
                        }
                    }
                    else
                    {

                        --buffer; // make sure the line is processed a second time
                        break;
                    }
                }
            }
        }
    }
    DefaultLogger::get()->error("AC3D: Unexpected EOF: \'kids\' line was expected");
}

// ------------------------------------------------------------------------------------------------
// Convert a material from AC3DImporter::Material to aiMaterial
void AC3DImporter::ConvertMaterial(const Object& object,
    const Material& matSrc,
    aiMaterial& matDest)
{
    aiString s;

    if (matSrc.name.length())
    {
        s.Set(matSrc.name);
        matDest.AddProperty(&s,AI_MATKEY_NAME);
    }
    if (object.texture.length())
    {
        s.Set(object.texture);
        matDest.AddProperty(&s,AI_MATKEY_TEXTURE_DIFFUSE(0));

        // UV transformation
        if (1.f != object.texRepeat.x || 1.f != object.texRepeat.y ||
            object.texOffset.x        || object.texOffset.y)
        {
            aiUVTransform transform;
            transform.mScaling = object.texRepeat;
            transform.mTranslation = object.texOffset;
            matDest.AddProperty(&transform,1,AI_MATKEY_UVTRANSFORM_DIFFUSE(0));
        }
    }

    matDest.AddProperty<aiColor3D>(&matSrc.rgb,1, AI_MATKEY_COLOR_DIFFUSE);
    matDest.AddProperty<aiColor3D>(&matSrc.amb,1, AI_MATKEY_COLOR_AMBIENT);
    matDest.AddProperty<aiColor3D>(&matSrc.emis,1,AI_MATKEY_COLOR_EMISSIVE);
    matDest.AddProperty<aiColor3D>(&matSrc.spec,1,AI_MATKEY_COLOR_SPECULAR);

    int n;
    if (matSrc.shin)
    {
        n = aiShadingMode_Phong;
        matDest.AddProperty<float>(&matSrc.shin,1,AI_MATKEY_SHININESS);
    }
    else n = aiShadingMode_Gouraud;
    matDest.AddProperty<int>(&n,1,AI_MATKEY_SHADING_MODEL);

    float f = 1.f - matSrc.trans;
    matDest.AddProperty<float>(&f,1,AI_MATKEY_OPACITY);
}

// ------------------------------------------------------------------------------------------------
// Converts the loaded data to the internal verbose representation
aiNode* AC3DImporter::ConvertObjectSection(Object& object,
    std::vector<aiMesh*>& meshes,
    std::vector<aiMaterial*>& outMaterials,
    const std::vector<Material>& materials,
    aiNode* parent)
{
    aiNode* node = new aiNode();
    node->mParent = parent;
    if (object.vertices.size())
    {
        if (!object.surfaces.size() || !object.numRefs)
        {
            /* " An object with 7 vertices (no surfaces, no materials defined).
                 This is a good way of getting point data into AC3D.
                 The Vertex->create convex-surface/object can be used on these
                 vertices to 'wrap' a 3d shape around them "
                 (http://www.opencity.info/html/ac3dfileformat.html)

                 therefore: if no surfaces are defined return point data only
             */

            DefaultLogger::get()->info("AC3D: No surfaces defined in object definition, "
                "a point list is returned");

            meshes.push_back(new aiMesh());
            aiMesh* mesh = meshes.back();

            mesh->mNumFaces = mesh->mNumVertices = (unsigned int)object.vertices.size();
            aiFace* faces = mesh->mFaces = new aiFace[mesh->mNumFaces];
            aiVector3D* verts = mesh->mVertices = new aiVector3D[mesh->mNumVertices];

            for (unsigned int i = 0; i < mesh->mNumVertices;++i,++faces,++verts)
            {
                *verts = object.vertices[i];
                faces->mNumIndices = 1;
                faces->mIndices = new unsigned int[1];
                faces->mIndices[0] = i;
            }

            // use the primary material in this case. this should be the
            // default material if all objects of the file contain points
            // and no faces.
            mesh->mMaterialIndex = 0;
            outMaterials.push_back(new aiMaterial());
            ConvertMaterial(object, materials[0], *outMaterials.back());
        }
        else
        {
            // need to generate one or more meshes for this object.
            // find out how many different materials we have
            typedef std::pair< unsigned int, unsigned int > IntPair;
            typedef std::vector< IntPair > MatTable;
            MatTable needMat(materials.size(),IntPair(0,0));

            std::vector<Surface>::iterator it,end = object.surfaces.end();
            std::vector<Surface::SurfaceEntry>::iterator it2,end2;

            for (it = object.surfaces.begin(); it != end; ++it)
            {
                unsigned int idx = (*it).mat;
                if (idx >= needMat.size())
                {
                    DefaultLogger::get()->error("AC3D: material index is out of range");
                    idx = 0;
                }
                if ((*it).entries.empty())
                {
                    DefaultLogger::get()->warn("AC3D: surface her zero vertex references");
                }

                // validate all vertex indices to make sure we won't crash here
                for (it2  = (*it).entries.begin(),
                     end2 = (*it).entries.end(); it2 != end2; ++it2)
                {
                    if ((*it2).first >= object.vertices.size())
                    {
                        DefaultLogger::get()->warn("AC3D: Invalid vertex reference");
                        (*it2).first = 0;
                    }
                }

                if (!needMat[idx].first)++node->mNumMeshes;

                switch ((*it).flags & 0xf)
                {
                    // closed line
                case 0x1:

                    needMat[idx].first  += (unsigned int)(*it).entries.size();
                    needMat[idx].second += (unsigned int)(*it).entries.size()<<1u;
                    break;

                    // unclosed line
                case 0x2:

                    needMat[idx].first  += (unsigned int)(*it).entries.size()-1;
                    needMat[idx].second += ((unsigned int)(*it).entries.size()-1)<<1u;
                    break;

                    // 0 == polygon, else unknown
                default:

                    if ((*it).flags & 0xf)
                    {
                        DefaultLogger::get()->warn("AC3D: The type flag of a surface is unknown");
                        (*it).flags &= ~(0xf);
                    }

                    // the number of faces increments by one, the number
                    // of vertices by surface.numref.
                    needMat[idx].first++;
                    needMat[idx].second += (unsigned int)(*it).entries.size();
                };
            }
            unsigned int* pip = node->mMeshes = new unsigned int[node->mNumMeshes];
            unsigned int mat = 0;
            const size_t oldm = meshes.size();
            for (MatTable::const_iterator cit = needMat.begin(), cend = needMat.end();
                cit != cend; ++cit, ++mat)
            {
                if (!(*cit).first)continue;

                // allocate a new aiMesh object
                *pip++ = (unsigned int)meshes.size();
                aiMesh* mesh = new aiMesh();
                meshes.push_back(mesh);

                mesh->mMaterialIndex = (unsigned int)outMaterials.size();
                outMaterials.push_back(new aiMaterial());
                ConvertMaterial(object, materials[mat], *outMaterials.back());

                // allocate storage for vertices and normals
                mesh->mNumFaces = (*cit).first;
                if (mesh->mNumFaces == 0) {
                    throw DeadlyImportError("AC3D: No faces");
                } else if (mesh->mNumFaces > AI_MAX_ALLOC(aiFace)) {
                    throw DeadlyImportError("AC3D: Too many faces, would run out of memory");
                }
                aiFace* faces = mesh->mFaces = new aiFace[mesh->mNumFaces];

                mesh->mNumVertices = (*cit).second;
                if (mesh->mNumVertices == 0) {
                    throw DeadlyImportError("AC3D: No vertices");
                } else if (mesh->mNumVertices > AI_MAX_ALLOC(aiVector3D)) {
                    throw DeadlyImportError("AC3D: Too many vertices, would run out of memory");
                }
                aiVector3D* vertices = mesh->mVertices = new aiVector3D[mesh->mNumVertices];
                unsigned int cur = 0;

                // allocate UV coordinates, but only if the texture name for the
                // surface is not empty
                aiVector3D* uv = NULL;
                if(object.texture.length())
                {
                    uv = mesh->mTextureCoords[0] = new aiVector3D[mesh->mNumVertices];
                    mesh->mNumUVComponents[0] = 2;
                }

                for (it = object.surfaces.begin(); it != end; ++it)
                {
                    if (mat == (*it).mat)
                    {
                        const Surface& src = *it;

                        // closed polygon
                        unsigned int type = (*it).flags & 0xf;
                        if (!type)
                        {
                            aiFace& face = *faces++;
                            if((face.mNumIndices = (unsigned int)src.entries.size()))
                            {
                                face.mIndices = new unsigned int[face.mNumIndices];
                                for (unsigned int i = 0; i < face.mNumIndices;++i,++vertices)
                                {
                                    const Surface::SurfaceEntry& entry = src.entries[i];
                                    face.mIndices[i] = cur++;

                                    // copy vertex positions
                                    if (static_cast<unsigned>(vertices - mesh->mVertices) >= mesh->mNumVertices) {
                                        throw DeadlyImportError("AC3D: Invalid number of vertices");
                                    }
                                    *vertices = object.vertices[entry.first] + object.translation;


                                    // copy texture coordinates
                                    if (uv)
                                    {
                                        uv->x =  entry.second.x;
                                        uv->y =  entry.second.y;
                                        ++uv;
                                    }
                                }
                            }
                        }
                        else
                        {

                            it2  = (*it).entries.begin();

                            // either a closed or an unclosed line
                            unsigned int tmp = (unsigned int)(*it).entries.size();
                            if (0x2 == type)--tmp;
                            for (unsigned int m = 0; m < tmp;++m)
                            {
                                aiFace& face = *faces++;

                                face.mNumIndices = 2;
                                face.mIndices = new unsigned int[2];
                                face.mIndices[0] = cur++;
                                face.mIndices[1] = cur++;

                                // copy vertex positions
                                if (it2 == (*it).entries.end() ) {
                                    throw DeadlyImportError("AC3D: Bad line");
                                }
                                ai_assert((*it2).first < object.vertices.size());
                                *vertices++ = object.vertices[(*it2).first];

                                // copy texture coordinates
                                if (uv)
                                {
                                    uv->x =  (*it2).second.x;
                                    uv->y =  (*it2).second.y;
                                    ++uv;
                                }


                                if (0x1 == type && tmp-1 == m)
                                {
                                    // if this is a closed line repeat its beginning now
                                    it2  = (*it).entries.begin();
                                }
                                else ++it2;

                                // second point
                                *vertices++ = object.vertices[(*it2).first];

                                if (uv)
                                {
                                    uv->x =  (*it2).second.x;
                                    uv->y =  (*it2).second.y;
                                    ++uv;
                                }
                            }
                        }
                    }
                }
            }

            // Now apply catmull clark subdivision if necessary. We split meshes into
            // materials which is not done by AC3D during smoothing, so we need to
            // collect all meshes using the same material group.
            if (object.subDiv)  {
                if (configEvalSubdivision) {
                    std::unique_ptr<Subdivider> div(Subdivider::Create(Subdivider::CATMULL_CLARKE));
                    DefaultLogger::get()->info("AC3D: Evaluating subdivision surface: "+object.name);

                    std::vector<aiMesh*> cpy(meshes.size()-oldm,NULL);
                    div->Subdivide(&meshes[oldm],cpy.size(),&cpy.front(),object.subDiv,true);
                    std::copy(cpy.begin(),cpy.end(),meshes.begin()+oldm);

                    // previous meshes are deleted vy Subdivide().
                }
                else {
                    DefaultLogger::get()->info("AC3D: Letting the subdivision surface untouched due to my configuration: "
                        +object.name);
                }
            }
        }
    }

    if (object.name.length())
        node->mName.Set(object.name);
    else
    {
        // generate a name depending on the type of the node
        switch (object.type)
        {
        case Object::Group:
            node->mName.length = ::ai_snprintf(node->mName.data, MAXLEN, "ACGroup_%i",groups++);
            break;
        case Object::Poly:
            node->mName.length = ::ai_snprintf(node->mName.data, MAXLEN, "ACPoly_%i",polys++);
            break;
        case Object::Light:
            node->mName.length = ::ai_snprintf(node->mName.data, MAXLEN, "ACLight_%i",lights++);
            break;

            // there shouldn't be more than one world, but we don't care
        case Object::World:
            node->mName.length = ::ai_snprintf(node->mName.data, MAXLEN, "ACWorld_%i",worlds++);
            break;
        }
    }


    // setup the local transformation matrix of the object
    // compute the transformation offset to the parent node
    node->mTransformation = aiMatrix4x4 ( object.rotation );

    if (object.type == Object::Group || !object.numRefs)
    {
        node->mTransformation.a4 = object.translation.x;
        node->mTransformation.b4 = object.translation.y;
        node->mTransformation.c4 = object.translation.z;
    }

    // add children to the object
    if (object.children.size())
    {
        node->mNumChildren = (unsigned int)object.children.size();
        node->mChildren = new aiNode*[node->mNumChildren];
        for (unsigned int i = 0; i < node->mNumChildren;++i)
        {
            node->mChildren[i] = ConvertObjectSection(object.children[i],meshes,outMaterials,materials,node);
        }
    }

    return node;
}

// ------------------------------------------------------------------------------------------------
void AC3DImporter::SetupProperties(const Importer* pImp)
{
    configSplitBFCull = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_AC_SEPARATE_BFCULL,1) ? true : false;
    configEvalSubdivision =  pImp->GetPropertyInteger(AI_CONFIG_IMPORT_AC_EVAL_SUBDIVISION,1) ? true : false;
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void AC3DImporter::InternReadFile( const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile, "rb"));

    // Check whether we can read from the file
    if( file.get() == NULL)
        throw DeadlyImportError( "Failed to open AC3D file " + pFile + ".");

    // allocate storage and copy the contents of the file to a memory buffer
    std::vector<char> mBuffer2;
    TextFileToBuffer(file.get(),mBuffer2);

    buffer = &mBuffer2[0];
    mNumMeshes = 0;

    lights = polys = worlds = groups = 0;

    if (::strncmp(buffer,"AC3D",4)) {
        throw DeadlyImportError("AC3D: No valid AC3D file, magic sequence not found");
    }

    // print the file format version to the console
    unsigned int version = HexDigitToDecimal( buffer[4] );
    char msg[3];
    ASSIMP_itoa10(msg,3,version);
    DefaultLogger::get()->info(std::string("AC3D file format version: ") + msg);

    std::vector<Material> materials;
    materials.reserve(5);

    std::vector<Object> rootObjects;
    rootObjects.reserve(5);

    std::vector<aiLight*> lights;
    mLights = & lights;

    while (GetNextLine())
    {
        if (TokenMatch(buffer,"MATERIAL",8))
        {
            materials.push_back(Material());
            Material& mat = materials.back();

            // manually parse the material ... sscanf would use the buldin atof ...
            // Format: (name) rgb %f %f %f  amb %f %f %f  emis %f %f %f  spec %f %f %f  shi %d  trans %f

            AI_AC_SKIP_TO_NEXT_TOKEN();
            if ('\"' == *buffer)
            {
                AI_AC_GET_STRING(mat.name);
                AI_AC_SKIP_TO_NEXT_TOKEN();
            }

            AI_AC_CHECKED_LOAD_FLOAT_ARRAY("rgb",3,3,&mat.rgb);
            AI_AC_CHECKED_LOAD_FLOAT_ARRAY("amb",3,3,&mat.amb);
            AI_AC_CHECKED_LOAD_FLOAT_ARRAY("emis",4,3,&mat.emis);
            AI_AC_CHECKED_LOAD_FLOAT_ARRAY("spec",4,3,&mat.spec);
            AI_AC_CHECKED_LOAD_FLOAT_ARRAY("shi",3,1,&mat.shin);
            AI_AC_CHECKED_LOAD_FLOAT_ARRAY("trans",5,1,&mat.trans);
        }
        LoadObjectSection(rootObjects);
    }

    if (rootObjects.empty() || !mNumMeshes)
    {
        throw DeadlyImportError("AC3D: No meshes have been loaded");
    }
    if (materials.empty())
    {
        DefaultLogger::get()->warn("AC3D: No material has been found");
        materials.push_back(Material());
    }

    mNumMeshes += (mNumMeshes>>2u) + 1;
    std::vector<aiMesh*> meshes;
    meshes.reserve(mNumMeshes);

    std::vector<aiMaterial*> omaterials;
    materials.reserve(mNumMeshes);

    // generate a dummy root if there are multiple objects on the top layer
    Object* root;
    if (1 == rootObjects.size())
        root = &rootObjects[0];
    else
    {
        root = new Object();
    }

    // now convert the imported stuff to our output data structure
    pScene->mRootNode = ConvertObjectSection(*root,meshes,omaterials,materials);
    if (1 != rootObjects.size())delete root;

    if (!::strncmp( pScene->mRootNode->mName.data, "Node", 4))
        pScene->mRootNode->mName.Set("<AC3DWorld>");

    // copy meshes
    if (meshes.empty())
    {
        throw DeadlyImportError("An unknown error occurred during converting");
    }
    pScene->mNumMeshes = (unsigned int)meshes.size();
    pScene->mMeshes = new aiMesh*[pScene->mNumMeshes];
    ::memcpy(pScene->mMeshes,&meshes[0],pScene->mNumMeshes*sizeof(void*));

    // copy materials
    pScene->mNumMaterials = (unsigned int)omaterials.size();
    pScene->mMaterials = new aiMaterial*[pScene->mNumMaterials];
    ::memcpy(pScene->mMaterials,&omaterials[0],pScene->mNumMaterials*sizeof(void*));

    // copy lights
    pScene->mNumLights = (unsigned int)lights.size();
    if (lights.size())
    {
        pScene->mLights = new aiLight*[lights.size()];
        ::memcpy(pScene->mLights,&lights[0],lights.size()*sizeof(void*));
    }
}

#endif //!defined ASSIMP_BUILD_NO_AC_IMPORTER
