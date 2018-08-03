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
#ifndef AI_OPENGEX_IMPORTER_H
#define AI_OPENGEX_IMPORTER_H

#ifndef ASSIMP_BUILD_NO_OPENGEX_IMPORTER

#include "BaseImporter.h"
#include <assimp/mesh.h>

#include <vector>
#include <list>
#include <map>
#include <memory>

namespace ODDLParser {
    class DDLNode;
    struct Context;
}

struct aiNode;
struct aiMaterial;
struct aiCamera;
struct aiLight;

namespace Assimp {
namespace OpenGEX {

struct MetricInfo {
    enum Type {
        Distance = 0,
        Angle,
        Time,
        Up,
        Max
    };

    std::string m_stringValue;
    float m_floatValue;
    int m_intValue;

    MetricInfo()
    : m_stringValue( "" )
    , m_floatValue( 0.0f )
    , m_intValue( -1 ) {
        // empty
    }
};

/** @brief  This class is used to implement the OpenGEX importer
 *
 *  See http://opengex.org/OpenGEX.pdf for spec.
 */
class OpenGEXImporter : public BaseImporter {
public:
    /// The class constructor.
    OpenGEXImporter();

    /// The class destructor.
    virtual ~OpenGEXImporter();

    /// BaseImporter override.
    virtual bool CanRead( const std::string &file, IOSystem *pIOHandler, bool checkSig ) const;

    /// BaseImporter override.
    virtual void InternReadFile( const std::string &file, aiScene *pScene, IOSystem *pIOHandler );

    /// BaseImporter override.
    virtual const aiImporterDesc *GetInfo() const;

    /// BaseImporter override.
    virtual void SetupProperties( const Importer *pImp );

protected:
    void handleNodes( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleMetricNode( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleNameNode( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleObjectRefNode( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleMaterialRefNode( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleGeometryNode( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleCameraNode( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleLightNode( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleGeometryObject( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleCameraObject( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleLightObject( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleTransformNode( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleMeshNode( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleVertexArrayNode( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleIndexArrayNode( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleMaterialNode( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleColorNode( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleTextureNode( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleParamNode( ODDLParser::DDLNode *node, aiScene *pScene );
    void handleAttenNode( ODDLParser::DDLNode *node, aiScene *pScene );
    void copyMeshes( aiScene *pScene );
    void copyCameras( aiScene *pScene );
    void copyLights( aiScene *pScene );
    void copyMaterials( aiScene *pScene );
    void resolveReferences();
    void pushNode( aiNode *node, aiScene *pScene );
    aiNode *popNode();
    aiNode *top() const;
    void clearNodeStack();
    void createNodeTree( aiScene *pScene );

private:
    struct VertexContainer {
        size_t m_numVerts;
        aiVector3D *m_vertices;
        size_t m_numColors;
        aiColor4D *m_colors;
        size_t m_numNormals;
        aiVector3D *m_normals;
        size_t m_numUVComps[ AI_MAX_NUMBER_OF_TEXTURECOORDS ];
        aiVector3D *m_textureCoords[ AI_MAX_NUMBER_OF_TEXTURECOORDS ];

        VertexContainer();
        ~VertexContainer();

        VertexContainer( const VertexContainer & ) = delete;
        VertexContainer &operator = ( const VertexContainer & ) = delete;
    };

    struct RefInfo {
        enum Type {
            MeshRef,
            MaterialRef
        };

        aiNode *m_node;
        Type m_type;
        std::vector<std::string> m_Names;

        RefInfo( aiNode *node, Type type, std::vector<std::string> &names );
        ~RefInfo();

        RefInfo( const RefInfo & ) = delete;
        RefInfo &operator = ( const RefInfo & ) = delete;
    };

    struct ChildInfo {
        typedef std::list<aiNode*> NodeList;
        std::list<aiNode*> m_children;
    };
    ChildInfo *m_root;
    typedef std::map<aiNode*, std::unique_ptr<ChildInfo> > NodeChildMap;
    NodeChildMap m_nodeChildMap;

    std::vector<aiMesh*> m_meshCache;
    typedef std::map<std::string, size_t> ReferenceMap;
    std::map<std::string, size_t> m_mesh2refMap;
    std::map<std::string, size_t> m_material2refMap;

    ODDLParser::Context *m_ctx;
    MetricInfo m_metrics[ MetricInfo::Max ];
    aiNode *m_currentNode;
    VertexContainer m_currentVertices;
    aiMesh *m_currentMesh;
    aiMaterial *m_currentMaterial;
    aiLight *m_currentLight;
    aiCamera *m_currentCamera;
    int m_tokenType;
    std::vector<aiMaterial*> m_materialCache;
    std::vector<aiCamera*> m_cameraCache;
    std::vector<aiLight*> m_lightCache;
    std::vector<aiNode*> m_nodeStack;
    std::vector<std::unique_ptr<RefInfo> > m_unresolvedRefStack;
};

} // Namespace OpenGEX
} // Namespace Assimp

#endif // ASSIMP_BUILD_NO_OPENGEX_IMPORTER

#endif // AI_OPENGEX_IMPORTER_H
