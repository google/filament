/* How to fuzz:

clang main.c -O2 -g -fsanitize=address,fuzzer -o fuzz
cp -r data temp
./fuzz temp/ -dict=gltf.dict -jobs=12 -workers=12

*/
#define CGLTF_IMPLEMENTATION
#include "../cgltf.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
	cgltf_options options = {cgltf_file_type_invalid};
	cgltf_data* data = NULL;
	cgltf_result res = cgltf_parse(&options, Data, Size, &data);
	if (res == cgltf_result_success)
	{
		cgltf_validate(data);
		cgltf_free(data);
	}
	return 0;
}
