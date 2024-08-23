// Filament highlevel APIs
#include "VizEngineAPIs.h"

#include <iostream>
#include <windowsx.h>
#include <random>

#include <tchar.h>
#include <shellscalingapi.h>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/vector_angle.hpp"

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Windows 8.1 및 Windows 10에서 DPI 인식을 설정하는 코드
void EnableDpiAwareness() {
    // Windows 10에서 사용할 수 있는 DPI 인식 설정
    HMODULE hUser32 = LoadLibrary(TEXT("user32.dll"));
    if (hUser32) {
        typedef BOOL(WINAPI* SetProcessDpiAwarenessContextProc)(DPI_AWARENESS_CONTEXT);
        SetProcessDpiAwarenessContextProc SetProcessDpiAwarenessContextFunc =
            (SetProcessDpiAwarenessContextProc)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");

        if (SetProcessDpiAwarenessContextFunc) {
            SetProcessDpiAwarenessContextFunc(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        }
        else {
            // Windows 8.1에서 사용할 수 있는 DPI 인식 설정
            HMODULE hShcore = LoadLibrary(TEXT("shcore.dll"));
            if (hShcore) {
                typedef HRESULT(WINAPI* SetProcessDpiAwarenessProc)(PROCESS_DPI_AWARENESS);
                SetProcessDpiAwarenessProc SetProcessDpiAwarenessFunc =
                    (SetProcessDpiAwarenessProc)GetProcAddress(hShcore, "SetProcessDpiAwareness");

                if (SetProcessDpiAwarenessFunc) {
                    SetProcessDpiAwarenessFunc(PROCESS_PER_MONITOR_DPI_AWARE);
                }
                FreeLibrary(hShcore);
            }
        }
        FreeLibrary(hUser32);
    }
}

HWND createNativeWindow(HINSTANCE hInstance, int nCmdShow, int width, int height) {
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL) {
        std::cerr << "Failed to create window." << std::endl;
        return NULL;
    }

    ShowWindow(hwnd, nCmdShow);

    return hwnd;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // DPI 인식을 활성화
    EnableDpiAwareness();

    HWND hwnd = createNativeWindow(hInstance, nCmdShow, 800, 600);
    if (!hwnd) {
        return -1;
    }

    HDC hdc = GetDC(hwnd);
    if (!hdc) {
        std::cerr << "Failed to get device context." << std::endl;
        return -1;
    }

    RECT rc;
    GetClientRect(hwnd, &rc);
    uint32_t w = rc.right - rc.left;
    uint32_t h = rc.bottom - rc.top;
    float dpi = 96.f;

    vzm::ParamMap<std::string> arguments;
    arguments.SetParam("api", std::string("opengl"));
    arguments.SetParam("vulkan-gpu-hint", std::string("0"));
    if (vzm::InitEngineLib(arguments) != VZ_OK) {
        std::cerr << "Failed to initialize engine library." << std::endl;
        return -1;
    }
    
    vzm::VzScene* scene = vzm::NewScene("my scene");
    scene->LoadIBL("../../../VisualStudio/samples/assets/ibl/lightroom_14b");

    std::vector<vzm::VzActor*> loaded_actors;
    vzm::VzActor* actor_axis = vzm::LoadModelFileIntoActors("../assets/xyz.obj", loaded_actors);
    glm::fvec3 scale(3.f);
    actor_axis->SetScale(__FP scale);
    
    vzm::VzRenderer* renderer = vzm::NewRenderer("my renderer");
    renderer->SetCanvas(w, h, dpi, hwnd);
    renderer->SetVisibleLayerMask(0x4, 0x4);
    
    vzm::VzCamera* cam = (vzm::VzCamera*)vzm::NewSceneComponent(vzm::SCENE_COMPONENT_TYPE::CAMERA, "my camera", 0);
    glm::fvec3 p(0, 0, 10);
    glm::fvec3 at(0, 0, -4);
    glm::fvec3 u(0, 1, 0);
    cam->SetWorldPose((float*)&p, (float*)&at, (float*)&u);
    //glm::fvec3 pp, att, uu;
    //cam->GetWorldPose((float*)&pp, (float*)&att, (float*)&uu);
    cam->SetPerspectiveProjection(0.1f, 1000.f, 45.f, (float)w / (float)h);
    cam->SetMatrixAutoUpdate(false);
    vzm::VzCamera::Controller* cc = cam->GetController();
    *(glm::fvec3*)cc->orbitHomePosition = p;
    cc->UpdateControllerSettings();
    cc->SetViewport(w, h);

    vzm::VzLight* light = (vzm::VzLight*)vzm::NewSceneComponent(vzm::SCENE_COMPONENT_TYPE::LIGHT, "my light");

    vzm::VzTexture* texture1 = (vzm::VzTexture*)vzm::NewResComponent(vzm::RES_COMPONENT_TYPE::TEXTURE, "my image 1");
    texture1->ReadImage("../assets/testimage.png");
    vzm::VzTexture* texture2 = (vzm::VzTexture*)vzm::NewResComponent(vzm::RES_COMPONENT_TYPE::TEXTURE, "my image 2");
    texture2->ReadImage("../assets/testimage1.png");

    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::uniform_real_distribution<> dis(0.0, 1.0);
    for (size_t i = 0; i < 10; ++i)
    {
        float randomValue1 = (float)dis(gen);
        float randomValue2 = (float)dis(gen);
        float randomValue3 = (float)dis(gen);

        vzm::VzSpriteActor* sprite = 
            (vzm::VzSpriteActor*)vzm::NewSceneComponent(vzm::SCENE_COMPONENT_TYPE::SPRITE_ACTOR, "my sprite " + std::to_string(i));

        sprite->SetGeometry((randomValue1 + 0.5) * 3.f, (randomValue2 + 0.5f) * 3.f, 0.5, 0.5);
        sprite->SetTexture(texture2->GetVID());
        glm::fvec3 sprite_p = glm::fvec3(randomValue1 - 0.5f, randomValue2 - 0.5f, randomValue3 - 0.5f) * 7.f;
        sprite->SetPosition(__FP sprite_p);
        sprite->EnableBillboard(true);

        vzm::AppendSceneCompTo(sprite, scene);
    }

    vzm::VzSpriteActor* sprite_on_cam =
        (vzm::VzSpriteActor*)vzm::NewSceneComponent(vzm::SCENE_COMPONENT_TYPE::SPRITE_ACTOR, "my sprite in front of cam");
    sprite_on_cam->SetGeometry(2.f, 2.f, 0.5, 0.5);
    sprite_on_cam->SetTexture(texture1->GetVID());
    glm::fvec3 sprite_p2 = glm::fvec3(2.f, 1.f, -7.f);
    sprite_on_cam->SetPosition(__FP sprite_p2);
    //sprite->EnableBillboard(true);
    vzm::AppendSceneCompTo(sprite_on_cam, cam); // parent is cam

    vzm::VzTextSpriteActor* textsprite_on_cam =
        (vzm::VzTextSpriteActor*)vzm::NewSceneComponent(vzm::SCENE_COMPONENT_TYPE::TEXT_SPRITE_ACTOR, "my text-sprite in front of cam");
    textsprite_on_cam->SetText("Graphica Text", 1.f, 0.5f, 0.5f);
    sprite_p2.x = -2.f;
    textsprite_on_cam->SetPosition(__FP sprite_p2);
    //sprite->EnableBillboard(true);
    vzm::AppendSceneCompTo(textsprite_on_cam, cam); // parent is cam

    vzm::AppendSceneCompTo(actor_axis, scene);
    vzm::AppendSceneCompTo(light, scene);
    vzm::AppendSceneCompTo(cam, scene);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                done = true;
            }
            else
            {
                renderer->Render(scene, cam);
            }
        }
        if (done)
            break;
    }
    vzm::DeinitEngineLib();

    ReleaseDC(hwnd, hdc);

    return 0;
}

