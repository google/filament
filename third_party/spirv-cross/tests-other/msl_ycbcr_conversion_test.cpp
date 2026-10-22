// Testbench for MSL constexpr samplers, with Y'CbCr conversion.
// It does not validate output, but it's useful for ad-hoc testing.

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <spirv_cross_c.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#define SPVC_CHECKED_CALL(x) do { \
	if ((x) != SPVC_SUCCESS) { \
		fprintf(stderr, "Failed at line %d.\n", __LINE__); \
		exit(1); \
	} \
} while(0)
#define SPVC_CHECKED_CALL_NEGATIVE(x) do { \
	g_fail_on_error = SPVC_FALSE; \
	if ((x) == SPVC_SUCCESS) { \
		fprintf(stderr, "Failed at line %d.\n", __LINE__); \
		exit(1); \
	} \
	g_fail_on_error = SPVC_TRUE; \
} while(0)

static std::vector<SpvId> read_file(const char *path)
{
	long len;
	FILE *file = fopen(path, "rb");

	if (!file)
		return {};

	fseek(file, 0, SEEK_END);
	len = ftell(file);
	rewind(file);

	std::vector<SpvId> buffer(len / sizeof(SpvId));
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

	spvc_context ctx;
	spvc_parsed_ir parsed_ir;
	spvc_compiler compiler;
	spvc_compiler_options options;

	SPVC_CHECKED_CALL(spvc_context_create(&ctx));
	SPVC_CHECKED_CALL(spvc_context_parse_spirv(ctx, buffer.data(), buffer.size(), &parsed_ir));
	SPVC_CHECKED_CALL(spvc_context_create_compiler(ctx, SPVC_BACKEND_MSL, parsed_ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler));
	SPVC_CHECKED_CALL(spvc_compiler_create_compiler_options(compiler, &options));
	SPVC_CHECKED_CALL(spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_MSL_VERSION, SPVC_MAKE_MSL_VERSION(2, 0, 0)));
	SPVC_CHECKED_CALL(spvc_compiler_install_compiler_options(compiler, options));

	spvc_msl_resource_binding binding;
	spvc_msl_resource_binding_init(&binding);
	binding.desc_set = 1;
	binding.binding = 2;
	binding.stage = SpvExecutionModelFragment;
	binding.msl_texture = 0;
	binding.msl_sampler = 0;
	SPVC_CHECKED_CALL(spvc_compiler_msl_add_resource_binding(compiler, &binding));

	spvc_msl_constexpr_sampler samp;
	spvc_msl_sampler_ycbcr_conversion conv;
	spvc_msl_constexpr_sampler_init(&samp);
	spvc_msl_sampler_ycbcr_conversion_init(&conv);
	conv.planes = 3;
	conv.resolution = SPVC_MSL_FORMAT_RESOLUTION_422;
	conv.chroma_filter = SPVC_MSL_SAMPLER_FILTER_LINEAR;
	conv.x_chroma_offset = SPVC_MSL_CHROMA_LOCATION_MIDPOINT;
	conv.ycbcr_model = SPVC_MSL_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_BT_2020;
	conv.ycbcr_range = SPVC_MSL_SAMPLER_YCBCR_RANGE_ITU_NARROW;
	conv.bpc = 8;
	SPVC_CHECKED_CALL(spvc_compiler_msl_remap_constexpr_sampler_by_binding_ycbcr(compiler, 1, 2, &samp, &conv));

	const char *str;
	SPVC_CHECKED_CALL(spvc_compiler_compile(compiler, &str));

	// Should be marked, as a sanity check.
	if (!spvc_compiler_msl_is_resource_used(compiler, SpvExecutionModelFragment, 1, 2))
		return EXIT_FAILURE;

	fprintf(stderr, "Output:\n%s\n", str);
}

