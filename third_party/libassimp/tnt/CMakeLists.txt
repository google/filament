cmake_minimum_required(VERSION 3.10)
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
    ${PRIVATE_HDR_DIR}/BaseProcess.h
    ${PRIVATE_HDR_DIR}/BlenderBMesh.h
    ${PRIVATE_HDR_DIR}/BlenderDNA.h
    ${PRIVATE_HDR_DIR}/BlenderIntermediate.h
    ${PRIVATE_HDR_DIR}/BlenderLoader.h
    ${PRIVATE_HDR_DIR}/BlenderModifier.h
    ${PRIVATE_HDR_DIR}/BlenderScene.h
    ${PRIVATE_HDR_DIR}/BlenderSceneGen.h
    ${PRIVATE_HDR_DIR}/BlenderTessellator.h
    ${PRIVATE_HDR_DIR}/CInterfaceIOWrapper.h
    ${PRIVATE_HDR_DIR}/CalcTangentsProcess.h
    ${PRIVATE_HDR_DIR}/ColladaHelper.h
    ${PRIVATE_HDR_DIR}/ColladaLoader.h
    ${PRIVATE_HDR_DIR}/ColladaParser.h
    ${PRIVATE_HDR_DIR}/ComputeUVMappingProcess.h
    ${PRIVATE_HDR_DIR}/ConvertToLHProcess.h
    ${PRIVATE_HDR_DIR}/DeboneProcess.h
    ${PRIVATE_HDR_DIR}/DefaultProgressHandler.h
    ${PRIVATE_HDR_DIR}/DropFaceNormalsProcess.h
    ${PRIVATE_HDR_DIR}/EmbedTexturesProcess.h
    ${PRIVATE_HDR_DIR}/FBXCommon.h
    ${PRIVATE_HDR_DIR}/FBXCompileConfig.h
    ${PRIVATE_HDR_DIR}/FBXConverter.h
    ${PRIVATE_HDR_DIR}/FBXDocument.h
    ${PRIVATE_HDR_DIR}/FBXDocumentUtil.h
    ${PRIVATE_HDR_DIR}/FBXImportSettings.h
    ${PRIVATE_HDR_DIR}/FBXImporter.h
    ${PRIVATE_HDR_DIR}/FBXMeshGeometry.h
    ${PRIVATE_HDR_DIR}/FBXParser.h
    ${PRIVATE_HDR_DIR}/FBXProperties.h
    ${PRIVATE_HDR_DIR}/FBXTokenizer.h
    ${PRIVATE_HDR_DIR}/FBXUtil.h
    ${PRIVATE_HDR_DIR}/FileLogStream.h
    ${PRIVATE_HDR_DIR}/FileSystemFilter.h
    ${PRIVATE_HDR_DIR}/FindDegenerates.h
    ${PRIVATE_HDR_DIR}/FindInstancesProcess.h
    ${PRIVATE_HDR_DIR}/FindInvalidDataProcess.h
    ${PRIVATE_HDR_DIR}/FixNormalsStep.h
    ${PRIVATE_HDR_DIR}/GenFaceNormalsProcess.h
    ${PRIVATE_HDR_DIR}/GenVertexNormalsProcess.h
    ${PRIVATE_HDR_DIR}/IFF.h
    ${PRIVATE_HDR_DIR}/Importer.h
    ${PRIVATE_HDR_DIR}/ImproveCacheLocality.h
    ${PRIVATE_HDR_DIR}/JoinVerticesProcess.h
    ${PRIVATE_HDR_DIR}/LimitBoneWeightsProcess.h
    ${PRIVATE_HDR_DIR}/MakeVerboseFormat.h
    ${PRIVATE_HDR_DIR}/MaterialSystem.h
    ${PRIVATE_HDR_DIR}/ObjFileData.h
    ${PRIVATE_HDR_DIR}/ObjFileImporter.h
    ${PRIVATE_HDR_DIR}/ObjFileMtlImporter.h
    ${PRIVATE_HDR_DIR}/ObjFileParser.h
    ${PRIVATE_HDR_DIR}/ObjTools.h
    ${PRIVATE_HDR_DIR}/OptimizeGraph.h
    ${PRIVATE_HDR_DIR}/OptimizeMeshes.h
    ${PRIVATE_HDR_DIR}/PlyLoader.h
    ${PRIVATE_HDR_DIR}/PlyParser.h
    ${PRIVATE_HDR_DIR}/PolyTools.h
    ${PRIVATE_HDR_DIR}/PretransformVertices.h
    ${PRIVATE_HDR_DIR}/ProcessHelper.h
    ${PRIVATE_HDR_DIR}/RemoveRedundantMaterials.h
    ${PRIVATE_HDR_DIR}/RemoveVCProcess.h
    ${PRIVATE_HDR_DIR}/ScaleProcess.h
    ${PRIVATE_HDR_DIR}/ScenePreprocessor.h
    ${PRIVATE_HDR_DIR}/ScenePrivate.h
    ${PRIVATE_HDR_DIR}/SortByPTypeProcess.h
    ${PRIVATE_HDR_DIR}/SplitByBoneCountProcess.h
    ${PRIVATE_HDR_DIR}/SplitLargeMeshes.h
    ${PRIVATE_HDR_DIR}/StdOStreamLogStream.h
    ${PRIVATE_HDR_DIR}/TargetAnimation.h
    ${PRIVATE_HDR_DIR}/TextureTransform.h
    ${PRIVATE_HDR_DIR}/TriangulateProcess.h
    ${PRIVATE_HDR_DIR}/ValidateDataStructure.h
    ${PRIVATE_HDR_DIR}/VertexTriangleAdjacency.h
    ${PRIVATE_HDR_DIR}/Win32DebugLogStream.h
    ${PRIVATE_HDR_DIR}/assbin_chunks.h
    ${PRIVATE_HDR_DIR}/glTF2Asset.h
    ${PRIVATE_HDR_DIR}/glTF2AssetWriter.h
    ${PRIVATE_HDR_DIR}/glTF2Importer.h
    ${PRIVATE_HDR_DIR}/glTFAsset.h
    ${PRIVATE_HDR_DIR}/glTFAssetWriter.h
    ${PRIVATE_HDR_DIR}/glTFImporter.h
    ${PRIVATE_HDR_DIR}/Importer/IFC/IFCLoader.h
    ${PRIVATE_HDR_DIR}/Importer/IFC/IFCReaderGen_2x3.h
    ${PRIVATE_HDR_DIR}/Importer/IFC/IFCReaderGen_4.h
    ${PRIVATE_HDR_DIR}/Importer/IFC/IFCUtil.h
    ${PRIVATE_HDR_DIR}/Importer/StepFile/StepFileImporter.h
    ${PRIVATE_HDR_DIR}/Importer/StepFile/StepReaderGen.h
    ${PRIVATE_HDR_DIR}/Importer/STEPParser/STEPFileEncoding.h
    ${PRIVATE_HDR_DIR}/Importer/STEPParser/STEPFileReader.h
)

