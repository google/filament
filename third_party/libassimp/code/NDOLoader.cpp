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

/** @file  NDOLoader.cpp
 *  Implementation of the NDO importer class.
 */


#ifndef ASSIMP_BUILD_NO_NDO_IMPORTER
#include "NDOLoader.h"
#include <assimp/DefaultLogger.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>
#include <assimp/importerdesc.h>
#include "StreamReader.h"
#include <map>

using namespace Assimp;

static const aiImporterDesc desc = {
    "Nendo Mesh Importer",
    "",
    "",
    "http://www.izware.com/nendo/index.htm",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "ndo"
};

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
NDOImporter::NDOImporter()
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
NDOImporter::~NDOImporter()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool NDOImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    // check file extension
    const std::string extension = GetExtension(pFile);

    if( extension == "ndo")
        return true;

    if ((checkSig || !extension.length()) && pIOHandler) {
        const char* tokens[] = {"nendo"};
        return SearchFileHeaderForToken(pIOHandler,pFile,tokens,1,5);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Build a string of all file extensions supported
const aiImporterDesc* NDOImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Setup configuration properties for the loader
void NDOImporter::SetupProperties(const Importer* /*pImp*/)
{
    // nothing to be done for the moment
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void NDOImporter::InternReadFile( const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
    StreamReaderBE reader(pIOHandler->Open( pFile, "rb"));

    // first 9 bytes are nendo file format ("nendo 1.n")
    const char* head = (const char*)reader.GetPtr();
    reader.IncPtr(9);

    if (strncmp("nendo ",head,6)) {
        throw DeadlyImportError("Not a Nendo file; magic signature missing");
    }
    // check if this is a supported version. if not, continue, too -- users,
    // please don't complain if it doesn't work then ...
    unsigned int file_format = 12;
    if (!strncmp("1.0",head+6,3)) {
        file_format = 10;
        DefaultLogger::get()->info("NDO file format is 1.0");
    }
    else if (!strncmp("1.1",head+6,3)) {
        file_format = 11;
        DefaultLogger::get()->info("NDO file format is 1.1");
    }
    else if (!strncmp("1.2",head+6,3)) {
        file_format = 12;
        DefaultLogger::get()->info("NDO file format is 1.2");
    }
    else {
        DefaultLogger::get()->warn(std::string("Unrecognized nendo file format version, continuing happily ... :") + (head+6));
    }

    reader.IncPtr(2); /* skip flags */
    if (file_format >= 12) {
        reader.IncPtr(2);
    }
    unsigned int temp = reader.GetU1();

    std::vector<Object> objects(temp); /* buffer to store all the loaded objects in */

    // read all objects
    for (unsigned int o = 0; o < objects.size(); ++o) {

//      if (file_format < 12) {
            if (!reader.GetI1()) {
                continue; /* skip over empty object */
            }
        //  reader.GetI2();
//      }
        Object& obj = objects[o];

        temp = file_format >= 12 ? reader.GetU4() : reader.GetU2();
        head = (const char*)reader.GetPtr();
        reader.IncPtr(temp + 76); /* skip unknown stuff */

        obj.name = std::string(head, temp);

        // read edge table
        temp = file_format >= 12 ? reader.GetU4() : reader.GetU2();
        obj.edges.reserve(temp);
        for (unsigned int e = 0; e < temp; ++e) {

            obj.edges.push_back(Edge());
            Edge& edge = obj.edges.back();

            for (unsigned int i = 0; i< 8; ++i) {
                edge.edge[i] = file_format >= 12 ? reader.GetU4() : reader.GetU2();
            }
            edge.hard =  file_format >= 11 ? reader.GetU1() : 0;
            for (unsigned int i = 0; i< 8; ++i) {
                edge.color[i] = reader.GetU1();
            }
        }

        // read face table
        temp = file_format >= 12 ? reader.GetU4() : reader.GetU2();
        obj.faces.reserve(temp);
        for (unsigned int e = 0; e < temp; ++e) {

            obj.faces.push_back(Face());
            Face& face = obj.faces.back();

            face.elem = file_format >= 12 ? reader.GetU4() : reader.GetU2();
        }

        // read vertex table
        temp = file_format >= 12 ? reader.GetU4() : reader.GetU2();
        obj.vertices.reserve(temp);
        for (unsigned int e = 0; e < temp; ++e) {

            obj.vertices.push_back(Vertex());
            Vertex& v = obj.vertices.back();

            v.num = file_format >= 12 ? reader.GetU4() : reader.GetU2();
            v.val.x = reader.GetF4();
            v.val.y = reader.GetF4();
            v.val.z = reader.GetF4();
        }

        // read UVs
        temp = file_format >= 12 ? reader.GetU4() : reader.GetU2();
        for (unsigned int e = 0; e < temp; ++e) {
             file_format >= 12 ? reader.GetU4() : reader.GetU2();
        }

        temp = file_format >= 12 ? reader.GetU4() : reader.GetU2();
        for (unsigned int e = 0; e < temp; ++e) {
             file_format >= 12 ? reader.GetU4() : reader.GetU2();
        }

        if (reader.GetU1()) {
            const unsigned int x = reader.GetU2(), y = reader.GetU2();
            temp = 0;
            while (temp < x*y)  {
                unsigned int repeat = reader.GetU1();
                reader.GetU1();
                reader.GetU1();
                reader.GetU1();
                temp += repeat;
            }
        }
    }

    // construct a dummy node graph and add all named objects as child nodes
    aiNode* root = pScene->mRootNode = new aiNode("$NDODummyRoot");
    aiNode** cc = root->mChildren = new aiNode* [ root->mNumChildren = static_cast<unsigned int>( objects.size()) ] ();
    pScene->mMeshes = new aiMesh* [ root->mNumChildren] ();

    std::vector<aiVector3D> vertices;
    std::vector<unsigned int> indices;

    for(const Object& obj : objects) {
        aiNode* nd = *cc++ = new aiNode(obj.name);
        nd->mParent = root;

        // translated from a python dict() - a vector might be sufficient as well
        typedef std::map<unsigned int, unsigned int>  FaceTable;
        FaceTable face_table;

        unsigned int n = 0;
        for(const Edge& edge : obj.edges) {

            face_table[edge.edge[2]] = n;
            face_table[edge.edge[3]] = n;

            ++n;
        }

        aiMesh* mesh = new aiMesh();
        mesh->mNumFaces=static_cast<unsigned int>(face_table.size());
        aiFace* faces = mesh->mFaces = new aiFace[mesh->mNumFaces];

        vertices.clear();
        vertices.reserve(4 * face_table.size()); // arbitrarily chosen
        for(FaceTable::value_type& v : face_table) {
            indices.clear();

            aiFace& f = *faces++;

            const unsigned int key = v.first;
            unsigned int cur_edge = v.second;
            while (1) {
                unsigned int next_edge, next_vert;
                if (key == obj.edges[cur_edge].edge[3]) {
                    next_edge = obj.edges[cur_edge].edge[5];
                    next_vert = obj.edges[cur_edge].edge[1];
                }
                else {
                    next_edge = obj.edges[cur_edge].edge[4];
                    next_vert = obj.edges[cur_edge].edge[0];
                }
                indices.push_back( static_cast<unsigned int>(vertices.size()) );
                vertices.push_back(obj.vertices[ next_vert ].val);

                cur_edge = next_edge;
                if (cur_edge == v.second) {
                    break;
                }
            }

            f.mIndices = new unsigned int[f.mNumIndices = static_cast<unsigned int>(indices.size())];
            std::copy(indices.begin(),indices.end(),f.mIndices);
        }

        mesh->mVertices = new aiVector3D[mesh->mNumVertices = static_cast<unsigned int>(vertices.size())];
        std::copy(vertices.begin(),vertices.end(),mesh->mVertices);

        if (mesh->mNumVertices) {
            pScene->mMeshes[pScene->mNumMeshes] = mesh;

            (nd->mMeshes = new unsigned int[nd->mNumMeshes=1])[0]=pScene->mNumMeshes++;
        }else
            delete mesh;
    }
}

#endif
