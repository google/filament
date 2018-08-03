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

/** @file Implementation of the STL importer class */


#ifndef ASSIMP_BUILD_NO_NFF_IMPORTER

// internal headers
#include "NFFLoader.h"
#include "ParsingUtils.h"
#include "StandardShapes.h"
#include "qnan.h"
#include "fast_atof.h"
#include "RemoveComments.h"
#include <assimp/IOSystem.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/scene.h>
#include <assimp/importerdesc.h>
#include <memory>


using namespace Assimp;

static const aiImporterDesc desc = {
    "Neutral File Format Importer",
    "",
    "",
    "",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "enff nff"
};

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
NFFImporter::NFFImporter()
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
NFFImporter::~NFFImporter()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool NFFImporter::CanRead( const std::string& pFile, IOSystem* /*pIOHandler*/, bool /*checkSig*/) const
{
    return SimpleExtensionCheck(pFile,"nff","enff");
}

// ------------------------------------------------------------------------------------------------
// Get the list of all supported file extensions
const aiImporterDesc* NFFImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
#define AI_NFF_PARSE_FLOAT(f) \
    SkipSpaces(&sz); \
    if (!::IsLineEnd(*sz))sz = fast_atoreal_move<float>(sz, (float&)f);

// ------------------------------------------------------------------------------------------------
#define AI_NFF_PARSE_TRIPLE(v) \
    AI_NFF_PARSE_FLOAT(v[0]) \
    AI_NFF_PARSE_FLOAT(v[1]) \
    AI_NFF_PARSE_FLOAT(v[2])

// ------------------------------------------------------------------------------------------------
#define AI_NFF_PARSE_SHAPE_INFORMATION() \
    aiVector3D center, radius(1.0f,get_qnan(),get_qnan()); \
    AI_NFF_PARSE_TRIPLE(center); \
    AI_NFF_PARSE_TRIPLE(radius); \
    if (is_qnan(radius.z))radius.z = radius.x; \
    if (is_qnan(radius.y))radius.y = radius.x; \
    currentMesh.radius = radius; \
    currentMesh.center = center;

// ------------------------------------------------------------------------------------------------
#define AI_NFF2_GET_NEXT_TOKEN() \
    do \
    { \
    if (!GetNextLine(buffer,line)) \
        {DefaultLogger::get()->warn("NFF2: Unexpected EOF, can't read next token");break;} \
    SkipSpaces(line,&sz); \
    } \
    while(IsLineEnd(*sz))


