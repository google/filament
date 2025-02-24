/* Copyright (c) 2021-2025 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * The Shader Module file is in charge of all things around creating and parsing an internal representation of a shader module
 */

#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "state_tracker/shader_instruction.h"
#include "state_tracker/state_object.h"
#include "state_tracker/sampler_state.h"
#include <spirv/unified1/spirv.hpp>

namespace vvl {
class Pipeline;
}  // namespace vvl

namespace spirv {
struct EntryPoint;
struct Module;

static constexpr uint32_t kInvalidValue = std::numeric_limits<uint32_t>::max();

// Need to find a way to know if actually array length of zero, or a runtime array.
static constexpr uint32_t kRuntimeArray = std::numeric_limits<uint32_t>::max();

// This is the common info for both OpDecorate and OpMemberDecorate
// Used to keep track of all decorations applied to any instruction
struct DecorationBase {
    enum FlagBit {
        patch_bit = 1 << 0,
        block_bit = 1 << 1,
        buffer_block_bit = 1 << 2,
        nonwritable_bit = 1 << 3,
        nonreadable_bit = 1 << 4,
        per_vertex_bit = 1 << 5,
        passthrough_bit = 1 << 6,
        aliased_bit = 1 << 7,
        input_attachment_bit = 1 << 8,
        per_task_nv = 1 << 9,
        per_primitive_ext = 1 << 10,
    };

    // bits to know if things have been set or not by a Decoration
    uint32_t flags = 0;

    // When being used as an User-defined Variable (input, output, rtx)
    uint32_t location = kInvalidValue;
    // Component and Index are optional and spec says it is 0 if not defined
    uint32_t component = 0;
    uint32_t index = 0;

    uint32_t offset = 0;

    // A given object can only have a single BuiltIn OpDecoration
    uint32_t builtin = kInvalidValue;

    void Add(uint32_t decoration, uint32_t value);
    bool Has(FlagBit flag_bit) const { return (flags & flag_bit) != 0; }
};

// subset only for OpDecorate
// Can't have nested structs with OpMemberDecorate, this class prevents accidently creating a 2nd level of member decorations,
struct DecorationSet : public DecorationBase {
    // For descriptors
    uint32_t set = 0;
    uint32_t binding = 0;

    // Value of InputAttachmentIndex the variable starts
    uint32_t input_attachment_index_start = kInvalidValue;

    // <index into struct, DecorationBase>
    vvl::unordered_map<uint32_t, DecorationBase> member_decorations;

    void Add(uint32_t decoration, uint32_t value);
    bool HasAnyBuiltIn() const;
    bool HasInMember(FlagBit flag_bit) const;
    bool AllMemberHave(FlagBit flag_bit) const;
};

// Tracking of OpExecutionMode / OpExecutionModeId values
struct ExecutionModeSet {
    enum FlagBit {
        output_points_bit = 1 << 0,
        point_mode_bit = 1 << 1,
        post_depth_coverage_bit = 1 << 2,
        local_size_bit = 1 << 3,
        local_size_id_bit = 1 << 4,
        iso_lines_bit = 1 << 5,
        xfb_bit = 1 << 6,
        early_fragment_test_bit = 1 << 7,
        subgroup_uniform_control_flow_bit = 1 << 8,

        signed_zero_inf_nan_preserve_width_16 = 1 << 9,
        signed_zero_inf_nan_preserve_width_32 = 1 << 10,
        signed_zero_inf_nan_preserve_width_64 = 1 << 11,
        denorm_preserve_width_16 = 1 << 12,
        denorm_preserve_width_32 = 1 << 13,
        denorm_preserve_width_64 = 1 << 14,
        denorm_flush_to_zero_width_16 = 1 << 15,
        denorm_flush_to_zero_width_32 = 1 << 16,
        denorm_flush_to_zero_width_64 = 1 << 17,
        rounding_mode_rte_width_16 = 1 << 18,
        rounding_mode_rte_width_32 = 1 << 19,
        rounding_mode_rte_width_64 = 1 << 20,
        rounding_mode_rtz_width_16 = 1 << 21,
        rounding_mode_rtz_width_32 = 1 << 22,
        rounding_mode_rtz_width_64 = 1 << 23,

        depth_replacing_bit = 1 << 24,
        stencil_ref_replacing_bit = 1 << 25,

        fp_fast_math_default = 1 << 26,

        derivative_group_linear = 1 << 27,
        derivative_group_quads = 1 << 28,
    };

    // bits to know if things have been set or not by a Decoration
    uint32_t flags = 0;

