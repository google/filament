cmake_minimum_required(VERSION 3.19)
project(assimp)

set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(TARGET              assimp)
set(SRC_DIR             ${OUR_DIR})
set(PUBLIC_HDR_DIR      ${OUR_DIR}/include)
set(PRIVATE_HDR_DIR     ${OUR_DIR}/code)

# public headers dependencies
set(PUBLIC_HDRS
    ${PUBLIC_HDR_DIR}/${TARGET}/DefaultIOStream.h
    ${PUBLIC_HDR_DIR}/${TARGET}/DefaultIOSystem.h
    ${PUBLIC_HDR_DIR}/${TARGET}/Defines.h
    ${PUBLIC_HDR_DIR}/${TARGET}/SceneCombiner.h
    ${PUBLIC_HDR_DIR}/${TARGET}/ai_assert.h
    ${PUBLIC_HDR_DIR}/${TARGET}/anim.h
    ${PUBLIC_HDR_DIR}/${TARGET}/camera.h
    ${PUBLIC_HDR_DIR}/${TARGET}/cexport.h
    ${PUBLIC_HDR_DIR}/${TARGET}/cfileio.h
    ${PUBLIC_HDR_DIR}/${TARGET}/cimport.h
    ${PUBLIC_HDR_DIR}/${TARGET}/color4.h
    ${PUBLIC_HDR_DIR}/${TARGET}/config.h
    ${PUBLIC_HDR_DIR}/${TARGET}/defs.h
    ${PUBLIC_HDR_DIR}/${TARGET}/importerdesc.h
    ${PUBLIC_HDR_DIR}/${TARGET}/light.h
    ${PUBLIC_HDR_DIR}/${TARGET}/material.h
    ${PUBLIC_HDR_DIR}/${TARGET}/matrix3x3.h
    ${PUBLIC_HDR_DIR}/${TARGET}/matrix4x4.h
    ${PUBLIC_HDR_DIR}/${TARGET}/mesh.h
    ${PUBLIC_HDR_DIR}/${TARGET}/metadata.h
    ${PUBLIC_HDR_DIR}/${TARGET}/postprocess.h
    ${PUBLIC_HDR_DIR}/${TARGET}/quaternion.h
    ${PUBLIC_HDR_DIR}/${TARGET}/scene.h
    ${PUBLIC_HDR_DIR}/${TARGET}/texture.h
    ${PUBLIC_HDR_DIR}/${TARGET}/types.h
    ${PUBLIC_HDR_DIR}/${TARGET}/vector2.h
    ${PUBLIC_HDR_DIR}/${TARGET}/vector3.h
    ${PUBLIC_HDR_DIR}/${TARGET}/version.h
)

