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

/** @file  Q3DLoader.cpp
 *  @brief Implementation of the Q3D importer class
 */


#ifndef ASSIMP_BUILD_NO_Q3D_IMPORTER

// internal headers
#include "Q3DLoader.h"
#include "StreamReader.h"
#include "fast_atof.h"
#include <assimp/IOSystem.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/scene.h>
#include <assimp/importerdesc.h>

using namespace Assimp;

static const aiImporterDesc desc = {
    "Quick3D Importer",
    "",
    "",
    "http://www.quick3d.com/",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "q3o q3s"
};

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
Q3DImporter::Q3DImporter()
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
Q3DImporter::~Q3DImporter()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool Q3DImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    const std::string extension = GetExtension(pFile);

    if (extension == "q3s" || extension == "q3o")
        return true;
    else if (!extension.length() || checkSig)   {
        if (!pIOHandler)
            return true;
        const char* tokens[] = {"quick3Do","quick3Ds"};
        return SearchFileHeaderForToken(pIOHandler,pFile,tokens,2);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
const aiImporterDesc* Q3DImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void Q3DImporter::InternReadFile( const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
    StreamReaderLE stream(pIOHandler->Open(pFile,"rb"));

    // The header is 22 bytes large
    if (stream.GetRemainingSize() < 22)
        throw DeadlyImportError("File is either empty or corrupt: " + pFile);

    // Check the file's signature
    if (ASSIMP_strincmp( (const char*)stream.GetPtr(), "quick3Do", 8 ) &&
        ASSIMP_strincmp( (const char*)stream.GetPtr(), "quick3Ds", 8 ))
    {
        throw DeadlyImportError("Not a Quick3D file. Signature string is: " +
            std::string((const char*)stream.GetPtr(),8));
    }

    // Print the file format version
    DefaultLogger::get()->info("Quick3D File format version: " +
        std::string(&((const char*)stream.GetPtr())[8],2));

    // ... an store it
    char major = ((const char*)stream.GetPtr())[8];
    char minor = ((const char*)stream.GetPtr())[9];

    stream.IncPtr(10);
    unsigned int numMeshes    = (unsigned int)stream.GetI4();
    unsigned int numMats      = (unsigned int)stream.GetI4();
    unsigned int numTextures  = (unsigned int)stream.GetI4();

    std::vector<Material> materials;
    materials.reserve(numMats);

    std::vector<Mesh> meshes;
    meshes.reserve(numMeshes);

    // Allocate the scene root node
    pScene->mRootNode = new aiNode();

    aiColor3D fgColor (0.6f,0.6f,0.6f);

    // Now read all file chunks
    while (true)
    {
        if (stream.GetRemainingSize() < 1)break;
        char c = stream.GetI1();
        switch (c)
        {
            // Meshes chunk
        case 'm':
            {
                for (unsigned int quak = 0; quak < numMeshes; ++quak)
                {
                    meshes.push_back(Mesh());
                    Mesh& mesh = meshes.back();

                    // read all vertices
                    unsigned int numVerts = (unsigned int)stream.GetI4();
                    if (!numVerts)
                        throw DeadlyImportError("Quick3D: Found mesh with zero vertices");

                    std::vector<aiVector3D>& verts = mesh.verts;
                    verts.resize(numVerts);

                    for (unsigned int i = 0; i < numVerts;++i)
                    {
                        verts[i].x = stream.GetF4();
                        verts[i].y = stream.GetF4();
                        verts[i].z = stream.GetF4();
                    }

                    // read all faces
                    numVerts = (unsigned int)stream.GetI4();
                    if (!numVerts)
                        throw DeadlyImportError("Quick3D: Found mesh with zero faces");

                    std::vector<Face >& faces = mesh.faces;
                    faces.reserve(numVerts);

                    // number of indices
                    for (unsigned int i = 0; i < numVerts;++i)
                    {
                        faces.push_back(Face(stream.GetI2()) );
                        if (faces.back().indices.empty())
                            throw DeadlyImportError("Quick3D: Found face with zero indices");
                    }

                    // indices
                    for (unsigned int i = 0; i < numVerts;++i)
                    {
                        Face& vec = faces[i];
                        for (unsigned int a = 0; a < (unsigned int)vec.indices.size();++a)
                            vec.indices[a] = stream.GetI4();
                    }

                    // material indices
                    for (unsigned int i = 0; i < numVerts;++i)
                    {
                        faces[i].mat = (unsigned int)stream.GetI4();
                    }

                    // read all normals
                    numVerts = (unsigned int)stream.GetI4();
                    std::vector<aiVector3D>& normals = mesh.normals;
                    normals.resize(numVerts);

                    for (unsigned int i = 0; i < numVerts;++i)
                    {
                        normals[i].x = stream.GetF4();
                        normals[i].y = stream.GetF4();
                        normals[i].z = stream.GetF4();
                    }

                    numVerts = (unsigned int)stream.GetI4();
                    if (numTextures && numVerts)
                    {
                        // read all texture coordinates
                        std::vector<aiVector3D>& uv = mesh.uv;
                        uv.resize(numVerts);

                        for (unsigned int i = 0; i < numVerts;++i)
                        {
                            uv[i].x = stream.GetF4();
                            uv[i].y = stream.GetF4();
                        }

                        // UV indices
                        for (unsigned int i = 0; i < (unsigned int)faces.size();++i)
                        {
                            Face& vec = faces[i];
                            for (unsigned int a = 0; a < (unsigned int)vec.indices.size();++a)
                            {
                                vec.uvindices[a] = stream.GetI4();
                                if (!i && !a)
                                    mesh.prevUVIdx = vec.uvindices[a];
                                else if (vec.uvindices[a] != mesh.prevUVIdx)
                                    mesh.prevUVIdx = UINT_MAX;
                            }
                        }
                    }

                    // we don't need the rest, but we need to get to the next chunk
                    stream.IncPtr(36);
                    if (minor > '0' && major == '3')
                        stream.IncPtr(mesh.faces.size());
                }
                // stream.IncPtr(4); // unknown value here
            }
            break;

            // materials chunk
        case 'c':

            for (unsigned int i = 0; i < numMats; ++i)
            {
                materials.push_back(Material());
                Material& mat = materials.back();

                // read the material name
                while (( c = stream.GetI1()))
                    mat.name.data[mat.name.length++] = c;

                // add the terminal character
                mat.name.data[mat.name.length] = '\0';

                // read the ambient color
                mat.ambient.r = stream.GetF4();
                mat.ambient.g = stream.GetF4();
                mat.ambient.b = stream.GetF4();

                // read the diffuse color
                mat.diffuse.r = stream.GetF4();
                mat.diffuse.g = stream.GetF4();
                mat.diffuse.b = stream.GetF4();

                // read the ambient color
                mat.specular.r = stream.GetF4();
                mat.specular.g = stream.GetF4();
                mat.specular.b = stream.GetF4();

                // read the transparency
                mat.transparency = stream.GetF4();

                // unknown value here
                // stream.IncPtr(4);
                // FIX: it could be the texture index ...
                mat.texIdx = (unsigned int)stream.GetI4();
            }

            break;

            // texture chunk
        case 't':

            pScene->mNumTextures = numTextures;
            if (!numTextures)break;
            pScene->mTextures    = new aiTexture*[pScene->mNumTextures];
            // to make sure we won't crash if we leave through an exception
            ::memset(pScene->mTextures,0,sizeof(void*)*pScene->mNumTextures);
            for (unsigned int i = 0; i < pScene->mNumTextures; ++i)
            {
                aiTexture* tex = pScene->mTextures[i] = new aiTexture();

                // skip the texture name
                while (stream.GetI1());

                // read texture width and height
                tex->mWidth  = (unsigned int)stream.GetI4();
                tex->mHeight = (unsigned int)stream.GetI4();

                if (!tex->mWidth || !tex->mHeight)
                    throw DeadlyImportError("Quick3D: Invalid texture. Width or height is zero");

                unsigned int mul = tex->mWidth * tex->mHeight;
                aiTexel* begin = tex->pcData = new aiTexel[mul];
                aiTexel* const end = & begin [mul];

                for (;begin != end; ++begin)
                {
                    begin->r = stream.GetI1();
                    begin->g = stream.GetI1();
                    begin->b = stream.GetI1();
                    begin->a = 0xff;
                }
            }

            break;

            // scene chunk
        case 's':
            {
                // skip position and rotation
                stream.IncPtr(12);

                for (unsigned int i = 0; i < 4;++i)
                    for (unsigned int a = 0; a < 4;++a)
                        pScene->mRootNode->mTransformation[i][a] = stream.GetF4();

                stream.IncPtr(16);

                // now setup a single camera
                pScene->mNumCameras = 1;
                pScene->mCameras = new aiCamera*[1];
                aiCamera* cam = pScene->mCameras[0] = new aiCamera();
                cam->mPosition.x = stream.GetF4();
                cam->mPosition.y = stream.GetF4();
                cam->mPosition.z = stream.GetF4();
                cam->mName.Set("Q3DCamera");

                // skip eye rotation for the moment
                stream.IncPtr(12);

                // read the default material color
                fgColor .r = stream.GetF4();
                fgColor .g = stream.GetF4();
                fgColor .b = stream.GetF4();

                // skip some unimportant properties
                stream.IncPtr(29);

                // setup a single point light with no attenuation
                pScene->mNumLights = 1;
                pScene->mLights = new aiLight*[1];
                aiLight* light = pScene->mLights[0] = new aiLight();
                light->mName.Set("Q3DLight");
                light->mType = aiLightSource_POINT;

                light->mAttenuationConstant  = 1;
                light->mAttenuationLinear    = 0;
                light->mAttenuationQuadratic = 0;

                light->mColorDiffuse.r = stream.GetF4();
                light->mColorDiffuse.g = stream.GetF4();
                light->mColorDiffuse.b = stream.GetF4();

                light->mColorSpecular = light->mColorDiffuse;


                // We don't need the rest, but we need to know where this chunk ends.
                unsigned int temp = (unsigned int)(stream.GetI4() * stream.GetI4());

                // skip the background file name
                while (stream.GetI1());

                // skip background texture data + the remaining fields
                stream.IncPtr(temp*3 + 20); // 4 bytes of unknown data here

                // TODO
                goto outer;
            }
            break;

        default:
            throw DeadlyImportError("Quick3D: Unknown chunk");
            break;
        };
    }
outer:

    // If we have no mesh loaded - break here
    if (meshes.empty())
        throw DeadlyImportError("Quick3D: No meshes loaded");

    // If we have no materials loaded - generate a default mat
    if (materials.empty())
    {
        DefaultLogger::get()->info("Quick3D: No material found, generating one");
        materials.push_back(Material());
        materials.back().diffuse  = fgColor ;
    }

    // find out which materials we'll need
    typedef std::pair<unsigned int, unsigned int> FaceIdx;
    typedef std::vector< FaceIdx > FaceIdxArray;
    FaceIdxArray* fidx = new FaceIdxArray[materials.size()];

    unsigned int p = 0;
    for (std::vector<Mesh>::iterator it = meshes.begin(), end = meshes.end();
         it != end; ++it,++p)
    {
        unsigned int q = 0;
        for (std::vector<Face>::iterator fit = (*it).faces.begin(), fend = (*it).faces.end();
             fit != fend; ++fit,++q)
        {
            if ((*fit).mat >= materials.size())
            {
                DefaultLogger::get()->warn("Quick3D: Material index overflow");
                (*fit).mat = 0;
            }
            if (fidx[(*fit).mat].empty())++pScene->mNumMeshes;
            fidx[(*fit).mat].push_back( FaceIdx(p,q) );
        }
    }
    pScene->mNumMaterials = pScene->mNumMeshes;
    pScene->mMaterials = new aiMaterial*[pScene->mNumMaterials];
    pScene->mMeshes = new aiMesh*[pScene->mNumMaterials];

    for (unsigned int i = 0, real = 0; i < (unsigned int)materials.size(); ++i)
    {
        if (fidx[i].empty())continue;

        // Allocate a mesh and a material
        aiMesh* mesh = pScene->mMeshes[real] = new aiMesh();
        aiMaterial* mat = new aiMaterial();
        pScene->mMaterials[real] = mat;

        mesh->mMaterialIndex = real;

        // Build the output material
        Material& srcMat = materials[i];
        mat->AddProperty(&srcMat.diffuse,  1,AI_MATKEY_COLOR_DIFFUSE);
        mat->AddProperty(&srcMat.specular, 1,AI_MATKEY_COLOR_SPECULAR);
        mat->AddProperty(&srcMat.ambient,  1,AI_MATKEY_COLOR_AMBIENT);

        // NOTE: Ignore transparency for the moment - it seems
        // unclear how to interpret the data
#if 0
        if (!(minor > '0' && major == '3'))
            srcMat.transparency = 1.0f - srcMat.transparency;
        mat->AddProperty(&srcMat.transparency, 1, AI_MATKEY_OPACITY);
#endif

        // add shininess - Quick3D seems to use it ins its viewer
        srcMat.transparency = 16.f;
        mat->AddProperty(&srcMat.transparency, 1, AI_MATKEY_SHININESS);

        int m = (int)aiShadingMode_Phong;
        mat->AddProperty(&m, 1, AI_MATKEY_SHADING_MODEL);

        if (srcMat.name.length)
            mat->AddProperty(&srcMat.name,AI_MATKEY_NAME);

        // Add a texture
        if (srcMat.texIdx < pScene->mNumTextures || real < pScene->mNumTextures)
        {
            srcMat.name.data[0] = '*';
            srcMat.name.length  = ASSIMP_itoa10(&srcMat.name.data[1],1000,
                (srcMat.texIdx < pScene->mNumTextures ? srcMat.texIdx : real));
            mat->AddProperty(&srcMat.name,AI_MATKEY_TEXTURE_DIFFUSE(0));
        }

        mesh->mNumFaces = (unsigned int)fidx[i].size();
        aiFace* faces = mesh->mFaces = new aiFace[mesh->mNumFaces];

        // Now build the output mesh. First find out how many
        // vertices we'll need
        for (FaceIdxArray::const_iterator it = fidx[i].begin(),end = fidx[i].end();
             it != end; ++it)
        {
            mesh->mNumVertices += (unsigned int)meshes[(*it).first].faces[
                (*it).second].indices.size();
        }

        aiVector3D* verts = mesh->mVertices = new aiVector3D[mesh->mNumVertices];
        aiVector3D* norms = mesh->mNormals  = new aiVector3D[mesh->mNumVertices];
        aiVector3D* uv;
        if (real < pScene->mNumTextures)
        {
            uv = mesh->mTextureCoords[0] =  new aiVector3D[mesh->mNumVertices];
            mesh->mNumUVComponents[0]    =  2;
        }
        else uv = NULL;

        // Build the final array
        unsigned int cnt = 0;
        for (FaceIdxArray::const_iterator it = fidx[i].begin(),end = fidx[i].end();
             it != end; ++it, ++faces)
        {
            Mesh& m    = meshes[(*it).first];
            Face& face = m.faces[(*it).second];
            faces->mNumIndices = (unsigned int)face.indices.size();
            faces->mIndices = new unsigned int [faces->mNumIndices];


            aiVector3D faceNormal;
            bool fnOK = false;

            for (unsigned int n = 0; n < faces->mNumIndices;++n, ++cnt, ++norms, ++verts)
            {
                if (face.indices[n] >= m.verts.size())
                {
                    DefaultLogger::get()->warn("Quick3D: Vertex index overflow");
                    face.indices[n] = 0;
                }

                // copy vertices
                *verts =  m.verts[ face.indices[n] ];

                if (face.indices[n] >= m.normals.size() && faces->mNumIndices >= 3)
                {
                    // we have no normal here - assign the face normal
                    if (!fnOK)
                    {
                        const aiVector3D& pV1 =  m.verts[ face.indices[0] ];
                        const aiVector3D& pV2 =  m.verts[ face.indices[1] ];
                        const aiVector3D& pV3 =  m.verts[ face.indices.size() - 1 ];
                        faceNormal = (pV2 - pV1) ^ (pV3 - pV1).Normalize();
                        fnOK = true;
                    }
                    *norms = faceNormal;
                }
                else *norms =  m.normals[ face.indices[n] ];

                // copy texture coordinates
                if (uv && m.uv.size())
                {
                    if (m.prevUVIdx != 0xffffffff && m.uv.size() >= m.verts.size()) // workaround
                    {
                        *uv = m.uv[face.indices[n]];
                    }
                    else
                    {
                        if (face.uvindices[n] >= m.uv.size())
                        {
                            DefaultLogger::get()->warn("Quick3D: Texture coordinate index overflow");
                            face.uvindices[n] = 0;
                        }
                        *uv = m.uv[face.uvindices[n]];
                    }
                    uv->y = 1.f - uv->y;
                    ++uv;
                }

                // setup the new vertex index
                faces->mIndices[n] = cnt;
            }

        }
        ++real;
    }

    // Delete our nice helper array
    delete[] fidx;

    // Now we need to attach the meshes to the root node of the scene
    pScene->mRootNode->mNumMeshes = pScene->mNumMeshes;
    pScene->mRootNode->mMeshes = new unsigned int [pScene->mNumMeshes];
    for (unsigned int i = 0; i < pScene->mNumMeshes;++i)
        pScene->mRootNode->mMeshes[i] = i;

    /*pScene->mRootNode->mTransformation *= aiMatrix4x4(
        1.f, 0.f, 0.f, 0.f,
        0.f, -1.f,0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f);*/

    // Add cameras and light sources to the scene root node
    pScene->mRootNode->mNumChildren = pScene->mNumLights+pScene->mNumCameras;
    if (pScene->mRootNode->mNumChildren)
    {
        pScene->mRootNode->mChildren = new aiNode* [ pScene->mRootNode->mNumChildren ];

        // the light source
        aiNode* nd = pScene->mRootNode->mChildren[0] = new aiNode();
        nd->mParent = pScene->mRootNode;
        nd->mName.Set("Q3DLight");
        nd->mTransformation = pScene->mRootNode->mTransformation;
        nd->mTransformation.Inverse();

        // camera
        nd = pScene->mRootNode->mChildren[1] = new aiNode();
        nd->mParent = pScene->mRootNode;
        nd->mName.Set("Q3DCamera");
        nd->mTransformation = pScene->mRootNode->mChildren[0]->mTransformation;
    }
}

#endif // !! ASSIMP_BUILD_NO_Q3D_IMPORTER
