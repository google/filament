// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef sw_SpirvShader_hpp
#define sw_SpirvShader_hpp

#include "SamplerCore.hpp"
#include "ShaderCore.hpp"
#include "SpirvBinary.hpp"
#include "SpirvID.hpp"
#include "Device/Config.hpp"
#include "Device/Sampler.hpp"
#include "System/Debug.hpp"
#include "System/Math.hpp"
#include "System/Types.hpp"
#include "Vulkan/VkConfig.hpp"
#include "Vulkan/VkDescriptorSet.hpp"

#define SPV_ENABLE_UTILITY_CODE
#include <spirv/unified1/spirv.hpp>

#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#undef Yield  // b/127920555

namespace vk {

class Device;
class PipelineLayout;
class ImageView;
class Sampler;
class RenderPass;
struct Attachments;
struct SampledImageDescriptor;
struct SamplerState;

}  // namespace vk

namespace sw {

// Forward declarations.
class SpirvRoutine;

// Incrementally constructed complex bundle of rvalues
// Effectively a restricted vector, supporting only:
// - allocation to a (runtime-known) fixed component count
// - in-place construction of elements
// - const operator[]
class Intermediate
{
public:
	Intermediate(uint32_t componentCount)
	    : componentCount(componentCount)
	    , scalar(new rr::Value *[componentCount])
	{
		for(auto i = 0u; i < componentCount; i++) { scalar[i] = nullptr; }
	}

	~Intermediate()
	{
		delete[] scalar;
	}

	// TypeHint is used as a hint for rr::PrintValue::Ty<sw::Intermediate> to
	// decide the format used to print the intermediate data.
	enum class TypeHint
	{
		Float,
		Int,
		UInt
	};

	void move(uint32_t i, RValue<SIMD::Float> &&scalar) { emplace(i, scalar.value(), TypeHint::Float); }
	void move(uint32_t i, RValue<SIMD::Int> &&scalar) { emplace(i, scalar.value(), TypeHint::Int); }
	void move(uint32_t i, RValue<SIMD::UInt> &&scalar) { emplace(i, scalar.value(), TypeHint::UInt); }

	void move(uint32_t i, const RValue<SIMD::Float> &scalar) { emplace(i, scalar.value(), TypeHint::Float); }
	void move(uint32_t i, const RValue<SIMD::Int> &scalar) { emplace(i, scalar.value(), TypeHint::Int); }
	void move(uint32_t i, const RValue<SIMD::UInt> &scalar) { emplace(i, scalar.value(), TypeHint::UInt); }

	// Value retrieval functions.
	RValue<SIMD::Float> Float(uint32_t i) const
	{
		ASSERT(i < componentCount);
		ASSERT(scalar[i] != nullptr);
		RR_PRINT_ONLY(typeHint = TypeHint::Float;)
		return As<SIMD::Float>(scalar[i]);  // TODO(b/128539387): RValue<SIMD::Float>(scalar)
	}

	RValue<SIMD::Int> Int(uint32_t i) const
	{
		ASSERT(i < componentCount);
		ASSERT(scalar[i] != nullptr);
		RR_PRINT_ONLY(typeHint = TypeHint::Int;)
		return As<SIMD::Int>(scalar[i]);  // TODO(b/128539387): RValue<SIMD::Int>(scalar)
	}

	RValue<SIMD::UInt> UInt(uint32_t i) const
	{
		ASSERT(i < componentCount);
		ASSERT(scalar[i] != nullptr);
		RR_PRINT_ONLY(typeHint = TypeHint::UInt;)
		return As<SIMD::UInt>(scalar[i]);  // TODO(b/128539387): RValue<SIMD::UInt>(scalar)
	}

	// No copy/move construction or assignment
	Intermediate(const Intermediate &) = delete;
	Intermediate(Intermediate &&) = delete;
	Intermediate &operator=(const Intermediate &) = delete;
	Intermediate &operator=(Intermediate &&) = delete;

	const uint32_t componentCount;

private:
	void emplace(uint32_t i, rr::Value *value, TypeHint type)
	{
		ASSERT(i < componentCount);
		ASSERT(scalar[i] == nullptr);
		scalar[i] = value;
		RR_PRINT_ONLY(typeHint = type;)
	}

	rr::Value **const scalar;

#ifdef ENABLE_RR_PRINT
	friend struct rr::PrintValue::Ty<sw::Intermediate>;
	mutable TypeHint typeHint = TypeHint::Float;
#endif  // ENABLE_RR_PRINT
};

// The Spirv class parses a SPIR-V binary and provides utilities for retrieving
// information about instructions, objects, types, etc.
class Spirv
{
public:
	Spirv(VkShaderStageFlagBits stage,
	      const char *entryPointName,
	      const SpirvBinary &insns);

	~Spirv();

	SpirvBinary insns;

	class Type;
	class Object;

	// Pseudo-iterator over SPIR-V instructions, designed to support range-based-for.
	class InsnIterator
	{
	public:
		InsnIterator() = default;
		InsnIterator(const InsnIterator &other) = default;
		InsnIterator &operator=(const InsnIterator &other) = default;

		explicit InsnIterator(SpirvBinary::const_iterator iter)
		    : iter{ iter }
		{
		}

		spv::Op opcode() const
		{
			return static_cast<spv::Op>(*iter & spv::OpCodeMask);
		}

		uint32_t wordCount() const
		{
			return *iter >> spv::WordCountShift;
		}

		uint32_t word(uint32_t n) const
		{
			ASSERT(n < wordCount());
			return iter[n];
		}

		const uint32_t *data() const
		{
			return &iter[0];
		}

		const char *string(uint32_t n) const
		{
			return reinterpret_cast<const char *>(&iter[n]);
		}

		// Returns the number of whole-words that a string literal starting at
		// word n consumes. If the end of the intruction is reached before the
		// null-terminator is found, then the function DABORT()s and 0 is
		// returned.
		uint32_t stringSizeInWords(uint32_t n) const
		{
			uint32_t c = wordCount();
			for(uint32_t i = n; n < c; i++)
			{
				const char *s = string(i);
				// SPIR-V spec 2.2.1. Instructions:
				// A string is interpreted as a nul-terminated stream of
				// characters. The character set is Unicode in the UTF-8
				// encoding scheme. The UTF-8 octets (8-bit bytes) are packed
				// four per word, following the little-endian convention (i.e.,
				// the first octet is in the lowest-order 8 bits of the word).
				// The final word contains the string's nul-termination
				// character (0), and all contents past the end of the string in
				// the final word are padded with 0.
				if(s[3] == 0)
				{
					return 1 + i - n;
				}
			}
			DABORT("SPIR-V string literal was not null-terminated");
			return 0;
		}

		bool hasResultAndType() const
		{
			bool hasResult = false, hasResultType = false;
			spv::HasResultAndType(opcode(), &hasResult, &hasResultType);

			return hasResultType;
		}

		SpirvID<Type> resultTypeId() const
		{
			ASSERT(hasResultAndType());
			return word(1);
		}

		SpirvID<Object> resultId() const
		{
			ASSERT(hasResultAndType());
			return word(2);
		}

		uint32_t distanceFrom(const InsnIterator &other) const
		{
			return static_cast<uint32_t>(iter - other.iter);
		}

		bool operator==(const InsnIterator &other) const
		{
			return iter == other.iter;
		}

		bool operator!=(const InsnIterator &other) const
		{
			return iter != other.iter;
		}

		InsnIterator operator*() const
		{
			return *this;
		}

		InsnIterator &operator++()
		{
			iter += wordCount();
			return *this;
		}

