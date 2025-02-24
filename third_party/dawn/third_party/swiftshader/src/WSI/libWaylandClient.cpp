// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "libWaylandClient.hpp"

#include "System/SharedLibrary.hpp"

#include <memory>

LibWaylandClientExports::LibWaylandClientExports(void *libwl)
{
	getFuncAddress(libwl, "wl_display_dispatch", &wl_display_dispatch);
	getFuncAddress(libwl, "wl_display_get_registry", &wl_display_get_registry);
	getFuncAddress(libwl, "wl_display_roundtrip", &wl_display_roundtrip);
	getFuncAddress(libwl, "wl_display_sync", &wl_display_sync);

	getFuncAddress(libwl, "wl_registry_add_listener", &wl_registry_add_listener);
	getFuncAddress(libwl, "wl_registry_bind", &wl_registry_bind);

	getFuncAddress(libwl, "wl_buffer_destroy", &wl_buffer_destroy);
	getFuncAddress(libwl, "wl_shm_create_pool", &wl_shm_create_pool);
	getFuncAddress(libwl, "wl_shm_pool_create_buffer", &wl_shm_pool_create_buffer);
	getFuncAddress(libwl, "wl_shm_pool_destroy", &wl_shm_pool_destroy);

	getFuncAddress(libwl, "wl_surface_attach", &wl_surface_attach);
	getFuncAddress(libwl, "wl_surface_damage", &wl_surface_damage);
	getFuncAddress(libwl, "wl_surface_commit", &wl_surface_commit);

	wl_shm_interface = reinterpret_cast<const wl_interface *>(getProcAddress(libwl, "wl_shm_interface"));
}

LibWaylandClientExports *LibWaylandClient::operator->()
{
	return loadExports();
}

LibWaylandClientExports *LibWaylandClient::loadExports()
{
	static LibWaylandClientExports exports = [] {
		void *libwl = nullptr;

		if(getProcAddress(RTLD_DEFAULT, "wl_display_dispatch"))  // Search the global scope for pre-loaded Wayland client library.
		{
			libwl = RTLD_DEFAULT;
		}
		else
		{
			libwl = loadLibrary("libwayland-client.so.0");
		}

		return LibWaylandClientExports(libwl);
	}();

	return exports.wl_display_dispatch ? &exports : nullptr;
}

LibWaylandClient libWaylandClient;