// ------------------------------------------------------------------------------------------------
// Loads the materail table for the NFF2 file format from an external file
void NFFImporter::LoadNFF2MaterialTable(std::vector<ShadingInfo>& output,
    const std::string& path, IOSystem* pIOHandler)
{
    std::unique_ptr<IOStream> file( pIOHandler->Open( path, "rb"));

    // Check whether we can read from the file
    if( !file.get())    {
        DefaultLogger::get()->error("NFF2: Unable to open material library " + path + ".");
        return;
    }

    // get the size of the file
    const unsigned int m = (unsigned int)file->FileSize();

    // allocate storage and copy the contents of the file to a memory buffer
    // (terminate it with zero)
    std::vector<char> mBuffer2(m+1);
    TextFileToBuffer(file.get(),mBuffer2);
    const char* buffer = &mBuffer2[0];

    // First of all: remove all comments from the file
    CommentRemover::RemoveLineComments("//",&mBuffer2[0]);

    // The file should start with the magic sequence "mat"
    if (!TokenMatch(buffer,"mat",3))    {
        DefaultLogger::get()->error("NFF2: Not a valid material library " + path + ".");
        return;
    }

    ShadingInfo* curShader = NULL;

    // No read the file line per line
    char line[4096];
    const char* sz;
    while (GetNextLine(buffer,line))
    {
        SkipSpaces(line,&sz);

        // 'version' defines the version of the file format
        if (TokenMatch(sz,"version",7))
        {
            DefaultLogger::get()->info("NFF (Sense8) material library file format: " + std::string(sz));
        }
        // 'matdef' starts a new material in the file
        else if (TokenMatch(sz,"matdef",6))
        {
            // add a new material to the list
            output.push_back( ShadingInfo() );
            curShader = & output.back();

            // parse the name of the material
        }
        else if (!TokenMatch(sz,"valid",5))
        {
            // check whether we have an active material at the moment
            if (!IsLineEnd(*sz))
            {
                if (!curShader)
                {
                    DefaultLogger::get()->error(std::string("NFF2 material library: Found element ") +
                        sz + "but there is no active material");
                    continue;
                }
            }
            else continue;

            // now read the material property and determine its type
            aiColor3D c;
            if (TokenMatch(sz,"ambient",7))
            {
                AI_NFF_PARSE_TRIPLE(c);
                curShader->ambient = c;
            }
            else if (TokenMatch(sz,"diffuse",7) || TokenMatch(sz,"ambientdiffuse",14) /* correct? */)
            {
                AI_NFF_PARSE_TRIPLE(c);
                curShader->diffuse = curShader->ambient = c;
            }
            else if (TokenMatch(sz,"specular",8))
            {
                AI_NFF_PARSE_TRIPLE(c);
                curShader->specular = c;
            }
            else if (TokenMatch(sz,"emission",8))
            {
                AI_NFF_PARSE_TRIPLE(c);
                curShader->emissive = c;
            }
            else if (TokenMatch(sz,"shininess",9))
            {
                AI_NFF_PARSE_FLOAT(curShader->shininess);
            }
            else if (TokenMatch(sz,"opacity",7))
            {
                AI_NFF_PARSE_FLOAT(curShader->opacity);
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void NFFImporter::InternReadFile( const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile, "rb"));

    // Check whether we can read from the file
    if( !file.get())
        throw DeadlyImportError( "Failed to open NFF file " + pFile + ".");

    // allocate storage and copy the contents of the file to a memory buffer
    // (terminate it with zero)
    std::vector<char> mBuffer2;
    TextFileToBuffer(file.get(),mBuffer2);
    const char* buffer = &mBuffer2[0];

    // mesh arrays - separate here to make the handling of the pointers below easier.
    std::vector<MeshInfo> meshes;
    std::vector<MeshInfo> meshesWithNormals;
    std::vector<MeshInfo> meshesWithUVCoords;
    std::vector<MeshInfo> meshesLocked;

    char line[4096];
    const char* sz;

    // camera parameters
    aiVector3D camPos, camUp(0.f,1.f,0.f), camLookAt(0.f,0.f,1.f);
    float angle = 45.f;
    aiVector2D resolution;

    bool hasCam = false;

    MeshInfo* currentMeshWithNormals = NULL;
    MeshInfo* currentMesh = NULL;
    MeshInfo* currentMeshWithUVCoords = NULL;

    ShadingInfo s; // current material info

    // degree of tesselation
    unsigned int iTesselation = 4;

    // some temporary variables we need to parse the file
    unsigned int sphere     = 0,
        cylinder            = 0,
        cone                = 0,
        numNamed            = 0,
        dodecahedron        = 0,
        octahedron          = 0,
        tetrahedron         = 0,
        hexahedron          = 0;

    // lights imported from the file
    std::vector<Light> lights;

    // check whether this is the NFF2 file format
    if (TokenMatch(buffer,"nff",3))
    {
        const float qnan = get_qnan();
        const aiColor4D  cQNAN = aiColor4D (qnan,0.f,0.f,1.f);
        const aiVector3D vQNAN = aiVector3D(qnan,0.f,0.f);

        // another NFF file format ... just a raw parser has been implemented
        // no support for further details, I don't think it is worth the effort
        // http://ozviz.wasp.uwa.edu.au/~pbourke/dataformats/nff/nff2.html
        // http://www.netghost.narod.ru/gff/graphics/summary/sense8.htm

        // First of all: remove all comments from the file
        CommentRemover::RemoveLineComments("//",&mBuffer2[0]);

        while (GetNextLine(buffer,line))
        {
            SkipSpaces(line,&sz);
            if (TokenMatch(sz,"version",7))
            {
                DefaultLogger::get()->info("NFF (Sense8) file format: " + std::string(sz));
            }
            else if (TokenMatch(sz,"viewpos",7))
            {
                AI_NFF_PARSE_TRIPLE(camPos);
                hasCam = true;
            }
            else if (TokenMatch(sz,"viewdir",7))
            {
                AI_NFF_PARSE_TRIPLE(camLookAt);
                hasCam = true;
            }
            // This starts a new object section
            else if (!IsSpaceOrNewLine(*sz))
            {
                unsigned int subMeshIdx = 0;

                // read the name of the object, skip all spaces
                // at the end of it.
                const char* sz3 = sz;
                while (!IsSpaceOrNewLine(*sz))++sz;
                std::string objectName = std::string(sz3,(unsigned int)(sz-sz3));

                const unsigned int objStart = (unsigned int)meshes.size();

                // There could be a material table in a separate file
                std::vector<ShadingInfo> materialTable;
                while (true)
                {
                    AI_NFF2_GET_NEXT_TOKEN();

                    // material table - an external file
                    if (TokenMatch(sz,"mtable",6))
                    {
                        SkipSpaces(&sz);
                        sz3 = sz;
                        while (!IsSpaceOrNewLine(*sz))++sz;
                        const unsigned int diff = (unsigned int)(sz-sz3);
                        if (!diff)DefaultLogger::get()->warn("NFF2: Found empty mtable token");
                        else
                        {
                            // The material table has the file extension .mat.
                            // If it is not there, we need to append it
                            std::string path = std::string(sz3,diff);
                            if(std::string::npos == path.find_last_of(".mat"))
                            {
                                path.append(".mat");
                            }

                            // Now extract the working directory from the path to
                            // this file and append the material library filename
                            // to it.
                            std::string::size_type s;
                            if ((std::string::npos == (s = path.find_last_of('\\')) || !s) &&
                                (std::string::npos == (s = path.find_last_of('/'))  || !s) )
                            {
                                s = pFile.find_last_of('\\');
                                if (std::string::npos == s)s = pFile.find_last_of('/');
                                if (std::string::npos != s)
                                {
                                    path = pFile.substr(0,s+1) + path;
                                }
                            }
                            LoadNFF2MaterialTable(materialTable,path,pIOHandler);
                        }
                    }
                    else break;
                }

                // read the numbr of vertices
                unsigned int num = ::strtoul10(sz,&sz);

                // temporary storage
                std::vector<aiColor4D>  tempColors;
                std::vector<aiVector3D> tempPositions,tempTextureCoords,tempNormals;

                bool hasNormals = false,hasUVs = false,hasColor = false;

                tempPositions.reserve      (num);
                tempColors.reserve         (num);
                tempNormals.reserve        (num);
                tempTextureCoords.reserve  (num);
                for (unsigned int i = 0; i < num; ++i)
                {
                    AI_NFF2_GET_NEXT_TOKEN();
                    aiVector3D v;
                    AI_NFF_PARSE_TRIPLE(v);
                    tempPositions.push_back(v);

                    // parse all other attributes in the line
                    while (true)
                    {
                        SkipSpaces(&sz);
                        if (IsLineEnd(*sz))break;

                        // color definition
                        if (TokenMatch(sz,"0x",2))
                        {
                            hasColor = true;
                            unsigned int numIdx = ::strtoul16(sz,&sz);
                            aiColor4D clr;
                            clr.a = 1.f;

                            // 0xRRGGBB
                            clr.r = ((numIdx >> 16u) & 0xff) / 255.f;
                            clr.g = ((numIdx >> 8u)  & 0xff) / 255.f;
                            clr.b = ((numIdx)        & 0xff) / 255.f;
                            tempColors.push_back(clr);
                        }
                        // normal vector
                        else if (TokenMatch(sz,"norm",4))
                        {
                            hasNormals = true;
                            AI_NFF_PARSE_TRIPLE(v);
                            tempNormals.push_back(v);
                        }
                        // UV coordinate
                        else if (TokenMatch(sz,"uv",2))
                        {
                            hasUVs = true;
                            AI_NFF_PARSE_FLOAT(v.x);
                            AI_NFF_PARSE_FLOAT(v.y);
                            v.z = 0.f;
                            tempTextureCoords.push_back(v);
                        }
                    }

                    // fill in dummies for all attributes that have not been set
                    if (tempNormals.size() != tempPositions.size())
                        tempNormals.push_back(vQNAN);

                    if (tempTextureCoords.size() != tempPositions.size())
                        tempTextureCoords.push_back(vQNAN);

                    if (tempColors.size() != tempPositions.size())
                        tempColors.push_back(cQNAN);
                }

                AI_NFF2_GET_NEXT_TOKEN();
                if (!num)throw DeadlyImportError("NFF2: There are zero vertices");
                num = ::strtoul10(sz,&sz);

                std::vector<unsigned int> tempIdx;
                tempIdx.reserve(10);
                for (unsigned int i = 0; i < num; ++i)
                {
                    AI_NFF2_GET_NEXT_TOKEN();
                    SkipSpaces(line,&sz);
                    unsigned int numIdx = ::strtoul10(sz,&sz);

                    // read all faces indices
                    if (numIdx)
                    {
                        // mesh.faces.push_back(numIdx);
                        // tempIdx.erase(tempIdx.begin(),tempIdx.end());
                        tempIdx.resize(numIdx);

                        for (unsigned int a = 0; a < numIdx;++a)
                        {
                            SkipSpaces(sz,&sz);
                            unsigned int m = ::strtoul10(sz,&sz);
                            if (m >= (unsigned int)tempPositions.size())
                            {
                                DefaultLogger::get()->error("NFF2: Vertex index overflow");
                                m= 0;
                            }
                            // mesh.vertices.push_back (tempPositions[idx]);
                            tempIdx[a] = m;
                        }
                    }

                    // build a temporary shader object for the face.
                    ShadingInfo shader;
                    unsigned int matIdx = 0;

                    // white material color - we have vertex colors
                    shader.color = aiColor3D(1.f,1.f,1.f);
                    aiColor4D c  = aiColor4D(1.f,1.f,1.f,1.f);
                    while (true)
                    {
                        SkipSpaces(sz,&sz);
                        if(IsLineEnd(*sz))break;

                        // per-polygon colors
                        if (TokenMatch(sz,"0x",2))
                        {
                            hasColor = true;
                            const char* sz2 = sz;
                            numIdx = ::strtoul16(sz,&sz);
                            const unsigned int diff = (unsigned int)(sz-sz2);

                            // 0xRRGGBB
                            if (diff > 3)
                            {
                                c.r = ((numIdx >> 16u) & 0xff) / 255.f;
                                c.g = ((numIdx >> 8u)  & 0xff) / 255.f;
                                c.b = ((numIdx)        & 0xff) / 255.f;
                            }
                            // 0xRGB
                            else
                            {
                                c.r = ((numIdx >> 8u) & 0xf) / 16.f;
                                c.g = ((numIdx >> 4u) & 0xf) / 16.f;
                                c.b = ((numIdx)       & 0xf) / 16.f;
                            }
                        }
                        // TODO - implement texture mapping here
#if 0
                        // mirror vertex texture coordinate?
                        else if (TokenMatch(sz,"mirror",6))
                        {
                        }
                        // texture coordinate scaling
                        else if (TokenMatch(sz,"scale",5))
                        {
                        }
                        // texture coordinate translation
                        else if (TokenMatch(sz,"trans",5))
                        {
                        }
                        // texture coordinate rotation angle
                        else if (TokenMatch(sz,"rot",3))
                        {
                        }
#endif

                        // texture file name for this polygon + mapping information
                        else if ('_' == sz[0])
                        {
                            // get mapping information
                            switch (sz[1])
                            {
                            case 'v':
                            case 'V':

                                shader.shaded = false;
                                break;

                            case 't':
                            case 'T':
                            case 'u':
                            case 'U':

                                DefaultLogger::get()->warn("Unsupported NFF2 texture attribute: trans");
                            };
                            if (!sz[1] || '_' != sz[2])
                            {
                                DefaultLogger::get()->warn("NFF2: Expected underscore after texture attributes");
                                continue;
                            }
                            const char* sz2 = sz+3;
                            while (!IsSpaceOrNewLine( *sz ))++sz;
                            const unsigned int diff = (unsigned int)(sz-sz2);
                            if (diff)shader.texFile = std::string(sz2,diff);
                        }

                        // Two-sided material?
                        else if (TokenMatch(sz,"both",4))
                        {
                            shader.twoSided = true;
                        }

                        // Material ID?
                        else if (!materialTable.empty() && TokenMatch(sz,"matid",5))
                        {
                            SkipSpaces(&sz);
                            matIdx = ::strtoul10(sz,&sz);
                            if (matIdx >= materialTable.size())
                            {
                                DefaultLogger::get()->error("NFF2: Material index overflow.");
                                matIdx = 0;
                            }

                            // now combine our current shader with the shader we
                            // read from the material table.
                            ShadingInfo& mat = materialTable[matIdx];
                            shader.ambient   = mat.ambient;
                            shader.diffuse   = mat.diffuse;
                            shader.emissive  = mat.emissive;
                            shader.opacity   = mat.opacity;
                            shader.specular  = mat.specular;
                            shader.shininess = mat.shininess;
                        }
                        else SkipToken(sz);
                    }

                    // search the list of all shaders we have for this object whether
                    // there is an identical one. In this case, we append our mesh
                    // data to it.
                    MeshInfo* mesh = NULL;
                    for (std::vector<MeshInfo>::iterator it = meshes.begin() + objStart, end = meshes.end();
                         it != end; ++it)
                    {
                        if ((*it).shader == shader && (*it).matIndex == matIdx)
                        {
                            // we have one, we can append our data to it
                            mesh = &(*it);
                        }
                    }
                    if (!mesh)
                    {
                        meshes.push_back(MeshInfo(PatchType_Simple,false));
                        mesh = &meshes.back();
                        mesh->matIndex = matIdx;

                        // We need to add a new mesh to the list. We assign
                        // an unique name to it to make sure the scene will
                        // pass the validation step for the moment.
                        // TODO: fix naming of objects in the scenegraph later
                        if (objectName.length())
                        {
                            ::strcpy(mesh->name,objectName.c_str());
                            ASSIMP_itoa10(&mesh->name[objectName.length()],30,subMeshIdx++);
                        }

                        // copy the shader to the mesh.
                        mesh->shader = shader;
                    }

                    // fill the mesh with data
                    if (!tempIdx.empty())
                    {
                        mesh->faces.push_back((unsigned int)tempIdx.size());
                        for (std::vector<unsigned int>::const_iterator it = tempIdx.begin(), end = tempIdx.end();
                            it != end;++it)
                        {
                            unsigned int m = *it;

                            // copy colors -vertex color specifications override polygon color specifications
                            if (hasColor)
                            {
                                const aiColor4D& clr = tempColors[m];
                                mesh->colors.push_back((is_qnan( clr.r ) ? c : clr));
                            }

                            // positions should always be there
                            mesh->vertices.push_back (tempPositions[m]);

                            // copy normal vectors
                            if (hasNormals)
                                mesh->normals.push_back  (tempNormals[m]);

                            // copy texture coordinates
                            if (hasUVs)
                                mesh->uvs.push_back      (tempTextureCoords[m]);
                        }
                    }
                }
                if (!num)throw DeadlyImportError("NFF2: There are zero faces");
            }
        }
        camLookAt = camLookAt + camPos;
    }
    else // "Normal" Neutral file format that is quite more common
    {
        while (GetNextLine(buffer,line))
        {
            sz = line;
            if ('p' == line[0] || TokenMatch(sz,"tpp",3))
            {
                MeshInfo* out = NULL;

                // 'tpp' - texture polygon patch primitive
                if ('t' == line[0])
                {
                    currentMeshWithUVCoords = NULL;
                    for (auto &mesh : meshesWithUVCoords)
                    {
                        if (mesh.shader == s)
                        {
                            currentMeshWithUVCoords = &mesh;
                            break;
                        }
                    }

                    if (!currentMeshWithUVCoords)
                    {
                        meshesWithUVCoords.push_back(MeshInfo(PatchType_UVAndNormals));
                        currentMeshWithUVCoords = &meshesWithUVCoords.back();
                        currentMeshWithUVCoords->shader = s;
                    }
                    out = currentMeshWithUVCoords;
                }
                // 'pp' - polygon patch primitive
                else if ('p' == line[1])
                {
                    currentMeshWithNormals = NULL;
                    for (auto &mesh : meshesWithNormals)
                    {
                        if (mesh.shader == s)
                        {
                            currentMeshWithNormals = &mesh;
                            break;
                        }
                    }

                    if (!currentMeshWithNormals)
                    {
                        meshesWithNormals.push_back(MeshInfo(PatchType_Normals));
                        currentMeshWithNormals = &meshesWithNormals.back();
                        currentMeshWithNormals->shader = s;
                    }
                    sz = &line[2];out = currentMeshWithNormals;
                }
                // 'p' - polygon primitive
                else
                {
                    currentMesh = NULL;
                    for (auto &mesh : meshes)
                    {
                        if (mesh.shader == s)
                        {
                            currentMesh = &mesh;
                            break;
                        }
                    }

                    if (!currentMesh)
                    {
                        meshes.push_back(MeshInfo(PatchType_Simple));
                        currentMesh = &meshes.back();
                        currentMesh->shader = s;
                    }
                    sz = &line[1];out = currentMesh;
                }
                SkipSpaces(sz,&sz);
                unsigned int m = strtoul10(sz);

                // ---- flip the face order
                out->vertices.resize(out->vertices.size()+m);
                if (out != currentMesh)
                {
                    out->normals.resize(out->vertices.size());
                }
                if (out == currentMeshWithUVCoords)
                {
                    out->uvs.resize(out->vertices.size());
                }
                for (unsigned int n = 0; n < m;++n)
                {
                    if(!GetNextLine(buffer,line))
                    {
                        DefaultLogger::get()->error("NFF: Unexpected EOF was encountered. Patch definition incomplete");
                        continue;
                    }

                    aiVector3D v; sz = &line[0];
                    AI_NFF_PARSE_TRIPLE(v);
                    out->vertices[out->vertices.size()-n-1] = v;

                    if (out != currentMesh)
                    {
                        AI_NFF_PARSE_TRIPLE(v);
                        out->normals[out->vertices.size()-n-1] = v;
                    }
                    if (out == currentMeshWithUVCoords)
                    {
                        // FIX: in one test file this wraps over multiple lines
                        SkipSpaces(&sz);
                        if (IsLineEnd(*sz))
                        {
                            GetNextLine(buffer,line);
                            sz = line;
                        }
                        AI_NFF_PARSE_FLOAT(v.x);
                        SkipSpaces(&sz);
                        if (IsLineEnd(*sz))
                        {
                            GetNextLine(buffer,line);
                            sz = line;
                        }
                        AI_NFF_PARSE_FLOAT(v.y);
                        v.y = 1.f - v.y;
                        out->uvs[out->vertices.size()-n-1] = v;
                    }
                }
                out->faces.push_back(m);
            }
            // 'f' - shading information block
            else if (TokenMatch(sz,"f",1))
            {
                float d;

                // read the RGB colors
                AI_NFF_PARSE_TRIPLE(s.color);

                // read the other properties
                AI_NFF_PARSE_FLOAT(s.diffuse.r);
                AI_NFF_PARSE_FLOAT(s.specular.r);
                AI_NFF_PARSE_FLOAT(d); // skip shininess and transmittance
                AI_NFF_PARSE_FLOAT(d);
                AI_NFF_PARSE_FLOAT(s.refracti);

                // NFF2 uses full colors here so we need to use them too
                // although NFF uses simple scaling factors
                s.diffuse.g  = s.diffuse.b = s.diffuse.r;
                s.specular.g = s.specular.b = s.specular.r;

                // if the next one is NOT a number we assume it is a texture file name
                // this feature is used by some NFF files on the internet and it has
                // been implemented as it can be really useful
                SkipSpaces(&sz);
                if (!IsNumeric(*sz))
                {
                    // TODO: Support full file names with spaces and quotation marks ...
                    const char* p = sz;
                    while (!IsSpaceOrNewLine( *sz ))++sz;

                    unsigned int diff = (unsigned int)(sz-p);
                    if (diff)
                    {
                        s.texFile = std::string(p,diff);
                    }
                }
                else
                {
                    AI_NFF_PARSE_FLOAT(s.ambient); // optional
                }
            }
            // 'shader' - other way to specify a texture
            else if (TokenMatch(sz,"shader",6))
            {
                SkipSpaces(&sz);
                const char* old = sz;
                while (!IsSpaceOrNewLine(*sz))++sz;
                s.texFile = std::string(old, (uintptr_t)sz - (uintptr_t)old);
            }
            // 'l' - light source
            else if (TokenMatch(sz,"l",1))
            {
                lights.push_back(Light());
                Light& light = lights.back();

                AI_NFF_PARSE_TRIPLE(light.position);
                AI_NFF_PARSE_FLOAT (light.intensity);
                AI_NFF_PARSE_TRIPLE(light.color);
            }
            // 's' - sphere
            else if (TokenMatch(sz,"s",1))
            {
                meshesLocked.push_back(MeshInfo(PatchType_Simple,true));
                MeshInfo& currentMesh = meshesLocked.back();
                currentMesh.shader = s;
                currentMesh.shader.mapping = aiTextureMapping_SPHERE;

                AI_NFF_PARSE_SHAPE_INFORMATION();

                // we don't need scaling or translation here - we do it in the node's transform
                StandardShapes::MakeSphere(iTesselation, currentMesh.vertices);
                currentMesh.faces.resize(currentMesh.vertices.size()/3,3);

                // generate a name for the mesh
                ::ai_snprintf(currentMesh.name,128,"sphere_%i",sphere++);
            }
            // 'dod' - dodecahedron
            else if (TokenMatch(sz,"dod",3))
            {
                meshesLocked.push_back(MeshInfo(PatchType_Simple,true));
                MeshInfo& currentMesh = meshesLocked.back();
                currentMesh.shader = s;
                currentMesh.shader.mapping = aiTextureMapping_SPHERE;

                AI_NFF_PARSE_SHAPE_INFORMATION();

                // we don't need scaling or translation here - we do it in the node's transform
                StandardShapes::MakeDodecahedron(currentMesh.vertices);
                currentMesh.faces.resize(currentMesh.vertices.size()/3,3);

                // generate a name for the mesh
                ::ai_snprintf(currentMesh.name,128,"dodecahedron_%i",dodecahedron++);
            }

            // 'oct' - octahedron
            else if (TokenMatch(sz,"oct",3))
            {
                meshesLocked.push_back(MeshInfo(PatchType_Simple,true));
                MeshInfo& currentMesh = meshesLocked.back();
                currentMesh.shader = s;
                currentMesh.shader.mapping = aiTextureMapping_SPHERE;

                AI_NFF_PARSE_SHAPE_INFORMATION();

                // we don't need scaling or translation here - we do it in the node's transform
                StandardShapes::MakeOctahedron(currentMesh.vertices);
                currentMesh.faces.resize(currentMesh.vertices.size()/3,3);

                // generate a name for the mesh
                ::ai_snprintf(currentMesh.name,128,"octahedron_%i",octahedron++);
            }

            // 'tet' - tetrahedron
            else if (TokenMatch(sz,"tet",3))
            {
                meshesLocked.push_back(MeshInfo(PatchType_Simple,true));
                MeshInfo& currentMesh = meshesLocked.back();
                currentMesh.shader = s;
                currentMesh.shader.mapping = aiTextureMapping_SPHERE;

                AI_NFF_PARSE_SHAPE_INFORMATION();

                // we don't need scaling or translation here - we do it in the node's transform
                StandardShapes::MakeTetrahedron(currentMesh.vertices);
                currentMesh.faces.resize(currentMesh.vertices.size()/3,3);

                // generate a name for the mesh
                ::ai_snprintf(currentMesh.name,128,"tetrahedron_%i",tetrahedron++);
            }

            // 'hex' - hexahedron
            else if (TokenMatch(sz,"hex",3))
            {
                meshesLocked.push_back(MeshInfo(PatchType_Simple,true));
                MeshInfo& currentMesh = meshesLocked.back();
                currentMesh.shader = s;
                currentMesh.shader.mapping = aiTextureMapping_BOX;

                AI_NFF_PARSE_SHAPE_INFORMATION();

                // we don't need scaling or translation here - we do it in the node's transform
                StandardShapes::MakeHexahedron(currentMesh.vertices);
                currentMesh.faces.resize(currentMesh.vertices.size()/3,3);

                // generate a name for the mesh
                ::ai_snprintf(currentMesh.name,128,"hexahedron_%i",hexahedron++);
            }
            // 'c' - cone
            else if (TokenMatch(sz,"c",1))
            {
                meshesLocked.push_back(MeshInfo(PatchType_Simple,true));
                MeshInfo& currentMesh = meshesLocked.back();
                currentMesh.shader = s;
                currentMesh.shader.mapping = aiTextureMapping_CYLINDER;

                if(!GetNextLine(buffer,line))
                {
                    DefaultLogger::get()->error("NFF: Unexpected end of file (cone definition not complete)");
                    break;
                }
                sz = line;

                // read the two center points and the respective radii
                aiVector3D center1, center2; float radius1, radius2;
                AI_NFF_PARSE_TRIPLE(center1);
                AI_NFF_PARSE_FLOAT(radius1);

                if(!GetNextLine(buffer,line))
                {
                    DefaultLogger::get()->error("NFF: Unexpected end of file (cone definition not complete)");
                    break;
                }
                sz = line;

                AI_NFF_PARSE_TRIPLE(center2);
                AI_NFF_PARSE_FLOAT(radius2);

                // compute the center point of the cone/cylinder -
                // it is its local transformation origin
                currentMesh.dir    =  center2-center1;
                currentMesh.center =  center1+currentMesh.dir/(ai_real)2.0;

                float f;
                if (( f = currentMesh.dir.Length()) < 10e-3f )
                {
                    DefaultLogger::get()->error("NFF: Cone height is close to zero");
                    continue;
                }
                currentMesh.dir /= f; // normalize

                // generate the cone - it consists of simple triangles
                StandardShapes::MakeCone(f, radius1, radius2,
                    integer_pow(4, iTesselation), currentMesh.vertices);

                // MakeCone() returns tris
                currentMesh.faces.resize(currentMesh.vertices.size()/3,3);

                // generate a name for the mesh. 'cone' if it a cone,
                // 'cylinder' if it is a cylinder. Funny, isn't it?
                if (radius1 != radius2)
                    ::ai_snprintf(currentMesh.name,128,"cone_%i",cone++);
                else ::ai_snprintf(currentMesh.name,128,"cylinder_%i",cylinder++);
            }
            // 'tess' - tesselation
            else if (TokenMatch(sz,"tess",4))
            {
                SkipSpaces(&sz);
                iTesselation = strtoul10(sz);
            }
            // 'from' - camera position
            else if (TokenMatch(sz,"from",4))
            {
                AI_NFF_PARSE_TRIPLE(camPos);
                hasCam = true;
            }
            // 'at' - camera look-at vector
            else if (TokenMatch(sz,"at",2))
            {
                AI_NFF_PARSE_TRIPLE(camLookAt);
                hasCam = true;
            }
            // 'up' - camera up vector
            else if (TokenMatch(sz,"up",2))
            {
                AI_NFF_PARSE_TRIPLE(camUp);
                hasCam = true;
            }
            // 'angle' - (half?) camera field of view
            else if (TokenMatch(sz,"angle",5))
            {
                AI_NFF_PARSE_FLOAT(angle);
                hasCam = true;
            }
            // 'resolution' - used to compute the screen aspect
            else if (TokenMatch(sz,"resolution",10))
            {
                AI_NFF_PARSE_FLOAT(resolution.x);
                AI_NFF_PARSE_FLOAT(resolution.y);
                hasCam = true;
            }
            // 'pb' - bezier patch. Not supported yet
            else if (TokenMatch(sz,"pb",2))
            {
                DefaultLogger::get()->error("NFF: Encountered unsupported ID: bezier patch");
            }
            // 'pn' - NURBS. Not supported yet
            else if (TokenMatch(sz,"pn",2) || TokenMatch(sz,"pnn",3))
            {
                DefaultLogger::get()->error("NFF: Encountered unsupported ID: NURBS");
            }
            // '' - comment
            else if ('#' == line[0])
            {
                const char* sz;SkipSpaces(&line[1],&sz);
                if (!IsLineEnd(*sz))DefaultLogger::get()->info(sz);
            }
        }
    }

    // copy all arrays into one large
    meshes.reserve (meshes.size()+meshesLocked.size()+meshesWithNormals.size()+meshesWithUVCoords.size());
    meshes.insert  (meshes.end(),meshesLocked.begin(),meshesLocked.end());
    meshes.insert  (meshes.end(),meshesWithNormals.begin(),meshesWithNormals.end());
    meshes.insert  (meshes.end(),meshesWithUVCoords.begin(),meshesWithUVCoords.end());

    // now generate output meshes. first find out how many meshes we'll need
    std::vector<MeshInfo>::const_iterator it = meshes.begin(), end = meshes.end();
    for (;it != end;++it)
    {
        if (!(*it).faces.empty())
        {
            ++pScene->mNumMeshes;
            if ((*it).name[0])++numNamed;
        }
    }

    // generate a dummy root node - assign all unnamed elements such
    // as polygons and polygon patches to the root node and generate
    // sub nodes for named objects such as spheres and cones.
    aiNode* const root = new aiNode();
    root->mName.Set("<NFF_Root>");
    root->mNumChildren = numNamed + (hasCam ? 1 : 0) + (unsigned int) lights.size();
    root->mNumMeshes = pScene->mNumMeshes-numNamed;

    aiNode** ppcChildren = NULL;
    unsigned int* pMeshes = NULL;
    if (root->mNumMeshes)
        pMeshes = root->mMeshes = new unsigned int[root->mNumMeshes];
    if (root->mNumChildren)
        ppcChildren = root->mChildren = new aiNode*[root->mNumChildren];

    // generate the camera
    if (hasCam)
    {
        ai_assert(ppcChildren);
        aiNode* nd = new aiNode();
        *ppcChildren = nd;
        nd->mName.Set("<NFF_Camera>");
        nd->mParent = root;

        // allocate the camera in the scene
        pScene->mNumCameras = 1;
        pScene->mCameras = new aiCamera*[1];
        aiCamera* c = pScene->mCameras[0] = new aiCamera;

        c->mName = nd->mName; // make sure the names are identical
        c->mHorizontalFOV = AI_DEG_TO_RAD( angle );
        c->mLookAt      = camLookAt - camPos;
        c->mPosition    = camPos;
        c->mUp          = camUp;

        // If the resolution is not specified in the file, we
        // need to set 1.0 as aspect.
        c->mAspect      = (!resolution.y ? 0.f : resolution.x / resolution.y);
        ++ppcChildren;
    }

    // generate light sources
    if (!lights.empty())
    {
        ai_assert(ppcChildren);
        pScene->mNumLights = (unsigned int)lights.size();
        pScene->mLights = new aiLight*[pScene->mNumLights];
        for (unsigned int i = 0; i < pScene->mNumLights;++i,++ppcChildren)
        {
            const Light& l = lights[i];

            aiNode* nd = new aiNode();
            *ppcChildren = nd;
            nd->mParent = root;

            nd->mName.length = ::ai_snprintf(nd->mName.data,1024,"<NFF_Light%u>",i);

            // allocate the light in the scene data structure
            aiLight* out = pScene->mLights[i] = new aiLight();
            out->mName = nd->mName; // make sure the names are identical
            out->mType = aiLightSource_POINT;
            out->mColorDiffuse = out->mColorSpecular = l.color * l.intensity;
            out->mPosition = l.position;
        }
    }

    if (!pScene->mNumMeshes)throw DeadlyImportError("NFF: No meshes loaded");
    pScene->mMeshes = new aiMesh*[pScene->mNumMeshes];
    pScene->mMaterials = new aiMaterial*[pScene->mNumMaterials = pScene->mNumMeshes];
    unsigned int m = 0;
    for (it = meshes.begin(); it != end;++it)
    {
        if ((*it).faces.empty())continue;

        const MeshInfo& src = *it;
        aiMesh* const mesh = pScene->mMeshes[m] = new aiMesh();
        mesh->mNumVertices = (unsigned int)src.vertices.size();
        mesh->mNumFaces = (unsigned int)src.faces.size();

        // Generate sub nodes for named meshes
        if ( src.name[ 0 ] && NULL != ppcChildren  ) {
            aiNode* const node = *ppcChildren = new aiNode();
            node->mParent = root;
            node->mNumMeshes = 1;
            node->mMeshes = new unsigned int[1];
            node->mMeshes[0] = m;
            node->mName.Set(src.name);

            // setup the transformation matrix of the node
            aiMatrix4x4::FromToMatrix(aiVector3D(0.f,1.f,0.f),
                src.dir,node->mTransformation);

            aiMatrix4x4& mat = node->mTransformation;
            mat.a1 *= src.radius.x; mat.b1 *= src.radius.x; mat.c1 *= src.radius.x;
            mat.a2 *= src.radius.y; mat.b2 *= src.radius.y; mat.c2 *= src.radius.y;
            mat.a3 *= src.radius.z; mat.b3 *= src.radius.z; mat.c3 *= src.radius.z;
            mat.a4 = src.center.x;
            mat.b4 = src.center.y;
            mat.c4 = src.center.z;

            ++ppcChildren;
        } else {
            *pMeshes++ = m;
        }

        // copy vertex positions
        mesh->mVertices = new aiVector3D[mesh->mNumVertices];
        ::memcpy(mesh->mVertices,&src.vertices[0],
            sizeof(aiVector3D)*mesh->mNumVertices);

        // NFF2: there could be vertex colors
        if (!src.colors.empty())
        {
            ai_assert(src.colors.size() == src.vertices.size());

            // copy vertex colors
            mesh->mColors[0] = new aiColor4D[mesh->mNumVertices];
            ::memcpy(mesh->mColors[0],&src.colors[0],
                sizeof(aiColor4D)*mesh->mNumVertices);
        }

        if (!src.normals.empty())
        {
            ai_assert(src.normals.size() == src.vertices.size());

            // copy normal vectors
            mesh->mNormals = new aiVector3D[mesh->mNumVertices];
            ::memcpy(mesh->mNormals,&src.normals[0],
                sizeof(aiVector3D)*mesh->mNumVertices);
        }

        if (!src.uvs.empty())
        {
            ai_assert(src.uvs.size() == src.vertices.size());

            // copy texture coordinates
            mesh->mTextureCoords[0] = new aiVector3D[mesh->mNumVertices];
            ::memcpy(mesh->mTextureCoords[0],&src.uvs[0],
                sizeof(aiVector3D)*mesh->mNumVertices);
        }

        // generate faces
        unsigned int p = 0;
        aiFace* pFace = mesh->mFaces = new aiFace[mesh->mNumFaces];
        for (std::vector<unsigned int>::const_iterator it2 = src.faces.begin(),
            end2 = src.faces.end();
            it2 != end2;++it2,++pFace)
        {
            pFace->mIndices = new unsigned int [ pFace->mNumIndices = *it2 ];
            for (unsigned int o = 0; o < pFace->mNumIndices;++o)
                pFace->mIndices[o] = p++;
        }

        // generate a material for the mesh
        aiMaterial* pcMat = (aiMaterial*)(pScene->mMaterials[m] = new aiMaterial());

        mesh->mMaterialIndex = m++;

        aiString s;
        s.Set(AI_DEFAULT_MATERIAL_NAME);
        pcMat->AddProperty(&s, AI_MATKEY_NAME);

        // FIX: Ignore diffuse == 0
        aiColor3D c = src.shader.color * (src.shader.diffuse.r ?  src.shader.diffuse : aiColor3D(1.f,1.f,1.f));
        pcMat->AddProperty(&c,1,AI_MATKEY_COLOR_DIFFUSE);
        c = src.shader.color * src.shader.specular;
        pcMat->AddProperty(&c,1,AI_MATKEY_COLOR_SPECULAR);

        // NFF2 - default values for NFF
        pcMat->AddProperty(&src.shader.ambient, 1,AI_MATKEY_COLOR_AMBIENT);
        pcMat->AddProperty(&src.shader.emissive,1,AI_MATKEY_COLOR_EMISSIVE);
        pcMat->AddProperty(&src.shader.opacity, 1,AI_MATKEY_OPACITY);

        // setup the first texture layer, if existing
        if (src.shader.texFile.length())
        {
            s.Set(src.shader.texFile);
            pcMat->AddProperty(&s,AI_MATKEY_TEXTURE_DIFFUSE(0));

            if (aiTextureMapping_UV != src.shader.mapping) {

                aiVector3D v(0.f,-1.f,0.f);
                pcMat->AddProperty(&v, 1,AI_MATKEY_TEXMAP_AXIS_DIFFUSE(0));
                pcMat->AddProperty((int*)&src.shader.mapping, 1,AI_MATKEY_MAPPING_DIFFUSE(0));
            }
        }

        // setup the name of the material
        if (src.shader.name.length())
        {
            s.Set(src.shader.texFile);
            pcMat->AddProperty(&s,AI_MATKEY_NAME);
        }

        // setup some more material properties that are specific to NFF2
        int i;
        if (src.shader.twoSided)
        {
            i = 1;
            pcMat->AddProperty(&i,1,AI_MATKEY_TWOSIDED);
        }
        i = (src.shader.shaded ? aiShadingMode_Gouraud : aiShadingMode_NoShading);
        if (src.shader.shininess)
        {
            i = aiShadingMode_Phong;
            pcMat->AddProperty(&src.shader.shininess,1,AI_MATKEY_SHININESS);
        }
        pcMat->AddProperty(&i,1,AI_MATKEY_SHADING_MODEL);
    }
    pScene->mRootNode = root;
}

#endif // !! ASSIMP_BUILD_NO_NFF_IMPORTER
