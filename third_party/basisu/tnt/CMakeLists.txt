project(basisu)

cmake_minimum_required(VERSION 3.19)

set(ENCODER_SRC
    ../encoder/basisu_backend.cpp
    ../encoder/basisu_basis_file.cpp
    ../encoder/basisu_comp.cpp
    ../encoder/basisu_enc.cpp
    ../encoder/basisu_etc.cpp
    ../encoder/basisu_frontend.cpp
    ../encoder/basisu_gpu_texture.cpp
    ../encoder/basisu_pvrtc1_4.cpp
    ../encoder/basisu_resampler.cpp
    ../encoder/basisu_resample_filters.cpp
    ../encoder/basisu_ssim.cpp
    ../encoder/basisu_uastc_enc.cpp
    ../encoder/basisu_bc7enc.cpp
    ../encoder/jpgd.cpp
    ../encoder/basisu_kernels_sse.cpp
    ../encoder/basisu_opencl.cpp
    ../encoder/pvpngreader.cpp
    ../transcoder/basisu_transcoder.cpp
    ../zstd/zstd.c
)

add_executable(basisu ${ENCODER_SRC} ../basisu_tool.cpp)
add_library(basis_encoder ${ENCODER_SRC})
add_library(basis_transcoder ../transcoder/basisu_transcoder.cpp)

target_include_directories(basis_encoder PUBLIC ../encoder)
target_include_directories(basis_transcoder PUBLIC ../transcoder)

target_compile_definitions(basisu PRIVATE BASISD_SUPPORT_KTX2_ZSTD=1)
target_compile_definitions(basis_encoder PRIVATE BASISD_SUPPORT_KTX2_ZSTD=1)
target_compile_definitions(basis_transcoder PRIVATE BASISD_SUPPORT_KTX2_ZSTD=1)

if (NOT MSVC)
   target_link_libraries(basisu m pthread ${BASISU_EXTRA_LIBS})
endif()

install(TARGETS basisu DESTINATION bin)
