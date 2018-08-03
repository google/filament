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
materials provided with the distribution

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
#ifndef ASSIMP_BUILD_NO_OPENGEX_IMPORTER

#include "OpenGEXImporter.h"
#include <assimp/DefaultIOSystem.h>
#include <assimp/DefaultLogger.hpp>
#include "MakeVerboseFormat.h"
#include "StringComparison.h"

#include <openddlparser/OpenDDLParser.h>
#include <assimp/scene.h>
#include <assimp/ai_assert.h>
#include <assimp/importerdesc.h>

#include <vector>

static const aiImporterDesc desc = {
    "Open Game Engine Exchange",
    "",
    "",
    "",
    aiImporterFlags_SupportTextFlavour,
    0,
    0,
    0,
    0,
    "ogex"
};

namespace Grammar {
    static const std::string MetricType          = "Metric";
    static const std::string Metric_DistanceType = "distance";
    static const std::string Metric_AngleType    = "angle";
    static const std::string Metric_TimeType     = "time";
    static const std::string Metric_UpType       = "up";
    static const std::string NameType            = "Name";
    static const std::string ObjectRefType       = "ObjectRef";
    static const std::string MaterialRefType     = "MaterialRef";
    static const std::string MetricKeyType       = "key";
    static const std::string GeometryNodeType    = "GeometryNode";
    static const std::string CameraNodeType      = "CameraNode";
    static const std::string LightNodeType       = "LightNode";
    static const std::string GeometryObjectType  = "GeometryObject";
    static const std::string CameraObjectType    = "CameraObject";
    static const std::string LightObjectType     = "LightObject";
    static const std::string TransformType       = "Transform";
    static const std::string MeshType            = "Mesh";
    static const std::string VertexArrayType     = "VertexArray";
    static const std::string IndexArrayType      = "IndexArray";
    static const std::string MaterialType        = "Material";
    static const std::string ColorType           = "Color";
    static const std::string ParamType           = "Param";
    static const std::string TextureType         = "Texture";
    static const std::string AttenType           = "Atten";

    static const std::string DiffuseColorToken  = "diffuse";
    static const std::string SpecularColorToken = "specular";
    static const std::string EmissionColorToken = "emission";

    static const std::string DiffuseTextureToken         = "diffuse";
    static const std::string DiffuseSpecularTextureToken = "specular";
    static const std::string SpecularPowerTextureToken   = "specular_power";
    static const std::string EmissionTextureToken        = "emission";
    static const std::string OpacyTextureToken           = "opacity";
    static const std::string TransparencyTextureToken    = "transparency";
    static const std::string NormalTextureToken          = "normal";

    enum TokenType {
        NoneType = -1,
        MetricToken,
        NameToken,
        ObjectRefToken,
        MaterialRefToken,
        MetricKeyToken,
        GeometryNodeToken,
        CameraNodeToken,
        LightNodeToken,
        GeometryObjectToken,
        CameraObjectToken,
        LightObjectToken,
        TransformToken,
        MeshToken,
        VertexArrayToken,
        IndexArrayToken,
        MaterialToken,
        ColorToken,
        ParamToken,
        TextureToken,
        AttenToken
    };

    static const std::string ValidMetricToken[ 4 ] = {
        Metric_DistanceType,
        Metric_AngleType,
        Metric_TimeType,
        Metric_UpType
    };

    static int isValidMetricType( const char *token ) {
        if( nullptr == token ) {
            return false;
        }

        int idx( -1 );
        for( size_t i = 0; i < 4; i++ ) {
            if( ValidMetricToken[ i ] == token ) {
                idx = (int) i;
                break;
            }
        }

        return idx;
    }

    static TokenType matchTokenType( const char *tokenType ) {
        if( MetricType == tokenType ) {
            return MetricToken;
        } else if(  NameType == tokenType ) {
            return NameToken;
        } else if( ObjectRefType == tokenType ) {
            return ObjectRefToken;
        } else if( MaterialRefType == tokenType ) {
            return MaterialRefToken;
        } else if( MetricKeyType == tokenType ) {
            return MetricKeyToken;
        } else if ( GeometryNodeType == tokenType ) {
            return GeometryNodeToken;
        } else if ( CameraNodeType == tokenType ) {
            return CameraNodeToken;
        } else if ( LightNodeType == tokenType ) {
            return LightNodeToken;
        } else if ( GeometryObjectType == tokenType ) {
            return GeometryObjectToken;
        } else if ( CameraObjectType == tokenType ) {
            return CameraObjectToken;
        } else if ( LightObjectType == tokenType ) {
            return LightObjectToken;
        } else if( TransformType == tokenType ) {
            return TransformToken;
        } else if( MeshType == tokenType ) {
            return MeshToken;
        } else if( VertexArrayType == tokenType ) {
            return VertexArrayToken;
        } else if( IndexArrayType == tokenType ) {
            return IndexArrayToken;
        } else if(  MaterialType == tokenType ) {
            return MaterialToken;
        } else if ( ColorType == tokenType ) {
            return ColorToken;
        } else if ( ParamType == tokenType ) {
            return ParamToken;
        } else if(  TextureType == tokenType ) {
            return TextureToken;
        } else if ( AttenType == tokenType ) {
            return AttenToken;
        }

        return NoneType;
    }
} // Namespace Grammar