    VkPrimitiveTopology input_primitive_topology = VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
    VkPrimitiveTopology primitive_topology = VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;

    // SPIR-V spec says only LocalSize or LocalSizeId can be used, so can share
    uint32_t local_size_x = kInvalidValue;
    uint32_t local_size_y = kInvalidValue;
    uint32_t local_size_z = kInvalidValue;

    uint32_t output_vertices = vvl::kU32Max;
    uint32_t output_primitives = 0;
    uint32_t invocations = 0;

    uint32_t tessellation_subdivision = 0;
    uint32_t tessellation_orientation = 0;
    uint32_t tessellation_spacing = 0;

    void Add(const Instruction &insn);
    bool Has(FlagBit flag_bit) const { return (flags & flag_bit) != 0; }
};

struct AtomicInstructionInfo {
    uint32_t storage_class;
    uint32_t bit_width;
    uint32_t type;  // ex. OpTypeInt
    uint32_t vector_size = 0;  // 0 for scalar, otherwise number of components
};

// This info *could* be found/saved in TypeStructInfo, but since
//  - Only a few places (Push Constants, workgroup size) use this
//  - It is only good when you know there are no nested strcuts
// we only get this info when needed, not for every struct
struct TypeStructSize {
    uint32_t offset;  // where first member is
    // This is the "padded" size, if you wanted the packed size, use GetTypeBytesSize(struct_type)
    uint32_t size;  // total size of block
};

// Contains all the details for a OpTypeStruct
struct TypeStructInfo {
    const uint32_t id;
    const uint32_t length;  // number of elements
    const DecorationSet &decorations;

    // data about each member in struct
    struct Member {
        uint32_t id;
        const Instruction *insn;
        const DecorationBase *decorations;
        std::shared_ptr<const TypeStructInfo> type_struct_info;  // for nested structs
    };
    std::vector<Member> members;

    TypeStructInfo(const Module &module_state, const Instruction &struct_insn);

    TypeStructSize GetSize(const Module &module_state) const;
};

namespace AccessBit {
const uint32_t empty = 0;
const uint32_t read = 1 << 0;
const uint32_t write = 1 << 1;
const uint32_t atomic_read = 1 << 2;
const uint32_t atomic_write = 1 << 3;
const uint32_t image_read = 1 << 4;
const uint32_t image_write = 1 << 5;

constexpr uint32_t atomic_mask = atomic_read | atomic_write;
constexpr uint32_t image_mask = image_read | image_write;
constexpr uint32_t read_mask = read | atomic_read | image_read;
constexpr uint32_t write_mask = write | atomic_write | image_write;
}  // namespace AccessBit

// Mapping of < variable ID, AccessBit >
using VariableAccessMap = vvl::unordered_map<uint32_t, uint32_t>;

// Track all paths from %param to %arg so can walk back functions
//
// %arg   = OpVariable
// %call  = OpFunctionCall %result %func %arg
// %param = OpFunctionParameter
//
// < %param, vector<%arg> >
using FuncParameterMap = vvl::unordered_map<uint32_t, std::vector<uint32_t>>;

// Represents the OpImage* instructions and how it maps to the variable
// This is created in the Module but then used with VariableBase objects
struct ImageAccess {
    const Instruction &image_insn;  // OpImage*
    std::vector<const Instruction *> variable_image_insn;
    // If there is a OpSampledImage there will also be a sampler variable
    std::vector<const Instruction *> variable_sampler_insn;
    // incase uncaught set of SPIR-V instruction is found, skips validating instead of crashing
    bool valid_access = true;

    bool is_dref = false;
    bool is_sampler_implicitLod_dref_proj = false;
    bool is_sampler_sampled = false;  // OpImageSample* or OpImageSparseSample*
    bool is_not_sampler_sampled = false;
    bool is_sampler_bias_offset = false;
    bool is_sampler_offset = false;  // ConstOffset or Offset (not ConstOffsets)
    bool is_sign_extended = false;
    bool is_zero_extended = false;
    uint32_t access_mask = AccessBit::empty;

    uint32_t image_access_chain_index = kInvalidValue;    // OpAccessChain's Index 0
    uint32_t sampler_access_chain_index = kInvalidValue;  // OpAccessChain's Index 0
    uint32_t texel_component_count = kInvalidValue;

