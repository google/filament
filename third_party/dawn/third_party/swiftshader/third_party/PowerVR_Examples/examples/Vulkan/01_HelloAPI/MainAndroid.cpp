/*!*********************************************************************************************************************
\File         MainAndroid.cpp
\Title        Main Android
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Adds the entry point for running the example on an Android platform.
***********************************************************************************************************************/

#include "VulkanHelloAPI.h"

#ifdef VK_USE_PLATFORM_ANDROID_KHR

static bool focus = false;
static bool init = false;

static int32_t processInput(struct android_app* app, AInputEvent* event) { return 0; }

static void processCommand(struct android_app* androidApp, int32_t cmd)
{
	VulkanHelloAPI* app = (VulkanHelloAPI*)androidApp->userData;
	switch (cmd)
	{
	case APP_CMD_INIT_WINDOW:
	{
		// The window is being shown, get it ready.
		if (androidApp->window != NULL)
		{
			usleep(100000);
			focus = true;

			app->surfaceData.window = androidApp->window;
			app->initialize();
			app->recordCommandBuffer();
			app->drawFrame();

			init = true;
		}
		break;
	}
	case APP_CMD_TERM_WINDOW:
	{
		// The window is being hidden or closed, clean it up.
		break;
	}
	case APP_CMD_GAINED_FOCUS:
	{
		focus = true;
		LOGI("Waking up");
		break;
	}
	case APP_CMD_LOST_FOCUS:
	{
		focus = false;
		LOGI("Going to sleep");
		break;
	}
	}
}

static void processTerminate() {}

// Typical Android NativeActivity entry function.
void android_main(struct android_app* state)
{
	VulkanHelloAPI VulkanExample;

	// Make sure glue is not stripped.
	state->userData = &VulkanExample;
	state->onAppCmd = processCommand;
	state->onInputEvent = processInput;

	while (1)
	{
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;

		while ((ident = ALooper_pollAll(focus ? 0 : -1, NULL, &events, (void**)&source)) >= 0)
		{
			// Process this event.
			if (source != NULL) { source->process(state, source); }

			// Check for exiting.
			if (state->destroyRequested != 0)
			{
				processTerminate();
				VulkanExample.deinitialize();
				return;
			}
		}

		if (focus && init) { VulkanExample.drawFrame(); }
	}
}
#endif