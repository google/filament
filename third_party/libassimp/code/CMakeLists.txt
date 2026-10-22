# Open Asset Import Library (assimp)
# ----------------------------------------------------------------------
#
# Copyright (c) 2006-2019, assimp team
#
# All rights reserved.
#
# Redistribution and use of this software in source and binary forms,
# with or without modification, are permitted provided that the
# following conditions are met:
#
# * Redistributions of source code must retain the above
#   copyright notice, this list of conditions and the
#   following disclaimer.
#
# * Redistributions in binary form must reproduce the above
#   copyright notice, this list of conditions and the
#   following disclaimer in the documentation and/or other
#   materials provided with the distribution.
#
# * Neither the name of the assimp team, nor the names of its
#   contributors may be used to endorse or promote products
#   derived from this software without specific prior
#   written permission of the assimp team.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#----------------------------------------------------------------------

# Listing and grouping of all the source files.
# 1) Set the file lists for each component
# 2) Create a Source Group for each component, for IDE project orginization
# 3) Add libassimp using the file lists (eliminates duplication of file names between
#    source groups and library command)
#
cmake_minimum_required( VERSION 3.0 )
SET( HEADER_PATH ../include/assimp )

if(NOT ANDROID AND ASSIMP_ANDROID_JNIIOSYSTEM)
    message(WARNING "Requesting Android JNI I/O-System in non-Android toolchain. Resetting ASSIMP_ANDROID_JNIIOSYSTEM to OFF.")
    set(ASSIMP_ANDROID_JNIIOSYSTEM OFF)
endif(NOT ANDROID AND ASSIMP_ANDROID_JNIIOSYSTEM)

SET( COMPILER_HEADERS
  ${HEADER_PATH}/Compiler/pushpack1.h
  ${HEADER_PATH}/Compiler/poppack1.h
  ${HEADER_PATH}/Compiler/pstdint.h
)
SOURCE_GROUP( Compiler FILES ${COMPILER_HEADERS})

SET( PUBLIC_HEADERS
  ${HEADER_PATH}/anim.h
  ${HEADER_PATH}/aabb.h
  ${HEADER_PATH}/ai_assert.h
  ${HEADER_PATH}/camera.h
  ${HEADER_PATH}/color4.h
  ${HEADER_PATH}/color4.inl
  ${CMAKE_CURRENT_BINARY_DIR}/../include/assimp/config.h
  ${HEADER_PATH}/defs.h
  ${HEADER_PATH}/Defines.h
  ${HEADER_PATH}/cfileio.h
  ${HEADER_PATH}/light.h
  ${HEADER_PATH}/material.h
  ${HEADER_PATH}/material.inl
  ${HEADER_PATH}/matrix3x3.h
  ${HEADER_PATH}/matrix3x3.inl
  ${HEADER_PATH}/matrix4x4.h
  ${HEADER_PATH}/matrix4x4.inl
  ${HEADER_PATH}/mesh.h
  ${HEADER_PATH}/pbrmaterial.h
  ${HEADER_PATH}/postprocess.h
  ${HEADER_PATH}/quaternion.h
  ${HEADER_PATH}/quaternion.inl
  ${HEADER_PATH}/scene.h
  ${HEADER_PATH}/metadata.h
  ${HEADER_PATH}/texture.h
  ${HEADER_PATH}/types.h
  ${HEADER_PATH}/vector2.h
  ${HEADER_PATH}/vector2.inl
  ${HEADER_PATH}/vector3.h
  ${HEADER_PATH}/vector3.inl
  ${HEADER_PATH}/version.h
  ${HEADER_PATH}/cimport.h
  ${HEADER_PATH}/importerdesc.h
  ${HEADER_PATH}/Importer.hpp
  ${HEADER_PATH}/DefaultLogger.hpp
  ${HEADER_PATH}/ProgressHandler.hpp
  ${HEADER_PATH}/IOStream.hpp
  ${HEADER_PATH}/IOSystem.hpp
  ${HEADER_PATH}/Logger.hpp
  ${HEADER_PATH}/LogStream.hpp
  ${HEADER_PATH}/NullLogger.hpp
  ${HEADER_PATH}/cexport.h
  ${HEADER_PATH}/Exporter.hpp
  ${HEADER_PATH}/DefaultIOStream.h
  ${HEADER_PATH}/DefaultIOSystem.h
  ${HEADER_PATH}/ZipArchiveIOSystem.h
  ${HEADER_PATH}/SceneCombiner.h
  ${HEADER_PATH}/fast_atof.h
  ${HEADER_PATH}/qnan.h
  ${HEADER_PATH}/BaseImporter.h
  ${HEADER_PATH}/Hash.h
  ${HEADER_PATH}/MemoryIOWrapper.h
  ${HEADER_PATH}/ParsingUtils.h
  ${HEADER_PATH}/StreamReader.h
  ${HEADER_PATH}/StreamWriter.h
  ${HEADER_PATH}/StringComparison.h
  ${HEADER_PATH}/StringUtils.h
  ${HEADER_PATH}/SGSpatialSort.h
  ${HEADER_PATH}/GenericProperty.h
  ${HEADER_PATH}/SpatialSort.h
  ${HEADER_PATH}/SkeletonMeshBuilder.h
  ${HEADER_PATH}/SmoothingGroups.h
  ${HEADER_PATH}/SmoothingGroups.inl
  ${HEADER_PATH}/StandardShapes.h
  ${HEADER_PATH}/RemoveComments.h
  ${HEADER_PATH}/Subdivision.h
  ${HEADER_PATH}/Vertex.h
  ${HEADER_PATH}/LineSplitter.h
  ${HEADER_PATH}/TinyFormatter.h
  ${HEADER_PATH}/Profiler.h
  ${HEADER_PATH}/LogAux.h
  ${HEADER_PATH}/Bitmap.h
  ${HEADER_PATH}/XMLTools.h
  ${HEADER_PATH}/IOStreamBuffer.h
  ${HEADER_PATH}/CreateAnimMesh.h
  ${HEADER_PATH}/irrXMLWrapper.h
  ${HEADER_PATH}/BlobIOSystem.h
  ${HEADER_PATH}/MathFunctions.h
  ${HEADER_PATH}/Macros.h
  ${HEADER_PATH}/Exceptional.h
  ${HEADER_PATH}/ByteSwapper.h
)

SET( Core_SRCS
  Common/Assimp.cpp
)

IF(MSVC)
  list(APPEND Core_SRCS "res/assimp.rc")
ENDIF(MSVC)

SET( Logging_SRCS
  ${HEADER_PATH}/DefaultLogger.hpp
  ${HEADER_PATH}/LogStream.hpp
  ${HEADER_PATH}/Logger.hpp
  ${HEADER_PATH}/NullLogger.hpp
  Common/Win32DebugLogStream.h
  Common/DefaultLogger.cpp
  Common/FileLogStream.h
  Common/StdOStreamLogStream.h
)
SOURCE_GROUP(Logging FILES ${Logging_SRCS})