    ImageAccess(const Module &module_state, const Instruction &image_insn, const FuncParameterMap &func_parameter_map);
};

// <Image OpVariable Result ID, [ImageAccess, ImageAccess, etc] > - used for faster lookup
// Many ImageAccess can point to a single Image Variable
using ImageAccessMap = vvl::unordered_map<uint32_t, std::vector<std::shared_ptr<const ImageAccess>>>;
// < Variable ID, [ OpAccessChain ] >
// Allows for grouping the access chains by which variables they are actually accessing
using AccessChainVariableMap = vvl::unordered_map<uint32_t, std::vector<const Instruction *>>;
// Mapping of OpName instructions
using DebugNameMap = vvl::unordered_map<uint32_t, const Instruction *>;

// A slot is a <Location, Component> mapping
struct InterfaceSlot {
    // A Location is made up of 4 Components
    // Example: Location 2, Component 1
    // L0 : [ C0, C1, C2, C3 ]
    // L1 : [ C0, C1, C2, C3 ]
    // L2 : [ C0, C1, C2, C3 ]
    //            ^
    // index == 9 == (Location * 4) + Component
    const uint32_t slot = 0;  // default

    // Information about the variable type
    // Easier to find this information once then re-look each time (mainly for structs)
    const uint32_t type = 0;  // Opcode of OpType*
    const uint32_t bit_width = 0;

    uint32_t Location() const { return slot / 4; }
    uint32_t Component() const { return slot % 4; }
    std::string Describe() const;
    // Having a single uint32_t slot allows a 64-bit Vec3 to pass in (Loc 0, Comp 5) and have it automatically mean (Loc 1, Comp 1)
    InterfaceSlot(uint32_t location, uint32_t component, uint32_t type, uint32_t bit_width)
        : slot(GetSlotValue(location, component)), type(type), bit_width(bit_width) {}
    InterfaceSlot(uint32_t slot, uint32_t type, uint32_t bit_width) : slot(slot), type(type), bit_width(bit_width) {}

    bool operator<(const InterfaceSlot &rhs) const { return slot < rhs.slot; }
    bool operator==(const InterfaceSlot &rhs) const { return slot == rhs.slot; }
    struct Hash {
        std::size_t operator()(const InterfaceSlot &object) const { return object.slot; }
    };

    uint32_t GetSlotValue(uint32_t location, uint32_t component) { return (location * 4) + component; }
};

// Represents the Image formats that can map to a SPIR-V format
enum NumericType {
    NumericTypeUnknown = 0,  // In case image is not used
    NumericTypeFloat = 1,    // UNORM, SNORM, FLOAT, USCALED, SSCALED, SRGB -- anything we consider float in the shader
    NumericTypeSint = 2,
    NumericTypeUint = 4,
};
uint32_t GetFormatType(VkFormat format);
char const *string_NumericType(uint32_t type);

// Common info needed for all OpVariable
struct VariableBase {
    const uint32_t id;
    const uint32_t type_id;
    const spv::StorageClass storage_class;
    const DecorationSet &decorations;
    std::shared_ptr<const TypeStructInfo> type_struct_info;  // null if no struct type
    // The variable may have different access for a given entrypoint
    uint32_t access_mask;  // AccessBit
    const VkShaderStageFlagBits stage;
    VariableBase(const Module &module_state, const Instruction &insn, VkShaderStageFlagBits stage,
                 const VariableAccessMap &variable_access_map, const DebugNameMap &debug_name_map);

    // When no SPIR-V debug info is used, this will be empty strings
    // We need to store a std::string since the original SPIR-V string will be gone when we need to print this in an error message
    const std::string debug_name;  // OpName
    std::string DescribeDescriptor() const;

    // These are helpers to show how the variable will be STATICALLY accessed.
    // (It would require a lot of GPU-AV overhead to detect if the access is dynamic and that level of fine control is currently not
    // required) While SPIR-V has its own terms/concepts, the following is designed to match the Vulkan Spec.
    // -------
    // Accessed == (read | write | atomic)
    // It is possible to have descriptors/inout/push constant/etc declared but never used
    bool IsAccessed() const { return access_mask != AccessBit::empty; }
    // Atomics are really both a read/write, but some HW doesn't support atomic on all int/float bit-width
    bool IsAtomic() const { return access_mask & AccessBit::atomic_mask; }
    // Read/Write here refer to the variable itself. For a buffer this means the memory has been accessed. For an Image, this only
    // means the descriptor itself has been accessed
    bool IsReadFrom() const { return access_mask & AccessBit::read_mask; }
    bool IsWrittenTo() const { return access_mask & AccessBit::write_mask; }
    // Images are special and will first have a read/write to the descriptor, then an Image Operation to the image memory itself.
    // - Some operations such as ImageSize() will read data from the descriptor and never actually read the image memory (it would
    // return false for IsImageReadFrom()).
    // - A storage image is always "loaded" before it is written, but it will only return true for IsImageReadFrom() if the image
    // memory was read
    bool IsImageReadFrom() const { return access_mask & AccessBit::image_read; }
    bool IsImageWrittenTo() const { return access_mask & AccessBit::image_write; }

