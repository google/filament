#include <gli/core/storage_linear.hpp>

int test_storage_layer_size()
{
	int Error(0);

	gli::storage_linear Storage(
		gli::FORMAT_RGBA8_UNORM_PACK8,
		gli::storage_linear::extent_type(2, 2, 1),
		2, 1, 1);

	std::vector<glm::u8vec4> Data(8, glm::u8vec4(0));
	for(std::size_t i = 0; i < 4; ++i)
		Data[i + 0] = glm::u8vec4(255, 127, 0, 255);
	for(std::size_t i = 0; i < 4; ++i)
		Data[i + 4] = glm::u8vec4(0, 127, 255, 255);

	memcpy(Storage.data(), &Data[0][0], Data.size() * sizeof(glm::u8vec4));

	Error += Storage.block_size() == sizeof(glm::u8vec4) ? 0 : 1;
	Error += Storage.level_size(0) == sizeof(glm::u8vec4) * 2 * 2 ? 0 : 1;
	Error += Storage.face_size(0, Storage.levels() - 1) == sizeof(glm::u8vec4) * 2 * 2 ? 0 : 1;
	Error += Storage.layer_size(0, Storage.faces() - 1, 0, Storage.levels() - 1) == sizeof(glm::u8vec4) * 2 * 2 ? 0 : 1;
	Error += Storage.size() == sizeof(glm::u8vec4) * 2 * 2 * 2 ? 0 : 1;

	return Error;
}

int test_storage_face_size()
{
	int Error(0);

	gli::storage_linear Storage(
		gli::FORMAT_RGBA8_UNORM_PACK8,
		gli::storage_linear::extent_type(2, 2, 1),
		1, 6, 1);

	gli::storage_linear::size_type BlockSize = Storage.block_size();
	Error += BlockSize == sizeof(glm::u8vec4) ? 0 : 1;

	gli::storage_linear::size_type LevelSize = Storage.level_size(0);
	Error += LevelSize == sizeof(glm::u8vec4) * 2 * 2 ? 0 : 1;

	gli::storage_linear::size_type FaceSize = Storage.face_size(0, Storage.levels() - 1);
	Error += FaceSize == sizeof(glm::u8vec4) * 2 * 2 ? 0 : 1;

	gli::storage_linear::size_type LayerSize = Storage.layer_size(0, Storage.faces() - 1, 0, Storage.levels() - 1);
	Error += LayerSize == sizeof(glm::u8vec4) * 2 * 2 * 6 ? 0 : 1;

	gli::storage_linear::size_type Size = Storage.size();
	Error += Size == sizeof(glm::u8vec4) * 2 * 2 * 6 ? 0 : 1;

	return Error;
}

int main()
{
	int Error(0);

	Error += test_storage_layer_size();
	Error += test_storage_face_size();

	GLI_ASSERT(!Error);

	return Error;
}
