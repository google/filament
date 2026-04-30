// Testbench for OpLine instructions getting parsed correctly

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <spirv_cross.hpp>
#include <spirv_cross_parsed_ir.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

using namespace SPIRV_CROSS_SPV_HEADER_NAMESPACE;

#define SPVC_CHECK(x) do { \
	if (!(x)) { \
		fprintf(stderr, "Failed at line %d.\n", __LINE__); \
		exit(1); \
	} \
} while(0)

#define SPVC_CHECKED_CALL(x) do { \
	if ((x) != SPVC_SUCCESS) { \
		fprintf(stderr, "Failed at line %d.\n", __LINE__); \
		exit(1); \
	} \
} while(0)

static std::vector<uint32_t> read_file(const char *path)
{
	long len;
	FILE *file = fopen(path, "rb");

	if (!file)
		return {};

	fseek(file, 0, SEEK_END);
	len = ftell(file);
	rewind(file);

	std::vector<uint32_t> buffer(len / sizeof(uint32_t));
	if (fread(buffer.data(), 1, len, file) != (size_t)len)
	{
		fclose(file);
		return {};
	}

	fclose(file);
	return buffer;
}

int main(int argc, char **argv)
{
	if (argc != 2)
		return EXIT_FAILURE;

	auto buffer = read_file(argv[1]);
	if (buffer.empty())
		return EXIT_FAILURE;

	spirv_cross::Compiler compiler(std::move(buffer));
	auto& ir = compiler.get_ir();
	SPVC_CHECK(ir.sources.size() == 3u);

	// See debug-lines.spv
	constexpr auto src0 = R"SRC(#include "test.h"

[shader("compute")]
void main() {
	int a = 7;
	float b = 8;
	uint c = foo(a, b);
}

)SRC";

	constexpr auto src1 = R"SRC(uint foo(int a, float b) {
	return uint(a + b);
}
)SRC";

	// first source is empty OpSource
	SPVC_CHECK(ir.sources[0].source.empty());
	SPVC_CHECK(ir.sources[0].lang == SourceLanguageSlang);

	SPVC_CHECK(ir.sources[1].source == src1);
	SPVC_CHECK(ir.sources[2].source == src0);

	SPVC_CHECK(ir.sources[1].line_markers.size() == 2);
	SPVC_CHECK(ir.sources[1].line_markers[0].line == 2);
	SPVC_CHECK(ir.sources[1].line_markers[0].col == 2);
	SPVC_CHECK(ir.sources[1].line_markers[1].function_id != 0);
	SPVC_CHECK(ir.sources[1].line_markers[1].block_id != 0);
	SPVC_CHECK(ir.sources[1].line_markers[1].line == 2);

	SPVC_CHECK(ir.sources[2].line_markers.size() == 5);
	SPVC_CHECK(ir.sources[2].line_markers[0].line == 5);
	SPVC_CHECK(ir.sources[2].line_markers[0].col == 2);
	SPVC_CHECK(ir.sources[2].line_markers[1].function_id != 0);
	SPVC_CHECK(ir.sources[2].line_markers[1].block_id != 0);
	SPVC_CHECK(ir.sources[2].line_markers[4].line == 7);

}