SET( Common_SRCS
  Common/BaseImporter.cpp
  Common/BaseProcess.cpp
  Common/BaseProcess.h
  Common/Importer.h
  Common/ScenePrivate.h
  Common/PostStepRegistry.cpp
  Common/ImporterRegistry.cpp
  Common/DefaultProgressHandler.h
  Common/DefaultIOStream.cpp
  Common/DefaultIOSystem.cpp
  Common/ZipArchiveIOSystem.cpp
  Common/PolyTools.h
  Common/Importer.cpp
  Common/IFF.h
  Common/SGSpatialSort.cpp
  Common/VertexTriangleAdjacency.cpp
  Common/VertexTriangleAdjacency.h
  Common/SpatialSort.cpp
  Common/SceneCombiner.cpp
  Common/ScenePreprocessor.cpp
  Common/ScenePreprocessor.h
  Common/SkeletonMeshBuilder.cpp
  Common/SplitByBoneCountProcess.cpp
  Common/SplitByBoneCountProcess.h
  Common/StandardShapes.cpp
  Common/TargetAnimation.cpp
  Common/TargetAnimation.h
  Common/RemoveComments.cpp
  Common/Subdivision.cpp
  Common/scene.cpp
  Common/Bitmap.cpp
  Common/Version.cpp
  Common/CreateAnimMesh.cpp
  Common/simd.h
  Common/simd.cpp
)
SOURCE_GROUP(Common FILES ${Common_SRCS})

SET( CApi_SRCS
  CApi/CInterfaceIOWrapper.cpp
  CApi/CInterfaceIOWrapper.h
)
SOURCE_GROUP(CApi FILES ${CApi_SRCS})

SET( STEPParser_SRCS
  Importer/STEPParser/STEPFileReader.h
  Importer/STEPParser/STEPFileReader.cpp
  Importer/STEPParser/STEPFileEncoding.cpp
  Importer/STEPParser/STEPFileEncoding.h
)
SOURCE_GROUP(STEPParser FILES ${STEPParser_SRCS})

IF ( ASSIMP_BUILD_NONFREE_C4D_IMPORTER )
  SET( C4D_SRCS
    C4D/C4DImporter.cpp
    C4D/C4DImporter.h
  )
  SOURCE_GROUP( C4D FILES ${C4D_SRCS})
ENDIF ( ASSIMP_BUILD_NONFREE_C4D_IMPORTER )

# if this variable is set to TRUE, the user can manually disable importers by setting
# ASSIMP_BUILD_XXX_IMPORTER to FALSE for each importer
# if this variable is set to FALSE, the user can manually enable importers by setting
# ASSIMP_BUILD_XXX_IMPORTER to TRUE for each importer
OPTION(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT "default value of all ASSIMP_BUILD_XXX_IMPORTER values" TRUE)

# macro to add the CMake Option ADD_ASSIMP_IMPORTER_<name> which enables compile of loader
# this way selective loaders can be compiled (reduces filesize + compile time)
MACRO(ADD_ASSIMP_IMPORTER name)
  IF (ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT)
    set(ASSIMP_IMPORTER_ENABLED TRUE)
    IF (DEFINED ASSIMP_BUILD_${name}_IMPORTER AND NOT ASSIMP_BUILD_${name}_IMPORTER)
      set(ASSIMP_IMPORTER_ENABLED FALSE)
    ENDIF ()
  ELSE ()
    set(ASSIMP_IMPORTER_ENABLED ${ASSIMP_BUILD_${name}_IMPORTER})
  ENDIF ()
  IF (ASSIMP_IMPORTER_ENABLED)
    LIST(APPEND ASSIMP_LOADER_SRCS ${ARGN})
    SET(ASSIMP_IMPORTERS_ENABLED "${ASSIMP_IMPORTERS_ENABLED} ${name}")
    SOURCE_GROUP(${name} FILES ${ARGN})
  ELSE()
    SET(${name}_SRC "")
    SET(ASSIMP_IMPORTERS_DISABLED "${ASSIMP_IMPORTERS_DISABLED} ${name}")
    add_definitions(-DASSIMP_BUILD_NO_${name}_IMPORTER)
  ENDIF()
ENDMACRO()

# if this variable is set to TRUE, the user can manually disable exporters by setting
# ASSIMP_BUILD_XXX_EXPORTER to FALSE for each exporter
# if this variable is set to FALSE, the user can manually enable exporters by setting
# ASSIMP_BUILD_XXX_EXPORTER to TRUE for each exporter
OPTION(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT "default value of all ASSIMP_BUILD_XXX_EXPORTER values" TRUE)

# macro to add the CMake Option ADD_ASSIMP_IMPORTER_<name> which enables compile of loader
# this way selective loaders can be compiled (reduces filesize + compile time)
MACRO(ADD_ASSIMP_EXPORTER name)
  IF (ASSIMP_NO_EXPORT)
	set(ASSIMP_EXPORTER_ENABLED FALSE)
  ELSEIF (ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT)
    set(ASSIMP_EXPORTER_ENABLED TRUE)
    IF (DEFINED ASSIMP_BUILD_${name}_EXPORTER AND NOT ASSIMP_BUILD_${name}_EXPORTER)
      set(ASSIMP_EXPORTER_ENABLED FALSE)
    ENDIF ()
  ELSE ()
    set(ASSIMP_EXPORTER_ENABLED ${ASSIMP_BUILD_${name}_EXPORTER})
  ENDIF ()

  IF (ASSIMP_EXPORTER_ENABLED)
    SET(ASSIMP_EXPORTERS_ENABLED "${ASSIMP_EXPORTERS_ENABLED} ${name}")
	LIST(APPEND ASSIMP_EXPORTER_SRCS ${ARGN})
    SOURCE_GROUP(${name}_EXPORTER FILES ${ARGN})
  ELSE()
    SET(ASSIMP_EXPORTERS_DISABLED "${ASSIMP_EXPORTERS_DISABLED} ${name}")
    add_definitions(-DASSIMP_BUILD_NO_${name}_EXPORTER)
  ENDIF()
ENDMACRO()

SET(ASSIMP_LOADER_SRCS "")
SET(ASSIMP_IMPORTERS_ENABLED "") # list of enabled importers
SET(ASSIMP_IMPORTERS_DISABLED "") # disabled importers list (used to print)

SET(ASSIMP_EXPORTER_SRCS "")
SET(ASSIMP_EXPORTERS_ENABLED "") # list of enabled exporters
SET(ASSIMP_EXPORTERS_DISABLED "") # disabled exporters list (used to print)

ADD_ASSIMP_IMPORTER( AMF
  AMF/AMFImporter.hpp
  AMF/AMFImporter_Macro.hpp
  AMF/AMFImporter_Node.hpp
  AMF/AMFImporter.cpp
  AMF/AMFImporter_Geometry.cpp
  AMF/AMFImporter_Material.cpp
  AMF/AMFImporter_Postprocess.cpp
)