// Main code
int main(int, char**)
{
    return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL);\
}

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    //VID vid_scene = vzm::GetFirstVidByName("my scene");
    VID vid_renderer = vzm::GetFirstVidByName("my renderer");
    VID vid_camera = vzm::GetFirstVidByName("my camera");
    //vzm::VzScene* scene = (vzm::VzScene*)vzm::GetVzComponent(vid_scene);
    vzm::VzRenderer* renderer = (vzm::VzRenderer*)vzm::GetVzComponent(vid_renderer);
    vzm::VzCamera* camera = (vzm::VzCamera*)vzm::GetVzComponent(vid_camera);
    vzm::VzCamera::Controller* cc = nullptr;
    bool is_valid = false;
    uint32_t w = 0, h = 0;
    if (camera && renderer)
    {
        cc = camera->GetController();
        renderer->GetCanvas(&w, &h, nullptr);
        is_valid = w > 0 && h > 0;
    }

    switch (msg)
    {
    case WM_CLOSE:
    {
        //vzm::RemoveComponent(vid_camera);
        vzm::ReleaseWindowHandlerTasks(hWnd);
        break;
    }
    case WM_KEYDOWN:
        switch (wParam) {
        case 'C': {
            VID lid = vzm::GetFirstVidByName("my light");
            vzm::VzLight* light = (vzm::VzLight*)vzm::GetVzComponent(lid);
            light->SetIntensity(210000);
            break;
        }
        case 'V': {
            VID lid = vzm::GetFirstVidByName("my light");
            vzm::VzLight* light = (vzm::VzLight*)vzm::GetVzComponent(lid);
            light->SetIntensity(10000);
            break;
        }
        case 'J': {
            VID aid = vzm::GetFirstVidByName("my test model");
            vzm::VzActor* actor = (vzm::VzActor*)vzm::GetVzComponent(aid);
            VID miid = actor->GetMI();
            vzm::VzMI* mi = (vzm::VzMI*)vzm::GetVzComponent(miid);
            glm::fvec4 b_color(1, 0, 0, 1);
            mi->SetParameter("baseColor", vzm::UniformType::FLOAT4, &b_color);
            break;
        }
        case 'K': {
            VID aid = vzm::GetFirstVidByName("my test model");
            vzm::VzActor* actor = (vzm::VzActor*)vzm::GetVzComponent(aid);
            VID miid = actor->GetMI();
            vzm::VzMI* mi = (vzm::VzMI*)vzm::GetVzComponent(miid);
            glm::fvec4 b_color(0, 0, 1, 1);
            mi->SetParameter("baseColor", vzm::UniformType::FLOAT4, &b_color);
            break;
        }
        case 'L': {
            VID aid = vzm::GetFirstVidByName("my test model");
            vzm::VzActor* actor = (vzm::VzActor*)vzm::GetVzComponent(aid);
            VID miid = actor->GetMI();
            vzm::VzMaterial::MaterialKey matkey{};

            vzm::VzMI* mi = (vzm::VzMI*)vzm::GetVzComponent(miid);
            vzm::VzMaterial* material =
                (vzm::VzMaterial*)vzm::GetVzComponent(mi->GetMaterial());
            //vzm::VzMaterial* material = (vzm::VzMaterial*)vzm::NewResComponent(
            //    vzm::RES_COMPONENT_TYPE::MATERIAL, "test Material");

            // vzm::VzMI* mi = (vzm::VzMI*)vzm::NewResComponent(
            //     vzm::RES_COMPONENT_TYPE::MATERIALINSTANCE, "test Material
            //     instance");
            material->SetStandardMaterialByKey(matkey);

            mi->SetMaterial(material->GetVID());
            actor->SetMI(mi->GetVID());
            glm::fvec3 b_color(1, 0.5, 0);
            mi->SetParameter("baseColorFactor", vzm::UniformType::FLOAT3, &b_color);
            break;
        }
        case 'P': {
            VID aid = vzm::GetFirstVidByName("my test model");
            vzm::VzActor* actor = (vzm::VzActor*)vzm::GetVzComponent(aid);
            VID miid = actor->GetMI();
            vzm::VzMaterial::MaterialKey matkey{};

            vzm::VzMaterial* material = (vzm::VzMaterial*)vzm::NewResComponent(
                vzm::RES_COMPONENT_TYPE::MATERIAL, "test Material");

             vzm::VzMI* mi = (vzm::VzMI*)vzm::NewResComponent(
                 vzm::RES_COMPONENT_TYPE::MATERIALINSTANCE, "test Material instance");
            material->SetStandardMaterialByKey(matkey);

            mi->SetMaterial(material->GetVID());
            actor->SetMI(mi->GetVID());
            glm::fvec3 b_color(0.2, 0.5, 1);
            mi->SetParameter("baseColorFactor", vzm::UniformType::FLOAT3, &b_color);
            float metallic = 0.5;
            mi->SetParameter("metallicFactor", vzm::UniformType::FLOAT, &metallic);
            float roughness = 0.8;
            mi->SetParameter("roughnessFactor", vzm::UniformType::FLOAT, &roughness);
            break;
        }
        case 'U': {
            VID aid = vzm::GetFirstVidByName("my test model");
            vzm::VzActor* actor = (vzm::VzActor*)vzm::GetVzComponent(aid);
            VID miid = actor->GetMI();
            vzm::VzMaterial::MaterialKey matkey{.unlit = true};

            vzm::VzMaterial* material = (vzm::VzMaterial*)vzm::NewResComponent(
                vzm::RES_COMPONENT_TYPE::MATERIAL, "test Material");

             vzm::VzMI* mi = (vzm::VzMI*)vzm::NewResComponent(
                 vzm::RES_COMPONENT_TYPE::MATERIALINSTANCE, "test Material instance");
            material->SetStandardMaterialByKey(matkey);

            mi->SetMaterial(material->GetVID());
            actor->SetMI(mi->GetVID());
            glm::fvec3 b_color(1, 0.2, 0.6);
            mi->SetParameter("baseColorFactor", vzm::UniformType::FLOAT3, &b_color);
            break;
        }
        default:
            break;
        }
        return 0;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        if (is_valid) {
            int x = GET_X_LPARAM(lParam);
            int y = h - GET_Y_LPARAM(lParam);
            //glm::fvec3 p, v, u;
            //camera->GetWorldPose((float*)&p, (float*)&v, (float*)&u);
            //*(glm::fvec3*)cc->orbitHomePosition = p;
            //cc->UpdateControllerSettings();
            cc->GrabBegin(x, y, msg == WM_RBUTTONDOWN);
        }
        break;
    }
    case WM_MOUSEMOVE:
    {
        if (is_valid) {
            int x = GET_X_LPARAM(lParam);
            int y = h - GET_Y_LPARAM(lParam);
            cc->GrabDrag(x, y);
        }
        break;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    {
        if (cc) {
            cc->GrabEnd();
        }
        break;
    }
    case WM_MOUSEWHEEL:
    {
        if (is_valid) {
            int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            int x = GET_X_LPARAM(lParam);
            int y = h - GET_Y_LPARAM(lParam);
            cc->Scroll(x, y, -(float)zDelta);
        }
        break;
    }
    case WM_SIZE:
    {
        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;
        if (is_valid)
        {
            cc->SetViewport(w, h);
            renderer->SetCanvas(width, height, 96.f, hWnd);
            float zNearP, zFarP, fovInDegree;
            camera->GetPerspectiveProjection(&zNearP, &zFarP, &fovInDegree, nullptr);
            camera->SetPerspectiveProjection(zNearP, zFarP, fovInDegree, (float)w / (float)h);
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