# private headers dependencies
set(PRIVATE_HDRS
    ${PRIVATE_HDR_DIR}/Common/BaseProcess.h
    ${PRIVATE_HDR_DIR}/CApi/CInterfaceIOWrapper.h
    ${PRIVATE_HDR_DIR}/PostProcessing/CalcTangentsProcess.h
    ${PRIVATE_HDR_DIR}/PostProcessing/ComputeUVMappingProcess.h
    ${PRIVATE_HDR_DIR}/PostProcessing/ConvertToLHProcess.h
    ${PRIVATE_HDR_DIR}/PostProcessing/DeboneProcess.h
    ${PRIVATE_HDR_DIR}/Common/DefaultProgressHandler.h
    ${PRIVATE_HDR_DIR}/PostProcessing/DropFaceNormalsProcess.h
    ${PRIVATE_HDR_DIR}/PostProcessing/EmbedTexturesProcess.h
    ${PRIVATE_HDR_DIR}/FBX/FBXCommon.h
    ${PRIVATE_HDR_DIR}/FBX/FBXCompileConfig.h
    ${PRIVATE_HDR_DIR}/FBX/FBXConverter.h
    ${PRIVATE_HDR_DIR}/FBX/FBXDocument.h
    ${PRIVATE_HDR_DIR}/FBX/FBXDocumentUtil.h
    ${PRIVATE_HDR_DIR}/FBX/FBXImportSettings.h
    ${PRIVATE_HDR_DIR}/FBX/FBXImporter.h
    ${PRIVATE_HDR_DIR}/FBX/FBXMeshGeometry.h
    ${PRIVATE_HDR_DIR}/FBX/FBXParser.h
    ${PRIVATE_HDR_DIR}/FBX/FBXProperties.h
    ${PRIVATE_HDR_DIR}/FBX/FBXTokenizer.h
    ${PRIVATE_HDR_DIR}/FBX/FBXUtil.h
    ${PRIVATE_HDR_DIR}/Common/FileLogStream.h
    ${PRIVATE_HDR_DIR}/Common/FileSystemFilter.h
    ${PRIVATE_HDR_DIR}/PostProcessing/FindDegenerates.h
    ${PRIVATE_HDR_DIR}/PostProcessing/FindInstancesProcess.h
    ${PRIVATE_HDR_DIR}/PostProcessing/FindInvalidDataProcess.h
    ${PRIVATE_HDR_DIR}/PostProcessing/FixNormalsStep.h
    ${PRIVATE_HDR_DIR}/PostProcessing/GenFaceNormalsProcess.h
    ${PRIVATE_HDR_DIR}/PostProcessing/GenVertexNormalsProcess.h
    ${PRIVATE_HDR_DIR}/Common/IFF.h
    ${PRIVATE_HDR_DIR}/Common/Importer.h
    ${PRIVATE_HDR_DIR}/PostProcessing/ImproveCacheLocality.h
    ${PRIVATE_HDR_DIR}/PostProcessing/JoinVerticesProcess.h
    ${PRIVATE_HDR_DIR}/PostProcessing/LimitBoneWeightsProcess.h
    ${PRIVATE_HDR_DIR}/PostProcessing/MakeVerboseFormat.h
    ${PRIVATE_HDR_DIR}/Material/MaterialSystem.h
    ${PRIVATE_HDR_DIR}/Obj/ObjFileData.h
    ${PRIVATE_HDR_DIR}/Obj/ObjFileImporter.h
    ${PRIVATE_HDR_DIR}/Obj/ObjFileMtlImporter.h
    ${PRIVATE_HDR_DIR}/Obj/ObjFileParser.h
    ${PRIVATE_HDR_DIR}/Obj/ObjTools.h
    ${PRIVATE_HDR_DIR}/PostProcessing/OptimizeGraph.h
    ${PRIVATE_HDR_DIR}/PostProcessing/OptimizeMeshes.h
    ${PRIVATE_HDR_DIR}/Common/PolyTools.h
    ${PRIVATE_HDR_DIR}/PostProcessing/PretransformVertices.h
    ${PRIVATE_HDR_DIR}/PostProcessing/ProcessHelper.h
    ${PRIVATE_HDR_DIR}/PostProcessing/RemoveRedundantMaterials.h
    ${PRIVATE_HDR_DIR}/PostProcessing/RemoveVCProcess.h
    ${PRIVATE_HDR_DIR}/PostProcessing/ScaleProcess.h
    ${PRIVATE_HDR_DIR}/Common/ScenePreprocessor.h
    ${PRIVATE_HDR_DIR}/Common/ScenePrivate.h
    ${PRIVATE_HDR_DIR}/PostProcessing/SortByPTypeProcess.h
    ${PRIVATE_HDR_DIR}/Common/SplitByBoneCountProcess.h
    ${PRIVATE_HDR_DIR}/PostProcessing/SplitLargeMeshes.h
    ${PRIVATE_HDR_DIR}/Common/StdOStreamLogStream.h
    ${PRIVATE_HDR_DIR}/Common/TargetAnimation.h
    ${PRIVATE_HDR_DIR}/PostProcessing/TextureTransform.h
    ${PRIVATE_HDR_DIR}/PostProcessing/TriangulateProcess.h
    ${PRIVATE_HDR_DIR}/PostProcessing/ValidateDataStructure.h
    ${PRIVATE_HDR_DIR}/Common/VertexTriangleAdjacency.h
    ${PRIVATE_HDR_DIR}/Common/Win32DebugLogStream.h
    ${PRIVATE_HDR_DIR}/Common/assbin_chunks.h
)

