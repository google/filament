// Filament highlevel APIs
#include "VizEngineAPIs.h"

#include <iostream>
#include <windowsx.h>

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
    vzm::InitEngineLib(arguments);

    vzm::VzScene* scene = vzm::NewScene("my scene");
    scene->LoadIBL("../../../VisualStudio/samples/assets/ibl/lightroom_14b");

    //vzm::VzAsset* asset = vzm::LoadFileIntoAsset("", "my gltf asset");
    //vzm::VzAsset* asset = vzm::LoadFileIntoAsset("../assets/Soldier.glb", "my gltf asset");
    vzm::VzAsset* asset = vzm::LoadFileIntoAsset("D:/data/car_gltf/ioniq.gltf", "my gltf asset");

    asset->GetAnimator()->AddPlayScene(scene->GetVID());
    asset->GetAnimator()->SetPlayMode(vzm::VzAsset::Animator::PlayMode::PLAY);
    asset->GetAnimator()->SetAnimation(1);

    vzm::VzRenderer* renderer = vzm::NewRenderer("my renderer");
    renderer->SetCanvas(w, h, dpi, hwnd);
    renderer->SetVisibleLayerMask(0x4, 0x4);

    std::vector<VID> cameras;
    vzm::GetSceneCompoenentVids(vzm::SCENE_COMPONENT_TYPE::CAMERA, 0, cameras);

    vzm::VzCamera* cam = (vzm::VzCamera*)vzm::NewSceneComponent(vzm::SCENE_COMPONENT_TYPE::CAMERA, "my camera");
    {
        cam = (vzm::VzCamera*)vzm::GetVzComponent(cameras[0]);
    }
    glm::fvec3 p(0, 0, 10);
    glm::fvec3 at(0, 0, -4);
    glm::fvec3 u(0, 1, 0);
    cam->SetWorldPose((float*)&p, (float*)&at, (float*)&u);
    cam->SetPerspectiveProjection(0.1f, 1000.f, 45.f, (float)w / (float)h);
    vzm::VzCamera::Controller* cc = cam->GetController();
    *(glm::fvec3*)cc->orbitHomePosition = p;
    cc->UpdateControllerSettings();
    cc->SetViewport(w, h);


    vzm::VzLight* light = (vzm::VzLight*)vzm::NewSceneComponent(vzm::SCENE_COMPONENT_TYPE::LIGHT, "my light", 0);

    std::vector<VID> root_vids = asset->GetGLTFRoots();
    if (root_vids.size() > 0)
    {
        vzm::AppendSceneCompVidTo(root_vids[0], scene->GetVID());
    }
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
        }
        if (done)
            break;
        renderer->Render(scene, cam);
    }
    vzm::DeinitEngineLib();

    ReleaseDC(hwnd, hdc);

    return 0;
}

// Main code
int main(int, char**)
{
    return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL); 
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
    RECT rc;
    GetClientRect(hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    is_valid &= width > 0 && height > 0;

    switch (msg)
    {
    case WM_CLOSE:
    {
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
            glm::fvec4 b_color(1.f);
            mi->SetMaterialProperty(vzm::VzMI::MProp::BASE_COLOR, { 1.f, 0, 0, 1.f });
            break;
        }
        case 'K': {
            VID aid = vzm::GetFirstVidByName("my test model");
            vzm::VzActor* actor = (vzm::VzActor*)vzm::GetVzComponent(aid);
            VID miid = actor->GetMI();
            vzm::VzMI* mi = (vzm::VzMI*)vzm::GetVzComponent(miid);
            glm::fvec4 b_color(1.f);
            mi->SetMaterialProperty(vzm::VzMI::MProp::BASE_COLOR, { 1.f, 0, 0, 0.4f });
            break;
        }
        case 'L': {
            VID aid = vzm::GetFirstVidByName("my test model");
            vzm::VzActor* actor = (vzm::VzActor*)vzm::GetVzComponent(aid);
            VID miid = actor->GetMI();
            vzm::VzMI* mi = (vzm::VzMI*)vzm::GetVzComponent(miid);
            glm::fvec4 b_color(1.f);
            mi->SetMaterialProperty(vzm::VzMI::MProp::BASE_COLOR, { 1.f, 1.f, 0, 0.4f });
            break;
        }
        default:
            break;
        }
        return 0;
    case WM_LBUTTONDOWN:
    {
        if (is_valid) {
            int x = GET_X_LPARAM(lParam);
            int y = h - GET_Y_LPARAM(lParam);
            //glm::fvec3 p, v, u;
            //camera->GetWorldPose((float*)&p, (float*)&v, (float*)&u);
            //*(glm::fvec3*)cc->orbitHomePosition = p;
            //cc->UpdateControllerSettings();
            cc->GrabBegin(x, y, false);
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
    {
        if (is_valid) {
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