ADD_ASSIMP_IMPORTER( 3DS
  3DS/3DSConverter.cpp
  3DS/3DSHelper.h
  3DS/3DSLoader.cpp
  3DS/3DSLoader.h
)

ADD_ASSIMP_EXPORTER( 3DS
  3DS/3DSExporter.h
  3DS/3DSExporter.cpp
)

ADD_ASSIMP_IMPORTER( AC
  AC/ACLoader.cpp
  AC/ACLoader.h
)

ADD_ASSIMP_IMPORTER( ASE
  ASE/ASELoader.cpp
  ASE/ASELoader.h
  ASE/ASEParser.cpp
  ASE/ASEParser.h
)

ADD_ASSIMP_IMPORTER( ASSBIN
  Assbin/AssbinLoader.h
  Assbin/AssbinLoader.cpp
)

ADD_ASSIMP_EXPORTER( ASSBIN
  Assbin/AssbinExporter.h
  Assbin/AssbinExporter.cpp
)

ADD_ASSIMP_EXPORTER( ASSXML
  Assxml/AssxmlExporter.h
  Assxml/AssxmlExporter.cpp
)

ADD_ASSIMP_IMPORTER( B3D
  B3D/B3DImporter.cpp
  B3D/B3DImporter.h
)

ADD_ASSIMP_IMPORTER( BVH
  BVH/BVHLoader.cpp
  BVH/BVHLoader.h
)

ADD_ASSIMP_IMPORTER( COLLADA
  Collada/ColladaHelper.h
  Collada/ColladaLoader.cpp
  Collada/ColladaLoader.h
  Collada/ColladaParser.cpp
  Collada/ColladaParser.h
)

ADD_ASSIMP_EXPORTER( COLLADA
  Collada/ColladaExporter.h
  Collada/ColladaExporter.cpp
)

ADD_ASSIMP_IMPORTER( DXF
  DXF/DXFLoader.cpp
  DXF/DXFLoader.h
  DXF/DXFHelper.h
)

ADD_ASSIMP_IMPORTER( CSM
  CSM/CSMLoader.cpp
  CSM/CSMLoader.h
)

ADD_ASSIMP_IMPORTER( HMP
  HMP/HMPFileData.h
  HMP/HMPLoader.cpp
  HMP/HMPLoader.h
  HMP/HalfLifeFileData.h
)

ADD_ASSIMP_IMPORTER( IRRMESH
  Irr/IRRMeshLoader.cpp
  Irr/IRRMeshLoader.h
  Irr/IRRShared.cpp
  Irr/IRRShared.h
)

ADD_ASSIMP_IMPORTER( IRR
  Irr/IRRLoader.cpp
  Irr/IRRLoader.h
  Irr/IRRShared.cpp
  Irr/IRRShared.h
)

ADD_ASSIMP_IMPORTER( LWO
  LWO/LWOAnimation.cpp
  LWO/LWOAnimation.h
  LWO/LWOBLoader.cpp
  LWO/LWOFileData.h
  LWO/LWOLoader.cpp
  LWO/LWOLoader.h
  LWO/LWOMaterial.cpp
)

ADD_ASSIMP_IMPORTER( LWS
  LWS/LWSLoader.cpp
  LWS/LWSLoader.h
)

ADD_ASSIMP_IMPORTER( MD2
  MD2/MD2FileData.h
  MD2/MD2Loader.cpp
  MD2/MD2Loader.h
  MD2/MD2NormalTable.h
)

ADD_ASSIMP_IMPORTER( MD3
  MD3/MD3FileData.h
  MD3/MD3Loader.cpp
  MD3/MD3Loader.h
)

ADD_ASSIMP_IMPORTER( MD5
  MD5/MD5Loader.cpp
  MD5/MD5Loader.h
  MD5/MD5Parser.cpp
  MD5/MD5Parser.h
)

ADD_ASSIMP_IMPORTER( MDC
  MDC/MDCFileData.h
  MDC/MDCLoader.cpp
  MDC/MDCLoader.h
  MDC/MDCNormalTable.h
)

ADD_ASSIMP_IMPORTER( MDL
  MDL/MDLDefaultColorMap.h
  MDL/MDLFileData.h
  MDL/MDLLoader.cpp
  MDL/MDLLoader.h
  MDL/MDLMaterialLoader.cpp
)

SET( MaterialSystem_SRCS
  Material/MaterialSystem.cpp
  Material/MaterialSystem.h
)
SOURCE_GROUP( MaterialSystem FILES ${MaterialSystem_SRCS})

ADD_ASSIMP_IMPORTER( NFF
  NFF/NFFLoader.cpp
  NFF/NFFLoader.h
)

ADD_ASSIMP_IMPORTER( NDO
  NDO/NDOLoader.cpp
  NDO/NDOLoader.h
)

ADD_ASSIMP_IMPORTER( OFF
  OFF/OFFLoader.cpp
  OFF/OFFLoader.h
)

ADD_ASSIMP_IMPORTER( OBJ
  Obj/ObjFileData.h
  Obj/ObjFileImporter.cpp
  Obj/ObjFileImporter.h
  Obj/ObjFileMtlImporter.cpp
  Obj/ObjFileMtlImporter.h
  Obj/ObjFileParser.cpp
  Obj/ObjFileParser.h
  Obj/ObjTools.h
)

ADD_ASSIMP_EXPORTER( OBJ
  Obj/ObjExporter.h
  Obj/ObjExporter.cpp
)

ADD_ASSIMP_IMPORTER( OGRE
  Ogre/OgreImporter.h
  Ogre/OgreStructs.h
  Ogre/OgreParsingUtils.h
  Ogre/OgreBinarySerializer.h
  Ogre/OgreXmlSerializer.h
  Ogre/OgreImporter.cpp
  Ogre/OgreStructs.cpp
  Ogre/OgreBinarySerializer.cpp
  Ogre/OgreXmlSerializer.cpp
  Ogre/OgreMaterial.cpp
)

ADD_ASSIMP_IMPORTER( OPENGEX
  OpenGEX/OpenGEXImporter.cpp
  OpenGEX/OpenGEXImporter.h
  OpenGEX/OpenGEXStructs.h
)

ADD_ASSIMP_EXPORTER( OPENGEX
  OpenGEX/OpenGEXExporter.cpp
  OpenGEX/OpenGEXExporter.h
)

ADD_ASSIMP_IMPORTER( PLY
  Ply/PlyLoader.cpp
  Ply/PlyLoader.h
  Ply/PlyParser.cpp
  Ply/PlyParser.h
)

ADD_ASSIMP_EXPORTER( PLY
  Ply/PlyExporter.cpp
  Ply/PlyExporter.h
)

ADD_ASSIMP_IMPORTER( MS3D
  MS3D/MS3DLoader.cpp
  MS3D/MS3DLoader.h
)

ADD_ASSIMP_IMPORTER( COB
  COB/COBLoader.cpp
  COB/COBLoader.h
  COB/COBScene.h
)

