/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2012, assimp team
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

/** @file  C4DImporter.cpp
 *  @brief Implementation of the Cinema4D importer class.
 */
#ifndef ASSIMP_BUILD_NO_C4D_IMPORTER

// no #ifdefing here, Cinema4D support is carried out in a branch of assimp
// where it is turned on in the CMake settings.

#ifndef _MSC_VER
#   error C4D support is currently MSVC only
#endif

#include "C4DImporter.h"
#include "TinyFormatter.h"
#include <memory>
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>
#include <assimp/ai_assert.h>

#if defined(_M_X64) || defined(__amd64__)
#   define __C4D_64BIT
#endif

#define __PC
#include "c4d_file.h"
#include "default_alien_overloads.h"

using namespace melange;

// overload this function and fill in your own unique data
void GetWriterInfo(int &id, String &appname)
{
    id = 2424226;
    appname = "Open Asset Import Library";
}

using namespace Assimp;
using namespace Assimp::Formatter;

namespace Assimp {
    template<> const std::string LogFunctions<C4DImporter>::log_prefix = "C4D: ";
}

static const aiImporterDesc desc = {
    "Cinema4D Importer",
    "",
    "",
    "",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "c4d"
};


// ------------------------------------------------------------------------------------------------
C4DImporter::C4DImporter()
{}

// ------------------------------------------------------------------------------------------------
C4DImporter::~C4DImporter()
{}

// ------------------------------------------------------------------------------------------------
bool C4DImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    const std::string& extension = GetExtension(pFile);
    if (extension == "c4d") {
        return true;
    }

    else if ((!extension.length() || checkSig) && pIOHandler)   {
        // TODO
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
const aiImporterDesc* C4DImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
void C4DImporter::SetupProperties(const Importer* /*pImp*/)
{
    // nothing to be done for the moment
}


// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void C4DImporter::InternReadFile( const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile));

    if( file.get() == NULL) {
        ThrowException("failed to open file " + pFile);
    }

    const size_t file_size = file->FileSize();

    std::vector<uint8_t> mBuffer(file_size);
    file->Read(&mBuffer[0], 1, file_size);

    Filename f;
    f.SetMemoryReadMode(&mBuffer[0], file_size);

    // open document first
    BaseDocument* doc = LoadDocument(f, SCENEFILTER_OBJECTS | SCENEFILTER_MATERIALS);
    if(doc == NULL) {
        ThrowException("failed to read document " + pFile);
    }

    pScene->mRootNode = new aiNode("<C4DRoot>");

    // first convert all materials
    ReadMaterials(doc->GetFirstMaterial());

    // process C4D scenegraph recursively
    try {
        RecurseHierarchy(doc->GetFirstObject(), pScene->mRootNode);
    }
    catch(...) {
        for(aiMesh* mesh : meshes) {
            delete mesh;
        }
        BaseDocument::Free(doc);
        throw;
    }
    BaseDocument::Free(doc);

    // copy meshes over
    pScene->mNumMeshes = static_cast<unsigned int>(meshes.size());
    pScene->mMeshes = new aiMesh*[pScene->mNumMeshes]();
    std::copy(meshes.begin(), meshes.end(), pScene->mMeshes);

    // copy materials over, adding a default material if necessary
    unsigned int mat_count = static_cast<unsigned int>(materials.size());
    for(aiMesh* mesh : meshes) {
        ai_assert(mesh->mMaterialIndex <= mat_count);
        if(mesh->mMaterialIndex >= mat_count) {
            ++mat_count;

            std::unique_ptr<aiMaterial> def_material(new aiMaterial());
            const aiString name(AI_DEFAULT_MATERIAL_NAME);
            def_material->AddProperty(&name, AI_MATKEY_NAME);

            materials.push_back(def_material.release());
            break;
        }
    }

    pScene->mNumMaterials = static_cast<unsigned int>(materials.size());
    pScene->mMaterials = new aiMaterial*[pScene->mNumMaterials]();
    std::copy(materials.begin(), materials.end(), pScene->mMaterials);
}


