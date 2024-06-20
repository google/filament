//#pragma warning(disable:4819)
#include "VizEngineAPIs.h" 

#include <iostream>

#include <filament/Engine.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <utils/EntityManager.h>
#include <utils/Log.h>

#include <filameshio/MeshReader.h>

//#include <filamentapp/FilamentApp.h>
//
//#include "generated/resources/resources.h"
//#include "generated/resources/monkey.h"

#include <filamentapp/Config.h>
#include "backend/platforms/VulkanPlatform.h" // requires blueVK.h

// name spaces
using namespace filament;
using namespace filamesh;
using namespace filament::math;
using namespace filament::backend;


class FilamentAppVulkanPlatform : public VulkanPlatform {
public:
    FilamentAppVulkanPlatform(char const* gpuHintCstr) {
        utils::CString gpuHint{ gpuHintCstr };
        if (gpuHint.empty()) {
            return;
        }
        VulkanPlatform::Customization::GPUPreference pref;
        // Check to see if it is an integer, if so turn it into an index.
        if (std::all_of(gpuHint.begin(), gpuHint.end(), ::isdigit)) {
            char* p_end{};
            pref.index = static_cast<int8_t>(std::strtol(gpuHint.c_str(), &p_end, 10));
        }
        else {
            pref.deviceName = gpuHint;
        }
        mCustomization = {
            .gpu = pref
        };
    }

    virtual VulkanPlatform::Customization getCustomization() const noexcept override {
        return mCustomization;
    }

private:
    VulkanPlatform::Customization mCustomization;
};

#if FILAMENT_DISABLE_MATOPT
#define OPTIMIZE_MATERIALS false
#else
#define OPTIMIZE_MATERIALS true
#endif

#define CANVAS_INIT_W 16u
#define CANVAS_INIT_H 16u
#define CANVAS_INIT_DPI 96.f

#pragma region // global enumerations
#pragma endregion

namespace vzm::backlog
{
    enum class LogLevel
    {
        None,
        Default,
        Warning,
        Error,
    };

    void post(const std::string& input, LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Default: utils::slog.i << input;
        case LogLevel::Warning: utils::slog.w << input;
        case LogLevel::Error: utils::slog.e << input;
        default: return;
        }
    }
}

namespace vzm
{
    struct Timer
    {
        std::chrono::high_resolution_clock::time_point timestamp = std::chrono::high_resolution_clock::now();

        // Record a reference timestamp
        inline void record()
        {
            timestamp = std::chrono::high_resolution_clock::now();
        }

        // Elapsed time in seconds between the vzm::Timer creation or last recording and "timestamp2"
        inline double elapsed_seconds_since(std::chrono::high_resolution_clock::time_point timestamp2)
        {
            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(timestamp2 - timestamp);
            return time_span.count();
        }

        // Elapsed time in seconds since the vzm::Timer creation or last recording
        inline double elapsed_seconds()
        {
            return elapsed_seconds_since(std::chrono::high_resolution_clock::now());
        }

        // Elapsed time in milliseconds since the vzm::Timer creation or last recording
        inline double elapsed_milliseconds()
        {
            return elapsed_seconds() * 1000.0;
        }

        // Elapsed time in milliseconds since the vzm::Timer creation or last recording
        inline double elapsed()
        {
            return elapsed_milliseconds();
        }

        // Record a reference timestamp and return elapsed time in seconds since the vzm::Timer creation or last recording
        inline double record_elapsed_seconds()
        {
            auto timestamp2 = std::chrono::high_resolution_clock::now();
            auto elapsed = elapsed_seconds_since(timestamp2);
            timestamp = timestamp2;
            return elapsed;
        }
    };
}

static bool g_is_display = true;
auto fail_ret = [](const std::string& err_str, const bool _warn = false)
    {
        if (g_is_display)
        {
            vzm::backlog::post(err_str, _warn ? vzm::backlog::LogLevel::Warning : vzm::backlog::LogLevel::Error);
        }
        return false;
    };

#define MathSet(Type, Name, Src, Num) Type Name; memcpy(&Name, Src, sizeof(Type)); 
inline float3 transformCoord(const mat4f& m, const float3& p)
{
    float4 _p(p, 1.f);
    _p = m * _p;
    return float3(_p.x / _p.w, _p.y / _p.w, _p.z / _p.w);
}

inline float3 transformVec(const mat3f& m, const float3& v)
{
    return m * v;
}

static Config gConfig;
static Engine::Config gEngineConfig = {};
static filament::backend::VulkanPlatform* gVulkanPlatform = nullptr;
static Engine* gEngine = nullptr;
static filament::SwapChain* gDummySwapChain = nullptr;

namespace vzm
{

    std::atomic_bool profileFrameFinished = { true };

