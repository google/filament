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

/** @file Implementation of the IrrMesh importer class */



#ifndef ASSIMP_BUILD_NO_IRRMESH_IMPORTER

#include "IRRMeshLoader.h"
#include <assimp/ParsingUtils.h>
#include <assimp/fast_atof.h>
#include <memory>
#include <assimp/IOSystem.hpp>
#include <assimp/mesh.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/importerdesc.h>
#include <assimp/Macros.h>

using namespace Assimp;
using namespace irr;
using namespace irr::io;

static const aiImporterDesc desc = {
    "Irrlicht Mesh Reader",
    "",
    "",
    "http://irrlicht.sourceforge.net/",
    aiImporterFlags_SupportTextFlavour,
    0,
    0,
    0,
    0,
    "xml irrmesh"
};

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
IRRMeshImporter::IRRMeshImporter()
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
IRRMeshImporter::~IRRMeshImporter()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool IRRMeshImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    /* NOTE: A simple check for the file extension is not enough
     * here. Irrmesh and irr are easy, but xml is too generic
     * and could be collada, too. So we need to open the file and
     * search for typical tokens.
     */
    const std::string extension = GetExtension(pFile);

    if (extension == "irrmesh")return true;
    else if (extension == "xml" || checkSig)
    {
        /*  If CanRead() is called to check whether the loader
         *  supports a specific file extension in general we
         *  must return true here.
         */
        if (!pIOHandler)return true;
        const char* tokens[] = {"irrmesh"};
        return SearchFileHeaderForToken(pIOHandler,pFile,tokens,1);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Get a list of all file extensions which are handled by this class
const aiImporterDesc* IRRMeshImporter::GetInfo () const
{
    return &desc;
}

static void releaseMaterial( aiMaterial **mat ) {
    if(*mat!= nullptr) {
        delete *mat;
        *mat = nullptr;
    }
}

static void releaseMesh( aiMesh **mesh ) {
    if (*mesh != nullptr){
        delete *mesh;
        *mesh = nullptr;
    }
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void IRRMeshImporter::InternReadFile( const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile));

    // Check whether we can read from the file
    if( file.get() == NULL)
        throw DeadlyImportError( "Failed to open IRRMESH file " + pFile + "");

    // Construct the irrXML parser
    CIrrXML_IOStreamReader st(file.get());
    reader = createIrrXMLReader((IFileReadCallBack*) &st);

    // final data
    std::vector<aiMaterial*> materials;
    std::vector<aiMesh*>     meshes;
    materials.reserve (5);
    meshes.reserve(5);

    // temporary data - current mesh buffer
    aiMaterial* curMat  = nullptr;
    aiMesh* curMesh     = nullptr;
    unsigned int curMatFlags = 0;

    std::vector<aiVector3D> curVertices,curNormals,curTangents,curBitangents;
    std::vector<aiColor4D>  curColors;
    std::vector<aiVector3D> curUVs,curUV2s;

    // some temporary variables
    int textMeaning = 0;
    int vertexFormat = 0; // 0 = normal; 1 = 2 tcoords, 2 = tangents
    bool useColors = false;

    // Parse the XML file
    while (reader->read())  {
        switch (reader->getNodeType())  {
        case EXN_ELEMENT:

            if (!ASSIMP_stricmp(reader->getNodeName(),"buffer") && (curMat || curMesh)) {
                // end of previous buffer. A material and a mesh should be there
                if ( !curMat || !curMesh)   {
                    ASSIMP_LOG_ERROR("IRRMESH: A buffer must contain a mesh and a material");
                    releaseMaterial( &curMat );
                    releaseMesh( &curMesh );
                } else {
                    materials.push_back(curMat);
                    meshes.push_back(curMesh);
                }
                curMat  = nullptr;
                curMesh = nullptr;

                curVertices.clear();
                curColors.clear();
                curNormals.clear();
                curUV2s.clear();
                curUVs.clear();
                curTangents.clear();
                curBitangents.clear();
            }


            if (!ASSIMP_stricmp(reader->getNodeName(),"material"))  {
                if (curMat) {
                    ASSIMP_LOG_WARN("IRRMESH: Only one material description per buffer, please");
                    releaseMaterial( &curMat );
                }
                curMat = ParseMaterial(curMatFlags);
            }
            /* no else here! */ if (!ASSIMP_stricmp(reader->getNodeName(),"vertices"))
            {
                int num = reader->getAttributeValueAsInt("vertexCount");

                if (!num)   {
                    // This is possible ... remove the mesh from the list and skip further reading
                    ASSIMP_LOG_WARN("IRRMESH: Found mesh with zero vertices");

                    releaseMaterial( &curMat );
                    releaseMesh( &curMesh );
                    textMeaning = 0;
                    continue;
                }

                curVertices.reserve(num);
                curNormals.reserve(num);
                curColors.reserve(num);
                curUVs.reserve(num);

                // Determine the file format
                const char* t = reader->getAttributeValueSafe("type");
                if (!ASSIMP_stricmp("2tcoords", t)) {
                    curUV2s.reserve (num);
                    vertexFormat = 1;

                    if (curMatFlags & AI_IRRMESH_EXTRA_2ND_TEXTURE) {
                        // *********************************************************
                        // We have a second texture! So use this UV channel
                        // for it. The 2nd texture can be either a normal
                        // texture (solid_2layer or lightmap_xxx) or a normal
                        // map (normal_..., parallax_...)
                        // *********************************************************
                        int idx = 1;
                        aiMaterial* mat = ( aiMaterial* ) curMat;

                        if (curMatFlags & AI_IRRMESH_MAT_lightmap){
                            mat->AddProperty(&idx,1,AI_MATKEY_UVWSRC_LIGHTMAP(0));
                        }
                        else if (curMatFlags & AI_IRRMESH_MAT_normalmap_solid){
                            mat->AddProperty(&idx,1,AI_MATKEY_UVWSRC_NORMALS(0));
                        }
                        else if (curMatFlags & AI_IRRMESH_MAT_solid_2layer) {
                            mat->AddProperty(&idx,1,AI_MATKEY_UVWSRC_DIFFUSE(1));
                        }
                    }
                }
                else if (!ASSIMP_stricmp("tangents", t))    {
                    curTangents.reserve (num);
                    curBitangents.reserve (num);
                    vertexFormat = 2;
                }
                else if (ASSIMP_stricmp("standard", t)) {
                    releaseMaterial( &curMat );
                    ASSIMP_LOG_WARN("IRRMESH: Unknown vertex format");
                }
                else vertexFormat = 0;
                textMeaning = 1;
            }
            else if (!ASSIMP_stricmp(reader->getNodeName(),"indices"))  {
                if (curVertices.empty() && curMat)  {
                    releaseMaterial( &curMat );
                    throw DeadlyImportError("IRRMESH: indices must come after vertices");
                }

                textMeaning = 2;

                // start a new mesh
                curMesh = new aiMesh();

                // allocate storage for all faces
                curMesh->mNumVertices = reader->getAttributeValueAsInt("indexCount");
                if (!curMesh->mNumVertices) {
                    // This is possible ... remove the mesh from the list and skip further reading
                    ASSIMP_LOG_WARN("IRRMESH: Found mesh with zero indices");

                    // mesh - away
                    releaseMesh( &curMesh );

                    // material - away
                    releaseMaterial( &curMat );

                    textMeaning = 0;
                    continue;
                }

                if (curMesh->mNumVertices % 3)  {
                    ASSIMP_LOG_WARN("IRRMESH: Number if indices isn't divisible by 3");
                }

                curMesh->mNumFaces = curMesh->mNumVertices / 3;
                curMesh->mFaces = new aiFace[curMesh->mNumFaces];

                // setup some members
                curMesh->mMaterialIndex = (unsigned int)materials.size();
                curMesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;

                // allocate storage for all vertices
                curMesh->mVertices = new aiVector3D[curMesh->mNumVertices];

                if (curNormals.size() == curVertices.size())    {
                    curMesh->mNormals = new aiVector3D[curMesh->mNumVertices];
                }
                if (curTangents.size() == curVertices.size())   {
                    curMesh->mTangents = new aiVector3D[curMesh->mNumVertices];
                }
                if (curBitangents.size() == curVertices.size()) {
                    curMesh->mBitangents = new aiVector3D[curMesh->mNumVertices];
                }
                if (curColors.size() == curVertices.size() && useColors)    {
                    curMesh->mColors[0] = new aiColor4D[curMesh->mNumVertices];
                }
                if (curUVs.size() == curVertices.size())    {
                    curMesh->mTextureCoords[0] = new aiVector3D[curMesh->mNumVertices];
                }
                if (curUV2s.size() == curVertices.size())   {
                    curMesh->mTextureCoords[1] = new aiVector3D[curMesh->mNumVertices];
                }
            }
            break;

        case EXN_TEXT:
            {
            const char* sz = reader->getNodeData();
            if (textMeaning == 1)   {
                textMeaning = 0;

                // read vertices
                do  {
                    SkipSpacesAndLineEnd(&sz);
                    aiVector3D temp;aiColor4D c;

                    // Read the vertex position
                    sz = fast_atoreal_move<float>(sz,(float&)temp.x);
                    SkipSpaces(&sz);

                    sz = fast_atoreal_move<float>(sz,(float&)temp.y);
                    SkipSpaces(&sz);

                    sz = fast_atoreal_move<float>(sz,(float&)temp.z);
                    SkipSpaces(&sz);
                    curVertices.push_back(temp);

                    // Read the vertex normals
                    sz = fast_atoreal_move<float>(sz,(float&)temp.x);
                    SkipSpaces(&sz);

                    sz = fast_atoreal_move<float>(sz,(float&)temp.y);
                    SkipSpaces(&sz);

                    sz = fast_atoreal_move<float>(sz,(float&)temp.z);
                    SkipSpaces(&sz);
                    curNormals.push_back(temp);

                    // read the vertex colors
                    uint32_t clr = strtoul16(sz,&sz);
                    ColorFromARGBPacked(clr,c);

                    if (!curColors.empty() && c != *(curColors.end()-1))
                        useColors = true;

                    curColors.push_back(c);
                    SkipSpaces(&sz);


                    // read the first UV coordinate set
                    sz = fast_atoreal_move<float>(sz,(float&)temp.x);
                    SkipSpaces(&sz);

                    sz = fast_atoreal_move<float>(sz,(float&)temp.y);
                    SkipSpaces(&sz);
                    temp.z = 0.f;
                    temp.y = 1.f - temp.y;  // DX to OGL
                    curUVs.push_back(temp);

                    // read the (optional) second UV coordinate set
                    if (vertexFormat == 1)  {
                        sz = fast_atoreal_move<float>(sz,(float&)temp.x);
                        SkipSpaces(&sz);

                        sz = fast_atoreal_move<float>(sz,(float&)temp.y);
                        temp.y = 1.f - temp.y; // DX to OGL
                        curUV2s.push_back(temp);
                    }
                    // read optional tangent and bitangent vectors
                    else if (vertexFormat == 2) {
                        // tangents
                        sz = fast_atoreal_move<float>(sz,(float&)temp.x);
                        SkipSpaces(&sz);

                        sz = fast_atoreal_move<float>(sz,(float&)temp.z);
                        SkipSpaces(&sz);

                        sz = fast_atoreal_move<float>(sz,(float&)temp.y);
                        SkipSpaces(&sz);
                        temp.y *= -1.0f;
                        curTangents.push_back(temp);

                        // bitangents
                        sz = fast_atoreal_move<float>(sz,(float&)temp.x);
                        SkipSpaces(&sz);

                        sz = fast_atoreal_move<float>(sz,(float&)temp.z);
                        SkipSpaces(&sz);

                        sz = fast_atoreal_move<float>(sz,(float&)temp.y);
                        SkipSpaces(&sz);
                        temp.y *= -1.0f;
                        curBitangents.push_back(temp);
                    }
                }

                /* IMPORTANT: We assume that each vertex is specified in one
                   line. So we can skip the rest of the line - unknown vertex
                   elements are ignored.
                 */

                while (SkipLine(&sz));
            }
            else if (textMeaning == 2)  {
                textMeaning = 0;

                // read indices
                aiFace* curFace = curMesh->mFaces;
                aiFace* const faceEnd = curMesh->mFaces  + curMesh->mNumFaces;

                aiVector3D* pcV  = curMesh->mVertices;
                aiVector3D* pcN  = curMesh->mNormals;
                aiVector3D* pcT  = curMesh->mTangents;
                aiVector3D* pcB  = curMesh->mBitangents;
                aiColor4D* pcC0  = curMesh->mColors[0];
                aiVector3D* pcT0 = curMesh->mTextureCoords[0];
                aiVector3D* pcT1 = curMesh->mTextureCoords[1];

                unsigned int curIdx = 0;
                unsigned int total = 0;
                while(SkipSpacesAndLineEnd(&sz))    {
                    if (curFace >= faceEnd) {
                        ASSIMP_LOG_ERROR("IRRMESH: Too many indices");
                        break;
                    }
                    if (!curIdx)    {
                        curFace->mNumIndices = 3;
                        curFace->mIndices = new unsigned int[3];
                    }

                    unsigned int idx = strtoul10(sz,&sz);
                    if (idx >= curVertices.size())  {
                        ASSIMP_LOG_ERROR("IRRMESH: Index out of range");
                        idx = 0;
                    }

                    curFace->mIndices[curIdx] = total++;

                    *pcV++ = curVertices[idx];
                    if (pcN)*pcN++ = curNormals[idx];
                    if (pcT)*pcT++ = curTangents[idx];
                    if (pcB)*pcB++ = curBitangents[idx];
                    if (pcC0)*pcC0++ = curColors[idx];
                    if (pcT0)*pcT0++ = curUVs[idx];
                    if (pcT1)*pcT1++ = curUV2s[idx];

                    if (++curIdx == 3)  {
                        ++curFace;
                        curIdx = 0;
                    }
                }

                if (curFace != faceEnd)
                    ASSIMP_LOG_ERROR("IRRMESH: Not enough indices");

                // Finish processing the mesh - do some small material workarounds
                if (curMatFlags & AI_IRRMESH_MAT_trans_vertex_alpha && !useColors)  {
                    // Take the opacity value of the current material
                    // from the common vertex color alpha
                    aiMaterial* mat = (aiMaterial*)curMat;
                    mat->AddProperty(&curColors[0].a,1,AI_MATKEY_OPACITY);
                }
            }}
            break;

            default:
                // GCC complains here ...
                break;

        };
    }

    // End of the last buffer. A material and a mesh should be there
    if (curMat || curMesh)  {
        if ( !curMat || !curMesh)   {
            ASSIMP_LOG_ERROR("IRRMESH: A buffer must contain a mesh and a material");
            releaseMaterial( &curMat );
            releaseMesh( &curMesh );
        }
        else    {
            materials.push_back(curMat);
            meshes.push_back(curMesh);
        }
    }

    if (materials.empty())
        throw DeadlyImportError("IRRMESH: Unable to read a mesh from this file");


    // now generate the output scene
    pScene->mNumMeshes = (unsigned int)meshes.size();
    pScene->mMeshes = new aiMesh*[pScene->mNumMeshes];
    for (unsigned int i = 0; i < pScene->mNumMeshes;++i)    {
        pScene->mMeshes[i] = meshes[i];

        // clean this value ...
        pScene->mMeshes[i]->mNumUVComponents[3] = 0;
    }

    pScene->mNumMaterials = (unsigned int)materials.size();
    pScene->mMaterials = new aiMaterial*[pScene->mNumMaterials];
    ::memcpy(pScene->mMaterials,&materials[0],sizeof(void*)*pScene->mNumMaterials);

    pScene->mRootNode = new aiNode();
    pScene->mRootNode->mName.Set("<IRRMesh>");
    pScene->mRootNode->mNumMeshes = pScene->mNumMeshes;
    pScene->mRootNode->mMeshes = new unsigned int[pScene->mNumMeshes];

    for (unsigned int i = 0; i < pScene->mNumMeshes;++i)
        pScene->mRootNode->mMeshes[i] = i;

    // clean up and return
    delete reader;
    AI_DEBUG_INVALIDATE_PTR(reader);
}

#endif // !! ASSIMP_BUILD_NO_IRRMESH_IMPORTER
