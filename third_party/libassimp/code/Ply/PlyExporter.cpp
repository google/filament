/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team


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



#if !defined(ASSIMP_BUILD_NO_EXPORT) && !defined(ASSIMP_BUILD_NO_PLY_EXPORTER)

#include "PlyExporter.h"
#include <memory>
#include <cmath>
#include <assimp/Exceptional.h>
#include <assimp/scene.h>
#include <assimp/version.h>
#include <assimp/IOSystem.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/qnan.h>


//using namespace Assimp;
namespace Assimp    {

// make sure type_of returns consistent output across different platforms
// also consider using: typeid(VAR).name()
template <typename T> const char* type_of(T&) { return "unknown"; }
template<> const char* type_of(float&) { return "float"; }
template<> const char* type_of(double&) { return "double"; }

// ------------------------------------------------------------------------------------------------
// Worker function for exporting a scene to PLY. Prototyped and registered in Exporter.cpp
void ExportScenePly(const char* pFile,IOSystem* pIOSystem, const aiScene* pScene, const ExportProperties* /*pProperties*/)
{
    // invoke the exporter
    PlyExporter exporter(pFile, pScene);

    if (exporter.mOutput.fail()) {
        throw DeadlyExportError("output data creation failed. Most likely the file became too large: " + std::string(pFile));
    }

    // we're still here - export successfully completed. Write the file.
    std::unique_ptr<IOStream> outfile (pIOSystem->Open(pFile,"wt"));
    if(outfile == NULL) {
        throw DeadlyExportError("could not open output .ply file: " + std::string(pFile));
    }

    outfile->Write( exporter.mOutput.str().c_str(), static_cast<size_t>(exporter.mOutput.tellp()),1);
}

void ExportScenePlyBinary(const char* pFile, IOSystem* pIOSystem, const aiScene* pScene, const ExportProperties* /*pProperties*/)
{
    // invoke the exporter
    PlyExporter exporter(pFile, pScene, true);

    // we're still here - export successfully completed. Write the file.
    std::unique_ptr<IOStream> outfile(pIOSystem->Open(pFile, "wb"));
    if (outfile == NULL) {
        throw DeadlyExportError("could not open output .ply file: " + std::string(pFile));
    }

    outfile->Write(exporter.mOutput.str().c_str(), static_cast<size_t>(exporter.mOutput.tellp()), 1);
}

#define PLY_EXPORT_HAS_NORMALS 0x1
#define PLY_EXPORT_HAS_TANGENTS_BITANGENTS 0x2
#define PLY_EXPORT_HAS_TEXCOORDS 0x4
#define PLY_EXPORT_HAS_COLORS (PLY_EXPORT_HAS_TEXCOORDS << AI_MAX_NUMBER_OF_TEXTURECOORDS)

// ------------------------------------------------------------------------------------------------
PlyExporter::PlyExporter(const char* _filename, const aiScene* pScene, bool binary)
: filename(_filename)
, endl("\n")
{
    // make sure that all formatting happens using the standard, C locale and not the user's current locale
    const std::locale& l = std::locale("C");
    mOutput.imbue(l);
    mOutput.precision(ASSIMP_AI_REAL_TEXT_PRECISION);

    unsigned int faces = 0u, vertices = 0u, components = 0u;
    for (unsigned int i = 0; i < pScene->mNumMeshes; ++i) {
        const aiMesh& m = *pScene->mMeshes[i];
        faces += m.mNumFaces;
        vertices += m.mNumVertices;

        if (m.HasNormals()) {
            components |= PLY_EXPORT_HAS_NORMALS;
        }
        if (m.HasTangentsAndBitangents()) {
            components |= PLY_EXPORT_HAS_TANGENTS_BITANGENTS;
        }
        for (unsigned int t = 0; m.HasTextureCoords(t); ++t) {
            components |= PLY_EXPORT_HAS_TEXCOORDS << t;
        }
        for (unsigned int t = 0; m.HasVertexColors(t); ++t) {
            components |= PLY_EXPORT_HAS_COLORS << t;
        }
    }

    mOutput << "ply" << endl;
    if (binary) {
#if (defined AI_BUILD_BIG_ENDIAN)
        mOutput << "format binary_big_endian 1.0" << endl;
#else
        mOutput << "format binary_little_endian 1.0" << endl;
#endif
    }
    else {
        mOutput << "format ascii 1.0" << endl;
    }
    mOutput << "comment Created by Open Asset Import Library - http://assimp.sf.net (v"
        << aiGetVersionMajor() << '.' << aiGetVersionMinor() << '.'
        << aiGetVersionRevision() << ")" << endl;

    // Look through materials for a diffuse texture, and add it if found
    for ( unsigned int i = 0; i < pScene->mNumMaterials; ++i )
    {
        const aiMaterial* const mat = pScene->mMaterials[i];
        aiString s;
        if ( AI_SUCCESS == mat->Get( AI_MATKEY_TEXTURE_DIFFUSE( 0 ), s ) )
        {
            mOutput << "comment TextureFile " << s.data << endl;
        }
    }

    // TODO: probably want to check here rather than just assume something
    //       definitely not good to always write float even if we might have double precision

    ai_real tmp = 0.0;
    const char * typeName = type_of(tmp);

    mOutput << "element vertex " << vertices << endl;
    mOutput << "property " << typeName << " x" << endl;
    mOutput << "property " << typeName << " y" << endl;
    mOutput << "property " << typeName << " z" << endl;

    if(components & PLY_EXPORT_HAS_NORMALS) {
        mOutput << "property " << typeName << " nx" << endl;
        mOutput << "property " << typeName << " ny" << endl;
        mOutput << "property " << typeName << " nz" << endl;
    }

    // write texcoords first, just in case an importer does not support tangents
    // bitangents and just skips over the rest of the line upon encountering
    // unknown fields (Ply leaves pretty much every vertex component open,
    // but in reality most importers only know about vertex positions, normals
    // and texture coordinates).
    for (unsigned int n = PLY_EXPORT_HAS_TEXCOORDS, c = 0; (components & n) && c != AI_MAX_NUMBER_OF_TEXTURECOORDS; n <<= 1, ++c) {
        if (!c) {
            mOutput << "property " << typeName << " s" << endl;
            mOutput << "property " << typeName << " t" << endl;
        }
        else {
            mOutput << "property " << typeName << " s" << c << endl;
            mOutput << "property " << typeName << " t" << c << endl;
        }
    }

    for (unsigned int n = PLY_EXPORT_HAS_COLORS, c = 0; (components & n) && c != AI_MAX_NUMBER_OF_COLOR_SETS; n <<= 1, ++c) {
        if (!c) {
            mOutput << "property " << "uchar" << " red" << endl;
            mOutput << "property " << "uchar" << " green" << endl;
            mOutput << "property " << "uchar" << " blue" << endl;
            mOutput << "property " << "uchar" << " alpha" << endl;
        }
        else {
            mOutput << "property " << "uchar" << " red" << c << endl;
            mOutput << "property " << "uchar" << " green" << c << endl;
            mOutput << "property " << "uchar" << " blue" << c << endl;
            mOutput << "property " << "uchar" << " alpha" << c << endl;
        }
    }

    if(components & PLY_EXPORT_HAS_TANGENTS_BITANGENTS) {
        mOutput << "property " << typeName << " tx" << endl;
        mOutput << "property " << typeName << " ty" << endl;
        mOutput << "property " << typeName << " tz" << endl;
        mOutput << "property " << typeName << " bx" << endl;
        mOutput << "property " << typeName << " by" << endl;
        mOutput << "property " << typeName << " bz" << endl;
    }

    mOutput << "element face " << faces << endl;

    // uchar seems to be the most common type for the number of indices per polygon and int seems to be most common for the vertex indices.
    // For instance, MeshLab fails to load meshes in which both types are uint. Houdini seems to have problems as well.
    // Obviously, using uchar will not work for meshes with polygons with more than 255 indices, but how realistic is this case?
    mOutput << "property list uchar int vertex_index" << endl;

    mOutput << "end_header" << endl;

    for (unsigned int i = 0; i < pScene->mNumMeshes; ++i) {
        if (binary) {
            WriteMeshVertsBinary(pScene->mMeshes[i], components);
        }
        else {
            WriteMeshVerts(pScene->mMeshes[i], components);
        }
    }
    for (unsigned int i = 0, ofs = 0; i < pScene->mNumMeshes; ++i) {
        if (binary) {
            WriteMeshIndicesBinary(pScene->mMeshes[i], ofs);
        }
        else {
            WriteMeshIndices(pScene->mMeshes[i], ofs);
        }
        ofs += pScene->mMeshes[i]->mNumVertices;
    }
}

// ------------------------------------------------------------------------------------------------
PlyExporter::~PlyExporter() {
    // empty
}

// ------------------------------------------------------------------------------------------------
void PlyExporter::WriteMeshVerts(const aiMesh* m, unsigned int components)
{
    static const ai_real inf = std::numeric_limits<ai_real>::infinity();

    // If a component (for instance normal vectors) is present in at least one mesh in the scene,
    // then default values are written for meshes that do not contain this component.
    for (unsigned int i = 0; i < m->mNumVertices; ++i) {
        mOutput <<
            m->mVertices[i].x << " " <<
            m->mVertices[i].y << " " <<
            m->mVertices[i].z
        ;
        if(components & PLY_EXPORT_HAS_NORMALS) {
            if (m->HasNormals() && is_not_qnan(m->mNormals[i].x) && std::fabs(m->mNormals[i].x) != inf) {
                mOutput <<
                    " " << m->mNormals[i].x <<
                    " " << m->mNormals[i].y <<
                    " " << m->mNormals[i].z;
            }
            else {
                mOutput << " 0.0 0.0 0.0";
            }
        }

        for (unsigned int n = PLY_EXPORT_HAS_TEXCOORDS, c = 0; (components & n) && c != AI_MAX_NUMBER_OF_TEXTURECOORDS; n <<= 1, ++c) {
            if (m->HasTextureCoords(c)) {
                mOutput <<
                    " " << m->mTextureCoords[c][i].x <<
                    " " << m->mTextureCoords[c][i].y;
            }
            else {
                mOutput << " -1.0 -1.0";
            }
        }

        for (unsigned int n = PLY_EXPORT_HAS_COLORS, c = 0; (components & n) && c != AI_MAX_NUMBER_OF_COLOR_SETS; n <<= 1, ++c) {
            if (m->HasVertexColors(c)) {
                mOutput <<
                    " " << (int)(m->mColors[c][i].r * 255) <<
                    " " << (int)(m->mColors[c][i].g * 255) <<
                    " " << (int)(m->mColors[c][i].b * 255) <<
                    " " << (int)(m->mColors[c][i].a * 255);
            }
            else {
                mOutput << " 0 0 0";
            }
        }

        if(components & PLY_EXPORT_HAS_TANGENTS_BITANGENTS) {
            if (m->HasTangentsAndBitangents()) {
                mOutput <<
                " " << m->mTangents[i].x <<
                " " << m->mTangents[i].y <<
                " " << m->mTangents[i].z <<
                " " << m->mBitangents[i].x <<
                " " << m->mBitangents[i].y <<
                " " << m->mBitangents[i].z
                ;
            }
            else {
                mOutput << " 0.0 0.0 0.0 0.0 0.0 0.0";
            }
        }

        mOutput << endl;
    }
}

// ------------------------------------------------------------------------------------------------
void PlyExporter::WriteMeshVertsBinary(const aiMesh* m, unsigned int components)
{
    // If a component (for instance normal vectors) is present in at least one mesh in the scene,
    // then default values are written for meshes that do not contain this component.
    aiVector3D defaultNormal(0, 0, 0);
    aiVector2D defaultUV(-1, -1);
    aiColor4D defaultColor(-1, -1, -1, -1);
    for (unsigned int i = 0; i < m->mNumVertices; ++i) {
        mOutput.write(reinterpret_cast<const char*>(&m->mVertices[i].x), 12);
        if (components & PLY_EXPORT_HAS_NORMALS) {
            if (m->HasNormals()) {
                mOutput.write(reinterpret_cast<const char*>(&m->mNormals[i].x), 12);
            }
            else {
                mOutput.write(reinterpret_cast<const char*>(&defaultNormal.x), 12);
            }
        }

        for (unsigned int n = PLY_EXPORT_HAS_TEXCOORDS, c = 0; (components & n) && c != AI_MAX_NUMBER_OF_TEXTURECOORDS; n <<= 1, ++c) {
            if (m->HasTextureCoords(c)) {
                mOutput.write(reinterpret_cast<const char*>(&m->mTextureCoords[c][i].x), 8);
            }
            else {
                mOutput.write(reinterpret_cast<const char*>(&defaultUV.x), 8);
            }
        }

        for (unsigned int n = PLY_EXPORT_HAS_COLORS, c = 0; (components & n) && c != AI_MAX_NUMBER_OF_COLOR_SETS; n <<= 1, ++c) {
            if (m->HasVertexColors(c)) {
                mOutput.write(reinterpret_cast<const char*>(&m->mColors[c][i].r), 16);
            }
            else {
                mOutput.write(reinterpret_cast<const char*>(&defaultColor.r), 16);
            }
        }

        if (components & PLY_EXPORT_HAS_TANGENTS_BITANGENTS) {
            if (m->HasTangentsAndBitangents()) {
                mOutput.write(reinterpret_cast<const char*>(&m->mTangents[i].x), 12);
                mOutput.write(reinterpret_cast<const char*>(&m->mBitangents[i].x), 12);
            }
            else {
                mOutput.write(reinterpret_cast<const char*>(&defaultNormal.x), 12);
                mOutput.write(reinterpret_cast<const char*>(&defaultNormal.x), 12);
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
void PlyExporter::WriteMeshIndices(const aiMesh* m, unsigned int offset)
{
    for (unsigned int i = 0; i < m->mNumFaces; ++i) {
        const aiFace& f = m->mFaces[i];
        mOutput << f.mNumIndices << " ";
        for(unsigned int c = 0; c < f.mNumIndices; ++c) {
            mOutput << (f.mIndices[c] + offset) << (c == f.mNumIndices-1 ? endl : " ");
        }
    }
}

// Generic method in case we want to use different data types for the indices or make this configurable.
template<typename NumIndicesType, typename IndexType>
void WriteMeshIndicesBinary_Generic(const aiMesh* m, unsigned int offset, std::ostringstream& output)
{
    for (unsigned int i = 0; i < m->mNumFaces; ++i) {
        const aiFace& f = m->mFaces[i];
        NumIndicesType numIndices = static_cast<NumIndicesType>(f.mNumIndices);
        output.write(reinterpret_cast<const char*>(&numIndices), sizeof(NumIndicesType));
        for (unsigned int c = 0; c < f.mNumIndices; ++c) {
            IndexType index = f.mIndices[c] + offset;
            output.write(reinterpret_cast<const char*>(&index), sizeof(IndexType));
        }
    }
}

void PlyExporter::WriteMeshIndicesBinary(const aiMesh* m, unsigned int offset)
{
    WriteMeshIndicesBinary_Generic<unsigned char, int>(m, offset, mOutput);
}

} // end of namespace Assimp

#endif // !defined(ASSIMP_BUILD_NO_EXPORT) && !defined(ASSIMP_BUILD_NO_PLY_EXPORTER)