    void TransformPoint(const float posSrc[3], const float mat[16], float posDst[3])
    {
        //MathSet(float3, p, posSrc);
        //MathSet(mat4f, m, mat);
        //float3 _p = transformCoord(m, p);
        //memcpy(posDst, &_p, sizeof(float3));
    }
    void TransformVector(const float vecSrc[3], const float mat[16], float vecDst[3])
    {
        //MathSet(float3, v, vecSrc);
        //MathSet(mat4f, m, mat);
        //mat3f _m(m);
        //float3 _v = transformVec(_m, v);
        //memcpy(vecDst, &_v, sizeof(float3));
    }
    void ComputeBoxTransformMatrix(const float cubeScale[3], const float posCenter[3],
        const float yAxis[3], const float zAxis[3], float mat[16], float matInv[16])
    {
        //XMVECTOR vec_scale = XMLoadFloat3((XMFLOAT3*)cubeScale);
        //XMVECTOR pos_eye = XMLoadFloat3((XMFLOAT3*)posCenter);
        //XMVECTOR vec_up = XMLoadFloat3((XMFLOAT3*)yAxis);
        //XMVECTOR vec_view = -XMLoadFloat3((XMFLOAT3*)zAxis);
        //XMMATRIX ws2cs = VZMatrixLookTo(pos_eye, vec_view, vec_up);
        //vec_scale = XMVectorReciprocal(vec_scale);
        //XMMATRIX scale = XMMatrixScaling(XMVectorGetX(vec_scale), XMVectorGetY(vec_scale), XMVectorGetZ(vec_scale));
        ////glm::fmat4x4 translate = glm::translate(glm::fvec3(0.5f));
        //
        //XMMATRIX ws2cs_unit = ws2cs * scale; // row major
        //*(XMMATRIX*)mat = rowMajor ? ws2cs_unit : XMMatrixTranspose(ws2cs_unit); // note that our renderer uses row-major
        //*(XMMATRIX*)matInv = XMMatrixInverse(NULL, ws2cs_unit);
    }
}

namespace vzm
{
#pragma region // VZM DATA STRUCTURES
    struct VzCanvas
    {
        uint32_t width = 0;
        uint32_t height = 0;
        float dpi = 96;
        float scaling = 1; // custom DPI scaling factor (optional)

        // Create a canvas from physical measurements
        inline void init(uint32_t width, uint32_t height, float dpi = 96)
        {
            this->width = width;
            this->height = height;
            this->dpi = dpi;
        }
        // Copy canvas from other canvas
        inline void init(const VzCanvas& other)
        {
            *this = other;
        }
    };

    class VzRenderer : VzCanvas
    {
    private:

        uint32_t prev_width = 0;
        uint32_t prev_height = 0;
        float prev_dpi = 96;
        bool prev_colorspace_conversion_required = false;

        VmCamera* vmCam = nullptr;
        TimeStamp timeStamp_vmUpdate;

        void TryResizeRenderTargets()
        {
            if (gEngine == nullptr)
                return;

            colorspace_conversion_required = colorspace != SWAP_CHAIN_CONFIG_SRGB_COLORSPACE;

            bool requireUpdateRenderTarget = prev_width != width || prev_height != height || prev_dpi != dpi
                || prev_colorspace_conversion_required != colorspace_conversion_required;
            if (!requireUpdateRenderTarget)
                return;

            init(width, height, dpi);
            swapChain = {};
            renderResult = {};
            renderInterResult = {};

            auto CreateRenderTarget = [&](wi::graphics::Texture& renderTexture, const bool isInterResult)
                {
                    if (!renderTexture.IsValid())
                    {
                        wi::graphics::TextureDesc desc;
                        desc.width = width;
                        desc.height = height;
                        desc.bind_flags = wi::graphics::BindFlag::RENDER_TARGET | wi::graphics::BindFlag::SHADER_RESOURCE;
                        if (!isInterResult)
                        {
                            // we assume the main GUI and engine use the same GPU device
                            // wi::graphics::ResourceMiscFlag::SHARED_ACROSS_ADAPTER;
                            desc.misc_flags = wi::graphics::ResourceMiscFlag::SHARED;
                            desc.format = wi::graphics::Format::R10G10B10A2_UNORM;
                        }
                        else
                        {
                            desc.format = wi::graphics::Format::R11G11B10_FLOAT;
                        }
                        bool success = graphicsDevice->CreateTexture(&desc, nullptr, &renderTexture);
                        assert(success);

                        graphicsDevice->SetName(&renderTexture, (isInterResult ? "VzRenderer::renderInterResult_" : "VzRenderer::renderResult_") + camEntity);
                    }
                };
            if (colorspace_conversion_required)
            {
                CreateRenderTarget(renderInterResult, true);
            }
            if (swapChain.IsValid())
            {
                // dojo to do ... create swapchain...
                // a window handler required 
            }
            else
            {
                CreateRenderTarget(renderResult, false);
            }

            Start(); // call ResizeBuffers();
            prev_width = width;
            prev_height = height;
            prev_dpi = dpi;
            prev_colorspace_conversion_required = colorspace_conversion_required;
        }

    public:

        uint64_t FRAMECOUNT = 0;

        float deltaTime = 0;
        float deltaTimeAccumulator = 0;
        float targetFrameRate = 60;		//
        bool frameskip = true;
        bool framerate_lock = false;	//
        vzm::Timer timer;

