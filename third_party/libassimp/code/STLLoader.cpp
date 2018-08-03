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


#ifndef ASSIMP_BUILD_NO_STL_IMPORTER

// internal headers
#include "STLLoader.h"
#include "ParsingUtils.h"
#include "fast_atof.h"
#include <memory>
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/importerdesc.h>

using namespace Assimp;

namespace {
    
static const aiImporterDesc desc = {
    "Stereolithography (STL) Importer",
    "",
    "",
    "",
    aiImporterFlags_SupportTextFlavour | aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "stl"
};

// A valid binary STL buffer should consist of the following elements, in order:
// 1) 80 byte header
// 2) 4 byte face count
// 3) 50 bytes per face
static bool IsBinarySTL(const char* buffer, unsigned int fileSize) {
    if( fileSize < 84 ) {
        return false;
    }

    const char *facecount_pos = buffer + 80;
    uint32_t faceCount( 0 );
    ::memcpy( &faceCount, facecount_pos, sizeof( uint32_t ) );
    const uint32_t expectedBinaryFileSize = faceCount * 50 + 84;

    return expectedBinaryFileSize == fileSize;
}

// An ascii STL buffer will begin with "solid NAME", where NAME is optional.
// Note: The "solid NAME" check is necessary, but not sufficient, to determine
// if the buffer is ASCII; a binary header could also begin with "solid NAME".
static bool IsAsciiSTL(const char* buffer, unsigned int fileSize) {
    if (IsBinarySTL(buffer, fileSize))
        return false;

    const char* bufferEnd = buffer + fileSize;

    if (!SkipSpaces(&buffer))
        return false;

    if (buffer + 5 >= bufferEnd)
        return false;

    bool isASCII( strncmp( buffer, "solid", 5 ) == 0 );
    if( isASCII ) {
        // A lot of importers are write solid even if the file is binary. So we have to check for ASCII-characters.
        if( fileSize >= 500 ) {
            isASCII = true;
            for( unsigned int i = 0; i < 500; i++ ) {
                if( buffer[ i ] > 127 ) {
                    isASCII = false;
                    break;
                }
            }
        }
    }
    return isASCII;
}
} // namespace

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
STLImporter::STLImporter()
    : mBuffer(),
    fileSize(),
    pScene()
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
STLImporter::~STLImporter()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool STLImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    const std::string extension = GetExtension(pFile);

    if( extension == "stl" ) {
        return true;
    } else if (!extension.length() || checkSig)   {
        if( !pIOHandler ) {
            return true;
        }
        const char* tokens[] = {"STL","solid"};
        return SearchFileHeaderForToken(pIOHandler,pFile,tokens,2);
    }

    return false;
}

// ------------------------------------------------------------------------------------------------
const aiImporterDesc* STLImporter::GetInfo () const {
    return &desc;
}