  private:
    static const char *FindDebugName(const VariableBase &variable, const DebugNameMap &debug_name_map);
};

// These are Input/Output OpVariable that go in-between stages
// (also for example the input to a Vertex and output of the Fragment).
// These are always ints/floats (not images or samplers).
// Besides the input vertex binding, all of these are fully known at pipeline creation time
//
// These include both BuiltIns and User Defined, while there are difference in member variables, the variables are needed for the
// common logic so its easier using the same object in the end
struct StageInterfaceVariable : public VariableBase {
    // Only will be true in BuiltIns
    const bool is_patch;
    const bool is_per_vertex;   // VK_KHR_fragment_shader_barycentric
    const bool is_per_task_nv;  // VK_NV_mesh_shader

    const bool is_array_interface;
    uint32_t array_size = 1;  // flatten size of all dimensions; 1 if no array
    const Instruction &base_type;
    const bool is_builtin;
    bool nested_struct;

    const std::vector<InterfaceSlot> interface_slots;  // Only for User Defined variables
    const std::vector<uint32_t> builtin_block;
    uint32_t total_builtin_components = 0;

    StageInterfaceVariable(const Module &module_state, const Instruction &insn, VkShaderStageFlagBits stage,
                           const VariableAccessMap &variable_access_map, const DebugNameMap &debug_name_map);

  protected:
    static bool IsPerTaskNV(const StageInterfaceVariable &variable);
    static bool IsArrayInterface(const StageInterfaceVariable &variable);
    static const Instruction &FindBaseType(StageInterfaceVariable &variable, const Module &module_state);
    static bool IsBuiltin(const StageInterfaceVariable &variable, const Module &module_state);
    static std::vector<InterfaceSlot> GetInterfaceSlots(StageInterfaceVariable &variable, const Module &module_state);
    static std::vector<uint32_t> GetBuiltinBlock(const StageInterfaceVariable &variable, const Module &module_state);
    static uint32_t GetBuiltinComponents(const StageInterfaceVariable &variable, const Module &module_state);
};

// vkspec.html#interfaces-resources describes 'Shader Resource Interface'
// These are the OpVariable attached to descriptors.
// The slots are known at Pipeline creation time, but the type images/sampler/etc is
// not known until the descriptors are bound.
// The main purpose of this struct is to track what operations are statically done so
// at draw/submit time we can cross reference with the last bound descriptor.
struct ResourceInterfaceVariable : public VariableBase {
    // If the type is a OpTypeArray save the length
    // Will be kRuntimeArray (non-zero) for runtime arrays
    uint32_t array_length;

    bool is_sampled_image;  // OpTypeSampledImage

    // List of samplers that sample a given image. The index of array is index of image.
    std::vector<vvl::unordered_set<SamplerUsedByImage>> samplers_used_by_image;

    // For storage images - list of Texel component length the OpImageWrite
    std::vector<uint32_t> write_without_formats_component_count_list;

    // A variable can have an array of indexes, need to track which are written to
    // can't use bitset because number of indexes isn't known until runtime
    // This array will match the OpTypeArray and not consider the InputAttachmentIndex
    std::vector<bool> input_attachment_index_read;

    // Type once array/pointer are stripped
    // most likly will be OpTypeImage, OpTypeStruct, OpTypeSampler, or OpTypeAccelerationStructureKHR
    const Instruction &base_type;

    // True if the Resource variable itself is runtime descriptor array
    // Online example to showcase various arrays we do/don't care about here https://godbolt.org/z/h9jhsKaPn
    bool is_runtime_descriptor_array;

    // Sampled Type width of the OpTypeImage the variable points to, 0 if doesn't use the image
    const uint32_t image_sampled_type_width;

    // All info regarding what will be validated from requirements imposed by the pipeline on a descriptor. These
    // can't be checked at pipeline creation time as they depend on the Image or ImageView bound.
    // That is perf-critical code and hashing if 2 variables have same info provides a 20% perf bonus
    struct Info {
        NumericType image_format_type;
        spv::Dim image_dim;
        bool is_image_array;
        bool is_multisampled;