        int fps_avg_counter = 0;
        float		time;
        float		time_previous;

        // render target 을 compose... 
        utils::Entity camEntity = utils::Entity(); // INVALID_ENTITY;
        bool colorspace_conversion_required = false;
        uint64_t colorspace = SWAP_CHAIN_CONFIG_SRGB_COLORSPACE; // swapchain color space

        // note swapChain and renderResult are exclusive
        //wi::graphics::Texture renderResult;
        filament::SwapChain* mSwapChain = nullptr;

        // display all-time engine information text
        std::string infodisplay_str;
        float deltatimes[20] = {};

        bool DisplayProfile = false; // this is only for profiling canvas

        // kind of initializer
        void Load()
        {
            //setSceneUpdateEnabled(true); // for multiple main-cameras support

            wi::font::UpdateAtlas(GetDPIScaling());

            RenderPath3D::Load();

            assert(width > 0 && height > 0);
            {
                const float fadeSeconds = 0.f;
                wi::Color fadeColor = wi::Color(0, 0, 0, 255);
                // Fade manager will activate on fadeout
                fadeManager.Clear();
                fadeManager.Start(fadeSeconds, fadeColor, [this]() {
                    Start();
                    });

                fadeManager.Update(0); // If user calls ActivatePath without fadeout, it will be instant
            }
        }

        void Update(float dt) override
        {
            if (vmCam)
            {
                vmCam->IsActivated = true;
            }
            UpdateRendererOptions();
            RenderPath3D::Update(dt);	// calls RenderPath2D::Update(dt);
        }

        void PostUpdate() override
        {
            RenderPath2D::PostUpdate();
            RenderPath3D::PostUpdate();
        }

        void FixedUpdate() override
        {
            RenderPath2D::FixedUpdate();
        }

