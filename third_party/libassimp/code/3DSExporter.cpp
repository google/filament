/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team

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

#ifndef ASSIMP_BUILD_NO_EXPORT
#ifndef ASSIMP_BUILD_NO_3DS_EXPORTER

#include "3DSExporter.h"
#include "3DSLoader.h"
#include "3DSHelper.h"
#include <assimp/SceneCombiner.h>
#include "SplitLargeMeshes.h"
#include "StringComparison.h"
#include <assimp/IOSystem.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/Exporter.hpp>
#include <memory>

using namespace Assimp;
namespace Assimp    {
using namespace D3DS;

namespace {

    //////////////////////////////////////////////////////////////////////////////////////
    // Scope utility to write a 3DS file chunk.
    //
    // Upon construction, the chunk header is written with the chunk type (flags)
    // filled out, but the chunk size left empty. Upon destruction, the correct chunk
    // size based on the then-position of the output stream cursor is filled in.
    class ChunkWriter {
        enum {
              CHUNK_SIZE_NOT_SET = 0xdeadbeef
            , SIZE_OFFSET        = 2
        };
    public:

        ChunkWriter(StreamWriterLE& writer, uint16_t chunk_type)
            : writer(writer)
        {
            chunk_start_pos = writer.GetCurrentPos();
            writer.PutU2(chunk_type);
            writer.PutU4(CHUNK_SIZE_NOT_SET);
        }

        ~ChunkWriter() {
            std::size_t head_pos = writer.GetCurrentPos();

            ai_assert(head_pos > chunk_start_pos);
            const std::size_t chunk_size = head_pos - chunk_start_pos;

            writer.SetCurrentPos(chunk_start_pos + SIZE_OFFSET);
            writer.PutU4(static_cast<uint32_t>(chunk_size));
            writer.SetCurrentPos(head_pos);
        }

    private:
        StreamWriterLE& writer;
        std::size_t chunk_start_pos;
    };


    // Return an unique name for a given |mesh| attached to |node| that
    // preserves the mesh's given name if it has one. |index| is the index
    // of the mesh in |aiScene::mMeshes|.
    std::string GetMeshName(const aiMesh& mesh, unsigned int index, const aiNode& node) {
        static const std::string underscore = "_";
        char postfix[10] = {0};
        ASSIMP_itoa10(postfix, index);

        std::string result = node.mName.C_Str();
        if (mesh.mName.length > 0) {
            result += underscore + mesh.mName.C_Str();
        }
        return result + underscore + postfix;
    }

    // Return an unique name for a given |mat| with original position |index|
    // in |aiScene::mMaterials|. The name preserves the original material
    // name if possible.
    std::string GetMaterialName(const aiMaterial& mat, unsigned int index) {
        static const std::string underscore = "_";
        char postfix[10] = {0};
        ASSIMP_itoa10(postfix, index);

        aiString mat_name;
        if (AI_SUCCESS == mat.Get(AI_MATKEY_NAME, mat_name)) {
            return mat_name.C_Str() + underscore + postfix;
        }

        return "Material" + underscore + postfix;
    }

    // Collect world transformations for each node
    void CollectTrafos(const aiNode* node, std::map<const aiNode*, aiMatrix4x4>& trafos) {
        const aiMatrix4x4& parent = node->mParent ? trafos[node->mParent] : aiMatrix4x4();
        trafos[node] = parent * node->mTransformation;
        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            CollectTrafos(node->mChildren[i], trafos);
        }
    }

