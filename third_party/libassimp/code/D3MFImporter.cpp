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

#ifndef ASSIMP_BUILD_NO_3MF_IMPORTER

#include "D3MFImporter.h"

#include <assimp/scene.h>
#include <assimp/IOSystem.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/importerdesc.h>
#include "StringComparison.h"
#include "StringUtils.h"

#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <memory>

#include "D3MFOpcPackage.h"
#include <contrib/unzip/unzip.h>
#include "irrXMLWrapper.h"
#include "3MFXmlTags.h"

namespace Assimp {
namespace D3MF {

class XmlSerializer {
public:
    XmlSerializer(XmlReader* xmlReader)
    : xmlReader(xmlReader) {
		// empty
    }

    ~XmlSerializer() {
        // empty
    }

    void ImportXml(aiScene* scene) {
        scene->mRootNode = new aiNode();
        std::vector<aiNode*> children;

        while(ReadToEndElement(D3MF::XmlTag::model)) {
            if(xmlReader->getNodeName() == D3MF::XmlTag::object) {
                children.push_back(ReadObject(scene));
            } else if(xmlReader->getNodeName() == D3MF::XmlTag::build) {

            }
        }

        if ( scene->mRootNode->mName.length == 0 ) {
            scene->mRootNode->mName.Set( "3MF" );
        }

        scene->mNumMeshes = static_cast<unsigned int>(meshes.size());
        scene->mMeshes = new aiMesh*[scene->mNumMeshes]();

        std::copy(meshes.begin(), meshes.end(), scene->mMeshes);

        scene->mRootNode->mNumChildren = static_cast<unsigned int>(children.size());
        scene->mRootNode->mChildren = new aiNode*[scene->mRootNode->mNumChildren]();

        std::copy(children.begin(), children.end(), scene->mRootNode->mChildren);
    }

private:
    aiNode* ReadObject(aiScene* scene)
    {
        std::unique_ptr<aiNode> node(new aiNode());

        std::vector<unsigned long> meshIds;

        const char *attrib( nullptr );
        std::string name, type;
        attrib = xmlReader->getAttributeValue( D3MF::XmlTag::name.c_str() );
        if ( nullptr != attrib ) {
            name = attrib;
        }
        attrib = xmlReader->getAttributeValue( D3MF::XmlTag::name.c_str() );
        if ( nullptr != attrib ) {
            type = attrib;
        }

        node->mParent = scene->mRootNode;
        node->mName.Set(name);

        size_t meshIdx = meshes.size();

        while(ReadToEndElement(D3MF::XmlTag::object))
        {
            if(xmlReader->getNodeName() == D3MF::XmlTag::mesh)
            {
                auto mesh = ReadMesh();

                mesh->mName.Set(name);
                meshes.push_back(mesh);
                meshIds.push_back(static_cast<unsigned long>(meshIdx));
                meshIdx++;

            }
        }

        node->mNumMeshes = static_cast<unsigned int>(meshIds.size());

        node->mMeshes = new unsigned int[node->mNumMeshes];

        std::copy(meshIds.begin(), meshIds.end(), node->mMeshes);

        return node.release();

    }

    aiMesh* ReadMesh() {
        aiMesh* mesh = new aiMesh();
        while(ReadToEndElement(D3MF::XmlTag::mesh))
        {
            if(xmlReader->getNodeName() == D3MF::XmlTag::vertices)
            {
                ImportVertices(mesh);
            }
            else if(xmlReader->getNodeName() == D3MF::XmlTag::triangles)
            {
                ImportTriangles(mesh);
            }
        }

        return mesh;
    }

    void ImportVertices(aiMesh* mesh)
    {
        std::vector<aiVector3D> vertices;

        while(ReadToEndElement(D3MF::XmlTag::vertices))
        {
            if(xmlReader->getNodeName() == D3MF::XmlTag::vertex)
            {
                vertices.push_back(ReadVertex());
            }
        }
        mesh->mNumVertices = static_cast<unsigned int>(vertices.size());
        mesh->mVertices = new aiVector3D[mesh->mNumVertices];

        std::copy(vertices.begin(), vertices.end(), mesh->mVertices);

    }

