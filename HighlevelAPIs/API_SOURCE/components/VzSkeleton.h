#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    __dojostruct VzSkeleton : VzBaseComp
    {
        using BoneVID = VID;
        // componentVID refers to the root bone
        std::vector<BoneVID> GetBones(); // including this
        std::vector<BoneVID> GetChildren();
        VID GetParent();

        void SetTransformTRS(const BoneVID vidBone, const float t[3], const float r[4], const float s[3]);
        void UpdateBoneMatrices();

        // future work... skinning...
    };
}
