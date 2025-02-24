/*!*********************************************************************************************************************
\File         VulkanIBLMapsGenerator.cpp
\Title        Vulkan Image Based Lighting Maps Generator
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        A command line tool using Headless (i.e. surface-less) Vulkan Graphics to calculate the Irradiance map (
			  used for Diffuse global illumination), and the Pre-filtered Reflection map (used for Specular global
			  illumination).
***********************************************************************************************************************/
#include "PVRCore/PVRCore.h"

#include "PVRUtils/OpenCL/OpenCLUtils.h"
#include <iostream>
#include <sstream>
#include <queue>
#include <iomanip>
#include <memory>
#include <array>

#if defined(_WIN32)
#include "PVRCore/Windows/WindowsResourceStream.h"
#elif defined(__ANDROID__)
#include "PVRCore/Android/AndroidAssetStream.h"
#endif

#include "Matrix.h"

// Print the supported command line parameters to standard out
void printHelp()
{
	std::cout << std::endl;
	std::cout << "Supported command line options:" << std::endl;
	std::cout << "    -h                    : Displays this help message" << std::endl;
	std::cout << "    -v                    : Verbose mode. Adds a lot more timing information" << std::endl;
	std::cout << "    -va                   : Validate mode: Will use a naive CPU-side matrix multiplication to validate all results against. This takes at least an order of "
				 "magnitude more time than OpenCL kernels."
			  << std::endl;
	std::cout << "    -kernel [kernelname]  : The name of a kernel. If left blank, a benchmarking/demo mode will be run, for all kernels. Supported kernels:" << std::endl;
	std::cout
		<< "                            matmul_naive          : NOT optimal. A naive implementation of matrix multiplication, as explained algorithmically on math textbooks, "
		   "no optimisations"
		<< std::endl;
	std::cout << "                            matmul_transposedA    : NOT optimal. The naive implementation, but the LEFT   matrix(A) is transposed to observe the effect on "
				 "performance due to cache locality"
			  << std::endl;
	std::cout << "                            matmul_transposedB    : NOT optimal. The naive implementation, but the RIGHT  matrix(B) is transposed to observe the effect on "
				 "performance due to cache locality"
			  << std::endl;
	std::cout << "                            matmul_transposedC    : NOT optimal. The naive implementation, but the RESULT matrix(C) is transposed to observe the effect on "
				 "performance due to cache locality"
			  << std::endl;
	std::cout << "                            matmul_transposedAC   : NOT optimal. The naive implementation, but the LEFT  and the RESULT matrices(A and C) are transposed to "
				 "observe the effect on performance due to cache locality"
			  << std::endl;
	std::cout << "                            matmul_transposedBC   : NOT optimal. The naive implementation, but the RIGHT and the RESULT matrices(B and C) are transposed to "
				 "observe the effect on performance due to cache locality"
			  << std::endl;
	std::cout
		<< "                            matmul_linearwg_AT    : NOT optimal. The naive implementation, but the Workgroup size is LINEAR (1xSIZEx1) and A is transposed so that "
		   "workgroup shape effects can be compared."
		<< std::endl;
	std::cout
		<< "                            matmul_linearwg_BT    : NOT optimal. The naive implementation, but the Workgroup size is LINEAR (1xSIZEx1) and A is transposed so that "
		   "workgroup shape effects can be compared."
		<< std::endl;
	std::cout << "                            matmul_linearwg_vec4  : NOT optimal. Simple optimisation similar to matmul_linearwg_BT, where the matrices have been expressed as "
				 "vec4s, increasing the work done per thread."
			  << std::endl;
	std::cout << "                            matmul_linearwg_vec4_local: NOT optimal. Simplistic local-memory optimisation, where a full row of A is loaded into local "
				 "memory, to take advantage of the linear-workgroup"
			  << std::endl
			  << "                                 paradigm above. The matrices are expressed as Vec4's, A is transposed, and an entire horizontal line "
				 "of A is preloaded for each thread, hence execution will fail if the shared dimension N is too large."
			  << std::endl;
	std::cout << "                            matmul_tile_square    : NOT completely optimal. It will use local memory optimisation to load an area of A and B into local"
				 " memory, and use It will calculate the output matrix "
			  << std::endl
			  << "                                 tile-by-tile, dramatically improving memory locality. The result is identical to the matmul_tile_rect algorithm for square "
				 "dimensions, "
				 "but is much simpler to read and understand."
			  << std::endl;
	std::cout
		<< "                            matmul_tile_rect      : ALMOST optimal - see below. This is a generalised tiled algorithm, where the tile's shape is arbitrary. There "
		   "are actually 3 tile dimensions: "
		<< std::endl
		<< "                                 The m size, the n size and the p size. Reading A uses m and n, reading B uses n and p, writing C uses m and p, and all 3 "
		   "dimensions are configurable calculate a workgroup-sized part of the output matrix piece by piece, "
		<< std::endl
		<< "                                 dramatically improving memory locality. Being able to tune the "
		   "size of the workgroup means being able to completely optimise for different architectures."
		<< std::endl;
	std::cout << "                            matmul_tile_rect_vec4 : OPTIMAL ALGORITHM. This is the generalised tiled algorithm, and the matrices are expressed as vec4's, "
				 "increasing the amount of work per thread. "
			  << std::endl
			  << "                                 The tile's shape is arbitrary, hence cam be optimised for different hardware. The tiling algorithm will calculate "
				 "a workgroup-sized part of the output matrix piece by piece, "
			  << std::endl
			  << "                                 dramatically improving memory locality. Being able to tune the size of the workgroup means "
				 "being able to completely optimise for different architectures."
			  << std::endl;
	std::cout << "    -m -n -p              : The corresponding matrix dimension (left matrix: MxN, right matrix: NxP. Must be a multiple of workgroup size. "
				 "Default: 1024,1024,1024"
			  << std::endl;
	std::cout << "    -wg_square_side       : The length of the side of the workgroup (e.g.: 8 for an 8x8 workgroup), for the matmul_tile_square kernel. Default 8." << std::endl;
	std::cout << "    -wg_linear_size       : The size of the linear workgroup" << std::endl;
	std::cout << "    -wg_rect_width        : The x size of the workgroup, for the matmul_tile_rect and matmul_tile_rect_vec4" << std::endl;
	std::cout << "    -wg_rect_height       : The y size of the workgroup, for the matmul_tile_rect and matmul_tile_rect_vec4" << std::endl;
	std::cout << "    -tile_square          : The size of the square tile" << std::endl;
	std::cout << "    -tile_rect_m          : The size of the M side of the tile rectangles." << std::endl;
	std::cout << "    -tile_rect_n          : The size of the N side of the tile rectangles." << std::endl;
	std::cout << "    -tile_rect_p          : The size of the P side of the tile rectangles." << std::endl;
}