// ------------------------------------------------------------------------------------------------
bool C4DImporter::ReadShader(aiMaterial* out, melange::BaseShader* shader)
{
    // based on Melange sample code (C4DImportExport.cpp)
    while(shader) {
        if(shader->GetType() == Xlayer) {
            BaseContainer* container = shader->GetDataInstance();
            GeData blend = container->GetData(SLA_LAYER_BLEND);
            iBlendDataType* blend_list = reinterpret_cast<iBlendDataType*>(blend.GetCustomDataType(CUSTOMDATA_BLEND_LIST));
            if (!blend_list)
            {
                LogWarn("ignoring XLayer shader: no blend list given");
                continue;
            }

            LayerShaderLayer *lsl = dynamic_cast<LayerShaderLayer*>(blend_list->m_BlendLayers.GetObject(0));

            // Ignore the actual layer blending - models for real-time rendering should not
            // use them in a non-trivial way. Just try to find textures that we can apply
            // to the model.
            while (lsl)
            {
                if (lsl->GetType() == TypeFolder)
                {
                    BlendFolder* const folder = dynamic_cast<BlendFolder*>(lsl);
                    LayerShaderLayer *subLsl = dynamic_cast<LayerShaderLayer*>(folder->m_Children.GetObject(0));

                    while (subLsl)
                    {
                        if (subLsl->GetType() == TypeShader) {
                            BlendShader* const shader = dynamic_cast<BlendShader*>(subLsl);
                            if(ReadShader(out, static_cast<BaseShader*>(shader->m_pLink->GetLink()))) {
                                return true;
                            }
                        }

                        subLsl = subLsl->GetNext();
                    }
                }
                else if (lsl->GetType() == TypeShader) {
                    BlendShader* const shader = dynamic_cast<BlendShader*>(lsl);
                    if(ReadShader(out, static_cast<BaseShader*>(shader->m_pLink->GetLink()))) {
                        return true;
                    }
                }

                lsl = lsl->GetNext();
            }
        }
        else if ( shader->GetType() == Xbitmap )
        {
            aiString path;
            shader->GetFileName().GetString().GetCString(path.data, MAXLEN-1);
            path.length = ::strlen(path.data);
            out->AddProperty(&path, AI_MATKEY_TEXTURE_DIFFUSE(0));
            return true;
        }
        else {
            LogWarn("ignoring shader type: " + std::string(GetObjectTypeName(shader->GetType())));
        }
        shader = shader->GetNext();
    }
    return false;
}


// ------------------------------------------------------------------------------------------------
void C4DImporter::ReadMaterials(melange::BaseMaterial* mat)
{
    // based on Melange sample code
    while (mat)
    {
        const String& name = mat->GetName();
        if (mat->GetType() == Mmaterial)
        {
            aiMaterial* out = new aiMaterial();
            material_mapping[mat] = static_cast<unsigned int>(materials.size());
            materials.push_back(out);

            aiString ai_name;
            name.GetCString(ai_name.data, MAXLEN-1);
            ai_name.length = ::strlen(ai_name.data);
            out->AddProperty(&ai_name, AI_MATKEY_NAME);

            Material& m = dynamic_cast<Material&>(*mat);

            if (m.GetChannelState(CHANNEL_COLOR))
            {
                GeData data;
                mat->GetParameter(MATERIAL_COLOR_COLOR, data);
                Vector color = data.GetVector();
                mat->GetParameter(MATERIAL_COLOR_BRIGHTNESS, data);
                const Float brightness = data.GetFloat();

                color *= brightness;

                aiVector3D v;
                v.x = color.x;
                v.y = color.y;
                v.z = color.z;
                out->AddProperty(&v, 1, AI_MATKEY_COLOR_DIFFUSE);
            }

            BaseShader* const shader = m.GetShader(MATERIAL_COLOR_SHADER);
            if(shader) {
                ReadShader(out, shader);
            }
        }
        else
        {
            LogWarn("ignoring plugin material: " + std::string(GetObjectTypeName(mat->GetType())));
        }
        mat = mat->GetNext();
    }
}

