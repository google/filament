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
#ifndef ASSIMP_BUILD_NO_OBJ_EXPORTER

#include "ObjExporter.h"
#include "Exceptional.h"
#include "StringComparison.h"
#include <assimp/version.h>
#include <assimp/IOSystem.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <memory>

using namespace Assimp;

namespace Assimp {

// ------------------------------------------------------------------------------------------------
// Worker function for exporting a scene to Wavefront OBJ. Prototyped and registered in Exporter.cpp
void ExportSceneObj(const char* pFile,IOSystem* pIOSystem, const aiScene* pScene, const ExportProperties* /*pProperties*/) {
    // invoke the exporter
    ObjExporter exporter(pFile, pScene);

    if (exporter.mOutput.fail() || exporter.mOutputMat.fail()) {
        throw DeadlyExportError("output data creation failed. Most likely the file became too large: " + std::string(pFile));
    }

    // we're still here - export successfully completed. Write both the main OBJ file and the material script
    {
        std::unique_ptr<IOStream> outfile (pIOSystem->Open(pFile,"wt"));
        if(outfile == NULL) {
            throw DeadlyExportError("could not open output .obj file: " + std::string(pFile));
        }
        outfile->Write( exporter.mOutput.str().c_str(), static_cast<size_t>(exporter.mOutput.tellp()),1);
    }
    {
        std::unique_ptr<IOStream> outfile (pIOSystem->Open(exporter.GetMaterialLibFileName(),"wt"));
        if(outfile == NULL) {
            throw DeadlyExportError("could not open output .mtl file: " + std::string(exporter.GetMaterialLibFileName()));
        }
        outfile->Write( exporter.mOutputMat.str().c_str(), static_cast<size_t>(exporter.mOutputMat.tellp()),1);
    }
}

// ------------------------------------------------------------------------------------------------
// Worker function for exporting a scene to Wavefront OBJ without the material file. Prototyped and registered in Exporter.cpp
void ExportSceneObjNoMtl(const char* pFile,IOSystem* pIOSystem, const aiScene* pScene, const ExportProperties* pProperties) {
    // invoke the exporter
    ObjExporter exporter(pFile, pScene, true);

    if (exporter.mOutput.fail() || exporter.mOutputMat.fail()) {
        throw DeadlyExportError("output data creation failed. Most likely the file became too large: " + std::string(pFile));
    }

    // we're still here - export successfully completed. Write both the main OBJ file and the material script
    {
        std::unique_ptr<IOStream> outfile (pIOSystem->Open(pFile,"wt"));
        if(outfile == NULL) {
            throw DeadlyExportError("could not open output .obj file: " + std::string(pFile));
        }
        outfile->Write( exporter.mOutput.str().c_str(), static_cast<size_t>(exporter.mOutput.tellp()),1);
    }


}

} // end of namespace Assimp

static const std::string MaterialExt = ".mtl";

// ------------------------------------------------------------------------------------------------
ObjExporter::ObjExporter(const char* _filename, const aiScene* pScene, bool noMtl)
: filename(_filename)
, pScene(pScene)
, vp()
, vn()
, vt()
, vc()
, mVpMap()
, mVnMap()
, mVtMap()
, mVcMap()
, mMeshes()
, endl("\n") {
    // make sure that all formatting happens using the standard, C locale and not the user's current locale
    const std::locale& l = std::locale("C");
    mOutput.imbue(l);
    mOutput.precision(16);
    mOutputMat.imbue(l);
    mOutputMat.precision(16);

    WriteGeometryFile(noMtl);
    if (!noMtl)
        WriteMaterialFile();
}

// ------------------------------------------------------------------------------------------------
ObjExporter::~ObjExporter() {

}

// ------------------------------------------------------------------------------------------------
std::string ObjExporter :: GetMaterialLibName()
{
    // within the Obj file, we use just the relative file name with the path stripped
    const std::string& s = GetMaterialLibFileName();
    std::string::size_type il = s.find_last_of("/\\");
    if (il != std::string::npos) {
        return s.substr(il + 1);
    }

    return s;
}

// ------------------------------------------------------------------------------------------------
std::string ObjExporter::GetMaterialLibFileName() {
    // Remove existing .obj file extension so that the final material file name will be fileName.mtl and not fileName.obj.mtl
    size_t lastdot = filename.find_last_of('.');
    if (lastdot != std::string::npos)
        return filename.substr(0, lastdot) + MaterialExt;

    return filename + MaterialExt;
}

// ------------------------------------------------------------------------------------------------
void ObjExporter::WriteHeader(std::ostringstream& out) {
    out << "# File produced by Open Asset Import Library (http://www.assimp.sf.net)" << endl;
    out << "# (assimp v" << aiGetVersionMajor() << '.' << aiGetVersionMinor() << '.'
        << aiGetVersionRevision() << ")" << endl  << endl;
}

