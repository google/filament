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

/** @file  IFCMaterial.cpp
 *  @brief Implementation of conversion routines to convert IFC materials to aiMaterial
 */

#ifndef ASSIMP_BUILD_NO_IFC_IMPORTER

#include "IFCUtil.h"
#include <limits>
#include <assimp/material.h>

namespace Assimp {
namespace IFC {

// ------------------------------------------------------------------------------------------------
static int ConvertShadingMode(const std::string& name) {
    if (name == "BLINN") {
        return aiShadingMode_Blinn;
    }
    else if (name == "FLAT" || name == "NOTDEFINED") {
        return aiShadingMode_NoShading;
    }
    else if (name == "PHONG") {
        return aiShadingMode_Phong;
    }
    IFCImporter::LogWarn("shading mode "+name+" not recognized by Assimp, using Phong instead");
    return aiShadingMode_Phong;
}

// ------------------------------------------------------------------------------------------------
static void FillMaterial(aiMaterial* mat,const IFC::Schema_2x3::IfcSurfaceStyle* surf,ConversionData& conv) {
    aiString name;
    name.Set((surf->Name? surf->Name.Get() : "IfcSurfaceStyle_Unnamed"));
    mat->AddProperty(&name,AI_MATKEY_NAME);

    // now see which kinds of surface information are present
    for(std::shared_ptr< const IFC::Schema_2x3::IfcSurfaceStyleElementSelect > sel2 : surf->Styles) {
        if (const IFC::Schema_2x3::IfcSurfaceStyleShading* shade = sel2->ResolveSelectPtr<IFC::Schema_2x3::IfcSurfaceStyleShading>(conv.db)) {
            aiColor4D col_base,col;

            ConvertColor(col_base, shade->SurfaceColour);
            mat->AddProperty(&col_base,1, AI_MATKEY_COLOR_DIFFUSE);

            if (const IFC::Schema_2x3::IfcSurfaceStyleRendering* ren = shade->ToPtr<IFC::Schema_2x3::IfcSurfaceStyleRendering>()) {

                if (ren->Transparency) {
                    const float t = 1.f-static_cast<float>(ren->Transparency.Get());
                    mat->AddProperty(&t,1, AI_MATKEY_OPACITY);
                }

                if (ren->DiffuseColour) {
                    ConvertColor(col, *ren->DiffuseColour.Get(),conv,&col_base);
                    mat->AddProperty(&col,1, AI_MATKEY_COLOR_DIFFUSE);
                }

                if (ren->SpecularColour) {
                    ConvertColor(col, *ren->SpecularColour.Get(),conv,&col_base);
                    mat->AddProperty(&col,1, AI_MATKEY_COLOR_SPECULAR);
                }

                if (ren->TransmissionColour) {
                    ConvertColor(col, *ren->TransmissionColour.Get(),conv,&col_base);
                    mat->AddProperty(&col,1, AI_MATKEY_COLOR_TRANSPARENT);
                }

                if (ren->ReflectionColour) {
                    ConvertColor(col, *ren->ReflectionColour.Get(),conv,&col_base);
                    mat->AddProperty(&col,1, AI_MATKEY_COLOR_REFLECTIVE);
                }

                const int shading = (ren->SpecularHighlight && ren->SpecularColour)?ConvertShadingMode(ren->ReflectanceMethod):static_cast<int>(aiShadingMode_Gouraud);
                mat->AddProperty(&shading,1, AI_MATKEY_SHADING_MODEL);

                if (ren->SpecularHighlight) {
                    if(const ::Assimp::STEP::EXPRESS::REAL* rt = ren->SpecularHighlight.Get()->ToPtr<::Assimp::STEP::EXPRESS::REAL>()) {
                        // at this point we don't distinguish between the two distinct ways of
                        // specifying highlight intensities. leave this to the user.
                        const float e = static_cast<float>(*rt);
                        mat->AddProperty(&e,1,AI_MATKEY_SHININESS);
                    }
                    else {
                        IFCImporter::LogWarn("unexpected type error, SpecularHighlight should be a REAL");
                    }
                }
            }
        } 
    }
}

// ------------------------------------------------------------------------------------------------
unsigned int ProcessMaterials(uint64_t id, unsigned int prevMatId, ConversionData& conv, bool forceDefaultMat) {
    STEP::DB::RefMapRange range = conv.db.GetRefs().equal_range(id);
    for(;range.first != range.second; ++range.first) {
        if(const IFC::Schema_2x3::IfcStyledItem* const styled = conv.db.GetObject((*range.first).second)->ToPtr<IFC::Schema_2x3::IfcStyledItem>()) {
            for(const IFC::Schema_2x3::IfcPresentationStyleAssignment& as : styled->Styles) {
                for(std::shared_ptr<const IFC::Schema_2x3::IfcPresentationStyleSelect> sel : as.Styles) {

                    if( const IFC::Schema_2x3::IfcSurfaceStyle* const surf = sel->ResolveSelectPtr<IFC::Schema_2x3::IfcSurfaceStyle>(conv.db) ) {
                        // try to satisfy from cache
                        ConversionData::MaterialCache::iterator mit = conv.cached_materials.find(surf);
                        if( mit != conv.cached_materials.end() )
                            return mit->second;

                        // not found, create new material
                        const std::string side = static_cast<std::string>(surf->Side);
                        if( side != "BOTH" ) {
                            IFCImporter::LogWarn("ignoring surface side marker on IFC::IfcSurfaceStyle: " + side);
                        }

                        std::unique_ptr<aiMaterial> mat(new aiMaterial());

                        FillMaterial(mat.get(), surf, conv);

                        conv.materials.push_back(mat.release());
                        unsigned int matindex = static_cast<unsigned int>(conv.materials.size() - 1);
                        conv.cached_materials[surf] = matindex;
                        return matindex;
                    }
                }
            }
        }
    }

    // no local material defined. If there's global one, use that instead
    if ( prevMatId != std::numeric_limits<uint32_t>::max() ) {
        return prevMatId;
    }

    // we're still here - create an default material if required, or simply fail otherwise
    if ( !forceDefaultMat ) {
        return std::numeric_limits<uint32_t>::max();
    }

    aiString name;
    name.Set("<IFCDefault>");
    //  ConvertColorToString( color, name);

    // look if there's already a default material with this base color
    for( size_t a = 0; a < conv.materials.size(); ++a ) {
        aiString mname;
        conv.materials[a]->Get(AI_MATKEY_NAME, mname);
        if ( name == mname ) {
            return ( unsigned int )a;
        }
    }

    // we're here, yet - no default material with suitable color available. Generate one
    std::unique_ptr<aiMaterial> mat(new aiMaterial());
    mat->AddProperty(&name,AI_MATKEY_NAME);

    const aiColor4D col = aiColor4D( 0.6f, 0.6f, 0.6f, 1.0f); // aiColor4D( color.r, color.g, color.b, 1.0f);
    mat->AddProperty(&col,1, AI_MATKEY_COLOR_DIFFUSE);

    conv.materials.push_back(mat.release());
    return (unsigned int) conv.materials.size() - 1;
}

} // ! IFC
} // ! Assimp

#endif // ASSIMP_BUILD_NO_IFC_IMPORTER
