/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2018, assimp team



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

/** @file  IRRShared.cpp
 *  @brief Shared utilities for the IRR and IRRMESH loaders
 */



//This section should be excluded only if both the Irrlicht AND the Irrlicht Mesh importers were omitted.
#if !(defined(ASSIMP_BUILD_NO_IRR_IMPORTER) && defined(ASSIMP_BUILD_NO_IRRMESH_IMPORTER))

#include "IRRShared.h"
#include <assimp/ParsingUtils.h>
#include <assimp/fast_atof.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/material.h>


using namespace Assimp;
using namespace irr;
using namespace irr::io;

// Transformation matrix to convert from Assimp to IRR space
const aiMatrix4x4 Assimp::AI_TO_IRR_MATRIX = aiMatrix4x4 (
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);

// ------------------------------------------------------------------------------------------------
// read a property in hexadecimal format (i.e. ffffffff)
void IrrlichtBase::ReadHexProperty    (HexProperty&    out)
{
    for (int i = 0; i < reader->getAttributeCount();++i)
    {
        if (!ASSIMP_stricmp(reader->getAttributeName(i),"name"))
        {
            out.name = std::string( reader->getAttributeValue(i) );
        }
        else if (!ASSIMP_stricmp(reader->getAttributeName(i),"value"))
        {
            // parse the hexadecimal value
            out.value = strtoul16(reader->getAttributeValue(i));
        }
    }
}

// ------------------------------------------------------------------------------------------------
// read a decimal property
void IrrlichtBase::ReadIntProperty    (IntProperty&    out)
{
    for (int i = 0; i < reader->getAttributeCount();++i)
    {
        if (!ASSIMP_stricmp(reader->getAttributeName(i),"name"))
        {
            out.name = std::string( reader->getAttributeValue(i) );
        }
        else if (!ASSIMP_stricmp(reader->getAttributeName(i),"value"))
        {
            // parse the ecimal value
            out.value = strtol10(reader->getAttributeValue(i));
        }
    }
}

// ------------------------------------------------------------------------------------------------
// read a string property
void IrrlichtBase::ReadStringProperty (StringProperty& out)
{
    for (int i = 0; i < reader->getAttributeCount();++i)
    {
        if (!ASSIMP_stricmp(reader->getAttributeName(i),"name"))
        {
            out.name = std::string( reader->getAttributeValue(i) );
        }
        else if (!ASSIMP_stricmp(reader->getAttributeName(i),"value"))
        {
            // simple copy the string
            out.value = std::string (reader->getAttributeValue(i));
        }
    }
}

// ------------------------------------------------------------------------------------------------
// read a boolean property
void IrrlichtBase::ReadBoolProperty   (BoolProperty&   out)
{
    for (int i = 0; i < reader->getAttributeCount();++i)
    {
        if (!ASSIMP_stricmp(reader->getAttributeName(i),"name"))
        {
            out.name = std::string( reader->getAttributeValue(i) );
        }
        else if (!ASSIMP_stricmp(reader->getAttributeName(i),"value"))
        {
            // true or false, case insensitive
            out.value = (ASSIMP_stricmp( reader->getAttributeValue(i),
                "true") ? false : true);
        }
    }
}

// ------------------------------------------------------------------------------------------------
// read a float property
void IrrlichtBase::ReadFloatProperty  (FloatProperty&  out)
{
    for (int i = 0; i < reader->getAttributeCount();++i)
    {
        if (!ASSIMP_stricmp(reader->getAttributeName(i),"name"))
        {
            out.name = std::string( reader->getAttributeValue(i) );
        }
        else if (!ASSIMP_stricmp(reader->getAttributeName(i),"value"))
        {
            // just parse the float
            out.value = fast_atof( reader->getAttributeValue(i) );
        }
    }
}

