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
#pragma once

namespace Assimp {
namespace D3MF {

namespace XmlTag {
    // Meta-data
    static const std::string meta = "metadata";
    static const std::string meta_name = "name";

    // Model-data specific tags
    static const std::string model = "model";
    static const std::string model_unit = "unit";
    static const std::string metadata = "metadata";
    static const std::string resources = "resources";
    static const std::string object = "object";
    static const std::string mesh = "mesh";
    static const std::string vertices = "vertices";
    static const std::string vertex = "vertex";
    static const std::string triangles = "triangles";
    static const std::string triangle = "triangle";
    static const std::string x = "x";
    static const std::string y = "y";
    static const std::string z = "z";
    static const std::string v1 = "v1";
    static const std::string v2 = "v2";
    static const std::string v3 = "v3";
    static const std::string id = "id";
    static const std::string pid = "pid";
    static const std::string p1 = "p1";
    static const std::string name = "name";
    static const std::string type = "type";
    static const std::string build = "build";
    static const std::string item = "item";
    static const std::string objectid = "objectid";
    static const std::string transform = "transform";

    // Material definitions
    static const std::string basematerials = "basematerials";
    static const std::string basematerials_id = "id";
    static const std::string basematerials_base = "base";
    static const std::string basematerials_name = "name";
    static const std::string basematerials_displaycolor = "displaycolor";

    // Meta info tags
    static const std::string CONTENT_TYPES_ARCHIVE = "[Content_Types].xml";
    static const std::string ROOT_RELATIONSHIPS_ARCHIVE = "_rels/.rels";
    static const std::string SCHEMA_CONTENTTYPES = "http://schemas.openxmlformats.org/package/2006/content-types";
    static const std::string SCHEMA_RELATIONSHIPS = "http://schemas.openxmlformats.org/package/2006/relationships";
    static const std::string RELS_RELATIONSHIP_CONTAINER = "Relationships";
    static const std::string RELS_RELATIONSHIP_NODE = "Relationship";
    static const std::string RELS_ATTRIB_TARGET = "Target";
    static const std::string RELS_ATTRIB_TYPE = "Type";
    static const std::string RELS_ATTRIB_ID = "Id";
    static const std::string PACKAGE_START_PART_RELATIONSHIP_TYPE = "http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel";
    static const std::string PACKAGE_PRINT_TICKET_RELATIONSHIP_TYPE = "http://schemas.microsoft.com/3dmanufacturing/2013/01/printticket";
    static const std::string PACKAGE_TEXTURE_RELATIONSHIP_TYPE = "http://schemas.microsoft.com/3dmanufacturing/2013/01/3dtexture";
    static const std::string PACKAGE_CORE_PROPERTIES_RELATIONSHIP_TYPE = "http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties";
    static const std::string PACKAGE_THUMBNAIL_RELATIONSHIP_TYPE = "http://schemas.openxmlformats.org/package/2006/relationships/metadata/thumbnail";
}

} // Namespace D3MF
} // Namespace Assimp
