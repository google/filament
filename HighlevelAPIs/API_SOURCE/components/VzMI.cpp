#include "VzMI.h"
#include "../VzEngineApp.h"
#include "../FIncludes.h"

extern Engine* gEngine;
extern vzm::VzEngineApp gEngineApp;

extern std::vector<std::string> gMProp;

namespace vzm
{
    void VzMI::SetTransparencyMode(const TransparencyMode tMode)
    {
        COMP_MI(mi, mi_res, );
        mi->setTransparencyMode((filament::TransparencyMode)tMode);
        UpdateTimeStamp();
    }
#define SET_PARAM_COMP(COMP, RES, M_RES, FAILRET) COMP_MI(COMP, RES, FAILRET); VzMaterialRes* M_RES = gEngineApp.GetMaterialRes(RES->vidMaterial); if (!M_RES->allowedParamters.contains(name)) return FAILRET;
    bool VzMI::SetParameter(const std::string& name, const vzm::UniformType vType, const void* v)
    {
        SET_PARAM_COMP(mi, mi_res, m_res, false);

        const char* cstr = name.c_str();
        switch (vType)
        {
        case vzm::UniformType::BOOL: mi->setParameter(cstr, *(bool*)v); break;
        case vzm::UniformType::BOOL2: mi->setParameter(cstr, *(math::bool2*)v); break;
        case vzm::UniformType::BOOL3: mi->setParameter(cstr, *(math::bool3*)v); break;
        case vzm::UniformType::BOOL4: mi->setParameter(cstr, *(math::bool4*)v); break;
        case vzm::UniformType::FLOAT: mi->setParameter(cstr, *(float*)v); break;
        case vzm::UniformType::FLOAT2: mi->setParameter(cstr, *(math::float2*)v); break;
        case vzm::UniformType::FLOAT3: mi->setParameter(cstr, *(math::float3*)v); break;
        case vzm::UniformType::FLOAT4: mi->setParameter(cstr, *(math::float4*)v); break;
        case vzm::UniformType::INT: mi->setParameter(cstr, *(int*)v); break;
        case vzm::UniformType::INT2: mi->setParameter(cstr, *(math::int2*)v); break;
        case vzm::UniformType::INT3: mi->setParameter(cstr, *(math::int3*)v); break;
        case vzm::UniformType::INT4: mi->setParameter(cstr, *(math::int4*)v); break;
        case vzm::UniformType::UINT: mi->setParameter(cstr, *(uint*)v); break;
        case vzm::UniformType::UINT2: mi->setParameter(cstr, *(math::uint2*)v); break;
        case vzm::UniformType::UINT3: mi->setParameter(cstr, *(math::uint3*)v); break;
        case vzm::UniformType::UINT4: mi->setParameter(cstr, *(math::uint4*)v); break;
        case vzm::UniformType::MAT3: mi->setParameter(cstr, *(math::mat3f*)v); break;
        case vzm::UniformType::MAT4: mi->setParameter(cstr, *(math::mat4f*)v); break;
        case vzm::UniformType::STRUCT: 
        default:
            return false;
        }
        UpdateTimeStamp();
        return true;
    }

    bool VzMI::SetParameter(const std::string& name, const vzm::RgbType vType, const float* v)
    {
        SET_PARAM_COMP(mi, mi_res, m_res, false);
        mi->setParameter(name.c_str(), (filament::RgbType)vType, *(math::float3*)v);
        UpdateTimeStamp();
        return true;
    }

    bool VzMI::SetParameter(const std::string& name, const vzm::RgbaType vType, const float* v)
    {
        SET_PARAM_COMP(mi, mi_res, m_res, false);
        mi->setParameter(name.c_str(), (filament::RgbaType)vType, *(math::float4*)v);
        UpdateTimeStamp();
        return true;
    }

    VID VzMI::GetMaterial()
    {
        COMP_MI(mi, mi_res, INVALID_VID);
        return mi_res->vidMaterial;
    }
}