		InsnIterator const operator++(int)
		{
			InsnIterator ret{ *this };
			iter += wordCount();
			return ret;
		}

	private:
		SpirvBinary::const_iterator iter;
	};

	// Range-based-for interface
	InsnIterator begin() const
	{
		// Skip over the header words
		return InsnIterator{ insns.cbegin() + 5 };
	}

	InsnIterator end() const
	{
		return InsnIterator{ insns.cend() };
	}

	// A range of contiguous instruction words.
	struct Span
	{
		Span(const InsnIterator &insn, uint32_t offset, uint32_t size)
		    : insn(insn)
		    , offset(offset)
		    , wordCount(size)
		{}

		uint32_t operator[](uint32_t index) const
		{
			ASSERT(index < wordCount);
			return insn.word(offset + index);
		}

		uint32_t size() const
		{
			return wordCount;
		}

	private:
		const InsnIterator &insn;
		const uint32_t offset;
		const uint32_t wordCount;
	};

	class Type
	{
	public:
		using ID = SpirvID<Type>;

		spv::Op opcode() const { return definition.opcode(); }

		InsnIterator definition;
		spv::StorageClass storageClass = static_cast<spv::StorageClass>(-1);
		uint32_t componentCount = 0;
		bool isBuiltInBlock = false;

		// Inner element type for pointers, arrays, vectors and matrices.
		ID element;
	};

	class Object
	{
	public:
		using ID = SpirvID<Object>;

		spv::Op opcode() const { return definition.opcode(); }
		Type::ID typeId() const { return definition.resultTypeId(); }
		Object::ID id() const { return definition.resultId(); }

		bool isConstantZero() const;

		InsnIterator definition;
		std::vector<uint32_t> constantValue;

		enum class Kind
		{
			// Invalid default kind.
			// If we get left with an object in this state, the module was
			// broken.
			Unknown,

			// TODO: Better document this kind.
			// A shader interface variable pointer.
			// Pointer with uniform address across all lanes.
			// Pointer held by SpirvRoutine::pointers
			InterfaceVariable,

			// Constant value held by Object::constantValue.
			Constant,

			// Value held by SpirvRoutine::intermediates.
			Intermediate,

			// Pointer held by SpirvRoutine::pointers
			Pointer,

			// Combination of an image pointer and a sampler ID
			SampledImage,

			// A pointer to a vk::DescriptorSet*.
			// Pointer held by SpirvRoutine::pointers.
			DescriptorSet,
		};

		Kind kind = Kind::Unknown;
	};

	// Block is an interval of SPIR-V instructions, starting with the
	// opening OpLabel, and ending with a termination instruction.
	class Block
	{
	public:
		using ID = SpirvID<Block>;
		using Set = std::unordered_set<ID>;

		// Edge represents the graph edge between two blocks.
		struct Edge
		{
			ID from;
			ID to;

			bool operator==(const Edge &other) const { return from == other.from && to == other.to; }

			struct Hash
			{
				std::size_t operator()(const Edge &edge) const noexcept
				{
					return std::hash<uint32_t>()(edge.from.value() * 31 + edge.to.value());
				}
			};
		};

		Block() = default;
		Block(const Block &other) = default;
		Block &operator=(const Block &other) = default;
		explicit Block(InsnIterator begin, InsnIterator end);

		/* range-based-for interface */
		inline InsnIterator begin() const { return begin_; }
		inline InsnIterator end() const { return end_; }

		enum Kind
		{
			Simple,                         // OpBranch or other simple terminator.
			StructuredBranchConditional,    // OpSelectionMerge + OpBranchConditional
			UnstructuredBranchConditional,  // OpBranchConditional
			StructuredSwitch,               // OpSelectionMerge + OpSwitch
			UnstructuredSwitch,             // OpSwitch
			Loop,                           // OpLoopMerge + [OpBranchConditional | OpBranch]
		};

		Kind kind = Simple;
		InsnIterator mergeInstruction;   // Structured control flow merge instruction.
		InsnIterator branchInstruction;  // Branch instruction.
		ID mergeBlock;                   // Structured flow merge block.
		ID continueTarget;               // Loop continue block.
		Set ins;                         // Blocks that branch into this block.
		Set outs;                        // Blocks that this block branches to.
		bool isLoopMerge = false;

	private:
		InsnIterator begin_;
		InsnIterator end_;
	};

	class Function
	{
	public:
		using ID = SpirvID<Function>;

		// Walks all reachable the blocks starting from id adding them to
		// reachable.
		void TraverseReachableBlocks(Block::ID id, Block::Set &reachable) const;

		// AssignBlockFields() performs the following for all reachable blocks:
		// * Assigns Block::ins with the identifiers of all blocks that contain
		//   this block in their Block::outs.
		// * Sets Block::isLoopMerge to true if the block is the merge of a
		//   another loop block.
		void AssignBlockFields();

		// ForeachBlockDependency calls f with each dependency of the given
		// block. A dependency is an incoming block that is not a loop-back
		// edge.
		void ForeachBlockDependency(Block::ID blockId, std::function<void(Block::ID)> f) const;

		// ExistsPath returns true if there's a direct or indirect flow from
		// the 'from' block to the 'to' block that does not pass through
		// notPassingThrough.
		bool ExistsPath(Block::ID from, Block::ID to, Block::ID notPassingThrough) const;

		const Block &getBlock(Block::ID id) const
		{
			auto it = blocks.find(id);
			ASSERT_MSG(it != blocks.end(), "Unknown block %d", id.value());
			return it->second;
		}

		Block::ID entry;          // function entry point block.
		HandleMap<Block> blocks;  // blocks belonging to this function.
		Type::ID type;            // type of the function.
		Type::ID result;          // return type.
	};

	using String = std::string;
	using StringID = SpirvID<std::string>;

	class Extension
	{
	public:
		using ID = SpirvID<Extension>;

		enum Name
		{
			Unknown,
			GLSLstd450,
			OpenCLDebugInfo100,
			NonSemanticInfo,
		};

		Name name;
	};

	struct TypeOrObject
	{};

	// TypeOrObjectID is an identifier that represents a Type or an Object,
	// and supports implicit casting to and from Type::ID or Object::ID.
	class TypeOrObjectID : public SpirvID<TypeOrObject>
	{
	public:
		using Hash = std::hash<SpirvID<TypeOrObject>>;

		inline TypeOrObjectID(uint32_t id)
		    : SpirvID(id)
		{}
		inline TypeOrObjectID(Type::ID id)
		    : SpirvID(id.value())
		{}
		inline TypeOrObjectID(Object::ID id)
		    : SpirvID(id.value())
		{}
		inline operator Type::ID() const { return Type::ID(value()); }
		inline operator Object::ID() const { return Object::ID(value()); }
	};

	// This method is for retrieving an ID that uniquely identifies the
	// shader entry point represented by this object.
	uint64_t getIdentifier() const
	{
		return ((uint64_t)entryPoint.value() << 32) | insns.getIdentifier();
	}

	struct ExecutionModes
	{
		bool EarlyFragmentTests : 1;
		bool DepthReplacing : 1;
		bool DepthGreater : 1;
		bool DepthLess : 1;
		bool DepthUnchanged : 1;
		bool StencilRefReplacing : 1;

		// Compute workgroup dimensions
		Object::ID WorkgroupSizeX = 1;
		Object::ID WorkgroupSizeY = 1;
		Object::ID WorkgroupSizeZ = 1;
		bool useWorkgroupSizeId = false;
	};

	const ExecutionModes &getExecutionModes() const
	{
		return executionModes;
	}

