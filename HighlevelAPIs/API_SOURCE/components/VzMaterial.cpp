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

    size_t VzMaterial::GetAllowedParameters(std::map<std::string, vzm::UniformType>& paramters)
    {
        COMP_MAT(material, m_res, 0);

        for (auto& kv : m_res->allowedParamters)
        {
            paramters[kv.first] = (vzm::UniformType)kv.second.type;
        }
        return m_res->allowedParamters.size();
    }
}