        void Compose(wi::graphics::CommandList cmd)
        {
            using namespace wi::graphics;

            wi::graphics::GraphicsDevice* graphicsDevice = GetDevice();
            if (!graphicsDevice)
                return;

            auto range = wi::profiler::BeginRangeCPU("Compose");

            wi::RenderPath3D::Compose(cmd);

            if (fadeManager.IsActive())
            {
                // display fade rect
                wi::image::Params fx;
                fx.enableFullScreen();
                fx.color = fadeManager.color;
                fx.opacity = fadeManager.opacity;
                wi::image::Draw(nullptr, fx, cmd);
            }

            // Draw the information display
            if (infoDisplay.active)
            {
                infodisplay_str.clear();
                if (infoDisplay.watermark)
                {
                    infodisplay_str += "Wicked Engine ";
                    infodisplay_str += wi::version::GetVersionString();
                    infodisplay_str += " ";

#if defined(_ARM)
                    infodisplay_str += "[ARM]";
#elif defined(_WIN64)
                    infodisplay_str += "[64-bit]";
#elif defined(_WIN32)
                    infodisplay_str += "[32-bit]";
#endif // _ARM

#ifdef PLATFORM_UWP
                    infodisplay_str += "[UWP]";
#endif // PLATFORM_UWP

#ifdef WICKEDENGINE_BUILD_DX12
                    if (dynamic_cast<GraphicsDevice_DX12*>(graphicsDevice))
                    {
                        infodisplay_str += "[DX12]";
                    }
#endif // WICKEDENGINE_BUILD_DX12
#ifdef WICKEDENGINE_BUILD_VULKAN
                    if (dynamic_cast<GraphicsDevice_Vulkan*>(graphicsDevice))
                    {
                        infodisplay_str += "[Vulkan]";
                    }
#endif // WICKEDENGINE_BUILD_VULKAN
#ifdef PLATFORM_PS5
                    if (dynamic_cast<GraphicsDevice_PS5*>(graphicsDevice.get()))
                    {
                        infodisplay_str += "[PS5]";
                    }
#endif // PLATFORM_PS5

#ifdef _DEBUG
                    infodisplay_str += "[DEBUG]";
#endif // _DEBUG
                    if (graphicsDevice->IsDebugDevice())
                    {
                        infodisplay_str += "[debugdevice]";
                    }
                    infodisplay_str += "\n";
                }
                if (infoDisplay.device_name)
                {
                    infodisplay_str += "Device: ";
                    infodisplay_str += graphicsDevice->GetAdapterName();
                    infodisplay_str += "\n";
                }
                if (infoDisplay.resolution)
                {
                    infodisplay_str += "Resolution: ";
                    infodisplay_str += std::to_string(GetPhysicalWidth());
                    infodisplay_str += " x ";
                    infodisplay_str += std::to_string(GetPhysicalHeight());
                    infodisplay_str += " (";
                    infodisplay_str += std::to_string(int(GetDPI()));
                    infodisplay_str += " dpi)\n";
                }
                if (infoDisplay.logical_size)
                {
                    infodisplay_str += "Logical Size: ";
                    infodisplay_str += std::to_string(int(GetLogicalWidth()));
                    infodisplay_str += " x ";
                    infodisplay_str += std::to_string(int(GetLogicalHeight()));
                    infodisplay_str += "\n";
                }
                if (infoDisplay.colorspace)
                {
                    infodisplay_str += "Color Space: ";
                    ColorSpace colorSpace = colorspace; // graphicsDevice->GetSwapChainColorSpace(&swapChain);
                    switch (colorSpace)
                    {
                    default:
                    case wi::graphics::ColorSpace::SRGB:
                        infodisplay_str += "sRGB";
                        break;
                    case wi::graphics::ColorSpace::HDR10_ST2084:
                        infodisplay_str += "ST.2084 (HDR10)";
                        break;
                    case wi::graphics::ColorSpace::HDR_LINEAR:
                        infodisplay_str += "Linear (HDR)";
                        break;
                    }
                    infodisplay_str += "\n";
                }
                if (infoDisplay.fpsinfo)
                {
                    deltatimes[fps_avg_counter++ % arraysize(deltatimes)] = deltaTime;
                    float displaydeltatime = deltaTime;
                    if (fps_avg_counter > arraysize(deltatimes))
                    {
                        float avg_time = 0;
                        for (int i = 0; i < arraysize(deltatimes); ++i)
                        {
                            avg_time += deltatimes[i];
                        }
                        displaydeltatime = avg_time / arraysize(deltatimes);
                    }

                    infodisplay_str += std::to_string(int(std::round(1.0f / displaydeltatime))) + " FPS\n";
                }
                if (infoDisplay.heap_allocation_counter)
                {
                    infodisplay_str += "Heap allocations per frame: ";
#ifdef WICKED_ENGINE_HEAP_ALLOCATION_COUNTER
                    infodisplay_str += std::to_string(number_of_heap_allocations.load());
                    infodisplay_str += " (";
                    infodisplay_str += std::to_string(size_of_heap_allocations.load());
                    infodisplay_str += " bytes)\n";
                    number_of_heap_allocations.store(0);
                    size_of_heap_allocations.store(0);
#else
                    infodisplay_str += "[disabled]\n";
#endif // WICKED_ENGINE_HEAP_ALLOCATION_COUNTER
                }
                if (infoDisplay.pipeline_count)
                {
                    infodisplay_str += "Graphics pipelines active: ";
                    infodisplay_str += std::to_string(graphicsDevice->GetActivePipelineCount());
                    infodisplay_str += "\n";
                }

                wi::font::Params params = wi::font::Params(
                    4,
                    4,
                    infoDisplay.size,
                    wi::font::WIFALIGN_LEFT,
                    wi::font::WIFALIGN_TOP,
                    wi::Color::White(),
                    wi::Color::Shadow()
                );
                params.shadow_softness = 0.4f;

                // Explanation: this compose pass is in LINEAR space if display output is linear or HDR10
                //	If HDR10, the HDR10 output mapping will be performed on whole image later when drawing to swapchain
                if (colorspace != ColorSpace::SRGB)
                {
                    params.enableLinearOutputMapping(9);
                }

                params.cursor = wi::font::Draw(infodisplay_str, params, cmd);

                // VRAM:
                {
                    GraphicsDevice::MemoryUsage vram = graphicsDevice->GetMemoryUsage();
                    bool warn = false;
                    if (vram.usage > vram.budget)
                    {
                        params.color = wi::Color::Error();
                        warn = true;
                    }
                    else if (float(vram.usage) / float(vram.budget) > 0.9f)
                    {
                        params.color = wi::Color::Warning();
                        warn = true;
                    }
                    if (infoDisplay.vram_usage || warn)
                    {
                        params.cursor = wi::font::Draw("VRAM usage: " + std::to_string(vram.usage / 1024 / 1024) + "MB / " + std::to_string(vram.budget / 1024 / 1024) + "MB\n", params, cmd);
                        params.color = wi::Color::White();
                    }
                }

                // Write warnings below:
                params.color = wi::Color::Warning();
#ifdef _DEBUG
                params.cursor = wi::font::Draw("Warning: This is a [DEBUG] build, performance will be slow!\n", params, cmd);
#endif
                if (graphicsDevice->IsDebugDevice())
                {
                    params.cursor = wi::font::Draw("Warning: Graphics is in [debugdevice] mode, performance will be slow!\n", params, cmd);
                }

                // Write errors below:
                params.color = wi::Color::Error();
                if (wi::renderer::GetShaderMissingCount() > 0)
                {
                    params.cursor = wi::font::Draw(std::to_string(wi::renderer::GetShaderMissingCount()) + " shaders missing! Check the backlog for more information!\n", params, cmd);
                }
                if (wi::renderer::GetShaderErrorCount() > 0)
                {
                    params.cursor = wi::font::Draw(std::to_string(wi::renderer::GetShaderErrorCount()) + " shader compilation errors! Check the backlog for more information!\n", params, cmd);
                }


                if (infoDisplay.colorgrading_helper)
                {
                    wi::image::Draw(wi::texturehelper::getColorGradeDefault(), wi::image::Params(0, 0, 256.0f / GetDPIScaling(), 16.0f / GetDPIScaling()), cmd);
                }
            }

            vzm::backlog::Draw(*this, cmd, colorspace);
            wi::profiler::EndRange(range); // Compose

            if (DisplayProfile)
            {
                wi::profiler::DrawData(*this, 4, 60, cmd, colorspace);
            }
        }