	struct Analysis
	{
		bool ContainsDiscard : 1;  // OpKill, OpTerminateInvocation, or OpDemoteToHelperInvocation
		bool ContainsControlBarriers : 1;
		bool NeedsCentroid : 1;
		bool ContainsSampleQualifier : 1;
		bool ContainsImageWrite : 1;
	};

	const Analysis &getAnalysis() const { return analysis; }
	bool containsImageWrite() const { return analysis.ContainsImageWrite; }

	bool coverageModified() const
	{
		return analysis.ContainsDiscard ||
		       (outputBuiltins.find(spv::BuiltInSampleMask) != outputBuiltins.end());
	}

	struct Capabilities
	{
		bool Matrix : 1;
		bool Shader : 1;
		bool StorageImageMultisample : 1;
		bool ClipDistance : 1;
		bool CullDistance : 1;
		bool ImageCubeArray : 1;
		bool SampleRateShading : 1;
		bool InputAttachment : 1;
		bool Sampled1D : 1;
		bool Image1D : 1;
		bool SampledBuffer : 1;
		bool SampledCubeArray : 1;
		bool ImageBuffer : 1;
		bool ImageMSArray : 1;
		bool StorageImageExtendedFormats : 1;
		bool ImageQuery : 1;
		bool DerivativeControl : 1;
		bool DotProductInputAll : 1;
		bool DotProductInput4x8Bit : 1;
		bool DotProductInput4x8BitPacked : 1;
		bool DotProduct : 1;
		bool InterpolationFunction : 1;
		bool StorageImageWriteWithoutFormat : 1;
		bool GroupNonUniform : 1;
		bool GroupNonUniformVote : 1;
		bool GroupNonUniformBallot : 1;
		bool GroupNonUniformShuffle : 1;
		bool GroupNonUniformShuffleRelative : 1;
		bool GroupNonUniformArithmetic : 1;
		bool GroupNonUniformQuad : 1;
		bool DeviceGroup : 1;
		bool MultiView : 1;
		bool SignedZeroInfNanPreserve : 1;
		bool DemoteToHelperInvocation : 1;
		bool StencilExportEXT : 1;
		bool VulkanMemoryModel : 1;
		bool VulkanMemoryModelDeviceScope : 1;
		bool ShaderNonUniform : 1;
		bool RuntimeDescriptorArray : 1;
		bool StorageBufferArrayNonUniformIndexing : 1;
		bool StorageTexelBufferArrayNonUniformIndexing : 1;
		bool StorageTexelBufferArrayDynamicIndexing : 1;
		bool UniformTexelBufferArrayNonUniformIndexing : 1;
		bool UniformTexelBufferArrayDynamicIndexing : 1;
		bool UniformBufferArrayNonUniformIndex : 1;
		bool SampledImageArrayNonUniformIndexing : 1;
		bool StorageImageArrayNonUniformIndexing : 1;
		bool PhysicalStorageBufferAddresses : 1;
	};

	const Capabilities &getUsedCapabilities() const
	{
		return capabilities;
	}

	// getNumOutputClipDistances() returns the number of ClipDistances
	// outputted by this shader.
	unsigned int getNumOutputClipDistances() const
	{
		if(getUsedCapabilities().ClipDistance)
		{
			auto it = outputBuiltins.find(spv::BuiltInClipDistance);
			if(it != outputBuiltins.end())
			{
				return it->second.SizeInComponents;
			}
		}
		return 0;
	}

	// getNumOutputCullDistances() returns the number of CullDistances
	// outputted by this shader.
	unsigned int getNumOutputCullDistances() const
	{
		if(getUsedCapabilities().CullDistance)
		{
			auto it = outputBuiltins.find(spv::BuiltInCullDistance);
			if(it != outputBuiltins.end())
			{
				return it->second.SizeInComponents;
			}
		}
		return 0;
	}

	enum AttribType : unsigned char
	{
		ATTRIBTYPE_FLOAT,
		ATTRIBTYPE_INT,
		ATTRIBTYPE_UINT,
		ATTRIBTYPE_UNUSED,

		ATTRIBTYPE_LAST = ATTRIBTYPE_UINT
	};

	bool hasBuiltinInput(spv::BuiltIn b) const
	{
		return inputBuiltins.find(b) != inputBuiltins.end();
	}

	bool hasBuiltinOutput(spv::BuiltIn b) const
	{
		return outputBuiltins.find(b) != outputBuiltins.end();
	}

	struct Decorations
	{
		int32_t Location = -1;
		int32_t Component = 0;
		spv::BuiltIn BuiltIn = static_cast<spv::BuiltIn>(-1);
		int32_t Offset = -1;
		int32_t ArrayStride = -1;
		int32_t MatrixStride = 1;

		bool HasLocation : 1;
		bool HasComponent : 1;
		bool HasBuiltIn : 1;
		bool HasOffset : 1;
		bool HasArrayStride : 1;
		bool HasMatrixStride : 1;
		bool HasRowMajor : 1;  // whether RowMajor bit is valid.

		bool Flat : 1;
		bool Centroid : 1;
		bool NoPerspective : 1;
		bool Block : 1;
		bool BufferBlock : 1;
		bool RelaxedPrecision : 1;
		bool RowMajor : 1;      // RowMajor if true; ColMajor if false
		bool InsideMatrix : 1;  // pseudo-decoration for whether we're inside a matrix.
		bool NonUniform : 1;

		Decorations()
		    : Location{ -1 }
		    , Component{ 0 }
		    , BuiltIn{ static_cast<spv::BuiltIn>(-1) }
		    , Offset{ -1 }
		    , ArrayStride{ -1 }
		    , MatrixStride{ -1 }
		    , HasLocation{ false }
		    , HasComponent{ false }
		    , HasBuiltIn{ false }
		    , HasOffset{ false }
		    , HasArrayStride{ false }
		    , HasMatrixStride{ false }
		    , HasRowMajor{ false }
		    , Flat{ false }
		    , Centroid{ false }
		    , NoPerspective{ false }
		    , Block{ false }
		    , BufferBlock{ false }
		    , RelaxedPrecision{ false }
		    , RowMajor{ false }
		    , InsideMatrix{ false }
		    , NonUniform{ false }
		{
		}

		Decorations(const Decorations &) = default;
		Decorations& operator= (const Decorations &) = default;

		void Apply(const Decorations &src);

		void Apply(spv::Decoration decoration, uint32_t arg);
	};

	std::unordered_map<TypeOrObjectID, Decorations, TypeOrObjectID::Hash> decorations;
	std::unordered_map<Type::ID, std::vector<Decorations>> memberDecorations;

	struct DescriptorDecorations
	{
		int32_t DescriptorSet = -1;
		int32_t Binding = -1;
		int32_t InputAttachmentIndex = -1;

		void Apply(const DescriptorDecorations &src);
	};

	std::unordered_map<Object::ID, DescriptorDecorations> descriptorDecorations;

	struct InterfaceComponent
	{
		AttribType Type;

		union
		{
			struct
			{
				bool Flat : 1;
				bool Centroid : 1;
				bool NoPerspective : 1;
			};

			uint8_t DecorationBits;
		};

		InterfaceComponent()
		    : Type{ ATTRIBTYPE_UNUSED }
		    , DecorationBits{ 0 }
		{
		}
	};

	struct BuiltinMapping
	{
		Object::ID Id;
		uint32_t FirstComponent;
		uint32_t SizeInComponents;
	};