    // Generate a flat list of the meshes (by index) assigned to each node
    void CollectMeshes(const aiNode* node, std::multimap<const aiNode*, unsigned int>& meshes) {
        for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            meshes.insert(std::make_pair(node, node->mMeshes[i]));
        }
        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            CollectMeshes(node->mChildren[i], meshes);
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Worker function for exporting a scene to 3DS. Prototyped and registered in Exporter.cpp
void ExportScene3DS(const char* pFile, IOSystem* pIOSystem, const aiScene* pScene, const ExportProperties* /*pProperties*/)
{
    std::shared_ptr<IOStream> outfile (pIOSystem->Open(pFile, "wb"));
    if(!outfile) {
        throw DeadlyExportError("Could not open output .3ds file: " + std::string(pFile));
    }

    // TODO: This extra copy should be avoided and all of this made a preprocess
    // requirement of the 3DS exporter.
    //
    // 3DS meshes can be max 0xffff (16 Bit) vertices and faces, respectively.
    // SplitLargeMeshes can do this, but it requires the correct limit to be set
    // which is not possible with the current way of specifying preprocess steps
    // in |Exporter::ExportFormatEntry|.
    aiScene* scenecopy_tmp;
    SceneCombiner::CopyScene(&scenecopy_tmp,pScene);
    std::unique_ptr<aiScene> scenecopy(scenecopy_tmp);

    SplitLargeMeshesProcess_Triangle tri_splitter;
    tri_splitter.SetLimit(0xffff);
    tri_splitter.Execute(scenecopy.get());

    SplitLargeMeshesProcess_Vertex vert_splitter;
    vert_splitter.SetLimit(0xffff);
    vert_splitter.Execute(scenecopy.get());

    // Invoke the actual exporter
    Discreet3DSExporter exporter(outfile, scenecopy.get());
}

} // end of namespace Assimp

// ------------------------------------------------------------------------------------------------
Discreet3DSExporter:: Discreet3DSExporter(std::shared_ptr<IOStream> outfile, const aiScene* scene)
: scene(scene)
, writer(outfile)
{
    CollectTrafos(scene->mRootNode, trafos);
    CollectMeshes(scene->mRootNode, meshes);

    ChunkWriter chunk(writer, Discreet3DS::CHUNK_MAIN);

    {
        ChunkWriter chunk(writer, Discreet3DS::CHUNK_OBJMESH);
        WriteMaterials();
        WriteMeshes();

        {
            ChunkWriter chunk(writer, Discreet3DS::CHUNK_MASTER_SCALE);
            writer.PutF4(1.0f);
        }
    }

    {
        ChunkWriter chunk(writer, Discreet3DS::CHUNK_KEYFRAMER);
        WriteHierarchy(*scene->mRootNode, -1, -1);
    }
}

// ------------------------------------------------------------------------------------------------
Discreet3DSExporter::~Discreet3DSExporter() {
    // empty
}


// ------------------------------------------------------------------------------------------------
int Discreet3DSExporter::WriteHierarchy(const aiNode& node, int seq, int sibling_level)
{
    // 3DS scene hierarchy is serialized as in http://www.martinreddy.net/gfx/3d/3DS.spec
    {
        ChunkWriter chunk(writer, Discreet3DS::CHUNK_TRACKINFO);
        {
            ChunkWriter chunk(writer, Discreet3DS::CHUNK_TRACKOBJNAME);

            // Assimp node names are unique and distinct from all mesh-node
            // names we generate; thus we can use them as-is
            WriteString(node.mName);

            // Two unknown int16 values - it is even unclear if 0 is a safe value
            // but luckily importers do not know better either.
            writer.PutI4(0);

            int16_t hierarchy_pos = static_cast<int16_t>(seq);
            if (sibling_level != -1) {
                hierarchy_pos = sibling_level;
            }

            // Write the hierarchy position
            writer.PutI2(hierarchy_pos);
        }
    }

    // TODO: write transformation chunks

    ++seq;
    sibling_level = seq;

    // Write all children
    for (unsigned int i = 0; i < node.mNumChildren; ++i) {
        seq = WriteHierarchy(*node.mChildren[i], seq, i == 0 ? -1 : sibling_level);
    }

    // Write all meshes as separate nodes to be able to reference the meshes by name
    for (unsigned int i = 0; i < node.mNumMeshes; ++i) {
        const bool first_child = node.mNumChildren == 0 && i == 0;

        const unsigned int mesh_idx = node.mMeshes[i];
        const aiMesh& mesh = *scene->mMeshes[mesh_idx];

        ChunkWriter chunk(writer, Discreet3DS::CHUNK_TRACKINFO);
        {
            ChunkWriter chunk(writer, Discreet3DS::CHUNK_TRACKOBJNAME);
            WriteString(GetMeshName(mesh, mesh_idx, node));

            writer.PutI4(0);
            writer.PutI2(static_cast<int16_t>(first_child ? seq : sibling_level));
            ++seq;
        }
    }
    return seq;
}

