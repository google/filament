/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2018, assimp team


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



#ifndef ASSIMP_BUILD_NO_OGRE_IMPORTER

#include "OgreImporter.h"
#include <assimp/TinyFormatter.h>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/fast_atof.h>

#include <vector>
#include <sstream>
#include <memory>

using namespace std;

namespace Assimp
{
namespace Ogre
{

static const string partComment    = "//";
static const string partBlockStart = "{";
static const string partBlockEnd   = "}";

void OgreImporter::ReadMaterials(const std::string &pFile, Assimp::IOSystem *pIOHandler, aiScene *pScene, Mesh *mesh)
{
    std::vector<aiMaterial*> materials;

    // Create materials that can be found and parsed via the IOSystem.
    for (size_t i=0, len=mesh->NumSubMeshes(); i<len; ++i)
    {
        SubMesh *submesh = mesh->GetSubMesh(i);
        if (submesh && !submesh->materialRef.empty())
        {
            aiMaterial *material = ReadMaterial(pFile, pIOHandler, submesh->materialRef);
            if (material)
            {
                submesh->materialIndex = static_cast<int>(materials.size());
                materials.push_back(material);
            }
        }
    }

    AssignMaterials(pScene, materials);
}

void OgreImporter::ReadMaterials(const std::string &pFile, Assimp::IOSystem *pIOHandler, aiScene *pScene, MeshXml *mesh)
{
    std::vector<aiMaterial*> materials;

    // Create materials that can be found and parsed via the IOSystem.
    for (size_t i=0, len=mesh->NumSubMeshes(); i<len; ++i)
    {
        SubMeshXml *submesh = mesh->GetSubMesh( static_cast<uint16_t>(i));
        if (submesh && !submesh->materialRef.empty())
        {
            aiMaterial *material = ReadMaterial(pFile, pIOHandler, submesh->materialRef);
            if (material)
            {
                submesh->materialIndex = static_cast<int>(materials.size());
                materials.push_back(material);
            }
        }
    }

    AssignMaterials(pScene, materials);
}

void OgreImporter::AssignMaterials(aiScene *pScene, std::vector<aiMaterial*> &materials)
{
    pScene->mNumMaterials = static_cast<unsigned int>(materials.size());
    if (pScene->mNumMaterials > 0)
    {
        pScene->mMaterials = new aiMaterial*[pScene->mNumMaterials];
        for(size_t i=0;i<pScene->mNumMaterials; ++i) {
            pScene->mMaterials[i] = materials[i];
        }
    }
}

aiMaterial* OgreImporter::ReadMaterial(const std::string &pFile, Assimp::IOSystem *pIOHandler, const std::string &materialName)
{
    if (materialName.empty()) {
        return 0;
    }

    // Full reference and examples of Ogre Material Script
    // can be found from http://www.ogre3d.org/docs/manual/manual_14.html

    /*and here is another one:

    import * from abstract_base_passes_depth.material
    import * from abstract_base.material
    import * from mat_shadow_caster.material
    import * from mat_character_singlepass.material

    material hero/hair/caster : mat_shadow_caster_skin_areject
    {
      set $diffuse_map "hero_hair_alpha_c.dds"
    }

    material hero/hair_alpha : mat_char_cns_singlepass_areject_4weights
    {
      set $diffuse_map  "hero_hair_alpha_c.dds"
      set $specular_map "hero_hair_alpha_s.dds"
      set $normal_map   "hero_hair_alpha_n.dds"
      set $light_map    "black_lightmap.dds"

      set $shadow_caster_material "hero/hair/caster"
    }
    */

    stringstream ss;

    // Scope for scopre_ptr auto release
    {
        /* There are three .material options in priority order:
            1) File with the material name (materialName)
            2) File with the mesh files base name (pFile)
            3) Optional user defined material library file (m_userDefinedMaterialLibFile) */
        std::vector<string> potentialFiles;
        potentialFiles.push_back(materialName + ".material");
        potentialFiles.push_back(pFile.substr(0, pFile.rfind(".mesh")) + ".material");
        if (!m_userDefinedMaterialLibFile.empty())
            potentialFiles.push_back(m_userDefinedMaterialLibFile);

        IOStream *materialFile = 0;
        for(size_t i=0; i<potentialFiles.size(); ++i)
        {
            materialFile = pIOHandler->Open(potentialFiles[i]);
            if (materialFile) {
                break;
            }
            ASSIMP_LOG_DEBUG_F( "Source file for material '", materialName, "' ", potentialFiles[i], " does not exist");
        }
        if (!materialFile)
        {
            ASSIMP_LOG_ERROR_F( "Failed to find source file for material '", materialName, "'");
            return 0;
        }

        std::unique_ptr<IOStream> stream(materialFile);
        if (stream->FileSize() == 0)
        {
            ASSIMP_LOG_WARN_F( "Source file for material '", materialName, "' is empty (size is 0 bytes)");
            return 0;
        }

        // Read bytes
        vector<char> data(stream->FileSize());
        stream->Read(&data[0], stream->FileSize(), 1);

        // Convert to UTF-8 and terminate the string for ss
        BaseImporter::ConvertToUTF8(data);
        data.push_back('\0');

        ss << &data[0];
    }

    ASSIMP_LOG_DEBUG_F("Reading material '", materialName, "'");

    aiMaterial *material = new aiMaterial();
    m_textures.clear();

    aiString ts(materialName);
    material->AddProperty(&ts, AI_MATKEY_NAME);

    // The stringstream will push words from a line until newline.
    // It will also trim whitespace from line start and between words.
    string linePart;
    ss >> linePart;

    const string partMaterial   = "material";
    const string partTechnique  = "technique";

    while(!ss.eof())
    {
        // Skip commented lines
        if (linePart == partComment)
        {
            NextAfterNewLine(ss, linePart);
            continue;
        }
        if (linePart != partMaterial)
        {
            ss >> linePart;
            continue;
        }

        ss >> linePart;
        if (linePart != materialName)
        {
            ss >> linePart;
            continue;
        }

        NextAfterNewLine(ss, linePart);
        if (linePart != partBlockStart)
        {
            ASSIMP_LOG_ERROR_F( "Invalid material: block start missing near index ", ss.tellg());
            return material;
        }

        ASSIMP_LOG_DEBUG_F("material '", materialName, "'");

        while(linePart != partBlockEnd)
        {
            // Proceed to the first technique
            ss >> linePart;

            if (linePart == partTechnique)
            {
                string techniqueName = SkipLine(ss);
                ReadTechnique(Trim(techniqueName), ss, material);
            }

            // Read information from a custom material
            /** @todo This "set $x y" does not seem to be a official Ogre material system feature.
                Materials can inherit other materials and override texture units by using the (unique)
                parent texture unit name in your cloned material.
                This is not yet supported and below code is probably some hack from the original
                author of this Ogre importer. Should be removed? */
            if (linePart=="set")
            {
                ss >> linePart;
                if (linePart=="$specular")//todo load this values:
                {
                }
                else if (linePart=="$diffuse")
                {
                }
                else if (linePart=="$ambient")
                {
                }
                else if (linePart=="$colormap")
                {
                    ss >> linePart;
                    aiString ts(linePart);
                    material->AddProperty(&ts, AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0));
                }
                else if (linePart=="$normalmap")
                {
                    ss >> linePart;
                    aiString ts(linePart);
                    material->AddProperty(&ts, AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0));
                }
                else if (linePart=="$shininess_strength")
                {
                    ss >> linePart;
                    float Shininess = fast_atof(linePart.c_str());
                    material->AddProperty(&Shininess, 1, AI_MATKEY_SHININESS_STRENGTH);
                }
                else if (linePart=="$shininess_exponent")
                {
                    ss >> linePart;
                    float Shininess = fast_atof(linePart.c_str());
                    material->AddProperty(&Shininess, 1, AI_MATKEY_SHININESS);
                }
                //Properties from Venetica:
                else if (linePart=="$diffuse_map")
                {
                    ss >> linePart;
                    if (linePart[0] == '"')// "file" -> file
                        linePart = linePart.substr(1, linePart.size()-2);
                    aiString ts(linePart);
                    material->AddProperty(&ts, AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0));
                }
                else if (linePart=="$specular_map")
                {
                    ss >> linePart;
                    if (linePart[0] == '"')// "file" -> file
                        linePart = linePart.substr(1, linePart.size()-2);
                    aiString ts(linePart);
                    material->AddProperty(&ts, AI_MATKEY_TEXTURE(aiTextureType_SHININESS, 0));
                }
                else if (linePart=="$normal_map")
                {
                    ss >> linePart;
                    if (linePart[0]=='"')// "file" -> file
                        linePart = linePart.substr(1, linePart.size()-2);
                    aiString ts(linePart);
                    material->AddProperty(&ts, AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0));
                }
                else if (linePart=="$light_map")
                {
                    ss >> linePart;
                    if (linePart[0]=='"') {
                        linePart = linePart.substr(1, linePart.size() - 2);
                    }
                    aiString ts(linePart);
                    material->AddProperty(&ts, AI_MATKEY_TEXTURE(aiTextureType_LIGHTMAP, 0));
                }
            }
        }
        ss >> linePart;
    }

    return material;
}

