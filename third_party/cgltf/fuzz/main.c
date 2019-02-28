#define CGLTF_IMPLEMENTATION
#include "../cgltf.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
	cgltf_options options = {0};
	cgltf_data* data = NULL;
	cgltf_result res = cgltf_parse(&options, Data, Size, &data);
	if (res == cgltf_result_success)
	{
		cgltf_validate(data);
		cgltf_free(data);
	}
	return 0;
}
