project(basisu)

cmake_minimum_required(VERSION 3.19)

set(ZSTD zstd-basisu)

set(TRANSCODER_SRC
    ../transcoder/basisu_transcoder.cpp
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

# The following BasisU setting is useful when diagnosing issues, but we're leaving it turned off
# even for debug builds, since it is quite verbose.
# set (BASIS_CONFIG ${BASIS_CONFIG} $<$<CONFIG:Debug>:BASISU_FORCE_DEVEL_MESSAGES=1>)

# DXT5A and DXT1 are both required for cTFBC3_RGBA aka DXT5_RGBA.
if (NOT IS_MOBILE_TARGET)
    set (BASIS_CONFIG ${BASIS_CONFIG} BASISD_SUPPORT_DXT5A=1 BASISD_SUPPORT_DXT1=1)
else()
    set (BASIS_CONFIG ${BASIS_CONFIG} BASISD_SUPPORT_DXT5A=0 BASISD_SUPPORT_DXT1=0)
endif()

add_library(${ZSTD} ../zstd/zstd.c)
add_library(basis_encoder ${ENCODER_SRC})
add_library(basis_transcoder ${TRANSCODER_SRC})

target_link_libraries(basis_transcoder PRIVATE ${ZSTD})
target_link_libraries(basis_encoder basis_transcoder)

target_include_directories(${ZSTD} PUBLIC ../zstd)
target_include_directories(basis_encoder PUBLIC ../encoder)
target_include_directories(basis_transcoder PUBLIC ../transcoder)

target_compile_definitions(basis_encoder PRIVATE ${BASIS_CONFIG})
target_compile_definitions(basis_transcoder PRIVATE ${BASIS_CONFIG})

set_target_properties(${ZSTD} PROPERTIES FOLDER ThirdParty)
set_target_properties(basis_encoder PROPERTIES FOLDER ThirdParty)
set_target_properties(basis_transcoder PROPERTIES FOLDER ThirdParty)

if (IS_HOST_PLATFORM)
    add_executable(basisu ../basisu_tool.cpp)
    target_link_libraries(basisu basis_encoder)
    set_target_properties(basisu PROPERTIES FOLDER ThirdParty)
    target_compile_definitions(basisu PRIVATE ${BASIS_CONFIG})
    install(TARGETS basisu DESTINATION bin)

    if (NOT MSVC AND NOT ANDROID)
       target_link_libraries(basisu m pthread)
    endif()
endif()

install(TARGETS basis_transcoder ARCHIVE DESTINATION lib/${DIST_DIR})
