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

/** @file ObjExporter.h
 * Declares the exporter class to write a scene to a Collada file
 */
#ifndef AI_OBJEXPORTER_H_INC
#define AI_OBJEXPORTER_H_INC

#include <assimp/types.h>
#include <sstream>
#include <vector>
#include <map>

struct aiScene;
struct aiNode;
struct aiMesh;

namespace Assimp {

// ------------------------------------------------------------------------------------------------
/** Helper class to export a given scene to an OBJ file. */
// ------------------------------------------------------------------------------------------------
class ObjExporter {
public:
    /// Constructor for a specific scene to export
    ObjExporter(const char* filename, const aiScene* pScene, bool noMtl=false);
    ~ObjExporter();
    std::string GetMaterialLibName();
    std::string GetMaterialLibFileName();
    
    /// public string-streams to write all output into
    std::ostringstream mOutput, mOutputMat;

private:
    // intermediate data structures
    struct FaceVertex {
        FaceVertex()
        : vp()
        , vn()
        , vt() {
            // empty
        }

        // one-based, 0 means: 'does not exist'
        unsigned int vp, vn, vt;
    };

    struct Face {
        char kind;
        std::vector<FaceVertex> indices;
    };

    struct MeshInstance {
        std::string name, matname;
        std::vector<Face> faces;
    };

    void WriteHeader(std::ostringstream& out);
    void WriteMaterialFile();
    void WriteGeometryFile(bool noMtl=false);
    std::string GetMaterialName(unsigned int index);
    void AddMesh(const aiString& name, const aiMesh* m, const aiMatrix4x4& mat);
    void AddNode(const aiNode* nd, const aiMatrix4x4& mParent);

private:
    std::string filename;
    const aiScene* const pScene;

    struct vertexData {
        aiVector3D vp;
        aiColor3D vc; // OBJ does not support 4D color
    };

    std::vector<aiVector3D> vn, vt;
    std::vector<aiColor4D> vc;
    std::vector<vertexData> vp;
    bool useVc;

    struct vertexDataCompare {
        bool operator() ( const vertexData& a, const vertexData& b ) const {
            // position
            if (a.vp.x < b.vp.x) return true;
            if (a.vp.x > b.vp.x) return false;
            if (a.vp.y < b.vp.y) return true;
            if (a.vp.y > b.vp.y) return false;
            if (a.vp.z < b.vp.z) return true;
            if (a.vp.z > b.vp.z) return false;

            // color
            if (a.vc.r < b.vc.r) return true;
            if (a.vc.r > b.vc.r) return false;
            if (a.vc.g < b.vc.g) return true;
            if (a.vc.g > b.vc.g) return false;
            if (a.vc.b < b.vc.b) return true;
            if (a.vc.b > b.vc.b) return false;
            return false;
        }
    };

    struct aiVectorCompare { 
        bool operator() (const aiVector3D& a, const aiVector3D& b) const { 
            if(a.x < b.x) return true; 
            if(a.x > b.x) return false; 
            if(a.y < b.y) return true; 
            if(a.y > b.y) return false; 
            if(a.z < b.z) return true; 
            return false;
        }
    };

    template <class T, class Compare = std::less<T>>
    class indexMap {
        int mNextIndex;
        typedef std::map<T, int, Compare> dataType;
        dataType vecMap;
    
    public:
        indexMap()
        : mNextIndex(1) {
            // empty
        }

        int getIndex(const T& key) {
            typename dataType::iterator vertIt = vecMap.find(key);
            // vertex already exists, so reference it
            if(vertIt != vecMap.end()){
                return vertIt->second;
            }
            return vecMap[key] = mNextIndex++;
        };

        void getKeys( std::vector<T>& keys ) {
            keys.resize(vecMap.size());
            for(typename dataType::iterator it = vecMap.begin(); it != vecMap.end(); ++it){
                keys[it->second-1] = it->first;
            }
        };
    };

    indexMap<aiVector3D, aiVectorCompare> mVnMap, mVtMap;
    indexMap<vertexData, vertexDataCompare> mVpMap;
    std::vector<MeshInstance> mMeshes;

    // this endl() doesn't flush() the stream
    const std::string endl;
};

}

#endif
