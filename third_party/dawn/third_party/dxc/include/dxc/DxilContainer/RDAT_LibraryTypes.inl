///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// RDAT_LibraryTypes.inl                                                     //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Defines types used in Dxil Library Runtime Data (RDAT).                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// clang-format off
// Macro indentation makes this file easier to read, but clang-format flattens
// everything.  Turn off clang-format for this file.

#ifdef DEF_RDAT_ENUMS

RDAT_ENUM_START(DxilResourceFlag, uint32_t)
  RDAT_ENUM_VALUE(None,                     0)
  RDAT_ENUM_VALUE(UAVGloballyCoherent,      1 << 0)
  RDAT_ENUM_VALUE(UAVCounter,               1 << 1)
  RDAT_ENUM_VALUE(UAVRasterizerOrderedView, 1 << 2)
  RDAT_ENUM_VALUE(DynamicIndexing,          1 << 3)
  RDAT_ENUM_VALUE(Atomics64Use,             1 << 4)
  RDAT_ENUM_VALUE(UAVReorderCoherent,       1 << 5)
RDAT_ENUM_END()

RDAT_ENUM_START(DxilShaderStageFlags, uint32_t)
  RDAT_ENUM_VALUE(Pixel, (1 << (uint32_t)hlsl::DXIL::ShaderKind::Pixel))
  RDAT_ENUM_VALUE(Vertex, (1 << (uint32_t)hlsl::DXIL::ShaderKind::Vertex))
  RDAT_ENUM_VALUE(Geometry, (1 << (uint32_t)hlsl::DXIL::ShaderKind::Geometry))
  RDAT_ENUM_VALUE(Hull, (1 << (uint32_t)hlsl::DXIL::ShaderKind::Hull))
  RDAT_ENUM_VALUE(Domain, (1 << (uint32_t)hlsl::DXIL::ShaderKind::Domain))
  RDAT_ENUM_VALUE(Compute, (1 << (uint32_t)hlsl::DXIL::ShaderKind::Compute))
  RDAT_ENUM_VALUE(Library, (1 << (uint32_t)hlsl::DXIL::ShaderKind::Library))
  RDAT_ENUM_VALUE(RayGeneration, (1 << (uint32_t)hlsl::DXIL::ShaderKind::RayGeneration))
  RDAT_ENUM_VALUE(Intersection, (1 << (uint32_t)hlsl::DXIL::ShaderKind::Intersection))
  RDAT_ENUM_VALUE(AnyHit, (1 << (uint32_t)hlsl::DXIL::ShaderKind::AnyHit))
  RDAT_ENUM_VALUE(ClosestHit, (1 << (uint32_t)hlsl::DXIL::ShaderKind::ClosestHit))
  RDAT_ENUM_VALUE(Miss, (1 << (uint32_t)hlsl::DXIL::ShaderKind::Miss))
  RDAT_ENUM_VALUE(Callable, (1 << (uint32_t)hlsl::DXIL::ShaderKind::Callable))
  RDAT_ENUM_VALUE(Mesh, (1 << (uint32_t)hlsl::DXIL::ShaderKind::Mesh))
  RDAT_ENUM_VALUE(Amplification, (1 << (uint32_t)hlsl::DXIL::ShaderKind::Amplification))
  RDAT_ENUM_VALUE(Node, (1 << (uint32_t)hlsl::DXIL::ShaderKind::Node))
RDAT_ENUM_END()

// Low 32-bits of ShaderFeatureInfo from DFCC_FeatureInfo
RDAT_ENUM_START(DxilFeatureInfo1, uint32_t)
  RDAT_ENUM_VALUE(Doubles, 0x0001)
  RDAT_ENUM_VALUE(ComputeShadersPlusRawAndStructuredBuffersViaShader4X, 0x0002)
  RDAT_ENUM_VALUE(UAVsAtEveryStage, 0x0004)
  RDAT_ENUM_VALUE(_64UAVs, 0x0008)
  RDAT_ENUM_VALUE(MinimumPrecision, 0x0010)
  RDAT_ENUM_VALUE(_11_1_DoubleExtensions, 0x0020)
  RDAT_ENUM_VALUE(_11_1_ShaderExtensions, 0x0040)
  RDAT_ENUM_VALUE(LEVEL9ComparisonFiltering, 0x0080)
  RDAT_ENUM_VALUE(TiledResources, 0x0100)
  RDAT_ENUM_VALUE(StencilRef, 0x0200)
  RDAT_ENUM_VALUE(InnerCoverage, 0x0400)
  RDAT_ENUM_VALUE(TypedUAVLoadAdditionalFormats, 0x0800)
  RDAT_ENUM_VALUE(ROVs, 0x1000)
  RDAT_ENUM_VALUE(ViewportAndRTArrayIndexFromAnyShaderFeedingRasterizer, 0x2000)
  RDAT_ENUM_VALUE(WaveOps, 0x4000)
  RDAT_ENUM_VALUE(Int64Ops, 0x8000)
  RDAT_ENUM_VALUE(ViewID, 0x10000)
  RDAT_ENUM_VALUE(Barycentrics, 0x20000)
  RDAT_ENUM_VALUE(NativeLowPrecision, 0x40000)
  RDAT_ENUM_VALUE(ShadingRate, 0x80000)
  RDAT_ENUM_VALUE(Raytracing_Tier_1_1, 0x100000)
  RDAT_ENUM_VALUE(SamplerFeedback, 0x200000)
  RDAT_ENUM_VALUE(AtomicInt64OnTypedResource, 0x400000)
  RDAT_ENUM_VALUE(AtomicInt64OnGroupShared, 0x800000)
  RDAT_ENUM_VALUE(DerivativesInMeshAndAmpShaders, 0x1000000)
  RDAT_ENUM_VALUE(ResourceDescriptorHeapIndexing, 0x2000000)
  RDAT_ENUM_VALUE(SamplerDescriptorHeapIndexing, 0x4000000)
  RDAT_ENUM_VALUE(Reserved, 0x8000000)
  RDAT_ENUM_VALUE(AtomicInt64OnHeapResource, 0x10000000)
  RDAT_ENUM_VALUE(AdvancedTextureOps, 0x20000000)
  RDAT_ENUM_VALUE(WriteableMSAATextures, 0x40000000)
  RDAT_ENUM_VALUE(SampleCmpGradientOrBias, 0x80000000)
