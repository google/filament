/* Smoke test for the C API. */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <spirv_cross_c.h>
#include <stdlib.h>
#include <stdio.h>

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

static int read_file(const char *path, SpvId **buffer, size_t *word_count)
{
	long len;
	FILE *file = fopen(path, "rb");

	if (!file)
		return -1;

	fseek(file, 0, SEEK_END);
	len = ftell(file);
	rewind(file);

	*buffer = malloc(len);
	if (fread(*buffer, 1, len, file) != (size_t)len)
	{
		fclose(file);
		free(*buffer);
		return -1;
	}

	fclose(file);
	*word_count = len / sizeof(SpvId);
	return 0;
}

static spvc_bool g_fail_on_error = SPVC_TRUE;

static void error_callback(void *userdata, const char *error)
{
	(void)userdata;
	if (g_fail_on_error)
	{
		fprintf(stderr, "Error: %s\n", error);
		exit(1);
	}
	else
		printf("Expected error hit: %s.\n", error);
}

static void dump_resource_list(spvc_compiler compiler, spvc_resources resources, spvc_resource_type type, const char *tag)
{
	const spvc_reflected_resource *list = NULL;
	size_t count = 0;
	size_t i;
	SPVC_CHECKED_CALL(spvc_resources_get_resource_list_for_type(resources, type, &list, &count));
	printf("%s\n", tag);
	for (i = 0; i < count; i++)
	{
		printf("ID: %u, BaseTypeID: %u, TypeID: %u, Name: %s\n", list[i].id, list[i].base_type_id, list[i].type_id,
		       list[i].name);
		printf("  Set: %u, Binding: %u\n",
		       spvc_compiler_get_decoration(compiler, list[i].id, SpvDecorationDescriptorSet),
		       spvc_compiler_get_decoration(compiler, list[i].id, SpvDecorationBinding));
	}
}

static void dump_resources(spvc_compiler compiler, spvc_resources resources)
{
	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, "UBO");
	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_STORAGE_BUFFER, "SSBO");
	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_PUSH_CONSTANT, "Push");
	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_SEPARATE_SAMPLERS, "Samplers");
	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_SEPARATE_IMAGE, "Image");
	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_SAMPLED_IMAGE, "Combined image samplers");
	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_STAGE_INPUT, "Stage input");
	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_STAGE_OUTPUT, "Stage output");
	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_STORAGE_IMAGE, "Storage image");
	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_SUBPASS_INPUT, "Subpass input");
}

static void compile(spvc_compiler compiler, const char *tag)
{
	const char *result = NULL;
	SPVC_CHECKED_CALL(spvc_compiler_compile(compiler, &result));
	printf("\n%s\n=======\n", tag);
	printf("%s\n=======\n", result);
}

int main(int argc, char **argv)
{
	const char *rev = NULL;

	spvc_context context = NULL;
	spvc_parsed_ir ir = NULL;
	spvc_compiler compiler_glsl = NULL;
	spvc_compiler compiler_hlsl = NULL;
	spvc_compiler compiler_msl = NULL;
	spvc_compiler compiler_cpp = NULL;
	spvc_compiler compiler_json = NULL;
	spvc_compiler compiler_none = NULL;
	spvc_compiler_options options = NULL;
	spvc_resources resources = NULL;
	SpvId *buffer = NULL;
	size_t word_count = 0;

	rev = spvc_get_commit_revision_and_timestamp();
	if (!rev || *rev == '\0')
		return 1;

	printf("Revision: %s\n", rev);

	if (argc != 5)
		return 1;

	if (read_file(argv[1], &buffer, &word_count) < 0)
		return 1;

	unsigned abi_major, abi_minor, abi_patch;
	spvc_get_version(&abi_major, &abi_minor, &abi_patch);
	if (abi_major != strtoul(argv[2], NULL, 0))
	{
		fprintf(stderr, "VERSION_MAJOR mismatch!\n");
		return 1;
	}

	if (abi_minor != strtoul(argv[3], NULL, 0))
	{
		fprintf(stderr, "VERSION_MINOR mismatch!\n");
		return 1;
	}

	if (abi_patch != strtoul(argv[4], NULL, 0))
	{
		fprintf(stderr, "VERSION_PATCH mismatch!\n");
		return 1;
	}

	SPVC_CHECKED_CALL(spvc_context_create(&context));
	spvc_context_set_error_callback(context, error_callback, NULL);
	SPVC_CHECKED_CALL(spvc_context_parse_spirv(context, buffer, word_count, &ir));
	SPVC_CHECKED_CALL(spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_COPY, &compiler_glsl));
	SPVC_CHECKED_CALL(spvc_context_create_compiler(context, SPVC_BACKEND_HLSL, ir, SPVC_CAPTURE_MODE_COPY, &compiler_hlsl));
	SPVC_CHECKED_CALL(spvc_context_create_compiler(context, SPVC_BACKEND_MSL, ir, SPVC_CAPTURE_MODE_COPY, &compiler_msl));
	SPVC_CHECKED_CALL(spvc_context_create_compiler(context, SPVC_BACKEND_CPP, ir, SPVC_CAPTURE_MODE_COPY, &compiler_cpp));
	SPVC_CHECKED_CALL(spvc_context_create_compiler(context, SPVC_BACKEND_JSON, ir, SPVC_CAPTURE_MODE_COPY, &compiler_json));
	SPVC_CHECKED_CALL(spvc_context_create_compiler(context, SPVC_BACKEND_NONE, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler_none));

	SPVC_CHECKED_CALL(spvc_compiler_create_compiler_options(compiler_none, &options));
	SPVC_CHECKED_CALL(spvc_compiler_install_compiler_options(compiler_none, options));
	SPVC_CHECKED_CALL(spvc_compiler_create_compiler_options(compiler_json, &options));
	SPVC_CHECKED_CALL(spvc_compiler_install_compiler_options(compiler_json, options));
	SPVC_CHECKED_CALL(spvc_compiler_create_compiler_options(compiler_cpp, &options));
	SPVC_CHECKED_CALL(spvc_compiler_install_compiler_options(compiler_cpp, options));
	SPVC_CHECKED_CALL(spvc_compiler_create_compiler_options(compiler_msl, &options));
	SPVC_CHECKED_CALL(spvc_compiler_install_compiler_options(compiler_msl, options));
	SPVC_CHECKED_CALL(spvc_compiler_create_compiler_options(compiler_hlsl, &options));
	SPVC_CHECKED_CALL(spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_HLSL_SHADER_MODEL, 50));
	SPVC_CHECKED_CALL_NEGATIVE(spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_MSL_PLATFORM, 1));
	SPVC_CHECKED_CALL(spvc_compiler_install_compiler_options(compiler_hlsl, options));
	SPVC_CHECKED_CALL(spvc_compiler_create_compiler_options(compiler_glsl, &options));
	SPVC_CHECKED_CALL(spvc_compiler_install_compiler_options(compiler_glsl, options));

	SPVC_CHECKED_CALL(spvc_compiler_create_shader_resources(compiler_none, &resources));
	dump_resources(compiler_none, resources);
	compile(compiler_glsl, "GLSL");
	compile(compiler_hlsl, "HLSL");
	compile(compiler_msl, "MSL");
	compile(compiler_json, "JSON");
	compile(compiler_cpp, "CPP");

	spvc_context_destroy(context);
	free(buffer);
	return 0;
}