ADD_ASSIMP_IMPORTER( BLEND
  Blender/BlenderLoader.cpp
  Blender/BlenderLoader.h
  Blender/BlenderDNA.cpp
  Blender/BlenderDNA.h
  Blender/BlenderDNA.inl
  Blender/BlenderScene.cpp
  Blender/BlenderScene.h
  Blender/BlenderSceneGen.h
  Blender/BlenderIntermediate.h
  Blender/BlenderModifier.h
  Blender/BlenderModifier.cpp
  Blender/BlenderBMesh.h
  Blender/BlenderBMesh.cpp
  Blender/BlenderTessellator.h
  Blender/BlenderTessellator.cpp
  Blender/BlenderCustomData.h
  Blender/BlenderCustomData.cpp
)

ADD_ASSIMP_IMPORTER( IFC
  Importer/IFC/IFCLoader.cpp
  Importer/IFC/IFCLoader.h
  Importer/IFC/IFCReaderGen1_2x3.cpp
  Importer/IFC/IFCReaderGen2_2x3.cpp
  Importer/IFC/IFCReaderGen_2x3.h
  Importer/IFC/IFCUtil.h
  Importer/IFC/IFCUtil.cpp
  Importer/IFC/IFCGeometry.cpp
  Importer/IFC/IFCMaterial.cpp
  Importer/IFC/IFCProfile.cpp
  Importer/IFC/IFCCurve.cpp
  Importer/IFC/IFCBoolean.cpp
  Importer/IFC/IFCOpenings.cpp
)

if (ASSIMP_BUILD_IFC_IMPORTER)
  if (MSVC)
    set_source_files_properties(Importer/IFC/IFCReaderGen1_2x3.cpp Importer/IFC/IFCReaderGen2_2x3.cpp PROPERTIES COMPILE_FLAGS "/bigobj")
  elseif(CMAKE_COMPILER_IS_MINGW)
    set_source_files_properties(Importer/IFC/IFCReaderGen1_2x3.cpp Importer/IFC/IFCReaderGen2_2x3.cpp PROPERTIES COMPILE_FLAGS "-O2 -Wa,-mbig-obj")
  endif()
endif (ASSIMP_BUILD_IFC_IMPORTER)

ADD_ASSIMP_IMPORTER( XGL
  XGL/XGLLoader.cpp
  XGL/XGLLoader.h
)

ADD_ASSIMP_IMPORTER( FBX
  FBX/FBXImporter.cpp
  FBX/FBXCompileConfig.h
  FBX/FBXImporter.h
  FBX/FBXParser.cpp
  FBX/FBXParser.h
  FBX/FBXTokenizer.cpp
  FBX/FBXTokenizer.h
  FBX/FBXImportSettings.h
  FBX/FBXConverter.h
  FBX/FBXConverter.cpp
  FBX/FBXUtil.h
  FBX/FBXUtil.cpp
  FBX/FBXDocument.h
  FBX/FBXDocument.cpp
  FBX/FBXProperties.h
  FBX/FBXProperties.cpp
  FBX/FBXMeshGeometry.h
  FBX/FBXMeshGeometry.cpp
  FBX/FBXMaterial.cpp
  FBX/FBXModel.cpp
  FBX/FBXAnimation.cpp
  FBX/FBXNodeAttribute.cpp
  FBX/FBXDeformer.cpp
  FBX/FBXBinaryTokenizer.cpp
  FBX/FBXDocumentUtil.cpp
  FBX/FBXCommon.h
)

ADD_ASSIMP_EXPORTER( FBX
  FBX/FBXExporter.h
  FBX/FBXExporter.cpp
  FBX/FBXExportNode.h
  FBX/FBXExportNode.cpp
  FBX/FBXExportProperty.h
  FBX/FBXExportProperty.cpp
)

SET( PostProcessing_SRCS
  PostProcessing/CalcTangentsProcess.cpp
  PostProcessing/CalcTangentsProcess.h
  PostProcessing/ComputeUVMappingProcess.cpp
  PostProcessing/ComputeUVMappingProcess.h
  PostProcessing/ConvertToLHProcess.cpp
  PostProcessing/ConvertToLHProcess.h
  PostProcessing/EmbedTexturesProcess.cpp
  PostProcessing/EmbedTexturesProcess.h
  PostProcessing/FindDegenerates.cpp
  PostProcessing/FindDegenerates.h
  PostProcessing/FindInstancesProcess.cpp
  PostProcessing/FindInstancesProcess.h
  PostProcessing/FindInvalidDataProcess.cpp
  PostProcessing/FindInvalidDataProcess.h
  PostProcessing/FixNormalsStep.cpp
  PostProcessing/FixNormalsStep.h
  PostProcessing/DropFaceNormalsProcess.cpp
  PostProcessing/DropFaceNormalsProcess.h
  PostProcessing/GenFaceNormalsProcess.cpp
  PostProcessing/GenFaceNormalsProcess.h
  PostProcessing/GenVertexNormalsProcess.cpp
  PostProcessing/GenVertexNormalsProcess.h
  PostProcessing/PretransformVertices.cpp
  PostProcessing/PretransformVertices.h
  PostProcessing/ImproveCacheLocality.cpp
  PostProcessing/ImproveCacheLocality.h
  PostProcessing/JoinVerticesProcess.cpp
  PostProcessing/JoinVerticesProcess.h
  PostProcessing/LimitBoneWeightsProcess.cpp
  PostProcessing/LimitBoneWeightsProcess.h
  PostProcessing/RemoveRedundantMaterials.cpp
  PostProcessing/RemoveRedundantMaterials.h
  PostProcessing/RemoveVCProcess.cpp
  PostProcessing/RemoveVCProcess.h
  PostProcessing/SortByPTypeProcess.cpp
  PostProcessing/SortByPTypeProcess.h
  PostProcessing/SplitLargeMeshes.cpp
  PostProcessing/SplitLargeMeshes.h
  PostProcessing/TextureTransform.cpp
  PostProcessing/TextureTransform.h
  PostProcessing/TriangulateProcess.cpp
  PostProcessing/TriangulateProcess.h
  PostProcessing/ValidateDataStructure.cpp
  PostProcessing/ValidateDataStructure.h
  PostProcessing/OptimizeGraph.cpp
  PostProcessing/OptimizeGraph.h
  PostProcessing/OptimizeMeshes.cpp
  PostProcessing/OptimizeMeshes.h
  PostProcessing/DeboneProcess.cpp
  PostProcessing/DeboneProcess.h
  PostProcessing/ProcessHelper.h
  PostProcessing/ProcessHelper.cpp
  PostProcessing/MakeVerboseFormat.cpp
  PostProcessing/MakeVerboseFormat.h
  PostProcessing/ScaleProcess.cpp
  PostProcessing/ScaleProcess.h
  PostProcessing/GenBoundingBoxesProcess.cpp
  PostProcessing/GenBoundingBoxesProcess.h
)
SOURCE_GROUP( PostProcessing FILES ${PostProcessing_SRCS})

