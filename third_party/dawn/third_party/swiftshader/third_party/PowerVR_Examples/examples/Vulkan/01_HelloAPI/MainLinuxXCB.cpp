/*!*********************************************************************************************************************
\File         MainLinuxX11.cpp
\Title        Main Linux X11
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Adds the entry point for running the example on a Linux X11 platform.
***********************************************************************************************************************/

#include "VulkanHelloAPI.h"

#ifdef VK_USE_PLATFORM_XCB_KHR

void createXcbWindowSurface(VulkanHelloAPI& vulkanExample)
{
	vulkanExample.surfaceData.width = 1280.0f;
	vulkanExample.surfaceData.height = 800.0f;

	vulkanExample.surfaceData.connection = xcb_connect(nullptr, nullptr);

	if (!vulkanExample.surfaceData.connection || xcb_connection_has_error(vulkanExample.surfaceData.connection))
	{
		printf("Failed to open XCB connection");
		exit(0);
	}

	// Retrieve data returned by the server when the connection was initialized
	const xcb_setup_t* setup = xcb_get_setup(vulkanExample.surfaceData.connection);

	int screenCount = xcb_setup_roots_length(setup);
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);

	for (uint32_t i = 0; i < screenCount; ++screenCount)
	{
		// Else retrieve the first valid screen
		if (iter.data)
		{
			vulkanExample.surfaceData.screen = iter.data;
			break;
		}
		xcb_screen_next(&iter);
	}

	if (!vulkanExample.surfaceData.screen)
	{
		printf("Failed to find a valid XCB screen");
		exit(0);
	}

	// Allocate an XID for the window
	vulkanExample.surfaceData.window = xcb_generate_id(vulkanExample.surfaceData.connection);

	if (!vulkanExample.surfaceData.window)
	{
		printf("Failed to allocate an id for an XCB window");
		exit(0);
	}

	// XCB_CW_BACK_PIXEL - A pixemap of undefined size filled with the specified background pixel is used for the background. Range-checking is not performed.
	// XCB_CW_BORDER_PIXMAP - Specifies the pixel color used for the border
	// XCB_CW_EVENT_MASK - The event-mask defines which events the client is interested in for this window
	uint32_t valueMask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXMAP | XCB_CW_EVENT_MASK;
	uint32_t valueList[3] = { vulkanExample.surfaceData.screen->black_pixel, 0,
		XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_POINTER_MOTION };

	xcb_create_window(vulkanExample.surfaceData.connection, XCB_COPY_FROM_PARENT, vulkanExample.surfaceData.window, vulkanExample.surfaceData.screen->root, 0, 0,
		vulkanExample.surfaceData.width, vulkanExample.surfaceData.height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, vulkanExample.surfaceData.screen->root_visual, valueMask, valueList);

	// Setup code that will send a notification when the window is destroyed.
	xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(vulkanExample.surfaceData.connection, 1, 12, "WM_PROTOCOLS");
	xcb_intern_atom_cookie_t wm_delete_window_cookie = xcb_intern_atom(vulkanExample.surfaceData.connection, 0, 16, "WM_DELETE_WINDOW");
	xcb_intern_atom_reply_t* wm_protocols_reply = xcb_intern_atom_reply(vulkanExample.surfaceData.connection, wm_protocols_cookie, 0);
	xcb_intern_atom_reply_t* wm_delete_window_reply = xcb_intern_atom_reply(vulkanExample.surfaceData.connection, wm_delete_window_cookie, 0);

	vulkanExample.surfaceData.deleteWindowAtom = wm_delete_window_reply->atom;
	xcb_change_property(vulkanExample.surfaceData.connection, XCB_PROP_MODE_REPLACE, vulkanExample.surfaceData.window, wm_protocols_reply->atom, XCB_ATOM_ATOM, 32, 1,
		&wm_delete_window_reply->atom);

	free(wm_protocols_reply);
	free(wm_delete_window_reply);

	// Change the title of the window to match the example title
	const char* title = "VulkanHelloAPI";
	xcb_change_property(vulkanExample.surfaceData.connection, XCB_PROP_MODE_REPLACE, vulkanExample.surfaceData.window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(title), title);

	xcb_map_window(vulkanExample.surfaceData.connection, vulkanExample.surfaceData.window);
	xcb_flush(vulkanExample.surfaceData.connection);
}

int main(int /*argc*/, char** /*argv*/)
{
	VulkanHelloAPI vulkanExample;
	createXcbWindowSurface(vulkanExample);
	vulkanExample.initialize();
	vulkanExample.recordCommandBuffer();

	for (uint32_t i = 0; i < 800; ++i)
	{
		xcb_generic_event_t* genericEvent;
		while ((genericEvent = xcb_poll_for_event(vulkanExample.surfaceData.connection)))
		{
			const uint8_t event_code = (genericEvent->response_type & 0x7f);
			if (event_code == XCB_CLIENT_MESSAGE)
			{
				const xcb_client_message_event_t* clientMessageEvent = (const xcb_client_message_event_t*)genericEvent;
				if (clientMessageEvent->data.data32[0] == vulkanExample.surfaceData.deleteWindowAtom) { return false; }
			}
			switch (genericEvent->response_type & ~0x80)
			{
			case XCB_DESTROY_NOTIFY: { return false;
			}
			default: break;
			}

			free(genericEvent);
		}
		vulkanExample.drawFrame();
	}

	vulkanExample.deinitialize();

	xcb_destroy_window(vulkanExample.surfaceData.connection, vulkanExample.surfaceData.window);
	xcb_disconnect(vulkanExample.surfaceData.connection);

	// Clean up our instance.
	// Vulkan can register a callback with Xlib. When the application calls
	// XCloseDisplay, this callback is called and will segfault if the driver had already been unloaded,
	// which could happen when the Vulkan instance is destroyed.
	vk::DestroyInstance(vulkanExample.appManager.instance, nullptr);
	return 0;
}

#endif