RDAT_ENUM_END()

// High 32-bits of ShaderFeatureInfo from DFCC_FeatureInfo
RDAT_ENUM_START(DxilFeatureInfo2, uint32_t)
  RDAT_ENUM_VALUE(ExtendedCommandInfo, 0x1)
  // OptFeatureInfo flags
  RDAT_ENUM_VALUE(Opt_UsesDerivatives, 0x100)
  RDAT_ENUM_VALUE(Opt_RequiresGroup, 0x200)
#if DEF_RDAT_ENUMS == DEF_RDAT_DUMP_IMPL
  static_assert(DXIL::ShaderFeatureInfoCount == 33,
                "otherwise, RDAT_ENUM definition needs updating");
  static_assert(DXIL::OptFeatureInfoCount == 2,
                "otherwise, RDAT_ENUM definition needs updating");
#endif
RDAT_ENUM_END()

#endif // DEF_RDAT_ENUMS

#ifdef DEF_DXIL_ENUMS

// Enums using RDAT_DXIL_ENUM_START use existing definitions of enums, rather
// than redefining the enum locally.  The definition here is mainly to
// implement the ToString function.
// A static_assert under DEF_RDAT_ENUMS == DEF_RDAT_DUMP_IMPL is used to
// check one enum value that would change if the enum were to be updated,
// making sure this definition is updated as well.

RDAT_DXIL_ENUM_START(hlsl::DXIL::ResourceClass, uint32_t)
  RDAT_ENUM_VALUE_NODEF(SRV)
  RDAT_ENUM_VALUE_NODEF(UAV)
  RDAT_ENUM_VALUE_NODEF(CBuffer)
  RDAT_ENUM_VALUE_NODEF(Sampler)
  RDAT_ENUM_VALUE_NODEF(Invalid)
#if DEF_RDAT_ENUMS == DEF_RDAT_DUMP_IMPL
  static_assert((unsigned)hlsl::DXIL::ResourceClass::Invalid == 4,
                "otherwise, RDAT_DXIL_ENUM definition needs updating");
#endif
RDAT_ENUM_END()

RDAT_DXIL_ENUM_START(hlsl::DXIL::ResourceKind, uint32_t)
  RDAT_ENUM_VALUE_NODEF(Invalid)
  RDAT_ENUM_VALUE_NODEF(Texture1D)
  RDAT_ENUM_VALUE_NODEF(Texture2D)
  RDAT_ENUM_VALUE_NODEF(Texture2DMS)
  RDAT_ENUM_VALUE_NODEF(Texture3D)
  RDAT_ENUM_VALUE_NODEF(TextureCube)
  RDAT_ENUM_VALUE_NODEF(Texture1DArray)
  RDAT_ENUM_VALUE_NODEF(Texture2DArray)
  RDAT_ENUM_VALUE_NODEF(Texture2DMSArray)
  RDAT_ENUM_VALUE_NODEF(TextureCubeArray)
  RDAT_ENUM_VALUE_NODEF(TypedBuffer)
  RDAT_ENUM_VALUE_NODEF(RawBuffer)
  RDAT_ENUM_VALUE_NODEF(StructuredBuffer)
  RDAT_ENUM_VALUE_NODEF(CBuffer)
  RDAT_ENUM_VALUE_NODEF(Sampler)
  RDAT_ENUM_VALUE_NODEF(TBuffer)
  RDAT_ENUM_VALUE_NODEF(RTAccelerationStructure)
  RDAT_ENUM_VALUE_NODEF(FeedbackTexture2D)
  RDAT_ENUM_VALUE_NODEF(FeedbackTexture2DArray)
  RDAT_ENUM_VALUE_NODEF(NumEntries)
#if DEF_RDAT_ENUMS == DEF_RDAT_DUMP_IMPL
  static_assert((unsigned)hlsl::DXIL::ResourceKind::NumEntries == 19,
                "otherwise, RDAT_DXIL_ENUM definition needs updating");
#endif
RDAT_ENUM_END()

RDAT_DXIL_ENUM_START(hlsl::DXIL::ShaderKind, uint32_t)
  RDAT_ENUM_VALUE_NODEF(Pixel)
  RDAT_ENUM_VALUE_NODEF(Vertex)
  RDAT_ENUM_VALUE_NODEF(Geometry)
  RDAT_ENUM_VALUE_NODEF(Hull)
  RDAT_ENUM_VALUE_NODEF(Domain)
  RDAT_ENUM_VALUE_NODEF(Compute)
  RDAT_ENUM_VALUE_NODEF(Library)
  RDAT_ENUM_VALUE_NODEF(RayGeneration)
  RDAT_ENUM_VALUE_NODEF(Intersection)
  RDAT_ENUM_VALUE_NODEF(AnyHit)
  RDAT_ENUM_VALUE_NODEF(ClosestHit)
  RDAT_ENUM_VALUE_NODEF(Miss)
  RDAT_ENUM_VALUE_NODEF(Callable)
  RDAT_ENUM_VALUE_NODEF(Mesh)
  RDAT_ENUM_VALUE_NODEF(Amplification)
  RDAT_ENUM_VALUE_NODEF(Node)
  RDAT_ENUM_VALUE_NODEF(Invalid)
