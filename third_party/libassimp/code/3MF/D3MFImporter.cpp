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

#ifndef ASSIMP_BUILD_NO_3MF_IMPORTER

#include "D3MFImporter.h"

#include <assimp/scene.h>
#include <assimp/IOSystem.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/importerdesc.h>
#include <assimp/StringComparison.h>
#include <assimp/StringUtils.h>
#include <assimp/ZipArchiveIOSystem.h>

#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <memory>

#include "D3MFOpcPackage.h"
#include <assimp/irrXMLWrapper.h>
#include "3MFXmlTags.h"
#include <assimp/fast_atof.h>

#include <iomanip>

namespace Assimp {
namespace D3MF {

class XmlSerializer {
public:
    using MatArray = std::vector<aiMaterial*>;
    using MatId2MatArray = std::map<unsigned int, std::vector<unsigned int>>;

    XmlSerializer(XmlReader* xmlReader)
    : mMeshes()
    , mMatArray()
    , mActiveMatGroup( 99999999 )
    , mMatId2MatArray()
    , xmlReader(xmlReader){
		// empty
    }

    ~XmlSerializer() {
        // empty
    }

    void ImportXml(aiScene* scene) {
        if ( nullptr == scene ) {
            return;
        }

        scene->mRootNode = new aiNode();
        std::vector<aiNode*> children;

        std::string nodeName;
        while(ReadToEndElement(D3MF::XmlTag::model)) {
            nodeName = xmlReader->getNodeName();
            if( nodeName == D3MF::XmlTag::object) {
                children.push_back(ReadObject(scene));
            } else if( nodeName == D3MF::XmlTag::build) {
                // 
            } else if ( nodeName == D3MF::XmlTag::basematerials ) {
                ReadBaseMaterials();
            } else if ( nodeName == D3MF::XmlTag::meta ) {
                ReadMetadata();
            }
        }

        if ( scene->mRootNode->mName.length == 0 ) {
            scene->mRootNode->mName.Set( "3MF" );
        }

        // import the metadata
        if ( !mMetaData.empty() ) {
            const size_t numMeta( mMetaData.size() );
            scene->mMetaData = aiMetadata::Alloc(static_cast<unsigned int>( numMeta ) );
            for ( size_t i = 0; i < numMeta; ++i ) {
                aiString val( mMetaData[ i ].value );
                scene->mMetaData->Set(static_cast<unsigned int>( i ), mMetaData[ i ].name, val );
            }
        }

        // import the meshes
        scene->mNumMeshes = static_cast<unsigned int>( mMeshes.size());
        scene->mMeshes = new aiMesh*[scene->mNumMeshes]();
        std::copy( mMeshes.begin(), mMeshes.end(), scene->mMeshes);

        // import the materials
        scene->mNumMaterials = static_cast<unsigned int>( mMatArray.size() );
        if ( 0 != scene->mNumMaterials ) {
            scene->mMaterials = new aiMaterial*[ scene->mNumMaterials ];
            std::copy( mMatArray.begin(), mMatArray.end(), scene->mMaterials );
        }

        // create the scenegraph
        scene->mRootNode->mNumChildren = static_cast<unsigned int>(children.size());
        scene->mRootNode->mChildren = new aiNode*[scene->mRootNode->mNumChildren]();
        std::copy(children.begin(), children.end(), scene->mRootNode->mChildren);
    }

private:
    aiNode* ReadObject(aiScene* scene) {
        std::unique_ptr<aiNode> node(new aiNode());

        std::vector<unsigned long> meshIds;

        const char *attrib( nullptr );
        std::string name, type;
        attrib = xmlReader->getAttributeValue( D3MF::XmlTag::id.c_str() );
        if ( nullptr != attrib ) {
            name = attrib;
        }
        attrib = xmlReader->getAttributeValue( D3MF::XmlTag::type.c_str() );
        if ( nullptr != attrib ) {
            type = attrib;
        }

        node->mParent = scene->mRootNode;
        node->mName.Set(name);

        size_t meshIdx = mMeshes.size();

        while(ReadToEndElement(D3MF::XmlTag::object)) {
            if(xmlReader->getNodeName() == D3MF::XmlTag::mesh) {
                auto mesh = ReadMesh();

                mesh->mName.Set(name);
                mMeshes.push_back(mesh);
                meshIds.push_back(static_cast<unsigned long>(meshIdx));
                ++meshIdx;
            }
        }

        node->mNumMeshes = static_cast<unsigned int>(meshIds.size());

        node->mMeshes = new unsigned int[node->mNumMeshes];

        std::copy(meshIds.begin(), meshIds.end(), node->mMeshes);

        return node.release();
    }

    aiMesh *ReadMesh() {
        aiMesh* mesh = new aiMesh();
        while(ReadToEndElement(D3MF::XmlTag::mesh)) {
            if(xmlReader->getNodeName() == D3MF::XmlTag::vertices) {
                ImportVertices(mesh);
            } else if(xmlReader->getNodeName() == D3MF::XmlTag::triangles) {
                ImportTriangles(mesh);
            }
        }

        return mesh;
    }

