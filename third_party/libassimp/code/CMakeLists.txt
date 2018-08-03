# Open Asset Import Library (assimp)
# ----------------------------------------------------------------------
#
# Copyright (c) 2006-2017, assimp team

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
cmake_minimum_required( VERSION 2.6 )
SET( HEADER_PATH ../include/assimp )

SET( COMPILER_HEADERS
  ${HEADER_PATH}/Compiler/pushpack1.h
  ${HEADER_PATH}/Compiler/poppack1.h
  ${HEADER_PATH}/Compiler/pstdint.h
)
SOURCE_GROUP( Compiler FILES ${COMPILER_HEADERS})

SET( PUBLIC_HEADERS
  ${HEADER_PATH}/anim.h
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
  ${HEADER_PATH}/SceneCombiner.h
)

SET( Core_SRCS
  Assimp.cpp
)

SET( Logging_SRCS
  ${HEADER_PATH}/DefaultLogger.hpp
  ${HEADER_PATH}/LogStream.hpp
  ${HEADER_PATH}/Logger.hpp
  ${HEADER_PATH}/NullLogger.hpp
  Win32DebugLogStream.h
  DefaultLogger.cpp
  FileLogStream.h
  StdOStreamLogStream.h
)
SOURCE_GROUP(Logging FILES ${Logging_SRCS})

SET( Common_SRCS
  fast_atof.h
  qnan.h
  BaseImporter.cpp
  BaseImporter.h
  BaseProcess.cpp
  BaseProcess.h
  Importer.h
  ScenePrivate.h
  PostStepRegistry.cpp
  ImporterRegistry.cpp
  ByteSwapper.h
  DefaultProgressHandler.h
  DefaultIOStream.cpp
  DefaultIOSystem.cpp
  CInterfaceIOWrapper.cpp
  CInterfaceIOWrapper.h
  Hash.h
  Importer.cpp
  IFF.h
  MemoryIOWrapper.h
  ParsingUtils.h
  StreamReader.h
  StreamWriter.h
  StringComparison.h
  StringUtils.h
  SGSpatialSort.cpp
  SGSpatialSort.h
  VertexTriangleAdjacency.cpp
  VertexTriangleAdjacency.h
  GenericProperty.h
  SpatialSort.cpp
  SpatialSort.h
  SceneCombiner.cpp
  ScenePreprocessor.cpp
  ScenePreprocessor.h
  SkeletonMeshBuilder.cpp
  SkeletonMeshBuilder.h
  SplitByBoneCountProcess.cpp
  SplitByBoneCountProcess.h
  ScaleProcess.cpp
  ScaleProcess.h
  SmoothingGroups.h
  StandardShapes.cpp
  StandardShapes.h
  TargetAnimation.cpp
  TargetAnimation.h
  RemoveComments.cpp
  RemoveComments.h
  Subdivision.cpp
  Subdivision.h
  scene.cpp
  Vertex.h
  LineSplitter.h
  TinyFormatter.h
  Profiler.h
  LogAux.h
  Bitmap.cpp
  Bitmap.h
  XMLTools.h
  Version.cpp
  IOStreamBuffer.h
  CreateAnimMesh.h
  CreateAnimMesh.cpp
)
SOURCE_GROUP(Common FILES ${Common_SRCS})

IF ( ASSIMP_BUILD_NONFREE_C4D_IMPORTER )
  SET( C4D_SRCS
    C4DImporter.cpp
    C4DImporter.h
  )
  SOURCE_GROUP( C4D FILES ${C4D_SRCS})
ENDIF ( ASSIMP_BUILD_NONFREE_C4D_IMPORTER )

# if this variable is set to TRUE, the user can manually disable importers by setting
# ASSIMP_BUILD_XXX_IMPORTER to FALSE for each importer
# if this variable is set to FALSE, the user can manually enable importers by setting
# ASSIMP_BUILD_XXX_IMPORTER to TRUE for each importer
OPTION(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT "default value of all ASSIMP_BUILD_XXX_IMPORTER value" TRUE)

