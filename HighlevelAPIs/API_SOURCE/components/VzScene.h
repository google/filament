#pragma once
#include "../VizComponentAPIs.h"

namespace vzm 
{
    __dojostruct VzScene : VzBaseComp
    {
        VzScene(const VID vid, const std::string& originFrom, const std::string& typeName)
            : VzBaseComp(vid, originFrom, typeName) {}
        
        std::vector<VID> GetSceneCompChildren();
        bool LoadIBL(const std::string& iblPath);
        void SetSkyboxVisibleLayerMask(const uint8_t layerBits = 0x7, const uint8_t maskBits = 0x4);
        void SetLightmapVisibleLayerMask(const uint8_t layerBits = 0x3, const uint8_t maskBits = 0x2); // check where to set
    };
}