SET( IrrXML_SRCS ${HEADER_PATH}/irrXMLWrapper.h )
SOURCE_GROUP( IrrXML FILES ${IrrXML_SRCS})

ADD_ASSIMP_IMPORTER( Q3D
  Q3D/Q3DLoader.cpp
  Q3D/Q3DLoader.h
)

ADD_ASSIMP_IMPORTER( Q3BSP
  Q3BSP/Q3BSPFileData.h
  Q3BSP/Q3BSPFileParser.h
  Q3BSP/Q3BSPFileParser.cpp
  Q3BSP/Q3BSPFileImporter.h
  Q3BSP/Q3BSPFileImporter.cpp
)

ADD_ASSIMP_IMPORTER( RAW
  Raw/RawLoader.cpp
  Raw/RawLoader.h
)

ADD_ASSIMP_IMPORTER( SIB
  SIB/SIBImporter.cpp
  SIB/SIBImporter.h
)

ADD_ASSIMP_IMPORTER( SMD
  SMD/SMDLoader.cpp
  SMD/SMDLoader.h
)

ADD_ASSIMP_IMPORTER( STL
  STL/STLLoader.cpp
  STL/STLLoader.h
)

ADD_ASSIMP_EXPORTER( STL
  STL/STLExporter.h
  STL/STLExporter.cpp
)

ADD_ASSIMP_IMPORTER( TERRAGEN
  Terragen/TerragenLoader.cpp
  Terragen/TerragenLoader.h
)

ADD_ASSIMP_IMPORTER( 3D
  Unreal/UnrealLoader.cpp
  Unreal/UnrealLoader.h
)

ADD_ASSIMP_IMPORTER( X
  X/XFileHelper.h
  X/XFileImporter.cpp
  X/XFileImporter.h
  X/XFileParser.cpp
  X/XFileParser.h
)

ADD_ASSIMP_EXPORTER( X
  X/XFileExporter.h
  X/XFileExporter.cpp
)

ADD_ASSIMP_IMPORTER( X3D
  X3D/X3DImporter.cpp
  X3D/X3DImporter.hpp
  X3D/X3DImporter_Geometry2D.cpp
  X3D/X3DImporter_Geometry3D.cpp
  X3D/X3DImporter_Group.cpp
  X3D/X3DImporter_Light.cpp
  X3D/X3DImporter_Macro.hpp
  X3D/X3DImporter_Metadata.cpp
  X3D/X3DImporter_Networking.cpp
  X3D/X3DImporter_Node.hpp
  X3D/X3DImporter_Postprocess.cpp
  X3D/X3DImporter_Rendering.cpp
  X3D/X3DImporter_Shape.cpp
  X3D/X3DImporter_Texturing.cpp
  X3D/FIReader.hpp
  X3D/FIReader.cpp
  X3D/X3DVocabulary.cpp
)

ADD_ASSIMP_EXPORTER( X3D
  X3D/X3DExporter.cpp
  X3D/X3DExporter.hpp
)

ADD_ASSIMP_IMPORTER( GLTF
  glTF/glTFCommon.h
  glTF/glTFCommon.cpp
  glTF/glTFAsset.h
  glTF/glTFAsset.inl
  glTF/glTFAssetWriter.h
  glTF/glTFAssetWriter.inl
  glTF/glTFImporter.cpp
  glTF/glTFImporter.h
  glTF2/glTF2Asset.h
  glTF2/glTF2Asset.inl
  glTF2/glTF2AssetWriter.h
  glTF2/glTF2AssetWriter.inl
  glTF2/glTF2Importer.cpp
  glTF2/glTF2Importer.h
)

ADD_ASSIMP_EXPORTER( GLTF
  glTF/glTFExporter.h
  glTF/glTFExporter.cpp
  glTF2/glTF2Exporter.h
  glTF2/glTF2Exporter.cpp
)

ADD_ASSIMP_IMPORTER( 3MF
  3MF/D3MFImporter.h
  3MF/D3MFImporter.cpp
  3MF/D3MFOpcPackage.h
  3MF/D3MFOpcPackage.cpp
  3MF/3MFXmlTags.h
)

ADD_ASSIMP_EXPORTER( 3MF
  3MF/D3MFExporter.h
  3MF/D3MFExporter.cpp
)

ADD_ASSIMP_IMPORTER( MMD
  MMD/MMDCpp14.h
  MMD/MMDImporter.cpp
  MMD/MMDImporter.h
  MMD/MMDPmdParser.h
  MMD/MMDPmxParser.h
  MMD/MMDPmxParser.cpp
  MMD/MMDVmdParser.h
)

ADD_ASSIMP_EXPORTER( ASSJSON
  Assjson/cencode.c
  Assjson/cencode.h
  Assjson/json_exporter.cpp
  Assjson/mesh_splitter.cpp
  Assjson/mesh_splitter.h
)

# Workaround for issue #2406 - force problematic large file to be optimized to prevent string table overflow error
# Used -Os instead of -O2 as previous issues had mentioned, since -Os is roughly speaking -O2, excluding any
# optimizations that take up extra space. Given that the issue is a string table overflowing, -Os seemed appropriate
# Also, I'm not positive if both link & compile flags are needed, but this hopefully ensures that the issue should not
# recur for edge cases such as static builds.
if ((CMAKE_COMPILER_IS_MINGW) AND (CMAKE_BUILD_TYPE MATCHES Debug))
  message("-- Applying MinGW StepFileGen1.cpp Debug Workaround")
  SET_SOURCE_FILES_PROPERTIES(Importer/StepFile/StepFileGen1.cpp PROPERTIES COMPILE_FLAGS -Os )
  SET_SOURCE_FILES_PROPERTIES(Importer/StepFile/StepFileGen1.cpp PROPERTIES LINK_FLAGS -Os )
  SET_SOURCE_FILES_PROPERTIES(Importer/StepFile/StepFileGen1.cpp PROPERTIES STATIC_LIBRARY_FLAGS -Os )
endif()

ADD_ASSIMP_IMPORTER( STEP
    Step/STEPFile.h
    Importer/StepFile/StepFileImporter.h
    Importer/StepFile/StepFileImporter.cpp
    Importer/StepFile/StepFileGen1.cpp
    Importer/StepFile/StepFileGen2.cpp
    Importer/StepFile/StepFileGen3.cpp
    Importer/StepFile/StepReaderGen.h
)

ADD_ASSIMP_EXPORTER( STEP
    Step/StepExporter.h
    Step/StepExporter.cpp
)

if ((NOT ASSIMP_NO_EXPORT) OR (NOT ASSIMP_EXPORTERS_ENABLED STREQUAL ""))
	SET( Exporter_SRCS
	  Common/Exporter.cpp
	  CApi/AssimpCExport.cpp
	  ${HEADER_PATH}/BlobIOSystem.h
	)
	SOURCE_GROUP( Exporter FILES ${Exporter_SRCS})
endif()