# macro to add the CMake Option ADD_ASSIMP_IMPORTER_<name> which enables compile of loader
# this way selective loaders can be compiled (reduces filesize + compile time)
MACRO(ADD_ASSIMP_IMPORTER name)
  OPTION(ASSIMP_BUILD_${name}_IMPORTER "build the ${name} importer" ${ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT})
  IF(ASSIMP_BUILD_${name}_IMPORTER)
    LIST(APPEND ASSIMP_LOADER_SRCS ${ARGN})
    SET(ASSIMP_IMPORTERS_ENABLED "${ASSIMP_IMPORTERS_ENABLED} ${name}")
    SET(${name}_SRCS ${ARGN})
    SOURCE_GROUP(${name} FILES ${ARGN})
  ELSE()
    SET(${name}_SRC "")
    SET(ASSIMP_IMPORTERS_DISABLED "${ASSIMP_IMPORTERS_DISABLED} ${name}")
    add_definitions(-DASSIMP_BUILD_NO_${name}_IMPORTER)
    add_definitions(-DASSIMP_BUILD_NO_${name}_EXPORTER)
  ENDIF()
ENDMACRO()

SET(ASSIMP_LOADER_SRCS "")
SET(ASSIMP_IMPORTERS_ENABLED "") # list of enabled importers
SET(ASSIMP_IMPORTERS_DISABLED "") # disabled list (used to print)

ADD_ASSIMP_IMPORTER( AMF
  AMFImporter.hpp
  AMFImporter_Macro.hpp
  AMFImporter_Node.hpp
  AMFImporter.cpp
  AMFImporter_Geometry.cpp
  AMFImporter_Material.cpp
  AMFImporter_Postprocess.cpp
)

ADD_ASSIMP_IMPORTER( 3DS
  3DSConverter.cpp
  3DSHelper.h
  3DSLoader.cpp
  3DSLoader.h
  3DSExporter.h
  3DSExporter.cpp
)

ADD_ASSIMP_IMPORTER( AC
  ACLoader.cpp
  ACLoader.h
)

ADD_ASSIMP_IMPORTER( ASE
  ASELoader.cpp
  ASELoader.h
  ASEParser.cpp
  ASEParser.h
)

ADD_ASSIMP_IMPORTER( ASSBIN
  AssbinExporter.h
  AssbinExporter.cpp
  AssbinLoader.h
  AssbinLoader.cpp
)

ADD_ASSIMP_IMPORTER( ASSXML
  AssxmlExporter.h
  AssxmlExporter.cpp
)

ADD_ASSIMP_IMPORTER( B3D
  B3DImporter.cpp
  B3DImporter.h
)

ADD_ASSIMP_IMPORTER( BVH
  BVHLoader.cpp
  BVHLoader.h
)

ADD_ASSIMP_IMPORTER( COLLADA
  ColladaHelper.h
  ColladaLoader.cpp
  ColladaLoader.h
  ColladaParser.cpp
  ColladaParser.h
  ColladaExporter.h
  ColladaExporter.cpp
)

ADD_ASSIMP_IMPORTER( DXF
  DXFLoader.cpp
  DXFLoader.h
  DXFHelper.h
)

ADD_ASSIMP_IMPORTER( CSM
  CSMLoader.cpp
  CSMLoader.h
)

ADD_ASSIMP_IMPORTER( HMP
  HMPFileData.h
  HMPLoader.cpp
  HMPLoader.h
  HalfLifeFileData.h
)

ADD_ASSIMP_IMPORTER( IRRMESH
  IRRMeshLoader.cpp
  IRRMeshLoader.h
  IRRShared.cpp
  IRRShared.h
)

ADD_ASSIMP_IMPORTER( IRR
  IRRLoader.cpp
  IRRLoader.h
  IRRShared.cpp
  IRRShared.h
)