	struct WorkgroupMemory
	{
		// allocates a new variable of size bytes with the given identifier.
		inline void allocate(Object::ID id, uint32_t size)
		{
			uint32_t offset = totalSize;
			auto it = offsets.emplace(id, offset);
			ASSERT_MSG(it.second, "WorkgroupMemory already has an allocation for object %d", int(id.value()));
			totalSize += size;
		}
		// returns the byte offset of the variable with the given identifier.
		inline uint32_t offsetOf(Object::ID id) const
		{
			auto it = offsets.find(id);
			ASSERT_MSG(it != offsets.end(), "WorkgroupMemory has no allocation for object %d", int(id.value()));
			return it->second;
		}
		// returns the total allocated size in bytes.
		inline uint32_t size() const { return totalSize; }

	private:
		uint32_t totalSize = 0;                            // in bytes
		std::unordered_map<Object::ID, uint32_t> offsets;  // in bytes
	};

	std::vector<InterfaceComponent> inputs;
	std::vector<InterfaceComponent> outputs;

	uint32_t getWorkgroupSizeX() const;
	uint32_t getWorkgroupSizeY() const;
	uint32_t getWorkgroupSizeZ() const;

	using BuiltInHash = std::hash<std::underlying_type<spv::BuiltIn>::type>;
	std::unordered_map<spv::BuiltIn, BuiltinMapping, BuiltInHash> inputBuiltins;
	std::unordered_map<spv::BuiltIn, BuiltinMapping, BuiltInHash> outputBuiltins;
	WorkgroupMemory workgroupMemory;

	Function::ID entryPoint;
	spv::ExecutionModel executionModel = spv::ExecutionModelMax;  // Invalid prior to OpEntryPoint parsing.
	ExecutionModes executionModes = {};
	Capabilities capabilities = {};
	spv::AddressingModel addressingModel = spv::AddressingModelLogical;
	spv::MemoryModel memoryModel = spv::MemoryModelSimple;
	HandleMap<Extension> extensionsByID;
	std::unordered_set<uint32_t> extensionsImported;

	Analysis analysis = {};

	HandleMap<Type> types;
	HandleMap<Object> defs;

	// TODO(b/247020580): Encapsulate
public:
	HandleMap<Function> functions;
	std::unordered_map<StringID, String> strings;

	// DeclareType creates a Type for the given OpTypeX instruction, storing
	// it into the types map. It is called from the analysis pass (constructor).
	void DeclareType(InsnIterator insn);

	void ProcessExecutionMode(InsnIterator it);

	uint32_t ComputeTypeSize(InsnIterator insn);
	Decorations GetDecorationsForId(TypeOrObjectID id) const;
	void ApplyDecorationsForId(Decorations *d, TypeOrObjectID id) const;
	void ApplyDecorationsForIdMember(Decorations *d, Type::ID id, uint32_t member) const;
	void ApplyDecorationsForAccessChain(Decorations *d, DescriptorDecorations *dd, Object::ID baseId, const Span &indexIds) const;

	// Creates an Object for the instruction's result in 'defs'.
	void DefineResult(const InsnIterator &insn);

	using InterfaceVisitor = std::function<void(Decorations const, AttribType)>;

	void VisitInterface(Object::ID id, const InterfaceVisitor &v) const;

	int VisitInterfaceInner(Type::ID id, Decorations d, const InterfaceVisitor &v) const;

	// MemoryElement describes a scalar element within a structure, and is
	// used by the callback function of VisitMemoryObject().
	struct MemoryElement
	{
		uint32_t index;    // index of the scalar element
		uint32_t offset;   // offset (in bytes) from the base of the object
		const Type &type;  // element type
	};

	using MemoryVisitor = std::function<void(const MemoryElement &)>;

	// VisitMemoryObject() walks a type tree in an explicitly laid out
	// storage class, calling the MemoryVisitor for each scalar element
	// within the
	void VisitMemoryObject(Object::ID id, bool resultIsPointer, const MemoryVisitor &v) const;

	// VisitMemoryObjectInner() is internally called by VisitMemoryObject()
	void VisitMemoryObjectInner(Type::ID id, Decorations d, uint32_t &index, uint32_t offset, bool resultIsPointer, const MemoryVisitor &v) const;

	Object &CreateConstant(InsnIterator it);

	void ProcessInterfaceVariable(Object &object);

	const Type &getType(Type::ID id) const
	{
		auto it = types.find(id);
		ASSERT_MSG(it != types.end(), "Unknown type %d", id.value());
		return it->second;
	}

	const Type &getType(const Object &object) const
	{
		return getType(object.typeId());
	}

	const Object &getObject(Object::ID id) const
	{
		auto it = defs.find(id);
		ASSERT_MSG(it != defs.end(), "Unknown object %d", id.value());
		return it->second;
	}

	const Type &getObjectType(Object::ID id) const
	{
		return getType(getObject(id));
	}

	const Function &getFunction(Function::ID id) const
	{
		auto it = functions.find(id);
		ASSERT_MSG(it != functions.end(), "Unknown function %d", id.value());
		return it->second;
	}

	const String &getString(StringID id) const
	{
		auto it = strings.find(id);
		ASSERT_MSG(it != strings.end(), "Unknown string %d", id.value());
		return it->second;
	}

	const Extension &getExtension(Extension::ID id) const
	{
		auto it = extensionsByID.find(id);
		ASSERT_MSG(it != extensionsByID.end(), "Unknown extension %d", id.value());
		return it->second;
	}

	// Returns the *component* offset in the literal for the given access chain.
	uint32_t WalkLiteralAccessChain(Type::ID id, const Span &indexes) const;

	uint32_t GetConstScalarInt(Object::ID id) const;
	void EvalSpecConstantOp(InsnIterator insn);
	void EvalSpecConstantUnaryOp(InsnIterator insn);
	void EvalSpecConstantBinaryOp(InsnIterator insn);

	// Fragment input interpolation functions
	uint32_t GetNumInputComponents(int32_t location) const;
	uint32_t GetPackedInterpolant(int32_t location) const;

	// WriteCFGGraphVizDotFile() writes a graphviz dot file of the shader's
	// control flow to the given file path.
	void WriteCFGGraphVizDotFile(const char *path) const;

	// OpcodeName() returns the name of the opcode op.
	static const char *OpcodeName(spv::Op opcode);
	static std::memory_order MemoryOrder(spv::MemorySemanticsMask memorySemantics);

	// IsStatement() returns true if the given opcode actually performs
	// work (as opposed to declaring a type, defining a function start / end,
	// etc).
	static bool IsStatement(spv::Op opcode);

	// HasTypeAndResult() returns true if the given opcode's instruction
	// has a result type ID and result ID, i.e. defines an Object.
	static bool HasTypeAndResult(spv::Op opcode);

	// Returns 0 when invalid.
	static VkShaderStageFlagBits executionModelToStage(spv::ExecutionModel model);

	static bool StoresInHelperInvocationsHaveNoEffect(spv::StorageClass storageClass);
	static bool IsExplicitLayout(spv::StorageClass storageClass);
	static bool IsTerminator(spv::Op opcode);
};

// The SpirvShader class holds a parsed SPIR-V shader but also the pipeline
// state which affects code emission when passing it to SpirvEmitter.
class SpirvShader : public Spirv
{
public:
	SpirvShader(VkShaderStageFlagBits stage,
	            const char *entryPointName,
	            const SpirvBinary &insns,
	            const vk::RenderPass *renderPass,
	            uint32_t subpassIndex,
		    const VkRenderingInputAttachmentIndexInfoKHR *inputAttachmentMapping,
	            bool robustBufferAccess);

	~SpirvShader();

	// TODO(b/247020580): Move to SpirvRoutine
	void emitProlog(SpirvRoutine *routine) const;
	void emit(SpirvRoutine *routine, const RValue<SIMD::Int> &activeLaneMask, const RValue<SIMD::Int> &storesAndAtomicsMask, const vk::DescriptorSet::Bindings &descriptorSets, const vk::Attachments *attachments = nullptr, unsigned int multiSampleCount = 0) const;
	void emitEpilog(SpirvRoutine *routine) const;