SET( Extra_SRCS
  MD4FileData.h
)
SOURCE_GROUP( Extra FILES ${Extra_SRCS})

# irrXML
IF(HUNTER_ENABLED)
  hunter_add_package(irrXML)
  find_package(irrXML CONFIG REQUIRED)
ELSE(HUNTER_ENABLED)
  # irrXML already included in contrib directory by parent CMakeLists.txt.
ENDIF(HUNTER_ENABLED)

# utf8
IF(HUNTER_ENABLED)
  hunter_add_package(utf8)
  find_package(utf8 CONFIG REQUIRED)
ELSE(HUNTER_ENABLED)
  # utf8 is header-only, so Assimp doesn't need to do anything.
ENDIF(HUNTER_ENABLED)

# polyclipping
IF(HUNTER_ENABLED)
  hunter_add_package(polyclipping)
  find_package(polyclipping CONFIG REQUIRED)
ELSE(HUNTER_ENABLED)
  SET( Clipper_SRCS
    ../contrib/clipper/clipper.hpp
    ../contrib/clipper/clipper.cpp
  )
  SOURCE_GROUP( Contrib\\Clipper FILES ${Clipper_SRCS})
ENDIF(HUNTER_ENABLED)

# poly2tri
IF(HUNTER_ENABLED)
  hunter_add_package(poly2tri)
  find_package(poly2tri CONFIG REQUIRED)
ELSE(HUNTER_ENABLED)
  SET( Poly2Tri_SRCS
    ../contrib/poly2tri/poly2tri/common/shapes.cc
    ../contrib/poly2tri/poly2tri/common/shapes.h
    ../contrib/poly2tri/poly2tri/common/utils.h
    ../contrib/poly2tri/poly2tri/sweep/advancing_front.h
    ../contrib/poly2tri/poly2tri/sweep/advancing_front.cc
    ../contrib/poly2tri/poly2tri/sweep/cdt.cc
    ../contrib/poly2tri/poly2tri/sweep/cdt.h
    ../contrib/poly2tri/poly2tri/sweep/sweep.cc
    ../contrib/poly2tri/poly2tri/sweep/sweep.h
    ../contrib/poly2tri/poly2tri/sweep/sweep_context.cc
    ../contrib/poly2tri/poly2tri/sweep/sweep_context.h
  )
  SOURCE_GROUP( Contrib\\Poly2Tri FILES ${Poly2Tri_SRCS})
ENDIF(HUNTER_ENABLED)

# minizip/unzip
IF(HUNTER_ENABLED)
  hunter_add_package(minizip)
  find_package(minizip CONFIG REQUIRED)
ELSE(HUNTER_ENABLED)
  SET( unzip_SRCS
    ../contrib/unzip/crypt.h
    ../contrib/unzip/ioapi.c
    ../contrib/unzip/ioapi.h
    ../contrib/unzip/unzip.c
    ../contrib/unzip/unzip.h
  )
  SOURCE_GROUP(Contrib\\unzip FILES ${unzip_SRCS})
ENDIF(HUNTER_ENABLED)

# zip (https://github.com/kuba--/zip)
IF(HUNTER_ENABLED)
  hunter_add_package(zip)
  find_package(zip CONFIG REQUIRED)
ELSE(HUNTER_ENABLED)
  SET( ziplib_SRCS
    ../contrib/zip/src/miniz.h
    ../contrib/zip/src/zip.c
    ../contrib/zip/src/zip.h
  )

  # TODO if cmake required version has been updated to >3.12.0, collapse this to the second case only
  if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
  	add_definitions(-DMINIZ_USE_UNALIGNED_LOADS_AND_STORES=0)
  else()
  	add_compile_definitions(MINIZ_USE_UNALIGNED_LOADS_AND_STORES=0)
  endif()

  SOURCE_GROUP( ziplib FILES ${ziplib_SRCS} )
ENDIF(HUNTER_ENABLED)

# openddlparser
IF(HUNTER_ENABLED)
  hunter_add_package(openddlparser)
  find_package(openddlparser CONFIG REQUIRED)
ELSE(HUNTER_ENABLED)
  SET ( openddl_parser_SRCS
    ../contrib/openddlparser/code/OpenDDLParser.cpp
    ../contrib/openddlparser/code/DDLNode.cpp
    ../contrib/openddlparser/code/OpenDDLCommon.cpp
    ../contrib/openddlparser/code/OpenDDLExport.cpp
    ../contrib/openddlparser/code/Value.cpp
    ../contrib/openddlparser/code/OpenDDLStream.cpp
    ../contrib/openddlparser/include/openddlparser/OpenDDLParser.h
    ../contrib/openddlparser/include/openddlparser/OpenDDLParserUtils.h
    ../contrib/openddlparser/include/openddlparser/OpenDDLCommon.h
    ../contrib/openddlparser/include/openddlparser/OpenDDLExport.h
    ../contrib/openddlparser/include/openddlparser/OpenDDLStream.h
    ../contrib/openddlparser/include/openddlparser/DDLNode.h
    ../contrib/openddlparser/include/openddlparser/Value.h
  )
  SOURCE_GROUP( Contrib\\openddl_parser FILES ${openddl_parser_SRCS})
ENDIF(HUNTER_ENABLED)

# Open3DGC
IF(HUNTER_ENABLED)
  # Nothing to do, not available in Hunter yet.
ELSE(HUNTER_ENABLED)
  SET ( open3dgc_SRCS
    ../contrib/Open3DGC/o3dgcAdjacencyInfo.h
    ../contrib/Open3DGC/o3dgcArithmeticCodec.cpp
    ../contrib/Open3DGC/o3dgcArithmeticCodec.h
    ../contrib/Open3DGC/o3dgcBinaryStream.h
    ../contrib/Open3DGC/o3dgcCommon.h
    ../contrib/Open3DGC/o3dgcDVEncodeParams.h
    ../contrib/Open3DGC/o3dgcDynamicVectorDecoder.cpp
    ../contrib/Open3DGC/o3dgcDynamicVectorDecoder.h
    ../contrib/Open3DGC/o3dgcDynamicVectorEncoder.cpp
    ../contrib/Open3DGC/o3dgcDynamicVectorEncoder.h
    ../contrib/Open3DGC/o3dgcDynamicVector.h
    ../contrib/Open3DGC/o3dgcFIFO.h
    ../contrib/Open3DGC/o3dgcIndexedFaceSet.h
    ../contrib/Open3DGC/o3dgcIndexedFaceSet.inl
    ../contrib/Open3DGC/o3dgcSC3DMCDecoder.h
    ../contrib/Open3DGC/o3dgcSC3DMCDecoder.inl
    ../contrib/Open3DGC/o3dgcSC3DMCEncodeParams.h
    ../contrib/Open3DGC/o3dgcSC3DMCEncoder.h
    ../contrib/Open3DGC/o3dgcSC3DMCEncoder.inl
    ../contrib/Open3DGC/o3dgcTimer.h
    ../contrib/Open3DGC/o3dgcTools.cpp
    ../contrib/Open3DGC/o3dgcTriangleFans.cpp
    ../contrib/Open3DGC/o3dgcTriangleFans.h
    ../contrib/Open3DGC/o3dgcTriangleListDecoder.h
    ../contrib/Open3DGC/o3dgcTriangleListDecoder.inl
    ../contrib/Open3DGC/o3dgcTriangleListEncoder.h
    ../contrib/Open3DGC/o3dgcTriangleListEncoder.inl
    ../contrib/Open3DGC/o3dgcVector.h
    ../contrib/Open3DGC/o3dgcVector.inl
  )
  SOURCE_GROUP( Contrib\\open3dgc FILES ${open3dgc_SRCS})