ADD_ASSIMP_IMPORTER( LWO
  LWOAnimation.cpp
  LWOAnimation.h
  LWOBLoader.cpp
  LWOFileData.h
  LWOLoader.cpp
  LWOLoader.h
  LWOMaterial.cpp
)

ADD_ASSIMP_IMPORTER( LWS
  LWSLoader.cpp
  LWSLoader.h
)

ADD_ASSIMP_IMPORTER( MD2
  MD2FileData.h
  MD2Loader.cpp
  MD2Loader.h
  MD2NormalTable.h
)

ADD_ASSIMP_IMPORTER( MD3
  MD3FileData.h
  MD3Loader.cpp
  MD3Loader.h
)

ADD_ASSIMP_IMPORTER( MD5
  MD5Loader.cpp
  MD5Loader.h
  MD5Parser.cpp
  MD5Parser.h
)

ADD_ASSIMP_IMPORTER( MDC
  MDCFileData.h
  MDCLoader.cpp
  MDCLoader.h
  MDCNormalTable.h
)

ADD_ASSIMP_IMPORTER( MDL
  MDLDefaultColorMap.h
  MDLFileData.h
  MDLLoader.cpp
  MDLLoader.h
  MDLMaterialLoader.cpp
)

SET( MaterialSystem_SRCS
  MaterialSystem.cpp
  MaterialSystem.h
)
SOURCE_GROUP( MaterialSystem FILES ${MaterialSystem_SRCS})

ADD_ASSIMP_IMPORTER( NFF
  NFFLoader.cpp
  NFFLoader.h
)

ADD_ASSIMP_IMPORTER( NDO
  NDOLoader.cpp
  NDOLoader.h
)

ADD_ASSIMP_IMPORTER( OFF
  OFFLoader.cpp
  OFFLoader.h
)

ADD_ASSIMP_IMPORTER( OBJ
  ObjFileData.h
  ObjFileImporter.cpp
  ObjFileImporter.h
  ObjFileMtlImporter.cpp
  ObjFileMtlImporter.h
  ObjFileParser.cpp
  ObjFileParser.h
  ObjTools.h
  ObjExporter.h
  ObjExporter.cpp
)

ADD_ASSIMP_IMPORTER( OGRE
  OgreImporter.h
  OgreStructs.h
  OgreParsingUtils.h
  OgreBinarySerializer.h
  OgreXmlSerializer.h
  OgreImporter.cpp
  OgreStructs.cpp
  OgreBinarySerializer.cpp
  OgreXmlSerializer.cpp
  OgreMaterial.cpp
)

ADD_ASSIMP_IMPORTER( OPENGEX
  OpenGEXExporter.cpp
  OpenGEXExporter.h
  OpenGEXImporter.cpp
  OpenGEXImporter.h
  OpenGEXStructs.h
)

ADD_ASSIMP_IMPORTER( PLY
  PlyLoader.cpp
  PlyLoader.h
  PlyParser.cpp
  PlyParser.h
  PlyExporter.cpp
  PlyExporter.h
)

ADD_ASSIMP_IMPORTER( MS3D
  MS3DLoader.cpp
  MS3DLoader.h
)

ADD_ASSIMP_IMPORTER( COB
  COBLoader.cpp
  COBLoader.h
  COBScene.h
)

ADD_ASSIMP_IMPORTER( BLEND
  BlenderLoader.cpp
  BlenderLoader.h
  BlenderDNA.cpp
  BlenderDNA.h
  BlenderDNA.inl
  BlenderScene.cpp
  BlenderScene.h
  BlenderSceneGen.h
  BlenderIntermediate.h
  BlenderModifier.h
  BlenderModifier.cpp
  BlenderBMesh.h
  BlenderBMesh.cpp
  BlenderTessellator.h
  BlenderTessellator.cpp
)

