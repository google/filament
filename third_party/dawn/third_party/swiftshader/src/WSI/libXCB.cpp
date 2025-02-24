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

#include "libXCB.hpp"

#include "System/SharedLibrary.hpp"

#include <memory>

LibXcbExports::LibXcbExports(void *libxcb, void *libshm)
{
	getFuncAddress(libxcb, "xcb_create_gc", &xcb_create_gc);
	getFuncAddress(libxcb, "xcb_flush", &xcb_flush);
	getFuncAddress(libxcb, "xcb_free_gc", &xcb_free_gc);
	getFuncAddress(libxcb, "xcb_generate_id", &xcb_generate_id);
	getFuncAddress(libxcb, "xcb_get_geometry", &xcb_get_geometry);
	getFuncAddress(libxcb, "xcb_get_geometry_reply", &xcb_get_geometry_reply);
	getFuncAddress(libxcb, "xcb_put_image", &xcb_put_image);
	getFuncAddress(libxcb, "xcb_copy_area", &xcb_copy_area);
	getFuncAddress(libxcb, "xcb_free_pixmap", &xcb_free_pixmap);
	getFuncAddress(libxcb, "xcb_get_extension_data", &xcb_get_extension_data);
	getFuncAddress(libxcb, "xcb_connection_has_error", &xcb_connection_has_error);
	getFuncAddress(libxcb, "xcb_get_maximum_request_length", &xcb_get_maximum_request_length);

	getFuncAddress(libshm, "xcb_shm_query_version", &xcb_shm_query_version);
	getFuncAddress(libshm, "xcb_shm_query_version_reply", &xcb_shm_query_version_reply);
	getFuncAddress(libshm, "xcb_shm_attach", &xcb_shm_attach);
	getFuncAddress(libshm, "xcb_shm_detach", &xcb_shm_detach);
	getFuncAddress(libshm, "xcb_shm_create_pixmap", &xcb_shm_create_pixmap);
	xcb_shm_id = (xcb_extension_t *)getProcAddress(libshm, "xcb_shm_id");
}

LibXcbExports *LibXCB::operator->()
{
	return loadExports();
}

LibXcbExports *LibXCB::loadExports()
{
	static LibXcbExports exports = [] {
		void *libxcb = nullptr;
		void *libshm = nullptr;
		if(getProcAddress(RTLD_DEFAULT, "xcb_create_gc"))  // Search the global scope for pre-loaded XCB library.
		{
			libxcb = RTLD_DEFAULT;
		}
		else
		{
			libxcb = loadLibrary("libxcb.so.1");
		}

		if(getProcAddress(RTLD_DEFAULT, "xcb_shm_query_version"))  // Search the global scope for pre-loaded XCB library.
		{
			libshm = RTLD_DEFAULT;
		}
		else
		{
			libshm = loadLibrary("libxcb-shm.so.0");
		}

		return LibXcbExports(libxcb, libshm);
	}();

	return exports.xcb_create_gc ? &exports : nullptr;
}

LibXCB libXCB;