# sources dependencies
set(SRCS
    ${SRC_DIR}/code/Assimp.cpp
    ${SRC_DIR}/code/BaseImporter.cpp
    ${SRC_DIR}/code/BaseProcess.cpp
    ${SRC_DIR}/code/Bitmap.cpp
    ${SRC_DIR}/code/BlenderBMesh.cpp
    ${SRC_DIR}/code/BlenderDNA.cpp
    ${SRC_DIR}/code/BlenderLoader.cpp
    ${SRC_DIR}/code/BlenderModifier.cpp
    ${SRC_DIR}/code/BlenderScene.cpp
    ${SRC_DIR}/code/BlenderTessellator.cpp
    ${SRC_DIR}/code/CInterfaceIOWrapper.cpp
    ${SRC_DIR}/code/CalcTangentsProcess.cpp
    ${SRC_DIR}/code/ColladaLoader.cpp
    ${SRC_DIR}/code/ColladaParser.cpp
    ${SRC_DIR}/code/ComputeUVMappingProcess.cpp
    ${SRC_DIR}/code/ConvertToLHProcess.cpp
    ${SRC_DIR}/code/CreateAnimMesh.cpp
    ${SRC_DIR}/code/DeboneProcess.cpp
    ${SRC_DIR}/code/DefaultIOStream.cpp
    ${SRC_DIR}/code/DefaultIOSystem.cpp
    ${SRC_DIR}/code/DefaultLogger.cpp
    ${SRC_DIR}/code/DropFaceNormalsProcess.cpp
    ${SRC_DIR}/code/EmbedTexturesProcess.cpp
    ${SRC_DIR}/code/Exporter.cpp
    ${SRC_DIR}/code/FBXAnimation.cpp
    ${SRC_DIR}/code/FBXBinaryTokenizer.cpp
    ${SRC_DIR}/code/FBXConverter.cpp
    ${SRC_DIR}/code/FBXDeformer.cpp
    ${SRC_DIR}/code/FBXDocument.cpp
    ${SRC_DIR}/code/FBXDocumentUtil.cpp
    ${SRC_DIR}/code/FBXImporter.cpp
    ${SRC_DIR}/code/FBXMaterial.cpp
    ${SRC_DIR}/code/FBXMeshGeometry.cpp
    ${SRC_DIR}/code/FBXModel.cpp
    ${SRC_DIR}/code/FBXNodeAttribute.cpp
    ${SRC_DIR}/code/FBXParser.cpp
    ${SRC_DIR}/code/FBXProperties.cpp
    ${SRC_DIR}/code/FBXTokenizer.cpp
    ${SRC_DIR}/code/FBXUtil.cpp
    ${SRC_DIR}/code/FIReader.cpp
    ${SRC_DIR}/code/FindDegenerates.cpp
    ${SRC_DIR}/code/FindInstancesProcess.cpp
    ${SRC_DIR}/code/FindInvalidDataProcess.cpp
    ${SRC_DIR}/code/FixNormalsStep.cpp
    ${SRC_DIR}/code/GenFaceNormalsProcess.cpp
    ${SRC_DIR}/code/GenVertexNormalsProcess.cpp
    ${SRC_DIR}/code/Importer.cpp
    ${SRC_DIR}/code/ImporterRegistry.cpp
    ${SRC_DIR}/code/ImproveCacheLocality.cpp
    ${SRC_DIR}/code/JoinVerticesProcess.cpp
    ${SRC_DIR}/code/LimitBoneWeightsProcess.cpp
    ${SRC_DIR}/code/MakeVerboseFormat.cpp
    ${SRC_DIR}/code/MaterialSystem.cpp
    ${SRC_DIR}/code/ObjFileImporter.cpp
    ${SRC_DIR}/code/ObjFileMtlImporter.cpp
    ${SRC_DIR}/code/ObjFileParser.cpp
    ${SRC_DIR}/code/OptimizeGraph.cpp
    ${SRC_DIR}/code/OptimizeMeshes.cpp
    ${SRC_DIR}/code/PlyLoader.cpp
    ${SRC_DIR}/code/PlyParser.cpp
    ${SRC_DIR}/code/PostStepRegistry.cpp
    ${SRC_DIR}/code/PretransformVertices.cpp
    ${SRC_DIR}/code/ProcessHelper.cpp
    ${SRC_DIR}/code/RemoveComments.cpp
    ${SRC_DIR}/code/RemoveRedundantMaterials.cpp
    ${SRC_DIR}/code/RemoveVCProcess.cpp
    ${SRC_DIR}/code/SGSpatialSort.cpp
    ${SRC_DIR}/code/ScaleProcess.cpp
    ${SRC_DIR}/code/SceneCombiner.cpp
    ${SRC_DIR}/code/ScenePreprocessor.cpp
    ${SRC_DIR}/code/SkeletonMeshBuilder.cpp
    ${SRC_DIR}/code/SortByPTypeProcess.cpp
    ${SRC_DIR}/code/SpatialSort.cpp
    ${SRC_DIR}/code/SplitByBoneCountProcess.cpp
    ${SRC_DIR}/code/SplitLargeMeshes.cpp
    ${SRC_DIR}/code/StandardShapes.cpp
    ${SRC_DIR}/code/Subdivision.cpp
    ${SRC_DIR}/code/TargetAnimation.cpp
    ${SRC_DIR}/code/TextureTransform.cpp
    ${SRC_DIR}/code/TriangulateProcess.cpp
    ${SRC_DIR}/code/ValidateDataStructure.cpp
    ${SRC_DIR}/code/Version.cpp
    ${SRC_DIR}/code/VertexTriangleAdjacency.cpp
    ${SRC_DIR}/code/glTF2Importer.cpp
    ${SRC_DIR}/code/glTFImporter.cpp
    ${SRC_DIR}/code/scene.cpp
    ${SRC_DIR}/code/Importer/IFC/IFCBoolean.cpp
    ${SRC_DIR}/code/Importer/IFC/IFCCurve.cpp
    ${SRC_DIR}/code/Importer/IFC/IFCGeometry.cpp
    ${SRC_DIR}/code/Importer/IFC/IFCLoader.cpp
    ${SRC_DIR}/code/Importer/IFC/IFCMaterial.cpp
    ${SRC_DIR}/code/Importer/IFC/IFCOpenings.cpp
    ${SRC_DIR}/code/Importer/IFC/IFCProfile.cpp
    ${SRC_DIR}/code/Importer/IFC/IFCReaderGen1_2x3.cpp
    ${SRC_DIR}/code/Importer/IFC/IFCReaderGen2_2x3.cpp
    ${SRC_DIR}/code/Importer/IFC/IFCReaderGen_4.cpp
    ${SRC_DIR}/code/Importer/IFC/IFCUtil.cpp
    ${SRC_DIR}/code/Importer/StepFile/StepFileImporter.cpp
    ${SRC_DIR}/code/Importer/StepFile/StepFileGen1.cpp
    ${SRC_DIR}/code/Importer/StepFile/StepFileGen2.cpp
    ${SRC_DIR}/code/Importer/StepFile/StepFileGen3.cpp
    ${SRC_DIR}/code/Importer/STEPParser/STEPFileEncoding.cpp
    ${SRC_DIR}/code/Importer/STEPParser/STEPFileReader.cpp
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
    -DASSIMP_BUILD_NO_CSM_IMPORTER
    -DASSIMP_BUILD_NO_DXF_IMPORTER
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
    -DASSIMP_BUILD_NO_Q3BSP_IMPORTER
    -DASSIMP_BUILD_NO_Q3D_IMPORTER
    -DASSIMP_BUILD_NO_RAW_IMPORTER
    -DASSIMP_BUILD_NO_SIB_IMPORTER
    -DASSIMP_BUILD_NO_SMD_IMPORTER
    -DASSIMP_BUILD_NO_STL_IMPORTER
    -DASSIMP_BUILD_NO_TERRAGEN_IMPORTER
    -DASSIMP_BUILD_NO_X3D_IMPORTER
    -DASSIMP_BUILD_NO_XGL_IMPORTER
    -DASSIMP_BUILD_NO_X_IMPORTER
    -DASSIMP_BUILD_NO_XX_IMPORTER
    -DASSIMP_BUILD_NO_EXPORTER
)

add_definitions(
    -DASSIMP_BUILD_NO_OWN_ZLIB
)

# specify where our headers are
include_directories(${SRC_DIR}/contrib/irrXML)
include_directories(${SRC_DIR}/contrib/rapidjson/include)
include_directories(${SRC_DIR})
include_directories(${SRC_DIR}/code/BoostWorkaround)

# we're building a library
add_library(${TARGET} STATIC ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})

if(NOT MSVC)
    target_compile_options(${TARGET}
        PRIVATE -Wno-strict-aliasing
        PRIVATE -Wno-sign-compare
        PRIVATE -Wno-unused-const-variable
        PRIVATE -Wno-unused-private-field
        PRIVATE -Wno-unused-variable
        PRIVATE -Wno-deprecated-declarations
        PRIVATE -Wno-parentheses
        PRIVATE -Wno-uninitialized
        PRIVATE -Wno-strict-overflow
        PRIVATE -Wno-deprecated-register
        PRIVATE -Wno-tautological-undefined-compare
        PRIVATE -Wno-incompatible-pointer-types
        PRIVATE -Wno-tautological-compare
        PRIVATE -Wno-undefined-var-template
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