#if DEF_RDAT_ENUMS == DEF_RDAT_DUMP_IMPL
  static_assert((unsigned)hlsl::DXIL::ShaderKind::Invalid == 16,
                "otherwise, RDAT_DXIL_ENUM definition needs updating");
#endif
RDAT_ENUM_END()

#endif // DEF_DXIL_ENUMS

#ifdef DEF_RDAT_TYPES

#define RECORD_TYPE RuntimeDataResourceInfo
RDAT_STRUCT_TABLE(RuntimeDataResourceInfo, ResourceTable)
  RDAT_ENUM(uint32_t, hlsl::DXIL::ResourceClass, Class)
  RDAT_ENUM(uint32_t, hlsl::DXIL::ResourceKind, Kind)
  RDAT_VALUE(uint32_t, ID)
  RDAT_VALUE(uint32_t, Space)
  RDAT_VALUE(uint32_t, LowerBound)
  RDAT_VALUE(uint32_t, UpperBound)
  RDAT_STRING(Name)
  RDAT_FLAGS(uint32_t, DxilResourceFlag, Flags)
RDAT_STRUCT_END()
#undef RECORD_TYPE

// ------------ RuntimeDataFunctionInfo ------------

#define RECORD_TYPE RuntimeDataFunctionInfo
RDAT_STRUCT_TABLE(RuntimeDataFunctionInfo, FunctionTable)
  // full function name
  RDAT_STRING(Name)
  // unmangled function name
  RDAT_STRING(UnmangledName)
  // list of global resources used by this function
  RDAT_RECORD_ARRAY_REF(RuntimeDataResourceInfo, Resources)
  // list of external function names this function calls
  RDAT_STRING_ARRAY_REF(FunctionDependencies)
  // Shader type, or library function
  RDAT_ENUM(uint32_t, hlsl::DXIL::ShaderKind, ShaderKind)
  // Payload Size:
  // 1) any/closest hit or miss shader: payload size
  // 2) call shader: parameter size
  RDAT_VALUE(uint32_t, PayloadSizeInBytes)
  // attribute size for closest hit and any hit
  RDAT_VALUE(uint32_t, AttributeSizeInBytes)
  // first 32 bits of feature flag
  RDAT_FLAGS(uint32_t, hlsl::RDAT::DxilFeatureInfo1, FeatureInfo1)
  // second 32 bits of feature flag
  RDAT_FLAGS(uint32_t, hlsl::RDAT::DxilFeatureInfo2, FeatureInfo2)
  // valid shader stage flag.
  RDAT_FLAGS(uint32_t, hlsl::RDAT::DxilShaderStageFlags, ShaderStageFlag)
  // minimum shader target.
  RDAT_VALUE_HEX(uint32_t, MinShaderTarget)

#if DEF_RDAT_TYPES == DEF_RDAT_TYPES_USE_HELPERS
  // void SetFeatureFlags(uint64_t flags) convenience method
  void SetFeatureFlags(uint64_t flags) {
    FeatureInfo1 = flags & 0xffffffff;
    FeatureInfo2 = (flags >> 32) & 0xffffffff;
  }
#endif

#if DEF_RDAT_TYPES == DEF_RDAT_READER_DECL
  // uint64_t GetFeatureFlags() convenience method
  uint64_t GetFeatureFlags() const;
#elif DEF_RDAT_TYPES == DEF_RDAT_READER_IMPL
  // uint64_t GetFeatureFlags() convenience method
  uint64_t RuntimeDataFunctionInfo_Reader::GetFeatureFlags() const {
    return asRecord() ? (((uint64_t)asRecord()->FeatureInfo2 << 32) |
                         (uint64_t)asRecord()->FeatureInfo1)
                      : 0;
  }
#endif

RDAT_STRUCT_END()
#undef RECORD_TYPE

#endif // DEF_RDAT_TYPES


//////////////////////////////////////////////////////////////////////
// The following require validator version 1.8 and above.

// ------------ RuntimeDataFunctionInfo2 dependencies ------------

#ifdef DEF_RDAT_ENUMS

RDAT_ENUM_START(DxilShaderFlags, uint32_t)
  RDAT_ENUM_VALUE(None, 0)
  RDAT_ENUM_VALUE(NodeProgramEntry, 1 << 0)
  // End of values supported by validator version 1.8
  RDAT_ENUM_VALUE(OutputPositionPresent, 1 << 1)
  RDAT_ENUM_VALUE(DepthOutput, 1 << 2)
  RDAT_ENUM_VALUE(SampleFrequency, 1 << 3)
  RDAT_ENUM_VALUE(UsesViewID, 1 << 4)
RDAT_ENUM_END()

