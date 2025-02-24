/*!
\brief Contains an implementation of pvr::platform::ShellOS for Microsoft Windows systems.
\file PVRShell/OS/Windows/ShellOS.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRShell/OS/ShellOS.h"
#include "PVRCore/stream/FilePath.h"
#include "PVRCore/Log.h"
#include "PVRShell/OS/Windows/WindowsOSData.h"
#include <WindowsX.h>

#define WINDOW_CLASS "PVRShellOS"

namespace pvr {
namespace platform {
class InternalOS
{
public:
	OSDATA osdata;
	HDC hDC;
	HWND hWnd;

	InternalOS() : hDC(0), hWnd(0) {}
};

// Setup the capabilities.
const ShellOS::Capabilities ShellOS::_capabilities = { Capability::Immutable, Capability::Immutable };

ShellOS::ShellOS(OSApplication hInstance, OSDATA osdata) : _instance(hInstance)
{
	_OSImplementation = std::make_unique<InternalOS>();

	if (osdata)
	{
		_OSImplementation->osdata = new WindowsOSData;

		if (_OSImplementation->osdata) { memcpy(_OSImplementation->osdata, osdata, sizeof(WindowsOSData)); }
	}
	else
	{
		_OSImplementation->osdata = 0;
	}
}

ShellOS::~ShellOS()
{
	if (_OSImplementation) { delete _OSImplementation->osdata; }
}

void ShellOS::updatePointingDeviceLocation()
{
	POINT point;
	if (GetCursorPos(&point) && ScreenToClient(_OSImplementation->hWnd, &point))
	{ _shell->updatePointerPosition(PointerLocation(static_cast<int16_t>(point.x), static_cast<int16_t>(point.y))); }
}

static Keys mapKeyWparamToPvrKey(WPARAM wParam) { return static_cast<Keys>(wParam); }

// The window procedure receives and handles messages from MS Windows.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static uint16_t capturer = 0;
#if defined(UNDER_CE)
	Shell* theShell = reinterpret_cast<Shell*>(GetWindowLong(hWnd, GWLP_USERDATA));
#else
	Shell* theShell = reinterpret_cast<Shell*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
#endif

	switch (message)
	{
	case WM_CREATE:
	{
#if defined(UNDER_CE)
		CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)(LONG_PTR)pCreate->lpCreateParams); // Not ideal, but WinCE doesn't have SetWindowLongPtr.
#else
		CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
#endif
		break;
	}
	case WM_PAINT: break;
	case WM_DESTROY: break;
	case WM_CLOSE: theShell->onSystemEvent(SystemEvent::SystemEvent_Quit); return 0;
	case WM_QUIT: return 0;
	case WM_MOVE:
	{
	}
	break;
	case WM_LBUTTONDOWN:
		if (hWnd != GetCapture())
		{
			capturer = 1;
			SetCapture(hWnd);
		}
		theShell->onPointingDeviceDown(0);
		break;
	case WM_RBUTTONDOWN:
		if (hWnd != GetCapture())
		{
			capturer = 2;
			SetCapture(hWnd);
		}
		theShell->onPointingDeviceDown(1);
		break;
	case WM_MBUTTONDOWN:
		if (hWnd != GetCapture())
		{
			capturer = 3;
			SetCapture(hWnd);
		}
		theShell->onPointingDeviceDown(2);
		break;
	case WM_LBUTTONUP:
		if (capturer == 1 && hWnd == GetCapture()) // Only send an event if we're still capturing.
		{ ReleaseCapture(); }
		theShell->onPointingDeviceUp(0);
		break;
	case WM_RBUTTONUP:
		if (capturer == 2 && hWnd == GetCapture()) // Only send an event if we're still capturing.
		{ ReleaseCapture(); }
		theShell->onPointingDeviceUp(1);
		break;
	case WM_MBUTTONUP:
	{
		if (capturer == 3 && hWnd == GetCapture()) // Only send an event if we're still capturing.
		{ ReleaseCapture(); }
		theShell->onPointingDeviceUp(2);
		break;
	}
	case WM_MOUSEMOVE:
	{
		break;
	}
	case WM_KEYDOWN: theShell->onKeyDown(mapKeyWparamToPvrKey(wParam)); break;
	case WM_KEYUP: theShell->onKeyUp(mapKeyWparamToPvrKey(wParam)); break;
	default: break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*This function and its usage is only necessary if you want this code
to be compatible with Win32 systems prior to the 'RegisterClassEx'
function that was added to Windows 95. It is important to call this function
so that the application will get 'well formed' small icons associated
with it.*/
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, "ICON");
	wc.hCursor = 0;
	wc.lpszMenuName = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = WINDOW_CLASS;

	return RegisterClass(&wc);
}