namespace Assimp {
namespace OpenGEX {

USE_ODDLPARSER_NS

//------------------------------------------------------------------------------------------------
static void propId2StdString( Property *prop, std::string &name, std::string &key ) {
    name = key = "";
    if ( nullptr == prop ) {
        return;
    }

    if ( nullptr != prop->m_key ) {
        name = prop->m_key->m_buffer;
        if ( Value::ddl_string == prop->m_value->m_type ) {
            key = prop->m_value->getString();
        }
    }
}

//------------------------------------------------------------------------------------------------
OpenGEXImporter::VertexContainer::VertexContainer()
: m_numVerts( 0 )
, m_vertices( nullptr )
, m_numColors( 0 )
, m_colors( nullptr )
, m_numNormals( 0 )
, m_normals( nullptr )
, m_numUVComps()
, m_textureCoords() {
    // empty
}

//------------------------------------------------------------------------------------------------
OpenGEXImporter::VertexContainer::~VertexContainer() {
    delete[] m_vertices;
    delete[] m_colors;
    delete[] m_normals;

    for(auto &texcoords : m_textureCoords) {
        delete [] texcoords;
    }
}

//------------------------------------------------------------------------------------------------
OpenGEXImporter::RefInfo::RefInfo( aiNode *node, Type type, std::vector<std::string> &names )
: m_node( node )
, m_type( type )
, m_Names( names ) {
    // empty
}

//------------------------------------------------------------------------------------------------
OpenGEXImporter::RefInfo::~RefInfo() {
    // empty
}

//------------------------------------------------------------------------------------------------
OpenGEXImporter::OpenGEXImporter()
: m_root( nullptr )
, m_nodeChildMap()
, m_meshCache()
, m_mesh2refMap()
, m_material2refMap()
, m_ctx( nullptr )
, m_metrics()
, m_currentNode( nullptr )
, m_currentVertices()
, m_currentMesh( nullptr )
, m_currentMaterial( nullptr )
, m_currentLight( nullptr )
, m_currentCamera( nullptr )
, m_tokenType( Grammar::NoneType )
, m_materialCache()
, m_cameraCache()
, m_lightCache()
, m_nodeStack()
, m_unresolvedRefStack() {
    // empty
}

//------------------------------------------------------------------------------------------------
OpenGEXImporter::~OpenGEXImporter() {
    m_ctx = nullptr;
}

//------------------------------------------------------------------------------------------------
bool OpenGEXImporter::CanRead( const std::string &file, IOSystem *pIOHandler, bool checkSig ) const {
    bool canRead( false );
    if( !checkSig ) {
        canRead = SimpleExtensionCheck( file, "ogex" );
    } else {
        static const char *token[] = { "Metric", "GeometryNode", "VertexArray (attrib", "IndexArray" };
        canRead = BaseImporter::SearchFileHeaderForToken( pIOHandler, file, token, 4 );
    }

    return canRead;
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::InternReadFile( const std::string &filename, aiScene *pScene, IOSystem *pIOHandler ) {
    // open source file
    IOStream *file = pIOHandler->Open( filename, "rb" );
    if( !file ) {
        throw DeadlyImportError( "Failed to open file " + filename );
    }

    std::vector<char> buffer;
    TextFileToBuffer( file, buffer );
    pIOHandler->Close( file );

    OpenDDLParser myParser;
    myParser.setBuffer( &buffer[ 0 ], buffer.size() );
    bool success( myParser.parse() );
    if( success ) {
        m_ctx = myParser.getContext();
        pScene->mRootNode = new aiNode;
        pScene->mRootNode->mName.Set( filename );
        handleNodes( m_ctx->m_root, pScene );
    }

    copyMeshes( pScene );
    copyCameras( pScene );
    copyLights( pScene );
    copyMaterials( pScene );
    resolveReferences();
    createNodeTree( pScene );
}

//------------------------------------------------------------------------------------------------
const aiImporterDesc *OpenGEXImporter::GetInfo() const {
    return &desc;
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::SetupProperties( const Importer *pImp ) {
    if( nullptr == pImp ) {
        return;
    }
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleNodes( DDLNode *node, aiScene *pScene ) {
    if( nullptr == node ) {
        return;
    }

    DDLNode::DllNodeList childs = node->getChildNodeList();
    for( DDLNode::DllNodeList::iterator it = childs.begin(); it != childs.end(); ++it ) {
        Grammar::TokenType tokenType( Grammar::matchTokenType( ( *it )->getType().c_str() ) );
        switch( tokenType ) {
            case Grammar::MetricToken:
                handleMetricNode( *it, pScene );
                break;

            case Grammar::NameToken:
                handleNameNode( *it, pScene );
                break;

            case Grammar::ObjectRefToken:
                handleObjectRefNode( *it, pScene );
                break;

            case Grammar::MaterialRefToken:
                handleMaterialRefNode( *it, pScene );
                break;

            case Grammar::MetricKeyToken:
                break;

            case Grammar::GeometryNodeToken:
                handleGeometryNode( *it, pScene );
                break;

            case Grammar::CameraNodeToken:
                handleCameraNode( *it, pScene );
                break;

            case Grammar::LightNodeToken:
                handleLightNode( *it, pScene );
                break;

            case Grammar::GeometryObjectToken:
                handleGeometryObject( *it, pScene );
                break;

            case Grammar::CameraObjectToken:
                handleCameraObject( *it, pScene );
                break;

            case Grammar::LightObjectToken:
                handleLightObject( *it, pScene );
                break;

            case Grammar::TransformToken:
                handleTransformNode( *it, pScene );
                break;

            case Grammar::MeshToken:
                handleMeshNode( *it, pScene );
                break;

            case Grammar::VertexArrayToken:
                handleVertexArrayNode( *it, pScene );
                break;

            case Grammar::IndexArrayToken:
                handleIndexArrayNode( *it, pScene );
                break;

            case Grammar::MaterialToken:
                handleMaterialNode( *it, pScene );
                break;

            case Grammar::ColorToken:
                handleColorNode( *it, pScene );
                break;

            case Grammar::ParamToken:
                handleParamNode( *it, pScene );
                break;

            case Grammar::TextureToken:
                handleTextureNode( *it, pScene );
                break;

            default:
                break;
        }
    }
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleMetricNode( DDLNode *node, aiScene * /*pScene*/ ) {
    if( nullptr == node || nullptr == m_ctx ) {
        return;
    }

    if( m_ctx->m_root != node->getParent() ) {
        return;
    }

    Property *prop( node->getProperties() );
    while( nullptr != prop ) {
        if( nullptr != prop->m_key ) {
            if( Value::ddl_string == prop->m_value->m_type ) {
                std::string valName( ( char* ) prop->m_value->m_data );
                int type( Grammar::isValidMetricType( valName.c_str() ) );
                if( Grammar::NoneType != type ) {
                    Value *val( node->getValue() );
                    if( nullptr != val ) {
                        if( Value::ddl_float == val->m_type ) {
                            m_metrics[ type ].m_floatValue = val->getFloat();
                        } else if( Value::ddl_int32 == val->m_type ) {
                            m_metrics[ type ].m_intValue = val->getInt32();
                        } else if( Value::ddl_string == val->m_type ) {
                            m_metrics[type].m_stringValue = std::string( val->getString() );
                        } else {
                            throw DeadlyImportError( "OpenGEX: invalid data type for Metric node." );
                        }
                    }
                }
            }
        }
        prop = prop->m_next;
    }
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleNameNode( DDLNode *node, aiScene * /*pScene*/ ) {
    if( nullptr == m_currentNode ) {
        throw DeadlyImportError( "No current node for name." );
        return;
    }

    Value *val( node->getValue() );
    if( nullptr != val ) {
        if( Value::ddl_string != val->m_type ) {
            throw DeadlyImportError( "OpenGEX: invalid data type for value in node name." );
            return;
        }

        const std::string name( val->getString() );
        if( m_tokenType == Grammar::GeometryNodeToken || m_tokenType == Grammar::LightNodeToken
                || m_tokenType == Grammar::CameraNodeToken ) {
            m_currentNode->mName.Set( name.c_str() );
        } else if( m_tokenType == Grammar::MaterialToken ) {
            aiString aiName;
            aiName.Set( name );
            m_currentMaterial->AddProperty( &aiName, AI_MATKEY_NAME );
            m_material2refMap[ name ] = m_materialCache.size() - 1;
        }
    }
}

//------------------------------------------------------------------------------------------------
static void getRefNames( DDLNode *node, std::vector<std::string> &names ) {
    ai_assert( nullptr != node );

    Reference *ref = node->getReferences();
    if( nullptr != ref ) {
        for( size_t i = 0; i < ref->m_numRefs; i++ )  {
            Name *currentName( ref->m_referencedName[ i ] );
            if( nullptr != currentName && nullptr != currentName->m_id ) {
                const std::string name( currentName->m_id->m_buffer );
                if( !name.empty() ) {
                    names.push_back( name );
                }
            }
        }
    }
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleObjectRefNode( DDLNode *node, aiScene * /*pScene*/ ) {
    if( nullptr == m_currentNode ) {
        throw DeadlyImportError( "No parent node for name." );
        return;
    }

    std::vector<std::string> objRefNames;
    getRefNames( node, objRefNames );

    // when we are dealing with a geometry node prepare the mesh cache
    if ( m_tokenType == Grammar::GeometryNodeToken ) {
        m_currentNode->mNumMeshes = static_cast<unsigned int>(objRefNames.size());
        m_currentNode->mMeshes = new unsigned int[ objRefNames.size() ];
        if ( !objRefNames.empty() ) {
            m_unresolvedRefStack.push_back( std::unique_ptr<RefInfo>( new RefInfo( m_currentNode, RefInfo::MeshRef, objRefNames ) ) );
        }
    } else if ( m_tokenType == Grammar::LightNodeToken ) {
        // TODO!
    } else if ( m_tokenType == Grammar::CameraNodeToken ) {
        // TODO!
    }
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleMaterialRefNode( ODDLParser::DDLNode *node, aiScene * /*pScene*/ ) {
    if( nullptr == m_currentNode ) {
        throw DeadlyImportError( "No parent node for name." );
        return;
    }

    std::vector<std::string> matRefNames;
    getRefNames( node, matRefNames );
    if( !matRefNames.empty() ) {
        m_unresolvedRefStack.push_back( std::unique_ptr<RefInfo>( new RefInfo( m_currentNode, RefInfo::MaterialRef, matRefNames ) ) );
    }
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleGeometryNode( DDLNode *node, aiScene *pScene ) {
    aiNode *newNode = new aiNode;
    pushNode( newNode, pScene );
    m_tokenType = Grammar::GeometryNodeToken;
    m_currentNode = newNode;
    handleNodes( node, pScene );

    popNode();
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleCameraNode( DDLNode *node, aiScene *pScene ) {
    aiCamera *camera( new aiCamera );
    m_cameraCache.push_back( camera );
    m_currentCamera = camera;

    aiNode *newNode = new aiNode;
    pushNode( newNode, pScene );
    m_tokenType = Grammar::CameraNodeToken;
    m_currentNode = newNode;

    handleNodes( node, pScene );

    popNode();

    m_currentCamera->mName.Set( newNode->mName.C_Str() );
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleLightNode( ODDLParser::DDLNode *node, aiScene *pScene ) {
    aiLight *light( new aiLight );
    m_lightCache.push_back( light );
    m_currentLight = light;

    aiNode *newNode = new aiNode;
    m_tokenType = Grammar::LightNodeToken;
    m_currentNode = newNode;
    pushNode( newNode, pScene );

    handleNodes( node, pScene );

    popNode();

    m_currentLight->mName.Set( newNode->mName.C_Str() );
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleGeometryObject( DDLNode *node, aiScene *pScene ) {
    // parameters will be parsed normally in the tree, so just go for it
    handleNodes( node, pScene );
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleCameraObject( ODDLParser::DDLNode *node, aiScene *pScene ) {
    // parameters will be parsed normally in the tree, so just go for it

    handleNodes( node, pScene );
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleLightObject( ODDLParser::DDLNode *node, aiScene *pScene ) {
    aiLight *light( new aiLight );
    m_lightCache.push_back( light );
    std::string objName = node->getName();
    if ( !objName.empty() ) {
        light->mName.Set( objName );
    }
    m_currentLight = light;

    Property *prop( node->findPropertyByName( "type" ) );
    if ( nullptr != prop ) {
        if ( nullptr != prop->m_value ) {
            std::string typeStr( prop->m_value->getString() );
            if ( "point" == typeStr ) {
                m_currentLight->mType = aiLightSource_POINT;
            } else if ( "spot" == typeStr ) {
                m_currentLight->mType = aiLightSource_SPOT;
            } else if ( "infinite" == typeStr ) {
                m_currentLight->mType = aiLightSource_DIRECTIONAL;
            }
        }
    }

    // parameters will be parsed normally in the tree, so just go for it
    handleNodes( node, pScene );
}

//------------------------------------------------------------------------------------------------
static void setMatrix( aiNode *node, DataArrayList *transformData ) {
    ai_assert( nullptr != node );
    ai_assert( nullptr != transformData );

    float m[ 16 ];
    size_t i( 1 );
    Value *next( transformData->m_dataList->m_next );
    m[ 0 ] = transformData->m_dataList->getFloat();
    while(  next != nullptr ) {
        m[ i ] = next->getFloat();
        next = next->m_next;
        i++;
    }

    ai_assert(i == 16);

    node->mTransformation.a1 = m[ 0 ];
    node->mTransformation.a2 = m[ 4 ];
    node->mTransformation.a3 = m[ 8 ];
    node->mTransformation.a4 = m[ 12 ];

    node->mTransformation.b1 = m[ 1 ];
    node->mTransformation.b2 = m[ 5 ];
    node->mTransformation.b3 = m[ 9 ];
    node->mTransformation.b4 = m[ 13 ];

    node->mTransformation.c1 = m[ 2 ];
    node->mTransformation.c2 = m[ 6 ];
    node->mTransformation.c3 = m[ 10 ];
    node->mTransformation.c4 = m[ 14 ];

    node->mTransformation.d1 = m[ 3 ];
    node->mTransformation.d2 = m[ 7 ];
    node->mTransformation.d3 = m[ 11 ];
    node->mTransformation.d4 = m[ 15 ];
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleTransformNode( ODDLParser::DDLNode *node, aiScene * /*pScene*/ ) {
    if( nullptr == m_currentNode ) {
        throw DeadlyImportError( "No parent node for name." );
        return;
    }

    DataArrayList *transformData( node->getDataArrayList() );
    if( nullptr != transformData ) {
        if( transformData->m_numItems != 16 ) {
            throw DeadlyImportError( "Invalid number of data for transform matrix." );
            return;
        }
        setMatrix( m_currentNode, transformData );
    }
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleMeshNode( ODDLParser::DDLNode *node, aiScene *pScene ) {
    m_currentMesh = new aiMesh;
    const size_t meshidx( m_meshCache.size() );
    m_meshCache.push_back( m_currentMesh );

    Property *prop = node->getProperties();
    if( nullptr != prop ) {
        std::string propName, propKey;
        propId2StdString( prop, propName, propKey );
        if( "primitive" == propName ) {
            if ( "points" == propKey ) {
                m_currentMesh->mPrimitiveTypes |= aiPrimitiveType_POINT;
            } else if ( "lines" == propKey ) {
                m_currentMesh->mPrimitiveTypes |= aiPrimitiveType_LINE;
            } else if( "triangles" == propKey ) {
                m_currentMesh->mPrimitiveTypes |= aiPrimitiveType_TRIANGLE;
            } else if ( "quads" == propKey ) {
                m_currentMesh->mPrimitiveTypes |= aiPrimitiveType_POLYGON;
            } else {
                DefaultLogger::get()->warn( propKey + " is not supported primitive type." );
            }
        }
    }

    handleNodes( node, pScene );

    DDLNode *parent( node->getParent() );
    if( nullptr != parent ) {
        const std::string &name = parent->getName();
        m_mesh2refMap[ name ] = meshidx;
    }
}

//------------------------------------------------------------------------------------------------
enum MeshAttribute {
    None,
    Position,
    Color,
    Normal,
    TexCoord
};

//------------------------------------------------------------------------------------------------
static MeshAttribute getAttributeByName( const char *attribName ) {
    ai_assert( nullptr != attribName  );

    if ( 0 == strncmp( "position", attribName, strlen( "position" ) ) ) {
        return Position;
    } else if ( 0 == strncmp( "color", attribName, strlen( "color" ) ) ) {
        return Color;
    } else if( 0 == strncmp( "normal", attribName, strlen( "normal" ) ) ) {
        return Normal;
    } else if( 0 == strncmp( "texcoord", attribName, strlen( "texcoord" ) ) ) {
        return TexCoord;
    }

    return None;
}

//------------------------------------------------------------------------------------------------
static void fillVector3( aiVector3D *vec3, Value *vals ) {
    ai_assert( nullptr != vec3 );
    ai_assert( nullptr != vals );

    float x( 0.0f ), y( 0.0f ), z( 0.0f );
    Value *next( vals );
    x = next->getFloat();
    next = next->m_next;
    y = next->getFloat();
    next = next->m_next;
    if( nullptr != next ) {
        z = next->getFloat();
    }

    vec3->Set( x, y, z );
}

//------------------------------------------------------------------------------------------------
static void fillColor4( aiColor4D *col4, Value *vals ) {
    ai_assert( nullptr != col4 );
    ai_assert( nullptr != vals );

    Value *next( vals );
    col4->r = next->getFloat();
    next = next->m_next;
    col4->g = next->getFloat();
    next = next->m_next;
    col4->b = next->getFloat();
    next = next->m_next;
    col4->a = next->getFloat();
}

//------------------------------------------------------------------------------------------------
static size_t countDataArrayListItems( DataArrayList *vaList ) {
    size_t numItems( 0 );
    if( nullptr == vaList ) {
        return numItems;
    }

    DataArrayList *next( vaList );
    while( nullptr != next ) {
        if( nullptr != vaList->m_dataList ) {
            numItems++;
        }
        next = next->m_next;
    }

    return numItems;
}

//------------------------------------------------------------------------------------------------
static void copyVectorArray( size_t numItems, DataArrayList *vaList, aiVector3D *vectorArray ) {
    for( size_t i = 0; i < numItems; i++ ) {
        Value *next( vaList->m_dataList );
        fillVector3( &vectorArray[ i ], next );
        vaList = vaList->m_next;
    }
}

//------------------------------------------------------------------------------------------------
static void copyColor4DArray( size_t numItems, DataArrayList *vaList, aiColor4D *colArray ) {
    for ( size_t i = 0; i < numItems; i++ ) {
        Value *next( vaList->m_dataList );
        fillColor4( &colArray[ i ], next );
    }
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleVertexArrayNode( ODDLParser::DDLNode *node, aiScene * /*pScene*/ ) {
    if( nullptr == node ) {
        throw DeadlyImportError( "No parent node for name." );
        return;
    }

    Property *prop( node->getProperties() );
    if( nullptr != prop ) {
        std::string propName, propKey;
        propId2StdString( prop, propName, propKey );
        MeshAttribute attribType( getAttributeByName( propKey.c_str() ) );
        if( None == attribType ) {
            return;
        }

        DataArrayList *vaList = node->getDataArrayList();
        if( nullptr == vaList ) {
            return;
        }

        const size_t numItems( countDataArrayListItems( vaList ) );

        if( Position == attribType ) {
            m_currentVertices.m_numVerts = numItems;
            m_currentVertices.m_vertices = new aiVector3D[ numItems ];
            copyVectorArray( numItems, vaList, m_currentVertices.m_vertices );
        } else if ( Color == attribType ) {
            m_currentVertices.m_numColors = numItems;
            m_currentVertices.m_colors = new aiColor4D[ numItems ];
            copyColor4DArray( numItems, vaList, m_currentVertices.m_colors );
        } else if( Normal == attribType ) {
            m_currentVertices.m_numNormals = numItems;
            m_currentVertices.m_normals = new aiVector3D[ numItems ];
            copyVectorArray( numItems, vaList, m_currentVertices.m_normals );
        } else if( TexCoord == attribType ) {
            m_currentVertices.m_numUVComps[ 0 ] = numItems;
            m_currentVertices.m_textureCoords[ 0 ] = new aiVector3D[ numItems ];
            copyVectorArray( numItems, vaList, m_currentVertices.m_textureCoords[ 0 ] );
        }
    }
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleIndexArrayNode( ODDLParser::DDLNode *node, aiScene * /*pScene*/ ) {
    if( nullptr == node ) {
        throw DeadlyImportError( "No parent node for name." );
        return;
    }

    if( nullptr == m_currentMesh ) {
        throw DeadlyImportError( "No current mesh for index data found." );
        return;
    }

    DataArrayList *vaList = node->getDataArrayList();
    if( nullptr == vaList ) {
        return;
    }

    const size_t numItems( countDataArrayListItems( vaList ) );
    m_currentMesh->mNumFaces = static_cast<unsigned int>(numItems);
    m_currentMesh->mFaces = new aiFace[ numItems ];
    m_currentMesh->mNumVertices = static_cast<unsigned int>(numItems * 3);
    m_currentMesh->mVertices = new aiVector3D[ m_currentMesh->mNumVertices ];
    bool hasColors( false );
    if ( m_currentVertices.m_numColors > 0 ) {
        m_currentMesh->mColors[0] = new aiColor4D[ m_currentVertices.m_numColors ];
        hasColors = true;
    }
    bool hasNormalCoords( false );
    if ( m_currentVertices.m_numNormals > 0 ) {
        m_currentMesh->mNormals = new aiVector3D[ m_currentMesh->mNumVertices ];
        hasNormalCoords = true;
    }
    bool hasTexCoords( false );
    if ( m_currentVertices.m_numUVComps[ 0 ] > 0 ) {
        m_currentMesh->mTextureCoords[ 0 ] = new aiVector3D[ m_currentMesh->mNumVertices ];
        hasTexCoords = true;
    }

    unsigned int index( 0 );
    for( size_t i = 0; i < m_currentMesh->mNumFaces; i++ ) {
        aiFace &current(  m_currentMesh->mFaces[ i ] );
        current.mNumIndices = 3;
        current.mIndices = new unsigned int[ current.mNumIndices ];
        Value *next( vaList->m_dataList );
        for( size_t indices = 0; indices < current.mNumIndices; indices++ ) {
            const int idx( next->getUnsignedInt32() );
            ai_assert( static_cast<size_t>( idx ) <= m_currentVertices.m_numVerts );
            ai_assert( index < m_currentMesh->mNumVertices );
            aiVector3D &pos = ( m_currentVertices.m_vertices[ idx ] );
            m_currentMesh->mVertices[ index ].Set( pos.x, pos.y, pos.z );
            if ( hasColors ) {
                aiColor4D &col = m_currentVertices.m_colors[ idx ];
                m_currentMesh->mColors[ 0 ][ index ] = col;
            }
            if ( hasNormalCoords ) {
                aiVector3D &normal = ( m_currentVertices.m_normals[ idx ] );
                m_currentMesh->mNormals[ index ].Set( normal.x, normal.y, normal.z );
            }
            if ( hasTexCoords ) {
                aiVector3D &tex = ( m_currentVertices.m_textureCoords[ 0 ][ idx ] );
                m_currentMesh->mTextureCoords[ 0 ][ index ].Set( tex.x, tex.y, tex.z );
            }
            current.mIndices[ indices ] = index;
            index++;

            next = next->m_next;
        }
        vaList = vaList->m_next;
    }
}

//------------------------------------------------------------------------------------------------
static void getColorRGB3( aiColor3D *pColor, DataArrayList *colList ) {
    if( nullptr == pColor || nullptr == colList ) {
        return;
    }

    ai_assert( 3 == colList->m_numItems );
    Value *val( colList->m_dataList );
    pColor->r = val->getFloat();
    val = val->getNext();
    pColor->g = val->getFloat();
    val = val->getNext();
    pColor->b = val->getFloat();
}

//------------------------------------------------------------------------------------------------
static void getColorRGB4( aiColor4D *pColor, DataArrayList *colList ) {
    if ( nullptr == pColor || nullptr == colList ) {
        return;
    }

    ai_assert( 4 == colList->m_numItems );
    Value *val( colList->m_dataList );
    pColor->r = val->getFloat();
    val = val->getNext();
    pColor->g = val->getFloat();
    val = val->getNext();
    pColor->b = val->getFloat();
    val = val->getNext();
    pColor->a = val->getFloat();
}

//------------------------------------------------------------------------------------------------
enum ColorType {
    NoneColor = 0,
    DiffuseColor,
    SpecularColor,
    EmissionColor,
    LightColor
};

//------------------------------------------------------------------------------------------------
static ColorType getColorType( Text *id ) {
    if ( nullptr == id ) {
        return NoneColor;
    }

    if( *id == Grammar::DiffuseColorToken ) {
        return DiffuseColor;
    } else if( *id == Grammar::SpecularColorToken ) {
        return SpecularColor;
    } else if( *id == Grammar::EmissionColorToken ) {
        return EmissionColor;
    } else if ( *id == "light" ) {
        return LightColor;
    }

    return NoneColor;
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleMaterialNode( ODDLParser::DDLNode *node, aiScene *pScene ) {
    m_currentMaterial = new aiMaterial;
    m_materialCache.push_back( m_currentMaterial );
    m_tokenType = Grammar::MaterialToken;
    handleNodes( node, pScene );
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleColorNode( ODDLParser::DDLNode *node, aiScene * /*pScene*/ ) {
    if( nullptr == node ) {
        return;
    }

    Property *prop = node->findPropertyByName( "attrib" );
    if( nullptr != prop ) {
        if( nullptr != prop->m_value ) {
            DataArrayList *colList( node->getDataArrayList() );
            if( nullptr == colList ) {
                return;
            }
            aiColor3D col;
            if ( 3 == colList->m_numItems ) {
                aiColor3D col3;
                getColorRGB3( &col3, colList );
                col = col3;
            } else {
                aiColor4D col4;
                getColorRGB4( &col4, colList );
                col.r = col4.r;
                col.g = col4.g;
                col.b = col4.b;
            }
            const ColorType colType( getColorType( prop->m_key ) );
            if( DiffuseColor == colType ) {
                m_currentMaterial->AddProperty( &col, 1, AI_MATKEY_COLOR_DIFFUSE );
            } else if( SpecularColor == colType ) {
                m_currentMaterial->AddProperty( &col, 1, AI_MATKEY_COLOR_SPECULAR );
            } else if( EmissionColor == colType ) {
                m_currentMaterial->AddProperty( &col, 1, AI_MATKEY_COLOR_EMISSIVE );
            } else if ( LightColor == colType ) {
                m_currentLight->mColorDiffuse = col;
            }
        }
    }
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleTextureNode( ODDLParser::DDLNode *node, aiScene * /*pScene*/ ) {
    if( nullptr == node ) {
        return;
    }

    Property *prop = node->findPropertyByName( "attrib" );
    if( nullptr != prop ) {
        if( nullptr != prop->m_value ) {
            Value *val( node->getValue() );
            if( nullptr != val ) {
                aiString tex;
                tex.Set( val->getString() );
                if( prop->m_value->getString() == Grammar::DiffuseTextureToken ) {
                    m_currentMaterial->AddProperty( &tex, AI_MATKEY_TEXTURE_DIFFUSE( 0 ) );
                } else if( prop->m_value->getString() == Grammar::SpecularPowerTextureToken ) {
                    m_currentMaterial->AddProperty( &tex, AI_MATKEY_TEXTURE_SPECULAR( 0 ) );
                } else if( prop->m_value->getString() == Grammar::EmissionTextureToken ) {
                    m_currentMaterial->AddProperty( &tex, AI_MATKEY_TEXTURE_EMISSIVE( 0 ) );
                } else if( prop->m_value->getString() == Grammar::OpacyTextureToken ) {
                    m_currentMaterial->AddProperty( &tex, AI_MATKEY_TEXTURE_OPACITY( 0 ) );
                } else if( prop->m_value->getString() == Grammar::TransparencyTextureToken ) {
                    // ToDo!
                    // m_currentMaterial->AddProperty( &tex, AI_MATKEY_TEXTURE_DIFFUSE( 0 ) );
                } else if( prop->m_value->getString() == Grammar::NormalTextureToken ) {
                    m_currentMaterial->AddProperty( &tex, AI_MATKEY_TEXTURE_NORMALS( 0 ) );
                } else {
                    ai_assert( false );
                }
            }
        }
    }
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleParamNode( ODDLParser::DDLNode *node, aiScene * /*pScene*/ ) {
    if ( nullptr == node ) {
        return;
    }

    Property *prop = node->findPropertyByName( "attrib" );
    if ( nullptr == prop ) {
        return;
    }

    if ( nullptr != prop->m_value ) {
        Value *val( node->getValue() );
        if ( nullptr == val ) {
            return;
        }
        const float floatVal( val->getFloat() );
        if ( prop->m_value  != nullptr ) {
            if ( 0 == ASSIMP_strincmp( "fov", prop->m_value->getString(), 3 ) ) {
                m_currentCamera->mHorizontalFOV = floatVal;
            } else if ( 0 == ASSIMP_strincmp( "near", prop->m_value->getString(), 3 ) ) {
                m_currentCamera->mClipPlaneNear = floatVal;
            } else if ( 0 == ASSIMP_strincmp( "far", prop->m_value->getString(), 3 ) ) {
                m_currentCamera->mClipPlaneFar = floatVal;
            }
        }
    }
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::handleAttenNode( ODDLParser::DDLNode *node, aiScene * /*pScene*/ ) {
    if ( nullptr == node ) {
        return;
    }

    Property *prop = node->findPropertyByName( "curve" );
    if ( nullptr != prop ) {
        if ( nullptr != prop->m_value ) {
            Value *val( node->getValue() );
            const float floatVal( val->getFloat() );
            if ( 0 == strncmp( "scale", prop->m_value->getString(), strlen( "scale" ) ) ) {
                m_currentLight->mAttenuationQuadratic = floatVal;
            }
        }
    }
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::copyMeshes( aiScene *pScene ) {
    ai_assert( nullptr != pScene );

    if( m_meshCache.empty() ) {
        return;
    }

    pScene->mNumMeshes = static_cast<unsigned int>(m_meshCache.size());
    pScene->mMeshes = new aiMesh*[ pScene->mNumMeshes ];
    std::copy( m_meshCache.begin(), m_meshCache.end(), pScene->mMeshes );
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::copyCameras( aiScene *pScene ) {
    ai_assert( nullptr != pScene );

    if ( m_cameraCache.empty() ) {
        return;
    }

    pScene->mNumCameras = static_cast<unsigned int>(m_cameraCache.size());
    pScene->mCameras = new aiCamera*[ pScene->mNumCameras ];
    std::copy( m_cameraCache.begin(), m_cameraCache.end(), pScene->mCameras );
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::copyLights( aiScene *pScene ) {
    ai_assert( nullptr != pScene );

    if ( m_lightCache.empty() ) {
        return;
    }

    pScene->mNumLights = static_cast<unsigned int>(m_lightCache.size());
    pScene->mLights = new aiLight*[ pScene->mNumLights ];
    std::copy( m_lightCache.begin(), m_lightCache.end(), pScene->mLights );
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::copyMaterials( aiScene *pScene ) {
    ai_assert( nullptr != pScene );

    if ( m_materialCache.empty() ) {
        return;
    }

    pScene->mNumMaterials = static_cast<unsigned int>(m_materialCache.size());
    pScene->mMaterials = new aiMaterial*[ pScene->mNumMaterials ];
    std::copy( m_materialCache.begin(), m_materialCache.end(), pScene->mMaterials );
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::resolveReferences() {
    if( m_unresolvedRefStack.empty() ) {
        return;
    }

    RefInfo *currentRefInfo( nullptr );
    for( auto it = m_unresolvedRefStack.begin(); it != m_unresolvedRefStack.end(); ++it ) {
        currentRefInfo = it->get();
        if( nullptr != currentRefInfo ) {
            aiNode *node( currentRefInfo->m_node );
            if( RefInfo::MeshRef == currentRefInfo->m_type ) {
                for( size_t i = 0; i < currentRefInfo->m_Names.size(); ++i ) {
                    const std::string &name( currentRefInfo->m_Names[ i ] );
                    ReferenceMap::const_iterator it( m_mesh2refMap.find( name ) );
                    if( m_mesh2refMap.end() != it ) {
                        unsigned int meshIdx = static_cast<unsigned int>(m_mesh2refMap[ name ]);
                        node->mMeshes[ i ] = meshIdx;
                    }
                }
            } else if( RefInfo::MaterialRef == currentRefInfo->m_type ) {
                for ( size_t i = 0; i < currentRefInfo->m_Names.size(); ++i ) {
                    const std::string name( currentRefInfo->m_Names[ i ] );
                    ReferenceMap::const_iterator it( m_material2refMap.find( name ) );
                    if ( m_material2refMap.end() != it ) {
                        if ( nullptr != m_currentMesh ) {
                            unsigned int matIdx = static_cast< unsigned int >( m_material2refMap[ name ] );
                            if ( m_currentMesh->mMaterialIndex != 0 ) {
                                DefaultLogger::get()->warn( "Override of material reference in current mesh by material reference." );
                            }
                            m_currentMesh->mMaterialIndex = matIdx;
                        }  else {
                            DefaultLogger::get()->warn( "Cannot resolve material reference, because no current mesh is there." );

                        }
                    }
                }
            } else {
                throw DeadlyImportError( "Unknown reference info to resolve." );
            }
        }
    }
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::createNodeTree( aiScene *pScene ) {
    if( nullptr == m_root ) {
        return;
    }

    if( m_root->m_children.empty() ) {
        return;
    }

    pScene->mRootNode->mNumChildren = static_cast<unsigned int>(m_root->m_children.size());
    pScene->mRootNode->mChildren = new aiNode*[ pScene->mRootNode->mNumChildren ];
    std::copy( m_root->m_children.begin(), m_root->m_children.end(), pScene->mRootNode->mChildren );
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::pushNode( aiNode *node, aiScene *pScene ) {
    ai_assert( nullptr != pScene );

    if ( nullptr == node ) {
        return;
    }

    ChildInfo *info( nullptr );
    if( m_nodeStack.empty() ) {
        node->mParent = pScene->mRootNode;
        NodeChildMap::iterator it( m_nodeChildMap.find( node->mParent ) );
        if( m_nodeChildMap.end() == it ) {
            info = new ChildInfo;
            m_root = info;
            m_nodeChildMap[ node->mParent ] = std::unique_ptr<ChildInfo>(info);
        } else {
            info = it->second.get();
        }
        info->m_children.push_back( node );
    } else {
        aiNode *parent( m_nodeStack.back() );
        ai_assert( nullptr != parent );
        node->mParent = parent;
        NodeChildMap::iterator it( m_nodeChildMap.find( node->mParent ) );
        if( m_nodeChildMap.end() == it ) {
            info = new ChildInfo;
            m_nodeChildMap[ node->mParent ] = std::unique_ptr<ChildInfo>(info);
        } else {
            info = it->second.get();
        }
        info->m_children.push_back( node );
    }
    m_nodeStack.push_back( node );
}

//------------------------------------------------------------------------------------------------
aiNode *OpenGEXImporter::popNode() {
    if( m_nodeStack.empty() ) {
        return nullptr;
    }

    aiNode *node( top() );
    m_nodeStack.pop_back();

    return node;
}

//------------------------------------------------------------------------------------------------
aiNode *OpenGEXImporter::top() const {
    if( m_nodeStack.empty() ) {
        return nullptr;
    }

    return m_nodeStack.back();
}

//------------------------------------------------------------------------------------------------
void OpenGEXImporter::clearNodeStack() {
    m_nodeStack.clear();
}

//------------------------------------------------------------------------------------------------

} // Namespace OpenGEX
} // Namespace Assimp

#endif // ASSIMP_BUILD_NO_OPENGEX_IMPORTER