ENDIF(HUNTER_ENABLED)

# Check dependencies for glTF importer with Open3DGC-compression.
# RT-extensions is used in "contrib/Open3DGC/o3dgcTimer.h" for collecting statistics. Pointed file
# has implementation for different platforms: WIN32, __MACH__ and other ("else" block).
FIND_PACKAGE(RT QUIET)
IF (NOT HUNTER_ENABLED AND (RT_FOUND OR MSVC))
  SET( ASSIMP_IMPORTER_GLTF_USE_OPEN3DGC 1 )
  ADD_DEFINITIONS( -DASSIMP_IMPORTER_GLTF_USE_OPEN3DGC=1 )
ELSE ()
  SET (open3dgc_SRCS "")
  MESSAGE (INFO " Hunter enabled or RT-extension not found. glTF import/export will be built without Open3DGC-compression.")
  #!TODO: off course is better to remove statistics timers from o3dgc codec. Or propose to choose what to use.
ENDIF ()

# RapidJSON
IF(HUNTER_ENABLED)
  hunter_add_package(RapidJSON)
  find_package(RapidJSON CONFIG REQUIRED)
ELSE(HUNTER_ENABLED)
  INCLUDE_DIRECTORIES( "../contrib/rapidjson/include" )
  INCLUDE_DIRECTORIES( "../contrib" )
ENDIF(HUNTER_ENABLED)

# VC2010 fixes
if(MSVC10)
  option( VC10_STDINT_FIX "Fix for VC10 Compiler regarding pstdint.h redefinition errors" OFF )
  if( VC10_STDINT_FIX )
    ADD_DEFINITIONS( -D_STDINT )
  endif( VC10_STDINT_FIX )
endif(MSVC10)

ADD_DEFINITIONS( -DASSIMP_BUILD_DLL_EXPORT )

if ( MSVC )
  ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS )
  ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
endif ( MSVC )

IF(NOT HUNTER_ENABLED)
  if (UNZIP_FOUND)
    SET (unzip_compile_SRCS "")
  else (UNZIP_FOUND)
    SET (unzip_compile_SRCS ${unzip_SRCS})
    INCLUDE_DIRECTORIES( "../contrib/unzip/" )
  endif (UNZIP_FOUND)
ENDIF(NOT HUNTER_ENABLED)

MESSAGE(STATUS "Enabled importer formats:${ASSIMP_IMPORTERS_ENABLED}")
MESSAGE(STATUS "Disabled importer formats:${ASSIMP_IMPORTERS_DISABLED}")

MESSAGE(STATUS "Enabled exporter formats:${ASSIMP_EXPORTERS_ENABLED}")
MESSAGE(STATUS "Disabled exporter formats:${ASSIMP_EXPORTERS_DISABLED}")

SET( assimp_src
  # Assimp Files
  ${Core_SRCS}
  ${CApi_SRCS}
  ${Common_SRCS}
  ${Logging_SRCS}
  ${Exporter_SRCS}
  ${PostProcessing_SRCS}
  ${MaterialSystem_SRCS}
  ${STEPParser_SRCS}
#  ${Step_SRCS} check if we need a different approach

  # Model Support
  ${ASSIMP_LOADER_SRCS}
  ${ASSIMP_EXPORTER_SRCS}

  # Third-party libraries
  ${IrrXML_SRCS}
  ${unzip_compile_SRCS}
  ${Poly2Tri_SRCS}
  ${Clipper_SRCS}
  ${openddl_parser_SRCS}
  ${open3dgc_SRCS}
  ${ziplib_SRCS}
  # Necessary to show the headers in the project when using the VC++ generator:

  ${PUBLIC_HEADERS}
  ${COMPILER_HEADERS}
)
ADD_DEFINITIONS( -DOPENDDLPARSER_BUILD )

IF(NOT HUNTER_ENABLED)
  INCLUDE_DIRECTORIES(
      ${IRRXML_INCLUDE_DIR}
      ../contrib/openddlparser/include
  )
ENDIF(NOT HUNTER_ENABLED)

IF (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)
  SET( assimp_src ${assimp_src} ${C4D_SRCS})
  INCLUDE_DIRECTORIES(${C4D_INCLUDES})
ENDIF (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)

ADD_LIBRARY( assimp ${assimp_src} )
ADD_LIBRARY(assimp::assimp ALIAS assimp)

TARGET_INCLUDE_DIRECTORIES ( assimp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>
  $<INSTALL_INTERFACE:include>
)

IF(HUNTER_ENABLED)
  TARGET_LINK_LIBRARIES(assimp
      PUBLIC
      polyclipping::polyclipping
      irrXML::irrXML
      openddlparser::openddl_parser
      poly2tri::poly2tri
      minizip::minizip
      ZLIB::zlib
      RapidJSON::rapidjson
      utf8::utf8
      zip::zip
  )
ELSE(HUNTER_ENABLED)
  TARGET_LINK_LIBRARIES(assimp ${ZLIB_LIBRARIES} ${OPENDDL_PARSER_LIBRARIES} ${IRRXML_LIBRARY} )
ENDIF(HUNTER_ENABLED)

if(ASSIMP_ANDROID_JNIIOSYSTEM)
  set(ASSIMP_ANDROID_JNIIOSYSTEM_PATH port/AndroidJNI)
  add_subdirectory(../${ASSIMP_ANDROID_JNIIOSYSTEM_PATH}/ ../${ASSIMP_ANDROID_JNIIOSYSTEM_PATH}/)
  target_link_libraries(assimp android_jniiosystem)
endif(ASSIMP_ANDROID_JNIIOSYSTEM)

IF (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)
  TARGET_LINK_LIBRARIES(assimp optimized ${C4D_RELEASE_LIBRARIES})
  TARGET_LINK_LIBRARIES(assimp debug ${C4D_DEBUG_LIBRARIES})
  TARGET_LINK_LIBRARIES(assimp ${C4D_EXTRA_LIBRARIES})
ENDIF (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)