	bool getRobustBufferAccess() const { return robustBufferAccess; }
	OutOfBoundsBehavior getOutOfBoundsBehavior(Object::ID pointerId, const vk::PipelineLayout *pipelineLayout) const;

	vk::Format getInputAttachmentFormat(const vk::Attachments &attachments, int32_t index) const;

private:
	const bool robustBufferAccess;

	// When reading from an input attachment, its format is needed.  When the fragment shader
	// pipeline library is created, the formats are available with render pass objects, but not
	// with dynamic rendering.  Instead, with dynamic rendering the formats are provided to the
	// fragment output interface pipeline library.
	//
	// This class is instantiated by the fragment shader pipeline library.  With dynamic
	// rendering, the mapping from input attachment indices to render pass attachments are
	// stored here at that point.  Later, when the formats are needed, the information is taken
	// out of the information provided to the fragment output interface pipeline library.
	//
	// In the following, `inputIndexToColorIndex` maps from an input attachment index docoration
	// in the shader to the attachment index (not the remapped location).
	//
	// The depthInputIndex and stencilInputIndex values are only valid for dynamic rendering and
	// indicate what input attachment index is supposed to map to each.  They are optional, as
	// the shader no longer has to decorate depth and stencil input attachments with
	// an InputAttachmentIndex decoration.
	//
	// Note: If SpirvEmitter::EmitImageRead were to take the format from the bound descriptor,
	// none of the following would be necessary.  With the current implementation, read-only
	// input attachments cannot be supported with dynamic rendering because they don't map to
	// any attachment.
	const bool isUsedWithDynamicRendering;
	std::unordered_map<uint32_t, uint32_t> inputIndexToColorIndex;
	int32_t depthInputIndex = -1;
	int32_t stencilInputIndex = -1;

	// With render passes objects, all formats are derived early from
	// VkSubpassDescription::pInputAttachments.
	std::vector<vk::Format> inputAttachmentFormats;
};

// The SpirvEmitter class translates the parsed SPIR-V shader into Reactor code.
class SpirvEmitter
{
	using Type = Spirv::Type;
	using Object = Spirv::Object;
	using Block = Spirv::Block;
	using InsnIterator = Spirv::InsnIterator;
	using Decorations = Spirv::Decorations;
	using Span = Spirv::Span;

public:
	static void emit(const SpirvShader &shader,
	                 SpirvRoutine *routine,
	                 Spirv::Function::ID entryPoint,
	                 RValue<SIMD::Int> activeLaneMask,
	                 RValue<SIMD::Int> storesAndAtomicsMask,
	                 const vk::Attachments *attachments,
	                 const vk::DescriptorSet::Bindings &descriptorSets,
	                 unsigned int multiSampleCount);

	// Helper for calling rr::Yield with result cast to an rr::Int.
	enum class YieldResult
	{
		ControlBarrier = 0,
	};

private:
	SpirvEmitter(const SpirvShader &shader,
	             SpirvRoutine *routine,
	             Spirv::Function::ID entryPoint,
	             RValue<SIMD::Int> activeLaneMask,
	             RValue<SIMD::Int> storesAndAtomicsMask,
	             const vk::Attachments *attachments,
	             const vk::DescriptorSet::Bindings &descriptorSets,
	             unsigned int multiSampleCount);

	// Returns the mask describing the active lanes as updated by dynamic
	// control flow. Active lanes include helper invocations, used for
	// calculating fragment derivitives, which must not perform memory
	// stores or atomic writes.
	//
	// Use activeStoresAndAtomicsMask() to consider both control flow and
	// lanes which are permitted to perform memory stores and atomic
	// operations
	RValue<SIMD::Int> activeLaneMask() const
	{
		ASSERT(activeLaneMaskValue != nullptr);
		return RValue<SIMD::Int>(activeLaneMaskValue);
	}

	// Returns the immutable lane mask that describes which lanes are
	// permitted to perform memory stores and atomic operations.
	// Note that unlike activeStoresAndAtomicsMask() this mask *does not*
	// consider lanes that have been made inactive due to control flow.
	RValue<SIMD::Int> storesAndAtomicsMask() const
	{
		ASSERT(storesAndAtomicsMaskValue != nullptr);
		return RValue<SIMD::Int>(storesAndAtomicsMaskValue);
	}

	// Returns a lane mask that describes which lanes are permitted to
	// perform memory stores and atomic operations, considering lanes that
	// may have been made inactive due to control flow.
	RValue<SIMD::Int> activeStoresAndAtomicsMask() const
	{
		return activeLaneMask() & storesAndAtomicsMask();
	}

	// Add a new active lane mask edge from the current block to out.
	// The edge mask value will be (mask AND activeLaneMaskValue).
	// If multiple active lane masks are added for the same edge, then
	// they will be ORed together.
	void addOutputActiveLaneMaskEdge(Block::ID out, RValue<SIMD::Int> mask);

	// Add a new active lane mask for the edge from -> to.
	// If multiple active lane masks are added for the same edge, then
	// they will be ORed together.
	void addActiveLaneMaskEdge(Block::ID from, Block::ID to, RValue<SIMD::Int> mask);

	// OpImageSample variants
	enum Variant : uint32_t
	{
		None,  // No Dref or Proj. Also used by OpImageFetch and OpImageQueryLod.
		Dref,
		Proj,
		ProjDref,
		VARIANT_LAST = ProjDref
	};

	// Compact representation of image instruction state that is passed to the
	// trampoline function for retrieving/generating the corresponding sampling routine.
	struct ImageInstructionSignature
	{
		ImageInstructionSignature(Variant variant, SamplerMethod samplerMethod)
		{
			this->variant = variant;
			this->samplerMethod = samplerMethod;
		}

		// Unmarshal from raw 32-bit data
		explicit ImageInstructionSignature(uint32_t signature)
		    : signature(signature)
		{}

		SamplerFunction getSamplerFunction() const
		{
			return { samplerMethod, offset != 0, sample != 0 };
		}

		bool isDref() const
		{
			return (variant == Dref) || (variant == ProjDref);
		}

		bool isProj() const
		{
			return (variant == Proj) || (variant == ProjDref);
		}

		bool hasLod() const
		{
			return samplerMethod == Lod || samplerMethod == Fetch;  // We always pass a Lod operand for Fetch operations.
		}

		bool hasGrad() const
		{
			return samplerMethod == Grad;
		}

		union
		{
			struct
			{
				Variant variant : BITS(VARIANT_LAST);
				SamplerMethod samplerMethod : BITS(SAMPLER_METHOD_LAST);
				uint32_t gatherComponent : 2;
				uint32_t dim : BITS(spv::DimSubpassData);  // spv::Dim
				uint32_t arrayed : 1;
				uint32_t imageFormat : BITS(spv::ImageFormatR64i);  // spv::ImageFormat

				// Parameters are passed to the sampling routine in this order:
				uint32_t coordinates : 3;       // 1-4 (does not contain projection component)
				/*	uint32_t dref : 1; */       // Indicated by Variant::ProjDref|Dref
				/*	uint32_t lodOrBias : 1; */  // Indicated by SamplerMethod::Lod|Bias|Fetch
				uint32_t grad : 2;              // 0-3 components (for each of dx / dy)
				uint32_t offset : 2;            // 0-3 components
				uint32_t sample : 1;            // 0-1 scalar integer
			};

			uint32_t signature = 0;
		};
	};

