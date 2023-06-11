// Testbench for MSL resource binding APIs.
// It does not validate output at the moment, but it's useful for ad-hoc testing.

#include <spirv_cross_c.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#define SPVC_CHECKED_CALL(x) do { \
	if ((x) != SPVC_SUCCESS) { \
		fprintf(stderr, "Failed at line %d.\n", __LINE__); \
		exit(1); \
	} \
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
	SPVC_CHECKED_CALL(spvc_compiler_msl_add_discrete_descriptor_set(compiler, 3));

	spvc_compiler_options opts;
	SPVC_CHECKED_CALL(spvc_compiler_create_compiler_options(compiler, &opts));
	SPVC_CHECKED_CALL(spvc_compiler_options_set_bool(opts, SPVC_COMPILER_OPTION_MSL_ARGUMENT_BUFFERS, SPVC_TRUE));
	SPVC_CHECKED_CALL(spvc_compiler_options_set_uint(opts, SPVC_COMPILER_OPTION_MSL_VERSION, 20000));
	SPVC_CHECKED_CALL(spvc_compiler_install_compiler_options(compiler, opts));

	spvc_msl_resource_binding binding;
	spvc_msl_resource_binding_init(&binding);
	binding.binding = SPVC_MSL_ARGUMENT_BUFFER_BINDING;
	binding.stage = SpvExecutionModelFragment;
	binding.desc_set = 0;
	binding.msl_buffer = 2;
	SPVC_CHECKED_CALL(spvc_compiler_msl_add_resource_binding(compiler, &binding));

	binding.desc_set = 1;
	binding.msl_buffer = 3;
	SPVC_CHECKED_CALL(spvc_compiler_msl_add_resource_binding(compiler, &binding));

	const char *str;
	SPVC_CHECKED_CALL(spvc_compiler_compile(compiler, &str));

	fprintf(stderr, "Output:\n%s\n", str);

	if (!spvc_compiler_msl_is_resource_used(compiler, SpvExecutionModelFragment, 0, SPVC_MSL_ARGUMENT_BUFFER_BINDING))
		return EXIT_FAILURE;

	if (!spvc_compiler_msl_is_resource_used(compiler, SpvExecutionModelFragment, 1, SPVC_MSL_ARGUMENT_BUFFER_BINDING))
		return EXIT_FAILURE;
}