bool ShellOS::init(DisplayAttributes& /*data*/)
{
	if (!_OSImplementation) { return false; }

	// Register our class.
	MyRegisterClass(static_cast<HINSTANCE>(_instance));

	// Construct our read and write path.
	{
		char moduleFilename[MAX_PATH] = "";

		if (GetModuleFileName(NULL, moduleFilename, sizeof(moduleFilename) - 1) == 0) { return false; }

		FilePath filepath(moduleFilename);
		setApplicationName(filepath.getFilenameNoExtension());
		_writePath = filepath.getDirectory() + FilePath::getDirectorySeparator();
		_readPaths.clear();
		_readPaths.emplace_back(filepath.getDirectory() + FilePath::getDirectorySeparator());
		_readPaths.emplace_back(std::string(".") + FilePath::getDirectorySeparator());
		_readPaths.emplace_back(filepath.getDirectory() + FilePath::getDirectorySeparator() + "Assets" + FilePath::getDirectorySeparator());
		_readPaths.emplace_back(filepath.getDirectory() + FilePath::getDirectorySeparator() + "Assets_" + filepath.getFilenameNoExtension() + FilePath::getDirectorySeparator());
	}

	return true;
}

bool ShellOS::initializeWindow(DisplayAttributes& data)
{
	HWND hWnd;
	POINT p;
	MONITORINFO sMInfo;
	RECT winRect;

	// Retrieve the monitor information. MonitorFromWindow() doesn't work, because the window hasn't been created yet.
	{
		HMONITOR hMonitor;

		p.x = data.x;
		p.y = data.y;
		if (data.x == DisplayAttributes::PosDefault) { p.x = 0; }
		if (data.y == DisplayAttributes::PosDefault) { p.y = 0; }

		hMonitor = MonitorFromPoint(p, MONITOR_DEFAULTTONEAREST);
		sMInfo.cbSize = sizeof(sMInfo);
		GetMonitorInfo(hMonitor, &sMInfo);
	}

	// Create our window.
	if (data.fullscreen)
	{
		data.x = data.y = 0;
		data.width = sMInfo.rcMonitor.right - sMInfo.rcMonitor.left;
		data.height = sMInfo.rcMonitor.bottom - sMInfo.rcMonitor.top;

		hWnd = CreateWindow(WINDOW_CLASS, data.windowTitle.c_str(), WS_VISIBLE | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, data.width, data.height, NULL, NULL,
			static_cast<HINSTANCE>(_instance), this->_shell.get());

		SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & ~WS_CAPTION);
		SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	}
	else
	{
		int x, y;

		SetRect(&winRect, 0, 0, static_cast<int>(data.width), static_cast<int>(data.height));
		AdjustWindowRectEx(&winRect, WS_CAPTION | WS_SYSMENU, false, 0);

		if (data.x == DisplayAttributes::PosDefault) { x = CW_USEDEFAULT; }
		else
		{
			x = data.x;
		}

		if (data.y == DisplayAttributes::PosDefault) { y = CW_USEDEFAULT; }
		else
		{
			y = data.y;
		}

		hWnd = CreateWindow(WINDOW_CLASS, data.windowTitle.c_str(), WS_VISIBLE | WS_CAPTION | WS_SYSMENU, x, y, winRect.right - winRect.left, winRect.bottom - winRect.top, NULL,
			NULL, static_cast<HINSTANCE>(_instance), this->_shell.get());
	}

	if (!hWnd) { return false; }

	ShowWindow(hWnd, static_cast<WindowsOSData*>(_OSImplementation->osdata)->cmdShow);
	UpdateWindow(hWnd);
	SetForegroundWindow(hWnd);

	_OSImplementation->hWnd = hWnd;
	_OSImplementation->hDC = GetDC(hWnd);

	return true;
}

void ShellOS::releaseWindow()
{
	if (_OSImplementation)
	{
		ReleaseDC(_OSImplementation->hWnd, _OSImplementation->hDC);
		DestroyWindow(_OSImplementation->hWnd);
		_OSImplementation->hWnd = 0;
		_OSImplementation->hDC = 0;
	}
}

OSApplication ShellOS::getApplication() const { return _instance; }

OSConnection ShellOS::getConnection() const { return nullptr; }

OSDisplay ShellOS::getDisplay() const { return _OSImplementation->hDC; }

OSWindow ShellOS::getWindow() const { return _OSImplementation->hWnd; }

bool ShellOS::handleOSEvents()
{
	MSG msg;

	// Process the message queue.
	while (PeekMessage(&msg, _OSImplementation->hWnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

bool ShellOS::isInitialized() { return _OSImplementation && _OSImplementation->hDC; }

bool ShellOS::popUpMessage(const char* title, const char* message, ...) const
{
	if (!title) { title = ""; }
	if (!message) { message = ""; }

	va_list arg;
	char buf[1024];
	memset(buf, 0, sizeof(buf));

	va_start(arg, message);
#if defined(_UNICODE)
	_vsnwprintf(buf, 1023, message, arg);
#else
	_vsnprintf(buf, 1023, message, arg);
#endif
	va_end(arg);

	return (MessageBox(NULL, buf, title, MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND) != 0);
}
} // namespace platform
} // namespace pvr
//!\endcond