    void ReadMetadata() {
        const std::string name = xmlReader->getAttributeValue( D3MF::XmlTag::meta_name.c_str() );
        xmlReader->read();
        const std::string value = xmlReader->getNodeData();

        if ( name.empty() ) {
            return;
        }

        MetaEntry entry;
        entry.name = name;
        entry.value = value;
        mMetaData.push_back( entry );
    }

    void ImportVertices(aiMesh* mesh) {
        std::vector<aiVector3D> vertices;
        while(ReadToEndElement(D3MF::XmlTag::vertices)) {
            if(xmlReader->getNodeName() == D3MF::XmlTag::vertex) {
                vertices.push_back(ReadVertex());
            }
        }
        mesh->mNumVertices = static_cast<unsigned int>(vertices.size());
        mesh->mVertices = new aiVector3D[mesh->mNumVertices];

        std::copy(vertices.begin(), vertices.end(), mesh->mVertices);
    }

    aiVector3D ReadVertex() {
        aiVector3D vertex;

        vertex.x = ai_strtof(xmlReader->getAttributeValue(D3MF::XmlTag::x.c_str()), nullptr);
        vertex.y = ai_strtof(xmlReader->getAttributeValue(D3MF::XmlTag::y.c_str()), nullptr);
        vertex.z = ai_strtof(xmlReader->getAttributeValue(D3MF::XmlTag::z.c_str()), nullptr);

        return vertex;
    }

    void ImportTriangles(aiMesh* mesh) {
         std::vector<aiFace> faces;

         while(ReadToEndElement(D3MF::XmlTag::triangles)) {
             const std::string nodeName( xmlReader->getNodeName() );
             if(xmlReader->getNodeName() == D3MF::XmlTag::triangle) {
                 faces.push_back(ReadTriangle());
                 const char *pidToken( xmlReader->getAttributeValue( D3MF::XmlTag::p1.c_str() ) );
                 if ( nullptr != pidToken ) {
                     int matIdx( std::atoi( pidToken ) );
                     mesh->mMaterialIndex = matIdx;
                 }
             }
         }

        mesh->mNumFaces = static_cast<unsigned int>(faces.size());
        mesh->mFaces = new aiFace[mesh->mNumFaces];
        mesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;

        std::copy(faces.begin(), faces.end(), mesh->mFaces);
    }

    aiFace ReadTriangle() {
        aiFace face;

        face.mNumIndices = 3;
        face.mIndices = new unsigned int[face.mNumIndices];
        face.mIndices[0] = static_cast<unsigned int>(std::atoi(xmlReader->getAttributeValue(D3MF::XmlTag::v1.c_str())));
        face.mIndices[1] = static_cast<unsigned int>(std::atoi(xmlReader->getAttributeValue(D3MF::XmlTag::v2.c_str())));
        face.mIndices[2] = static_cast<unsigned int>(std::atoi(xmlReader->getAttributeValue(D3MF::XmlTag::v3.c_str())));

        return face;
    }

    void ReadBaseMaterials() {
        std::vector<unsigned int> MatIdArray;
        const char *baseMaterialId( xmlReader->getAttributeValue( D3MF::XmlTag::basematerials_id.c_str() ) );
        if ( nullptr != baseMaterialId ) {
            unsigned int id = std::atoi( baseMaterialId );
            const size_t newMatIdx( mMatArray.size() );
            if ( id != mActiveMatGroup ) {
                mActiveMatGroup = id;
                MatId2MatArray::const_iterator it( mMatId2MatArray.find( id ) );
                if ( mMatId2MatArray.end() == it ) {
                    MatIdArray.clear();
                    mMatId2MatArray[ id ] = MatIdArray;
                } else {
                    MatIdArray = it->second;
                }
            }
            MatIdArray.push_back( static_cast<unsigned int>( newMatIdx ) );
            mMatId2MatArray[ mActiveMatGroup ] = MatIdArray;
        }

        while ( ReadToEndElement( D3MF::XmlTag::basematerials ) ) {
            mMatArray.push_back( readMaterialDef() );
        }
    }

    bool parseColor( const char *color, aiColor4D &diffuse ) {
        if ( nullptr == color ) {
            return false;
        }

        //format of the color string: #RRGGBBAA or #RRGGBB (3MF Core chapter 5.1.1)
        const size_t len( strlen( color ) );
        if ( 9 != len && 7 != len) {
            return false;
        }

        const char *buf( color );
        if ( '#' != *buf ) {
            return false;
        }
        ++buf;
        char comp[ 3 ] = { 0,0,'\0' };

        comp[ 0 ] = *buf;
        ++buf;
        comp[ 1 ] = *buf;
        ++buf;
        diffuse.r = static_cast<ai_real>( strtol( comp, NULL, 16 ) ) / ai_real(255.0);


        comp[ 0 ] = *buf;
        ++buf;
        comp[ 1 ] = *buf;
        ++buf;
        diffuse.g = static_cast< ai_real >( strtol( comp, NULL, 16 ) ) / ai_real(255.0);

        comp[ 0 ] = *buf;
        ++buf;
        comp[ 1 ] = *buf;
        ++buf;
        diffuse.b = static_cast< ai_real >( strtol( comp, NULL, 16 ) ) / ai_real(255.0);

        if(7 == len)
            return true;
        comp[ 0 ] = *buf;
        ++buf;
        comp[ 1 ] = *buf;
        ++buf;
        diffuse.a = static_cast< ai_real >( strtol( comp, NULL, 16 ) ) / ai_real(255.0);

        return true;
    }