        void RenderFinalize()
        {
            using namespace wi::graphics;

            wi::graphics::GraphicsDevice* graphicsDevice = GetDevice();
            if (!graphicsDevice)
                return;

            // Begin final compositing:
            CommandList cmd = graphicsDevice->BeginCommandList();
            wi::image::SetCanvas(*this);
            wi::font::SetCanvas(*this);

            Viewport viewport;
            viewport.width = (float)width;
            viewport.height = (float)height;
            graphicsDevice->BindViewports(1, &viewport, cmd);

            if (colorspace_conversion_required)
            {
                RenderPassImage rp[] = {
                    RenderPassImage::RenderTarget(&renderInterResult, RenderPassImage::LoadOp::CLEAR),
                };
                graphicsDevice->RenderPassBegin(rp, arraysize(rp), cmd);
            }
            else
            {
                // If swapchain is SRGB or Linear HDR, it can be used for blending
                //	- If it is SRGB, the render path will ensure tonemapping to SDR
                //	- If it is Linear HDR, we can blend trivially in linear space
                renderInterResult = {};
                if (swapChain.IsValid())
                {
                    graphicsDevice->RenderPassBegin(&swapChain, cmd);
                }
                else
                {
                    RenderPassImage rp[] = {
                        RenderPassImage::RenderTarget(&renderResult, RenderPassImage::LoadOp::CLEAR),
                    };
                    graphicsDevice->RenderPassBegin(rp, arraysize(rp), cmd);
                }
            }

            Compose(cmd);
            graphicsDevice->RenderPassEnd(cmd);

            if (colorspace_conversion_required)
            {
                // In HDR10, we perform a final mapping from linear to HDR10, into the swapchain
                if (swapChain.IsValid())
                {
                    graphicsDevice->RenderPassBegin(&swapChain, cmd);
                }
                else
                {
                    RenderPassImage rp[] = {
                        RenderPassImage::RenderTarget(&renderResult, RenderPassImage::LoadOp::CLEAR),
                    };
                    graphicsDevice->RenderPassBegin(rp, arraysize(rp), cmd);
                }
                wi::image::Params fx;
                fx.enableFullScreen();
                fx.enableHDR10OutputMapping();
                wi::image::Draw(&renderInterResult, fx, cmd);
                graphicsDevice->RenderPassEnd(cmd);
            }

            if (DisplayProfile)
            {
                profileFrameFinished = true;
                wi::profiler::EndFrame(cmd); // cmd must be assigned before SubmitCommandLists
            }
            graphicsDevice->SubmitCommandLists();
        }

        void WaitRender()
        {
            wi::graphics::GraphicsDevice* graphicsDevice = wi::graphics::GetDevice();
            if (!graphicsDevice)
                return;

            wi::graphics::CommandList cmd = graphicsDevice->BeginCommandList();
            if (swapChain.IsValid())
            {
                graphicsDevice->RenderPassBegin(&swapChain, cmd);
            }
            else
            {
                wi::graphics::RenderPassImage rt[] = {
                    wi::graphics::RenderPassImage::RenderTarget(&renderResult, wi::graphics::RenderPassImage::LoadOp::CLEAR)
                };
                graphicsDevice->RenderPassBegin(rt, 1, cmd);
            }

            wi::graphics::Viewport viewport;
            viewport.width = (float)width;
            viewport.height = (float)height;
            graphicsDevice->BindViewports(1, &viewport, cmd);
            if (wi::initializer::IsInitializeFinished(wi::initializer::INITIALIZED_SYSTEM_FONT))
            {
                vzm::backlog::DrawOutputText(*this, cmd, colorspace);
            }
            graphicsDevice->RenderPassEnd(cmd);
            graphicsDevice->SubmitCommandLists();
        }

        bool UpdateVmCamera(const VmCamera* _vmCam = nullptr)
        {
            // note Scene::Merge... rearrange the components
            camera = scene->cameras.GetComponent(camEntity);
            if (!getSceneUpdateEnabled())
            {
                scene->camera = *camera;
            }

            if (_vmCam != nullptr)
            {
                vmCam = (VmCamera*)_vmCam;
                camEntity = vmCam->componentVID;
            }
            if (vmCam)
            {
                std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(timeStamp_vmUpdate - vmCam->timeStamp);
                if (time_span.count() > 0)
                {
                    return true;
                }
            }
            TryResizeRenderTargets();

            // ???? DOJO TO DO... SetPose 에서 해 보기... flickering 동작 확인
            TransformComponent* transform = scene->transforms.GetComponent(camEntity);
            if (transform)
            {
                transform->UpdateTransform();
            }
            else
            {
                camera->UpdateCamera();
            }
            timeStamp_vmUpdate = std::chrono::high_resolution_clock::now();

            return true;
        }

        VmCamera* GetVmCamera()
        {
            return vmCam;
        }
    };

    struct VzmScene : Scene
    {
        VID sceneVid = INVALID_VID;
        VmWeather vmWeather;
        std::string name;

