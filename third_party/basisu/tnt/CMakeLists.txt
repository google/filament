project(basisu)

cmake_minimum_required(VERSION 3.19)

set(TRANSCODER_SRC
    ../transcoder/basisu_transcoder.cpp
    ../zstd/zstd.c
)

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
)

# Filament does not support certain compressed formats that BasisU can transcode
# to, so we disable them in the build to save space. Some of these mappings are
# confusing. (e.g., DXT5A corresponds to transcoder_texture_format::cTFBC4_R).
set (BASIS_CONFIG
    BASISD_SUPPORT_KTX2=1
    BASISD_SUPPORT_KTX2_ZSTD=1
    BASISD_SUPPORT_ATC=0
    BASISD_SUPPORT_BC7=0
    BASISD_SUPPORT_PVRTC1=0
    BASISD_SUPPORT_PVRTC2=0
    BASISD_SUPPORT_FXT1=0
)

# DXT5A and DXT1 are both required for cTFBC3_RGBA aka DXT3_RGBA.
if (NOT IS_MOBILE_TARGET)
    set (BASIS_CONFIG ${BASIS_CONFIG} BASISD_SUPPORT_DXT5A=1 BASISD_SUPPORT_DXT1=1)
else()
    set (BASIS_CONFIG ${BASIS_CONFIG} BASISD_SUPPORT_DXT5A=0 BASISD_SUPPORT_DXT1=0)
endif()

add_executable(basisu ${ENCODER_SRC} ${TRANSCODER_SRC} ../basisu_tool.cpp)
add_library(basis_encoder ${ENCODER_SRC} ${TRANSCODER_SRC})
add_library(basis_transcoder ${TRANSCODER_SRC})

target_include_directories(basis_encoder PUBLIC ../encoder)
target_include_directories(basis_transcoder PUBLIC ../transcoder)

target_compile_definitions(basisu PRIVATE ${BASIS_CONFIG})
target_compile_definitions(basis_encoder PRIVATE ${BASIS_CONFIG})
target_compile_definitions(basis_transcoder PRIVATE ${BASIS_CONFIG})

if (NOT MSVC)
   target_link_libraries(basisu m pthread)
endif()

install(TARGETS basisu DESTINATION bin)