RDAT_ENUM_START(NodeFuncAttribKind, uint32_t)
  RDAT_ENUM_VALUE(None, 0)
  RDAT_ENUM_VALUE(ID, 1)
  RDAT_ENUM_VALUE(NumThreads, 2)
  RDAT_ENUM_VALUE(ShareInputOf, 3)
  RDAT_ENUM_VALUE(DispatchGrid, 4)
  RDAT_ENUM_VALUE(MaxRecursionDepth, 5)
  RDAT_ENUM_VALUE(LocalRootArgumentsTableIndex, 6)
  RDAT_ENUM_VALUE(MaxDispatchGrid, 7)
  RDAT_ENUM_VALUE(Reserved_MeshNodePreview1, 8)
  RDAT_ENUM_VALUE(Reserved_MeshNodePreview2, 9)
  RDAT_ENUM_VALUE_NODEF(LastValue)
RDAT_ENUM_END()

RDAT_ENUM_START(NodeAttribKind, uint32_t)
  RDAT_ENUM_VALUE(None, 0)
  RDAT_ENUM_VALUE(OutputID, 1)
  RDAT_ENUM_VALUE(MaxRecords, 2)
  RDAT_ENUM_VALUE(MaxRecordsSharedWith, 3)
  RDAT_ENUM_VALUE(RecordSizeInBytes, 4)
  RDAT_ENUM_VALUE(RecordDispatchGrid, 5)
  RDAT_ENUM_VALUE(OutputArraySize, 6)
  RDAT_ENUM_VALUE(AllowSparseNodes, 7)
  RDAT_ENUM_VALUE(RecordAlignmentInBytes, 8)
  RDAT_ENUM_VALUE_NODEF(LastValue)
RDAT_ENUM_END()

#endif // DEF_RDAT_ENUMS

#ifdef DEF_DXIL_ENUMS

RDAT_DXIL_ENUM_START(hlsl::DXIL::NodeIOKind, uint32_t)
  RDAT_ENUM_VALUE_NODEF(EmptyInput)
  RDAT_ENUM_VALUE_NODEF(NodeOutput)
  RDAT_ENUM_VALUE_NODEF(NodeOutputArray)
  RDAT_ENUM_VALUE_NODEF(EmptyOutput)
  RDAT_ENUM_VALUE_NODEF(EmptyOutputArray)
  RDAT_ENUM_VALUE_NODEF(DispatchNodeInputRecord)
  RDAT_ENUM_VALUE_NODEF(RWDispatchNodeInputRecord)
  RDAT_ENUM_VALUE_NODEF(GroupNodeInputRecords)
  RDAT_ENUM_VALUE_NODEF(RWGroupNodeInputRecords)
  RDAT_ENUM_VALUE_NODEF(ThreadNodeInputRecord)
  RDAT_ENUM_VALUE_NODEF(RWThreadNodeInputRecord)
  RDAT_ENUM_VALUE_NODEF(GroupNodeOutputRecords)
  RDAT_ENUM_VALUE_NODEF(ThreadNodeOutputRecords)
  RDAT_ENUM_VALUE_NODEF(Invalid)
RDAT_ENUM_END()

RDAT_DXIL_ENUM_START(hlsl::DXIL::NodeLaunchType, uint32_t)
  RDAT_ENUM_VALUE_NODEF(Invalid)
  RDAT_ENUM_VALUE_NODEF(Broadcasting)
  RDAT_ENUM_VALUE_NODEF(Coalescing)
  RDAT_ENUM_VALUE_NODEF(Thread)
  RDAT_ENUM_VALUE_NODEF(Reserved_Mesh)
  RDAT_ENUM_VALUE_NODEF(LastEntry)
#if DEF_RDAT_ENUMS == DEF_RDAT_DUMP_IMPL
  static_assert((unsigned)hlsl::DXIL::NodeLaunchType::LastEntry == 5,
                "otherwise, RDAT_DXIL_ENUM definition needs updating");
#endif
RDAT_ENUM_END()

#endif // DEF_DXIL_ENUMS

#ifdef DEF_RDAT_TYPES

#define RECORD_TYPE RecordDispatchGrid
RDAT_STRUCT(RecordDispatchGrid)
  RDAT_VALUE(uint16_t, ByteOffset)
  RDAT_VALUE(uint16_t, ComponentNumAndType) // 0:2 = NumComponents (0-3), 3:15 =
                                          // hlsl::DXIL::ComponentType enum
#if DEF_RDAT_TYPES == DEF_RDAT_TYPES_USE_HELPERS
  uint8_t GetNumComponents() const { return (ComponentNumAndType & 0x3); }
  hlsl::DXIL::ComponentType GetComponentType() const {
    return (hlsl::DXIL::ComponentType)(ComponentNumAndType >> 2);
  }
  void SetNumComponents(uint8_t num) { ComponentNumAndType |= (num & 0x3); }
  void SetComponentType(hlsl::DXIL::ComponentType type) {
    ComponentNumAndType |= (((uint16_t)type) << 2);
  }
#endif
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE NodeID
RDAT_STRUCT_TABLE(NodeID, NodeIDTable)
  RDAT_STRING(Name)
  RDAT_VALUE(uint32_t, Index)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE NodeShaderFuncAttrib
