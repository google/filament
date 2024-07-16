#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    __dojostruct VzRenderer : VzBaseComp
    {
        void SetCanvas(const uint32_t w, const uint32_t h, const float dpi, void* window = nullptr);
        void GetCanvas(uint32_t* w, uint32_t* h, float* dpi, void** window = nullptr);

        void SetViewport(const uint32_t x, const uint32_t y, const uint32_t w, const uint32_t h);
        void GetViewport(uint32_t* x, uint32_t* y, uint32_t* w, uint32_t* h);

        void SetVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits);
        // setters and getters of rendering options
        VZRESULT Render(const VID vidScene, const VID vidCam);
    };
}
