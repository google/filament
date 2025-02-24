/*!*********************************************************************************************************************
\File         MainLinuxX11.cpp
\Title        Main Linux X11
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Adds the entry point for running the example on a Linux X11 platform.
***********************************************************************************************************************/

#include "VulkanHelloAPI.h"

#ifdef VK_USE_PLATFORM_XLIB_KHR

void createXlibWindowSurface(VulkanHelloAPI& vulkanExample)
{
	vulkanExample.surfaceData.width = 1280.0f;
	vulkanExample.surfaceData.height = 800.0f;
	vulkanExample.surfaceData.display = XOpenDisplay(0);

	if (vulkanExample.surfaceData.display != nullptr)
	{
		// Get the default screen for the display.
		int defaultScreen = XDefaultScreen(vulkanExample.surfaceData.display);

		// Get the default depth of the display.
		int defaultDepth = DefaultDepth(vulkanExample.surfaceData.display, defaultScreen);

		// Select a visual info.
		std::unique_ptr<XVisualInfo> visualInfo(new XVisualInfo);
		XMatchVisualInfo(vulkanExample.surfaceData.display, defaultScreen, defaultDepth, TrueColor, visualInfo.get());
		if (!visualInfo.get())
		{
			printf("Error: Unable to acquire visual\n");
			exit(0);
		}

		// Get the root window for the display and default screen.
		Window rootWindow = RootWindow(vulkanExample.surfaceData.display, defaultScreen);

		// Create a colour map from the display, root window, and visual info.
		Colormap colorMap = XCreateColormap(vulkanExample.surfaceData.display, rootWindow, visualInfo->visual, AllocNone);

		// Now set up the final window by specifying some attributes.
		XSetWindowAttributes windowAttributes;

		// Set the colour map that was just created.
		windowAttributes.colormap = colorMap;

		// Set events that will be handled by the app, add to these for other events.
		windowAttributes.event_mask = StructureNotifyMask | ExposureMask | ButtonPressMask;

		// Create the window.
		vulkanExample.surfaceData.window = XCreateWindow(vulkanExample.surfaceData.display, // The display used to create the window.
			rootWindow, // The parent (root) window - the desktop.
			0, // The horizontal (x) origin of the window.
			0, // The vertical (y) origin of the window.
			vulkanExample.surfaceData.width, // The width of the window.
			vulkanExample.surfaceData.height, // The height of the window.
			0, // Border size - set it to zero.
			visualInfo->depth, // Depth from the visual info.
			InputOutput, // Window type - this specifies InputOutput.
			visualInfo->visual, // Visual to use.
			CWEventMask | CWColormap, // Mask specifying these have been defined in the window attributes.
			&windowAttributes); // Pointer to the window attribute structure.

		// Make the window viewable by mapping it to the display.
		XMapWindow(vulkanExample.surfaceData.display, vulkanExample.surfaceData.window);

		// Set the window title.
		XStoreName(vulkanExample.surfaceData.display, vulkanExample.surfaceData.window, "VulkanHelloAPI");

		// Setup the window manager protocols to handle window deletion events.
		Atom windowManagerDelete = XInternAtom(vulkanExample.surfaceData.display, "WM_DELETE_WINDOW", True);
		XSetWMProtocols(vulkanExample.surfaceData.display, vulkanExample.surfaceData.window, &windowManagerDelete, 1);
	}
}

int main(int /*argc*/, char** /*argv*/)
{
	VulkanHelloAPI vulkanExample;
	createXlibWindowSurface(vulkanExample);
	vulkanExample.initialize();
	vulkanExample.recordCommandBuffer();

	for (uint32_t i = 0; i < 800; ++i)
	{
		// Check for messages from the windowing system.
		int numberOfMessages = XPending(vulkanExample.surfaceData.display);
		for (int i = 0; i < numberOfMessages; i++)
		{
			XEvent event;
			XNextEvent(vulkanExample.surfaceData.display, &event);

			switch (event.type)
			{
			// Exit on window close.
			case ClientMessage:
			// Exit on mouse click.
			case ButtonPress:
			case DestroyNotify: return false;
			default: break;
			}
		}
		vulkanExample.drawFrame();
	}

	vulkanExample.deinitialize();

	// Destroy the window.
	if (vulkanExample.surfaceData.window) { XDestroyWindow(vulkanExample.surfaceData.display, vulkanExample.surfaceData.window); }

	// Release the display.
	if (vulkanExample.surfaceData.display) { XCloseDisplay(vulkanExample.surfaceData.display); }

	// Clean up the instance.
	// Vulkan can register a callback with Xlib. When the application calls
	// XCloseDisplay, this callback is called and will segfault if the driver had already been unloaded,
	// which could happen when the Vulkan instance is destroyed.
	vk::DestroyInstance(vulkanExample.appManager.instance, nullptr);
	return 0;
}

#endif