	// This gets stored as a literal in the generated code, so it should be compact.
	static_assert(sizeof(ImageInstructionSignature) == sizeof(uint32_t), "ImageInstructionSignature must be 32-bit");

	struct ImageInstruction : public ImageInstructionSignature
	{
		ImageInstruction(InsnIterator insn, const Spirv &shader, const SpirvEmitter &state);

		const uint32_t position;

		Type::ID resultTypeId = 0;
		Object::ID resultId = 0;
		Object::ID imageId = 0;
		Object::ID samplerId = 0;
		Object::ID coordinateId = 0;
		Object::ID texelId = 0;
		Object::ID drefId = 0;
		Object::ID lodOrBiasId = 0;
		Object::ID gradDxId = 0;
		Object::ID gradDyId = 0;
		Object::ID offsetId = 0;
		Object::ID sampleId = 0;

	private:
		static ImageInstructionSignature parseVariantAndMethod(InsnIterator insn);
		static uint32_t getImageOperandsIndex(InsnIterator insn);
		static uint32_t getImageOperandsMask(InsnIterator insn);
	};

	class SampledImagePointer : public SIMD::Pointer
	{
	public:
		SampledImagePointer(SIMD::Pointer image, Object::ID sampler)
		    : SIMD::Pointer(image)
		    , samplerId(sampler)
		{}
		Object::ID samplerId;
	};

	// Generic wrapper over either per-lane intermediate value, or a constant.
	// Constants are transparently widened to per-lane values in operator[].
	// This is appropriate in most cases -- if we're not going to do something
	// significantly different based on whether the value is uniform across lanes.
	class Operand
	{
	public:
		Operand(const Spirv &shader, const SpirvEmitter &state, Object::ID objectId);
		Operand(const Intermediate &value);

		RValue<SIMD::Float> Float(uint32_t i) const
		{
			ASSERT(i < componentCount);

			if(intermediate)
			{
				return intermediate->Float(i);
			}

			// Constructing a constant SIMD::Float is not guaranteed to preserve the data's exact
			// bit pattern, but SPIR-V provides 32-bit words representing "the bit pattern for the constant".
			// Thus we must first construct an integer constant, and bitcast to float.
			return As<SIMD::Float>(SIMD::UInt(constant[i]));
		}

		RValue<SIMD::Int> Int(uint32_t i) const
		{
			ASSERT(i < componentCount);

			if(intermediate)
			{
				return intermediate->Int(i);
			}

			return SIMD::Int(constant[i]);
		}

		RValue<SIMD::UInt> UInt(uint32_t i) const
		{
			ASSERT(i < componentCount);

			if(intermediate)
			{
				return intermediate->UInt(i);
			}

			return SIMD::UInt(constant[i]);
		}

		const SIMD::Pointer &Pointer() const
		{
			ASSERT(intermediate == nullptr);

			return *pointer;
		}

		bool isPointer() const
		{
			return (pointer != nullptr);
		}

		const SampledImagePointer &SampledImage() const
		{
			ASSERT(intermediate == nullptr);

			return *sampledImage;
		}

		bool isSampledImage() const
		{
			return (sampledImage != nullptr);
		}

	private:
		RR_PRINT_ONLY(friend struct rr::PrintValue::Ty<Operand>;)

		// Delegate constructor
		Operand(const SpirvEmitter &state, const Object &object);

		const uint32_t *constant = nullptr;
		const Intermediate *intermediate = nullptr;
		const SIMD::Pointer *pointer = nullptr;
		const SampledImagePointer *sampledImage = nullptr;

	public:
		const uint32_t componentCount;
	};

	RR_PRINT_ONLY(friend struct rr::PrintValue::Ty<Operand>;)

	Intermediate &createIntermediate(Object::ID id, uint32_t componentCount)
	{
		auto it = intermediates.emplace(std::piecewise_construct,
		                                std::forward_as_tuple(id),
		                                std::forward_as_tuple(componentCount));
		ASSERT_MSG(it.second, "Intermediate %d created twice", id.value());
		return it.first->second;
	}

	const Intermediate &getIntermediate(Object::ID id) const
	{
		auto it = intermediates.find(id);
		ASSERT_MSG(it != intermediates.end(), "Unknown intermediate %d", id.value());
		return it->second;
	}

	void createPointer(Object::ID id, SIMD::Pointer ptr)
	{
		bool added = pointers.emplace(id, ptr).second;
		ASSERT_MSG(added, "Pointer %d created twice", id.value());
	}

	const SIMD::Pointer &getPointer(Object::ID id) const
	{
		auto it = pointers.find(id);
		ASSERT_MSG(it != pointers.end(), "Unknown pointer %d", id.value());
		return it->second;
	}

	void createSampledImage(Object::ID id, SampledImagePointer ptr)
	{
		bool added = sampledImages.emplace(id, ptr).second;
		ASSERT_MSG(added, "Sampled image %d created twice", id.value());
	}

	const SampledImagePointer &getSampledImage(Object::ID id) const
	{
		auto it = sampledImages.find(id);
		ASSERT_MSG(it != sampledImages.end(), "Unknown sampled image %d", id.value());
		return it->second;
	}

	bool isSampledImage(Object::ID id) const
	{
		return sampledImages.find(id) != sampledImages.end();
	}

	const SIMD::Pointer &getImage(Object::ID id) const
	{
		return isSampledImage(id) ? getSampledImage(id) : getPointer(id);
	}

	void EmitVariable(InsnIterator insn);
	void EmitLoad(InsnIterator insn);
	void EmitStore(InsnIterator insn);
	void EmitAccessChain(InsnIterator insn);
	void EmitCompositeConstruct(InsnIterator insn);
	void EmitCompositeInsert(InsnIterator insn);
	void EmitCompositeExtract(InsnIterator insn);
	void EmitVectorShuffle(InsnIterator insn);
	void EmitVectorTimesScalar(InsnIterator insn);
	void EmitMatrixTimesVector(InsnIterator insn);
	void EmitVectorTimesMatrix(InsnIterator insn);
	void EmitMatrixTimesMatrix(InsnIterator insn);
	void EmitOuterProduct(InsnIterator insn);
	void EmitTranspose(InsnIterator insn);
	void EmitVectorExtractDynamic(InsnIterator insn);
	void EmitVectorInsertDynamic(InsnIterator insn);
	void EmitUnaryOp(InsnIterator insn);
	void EmitBinaryOp(InsnIterator insn);
	void EmitDot(InsnIterator insn);
	void EmitSelect(InsnIterator insn);
	void EmitExtendedInstruction(InsnIterator insn);
	void EmitExtGLSLstd450(InsnIterator insn);
	void EmitAny(InsnIterator insn);
	void EmitAll(InsnIterator insn);
	void EmitBranch(InsnIterator insn);
	void EmitBranchConditional(InsnIterator insn);
	void EmitSwitch(InsnIterator insn);
	void EmitUnreachable(InsnIterator insn);
	void EmitReturn(InsnIterator insn);
	void EmitTerminateInvocation(InsnIterator insn);
	void EmitDemoteToHelperInvocation(InsnIterator insn);
	void EmitIsHelperInvocation(InsnIterator insn);
	void EmitFunctionCall(InsnIterator insn);
	void EmitPhi(InsnIterator insn);
	void EmitImageSample(const ImageInstruction &instruction);
	void EmitImageQuerySizeLod(InsnIterator insn);
	void EmitImageQuerySize(InsnIterator insn);
	void EmitImageQueryLevels(InsnIterator insn);
	void EmitImageQuerySamples(InsnIterator insn);
	void EmitImageRead(const ImageInstruction &instruction);
	void EmitImageWrite(const ImageInstruction &instruction);
	void EmitImageTexelPointer(const ImageInstruction &instruction);
	void EmitAtomicOp(InsnIterator insn);
	void EmitAtomicCompareExchange(InsnIterator insn);
	void EmitSampledImage(InsnIterator insn);
	void EmitImage(InsnIterator insn);
	void EmitCopyObject(InsnIterator insn);
	void EmitCopyMemory(InsnIterator insn);
	void EmitControlBarrier(InsnIterator insn);
	void EmitMemoryBarrier(InsnIterator insn);
	void EmitGroupNonUniform(InsnIterator insn);
	void EmitArrayLength(InsnIterator insn);
	void EmitBitcastPointer(Object::ID resultID, Operand &src);

