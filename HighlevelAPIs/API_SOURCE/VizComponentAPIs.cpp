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
    void VzSceneComp::setQuaternionFromEuler()
    {
        const float x = rotation_[0];
        const float y = rotation_[1];
        const float z = rotation_[2];
        const float c1 = cos(x / 2);
        const float c2 = cos(y / 2);
        const float c3 = cos(z / 2);
        const float s1 = sin(x / 2);
        const float s2 = sin(y / 2);
        const float s3 = sin(z / 2);
        switch (order_) {
            case EULER_ORDER::XYZ:
                quaternion_[0] = s1 * c2 * c3 + c1 * s2 * s3;
                quaternion_[1] = c1 * s2 * c3 - s1 * c2 * s3;
                quaternion_[2] = c1 * c2 * s3 + s1 * s2 * c3;
                quaternion_[3] = c1 * c2 * c3 - s1 * s2 * s3;
                break;
            case EULER_ORDER::YXZ:
                quaternion_[0] = s1 * c2 * c3 + c1 * s2 * s3;
                quaternion_[1] = c1 * s2 * c3 - s1 * c2 * s3;
                quaternion_[2] = c1 * c2 * s3 - s1 * s2 * c3;
                quaternion_[3] = c1 * c2 * c3 + s1 * s2 * s3;
                break;
            case EULER_ORDER::ZXY:
                quaternion_[0] = s1 * c2 * c3 - c1 * s2 * s3;
                quaternion_[1] = c1 * s2 * c3 + s1 * c2 * s3;
                quaternion_[2] = c1 * c2 * s3 + s1 * s2 * c3;
                quaternion_[3] = c1 * c2 * c3 - s1 * s2 * s3;
                break;
            case EULER_ORDER::ZYX:
                quaternion_[0] = s1 * c2 * c3 - c1 * s2 * s3;
                quaternion_[1] = c1 * s2 * c3 + s1 * c2 * s3;
                quaternion_[2] = c1 * c2 * s3 - s1 * s2 * c3;
                quaternion_[3] = c1 * c2 * c3 + s1 * s2 * s3;
                break;
            case EULER_ORDER::YZX:
                quaternion_[0] = s1 * c2 * c3 + c1 * s2 * s3;
                quaternion_[1] = c1 * s2 * c3 + s1 * c2 * s3;
                quaternion_[2] = c1 * c2 * s3 - s1 * s2 * c3;
                quaternion_[3] = c1 * c2 * c3 - s1 * s2 * s3;
                break;
            case EULER_ORDER::XZY:
                quaternion_[0] = s1 * c2 * c3 - c1 * s2 * s3;
                quaternion_[1] = c1 * s2 * c3 - s1 * c2 * s3;
                quaternion_[2] = c1 * c2 * s3 + s1 * s2 * c3;
                quaternion_[3] = c1 * c2 * c3 + s1 * s2 * s3;
                break;
        }
    }
    void VzSceneComp::setEulerFromQuaternion()
    {
        mat4f m(*(quatf*) quaternion_);
        const float* te = m.asArray();
        const float m11 = te[0], m12 = te[4], m13 = te[8];
        const float m21 = te[1], m22 = te[5], m23 = te[9];
        const float m31 = te[2], m32 = te[6], m33 = te[10];
        switch (order_) {
            case EULER_ORDER::XYZ:
                rotation_[1] = std::asin(std::clamp(m13, -1.0f, 1.0f));
                if (std::abs(m13) < 0.9999999f) {
                    rotation_[0] = std::atan2(-m23, m33);
                    rotation_[2] = std::atan2(-m12, m11);
                } else {
                    rotation_[0] = std::atan2(m32, m22);
                    rotation_[2] = 0.0f;
                }
                break;
            case EULER_ORDER::YXZ:
                rotation_[0] = std::asin(-std::clamp(m23, -1.0f, 1.0f));
                if (std::abs(m23) < 0.9999999f) {
                    rotation_[1] = std::atan2(m13, m33);
                    rotation_[2] = std::atan2(m21, m22);
                } else {
                    rotation_[1] = std::atan2(-m31, m11);
                    rotation_[2] = 0.0f;
                }
                break;
            case EULER_ORDER::ZXY:
                rotation_[0] = std::asin(std::clamp(m32, -1.0f, 1.0f));
                if (std::abs(m32) < 0.9999999f) {
                    rotation_[1] = std::atan2(-m31, m33);
                    rotation_[2] = std::atan2(-m12, m22);
                } else {
                    rotation_[1] = 0.0f;
                    rotation_[2] = std::atan2(m21, m11);
                }
                break;
            case EULER_ORDER::ZYX:
                rotation_[1] = std::asin(-std::clamp(m31, -1.0f, 1.0f));
                if (std::abs(m31) < 0.9999999f) {
                    rotation_[0] = std::atan2(m32, m33);
                    rotation_[2] = std::atan2(m21, m11);
                } else {
                    rotation_[0] = 0.0f;
                    rotation_[2] = std::atan2(-m12, m22);
                }
                break;
            case EULER_ORDER::YZX:
                rotation_[2] = std::asin(std::clamp(m21, -1.0f, 1.0f));
                if (std::abs(m21) < 0.9999999f) {
                    rotation_[0] = std::atan2(-m23, m22);
                    rotation_[1] = std::atan2(-m31, m11);
                } else {
                    rotation_[0] = 0.0f;
                    rotation_[1] = std::atan2(m13, m33);
                }
                break;
            case EULER_ORDER::XZY:
                rotation_[2] = std::asin(-std::clamp(m12, -1.0f, 1.0f));
                if (std::abs(m12) < 0.9999999f) {
                    rotation_[0] = std::atan2(m32, m22);
                    rotation_[1] = std::atan2(m13, m11);
                } else {
                    rotation_[0] = std::atan2(-m23, m33);
                    rotation_[1] = 0.0f;
                }
                break;
        }
    }
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
        mat4f localTransform = composeMatrix(*(float3*) t, *(quatf*) q, *(float3*) s);
        tc.setTransform(ins, additiveTransform ? localTransform * tc.getTransform(ins) : localTransform);
        UpdateTimeStamp();
    }
    void VzSceneComp::SetMatrix(const float value[16], const bool additiveTransform, const bool rowMajor)
    {
        COMP_TRANSFORM(tc, ett, ins, );
        mat4f mat = rowMajor ? transpose(*(mat4f*)value) : *(mat4f*)value;
        tc.setTransform(ins, additiveTransform ? mat * tc.getTransform(ins) : mat);
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
    void VzSceneComp::GetPosition(float position[3]) const
    {
        position[0] = position_[0];
        position[1] = position_[1];
        position[2] = position_[2];
    }
    void VzSceneComp::GetRotation(float rotation[3], EULER_ORDER* order) const
    {
        rotation[0] = rotation_[0];
        rotation[1] = rotation_[1];
        rotation[2] = rotation_[2];
        if (order != nullptr)
        {
            *order = order_;
        }
    }
    void VzSceneComp::GetQuaternion(float quaternion[4]) const
    {
        quaternion[0] = quaternion_[0];
        quaternion[1] = quaternion_[1];
        quaternion[2] = quaternion_[2];
        quaternion[3] = quaternion_[3];
    }
    void VzSceneComp::GetScale(float scale[3]) const
    {
        scale[0] = scale_[0];
        scale[1] = scale_[1];
        scale[2] = scale_[2];
    }
    void VzSceneComp::SetPosition(const float position[3])
    {
        position_[0] = position[0];
        position_[1] = position[1];
        position_[2] = position[2];
        UpdateTimeStamp();
    }
    void VzSceneComp::SetRotation(const float rotation[3], const EULER_ORDER order)
    {
        rotation_[0] = rotation[0];
        rotation_[1] = rotation[1];
        rotation_[2] = rotation[2];
        order_ = order;
        setQuaternionFromEuler();
        UpdateTimeStamp();
    }
    void VzSceneComp::SetQuaternion(const float quaternion[4])
    {
        quaternion_[0] = quaternion[0];
        quaternion_[1] = quaternion[1];
        quaternion_[2] = quaternion[2];
        quaternion_[3] = quaternion[3];
        setEulerFromQuaternion();
        UpdateTimeStamp();
    }
    void VzSceneComp::SetScale(const float scale[3])
    {
        scale_[0] = scale[0];
        scale_[1] = scale[1];
        scale_[2] = scale[2];
        UpdateTimeStamp();
    }
    bool VzSceneComp::IsMatrixAutoUpdate() const
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
        tc.setTransform(ins, composeMatrix(*(float3*) position_, *(quatf*) quaternion_, *(float3*) scale_));
        UpdateTimeStamp();
    }
#pragma endregion 
}