        bool is_sampler_sampled{false};  // OpImageSample* or OpImageSparseSample*
        bool is_not_sampler_sampled{false};
        bool is_sampler_implicitLod_dref_proj{false};
        bool is_sampler_bias_offset{false};
        bool is_sampler_offset{false};        // ConstOffset or Offset (not ConstOffsets)
        bool is_read_without_format{false};   // For storage images
        bool is_write_without_format{false};  // For storage images
        bool is_dref{false};

        // vkspec.html#spirvenv-image-signedness describes how SignExtend/ZeroExtend can be used per-access to adjust the Signedness
        // Only need to check if one access has explicit signedness, mixing should be caught in spirv-val
        bool is_sign_extended{false};  // if at least one access has SignExtended
        bool is_zero_extended{false};  // if at least one access has ZeroExtended

        // If a variable is used as a function arguement, but never actually used, it will be found in EntryPoint::accessible_ids so
        // we need to have a dedicated mark if it was accessed.
        // We use this for variable hashing, but the VariableBase has the helper functions to read this value.
        uint32_t access_mask{AccessBit::empty};
    } info;
    uint64_t descriptor_hash = 0;
    bool IsImage() const { return info.image_format_type != NumericTypeUnknown; }

    // Type of resource type (vkspec.html#interfaces-resources-storage-class-correspondence)
    bool is_storage_image{false};
    bool is_storage_texel_buffer{false};
    const bool is_storage_buffer;
    bool is_input_attachment{false};

    ResourceInterfaceVariable(const Module &module_state, const EntryPoint &entrypoint, const Instruction &insn,
                              const ImageAccessMap &image_access_map, const AccessChainVariableMap &access_chain_map,
                              const VariableAccessMap &variable_access_map, const DebugNameMap &debug_name_map);

  protected:
    static const Instruction &FindBaseType(ResourceInterfaceVariable &variable, const Module &module_state);
    static uint32_t FindImageSampledTypeWidth(const Module &module_state, const Instruction &base_type);
    static NumericType FindImageFormatType(const Module &module_state, const Instruction &base_type);
    static bool IsStorageBuffer(const ResourceInterfaceVariable &variable);
};

// Used to help detect if different variable is being used
inline bool operator==(const ResourceInterfaceVariable &a, const ResourceInterfaceVariable &b) noexcept { return a.id == b.id; }
inline bool operator<(const ResourceInterfaceVariable &a, const ResourceInterfaceVariable &b) noexcept { return a.id < b.id; }

// vkspec.html#interfaces-resources-pushconst
// Push constants need to be statically used in shader
// Push constants are always OpTypeStruct and Block decorated
struct PushConstantVariable : public VariableBase {
    // This info could be found/saved in TypeStructInfo, but since Push Constants are the only ones using it right now, no point to
    // do it for every struct
    uint32_t offset;  // where first member is
    uint32_t size;    // total size of block

    PushConstantVariable(const Module &module_state, const Instruction &insn, VkShaderStageFlagBits stage,
                         const VariableAccessMap &variable_access_map, const DebugNameMap &debug_name_map);
};

// Represents a single Entrypoint into a Shader Module
struct EntryPoint {
    // "A module must not have two OpEntryPoint instructions with the same Execution Model and the same Name string."
    // There is no single unique item for a single entry point
    const Instruction &entrypoint_insn;  // OpEntryPoint instruction
    // For things like MeshNV vs MeshEXT, we need the execution_model
    const spv::ExecutionModel execution_model;
    const VkShaderStageFlagBits stage;
    const uint32_t id;
    const std::string name;
    const ExecutionModeSet &execution_mode;

    // Values found while gather the Accessible Ids
    bool emit_vertex_geometry;

    // All ids that can be accessed from the entry point
    // being accessed doesn't guarantee it is statically used
    const vvl::unordered_set<uint32_t> accessible_ids;

    // only one Push Constant block is allowed per entry point
    std::shared_ptr<const PushConstantVariable> push_constant_variable;
    const std::vector<ResourceInterfaceVariable> resource_interface_variables;
    const std::vector<StageInterfaceVariable> stage_interface_variables;
    // Easier to lookup without having to check for the is_builtin bool
    // "Built-in interface variables" - vkspec.html#interfaces-iointerfaces-builtin
    std::vector<const StageInterfaceVariable *> built_in_variables;
    // "User-defined Variable Interface" - vkspec.html#interfaces-iointerfaces-user
    std::vector<const StageInterfaceVariable *> user_defined_interface_variables;