void addFacesToMesh(aiMesh* pMesh)
{
    pMesh->mFaces = new aiFace[pMesh->mNumFaces];
    for (unsigned int i = 0, p = 0; i < pMesh->mNumFaces;++i)    {

        aiFace& face = pMesh->mFaces[i];
        face.mIndices = new unsigned int[face.mNumIndices = 3];
        for (unsigned int o = 0; o < 3;++o,++p) {
            face.mIndices[o] = p;
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void STLImporter::InternReadFile( const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler )
{
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile, "rb"));

    // Check whether we can read from the file
    if( file.get() == NULL) {
        throw DeadlyImportError( "Failed to open STL file " + pFile + ".");
    }

    fileSize = (unsigned int)file->FileSize();

    // allocate storage and copy the contents of the file to a memory buffer
    // (terminate it with zero)
    std::vector<char> mBuffer2;
    TextFileToBuffer(file.get(),mBuffer2);

    this->pScene = pScene;
    this->mBuffer = &mBuffer2[0];

    // the default vertex color is light gray.
    clrColorDefault.r = clrColorDefault.g = clrColorDefault.b = clrColorDefault.a = (ai_real) 0.6;

    // allocate a single node
    pScene->mRootNode = new aiNode();

    bool bMatClr = false;

    if (IsBinarySTL(mBuffer, fileSize)) {
        bMatClr = LoadBinaryFile();
    } else if (IsAsciiSTL(mBuffer, fileSize)) {
        LoadASCIIFile( pScene->mRootNode );
    } else {
        throw DeadlyImportError( "Failed to determine STL storage representation for " + pFile + ".");
    }

    // create a single default material, using a white diffuse color for consistency with
    // other geometric types (e.g., PLY).
    aiMaterial* pcMat = new aiMaterial();
    aiString s;
    s.Set(AI_DEFAULT_MATERIAL_NAME);
    pcMat->AddProperty(&s, AI_MATKEY_NAME);

    aiColor4D clrDiffuse(ai_real(1.0),ai_real(1.0),ai_real(1.0),ai_real(1.0));
    if (bMatClr) {
        clrDiffuse = clrColorDefault;
    }
    pcMat->AddProperty(&clrDiffuse,1,AI_MATKEY_COLOR_DIFFUSE);
    pcMat->AddProperty(&clrDiffuse,1,AI_MATKEY_COLOR_SPECULAR);
    clrDiffuse = aiColor4D( ai_real(1.0), ai_real(1.0), ai_real(1.0), ai_real(1.0));
    pcMat->AddProperty(&clrDiffuse,1,AI_MATKEY_COLOR_AMBIENT);

    pScene->mNumMaterials = 1;
    pScene->mMaterials = new aiMaterial*[1];
    pScene->mMaterials[0] = pcMat;
}

// ------------------------------------------------------------------------------------------------
// Read an ASCII STL file
void STLImporter::LoadASCIIFile( aiNode *root ) {
    std::vector<aiMesh*> meshes;
    std::vector<aiNode*> nodes;
    const char* sz = mBuffer;
    const char* bufferEnd = mBuffer + fileSize;
    std::vector<aiVector3D> positionBuffer;
    std::vector<aiVector3D> normalBuffer;

    // try to guess how many vertices we could have
    // assume we'll need 160 bytes for each face
    size_t sizeEstimate = std::max(1u, fileSize / 160u ) * 3;
    positionBuffer.reserve(sizeEstimate);
    normalBuffer.reserve(sizeEstimate);

    while (IsAsciiSTL(sz, static_cast<unsigned int>(bufferEnd - sz))) {
        std::vector<unsigned int> meshIndices;
        aiMesh* pMesh = new aiMesh();
        pMesh->mMaterialIndex = 0;
        meshIndices.push_back((unsigned int) meshes.size() );
        meshes.push_back(pMesh);
        aiNode *node = new aiNode;
        node->mParent = root;
        nodes.push_back( node );
        SkipSpaces(&sz);
        ai_assert(!IsLineEnd(sz));

        sz += 5; // skip the "solid"
        SkipSpaces(&sz);
        const char* szMe = sz;
        while (!::IsSpaceOrNewLine(*sz)) {
            sz++;
        }

        size_t temp;
        // setup the name of the node
        if ((temp = (size_t)(sz-szMe))) {
            if (temp >= MAXLEN) {
                throw DeadlyImportError( "STL: Node name too long" );
            }
            std::string name( szMe, temp );
            node->mName.Set( name.c_str() );
            //pScene->mRootNode->mName.length = temp;
            //memcpy(pScene->mRootNode->mName.data,szMe,temp);
            //pScene->mRootNode->mName.data[temp] = '\0';
        } else {
            pScene->mRootNode->mName.Set("<STL_ASCII>");
        }

        unsigned int faceVertexCounter = 3;
        for ( ;; ) {
            // go to the next token
            if(!SkipSpacesAndLineEnd(&sz))
            {
                // seems we're finished although there was no end marker
                DefaultLogger::get()->warn("STL: unexpected EOF. \'endsolid\' keyword was expected");
                break;
            }
            // facet normal -0.13 -0.13 -0.98
            if (!strncmp(sz,"facet",5) && IsSpaceOrNewLine(*(sz+5)) && *(sz + 5) != '\0')    {

                if (faceVertexCounter != 3) {
                    DefaultLogger::get()->warn("STL: A new facet begins but the old is not yet complete");
                }
                faceVertexCounter = 0;
                normalBuffer.push_back(aiVector3D());
                aiVector3D* vn = &normalBuffer.back();

                sz += 6;
                SkipSpaces(&sz);
                if (strncmp(sz,"normal",6))    {
                    DefaultLogger::get()->warn("STL: a facet normal vector was expected but not found");
                } else {
                    if (sz[6] == '\0') {
                        throw DeadlyImportError("STL: unexpected EOF while parsing facet");
                    }
                    sz += 7;
                    SkipSpaces(&sz);
                    sz = fast_atoreal_move<ai_real>(sz, (ai_real&)vn->x );
                    SkipSpaces(&sz);
                    sz = fast_atoreal_move<ai_real>(sz, (ai_real&)vn->y );
                    SkipSpaces(&sz);
                    sz = fast_atoreal_move<ai_real>(sz, (ai_real&)vn->z );
                    normalBuffer.push_back(*vn);
                    normalBuffer.push_back(*vn);
                }
            } else if (!strncmp(sz,"vertex",6) && ::IsSpaceOrNewLine(*(sz+6))) { // vertex 1.50000 1.50000 0.00000
                if (faceVertexCounter >= 3) {
                    DefaultLogger::get()->error("STL: a facet with more than 3 vertices has been found");
                    ++sz;
                } else {
                    if (sz[6] == '\0') {
                        throw DeadlyImportError("STL: unexpected EOF while parsing facet");
                    }
                    sz += 7;
                    SkipSpaces(&sz);
                    positionBuffer.push_back(aiVector3D());
                    aiVector3D* vn = &positionBuffer.back();
                    sz = fast_atoreal_move<ai_real>(sz, (ai_real&)vn->x );
                    SkipSpaces(&sz);
                    sz = fast_atoreal_move<ai_real>(sz, (ai_real&)vn->y );
                    SkipSpaces(&sz);
                    sz = fast_atoreal_move<ai_real>(sz, (ai_real&)vn->z );
                    faceVertexCounter++;
                }
            } else if (!::strncmp(sz,"endsolid",8))    {
                do {
                    ++sz;
                } while (!::IsLineEnd(*sz));
                SkipSpacesAndLineEnd(&sz);
                // finished!
                break;
            } else { // else skip the whole identifier
                do {
                    ++sz;
                } while (!::IsSpaceOrNewLine(*sz));
            }
        }

        if (positionBuffer.empty())    {
            pMesh->mNumFaces = 0;
            throw DeadlyImportError("STL: ASCII file is empty or invalid; no data loaded");
        }
        if (positionBuffer.size() % 3 != 0)    {
            pMesh->mNumFaces = 0;
            throw DeadlyImportError("STL: Invalid number of vertices");
        }
        if (normalBuffer.size() != positionBuffer.size())    {
            pMesh->mNumFaces = 0;
            throw DeadlyImportError("Normal buffer size does not match position buffer size");
        }
        pMesh->mNumFaces = static_cast<unsigned int>(positionBuffer.size() / 3);
        pMesh->mNumVertices = static_cast<unsigned int>(positionBuffer.size());
        pMesh->mVertices = new aiVector3D[pMesh->mNumVertices];
        memcpy(pMesh->mVertices, &positionBuffer[0].x, pMesh->mNumVertices * sizeof(aiVector3D));
        positionBuffer.clear();
        pMesh->mNormals = new aiVector3D[pMesh->mNumVertices];
        memcpy(pMesh->mNormals, &normalBuffer[0].x, pMesh->mNumVertices * sizeof(aiVector3D));
        normalBuffer.clear();

        // now copy faces
        addFacesToMesh(pMesh);

        // assign the meshes to the current node
        pushMeshesToNode( meshIndices, node );
    }

    // now add the loaded meshes
    pScene->mNumMeshes = (unsigned int)meshes.size();
    pScene->mMeshes = new aiMesh*[pScene->mNumMeshes];
    for (size_t i = 0; i < meshes.size(); i++) {
        pScene->mMeshes[ i ] = meshes[i];
    }

    root->mNumChildren = (unsigned int) nodes.size();
    root->mChildren = new aiNode*[ root->mNumChildren ];
    for ( size_t i=0; i<nodes.size(); ++i ) {
        root->mChildren[ i ] = nodes[ i ];
    }
}

// ------------------------------------------------------------------------------------------------
// Read a binary STL file
bool STLImporter::LoadBinaryFile()
{
    // allocate one mesh
    pScene->mNumMeshes = 1;
    pScene->mMeshes = new aiMesh*[1];
    aiMesh* pMesh = pScene->mMeshes[0] = new aiMesh();
    pMesh->mMaterialIndex = 0;

    // skip the first 80 bytes
    if (fileSize < 84) {
        throw DeadlyImportError("STL: file is too small for the header");
    }
    bool bIsMaterialise = false;

    // search for an occurrence of "COLOR=" in the header
    const unsigned char* sz2 = (const unsigned char*)mBuffer;
    const unsigned char* const szEnd = sz2+80;
    while (sz2 < szEnd) {

        if ('C' == *sz2++ && 'O' == *sz2++ && 'L' == *sz2++ &&
            'O' == *sz2++ && 'R' == *sz2++ && '=' == *sz2++)    {

            // read the default vertex color for facets
            bIsMaterialise = true;
            DefaultLogger::get()->info("STL: Taking code path for Materialise files");
            const ai_real invByte = (ai_real)1.0 / ( ai_real )255.0;
            clrColorDefault.r = (*sz2++) * invByte;
            clrColorDefault.g = (*sz2++) * invByte;
            clrColorDefault.b = (*sz2++) * invByte;
            clrColorDefault.a = (*sz2++) * invByte;
            break;
        }
    }
    const unsigned char* sz = (const unsigned char*)mBuffer + 80;

    // now read the number of facets
    pScene->mRootNode->mName.Set("<STL_BINARY>");

    pMesh->mNumFaces = *((uint32_t*)sz);
    sz += 4;

    if (fileSize < 84 + pMesh->mNumFaces*50) {
        throw DeadlyImportError("STL: file is too small to hold all facets");
    }

    if (!pMesh->mNumFaces) {
        throw DeadlyImportError("STL: file is empty. There are no facets defined");
    }

    pMesh->mNumVertices = pMesh->mNumFaces*3;

    aiVector3D* vp,*vn;
    vp = pMesh->mVertices = new aiVector3D[pMesh->mNumVertices];
    vn = pMesh->mNormals = new aiVector3D[pMesh->mNumVertices];

    for (unsigned int i = 0; i < pMesh->mNumFaces;++i) {

        // NOTE: Blender sometimes writes empty normals ... this is not
        // our fault ... the RemoveInvalidData helper step should fix that
        *vn = *((aiVector3D*)sz);
        sz += sizeof(aiVector3D);
        *(vn+1) = *vn;
        *(vn+2) = *vn;
        vn += 3;

        *vp++ = *((aiVector3D*)sz);
        sz += sizeof(aiVector3D);

        *vp++ = *((aiVector3D*)sz);
        sz += sizeof(aiVector3D);

        *vp++ = *((aiVector3D*)sz);
        sz += sizeof(aiVector3D);

        uint16_t color = *((uint16_t*)sz);
        sz += 2;

        if (color & (1 << 15))
        {
            // seems we need to take the color
            if (!pMesh->mColors[0])
            {
                pMesh->mColors[0] = new aiColor4D[pMesh->mNumVertices];
                for (unsigned int i = 0; i <pMesh->mNumVertices;++i)
                    *pMesh->mColors[0]++ = this->clrColorDefault;
                pMesh->mColors[0] -= pMesh->mNumVertices;

                DefaultLogger::get()->info("STL: Mesh has vertex colors");
            }
            aiColor4D* clr = &pMesh->mColors[0][i*3];
            clr->a = 1.0;
            const ai_real invVal( (ai_real)1.0 / ( ai_real )31.0 );
            if (bIsMaterialise) // this is reversed
            {
                clr->r = (color & 0x31u) *invVal;
                clr->g = ((color & (0x31u<<5))>>5u) *invVal;
                clr->b = ((color & (0x31u<<10))>>10u) *invVal;
            }
            else
            {
                clr->b = (color & 0x31u) *invVal;
                clr->g = ((color & (0x31u<<5))>>5u) *invVal;
                clr->r = ((color & (0x31u<<10))>>10u) *invVal;
            }
            // assign the color to all vertices of the face
            *(clr+1) = *clr;
            *(clr+2) = *clr;
        }
    }

    // now copy faces
    addFacesToMesh(pMesh);

    // add all created meshes to the single node
    pScene->mRootNode->mNumMeshes = pScene->mNumMeshes;
    pScene->mRootNode->mMeshes = new unsigned int[pScene->mNumMeshes];
    for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
        pScene->mRootNode->mMeshes[i] = i;

    if (bIsMaterialise && !pMesh->mColors[0])
    {
        // use the color as diffuse material color
        return true;
    }
    return false;
}

void STLImporter::pushMeshesToNode( std::vector<unsigned int> &meshIndices, aiNode *node ) {
    ai_assert( nullptr != node );
    if ( meshIndices.empty() ) {
        return;
    }

    node->mNumMeshes = static_cast<unsigned int>( meshIndices.size() );
    node->mMeshes = new unsigned int[ meshIndices.size() ];
    for ( size_t i=0; i<meshIndices.size(); ++i ) {
        node->mMeshes[ i ] = meshIndices[ i ];
    }
    meshIndices.clear();
}

#endif // !! ASSIMP_BUILD_NO_STL_IMPORTER