    void assignDiffuseColor( aiMaterial *mat ) {
        const char *color = xmlReader->getAttributeValue( D3MF::XmlTag::basematerials_displaycolor.c_str() );
        aiColor4D diffuse;
        if ( parseColor( color, diffuse ) ) {
            mat->AddProperty<aiColor4D>( &diffuse, 1, AI_MATKEY_COLOR_DIFFUSE );
        }

    }
    aiMaterial *readMaterialDef() {
        aiMaterial *mat( nullptr );
        const char *name( nullptr );
        const std::string nodeName( xmlReader->getNodeName() );
        if ( nodeName == D3MF::XmlTag::basematerials_base ) {
            name = xmlReader->getAttributeValue( D3MF::XmlTag::basematerials_name.c_str() );
            std::string stdMatName;
            aiString matName;
            std::string strId( to_string( mActiveMatGroup ) );
            stdMatName += "id";
            stdMatName += strId;
            stdMatName += "_";
            if ( nullptr != name ) {
                stdMatName += std::string( name );
            } else {
                stdMatName += "basemat";
            }
            matName.Set( stdMatName );

            mat = new aiMaterial;
            mat->AddProperty( &matName, AI_MATKEY_NAME );

            assignDiffuseColor( mat );
        }

        return mat;
    }

private:
    bool ReadToStartElement(const std::string& startTag) {
        while(xmlReader->read()) {
            const std::string &nodeName( xmlReader->getNodeName() );
            if (xmlReader->getNodeType() == irr::io::EXN_ELEMENT && nodeName == startTag) {
                return true;
            } else if (xmlReader->getNodeType() == irr::io::EXN_ELEMENT_END && nodeName == startTag) {
                return false;
            }
        }

        return false;
    }

    bool ReadToEndElement(const std::string& closeTag) {
        while(xmlReader->read()) {
            const std::string &nodeName( xmlReader->getNodeName() );
            if (xmlReader->getNodeType() == irr::io::EXN_ELEMENT) {
                return true;
            } else if (xmlReader->getNodeType() == irr::io::EXN_ELEMENT_END && nodeName == closeTag) {
                return false;
            }
        }
        ASSIMP_LOG_ERROR("unexpected EOF, expected closing <" + closeTag + "> tag");

        return false;
    }

private:
    struct MetaEntry {
        std::string name;
        std::string value;
    };
    std::vector<MetaEntry> mMetaData;
    std::vector<aiMesh*> mMeshes;
    MatArray mMatArray;
    unsigned int mActiveMatGroup;
    MatId2MatArray mMatId2MatArray;
    XmlReader* xmlReader;
};

} //namespace D3MF

static const aiImporterDesc desc = {
    "3mf Importer",
    "",
    "",
    "http://3mf.io/",
    aiImporterFlags_SupportBinaryFlavour | aiImporterFlags_SupportCompressedFlavour,
    0,
    0,
    0,
    0,
    "3mf"
};

D3MFImporter::D3MFImporter()
: BaseImporter() {
    // empty
}

D3MFImporter::~D3MFImporter() {
    // empty
}

bool D3MFImporter::CanRead(const std::string &filename, IOSystem *pIOHandler, bool checkSig) const {
    const std::string extension( GetExtension( filename ) );
    if(extension == desc.mFileExtensions ) {
        return true;
    } else if ( !extension.length() || checkSig ) {
        if ( nullptr == pIOHandler ) {
            return false;
        }
        if ( !ZipArchiveIOSystem::isZipArchive( pIOHandler, filename ) ) {
            return false;
        }
        D3MF::D3MFOpcPackage opcPackage( pIOHandler, filename );
        return opcPackage.validate();
    }

    return false;
}

void D3MFImporter::SetupProperties(const Importer * /*pImp*/) {
    // empty
}

const aiImporterDesc *D3MFImporter::GetInfo() const {
    return &desc;
}

void D3MFImporter::InternReadFile( const std::string &filename, aiScene *pScene, IOSystem *pIOHandler ) {
    D3MF::D3MFOpcPackage opcPackage(pIOHandler, filename);

    std::unique_ptr<CIrrXML_IOStreamReader> xmlStream(new CIrrXML_IOStreamReader(opcPackage.RootStream()));
    std::unique_ptr<D3MF::XmlReader> xmlReader(irr::io::createIrrXMLReader(xmlStream.get()));

    D3MF::XmlSerializer xmlSerializer(xmlReader.get());

    xmlSerializer.ImportXml(pScene);
}

} // Namespace Assimp

#endif // ASSIMP_BUILD_NO_3MF_IMPORTER