RDAT_STRUCT_TABLE(NodeShaderFuncAttrib, NodeShaderFuncAttribTable)
  RDAT_ENUM(uint32_t, hlsl::RDAT::NodeFuncAttribKind, AttribKind)
  RDAT_UNION()
    RDAT_UNION_IF(ID, getAttribKind() == hlsl::RDAT::NodeFuncAttribKind::ID)
      RDAT_RECORD_REF(NodeID, ID)
    RDAT_UNION_ELIF(NumThreads, getAttribKind() ==
                                    hlsl::RDAT::NodeFuncAttribKind::NumThreads)
      RDAT_INDEX_ARRAY_REF(NumThreads) // ref to array of X, Y, Z.  If < 3
                                       // elements, default value is 1
    RDAT_UNION_ELIF(SharedInput,
                    getAttribKind() ==
                        hlsl::RDAT::NodeFuncAttribKind::ShareInputOf)
      RDAT_RECORD_REF(NodeID, ShareInputOf)
    RDAT_UNION_ELIF(DispatchGrid,
                    getAttribKind() ==
                        hlsl::RDAT::NodeFuncAttribKind::DispatchGrid)
      RDAT_INDEX_ARRAY_REF(DispatchGrid)
    RDAT_UNION_ELIF(MaxRecursionDepth,
                    getAttribKind() ==
                        hlsl::RDAT::NodeFuncAttribKind::MaxRecursionDepth)
      RDAT_VALUE(uint32_t, MaxRecursionDepth)
    RDAT_UNION_ELIF(
        LocalRootArgumentsTableIndex,
        getAttribKind() ==
            hlsl::RDAT::NodeFuncAttribKind::LocalRootArgumentsTableIndex)
      RDAT_VALUE(uint32_t, LocalRootArgumentsTableIndex)
    RDAT_UNION_ELIF(MaxDispatchGrid,
                    getAttribKind() ==
                        hlsl::RDAT::NodeFuncAttribKind::MaxDispatchGrid)
      RDAT_INDEX_ARRAY_REF(MaxDispatchGrid)
    RDAT_UNION_ENDIF()
  RDAT_UNION_END()
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE NodeShaderIOAttrib
RDAT_STRUCT_TABLE(NodeShaderIOAttrib, NodeShaderIOAttribTable)
  RDAT_ENUM(uint32_t, hlsl::RDAT::NodeAttribKind, AttribKind)
  RDAT_UNION()
    RDAT_UNION_IF(ID, getAttribKind() == hlsl::RDAT::NodeAttribKind::OutputID)
      RDAT_RECORD_REF(NodeID, OutputID)
    RDAT_UNION_ELIF(MaxRecords,
                    getAttribKind() == hlsl::RDAT::NodeAttribKind::MaxRecords)
      RDAT_VALUE(uint32_t, MaxRecords)
    RDAT_UNION_ELIF(MaxRecordsSharedWith,
                    getAttribKind() ==
                        hlsl::RDAT::NodeAttribKind::MaxRecordsSharedWith)
      RDAT_VALUE(uint32_t, MaxRecordsSharedWith)
    RDAT_UNION_ELIF(RecordSizeInBytes,
                    getAttribKind() ==
                        hlsl::RDAT::NodeAttribKind::RecordSizeInBytes)
      RDAT_VALUE(uint32_t, RecordSizeInBytes)
    RDAT_UNION_ELIF(RecordDispatchGrid,
                    getAttribKind() ==
                        hlsl::RDAT::NodeAttribKind::RecordDispatchGrid)
      RDAT_RECORD_VALUE(RecordDispatchGrid, RecordDispatchGrid)
    RDAT_UNION_ELIF(OutputArraySize,
                    getAttribKind() ==
                        hlsl::RDAT::NodeAttribKind::OutputArraySize)
      RDAT_VALUE(uint32_t, OutputArraySize)
    RDAT_UNION_ELIF(AllowSparseNodes,
                    getAttribKind() ==
                        hlsl::RDAT::NodeAttribKind::AllowSparseNodes)
      RDAT_VALUE(uint32_t, AllowSparseNodes)
    RDAT_UNION_ELIF(RecordAlignmentInBytes,
                    getAttribKind() ==
                        hlsl::RDAT::NodeAttribKind::RecordAlignmentInBytes)
      RDAT_VALUE(uint32_t, RecordAlignmentInBytes)
    RDAT_UNION_ENDIF()
  RDAT_UNION_END()
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE IONode
  RDAT_STRUCT_TABLE(IONode, IONodeTable)
  // Required field
  RDAT_VALUE(uint32_t, IOFlagsAndKind)
  // Optional fields
  RDAT_RECORD_ARRAY_REF(NodeShaderIOAttrib, Attribs)
#if DEF_RDAT_TYPES == DEF_RDAT_TYPES_USE_HELPERS
  uint32_t GetIOFlags() const {
    return IOFlagsAndKind & (uint32_t)DXIL::NodeIOFlags::NodeFlagsMask;
  }
  hlsl::DXIL::NodeIOKind GetIOKind() const {
    return (hlsl::DXIL::NodeIOKind)(IOFlagsAndKind &
                                    (uint32_t)DXIL::NodeIOFlags::NodeIOKindMask);
  }
  void SetIOFlags(uint32_t flags) { IOFlagsAndKind |= flags; }
  void SetIOKind(hlsl::DXIL::NodeIOKind kind) {
    IOFlagsAndKind |= (uint32_t)kind;
  }
#endif
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE NodeShaderInfo
RDAT_STRUCT_TABLE(NodeShaderInfo, NodeShaderInfoTable)
  // Function Attributes
  RDAT_ENUM(uint32_t, hlsl::DXIL::NodeLaunchType, LaunchType)
  RDAT_VALUE(uint32_t, GroupSharedBytesUsed)
  RDAT_RECORD_ARRAY_REF(NodeShaderFuncAttrib, Attribs)
  RDAT_RECORD_ARRAY_REF(IONode, Outputs)
  RDAT_RECORD_ARRAY_REF(IONode, Inputs)

