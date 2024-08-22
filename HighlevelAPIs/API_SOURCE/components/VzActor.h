#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    __dojostruct VzActor : VzSceneComp
    {
        VzActor(const VID vid, const std::string& originFrom)
            : VzSceneComp(vid, originFrom, "VzActor", SCENE_COMPONENT_TYPE::ACTOR) {}

        void SetVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits);

        void SetRenderableRes(const VID vidGeo, const std::vector<VID>& vidMIs);
        void SetMI(const VID vidMI, const int slot = 0);

        void SetCastShadows(const bool enabled);
        void SetReceiveShadows(const bool enabled);
        void SetScreenSpaceContactShadows(const bool enabled);

        std::vector<VID> GetMIs();
        VID GetMI(const int slot = 0);
        VID GetMaterial(const int slot = 0);
        VID GetGeometry();
    };

    __dojostruct VzSpriteActor : VzSceneComp
    {
        VzSpriteActor(const VID vid, const std::string & originFrom)
            : VzSceneComp(vid, originFrom, "VzSpriteActor", SCENE_COMPONENT_TYPE::SPRITE_ACTOR) {}

        void SetVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits);

        // use internal geometry (quad) and material (optimal shader for sprite rendering)
        // 
        // The sprite's anchor point, and the point around which the sprite rotates. 
        // A value of (0.5, 0.5) corresponds to the midpoint of the sprite. 
        // A value of (0, 0) corresponds to the lower left corner of the sprite. The default is (0.5, 0.5).
        // basic local frame is x:(1, 0, 0), y:(0, 1, 0), z:(0, 0, 1), sprite plane is defined on xy-plane
        void SetGeometry(const float w = 1.f, const float h = 1.f, const float anchorU = 0.5f, const float anchorV = 0.5f);
        void EnableBillboard(const bool billboardEnabled);
        //void SetAnchorPoint(const float u = 0.5f, const float v = 0.5f);
        //void SetSize(const float w = 1.f, const float h = 1.f);
        
        // material settings
        // 
        // The rotation of the sprite in degrees. Default is 0.
        void SetRotatation(const float rotDeg);
        void SetTexture(const VID vidTexture);
    };
}