    // Lookup map from Interface slot to the variable in that spot
    // spirv-val guarantees no overlap so 2 variables won't have same slot
    vvl::unordered_map<InterfaceSlot, const StageInterfaceVariable *, InterfaceSlot::Hash> input_interface_slots;
    vvl::unordered_map<InterfaceSlot, const StageInterfaceVariable *, InterfaceSlot::Hash> output_interface_slots;
    // Uesd for limit check
    const StageInterfaceVariable *max_input_slot_variable = nullptr;
    const StageInterfaceVariable *max_output_slot_variable = nullptr;
    const InterfaceSlot *max_input_slot = nullptr;
    const InterfaceSlot *max_output_slot = nullptr;
    uint32_t builtin_input_components = 0;
    uint32_t builtin_output_components = 0;

    // Mark if a BuiltIn is written to
    bool written_builtin_point_size{false};
    bool written_builtin_layer{false};
    bool written_builtin_primitive_shading_rate_khr{false};
    bool written_builtin_viewport_index{false};
    bool written_builtin_viewport_mask_nv{false};

    bool has_passthrough{false};
    bool has_alpha_to_coverage_variable{false};  // only for Fragment shaders

    bool has_physical_storage_buffer_interface{false};

    EntryPoint(const Module &module_state, const Instruction &entrypoint_insn, const ImageAccessMap &image_access_map,
               const AccessChainVariableMap &access_chain_map, const VariableAccessMap &variable_access_map,
               const DebugNameMap &debug_name_map);

    bool HasBuiltIn(spv::BuiltIn built_in) const;

  protected:
    static vvl::unordered_set<uint32_t> GetAccessibleIds(const Module &module_state, EntryPoint &entrypoint);
    static std::vector<StageInterfaceVariable> GetStageInterfaceVariables(const Module &module_state, const EntryPoint &entrypoint,
                                                                          const VariableAccessMap &variable_access_map,
                                                                          const DebugNameMap &debug_name_map);
    static std::vector<ResourceInterfaceVariable> GetResourceInterfaceVariables(const Module &module_state, EntryPoint &entrypoint,
                                                                                const ImageAccessMap &image_access_map,
                                                                                const AccessChainVariableMap &access_chain_map,
                                                                                const VariableAccessMap &variable_access_map,
                                                                                const DebugNameMap &debug_name_map);
    static bool IsBuiltInWritten(spv::BuiltIn built_in, const Module &module_state, const StageInterfaceVariable &variable,
                                 const AccessChainVariableMap &access_chain_map);
};

// Info to capture while parsing the SPIR-V, but will only be used by ValidateSpirvStateless and don't need to save after
struct StatelessData {
    // Used if the Shader Module is being passed in VkPipelineShaderStageCreateInfo
    std::shared_ptr<spirv::Module> pipeline_pnext_module;

    // These instruction mapping were designed to quickly find the few instructions without having to loop the entire pass
    // In theory, these could be removed checked during the 2nd pass in ValidateSpirvStateless()
    // TODO - Get perf numbers if better to understand if these make sense here
    std::vector<const Instruction *> read_clock_inst;
    std::vector<const Instruction *> atomic_inst;
    std::vector<const Instruction *> group_inst;
    // OpEmitStreamVertex/OpEndStreamPrimitive - only allowed in Geometry shader
    std::vector<const Instruction *> transform_feedback_stream_inst;

    // simpler to just track all OpExecutionModeId and parse things needed later
    std::vector<const Instruction *> execution_mode_id_inst;

    bool has_builtin_fully_covered{false};
    bool has_invocation_repack_instruction{false};
    bool has_group_decoration{false};
};

// Represents a SPIR-V Module
// This holds the SPIR-V source and parse it
struct Module {
    // Static/const data extracted from a SPIRV module at initialization time
    // The goal of this struct is to move everything that is ready only into here
    struct StaticData {
        StaticData() = default;
        StaticData(const Module &module_state, StatelessData *stateless_data = nullptr);
        StaticData &operator=(StaticData &&) = default;
        StaticData(StaticData &&) = default;

        // List of all instructions in the order they appear in the binary
        std::vector<Instruction> instructions;
        // Instructions that can be referenced by Ids
        // A mapping of <id> to the first word of its def. this is useful because walking type
        // trees, constant expressions, etc requires jumping all over the instruction stream.
        vvl::unordered_map<uint32_t, const Instruction *> definitions;

        vvl::unordered_map<uint32_t, DecorationSet> decorations;
        DecorationSet empty_decoration;  // all zero values, allows use to return a reference and not a copy each time