// ------------------------------------------------------------------------------------------------
std::string ObjExporter::GetMaterialName(unsigned int index)
{
    const aiMaterial* const mat = pScene->mMaterials[index];
    if ( nullptr == mat ) {
        static const std::string EmptyStr;
        return EmptyStr;
    }

    aiString s;
    if(AI_SUCCESS == mat->Get(AI_MATKEY_NAME,s)) {
        return std::string(s.data,s.length);
    }

    char number[ sizeof(unsigned int) * 3 + 1 ];
    ASSIMP_itoa10(number,index);
    return "$Material_" + std::string(number);
}

// ------------------------------------------------------------------------------------------------
void ObjExporter::WriteMaterialFile()
{
    WriteHeader(mOutputMat);

    for(unsigned int i = 0; i < pScene->mNumMaterials; ++i) {
        const aiMaterial* const mat = pScene->mMaterials[i];

        int illum = 1;
        mOutputMat << "newmtl " << GetMaterialName(i)  << endl;

        aiColor4D c;
        if(AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_DIFFUSE,c)) {
            mOutputMat << "Kd " << c.r << " " << c.g << " " << c.b << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_AMBIENT,c)) {
            mOutputMat << "Ka " << c.r << " " << c.g << " " << c.b << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_SPECULAR,c)) {
            mOutputMat << "Ks " << c.r << " " << c.g << " " << c.b << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_EMISSIVE,c)) {
            mOutputMat << "Ke " << c.r << " " << c.g << " " << c.b << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_TRANSPARENT,c)) {
            mOutputMat << "Tf " << c.r << " " << c.g << " " << c.b << endl;
        }

        ai_real o;
        if(AI_SUCCESS == mat->Get(AI_MATKEY_OPACITY,o)) {
            mOutputMat << "d " << o << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_REFRACTI,o)) {
            mOutputMat << "Ni " << o << endl;
        }

        if(AI_SUCCESS == mat->Get(AI_MATKEY_SHININESS,o) && o) {
            mOutputMat << "Ns " << o << endl;
            illum = 2;
        }

        mOutputMat << "illum " << illum << endl;

        aiString s;
        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE_DIFFUSE(0),s)) {
            mOutputMat << "map_Kd " << s.data << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE_AMBIENT(0),s)) {
            mOutputMat << "map_Ka " << s.data << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE_SPECULAR(0),s)) {
            mOutputMat << "map_Ks " << s.data << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE_SHININESS(0),s)) {
            mOutputMat << "map_Ns " << s.data << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE_OPACITY(0),s)) {
            mOutputMat << "map_d " << s.data << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE_HEIGHT(0),s) || AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE_NORMALS(0),s)) {
            // implementations seem to vary here, so write both variants
            mOutputMat << "bump " << s.data << endl;
            mOutputMat << "map_bump " << s.data << endl;
        }

        mOutputMat << endl;
    }
}

void ObjExporter::WriteGeometryFile(bool noMtl) {
    WriteHeader(mOutput);
    if (!noMtl)
        mOutput << "mtllib "  << GetMaterialLibName() << endl << endl;

    // collect mesh geometry
    aiMatrix4x4 mBase;
    AddNode(pScene->mRootNode, mBase);

    // write vertex positions with colors, if any
    mVpMap.getVectors( vp );
    mVcMap.getColors( vc );
    if ( vc.empty() ) {
        mOutput << "# " << vp.size() << " vertex positions" << endl;
        for ( const aiVector3D& v : vp ) {
            mOutput << "v  " << v.x << " " << v.y << " " << v.z << endl;
        }
    } else {
        mOutput << "# " << vp.size() << " vertex positions and colors" << endl;
        size_t colIdx = 0;
        for ( const aiVector3D& v : vp ) {
            if ( colIdx < vc.size() ) {
                mOutput << "v  " << v.x << " " << v.y << " " << v.z << " " << vc[ colIdx ].r << " " << vc[ colIdx ].g << " " << vc[ colIdx ].b << endl;
            }
            ++colIdx;
        }
    }
    mOutput << endl;

    // write uv coordinates
    mVtMap.getVectors(vt);
    mOutput << "# " << vt.size() << " UV coordinates" << endl;
    for(const aiVector3D& v : vt) {
        mOutput << "vt " << v.x << " " << v.y << " " << v.z << endl;
    }
    mOutput << endl;

    // write vertex normals
    mVnMap.getVectors(vn);
    mOutput << "# " << vn.size() << " vertex normals" << endl;
    for(const aiVector3D& v : vn) {
        mOutput << "vn " << v.x << " " << v.y << " " << v.z << endl;
    }
    mOutput << endl;

    // now write all mesh instances
    for(const MeshInstance& m : mMeshes) {
        mOutput << "# Mesh \'" << m.name << "\' with " << m.faces.size() << " faces" << endl;
        if (!m.name.empty()) {
            mOutput << "g " << m.name << endl;
        }
        if (!noMtl)
            mOutput << "usemtl " << m.matname << endl;

        for(const Face& f : m.faces) {
            mOutput << f.kind << ' ';
            for(const FaceVertex& fv : f.indices) {
                mOutput << ' ' << fv.vp;

                if (f.kind != 'p') {
                    if (fv.vt || f.kind == 'f') {
                        mOutput << '/';
                    }
                    if (fv.vt) {
                        mOutput << fv.vt;
                    }
                    if (f.kind == 'f' && fv.vn) {
                        mOutput << '/' << fv.vn;
                    }
                }
            }

            mOutput << endl;
        }
        mOutput << endl;
    }
}

