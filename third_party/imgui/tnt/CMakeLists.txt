cmake_minimum_required(VERSION 3.19)
project(imgui)

set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(TARGET              imgui)

set(SRC_DIR             ${OUR_DIR})
set(PUBLIC_HDR_DIR      ${OUR_DIR})
set(PRIVATE_HDR_DIR     ${OUR_DIR})

set(PUBLIC_HDRS
    ${PUBLIC_HDR_DIR}/imconfig.h
    ${PUBLIC_HDR_DIR}/imgui.h
)

set(PRIVATE_HDRS
    ${PRIVATE_HDR_DIR}/imgui_internal.h
    ${PRIVATE_HDR_DIR}/imstb_rectpack.h
    ${PRIVATE_HDR_DIR}/imstb_textedit.h
    ${PRIVATE_HDR_DIR}/imstb_truetype.h
)

set(SRCS
    ${SRC_DIR}/imgui.cpp
    ${SRC_DIR}/imgui_demo.cpp
    ${SRC_DIR}/imgui_draw.cpp
    ${SRC_DIR}/imgui_tables.cpp
    ${SRC_DIR}/imgui_widgets.cpp
)

add_library(${TARGET} STATIC ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR} ${PRIVATE_HDR_DIR})

set_target_properties(${TARGET} PROPERTIES FOLDER ThirdParty)
