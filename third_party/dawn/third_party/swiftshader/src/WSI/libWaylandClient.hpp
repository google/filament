// Copyright 2022 The SwiftShader Authors. All Rights Reserved.
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

#ifndef libWaylandClient_hpp
#define libWaylandClient_hpp

#include <wayland-client.h>

struct LibWaylandClientExports
{
	LibWaylandClientExports() {}
	LibWaylandClientExports(void *libwl);

	int (*wl_display_dispatch)(wl_display *d) = nullptr;
	wl_registry *(*wl_display_get_registry)(wl_display *d) = nullptr;
	int (*wl_display_roundtrip)(wl_display *d) = nullptr;
	wl_callback *(*wl_display_sync)(wl_display *d) = nullptr;

	int (*wl_registry_add_listener)(wl_registry *r,
	                                const wl_registry_listener *l, void *data) = nullptr;
	void *(*wl_registry_bind)(wl_registry *r, uint32_t name, const wl_interface *i, uint32_t version) = nullptr;

	void (*wl_buffer_destroy)(wl_buffer *b) = nullptr;
	wl_shm_pool *(*wl_shm_create_pool)(wl_shm *shm, int32_t fd, int32_t size) = nullptr;
	wl_buffer *(*wl_shm_pool_create_buffer)(wl_shm_pool *p, int32_t offset, int32_t width, int32_t height, int32_t stride, uint32_t format) = nullptr;
	void (*wl_shm_pool_destroy)(wl_shm_pool *p) = nullptr;

	void (*wl_surface_attach)(wl_surface *s, wl_buffer *b, int32_t x, int32_t y) = nullptr;
	void (*wl_surface_damage)(wl_surface *s, int32_t x, int32_t y, int32_t width, int32_t height) = nullptr;
	void (*wl_surface_commit)(wl_surface *s) = nullptr;

	const wl_interface *wl_shm_interface = nullptr;
};

class LibWaylandClient
{
public:
	bool isPresent()
	{
		return loadExports() != nullptr;
	}

	LibWaylandClientExports *operator->();

private:
	LibWaylandClientExports *loadExports();
};

extern LibWaylandClient libWaylandClient;

#endif  // libWaylandClient_hpp