// ------------------------------------------------------------------------------------------------
int ObjExporter::vecIndexMap::getIndex(const aiVector3D& vec) {
    vecIndexMap::dataType::iterator vertIt = vecMap.find(vec);
    // vertex already exists, so reference it
    if(vertIt != vecMap.end()){
        return vertIt->second;
    }
    vecMap[vec] = mNextIndex;
    int ret = mNextIndex;
    mNextIndex++;
    return ret;
}

// ------------------------------------------------------------------------------------------------
void ObjExporter::vecIndexMap::getVectors( std::vector<aiVector3D>& vecs ) {
    vecs.resize(vecMap.size());
    for(vecIndexMap::dataType::iterator it = vecMap.begin(); it != vecMap.end(); ++it){
        vecs[it->second-1] = it->first;
    }
}

// ------------------------------------------------------------------------------------------------
int ObjExporter::colIndexMap::getIndex( const aiColor4D& col ) {
    colIndexMap::dataType::iterator vertIt = colMap.find( col );
    // vertex already exists, so reference it
    if ( vertIt != colMap.end() ) {
        return vertIt->second;
    }
    colMap[ col ] = mNextIndex;
    int ret = mNextIndex;
    mNextIndex++;

    return ret;
}

// ------------------------------------------------------------------------------------------------
void ObjExporter::colIndexMap::getColors( std::vector<aiColor4D> &colors ) {
    colors.resize( colMap.size() );
    for ( colIndexMap::dataType::iterator it = colMap.begin(); it != colMap.end(); ++it ) {
        colors[ it->second - 1 ] = it->first;
    }
}

// ------------------------------------------------------------------------------------------------
void ObjExporter::AddMesh(const aiString& name, const aiMesh* m, const aiMatrix4x4& mat) {
    mMeshes.push_back(MeshInstance());
    MeshInstance& mesh = mMeshes.back();

    mesh.name = std::string( name.data, name.length );
    mesh.matname = GetMaterialName(m->mMaterialIndex);

    mesh.faces.resize(m->mNumFaces);

    for(unsigned int i = 0; i < m->mNumFaces; ++i) {
        const aiFace& f = m->mFaces[i];

        Face& face = mesh.faces[i];
        switch (f.mNumIndices) {
            case 1:
                face.kind = 'p';
                break;
            case 2:
                face.kind = 'l';
                break;
            default:
                face.kind = 'f';
        }
        face.indices.resize(f.mNumIndices);

        for(unsigned int a = 0; a < f.mNumIndices; ++a) {
            const unsigned int idx = f.mIndices[a];

            aiVector3D vert = mat * m->mVertices[idx];
            face.indices[a].vp = mVpMap.getIndex(vert);

            if (m->mNormals) {
                aiVector3D norm = aiMatrix3x3(mat) * m->mNormals[idx];
                face.indices[a].vn = mVnMap.getIndex(norm);
            } else {
                face.indices[a].vn = 0;
            }

            if ( nullptr != m->mColors[ 0 ] ) {
                aiColor4D col4 = m->mColors[ 0 ][ idx ];
                face.indices[ a ].vc = mVcMap.getIndex( col4 );
            } else {
                face.indices[ a ].vc = 0;
            }

            if ( m->mTextureCoords[ 0 ] ) {
                face.indices[a].vt = mVtMap.getIndex(m->mTextureCoords[0][idx]);
            } else {
                face.indices[a].vt = 0;
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
void ObjExporter::AddNode(const aiNode* nd, const aiMatrix4x4& mParent)
{
    const aiMatrix4x4& mAbs = mParent * nd->mTransformation;

    for(unsigned int i = 0; i < nd->mNumMeshes; ++i) {
        AddMesh(nd->mName, pScene->mMeshes[nd->mMeshes[i]], mAbs);
    }

    for(unsigned int i = 0; i < nd->mNumChildren; ++i) {
        AddNode(nd->mChildren[i], mAbs);
    }
}

// ------------------------------------------------------------------------------------------------

#endif // ASSIMP_BUILD_NO_OBJ_EXPORTER
#endif // ASSIMP_BUILD_NO_EXPORT