bool OgreImporter::ReadTechnique(const std::string &techniqueName, stringstream &ss, aiMaterial *material)
{
    string linePart;
    ss >> linePart;

    if (linePart != partBlockStart)
    {
        ASSIMP_LOG_ERROR_F( "Invalid material: Technique block start missing near index ", ss.tellg());
        return false;
    }

    ASSIMP_LOG_DEBUG_F(" technique '", techniqueName, "'");

    const string partPass  = "pass";

    while(linePart != partBlockEnd)
    {
        ss >> linePart;

        // Skip commented lines
        if (linePart == partComment)
        {
            SkipLine(ss);
            continue;
        }

        /// @todo Techniques have other attributes than just passes.
        if (linePart == partPass)
        {
            string passName = SkipLine(ss);
            ReadPass(Trim(passName), ss, material);
        }
    }
    return true;
}

bool OgreImporter::ReadPass(const std::string &passName, stringstream &ss, aiMaterial *material)
{
    string linePart;
    ss >> linePart;

    if (linePart != partBlockStart)
    {
        ASSIMP_LOG_ERROR_F( "Invalid material: Pass block start missing near index ", ss.tellg());
        return false;
    }

    ASSIMP_LOG_DEBUG_F("  pass '", passName, "'");

    const string partAmbient     = "ambient";
    const string partDiffuse     = "diffuse";
    const string partSpecular    = "specular";
    const string partEmissive    = "emissive";
    const string partTextureUnit = "texture_unit";

    while(linePart != partBlockEnd)
    {
        ss >> linePart;

        // Skip commented lines
        if (linePart == partComment)
        {
            SkipLine(ss);
            continue;
        }

        // Colors
        /// @todo Support alpha via aiColor4D.
        if (linePart == partAmbient || linePart == partDiffuse || linePart == partSpecular || linePart == partEmissive)
        {
            float r, g, b;
            ss >> r >> g >> b;
            const aiColor3D color(r, g, b);

            ASSIMP_LOG_DEBUG_F( "   ", linePart, " ", r, " ", g, " ", b);

            if (linePart == partAmbient)
            {
                material->AddProperty(&color, 1, AI_MATKEY_COLOR_AMBIENT);
            }
            else if (linePart == partDiffuse)
            {
                material->AddProperty(&color, 1, AI_MATKEY_COLOR_DIFFUSE);
            }
            else if (linePart == partSpecular)
            {
                material->AddProperty(&color, 1, AI_MATKEY_COLOR_SPECULAR);
            }
            else if (linePart == partEmissive)
            {
                material->AddProperty(&color, 1, AI_MATKEY_COLOR_EMISSIVE);
            }
        }
        else if (linePart == partTextureUnit)
        {
            string textureUnitName = SkipLine(ss);
            ReadTextureUnit(Trim(textureUnitName), ss, material);
        }
    }
    return true;
}

