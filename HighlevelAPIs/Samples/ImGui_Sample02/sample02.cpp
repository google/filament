// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

// Filament highlevel APIs
#include "VizEngineAPIs.h"

#include <iostream>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>


#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/vector_angle.hpp"

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    //WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    //::RegisterClassExW(&wc);
    //HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Windows Test", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, nullptr, nullptr, wc.hInstance, nullptr);
    //// Show the window
    //::ShowWindow(hwnd, SW_SHOWDEFAULT);
    //::UpdateWindow(hwnd);

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
    VID sid = vzm::NewScene("my scene");
    VID aid = vzm::LoadTestModel("my test model");
    vzm::VzCamera* cam;
    VID cid = vzm::NewSceneComponent(vzm::SCENE_COMPONENT_TYPE::CAMERA, "my camera", 0, SCPP(cam));
    cam->SetCanvas(w, h, dpi, hwnd);
    glm::fvec3 p(0, 0, 100);
    glm::fvec3 at(0, 0, -4);
    glm::fvec3 u(0, 1, 0);
    cam->SetWorldPose((float*)&p, (float*)&at, (float*)&u);
    cam->SetPerspectiveProjection(0.1f, 1000.f, 45.f, (float)h / (float)w);
    //vzm::VzLight* light;
    //VID lid = vzm::NewSceneComponent(vzm::SCENE_COMPONENT_TYPE::LIGHT, "my light", 0, SCPP(light));
    vzm::AppendSceneComponentTo(aid, sid);
    //vzm::AppendSceneComponentTo(lid, sid);
    vzm::AppendSceneComponentTo(cid, sid);

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
                vzm::DeinitEngineLib();
                done = true;
            }
            else if (msg.message == WM_CLOSE)
            {
                VID cid = vzm::GetFirstVidByName("my camera");
                vzm::RemoveComponent(cid);
            }
            else
            {
                vzm::Render(cid);
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
    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wParam) {
        case 'A': {
            VID cid = vzm::GetFirstVidByName("my camera");
            vzm::RemoveComponent(cid);
            break;
        }
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