struct KernelParams
{
	pvr::Time myclock;
	pvr::Time mytime;
	cl_program program;
	cl_command_queue commandqueue;
	size_t* globalSize;
	size_t* localSize;
};

template<typename T>
void validate_result(const T& a, const T& b)
{
	std::cout << (notEquals(a, b, 1.f).isZero() ? "*** SUCCESSFUL ***" : " +++  FAILED  +++ ");
}

std::map<float, std::string> results;

bool VERBOSE = false;
bool VALIDATE = false;

template<typename TC>
void execute_kernel(KernelParams& k, const char* kernelName, cl_mem memA, cl_mem memB, cl_mem memC, TC& C, const TC& D, const std::string& note = "")
{
	cl_int err;
	std::cout << std::endl << "===== EXECUTION:  " << std::left << std::setw(30) << std::string(kernelName) << std::setw(10) << note << std::flush;
	if (VERBOSE)
	{
		std::cout << "      =====" << std::endl;
		std::cout << "==> Creating kernels                          - Time: " << k.myclock.getElapsedSecsF() << std::flush;
		k.mytime.Reset();
	}
	cl_kernel kernel = cl::CreateKernel(k.program, kernelName, &err);

	if (VERBOSE)
	{
		std::cout << "(" << k.mytime.getElapsedMilliSecsF() << "ms)" << std::endl
				  << "==> Setting up kernel arguments               - Time: " << k.myclock.getElapsedSecsF() << std::flush;
		k.mytime.Reset();
	}
	clutils::throwOnFailure(err, "Create kernel");
	clutils::throwOnFailure(cl::SetKernelArg(kernel, 0, sizeof(cl_mem), &memA), "Set kernel arg A");
	clutils::throwOnFailure(cl::SetKernelArg(kernel, 1, sizeof(cl_mem), &memB), "Set kernel arg B");
	clutils::throwOnFailure(cl::SetKernelArg(kernel, 2, sizeof(cl_mem), &memC), "Set kernel arg C");

	cl_event kernel_executing;
	if (VERBOSE)
	{
		std::cout << "(" << k.mytime.getElapsedMilliSecsF() << "ms)" << std::endl
				  << "==> Executing                                 - Time: " << k.myclock.getElapsedSecsF() << std::flush;
	}
	k.mytime.Reset();
	clutils::throwOnFailure(
		cl::EnqueueNDRangeKernel(k.commandqueue, kernel, k.globalSize[1] == 0 ? 1 : 2, NULL, k.globalSize, k.localSize, 0, nullptr, &kernel_executing), "Enqueue Kernel");
	clutils::throwOnFailure(cl::WaitForEvents(1, &kernel_executing), "Waiting for Kernel results");
	std::cout << "(" << std::fixed << std::setprecision(2) << std::setw(7) << k.mytime.getElapsedMilliSecsF() << "ms)" << std::flush;
	if (VERBOSE) { std::cout << std::endl << "==> Reading back the results                  - Time: " << k.myclock.getElapsedSecsF() << std::flush; }
	results[k.mytime.getElapsedMilliSecsF()] = kernelName + note;

	cl::ReleaseKernel(kernel);

	k.mytime.Reset();

	clutils::throwOnFailure(cl::EnqueueReadBuffer(k.commandqueue, memC, true, 0, C.size() * sizeof(float), C.data(), 1, &kernel_executing, nullptr), "Enqueue read data");
	if (VERBOSE)
	{
		std::cout << "( ";
		std::cout << k.mytime.getElapsedMilliSecsF();
		std::cout << " ms)" << std::endl;
	}
	k.mytime.Reset();

	if (VALIDATE)
	{
		if (VERBOSE) { std::cout << std::endl << "==> Validating results:   " << std::flush; }
		validate_result(C, D);
		if (VERBOSE) { std::cout << "  - Time: " << k.myclock.getElapsedSecsF() << std::endl; }
	}
}