        // Execution Modes are tied to a Function <id>, multiple EntryPoints can point to the same Funciton <id>
        // Keep a mapping so each EntryPoint can grab a reference to it
        vvl::unordered_map<uint32_t, ExecutionModeSet> execution_modes;
        ExecutionModeSet empty_execution_mode;  // all zero values, allows use to return a reference and not a copy each time

        // [OpSpecConstant Result ID -> OpDecorate SpecID value] mapping
        vvl::unordered_map<uint32_t, uint32_t> id_to_spec_id;
        // Find all decoration instructions to prevent relooping module later - many checks need this info
        std::vector<const Instruction *> decoration_inst;
        std::vector<const Instruction *> member_decoration_inst;
        // Find all variable instructions to prevent relookping module later
        std::vector<const Instruction *> variable_inst;
        // For shader tile image - OpDepthAttachmentReadEXT/OpStencilAttachmentReadEXT/OpColorAttachmentReadEXT
        bool has_shader_tile_image_depth_read{false};
        bool has_shader_tile_image_stencil_read{false};
        bool has_shader_tile_image_color_read{false};
        // BuiltIn we just care about existing or not, don't have to be written to
        // TODO - Make bitmask
        bool has_builtin_layer{false};
        bool has_builtin_draw_index{false};
        bool has_builtin_workgroup_size{false};
        uint32_t builtin_workgroup_size_id = 0;

        std::vector<const Instruction *> cooperative_matrix_inst;

        std::vector<const Instruction *> cooperative_vector_inst;

        std::vector<spv::Capability> capability_list;
        // Code on the hot path can cache capabilities for fast access.
        bool has_capability_runtime_descriptor_array{false};

        bool has_specialization_constants{false};
        bool uses_interpolate_at_sample{false};

        // Will check if there is source debug information
        // Won't save any other info and will retrieve the debug info if requested in a VU error message
        bool using_legacy_debug_info{false};
        uint32_t shader_debug_info_set_id = 0;  // non-zero means shader has NonSemantic.Shader.DebugInfo.100

        // EntryPoint has pointer references inside it that need to be preserved
        std::vector<std::shared_ptr<EntryPoint>> entry_points;

        std::vector<std::shared_ptr<TypeStructInfo>> type_structs;  // All OpTypeStruct objects
        // <OpTypeStruct ID, info> - used for faster lookup as there can many structs
        vvl::unordered_map<uint32_t, std::shared_ptr<const TypeStructInfo>> type_struct_map;

        // Tracks accesses (load, store, atomic) to the instruction calling them
        // Example: the OpLoad does the "access" but need to know if a OpImageRead uses that OpLoad later
        vvl::unordered_map<const Instruction *, uint32_t> image_write_load_id_map;  // <OpImageWrite, load id>
    };

    // VK_KHR_maintenance5 allows VkShaderModuleCreateInfo (the SPIR-V binary) to be passed at pipeline creation time, because the
    // way we create our pipeline state objects first, we need to still create a valid Module object, but can signal that the
    // underlying spirv is not worth validating further
    const bool valid_spirv;

    // This is the SPIR-V module data content
    const std::vector<uint32_t> words_;

    const StaticData static_data_;

    // Hold a handle so error message can know where the SPIR-V was from (VkShaderModule or VkShaderEXT)
    VulkanTypedHandle handle_;                            // Will be updated once its known its valid SPIR-V
    VulkanTypedHandle handle() const { return handle_; }  // matches normal convention to get handle

    // Used for when modifying the SPIR-V (spirv-opt, GPU-AV instrumentation, etc) and need reparse it for VVL validation
    Module(vvl::span<const uint32_t> code) : valid_spirv(true), words_(code.begin(), code.end()), static_data_(*this) {}

    // StatelessData is a pointer as we have cases were we don't need it and simpler to just null check the few cases that use it
    Module(size_t codeSize, const uint32_t *pCode, StatelessData *stateless_data = nullptr)
        : valid_spirv(pCode && pCode[0] == spv::MagicNumber && ((codeSize % 4) == 0)),
          words_(pCode, pCode + codeSize / sizeof(uint32_t)),
          static_data_(*this, stateless_data) {}

    const Instruction *FindDef(uint32_t id) const {
        auto it = static_data_.definitions.find(id);
        if (it == static_data_.definitions.end()) return nullptr;
        return it->second;
    }

    const std::vector<Instruction> &GetInstructions() const { return static_data_.instructions; }

    const DecorationSet &GetDecorationSet(uint32_t id) const {
        // return the actual decorations for this id, or a default empty set.
        const auto it = static_data_.decorations.find(id);
        return (it != static_data_.decorations.end()) ? it->second : static_data_.empty_decoration;
    }