ADD_ASSIMP_IMPORTER( IFC
  IFCLoader.cpp
  IFCLoader.h
  IFCReaderGen1.cpp
  IFCReaderGen2.cpp
  IFCReaderGen.h
  IFCUtil.h
  IFCUtil.cpp
  IFCGeometry.cpp
  IFCMaterial.cpp
  IFCProfile.cpp
  IFCCurve.cpp
  IFCBoolean.cpp
  IFCOpenings.cpp
  STEPFile.h
  STEPFileReader.h
  STEPFileReader.cpp
  STEPFileEncoding.cpp
  STEPFileEncoding.h
)
if (ASSIMP_BUILD_IFC_IMPORTER)
  if (MSVC)
    set_source_files_properties(IFCReaderGen1.cpp IFCReaderGen2.cpp PROPERTIES COMPILE_FLAGS "/bigobj")
  elseif(CMAKE_COMPILER_IS_MINGW)
    set_source_files_properties(IFCReaderGen1.cpp IFCReaderGen2.cpp PROPERTIES COMPILE_FLAGS "-O2 -Wa,-mbig-obj")
  endif()
endif (ASSIMP_BUILD_IFC_IMPORTER)

ADD_ASSIMP_IMPORTER( XGL
  XGLLoader.cpp
  XGLLoader.h
)


ADD_ASSIMP_IMPORTER( FBX
  FBXImporter.cpp
  FBXCompileConfig.h
  FBXImporter.h
  FBXParser.cpp
  FBXParser.h
  FBXTokenizer.cpp
  FBXTokenizer.h
  FBXImportSettings.h
  FBXConverter.h
  FBXConverter.cpp
  FBXUtil.h
  FBXUtil.cpp
  FBXDocument.h
  FBXDocument.cpp
  FBXProperties.h
  FBXProperties.cpp
  FBXMeshGeometry.h
  FBXMeshGeometry.cpp
  FBXMaterial.cpp
  FBXModel.cpp
  FBXAnimation.cpp
  FBXNodeAttribute.cpp
  FBXDeformer.cpp
  FBXBinaryTokenizer.cpp
  FBXDocumentUtil.cpp
)

SET( PostProcessing_SRCS
  CalcTangentsProcess.cpp
  CalcTangentsProcess.h
  ComputeUVMappingProcess.cpp
  ComputeUVMappingProcess.h
  ConvertToLHProcess.cpp
  ConvertToLHProcess.h
  FindDegenerates.cpp
  FindDegenerates.h
  FindInstancesProcess.cpp
  FindInstancesProcess.h
  FindInvalidDataProcess.cpp
  FindInvalidDataProcess.h
  FixNormalsStep.cpp
  FixNormalsStep.h
  GenFaceNormalsProcess.cpp
  GenFaceNormalsProcess.h
  GenVertexNormalsProcess.cpp
  GenVertexNormalsProcess.h
  PretransformVertices.cpp
  PretransformVertices.h
  ImproveCacheLocality.cpp
  ImproveCacheLocality.h
  JoinVerticesProcess.cpp
  JoinVerticesProcess.h
  LimitBoneWeightsProcess.cpp
  LimitBoneWeightsProcess.h
  RemoveRedundantMaterials.cpp
  RemoveRedundantMaterials.h
  RemoveVCProcess.cpp
  RemoveVCProcess.h
  SortByPTypeProcess.cpp
  SortByPTypeProcess.h
  SplitLargeMeshes.cpp
  SplitLargeMeshes.h
  TextureTransform.cpp
  TextureTransform.h
  TriangulateProcess.cpp
  TriangulateProcess.h
  ValidateDataStructure.cpp
  ValidateDataStructure.h
  OptimizeGraph.cpp
  OptimizeGraph.h
  OptimizeMeshes.cpp
  OptimizeMeshes.h
  DeboneProcess.cpp
  DeboneProcess.h
  ProcessHelper.h
  ProcessHelper.cpp
  PolyTools.h
  MakeVerboseFormat.cpp
  MakeVerboseFormat.h
)
SOURCE_GROUP( PostProcessing FILES ${PostProcessing_SRCS})

