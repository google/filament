// basisu_tool.cpp
// Copyright (C) 2019-2022 Binomial LLC. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#if _MSC_VER
// For sprintf(), strcpy() 
#define _CRT_SECURE_NO_WARNINGS (1)
#endif

#include "transcoder/basisu.h"
#include "transcoder/basisu_transcoder_internal.h"
#include "encoder/basisu_enc.h"
#include "encoder/basisu_etc.h"
#include "encoder/basisu_gpu_texture.h"
#include "encoder/basisu_frontend.h"
#include "encoder/basisu_backend.h"
#include "encoder/basisu_comp.h"
#include "transcoder/basisu_transcoder.h"
#include "encoder/basisu_ssim.h"
#include "encoder/basisu_opencl.h"

#define MINIZ_HEADER_FILE_ONLY
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include "encoder/basisu_miniz.h"

// Set BASISU_CATCH_EXCEPTIONS if you want exceptions to crash the app, otherwise main() catches them.
#ifndef BASISU_CATCH_EXCEPTIONS
	#define BASISU_CATCH_EXCEPTIONS 0
#endif

using namespace basisu;
using namespace buminiz;

#define BASISU_TOOL_VERSION "1.16.3"

enum tool_mode
{
	cDefault,
	cCompress,
	cValidate,
	cInfo,
	cUnpack,
	cCompare,
	cVersion,
	cBench,
	cCompSize,
	cTest,
	cSplitImage,
	cCombineImages
};

static void print_usage()
{
	printf("\nUsage: basisu filename [filename ...] <options>\n");
	
	puts("\n"
		"The default mode is compression of one or more PNG/BMP/TGA/JPG files to a .basis file. Alternate modes:\n"
		" -unpack: Use transcoder to unpack .basis file to one or more .ktx/.png files\n"
		" -validate: Validate and display information about a .basis file\n"
		" -info: Display high-level information about a .basis file\n"
		" -compare: Compare two PNG/BMP/TGA/JPG images specified with -file, output PSNR and SSIM statistics and RGB/A delta images\n"
		" -version: Print basisu version and exit\n"
		"Unless an explicit mode is specified, if one or more files have the .basis extension this tool defaults to unpack mode.\n"
		"\n"
		"Important: By default, the compressor assumes the input is in the sRGB colorspace (like photos/albedo textures).\n"
		"If the input is NOT sRGB (like a normal map), be sure to specify -linear for less artifacts. Depending on the content type, some experimentation may be needed.\n"
		"\n"
		"Filenames prefixed with a @ symbol are read as filename listing files. Listing text files specify which actual filenames to process (one filename per line).\n"
		"\n"
		"Options:\n"
		" -opencl: Enable OpenCL usage\n"
		" -opencl_serialize: Serialize all calls to the OpenCL driver (to work around buggy drivers, only useful with -parallel)\n"
		" -parallel: Compress multiple textures simumtanously (one per thread), instead of one at a time. Compatible with OpenCL mode. This is much faster, but in OpenCL mode the driver is pushed harder, and the CLI output will be jumbled.\n"
		" -ktx2: Write .KTX2 ETC1S/UASTC files instead of .basis files. By default, UASTC files will be compressed using Zstandard unless -ktx2_no_zstandard is specified.\n"
		" -ktx2_no_zstandard: Don't compress UASTC texture data using Zstandard, store it uncompressed instead.\n"
		" -ktx2_zstandard_level X: Set ZStandard compression level to X (see Zstandard documentation, default level is 6)\n"
		" -ktx2_animdata_duration X: Set KTX2animData duration field to integer value X (only valid/useful for -tex_type video, default is 1)\n"
		" -ktx2_animdata_timescale X: Set KTX2animData timescale field to integer value X (only valid/useful for -tex_type video, default is 15)\n"
		" -ktx2_animdata_loopcount X: Set KTX2animData loopcount field to integer value X (only valid/useful for -tex_type video, default is 0)\n"
		" -file filename.png/bmp/tga/jpg/qoi: Input image filename, multiple images are OK, use -file X for each input filename (prefixing input filenames with -file is optional)\n"
		" -alpha_file filename.png/bmp/tga/jpg/qoi: Input alpha image filename, multiple images are OK, use -file X for each input filename (must be paired with -file), images converted to REC709 grayscale and used as input alpha\n"
		" -multifile_printf: printf() format strint to use to compose multiple filenames\n"
		" -multifile_first: The index of the first file to process, default is 0 (must specify -multifile_printf and -multifile_num)\n"
		" -multifile_num: The total number of files to process.\n"
		" -q X: Set ETC1S quality level, 1-255, default is 128, lower=better compression/lower quality/faster, higher=less compression/higher quality/slower, default is 128. For even higher quality, use -max_endpoints/-max_selectors.\n"
		" -linear: Use linear colorspace metrics (instead of the default sRGB), and by default linear (not sRGB) mipmap filtering.\n"
		" -output_file filename: Output .basis/.ktx filename\n"
		" -output_path: Output .basis/.ktx files to specified directory.\n"
		" -debug: Enable codec debug print to stdout (slightly slower).\n"
		" -verbose: Same as -debug (debug output to stdout).\n"
		" -debug_images: Enable codec debug images (much slower).\n"
		" -stats: Compute and display image quality metrics (slightly slower).\n"
		" -tex_type <2d, 2darray, 3d, video, cubemap>: Set Basis file header's texture type field. Cubemap arrays require multiples of 6 images, in X+, X-, Y+, Y-, Z+, Z- order, each image must be the same resolutions.\n"
		"  2d=arbitrary 2D images, 2darray=2D array, 3D=volume texture slices, video=video frames, cubemap=array of faces. For 2darray/3d/cubemaps/video, each source image's dimensions and # of mipmap levels must be the same.\n"
		" For video, the .basis file will be written with the first frame being an I-Frame, and subsequent frames being P-Frames (using conditional replenishment). Playback must always occur in order from first to last image.\n"
		" -framerate X: Set framerate in .basis header to X/frames sec.\n"
		" -individual: Process input images individually and output multiple .basis/.ktx2 files (not as a texture array - this is now the default as of v1.16)\n"
		" -tex_array: Process input images as a single texture array and write a single .basis/.ktx2 file (the former default before v1.16)\n"
		" -comp_level X: Set ETC1S encoding speed vs. quality tradeoff. Range is 0-6, default is 1. Higher values=MUCH slower, but slightly higher quality. Higher levels intended for videos. Use -q first!\n"
		" -fuzz_testing: Use with -validate: Disables CRC16 validation of file contents before transcoding\n"
		"\nUASTC options:\n"
		" -uastc: Enable UASTC texture mode, instead of the default ETC1S mode. Significantly higher texture quality, but larger files. (Note that UASTC .basis files must be losslessly compressed by the user.)\n"
		" -uastc_level: Set UASTC encoding level. Range is [0,4], default is 2, higher=slower but higher quality. 0=fastest/lowest quality, 3=slowest practical option, 4=impractically slow/highest achievable quality\n"
		" -uastc_rdo_l X: Enable UASTC RDO post-processing and set UASTC RDO quality scalar (lambda) to X. Lower values=higher quality/larger LZ\n"
		"                 compressed files, higher values=lower quality/smaller LZ compressed files. Good range to try is [.25-10].\n"
		"                 Note: Previous versons used the -uastc_rdo_q option, which was removed because the RDO algorithm was changed.\n"
		" -uastc_rdo_d X: Set UASTC RDO dictionary size in bytes. Default is 4096, max is 65536. Lower values=faster, but less compression.\n"
		" -uastc_rdo_b X: Set UASTC RDO max smooth block error scale. Range is [1,300]. Default is 10.0, 1.0=disabled. Larger values suppress more artifacts (and allocate more bits) on smooth blocks.\n"
		" -uastc_rdo_s X: Set UASTC RDO max smooth block standard deviation. Range is [.01,65536]. Default is 18.0. Larger values expand the range of blocks considered smooth.\n"
		" -uastc_rdo_f: Don't favor simpler UASTC modes in RDO mode.\n"
		" -uastc_rdo_m: Disable RDO multithreading (slightly higher compression, deterministic).\n"
		"\n"
		"More options:\n"
		" -test: Run an automated ETC1S/UASTC encoding and transcoding test. Returns EXIT_FAILURE if any failures\n"
		" -test_dir: Optional directory of test files. Defaults to \"../test_files\".\n"
		" -max_endpoints X: Manually set the max number of color endpoint clusters from 1-16128, use instead of -q\n"
		" -max_selectors X: Manually set the max number of color selector clusters from 1-16128, use instead of -q\n"
		" -y_flip: Flip input images vertically before compression\n"
		" -normal_map: Tunes codec parameters for better quality on normal maps (linear colorspace metrics, linear mipmap filtering, no selector RDO, no sRGB)\n"
		" -no_alpha: Always output non-alpha basis files, even if one or more inputs has alpha\n"
		" -force_alpha: Always output alpha basis files, even if no inputs has alpha\n"
		" -separate_rg_to_color_alpha: Separate input R and G channels to RGB and A (for tangent space XY normal maps)\n"
		" -swizzle rgba: Specify swizzle for the 4 input color channels using r, g, b and a (the -separate_rg_to_color_alpha flag is equivalent to rrrg)\n"
		" -renorm: Renormalize each input image before any further processing/compression\n"
		" -no_multithreading: Disable multithreading\n"
		" -max_threads X: Use at most X threads total when multithreading is enabled (this includes the main thread)\n"
		" -no_ktx: Disable KTX writing when unpacking (faster, less output files)\n"
		" -ktx_only: Only write KTX files when unpacking (faster, less output files)\n"
		" -write_out: Write 3dfx OUT files when unpacking FXT1 textures\n"
		" -etc1_only: Only unpack to ETC1, skipping the other texture formats during -unpack\n"
		" -disable_hierarchical_endpoint_codebooks: Disable hierarchical endpoint codebook usage, slower but higher quality on some compression levels\n"
		" -compare_ssim: Compute and display SSIM of image comparison (slow)\n"
		" -bench: UASTC benchmark mode, for development only\n"
		" -resample X Y: Resample all input textures to XxY pixels using a box filter\n"
		" -resample_factor X: Resample all input textures by scale factor X using a box filter\n"
		" -no_sse: Forbid all SSE instruction set usage\n"
		" -validate_etc1s: Validate internal ETC1S compressor's data structures during compression (slower, intended for development).\n"
		"\n"
		"Mipmap generation options:\n"
		" -mipmap: Generate mipmaps for each source image\n"
		" -mip_srgb: Convert image to linear before filtering, then back to sRGB\n"
		" -mip_linear: Keep image in linear light during mipmap filtering (i.e. do not convert to/from sRGB for filtering purposes)\n"
		" -mip_scale X: Set mipmap filter kernel's scale, lower=sharper, higher=more blurry, default is 1.0\n"
		" -mip_filter X: Set mipmap filter kernel, default is kaiser, filters: box, tent, bell, blackman, catmullrom, mitchell, etc.\n"
		" -mip_renorm: Renormalize normal map to unit length vectors after filtering\n"
		" -mip_clamp: Use clamp addressing on borders, instead of wrapping\n"
		" -mip_fast: Use faster mipmap generation (resample from previous mip, not always first/largest mip level). The default (as of 1/2021)\n"
		" -mip_slow: Always resample each mipmap level starting from the largest mipmap. Higher quality, but slower. Opposite of -mip_fast. Was the prior default before 1/2021.\n"
		" -mip_smallest X: Set smallest pixel dimension for generated mipmaps, default is 1 pixel\n"
		"By default, textures will be converted from sRGB to linear light before mipmap filtering, then back to sRGB (for the RGB color channels) unless -linear is specified.\n"
		"You can override this behavior with -mip_srgb/-mip_linear.\n"
		"\n"
		"Backend endpoint/selector RDO codec options:\n"
		" -no_selector_rdo: Disable backend's selector rate distortion optimizations (slightly faster, less noisy output, but lower quality per output bit)\n"
		" -selector_rdo_thresh X: Set selector RDO quality threshold, default is 1.25, lower is higher quality but less quality per output bit (try 1.0-3.0)\n"
		" -no_endpoint_rdo: Disable backend's endpoint rate distortion optimizations (slightly faster, less noisy output, but lower quality per output bit)\n"
		" -endpoint_rdo_thresh X: Set endpoint RDO quality threshold, default is 1.5, lower is higher quality but less quality per output bit (try 1.0-3.0)\n"
		"\n"
		"Set various fields in the Basis file header:\n"
		" -userdata0 X: Set 32-bit userdata0 field in Basis file header to X (X is a signed 32-bit int)\n"
		" -userdata1 X: Set 32-bit userdata1 field in Basis file header to X (X is a signed 32-bit int)\n"
		"\n"
		"Various command line examples:\n"
		" basisu x.png : Compress sRGB image x.png to x.basis using default settings (multiple filenames OK, use -tex_array if you want a tex array vs. multiple output files)\n"
		" basisu x.basis : Unpack x.basis to PNG/KTX files (multiple filenames OK)\n"
		" basisu -file x.png -mipmap -y_flip : Compress a mipmapped x.basis file from an sRGB image named x.png, Y flip each source image\n"
		" basisu -validate -file x.basis : Validate x.basis (check header, check file CRC's, attempt to transcode all slices)\n"
		" basisu -unpack -file x.basis : Validates, transcodes and unpacks x.basis to mipmapped .KTX and RGB/A .PNG files (transcodes to all supported GPU texture formats)\n"
		" basisu -q 255 -file x.png -mipmap -debug -stats : Compress sRGB x.png to x.basis at quality level 255 with compressor debug output/statistics\n"
		" basisu -linear -max_endpoints 16128 -max_selectors 16128 -file x.png : Compress non-sRGB x.png to x.basis using the largest supported manually specified codebook sizes\n"
		" basisu -comp_level 2 -max_selectors 8192 -max_endpoints 8192 -tex_type video -framerate 20 -multifile_printf \"x%02u.png\" -multifile_first 1 -multifile_count 20 : Compress a 20 sRGB source image video sequence (x01.png, x02.png, x03.png, etc.) to x01.basis\n"
		"\n"
		"Note: For video use, it's recommended you use a very powerful machine with many cores. Use -comp_level 2 or higher for better codebook\n"
		"generation, specify very large codebooks using -max_endpoints and -max_selectors, and reduce the default endpoint RDO threshold\n"
		"(-endpoint_rdo_thresh) to around 1.25. Videos may have mipmaps and alpha channels. Videos must always be played back by the transcoder\n"
		"in first to last image order.\n"
		"Video files currently use I-Frames on the first image, and P-Frames using conditional replenishment on subsequent frames.\n"
		"\nCompression level (-comp_level X) details:\n"
		" Level 0: Fastest, but has marginal quality and can be brittle on complex images. Avg. Y dB: 35.45\n"
		" Level 1: Hierarchical codebook searching, faster ETC1S encoding. 36.87 dB, ~1.4x slower vs. level 0. (This is the default setting.)\n"
		" Level 2: Use this or higher for video. Hierarchical codebook searching. 36.87 dB, ~1.4x slower vs. level 0. (This is the v1.12's default setting.)\n"
		" Level 3: Full codebook searching. 37.13 dB, ~1.8x slower vs. level 0. (Equivalent the the initial release's default settings.)\n"
		" Level 4: Hierarchical codebook searching, codebook k-means iterations. 37.15 dB, ~4x slower vs. level 0\n"
		" Level 5: Full codebook searching, codebook k-means iterations. 37.41 dB, ~5.5x slower vs. level 0. (Equivalent to the initial release's -slower setting.)\n"
		" Level 6: Full codebook searching, twice as many codebook k-means iterations, best ETC1 endpoint opt. 37.43 dB, ~12x slower vs. level 0\n"
	);
}

static bool load_listing_file(const std::string &f, basisu::vector<std::string> &filenames)
{
	std::string filename(f);
	filename.erase(0, 1);

	FILE *pFile = nullptr;
#ifdef _WIN32
	fopen_s(&pFile, filename.c_str(), "r");
#else
	pFile = fopen(filename.c_str(), "r");
#endif

	if (!pFile)
	{
		error_printf("Failed opening listing file: \"%s\"\n", filename.c_str());
		return false;
	}

	uint32_t total_filenames = 0;

	for ( ; ; )
	{
		char buf[3072];
		buf[0] = '\0';

		char *p = fgets(buf, sizeof(buf), pFile);
		if (!p)
		{
			if (ferror(pFile))
			{
				error_printf("Failed reading from listing file: \"%s\"\n", filename.c_str());

				fclose(pFile);
				return false;
			}
			else
				break;
		}

		std::string read_filename(p);
		while (read_filename.size())
		{
			if (read_filename[0] == ' ')
				read_filename.erase(0, 1);
			else 
				break;
		}

		while (read_filename.size())
		{
			const char c = read_filename.back();
			if ((c == ' ') || (c == '\n') || (c == '\r'))
				read_filename.erase(read_filename.size() - 1, 1);
			else 
				break;
		}

		if (read_filename.size())
		{
			filenames.push_back(read_filename);
			total_filenames++;
		}
	}

	fclose(pFile);

	printf("Successfully read %u filenames(s) from listing file \"%s\"\n", total_filenames, filename.c_str());

	return true;
}

class command_line_params
{
	BASISU_NO_EQUALS_OR_COPY_CONSTRUCT(command_line_params);

public:
	command_line_params() :
		m_mode(cDefault),
		m_ktx2_mode(false),
		m_ktx2_zstandard(true),
		m_ktx2_zstandard_level(6),
		m_ktx2_animdata_duration(1),
		m_ktx2_animdata_timescale(15),
		m_ktx2_animdata_loopcount(0),
		m_multifile_first(0),
		m_multifile_num(0),
		m_max_threads(1024), // surely this is high enough
		m_individual(true),
		m_no_ktx(false),
		m_ktx_only(false),
		m_write_out(false),
		m_etc1_only(false),
		m_fuzz_testing(false),
		m_compare_ssim(false),
		m_bench(false),
		m_parallel_compression(false)
	{
		m_comp_params.m_compression_level = basisu::maximum<int>(0, BASISU_DEFAULT_COMPRESSION_LEVEL - 1);
		m_test_file_dir = "../test_files";
	}

