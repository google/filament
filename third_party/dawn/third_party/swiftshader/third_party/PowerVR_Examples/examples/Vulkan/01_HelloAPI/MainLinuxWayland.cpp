/*!*********************************************************************************************************************
\File         MainWayland.cpp
\Title        Main Linux Wayland
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Adds the entry point for running the example on an Linux Wayland platform.
***********************************************************************************************************************/

#include "VulkanHelloAPI.h"

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

static void pointer_handle_enter(void* data, struct wl_pointer* pointer, uint32_t serial, struct wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy) {}
static void pointer_handle_leave(void* data, struct wl_pointer* pointer, uint32_t serial, struct wl_surface* surface) {}
static void pointer_handle_motion(void* data, struct wl_pointer* pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy) {}
static void pointer_handle_button(void* data, struct wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {}
static void pointer_handle_axis(void* data, struct wl_pointer* pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {}

static const struct wl_pointer_listener pointerListener = {
	.enter = pointer_handle_enter,
	.leave = pointer_handle_leave,
	.motion = pointer_handle_motion,
	.button = pointer_handle_button,
	.axis = pointer_handle_axis,
};

static void seat_handle_capabilities(void* data, struct wl_seat* seat, uint32_t caps)
{
	if (caps & WL_SEAT_CAPABILITY_POINTER) { wl_pointer_add_listener(wl_seat_get_pointer(seat), &pointerListener, data); }
}

static void seat_handle_name(void* data, struct wl_seat* seat, const char* name) {}

static void handle_ping(void* data, struct wl_shell_surface* surface, uint32_t serial) { wl_shell_surface_pong(surface, serial); }

static void handle_configure(void* data, struct wl_shell_surface* surface, uint32_t edges, int32_t width, int32_t height) {}
static void handle_popup_done(void* data, struct wl_shell_surface* surface) {}

static const struct wl_seat_listener seatListener = {
	.capabilities = seat_handle_capabilities,
	.name = seat_handle_name,
};

static void registry_handle_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
	SurfaceData* surfaceData = (SurfaceData*)data;

	if (strcmp(interface, "wl_compositor") == 0) { surfaceData->wlCompositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 1); }
	else if (strcmp(interface, "wl_shell") == 0)
	{
		surfaceData->wlShell = (wl_shell*)wl_registry_bind(registry, name, &wl_shell_interface, 1);
	}
	else if (strcmp(interface, "wl_seat") == 0)
	{
		surfaceData->wlSeat = (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, 1);
		wl_seat_add_listener(surfaceData->wlSeat, &seatListener, data);
	}
}

static void registry_handle_global_remove(void* data, struct wl_registry* registry, uint32_t name) {}

static const struct wl_shell_surface_listener shellSurfaceListener = { .ping = handle_ping, .configure = handle_configure, .popup_done = handle_popup_done };

static const struct wl_registry_listener registryListener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};

void createWaylandWindowSurface(VulkanHelloAPI& vulkanExample)
{
	vulkanExample.surfaceData.width = 1280;
	vulkanExample.surfaceData.height = 800;

	vulkanExample.surfaceData.display = wl_display_connect(NULL);
	if (!vulkanExample.surfaceData.display)
	{
		LOGE("Could not open Wayland display connection\n");
		exit(1);
	}

	vulkanExample.surfaceData.wlRegistry = wl_display_get_registry(vulkanExample.surfaceData.display);
	if (!vulkanExample.surfaceData.wlRegistry)
	{
		LOGE("Could not get Wayland registry\n");
		exit(1);
	}

	wl_registry_add_listener(vulkanExample.surfaceData.wlRegistry, &registryListener, &vulkanExample.surfaceData);
	wl_display_dispatch(vulkanExample.surfaceData.display);
	vulkanExample.surfaceData.surface = wl_compositor_create_surface(vulkanExample.surfaceData.wlCompositor);

	if (!vulkanExample.surfaceData.surface)
	{
		LOGE("Could create Wayland compositor surface\n");
		exit(1);
	}

	vulkanExample.surfaceData.wlShellSurface = wl_shell_get_shell_surface(vulkanExample.surfaceData.wlShell, vulkanExample.surfaceData.surface);
	if (!vulkanExample.surfaceData.wlShellSurface)
	{
		LOGE("Could create Wayland shell surface\n");
		exit(1);
	}

	wl_shell_surface_add_listener(vulkanExample.surfaceData.wlShellSurface, &shellSurfaceListener, &vulkanExample.surfaceData);
	wl_shell_surface_set_title(vulkanExample.surfaceData.wlShellSurface, "HelloApiVk");
	wl_shell_surface_set_toplevel(vulkanExample.surfaceData.wlShellSurface);
}

void releaseWaylandConnection(VulkanHelloAPI& vulkanExample)
{
	wl_shell_surface_destroy(vulkanExample.surfaceData.wlShellSurface);
	wl_surface_destroy(vulkanExample.surfaceData.surface);
	if (vulkanExample.surfaceData.wlPointer) { wl_pointer_destroy(vulkanExample.surfaceData.wlPointer); }
	wl_seat_destroy(vulkanExample.surfaceData.wlSeat);
	wl_compositor_destroy(vulkanExample.surfaceData.wlCompositor);
	wl_registry_destroy(vulkanExample.surfaceData.wlRegistry);
	wl_display_disconnect(vulkanExample.surfaceData.display);
}

int main(int /*argc*/, char** /*argv*/)
{
	VulkanHelloAPI vulkanExample;
	createWaylandWindowSurface(vulkanExample);
	vulkanExample.initialize();
	vulkanExample.recordCommandBuffer();

	for (uint32_t i = 0; i < 800; ++i)
	{
		wl_display_dispatch_pending(vulkanExample.surfaceData.display);
		vulkanExample.drawFrame();
	}

	vulkanExample.deinitialize();

	// Clean up the Wayland
	releaseWaylandConnection(vulkanExample);

	return 0;
}
#endif
