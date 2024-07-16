#include "VzActor.h"
#include "../VzEngineApp.h"
#include "../FIncludes.h"

extern Engine* gEngine;
extern vzm::VzEngineApp gEngineApp;

namespace vzm
{
    void VzActor::SetVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits)
    {
        COMP_ACTOR(rcm, ett, ins, );
        rcm.setLayerMask(ins, layerBits, maskBits);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    void VzActor::SetMI(const VID vidMI, const int slot)
    {
        VzMIRes* mi_res = gEngineApp.GetMIRes(vidMI);
        if (mi_res == nullptr)
        {
            backlog::post("invalid material instance!", backlog::LogLevel::Error);
            return;
        }
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        if (!actor_res->SetMI(vidMI, slot))
        {
            return;
        }
        auto& rcm = gEngine->getRenderableManager();
        utils::Entity ett_actor = utils::Entity::import(componentVID);
        auto ins = rcm.getInstance(ett_actor);
        rcm.setMaterialInstanceAt(ins, slot, mi_res->mi);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    void VzActor::SetRenderableRes(const VID vidGeo, const std::vector<VID>& vidMIs)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        actor_res->SetGeometry(vidGeo);
        actor_res->SetMIs(vidMIs);
        gEngineApp.BuildRenderable(componentVID);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    std::vector<VID> VzActor::GetMIs()
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        return actor_res->GetMIVids();
    }
    VID VzActor::GetMI(const int slot)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        return actor_res->GetMIVid(slot);
    }
    VID VzActor::GetMaterial(const int slot)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        MInstanceVID vid_mi = actor_res->GetMIVid(slot);
        VzMIRes* mi_res = gEngineApp.GetMIRes(vid_mi);
        if (mi_res == nullptr)
        {
            return INVALID_VID;
        }
        MaterialInstance* mi = mi_res->mi;
        assert(mi);
        const Material* mat = mi->getMaterial();
        assert(mat != nullptr);
        return gEngineApp.FindMaterialVID(mat);
    }
    VID VzActor::GetGeometry()
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        return actor_res->GetGeometryVid();
    }
}