	bool parse(int arg_c, const char **arg_v)
	{
		int arg_index = 1;
		while (arg_index < arg_c)
		{
			const char *pArg = arg_v[arg_index];
			const int num_remaining_args = arg_c - (arg_index + 1);
			int arg_count = 1;

#define REMAINING_ARGS_CHECK(n) if (num_remaining_args < (n)) { error_printf("Error: Expected %u values to follow %s!\n", n, pArg); return false; }

			if (strcasecmp(pArg, "-ktx2") == 0)
			{
				m_ktx2_mode = true;
			}
			else if (strcasecmp(pArg, "-ktx2_no_zstandard") == 0)
			{
				m_ktx2_zstandard = false;
			}
			else if (strcasecmp(pArg, "-ktx2_zstandard_level") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_ktx2_zstandard_level = atoi(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-ktx2_animdata_duration") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_ktx2_animdata_duration = atoi(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-ktx2_animdata_timescale") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_ktx2_animdata_timescale = atoi(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-ktx2_animdata_loopcount") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_ktx2_animdata_loopcount = atoi(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-compress") == 0)
				m_mode = cCompress;
			else if (strcasecmp(pArg, "-compare") == 0)
				m_mode = cCompare;
			else if (strcasecmp(pArg, "-split") == 0)
				m_mode = cSplitImage;
			else if (strcasecmp(pArg, "-combine") == 0)
				m_mode = cCombineImages;
			else if (strcasecmp(pArg, "-unpack") == 0)
				m_mode = cUnpack;
			else if (strcasecmp(pArg, "-validate") == 0)
				m_mode = cValidate;
			else if (strcasecmp(pArg, "-info") == 0)
				m_mode = cInfo;
			else if (strcasecmp(pArg, "-version") == 0)
				m_mode = cVersion;
			else if (strcasecmp(pArg, "-compare_ssim") == 0)
				m_compare_ssim = true;
			else if (strcasecmp(pArg, "-bench") == 0)
				m_mode = cBench;
			else if (strcasecmp(pArg, "-comp_size") == 0)
				m_mode = cCompSize;
			else if (strcasecmp(pArg, "-test") == 0)
				m_mode = cTest;
			else if (strcasecmp(pArg, "-test_dir") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_test_file_dir = std::string(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-no_sse") == 0)
			{
#if BASISU_SUPPORT_SSE
				g_cpu_supports_sse41 = false;
#endif
			}
			else if (strcasecmp(pArg, "-no_status_output") == 0)
			{
				m_comp_params.m_status_output = false;
			}
			else if (strcasecmp(pArg, "-file") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_input_filenames.push_back(std::string(arg_v[arg_index + 1]));
				arg_count++;
			}
			else if (strcasecmp(pArg, "-alpha_file") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_input_alpha_filenames.push_back(std::string(arg_v[arg_index + 1]));
				arg_count++;
			}
			else if (strcasecmp(pArg, "-multifile_printf") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_multifile_printf = std::string(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-multifile_first") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_multifile_first = atoi(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-multifile_num") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_multifile_num = atoi(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-uastc") == 0)
				m_comp_params.m_uastc = true;
			else if (strcasecmp(pArg, "-uastc_level") == 0)
			{
				REMAINING_ARGS_CHECK(1);

				int uastc_level = atoi(arg_v[arg_index + 1]);

				uastc_level = clamp<int>(uastc_level, 0, TOTAL_PACK_UASTC_LEVELS - 1);
								
				static_assert(TOTAL_PACK_UASTC_LEVELS == 5, "TOTAL_PACK_UASTC_LEVELS==5");
				static const uint32_t s_level_flags[TOTAL_PACK_UASTC_LEVELS] = { cPackUASTCLevelFastest, cPackUASTCLevelFaster, cPackUASTCLevelDefault, cPackUASTCLevelSlower, cPackUASTCLevelVerySlow };
				
				m_comp_params.m_pack_uastc_flags &= ~cPackUASTCLevelMask;
				m_comp_params.m_pack_uastc_flags |= s_level_flags[uastc_level];
				
				arg_count++;
			}
			else if (strcasecmp(pArg, "-resample") == 0)
			{
				REMAINING_ARGS_CHECK(2);
				m_comp_params.m_resample_width = atoi(arg_v[arg_index + 1]);
				m_comp_params.m_resample_height = atoi(arg_v[arg_index + 2]);
				arg_count += 2;
			}
			else if (strcasecmp(pArg, "-resample_factor") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_comp_params.m_resample_factor = (float)atof(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-uastc_rdo_l") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_comp_params.m_rdo_uastc_quality_scalar = (float)atof(arg_v[arg_index + 1]);
				m_comp_params.m_rdo_uastc = true;
				arg_count++;
			}
			else if (strcasecmp(pArg, "-uastc_rdo_d") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_comp_params.m_rdo_uastc_dict_size = atoi(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-uastc_rdo_b") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_comp_params.m_rdo_uastc_max_smooth_block_error_scale = (float)atof(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-uastc_rdo_s") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_comp_params.m_rdo_uastc_smooth_block_max_std_dev = (float)atof(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-uastc_rdo_f") == 0)
				m_comp_params.m_rdo_uastc_favor_simpler_modes_in_rdo_mode = false;
			else if (strcasecmp(pArg, "-uastc_rdo_m") == 0)
				m_comp_params.m_rdo_uastc_multithreading = false;
			else if (strcasecmp(pArg, "-linear") == 0)
				m_comp_params.m_perceptual = false;
			else if (strcasecmp(pArg, "-srgb") == 0)
				m_comp_params.m_perceptual = true;
			else if (strcasecmp(pArg, "-q") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_comp_params.m_quality_level = clamp<int>(atoi(arg_v[arg_index + 1]), BASISU_QUALITY_MIN, BASISU_QUALITY_MAX);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-output_file") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_output_filename = arg_v[arg_index + 1];
				arg_count++;
			}
			else if (strcasecmp(pArg, "-output_path") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_output_path = arg_v[arg_index + 1];
				arg_count++;
			}
			else if ((strcasecmp(pArg, "-debug") == 0) || (strcasecmp(pArg, "-verbose") == 0))
			{
				m_comp_params.m_debug = true;
				enable_debug_printf(true);
			}
			else if (strcasecmp(pArg, "-validate_etc1s") == 0)
			{
				m_comp_params.m_validate_etc1s = true;
			}
			else if (strcasecmp(pArg, "-validate_output") == 0)
			{
				m_comp_params.m_validate_output_data = true;
			}
			else if (strcasecmp(pArg, "-debug_images") == 0)
				m_comp_params.m_debug_images = true;
			else if (strcasecmp(pArg, "-stats") == 0)
				m_comp_params.m_compute_stats = true;
			else if (strcasecmp(pArg, "-gen_global_codebooks") == 0)
			{
				// TODO
			}
			else if (strcasecmp(pArg, "-use_global_codebooks") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_etc1s_use_global_codebooks_file = arg_v[arg_index + 1];
				arg_count++;
			}
			else if (strcasecmp(pArg, "-comp_level") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_comp_params.m_compression_level = atoi(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-slower") == 0)
			{
				// This option is gone, but we'll do something reasonable with it anyway. Level 4 is equivalent to the original release's -slower, but let's just go to level 2.
				m_comp_params.m_compression_level = BASISU_DEFAULT_COMPRESSION_LEVEL + 1;
			}
			else if (strcasecmp(pArg, "-max_endpoints") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_comp_params.m_max_endpoint_clusters = clamp<int>(atoi(arg_v[arg_index + 1]), 1, BASISU_MAX_ENDPOINT_CLUSTERS);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-max_selectors") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_comp_params.m_max_selector_clusters = clamp<int>(atoi(arg_v[arg_index + 1]), 1, BASISU_MAX_SELECTOR_CLUSTERS);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-y_flip") == 0)
				m_comp_params.m_y_flip = true;
			else if (strcasecmp(pArg, "-normal_map") == 0)
			{
				m_comp_params.m_perceptual = false;
				m_comp_params.m_mip_srgb = false;
				m_comp_params.m_no_selector_rdo = true;
				m_comp_params.m_no_endpoint_rdo = true;
			}
			else if (strcasecmp(pArg, "-no_alpha") == 0)
				m_comp_params.m_check_for_alpha = false;
			else if (strcasecmp(pArg, "-force_alpha") == 0)
				m_comp_params.m_force_alpha = true;
			else if ((strcasecmp(pArg, "-separate_rg_to_color_alpha") == 0) ||
			        (strcasecmp(pArg, "-seperate_rg_to_color_alpha") == 0)) // was mispelled for a while - whoops!
			{
				m_comp_params.m_swizzle[0] = 0;
				m_comp_params.m_swizzle[1] = 0;
				m_comp_params.m_swizzle[2] = 0;
				m_comp_params.m_swizzle[3] = 1;
			}
			else if (strcasecmp(pArg, "-swizzle") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				const char *swizzle = arg_v[arg_index + 1];
				if (strlen(swizzle) != 4)
				{
					error_printf("Swizzle requires exactly 4 characters\n");
					return false;
				}
				for (int i=0; i<4; ++i)
				{
					if (swizzle[i] == 'r')
						m_comp_params.m_swizzle[i] = 0;
					else if (swizzle[i] == 'g')
						m_comp_params.m_swizzle[i] = 1;
					else if (swizzle[i] == 'b')
						m_comp_params.m_swizzle[i] = 2;
					else if (swizzle[i] == 'a')
						m_comp_params.m_swizzle[i] = 3;
					else
					{
						error_printf("Swizzle must be one of [rgba]");
						return false;
					}
				}
				arg_count++;
			}
			else if (strcasecmp(pArg, "-renorm") == 0)
				m_comp_params.m_renormalize = true;
			else if (strcasecmp(pArg, "-no_multithreading") == 0)
			{
				m_comp_params.m_multithreading = false;
			}
			else if (strcasecmp(pArg, "-parallel") == 0)
			{
				m_parallel_compression = true;
			}
			else if (strcasecmp(pArg, "-max_threads") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_max_threads = atoi(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-mipmap") == 0)
				m_comp_params.m_mip_gen = true;
			else if (strcasecmp(pArg, "-no_ktx") == 0)
				m_no_ktx = true;
			else if (strcasecmp(pArg, "-ktx_only") == 0)
				m_ktx_only = true;
			else if (strcasecmp(pArg, "-write_out") == 0)
				m_write_out = true;
			else if (strcasecmp(pArg, "-etc1_only") == 0)
				m_etc1_only = true;
			else if (strcasecmp(pArg, "-disable_hierarchical_endpoint_codebooks") == 0)
				m_comp_params.m_disable_hierarchical_endpoint_codebooks = true;
			else if (strcasecmp(pArg, "-opencl") == 0)
			{
				m_comp_params.m_use_opencl = true;
			}
			else if (strcasecmp(pArg, "-opencl_serialize") == 0)
			{
			}
			else if (strcasecmp(pArg, "-mip_scale") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_comp_params.m_mip_scale = (float)atof(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-mip_filter") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_comp_params.m_mip_filter = arg_v[arg_index + 1];
				// TODO: Check filter 
				arg_count++;
			}
			else if (strcasecmp(pArg, "-mip_renorm") == 0)
				m_comp_params.m_mip_renormalize = true;
			else if (strcasecmp(pArg, "-mip_clamp") == 0)
				m_comp_params.m_mip_wrapping = false;
			else if (strcasecmp(pArg, "-mip_fast") == 0)
				m_comp_params.m_mip_fast = true;
			else if (strcasecmp(pArg, "-mip_slow") == 0)
				m_comp_params.m_mip_fast = false;
			else if (strcasecmp(pArg, "-mip_smallest") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_comp_params.m_mip_smallest_dimension = atoi(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-mip_srgb") == 0)
				m_comp_params.m_mip_srgb = true;
			else if (strcasecmp(pArg, "-mip_linear") == 0)
				m_comp_params.m_mip_srgb = false;
			else if (strcasecmp(pArg, "-no_selector_rdo") == 0)
				m_comp_params.m_no_selector_rdo = true;
			else if (strcasecmp(pArg, "-selector_rdo_thresh") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_comp_params.m_selector_rdo_thresh = (float)atof(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-no_endpoint_rdo") == 0)
				m_comp_params.m_no_endpoint_rdo = true;
			else if (strcasecmp(pArg, "-endpoint_rdo_thresh") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_comp_params.m_endpoint_rdo_thresh = (float)atof(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-userdata0") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_comp_params.m_userdata0 = atoi(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-userdata1") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_comp_params.m_userdata1 = atoi(arg_v[arg_index + 1]);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-framerate") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				double fps = atof(arg_v[arg_index + 1]);
				double us_per_frame = 0;
				if (fps > 0)
					us_per_frame = 1000000.0f / fps;

				m_comp_params.m_us_per_frame = clamp<int>(static_cast<int>(us_per_frame + .5f), 0, basist::cBASISMaxUSPerFrame);
				arg_count++;
			}
			else if (strcasecmp(pArg, "-tex_type") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				const char* pType = arg_v[arg_index + 1];
				if (strcasecmp(pType, "2d") == 0)
					m_comp_params.m_tex_type = basist::cBASISTexType2D;
				else if (strcasecmp(pType, "2darray") == 0)
				{
					m_comp_params.m_tex_type = basist::cBASISTexType2DArray;
					m_individual = false;
				}
				else if (strcasecmp(pType, "3d") == 0)
				{
					m_comp_params.m_tex_type = basist::cBASISTexTypeVolume;
					m_individual = false;
				}
				else if (strcasecmp(pType, "cubemap") == 0)
				{
					m_comp_params.m_tex_type = basist::cBASISTexTypeCubemapArray;
					m_individual = false;
				}
				else if (strcasecmp(pType, "video") == 0)
				{
					m_comp_params.m_tex_type = basist::cBASISTexTypeVideoFrames;
					m_individual = false;
				}
				else
				{
					error_printf("Invalid texture type: %s\n", pType);
					return false;
				}
				arg_count++;
			}
			else if (strcasecmp(pArg, "-individual") == 0)
				m_individual = true;
			else if (strcasecmp(pArg, "-tex_array") == 0)
				m_individual = false;
			else if (strcasecmp(pArg, "-fuzz_testing") == 0)
				m_fuzz_testing = true;
			else if (strcasecmp(pArg, "-csv_file") == 0)
			{
				REMAINING_ARGS_CHECK(1);
				m_csv_file = arg_v[arg_index + 1];
				m_comp_params.m_compute_stats = true;

				arg_count++;
			}
			else if (pArg[0] == '-')
			{
				error_printf("Unrecognized command line option: %s\n", pArg);
				return false;
			}
			else
			{
				// Let's assume it's a source filename, so globbing works
				//error_printf("Unrecognized command line option: %s\n", pArg);
				m_input_filenames.push_back(pArg);
			}

			arg_index += arg_count;
		}
		
		if (m_comp_params.m_quality_level != -1)
		{
			m_comp_params.m_max_endpoint_clusters = 0;
			m_comp_params.m_max_selector_clusters = 0;
		}
		else if ((!m_comp_params.m_max_endpoint_clusters) || (!m_comp_params.m_max_selector_clusters))
		{
			m_comp_params.m_max_endpoint_clusters = 0;
			m_comp_params.m_max_selector_clusters = 0;

			m_comp_params.m_quality_level = 128;
		}

		if (!m_comp_params.m_mip_srgb.was_changed())
		{
			// They didn't specify what colorspace to do mipmap filtering in, so choose sRGB if they've specified that the texture is sRGB.
			if (m_comp_params.m_perceptual)
				m_comp_params.m_mip_srgb = true;
			else
				m_comp_params.m_mip_srgb = false;
		}
				
		return true;
	}

	bool process_listing_files()
	{
		basisu::vector<std::string> new_input_filenames;
		for (uint32_t i = 0; i < m_input_filenames.size(); i++)
		{
			if (m_input_filenames[i][0] == '@')
			{
				if (!load_listing_file(m_input_filenames[i], new_input_filenames))
					return false;
			}
			else
				new_input_filenames.push_back(m_input_filenames[i]);
		}
		new_input_filenames.swap(m_input_filenames);

		basisu::vector<std::string> new_input_alpha_filenames;
		for (uint32_t i = 0; i < m_input_alpha_filenames.size(); i++)
		{
			if (m_input_alpha_filenames[i][0] == '@')
			{
				if (!load_listing_file(m_input_alpha_filenames[i], new_input_alpha_filenames))
					return false;
			}
			else
				new_input_alpha_filenames.push_back(m_input_alpha_filenames[i]);
		}
		new_input_alpha_filenames.swap(m_input_alpha_filenames);
		
		return true;
	}

	basis_compressor_params m_comp_params;
		
	tool_mode m_mode;

	bool m_ktx2_mode;
	bool m_ktx2_zstandard;
	int m_ktx2_zstandard_level;
	uint32_t m_ktx2_animdata_duration;
	uint32_t m_ktx2_animdata_timescale;
	uint32_t m_ktx2_animdata_loopcount;
		
	basisu::vector<std::string> m_input_filenames;
	basisu::vector<std::string> m_input_alpha_filenames;

	std::string m_output_filename;
	std::string m_output_path;

	std::string m_multifile_printf;
	uint32_t m_multifile_first;
	uint32_t m_multifile_num;

	std::string m_csv_file;

	std::string m_etc1s_use_global_codebooks_file;

	std::string m_test_file_dir;
	
	uint32_t m_max_threads;
		
	bool m_individual;
	bool m_no_ktx;
	bool m_ktx_only;
	bool m_write_out;
	bool m_etc1_only;
	bool m_fuzz_testing;
	bool m_compare_ssim;
	bool m_bench;
	bool m_parallel_compression;
};

static bool expand_multifile(command_line_params &opts)
{
	if (!opts.m_multifile_printf.size())
		return true;
	
	if (!opts.m_multifile_num)
	{
		error_printf("-multifile_printf specified, but not -multifile_num\n");
		return false;
	}
	
	std::string fmt(opts.m_multifile_printf);
	// Workaround for MSVC debugger issues. Questionable to leave in here.
	size_t x = fmt.find_first_of('!');
	if (x != std::string::npos)
		fmt[x] = '%';

	if (string_find_right(fmt, '%') == -1)
	{
		error_printf("Must include C-style printf() format character '%%' in -multifile_printf string\n");
		return false;
	}
		
	for (uint32_t i = opts.m_multifile_first; i < opts.m_multifile_first + opts.m_multifile_num; i++)
	{
		char buf[1024];
#ifdef _WIN32		
		sprintf_s(buf, sizeof(buf), fmt.c_str(), i);
#else
		snprintf(buf, sizeof(buf), fmt.c_str(), i);
#endif		

		if (buf[0])
			opts.m_input_filenames.push_back(buf);
	}

	return true;
}

struct basis_data
{
	basis_data() : 
		m_transcoder() 
	{
	}
	uint8_vec m_file_data;
	basist::basisu_transcoder m_transcoder;
};

static basis_data *load_basis_file(const char *pInput_filename, bool force_etc1s)
{
	basis_data* p = new basis_data;
	uint8_vec &basis_data = p->m_file_data;
	if (!basisu::read_file_to_vec(pInput_filename, basis_data))
	{
		error_printf("Failed reading file \"%s\"\n", pInput_filename);
		delete p;
		return nullptr;
	}
	printf("Input file \"%s\"\n", pInput_filename);
	if (!basis_data.size())
	{
		error_printf("File is empty!\n");
		delete p;
		return nullptr;
	}
	if (basis_data.size() > UINT32_MAX)
	{
		error_printf("File is too large!\n");
		delete p;
		return nullptr;
	}
	if (force_etc1s)
	{
		if (p->m_transcoder.get_tex_format((const void*)&p->m_file_data[0], (uint32_t)p->m_file_data.size()) != basist::basis_tex_format::cETC1S)
		{
			error_printf("Global codebook file must be in ETC1S format!\n");
			delete p;
			return nullptr;
		}
	}
	if (!p->m_transcoder.start_transcoding(&basis_data[0], (uint32_t)basis_data.size()))
	{
		error_printf("start_transcoding() failed!\n");
		delete p;
		return nullptr;
	}
	return p;
}

static bool compress_mode(command_line_params &opts)
{
	uint32_t num_threads = 1;

	if (opts.m_comp_params.m_multithreading)
	{
		// We use std::thread::hardware_concurrency() as a hint to determine the default # of threads to put into a pool.
		num_threads = std::thread::hardware_concurrency();
		if (num_threads < 1)
			num_threads = 1;
		if (num_threads > opts.m_max_threads)
			num_threads = opts.m_max_threads;
	}

	job_pool compressor_jpool(opts.m_parallel_compression ? 1 : num_threads);
	if (!opts.m_parallel_compression)
		opts.m_comp_params.m_pJob_pool = &compressor_jpool;
		
	if (!expand_multifile(opts))
	{
		error_printf("-multifile expansion failed!\n");
		return false;
	}

	if (!opts.m_input_filenames.size())
	{
		error_printf("No input files to process!\n");
		return false;
	}
		
	basis_data* pGlobal_codebook_data = nullptr;
	if (opts.m_etc1s_use_global_codebooks_file.size())
	{
		pGlobal_codebook_data = load_basis_file(opts.m_etc1s_use_global_codebooks_file.c_str(), true);
		if (!pGlobal_codebook_data)
			return false;

		printf("Loaded global codebooks from .basis file \"%s\"\n", opts.m_etc1s_use_global_codebooks_file.c_str());

#if 0
		// Development/test code. TODO: Remove.
		basis_data* pGlobal_codebook_data2 = load_basis_file("xmen_1024.basis", sel_codebook, true);
		const basist::basisu_lowlevel_etc1s_transcoder &ta = pGlobal_codebook_data->m_transcoder.get_lowlevel_etc1s_decoder();
		const basist::basisu_lowlevel_etc1s_transcoder &tb = pGlobal_codebook_data2->m_transcoder.get_lowlevel_etc1s_decoder();
		if (ta.get_endpoints().size() != tb.get_endpoints().size())
		{
			printf("Endpoint CB's don't match\n");
		}
		else if (ta.get_selectors().size() != tb.get_selectors().size())
		{
			printf("Selector CB's don't match\n");
		}
		else
		{
			for (uint32_t i = 0; i < ta.get_endpoints().size(); i++)
			{
				if (ta.get_endpoints()[i] != tb.get_endpoints()[i])
				{
					printf("Endoint CB mismatch entry %u\n", i);
				}
			}
			for (uint32_t i = 0; i < ta.get_selectors().size(); i++)
			{
				if (ta.get_selectors()[i] != tb.get_selectors()[i])
				{
					printf("Selector CB mismatch entry %u\n", i);
				}
			}
		}
		delete pGlobal_codebook_data2;
		pGlobal_codebook_data2 = nullptr;
#endif
	}
						
	basis_compressor_params &params = opts.m_comp_params;

	if (opts.m_ktx2_mode)
	{
		params.m_create_ktx2_file = true;
		if (opts.m_ktx2_zstandard)
			params.m_ktx2_uastc_supercompression = basist::KTX2_SS_ZSTANDARD;
		else
			params.m_ktx2_uastc_supercompression = basist::KTX2_SS_NONE;
		
		params.m_ktx2_srgb_transfer_func = opts.m_comp_params.m_perceptual;

		if (params.m_tex_type == basist::basis_texture_type::cBASISTexTypeVideoFrames)
		{
			// Create KTXanimData key value entry
			// TODO: Move this to basisu_comp.h
			basist::ktx2_transcoder::key_value kv;

			const char* pAD = "KTXanimData";
			kv.m_key.resize(strlen(pAD) + 1);
			strcpy((char*)kv.m_key.data(), pAD);
			
			basist::ktx2_animdata ad;
			ad.m_duration = opts.m_ktx2_animdata_duration;
			ad.m_timescale = opts.m_ktx2_animdata_timescale;
			ad.m_loopcount = opts.m_ktx2_animdata_loopcount;

			kv.m_value.resize(sizeof(ad));
			memcpy(kv.m_value.data(), &ad, sizeof(ad));

			params.m_ktx2_key_values.push_back(kv);
		}
		
		// TODO- expose this to command line.
		params.m_ktx2_zstd_supercompression_level = opts.m_ktx2_zstandard_level;
	}

	params.m_read_source_images = true;
	params.m_write_output_basis_files = true;
	params.m_pGlobal_codebooks = pGlobal_codebook_data ? &pGlobal_codebook_data->m_transcoder.get_lowlevel_etc1s_decoder() : nullptr; 
	FILE *pCSV_file = nullptr;
	if (opts.m_csv_file.size())
	{
		//pCSV_file = fopen_safe(opts.m_csv_file.c_str(), "a");
		pCSV_file = fopen_safe(opts.m_csv_file.c_str(), "w");
		if (!pCSV_file)
		{
			error_printf("Failed opening CVS file \"%s\"\n", opts.m_csv_file.c_str());
			delete pGlobal_codebook_data; pGlobal_codebook_data = nullptr;
			return false;
		}
		fprintf(pCSV_file, "Filename, Size, Slices, Width, Height, HasAlpha, BitsPerTexel, Slice0RGBAvgPSNR, Slice0RGBAAvgPSNR, Slice0Luma709PSNR, Slice0BestETC1SLuma709PSNR, Q, CL, Time, RGBAvgPSNRMin, RGBAvgPSNRAvg, AAvgPSNRMin, AAvgPSNRAvg, Luma709PSNRMin, Luma709PSNRAvg\n");
	}

	printf("Processing %u total file(s)\n", (uint32_t)opts.m_input_filenames.size());
				
	interval_timer all_tm;
	all_tm.start();

	basisu::vector<basis_compressor_params> comp_params_vec;

	const size_t total_files = (opts.m_individual ? opts.m_input_filenames.size() : 1U);
	bool result = true;

	if ((opts.m_individual) && (opts.m_output_filename.size()))
	{
		if (total_files > 1)
		{
			error_printf("-output_file specified in individual mode, but multiple input files have been specified which would cause the output file to be written multiple times.\n");
			delete pGlobal_codebook_data; pGlobal_codebook_data = nullptr;
			return false;
		}
	}

	for (size_t file_index = 0; file_index < total_files; file_index++)
	{
		if (opts.m_individual)
		{
			params.m_source_filenames.resize(1);
			params.m_source_filenames[0] = opts.m_input_filenames[file_index];

			if (file_index < opts.m_input_alpha_filenames.size())
			{
				params.m_source_alpha_filenames.resize(1);
				params.m_source_alpha_filenames[0] = opts.m_input_alpha_filenames[file_index];

				if (params.m_status_output)
					printf("Processing source file \"%s\", alpha file \"%s\"\n", params.m_source_filenames[0].c_str(), params.m_source_alpha_filenames[0].c_str());
			}
			else
			{
				params.m_source_alpha_filenames.resize(0);

				if (params.m_status_output)
					printf("Processing source file \"%s\"\n", params.m_source_filenames[0].c_str());
			}
		}
		else
		{
			params.m_source_filenames = opts.m_input_filenames;
			params.m_source_alpha_filenames = opts.m_input_alpha_filenames;
		}
				
		if (opts.m_output_filename.size())
			params.m_out_filename = opts.m_output_filename;
		else
		{
			std::string filename;

			string_get_filename(opts.m_input_filenames[file_index].c_str(), filename);
			string_remove_extension(filename);

			if (opts.m_ktx2_mode)
				filename += ".ktx2";
			else
				filename += ".basis";

			if (opts.m_output_path.size())
				string_combine_path(filename, opts.m_output_path.c_str(), filename.c_str());

			params.m_out_filename = filename;
		}

		if (opts.m_parallel_compression)
		{
			comp_params_vec.push_back(params);
		}
		else
		{
			basis_compressor c;

			if (!c.init(opts.m_comp_params))
			{
				error_printf("basis_compressor::init() failed!\n");

				if (pCSV_file)
				{
					fclose(pCSV_file);
					pCSV_file = nullptr;
				}

				delete pGlobal_codebook_data; pGlobal_codebook_data = nullptr;
				return false;
			}

			interval_timer tm;
			tm.start();

			basis_compressor::error_code ec = c.process();

			tm.stop();

			if (ec == basis_compressor::cECSuccess)
			{
				if (params.m_status_output)
				{
					printf("Compression succeeded to file \"%s\" size %u bytes in %3.3f secs\n", params.m_out_filename.c_str(),
						opts.m_ktx2_mode ? c.get_output_ktx2_file().size() : c.get_output_basis_file().size(),
						tm.get_elapsed_secs());
				}
			}
			else
			{
				result = false;

				if (!params.m_status_output)
				{
					error_printf("Compression failed on file \"%s\"\n", params.m_out_filename.c_str());
				}

				bool exit_flag = true;

				switch (ec)
				{
				case basis_compressor::cECFailedReadingSourceImages:
				{
					error_printf("Compressor failed reading a source image!\n");

					if (opts.m_individual)
						exit_flag = false;

					break;
				}
				case basis_compressor::cECFailedValidating:
					error_printf("Compressor failed 2darray/cubemap/video validation checks!\n");
					break;
				case basis_compressor::cECFailedEncodeUASTC:
					error_printf("Compressor UASTC encode failed!\n");
					break;
				case basis_compressor::cECFailedFrontEnd:
					error_printf("Compressor frontend stage failed!\n");
					break;
				case basis_compressor::cECFailedFontendExtract:
					error_printf("Compressor frontend data extraction failed!\n");
					break;
				case basis_compressor::cECFailedBackend:
					error_printf("Compressor backend stage failed!\n");
					break;
				case basis_compressor::cECFailedCreateBasisFile:
					error_printf("Compressor failed creating Basis file data!\n");
					break;
				case basis_compressor::cECFailedWritingOutput:
					error_printf("Compressor failed writing to output Basis file!\n");
					break;
				case basis_compressor::cECFailedUASTCRDOPostProcess:
					error_printf("Compressor failed during the UASTC post process step!\n");
					break;
				case basis_compressor::cECFailedCreateKTX2File:
					error_printf("Compressor failed creating KTX2 file data!\n");
					break;
				default:
					error_printf("basis_compress::process() failed!\n");
					break;
				}

				if (exit_flag)
				{
					if (pCSV_file)
					{
						fclose(pCSV_file);
						pCSV_file = nullptr;
					}

					delete pGlobal_codebook_data; pGlobal_codebook_data = nullptr;
					return false;
				}
			}

			if ((pCSV_file) && (c.get_stats().size()))
			{
				if (c.get_stats().size())
				{
					float rgb_avg_psnr_min = 1e+9f, rgb_avg_psnr_avg = 0.0f;
					float a_avg_psnr_min = 1e+9f, a_avg_psnr_avg = 0.0f;
					float luma_709_psnr_min = 1e+9f, luma_709_psnr_avg = 0.0f;

					for (size_t slice_index = 0; slice_index < c.get_stats().size(); slice_index++)
					{
						rgb_avg_psnr_min = basisu::minimum(rgb_avg_psnr_min, c.get_stats()[slice_index].m_basis_rgb_avg_psnr);
						rgb_avg_psnr_avg += c.get_stats()[slice_index].m_basis_rgb_avg_psnr;

						a_avg_psnr_min = basisu::minimum(a_avg_psnr_min, c.get_stats()[slice_index].m_basis_a_avg_psnr);
						a_avg_psnr_avg += c.get_stats()[slice_index].m_basis_a_avg_psnr;

						luma_709_psnr_min = basisu::minimum(luma_709_psnr_min, c.get_stats()[slice_index].m_basis_luma_709_psnr);
						luma_709_psnr_avg += c.get_stats()[slice_index].m_basis_luma_709_psnr;
					}

					rgb_avg_psnr_avg /= c.get_stats().size();
					a_avg_psnr_avg /= c.get_stats().size();
					luma_709_psnr_avg /= c.get_stats().size();

					fprintf(pCSV_file, "\"%s\", %u, %u, %u, %u, %u, %f, %f, %f, %f, %f, %u, %u, %f, %f, %f, %f, %f, %f, %f\n",
						params.m_out_filename.c_str(),
						c.get_basis_file_size(),
						(uint32_t)c.get_stats().size(),
						c.get_stats()[0].m_width, c.get_stats()[0].m_height, (uint32_t)c.get_any_source_image_has_alpha(),
						c.get_basis_bits_per_texel(),
						c.get_stats()[0].m_basis_rgb_avg_psnr,
						c.get_stats()[0].m_basis_rgba_avg_psnr,
						c.get_stats()[0].m_basis_luma_709_psnr,
						c.get_stats()[0].m_best_etc1s_luma_709_psnr,
						params.m_quality_level, (int)params.m_compression_level, tm.get_elapsed_secs(),
						rgb_avg_psnr_min, rgb_avg_psnr_avg,
						a_avg_psnr_min, a_avg_psnr_avg,
						luma_709_psnr_min, luma_709_psnr_avg);
					fflush(pCSV_file);
				}
			}

			//if ((opts.m_individual) && (params.m_status_output))
			//	printf("\n");

		} // if (opts.m_parallel_compression)

	} // file_index

	if (opts.m_parallel_compression)
	{
		basisu::vector<parallel_results> results;

		bool any_failed = basis_parallel_compress(
			num_threads,
			comp_params_vec,
			results);
		BASISU_NOTE_UNUSED(any_failed);

		uint32_t total_successes = 0, total_failures = 0;
		for (uint32_t i = 0; i < comp_params_vec.size(); i++)
		{
			if (results[i].m_error_code != basis_compressor::cECSuccess)
			{
				result = false;

				total_failures++;

				error_printf("File %u (first source image: \"%s\", output file: \"%s\") failed with error code %i!\n", i, 
					comp_params_vec[i].m_source_filenames[0].c_str(), 
					comp_params_vec[i].m_out_filename.c_str(),
					(int)results[i].m_error_code);
			}
			else
			{
				total_successes++;
			}
		}

		printf("Total successes: %u failures: %u\n", total_successes, total_failures);

	} // if (opts.m_parallel_compression)
		
	all_tm.stop();

	if (total_files > 1)
		printf("Total compression time: %3.3f secs\n", all_tm.get_elapsed_secs());

	if (pCSV_file)
	{
		fclose(pCSV_file);
		pCSV_file = nullptr;
	}
	delete pGlobal_codebook_data; 
	pGlobal_codebook_data = nullptr;
		
	return result;
}

static bool unpack_and_validate_ktx2_file(
	uint32_t file_index,
	const std::string& base_filename,
	uint8_vec& ktx2_file_data,
	command_line_params& opts,
	FILE* pCSV_file,
	basis_data* pGlobal_codebook_data,
	uint32_t& total_unpack_warnings,
	uint32_t& total_pvrtc_nonpow2_warnings)
{
	// TODO
	(void)pCSV_file;
	(void)file_index;

	const bool validate_flag = (opts.m_mode == cValidate);

	basist::ktx2_transcoder dec;

	if (!dec.init(ktx2_file_data.data(), ktx2_file_data.size()))
	{
		error_printf("ktx2_transcoder::init() failed! File either uses an unsupported feature, is invalid, was corrupted, or this is a bug.\n");
		return false;
	}

	if (!dec.start_transcoding())
	{
		error_printf("ktx2_transcoder::start_transcoding() failed! File either uses an unsupported feature, is invalid, was corrupted, or this is a bug.\n");
		return false;
	}
		
	printf("Resolution: %ux%u\n", dec.get_width(), dec.get_height());
	printf("Mipmap Levels: %u\n", dec.get_levels());
	printf("Texture Array Size (layers): %u\n", dec.get_layers());
	printf("Total Faces: %u (%s)\n", dec.get_faces(), (dec.get_faces() == 6) ? "CUBEMAP" : "2D");
	printf("Is Texture Video: %u\n", dec.is_video());
	
	const bool is_etc1s = dec.get_format() == basist::basis_tex_format::cETC1S;
	printf("Supercompression Format: %s\n", is_etc1s ? "ETC1S" : "UASTC");
	
	printf("Supercompression Scheme: ");
	switch (dec.get_header().m_supercompression_scheme)
	{
	case basist::KTX2_SS_NONE: printf("NONE\n"); break;
	case basist::KTX2_SS_BASISLZ: printf("BASISLZ\n"); break;
	case basist::KTX2_SS_ZSTANDARD: printf("ZSTANDARD\n"); break;
	default:
		error_printf("Invalid/unknown/unsupported\n");
		return false;
	}

	printf("Has Alpha: %u\n", (uint32_t)dec.get_has_alpha());

	printf("\nData Format Descriptor (DFD):\n");
	printf("DFD length in bytes: %u\n", dec.get_dfd().size());
	printf("DFD color model: %u\n", dec.get_dfd_color_model());
	printf("DFD color primaries: %u (%s)\n", dec.get_dfd_color_primaries(), basist::ktx2_get_df_color_primaries_str(dec.get_dfd_color_primaries()));
	printf("DFD transfer func: %u (%s)\n", dec.get_dfd_transfer_func(),
		(dec.get_dfd_transfer_func() == basist::KTX2_KHR_DF_TRANSFER_LINEAR) ? "LINEAR" : ((dec.get_dfd_transfer_func() == basist::KTX2_KHR_DF_TRANSFER_SRGB) ? "SRGB" : "?"));
	printf("DFD flags: %u\n", dec.get_dfd_flags());
	printf("DFD samples: %u\n", dec.get_dfd_total_samples());
	if (is_etc1s)
	{
		printf("DFD chan0: %s\n", basist::ktx2_get_etc1s_df_channel_id_str(dec.get_dfd_channel_id0()));
		if (dec.get_dfd_total_samples() == 2)
			printf("DFD chan1: %s\n", basist::ktx2_get_etc1s_df_channel_id_str(dec.get_dfd_channel_id1()));
	}
	else
		printf("DFD chan0: %s\n", basist::ktx2_get_uastc_df_channel_id_str(dec.get_dfd_channel_id0()));
		
	printf("DFD hex values:\n");
	for (uint32_t i = 0; i < dec.get_dfd().size(); i++)
	{
		if (i)
			printf(",");
		printf("0x%X", dec.get_dfd()[i]);
	}
	printf("\n\n");


	printf("Total key values: %u\n", dec.get_key_values().size());
	for (uint32_t i = 0; i < dec.get_key_values().size(); i++)
	{
		printf("%u. Key: \"%s\", Value length in bytes: %u", i, (const char*)dec.get_key_values()[i].m_key.data(), dec.get_key_values()[i].m_value.size());

		if (dec.get_key_values()[i].m_value.size() > 256)
			continue;
		
		bool is_ascii = true;
		for (uint32_t j = 0; j < dec.get_key_values()[i].m_value.size(); j++)
		{
			uint8_t c = dec.get_key_values()[i].m_value[j];
			if (!( 
				((c >= ' ') && (c < 0x80)) || 
				((j == dec.get_key_values()[i].m_value.size() - 1) && (!c))
				))
			{
				is_ascii = false;
				break;
			}
		}

		if (is_ascii)
		{
			uint8_vec s(dec.get_key_values()[i].m_value);
			s.push_back(0);
			printf(" Value String: \"%s\"", (const char *)s.data());
		}
		else
		{
			printf(" Value Bytes: ");
			for (uint32_t j = 0; j < dec.get_key_values()[i].m_value.size(); j++)
			{
				if (j)
					printf(",");
				printf("0x%X", dec.get_key_values()[i].m_value[j]);
			}
		}
		printf("\n");
	}

	if (is_etc1s)
	{
		printf("ETC1S header:\n");

		printf("Endpoint Count: %u, Selector Count: %u, Endpoint Length: %u, Selector Length: %u, Tables Length: %u, Extended Length: %u\n",
			(uint32_t)dec.get_etc1s_header().m_endpoint_count, (uint32_t)dec.get_etc1s_header().m_selector_count,
			(uint32_t)dec.get_etc1s_header().m_endpoints_byte_length, (uint32_t)dec.get_etc1s_header().m_selectors_byte_length,
			(uint32_t)dec.get_etc1s_header().m_tables_byte_length, (uint32_t)dec.get_etc1s_header().m_extended_byte_length);

		printf("Total ETC1S image descs: %u\n", dec.get_etc1s_image_descs().size());
		for (uint32_t i = 0; i < dec.get_etc1s_image_descs().size(); i++)
		{
			printf("%u. Flags: 0x%X, RGB Ofs: %u Len: %u, Alpha Ofs: %u, Len: %u\n", i,
				(uint32_t)dec.get_etc1s_image_descs()[i].m_image_flags,
				(uint32_t)dec.get_etc1s_image_descs()[i].m_rgb_slice_byte_offset, (uint32_t)dec.get_etc1s_image_descs()[i].m_rgb_slice_byte_length,
				(uint32_t)dec.get_etc1s_image_descs()[i].m_alpha_slice_byte_offset, (uint32_t)dec.get_etc1s_image_descs()[i].m_alpha_slice_byte_length);
		}
	}

	printf("Levels:\n");
	for (uint32_t i = 0; i < dec.get_levels(); i++)
	{
		printf("%u. Offset: %llu, Length: %llu, Uncompressed Length: %llu\n",
			i, (long long unsigned int)dec.get_level_index()[i].m_byte_offset,
			(long long unsigned int)dec.get_level_index()[i].m_byte_length,
			(long long unsigned int)dec.get_level_index()[i].m_uncompressed_byte_length);
	}

	if (opts.m_mode == cInfo)
	{
		return true;
	}

	// gpu_images[format][face][layer][level]

	basisu::vector< gpu_image_vec > gpu_images[(int)basist::transcoder_texture_format::cTFTotalTextureFormats][6];

	int first_format = 0;
	int last_format = (int)basist::transcoder_texture_format::cTFTotalTextureFormats;

	if (opts.m_etc1_only)
	{
		first_format = (int)basist::transcoder_texture_format::cTFETC1_RGB;
		last_format = first_format + 1;
	}

	const uint32_t total_layers = maximum<uint32_t>(1, dec.get_layers());

	for (int format_iter = first_format; format_iter < last_format; format_iter++)
	{
		basist::transcoder_texture_format tex_fmt = static_cast<basist::transcoder_texture_format>(format_iter);

		if (basist::basis_transcoder_format_is_uncompressed(tex_fmt))
			continue;

		if (!basis_is_format_supported(tex_fmt, dec.get_format()))
			continue;

		if (tex_fmt == basist::transcoder_texture_format::cTFBC7_ALT)
			continue;

		for (uint32_t face_index = 0; face_index < dec.get_faces(); face_index++)
		{
			gpu_images[(int)tex_fmt][face_index].resize(total_layers);

			for (uint32_t layer_index = 0; layer_index < total_layers; layer_index++)
				gpu_images[(int)tex_fmt][face_index][layer_index].resize(dec.get_levels());
		}
	}

	// Now transcode the file to all supported texture formats and save mipmapped KTX files
	for (int format_iter = first_format; format_iter < last_format; format_iter++)
	{
		const basist::transcoder_texture_format transcoder_tex_fmt = static_cast<basist::transcoder_texture_format>(format_iter);

		if (basist::basis_transcoder_format_is_uncompressed(transcoder_tex_fmt))
			continue;
		if (!basis_is_format_supported(transcoder_tex_fmt, dec.get_format()))
			continue;
		if (transcoder_tex_fmt == basist::transcoder_texture_format::cTFBC7_ALT)
			continue;

		for (uint32_t level_index = 0; level_index < dec.get_levels(); level_index++)
		{
			for (uint32_t layer_index = 0; layer_index < total_layers; layer_index++)
			{
				for (uint32_t face_index = 0; face_index < dec.get_faces(); face_index++)
				{
					basist::ktx2_image_level_info level_info;

					if (!dec.get_image_level_info(level_info, level_index, layer_index, face_index))
					{
						error_printf("Failed retrieving image level information (%u %u %u)!\n", layer_index, level_index, face_index);
						return false;
					}

					if ((transcoder_tex_fmt == basist::transcoder_texture_format::cTFPVRTC1_4_RGB) || (transcoder_tex_fmt == basist::transcoder_texture_format::cTFPVRTC1_4_RGBA))
					{
						if (!is_pow2(level_info.m_width) || !is_pow2(level_info.m_height))
						{
							total_pvrtc_nonpow2_warnings++;

							printf("Warning: Will not transcode image %u level %u res %ux%u to PVRTC1 (one or more dimension is not a power of 2)\n", layer_index, level_index, level_info.m_width, level_info.m_height);

							// Can't transcode this image level to PVRTC because it's not a pow2 (we're going to support transcoding non-pow2 to the next larger pow2 soon)
							continue;
						}
					}

					basisu::texture_format tex_fmt = basis_get_basisu_texture_format(transcoder_tex_fmt);

					gpu_image& gi = gpu_images[(int)transcoder_tex_fmt][face_index][layer_index][level_index];
					gi.init(tex_fmt, level_info.m_orig_width, level_info.m_orig_height);

					// Fill the buffer with psuedo-random bytes, to help more visibly detect cases where the transcoder fails to write to part of the output.
					fill_buffer_with_random_bytes(gi.get_ptr(), gi.get_size_in_bytes());

					uint32_t decode_flags = 0;

					if (!dec.transcode_image_level(level_index, layer_index, face_index, gi.get_ptr(), gi.get_total_blocks(), transcoder_tex_fmt, decode_flags))
					{
						error_printf("Failed transcoding image level (%u %u %u %u)!\n", layer_index, level_index, face_index, format_iter);
						return false;
					}

					printf("Transcode of layer %u level %u face %u res %ux%u format %s succeeded\n", layer_index, level_index, face_index, level_info.m_orig_width, level_info.m_orig_height, basist::basis_get_format_name(transcoder_tex_fmt));
				}

			} // format_iter

		} // level_index

	} // image_info

	if (!validate_flag)
	{
		// Now write KTX files and unpack them to individual PNG's
		const bool is_cubemap_array = (dec.get_faces() > 1) && (total_layers > 1);

		for (int format_iter = first_format; format_iter < last_format; format_iter++)
		{
			const basist::transcoder_texture_format transcoder_tex_fmt = static_cast<basist::transcoder_texture_format>(format_iter);

			if (basist::basis_transcoder_format_is_uncompressed(transcoder_tex_fmt))
				continue;
			if (!basis_is_format_supported(transcoder_tex_fmt, dec.get_format()))
				continue;
			if (transcoder_tex_fmt == basist::transcoder_texture_format::cTFBC7_ALT)
				continue;

			if ((!opts.m_no_ktx) && (is_cubemap_array))
			{
				// No KTX tool that we know of supports cubemap arrays, so write individual cubemap files.
				for (uint32_t layer_index = 0; layer_index < total_layers; layer_index++)
				{
					basisu::vector<gpu_image_vec> cubemap;
					for (uint32_t face_index = 0; face_index < 6; face_index++)
						cubemap.push_back(gpu_images[format_iter][face_index][layer_index]);

					std::string ktx_filename(base_filename + string_format("_transcoded_cubemap_%s_%u.ktx", basist::basis_get_format_name(transcoder_tex_fmt), layer_index));

					if (!write_compressed_texture_file(ktx_filename.c_str(), cubemap, true))
					{
						error_printf("Failed writing KTX file \"%s\"!\n", ktx_filename.c_str());
						return false;
					}
					printf("Wrote KTX file \"%s\"\n", ktx_filename.c_str());
				}
			}

			for (uint32_t layer_index = 0; layer_index < total_layers; layer_index++)
			{
				for (uint32_t face_index = 0; face_index < dec.get_faces(); face_index++)
				{
					gpu_image_vec& gi = gpu_images[format_iter][face_index][layer_index];

					if (!gi.size())
						continue;

					uint32_t level;
					for (level = 0; level < gi.size(); level++)
						if (!gi[level].get_total_blocks())
							break;

					if (level < gi.size())
						continue;

					if ((!opts.m_no_ktx) && (!is_cubemap_array))
					{
						std::string ktx_filename(base_filename + string_format("_transcoded_%s_%04u.ktx", basist::basis_get_format_name(transcoder_tex_fmt), layer_index));
						if (!write_compressed_texture_file(ktx_filename.c_str(), gi))
						{
							error_printf("Failed writing KTX file \"%s\"!\n", ktx_filename.c_str());
							return false;
						}
						printf("Wrote KTX file \"%s\"\n", ktx_filename.c_str());
					}

					for (uint32_t level_index = 0; level_index < gi.size(); level_index++)
					{
						basist::ktx2_image_level_info level_info;

						if (!dec.get_image_level_info(level_info, level_index, layer_index, face_index))
						{
							error_printf("Failed retrieving image level information (%u %u %u)!\n", layer_index, level_index, face_index);
							return false;
						}

						image u;
						if (!gi[level_index].unpack(u))
						{
							printf("Warning: Failed unpacking GPU texture data (%u %u %u %u). Unpacking as much as possible.\n", format_iter, layer_index, level_index, face_index);
							total_unpack_warnings++;
						}
						//u.crop(level_info.m_orig_width, level_info.m_orig_height);

						bool is_astc = (transcoder_tex_fmt == basist::transcoder_texture_format::cTFASTC_4x4_RGBA);
						bool write_png = true;
#if !BASISU_USE_ASTC_DECOMPRESS
						if (is_astc)
							write_png = false;
#endif

						if ((!opts.m_ktx_only) && (write_png))
						{
							std::string rgb_filename;
							if (gi.size() > 1)
								rgb_filename = base_filename + string_format("_unpacked_rgb_%s_%u_%u_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), level_index, face_index, layer_index);
							else
								rgb_filename = base_filename + string_format("_unpacked_rgb_%s_%u_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), face_index, layer_index);
							if (!save_png(rgb_filename, u, cImageSaveIgnoreAlpha))
							{
								error_printf("Failed writing to PNG file \"%s\"\n", rgb_filename.c_str());
								delete pGlobal_codebook_data; pGlobal_codebook_data = nullptr;
								return false;
							}
							printf("Wrote PNG file \"%s\"\n", rgb_filename.c_str());
						}

						if ((transcoder_tex_fmt == basist::transcoder_texture_format::cTFFXT1_RGB) && (opts.m_write_out))
						{
							std::string out_filename;
							if (gi.size() > 1)
								out_filename = base_filename + string_format("_unpacked_rgb_%s_%u_%u_%04u.out", basist::basis_get_format_name(transcoder_tex_fmt), level_index, face_index, layer_index);
							else
								out_filename = base_filename + string_format("_unpacked_rgb_%s_%u_%04u.out", basist::basis_get_format_name(transcoder_tex_fmt), face_index, layer_index);
							if (!write_3dfx_out_file(out_filename.c_str(), gi[level_index]))
							{
								error_printf("Failed writing to OUT file \"%s\"\n", out_filename.c_str());
								return false;
							}
							printf("Wrote .OUT file \"%s\"\n", out_filename.c_str());
						}

						if (basis_transcoder_format_has_alpha(transcoder_tex_fmt) && (!opts.m_ktx_only) && (write_png))
						{
							std::string a_filename;
							if (gi.size() > 1)
								a_filename = base_filename + string_format("_unpacked_a_%s_%u_%u_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), level_index, face_index, layer_index);
							else
								a_filename = base_filename + string_format("_unpacked_a_%s_%u_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), face_index, layer_index);
							if (!save_png(a_filename, u, cImageSaveGrayscale, 3))
							{
								error_printf("Failed writing to PNG file \"%s\"\n", a_filename.c_str());
								return false;
							}
							printf("Wrote PNG file \"%s\"\n", a_filename.c_str());

							std::string rgba_filename;
							if (gi.size() > 1)
								rgba_filename = base_filename + string_format("_unpacked_rgba_%s_%u_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), level_index, face_index, layer_index);
							else
								rgba_filename = base_filename + string_format("_unpacked_rgba_%s_%u_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), face_index, layer_index);
							if (!save_png(rgba_filename, u))
							{
								error_printf("Failed writing to PNG file \"%s\"\n", rgba_filename.c_str());
								return false;
							}
							printf("Wrote PNG file \"%s\"\n", rgba_filename.c_str());
						}

					} // level_index

				} // face_index

			} // layer_index

		} // format_iter

	} // if (!validate_flag)

	return true;
}

static bool unpack_and_validate_basis_file(
	uint32_t file_index,
	const std::string &base_filename,
	uint8_vec &basis_file_data,
	command_line_params& opts, 
	FILE *pCSV_file,
	basis_data* pGlobal_codebook_data,
	uint32_t &total_unpack_warnings,
	uint32_t &total_pvrtc_nonpow2_warnings)
{
	const bool validate_flag = (opts.m_mode == cValidate);

	basist::basisu_transcoder dec;

	if (pGlobal_codebook_data)
	{
		dec.set_global_codebooks(&pGlobal_codebook_data->m_transcoder.get_lowlevel_etc1s_decoder());
	}

	if (!opts.m_fuzz_testing)
	{
		// Skip the full validation, which CRC16's the entire file.

		// Validate the file - note this isn't necessary for transcoding
		if (!dec.validate_file_checksums(&basis_file_data[0], (uint32_t)basis_file_data.size(), true))
		{
			error_printf("File version is unsupported, or file failed one or more CRC checks!\n");
			
			return false;
		}
	}

	printf("File version and CRC checks succeeded\n");

	basist::basisu_file_info fileinfo;
	if (!dec.get_file_info(&basis_file_data[0], (uint32_t)basis_file_data.size(), fileinfo))
	{
		error_printf("Failed retrieving Basis file information!\n");
		return false;
	}

	assert(fileinfo.m_total_images == fileinfo.m_image_mipmap_levels.size());
	assert(fileinfo.m_total_images == dec.get_total_images(&basis_file_data[0], (uint32_t)basis_file_data.size()));

	printf("File info:\n");
	printf("  Version: %X\n", fileinfo.m_version);
	printf("  Total header size: %u\n", fileinfo.m_total_header_size);
	printf("  Total selectors: %u\n", fileinfo.m_total_selectors);
	printf("  Selector codebook size: %u\n", fileinfo.m_selector_codebook_size);
	printf("  Total endpoints: %u\n", fileinfo.m_total_endpoints);
	printf("  Endpoint codebook size: %u\n", fileinfo.m_endpoint_codebook_size);
	printf("  Tables size: %u\n", fileinfo.m_tables_size);
	printf("  Slices size: %u\n", fileinfo.m_slices_size);
	printf("  Texture format: %s\n", (fileinfo.m_tex_format == basist::basis_tex_format::cUASTC4x4) ? "UASTC" : "ETC1S");
	printf("  Texture type: %s\n", basist::basis_get_texture_type_name(fileinfo.m_tex_type));
	printf("  us per frame: %u (%f fps)\n", fileinfo.m_us_per_frame, fileinfo.m_us_per_frame ? (1.0f / ((float)fileinfo.m_us_per_frame / 1000000.0f)) : 0.0f);
	printf("  Total slices: %u\n", (uint32_t)fileinfo.m_slice_info.size());
	printf("  Total images: %i\n", fileinfo.m_total_images);
	printf("  Y Flipped: %u, Has alpha slices: %u\n", fileinfo.m_y_flipped, fileinfo.m_has_alpha_slices);
	printf("  userdata0: 0x%X userdata1: 0x%X\n", fileinfo.m_userdata0, fileinfo.m_userdata1);
	printf("  Per-image mipmap levels: ");
	for (uint32_t i = 0; i < fileinfo.m_total_images; i++)
		printf("%u ", fileinfo.m_image_mipmap_levels[i]);
	printf("\n");

	uint32_t total_texels = 0;

	printf("\nImage info:\n");
	for (uint32_t i = 0; i < fileinfo.m_total_images; i++)
	{
		basist::basisu_image_info ii;
		if (!dec.get_image_info(&basis_file_data[0], (uint32_t)basis_file_data.size(), ii, i))
		{
			error_printf("get_image_info() failed!\n");
			return false;
		}

		printf("Image %u: MipLevels: %u OrigDim: %ux%u, BlockDim: %ux%u, FirstSlice: %u, HasAlpha: %u\n", i, ii.m_total_levels, ii.m_orig_width, ii.m_orig_height,
			ii.m_num_blocks_x, ii.m_num_blocks_y, ii.m_first_slice_index, (uint32_t)ii.m_alpha_flag);

		total_texels += ii.m_width * ii.m_height;
	}

	printf("\nSlice info:\n");

	for (uint32_t i = 0; i < fileinfo.m_slice_info.size(); i++)
	{
		const basist::basisu_slice_info& sliceinfo = fileinfo.m_slice_info[i];
		printf("%u: OrigWidthHeight: %ux%u, BlockDim: %ux%u, TotalBlocks: %u, Compressed size: %u, Image: %u, Level: %u, UnpackedCRC16: 0x%X, alpha: %u, iframe: %i\n",
			i,
			sliceinfo.m_orig_width, sliceinfo.m_orig_height,
			sliceinfo.m_num_blocks_x, sliceinfo.m_num_blocks_y,
			sliceinfo.m_total_blocks,
			sliceinfo.m_compressed_size,
			sliceinfo.m_image_index, sliceinfo.m_level_index,
			sliceinfo.m_unpacked_slice_crc16,
			(uint32_t)sliceinfo.m_alpha_flag,
			(uint32_t)sliceinfo.m_iframe_flag);
	}
	printf("\n");

	size_t comp_size = 0;
	void* pComp_data = tdefl_compress_mem_to_heap(&basis_file_data[0], basis_file_data.size(), &comp_size, TDEFL_MAX_PROBES_MASK);// TDEFL_DEFAULT_MAX_PROBES);
	mz_free(pComp_data);

	const float basis_bits_per_texel = basis_file_data.size() * 8.0f / total_texels;
	const float comp_bits_per_texel = comp_size * 8.0f / total_texels;

	printf("Original size: %u, bits per texel: %3.3f\nCompressed size (Deflate): %u, bits per texel: %3.3f\n", (uint32_t)basis_file_data.size(), basis_bits_per_texel, (uint32_t)comp_size, comp_bits_per_texel);

	if (opts.m_mode == cInfo)
	{
		return true;
	}

	if ((fileinfo.m_etc1s) && (fileinfo.m_selector_codebook_size == 0) && (fileinfo.m_endpoint_codebook_size == 0))
	{
		// File is ETC1S and uses global codebooks - make sure we loaded one
		if (!pGlobal_codebook_data)
		{
			error_printf("ETC1S file uses global codebooks, but none were loaded (see the -use_global_codebooks option)\n");
			return false;
		}

		if ((pGlobal_codebook_data->m_transcoder.get_lowlevel_etc1s_decoder().get_endpoints().size() != fileinfo.m_total_endpoints) ||
			(pGlobal_codebook_data->m_transcoder.get_lowlevel_etc1s_decoder().get_selectors().size() != fileinfo.m_total_selectors))
		{
			error_printf("Supplied global codebook is not compatible with this file\n");
			return false;
		}
	}

	interval_timer tm;
	tm.start();

	if (!dec.start_transcoding(&basis_file_data[0], (uint32_t)basis_file_data.size()))
	{
		error_printf("start_transcoding() failed!\n");
		return false;
	}

	const double start_transcoding_time_ms = tm.get_elapsed_ms();

	printf("start_transcoding time: %3.3f ms\n", start_transcoding_time_ms);

	basisu::vector< gpu_image_vec > gpu_images[(int)basist::transcoder_texture_format::cTFTotalTextureFormats];
	
	double total_format_transcoding_time_ms[(int)basist::transcoder_texture_format::cTFTotalTextureFormats];
	clear_obj(total_format_transcoding_time_ms);

	int first_format = 0;
	int last_format = (int)basist::transcoder_texture_format::cTFTotalTextureFormats;

	if (opts.m_etc1_only)
	{
		first_format = (int)basist::transcoder_texture_format::cTFETC1_RGB;
		last_format = first_format + 1;
	}

	if ((pCSV_file) && (file_index == 0))
	{
		std::string desc;
		desc = "filename,basis_bitrate,comp_bitrate,images,levels,slices,start_transcoding_time,";
		for (int format_iter = first_format; format_iter < last_format; format_iter++)
		{
			const basist::transcoder_texture_format transcoder_tex_fmt = static_cast<basist::transcoder_texture_format>(format_iter);

			if (!basis_is_format_supported(transcoder_tex_fmt, fileinfo.m_tex_format))
				continue;
			if (transcoder_tex_fmt == basist::transcoder_texture_format::cTFBC7_ALT)
				continue;

			desc += std::string(basis_get_format_name(transcoder_tex_fmt));
			if (format_iter != last_format - 1)
				desc += ",";
		}
		fprintf(pCSV_file, "%s\n", desc.c_str());
	}

	for (int format_iter = first_format; format_iter < last_format; format_iter++)
	{
		basist::transcoder_texture_format tex_fmt = static_cast<basist::transcoder_texture_format>(format_iter);

		if (basist::basis_transcoder_format_is_uncompressed(tex_fmt))
			continue;

		if (!basis_is_format_supported(tex_fmt, fileinfo.m_tex_format))
			continue;

		if (tex_fmt == basist::transcoder_texture_format::cTFBC7_ALT)
			continue;

		gpu_images[(int)tex_fmt].resize(fileinfo.m_total_images);

		for (uint32_t image_index = 0; image_index < fileinfo.m_total_images; image_index++)
			gpu_images[(int)tex_fmt][image_index].resize(fileinfo.m_image_mipmap_levels[image_index]);
	}

	// Now transcode the file to all supported texture formats and save mipmapped KTX files
	for (int format_iter = first_format; format_iter < last_format; format_iter++)
	{
		const basist::transcoder_texture_format transcoder_tex_fmt = static_cast<basist::transcoder_texture_format>(format_iter);

		if (basist::basis_transcoder_format_is_uncompressed(transcoder_tex_fmt))
			continue;
		if (!basis_is_format_supported(transcoder_tex_fmt, fileinfo.m_tex_format))
			continue;
		if (transcoder_tex_fmt == basist::transcoder_texture_format::cTFBC7_ALT)
			continue;

		for (uint32_t image_index = 0; image_index < fileinfo.m_total_images; image_index++)
		{
			for (uint32_t level_index = 0; level_index < fileinfo.m_image_mipmap_levels[image_index]; level_index++)
			{
				basist::basisu_image_level_info level_info;

				if (!dec.get_image_level_info(&basis_file_data[0], (uint32_t)basis_file_data.size(), level_info, image_index, level_index))
				{
					error_printf("Failed retrieving image level information (%u %u)!\n", image_index, level_index);
					return false;
				}

				if ((transcoder_tex_fmt == basist::transcoder_texture_format::cTFPVRTC1_4_RGB) || (transcoder_tex_fmt == basist::transcoder_texture_format::cTFPVRTC1_4_RGBA))
				{
					if (!is_pow2(level_info.m_width) || !is_pow2(level_info.m_height))
					{
						total_pvrtc_nonpow2_warnings++;

						printf("Warning: Will not transcode image %u level %u res %ux%u to PVRTC1 (one or more dimension is not a power of 2)\n", image_index, level_index, level_info.m_width, level_info.m_height);

						// Can't transcode this image level to PVRTC because it's not a pow2 (we're going to support transcoding non-pow2 to the next larger pow2 soon)
						continue;
					}
				}

				basisu::texture_format tex_fmt = basis_get_basisu_texture_format(transcoder_tex_fmt);

				gpu_image& gi = gpu_images[(int)transcoder_tex_fmt][image_index][level_index];
				gi.init(tex_fmt, level_info.m_orig_width, level_info.m_orig_height);

				// Fill the buffer with psuedo-random bytes, to help more visibly detect cases where the transcoder fails to write to part of the output.
				fill_buffer_with_random_bytes(gi.get_ptr(), gi.get_size_in_bytes());

				uint32_t decode_flags = 0;

				tm.start();

				if (!dec.transcode_image_level(&basis_file_data[0], (uint32_t)basis_file_data.size(), image_index, level_index, gi.get_ptr(), gi.get_total_blocks(), transcoder_tex_fmt, decode_flags))
				{
					error_printf("Failed transcoding image level (%u %u %u)!\n", image_index, level_index, format_iter);
					return false;
				}

				double total_transcode_time = tm.get_elapsed_ms();

				total_format_transcoding_time_ms[format_iter] += total_transcode_time;

				printf("Transcode of image %u level %u res %ux%u format %s succeeded in %3.3f ms\n", image_index, level_index, level_info.m_orig_width, level_info.m_orig_height, basist::basis_get_format_name(transcoder_tex_fmt), total_transcode_time);

			} // format_iter

		} // level_index

	} // image_info

	// Upack UASTC files separarely, to validate we can transcode slices to UASTC and unpack them to pixels.
	// This is a special path because UASTC is not yet a valid transcoder_texture_format, but a lower-level block_format.
	if (fileinfo.m_tex_format == basist::basis_tex_format::cUASTC4x4)
	{
		for (uint32_t image_index = 0; image_index < fileinfo.m_total_images; image_index++)
		{
			for (uint32_t level_index = 0; level_index < fileinfo.m_image_mipmap_levels[image_index]; level_index++)
			{
				basist::basisu_image_level_info level_info;

				if (!dec.get_image_level_info(&basis_file_data[0], (uint32_t)basis_file_data.size(), level_info, image_index, level_index))
				{
					error_printf("Failed retrieving image level information (%u %u)!\n", image_index, level_index);
					return false;
				}

				gpu_image gi;
				gi.init(basisu::texture_format::cUASTC4x4, level_info.m_orig_width, level_info.m_orig_height);

				// Fill the buffer with psuedo-random bytes, to help more visibly detect cases where the transcoder fails to write to part of the output.
				fill_buffer_with_random_bytes(gi.get_ptr(), gi.get_size_in_bytes());

				//uint32_t decode_flags = 0;

				tm.start();

				if (!dec.transcode_slice(
					&basis_file_data[0], (uint32_t)basis_file_data.size(), 
					level_info.m_first_slice_index, gi.get_ptr(), gi.get_total_blocks(), basist::block_format::cUASTC_4x4, gi.get_bytes_per_block()))
				{
					error_printf("Failed transcoding image level (%u %u) to UASTC!\n", image_index, level_index);
					return false;
				}

				double total_transcode_time = tm.get_elapsed_ms();

				printf("Transcode of image %u level %u res %ux%u format UASTC_4x4 succeeded in %3.3f ms\n", image_index, level_index, level_info.m_orig_width, level_info.m_orig_height, total_transcode_time);

				if ((!validate_flag) && (!opts.m_ktx_only))
				{
					image u;
					if (!gi.unpack(u))
					{
						error_printf("Warning: Failed unpacking GPU texture data (%u %u) to UASTC. \n", image_index, level_index);
						return false;
					}
					//u.crop(level_info.m_orig_width, level_info.m_orig_height);

					std::string rgb_filename;
					if (fileinfo.m_image_mipmap_levels[image_index] > 1)
						rgb_filename = base_filename + string_format("_unpacked_rgb_UASTC_4x4_%u_%04u.png", level_index, image_index);
					else
						rgb_filename = base_filename + string_format("_unpacked_rgb_UASTC_4x4_%04u.png", image_index);

					if (!save_png(rgb_filename, u, cImageSaveIgnoreAlpha))
					{
						error_printf("Failed writing to PNG file \"%s\"\n", rgb_filename.c_str());
						delete pGlobal_codebook_data; pGlobal_codebook_data = nullptr;
						return false;
					}
					printf("Wrote PNG file \"%s\"\n", rgb_filename.c_str());

					std::string alpha_filename;
					if (fileinfo.m_image_mipmap_levels[image_index] > 1)
						alpha_filename = base_filename + string_format("_unpacked_a_UASTC_4x4_%u_%04u.png", level_index, image_index);
					else
						alpha_filename = base_filename + string_format("_unpacked_a_UASTC_4x4_%04u.png", image_index);
					if (!save_png(alpha_filename, u, cImageSaveGrayscale, 3))
					{
						error_printf("Failed writing to PNG file \"%s\"\n", rgb_filename.c_str());
						delete pGlobal_codebook_data; pGlobal_codebook_data = nullptr;
						return false;
					}
					printf("Wrote PNG file \"%s\"\n", alpha_filename.c_str());

				}
			}
		}
	}

	if (!validate_flag)
	{
		// Now write KTX files and unpack them to individual PNG's

		for (int format_iter = first_format; format_iter < last_format; format_iter++)
		{
			const basist::transcoder_texture_format transcoder_tex_fmt = static_cast<basist::transcoder_texture_format>(format_iter);

			if (basist::basis_transcoder_format_is_uncompressed(transcoder_tex_fmt))
				continue;
			if (!basis_is_format_supported(transcoder_tex_fmt, fileinfo.m_tex_format))
				continue;
			if (transcoder_tex_fmt == basist::transcoder_texture_format::cTFBC7_ALT)
				continue;

			if ((!opts.m_no_ktx) && (fileinfo.m_tex_type == basist::cBASISTexTypeCubemapArray))
			{
				// No KTX tool that we know of supports cubemap arrays, so write individual cubemap files.
				for (uint32_t image_index = 0; image_index < fileinfo.m_total_images; image_index += 6)
				{
					basisu::vector<gpu_image_vec> cubemap;
					for (uint32_t i = 0; i < 6; i++)
						cubemap.push_back(gpu_images[format_iter][image_index + i]);

					std::string ktx_filename(base_filename + string_format("_transcoded_cubemap_%s_%u.ktx", basist::basis_get_format_name(transcoder_tex_fmt), image_index / 6));
					if (!write_compressed_texture_file(ktx_filename.c_str(), cubemap, true))
					{
						error_printf("Failed writing KTX file \"%s\"!\n", ktx_filename.c_str());
						return false;
					}
					printf("Wrote KTX file \"%s\"\n", ktx_filename.c_str());
				}
			}

			for (uint32_t image_index = 0; image_index < fileinfo.m_total_images; image_index++)
			{
				gpu_image_vec& gi = gpu_images[format_iter][image_index];

				if (!gi.size())
					continue;

				uint32_t level;
				for (level = 0; level < gi.size(); level++)
					if (!gi[level].get_total_blocks())
						break;

				if (level < gi.size())
					continue;

				if ((!opts.m_no_ktx) && (fileinfo.m_tex_type != basist::cBASISTexTypeCubemapArray))
				{
					std::string ktx_filename(base_filename + string_format("_transcoded_%s_%04u.ktx", basist::basis_get_format_name(transcoder_tex_fmt), image_index));
					if (!write_compressed_texture_file(ktx_filename.c_str(), gi))
					{
						error_printf("Failed writing KTX file \"%s\"!\n", ktx_filename.c_str());
						return false;
					}
					printf("Wrote KTX file \"%s\"\n", ktx_filename.c_str());
				}

				for (uint32_t level_index = 0; level_index < gi.size(); level_index++)
				{
					basist::basisu_image_level_info level_info;

					if (!dec.get_image_level_info(&basis_file_data[0], (uint32_t)basis_file_data.size(), level_info, image_index, level_index))
					{
						error_printf("Failed retrieving image level information (%u %u)!\n", image_index, level_index);
						return false;
					}

					image u;
					if (!gi[level_index].unpack(u))
					{
						printf("Warning: Failed unpacking GPU texture data (%u %u %u). Unpacking as much as possible.\n", format_iter, image_index, level_index);
						total_unpack_warnings++;
					}
					//u.crop(level_info.m_orig_width, level_info.m_orig_height);

					bool is_astc = (transcoder_tex_fmt == basist::transcoder_texture_format::cTFASTC_4x4_RGBA);
					bool write_png = true;
#if !BASISU_USE_ASTC_DECOMPRESS
					if (is_astc)
						write_png = false;
#endif

					if ((!opts.m_ktx_only) && (write_png))
					{
						std::string rgb_filename;
						if (gi.size() > 1)
							rgb_filename = base_filename + string_format("_unpacked_rgb_%s_%u_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), level_index, image_index);
						else
							rgb_filename = base_filename + string_format("_unpacked_rgb_%s_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), image_index);
						if (!save_png(rgb_filename, u, cImageSaveIgnoreAlpha))
						{
							error_printf("Failed writing to PNG file \"%s\"\n", rgb_filename.c_str());
							delete pGlobal_codebook_data; pGlobal_codebook_data = nullptr;
							return false;
						}
						printf("Wrote PNG file \"%s\"\n", rgb_filename.c_str());
					}

					if ((transcoder_tex_fmt == basist::transcoder_texture_format::cTFFXT1_RGB) && (opts.m_write_out))
					{
						std::string out_filename;
						if (gi.size() > 1)
							out_filename = base_filename + string_format("_unpacked_rgb_%s_%u_%04u.out", basist::basis_get_format_name(transcoder_tex_fmt), level_index, image_index);
						else
							out_filename = base_filename + string_format("_unpacked_rgb_%s_%04u.out", basist::basis_get_format_name(transcoder_tex_fmt), image_index);
						if (!write_3dfx_out_file(out_filename.c_str(), gi[level_index]))
						{
							error_printf("Failed writing to OUT file \"%s\"\n", out_filename.c_str());
							return false;
						}
						printf("Wrote .OUT file \"%s\"\n", out_filename.c_str());
					}

					if (basis_transcoder_format_has_alpha(transcoder_tex_fmt) && (!opts.m_ktx_only) && (write_png))
					{
						std::string a_filename;
						if (gi.size() > 1)
							a_filename = base_filename + string_format("_unpacked_a_%s_%u_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), level_index, image_index);
						else
							a_filename = base_filename + string_format("_unpacked_a_%s_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), image_index);
						if (!save_png(a_filename, u, cImageSaveGrayscale, 3))
						{
							error_printf("Failed writing to PNG file \"%s\"\n", a_filename.c_str());
							return false;
						}
						printf("Wrote PNG file \"%s\"\n", a_filename.c_str());

						std::string rgba_filename;
						if (gi.size() > 1)
							rgba_filename = base_filename + string_format("_unpacked_rgba_%s_%u_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), level_index, image_index);
						else
							rgba_filename = base_filename + string_format("_unpacked_rgba_%s_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), image_index);
						if (!save_png(rgba_filename, u))
						{
							error_printf("Failed writing to PNG file \"%s\"\n", rgba_filename.c_str());
							return false;
						}
						printf("Wrote PNG file \"%s\"\n", rgba_filename.c_str());
					}

				} // level_index

			} // image_index

		} // format_iter

	} // if (!validate_flag)

	uint32_t max_mipmap_levels = 0;

	if (!opts.m_etc1_only)
	{
		// Now unpack to RGBA using the transcoder itself to do the unpacking to raster images
		for (uint32_t image_index = 0; image_index < fileinfo.m_total_images; image_index++)
		{
			for (uint32_t level_index = 0; level_index < fileinfo.m_image_mipmap_levels[image_index]; level_index++)
			{
				const basist::transcoder_texture_format transcoder_tex_fmt = basist::transcoder_texture_format::cTFRGBA32;

				basist::basisu_image_level_info level_info;

				if (!dec.get_image_level_info(&basis_file_data[0], (uint32_t)basis_file_data.size(), level_info, image_index, level_index))
				{
					error_printf("Failed retrieving image level information (%u %u)!\n", image_index, level_index);
					return false;
				}

				image img(level_info.m_orig_width, level_info.m_orig_height);

				fill_buffer_with_random_bytes(&img(0, 0), img.get_total_pixels() * sizeof(uint32_t));

				tm.start();

				if (!dec.transcode_image_level(&basis_file_data[0], (uint32_t)basis_file_data.size(), image_index, level_index, &img(0, 0).r, img.get_total_pixels(), transcoder_tex_fmt, 0, img.get_pitch(), nullptr, img.get_height()))
				{
					error_printf("Failed transcoding image level (%u %u %u)!\n", image_index, level_index, transcoder_tex_fmt);
					return false;
				}

				double total_transcode_time = tm.get_elapsed_ms();

				total_format_transcoding_time_ms[(int)transcoder_tex_fmt] += total_transcode_time;

				printf("Transcode of image %u level %u res %ux%u format %s succeeded in %3.3f ms\n", image_index, level_index, level_info.m_orig_width, level_info.m_orig_height, basist::basis_get_format_name(transcoder_tex_fmt), total_transcode_time);

				if ((!validate_flag) && (!opts.m_ktx_only))
				{
					std::string rgb_filename(base_filename + string_format("_unpacked_rgb_%s_%u_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), level_index, image_index));
					if (!save_png(rgb_filename, img, cImageSaveIgnoreAlpha))
					{
						error_printf("Failed writing to PNG file \"%s\"\n", rgb_filename.c_str());
						return false;
					}
					printf("Wrote PNG file \"%s\"\n", rgb_filename.c_str());

					std::string a_filename(base_filename + string_format("_unpacked_a_%s_%u_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), level_index, image_index));
					if (!save_png(a_filename, img, cImageSaveGrayscale, 3))
					{
						error_printf("Failed writing to PNG file \"%s\"\n", a_filename.c_str());
						return false;
					}
					printf("Wrote PNG file \"%s\"\n", a_filename.c_str());
				}

			} // level_index
		} // image_index

		// Now unpack to RGB565 using the transcoder itself to do the unpacking to raster images
		for (uint32_t image_index = 0; image_index < fileinfo.m_total_images; image_index++)
		{
			for (uint32_t level_index = 0; level_index < fileinfo.m_image_mipmap_levels[image_index]; level_index++)
			{
				const basist::transcoder_texture_format transcoder_tex_fmt = basist::transcoder_texture_format::cTFRGB565;

				basist::basisu_image_level_info level_info;

				if (!dec.get_image_level_info(&basis_file_data[0], (uint32_t)basis_file_data.size(), level_info, image_index, level_index))
				{
					error_printf("Failed retrieving image level information (%u %u)!\n", image_index, level_index);
					return false;
				}

				basisu::vector<uint16_t> packed_img(level_info.m_orig_width * level_info.m_orig_height);

				fill_buffer_with_random_bytes(&packed_img[0], packed_img.size() * sizeof(uint16_t));

				tm.start();

				if (!dec.transcode_image_level(&basis_file_data[0], (uint32_t)basis_file_data.size(), image_index, level_index, &packed_img[0], (uint32_t)packed_img.size(), transcoder_tex_fmt, 0, level_info.m_orig_width, nullptr, level_info.m_orig_height))
				{
					error_printf("Failed transcoding image level (%u %u %u)!\n", image_index, level_index, transcoder_tex_fmt);
					return false;
				}

				double total_transcode_time = tm.get_elapsed_ms();

				total_format_transcoding_time_ms[(int)transcoder_tex_fmt] += total_transcode_time;

				image img(level_info.m_orig_width, level_info.m_orig_height);
				for (uint32_t y = 0; y < level_info.m_orig_height; y++)
				{
					for (uint32_t x = 0; x < level_info.m_orig_width; x++)
					{
						const uint16_t p = packed_img[x + y * level_info.m_orig_width];
						uint32_t r = p >> 11, g = (p >> 5) & 63, b = p & 31;
						r = (r << 3) | (r >> 2);
						g = (g << 2) | (g >> 4);
						b = (b << 3) | (b >> 2);
						img(x, y).set(r, g, b, 255);
					}
				}

				printf("Transcode of image %u level %u res %ux%u format %s succeeded in %3.3f ms\n", image_index, level_index, level_info.m_orig_width, level_info.m_orig_height, basist::basis_get_format_name(transcoder_tex_fmt), total_transcode_time);

				if ((!validate_flag) && (!opts.m_ktx_only))
				{
					std::string rgb_filename(base_filename + string_format("_unpacked_rgb_%s_%u_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), level_index, image_index));
					if (!save_png(rgb_filename, img, cImageSaveIgnoreAlpha))
					{
						error_printf("Failed writing to PNG file \"%s\"\n", rgb_filename.c_str());
						return false;
					}
					printf("Wrote PNG file \"%s\"\n", rgb_filename.c_str());
				}

			} // level_index
		} // image_index

		// Now unpack to RGBA4444 using the transcoder itself to do the unpacking to raster images
		for (uint32_t image_index = 0; image_index < fileinfo.m_total_images; image_index++)
		{
			for (uint32_t level_index = 0; level_index < fileinfo.m_image_mipmap_levels[image_index]; level_index++)
			{
				max_mipmap_levels = basisu::maximum(max_mipmap_levels, fileinfo.m_image_mipmap_levels[image_index]);

				const basist::transcoder_texture_format transcoder_tex_fmt = basist::transcoder_texture_format::cTFRGBA4444;

				basist::basisu_image_level_info level_info;

				if (!dec.get_image_level_info(&basis_file_data[0], (uint32_t)basis_file_data.size(), level_info, image_index, level_index))
				{
					error_printf("Failed retrieving image level information (%u %u)!\n", image_index, level_index);
					return false;
				}

				basisu::vector<uint16_t> packed_img(level_info.m_orig_width * level_info.m_orig_height);

				fill_buffer_with_random_bytes(&packed_img[0], packed_img.size() * sizeof(uint16_t));

				tm.start();

				if (!dec.transcode_image_level(&basis_file_data[0], (uint32_t)basis_file_data.size(), image_index, level_index, &packed_img[0], (uint32_t)packed_img.size(), transcoder_tex_fmt, 0, level_info.m_orig_width, nullptr, level_info.m_orig_height))
				{
					error_printf("Failed transcoding image level (%u %u %u)!\n", image_index, level_index, transcoder_tex_fmt);
					return false;
				}

				double total_transcode_time = tm.get_elapsed_ms();

				total_format_transcoding_time_ms[(int)transcoder_tex_fmt] += total_transcode_time;

				image img(level_info.m_orig_width, level_info.m_orig_height);
				for (uint32_t y = 0; y < level_info.m_orig_height; y++)
				{
					for (uint32_t x = 0; x < level_info.m_orig_width; x++)
					{
						const uint16_t p = packed_img[x + y * level_info.m_orig_width];
						uint32_t r = p >> 12, g = (p >> 8) & 15, b = (p >> 4) & 15, a = p & 15;
						r = (r << 4) | r;
						g = (g << 4) | g;
						b = (b << 4) | b;
						a = (a << 4) | a;
						img(x, y).set(r, g, b, a);
					}
				}

				printf("Transcode of image %u level %u res %ux%u format %s succeeded in %3.3f ms\n", image_index, level_index, level_info.m_orig_width, level_info.m_orig_height, basist::basis_get_format_name(transcoder_tex_fmt), total_transcode_time);

				if ((!validate_flag) && (!opts.m_ktx_only))
				{
					std::string rgb_filename(base_filename + string_format("_unpacked_rgb_%s_%u_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), level_index, image_index));
					if (!save_png(rgb_filename, img, cImageSaveIgnoreAlpha))
					{
						error_printf("Failed writing to PNG file \"%s\"\n", rgb_filename.c_str());
						return false;
					}
					printf("Wrote PNG file \"%s\"\n", rgb_filename.c_str());

					std::string a_filename(base_filename + string_format("_unpacked_a_%s_%u_%04u.png", basist::basis_get_format_name(transcoder_tex_fmt), level_index, image_index));
					if (!save_png(a_filename, img, cImageSaveGrayscale, 3))
					{
						error_printf("Failed writing to PNG file \"%s\"\n", a_filename.c_str());
						return false;
					}
					printf("Wrote PNG file \"%s\"\n", a_filename.c_str());
				}

			} // level_index
		} // image_index

	} // if (!m_etc1_only)

	if (pCSV_file)
	{
		fprintf(pCSV_file, "%s, %3.3f, %3.3f, %u, %u, %u, %3.3f, ",
			base_filename.c_str(),
			basis_bits_per_texel,
			comp_bits_per_texel,
			fileinfo.m_total_images,
			max_mipmap_levels,
			(uint32_t)fileinfo.m_slice_info.size(),
			start_transcoding_time_ms);

		for (int format_iter = first_format; format_iter < last_format; format_iter++)
		{
			const basist::transcoder_texture_format transcoder_tex_fmt = static_cast<basist::transcoder_texture_format>(format_iter);

			if (!basis_is_format_supported(transcoder_tex_fmt, fileinfo.m_tex_format))
				continue;
			if (transcoder_tex_fmt == basist::transcoder_texture_format::cTFBC7_ALT)
				continue;

			fprintf(pCSV_file, "%3.3f", total_format_transcoding_time_ms[format_iter]);
			if (format_iter != (last_format - 1))
				fprintf(pCSV_file, ",");
		}
		fprintf(pCSV_file, "\n");
	}

	return true;
}

static bool unpack_and_validate_mode(command_line_params &opts)
{
	interval_timer tm;
	tm.start();

	//const bool validate_flag = (opts.m_mode == cValidate);
		
	basis_data* pGlobal_codebook_data = nullptr;
	if (opts.m_etc1s_use_global_codebooks_file.size())
	{
		pGlobal_codebook_data = load_basis_file(opts.m_etc1s_use_global_codebooks_file.c_str(), true);
		if (!pGlobal_codebook_data)
		{
			error_printf("Failed loading global codebook data from file \"%s\"\n", opts.m_etc1s_use_global_codebooks_file.c_str());
			return false;
		}
		printf("Loaded global codebooks from file \"%s\"\n", opts.m_etc1s_use_global_codebooks_file.c_str());
	}

	if (!opts.m_input_filenames.size())
	{
		error_printf("No input files to process!\n");
		delete pGlobal_codebook_data; pGlobal_codebook_data = nullptr;
		return false;
	}

	FILE* pCSV_file = nullptr;
	if ((opts.m_csv_file.size()) && (opts.m_mode == cValidate))
	{
		pCSV_file = fopen_safe(opts.m_csv_file.c_str(), "w");
		if (!pCSV_file)
		{
			error_printf("Failed opening CVS file \"%s\"\n", opts.m_csv_file.c_str());
			delete pGlobal_codebook_data; pGlobal_codebook_data = nullptr;
			return false;
		}
		//fprintf(pCSV_file, "Filename, Size, Slices, Width, Height, HasAlpha, BitsPerTexel, Slice0RGBAvgPSNR, Slice0RGBAAvgPSNR, Slice0Luma709PSNR, Slice0BestETC1SLuma709PSNR, Q, CL, Time, RGBAvgPSNRMin, RGBAvgPSNRAvg, AAvgPSNRMin, AAvgPSNRAvg, Luma709PSNRMin, Luma709PSNRAvg\n");
	}

	uint32_t total_unpack_warnings = 0;
	uint32_t total_pvrtc_nonpow2_warnings = 0;

	for (uint32_t file_index = 0; file_index < opts.m_input_filenames.size(); file_index++)
	{
		const char* pInput_filename = opts.m_input_filenames[file_index].c_str();

		std::string base_filename;
		string_split_path(pInput_filename, nullptr, nullptr, &base_filename, nullptr);

		uint8_vec file_data;
		if (!basisu::read_file_to_vec(pInput_filename, file_data))
		{
			error_printf("Failed reading file \"%s\"\n", pInput_filename);
			if (pCSV_file) fclose(pCSV_file);
			delete pGlobal_codebook_data; pGlobal_codebook_data = nullptr;
			return false;
		}

		if (!file_data.size())
		{
			error_printf("File is empty!\n");
			if (pCSV_file) fclose(pCSV_file);
			delete pGlobal_codebook_data; pGlobal_codebook_data = nullptr;
			return false;
		}

		if (file_data.size() > UINT32_MAX)
		{
			error_printf("File is too large!\n");
			if (pCSV_file) fclose(pCSV_file);
			delete pGlobal_codebook_data; pGlobal_codebook_data = nullptr;
			return false;
		}
		
		bool is_ktx2 = false;
		if (file_data.size() >= sizeof(basist::g_ktx2_file_identifier))
		{
			is_ktx2 = (memcmp(file_data.data(), basist::g_ktx2_file_identifier, sizeof(basist::g_ktx2_file_identifier)) == 0);
		}

		printf("Input file \"%s\", KTX2: %u\n", pInput_filename, is_ktx2);

		bool status;
		if (is_ktx2)
		{
			status = unpack_and_validate_ktx2_file(
				file_index,
				base_filename,
				file_data,
				opts,
				pCSV_file,
				pGlobal_codebook_data,
				total_unpack_warnings,
				total_pvrtc_nonpow2_warnings);
		}
		else
		{
			status = unpack_and_validate_basis_file(
				file_index,
				base_filename,
				file_data,
				opts,
				pCSV_file,
				pGlobal_codebook_data,
				total_unpack_warnings,
				total_pvrtc_nonpow2_warnings);
		}

		if (!status)
		{
			if (pCSV_file) 
				fclose(pCSV_file);

			delete pGlobal_codebook_data; 
			pGlobal_codebook_data = nullptr;

			return false;
		}

	} // file_index

	if (total_pvrtc_nonpow2_warnings)
		printf("Warning: %u images could not be transcoded to PVRTC1 because one or both dimensions were not a power of 2\n", total_pvrtc_nonpow2_warnings);

	if (total_unpack_warnings)
		printf("ATTENTION: %u total images had invalid GPU texture data!\n", total_unpack_warnings);
	else
		printf("Success\n");

	debug_printf("Elapsed time: %3.3f secs\n", tm.get_elapsed_secs());

	if (pCSV_file)
	{
		fclose(pCSV_file);
		pCSV_file = nullptr;
	}
	delete pGlobal_codebook_data; 
	pGlobal_codebook_data = nullptr;

	return true;
}

static bool compare_mode(command_line_params &opts)
{
	if (opts.m_input_filenames.size() != 2)
	{
		error_printf("Must specify two PNG filenames using -file\n");
		return false;
	}

	image a, b;
	if (!load_image(opts.m_input_filenames[0].c_str(), a))
	{
		error_printf("Failed loading image from file \"%s\"!\n", opts.m_input_filenames[0].c_str());
		return false;
	}

	printf("Loaded \"%s\", %ux%u, has alpha: %u\n", opts.m_input_filenames[0].c_str(), a.get_width(), a.get_height(), a.has_alpha());

	if (!load_image(opts.m_input_filenames[1].c_str(), b))
	{
		error_printf("Failed loading image from file \"%s\"!\n", opts.m_input_filenames[1].c_str());
		return false;
	}

	printf("Loaded \"%s\", %ux%u, has alpha: %u\n", opts.m_input_filenames[1].c_str(), b.get_width(), b.get_height(), b.has_alpha());

	if ((a.get_width() != b.get_width()) || (a.get_height() != b.get_height()))
	{
		printf("Images don't have the same dimensions - cropping input images to smallest common dimensions\n");

		uint32_t w = minimum(a.get_width(), b.get_width());
		uint32_t h = minimum(a.get_height(), b.get_height());

		a.crop(w, h);
		b.crop(w, h);
	}

	printf("Comparison image res: %ux%u\n", a.get_width(), a.get_height());

	image_metrics im;
	im.calc(a, b, 0, 3);
	im.print("RGB    ");

	im.calc(a, b, 0, 4);
	im.print("RGBA   ");

	im.calc(a, b, 0, 1);
	im.print("R      ");

	im.calc(a, b, 1, 1);
	im.print("G      ");

	im.calc(a, b, 2, 1);
	im.print("B      ");

	im.calc(a, b, 3, 1);
	im.print("A      ");

	im.calc(a, b, 0, 0);
	im.print("Y 709  " );

	im.calc(a, b, 0, 0, true, true);
	im.print("Y 601  " );
	
	if (opts.m_compare_ssim)
	{
		vec4F s_rgb(compute_ssim(a, b, false, false));

		printf("R SSIM: %f\n", s_rgb[0]);
		printf("G SSIM: %f\n", s_rgb[1]);
		printf("B SSIM: %f\n", s_rgb[2]);
		printf("RGB Avg SSIM: %f\n", (s_rgb[0] + s_rgb[1] + s_rgb[2]) / 3.0f);
		printf("A SSIM: %f\n", s_rgb[3]);

		vec4F s_y_709(compute_ssim(a, b, true, false));
		printf("Y 709 SSIM: %f\n", s_y_709[0]);

		vec4F s_y_601(compute_ssim(a, b, true, true));
		printf("Y 601 SSIM: %f\n", s_y_601[0]);
	}

	image delta_img(a.get_width(), a.get_height());

	const int X = 2;

	for (uint32_t y = 0; y < a.get_height(); y++)
	{
		for (uint32_t x = 0; x < a.get_width(); x++)
		{
			color_rgba &d = delta_img(x, y);

			for (int c = 0; c < 4; c++)
				d[c] = (uint8_t)clamp<int>((a(x, y)[c] - b(x, y)[c]) * X + 128, 0, 255);
		} // x
	} // y

	save_png("a_rgb.png", a, cImageSaveIgnoreAlpha);
	save_png("a_alpha.png", a, cImageSaveGrayscale, 3);
	printf("Wrote a_rgb.png and a_alpha.png\n");

	save_png("b_rgb.png", b, cImageSaveIgnoreAlpha);
	save_png("b_alpha.png", b, cImageSaveGrayscale, 3);
	printf("Wrote b_rgb.png and b_alpha.png\n");

	save_png("delta_img_rgb.png", delta_img, cImageSaveIgnoreAlpha);
	printf("Wrote delta_img_rgb.png\n");
	
	save_png("delta_img_a.png", delta_img, cImageSaveGrayscale, 3);
	printf("Wrote delta_img_a.png\n");

	uint32_t bins[5][512];
	clear_obj(bins);

	running_stat delta_stats[5];
	basisu::rand rm;

	double avg[5];
	clear_obj(avg);

	for (uint32_t y = 0; y < a.get_height(); y++)
	{
		for (uint32_t x = 0; x < a.get_width(); x++)
		{
			//color_rgba& d = delta_img(x, y);

			for (int c = 0; c < 4; c++)
			{
				int delta = a(x, y)[c] - b(x, y)[c];
				
				//delta = clamp<int>((int)std::round(rm.gaussian(70.0f, 10.0f)), -255, 255);
				
				bins[c][delta + 256]++;
				delta_stats[c].push(delta);

				avg[c] += delta;
			}
			
			int y_delta = a(x, y).get_709_luma() - b(x, y).get_709_luma();
			bins[4][y_delta + 256]++;
			delta_stats[4].push(y_delta);
			
			avg[4] += y_delta;

		} // x
	} // y

	for (uint32_t i = 0; i <= 4; i++)
		avg[i] /= a.get_total_pixels();

	printf("\n");

	//bins[2][256+-255] = 100000;
	//bins[2][256-56] = 50000;

	const uint32_t X_SIZE = 128, Y_SIZE = 40;

	for (uint32_t c = 0; c <= 4; c++)
	{
		std::vector<uint8_t> plot[Y_SIZE + 1];
		for (uint32_t i = 0; i < Y_SIZE; i++)
		{
			plot[i].resize(X_SIZE + 2);
			memset(plot[i].data(), ' ', X_SIZE + 1);
		}

		uint32_t max_val = 0;
		int max_val_bin_index = 0;
		int lowest_bin_index = INT_MAX, highest_bin_index = INT_MIN;
		double avg_val = 0;
		double total_val = 0;
		running_stat bin_stats;

		for (int y = -255; y <= 255; y++)
		{
			uint32_t val = bins[c][256 + y];
			if (!val)
				continue;

			bin_stats.push(y);
			
			total_val += (double)val;

			lowest_bin_index = minimum(lowest_bin_index, y);
			highest_bin_index = maximum(highest_bin_index, y);

			if (val > max_val)
			{
				max_val = val;
				max_val_bin_index = y;
			}
			avg_val += y * (double)val;
		}
		avg_val /= total_val;

		int lo_limit = -(int)X_SIZE / 2;
		int hi_limit = X_SIZE / 2;
		for (int x = lo_limit; x <= hi_limit; x++)
		{
			uint32_t total = 0;
			if (x == lo_limit)
			{
				for (int i = -255; i <= lo_limit; i++)
					total += bins[c][256 + i];
			}
			else if (x == hi_limit)
			{
				for (int i = hi_limit; i <= 255; i++)
					total += bins[c][256 + i];
			}
			else
			{
				total = bins[c][256 + x];
			}

			uint32_t height = max_val ? (total * Y_SIZE + max_val - 1) / max_val : 0;
			
			if (height)
			{
				for (uint32_t y = (Y_SIZE - 1) - (height - 1); y <= (Y_SIZE - 1); y++)
					plot[y][x + X_SIZE / 2] = '*';
			}
		}

		printf("%c delta histogram: total samples: %5.0f, max bin value: %u index: %i (%3.3f%% of total), range %i [%i,%i], weighted mean: %f\n", "RGBAY"[c], total_val, max_val, max_val_bin_index, max_val * 100.0f / total_val, highest_bin_index - lowest_bin_index + 1, lowest_bin_index, highest_bin_index, avg_val);
		printf("bin mean: %f, bin std deviation: %f, non-zero bins: %u\n", bin_stats.get_mean(), bin_stats.get_std_dev(), bin_stats.get_num());
		printf("delta mean: %f, delta std deviation: %f\n", delta_stats[c].get_mean(), delta_stats[c].get_std_dev());
		printf("\n");

		for (uint32_t y = 0; y < Y_SIZE; y++)
			printf("%s\n", (char*)plot[y].data());

		char tics[1024];
		tics[0] = '\0';
		char tics2[1024];
		tics2[0] = '\0';

		for (int x = 0; x <= (int)X_SIZE; x++)
		{
			char buf[64];
			if (x == X_SIZE / 2)
			{
				while ((int)strlen(tics) < x)
					strcat(tics, ".");
				
				while ((int)strlen(tics2) < x)
					strcat(tics2, " ");

				sprintf(buf, "0");
				strcat(tics, buf);
			}
			else if (((x & 7) == 0) || (x == X_SIZE))
			{
				while ((int)strlen(tics) < x)
					strcat(tics, ".");
				
				while ((int)strlen(tics2) < x)
					strcat(tics2, " ");

				int v = (x - (int)X_SIZE / 2);
				sprintf(buf, "%i", v / 10);
				strcat(tics, buf);
				
				if (v < 0)
				{
					if (-v < 10)
						sprintf(buf, "%i", v % 10);
					else
						sprintf(buf, " %i", -v % 10);
				}
				else
					sprintf(buf, "%i", v % 10);
				strcat(tics2, buf);
			}
			else
			{
				while ((int)strlen(tics) < x)
					strcat(tics, ".");
			}
		}
		printf("%s\n", tics);
		printf("%s\n", tics2);

		printf("\n");
	}
	
	return true;
}

static bool split_image_mode(command_line_params& opts)
{
	if (opts.m_input_filenames.size() != 1)
	{
		error_printf("Must specify one image filename using -file\n");
		return false;
	}

	image a;
	if (!load_image(opts.m_input_filenames[0].c_str(), a))
	{
		error_printf("Failed loading image from file \"%s\"!\n", opts.m_input_filenames[0].c_str());
		return false;
	}

	printf("Loaded \"%s\", %ux%u, has alpha: %u\n", opts.m_input_filenames[0].c_str(), a.get_width(), a.get_height(), a.has_alpha());

	if (!save_png("split_rgb.png", a, cImageSaveIgnoreAlpha))
	{
		fprintf(stderr, "Failed writing file split_rgb.png\n");
		return false;
	}
	printf("Wrote file split_rgb.png\n");

	for (uint32_t i = 0; i < 4; i++)
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "split_%c.png", "RGBA"[i]);
		if (!save_png(buf, a, cImageSaveGrayscale, i))
		{
			fprintf(stderr, "Failed writing file %s\n", buf);
			return false;
		}
		printf("Wrote file %s\n", buf);
	}
	
	return true;
}

static bool combine_images_mode(command_line_params& opts)
{
	if (opts.m_input_filenames.size() != 2)
	{
		error_printf("Must specify two image filename using -file\n");
		return false;
	}

	image a, b;
	if (!load_image(opts.m_input_filenames[0].c_str(), a))
	{
		error_printf("Failed loading image from file \"%s\"!\n", opts.m_input_filenames[0].c_str());
		return false;
	}

	printf("Loaded \"%s\", %ux%u, has alpha: %u\n", opts.m_input_filenames[0].c_str(), a.get_width(), a.get_height(), a.has_alpha());

	if (!load_image(opts.m_input_filenames[1].c_str(), b))
	{
		error_printf("Failed loading image from file \"%s\"!\n", opts.m_input_filenames[1].c_str());
		return false;
	}

	printf("Loaded \"%s\", %ux%u, has alpha: %u\n", opts.m_input_filenames[1].c_str(), b.get_width(), b.get_height(), b.has_alpha());

	const uint32_t width = minimum(a.get_width(), b.get_width());
	const uint32_t height = minimum(b.get_height(), b.get_height());

	image combined_img(width, height);
	for (uint32_t y = 0; y < height; y++)
	{
		for (uint32_t x = 0; x < width; x++)
		{
			combined_img(x, y) = a(x, y);
			combined_img(x, y).a = b(x, y).g;
		}
	}

	const char* pOutput_filename = "combined.png";
	if (opts.m_output_filename.size())
		pOutput_filename = opts.m_output_filename.c_str();
		
	if (!save_png(pOutput_filename, combined_img))
	{
		fprintf(stderr, "Failed writing file %s\n", pOutput_filename);
		return false;
	}
	printf("Wrote file %s\n", pOutput_filename);

	return true;
}

//#include "encoder/basisu_astc_decomp.h"
#include "encoder/basisu_pvrtc1_4.h"

static bool bench_mode(command_line_params& opts)
{
#if 0
	ispc::bc7e_compress_block_init();

	ispc::bc7e_compress_block_params pack_params;
	memset(&pack_params, 0, sizeof(pack_params));
	ispc::bc7e_compress_block_params_init_slow(&pack_params, false);
#endif

	const uint32_t JOB_POOL_SIZE = 7;
	job_pool jpool(JOB_POOL_SIZE);

	float total_uastc_psnr = 0, total_uastc_a_psnr = 0, total_uastc_rgba_psnr = 0;
	float total_rdo_uastc_psnr = 0, total_rdo_uastc_a_psnr = 0, total_rdo_uastc_rgba_psnr = 0;
	float total_uastc2_psnr = 0, total_uastc2_a_psnr = 0, total_uastc2_rgba_psnr = 0;
	float total_bc7_psnr = 0, total_bc7_a_psnr = 0, total_bc7_rgba_psnr = 0;
	float total_rdo_bc7_psnr = 0, total_rdo_bc7_a_psnr = 0, total_rdo_bc7_rgba_psnr = 0;
	float total_obc1_psnr = 0;
	float total_obc1_2_psnr = 0;
	float total_obc1_psnr_sq = 0;
	float total_obc1_2_psnr_sq = 0;
	float total_bc1_psnr = 0;
	float total_bc1_psnr_sq = 0;
	//float total_obc7_psnr = 0, total_obc7_rgba_psnr = 0;
	//float total_obc7_a_psnr = 0;
	//float total_oastc_psnr = 0, total_oastc_rgba_psnr = 0;
	float total_bc7enc_psnr = 0, total_bc7enc_rgba_psnr = 0, total_bc7enc_a_psnr = 0;
	//float total_oastc_a_psnr = 0;
	float total_etc1_psnr = 0;
	float total_etc1_y_psnr = 0;
	float total_etc1_g_psnr = 0;
	float total_etc2_psnr = 0, total_etc2_rgba_psnr = 0;
	float total_etc2_a_psnr = 0;
	float total_bc3_psnr = 0, total_bc3_rgba_psnr = 0;
	float total_bc3_a_psnr = 0;
	float total_eac_r11_psnr = 0;
	float total_eac_rg11_psnr = 0;
	float total_pvrtc1_rgb_psnr = 0, total_pvrtc1_rgba_psnr = 0;
	float total_pvrtc1_a_psnr = 0;
	uint32_t total_images = 0;
	uint32_t total_a_images = 0;
	uint32_t total_pvrtc1_images = 0;

	uint64_t overall_mode_hist[basist::TOTAL_UASTC_MODES];
	memset(overall_mode_hist, 0, sizeof(overall_mode_hist));

	std::mutex mode_hist_mutex;

	uint32_t etc1_hint_hist[32];
	memset(etc1_hint_hist, 0, sizeof(etc1_hint_hist));

	srand(1023);
	uint32_t first_image = 96;
	uint32_t last_image = 96; //34

	if (opts.m_input_filenames.size() >= 1)
	{
		first_image = 1;
		last_image = 1;
	}

	const bool perceptual = false;
	const bool force_la = false;

	interval_timer otm;
	otm.start();

	//const uint32_t flags = cPackUASTCLevelFastest;// | cPackUASTCETC1DisableFlipAndIndividual;// Slower;
	//const uint32_t flags = cPackUASTCLevelFaster;
	//const uint32_t flags = cPackUASTCLevelVerySlow;
	const uint32_t flags = cPackUASTCLevelDefault;

	uint32_t etc1_inten_hist[8] = { 0,0,0,0,0,0,0,0 };
	uint32_t etc1_flip_hist[2] = { 0, 0 };
	uint32_t etc1_diff_hist[2] = { 0, 0 };

	double overall_total_enc_time = 0;
	double overall_total_bench_time = 0;
	double overall_total_bench2_time = 0;
	uint64_t overall_blocks = 0;

	//bc7enc_compress_block_params bc7enc_p;
	//bc7enc_compress_block_params_init(&bc7enc_p);
	//bc7enc_compress_block_params_init_linear_weights(&bc7enc_p);
	//bc7enc_p.m_uber_level = 3;

	uint64_t total_comp_size = 0;
	uint64_t total_raw_size = 0;
	uint64_t total_rdo_comp_size = 0;
	uint64_t total_rdo_raw_size = 0;
	uint64_t total_comp_blocks = 0;

	for (uint32_t image_index = first_image; image_index <= last_image; image_index++)
	{
		uint64_t mode_hist[basist::TOTAL_UASTC_MODES];
		memset(mode_hist, 0, sizeof(mode_hist));

		char buf[1024];
		if (opts.m_input_filenames.size() >= 1)
			strcpy(buf, opts.m_input_filenames[0].c_str());
		else
			sprintf(buf, "c:/dev/test_images/photo_png/kodim%02u.png", image_index);

		printf("Image: %s\n", buf);

		image img;
		if (!load_image(buf, img))
			return 0;

		if (opts.m_input_filenames.size() == 2)
		{
			image alpha_img;
			if (!load_image(opts.m_input_filenames[1].c_str(), alpha_img))
				return 0;

			printf("Alpha image: %s, %ux%u\n", opts.m_input_filenames[1].c_str(), alpha_img.get_width(), alpha_img.get_height());

			for (uint32_t x = 0; x < alpha_img.get_width(); x++)
				for (uint32_t y = 0; y < alpha_img.get_height(); y++)
				{
					if (x < img.get_width() && y < img.get_height())
						img(x, y)[3] = (uint8_t)alpha_img(x, y).get_709_luma();
				}
		}

		if (force_la)
		{
			for (uint32_t x = 0; x < img.get_width(); x++)
			{
				for (uint32_t y = 0; y < img.get_height(); y++)
				{
					const color_rgba& c = img(x, y);
					img(x, y).set(c.r, c.r, c.r, c.g);
				}
			}
		}

		// HACK HACK
		//if (!img.has_alpha())
		//	continue;

		// HACK HACK
		//img.crop(1024, 1024);

		const uint32_t num_blocks_x = img.get_block_width(4);
		const uint32_t num_blocks_y = img.get_block_height(4);
		const uint32_t total_blocks = num_blocks_x * num_blocks_y;
		const bool img_has_alpha = img.has_alpha();

		img.crop_dup_borders(num_blocks_x * 4, num_blocks_y * 4);

		printf("%ux%u, has alpha: %u\n", img.get_width(), img.get_height(), img_has_alpha);

		image uastc_img(num_blocks_x * 4, num_blocks_y * 4);
		image rdo_uastc_img(num_blocks_x * 4, num_blocks_y * 4);
		image uastc2_img(num_blocks_x * 4, num_blocks_y * 4);
		image opt_bc1_img(num_blocks_x * 4, num_blocks_y * 4);
		image opt_bc1_2_img(num_blocks_x * 4, num_blocks_y * 4);
		image bc1_img(num_blocks_x * 4, num_blocks_y * 4);
		image bc3_img(num_blocks_x * 4, num_blocks_y * 4);
		image eac_r11_img(num_blocks_x * 4, num_blocks_y * 4);
		image eac_rg11_img(num_blocks_x * 4, num_blocks_y * 4);
		image bc7_img(num_blocks_x * 4, num_blocks_y * 4);
		image rdo_bc7_img(num_blocks_x * 4, num_blocks_y * 4);
		image opt_bc7_img(num_blocks_x * 4, num_blocks_y * 4);
		image etc1_img(num_blocks_x * 4, num_blocks_y * 4);
		image etc1_g_img(num_blocks_x * 4, num_blocks_y * 4);
		image etc2_img(num_blocks_x * 4, num_blocks_y * 4);
		image part_img(num_blocks_x * 4, num_blocks_y * 4);
		image opt_astc_img(num_blocks_x * 4, num_blocks_y * 4);
		image bc7enc_img(num_blocks_x * 4, num_blocks_y * 4);

		uint32_t total_bc1_hint0s = 0;
		uint32_t total_bc1_hint1s = 0;
		uint32_t total_bc1_hint01s = 0;

		double total_enc_time = 0;
		double total_bench_time = 0;
		double total_bench2_time = 0;

		basisu::vector<basist::uastc_block> ublocks(total_blocks);

#if 0
		astc_enc_settings astc_settings;
		//if (img_has_alpha)
		GetProfile_astc_alpha_slow(&astc_settings, 4, 4);
		//else
		//	GetProfile_astc_fast(&astc_settings, 4, 4);
#endif

#if 0
//#pragma omp parallel for
		for (int by = 0; by < (int)num_blocks_y; by++)
		{
			// Process 64 blocks at a time, for efficient SIMD processing.
			// Ideally, N >= 8 (or more) and (N % 8) == 0.
			const int N = 64;

			for (uint32_t bx = 0; bx < num_blocks_x; bx += N)
			{
				const uint32_t num_blocks_to_process = basisu::minimum<uint32_t>(num_blocks_x - bx, N);

				color_rgba pixels[16 * N];

#if 0
				// BC7E
				// Extract num_blocks_to_process 4x4 pixel blocks from the source image and put them into the pixels[] array.
				for (uint32_t b = 0; b < num_blocks_to_process; b++)
					img.extract_block_clamped(pixels + b * 16, (bx + b) * 4, by * 4, 4, 4);

				// Compress the blocks to BC7.
				// Note: If you've used Intel's ispc_texcomp, the input pixels are different. BC7E requires a pointer to an array of 16 pixels for each block.
				basist::bc7_block packed_blocks[N];
				ispc::bc7e_compress_blocks(num_blocks_to_process, (uint64_t*)packed_blocks, reinterpret_cast<const uint32_t*>(pixels), &pack_params);

				for (uint32_t i = 0; i < num_blocks_to_process; i++)
				{
					color_rgba decoded_block[4][4];

					//detexDecompressBlockBPTC((uint8_t *)&packed_blocks[i], 0xFF, 0, (uint8_t *)&decoded_block[0][0]);
					unpack_block(texture_format::cBC7, &packed_blocks[i], &decoded_block[0][0]);

					opt_bc7_img.set_block_clipped(&decoded_block[0][0], (bx + i) * 4, by * 4, 4, 4);
				}
#endif

#if 0
				// ispc_texcomp
				color_rgba raster_pixels[(N * 4) * 4];

				const uint32_t raster_width = num_blocks_to_process * 4;
				const uint32_t raster_height = 4;

				rgba_surface surf;
				surf.ptr = &raster_pixels[0].r;
				surf.width = raster_width;
				surf.height = 4;
				surf.stride = raster_width * 4;

				for (uint32_t b = 0; b < num_blocks_to_process; b++)
					for (uint32_t y = 0; y < 4; y++)
						for (uint32_t x = 0; x < 4; x++)
							raster_pixels[y * raster_width + b * 4 + x] = pixels[b * 16 + y * 4 + x];

				uint8_t astc_blocks[16 * N];
				CompressBlocksASTC(&surf, astc_blocks, &astc_settings);

				for (uint32_t i = 0; i < num_blocks_to_process; i++)
				{
					color_rgba decoded_astc_block[4][4];
					basisu_astc::astc::decompress((uint8_t*)decoded_astc_block, (uint8_t*)&astc_blocks[i * 16], false, 4, 4);

					opt_astc_img.set_block_clipped(&decoded_astc_block[0][0], (bx + i) * 4, by * 4, 4, 4);
				}
#endif
			}
		}
#endif

		const uint32_t N = 128;
		for (uint32_t block_index_iter = 0; block_index_iter < total_blocks; block_index_iter += N)
		{
			const uint32_t first_index = block_index_iter;
			const uint32_t last_index = minimum<uint32_t>(total_blocks, block_index_iter + N);

			jpool.add_job([first_index, last_index, &img, num_blocks_x, num_blocks_y,
				&opt_bc1_img, &opt_bc1_2_img, &mode_hist, &overall_mode_hist, &uastc_img, &uastc2_img, &bc7_img, &part_img, &mode_hist_mutex, &bc1_img, &etc1_img, &etc1_g_img, &etc2_img, &etc1_hint_hist, &perceptual,
				&total_bc1_hint0s, &total_bc1_hint1s, &total_bc1_hint01s, &bc3_img, &total_enc_time, &eac_r11_img, &eac_rg11_img, &ublocks, &flags, &etc1_inten_hist, &etc1_flip_hist, &etc1_diff_hist, &total_bench_time, &total_bench2_time,
				//&bc7enc_p, &bc7enc_img] {
				&bc7enc_img] {

					BASISU_NOTE_UNUSED(num_blocks_y);
					BASISU_NOTE_UNUSED(perceptual);
					BASISU_NOTE_UNUSED(flags);

					for (uint32_t block_index = first_index; block_index < last_index; block_index++)
					{
						const uint32_t block_x = block_index % num_blocks_x;
						const uint32_t block_y = block_index / num_blocks_x;

						//uint32_t block_x = 170;
						//uint32_t block_y = 167;

						// HACK HACK
						//if ((block_x == 77) && (block_y == 54))
						//	printf("!");

						color_rgba block[4][4];
						img.extract_block_clamped(&block[0][0], block_x * 4, block_y * 4, 4, 4);

						uint8_t bc7_block[16];
						//bc7enc_compress_block(bc7_block, block, &bc7enc_p);
						color_rgba decoded_bc7enc_blk[4][4];
						unpack_block(texture_format::cBC7, &bc7_block, &decoded_bc7enc_blk[0][0]);
						bc7enc_img.set_block_clipped(&decoded_bc7enc_blk[0][0], block_x * 4, block_y * 4, 4, 4);

						// Pack near-optimal BC1
						// stb_dxt BC1 encoder
						uint8_t bc1_block[8];

						interval_timer btm;
						btm.start();

						//stb_compress_dxt_block(bc1_block, (uint8_t*)&block[0][0], 0, STB_DXT_HIGHQUAL);
						basist::encode_bc1(bc1_block, (uint8_t*)&block[0][0], 0);// basist::cEncodeBC1HighQuality);
						double total_b_time = btm.get_elapsed_secs();
						{
							std::lock_guard<std::mutex> lck(mode_hist_mutex);
							total_bench_time += total_b_time;
						}

						color_rgba block_bc1[4][4];
						unpack_block(texture_format::cBC1, bc1_block, &block_bc1[0][0]);
						opt_bc1_img.set_block_clipped(&block_bc1[0][0], block_x * 4, block_y * 4, 4, 4);

						//uint64_t e1 = 0;
						//for (uint32_t i = 0; i < 16; i++)
						//	e1 += color_distance(((color_rgba*)block_bc1)[i], ((color_rgba*)block)[i], false);

						// My BC1 encoder
						uint8_t bc1_block_2[8];
						color_rgba block_bc1_2[4][4];

						btm.start();
						basist::encode_bc1_alt(bc1_block_2, (uint8_t*)&block[0][0], basist::cEncodeBC1HighQuality);
						double total_b2_time = btm.get_elapsed_secs();
						{
							std::lock_guard<std::mutex> lck(mode_hist_mutex);
							total_bench2_time += total_b2_time;
						}

						unpack_block(texture_format::cBC1, bc1_block_2, &block_bc1_2[0][0]);
						//uint64_t e2 = 0;
						//for (uint32_t i = 0; i < 16; i++)
						//	e2 += color_distance(((color_rgba *)block_bc1_2)[i], ((color_rgba*)block)[i], false);

						opt_bc1_2_img.set_block_clipped(&block_bc1_2[0][0], block_x * 4, block_y * 4, 4, 4);

						// Encode to UASTC
						basist::uastc_block encoded_uastc_blk;

						interval_timer tm;
						tm.start();
						encode_uastc(&block[0][0].r, encoded_uastc_blk, flags);
						double total_time = tm.get_elapsed_secs();
						{
							std::lock_guard<std::mutex> lck(mode_hist_mutex);
							total_enc_time += total_time;
						}

						ublocks[block_x + block_y * num_blocks_x] = encoded_uastc_blk;

#if 0
						for (uint32_t i = 0; i < 16; i++)
							printf("0x%X,", encoded_uastc_blk.m_bytes[i]);
						printf("\n");
#endif

						// Unpack UASTC
						basist::unpacked_uastc_block unpacked_uastc_blk;
						unpack_uastc(encoded_uastc_blk, unpacked_uastc_blk, false);

						color_rgba unpacked_uastc_block_pixels[4][4];
						bool success = basist::unpack_uastc(unpacked_uastc_blk, (basist::color32*) & unpacked_uastc_block_pixels[0][0], false);
						(void)success;
						assert(success);

						uastc_img.set_block_clipped(&unpacked_uastc_block_pixels[0][0], block_x * 4, block_y * 4, 4, 4);

						const uint32_t best_mode = unpacked_uastc_blk.m_mode;

						{
							std::lock_guard<std::mutex> lck(mode_hist_mutex);
							assert(best_mode < basist::TOTAL_UASTC_MODES);
							if (best_mode < basist::TOTAL_UASTC_MODES)
							{
								mode_hist[best_mode]++;
								overall_mode_hist[best_mode]++;
							}

							if (basist::g_uastc_mode_has_etc1_bias[best_mode])
								etc1_hint_hist[unpacked_uastc_blk.m_etc1_bias]++;

							total_bc1_hint0s += unpacked_uastc_blk.m_bc1_hint0;
							total_bc1_hint1s += unpacked_uastc_blk.m_bc1_hint1;
							total_bc1_hint01s += (unpacked_uastc_blk.m_bc1_hint0 || unpacked_uastc_blk.m_bc1_hint1);

							etc1_inten_hist[unpacked_uastc_blk.m_etc1_inten0]++;
							etc1_inten_hist[unpacked_uastc_blk.m_etc1_inten1]++;

							etc1_flip_hist[unpacked_uastc_blk.m_etc1_flip]++;
							etc1_diff_hist[unpacked_uastc_blk.m_etc1_diff]++;
						}

						// Transcode to BC1
						color_rgba tblock_bc1[4][4];

						uint8_t tbc1_block[8];
						transcode_uastc_to_bc1(encoded_uastc_blk, tbc1_block, false);
						unpack_block(texture_format::cBC1, tbc1_block, &tblock_bc1[0][0]);
						bc1_img.set_block_clipped(&tblock_bc1[0][0], block_x * 4, block_y * 4, 4, 4);

						// Transcode to BC7
						basist::bc7_optimization_results best_bc7_results;
						transcode_uastc_to_bc7(unpacked_uastc_blk, best_bc7_results);

						{
							basist::bc7_block bc7_data;
							encode_bc7_block(&bc7_data, &best_bc7_results);

							color_rgba decoded_bc7_blk[4][4];
							unpack_block(texture_format::cBC7, &bc7_data, &decoded_bc7_blk[0][0]);

							bc7_img.set_block_clipped(&decoded_bc7_blk[0][0], block_x * 4, block_y * 4, 4, 4);

							// Compute partition visualization image
							for (uint32_t y = 0; y < 4; y++)
							{
								for (uint32_t x = 0; x < 4; x++)
								{
									uint32_t part = 0;
									switch (best_bc7_results.m_mode)
									{
									case 1:
									case 3:
									case 7:
										part = basist::g_bc7_partition2[best_bc7_results.m_partition * 16 + x + y * 4];
										break;
									case 0:
									case 2:
										part = basist::g_bc7_partition3[best_bc7_results.m_partition * 16 + x + y * 4];
										break;
									}

									color_rgba c(0, 255, 0, 255);
									if (part == 1)
										c.set(255, 0, 0, 255);
									else if (part == 2)
										c.set(0, 0, 255, 255);

									part_img.set_clipped(block_x * 4 + x, block_y * 4 + y, c);
								}
							}
						}

						bool high_quality = false;

						// Transcode UASTC->BC3
						uint8_t ublock_bc3[16];
						transcode_uastc_to_bc3(encoded_uastc_blk, ublock_bc3, high_quality);
						color_rgba ublock_bc3_unpacked[4][4];
						unpack_block(texture_format::cBC3, &ublock_bc3, &ublock_bc3_unpacked[0][0]);
						bc3_img.set_block_clipped(&ublock_bc3_unpacked[0][0], block_x * 4, block_y * 4, 4, 4);

						// Transcode UASTC->R11
						uint8_t ublock_eac_r11[8];
						transcode_uastc_to_etc2_eac_r11(encoded_uastc_blk, ublock_eac_r11, high_quality, 0);
						color_rgba ublock_eac_r11_unpacked[4][4];
						for (uint32_t y = 0; y < 4; y++)
							for (uint32_t x = 0; x < 4; x++)
								ublock_eac_r11_unpacked[y][x].set(0, 0, 0, 255);
						unpack_block(texture_format::cETC2_R11_EAC, &ublock_eac_r11, &ublock_eac_r11_unpacked[0][0]);
						eac_r11_img.set_block_clipped(&ublock_eac_r11_unpacked[0][0], block_x * 4, block_y * 4, 4, 4);

						// Transcode UASTC->RG11
						uint8_t ublock_eac_rg11[16];
						transcode_uastc_to_etc2_eac_rg11(encoded_uastc_blk, ublock_eac_rg11, high_quality, 0, 1);
						color_rgba ublock_eac_rg11_unpacked[4][4];
						for (uint32_t y = 0; y < 4; y++)
							for (uint32_t x = 0; x < 4; x++)
								ublock_eac_rg11_unpacked[y][x].set(0, 0, 0, 255);
						unpack_block(texture_format::cETC2_RG11_EAC, &ublock_eac_rg11, &ublock_eac_rg11_unpacked[0][0]);
						eac_rg11_img.set_block_clipped(&ublock_eac_rg11_unpacked[0][0], block_x * 4, block_y * 4, 4, 4);

						// ETC1
						etc_block unpacked_etc1;
						transcode_uastc_to_etc1(encoded_uastc_blk, &unpacked_etc1);
						color_rgba unpacked_etc1_block[16];
						unpack_etc1(unpacked_etc1, unpacked_etc1_block);
						etc1_img.set_block_clipped(unpacked_etc1_block, block_x * 4, block_y * 4, 4, 4);

						// ETC1 Y
						etc_block unpacked_etc1_g;

						transcode_uastc_to_etc1(encoded_uastc_blk, &unpacked_etc1_g, 1);

						color_rgba unpacked_etc1_g_block[16];
						unpack_etc1(unpacked_etc1_g, unpacked_etc1_g_block);
						etc1_g_img.set_block_clipped(unpacked_etc1_g_block, block_x * 4, block_y * 4, 4, 4);

						// ETC2
						etc2_rgba_block unpacked_etc2;
						transcode_uastc_to_etc2_rgba(encoded_uastc_blk, &unpacked_etc2);

						color_rgba unpacked_etc2_block[16];
						unpack_block(texture_format::cETC2_RGBA, &unpacked_etc2, unpacked_etc2_block);
						etc2_img.set_block_clipped(unpacked_etc2_block, block_x * 4, block_y * 4, 4, 4);

						// UASTC->ASTC
						uint32_t tastc_data[4];

						transcode_uastc_to_astc(encoded_uastc_blk, tastc_data);

						color_rgba decoded_tastc_block[4][4];
						//basisu_astc::astc::decompress((uint8_t*)decoded_tastc_block, (uint8_t*)&tastc_data, false, 4, 4);

						uastc2_img.set_block_clipped(&decoded_tastc_block[0][0], block_x * 4, block_y * 4, 4, 4);

						for (uint32_t y = 0; y < 4; y++)
						{
							for (uint32_t x = 0; x < 4; x++)
							{
								if (decoded_tastc_block[y][x] != unpacked_uastc_block_pixels[y][x])
								{
									printf("UASTC!=ASTC!\n");
								}
							}
						}

					} // block_index

				});

		} // block_index_iter

		jpool.wait_for_all();

		{
			size_t comp_size = 0;
			void* pComp_data = tdefl_compress_mem_to_heap(&ublocks[0], ublocks.size() * 16, &comp_size, TDEFL_MAX_PROBES_MASK);// TDEFL_DEFAULT_MAX_PROBES);

			size_t decomp_size;
			void* pDecomp_data = tinfl_decompress_mem_to_heap(pComp_data, comp_size, &decomp_size, 0);

			if ((decomp_size != ublocks.size() * 16) || (memcmp(pDecomp_data, &ublocks[0], decomp_size) != 0))
			{
				printf("Compression or decompression failed!\n");
				exit(1);
			}

			mz_free(pComp_data);
			mz_free(pDecomp_data);

			printf("Pre-RDO UASTC size: %u, compressed size: %u, %3.2f bits/texel\n",
				(uint32_t)ublocks.size() * 16,
				(uint32_t)comp_size,
				comp_size * 8.0f / img.get_total_pixels());

			total_comp_size += comp_size;
			total_raw_size += ublocks.size() * 16;
		}

		basisu::vector<color_rgba> orig_block_pixels(ublocks.size() * 16);
		for (uint32_t block_y = 0; block_y < num_blocks_y; block_y++)
			for (uint32_t block_x = 0; block_x < num_blocks_x; block_x++)
				img.extract_block_clamped(&orig_block_pixels[(block_x + block_y * num_blocks_x) * 16], block_x * 4, block_y * 4, 4, 4);

		// HACK HACK
		const uint32_t max_rdo_jobs = 4;
		
		char rdo_fname[256];
		FILE* pFile = nullptr;
		for (uint32_t try_index = 0; try_index < 100; try_index++)
		{
			sprintf(rdo_fname, "rdo_%02u_%u.csv", image_index, try_index);
			pFile = fopen(rdo_fname, "rb");
			if (pFile)
			{
				fclose(pFile);
				continue;
			}
			
			pFile = fopen(rdo_fname, "w");
			if (!pFile)
				printf("Cannot open CSV file %s\n", rdo_fname);
			else
			{
				printf("Opened CSV file %s\n", rdo_fname);
				break;
			}
		}

		for (float q = .2f; q <= 10.0f; q += (q >= 1.0f ? .5f : .1f))
		{
			printf("Q: %f\n", q);

			uastc_rdo_params p;
			p.m_lambda = q;
			p.m_max_allowed_rms_increase_ratio = 10.0f;
			p.m_skip_block_rms_thresh = 8.0f;
			
			bool rdo_status = uastc_rdo((uint32_t)ublocks.size(), &ublocks[0], &orig_block_pixels[0], p, flags, &jpool, max_rdo_jobs);
			if (!rdo_status)
			{
				printf("uastc_rdo() failed!\n");
				return false;
			}
			for (uint32_t block_y = 0; block_y < num_blocks_y; block_y++)
			{
				for (uint32_t block_x = 0; block_x < num_blocks_x; block_x++)
				{
					const basist::uastc_block& blk = ublocks[block_x + block_y * num_blocks_x];

					color_rgba unpacked_block[4][4];
					if (!basist::unpack_uastc(blk, (basist::color32*)unpacked_block, false))
					{
						printf("Block unpack failed!\n");
						exit(1);
					}

					rdo_uastc_img.set_block_clipped(&unpacked_block[0][0], block_x * 4, block_y * 4, 4, 4);

					basist::bc7_optimization_results best_bc7_results;
					transcode_uastc_to_bc7(blk, best_bc7_results);

					basist::bc7_block bc7_data;
					encode_bc7_block(&bc7_data, &best_bc7_results);

					color_rgba decoded_bc7_blk[4][4];
					unpack_block(texture_format::cBC7, &bc7_data, &decoded_bc7_blk[0][0]);

					rdo_bc7_img.set_block_clipped(&decoded_bc7_blk[0][0], block_x * 4, block_y * 4, 4, 4);
				}
			}

			image_metrics em;
			em.calc(img, rdo_uastc_img, 0, 3);
			em.print("RDOUASTC RGB ");

			size_t comp_size = 0;
			void* pComp_data = tdefl_compress_mem_to_heap(&ublocks[0], ublocks.size() * 16, &comp_size, TDEFL_MAX_PROBES_MASK);// TDEFL_DEFAULT_MAX_PROBES);

			size_t decomp_size;
			void* pDecomp_data = tinfl_decompress_mem_to_heap(pComp_data, comp_size, &decomp_size, 0);

			if ((decomp_size != ublocks.size() * 16) || (memcmp(pDecomp_data, &ublocks[0], decomp_size) != 0))
			{
				printf("Compression or decompression failed!\n");
				exit(1);
			}

			mz_free(pComp_data);
			mz_free(pDecomp_data);

			printf("RDO UASTC size: %u, compressed size: %u, %3.2f bits/texel\n",
				(uint32_t)ublocks.size() * 16,
				(uint32_t)comp_size,
				comp_size * 8.0f / img.get_total_pixels());

			if (pFile)
				fprintf(pFile, "%f, %f, %f\n", q, comp_size * 8.0f / img.get_total_pixels(), em.m_psnr);
		}
		if (pFile)
			fclose(pFile);
		
		{
			size_t comp_size = 0;
			void* pComp_data = tdefl_compress_mem_to_heap(&ublocks[0], ublocks.size() * 16, &comp_size, TDEFL_MAX_PROBES_MASK);// TDEFL_DEFAULT_MAX_PROBES);

			size_t decomp_size;
			void* pDecomp_data = tinfl_decompress_mem_to_heap(pComp_data, comp_size, &decomp_size, 0);

			if ((decomp_size != ublocks.size() * 16) || (memcmp(pDecomp_data, &ublocks[0], decomp_size) != 0))
			{
				printf("Compression or decompression failed!\n");
				exit(1);
			}

			mz_free(pComp_data);
			mz_free(pDecomp_data);

			printf("RDO UASTC size: %u, compressed size: %u, %3.2f bits/texel\n",
				(uint32_t)ublocks.size() * 16,
				(uint32_t)comp_size,
				comp_size * 8.0f / img.get_total_pixels());

			total_rdo_comp_size += comp_size;
			total_rdo_raw_size += ublocks.size() * 16;
			total_comp_blocks += ublocks.size();
		}
										
		printf("Total blocks: %u\n", total_blocks);
		printf("Total BC1 hint 0's: %u %3.1f%%\n", total_bc1_hint0s, total_bc1_hint0s * 100.0f / total_blocks);
		printf("Total BC1 hint 1's: %u %3.1f%%\n", total_bc1_hint1s, total_bc1_hint1s * 100.0f / total_blocks);
		printf("Total BC1 hint 01's: %u %3.1f%%\n", total_bc1_hint01s, total_bc1_hint01s * 100.0f / total_blocks);
		printf("Total enc time per block: %f us\n", total_enc_time / total_blocks * 1000000.0f);
		printf("Total bench time per block: %f us\n", total_bench_time / total_blocks * 1000000.0f);
		printf("Total bench2 time per block: %f us\n", total_bench2_time / total_blocks * 1000000.0f);

		overall_total_enc_time += total_enc_time;
		overall_total_bench_time += total_bench_time;
		overall_total_bench2_time += total_bench2_time;
		overall_blocks += total_blocks;

		printf("ETC1 inten hist: %u %u %u %u %u %u %u %u\n", etc1_inten_hist[0], etc1_inten_hist[1], etc1_inten_hist[2], etc1_inten_hist[3],
			etc1_inten_hist[4], etc1_inten_hist[5], etc1_inten_hist[6], etc1_inten_hist[7]);
		printf("ETC1 flip hist: %u %u\n", etc1_flip_hist[0], etc1_flip_hist[1]);
		printf("ETC1 diff hist: %u %u\n", etc1_diff_hist[0], etc1_diff_hist[1]);

		printf("UASTC mode histogram:\n");
		uint64_t total_hist = 0;
		for (uint32_t i = 0; i < basist::TOTAL_UASTC_MODES; i++)
			total_hist += mode_hist[i];
		for (uint32_t i = 0; i < basist::TOTAL_UASTC_MODES; i++)
			printf("%u: %u %3.2f%%\n", i, (uint32_t)mode_hist[i], mode_hist[i] * 100.0f / total_hist);

		char fn[256];

#if 0
		for (uint32_t y = 0; y < img.get_height(); y++)
			for (uint32_t x = 0; x < img.get_width(); x++)
			{
				//static inline uint8_t to_5(uint32_t v) { ; }
				color_rgba &c = img(x, y);

				for (uint32_t i = 0; i < 3; i++)
				{
					const uint32_t limit = (i == 1) ? 63 : 31;

					uint32_t v = c[i];
					v = v * limit + 128; v = (uint8_t)((v + (v >> 8)) >> 8);
					v = (v * 255 + (limit / 2)) / limit;

					c[i] = (uint8_t)v;
				}
			
			}
#endif

		sprintf(fn, "orig_%02u.png", image_index);
		save_png(fn, img, cImageSaveIgnoreAlpha);

		sprintf(fn, "orig_a_%02u.png", image_index);
		save_png(fn, img, cImageSaveGrayscale, 3);

		sprintf(fn, "unpacked_uastc_%02u.png", image_index);
		save_png(fn, uastc_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_uastc_a_%02u.png", image_index);
		save_png(fn, uastc_img, cImageSaveGrayscale, 3);

		sprintf(fn, "unpacked_rdo_uastc_%02u.png", image_index);
		save_png(fn, rdo_uastc_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_rdo_uastc_a_%02u.png", image_index);
		save_png(fn, rdo_uastc_img, cImageSaveGrayscale, 3);

		sprintf(fn, "unpacked_uastc2_%02u.png", image_index);
		save_png(fn, uastc2_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_bc7_%02u.png", image_index);
		save_png(fn, bc7_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_bc7_a_%02u.png", image_index);
		save_png(fn, bc7_img, cImageSaveGrayscale, 3);

		sprintf(fn, "unpacked_rdo_bc7_%02u.png", image_index);
		save_png(fn, rdo_bc7_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_rdo_bc7_a_%02u.png", image_index);
		save_png(fn, rdo_bc7_img, cImageSaveGrayscale, 3);

		sprintf(fn, "unpacked_opt_bc7_%02u.png", image_index);
		save_png(fn, opt_bc7_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_opt_bc7_a_%02u.png", image_index);
		save_png(fn, opt_bc7_img, cImageSaveGrayscale, 3);

		sprintf(fn, "unpacked_opt_astc_%02u.png", image_index);
		save_png(fn, opt_astc_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_opt_astc_a_%02u.png", image_index);
		save_png(fn, opt_astc_img, cImageSaveGrayscale, 3);

		sprintf(fn, "unpacked_bc7enc_%02u.png", image_index);
		save_png(fn, bc7enc_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_bc7enc_a_%02u.png", image_index);
		save_png(fn, bc7enc_img, cImageSaveGrayscale, 3);

		sprintf(fn, "unpacked_opt_bc1_%02u.png", image_index);
		save_png(fn, opt_bc1_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_opt_bc1_2_%02u.png", image_index);
		save_png(fn, opt_bc1_2_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_tbc1_%02u.png", image_index);
		save_png(fn, bc1_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_bc3_%02u.png", image_index);
		save_png(fn, bc3_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_eac_r11_%02u.png", image_index);
		save_png(fn, eac_r11_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_eac_rg11_%02u.png", image_index);
		save_png(fn, eac_rg11_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_bc3_a_%02u.png", image_index);
		save_png(fn, bc3_img, cImageSaveGrayscale, 3);

		sprintf(fn, "part_vis_%02u.png", image_index);
		save_png(fn, part_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_etc1_%02u.png", image_index);
		save_png(fn, etc1_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_etc1_g_%02u.png", image_index);
		save_png(fn, etc1_g_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_etc2_%02u.png", image_index);
		save_png(fn, etc2_img, cImageSaveIgnoreAlpha);

		sprintf(fn, "unpacked_etc2_a_%02u.png", image_index);
		save_png(fn, etc2_img, cImageSaveGrayscale, 3);

		image_metrics em;

		// UASTC
		em.calc(img, uastc_img, 0, 3);
		em.print("UASTC RGB  ");
		total_uastc_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, uastc_img, 3, 1);
		em.print("UASTC A    ");
		if (img_has_alpha)
			total_uastc_a_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, uastc_img, 0, 4);
		em.print("UASTC RGBA ");
		total_uastc_rgba_psnr += basisu::minimum(99.0f, em.m_psnr);

		// RDO UASTC
		em.calc(img, rdo_uastc_img, 0, 3);
		em.print("RDOUASTC RGB ");
		total_rdo_uastc_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, rdo_uastc_img, 3, 1);
		em.print("RDOUASTC A ");
		if (img_has_alpha)
			total_rdo_uastc_a_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, rdo_uastc_img, 0, 4);
		em.print("RDOUASTC RGBA ");
		total_rdo_uastc_rgba_psnr += basisu::minimum(99.0f, em.m_psnr);

		// UASTC2 
		em.calc(img, uastc2_img, 0, 3);
		em.print("UASTC2 RGB ");
		total_uastc2_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, uastc2_img, 3, 1);
		em.print("UASTC2 A   ");
		if (img_has_alpha)
			total_uastc2_a_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, uastc2_img, 0, 4);
		em.print("UASTC2 RGBA ");
		total_uastc2_rgba_psnr += basisu::minimum(99.0f, em.m_psnr);

		// BC7
		em.calc(img, bc7_img, 0, 3);
		em.print("BC7 RGB    ");
		total_bc7_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, bc7_img, 3, 1);
		em.print("BC7 A      ");
		if (img_has_alpha)
			total_bc7_a_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, bc7_img, 0, 4);
		em.print("BC7 RGBA   ");
		total_bc7_rgba_psnr += basisu::minimum(99.0f, em.m_psnr);

		// RDO BC7
		em.calc(img, rdo_bc7_img, 0, 3);
		em.print("RDOBC7 RGB ");
		total_rdo_bc7_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, rdo_bc7_img, 3, 1);
		em.print("RDOBC7 A   ");
		if (img_has_alpha)
			total_rdo_bc7_a_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, rdo_bc7_img, 0, 4);
		em.print("RDOBC7 RGBA ");
		total_rdo_bc7_rgba_psnr += basisu::minimum(99.0f, em.m_psnr);

#if 0
		// OBC7
		em.calc(img, opt_bc7_img, 0, 3);
		em.print("OBC7 RGB   ");
		total_obc7_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, opt_bc7_img, 3, 1);
		em.print("OBC7 A     ");
		if (img_has_alpha)
			total_obc7_a_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, opt_bc7_img, 0, 4);
		em.print("OBC7 RGBA  ");
		total_obc7_rgba_psnr += basisu::minimum(99.0f, em.m_psnr);

		// OASTC
		em.calc(img, opt_astc_img, 0, 3);
		em.print("OASTC RGB   ");
		total_oastc_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, opt_astc_img, 3, 1);
		em.print("OASTC A     ");
		if (img_has_alpha)
			total_oastc_a_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, opt_astc_img, 0, 4);
		em.print("OASTC RGBA  ");
		total_oastc_rgba_psnr += basisu::minimum(99.0f, em.m_psnr);
#endif

		// bc7enc
		em.calc(img, bc7enc_img, 0, 3);
		em.print("BC7ENC RGB  ");
		total_bc7enc_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, bc7enc_img, 3, 1);
		em.print("BC7ENC A    ");
		if (img_has_alpha)
			total_bc7enc_a_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, bc7enc_img, 0, 4);
		em.print("BC7ENC RGBA ");
		total_bc7enc_rgba_psnr += basisu::minimum(99.0f, em.m_psnr);

#if 1
		// OBC1
		em.calc(img, opt_bc1_img, 0, 3);
		em.print("OBC1 RGB   ");
		total_obc1_psnr += basisu::minimum(99.0f, em.m_psnr);
		total_obc1_psnr_sq += basisu::minimum(99.0f, em.m_psnr) * basisu::minimum(99.0f, em.m_psnr);
#endif
				
		em.calc(img, opt_bc1_2_img, 0, 3);
		em.print("OBC1 2 RGB ");
		total_obc1_2_psnr += basisu::minimum(99.0f, em.m_psnr);
		total_obc1_2_psnr_sq += basisu::minimum(99.0f, em.m_psnr) * basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, bc1_img, 0, 3);
		em.print("BC1 RGB    ");
		total_bc1_psnr += basisu::minimum(99.0f, em.m_psnr);
		total_bc1_psnr_sq += basisu::minimum(99.0f, em.m_psnr) * basisu::minimum(99.0f, em.m_psnr);

		// ETC1
		em.calc(img, etc1_img, 0, 3);
		em.print("ETC1 RGB   ");
		total_etc1_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, etc1_img, 0, 0);
		em.print("ETC1 Y     ");
		total_etc1_y_psnr += basisu::minimum(99.0f, em.m_psnr);

		// ETC1
		em.calc(img, etc1_g_img, 1, 1);
		em.print("ETC1 G     ");
		total_etc1_g_psnr += basisu::minimum(99.0f, em.m_psnr);

		// ETC2
		em.calc(img, etc2_img, 0, 3);
		em.print("ETC2 RGB   ");
		total_etc2_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, etc2_img, 3, 1);
		em.print("ETC2 A     ");
		if (img_has_alpha)
			total_etc2_a_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, etc2_img, 0, 4);
		em.print("ETC2 RGBA  ");
		total_etc2_rgba_psnr += basisu::minimum(99.0f, em.m_psnr);

		// BC3
		em.calc(img, bc3_img, 0, 3);
		em.print("BC3 RGB    ");
		total_bc3_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, bc3_img, 3, 1);
		em.print("BC3 A      ");
		if (img_has_alpha)
			total_bc3_a_psnr += basisu::minimum(99.0f, em.m_psnr);

		em.calc(img, bc3_img, 0, 4);
		em.print("BC3 RGBA   ");
		total_bc3_rgba_psnr += basisu::minimum(99.0f, em.m_psnr);

		// EAC R11
		em.calc(img, eac_r11_img, 0, 1);
		em.print("EAC R11    ");
		total_eac_r11_psnr += basisu::minimum(99.0f, em.m_psnr);

		// EAC RG11
		em.calc(img, eac_rg11_img, 0, 2);
		em.print("EAC RG11   ");
		total_eac_rg11_psnr += basisu::minimum(99.0f, em.m_psnr);

		const uint32_t width = num_blocks_x * 4;
		const uint32_t height = num_blocks_y * 4;
		if (is_pow2(width) && is_pow2(height))
		{
			pvrtc4_image pi(width, height);

			transcode_uastc_to_pvrtc1_4_rgba(&ublocks[0], pi.get_blocks().get_ptr(), num_blocks_x, num_blocks_y, false);

			pi.deswizzle();

			//pi.map_all_pixels(img, perceptual, false);

			image pi_unpacked;
			pi.unpack_all_pixels(pi_unpacked);

#if 0
			sprintf(fn, "unpacked_pvrtc1_rgb_before_%02u.png", image_index);
			save_png(fn, pi_unpacked, cImageSaveIgnoreAlpha);

			em.calc(img, pi_unpacked, 0, 3);
			em.print("PVRTC1 RGB Before ");

			for (uint32_t pass = 0; pass < 1; pass++)
			{
				for (uint32_t by = 0; by < num_blocks_y; by++)
				{
					for (uint32_t bx = 0; bx < num_blocks_x; bx++)
					{
						pi.local_endpoint_optimization_opaque(bx, by, img, perceptual, false);
					}
				}
			}

			//pi.map_all_pixels(img, perceptual, false);

			pi.unpack_all_pixels(pi_unpacked);
#endif

			sprintf(fn, "unpacked_pvrtc1_%02u.png", image_index);
			save_png(fn, pi_unpacked, cImageSaveIgnoreAlpha);

			sprintf(fn, "unpacked_pvrtc1_a_%02u.png", image_index);
			save_png(fn, pi_unpacked, cImageSaveGrayscale, 3);

			em.calc(img, pi_unpacked, 0, 3);
			em.print("PVRTC1 After RGB  ");
			total_pvrtc1_rgb_psnr += basisu::minimum(99.0f, em.m_psnr);

			em.calc(img, pi_unpacked, 3, 1);
			em.print("PVRTC1 After A    ");
			total_pvrtc1_a_psnr += basisu::minimum(99.0f, em.m_psnr);

			em.calc(img, pi_unpacked, 0, 4);
			em.print("PVRTC1 After RGBA ");
			total_pvrtc1_rgba_psnr += basisu::minimum(99.0f, em.m_psnr);

			total_pvrtc1_images++;
		}

		printf("ETC1 hint histogram:\n");
		for (uint32_t i = 0; i < 32; i++)
			printf("%u ", etc1_hint_hist[i]);
		printf("\n");

		total_images++;
		if (img_has_alpha)
			total_a_images++;

	} // image_index

	printf("Total time: %f secs\n", otm.get_elapsed_secs());
	
	printf("Total Non-RDO UASTC size: %llu, compressed size: %llu, %3.2f bits/texel\n",
		(unsigned long long)total_raw_size,
		(unsigned long long)total_comp_size,
		total_comp_size * 8.0f / (total_comp_blocks * 16));

	printf("Total RDO UASTC size: %llu, compressed size: %llu, %3.2f bits/texel\n",
		(unsigned long long)total_rdo_raw_size,
		(unsigned long long)total_rdo_comp_size,
		total_rdo_comp_size * 8.0f / (total_comp_blocks * 16));

	printf("Overall enc time per block: %f us\n", overall_total_enc_time / overall_blocks * 1000000.0f);
	printf("Overall bench time per block: %f us\n", overall_total_bench_time / overall_blocks * 1000000.0f);
	printf("Overall bench2 time per block: %f us\n", overall_total_bench2_time / overall_blocks * 1000000.0f);

	printf("Overall ASTC mode histogram:\n");
	uint64_t total_hist = 0;
	for (uint32_t i = 0; i < basist::TOTAL_UASTC_MODES; i++)
		total_hist += overall_mode_hist[i];

	for (uint32_t i = 0; i < basist::TOTAL_UASTC_MODES; i++)
		printf("%u: %u %3.2f%%\n", i, (uint32_t)overall_mode_hist[i], overall_mode_hist[i] * 100.0f / total_hist);

	printf("Total images: %u, total images with alpha: %u, total PVRTC1 images: %u\n", total_images, total_a_images, total_pvrtc1_images);

	if (!total_a_images)
		total_a_images = 1;

	printf("Avg UASTC RGB PSNR:    %f, A PSNR: %f, RGBA PSNR: %f\n", total_uastc_psnr / total_images, total_uastc_a_psnr / total_a_images, total_uastc_rgba_psnr / total_images);
	printf("Avg UASTC2 RGB PSNR:   %f, A PSNR: %f, RGBA PSNR: %f\n", total_uastc2_psnr / total_images, total_uastc2_a_psnr / total_a_images, total_uastc2_rgba_psnr / total_images);
	printf("Avg RDO UASTC RGB PSNR: %f, A PSNR: %f, RGBA PSNR: %f\n", total_rdo_uastc_psnr / total_images, total_rdo_uastc_a_psnr / total_a_images, total_rdo_uastc_rgba_psnr / total_images);
	printf("Avg BC7 RGB PSNR:      %f, A PSNR: %f, RGBA PSNR: %f\n", total_bc7_psnr / total_images, total_bc7_a_psnr / total_a_images, total_bc7_rgba_psnr / total_images);
	printf("Avg RDO BC7 RGB PSNR:  %f, A PSNR: %f, RGBA PSNR: %f\n", total_rdo_bc7_psnr / total_images, total_rdo_bc7_a_psnr / total_a_images, total_rdo_bc7_rgba_psnr / total_images);
	//printf("Avg Opt BC7 RGB PSNR:  %f, A PSNR: %f, RGBA PSNR: %f\n", total_obc7_psnr / total_images, total_obc7_a_psnr / total_a_images, total_obc7_rgba_psnr / total_images);
	//printf("Avg Opt ASTC RGB PSNR: %f, A PSNR: %f, RGBA PSNR: %f\n", total_oastc_psnr / total_images, total_oastc_a_psnr / total_a_images, total_oastc_rgba_psnr / total_images);
	printf("Avg BC7ENC RGB PSNR:   %f, A PSNR: %f, RGBA PSNR: %f\n", total_bc7enc_psnr / total_images, total_bc7enc_a_psnr / total_a_images, total_bc7enc_rgba_psnr / total_images);

	printf("Avg Opt BC1 PSNR: %f, std dev: %f\n", total_obc1_psnr / total_images, sqrtf(basisu::maximum(0.0f, (total_obc1_psnr_sq / total_images) - (total_obc1_psnr / total_images) * (total_obc1_psnr / total_images))));

	printf("Avg Opt BC1 2 PSNR: %f, std dev: %f\n", total_obc1_2_psnr / total_images, sqrtf(basisu::maximum(0.0f, (total_obc1_2_psnr_sq / total_images) - (total_obc1_2_psnr / total_images) * (total_obc1_2_psnr / total_images))));

	printf("Avg BC1 PSNR: %f, std dev: %f\n", total_bc1_psnr / total_images, sqrtf(basisu::maximum(0.0f, (total_bc1_psnr_sq / total_images) - (total_bc1_psnr / total_images) * (total_bc1_psnr / total_images))));

	printf("Avg ETC1 RGB PSNR: %f\n", total_etc1_psnr / total_images);
	printf("Avg ETC1 Y PSNR: %f\n", total_etc1_y_psnr / total_images);
	printf("Avg ETC1 G PSNR: %f\n", total_etc1_g_psnr / total_images);

	printf("Avg ETC2 RGB PSNR: %f\n", total_etc2_psnr / total_images);
	printf("Avg ETC2 A PSNR: %f\n", total_etc2_a_psnr / total_a_images);
	printf("Avg ETC2 RGBA PSNR: %f\n", total_etc2_rgba_psnr / total_images);

	printf("Avg BC3 RGB PSNR: %f\n", total_bc3_psnr / total_images);
	printf("Avg BC3 A PSNR: %f\n", total_bc3_a_psnr / total_a_images);
	printf("Avg BC3 RGBA PSNR: %f\n", total_bc3_rgba_psnr / total_images);

	printf("Avg EAC R11 PSNR: %f\n", total_eac_r11_psnr / total_images);
	printf("Avg EAC RG11 PSNR: %f\n", total_eac_rg11_psnr / total_images);

	if (total_pvrtc1_images)
	{
		printf("Avg PVRTC1 RGB PSNR: %f\n", total_pvrtc1_rgb_psnr / total_pvrtc1_images);
		printf("Avg PVRTC1 A PSNR: %f\n", total_pvrtc1_a_psnr / total_pvrtc1_images);
		printf("Avg PVRTC1 RGBA PSNR: %f\n", total_pvrtc1_rgba_psnr / total_pvrtc1_images);
	}

	return true;
}

static uint32_t compute_miniz_compressed_size(const char* pFilename, uint32_t &orig_size)
{
	orig_size = 0;

	uint8_vec buf;
	if (!read_file_to_vec(pFilename, buf))
		return 0;

	if (!buf.size())
		return 0;

	orig_size = buf.size();

	size_t comp_size = 0;
	void* pComp_data = tdefl_compress_mem_to_heap(&buf[0], buf.size(), &comp_size, TDEFL_MAX_PROBES_MASK);// TDEFL_DEFAULT_MAX_PROBES);

	mz_free(pComp_data);

	return (uint32_t)comp_size;
}

static bool compsize_mode(command_line_params& opts)
{
	if (opts.m_input_filenames.size() != 1)
	{
		error_printf("Must specify a filename using -file\n");
		return false;
	}

	uint32_t orig_size;
	uint32_t comp_size = compute_miniz_compressed_size(opts.m_input_filenames[0].c_str(), orig_size);
	printf("Original file size: %u bytes\n", orig_size);
	printf("miniz compressed size: %u bytes\n", comp_size);

	return true;
}

const struct test_file
{
	const char* m_pFilename;
	uint32_t m_etc1s_size;
	float m_etc1s_psnr;
	float m_uastc_psnr;
} g_test_files[] = 
{
	{ "black_1x1.png", 189, 100.0f, 100.0f },
	{ "kodim01.png", 30993, 27.40f, 44.14f },
	{ "kodim02.png", 28529, 32.20f, 41.06f },
	{ "kodim03.png", 23411, 32.57f, 44.87f },
	{ "kodim04.png", 28256, 31.76f, 43.02f },
	{ "kodim05.png", 32646, 25.94f, 40.28f },
	{ "kodim06.png", 27336, 28.66f, 44.57f },
	{ "kodim07.png", 26618, 31.51f, 43.94f },
	{ "kodim08.png", 31133, 25.28f, 41.15f },
	{ "kodim09.png", 24777, 32.05f, 45.85f },
	{ "kodim10.png", 27247, 32.20f, 45.77f },
	{ "kodim11.png", 26579, 29.22f, 43.68f },
	{ "kodim12.png", 25102, 32.96f, 46.77f },
	{ "kodim13.png", 31604, 24.25f, 41.25f },
	{ "kodim14.png", 31162, 27.81f, 39.65f },
	{ "kodim15.png", 25528, 31.26f, 42.87f },
	{ "kodim16.png", 26894, 32.21f, 47.78f },
	{ "kodim17.png", 29334, 31.40f, 45.66f },
	{ "kodim18.png", 30929, 27.46f, 41.54f },
	{ "kodim19.png", 27889, 29.69f, 44.95f },
	{ "kodim20.png", 21104, 31.30f, 45.31f },
	{ "kodim21.png", 25943, 28.53f, 44.45f },
	{ "kodim22.png", 29277, 29.85f, 42.63f },
	{ "kodim23.png", 23550, 31.69f, 45.11f },
	{ "kodim24.png", 29613, 26.75f, 40.61f },
	{ "white_1x1.png", 189, 100.0f, 100.0f },
	{ "wikipedia.png", 38961, 24.10f, 30.47f },
	{ "alpha0.png", 766, 100.0f, 56.16f }
};
const uint32_t TOTAL_TEST_FILES = sizeof(g_test_files) / sizeof(g_test_files[0]);

static bool test_mode(command_line_params& opts)
{
	uint32_t total_mismatches = 0;

	// TODO: Record min/max/avgs
	// TODO: Add another ETC1S quality level

	for (uint32_t i = 0; i < TOTAL_TEST_FILES; i++)
	{
		std::string filename(opts.m_test_file_dir);
		if (filename.size())
		{
			filename.push_back('/');
		}
		filename += std::string(g_test_files[i].m_pFilename);

		basisu::vector<image> source_images(1);

		image& source_image = source_images[0];
		if (!load_png(filename.c_str(), source_image))
		{
			error_printf("Failed loading test image \"%s\"\n", filename.c_str());
			return false;
		}

		printf("Loaded file \"%s\", dimemsions %ux%u has alpha: %u\n", filename.c_str(), source_image.get_width(), source_image.get_height(), source_image.has_alpha());

		image_stats stats;

		uint32_t flags_and_quality;
		float uastc_rdo_quality = 0.0f;
		size_t data_size = 0;

		// Test ETC1S
		flags_and_quality = (opts.m_comp_params.m_multithreading ? cFlagThreaded : 0);

		void* pData = basis_compress(source_images, flags_and_quality, uastc_rdo_quality, &data_size, &stats);
		if (!pData)
		{
			error_printf("basis_compress() failed!\n");
			return false;
		}
		basis_free_data(pData);

		printf("ETC1S Size: %u, PSNR: %f\n", (uint32_t)data_size, stats.m_basis_rgba_avg_psnr);

		float file_size_ratio = fabs((data_size / (float)g_test_files[i].m_etc1s_size) - 1.0f);
		if (file_size_ratio > .015f)
		{
			error_printf("Expected ETC1S file size was %u, but got %u instead!\n", g_test_files[i].m_etc1s_size, (uint32_t)data_size);
			total_mismatches++;
		}

		if (fabs(stats.m_basis_rgba_avg_psnr - g_test_files[i].m_etc1s_psnr) > .02f)
		{
			error_printf("Expected ETC1S RGBA Avg PSNR was %f, but got %f instead!\n", g_test_files[i].m_etc1s_psnr, stats.m_basis_rgba_avg_psnr);
			total_mismatches++;
		}

		if (opencl_is_available())
		{
			flags_and_quality = (opts.m_comp_params.m_multithreading ? cFlagThreaded : 0) | cFlagUseOpenCL;

			pData = basis_compress(source_images, flags_and_quality, uastc_rdo_quality, &data_size, &stats);
			if (!pData)
			{
				error_printf("basis_compress() failed!\n");
				return false;
			}
			basis_free_data(pData);

			printf("ETC1S+OpenCL Size: %u, PSNR: %f\n", (uint32_t)data_size, stats.m_basis_rgba_avg_psnr);

			file_size_ratio = fabs((data_size / (float)g_test_files[i].m_etc1s_size) - 1.0f);
			if (file_size_ratio > .04f)
			{
				error_printf("Expected ETC1S+OpenCL file size was %u, but got %u instead!\n", g_test_files[i].m_etc1s_size, (uint32_t)data_size);
				total_mismatches++;
			}

			if (g_test_files[i].m_etc1s_psnr == 100.0f)
			{
				// TODO
				if (stats.m_basis_rgba_avg_psnr < 69.0f)
				{
					error_printf("Expected ETC1S+OpenCL RGBA Avg PSNR was %f, but got %f instead!\n", g_test_files[i].m_etc1s_psnr, stats.m_basis_rgba_avg_psnr);
					total_mismatches++;
				}
			}
			else if (fabs(stats.m_basis_rgba_avg_psnr - g_test_files[i].m_etc1s_psnr) > .2f)
			{
				error_printf("Expected ETC1S+OpenCL RGBA Avg PSNR was %f, but got %f instead!\n", g_test_files[i].m_etc1s_psnr, stats.m_basis_rgba_avg_psnr);
				total_mismatches++;
			}
		}

		// Test UASTC
		flags_and_quality = (opts.m_comp_params.m_multithreading ? cFlagThreaded : 0) | cFlagUASTC;

		pData = basis_compress(source_images, flags_and_quality, uastc_rdo_quality, &data_size, &stats);
		if (!pData)
		{
			error_printf("basis_compress() failed!\n");
			return false;
		}
		basis_free_data(pData);

		printf("UASTC Size: %u, PSNR: %f\n", (uint32_t)data_size, stats.m_basis_rgba_avg_psnr);

		if (fabs(stats.m_basis_rgba_avg_psnr - g_test_files[i].m_uastc_psnr) > .005f)
		{
			error_printf("Expected UASTC RGBA Avg PSNR was %f, but got %f instead!\n", g_test_files[i].m_etc1s_psnr, stats.m_basis_rgba_avg_psnr);
			total_mismatches++;
		}
	}

	printf("Total mismatches: %u\n", total_mismatches);

	bool result = true;
	if (total_mismatches)
	{
		error_printf("Test FAILED\n");
		result = false;
	}
	else
		printf("Test succeeded\n");

	return result;
}

static int main_internal(int argc, const char **argv)
{
	printf("Basis Universal GPU Texture Compressor v" BASISU_TOOL_VERSION "\nCopyright (C) 2019-2022 Binomial LLC, All rights reserved\n");

	//interval_timer tm;
	//tm.start();

	// See if OpenCL support has been disabled. We don't want to parse the command line until the lib is initialized
	bool use_opencl = false;
	bool opencl_force_serialization = false;
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-opencl") == 0)
			use_opencl = true;
		if (strcmp(argv[i], "-opencl_serialize") == 0)
			opencl_force_serialization = true;
	}

#ifndef BASISU_SUPPORT_OPENCL
	if (use_opencl)
	{
		fprintf(stderr, "WARNING: -opencl specified, but OpenCL support was not enabled at compile time! With cmake, use -D OPENCL=1. Falling back to CPU compression.\n");
	}
#endif

	basisu_encoder_init(use_opencl, opencl_force_serialization);
		
	//printf("Encoder and transcoder libraries initialized in %3.3f ms\n", tm.get_elapsed_ms());

#if defined(DEBUG) || defined(_DEBUG)
	printf("DEBUG build\n");
#endif

	if (argc == 1)
	{
		print_usage();
		return EXIT_FAILURE;
	}

	command_line_params opts;
	if (!opts.parse(argc, argv))
	{
		//print_usage();
		return EXIT_FAILURE;
	}

#if BASISU_SUPPORT_SSE
	printf("Using SSE 4.1: %u, Multithreading: %u, Zstandard support: %u, OpenCL: %u\n", g_cpu_supports_sse41, (uint32_t)opts.m_comp_params.m_multithreading, basist::basisu_transcoder_supports_ktx2_zstd(), opencl_is_available());
#else
	printf("Multithreading: %u, Zstandard support: %u, OpenCL: %u\n", (uint32_t)opts.m_comp_params.m_multithreading, basist::basisu_transcoder_supports_ktx2_zstd(), opencl_is_available());
#endif

	if (!opts.process_listing_files())
		return EXIT_FAILURE;

	if (opts.m_mode == cDefault)
	{
		for (size_t i = 0; i < opts.m_input_filenames.size(); i++)
		{
			std::string ext(string_get_extension(opts.m_input_filenames[i]));
			if ((strcasecmp(ext.c_str(), "basis") == 0) || (strcasecmp(ext.c_str(), "ktx") == 0) || (strcasecmp(ext.c_str(), "ktx2") == 0))
			{
				// If they haven't specified any modes, and they give us a .basis file, then assume they want to unpack it.
				opts.m_mode = cUnpack;
				break;
			}
		}
	}

	bool status = false;

	switch (opts.m_mode)
	{
	case cDefault:
	case cCompress:
		status = compress_mode(opts);
		break;
	case cValidate:
	case cInfo:
	case cUnpack:
		status = unpack_and_validate_mode(opts);
		break;
	case cCompare:
		status = compare_mode(opts);
		break;
	case cVersion:
		status = true; // We printed the version at the beginning of main_internal
		break;
	case cBench:
		status = bench_mode(opts);
		break;
	case cCompSize:
		status = compsize_mode(opts);
		break;
	case cTest:
		status = test_mode(opts);
		break;
	case cSplitImage:
		status = split_image_mode(opts);
		break;
	case cCombineImages:
		status = combine_images_mode(opts);
		break;
	default:
		assert(0);
		break;
	}

	return status ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, const char** argv)
{
#ifdef _DEBUG
	printf("DEBUG\n");
#endif
		
	int status = EXIT_FAILURE;

#if BASISU_CATCH_EXCEPTIONS
	try
	{
		 status = main_internal(argc, argv);
	}
	catch (const std::exception &exc)
	{
		 fprintf(stderr, "Fatal error: Caught exception \"%s\"\n", exc.what());
	}
	catch (...)
	{
		fprintf(stderr, "Fatal error: Uncaught exception!\n");
	}
#else
	status = main_internal(argc, argv);
#endif

	return status;
}