SET( IrrXML_SRCS irrXMLWrapper.h )
SOURCE_GROUP( IrrXML FILES ${IrrXML_SRCS})

ADD_ASSIMP_IMPORTER( Q3D
  Q3DLoader.cpp
  Q3DLoader.h
)

ADD_ASSIMP_IMPORTER( Q3BSP
  Q3BSPFileData.h
  Q3BSPFileParser.h
  Q3BSPFileParser.cpp
  Q3BSPFileImporter.h
  Q3BSPFileImporter.cpp
  Q3BSPZipArchive.h
  Q3BSPZipArchive.cpp
)

ADD_ASSIMP_IMPORTER( RAW
  RawLoader.cpp
  RawLoader.h
)

ADD_ASSIMP_IMPORTER( SIB
  SIBImporter.cpp
  SIBImporter.h
)

ADD_ASSIMP_IMPORTER( SMD
  SMDLoader.cpp
  SMDLoader.h
)

ADD_ASSIMP_IMPORTER( STL
  STLLoader.cpp
  STLLoader.h
  STLExporter.h
  STLExporter.cpp
)

ADD_ASSIMP_IMPORTER( TERRAGEN
  TerragenLoader.cpp
  TerragenLoader.h
)

ADD_ASSIMP_IMPORTER( 3D
  UnrealLoader.cpp
  UnrealLoader.h
)

ADD_ASSIMP_IMPORTER( X
  XFileHelper.h
  XFileImporter.cpp
  XFileImporter.h
  XFileParser.cpp
  XFileParser.h
  XFileExporter.h
  XFileExporter.cpp
)

ADD_ASSIMP_IMPORTER(X3D
  X3DExporter.cpp
  X3DExporter.hpp
  X3DImporter.cpp
  X3DImporter.hpp
  X3DImporter_Geometry2D.cpp
  X3DImporter_Geometry3D.cpp
  X3DImporter_Group.cpp
  X3DImporter_Light.cpp
  X3DImporter_Macro.hpp
  X3DImporter_Metadata.cpp
  X3DImporter_Networking.cpp
  X3DImporter_Node.hpp
  X3DImporter_Postprocess.cpp
  X3DImporter_Rendering.cpp
  X3DImporter_Shape.cpp
  X3DImporter_Texturing.cpp
  FIReader.hpp
  FIReader.cpp
  X3DVocabulary.cpp
)

ADD_ASSIMP_IMPORTER( GLTF
  glTFAsset.h
  glTFAsset.inl
  glTFAssetWriter.h
  glTFAssetWriter.inl
  glTFImporter.cpp
  glTFImporter.h
  glTFExporter.h
  glTFExporter.cpp
  glTF2Asset.h
  glTF2Asset.inl
  glTF2AssetWriter.h
  glTF2AssetWriter.inl
  glTF2Importer.cpp
  glTF2Importer.h
  glTF2Exporter.h
  glTF2Exporter.cpp
)

ADD_ASSIMP_IMPORTER( 3MF
    D3MFImporter.h
    D3MFImporter.cpp
    D3MFExporter.h
    D3MFExporter.cpp
    D3MFOpcPackage.h
    D3MFOpcPackage.cpp
    3MFXmlTags.h
)

ADD_ASSIMP_IMPORTER( MMD
  MMDCpp14.h
  MMDImporter.cpp
  MMDImporter.h
  MMDPmdParser.h
  MMDPmxParser.h
  MMDPmxParser.cpp
  MMDVmdParser.h
)

SET( Step_SRCS
  StepExporter.h
  StepExporter.cpp
)
SOURCE_GROUP( Step FILES ${Step_SRCS})