// ------------------------------------------------------------------------------------------------
void Discreet3DSExporter::WriteMaterials()
{
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
        ChunkWriter chunk(writer, Discreet3DS::CHUNK_MAT_MATERIAL);
        const aiMaterial& mat = *scene->mMaterials[i];

        {
            ChunkWriter chunk(writer, Discreet3DS::CHUNK_MAT_MATNAME);
            const std::string& name = GetMaterialName(mat, i);
            WriteString(name);
        }

        aiColor3D color;
        if (mat.Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
            ChunkWriter chunk(writer, Discreet3DS::CHUNK_MAT_DIFFUSE);
            WriteColor(color);
        }

        if (mat.Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
            ChunkWriter chunk(writer, Discreet3DS::CHUNK_MAT_SPECULAR);
            WriteColor(color);
        }

        if (mat.Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
            ChunkWriter chunk(writer, Discreet3DS::CHUNK_MAT_AMBIENT);
            WriteColor(color);
        }

        if (mat.Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
            ChunkWriter chunk(writer, Discreet3DS::CHUNK_MAT_SELF_ILLUM);
            WriteColor(color);
        }

        aiShadingMode shading_mode = aiShadingMode_Flat;
        if (mat.Get(AI_MATKEY_SHADING_MODEL, shading_mode) == AI_SUCCESS) {
            ChunkWriter chunk(writer, Discreet3DS::CHUNK_MAT_SHADING);

            Discreet3DS::shadetype3ds shading_mode_out;
            switch(shading_mode) {
            case aiShadingMode_Flat:
            case aiShadingMode_NoShading:
                shading_mode_out = Discreet3DS::Flat;
                break;

            case aiShadingMode_Gouraud:
            case aiShadingMode_Toon:
            case aiShadingMode_OrenNayar:
            case aiShadingMode_Minnaert:
                shading_mode_out = Discreet3DS::Gouraud;
                break;

            case aiShadingMode_Phong:
            case aiShadingMode_Blinn:
            case aiShadingMode_CookTorrance:
            case aiShadingMode_Fresnel:
                shading_mode_out = Discreet3DS::Phong;
                break;

            default:
                shading_mode_out = Discreet3DS::Flat;
                ai_assert(false);
            };
            writer.PutU2(static_cast<uint16_t>(shading_mode_out));
        }


        float f;
        if (mat.Get(AI_MATKEY_SHININESS, f) == AI_SUCCESS) {
            ChunkWriter chunk(writer, Discreet3DS::CHUNK_MAT_SHININESS);
            WritePercentChunk(f);
        }

        if (mat.Get(AI_MATKEY_SHININESS_STRENGTH, f) == AI_SUCCESS) {
            ChunkWriter chunk(writer, Discreet3DS::CHUNK_MAT_SHININESS_PERCENT);
            WritePercentChunk(f);
        }

        int twosided;
        if (mat.Get(AI_MATKEY_TWOSIDED, twosided) == AI_SUCCESS && twosided != 0) {
            ChunkWriter chunk(writer, Discreet3DS::CHUNK_MAT_TWO_SIDE);
            writer.PutI2(1);
        }

        WriteTexture(mat, aiTextureType_DIFFUSE, Discreet3DS::CHUNK_MAT_TEXTURE);
        WriteTexture(mat, aiTextureType_HEIGHT, Discreet3DS::CHUNK_MAT_BUMPMAP);
        WriteTexture(mat, aiTextureType_OPACITY, Discreet3DS::CHUNK_MAT_OPACMAP);
        WriteTexture(mat, aiTextureType_SHININESS, Discreet3DS::CHUNK_MAT_MAT_SHINMAP);
        WriteTexture(mat, aiTextureType_SPECULAR, Discreet3DS::CHUNK_MAT_SPECMAP);
        WriteTexture(mat, aiTextureType_EMISSIVE, Discreet3DS::CHUNK_MAT_SELFIMAP);
        WriteTexture(mat, aiTextureType_REFLECTION, Discreet3DS::CHUNK_MAT_REFLMAP);
    }
}