// ------------------------------------------------------------------------------------------------
void C4DImporter::RecurseHierarchy(BaseObject* object, aiNode* parent)
{
    ai_assert(parent != NULL);
    std::vector<aiNode*> nodes;

    // based on Melange sample code
    while (object)
    {
        const String& name = object->GetName();
        const LONG type = object->GetType();
        const Matrix& ml = object->GetMl();

        aiString string;
        name.GetCString(string.data, MAXLEN-1);
        string.length = ::strlen(string.data);
        aiNode* const nd = new aiNode();

        nd->mParent = parent;
        nd->mName = string;

        nd->mTransformation.a1 = ml.v1.x;
        nd->mTransformation.b1 = ml.v1.y;
        nd->mTransformation.c1 = ml.v1.z;

        nd->mTransformation.a2 = ml.v2.x;
        nd->mTransformation.b2 = ml.v2.y;
        nd->mTransformation.c2 = ml.v2.z;

        nd->mTransformation.a3 = ml.v3.x;
        nd->mTransformation.b3 = ml.v3.y;
        nd->mTransformation.c3 = ml.v3.z;

        nd->mTransformation.a4 = ml.off.x;
        nd->mTransformation.b4 = ml.off.y;
        nd->mTransformation.c4 = ml.off.z;

        nodes.push_back(nd);

        GeData data;
        if (type == Ocamera)
        {
            object->GetParameter(CAMERAOBJECT_FOV, data);
            // TODO: read camera
        }
        else if (type == Olight)
        {
            // TODO: read light
        }
        else if (type == Opolygon)
        {
            aiMesh* const mesh = ReadMesh(object);
            if(mesh != NULL) {
                nd->mNumMeshes = 1;
                nd->mMeshes = new unsigned int[1];
                nd->mMeshes[0] = static_cast<unsigned int>(meshes.size());
                meshes.push_back(mesh);
            }
        }
        else {
            LogWarn("ignoring object: " + std::string(GetObjectTypeName(type)));
        }

        RecurseHierarchy(object->GetDown(), nd);
        object = object->GetNext();
    }

    // copy nodes over to parent
    parent->mNumChildren = static_cast<unsigned int>(nodes.size());
    parent->mChildren = new aiNode*[parent->mNumChildren]();
    std::copy(nodes.begin(), nodes.end(), parent->mChildren);
}


