#include "VizComponentAPIs.h"
#include "VzEngineApp.h"
#include "VzNameComponents.hpp"
#include "FIncludes.h"

extern Engine* gEngine;
extern vzm::VzEngineApp gEngineApp;

namespace vzm
{
#pragma region // VzBaseComp
    std::string VzBaseComp::GetName()
    {
        COMP_NAME(ncm, ett, "");
        return ncm.GetName(ett);
    }
    void VzBaseComp::SetName(const std::string& name)
    {
        COMP_NAME(ncm, ett, );
        ncm.SetName(ett, name);
        UpdateTimeStamp();
    }
#pragma endregion

#pragma region // VzSceneComp
    void VzSceneComp::GetWorldPosition(float v[3])
    {
        COMP_TRANSFORM(tc, ett, ins, );
        const math::mat4f& mat = tc.getWorldTransform(ins);
        *(float3*)v = mat[3].xyz;
    }
    void VzSceneComp::GetWorldForward(float v[3])
    {
        COMP_TRANSFORM(tc, ett, ins, );
        const math::mat4f& mat = tc.getWorldTransform(ins);
        *(float3*)v = mat[2].xyz; // view
    }
    void VzSceneComp::GetWorldRight(float v[3])
    {
        COMP_TRANSFORM(tc, ett, ins, );
        const math::mat4f& mat = tc.getWorldTransform(ins);
        *(float3*)v = mat[0].xyz;
    }
    void VzSceneComp::GetWorldUp(float v[3])
    {
        COMP_TRANSFORM(tc, ett, ins, );
        const math::mat4f& mat = tc.getWorldTransform(ins);
        *(float3*)v = mat[1].xyz;
    }
    void VzSceneComp::GetWorldTransform(float mat[16], const bool rowMajor)
    {
        // note that
        // filament math stores the column major matrix
        // logically it also uses column major matrix computation
        // c.f., glm:: uses column major matrix computation but it stores a matrix according to column major convention
        COMP_TRANSFORM(tc, ett, ins, );
        const math::mat4f& _mat = tc.getWorldTransform(ins);
        *(mat4f*)mat = _mat;
    }
    void VzSceneComp::GetLocalTransform(float mat[16], const bool rowMajor)
    {
        COMP_TRANSFORM(tc, ett, ins, );
        const math::mat4f& _mat = tc.getTransform(ins);
        *(mat4f*)mat = _mat;
    }
    void VzSceneComp::GetWorldInvTransform(float mat[16], const bool rowMajor)
    {
        COMP_TRANSFORM(tc, ett, ins, );
        const math::mat4f& _mat = tc.getWorldTransform(ins);
        *(mat4f*)mat = inverse(_mat);
    }
    void VzSceneComp::GetLocalInvTransform(float mat[16], const bool rowMajor)
    {
        COMP_TRANSFORM(tc, ett, ins, );
        const math::mat4f& _mat = tc.getTransform(ins);
        *(mat4f*)mat = inverse(_mat);
    }
    void VzSceneComp::SetTransform(const float s[3], const float q[4], const float t[3], const bool additiveTransform)
    {
        COMP_TRANSFORM(tc, ett, ins, );
        auto translation = float3(t[0], t[1], t[2]);
        auto rotation = quatf(q[3], q[0], q[1], q[2]);
        auto scale = float3(s[0], s[1], s[2]);
        tc.setTransform(ins, composeMatrix(translation, rotation, scale));
        UpdateTimeStamp();
    }
    void VzSceneComp::SetMatrix(const float value[16], const bool additiveTransform, const bool rowMajor)
    {
        COMP_TRANSFORM(tc, ett, ins, );
        mat4f mat = rowMajor ? transpose(*(mat4f*)value) : *(mat4f*)value;
        tc.setTransform(ins, additiveTransform ? mat * tc.getTransform(ins) : mat);
        UpdateTimeStamp();
    }
    void VzSceneComp::GetPosition(float v[3])
    {
        v[0] = position_[0];
        v[1] = position_[1];
        v[2] = position_[2];
    }
    void VzSceneComp::GetQuaternion(float v[4])
    {
        v[0] = quaternion_[0];
        v[1] = quaternion_[1];
        v[2] = quaternion_[2];
        v[3] = quaternion_[3];
    }
    void VzSceneComp::GetScale(float v[3])
    {
        v[0] = scale_[0];
        v[1] = scale_[1];
        v[2] = scale_[2];
    }
    void VzSceneComp::SetPosition(const float v[3])
    {
        position_[0] = v[0];
        position_[1] = v[1];
        position_[2] = v[2];
        UpdateTimeStamp();
    }
    void VzSceneComp::SetQuaternion(const float v[4])
    {
        quaternion_[0] = v[0];
        quaternion_[1] = v[1];
        quaternion_[2] = v[2];
        quaternion_[3] = v[3];
        UpdateTimeStamp();
    }
    void VzSceneComp::SetScale(const float v[3])
    {
        scale_[0] = v[0];
        scale_[1] = v[1];
        scale_[2] = v[2];
        UpdateTimeStamp();
    }
    bool VzSceneComp::GetMatrixAutoUpdate()
    {
        return matrixAutoUpdate_;
    }
    void VzSceneComp::SetMatrixAutoUpdate(const bool matrixAutoUpdate)
    {
        matrixAutoUpdate_ = matrixAutoUpdate;
        UpdateTimeStamp();
    }
    void VzSceneComp::UpdateMatrix()
    {
        COMP_TRANSFORM(tc, ett, ins, );
        auto translation = float3(position_[0], position_[1], position_[2]);
        auto rotation = quatf(quaternion_[3], quaternion_[0], quaternion_[1], quaternion_[2]);
        auto scale = float3(scale_[0], scale_[1], scale_[2]);
        tc.setTransform(ins, composeMatrix(translation, rotation, scale));
        UpdateTimeStamp();
    }
    VID VzSceneComp::GetParent()
    {
        COMP_TRANSFORM(tc, ett, ins, INVALID_VID);
        Entity ett_parent = tc.getParent(ins);
        return ett_parent.getId();
    }
    std::vector<VID> VzSceneComp::GetChildren()
    {
        std::vector<VID> children;
        COMP_TRANSFORM(tc, ett, ins, children);
        for (auto it = tc.getChildrenBegin(ins); it != tc.getChildrenEnd(ins); it++)
        {
            utils::Entity ett_child = tc.getEntity(*it);
            children.push_back(ett_child.getId());
        }
        return children;
    }
    VID VzSceneComp::GetScene()
    {
        return gEngineApp.GetSceneVidBelongTo(GetVID());
    }
#pragma endregion 
}