	enum InterpolationType
	{
		Centroid,
		AtSample,
		AtOffset,
	};
	SIMD::Float EmitInterpolate(const SIMD::Pointer &ptr, int32_t location, Object::ID paramId,
	                            uint32_t component, InterpolationType type) const;

	SIMD::Pointer WalkExplicitLayoutAccessChain(Object::ID id, Object::ID elementId, const Span &indexIds, bool nonUniform) const;
	SIMD::Pointer WalkAccessChain(Object::ID id, Object::ID elementId, const Span &indexIds, bool nonUniform) const;

	// Returns true if data in the given storage class is word-interleaved
	// by each SIMD vector lane, otherwise data is stored linerally.
	//
	// Each lane addresses a single word, picked by a base pointer and an
	// integer offset.
	//
	// A word is currently 32 bits (single float, int32_t, uint32_t).
	// A lane is a single element of a SIMD vector register.
	//
	// Storage interleaved by lane - (IsStorageInterleavedByLane() == true):
	// ---------------------------------------------------------------------
	//
	// Address = PtrBase + sizeof(Word) * (SIMD::Width * LaneOffset + LaneIndex)
	//
	// Assuming SIMD::Width == 4:
	//
	//                   Lane[0]  |  Lane[1]  |  Lane[2]  |  Lane[3]
	//                 ===========+===========+===========+==========
	//  LaneOffset=0: |  Word[0]  |  Word[1]  |  Word[2]  |  Word[3]
	// ---------------+-----------+-----------+-----------+----------
	//  LaneOffset=1: |  Word[4]  |  Word[5]  |  Word[6]  |  Word[7]
	// ---------------+-----------+-----------+-----------+----------
	//  LaneOffset=2: |  Word[8]  |  Word[9]  |  Word[a]  |  Word[b]
	// ---------------+-----------+-----------+-----------+----------
	//  LaneOffset=3: |  Word[c]  |  Word[d]  |  Word[e]  |  Word[f]
	//
	//
	// Linear storage - (IsStorageInterleavedByLane() == false):
	// ---------------------------------------------------------
	//
	// Address = PtrBase + sizeof(Word) * LaneOffset
	//
	//                   Lane[0]  |  Lane[1]  |  Lane[2]  |  Lane[3]
	//                 ===========+===========+===========+==========
	//  LaneOffset=0: |  Word[0]  |  Word[0]  |  Word[0]  |  Word[0]
	// ---------------+-----------+-----------+-----------+----------
	//  LaneOffset=1: |  Word[1]  |  Word[1]  |  Word[1]  |  Word[1]
	// ---------------+-----------+-----------+-----------+----------
	//  LaneOffset=2: |  Word[2]  |  Word[2]  |  Word[2]  |  Word[2]
	// ---------------+-----------+-----------+-----------+----------
	//  LaneOffset=3: |  Word[3]  |  Word[3]  |  Word[3]  |  Word[3]
	//

	static bool IsStorageInterleavedByLane(spv::StorageClass storageClass);
	static SIMD::Pointer GetElementPointer(SIMD::Pointer structure, uint32_t offset, spv::StorageClass storageClass);

	// Returns a SIMD::Pointer to the underlying data for the given pointer
	// object.
	// Handles objects of the following kinds:
	//  - DescriptorSet
	//  - Pointer
	//  - InterfaceVariable
	// Calling GetPointerToData with objects of any other kind will assert.
	SIMD::Pointer GetPointerToData(Object::ID id, SIMD::Int arrayIndex, bool nonUniform) const;
	void OffsetToElement(SIMD::Pointer &ptr, Object::ID elementId, int32_t arrayStride) const;

	/* image istructions */

	// Emits code to sample an image, regardless of whether any SIMD lanes are active.
	void EmitImageSampleUnconditional(Array<SIMD::Float> &out, const ImageInstruction &instruction) const;

	Pointer<Byte> getSamplerDescriptor(Pointer<Byte> imageDescriptor, const ImageInstruction &instruction) const;
	Pointer<Byte> getSamplerDescriptor(Pointer<Byte> imageDescriptor, const ImageInstruction &instruction, int laneIdx) const;
	Pointer<Byte> lookupSamplerFunction(Pointer<Byte> imageDescriptor, Pointer<Byte> samplerDescriptor, const ImageInstruction &instruction) const;
	void callSamplerFunction(Pointer<Byte> samplerFunction, Array<SIMD::Float> &out, Pointer<Byte> imageDescriptor, const ImageInstruction &instruction) const;

	void GetImageDimensions(const Type &resultTy, Object::ID imageId, Object::ID lodId, Intermediate &dst) const;
	struct TexelAddressData
	{
		bool isArrayed;
		spv::Dim dim;
		int dims, texelSize;
		SIMD::Int u, v, w, ptrOffset;
	};
	static TexelAddressData setupTexelAddressData(SIMD::Int rowPitch, SIMD::Int slicePitch, SIMD::Int samplePitch, ImageInstructionSignature instruction, SIMD::Int coordinate[], SIMD::Int sample, vk::Format imageFormat, const SpirvRoutine *routine);
	static SIMD::Pointer GetNonUniformTexelAddress(ImageInstructionSignature instruction, SIMD::Pointer descriptor, SIMD::Int coordinate[], SIMD::Int sample, vk::Format imageFormat, OutOfBoundsBehavior outOfBoundsBehavior, SIMD::Int activeLaneMask, const SpirvRoutine *routine);
	static SIMD::Pointer GetTexelAddress(ImageInstructionSignature instruction, Pointer<Byte> descriptor, SIMD::Int coordinate[], SIMD::Int sample, vk::Format imageFormat, OutOfBoundsBehavior outOfBoundsBehavior, const SpirvRoutine *routine);
	static void WriteImage(ImageInstructionSignature instruction, Pointer<Byte> descriptor, const Pointer<SIMD::Int> &coord, const Pointer<SIMD::Int> &texelAndMask, vk::Format imageFormat);

	/* control flow */

	// Lookup the active lane mask for the edge from -> to.
	// If from is unreachable, then a mask of all zeros is returned.
	// Asserts if from is reachable and the edge does not exist.
	RValue<SIMD::Int> GetActiveLaneMaskEdge(Block::ID from, Block::ID to) const;

	// Updates the current active lane mask.
	void SetActiveLaneMask(RValue<SIMD::Int> mask);
	void SetStoresAndAtomicsMask(RValue<SIMD::Int> mask);

	// Emit all the unvisited blocks (except for ignore) in DFS order,
	// starting with id.
	void EmitBlocks(Block::ID id, Block::ID ignore = 0);
	void EmitNonLoop();
	void EmitLoop();

	void EmitInstructions(InsnIterator begin, InsnIterator end);
	void EmitInstruction(InsnIterator insn);

	// Helper for implementing OpStore, which doesn't take an InsnIterator so it
	// can also store independent operands.
	void Store(Object::ID pointerId, const Operand &value, bool atomic, std::memory_order memoryOrder) const;

