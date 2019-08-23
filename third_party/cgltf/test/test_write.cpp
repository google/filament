#define CGLTF_IMPLEMENTATION
#define CGLTF_WRITE_IMPLEMENTATION
#include "../cgltf_write.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("err\n");
		return -1;
	}

	cgltf_options options = {};
	cgltf_data* data0 = NULL;
	cgltf_result result = cgltf_parse_file(&options, argv[1], &data0);

	// Silently skip over files that are unreadable since this is a writing test.
	if (result != cgltf_result_success)
	{
		return cgltf_result_success;
	}

	result = cgltf_write_file(&options, "out.gltf", data0);
	if (result != cgltf_result_success)
	{
		return result;
	}
	cgltf_data* data1 = NULL;
	result = cgltf_parse_file(&options, "out.gltf", &data1);
	if (result != cgltf_result_success)
	{
		return result;
	}
	if (data0->meshes_count != data1->meshes_count) {
		return -1;
	}
	cgltf_free(data1);
	cgltf_free(data0);
	return cgltf_result_success;
}