template<typename TC>
void try_catch_execute_kernel(KernelParams& k, const char* kernelName, cl_mem memA, cl_mem memB, cl_mem memC, TC& C, const TC& D, const std::string& note = "")
{
	try
	{
		execute_kernel(k, kernelName, memA, memB, memC, C, D, note);
	}
	catch (std::exception& err)
	{
		std::cout << "Error encountered: " << err.what();
	}
	catch (...)
	{
		std::cout << "Execution encountered an unknown error";
	}
}

bool assert_param(const std::string& message, bool result)
{
	if (!result) { std::cout << "Input error: " << message << std::endl; }
	return result;
}

int main(int argc, char** argv)
{
	using std::cout;
	using std::cerr;
	try
	{
		pvr::platform::CommandLineParser parser(argc - 1, argv + 1);
		const pvr::CommandLine& cmdLine = parser.getParsedCommandLine();
		cout << "\nSingle Precision General Matrix Multiplication (SGEMM) benchmarking test." << std::endl;

		std::string kernelfile("kernel.cl");
		cmdLine.getStringOption("-k", kernelfile);
		cmdLine.getBoolOptionSetTrueIfPresent("-v", VERBOSE);
		cmdLine.getBoolOptionSetTrueIfPresent("-va", VALIDATE);
		bool help = false;
		cmdLine.getBoolOptionSetTrueIfPresent("-h", help);
		if (help)
		{
			printHelp();
			return 0;
		}
		pvr::FilePath path(argv[0]);
		auto fs = pvr::FileStream::createFileStream(kernelfile, "r", false);

		if (!fs->isReadable())
		{
			fs = pvr::FileStream::createFileStream(
				path.getDirectory() + pvr::FilePath::getDirectorySeparator() + "Assets_" + path.getFilenameNoExtension() + pvr::FilePath::getDirectorySeparator() + kernelfile, "r",
				false);
		}
#ifdef _WIN32
		if (!fs->isReadable()) { fs.reset(new pvr::WindowsResourceStream(kernelfile, false)); }
#endif

		if (!fs->isReadable()) { throw pvr::FileNotFoundError(kernelfile); }

		cl_platform_id platform;
		cl_device_id device;
		cl_context context;
		cl_command_queue commandqueue;

		clutils::createOpenCLContext(platform, device, context, commandqueue);

		cl_int err;

		pvr::Time mytime;
		pvr::Time myclock;

		int M = 512, N = 1536, P = 1024;

		// This will typically be the output matrix
		size_t global_size[] = { (size_t)M, (size_t)P };
		size_t local_size[] = { 0, 0 };

		int32_t wg_width = 8;
		int32_t wg_height = 4;
		int32_t square_wg_side = 16;
		int32_t linear_wg_size = 32;
		int32_t tile_square_side = 8;
		int32_t tile_rect_m = 8;
		int32_t tile_rect_n = 64;
		int32_t tile_rect_p = 4;

		cmdLine.getIntOption("-M", M);
		cmdLine.getIntOption("-N", N);
		cmdLine.getIntOption("-P", P);
		cmdLine.getIntOption("-m", M);
		cmdLine.getIntOption("-n", N);
		cmdLine.getIntOption("-p", P);

		dynmatrix<> A(M, N);
		dynmatrix<> B(N, P);
		dynmatrix<> C(M, P);
		dynmatrix<> CT(P, M);

		cmdLine.getIntOption("-wg_square_side", square_wg_side);
		cmdLine.getIntOption("-wg_linear_size", linear_wg_size);
		cmdLine.getIntOption("-wg_rect_width", wg_width);
		cmdLine.getIntOption("-wg_rect_height", wg_width);
		cmdLine.getIntOption("-tile_square", tile_square_side);
		cmdLine.getIntOption("-tile_rect_m", tile_rect_m);
		cmdLine.getIntOption("-tile_rect_n", tile_rect_n);
		cmdLine.getIntOption("-tile_rect_p", tile_rect_p);

		std::string kernel_name;
		cmdLine.getStringOption("-kernel", kernel_name);

		std::array<std::string, 13> kernel_names = { "matmul_naive", "matmul_transposedA", "matmul_transposedB", "matmul_transposedC", "matmul_transposedAC", "matmul_transposedBC",
			"matmul_linearwg_AT", "matmul_linearwg_BT", "matmul_linearwg_vec4", "matmul_linearwg_vec4_local", "matmul_tile_square", "matmul_tile_rect", "matmul_tile_rect_vec4" };
		std::array<bool, 13> kernel_bools = { true, true, true, true, true, true, true, true, true, true, true, true, true };

		if (!kernel_name.empty())
		{
			std::fill(kernel_bools.begin(), kernel_bools.end(), false);
			for (size_t i = 0; i < kernel_names.size(); ++i)
			{
				if (kernel_names[i] == kernel_name)
				{
					kernel_bools[i] = true;
					break;
				}
			}
		}

		// Ensure correct behaviour
		bool success = true;
		success = (assert_param("M must be a multiple of wg_height", M % wg_height == 0) && success);
		success = (assert_param("P must be a multiple of wg_width", P % wg_width == 0) && success);

		success = (assert_param("M must be a multiple of tile_square", M % tile_square_side == 0) && success);
		success = (assert_param("N must be a multiple of tile_square", N % tile_square_side == 0) && success);
		success = (assert_param("P must be a multiple of tile_square", P % tile_square_side == 0) && success);

		success = (assert_param("N must be a multiple of wg_linear_size", N % linear_wg_size == 0) && success);

		success = (assert_param("M must be a multiple of tile_rect_m", M % tile_rect_m == 0) && success);
		success = (assert_param("N must be a multiple of tile_rect_n", N % tile_rect_n == 0) && success);
		success = (assert_param("N must be a multiple of tile_rect_p", P % tile_rect_p == 0) && success);

		success = (assert_param("tile_size_n must be a multiple of (least common multiple of tile_rect_m,tile_rect_p)x4",
					   (tile_rect_n % (pvr::math::lcm(tile_rect_m, tile_rect_p) * 4) == 0)) &&
			success);

		if (!success) return 1;

		if (cmdLine.getOptionsList().empty()) { std::cout << "Running DEMO mode. "; }

		cout << "\nM: " << std::setw(6) << M << "     N: " << std::setw(6) << N << "     P:" << std::setw(6) << P << std::endl
			 << "Left Matrix(MxN):  " << M << "x" << N << "      Right Matrix(NxP): " << N << "x" << P << std::endl;

		cmdLine.getBoolOptionSetTrueIfPresent("-va", VALIDATE);
		cmdLine.getBoolOptionSetTrueIfPresent("-v", VERBOSE);

		std::string def00 = pvr::strings::createFormatted("#define M %d\n" //
														  "#define N %d\n" //
														  "#define P %d\n" //
														  "#define WG_RECT_WIDTH %d\n" //
														  "#define WG_RECT_HEIGHT %d\n" //
														  "#define WG_SQUARE_SIDE %d\n" //
														  "#define WG_LINEAR_SIZE %d\n" //
														  "#define TILE_SQUARE %d\n" //
														  "#define TILE_RECT_M %d\n" //
														  "#define TILE_RECT_N %d\n" //
														  "#define TILE_RECT_P %d\n", //
			M, N, P, wg_width, wg_height, square_wg_side, linear_wg_size, tile_square_side, tile_rect_m, tile_rect_n, tile_rect_p);
		std::cout << "==> Creating OpenCL program                   - Time: " << mytime.getElapsedMicroSecs() / float(1000) << std::endl;

		const char* defines00[] = { def00.c_str() };
		const char* options00 = "-cl-fast-relaxed-math";
		cl_program program = clutils::loadKernelProgram(context, device, *fs, options00, defines00, 1);

		std::cout << "==> Beginning Matrix setup                    - Time: " << mytime.getElapsedMicroSecs() / float(1000);

		std::cout << "(" << mytime.getElapsedMilliSecsF() << "ms)" << std::endl << "==> Populating Matrices                       - Time: " << myclock.getElapsedSecsF();
		mytime.Reset();

		for (int i = 0; i < M; ++i)
		{
			for (int j = 0; j < N; ++j) { A(i, j) = pvr::randomrange(-20.f, 20.f); }
			for (int j = 0; j < P; ++j) { C(i, j) = 99999.f; }
		}

		for (int i = 0; i < N; ++i)
		{
			for (int j = 0; j < P; ++j) { B(i, j) = pvr::randomrange(-10.f, 10.f); }
		}

		std::cout << "(" << mytime.getElapsedMilliSecsF() << "ms)" << std::endl << "==> Transposing Matrix B                      - Time: " << myclock.getElapsedSecsF();
		auto AT = transpose(A);
		auto BT = transpose(B);

		std::cout << "(" << mytime.getElapsedMilliSecsF() << "ms)" << std::endl << "==> Setting up buffers                        - Time: " << myclock.getElapsedSecsF();
		mytime.Reset();

		cl_mem memA = cl::CreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY, A.size() * sizeof(float), A.data(), &err);
		clutils::throwOnFailure(err, "Create buffer A");
		cl_mem memAT = cl::CreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY, AT.size() * sizeof(float), AT.data(), &err);
		clutils::throwOnFailure(err, "Create buffer A_Transposed");
		cl_mem memB = cl::CreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY, B.size() * sizeof(float), B.data(), &err);
		clutils::throwOnFailure(err, "Create buffer B");
		cl_mem memBT = cl::CreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY, B.size() * sizeof(float), BT.data(), &err);
		clutils::throwOnFailure(err, "Create buffer B_Transposed");
		cl_mem memC = cl::CreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE, C.size() * sizeof(float), nullptr, &err);
		clutils::throwOnFailure(err, "Create buffer C");
		std::cout << "(" << mytime.getElapsedMilliSecsF() << "ms)" << std::endl;
		std::cout << "==> Preparing ground truth: CPU-side multiply  - Time: " << myclock.getElapsedSecsF();
		mytime.Reset();

		const auto D = VALIDATE ? A * B : dynmatrix<>(M, P);
		const auto DT = VALIDATE ? transpose(D) : dynmatrix<>(P, M);
		std::cout << "(" << mytime.getElapsedMilliSecsF() << "ms)" << std::endl;

		KernelParams k;
		k.commandqueue = commandqueue;
		k.myclock = myclock;
		k.mytime = mytime;
		k.program = program;
		k.globalSize = global_size;
		k.localSize = local_size;

		global_size[0] = M;
		global_size[1] = P;
		local_size[0] = wg_width;
		local_size[1] = wg_height;

		if (kernel_bools[1]) try_catch_execute_kernel(k, "matmul_transposedA", memAT, memB, memC, C, D);
		if (kernel_bools[2]) try_catch_execute_kernel(k, "matmul_transposedB", memA, memBT, memC, C, D);
		if (kernel_bools[3]) try_catch_execute_kernel(k, "matmul_transposedC", memA, memB, memC, CT, DT);

		if (kernel_bools[4]) try_catch_execute_kernel(k, "matmul_transposedAC", memAT, memB, memC, CT, DT);
		if (kernel_bools[5]) try_catch_execute_kernel(k, "matmul_transposedBC", memA, memBT, memC, CT, DT);

		local_size[0] = 1;
		local_size[1] = linear_wg_size;

		if (kernel_bools[6]) try_catch_execute_kernel(k, "matmul_linearwg_AT", memAT, memB, memC, C, D);
		if (kernel_bools[7]) try_catch_execute_kernel(k, "matmul_linearwg_BT", memA, memBT, memC, C, D);
		if (kernel_bools[8]) try_catch_execute_kernel(k, "matmul_linearwg_vec4", memA, memBT, memC, C, D);
		if (kernel_bools[9]) try_catch_execute_kernel(k, "matmul_linearwg_vec4_local", memA, memBT, memC, C, D);

		local_size[0] = square_wg_side;
		local_size[1] = square_wg_side;
		if (kernel_bools[10]) try_catch_execute_kernel(k, "matmul_tile_square", memA, memBT, memC, C, D);

		local_size[0] = tile_rect_m;
		local_size[1] = tile_rect_p;
		if (kernel_name.empty()) try_catch_execute_kernel(k, "matmul_tile_rect", memA, memBT, memC, C, D);

		int pmin = 4;
		int pmax = 32;
		int mmin = 2;
		int mmax = 32;
		int nmax = 128;

		if (kernel_name.empty())
		{
			cl::ReleaseProgram(k.program);
			for (int p = pmin; p < pmax; p <<= 1)
			{
				for (int m = mmin; m < mmax; m <<= 1)
				{
					for (int n = pvr::math::lcm(p * 4, m * 4); n < nmax; n <<= 1)
					{
						std::stringstream ss;
						try
						{
							std::string def0 = pvr::strings::createFormatted("#define M %d\n" //
																			 "#define N %d\n" //
																			 "#define P %d\n" //
																			 "#define WG_RECT_WIDTH %d\n" //
																			 "#define WG_RECT_HEIGHT %d\n" //
																			 "#define WG_SQUARE_SIDE %d\n" //
																			 "#define WG_LINEAR_SIZE %d\n" //
																			 "#define TILE_SQUARE %d\n" //
																			 "#define TILE_RECT_M %d\n" //
																			 "#define TILE_RECT_N %d\n" //
																			 "#define TILE_RECT_P %d\n", //
								M, N, P, wg_width, wg_height, square_wg_side, linear_wg_size, tile_square_side, m, n, p);

							ss << m << "x" << n << "x" << p;
							if (VERBOSE) { std::cout << std::endl << "==> Creating OpenCL program " << ss.str() << " - Time : " << mytime.getElapsedMicroSecs() / float(1000); }

							const char* defines[] = { def0.c_str() };
							const char* options = "-cl-fast-relaxed-math";
							fs->seek(0, pvr::Stream::SeekOrigin::SeekOriginFromStart);
							k.program = clutils::loadKernelProgram(context, device, *fs, options, defines, 1);

							local_size[0] = m;
							local_size[1] = p;
							execute_kernel(k, "matmul_tile_rect_vec4", memA, memBT, memC, C, D, " " + ss.str());
							cl::ReleaseProgram(k.program);
						}
						catch (...)
						{
							std::cout << "+++ Kernel matmul_tile_rect_vec4_" << ss.str() << " failed to execute!" << std::endl;
						}
					}
				}
			}
		}

		int rank = 1;
		std::cout << std::endl << std::endl << std::endl << "*** HALL OF FAME ***" << std::endl;
		std::cout << "--------------------------------------------------" << std::endl;
		for (auto it = results.begin(); it != results.end(); ++it, ++rank)
		{ std::cout << std::right << std::setw(2) << rank << " : " << std::left << std::setw(32) << it->second << std::right << "\t(" << it->first << "ms)" << std::endl; }

		cl::ReleaseMemObject(memA);
		cl::ReleaseMemObject(memAT);
		cl::ReleaseMemObject(memB);
		cl::ReleaseMemObject(memBT);
		cl::ReleaseMemObject(memC);
		cl::ReleaseCommandQueue(commandqueue);
		cl::ReleaseDevice(device);
		cl::ReleaseContext(context);
	}
	catch (std::exception& err)
	{
		cout << "Error encountered: " << err.what();
	}
	catch (...)
	{
		cout << "Execution encountered an unknown error";
	}
	cout << "\n";
	return 0;
}
