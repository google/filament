#define _ITERATOR_DEBUG_LEVEL 0 // this is for using STL containers as our standard parameters

#include <windows.h>
#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/View.h>
#include <filament/Camera.h>
#include <filament/SwapChain.h>
#include <utils/EntityManager.h>
#include <filamat/MaterialBuilder.h>
#include <filament/Viewport.h>
#include <utils/EntityManager.h>

using namespace filament;

Engine* engine;
Renderer* renderer;
SwapChain* swapChain;
View* view;
Scene* scene;
Camera* camera;
utils::Entity cameraEntity;

// Win32 창 프로시저
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow) {
    // Win32 창 생성
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"Filament Win32 Window Class";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        L"Filament Win32 Example",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    // Filament 엔진 초기화
    engine = Engine::create();
    swapChain = engine->createSwapChain((void*)hwnd);
    renderer = engine->createRenderer();
    scene = engine->createScene();
    view = engine->createView();
    cameraEntity = utils::EntityManager::get().create();
    camera = engine->createCamera(cameraEntity);

    // 카메라 설정
    camera->setProjection(60.0f, 800.0f / 600.0f, 0.1f, 100.0f);
    camera->lookAt({ 0, 0, 4 }, { 0, 0, 0 });

    // 뷰 설정
    view->setScene(scene);
    view->setCamera(camera);
    view->setViewport(Viewport(0, 0, 800, 600));
    bool shouldClose = false;
    // 메인 루프

    for (int i = 0; i < 10; i++)
        if (renderer->beginFrame(swapChain)) {
            renderer->render(view);
            renderer->endFrame();
        }

    while (!shouldClose) {
        MSG msg = {};
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                shouldClose = true;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!shouldClose) {
            // Filament 렌더링
            if (renderer->beginFrame(swapChain)) {
                renderer->render(view);
                renderer->endFrame();
            }
        }
    }
    // 정리
    if (engine) {
        engine->destroy(view);
        engine->destroy(renderer);
        engine->destroy(scene);
        engine->destroyCameraComponent(cameraEntity);

        engine->destroy(swapChain);
        Engine::destroy(&engine);
        engine = nullptr;
    }

    return 0;
}
// Main code
int main(int, char**)
{
    return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL); \
}
