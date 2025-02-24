/*!*********************************************************************************************************************
\File         MainWindows.cpp
\Title        Main Windows
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Adds the entry point for running the example on a Windows platform.
***********************************************************************************************************************/

#include "VulkanHelloAPI.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE: PostQuitMessage(0); break;
	case WM_PAINT: return 0;
	case WM_SIZE: return 0;
	default: break;
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

void createWin32WIndowSurface(VulkanHelloAPI& vulkanExample)
{
	vulkanExample.surfaceData.width = 1280.0f;
	vulkanExample.surfaceData.height = 800.0f;

	WNDCLASS winClass;
	vulkanExample.surfaceData.connection = GetModuleHandle(NULL);

	winClass.style = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc = (WNDPROC)WndProc;
	winClass.cbClsExtra = 0;
	winClass.cbWndExtra = 0;
	winClass.hInstance = vulkanExample.surfaceData.connection;
	winClass.hIcon = LoadIcon(vulkanExample.surfaceData.connection, "ICON");
	winClass.hCursor = 0;
	winClass.lpszMenuName = 0;
	winClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	winClass.lpszClassName = "Vulkan Hello API Sample";

	if (!RegisterClass(&winClass))
	{
		Log(true, "Unexpected Error - WIN32 Window Class creation Failed \n");
		exit(1);
	}

	RECT WndRect = { 0, 0, (LONG)vulkanExample.surfaceData.width, (LONG)vulkanExample.surfaceData.height };
	AdjustWindowRect(&WndRect, WS_OVERLAPPEDWINDOW, FALSE);
	vulkanExample.surfaceData.window = CreateWindowEx(0, winClass.lpszClassName, winClass.lpszClassName, WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU, 100, 100,
		WndRect.right - WndRect.left, WndRect.bottom - WndRect.top, NULL, NULL, vulkanExample.surfaceData.connection, NULL);
	if (!vulkanExample.surfaceData.window)
	{
		Log(true, "Unexpected Error - WIN32 Window creation Failed \n");
		exit(1);
	}
}

static void destroyWin32WindowSurface(VulkanHelloAPI& vulkanExample)
{
	DestroyWindow(vulkanExample.surfaceData.window);
	PostQuitMessage(0);
}

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	VulkanHelloAPI vulkanExample;
	createWin32WIndowSurface(vulkanExample);
	vulkanExample.initialize();
	vulkanExample.recordCommandBuffer();

	for (uint32_t i = 0; i < 800; ++i) { vulkanExample.drawFrame(); }
	vulkanExample.deinitialize();
	destroyWin32WindowSurface(vulkanExample);

	return 0;
}

#endif
