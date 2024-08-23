#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    struct API_EXPORT VzBaseActor : VzSceneComp
    {
        VzBaseActor(const VID vid, const std::string & originFrom, const std::string & typeName, const SCENE_COMPONENT_TYPE scenecompType)
            : VzSceneComp(vid, originFrom, typeName, scenecompType) {}

        void SetVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits);
    };

    struct API_EXPORT VzActor : VzBaseActor
    {
        VzActor(const VID vid, const std::string& originFrom)
            : VzBaseActor(vid, originFrom, "VzActor", SCENE_COMPONENT_TYPE::ACTOR) {}

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

    struct API_EXPORT VzBaseSprite
    {
    private:
        VzBaseActor* baseActor_;
    public:
        VzBaseSprite(VzBaseActor* baseActor) : baseActor_(baseActor) {};

        void EnableBillboard(const bool billboardEnabled);

        // The rotation of the sprite in degrees. Default is 0.
        void SetRotatation(const float rotDeg);
    };

    struct API_EXPORT VzSpriteActor : VzBaseActor, VzBaseSprite
    {
        VzSpriteActor(const VID vid, const std::string & originFrom)
            : VzBaseActor(vid, originFrom, "VzSpriteActor", SCENE_COMPONENT_TYPE::SPRITE_ACTOR), VzBaseSprite(this) {}

        // The sprite's anchor point, and the point around which the sprite rotates. 
        // A value of (0.5, 0.5) corresponds to the midpoint of the sprite. 
        // A value of (0, 0) corresponds to the lower left corner of the sprite. The default is (0.5, 0.5).
        // basic local frame is x:(1, 0, 0), y:(0, 1, 0), z:(0, 0, 1), sprite plane is defined on xy-plane
        void SetGeometry(const float w = 1.f, const float h = 1.f, const float anchorU = 0.5f, const float anchorV = 0.5f);
        //void SetAnchorPoint(const float u = 0.5f, const float v = 0.5f);
        //void SetSize(const float w = 1.f, const float h = 1.f);
        void SetTexture(const VID vidTexture);
    };

    struct API_EXPORT VzTextSpriteActor : VzBaseActor, VzBaseSprite
    {
        VzTextSpriteActor(const VID vid, const std::string & originFrom)
            : VzBaseActor(vid, originFrom, "VzTextSpriteActor", SCENE_COMPONENT_TYPE::TEXT_SPRITE_ACTOR), VzBaseSprite(this) {}

        // The sprite's anchor point, and the point around which the sprite rotates. 
        // A value of (0.5, 0.5) corresponds to the midpoint of the sprite. 
        // A value of (0, 0) corresponds to the lower left corner of the sprite. The default is (0.5, 0.5).
        // basic local frame is x:(1, 0, 0), y:(0, 1, 0), z:(0, 0, 1), sprite plane is defined on xy-plane
        void SetText(const std::string& text, const float fontHeight, const float anchorU = 0.5f, const float anchorV = 0.5f);
    };
}