// ------------------------------------------------------------------------------------------------
aiMesh* C4DImporter::ReadMesh(BaseObject* object)
{
    ai_assert(object != NULL && object->GetType() == Opolygon);

    // based on Melange sample code
    PolygonObject* const polyObject = dynamic_cast<PolygonObject*>(object);
    ai_assert(polyObject != NULL);

    const LONG pointCount = polyObject->GetPointCount();
    const LONG polyCount = polyObject->GetPolygonCount();
    if(!polyObject || !pointCount) {
        LogWarn("ignoring mesh with zero vertices or faces");
        return NULL;
    }

    const Vector* points = polyObject->GetPointR();
    ai_assert(points != NULL);

    const CPolygon* polys = polyObject->GetPolygonR();
    ai_assert(polys != NULL);

    std::unique_ptr<aiMesh> mesh(new aiMesh());
    mesh->mNumFaces = static_cast<unsigned int>(polyCount);
    aiFace* face = mesh->mFaces = new aiFace[mesh->mNumFaces]();

    mesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
    mesh->mMaterialIndex = 0;

    unsigned int vcount = 0;

    // first count vertices
    for (LONG i = 0; i < polyCount; i++)
    {
        vcount += 3;

        // TODO: do we also need to handle lines or points with similar checks?
        if (polys[i].c != polys[i].d)
        {
            mesh->mPrimitiveTypes |= aiPrimitiveType_POLYGON;
            ++vcount;
        }
    }

    ai_assert(vcount > 0);

    mesh->mNumVertices = vcount;
    aiVector3D* verts = mesh->mVertices = new aiVector3D[mesh->mNumVertices];
    aiVector3D* normals, *uvs, *tangents, *bitangents;
    unsigned int n = 0;

    // check if there are normals, tangents or UVW coordinates
    BaseTag* tag = object->GetTag(Tnormal);
    NormalTag* normals_src = NULL;
    if(tag) {
        normals_src = dynamic_cast<NormalTag*>(tag);
        normals = mesh->mNormals = new aiVector3D[mesh->mNumVertices]();
    }

    tag = object->GetTag(Ttangent);
    TangentTag* tangents_src = NULL;
    if(tag) {
        tangents_src = dynamic_cast<TangentTag*>(tag);
        tangents = mesh->mTangents = new aiVector3D[mesh->mNumVertices]();
        bitangents = mesh->mBitangents = new aiVector3D[mesh->mNumVertices]();
    }

    tag = object->GetTag(Tuvw);
    UVWTag* uvs_src = NULL;
    if(tag) {
        uvs_src = dynamic_cast<UVWTag*>(tag);
        uvs = mesh->mTextureCoords[0] = new aiVector3D[mesh->mNumVertices]();
    }

    // copy vertices and extra channels over and populate faces
    for (LONG i = 0; i < polyCount; ++i, ++face)
    {
        ai_assert(polys[i].a < pointCount && polys[i].a >= 0);
        const Vector& pointA = points[polys[i].a];
        verts->x = pointA.x;
        verts->y = pointA.y;
        verts->z = pointA.z;
        ++verts;

        ai_assert(polys[i].b < pointCount && polys[i].b >= 0);
        const Vector& pointB = points[polys[i].b];
        verts->x = pointB.x;
        verts->y = pointB.y;
        verts->z = pointB.z;
        ++verts;

        ai_assert(polys[i].c < pointCount && polys[i].c >= 0);
        const Vector& pointC = points[polys[i].c];
        verts->x = pointC.x;
        verts->y = pointC.y;
        verts->z = pointC.z;
        ++verts;

        // TODO: do we also need to handle lines or points with similar checks?
        if (polys[i].c != polys[i].d)
        {
            ai_assert(polys[i].d < pointCount && polys[i].d >= 0);

            face->mNumIndices = 4;
            mesh->mPrimitiveTypes |= aiPrimitiveType_POLYGON;
            const Vector& pointD = points[polys[i].d];
            verts->x = pointD.x;
            verts->y = pointD.y;
            verts->z = pointD.z;
            ++verts;
        }
        else {
            face->mNumIndices = 3;
        }
        face->mIndices = new unsigned int[face->mNumIndices];
        for(unsigned int j = 0; j < face->mNumIndices; ++j) {
            face->mIndices[j] = n++;
        }

        // copy normals
        if (normals_src) {
            if(i >= normals_src->GetDataCount()) {
                LogError("unexpected number of normals, ignoring");
            }
            else {
                ConstNormalHandle normal_handle = normals_src->GetDataAddressR();
                NormalStruct nor;
                NormalTag::Get(normal_handle, i, nor);
                normals->x = nor.a.x;
                normals->y = nor.a.y;
                normals->z = nor.a.z;
                ++normals;

                normals->x = nor.b.x;
                normals->y = nor.b.y;
                normals->z = nor.b.z;
                ++normals;

                normals->x = nor.c.x;
                normals->y = nor.c.y;
                normals->z = nor.c.z;
                ++normals;

                if(face->mNumIndices == 4) {
                    normals->x = nor.d.x;
                    normals->y = nor.d.y;
                    normals->z = nor.d.z;
                    ++normals;
                }
            }
        }

        // copy tangents and bitangents
        if (tangents_src) {

            for(unsigned int k = 0; k < face->mNumIndices; ++k) {
                LONG l;
                switch(k) {
                case 0:
                    l = polys[i].a;
                    break;
                case 1:
                    l = polys[i].b;
                    break;
                case 2:
                    l = polys[i].c;
                    break;
                case 3:
                    l = polys[i].d;
                    break;
                default:
                    ai_assert(false);
                }
                if(l >= tangents_src->GetDataCount()) {
                    LogError("unexpected number of tangents, ignoring");
                    break;
                }

                Tangent tan = tangents_src->GetDataR()[l];
                tangents->x = tan.vl.x;
                tangents->y = tan.vl.y;
                tangents->z = tan.vl.z;
                ++tangents;

                bitangents->x = tan.vr.x;
                bitangents->y = tan.vr.y;
                bitangents->z = tan.vr.z;
                ++bitangents;
            }
        }

        // copy UVs
        if (uvs_src) {
            if(i >= uvs_src->GetDataCount()) {
                LogError("unexpected number of UV coordinates, ignoring");
            }
            else {
                UVWStruct uvw;
                uvs_src->Get(uvs_src->GetDataAddressR(),i,uvw);

                uvs->x = uvw.a.x;
                uvs->y = 1.0f-uvw.a.y;
                uvs->z = uvw.a.z;
                ++uvs;

                uvs->x = uvw.b.x;
                uvs->y = 1.0f-uvw.b.y;
                uvs->z = uvw.b.z;
                ++uvs;

                uvs->x = uvw.c.x;
                uvs->y = 1.0f-uvw.c.y;
                uvs->z = uvw.c.z;
                ++uvs;

                if(face->mNumIndices == 4) {
                    uvs->x = uvw.d.x;
                    uvs->y = 1.0f-uvw.d.y;
                    uvs->z = uvw.d.z;
                    ++uvs;
                }
            }
        }
    }

    mesh->mMaterialIndex = ResolveMaterial(polyObject);
    return mesh.release();
}


// ------------------------------------------------------------------------------------------------
unsigned int C4DImporter::ResolveMaterial(PolygonObject* obj)
{
    ai_assert(obj != NULL);

    const unsigned int mat_count = static_cast<unsigned int>(materials.size());

    BaseTag* tag = obj->GetTag(Ttexture);
    if(tag == NULL) {
        return mat_count;
    }

    TextureTag& ttag = dynamic_cast<TextureTag&>(*tag);

    BaseMaterial* const mat = ttag.GetMaterial();
    ai_assert(mat != NULL);

    const MaterialMap::const_iterator it = material_mapping.find(mat);
    if(it == material_mapping.end()) {
        return mat_count;
    }

    ai_assert((*it).second < mat_count);
    return (*it).second;
}

#endif // ASSIMP_BUILD_NO_C4D_IMPORTER

