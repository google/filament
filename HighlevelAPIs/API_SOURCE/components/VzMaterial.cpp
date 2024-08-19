#include "VzMaterial.h"
#include "../VzEngineApp.h"
#include "../FIncludes.h"

extern Engine* gEngine;
extern vzm::VzEngineApp gEngineApp;

namespace vzm
{
    void VzMaterial::SetMaterialType(const MaterialType type)
    {
        COMP_MAT(material, m_res, );


        //std::vector<Material::ParameterInfo> p(material->getParameterCount());
        //material->getParameters(&p[0], p.size());
        //material->getParameters(filament::Material::ParameterInfo);
        UpdateTimeStamp();
    }

    size_t VzMaterial::GetAllowedParameters(std::map<std::string, ParameterInfo>& paramters)
    {
        COMP_MAT(material, m_res, 0);

        for (auto& kv : m_res->allowedParamters)
        {
            ParameterInfo pi;

            pi.name = kv.first.c_str();
            pi.isSampler = kv.second.isSampler;
            pi.isSubpass = kv.second.isSubpass;
            pi.type = (vzm::UniformType)kv.second.type;
            pi.samplerType = (vzm::SamplerType)kv.second.samplerType;
            pi.subpassType = (vzm::SubpassType)kv.second.subpassType;
            pi.count = kv.second.count;
            pi.precision = (vzm::Precision)kv.second.precision;

            paramters[kv.first] = pi;
        }
        return m_res->allowedParamters.size();
    }

    //MaterialKey VzMaterial::GetVzmMaterialKey() { 
    //    COMP_MAT(material, m_res, );

    //    bool unlit = material->getShading() == Shading::UNLIT;
    //    
    //    MaterialKey matkey {
    //      .unlit = unlit,
    //      .hasBaseColorTexture = material->hasParameter("baseColorMap"),
    //      .hasNormalTexture = material->hasParameter("normalMap"),
    //      .hasOcclusionTexture = material->hasParameter("occlusionMap"),
    //      .hasEmissiveTexture = material->hasParameter("emissiveMap"),
    //      .hasClearCoatTexture = material->hasParameter("clearCoatMap"),
    //      .hasClearCoatRoughnessTexture = material->hasParameter("clearCoatRoughnessMap"),
    //      .hasClearCoatNormalTexture = material->hasParameter("clearCoatNormalMap"),
    //      .hasClearCoat = material->hasParameter("clearCoatFactor"),
    //      .hasTransmission = material->hasParameter("transmissionFactor"),
    //      .hasTextureTransforms = material->hasParameter("baseColorUvMatrix"),
    //      .hasTransmissionTexture = material->hasParameter("transmissionMap"),
    //      .hasSheenColorTexture = material->hasParameter("sheenColorMap"),
    //      .hasSheenRoughnessTexture = material->hasParameter("sheenRoughnessMap"),
    //      .hasVolumeThicknessTexture = material->hasParameter("volumeThicknessMap"),
    //      .hasSheen = material->hasParameter("sheenColorFactor"),          
    //      .hasIOR = material->hasParameter("ior"),
    //      .hasVolume = material->hasParameter("volumeThicknessFactor"),
    //      .hasSpecular =  material->hasParameter("specularColorFactor"),
    //      .hasSpecularTexture =  material->hasParameter("specularMap"),
    //      .hasSpecularColorTexture =  material->hasParameter("specularColorMap"),
    //    };
    //    
    //    if (material->hasParameter("metallicRoughnessMap")) {
    //      matkey.hasMetallicRoughnessTexture = true;
    //    }
    //    BlendingMode blending = material->getBlendingMode();
    //    if (blending == BlendingMode::OPAQUE) {
    //      matkey.alphaMode = AlphaMode::OPAQUE;
    //    } else if (blending == BlendingMode::MASKED) {
    //      matkey.alphaMode = AlphaMode::MASK;
    //    } else{
    //      matkey.alphaMode = AlphaMode::BLEND;
    //    }

    //    return matkey;
    //}

    vzm::VzMaterial::MaterialKey VzMaterial::GetVzmMaterialKey() {
      MaterialKey tempkey{};
      COMP_MAT(material, m_res, tempkey);

        bool unlit = material->getShading() == Shading::UNLIT;

        MaterialKey matkey{
            .unlit = unlit,
            .hasBaseColorTexture = material->hasParameter("baseColorMap"),
            .hasNormalTexture = material->hasParameter("normalMap"),
            .hasOcclusionTexture = material->hasParameter("occlusionMap"),
            .hasEmissiveTexture = material->hasParameter("emissiveMap"),
            .hasClearCoatTexture = material->hasParameter("clearCoatMap"),
            .hasClearCoatRoughnessTexture =
                material->hasParameter("clearCoatRoughnessMap"),
            .hasClearCoatNormalTexture =
                material->hasParameter("clearCoatNormalMap"),
            .hasClearCoat = material->hasParameter("clearCoatFactor"),
            .hasTransmission = material->hasParameter("transmissionFactor"),
            .hasTextureTransforms = material->hasParameter("baseColorUvMatrix"),
            .hasTransmissionTexture = material->hasParameter("transmissionMap"),
            .hasSheenColorTexture = material->hasParameter("sheenColorMap"),
            .hasSheenRoughnessTexture =
                material->hasParameter("sheenRoughnessMap"),
            .hasVolumeThicknessTexture =
                material->hasParameter("volumeThicknessMap"),
            .hasSheen = material->hasParameter("sheenColorFactor"),
            .hasIOR = material->hasParameter("ior"),
            .hasVolume = material->hasParameter("volumeThicknessFactor"),
            .hasSpecular = material->hasParameter("specularColorFactor"),
            .hasSpecularTexture = material->hasParameter("specularMap"),
            .hasSpecularColorTexture =
                material->hasParameter("specularColorMap"),
        };

        if (material->hasParameter("metallicRoughnessMap")) {
          matkey.hasMetallicRoughnessTexture = true;
        }
        BlendingMode blending = material->getBlendingMode();
        if (blending == BlendingMode::OPAQUE) {
          matkey.alphaMode = AlphaMode::OPAQUE;
        } else if (blending == BlendingMode::MASKED) {
          matkey.alphaMode = AlphaMode::MASK;
        } else {
          matkey.alphaMode = AlphaMode::BLEND;
        }

        return matkey;
    }
    

    }  // namespace vzm