        float deltaTime = 0;
        vzm::Timer timer;
    };

    class SceneManager // application
    {
    private:
        filament::Material const* mDefaultMaterial = nullptr;
        filament::Material const* mTransparentMaterial = nullptr;
        filament::Material const* mDepthMaterial = nullptr;
        filament::MaterialInstance* mDepthMI = nullptr;

        //engine...
        //renderer

        // archive
        std::map<VID, VzmScene> scenes;						// <SceneEntity, Scene>
        // one camera component to one renderer
        std::map<VID, VzRenderer> renderers;				// <CamEntity, VzRenderer> (including camera and scene)
        wi::unordered_map<VID, std::unique_ptr<VmBaseComponent>> vmComponents;

    public:
        void Initialize(vzm::ParamMap<std::string>& argument)
        {
            // device creation
            // User can also create a graphics device if custom logic is desired, but they must do before this function!
            wi::platform::window_type window = argument.GetParam("window", wi::platform::window_type(nullptr));
            if (graphicsDevice == nullptr)
            {
                using namespace wi::graphics;

                ValidationMode validationMode = ValidationMode::Disabled;
                if (argument.GetParam("debugdevice", false))
                {
                    validationMode = ValidationMode::Enabled;
                }
                if (argument.GetParam("gpuvalidation", false))
                {
                    validationMode = ValidationMode::GPU;
                }
                if (argument.GetParam("gpu_verbose", false))
                {
                    validationMode = ValidationMode::Verbose;
                }

                GPUPreference preference = GPUPreference::Discrete;
                if (argument.GetParam("igpu", false))
                {
                    preference = GPUPreference::Integrated;
                }

                bool use_dx12 = wi::arguments::HasArgument("dx12");
                bool use_vulkan = wi::arguments::HasArgument("vulkan");

#ifndef WICKEDENGINE_BUILD_DX12
                if (use_dx12) {
                    wi::helper::messageBox("The engine was built without DX12 support!", "Error");
                    use_dx12 = false;
                }
#endif // WICKEDENGINE_BUILD_DX12
#ifndef WICKEDENGINE_BUILD_VULKAN
                if (use_vulkan) {
                    wi::helper::messageBox("The engine was built without Vulkan support!", "Error");
                    use_vulkan = false;
                }
#endif // WICKEDENGINE_BUILD_VULKAN

                if (!use_dx12 && !use_vulkan)
                {
#if defined(WICKEDENGINE_BUILD_DX12)
                    use_dx12 = true;
#elif defined(WICKEDENGINE_BUILD_VULKAN)
                    use_vulkan = true;
#else
                    vzm::backlog::post("No rendering backend is enabled! Please enable at least one so we can use it as default", vzm::backlog::LogLevel::Error);
                    assert(false);
#endif
                }
                assert(use_dx12 || use_vulkan);

                if (use_vulkan)
                {
#ifdef WICKEDENGINE_BUILD_VULKAN
                    wi::renderer::SetShaderPath(wi::renderer::GetShaderPath() + "spirv/");
                    graphicsDevice = std::make_unique<GraphicsDevice_Vulkan>(window, validationMode, preference);
#endif
                }
                else if (use_dx12)
                {
#ifdef WICKEDENGINE_BUILD_DX12
                    wi::renderer::SetShaderPath(wi::renderer::GetShaderPath() + "hlsl6/");
                    graphicsDevice = std::make_unique<GraphicsDevice_DX12>(validationMode, preference);
#endif
                }
            }
            wi::graphics::GetDevice() = graphicsDevice.get();

            wi::initializer::InitializeComponentsAsync();
            //wi::initializer::InitializeComponentsImmediate();

            // Reset all state that tests might have modified:
            wi::eventhandler::SetVSync(false);
            //wi::renderer::SetToDrawGridHelper(false);
            //wi::renderer::SetTemporalAAEnabled(false);
            //wi::renderer::ClearWorld(wi::scene::GetScene());
        }

        // Runtime can create a new entity with this
        inline VID CreateSceneEntity(const std::string& name)
        {
            if (GetFirstSceneByName(name))
            {
                vzm::backlog::post(name + " is already registered as a scene!", backlog::LogLevel::Error);
                return INVALID_ENTITY;
            }

            Entity ett = CreateEntity();

            if (ett != INVALID_ENTITY) {
                VzmScene& scene = scenes[ett];
                wi::renderer::ClearWorld(scene);
                scene.weather = WeatherComponent();
                scene.weather.ambient = XMFLOAT3(0.9f, 0.9f, 0.9f);
                wi::Color default_sky_zenith = wi::Color(30, 40, 60, 200);
                scene.weather.zenith = default_sky_zenith;
                wi::Color default_sky_horizon = wi::Color(10, 10, 20, 220); // darker elements will lerp towards this
                scene.weather.horizon = default_sky_horizon;
                scene.weather.fogStart = std::numeric_limits<float>::max();
                scene.weather.fogDensity = 0;
                {
                    scene.vmWeather.componentVID = ett;
                    scene.vmWeather.compType = COMPONENT_TYPE::WEATHER;
                }
                scene.name = name;
                scene.sceneVid = ett;
            }

            return ett;
        }
        inline VzRenderer* CreateRenderer(const VID camEntity)
        {
            auto it = renderers.find(camEntity);
            assert(it == renderers.end());

            VzRenderer* renderer = &renderers[camEntity];
            renderer->camEntity = camEntity;

            for (auto it = scenes.begin(); it != scenes.end(); it++)
            {
                VzmScene* scene = &it->second;
                CameraComponent* camera = scene->cameras.GetComponent(camEntity);
                if (camera)
                {
                    renderer->scene = scene;
                    renderer->camera = camera;
                    return renderer;
                }
            }
            renderers.erase(camEntity);
            return nullptr;
        }

