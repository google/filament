#ifdef WITH_BASISU

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wuninitialized-const-reference"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#pragma GCC diagnostic ignored "-Wstrict-aliasing" // TODO: https://github.com/BinomialLLC/basis_universal/pull/275
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4702) // unreachable code
#pragma warning(disable : 4005) // macro redefinition
#endif

#define BASISU_NO_ITERATOR_DEBUG_LEVEL

#if defined(_MSC_VER) && !defined(__clang__) && (defined(_M_IX86) || defined(_M_X64))
#define BASISU_SUPPORT_SSE 1
#endif

#if defined(__SSE4_1__)
#define BASISU_SUPPORT_SSE 1
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif

#include "encoder/basisu_backend.cpp"
#include "encoder/basisu_basis_file.cpp"
#include "encoder/basisu_bc7enc.cpp"
#include "encoder/basisu_comp.cpp"
#include "encoder/basisu_enc.cpp"
#include "encoder/basisu_etc.cpp"
#include "encoder/basisu_frontend.cpp"
#include "encoder/basisu_gpu_texture.cpp"
#include "encoder/basisu_kernels_sse.cpp"
#include "encoder/basisu_opencl.cpp"
#include "encoder/basisu_pvrtc1_4.cpp"
#include "encoder/basisu_resample_filters.cpp"
#include "encoder/basisu_resampler.cpp"
#include "encoder/basisu_ssim.cpp"
#include "encoder/basisu_uastc_enc.cpp"
#include "encoder/jpgd.cpp"
#include "encoder/pvpngreader.cpp"
#include "transcoder/basisu_transcoder.cpp"

#undef CLAMP
#include "zstd/zstd.c"

#endif