SET( Exporter_SRCS
  Exporter.cpp
  AssimpCExport.cpp
  BlobIOSystem.h
)
SOURCE_GROUP( Exporter FILES ${Exporter_SRCS})

SET( Extra_SRCS
  MD4FileData.h
)
SOURCE_GROUP( Extra FILES ${Extra_SRCS})


SET( Clipper_SRCS
  ../contrib/clipper/clipper.hpp
  ../contrib/clipper/clipper.cpp
)
SOURCE_GROUP( Clipper FILES ${Clipper_SRCS})

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
SOURCE_GROUP( Poly2Tri FILES ${Poly2Tri_SRCS})

SET( unzip_SRCS
  ../contrib/unzip/crypt.h
  ../contrib/unzip/ioapi.c
  ../contrib/unzip/ioapi.h
  ../contrib/unzip/unzip.c
  ../contrib/unzip/unzip.h
)
SOURCE_GROUP( unzip FILES ${unzip_SRCS})

SET( ziplib_SRCS
  ../contrib/zip/src/miniz.h
  ../contrib/zip/src/zip.c
  ../contrib/zip/src/zip.h
)

SOURCE_GROUP( ziplib FILES ${ziplib_SRCS} )

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
SOURCE_GROUP( openddl_parser FILES ${openddl_parser_SRCS})

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
SOURCE_GROUP( open3dgc FILES ${open3dgc_SRCS})

# Check dependencies for glTF importer with Open3DGC-compression.
# RT-extensions is used in "contrib/Open3DGC/o3dgcTimer.h" for collecting statistics. Pointed file
# has implementation for different platforms: WIN32, __MACH__ and other ("else" block).
FIND_PACKAGE(RT QUIET)
IF (RT_FOUND OR MSVC)
  SET( ASSIMP_IMPORTER_GLTF_USE_OPEN3DGC 1 )
  ADD_DEFINITIONS( -DASSIMP_IMPORTER_GLTF_USE_OPEN3DGC=1 )
ELSE ()
  SET (open3dgc_SRCS "")
  MESSAGE (INFO " RT-extension not found. glTF import/export will be built without Open3DGC-compression.")
  #!TODO: off course is better to remove statistics timers from o3dgc codec. Or propose to choose what to use.
ENDIF ()

INCLUDE_DIRECTORIES( "../contrib/rapidjson/include" )
INCLUDE_DIRECTORIES( "../contrib" )

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

if (UNZIP_FOUND)
  SET (unzip_compile_SRCS "")
else (UNZIP_FOUND)
  SET (unzip_compile_SRCS ${unzip_SRCS})
  INCLUDE_DIRECTORIES( "../contrib/unzip/" )
endif (UNZIP_FOUND)

MESSAGE(STATUS "Enabled formats:${ASSIMP_IMPORTERS_ENABLED}")
MESSAGE(STATUS "Disabled formats:${ASSIMP_IMPORTERS_DISABLED}")

