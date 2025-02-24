/*!
\brief A pvr::assets::Effect is the description of the entire rendering setup and can be used to create
objects and use them for rendering.
\file PVRCore/pfx/Effect.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/texture/Texture.h"
#include "PVRCore/types/GpuDataTypes.h"
#include "PVRCore/types/Types.h"
#include "PVRCore/stream/Stream.h"
#include "PVRCore/strings/StringHash.h"
#include <set>

namespace pvr {
/// <summary>The possible scope, or frequency of change, of a variable</summary>
enum class VariableScope
{
	Unknown, ///< Scope is unknown, undefined, or custom
	Automatic, ///< Scope is being inferred from use
	Model, ///< The variable is per model (stays constant for a given model, for example, ViewMatrix)
	Effect, ///< The variable is per effect (stays constant for a given effect, for example, directional light intensity)
	Node, ///< The variable is per Mesh Node (stays constant for a given node of a model, for example, ModelViewProjection matrix)
	BoneBatch, ///< The variable is per Bone Batch (deprecated - stays constant for a given Bone Batch, for example, bones)
};

namespace effect {
//!\cond NO_DOXYGEN
template<typename MyType_>
class NameComparable
{
public:
	StringHash name;
	NameComparable() {}
	NameComparable(StringHash&& name) : name(std::move(name)) {}
	NameComparable(const StringHash& name) : name(name) {}

	typedef MyType_ MyType;
	bool operator<(const MyType& rhs) const { return name < rhs.name; }
	bool operator>(const MyType& rhs) const { return name > rhs.name; }

	bool operator>=(const MyType& rhs) const { return name >= rhs.name; }
	bool operator<=(const MyType& rhs) const { return name <= rhs.name; }

	bool operator==(const MyType& rhs) const { return name == rhs.name; }
	bool operator!=(const MyType& rhs) const { return name != rhs.name; }
};
//!\endcond

/// <summary>Stores effect texture information.</summary>
struct TextureDefinition : public NameComparable<TextureDefinition>
{
	StringHash path; ///< File name
	uint32_t width; ///< Texture width
	uint32_t height; ///< Texture height
	ImageDataFormat format; ///< Texture format
	/// <summary>Constructor. Non-initializing.</summary>
	TextureDefinition() {}
	/// <summary>Constructor from individual elements</summary>
	/// <param name="name">The name of the texture</param>
	/// <param name="path">The path of the texture data</param>
	/// <param name="width">Width of the texture</param>
	/// <param name="height">Height of the texture</param>
	/// <param name="format">Format of the texture</param>
	TextureDefinition(const StringHash& name, const StringHash& path, uint32_t width, uint32_t height, const ImageDataFormat& format)
		: NameComparable(name), path(path), width(width), height(height), format(format)
	{}

	/// <summary>Check if this texture definition is read from a file (or defined by the application)</summary>
	/// <returns>True if a file, otherwise false<returns>
	bool isFile() { return path.empty(); }
};

/// <summary>Stores effect texture information.</summary>
struct TextureRef
{
	StringHash textureName; ///< Name of the texture
	uint8_t set; ///< Descriptor set number where the texture is bound
	uint8_t binding; ///< Binding number of the texture in the set
	StringHash variableName; ///< The variable name that this texture refers to in the shader.

	/// <summary>Constructor</summary>
	TextureRef() {}

	/// <summary>Constructor</summary>
	/// <param name="textureName">Name of the texture definition referenced</param>
	/// <param name="set">The descriptor set number to which this texture will be bound</param>
	/// <param name="binding">The index in the descriptor set where this texture will be bound</param>
	/// <param name="variableName">The variable name of the sampler in the shader. Only used for reflective
	/// shader apis (like OpenGL ES) with non-explicit bindings.</param>
	TextureRef(StringHash textureName, uint8_t set, uint8_t binding, StringHash variableName) : textureName(textureName), set(set), binding(binding), variableName(variableName) {}
};

/// <summary>Stores effect texture information.</summary>
struct TextureReference : public TextureRef
{
	PackedSamplerFilter samplerFilter; ///< Sampler Filters
	SamplerAddressMode wrapS; ///< Either Clamp or Repeat
	SamplerAddressMode wrapT; ///< Either Clamp or Repeat
	SamplerAddressMode wrapR; ///< Either Clamp or Repeat
	StringHash semantic; ///< The semantic from which this texture will get its value.
};

/// <summary>Store effect data from the shader block.</summary>
struct Shader : public NameComparable<Shader>
{
	std::string source; //!< Source data of the Shader
	ShaderType type; //!< Type of the shader
	/// <summary>Constructor. Non-initializing.</summary>
	Shader() {}
	/// <summary>Constructor from individual elements.</summary>
	/// <param name="name">The identifier of the shader.</param>
	/// <param name="type">The type of the shader (Vertex, Fragment etc)</param>
	/// <param name="source">The data of the shader (text or binary)</param>
	Shader(StringHash&& name, ShaderType type, std::string&& source) : NameComparable<Shader>(std::move(name)), source(std::move(source)), type(type) {}
};
/// <summary>Pointer to a shader</summary>
typedef const Shader* ShaderReference;

/// <summary>Information about a Buffer</summary>
struct BufferDefinition : public NameComparable<BufferDefinition>
{
	/// <summary>A variable entry into the buffer definition</summary>
	struct Entry
	{
		StringHash semantic; ///< The semantic name of the variable
		GpuDatatypes dataType; ///< The datatype of the variable
		uint32_t arrayElements; ///< If an array, the number of elements
	};
	BufferUsageFlags allSupportedBindings; ///< The binding types this buffer supports
	bool isDynamic; ///< If the buffer can be piecemally (dynamically) bound
	std::vector<Entry> entries; ///< Storage of the variable entries
	VariableScope scope; ///< Scope of the buffer
	bool multibuffering; ///< If this buffer requires to have one instance per frame
	BufferDefinition() : allSupportedBindings(BufferUsageFlags(0)), isDynamic(false), scope(VariableScope::Effect), multibuffering(0) {}
};

/// <summary>Base class for classes that represent semantic references to an image or buffer</summary>
struct DescriptorRef
{
	int8_t set; ///< The descriptor set index where the object will be bound
	int8_t binding; ///< The index of the binding inside the descriptor set
	DescriptorRef() : set(0), binding(0) {}
};

/// <summary>A Reference to a buffer object inside the effect. Can be accessed either in its
/// entirety through a Semantic, or through the semantics of each of its elements.</summary>
struct BufferRef : DescriptorRef
{
	/// <summary>Optionally, the Semantic to provide the value for this entire buffer reference
	/// (otherwise you can /// use the variable semantics of its contents)
	StringHash semantic;
	StringHash bufferName; ///< The name (identifier) of the buffer
	DescriptorType type; ///< The descriptor type of this buffer reference
};

/// <summary>A Reference to a Uniform semantic (a free-standing shader variable).</summary>
struct UniformSemantic : DescriptorRef
{
	StringHash semantic; ///< The Semantic that should provide the value of this uniform
	StringHash variableName; ///< The variable name of the uniform in the shader
	GpuDatatypes dataType; ///< The datatype of the semantic
	uint32_t arrayElements; ///< If an array, the number of elements
	VariableScope scope; ///< The scope (i.e. rate of change) of the semantic
};

/// <summary>A Reference to an Attribute semantic.</summary>
struct AttributeSemantic
{
	StringHash semantic; ///< The Semantic that should provide the value of this attribute
	StringHash variableName; ///< The variable name of the attribute in the shader
	GpuDatatypes dataType; ///< The datatype of the semantic
	uint8_t location; ///< The attribute index
	uint8_t vboBinding; ///< The binding index of the VBO (which VBO binding this attibute should be sourced from
};

/// <summary>A Reference to an Input Attachment.</summary>
struct InputAttachmentRef : DescriptorRef
{
	int8_t targetIndex; ///< The input attachment target index
	InputAttachmentRef() : targetIndex(-1) {}
};

/// <summary>A description of binding a vertex to a pipeline.</summary>
struct PipelineVertexBinding
{
	uint32_t index; ///< The index where to bind this attribute
	StepRate stepRate; ///< The step rate of the vertex

	/// <summary>Constructor.</summary>
	PipelineVertexBinding() : stepRate(StepRate::Vertex) {}

	/// <summary>Constructor.</summary>
	/// <param name="index">The vertex attribute index</param>
	/// <param name="stepRate">The vertex step rate (Vertex, Draw call)</param>
	PipelineVertexBinding(uint32_t index, StepRate stepRate) : index(index), stepRate(stepRate) {}
};

/// <summary>A definition of a pipeline object/configuration.</summary>
struct PipelineDefinition : public NameComparable<PipelineDefinition>
{
	std::vector<ShaderReference> shaders; ///< The Shaders this pipeline uses
	std::vector<UniformSemantic> uniforms; ///< The Uniforms that are defined by this pipeline
	std::vector<AttributeSemantic> attributes; ///< The Attributes that are defined by this pipeline
	std::vector<TextureReference> textures; ///< The Textures that are referenced.
	std::vector<BufferRef> buffers; ///< The buffers that are referenced.
	BlendingConfig blending; ///< The blending configuration
	std::vector<InputAttachmentRef> inputAttachments; ///< The input attachments referenced
	std::vector<PipelineVertexBinding> vertexBinding; ///< The bindings of vertex attributes
	// depth states
	bool enableDepthTest; ///< Is depth test enabled
	bool enableDepthWrite; ///< Is depth write enabled
	CompareOp depthCmpFunc; ///< Depth testing comparison function

	// stencil states
	bool enableStencilTest; ///< Is stencil test enabled
	StencilState stencilFront; ///< Stencil test state for Front facing polygons
	StencilState stencilBack; ///< Stencil test state for Back facing polygons

	// rasterization states
	PolygonWindingOrder windingOrder; ///< Polygon winding order (is Clockwise or Counterclockwise front?)
	Face cullFace; ///< Face culling mode (Cull front, back, both or none?)

	/// <summary>Constructor. initializes to no depth/stencil test/write, no backface culling.
	PipelineDefinition()
		: enableDepthTest(false), enableDepthWrite(true), depthCmpFunc(CompareOp::Less), enableStencilTest(false), windingOrder(PolygonWindingOrder::FrontFaceCCW), cullFace(Face::None)
	{}
};

/// <summary>A Condition that can be used to select a specific pipeline to render a specific object</summary>
struct PipelineCondition
{
	/// <summary>The type of the condition</summary>
	enum ConditionType
	{
		Always, ///< The pipeline is always selected
		UniformRequired, ///< The pipeline requires a specific Uniform Semantic to be provided by the mesh (e.g. must have "NORMALMAP" texture)
		AttributeRequired, ///< The pipeline requires a specific Attribute Semantic to be provided by the mesh (e.g. must have "BONEWEIGHT" attribute)
		UniformRequiredNo, ///< The pipeline requires that a specific Uniform semantic is NOT present (e.g. must not have "GLOSSINESS" texture)
		AttributeRequiredNo, ///< The pipeline requires that a specific Attribute semantic is NOT present (e.g. must not have "BITANGENT" attribute)
		AdditionalExport, ///< Forces a semantic to be set to a value
	} type; //!< The type of the condition
	StringHash value; ///< The actual value of the condition (for example the value "BONEWEIGHT" of AttributeRequired condition.
};

/// <summary>A Reference to a pipeline definition (used in the composition of the effects)</summary>
struct PipelineReference
{
	StringHash pipelineName; ///< The name of the referenced pipeline
	std::vector<PipelineCondition> conditions; ///< The conditions of the pipeline. Used to select different pipelines for different objects
	std::vector<StringHash> identifiers; ///< Any custom semantic identifiers that a pipeline reference may have exported with the conditions
};

/// <summary>A collection of one or more pipelines, one of which should be conditionally picked per object to render this subpass.</summary>
struct SubpassGroup
{
	StringHash name; ///< The name of the group
	std::vector<PipelineReference> pipelines; ///< The group of pipeline references (pipelines with their selection criteria)
};

/// <summary>A collection of subpass groups that will be sequentially executed. The different groups can be used to set the drawing order within
/// the same subpass.</summary>
struct Subpass
{
	/// <summary>Specifies maximum values which can be used for Subpasses.</summary>
	enum
	{
		MaxTargets = 4,
		MaxInputs = 4
	};
	StringHash targets[MaxTargets]; ///< The targets of this subpass. May be intermediate textures/attachments, or the final render target
	StringHash inputs[MaxInputs]; ///< The inputs to this subpass (normally outputs from a previous subpass)
	bool useDepthStencil; ///< If this subpass has the depth/stencil buffer enabled
	std::vector<SubpassGroup> groups; ///< The groups comprising this subpass
};

/// <summary>One full collection of drawing into a specific render target, from start to finish. Comprised of one or more subpasses.</summary>
struct Pass
{
	StringHash name; ///< The name of this pass
	StringHash targetDepthStencil; ///< The texture to use as the Depth/Stencil buffer for this subpass
	std::vector<Subpass> subpasses; ///< The subpasses comprising this pass
};

/// <summary>An entire effect with all its metadata. An effect is intended to describe an entire, possibly multipass, rendering configuration,
/// for several different objects - potentially an entire scene - by defining all the targets, textures, buffers, different pipelines for different
/// objects, as long as they fulfil some similar conceptual roles (see subpass groups). By using "SEMANTICS", i.e. descriptions of different
/// items of information that may be provided from outside (e.g. the model, the animation system, or other), it provides a clear interface for
/// the application to provide this information and render automatically. For an implementation of a rendering system using that, see the
/// RenderManager in Vulkan Utilities.</summary>
struct Effect
{
	using EffectHandle = std::shared_ptr<Effect>;
	using EffectReader = void (*)(const ::pvr::Stream& stream, Effect& thisEffect);
	/// <param name="reader">An AssetReader of the correct type. Must have a valid Stream opened.</param>
	/// <returns>A handle to the new Asset. Will be null if failed to load.</returns>
	static EffectHandle createWithReader(EffectReader reader, const ::pvr::Stream& stream)
	{
		EffectHandle handle = std::make_shared<Effect>();
		reader(stream, *handle);
		return handle;
	}

	/// <summary>Load the data of this asset from an AssetReader. This function requires an already constructed object,
	/// so it is commonly used to reuse an asset.</summary>
	/// <param name="reader">An AssetReader instance. Can be empty (without a stream).</param>
	/// <param name="stream">A stream that contains the data to load into this asset.</param>
	void loadWithReader(EffectReader reader, const ::pvr::Stream& stream) { reader(stream, *this); }

	StringHash name; ///< The name of this effect
	std::map<StringHash, std::string> headerAttributes; ///< Free name value pairs provided in the header of the effect

	std::map<StringHash, std::map<StringHash, Shader>> versionedShaders; ///< Lists of shaders along with their corresponding apis
	std::map<StringHash, std::map<StringHash, PipelineDefinition>> versionedPipelines; ///< Lists of pipelines, along with their corresponding apis that they apply to

	std::map<StringHash, TextureDefinition> textures; ///< All the (possible) textures defined for this effect
	std::map<StringHash, BufferDefinition> buffers; ///< All the (possible) buffers defined for this effect

	std::vector<Pass> passes; ///< All the passes for this effect
	mutable std::vector<StringHash> versions; ///< All api versions that are defined for this effect

	/// <summary>Get all api versions defined for this effect</summary>
	/// <returns>A vector with all supported version strings</returns>
	const std::vector<StringHash>& getVersions() const
	{
		if (versions.empty())
		{
			for(auto it = versionedPipelines.begin(); it != versionedPipelines.end(); ++it) { versions.emplace_back(it->first); }
		}
		return versions;
	}

	/// <summary>Add an additional supported version.</summary>
	/// <param name="apiName">A string denoting a supported version.</param>
	void addVersion(const StringHash& apiName)
	{
		versionedShaders[apiName];
		versionedPipelines[apiName];
	}

	/// <summary>Add a shader for a specific api version</summary>
	/// <param name="apiName">A string denoting a supported version.</param>
	/// <param name="shader">A Shader object containing the Source and type of the shader.</param>
	void addShader(const StringHash& apiName, Shader&& shader) { versionedShaders[apiName][shader.name] = std::move(shader); }
	/// <summary>Add a shader for a specific api version</summary>
	/// <param name="apiName">A string denoting a supported version.</param>
	/// <param name="shader">A Shader object containing the Source and type of the shader.</param>
	void addShader(const StringHash& apiName, const Shader& shader) { versionedShaders[apiName][shader.name] = shader; }

	/// <summary>Add a texture to this effect object</summary>
	/// <param name="texture">A TextureDefinition object to add to this effect.</param>
	void addTexture(TextureDefinition&& texture) { textures[texture.name] = std::move(texture); }
	/// <summary>Add a texture to this effect object</summary>
	/// <param name="texture">A TextureDefinition object to add to this effect.</param>
	void addTexture(const TextureDefinition& texture) { textures[texture.name] = texture; }

	/// <summary>Add a buffer to this effect object</summary>
	/// <param name="buffer">A BufferDefinition object to add to this effect.</param>
	void addBuffer(BufferDefinition&& buffer) { buffers[buffer.name] = std::move(buffer); }

	/// <summary>Add a buffer to this effect object</summary>
	/// <param name="buffer">A BufferDefinition object to add to this effect.</param>
	void addBuffer(const BufferDefinition& buffer) { buffers[buffer.name] = buffer; }

	/// <summary>Add a pipeline to this effect object for a specific api version</summary>
	/// <param name="apiName">A string for the specific API version for which this pipeline will be defined.</param>
	/// <param name="pipeline">A Pipeline object to add to this effect.</param>
	void addPipeline(const StringHash& apiName, PipelineDefinition&& pipeline)
	{
		versionedPipelines[apiName][pipeline.name] = std::move(pipeline);
		versions.clear();
	}

	/// <summary>Add a pipeline to this effect object for a specific api version</summary>
	/// <param name="apiName">A string for the specific API version for which this pipeline will be defined.</param>
	/// <param name="pipeline">A Pipeline object to add to this effect.</param>
	void addPipeline(const StringHash& apiName, const PipelineDefinition& pipeline)
	{
		versionedPipelines[apiName][pipeline.name] = pipeline;
		versions.clear();
	}

	/// <summary>Empty this effect object</summary>
	void clear()
	{
		name.clear();
		headerAttributes.clear();
		passes.clear();
		textures.clear();
		buffers.clear();
		versionedPipelines.clear();
		versionedShaders.clear();
	}
};

} // namespace effect
typedef effect::Effect Effect; ///< Typedef of the effect
} // namespace pvr
