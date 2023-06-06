// Testbench for MSL constexpr samplers.
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

	SPVC_CHECKED_CALL(spvc_context_create(&ctx));
	SPVC_CHECKED_CALL(spvc_context_parse_spirv(ctx, buffer.data(), buffer.size(), &parsed_ir));
	SPVC_CHECKED_CALL(spvc_context_create_compiler(ctx, SPVC_BACKEND_MSL, parsed_ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler));

	spvc_msl_resource_binding binding;
	spvc_msl_resource_binding_init(&binding);
	binding.desc_set = 1;
	binding.binding = 2;
	binding.stage = SpvExecutionModelFragment;
	binding.msl_texture = 0;
	binding.msl_sampler = 0;
	SPVC_CHECKED_CALL(spvc_compiler_msl_add_resource_binding(compiler, &binding));

	binding.binding = 3;
	binding.msl_texture = 1;
	binding.msl_sampler = 1000; // Will be remapped anyways, sanity check.
	SPVC_CHECKED_CALL(spvc_compiler_msl_add_resource_binding(compiler, &binding));

	binding.desc_set = 2;
	binding.binding = 2;
	binding.msl_texture = 2;
	SPVC_CHECKED_CALL(spvc_compiler_msl_add_resource_binding(compiler, &binding));

	binding.binding = 3;
	binding.msl_texture = 3;
	SPVC_CHECKED_CALL(spvc_compiler_msl_add_resource_binding(compiler, &binding));

	spvc_msl_constexpr_sampler samp;
	spvc_msl_constexpr_sampler_init(&samp);
	samp.s_address = SPVC_MSL_SAMPLER_ADDRESS_REPEAT;
	samp.t_address = SPVC_MSL_SAMPLER_ADDRESS_REPEAT;
	samp.r_address = SPVC_MSL_SAMPLER_ADDRESS_REPEAT;
	SPVC_CHECKED_CALL(spvc_compiler_msl_remap_constexpr_sampler_by_binding(compiler, 1, 3, &samp));

	samp.s_address = SPVC_MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE;
	samp.t_address = SPVC_MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE;
	samp.r_address = SPVC_MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE;
	SPVC_CHECKED_CALL(spvc_compiler_msl_remap_constexpr_sampler_by_binding(compiler, 2, 4, &samp));

	samp.compare_enable = SPVC_TRUE;
	samp.compare_func = SPVC_MSL_SAMPLER_COMPARE_FUNC_LESS;
	samp.s_address = SPVC_MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE;
	samp.t_address = SPVC_MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE;
	samp.r_address = SPVC_MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE;
	SPVC_CHECKED_CALL(spvc_compiler_msl_remap_constexpr_sampler_by_binding(compiler, 2, 5, &samp));

	const char *str;
	SPVC_CHECKED_CALL(spvc_compiler_compile(compiler, &str));

	// Should not be marked as used.
	if (spvc_compiler_msl_is_resource_used(compiler, SpvExecutionModelFragment, 2, 4))
		return EXIT_FAILURE;

	// Should not be marked as used.
	if (spvc_compiler_msl_is_resource_used(compiler, SpvExecutionModelFragment, 2, 5))
		return EXIT_FAILURE;

	// Should be marked, as a sanity check.
	if (!spvc_compiler_msl_is_resource_used(compiler, SpvExecutionModelFragment, 1, 2))
		return EXIT_FAILURE;
	if (!spvc_compiler_msl_is_resource_used(compiler, SpvExecutionModelFragment, 1, 3))
		return EXIT_FAILURE;
	if (!spvc_compiler_msl_is_resource_used(compiler, SpvExecutionModelFragment, 2, 2))
		return EXIT_FAILURE;
	if (!spvc_compiler_msl_is_resource_used(compiler, SpvExecutionModelFragment, 2, 3))
		return EXIT_FAILURE;

	fprintf(stderr, "Output:\n%s\n", str);
}