RDAT_STRUCT_END()
#undef RECORD_TYPE

// ------------ RuntimeDataFunctionInfo2 ------------

#define RECORD_TYPE RuntimeDataFunctionInfo2
RDAT_STRUCT_TABLE_DERIVED(RuntimeDataFunctionInfo2, RuntimeDataFunctionInfo,
                          FunctionTable)

  // 128 lanes is maximum that could be supported by HLSL
  RDAT_VALUE(uint8_t, MinimumExpectedWaveLaneCount) // 0 = none specified
  RDAT_VALUE(uint8_t, MaximumExpectedWaveLaneCount) // 0 = none specified
  RDAT_FLAGS(uint16_t, hlsl::RDAT::DxilShaderFlags, ShaderFlags)

  RDAT_UNION()
    RDAT_UNION_IF(RawShaderRef,
                    (getShaderKind() == hlsl::DXIL::ShaderKind::Invalid))
      RDAT_VALUE(uint32_t, RawShaderRef)
    RDAT_UNION_ELIF(Node, (getShaderKind() == hlsl::DXIL::ShaderKind::Node))
      RDAT_RECORD_REF(NodeShaderInfo, Node)
    // End of values supported by validator version 1.8
    RDAT_UNION_ELIF(VS, (getShaderKind() == hlsl::DXIL::ShaderKind::Vertex))
      RDAT_RECORD_REF(VSInfo, VS)
    RDAT_UNION_ELIF(PS, (getShaderKind() == hlsl::DXIL::ShaderKind::Pixel))
      RDAT_RECORD_REF(PSInfo, PS)
    RDAT_UNION_ELIF(HS, (getShaderKind() == hlsl::DXIL::ShaderKind::Hull))
      RDAT_RECORD_REF(HSInfo, HS)
    RDAT_UNION_ELIF(DS, (getShaderKind() == hlsl::DXIL::ShaderKind::Domain))
      RDAT_RECORD_REF(DSInfo, DS)
    RDAT_UNION_ELIF(GS, (getShaderKind() == hlsl::DXIL::ShaderKind::Geometry))
      RDAT_RECORD_REF(GSInfo, GS)
    RDAT_UNION_ELIF(CS, (getShaderKind() == hlsl::DXIL::ShaderKind::Compute))
      RDAT_RECORD_REF(CSInfo, CS)
    RDAT_UNION_ELIF(MS, (getShaderKind() == hlsl::DXIL::ShaderKind::Mesh))
      RDAT_RECORD_REF(MSInfo, MS)
    RDAT_UNION_ELIF(AS,
                    (getShaderKind() == hlsl::DXIL::ShaderKind::Amplification))
      RDAT_RECORD_REF(ASInfo, AS)
    RDAT_UNION_ENDIF()
  RDAT_UNION_END()

RDAT_STRUCT_END()
#undef RECORD_TYPE

#endif // DEF_RDAT_TYPES


///////////////////////////////////////////////////////////////////////////////
// The following are experimental, and are not currently supported on any
// validator version.

#ifdef DEF_DXIL_ENUMS

RDAT_DXIL_ENUM_START(hlsl::DXIL::SemanticKind, uint32_t)
  /* <py::lines('SemanticKind-ENUM')>hctdb_instrhelp.get_rdat_enum_decl("SemanticKind", nodef=True)</py>*/
  // SemanticKind-ENUM:BEGIN
  RDAT_ENUM_VALUE_NODEF(Arbitrary)
  RDAT_ENUM_VALUE_NODEF(VertexID)
  RDAT_ENUM_VALUE_NODEF(InstanceID)
  RDAT_ENUM_VALUE_NODEF(Position)
  RDAT_ENUM_VALUE_NODEF(RenderTargetArrayIndex)
  RDAT_ENUM_VALUE_NODEF(ViewPortArrayIndex)
  RDAT_ENUM_VALUE_NODEF(ClipDistance)
  RDAT_ENUM_VALUE_NODEF(CullDistance)
  RDAT_ENUM_VALUE_NODEF(OutputControlPointID)
  RDAT_ENUM_VALUE_NODEF(DomainLocation)
  RDAT_ENUM_VALUE_NODEF(PrimitiveID)
  RDAT_ENUM_VALUE_NODEF(GSInstanceID)
  RDAT_ENUM_VALUE_NODEF(SampleIndex)
  RDAT_ENUM_VALUE_NODEF(IsFrontFace)
  RDAT_ENUM_VALUE_NODEF(Coverage)
  RDAT_ENUM_VALUE_NODEF(InnerCoverage)
  RDAT_ENUM_VALUE_NODEF(Target)
  RDAT_ENUM_VALUE_NODEF(Depth)
  RDAT_ENUM_VALUE_NODEF(DepthLessEqual)
  RDAT_ENUM_VALUE_NODEF(DepthGreaterEqual)
  RDAT_ENUM_VALUE_NODEF(StencilRef)
  RDAT_ENUM_VALUE_NODEF(DispatchThreadID)
  RDAT_ENUM_VALUE_NODEF(GroupID)
  RDAT_ENUM_VALUE_NODEF(GroupIndex)
  RDAT_ENUM_VALUE_NODEF(GroupThreadID)
  RDAT_ENUM_VALUE_NODEF(TessFactor)
  RDAT_ENUM_VALUE_NODEF(InsideTessFactor)
  RDAT_ENUM_VALUE_NODEF(ViewID)
  RDAT_ENUM_VALUE_NODEF(Barycentrics)
  RDAT_ENUM_VALUE_NODEF(ShadingRate)
  RDAT_ENUM_VALUE_NODEF(CullPrimitive)
  RDAT_ENUM_VALUE_NODEF(StartVertexLocation)
  RDAT_ENUM_VALUE_NODEF(StartInstanceLocation)
  RDAT_ENUM_VALUE_NODEF(Invalid)
  // SemanticKind-ENUM:END