# sources dependencies
set(SRCS
    ${SRC_DIR}/code/Common/Assimp.cpp
    ${SRC_DIR}/code/Common/BaseImporter.cpp
    ${SRC_DIR}/code/Common/BaseProcess.cpp
    ${SRC_DIR}/code/Common/Bitmap.cpp
    ${SRC_DIR}/code/CApi/CInterfaceIOWrapper.cpp
    ${SRC_DIR}/code/PostProcessing/CalcTangentsProcess.cpp
    ${SRC_DIR}/code/PostProcessing/ComputeUVMappingProcess.cpp
    ${SRC_DIR}/code/PostProcessing/ConvertToLHProcess.cpp
    ${SRC_DIR}/code/Common/CreateAnimMesh.cpp
    ${SRC_DIR}/code/PostProcessing/DeboneProcess.cpp
    ${SRC_DIR}/code/Common/DefaultIOStream.cpp
    ${SRC_DIR}/code/Common/DefaultIOSystem.cpp
    ${SRC_DIR}/code/Common/DefaultLogger.cpp
    ${SRC_DIR}/code/PostProcessing/DropFaceNormalsProcess.cpp
    ${SRC_DIR}/code/PostProcessing/EmbedTexturesProcess.cpp
    ${SRC_DIR}/code/Common/Exporter.cpp
    ${SRC_DIR}/code/FBX/FBXAnimation.cpp
    ${SRC_DIR}/code/FBX/FBXBinaryTokenizer.cpp
    ${SRC_DIR}/code/FBX/FBXConverter.cpp
    ${SRC_DIR}/code/FBX/FBXDeformer.cpp
    ${SRC_DIR}/code/FBX/FBXDocument.cpp
    ${SRC_DIR}/code/FBX/FBXDocumentUtil.cpp
    ${SRC_DIR}/code/FBX/FBXImporter.cpp
    ${SRC_DIR}/code/FBX/FBXMaterial.cpp
    ${SRC_DIR}/code/FBX/FBXMeshGeometry.cpp
    ${SRC_DIR}/code/FBX/FBXModel.cpp
    ${SRC_DIR}/code/FBX/FBXNodeAttribute.cpp
    ${SRC_DIR}/code/FBX/FBXParser.cpp
    ${SRC_DIR}/code/FBX/FBXProperties.cpp
    ${SRC_DIR}/code/FBX/FBXTokenizer.cpp
    ${SRC_DIR}/code/FBX/FBXUtil.cpp
    ${SRC_DIR}/code/X3D/FIReader.cpp
    ${SRC_DIR}/code/PostProcessing/FindDegenerates.cpp
    ${SRC_DIR}/code/PostProcessing/FindInstancesProcess.cpp
    ${SRC_DIR}/code/PostProcessing/FindInvalidDataProcess.cpp
    ${SRC_DIR}/code/PostProcessing/FixNormalsStep.cpp
    ${SRC_DIR}/code/PostProcessing/GenFaceNormalsProcess.cpp
    ${SRC_DIR}/code/PostProcessing/GenBoundingBoxesProcess.cpp
    ${SRC_DIR}/code/PostProcessing/GenVertexNormalsProcess.cpp
    ${SRC_DIR}/code/Common/Importer.cpp
    ${SRC_DIR}/code/Common/ImporterRegistry.cpp
    ${SRC_DIR}/code/PostProcessing/ImproveCacheLocality.cpp
    ${SRC_DIR}/code/PostProcessing/JoinVerticesProcess.cpp
    ${SRC_DIR}/code/PostProcessing/LimitBoneWeightsProcess.cpp
    ${SRC_DIR}/code/PostProcessing/MakeVerboseFormat.cpp
    ${SRC_DIR}/code/Material/MaterialSystem.cpp
    ${SRC_DIR}/code/Obj/ObjFileImporter.cpp
    ${SRC_DIR}/code/Obj/ObjFileMtlImporter.cpp
    ${SRC_DIR}/code/Obj/ObjFileParser.cpp
    ${SRC_DIR}/code/PostProcessing/OptimizeGraph.cpp
    ${SRC_DIR}/code/PostProcessing/OptimizeMeshes.cpp
    ${SRC_DIR}/code/Common/PostStepRegistry.cpp
    ${SRC_DIR}/code/PostProcessing/PretransformVertices.cpp
    ${SRC_DIR}/code/PostProcessing/ProcessHelper.cpp
    ${SRC_DIR}/code/Common/RemoveComments.cpp
    ${SRC_DIR}/code/PostProcessing/RemoveRedundantMaterials.cpp
    ${SRC_DIR}/code/PostProcessing/RemoveVCProcess.cpp
    ${SRC_DIR}/code/Common/SGSpatialSort.cpp
    ${SRC_DIR}/code/PostProcessing/ScaleProcess.cpp
    ${SRC_DIR}/code/Common/SceneCombiner.cpp
    ${SRC_DIR}/code/Common/ScenePreprocessor.cpp
    ${SRC_DIR}/code/Common/SkeletonMeshBuilder.cpp
    ${SRC_DIR}/code/PostProcessing/SortByPTypeProcess.cpp
    ${SRC_DIR}/code/Common/SpatialSort.cpp
    ${SRC_DIR}/code/Common/SplitByBoneCountProcess.cpp
    ${SRC_DIR}/code/PostProcessing/SplitLargeMeshes.cpp
    ${SRC_DIR}/code/Common/StandardShapes.cpp
    ${SRC_DIR}/code/Common/Subdivision.cpp
    ${SRC_DIR}/code/Common/TargetAnimation.cpp
    ${SRC_DIR}/code/PostProcessing/TextureTransform.cpp
    ${SRC_DIR}/code/PostProcessing/TriangulateProcess.cpp
    ${SRC_DIR}/code/PostProcessing/ValidateDataStructure.cpp
    ${SRC_DIR}/code/Common/Version.cpp
    ${SRC_DIR}/code/Common/VertexTriangleAdjacency.cpp
    ${SRC_DIR}/code/Common/scene.cpp
    ${SRC_DIR}/contrib/clipper/clipper.cpp
    ${SRC_DIR}/contrib/irrXML/irrXML.cpp
    ${SRC_DIR}/contrib/poly2tri/poly2tri/common/shapes.cc
    ${SRC_DIR}/contrib/poly2tri/poly2tri/sweep/advancing_front.cc
    ${SRC_DIR}/contrib/poly2tri/poly2tri/sweep/cdt.cc
    ${SRC_DIR}/contrib/poly2tri/poly2tri/sweep/sweep_context.cc
    ${SRC_DIR}/contrib/poly2tri/poly2tri/sweep/sweep.cc
    ${SRC_DIR}/contrib/unzip/ioapi.c
    ${SRC_DIR}/contrib/unzip/unzip.c
)

