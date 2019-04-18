#define CGLTF_IMPLEMENTATION
#include "../cgltf.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

static bool is_near(cgltf_float a, cgltf_float b)
{
	return std::abs(a - b) < 10 * std::numeric_limits<cgltf_float>::min();
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("err\n");
		return -1;
	}

	cgltf_options options = {};
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, argv[1], &data);

	if (result == cgltf_result_success)
		result = cgltf_load_buffers(&options, data, argv[1]);

	if (result != cgltf_result_success || strstr(argv[1], "Draco"))
		return result;

	const cgltf_accessor* blobs = data->accessors;
	cgltf_float element[16];
	for (cgltf_size blob_index = 0; blob_index < data->accessors_count; ++blob_index)
	{
		const cgltf_accessor* blob = data->accessors + blob_index;
		if (blob->is_sparse)
		{
			continue;
		}
		if (blob->has_max && blob->has_min)
		{
			cgltf_float min0 = std::numeric_limits<float>::max();
			cgltf_float max0 = std::numeric_limits<float>::lowest();
			for (cgltf_size index = 0; index < blob->count; index++)
			{
				cgltf_accessor_read_float(blob, index, element, 16);
				min0 = std::min(min0, element[0]);
				max0 = std::max(max0, element[0]);
			}
			if (!is_near(min0, blob->min[0]) || !is_near(max0, blob->max[0]))
			{
				printf("Computed [%f, %f] but expected [%f, %f]\n", min0, max0, blob->min[0], blob->max[0]);
				return -1;
			}
		}
	}

	cgltf_free(data);

	return result;
}