    aiVector3D ReadVertex()
    {
        aiVector3D vertex;

        vertex.x = ai_strtof(xmlReader->getAttributeValue(D3MF::XmlTag::x.c_str()), nullptr);
        vertex.y = ai_strtof(xmlReader->getAttributeValue(D3MF::XmlTag::y.c_str()), nullptr);
        vertex.z = ai_strtof(xmlReader->getAttributeValue(D3MF::XmlTag::z.c_str()), nullptr);

        return vertex;
    }

    void ImportTriangles(aiMesh* mesh)
    {
         std::vector<aiFace> faces;


         while(ReadToEndElement(D3MF::XmlTag::triangles))
         {
             if(xmlReader->getNodeName() == D3MF::XmlTag::triangle)
             {
                 faces.push_back(ReadTriangle());
             }
         }

        mesh->mNumFaces = static_cast<unsigned int>(faces.size());
        mesh->mFaces = new aiFace[mesh->mNumFaces];
        mesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;

        std::copy(faces.begin(), faces.end(), mesh->mFaces);
    }

    aiFace ReadTriangle()
    {
        aiFace face;

        face.mNumIndices = 3;
        face.mIndices = new unsigned int[face.mNumIndices];
        face.mIndices[0] = static_cast<unsigned int>(std::atoi(xmlReader->getAttributeValue(D3MF::XmlTag::v1.c_str())));
        face.mIndices[1] = static_cast<unsigned int>(std::atoi(xmlReader->getAttributeValue(D3MF::XmlTag::v2.c_str())));
        face.mIndices[2] = static_cast<unsigned int>(std::atoi(xmlReader->getAttributeValue(D3MF::XmlTag::v3.c_str())));

        return face;
    }

private:
    bool ReadToStartElement(const std::string& startTag)
    {
        while(xmlReader->read())
        {
            if (xmlReader->getNodeType() == irr::io::EXN_ELEMENT && xmlReader->getNodeName() == startTag)
            {
                return true;
            }
            else if (xmlReader->getNodeType() == irr::io::EXN_ELEMENT_END &&
                     xmlReader->getNodeName() == startTag)
            {
                return false;
            }
        }
        //DefaultLogger::get()->error("unexpected EOF, expected closing <" + closeTag + "> tag");
        return false;
    }

    bool ReadToEndElement(const std::string& closeTag)
    {
        while(xmlReader->read())
        {
            if (xmlReader->getNodeType() == irr::io::EXN_ELEMENT) {
                return true;
            }
            else if (xmlReader->getNodeType() == irr::io::EXN_ELEMENT_END
                     && xmlReader->getNodeName() == closeTag)
            {
                return false;
            }
        }
        DefaultLogger::get()->error("unexpected EOF, expected closing <" + closeTag + "> tag");
        return false;
    }


private:
    std::vector<aiMesh*> meshes;
    XmlReader* xmlReader;
};

} //namespace D3MF

static const std::string Extension = "3mf";

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
    Extension.c_str()
};


D3MFImporter::D3MFImporter()
: BaseImporter() {
    // empty
}

D3MFImporter::~D3MFImporter() {
    // empty
}

bool D3MFImporter::CanRead(const std::string &pFile, IOSystem *pIOHandler, bool checkSig) const {
    const std::string extension = GetExtension(pFile);
    if(extension == Extension ) {
        return true;
    } else if ( !extension.length() || checkSig ) {
        if (nullptr == pIOHandler ) {
            return true;
        }
    }

    return false;
}

void D3MFImporter::SetupProperties(const Importer * /*pImp*/) {
    // empty
}

const aiImporterDesc *D3MFImporter::GetInfo() const {
    return &desc;
}

void D3MFImporter::InternReadFile(const std::string &pFile, aiScene *pScene, IOSystem *pIOHandler) {
    D3MF::D3MFOpcPackage opcPackage(pIOHandler, pFile);

    std::unique_ptr<CIrrXML_IOStreamReader> xmlStream(new CIrrXML_IOStreamReader(opcPackage.RootStream()));
    std::unique_ptr<D3MF::XmlReader> xmlReader(irr::io::createIrrXMLReader(xmlStream.get()));

    D3MF::XmlSerializer xmlSerializer(xmlReader.get());

    xmlSerializer.ImportXml(pScene);
}

} // Namespace Assimp

#endif // ASSIMP_BUILD_NO_3MF_IMPORTER