    const ExecutionModeSet &GetExecutionModeSet(uint32_t function_id) const {
        // return the actual execution modes for this id, or a default empty set.
        const auto it = static_data_.execution_modes.find(function_id);
        return (it != static_data_.execution_modes.end()) ? it->second : static_data_.empty_execution_mode;
    }

    std::shared_ptr<const TypeStructInfo> GetTypeStructInfo(uint32_t struct_id) const {
        // return the actual execution modes for this id, or a default empty set.
        const auto it = static_data_.type_struct_map.find(struct_id);
        return (it != static_data_.type_struct_map.end()) ? it->second : nullptr;
    }
    // Overload to walk down and find the OpTypeStruct
    std::shared_ptr<const TypeStructInfo> GetTypeStructInfo(const Instruction *insn) const {
        while (true) {
            if (insn->Opcode() == spv::OpVariable) {
                insn = FindDef(insn->Word(1));
            } else if (insn->Opcode() == spv::OpTypePointer) {
                insn = FindDef(insn->Word(3));
            } else if (insn->IsArray()) {
                insn = FindDef(insn->Word(2));
            } else if (insn->Opcode() == spv::OpTypeStruct) {
                return GetTypeStructInfo(insn->Word(1));
            } else {
                return nullptr;
            }
        }
    }

    // Used to get human readable strings for error messages
    std::string GetDecorations(uint32_t id) const;
    std::string GetName(uint32_t id) const;
    std::string GetMemberName(uint32_t id, uint32_t member_index) const;
    void DescribeTypeInner(std::ostringstream &ss, uint32_t type, uint32_t indent) const;
    std::string DescribeType(uint32_t type) const;
    std::string DescribeVariable(uint32_t id) const;
    std::string DescribeInstruction(const Instruction &error_insn) const;

    // Note that some shaders can have an input and output topology
    std::optional<VkPrimitiveTopology> GetTopology(const EntryPoint &entrypoint) const;

    std::shared_ptr<const EntryPoint> FindEntrypoint(char const *name, VkShaderStageFlagBits stageBits) const;
    bool FindLocalSize(const EntryPoint &entrypoint, uint32_t &local_size_x, uint32_t &local_size_y, uint32_t &local_size_z) const;

    uint32_t CalculateWorkgroupSharedMemory() const;

    const Instruction *GetConstantDef(uint32_t id) const;
    uint32_t GetConstantValueById(uint32_t id) const;
    uint32_t GetLocationsConsumedByType(uint32_t type) const;
    uint32_t GetComponentsConsumedByType(uint32_t type) const;
    NumericType GetNumericType(uint32_t type) const;

    bool HasRuntimeArray(uint32_t type_id) const;

    // Instruction helpers that need the knowledge of the whole SPIR-V module
    uint32_t GetNumComponentsInBaseType(const Instruction *insn) const;
    uint32_t GetTypeBitsSize(const Instruction *insn) const;
    uint32_t GetTypeBytesSize(const Instruction *insn) const;
    uint32_t GetBaseType(const Instruction *insn) const;
    const Instruction *GetBaseTypeInstruction(uint32_t type) const;
    uint32_t GetTypeId(uint32_t id) const;
    uint32_t GetTexelComponentCount(const Instruction &insn) const;
    uint32_t GetFlattenArraySize(const Instruction &insn) const;
    AtomicInstructionInfo GetAtomicInfo(const Instruction &insn) const;

    bool HasCapability(spv::Capability find_capability) const {
        return std::any_of(static_data_.capability_list.begin(), static_data_.capability_list.end(),
                           [find_capability](const spv::Capability &capability) { return capability == find_capability; });
    }
};

}  // namespace spirv

// Represents a VkShaderModule handle
namespace vvl {
struct ShaderModule : public StateObject {
    ShaderModule(VkShaderModule handle, std::shared_ptr<spirv::Module> &spirv_module)
        : StateObject(handle, kVulkanObjectTypeShaderModule), spirv(spirv_module) {
        spirv->handle_ = handle_;
    }

    // For when we need to create a module with no SPIR-V backing it
    ShaderModule() : StateObject(static_cast<VkShaderModule>(VK_NULL_HANDLE), kVulkanObjectTypeShaderModule) {}

    VkShaderModule VkHandle() const { return handle_.Cast<VkShaderModule>(); }

    // If null, means this is a empty object and no shader backing it
    // TODO - This (and vvl::ShaderObject) could be unique, but need handle multiple ValidationObjects
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/6265/files
    std::shared_ptr<spirv::Module> spirv;
};
}  // namespace vvl
