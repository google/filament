#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    __dojostruct VzActor : VzSceneComp
    {
        void SetVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits);

        void SetRenderableRes(const VID vidGeo, const std::vector<VID>& vidMIs);
        void SetMI(const VID vidMI, const int slot = 0);

        std::vector<VID> GetMIs();
        VID GetMI(const int slot = 0);
        VID GetMaterial(const int slot = 0);
        VID GetGeometry();
    };
}
