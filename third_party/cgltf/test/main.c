

#define CGLTF_IMPLEMENTATION
#include "../cgltf.h"

#include <stdio.h>

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("err\n");
		return -1;
	}

	cgltf_options options;
	memset(&options, 0, sizeof(cgltf_options));
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, argv[1], &data);

	if (result == cgltf_result_success)
		result = cgltf_load_buffers(&options, data, argv[1]);

	if (result == cgltf_result_success)
		result = cgltf_validate(data);

	printf("Result: %d\n", result);

	if (result == cgltf_result_success)
	{
		printf("Type: %u\n", data->file_type);
		printf("Meshes: %u\n", (unsigned)data->meshes_count);
	}

	cgltf_free(data);

	return result;
}