bool OgreImporter::ReadTextureUnit(const std::string &textureUnitName, stringstream &ss, aiMaterial *material)
{
    string linePart;
    ss >> linePart;

    if (linePart != partBlockStart)
    {
        ASSIMP_LOG_ERROR_F( "Invalid material: Texture unit block start missing near index ", ss.tellg());
        return false;
    }

    ASSIMP_LOG_DEBUG_F("   texture_unit '", textureUnitName, "'");

    const string partTexture      = "texture";
    const string partTextCoordSet = "tex_coord_set";
    const string partColorOp      = "colour_op";

    aiTextureType textureType = aiTextureType_NONE;
    std::string textureRef;
    int uvCoord = 0;

    while(linePart != partBlockEnd)
    {
        ss >> linePart;

        // Skip commented lines
        if (linePart == partComment)
        {
            SkipLine(ss);
            continue;
        }

        if (linePart == partTexture)
        {
            ss >> linePart;
            textureRef = linePart;

            // User defined Assimp config property to detect texture type from filename.
            if (m_detectTextureTypeFromFilename)
            {
                size_t posSuffix = textureRef.find_last_of(".");
                size_t posUnderscore = textureRef.find_last_of("_");

                if (posSuffix != string::npos && posUnderscore != string::npos && posSuffix > posUnderscore)
                {
                    string identifier = Ogre::ToLower(textureRef.substr(posUnderscore, posSuffix - posUnderscore));
                    ASSIMP_LOG_DEBUG_F( "Detecting texture type from filename postfix '", identifier, "'");

                    if (identifier == "_n" || identifier == "_nrm" || identifier == "_nrml" || identifier == "_normal" || identifier == "_normals" || identifier == "_normalmap")
                    {
                        textureType = aiTextureType_NORMALS;
                    }
                    else if (identifier == "_s" || identifier == "_spec" || identifier == "_specular" || identifier == "_specularmap")
                    {
                        textureType = aiTextureType_SPECULAR;
                    }
                    else if (identifier == "_l" || identifier == "_light" || identifier == "_lightmap" || identifier == "_occ" || identifier == "_occlusion")
                    {
                        textureType = aiTextureType_LIGHTMAP;
                    }
                    else if (identifier == "_disp" || identifier == "_displacement")
                    {
                        textureType = aiTextureType_DISPLACEMENT;
                    }
                    else
                    {
                        textureType = aiTextureType_DIFFUSE;
                    }
                }
                else
                {
                    textureType = aiTextureType_DIFFUSE;
                }
            }
            // Detect from texture unit name. This cannot be too broad as
            // authors might give names like "LightSaber" or "NormalNinja".
            else
            {
                string unitNameLower = Ogre::ToLower(textureUnitName);
                if (unitNameLower.find("normalmap") != string::npos)
                {
                    textureType = aiTextureType_NORMALS;
                }
                else if (unitNameLower.find("specularmap") != string::npos)
                {
                    textureType = aiTextureType_SPECULAR;
                }
                else if (unitNameLower.find("lightmap") != string::npos)
                {
                    textureType = aiTextureType_LIGHTMAP;
                }
                else if (unitNameLower.find("displacementmap") != string::npos)
                {
                    textureType = aiTextureType_DISPLACEMENT;
                }
                else
                {
                    textureType = aiTextureType_DIFFUSE;
                }
            }
        }
        else if (linePart == partTextCoordSet)
        {
            ss >> uvCoord;
        }
        /// @todo Implement
        else if(linePart == partColorOp)
        {
            /*
            ss >> linePart;
            if("replace"==linePart)//I don't think, assimp has something for this...
            {
            }
            else if("modulate"==linePart)
            {
                //TODO: set value
                //material->AddProperty(aiTextureOp_Multiply)
            }
            */
        }
    }

    if (textureRef.empty())
    {
        ASSIMP_LOG_WARN("Texture reference is empty, ignoring texture_unit.");
        return false;
    }
    if (textureType == aiTextureType_NONE)
    {
        ASSIMP_LOG_WARN("Failed to detect texture type for '" + textureRef  + "', ignoring texture_unit.");
        return false;
    }

    unsigned int textureTypeIndex = m_textures[textureType];
    m_textures[textureType]++;

    ASSIMP_LOG_DEBUG_F( "    texture '", textureRef, "' type ", textureType,
        " index ", textureTypeIndex, " UV ", uvCoord);

    aiString assimpTextureRef(textureRef);
    material->AddProperty(&assimpTextureRef, AI_MATKEY_TEXTURE(textureType, textureTypeIndex));
    material->AddProperty(&uvCoord, 1, AI_MATKEY_UVWSRC(textureType, textureTypeIndex));

    return true;
}

} // Ogre
} // Assimp

#endif // ASSIMP_BUILD_NO_OGRE_IMPORTER