RDAT_ENUM_END()

RDAT_DXIL_ENUM_START(hlsl::DXIL::ComponentType, uint32_t)
  RDAT_ENUM_VALUE_NODEF(Invalid)
  RDAT_ENUM_VALUE_NODEF(I1)
  RDAT_ENUM_VALUE_NODEF(I16)
  RDAT_ENUM_VALUE_NODEF(U16)
  RDAT_ENUM_VALUE_NODEF(I32)
  RDAT_ENUM_VALUE_NODEF(U32)
  RDAT_ENUM_VALUE_NODEF(I64)
  RDAT_ENUM_VALUE_NODEF(U64)
  RDAT_ENUM_VALUE_NODEF(F16)
  RDAT_ENUM_VALUE_NODEF(F32)
  RDAT_ENUM_VALUE_NODEF(F64)
  RDAT_ENUM_VALUE_NODEF(SNormF16)
  RDAT_ENUM_VALUE_NODEF(UNormF16)
  RDAT_ENUM_VALUE_NODEF(SNormF32)
  RDAT_ENUM_VALUE_NODEF(UNormF32)
  RDAT_ENUM_VALUE_NODEF(SNormF64)
  RDAT_ENUM_VALUE_NODEF(UNormF64)
  RDAT_ENUM_VALUE_NODEF(PackedS8x32)
  RDAT_ENUM_VALUE_NODEF(PackedU8x32)
  RDAT_ENUM_VALUE_NODEF(U8)
  RDAT_ENUM_VALUE_NODEF(I8)
  RDAT_ENUM_VALUE_NODEF(F8_E4M3)
  RDAT_ENUM_VALUE_NODEF(F8_E5M2)
  RDAT_ENUM_VALUE_NODEF(LastEntry)
#if DEF_RDAT_ENUMS == DEF_RDAT_DUMP_IMPL
  static_assert((unsigned)hlsl::DXIL::ComponentType::LastEntry == 23,
                "otherwise, RDAT_DXIL_ENUM definition needs updating");
#endif
RDAT_ENUM_END()

RDAT_DXIL_ENUM_START(hlsl::DXIL::InterpolationMode, uint32_t)
  RDAT_ENUM_VALUE_NODEF(Undefined)
  RDAT_ENUM_VALUE_NODEF(Constant)
  RDAT_ENUM_VALUE_NODEF(Linear)
  RDAT_ENUM_VALUE_NODEF(LinearCentroid)
  RDAT_ENUM_VALUE_NODEF(LinearNoperspective)
  RDAT_ENUM_VALUE_NODEF(LinearNoperspectiveCentroid)
  RDAT_ENUM_VALUE_NODEF(LinearSample)
  RDAT_ENUM_VALUE_NODEF(LinearNoperspectiveSample)
  RDAT_ENUM_VALUE_NODEF(Invalid)
#if DEF_RDAT_ENUMS == DEF_RDAT_DUMP_IMPL
  static_assert((unsigned)hlsl::DXIL::InterpolationMode::Invalid == 8,
                "otherwise, RDAT_DXIL_ENUM definition needs updating");
#endif
RDAT_ENUM_END()

#endif // DEF_DXIL_ENUMS

#ifdef DEF_RDAT_TYPES

#define RECORD_TYPE SignatureElement
RDAT_STRUCT_TABLE(SignatureElement, SignatureElementTable)
  RDAT_STRING(SemanticName)
  RDAT_INDEX_ARRAY_REF(SemanticIndices) // Rows = SemanticIndices.Count()
  RDAT_ENUM(uint8_t, hlsl::DXIL::SemanticKind, SemanticKind)
  RDAT_ENUM(uint8_t, hlsl::DXIL::ComponentType, ComponentType)
  RDAT_ENUM(uint8_t, hlsl::DXIL::InterpolationMode, InterpolationMode)
  RDAT_VALUE(
      uint8_t,
      StartRow) // Starting row of packed location if allocated, otherwise 0xFF
  // TODO: use struct with bitfields or accessors for ColsAndStream and
  // UsageAndDynIndexMasks
  RDAT_VALUE(uint8_t, ColsAndStream) // 0:2 = (Cols-1) (0-3), 2:4 = StartCol
                                     // (0-3), 4:6 = OutputStream (0-3)
  RDAT_VALUE(uint8_t,
             UsageAndDynIndexMasks) // 0:4 = UsageMask, 4:8 = DynamicIndexMask