// ------------------------------------------------------------------------------------------------
// read a vector property
void IrrlichtBase::ReadVectorProperty  (VectorProperty&  out)
{
    for (int i = 0; i < reader->getAttributeCount();++i)
    {
        if (!ASSIMP_stricmp(reader->getAttributeName(i),"name"))
        {
            out.name = std::string( reader->getAttributeValue(i) );
        }
        else if (!ASSIMP_stricmp(reader->getAttributeName(i),"value"))
        {
            // three floats, separated with commas
            const char* ptr = reader->getAttributeValue(i);

            SkipSpaces(&ptr);
            ptr = fast_atoreal_move<float>( ptr,(float&)out.value.x );
            SkipSpaces(&ptr);
            if (',' != *ptr)
            {
                ASSIMP_LOG_ERROR("IRR(MESH): Expected comma in vector definition");
            }
            else SkipSpaces(ptr+1,&ptr);
            ptr = fast_atoreal_move<float>( ptr,(float&)out.value.y );
            SkipSpaces(&ptr);
            if (',' != *ptr)
            {
                ASSIMP_LOG_ERROR("IRR(MESH): Expected comma in vector definition");
            }
            else SkipSpaces(ptr+1,&ptr);
            ptr = fast_atoreal_move<float>( ptr,(float&)out.value.z );
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Convert a string to a proper aiMappingMode
int ConvertMappingMode(const std::string& mode)
{
    if (mode == "texture_clamp_repeat")
    {
        return aiTextureMapMode_Wrap;
    }
    else if (mode == "texture_clamp_mirror")
        return aiTextureMapMode_Mirror;

    return aiTextureMapMode_Clamp;
}

// ------------------------------------------------------------------------------------------------
// Parse a material from the XML file
aiMaterial* IrrlichtBase::ParseMaterial(unsigned int& matFlags)
{
    aiMaterial* mat = new aiMaterial();
    aiColor4D clr;
    aiString s;

    matFlags = 0; // zero output flags
    int cnt  = 0; // number of used texture channels
    unsigned int nd = 0;

    // Continue reading from the file
    while (reader->read())
    {
        switch (reader->getNodeType())
        {
        case EXN_ELEMENT:

            // Hex properties
            if (!ASSIMP_stricmp(reader->getNodeName(),"color"))
            {
                HexProperty prop;
                ReadHexProperty(prop);
                if (prop.name == "Diffuse")
                {
                    ColorFromARGBPacked(prop.value,clr);
                    mat->AddProperty(&clr,1,AI_MATKEY_COLOR_DIFFUSE);
                }
                else if (prop.name == "Ambient")
                {
                    ColorFromARGBPacked(prop.value,clr);
                    mat->AddProperty(&clr,1,AI_MATKEY_COLOR_AMBIENT);
                }
                else if (prop.name == "Specular")
                {
                    ColorFromARGBPacked(prop.value,clr);
                    mat->AddProperty(&clr,1,AI_MATKEY_COLOR_SPECULAR);
                }

                // NOTE: The 'emissive' property causes problems. It is
                // often != 0, even if there is obviously no light
                // emitted by the described surface. In fact I think
                // IRRLICHT ignores this property, too.
#if 0
                else if (prop.name == "Emissive")
                {
                    ColorFromARGBPacked(prop.value,clr);
                    mat->AddProperty(&clr,1,AI_MATKEY_COLOR_EMISSIVE);
                }
#endif
            }
            // Float properties
            else if (!ASSIMP_stricmp(reader->getNodeName(),"float"))
            {
                FloatProperty prop;
                ReadFloatProperty(prop);
                if (prop.name == "Shininess")
                {
                    mat->AddProperty(&prop.value,1,AI_MATKEY_SHININESS);
                }
            }
            // Bool properties
            else if (!ASSIMP_stricmp(reader->getNodeName(),"bool"))
            {
                BoolProperty prop;
                ReadBoolProperty(prop);
                if (prop.name == "Wireframe")
                {
                    int val = (prop.value ? true : false);
                    mat->AddProperty(&val,1,AI_MATKEY_ENABLE_WIREFRAME);
                }
                else if (prop.name == "GouraudShading")
                {
                    int val = (prop.value ? aiShadingMode_Gouraud
                        : aiShadingMode_NoShading);
                    mat->AddProperty(&val,1,AI_MATKEY_SHADING_MODEL);
                }
                else if (prop.name == "BackfaceCulling")
                {
                    int val = (!prop.value);
                    mat->AddProperty(&val,1,AI_MATKEY_TWOSIDED);
                }
            }
            // String properties - textures and texture related properties
            else if (!ASSIMP_stricmp(reader->getNodeName(),"texture") ||
                     !ASSIMP_stricmp(reader->getNodeName(),"enum"))
            {
                StringProperty prop;
                ReadStringProperty(prop);
                if (prop.value.length())
                {
                    // material type (shader)
                    if (prop.name == "Type")
                    {
                        if (prop.value == "solid")
                        {
                            // default material ...
                        }
                        else if (prop.value == "trans_vertex_alpha")
                        {
                            matFlags = AI_IRRMESH_MAT_trans_vertex_alpha;
                        }
                        else if (prop.value == "lightmap")
                        {
                            matFlags = AI_IRRMESH_MAT_lightmap;
                        }
                        else if (prop.value == "solid_2layer")
                        {
                            matFlags = AI_IRRMESH_MAT_solid_2layer;
                        }
                        else if (prop.value == "lightmap_m2")
                        {
                            matFlags = AI_IRRMESH_MAT_lightmap_m2;
                        }
                        else if (prop.value == "lightmap_m4")
                        {
                            matFlags = AI_IRRMESH_MAT_lightmap_m4;
                        }
                        else if (prop.value == "lightmap_light")
                        {
                            matFlags = AI_IRRMESH_MAT_lightmap_light;
                        }
                        else if (prop.value == "lightmap_light_m2")
                        {
                            matFlags = AI_IRRMESH_MAT_lightmap_light_m2;
                        }
                        else if (prop.value == "lightmap_light_m4")
                        {
                            matFlags = AI_IRRMESH_MAT_lightmap_light_m4;
                        }
                        else if (prop.value == "lightmap_add")
                        {
                            matFlags = AI_IRRMESH_MAT_lightmap_add;
                        }
                        // Normal and parallax maps are treated equally
                        else if (prop.value == "normalmap_solid" ||
                            prop.value == "parallaxmap_solid")
                        {
                            matFlags = AI_IRRMESH_MAT_normalmap_solid;
                        }
                        else if (prop.value == "normalmap_trans_vertex_alpha" ||
                            prop.value == "parallaxmap_trans_vertex_alpha")
                        {
                            matFlags = AI_IRRMESH_MAT_normalmap_tva;
                        }
                        else if (prop.value == "normalmap_trans_add" ||
                            prop.value == "parallaxmap_trans_add")
                        {
                            matFlags = AI_IRRMESH_MAT_normalmap_ta;
                        }
                        else {
                            ASSIMP_LOG_WARN("IRRMat: Unrecognized material type: " + prop.value);
                        }
                    }

                    // Up to 4 texture channels are supported
                    if (prop.name == "Texture1")
                    {
                        // Always accept the primary texture channel
                        ++cnt;
                        s.Set(prop.value);
                        mat->AddProperty(&s,AI_MATKEY_TEXTURE_DIFFUSE(0));
                    }
                    else if (prop.name == "Texture2" && cnt == 1)
                    {
                        // 2-layer material lightmapped?
                        if (matFlags & AI_IRRMESH_MAT_lightmap) {
                            ++cnt;
                            s.Set(prop.value);
                            mat->AddProperty(&s,AI_MATKEY_TEXTURE_LIGHTMAP(0));

                            // set the corresponding material flag
                            matFlags |= AI_IRRMESH_EXTRA_2ND_TEXTURE;
                        }
                        // alternatively: normal or parallax mapping
                        else if (matFlags & AI_IRRMESH_MAT_normalmap_solid) {
                            ++cnt;
                            s.Set(prop.value);
                            mat->AddProperty(&s,AI_MATKEY_TEXTURE_NORMALS(0));

                            // set the corresponding material flag
                            matFlags |= AI_IRRMESH_EXTRA_2ND_TEXTURE;
                        } else if (matFlags & AI_IRRMESH_MAT_solid_2layer)    {// or just as second diffuse texture
                            ++cnt;
                            s.Set(prop.value);
                            mat->AddProperty(&s,AI_MATKEY_TEXTURE_DIFFUSE(1));
                            ++nd;

                            // set the corresponding material flag
                            matFlags |= AI_IRRMESH_EXTRA_2ND_TEXTURE;
                        } else {
                            ASSIMP_LOG_WARN("IRRmat: Skipping second texture");
                        }
                    } else if (prop.name == "Texture3" && cnt == 2) {
                        // Irrlicht does not seem to use these channels.
                        ++cnt;
                        s.Set(prop.value);
                        mat->AddProperty(&s,AI_MATKEY_TEXTURE_DIFFUSE(nd+1));
                    } else if (prop.name == "Texture4" && cnt == 3) {
                        // Irrlicht does not seem to use these channels.
                        ++cnt;
                        s.Set(prop.value);
                        mat->AddProperty(&s,AI_MATKEY_TEXTURE_DIFFUSE(nd+2));
                    }

                    // Texture mapping options
                    if (prop.name == "TextureWrap1" && cnt >= 1)
                    {
                        int map = ConvertMappingMode(prop.value);
                        mat->AddProperty(&map,1,AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0));
                        mat->AddProperty(&map,1,AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0));
                    }
                    else if (prop.name == "TextureWrap2" && cnt >= 2)
                    {
                        int map = ConvertMappingMode(prop.value);
                        if (matFlags & AI_IRRMESH_MAT_lightmap) {
                            mat->AddProperty(&map,1,AI_MATKEY_MAPPINGMODE_U_LIGHTMAP(0));
                            mat->AddProperty(&map,1,AI_MATKEY_MAPPINGMODE_V_LIGHTMAP(0));
                        }
                        else if (matFlags & (AI_IRRMESH_MAT_normalmap_solid)) {
                            mat->AddProperty(&map,1,AI_MATKEY_MAPPINGMODE_U_NORMALS(0));
                            mat->AddProperty(&map,1,AI_MATKEY_MAPPINGMODE_V_NORMALS(0));
                        }
                        else if (matFlags & AI_IRRMESH_MAT_solid_2layer) {
                            mat->AddProperty(&map,1,AI_MATKEY_MAPPINGMODE_U_DIFFUSE(1));
                            mat->AddProperty(&map,1,AI_MATKEY_MAPPINGMODE_V_DIFFUSE(1));
                        }
                    }
                    else if (prop.name == "TextureWrap3" && cnt >= 3)
                    {
                        int map = ConvertMappingMode(prop.value);
                        mat->AddProperty(&map,1,AI_MATKEY_MAPPINGMODE_U_DIFFUSE(nd+1));
                        mat->AddProperty(&map,1,AI_MATKEY_MAPPINGMODE_V_DIFFUSE(nd+1));
                    }
                    else if (prop.name == "TextureWrap4" && cnt >= 4)
                    {
                        int map = ConvertMappingMode(prop.value);
                        mat->AddProperty(&map,1,AI_MATKEY_MAPPINGMODE_U_DIFFUSE(nd+2));
                        mat->AddProperty(&map,1,AI_MATKEY_MAPPINGMODE_V_DIFFUSE(nd+2));
                    }
                }
            }
            break;
            case EXN_ELEMENT_END:

                /* Assume there are no further nested nodes in <material> elements
                 */
                if (/* IRRMESH */ !ASSIMP_stricmp(reader->getNodeName(),"material") ||
                    /* IRR     */ !ASSIMP_stricmp(reader->getNodeName(),"attributes"))
                {
                    // Now process lightmapping flags
                    // We should have at least one textur to do that ..
                    if (cnt && matFlags & AI_IRRMESH_MAT_lightmap)
                    {
                        float f = 1.f;
                        unsigned int unmasked = matFlags&~AI_IRRMESH_MAT_lightmap;

                        // Additive lightmap?
                        int op = (unmasked & AI_IRRMESH_MAT_lightmap_add
                            ? aiTextureOp_Add : aiTextureOp_Multiply);

                        // Handle Irrlicht's lightmapping scaling factor
                        if (unmasked & AI_IRRMESH_MAT_lightmap_m2 ||
                            unmasked & AI_IRRMESH_MAT_lightmap_light_m2)
                        {
                            f = 2.f;
                        }
                        else if (unmasked & AI_IRRMESH_MAT_lightmap_m4 ||
                            unmasked & AI_IRRMESH_MAT_lightmap_light_m4)
                        {
                            f = 4.f;
                        }
                        mat->AddProperty( &f, 1, AI_MATKEY_TEXBLEND_LIGHTMAP(0));
                        mat->AddProperty( &op,1, AI_MATKEY_TEXOP_LIGHTMAP(0));
                    }

                    return mat;
                }
            default:

                // GCC complains here ...
                break;
        }
    }
    ASSIMP_LOG_ERROR("IRRMESH: Unexpected end of file. Material is not complete");

    return mat;
}

#endif // !(defined(ASSIMP_BUILD_NO_IRR_IMPORTER) && defined(ASSIMP_BUILD_NO_IRRMESH_IMPORTER))