        inline uint32_t GetVidsByName(const std::string& name, std::vector<VID>& vids)
        {
            for (auto it = scenes.begin(); it != scenes.end(); it++)
            {
                VzmScene& scene = it->second;
                for (Entity ett : scene.names.GetEntityArray())
                {
                    NameComponent* nameComp = scene.names.GetComponent(ett);
                    if (nameComp != nullptr)
                    {
                        vids.push_back(ett);
                    }
                }
            }
            return (uint32_t)vids.size();
        }
        inline VID GetFirstVidByName(const std::string& name)
        {
            for (auto it = scenes.begin(); it != scenes.end(); it++)
            {
                Entity ett = it->second.Entity_FindByName(name);
                if (ett != INVALID_ENTITY)
                {
                    return ett;
                }
                if (it->second.name == name)
                {
                    return it->second.sceneVid;
                }
            }
            return INVALID_VID;
        }
        inline std::string GetNameByVid(const VID vid)
        {
            for (auto it = scenes.begin(); it != scenes.end(); it++)
            {
                NameComponent* nameComp = it->second.names.GetComponent(vid);
                if (nameComp)
                {
                    return nameComp->name;
                }
            }
            auto it = scenes.find(vid);
            if (it != scenes.end())
            {
                return it->second.name;
            }
            return "";
        }
        inline VzmScene* GetScene(const VID sid)
        {
            auto it = scenes.find(sid);
            if (it == scenes.end())
            {
                return nullptr;
            }
            return &it->second;
        }
        inline VzmScene* GetFirstSceneByName(const std::string& name)
        {
            for (auto it = scenes.begin(); it != scenes.end(); it++)
            {
                VzmScene* scene = &it->second;
                if (scene->name == name)
                {
                    return scene;
                }
            }
            return nullptr;
        }
        inline std::map<VID, VzmScene>* GetScenes()
        {
            return &scenes;
        }
        inline VzRenderer* GetRenderer(const VID camEntity)
        {
            auto it = renderers.find(camEntity);
            if (it == renderers.end())
            {
                return nullptr;
            }
            return &it->second;
        }
        inline VzRenderer* GetFirstRendererByName(const std::string& name)
        {
            return GetRenderer(GetFirstVidByName(name));
        }

        template <typename VMCOMP>
        inline VMCOMP* CreateVmComp(const VID vid)
        {
            auto it = vmComponents.find(vid);
            assert(it == vmComponents.end());

            std::string typeName = typeid(VMCOMP).name();
            COMPONENT_TYPE compType = vmcomptypes[typeName];
            if (compType == COMPONENT_TYPE::UNDEFINED)
            {
                return nullptr;
            }

            for (auto sit = scenes.begin(); sit != scenes.end(); sit++)
            {
                VzmScene* scene = &sit->second;
                wi::unordered_set<Entity> entities;
                scene->FindAllEntities(entities);
                auto it = entities.find(vid);
                if (it != entities.end())
                {
                    VMCOMP vmComp;
                    vmComp.componentVID = vid;
                    vmComp.compType = compType;
                    vmComponents.insert(std::make_pair(vid, std::make_unique<VMCOMP>(vmComp)));
                    return (VMCOMP*)vmComponents[vid].get();
                }
            }
            return nullptr;
        }

