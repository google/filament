#ifndef VZRENDERPATH_H
#define VZRENDERPATH_H
#include "VzComponents.h"

#define CANVAS_INIT_W 16u
#define CANVAS_INIT_H 16u
#define CANVAS_INIT_DPI 96.f

namespace filament
{
    class View;
    class Renderer;
    class SwapChain;
}

namespace vzm
{

    struct VzCanvas
    {
    protected:
        uint32_t width_ = CANVAS_INIT_W;
        uint32_t height_ = CANVAS_INIT_H;
        float dpi_ = CANVAS_INIT_DPI;
        float scaling_ = 1; // custom DPI scaling factor (optional)
        void* nativeWindow_ = nullptr;
    public:
        VzCanvas() = default;
        ~VzCanvas() = default;
    };

    // note that renderPath involves 
    // 1. canvas (render targets), 2. camera, 3. scene
    class VzRenderPath : public VzCanvas
    {
    private:
        uint32_t prevWidth_ = 0;
        uint32_t prevHeight_ = 0;
        float prevDpi_ = 0;
        void* prevNativeWindow_ = nullptr;
        bool prevColorspaceConversionRequired_ = false;

        VzCamera* vzCam_ = nullptr;
        TimeStamp timeStamp_ = {};

        float targetFrameRate_ = 60.f;

        bool colorspaceConversionRequired_ = false;
        uint64_t colorSpace_ = 0; // swapchain color space

        // note "view" involves
        // 1. camera
        // 2. scene
        filament::View* view_ = nullptr;
        filament::SwapChain* swapChain_ = nullptr;
        filament::Renderer* renderer_ = nullptr;

        void resize();

    public:
        VzRenderPath();

        ~VzRenderPath();

        bool TryResizeRenderTargets();

        void SetFixedTimeUpdate(const float targetFPS);
        float GetFixedTimeUpdate() const;

        void GetCanvas(uint32_t* w, uint32_t* h, float* dpi, void** window);
        void SetCanvas(const uint32_t w, const uint32_t h, const float dpi, void* window = nullptr);
        filament::SwapChain* GetSwapChain();

        uint64_t FRAMECOUNT = 0;
        float deltaTime = 0;
        float deltaTimeAccumulator = 0;

        filament::View* GetView();
        filament::Renderer* GetRenderer();
    };
}
#endif