	// LoadPhi loads the phi values from the alloca storage and places the
	// load values into the intermediate with the phi's result id.
	void LoadPhi(InsnIterator insn);

	// StorePhi updates the phi's alloca storage value using the incoming
	// values from blocks that are both in the OpPhi instruction and in
	// filter.
	void StorePhi(Block::ID blockID, InsnIterator insn, const std::unordered_set<Block::ID> &filter);

	// Emits a rr::Fence for the given MemorySemanticsMask.
	void Fence(spv::MemorySemanticsMask semantics) const;

	void Yield(YieldResult res) const;

	// Helper as we often need to take dot products as part of doing other things.
	static SIMD::Float FDot(unsigned numComponents, const Operand &x, const Operand &y);
	static SIMD::Int SDot(unsigned numComponents, const Operand &x, const Operand &y, const Operand *accum);
	static SIMD::UInt UDot(unsigned numComponents, const Operand &x, const Operand &y, const Operand *accum);
	static SIMD::Int SUDot(unsigned numComponents, const Operand &x, const Operand &y, const Operand *accum);
	static SIMD::Int AddSat(RValue<SIMD::Int> a, RValue<SIMD::Int> b);
	static SIMD::UInt AddSat(RValue<SIMD::UInt> a, RValue<SIMD::UInt> b);

	using ImageSampler = void(void *texture, void *uvsIn, void *texelOut, void *constants);
	static ImageSampler *getImageSampler(const vk::Device *device, uint32_t signature, uint32_t samplerId, uint32_t imageViewId);
	static std::shared_ptr<rr::Routine> emitSamplerRoutine(ImageInstructionSignature instruction, const Sampler &samplerState);
	static std::shared_ptr<rr::Routine> emitWriteRoutine(ImageInstructionSignature instruction, const Sampler &samplerState);

	// TODO(b/129523279): Eliminate conversion and use vk::Sampler members directly.
	static sw::FilterType convertFilterMode(const vk::SamplerState *samplerState, VkImageViewType imageViewType, SamplerMethod samplerMethod);
	static sw::MipmapType convertMipmapMode(const vk::SamplerState *samplerState);
	static sw::AddressingMode convertAddressingMode(int coordinateIndex, const vk::SamplerState *samplerState, VkImageViewType imageViewType);

	const SpirvShader &shader;
	SpirvRoutine *const routine;                     // The current routine being built.
	Spirv::Function::ID function;                    // The current function being built.
	Block::ID block;                                 // The current block being built.
	rr::Value *activeLaneMaskValue = nullptr;        // The current active lane mask.
	rr::Value *storesAndAtomicsMaskValue = nullptr;  // The current atomics mask.
	Spirv::Block::Set visited;                       // Blocks already built.
	std::unordered_map<Block::Edge, RValue<SIMD::Int>, Block::Edge::Hash> edgeActiveLaneMasks;
	std::deque<Block::ID> *pending;

	const vk::Attachments *attachments;
	const vk::DescriptorSet::Bindings &descriptorSets;

	std::unordered_map<Object::ID, Intermediate> intermediates;
	std::unordered_map<Object::ID, std::vector<SIMD::Float>> phis;
	std::unordered_map<Object::ID, SIMD::Pointer> pointers;
	std::unordered_map<Object::ID, SampledImagePointer> sampledImages;

	const unsigned int multiSampleCount;
};

class SpirvRoutine
{
	using Object = Spirv::Object;

public:
	SpirvRoutine(const vk::PipelineLayout *pipelineLayout);

	using Variable = Array<SIMD::Float>;

	// Single-entry 'inline' sampler routine cache.
	struct SamplerCache
	{
		Pointer<Byte> imageDescriptor = nullptr;
		Int samplerId;

		Pointer<Byte> function;
	};

	enum Interpolation
	{
		Perspective = 0,
		Linear,
		Flat,
	};

	struct InterpolationData
	{
		Pointer<Byte> primitive;
		SIMD::Float x;
		SIMD::Float y;
		SIMD::Float rhw;
		SIMD::Float xCentroid;
		SIMD::Float yCentroid;
		SIMD::Float rhwCentroid;
	};

	const vk::PipelineLayout *const pipelineLayout;

	std::unordered_map<Object::ID, Variable> variables;
	std::unordered_map<uint32_t, SamplerCache> samplerCache;  // Indexed by the instruction position, in words.
	SIMD::Float inputs[MAX_INTERFACE_COMPONENTS];
	Interpolation inputsInterpolation[MAX_INTERFACE_COMPONENTS];
	SIMD::Float outputs[MAX_INTERFACE_COMPONENTS];
	InterpolationData interpolationData;

	Pointer<Byte> device;
	Pointer<Byte> workgroupMemory;
	Pointer<Pointer<Byte>> descriptorSets;
	Pointer<Int> descriptorDynamicOffsets;
	Pointer<Byte> pushConstants;
	Pointer<Byte> constants;
	Int discardMask = 0;

	// Shader invocation state.
	// Not all of these variables are used for every type of shader, and some
	// are only used when debugging. See b/146486064 for more information.
	// Give careful consideration to the runtime performance loss before adding
	// more state here.
	std::array<SIMD::Int, 2> windowSpacePosition;  // TODO(b/236162233): SIMD::Int2
	Int layer;                                     // slice offset into input attachments for multiview, even if the shader doesn't use ViewIndex
	Int instanceID;
	SIMD::Int vertexIndex;
	std::array<SIMD::Float, 4> fragCoord;   // TODO(b/236162233): SIMD::Float4
	std::array<SIMD::Float, 2> pointCoord;  // TODO(b/236162233): SIMD::Float2
	SIMD::Int helperInvocation;
	Int4 numWorkgroups;
	Int4 workgroupID;
	Int4 workgroupSize;
	Int subgroupsPerWorkgroup;
	Int invocationsPerSubgroup;
	Int subgroupIndex;
	SIMD::Int localInvocationIndex;
	std::array<SIMD::Int, 3> localInvocationID;   // TODO(b/236162233): SIMD::Int3
	std::array<SIMD::Int, 3> globalInvocationID;  // TODO(b/236162233): SIMD::Int3

	void createVariable(Object::ID id, uint32_t componentCount)
	{
		bool added = variables.emplace(id, Variable(componentCount)).second;
		ASSERT_MSG(added, "Variable %d created twice", id.value());
	}

	Variable &getVariable(Object::ID id)
	{
		auto it = variables.find(id);
		ASSERT_MSG(it != variables.end(), "Unknown variables %d", id.value());
		return it->second;
	}

	// setImmutableInputBuiltins() sets all the immutable input builtins,
	// common for all shader types.
	void setImmutableInputBuiltins(const SpirvShader *shader);

	static SIMD::Float interpolateAtXY(const SIMD::Float &x, const SIMD::Float &y, const SIMD::Float &rhw, Pointer<Byte> planeEquation, Interpolation interpolation);

	// setInputBuiltin() calls f() with the builtin and value if the shader
	// uses the input builtin, otherwise the call is a no-op.
	// F is a function with the signature:
	// void(const Spirv::BuiltinMapping& builtin, Array<SIMD::Float>& value)
	template<typename F>
	inline void setInputBuiltin(const SpirvShader *shader, spv::BuiltIn id, F &&f)
	{
		auto it = shader->inputBuiltins.find(id);
		if(it != shader->inputBuiltins.end())
		{
			const auto &builtin = it->second;
			f(builtin, getVariable(builtin.Id));
		}
	}
};

}  // namespace sw

#endif  // sw_SpirvShader_hpp