add_definitions(
    -DASSIMP_BUILD_NO_3D_IMPORTER
    -DASSIMP_BUILD_NO_3DS_IMPORTER
    -DASSIMP_BUILD_NO_3MF_IMPORTER
    -DASSIMP_BUILD_NO_AC_IMPORTER
    -DASSIMP_BUILD_NO_AMF_IMPORTER
    -DASSIMP_BUILD_NO_ASSBIN_IMPORTER
    -DASSIMP_BUILD_NO_ASE_IMPORTER
    -DASSIMP_BUILD_NO_B3D_IMPORTER
    -DASSIMP_BUILD_NO_BLEND_IMPORTER
    -DASSIMP_BUILD_NO_BVH_IMPORTER
    -DASSIMP_BUILD_NO_C4D_IMPORTER
    -DASSIMP_BUILD_NO_COB_IMPORTER
    -DASSIMP_BUILD_NO_COLLADA_IMPORTER
    -DASSIMP_BUILD_NO_CSM_IMPORTER
    -DASSIMP_BUILD_NO_DXF_IMPORTER
    -DASSIMP_BUILD_NO_GLTF_IMPORTER
    -DASSIMP_BUILD_NO_GLTF2_IMPORTER
    -DASSIMP_BUILD_NO_HMP_IMPORTER
    -DASSIMP_BUILD_NO_IFC_IMPORTER
    -DASSIMP_BUILD_NO_IRR_IMPORTER
    -DASSIMP_BUILD_NO_IRRMESH_IMPORTER
    -DASSIMP_BUILD_NO_LWO_IMPORTER
    -DASSIMP_BUILD_NO_LWS_IMPORTER
    -DASSIMP_BUILD_NO_M3_IMPORTER
    -DASSIMP_BUILD_NO_MD2_IMPORTER
    -DASSIMP_BUILD_NO_MD3_IMPORTER
    -DASSIMP_BUILD_NO_MD5_IMPORTER
    -DASSIMP_BUILD_NO_MDC_IMPORTER
    -DASSIMP_BUILD_NO_MDL_IMPORTER
    -DASSIMP_BUILD_NO_MMD_IMPORTER
    -DASSIMP_BUILD_NO_MS3D_IMPORTER
    -DASSIMP_BUILD_NO_NDO_IMPORTER
    -DASSIMP_BUILD_NO_NFF_IMPORTER
    -DASSIMP_BUILD_NO_OFF_IMPORTER
    -DASSIMP_BUILD_NO_OGRE_IMPORTER
    -DASSIMP_BUILD_NO_OPENGEX_IMPORTER
    -DASSIMP_BUILD_NO_PLY_IMPORTER
    -DASSIMP_BUILD_NO_Q3BSP_IMPORTER
    -DASSIMP_BUILD_NO_Q3D_IMPORTER
    -DASSIMP_BUILD_NO_RAW_IMPORTER
    -DASSIMP_BUILD_NO_SIB_IMPORTER
    -DASSIMP_BUILD_NO_SMD_IMPORTER
    -DASSIMP_BUILD_NO_STEPFILE_IMPORTER
    -DASSIMP_BUILD_NO_STL_IMPORTER
    -DASSIMP_BUILD_NO_TERRAGEN_IMPORTER
    -DASSIMP_BUILD_NO_X3D_IMPORTER
    -DASSIMP_BUILD_NO_XGL_IMPORTER
    -DASSIMP_BUILD_NO_X_IMPORTER
    -DASSIMP_BUILD_NO_XX_IMPORTER
    -DASSIMP_BUILD_NO_STEP_IMPORTER
    -DASSIMP_BUILD_NO_EXPORTER
)