SET( assimp_src
  # Assimp Files
  ${Core_SRCS}
  ${Common_SRCS}
  ${Logging_SRCS}
  ${Exporter_SRCS}
  ${PostProcessing_SRCS}
  ${MaterialSystem_SRCS}
  ${Step_SRCS}

  # Model Support
  ${ASSIMP_LOADER_SRCS}

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

INCLUDE_DIRECTORIES(
    ${IRRXML_INCLUDE_DIR}
    ../contrib/openddlparser/include
)

IF (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)
  SET( assimp_src ${assimp_src} ${C4D_SRCS})
  INCLUDE_DIRECTORIES(${C4D_INCLUDES})
ENDIF (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)

ADD_LIBRARY( assimp ${assimp_src} )

TARGET_LINK_LIBRARIES(assimp ${ZLIB_LIBRARIES} ${OPENDDL_PARSER_LIBRARIES} ${IRRXML_LIBRARY} )

if(ANDROID AND ASSIMP_ANDROID_JNIIOSYSTEM)
  set(ASSIMP_ANDROID_JNIIOSYSTEM_PATH port/AndroidJNI)
  add_subdirectory(../${ASSIMP_ANDROID_JNIIOSYSTEM_PATH}/ ../${ASSIMP_ANDROID_JNIIOSYSTEM_PATH}/)
  target_link_libraries(assimp android_jniiosystem)
endif(ANDROID AND ASSIMP_ANDROID_JNIIOSYSTEM)

IF (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)
  TARGET_LINK_LIBRARIES(assimp optimized ${C4D_RELEASE_LIBRARIES})
  TARGET_LINK_LIBRARIES(assimp debug ${C4D_DEBUG_LIBRARIES})
  TARGET_LINK_LIBRARIES(assimp ${C4D_EXTRA_LIBRARIES})
ENDIF (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)

if( MSVC )
  # in order to prevent DLL hell, each of the DLLs have to be suffixed with the major version and msvc prefix
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
  elseif( MSVC14 )
    set(MSVC_PREFIX "vc140")
  else()
    set(MSVC_PREFIX "vc150")
  endif()
  set(LIBRARY_SUFFIX "${ASSIMP_LIBRARY_SUFFIX}-${MSVC_PREFIX}-mt" CACHE STRING "the suffix for the assimp windows library")
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
  endif(BUILD_FRAMEWORK)
endif(APPLE)

# Build against external unzip, or add ../contrib/unzip so
# assimp can #include "unzip.h"
if (UNZIP_FOUND)
  INCLUDE_DIRECTORIES(${UNZIP_INCLUDE_DIRS})
  TARGET_LINK_LIBRARIES(assimp ${UNZIP_LIBRARIES})
else (UNZIP_FOUND)
  INCLUDE_DIRECTORIES("../")
endif (UNZIP_FOUND)

# Add RT-extension library for glTF importer with Open3DGC-compression.
IF (RT_FOUND AND ASSIMP_IMPORTER_GLTF_USE_OPEN3DGC)
  TARGET_LINK_LIBRARIES(assimp ${RT_LIBRARY})
ENDIF (RT_FOUND AND ASSIMP_IMPORTER_GLTF_USE_OPEN3DGC)

INSTALL( TARGETS assimp
  LIBRARY DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
  ARCHIVE DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
  RUNTIME DESTINATION ${ASSIMP_BIN_INSTALL_DIR}
  FRAMEWORK DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
  COMPONENT ${LIBASSIMP_COMPONENT})
INSTALL( FILES ${PUBLIC_HEADERS} DESTINATION ${ASSIMP_INCLUDE_INSTALL_DIR}/assimp COMPONENT assimp-dev)
INSTALL( FILES ${COMPILER_HEADERS} DESTINATION ${ASSIMP_INCLUDE_INSTALL_DIR}/assimp/Compiler COMPONENT assimp-dev)

if (ASSIMP_ANDROID_JNIIOSYSTEM)
  INSTALL(FILES ${HEADER_PATH}/${ASSIMP_ANDROID_JNIIOSYSTEM_PATH}/AndroidJNIIOSystem.h
    DESTINATION ${ASSIMP_INCLUDE_INSTALL_DIR}
    COMPONENT assimp-dev)
endif(ASSIMP_ANDROID_JNIIOSYSTEM)

if(MSVC AND ASSIMP_INSTALL_PDB)
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
endif ()

if (ASSIMP_COVERALLS)
    include(Coveralls)

    set(COVERAGE_SRCS ${assimp_src} ${TEST_SRCS} )

    # Create the coveralls target.
    coveralls_setup(
        "${COVERAGE_SRCS}" # The source files.
        ON                 # If we should upload.
        "${PROJECT_SOURCE_DIR}/cmake-modules/") # (Optional) Alternate project cmake module path.
endif()