#if DEF_RDAT_TYPES == DEF_RDAT_TYPES_USE_HELPERS
  uint8_t GetCols() const { return (ColsAndStream & 3) + 1; }
  uint8_t GetStartCol() const { return (ColsAndStream >> 2) & 3; }
  uint8_t GetOutputStream() const { return (ColsAndStream >> 4) & 3; }
  uint8_t GetUsageMask() const { return UsageAndDynIndexMasks & 0xF; }
  uint8_t GetDynamicIndexMask() const {
    return (UsageAndDynIndexMasks >> 4) & 0xF;
  }
  void SetCols(unsigned cols) {
    ColsAndStream &= ~3;
    ColsAndStream |= (cols - 1) & 3;
  }
  void SetStartCol(unsigned col) {
    ColsAndStream &= ~(3 << 2);
    ColsAndStream |= (col & 3) << 2;
  }
  void SetOutputStream(unsigned stream) {
    ColsAndStream &= ~(3 << 4);
    ColsAndStream |= (stream & 3) << 4;
  }
  void SetUsageMask(unsigned mask) {
    UsageAndDynIndexMasks &= ~0xF;
    UsageAndDynIndexMasks |= mask & 0xF;
  }
  void SetDynamicIndexMask(unsigned mask) {
    UsageAndDynIndexMasks &= ~(0xF << 4);
    UsageAndDynIndexMasks |= (mask & 0xF) << 4;
  }
#endif
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE VSInfo
RDAT_STRUCT_TABLE(VSInfo, VSInfoTable)
  RDAT_RECORD_ARRAY_REF(SignatureElement, SigInputElements)
  RDAT_RECORD_ARRAY_REF(SignatureElement, SigOutputElements)
  RDAT_BYTES(ViewIDOutputMask)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE PSInfo
RDAT_STRUCT_TABLE(PSInfo, PSInfoTable)
  RDAT_RECORD_ARRAY_REF(SignatureElement, SigInputElements)
  RDAT_RECORD_ARRAY_REF(SignatureElement, SigOutputElements)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE HSInfo
RDAT_STRUCT_TABLE(HSInfo, HSInfoTable)
  RDAT_RECORD_ARRAY_REF(SignatureElement, SigInputElements)
  RDAT_RECORD_ARRAY_REF(SignatureElement, SigOutputElements)
  RDAT_RECORD_ARRAY_REF(SignatureElement, SigPatchConstOutputElements)
  RDAT_BYTES(ViewIDOutputMask)
  RDAT_BYTES(ViewIDPatchConstOutputMask)
  RDAT_BYTES(InputToOutputMasks)
  RDAT_BYTES(InputToPatchConstOutputMasks)
  RDAT_VALUE(uint8_t, InputControlPointCount)
  RDAT_VALUE(uint8_t, OutputControlPointCount)
  RDAT_VALUE(uint8_t, TessellatorDomain)
  RDAT_VALUE(uint8_t, TessellatorOutputPrimitive)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE DSInfo
RDAT_STRUCT_TABLE(DSInfo, DSInfoTable)
  RDAT_RECORD_ARRAY_REF(SignatureElement, SigInputElements)
  RDAT_RECORD_ARRAY_REF(SignatureElement, SigOutputElements)
  RDAT_RECORD_ARRAY_REF(SignatureElement, SigPatchConstInputElements)
  RDAT_BYTES(ViewIDOutputMask)
  RDAT_BYTES(InputToOutputMasks)
  RDAT_BYTES(PatchConstInputToOutputMasks)
  RDAT_VALUE(uint8_t, InputControlPointCount)
  RDAT_VALUE(uint8_t, TessellatorDomain)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE GSInfo
RDAT_STRUCT_TABLE(GSInfo, GSInfoTable)
  RDAT_RECORD_ARRAY_REF(SignatureElement, SigInputElements)
  RDAT_RECORD_ARRAY_REF(SignatureElement, SigOutputElements)
  RDAT_BYTES(ViewIDOutputMask)
  RDAT_BYTES(InputToOutputMasks)
  RDAT_VALUE(uint8_t, InputPrimitive)
  RDAT_VALUE(uint8_t, OutputTopology)
  RDAT_VALUE(uint8_t, MaxVertexCount)
  RDAT_VALUE(uint8_t, OutputStreamMask)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE CSInfo
RDAT_STRUCT_TABLE(CSInfo, CSInfoTable)
  RDAT_INDEX_ARRAY_REF(NumThreads) // ref to array of X, Y, Z.  If < 3 elements,
                                   // default value is 1
  RDAT_VALUE(uint32_t, GroupSharedBytesUsed)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE MSInfo
RDAT_STRUCT_TABLE(MSInfo, MSInfoTable)
  RDAT_RECORD_ARRAY_REF(SignatureElement, SigOutputElements)
  RDAT_RECORD_ARRAY_REF(SignatureElement, SigPrimOutputElements)
  RDAT_BYTES(ViewIDOutputMask)
  RDAT_BYTES(ViewIDPrimOutputMask)
  RDAT_INDEX_ARRAY_REF(NumThreads) // ref to array of X, Y, Z.  If < 3 elements,
                                   // default value is 1
  RDAT_VALUE(uint32_t, GroupSharedBytesUsed)
  RDAT_VALUE(uint32_t, GroupSharedBytesDependentOnViewID)
  RDAT_VALUE(uint32_t, PayloadSizeInBytes)
  RDAT_VALUE(uint16_t, MaxOutputVertices)
  RDAT_VALUE(uint16_t, MaxOutputPrimitives)
  RDAT_VALUE(uint8_t, MeshOutputTopology)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE ASInfo
RDAT_STRUCT_TABLE(ASInfo, ASInfoTable)
  RDAT_INDEX_ARRAY_REF(NumThreads) // ref to array of X, Y, Z.  If < 3 elements,
                                   // default value is 1
  RDAT_VALUE(uint32_t, GroupSharedBytesUsed)
  RDAT_VALUE(uint32_t, PayloadSizeInBytes)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#endif // DEF_RDAT_TYPES

// clang-format on