        template <typename VMCOMP>
        inline VMCOMP* GetVmComp(const VID vid)
        {
            auto it = vmComponents.find(vid);
            if (it == vmComponents.end())
            {
                return nullptr;
            }
            return (VMCOMP*)it->second.get();
        }
        template <typename COMP>
        inline COMP* GetEngineComp(const VID vid)
        {
            std::string typeName = typeid(COMP).name();
            CompType compType = comptypes[typeName];
            COMP* comp = nullptr;
            for (auto it = scenes.begin(); it != scenes.end(); it++)
            {
                VzmScene* scene = &it->second;
                switch (compType)
                {
                case CompType::CameraComponent_:
                    comp = (COMP*)scene->cameras.GetComponent(vid); break;
                case CompType::ObjectComponent_:
                    comp = (COMP*)scene->objects.GetComponent(vid); break;
                case CompType::TransformComponent_:
                    comp = (COMP*)scene->transforms.GetComponent(vid); break;
                case CompType::HierarchyComponent_:
                    comp = (COMP*)scene->hierarchy.GetComponent(vid); break;
                case CompType::NameComponent_:
                    comp = (COMP*)scene->names.GetComponent(vid); break;
                case CompType::LightComponent_:
                    comp = (COMP*)scene->lights.GetComponent(vid); break;
                case CompType::AnimationComponent_:
                    comp = (COMP*)scene->animations.GetComponent(vid); break;
                case CompType::EmittedParticleSystem_:
                    comp = (COMP*)scene->emitters.GetComponent(vid); break;
                case CompType::ColliderComponent_:
                    comp = (COMP*)scene->colliders.GetComponent(vid); break;
                case CompType::WeatherComponent_:
                    comp = (COMP*)scene->weathers.GetComponent(vid); break;
                default: assert(0 && "Not allowed GetComponent");  return nullptr;
                }
                if (comp) break;
            }

            return comp;
        }
        inline TransformComponent* GetEngineTransformComponent(const VID vid)
        {
            for (auto it = scenes.begin(); it != scenes.end(); it++)
            {
                VzmScene* scene = &it->second;
                TransformComponent* tc = scene->transforms.GetComponent(vid);
                if (tc)
                {
                    return tc;
                }
            }
            return nullptr;
        }

        inline void RemoveEntity(Entity entity)
        {
            VzmScene* scene = GetScene(entity);
            if (scene)
            {
                wi::unordered_set<Entity> entities;
                scene->FindAllEntities(entities);

                for (auto it = entities.begin(); it != entities.end(); it++)
                {
                    renderers.erase(*it);
                    vmComponents.erase(*it);
                }
                scenes.erase(entity);
            }
            else
            {

                for (auto it = scenes.begin(); it != scenes.end(); it++)
                {
                    VzmScene* scene = &it->second;
                    scene->Entity_Remove(entity);
                }
                renderers.erase(entity);
                vmComponents.erase(entity);
            }
        }
    };
#pragma endregion

    SceneManager sceneManager;
    /**/
}

#define COMP_GET(COMP, PARAM, RET) COMP* PARAM = sceneManager.GetEngineComp<COMP>(componentVID); if (!PARAM) return RET;

namespace vzm
{
    struct SafeReleaseChecker
    {
        SafeReleaseChecker() {};
        bool destroyed = false;
        ~SafeReleaseChecker()
        {
            if (!destroyed)
            {
                std::cout << "MUST CALL DeinitEngineLib before finishing the application!" << std::endl;
                DeinitEngineLib();
            }
            std::cout << "Safely finished ^^" << std::endl;
        };
    };
    std::unique_ptr<SafeReleaseChecker> safeReleaseChecker;

    VZRESULT InitEngineLib(const std::string& coreName, const std::string& logFileName)
    {
        if (gEngine)
        {
            backlog::post("Already initialized!", backlog::LogLevel::Error);
            return VZ_WARNNING;
        }

        auto& em = utils::EntityManager::get();
        backlog::post("Entity Manager is activated (# of entities : " + std::to_string(em.getEntityCount()) + ")", 
            backlog::LogLevel::Default);

        std::string vulkanGPUHint = "0";

        // to do : gConfig and gEngineConfig
        // using arguments

        gConfig.backend = filament::Engine::Backend::VULKAN;
        gConfig.vulkanGPUHint = "0";
        gConfig.headless = true;
                
        gVulkanPlatform = new FilamentAppVulkanPlatform(gConfig.vulkanGPUHint.c_str());
        gEngine = Engine::Builder()
            .backend(gConfig.backend)
            .platform(gVulkanPlatform)
            .featureLevel(filament::backend::FeatureLevel::FEATURE_LEVEL_3)
            .config(&gEngineConfig)
            .build();

        // this is to avoid the issue of filament safe-resource logic for Vulkan,
        // which assumes that there is at least one swapchain.
        gDummySwapChain = gEngine->createSwapChain((uint32_t)1, (uint32_t)1);

        if (safeReleaseChecker == nullptr)
        {
            safeReleaseChecker = std::make_unique<SafeReleaseChecker>();
        }
        else
        {
            safeReleaseChecker->destroyed = false;
        }

        return VZ_OK;
    }

    VZRESULT DeinitEngineLib()
    {
        if (safeReleaseChecker.get() == nullptr)
        {
            vzm::backlog::post("MUST CALL vzm::InitEngineLib before calling vzm::DeinitEngineLib()", backlog::LogLevel::Error);
            return VZ_WARNNING;
        }

        //gEngine->destroy(gRenderer);
        gEngine->destroy(gDummySwapChain);
        gDummySwapChain = nullptr;
        Engine::destroy(&gEngine); // calls FEngine::shutdown()
        // note 
        // gEngine involves mJobSystem
        // when releasing gEngine, mJobSystem will be released!!
        // this calls JobSystem::requestExit() that including JobSystem::wakeAll()
        gEngine = nullptr;

        if (gVulkanPlatform) {
            delete gVulkanPlatform;
            gVulkanPlatform = nullptr;
        }

        safeReleaseChecker->destroyed = true;
        return VZ_OK;
    }
}