if( MSVC )
  # in order to prevent DLL hell, each of the DLLs have to be suffixed with the major version and msvc prefix
  # CMake 3.12 added a variable for this
  if(MSVC_TOOLSET_VERSION)
    set(MSVC_PREFIX "vc${MSVC_TOOLSET_VERSION}")
  else()
    if( MSVC70 OR MSVC71 )
      set(MSVC_PREFIX "vc70")
    elseif( MSVC80 )
      set(MSVC_PREFIX "vc80")
    elseif( MSVC90 )
      set(MSVC_PREFIX "vc90")
    elseif( MSVC10 )
      set(MSVC_PREFIX "vc100")
    elseif( MSVC11 )
      set(MSVC_PREFIX "vc110")
    elseif( MSVC12 )
      set(MSVC_PREFIX "vc120")
    elseif( MSVC_VERSION LESS 1910)
      set(MSVC_PREFIX "vc140")
    elseif( MSVC_VERSION LESS 1920)
      set(MSVC_PREFIX "vc141")
    elseif( MSVC_VERSION LESS 1930)
      set(MSVC_PREFIX "vc142")
    else()
      MESSAGE(WARNING "unknown msvc version ${MSVC_VERSION}")
      set(MSVC_PREFIX "vc150")
    endif()
  endif()
  set(LIBRARY_SUFFIX "${ASSIMP_LIBRARY_SUFFIX}-${MSVC_PREFIX}-mt" CACHE STRING "the suffix for the assimp windows library")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "WindowsStore")
    target_compile_definitions(assimp PUBLIC WindowsStore)
    TARGET_LINK_LIBRARIES(assimp advapi32)
    #set(WindowsStore TRUE)
endif()
SET_TARGET_PROPERTIES( assimp PROPERTIES
  VERSION ${ASSIMP_VERSION}
  SOVERSION ${ASSIMP_SOVERSION} # use full version
  OUTPUT_NAME assimp${LIBRARY_SUFFIX}
)

if (APPLE)
  SET_TARGET_PROPERTIES( assimp PROPERTIES
    INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/${ASSIMP_LIB_INSTALL_DIR}"
  )

  if (BUILD_FRAMEWORK)
    SET_TARGET_PROPERTIES( assimp PROPERTIES
      FRAMEWORK TRUE
      FRAMEWORK_VERSION C
      MACOSX_FRAMEWORK_IDENTIFIER net.sf.assimp
      PUBLIC_HEADER "${PUBLIC_HEADERS}"
    )

    # PUBLIC_HEADER option does not support directory structure creation
    # add ./Compiler/*.h to assimp.framework via copy command
    ADD_CUSTOM_COMMAND(TARGET assimp POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_directory
         "../${HEADER_PATH}/Compiler"
         assimp.framework/Headers/Compiler
      COMMENT "Copying public ./Compiler/ header files to framework bundle's Headers/Compiler/")
  ENDIF(BUILD_FRAMEWORK)
ENDIF(APPLE)

# Build against external unzip, or add ../contrib/unzip so
# assimp can #include "unzip.h"
IF(NOT HUNTER_ENABLED)
  if (UNZIP_FOUND)
    INCLUDE_DIRECTORIES(${UNZIP_INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(assimp ${UNZIP_LIBRARIES})
  else (UNZIP_FOUND)
    INCLUDE_DIRECTORIES("../")
  endif (UNZIP_FOUND)
ENDIF(NOT HUNTER_ENABLED)

# Add RT-extension library for glTF importer with Open3DGC-compression.
IF (RT_FOUND AND ASSIMP_IMPORTER_GLTF_USE_OPEN3DGC)
  TARGET_LINK_LIBRARIES(assimp ${RT_LIBRARY})
ENDIF (RT_FOUND AND ASSIMP_IMPORTER_GLTF_USE_OPEN3DGC)

IF(HUNTER_ENABLED)
  INSTALL( TARGETS assimp
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
    RUNTIME DESTINATION ${ASSIMP_BIN_INSTALL_DIR}
    FRAMEWORK DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
    COMPONENT ${LIBASSIMP_COMPONENT}
    INCLUDES DESTINATION "include")
ELSE(HUNTER_ENABLED)
INSTALL( TARGETS assimp
    LIBRARY DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
    RUNTIME DESTINATION ${ASSIMP_BIN_INSTALL_DIR}
    FRAMEWORK DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
    COMPONENT ${LIBASSIMP_COMPONENT})
ENDIF(HUNTER_ENABLED)
INSTALL( FILES ${PUBLIC_HEADERS} DESTINATION ${ASSIMP_INCLUDE_INSTALL_DIR}/assimp COMPONENT assimp-dev)
INSTALL( FILES ${COMPILER_HEADERS} DESTINATION ${ASSIMP_INCLUDE_INSTALL_DIR}/assimp/Compiler COMPONENT assimp-dev)

if (ASSIMP_ANDROID_JNIIOSYSTEM)
  INSTALL(FILES ${HEADER_PATH}/${ASSIMP_ANDROID_JNIIOSYSTEM_PATH}/AndroidJNIIOSystem.h
    DESTINATION ${ASSIMP_INCLUDE_INSTALL_DIR}
    COMPONENT assimp-dev)
ENDIF(ASSIMP_ANDROID_JNIIOSYSTEM)

if(MSVC AND ASSIMP_INSTALL_PDB)
  # When only the static library is built, these properties must
  # be set to ensure the static lib .pdb is staged for installation.
  IF(NOT BUILD_SHARED_LIBS)
    SET_TARGET_PROPERTIES( assimp PROPERTIES
      COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMPILE_PDB_NAME assimp${LIBRARY_SUFFIX}
      COMPILE_PDB_NAME_DEBUG assimp${LIBRARY_SUFFIX}${CMAKE_DEBUG_POSTFIX}
    )
  ENDIF()

  IF(CMAKE_GENERATOR MATCHES "^Visual Studio")
    install(FILES ${Assimp_BINARY_DIR}/code/Debug/assimp${LIBRARY_SUFFIX}${CMAKE_DEBUG_POSTFIX}.pdb
      DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
      CONFIGURATIONS Debug
    )
    install(FILES ${Assimp_BINARY_DIR}/code/RelWithDebInfo/assimp${LIBRARY_SUFFIX}.pdb
      DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
      CONFIGURATIONS RelWithDebInfo
    )
  ELSE()
    install(FILES ${Assimp_BINARY_DIR}/code/assimp${LIBRARY_SUFFIX}${CMAKE_DEBUG_POSTFIX}.pdb
      DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
      CONFIGURATIONS Debug
    )
    install(FILES ${Assimp_BINARY_DIR}/code/assimp${LIBRARY_SUFFIX}.pdb
      DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
      CONFIGURATIONS RelWithDebInfo
    )
  ENDIF()
ENDIF ()

if (ASSIMP_COVERALLS)
    include(Coveralls)

    set(COVERAGE_SRCS ${assimp_src} ${TEST_SRCS} )

    # Create the coveralls target.
    coveralls_setup(
        "${COVERAGE_SRCS}" # The source files.
        ON                 # If we should upload.
        "${PROJECT_SOURCE_DIR}/cmake-modules/") # (Optional) Alternate project cmake module path.
ENDIF()