// ------------------------------------------------------------------------------------------------
void Discreet3DSExporter::WriteTexture(const aiMaterial& mat, aiTextureType type, uint16_t chunk_flags)
{
    aiString path;
    aiTextureMapMode map_mode[2] = {
        aiTextureMapMode_Wrap, aiTextureMapMode_Wrap
    };
    ai_real blend = 1.0;
    if (mat.GetTexture(type, 0, &path, NULL, NULL, &blend, NULL, map_mode) != AI_SUCCESS || !path.length) {
        return;
    }

    // TODO: handle embedded textures properly
    if (path.data[0] == '*') {
        DefaultLogger::get()->error("Ignoring embedded texture for export: " + std::string(path.C_Str()));
        return;
    }

    ChunkWriter chunk(writer, chunk_flags);
    {
        ChunkWriter chunk(writer, Discreet3DS::CHUNK_MAPFILE);
        WriteString(path);
    }

    WritePercentChunk(blend);

    {
        ChunkWriter chunk(writer, Discreet3DS::CHUNK_MAT_MAP_TILING);
        uint16_t val = 0; // WRAP
        if (map_mode[0] == aiTextureMapMode_Mirror) {
            val = 0x2;
        }
        else if (map_mode[0] == aiTextureMapMode_Decal) {
            val = 0x10;
        }
        writer.PutU2(val);
    }
    // TODO: export texture transformation (i.e. UV offset, scale, rotation)
}