add_definitions(
    -DASSIMP_BUILD_NO_OWN_ZLIB
)

# specify where our headers are
include_directories(${SRC_DIR}/contrib/irrXML)
include_directories(${SRC_DIR}/contrib/rapidjson/include)
include_directories(${SRC_DIR})
include_directories(${PRIVATE_HDR_DIR})

# we're building a library
add_library(${TARGET} STATIC ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})

if(NOT MSVC)
    target_compile_options(${TARGET}
        PRIVATE -Wno-deprecated-declarations
        PRIVATE -Wno-deprecated-register
        PRIVATE -Wno-incompatible-pointer-types
        PRIVATE -Wno-ordered-compare-function-pointers
        PRIVATE -Wno-parentheses
        PRIVATE -Wno-sign-compare
        PRIVATE -Wno-strict-aliasing
        PRIVATE -Wno-strict-overflow
        PRIVATE -Wno-tautological-compare
        PRIVATE -Wno-tautological-undefined-compare
        PRIVATE -Wno-undefined-var-template
        PRIVATE -Wno-uninitialized
        PRIVATE -Wno-unused-const-variable
        PRIVATE -Wno-unused-private-field
        PRIVATE -Wno-unused-variable
        PRIVATE -Wno-deprecated-non-prototype # See https://github.com/madler/zlib/issues/633
    )
else()
    target_compile_options(${TARGET} PRIVATE /bigobj)
endif()

if (NOT WIN32)
    target_compile_options(${TARGET} PRIVATE -Wno-address-of-packed-member)
elseif(NOT MSVC)
    # Making this option public because the warnings occurs in public headers of libassimp
    target_compile_options(${TARGET} PUBLIC -Wno-pragma-pack)
    # RapidJSON disables warnings in-code that clang-cl doesnt know about
    target_compile_options(${TARGET} PRIVATE -Wno-unknown-warning-option)
endif()

# specify where the public headers of this library are
target_include_directories (${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

# and we're linking against the libraries below, importing their public headers
target_link_libraries(${TARGET} LINK_PUBLIC z)

set_target_properties(${TARGET} PROPERTIES FOLDER ThirdParty)
