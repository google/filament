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

#ifndef libXCB_hpp
#define libXCB_hpp

#include <xcb/shm.h>
#include <xcb/xcb.h>

struct LibXcbExports
{
	LibXcbExports() {}
	LibXcbExports(void *libxcb, void *libshm);

	xcb_void_cookie_t (*xcb_create_gc)(xcb_connection_t *c, xcb_gcontext_t cid, xcb_drawable_t drawable, uint32_t value_mask, const void *value_list) = nullptr;
	int (*xcb_flush)(xcb_connection_t *c) = nullptr;
	xcb_void_cookie_t (*xcb_free_gc)(xcb_connection_t *c, xcb_gcontext_t gc) = nullptr;
	uint32_t (*xcb_generate_id)(xcb_connection_t *c) = nullptr;
	xcb_get_geometry_cookie_t (*xcb_get_geometry)(xcb_connection_t *c, xcb_drawable_t drawable) = nullptr;
	xcb_get_geometry_reply_t *(*xcb_get_geometry_reply)(xcb_connection_t *c, xcb_get_geometry_cookie_t cookie, xcb_generic_error_t **e) = nullptr;
	xcb_void_cookie_t (*xcb_put_image)(xcb_connection_t *c, uint8_t format, xcb_drawable_t drawable, xcb_gcontext_t gc, uint16_t width, uint16_t height, int16_t dst_x, int16_t dst_y, uint8_t left_pad, uint8_t depth, uint32_t data_len, const uint8_t *data) = nullptr;
	xcb_void_cookie_t (*xcb_copy_area)(xcb_connection_t *conn, xcb_drawable_t src_drawable, xcb_drawable_t dst_drawable, xcb_gcontext_t gc, int16_t src_x, int16_t src_y, int16_t dst_x, int16_t dst_y, uint16_t width, uint16_t height);
	xcb_void_cookie_t (*xcb_free_pixmap)(xcb_connection_t *conn, xcb_pixmap_t pixmap);
	xcb_query_extension_reply_t *(*xcb_get_extension_data)(xcb_connection_t *c, xcb_extension_t *extension) = nullptr;
	int	(*xcb_connection_has_error)(xcb_connection_t *c);
	uint32_t (*xcb_get_maximum_request_length)(xcb_connection_t *c);

	xcb_shm_query_version_cookie_t (*xcb_shm_query_version)(xcb_connection_t *c);
	xcb_shm_query_version_reply_t *(*xcb_shm_query_version_reply)(xcb_connection_t *c, xcb_shm_query_version_cookie_t cookie, xcb_generic_error_t **e);
	xcb_void_cookie_t (*xcb_shm_attach)(xcb_connection_t *c, xcb_shm_seg_t shmseg, uint32_t shmid, uint8_t read_only);
	xcb_void_cookie_t (*xcb_shm_detach)(xcb_connection_t *c, xcb_shm_seg_t shmseg);
	xcb_void_cookie_t (*xcb_shm_create_pixmap)(xcb_connection_t *c, xcb_pixmap_t pid, xcb_drawable_t drawable, uint16_t width, uint16_t height, uint8_t depth, xcb_shm_seg_t shmseg, uint32_t offset);
	xcb_extension_t *xcb_shm_id;
};

class LibXCB
{
public:
	bool isPresent()
	{
		return loadExports() != nullptr;
	}

	LibXcbExports *operator->();

private:
	LibXcbExports *loadExports();
};

extern LibXCB libXCB;

#endif  // libXCB_hpp
