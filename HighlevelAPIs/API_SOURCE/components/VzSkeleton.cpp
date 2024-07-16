#include "VzSkeleton.h"
#include "../VzEngineApp.h"
#include "../FIncludes.h"

extern Engine* gEngine;
extern vzm::VzEngineApp gEngineApp;

namespace vzm
{
#define COMP_SKELETON(COMP, RESMAP, FAILRET)  auto it = RESMAP.find(componentVID); if (it == RESMAP.end()) return FAILRET; VzAssetRes* COMP = it->second.get();
    std::vector<VID> VzSkeleton::GetBones()
    {
        std::vector<VID> root_vids;
        VzSkeletonRes* skeleton_res = gEngineApp.GetSkeletonRes(componentVID);
        if (skeleton_res == nullptr) return root_vids;

        root_vids.reserve(skeleton_res->bones.size());
        for (auto& it : skeleton_res->bones)
        {
            root_vids.push_back(it.first);
        }
        return root_vids;
    }
    VID VzSkeleton::GetParent()
    {
        COMP_TRANSFORM(tc, ett, ins, INVALID_VID);
        Entity ett_parent = tc.getParent(ins);
        return ett_parent.getId();
    }
    std::vector<VID> VzSkeleton::GetChildren()
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
    void VzSkeleton::SetTransformTRS(const BoneVID vidBone, const float t[3], const float r[4], const float s[3])
    {
        assert(0 && "to do");
    }
    void VzSkeleton::UpdateBoneMatrices()
    {
        assert(0 && "to do");
    }
}