// ------------------------------------------------------------------------------------------------
void Discreet3DSExporter::WriteMeshes()
{
    // NOTE: 3DS allows for instances. However:
    //   i)  not all importers support reading them
    //   ii) instances are not as flexible as they are in assimp, in particular,
    //        nodes can carry (and instance) only one mesh.
    //
    // This exporter currently deep clones all instanced meshes, i.e. for each mesh
    // attached to a node a full TRIMESH chunk is written to the file.
    //
    // Furthermore, the TRIMESH is transformed into world space so that it will
    // appear correctly if importers don't read the scene hierarchy at all.
    for (MeshesByNodeMap::const_iterator it = meshes.begin(); it != meshes.end(); ++it) {
        const aiNode& node = *(*it).first;
        const unsigned int mesh_idx = (*it).second;

        const aiMesh& mesh = *scene->mMeshes[mesh_idx];

        // This should not happen if the SLM step is correctly executed
        // before the scene is handed to the exporter
        ai_assert(mesh.mNumVertices <= 0xffff);
        ai_assert(mesh.mNumFaces <= 0xffff);

        const aiMatrix4x4& trafo = trafos[&node];

        ChunkWriter chunk(writer, Discreet3DS::CHUNK_OBJBLOCK);

        // Mesh name is tied to the node it is attached to so it can later be referenced
        const std::string& name = GetMeshName(mesh, mesh_idx, node);
        WriteString(name);


        // TRIMESH chunk
        ChunkWriter chunk2(writer, Discreet3DS::CHUNK_TRIMESH);

        // Vertices in world space
        {
            ChunkWriter chunk(writer, Discreet3DS::CHUNK_VERTLIST);

            const uint16_t count = static_cast<uint16_t>(mesh.mNumVertices);
            writer.PutU2(count);
            for (unsigned int i = 0; i < mesh.mNumVertices; ++i) {
                const aiVector3D& v = trafo * mesh.mVertices[i];
                writer.PutF4(v.x);
                writer.PutF4(v.y);
                writer.PutF4(v.z);
            }
        }

        // UV coordinates
        if (mesh.HasTextureCoords(0)) {
            ChunkWriter chunk(writer, Discreet3DS::CHUNK_MAPLIST);
            const uint16_t count = static_cast<uint16_t>(mesh.mNumVertices);
            writer.PutU2(count);

            for (unsigned int i = 0; i < mesh.mNumVertices; ++i) {
                const aiVector3D& v = mesh.mTextureCoords[0][i];
                writer.PutF4(v.x);
                writer.PutF4(v.y);
            }
        }

        // Faces (indices)
        {
            ChunkWriter chunk(writer, Discreet3DS::CHUNK_FACELIST);

            ai_assert(mesh.mNumFaces <= 0xffff);

            // Count triangles, discard lines and points
            uint16_t count = 0;
            for (unsigned int i = 0; i < mesh.mNumFaces; ++i) {
                const aiFace& f = mesh.mFaces[i];
                if (f.mNumIndices < 3) {
                    continue;
                }
                // TRIANGULATE step is a pre-requisite so we should not see polys here
                ai_assert(f.mNumIndices == 3);
                ++count;
            }

            writer.PutU2(count);
            for (unsigned int i = 0; i < mesh.mNumFaces; ++i) {
                const aiFace& f = mesh.mFaces[i];
                if (f.mNumIndices < 3) {
                    continue;
                }

                for (unsigned int j = 0; j < 3; ++j) {
                    ai_assert(f.mIndices[j] <= 0xffff);
                    writer.PutI2(static_cast<uint16_t>(f.mIndices[j]));
                }

                // Edge visibility flag
                writer.PutI2(0x0);
            }

            // TODO: write smoothing groups (CHUNK_SMOOLIST)

            WriteFaceMaterialChunk(mesh);
        }

        // Transformation matrix by which the mesh vertices have been pre-transformed with.
        {
            ChunkWriter chunk(writer, Discreet3DS::CHUNK_TRMATRIX);
            for (unsigned int r = 0; r < 4; ++r) {
                for (unsigned int c = 0; c < 3; ++c) {
                    writer.PutF4(trafo[r][c]);
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
void Discreet3DSExporter::WriteFaceMaterialChunk(const aiMesh& mesh)
{
    ChunkWriter chunk(writer, Discreet3DS::CHUNK_FACEMAT);
    const std::string& name = GetMaterialName(*scene->mMaterials[mesh.mMaterialIndex], mesh.mMaterialIndex);
    WriteString(name);

    // Because assimp splits meshes by material, only a single
    // FACEMAT chunk needs to be written
    ai_assert(mesh.mNumFaces <= 0xffff);
    const uint16_t count = static_cast<uint16_t>(mesh.mNumFaces);
    writer.PutU2(count);

    for (unsigned int i = 0; i < mesh.mNumFaces; ++i) {
        writer.PutU2(static_cast<uint16_t>(i));
    }
}

// ------------------------------------------------------------------------------------------------
void Discreet3DSExporter::WriteString(const std::string& s) {
    for (std::string::const_iterator it = s.begin(); it != s.end(); ++it) {
        writer.PutI1(*it);
    }
    writer.PutI1('\0');
}

// ------------------------------------------------------------------------------------------------
void Discreet3DSExporter::WriteString(const aiString& s) {
    for (std::size_t i = 0; i < s.length; ++i) {
        writer.PutI1(s.data[i]);
    }
    writer.PutI1('\0');
}

// ------------------------------------------------------------------------------------------------
void Discreet3DSExporter::WriteColor(const aiColor3D& color) {
    ChunkWriter chunk(writer, Discreet3DS::CHUNK_RGBF);
    writer.PutF4(color.r);
    writer.PutF4(color.g);
    writer.PutF4(color.b);
}

// ------------------------------------------------------------------------------------------------
void Discreet3DSExporter::WritePercentChunk(float f) {
    ChunkWriter chunk(writer, Discreet3DS::CHUNK_PERCENTF);
    writer.PutF4(f);
}

// ------------------------------------------------------------------------------------------------
void Discreet3DSExporter::WritePercentChunk(double f) {
    ChunkWriter chunk(writer, Discreet3DS::CHUNK_PERCENTD);
    writer.PutF8(f);
}


#endif // ASSIMP_BUILD_NO_3DS_EXPORTER
#endif // ASSIMP_BUILD_NO_EXPORT
