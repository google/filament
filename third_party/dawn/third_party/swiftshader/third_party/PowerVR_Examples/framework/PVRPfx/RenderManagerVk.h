/*!
\brief The RenderManager class. Provides basic engine rendering functionality. See class documentation for basic
use.
\file PVRUtils/Vulkan/RenderManagerVk.h
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/SwapchainVk.h"
#include "PVRVk/CommandPoolVk.h"
#include "PVRVk/CommandBufferVk.h"
#include "PVRVk/FramebufferVk.h"
#include "PVRVk/QueueVk.h"
#include "PVRPfx/EffectVk.h"
#include "PVRUtils/StructuredMemory.h"
#include "PVRVk/FenceVk.h"
#include "PVRAssets/Model.h"
#include <deque>

//#define PVR_RENDERMANAGER_DEBUG

#ifdef PVR_RENDERMANAGER_DEBUG
#define PVR_RENDERMANAGER_DEBUG_SEMANTIC_UPDATES
#define PVR_RENDERMANAGER_DEBUG_BUFFERS
#define PVR_RENDERMANAGER_DEBUG_RENDERING_COMMANDS
#endif

namespace pvr {
namespace utils {

/// <summary>Information about an attribute layout.</summary>
struct Attribute
{
	StringHash semantic; //!< Semantic name
	std::string variableName; //!< attribute name
	DataType datatype; //!< Data type of each element of the attribute
	uint16_t offset; //!< Offset of the first element in the buffer
	uint16_t width; //!< Number of elements in attribute, e.g 1,2,3,4

	/// <summary>Default constructor. Uninitialized values, except for variableName and semantic.</summary>
	Attribute() : datatype(DataType::None), offset(0), width(0) {}

	/// <summary>Create a new Attribute object.</summary>
	/// <param name="semantic">The name of the semantic using the attribute</param>
	/// <param name="datatype">Attribute data type</param>
	/// <param name="width">Number of elements in attribute</param>
	/// <param name="offset">Interleaved: offset of the attribute from the start of data of each vertex</param>
	/// <param name="variableName">Name of the attribute.</param>
	Attribute(StringHash semantic, DataType datatype, uint16_t width, uint16_t offset, const std::string& variableName)
		: semantic(semantic), variableName(variableName), datatype(datatype), offset(offset), width(width)
	{}
};

/// <summary>Information about a set of attribute layouts.</summary>
struct AttributeLayout : public std::vector<Attribute>
{
	uint32_t stride; //!< The stride for the set of attributes

	/// <summary>Default constructor.</summary>
	AttributeLayout() : stride(0) {}
};

struct RendermanModel;
class RenderManager;

struct RendermanSubpassGroupModel;
struct RendermanPipeline;
struct RendermanSubpassMaterial;
struct RendermanSubpass;
struct RendermanPass;
struct RendermanEffect;
struct RendermanNode;
struct RendermanSubpassGroup;

/// <summary>INTERNAL. This struct is used to internally store buffers such as ubo and ssbo, with all
/// the necessary binding info.</summary>
struct RendermanBufferDefinition
{
	StringHash name; //!< buffer name
	utils::StructuredBufferView structuredBufferView; //!< pvrvk::Buffer structure
	pvrvk::Buffer buffer; //!< The buffer object
	BufferUsageFlags allSupportedBindings; //!< Allowed usages for this buffer.
	bool isDynamic; //!< If it is a Static buffer, or Dynamic. If dynamic, it is multiplied by its "scope"
	VariableScope scope; //!< buffer scope - Is it global, or does it refer to different objects? At what granularity?
	uint16_t numMultiBuffers; //!< The number of multiple buffers. Dynamic clients get multiplied by that to calculate the number of dynamic slices
	uint32_t numDynamicClients; //!< The number of dynamic clients - will normally just be the number of "scope" objects added to parts of the effect that use this buffer.
	pvr::utils::StructuredMemoryDescription memoryDescription; //!< The buffer structure (before cooking)
	RendermanBufferDefinition() : isDynamic(false), numMultiBuffers(1), numDynamicClients(0) {}
};

/// <summary>This struct is used to store a material.</summary>
/// <remarks>(Exists to avoid duplication between different textures.)</remarks>
struct RendermanMaterial
{
	RendermanModel* renderModel_; //!< the model this material belongs to
	std::map<StringHash, pvrvk::ImageView> textures; //!< material textures
	assets::MaterialHandle assetMaterial; //!< asset material
	uint32_t assetMaterialId; //!< material id

	/// <summary>Return RendermanModel which own this object (const).</summary>
	/// <returns>RendermanModel</returns>
	const RendermanModel& backToRendermanModel() const;

	/// <summary>Return RendermanModel which own this object</summary>
	/// <returns>RendermanModel</returns>
	const RendermanModel& backToRendermanModel();

	/// <summary>Return RenderManager which own this object (const)</summary>
	/// <returns>Return RenderManager</returns>
	const RenderManager& backToRenderManager() const;

	/// <summary>Return RenderManager which own this object</summary>
	/// <returns>Return RenderManager</returns>
	RenderManager& backToRenderManager();
};

/// <summary>Part of RendermanStructure. This class is used to store VBOs/IBOs. Unique per mesh RendermanNodes
/// inside passes/subpasses reference these items by pointer.</summary>
struct RendermanMesh
{
	RendermanModel* renderModel_; //!< parent rendermodel
	assets::MeshHandle assetMesh; //!< asset mesh handle
	uint32_t assetMeshId; //!< asset mesh id
	std::vector<pvrvk::Buffer> vbos; //!< ONLY ONE - OPTIMISED FOR ALL PIPELINES
	pvrvk::Buffer ibo; //!< ONLY ONE - OPTIMISED FOR ALL PIPELINES
	IndexType indexType; //!< draw index type

	/// <summary>Return RendermanModel which owns this object (const).</summary>
	/// <returns>RendermanModel</returns>
	const RendermanModel& backToRendermanModel() const { return *renderModel_; }

	/// <summary>Return RendermanModel which owns this object.</summary>
	/// <returns>RendermanModel</returns>
	RendermanModel& backToRendermanModel() { return *renderModel_; }

	/// <summary>Return RenderManager which own this object (const)</summary>
	/// <returns>Return RenderManager</returns>
	const RenderManager& backToRenderManager() const;

	/// <summary>Return RenderManager which own this object</summary>
	/// <returns>Return RenderManager</returns>
	RenderManager& backToRenderManager();
};

/// <summary>A pointer to a function that sets a specific Model Semantic from the model to the
/// provided memory object</summary>
typedef bool (*ModelSemanticSetter)(TypedMem& mem, const RendermanModel& model);

/// <summary>Part of RendermanStructure. This class is used to store RendermanMeshes. Unique per model
/// RendermanModelEffects inside passes/subpasses reference these items by pointer.</summary>
struct RendermanModel
{
	RenderManager* mgr_; //!< render manager
	assets::ModelHandle assetModel; //!< handle to the model
	std::deque<RendermanMesh> meshes; //!< renderable meshes
	std::deque<RendermanMaterial> materials; //!< materials

	/// <summary>Get model semantic data</summary>
	/// <param name="semantic">Semantic</param>
	/// <param name="memory">Data returned</param>
	/// <returns>Return true if found</returns>
	bool getModelSemantic(const StringHash& semantic, TypedMem& memory) const;

	/// <summary>Get a Model Semantic Setter for the specified Semantic, if it is a known semantic.
	/// If successful, it will return a pointer to a function that, when called, will get the current
	/// value of the semantic from the Model.</summary>
	/// <param name="semantic">A Semantic name. Should be a per-model semantic provided by the Model.</param>
	/// <returns>The requested semantic function pointer. NULL if the semantic is unknown</returns>
	ModelSemanticSetter getModelSemanticSetter(const StringHash& semantic) const;

	/// <summary>Return RenderManager which own this object (const)</summary>
	/// <returns>Return RenderManager</returns>
	const RenderManager& backToRenderManager() const { return *mgr_; }

	/// <summary>Return RenderManager which own this object</summary>
	/// <returns>Return RenderManager</returns>
	RenderManager& backToRenderManager() { return *mgr_; }

	/// <summary>Get renderman mesh object belongs to this model</summary>
	/// <param name="mesh">Mesh index</param>
	/// <returns>RendermanMesh</returns>
	RendermanMesh& toRendermanMesh(uint32_t mesh)
	{
		debug_assertion(mesh < meshes.size(), "Mesh index out of bound");
		return meshes[mesh];
	}

	/// <summary>Get renderman mesh object belongs to this model (const)</summary>
	/// <param name="mesh">Mesh index</param>
	/// <returns>RendermanMesh</returns>
	const RendermanMesh& toRendermanMesh(uint32_t mesh) const
	{
		debug_assertion(mesh < meshes.size(), "Mesh index out of bound");
		return meshes[mesh];
	}

	/// <summary>Get renderman material object belongs to this model</summary>
	/// <param name="material">Material index</param>
	/// <returns>RendermanMaterial</returns>
	RendermanMaterial& toRendermanMaterial(uint32_t material)
	{
		debug_assertion(material < materials.size(), "material index out of bound");
		return materials[material];
	}

	/// <summary>Get renderman material object belongs to this model</summary>
	/// <param name="name">Material name</param>
	/// <returns>RendermanMaterial</returns>
	RendermanMaterial* toRendermanMaterial(const StringHash& name)
	{
		auto it = std::find_if(materials.begin(), materials.end(), [&](RendermanMaterial& mat) { return name == mat.assetMaterial->getName(); });
		return (it != materials.end() ? &(*it) : nullptr);
	}

	/// <summary>Get renderman material object belongs to this model (const)</summary>
	/// <param name="material">Material index</param>
	/// <returns>RendermanMaterial</returns>
	const RendermanMaterial& toRendermanMaterial(uint32_t material) const
	{
		debug_assertion(material < materials.size(), "material index out of bound");
		return materials[material];
	}
};

/// <summary>Contains information to bind a buffer to a specific pipeline's descriptor sets.</summary>
struct RendermanBufferBinding
{
	RendermanBufferDefinition* bufferDefinition; //!< Points to the buffer definition this binding refers to
	StringHash semantic; //!< If the entire buffer has a semantic as a whole, that semantic. Otherwise empty.
	pvrvk::DescriptorType type; //!< The descriptor type of the buffer
	uint8_t set; //!< The descriptor set index of this binding
	uint8_t binding; //!< The index in the descriptor set of this binding
};

/// <summary>Part of RendermanStructure. This class is a Material's instances as used by a pipeline</summary>
struct RendermanMaterialSubpassPipeline
{
	RendermanPipeline* pipeline_; //!< The pipeline
	RendermanSubpassMaterial* materialSubpass_; //!< A Material / Subpass reference
	Multi<pvrvk::DescriptorSet> sets[4]; //!< The descriptor sets where all the material objects are stored

	/// <summary>Get RendermanPipeline object (const)</summary>
	/// <returns>RendermanPipeline</returns>
	const RendermanPipeline& toPipeline() const;

	/// <summary>Get RendermanPipeline object</summary>
	/// <returns>RendermanPipeline</returns>
	RendermanPipeline& toPipeline();

	/// <summary>Return the RendermanSubpassMaterial object which owns this object (const)</summary>
	/// <returns>RendermanSubpassMaterial</returns>
	const RendermanSubpassMaterial& backToSubpassMaterial() const { return *materialSubpass_; }

	/// <summary>Return the RendermanSubpassMaterial object which owns this object</summary>
	/// <returns>RendermanSubpassMaterial</returns>
	RendermanSubpassMaterial& backToSubpassMaterial() { return *materialSubpass_; }
};

/// <summary>Part of RendermanStructure. This class contains the Material's instances that are used by a pipeline
/// The reason is that a pipeline is selected BOTH by material AND by mesh, making it possible for one
/// material in one subpass to be used by different pipelines.</summary>
struct RendermanSubpassMaterial
{
	std::vector<RendermanMaterialSubpassPipeline> materialSubpassPipelines; //!< The MaterialSubpassPipeline cobination objects that belong to this object
	RendermanSubpassGroupModel* modelSubpass_; //!< The parent Model Subpass
	RendermanMaterial* material; //!< The Material this object uses

	/// <summary>Return RendermanMaterialSubpassPipeline object</summary>
	/// <param name="index">RendermanMaterialSubpassPipeline index</param>
	/// <returns>RendermanMaterialSubpassPipeline</returns>
	RendermanMaterialSubpassPipeline& toMaterialSubpassPipeline(uint32_t index)
	{
		debug_assertion(index < materialSubpassPipelines.size(), "Material subpass pipeline index out of bound");
		return materialSubpassPipelines[index];
	}

	/// <summary>Return RendermanMaterialSubpassPipeline object (const)</summary>
	/// <param name="index">RendermanMaterialSubpassPipeline index</param>
	/// <returns>RendermanMaterialSubpassPipeline</returns>
	const RendermanMaterialSubpassPipeline& toMaterialSubpassPipeline(uint32_t index) const
	{
		debug_assertion(index < materialSubpassPipelines.size(), "Material subpass pipeline index out of bound");
		return materialSubpassPipelines[index];
	}

	/// <summary>Return RendermanSubpassModel object which owns this object (const).</summary>
	/// <returns>RendermanSubpassModel</returns>
	RendermanSubpassGroupModel& backToSubpassGroupModel() { return *modelSubpass_; }

	/// <summary>Return RendermanSubpassModel object which owns this object (const).</summary>
	/// <returns>RendermanSubpassModel</returns>
	const RendermanSubpassGroupModel& backToSubpassGroupModel() const { return *modelSubpass_; }

	/// <summary>Return RendermanModel which owns this object (const)</summary>
	/// <returns>RendermanModel</returns>
	const RendermanModel& backToModel() const;

	/// <summary>Return RendermanModel which owns this object</summary>
	/// <returns>RendermanModel</returns>
	RendermanModel& backToModel();

	/// <summary>Return RendermanSubpass which owns this object.</summary>
	/// <returns>RendermanSubpass</returns>
	RendermanSubpassGroup& backToSubpassGroup();

	/// <summary>Return RendermanSubpass which owns this object (const).</summary>
	/// <returns>RendermanSubpass</returns>
	const RendermanSubpassGroup& backToSubpassGroup() const;

	/// <summary>Return material</summary>
	/// <returns>RendermanMaterial</returns>
	RendermanMaterial& toMaterial() { return *material; }

	/// <summary>Return material (const)</summary>
	/// <returns>RendermanMaterial</returns>
	const RendermanMaterial& toMaterial() const { return *material; }
};

/// <summary>Part of RendermanStructure. This class is a Mesh's instances as used by a pipeline
/// The "usedByPipelines" is only a helper.</summary>
struct RendermanSubpassMesh
{
	RendermanSubpassGroupModel* modelSubpass_; //!< The ModelSubpass that is a parent to this object
	RendermanMesh* rendermesh_; //!< The RenderMesh that is a parent of this object
	std::set<RendermanPipeline*> usedByPipelines; //!< A reference to each pipeline using this object

	/// <summary>Return RendermanSubpassModel object which owns this object</summary>
	/// <returns>RendermanSubpassModel</returns>
	RendermanSubpassGroupModel& backToSubpassGroupModel() { return *modelSubpass_; }

	/// <summary>Return RendermanSubpassModel object which owns this object (const)</summary>
	/// <returns>RendermanSubpassModel</returns>
	const RendermanSubpassGroupModel& backToSubpassGroupModel() const { return *modelSubpass_; }

	/// <summary>Return RendermanModel which owns this object</summary>
	/// <returns>RendermanModel</returns>
	RendermanModel& backToModel();

	/// <summary>Return RendermanModel which owns this object (const).</summary>
	/// <returns>RendermanModel</returns>
	const RendermanModel& backToModel() const;

	/// <summary>Return RendermanSubpass which owns this object (const).</summary>
	/// <returns>RendermanSubpass</returns>
	const RendermanSubpassGroup& backToSubpassGroup() const;

	/// <summary>Return RendermanSubpass which owns this object.</summary>
	/// <returns>RendermanSubpass</returns>
	RendermanSubpassGroup& backToSubpassGroup();

	/// <summary>Return RendermanMesh object which owns this object (const)</summary>
	/// <returns>RendermanMesh</returns>
	RendermanMesh& backToMesh() { return *rendermesh_; }

	/// <summary>Return RendermanMesh object which owns this object (const)</summary>
	/// <returns>RendermanMesh</returns>
	const RendermanMesh& backToMesh() const { return *rendermesh_; }
};

/// <summary>This class contains information for an Effect Semantic that is
/// used as a variable in a pvrvk::Buffer, and stores the connections between all necessary
/// components: The actual buffer, the descriptor set bound to, the layout of the buffer,
/// the dynamic id (if part of a buffer, which slice this object refers to) and so on.
/// Will normally be used with the actual Semantic as a key in a map, hence it does
/// not contain it.</summary>
struct BufferEntrySemantic
{
	pvr::utils::StructuredBufferView* structuredBufferView; //!< A pointer to the structuredBufferView describing the layout of the buffer
	pvrvk::Buffer* buffer; //!< The buffer itself
	uint16_t setId; //!< The descriptor set that the buffer belongs to
	int16_t dynamicOffsetNodeId; //!< In the node's array of dynamic client id's, the actual offset. So, for each node, use dynamicClientIds[setId][dynamicOffsetNodeId]
	uint16_t entryIndex; //!< The index of this entry's inside the structuredBufferView
};

/// <summary>This class contains information for an Effect Semantic that is used as a Uniform or
/// PushConstant, and keeps the memory where this value will be intermediately kept (i.e. the "memory"
/// variable is updated by the application or automatic semantics, and then can be uploaded to the
/// shader. Will normally be used with the actual Semantic as a key in a map, hence it does
/// not contain it.</summary>
struct UniformSemantic
{
	StringHash variablename; //!< The name of the uniform variable. May not be supported.
	int32_t uniformLocation; //!< The location of the uniform
	TypedMem memory; //!< The memory that will be used for the uniform
};

/// <summary>A NodeSemanticSetter is a function that sets a specified Semantic to a value. It always
/// applies to a specific semantic, and sources all required information from the node parameter,
/// then writes the new updated value in the mem parameter</summary>
/// <param name="mem">Output: The new value of the semantic will be written here</param>
/// <param name="node">The node to get the semantic from</param>
typedef bool (*NodeSemanticSetter)(TypedMem& mem, const RendermanNode& node);

/// <summary>An Automatic Node Semantic is a semantic that has been defined in the Effect ("consumed" by
/// the effect) and that at the same time is defined in each Node of a Model (is "produced" in the node).
/// This class contains information about a Per-Node semantic that the effect uses as a pvrvk::Buffer Entry.
/// For example, if "MODELVIEWPROJECTION" (a "default" Model Semantic) was defined in the effect as a
/// variable inside a Uniform pvrvk::Buffer, an AutomaticNodeUniformSemantic could then be automatically generated
/// by a corresponding createAutomaticNodeSemantics() call, and that will in turn be used to auto-update
/// the buffer entry when calling updateAutomaticNodeSemantics().</summary>
struct AutomaticNodeBufferEntrySemantic
{
	const StringHash* semantic; //!< The semantic String (e.g. "MODELVIEWPROJECTION")
	utils::StructuredBufferView* structuredBufferView; //!< The buffer structure object it refers to.
	pvrvk::Buffer* buffer; //!< The buffer object
	uint16_t entryIndex; //!< The index of the entry in structuredBufferView
	NodeSemanticSetter semanticSetFunc; //!< A function pointer used to actually set the value (for example, &getModelViewProjection
	uint16_t setId; //!< The descriptor Set ID this buffer belongs to
	int16_t dynamicOffsetNodeId; //!< The Dynamic Offset of this node - calculated automatically, it is the index of the slice that this Node owns in pvrvk::Buffer.
	uint32_t bufferDynamicOffset[pvrvk::FrameworkCaps::MaxSwapChains]; //!< The list of dynamic buffer offsets
	uint32_t bufferDynamicSlice[pvrvk::FrameworkCaps::MaxSwapChains]; //!< The list of dynamic buffer slices
};

/// <summary>An Automatic Node Semantic is a semantic that has been defined in the Effect ("consumed" by
/// the effect) and that at the same time is defined in each Node of a Model (is "produced" in the node).
/// This class contains information about a Per-Node semantic that the effect uses as a Uniform.
/// For example, if "MODELVIEWPROJECTION" (a "default" Model Semantic) was defined in the effect as a
/// Uniform, an AutomaticNodeUniformSemantic could then be automatically generated by a corresponding
/// createAutomaticNodeSemantics() call, and that will in turn be used to auto-update the buffer entry
/// when calling updateAutomaticNodeSemantics().</summary>
struct AutomaticNodeUniformSemantic
{
	const StringHash* semantic; //!< The semantic String (e.g. "MODELVIEWPROJECTION")
	TypedMem* memory; //!< The memory storing the value to be transferred
	NodeSemanticSetter semanticSetFunc; //!< A function pointer used to actually set the value (for example, &getModelViewProjection
};

/// <summary>An Automatic Model Semantic is a semantic that has been defined in the Effect ("consumed" by
/// the effect) and that at the same time is defined somewhere in the Model ( is "produced" in the model).
/// This class contains information about a Per-Model semantic that the effect uses as a pvrvk::Buffer Entry(is a variable
/// inside a pvrvk::Buffer). For example, if "LIGHTPOSITION0" (a "default" Model Semantic) was defined in the effect as a
/// pvrvk::Buffer Entry, an AutomaticModelBufferEntrySemantic could then be automatically generated by a corresponding
/// createAutomaticModelSemantics() call, and that will in turn be used to auto-update the Uniform when calling
/// updateAutomaticModelsSemantics().</summary>
struct AutomaticModelBufferEntrySemantic
{
	const StringHash* semantic; //!< The semantic string (e.g. "LIGHTPOSITION0")
	RendermanModel* model; //!< The model this semantic is connected to
	utils::StructuredBufferView* structuredBufferView; //!< The buffer structure object it refers to.
	pvrvk::Buffer* buffer; //!< The buffer object
	uint16_t entryIndex; //!< The index of the entry in structuredBufferView
	ModelSemanticSetter semanticSetFunc; //!< A function pointer used to actually set the value (for example, &getLightPosition0)
};

/// <summary>An Automatic Model Semantic is a semantic that has been defined in the Effect ("consumed" by the effect)
///  and that at the same time is defined somewhere in the Model ( is "produced" in the model).
/// This class contains information about a Per-Model semantic that the effect uses as a Uniform.
/// For example, if "PROJECTIONMATRIX" was defined in the effect as a Uniform, and was accessible as a Model Semantic
/// in the model, an automatic semantic can be automatically generated (by a corresponding
/// createAutomaticModelSemantics() call) that will then be used to auto-update the uniforms
/// with updateAutomaticModelsSemantics().</summary>
struct AutomaticModelUniformSemantic
{
	const StringHash* semantic; //!< The semantic name
	RendermanModel* model; //!< The model from which the semantic is sourced (i.e. the model being rendered)
	TypedMem* memory; //!< A bit of memory used to transfer the value
	ModelSemanticSetter semanticSetFunc; //!< A function pointer that will be used to actually set the value (for example, &getViewProjectionMatrix0)
};

/// <summary>
/// Part of RendermanStructure. This class matches everything together:
/// A pipelineMaterial, with a RendermanMeshSubpass, to render.
/// Unique per rendering node AND mesh bone batch combination. NOTE: if bone batching is used, then multiple nodes will
/// be generated per meshnode.
/// Contains a reference to the mesh, the material, and also contains the dynamic offsets required to render with it.
/// Loop through those to render.</summary>
struct RendermanNode
{
	assets::NodeHandle assetNode; //!< Reference to the PVRAssets Node this object refers to
	uint32_t assetNodeId; //!< The NodeID of the PVRAssets node
	RendermanSubpassMesh* subpassMesh_; //!< The parent SubpassMesh combination
	RendermanMaterialSubpassPipeline* pipelineMaterial_; //!< The parent PipelineMaterial
	uint32_t batchId; //!< The Bone Batch ID - if not using Bone Batching, always 0
	std::vector<uint32_t> dynamicClientId[4]; //!< The indexes of each dynamic slice that this node owns in its buffers, respectively. Fixed index is pvrvk::DescriptorSet.
	std::vector<uint32_t> dynamicOffset[4][pvrvk::FrameworkCaps::MaxSwapChains]; //!< The (pre-calculated) byte offsets of the dynamic slices that this node owns in its buffers, respectively. Fixed index is pvrvk::DescriptorSet.
	std::vector<uint32_t> dynamicSliceId[4][pvrvk::FrameworkCaps::MaxSwapChains]; //!< The (pre-calculated) byte offsets of the dynamic slices that this node owns in its buffers, respectively. Fixed index is pvrvk::DescriptorSet.
	std::vector<RendermanBufferDefinition*> dynamicBuffer[4]; //!< The Dynamic Buffers that this node is being contained in. Fixed index is pvrvk::DescriptorSet.
	std::map<StringHash, UniformSemantic> uniformSemantics; //!< Uniform semantics used by this node.

	std::vector<AutomaticNodeBufferEntrySemantic> automaticEntrySemantics; //!< Automatic pvrvk::Buffer Entry semantics that were generated for this node. Used for auto-updating of
																		   //!< shader buffer variables. (Automatic variables can be generated when an effect and a model's
																		   //!< Semantics match, each such match can generate an automatic semantic.)
	std::vector<AutomaticNodeUniformSemantic> automaticUniformSemantics; //!<  Automatic Uniform semantics that were generated for this node. Used for auto-updating of shader
																		 //!<  uniform variables(Automatic variables can be generated when an effect and a model's Semantics match,
																		 //!<  each such match can generate an automatic semantic.)

	/// <summary>Retrieves a pointer to the list of dynamic offsets in use.</summary>
	/// <param name="setId">The descriptor set identifier to find dynamic offsets for</param>
	/// <param name="swapchainId">The swapchain identifier to find dynamic offsets for</param>
	/// <returns>A pointer to the list of dynamic offsets in use by the specified descriptor set and swapchain</returns>
	const uint32_t* getDynamicOffsetsPtr(uint32_t setId, uint32_t swapchainId) const { return dynamicOffset[setId][swapchainId].data(); }

	/// <summary>Retrieves the list of dynamic offsets in use.</summary>
	/// <param name="setId">The descriptor set identifier to find dynamic offsets for</param>
	/// <param name="swapchainId">The swapchain identifier to find dynamic offsets for</param>
	/// <param name="dynamicClientIdx">The dynamic client identifier to find dynamic offsets for</param>
	/// <returns>The list of dynamic offsets in use by the specified descriptor set, swapchain and dynamic client id</returns>
	uint32_t getDynamicOffset(uint32_t setId, uint32_t swapchainId, uint32_t dynamicClientIdx) const { return dynamicOffset[setId][swapchainId][dynamicClientIdx]; }

	/// <summary>Retrieves the list of dynamic offsets in use.</summary>
	/// <param name="setId">The descriptor set identifier to find dynamic offsets for</param>
	/// <param name="swapchainId">The swapchain identifier to find dynamic offsets for</param>
	/// <returns>The list of dynamic offsets in use by the specified descriptor set and swapchain</returns>
	const std::vector<uint32_t>& getDynamicOffsets(uint32_t setId, uint32_t swapchainId) { return dynamicOffset[setId][swapchainId]; }

	/// <summary>Get a reference of the semantic of this node. The value is saved in a provided memory object.</summary>
	/// <param name="semantic">The semantic name to get the value of</param>
	/// <param name="memory">The semantic value is returned here</param>
	/// <returns>Return true if found, otherwise false</returns>
	bool getNodeSemantic(const StringHash& semantic, TypedMem& memory) const;

	/// <summary>Get the function object (NodeSemanticSetter) that will be used for a specific semantic</summary>
	/// <param name="semantic">A node-specific semantic name (WORLDMATRIX, BONECOUNT, BONEMATRIXARRAY0 etc.)</param>
	/// <returns>A NodeSemanticSetter function object that can be called to set this node semantic.</returns>
	NodeSemanticSetter getNodeSemanticSetter(const StringHash& semantic) const;

	/// <summary>Update the value of a semantic of this node</summary>
	/// <param name="semantic">The semantic's name</param>
	/// <param name="value">The value to set the semantic to</param>
	/// <param name="swapid">The swapchain id to set the value for. You should always pass the current swapchain index
	/// - even if the semantic is not in multibuffered storage, the case will be handled correctly.</param>
	/// <returns>Return true if successful.</returns>
	bool updateNodeValueSemantic(const StringHash& semantic, const FreeValue& value, uint32_t swapid);

	/// <summary>Iterates any per-node semantics, and updates their values to their automatic per-node values. In order for
	/// this function to work, createAutomaticSemantics needs to have been called before to create the connections of
	/// the automatic semantics.</summary>
	/// <param name="swapidx">The current swapchain (framebuffer) image.</param>
	void updateAutomaticSemantics(uint32_t swapidx);

	/// <summary>Generates a list of all the semantics that this node's pipeline requires, that are defined per-node (e.g.
	/// MV/MVP matrices etc). Then, searches the connected asset node (model, mesh, material etc.) for these
	/// semantics, and creates connections between them so that when updateAutomaticSemantics is called, the updated
	/// values are updated in the semantics so that they can be read (with setUniformPtr, or updating buffers etc.).</summary>
	void createAutomaticSemantics();

	/// <summary>Get the commands necessary to render this node (bind pipeline, descriptor sets, draw commands etc.)
	/// Assumes correctly begun render passes, subpasses etc. All commands generated can be enabled/disabled in order
	/// to allow custom rendering.</summary>
	/// <param name="cmdBuffer">A command buffer to record the commands into</param>
	/// <param name="swapIdx">The current swap chain (framebuffer image) index to record commands for.</param>
	/// <param name="recordBindPipeline">If set to false, do not generate any the bind pipeline command (use to
	/// optimize nodes rendered with the same pipelines)</param>
	/// <param name="recordBindDescriptorSets">If set to false, do not generate the bind descriptor sets commands
	/// (use to optimize nodes rendered with the same sets)</param>
	/// <param name="recordBindVboIbo">If set to false, skip the generation of the bind vertex / index buffer
	/// commands (use to optimize nodes rendering the same mesh)</param>
	/// <param name="recordDrawCalls">If set to false, skip the generation of the draw calls.</param>
	void recordRenderingCommands(pvrvk::CommandBufferBase cmdBuffer, uint16_t swapIdx, bool recordBindPipeline = true, bool* recordBindDescriptorSets = nullptr,
		bool recordBindVboIbo = true, bool recordDrawCalls = true);

	/// <summary>Navigate(in the Rendering structure) to the RendermanPipeline object that is used by this node</summary>
	/// <returns>A reference to the pipeline object that is used by this node</returns>
	RendermanPipeline& toRendermanPipeline() { return *pipelineMaterial_->pipeline_; }

	/// <summary>Navigate(in the Rendering structure) to the RendermanPipeline object that is used by this node</summary>
	/// <returns>A reference to the pipeline object that is used by this node</returns>
	const RendermanPipeline& toRendermanPipeline() const { return *pipelineMaterial_->pipeline_; }

	/// <summary>Navigate(in the Rendering structure) to the RendermanMesh object that is used by this node</summary>
	/// <returns>A reference to the mesh object that is used by this node</returns>
	RendermanMesh& toRendermanMesh() { return *subpassMesh_->rendermesh_; }

	/// <summary>Navigate(in the Rendering structure) to the RendermanMesh object that is used by this node</summary>
	/// <returns>A reference to the mesh object that is used by this node</returns>
	const RendermanMesh& toRendermanMesh() const { return *subpassMesh_->rendermesh_; }
};
struct RendermanSubpassGroup;

/// <summary>Part of RendermanStructure. This class stores RendermanNodes and RendermanMaterialEffects The list of
/// nodes here references the list of materials. It references the Models in the original RendermanModelStore list.</summary>
struct RendermanSubpassGroupModel
{
	RendermanSubpassGroup* renderSubpassGroup_; //!< The SubpassGroup parent of this object
	RendermanModel* renderModel_; //!< The Model that is parent to this object
	std::deque<RendermanSubpassMesh> subpassMeshes; //!< Child objects that are combinations of Subpasses with Meshes
	std::deque<RendermanSubpassMaterial> materialEffects; //!< Child objects that are combinations of Materials with Effects.
	std::deque<RendermanNode> nodes; //!< The nodes that are children of this effect

	/// <summary>get number of Renderman node</summary>
	/// <returns>uint32_t</returns>
	uint32_t getNumRendermanNodes() const { return static_cast<uint32_t>(nodes.size()); }

	/// <summary>Get renderman node</summary>(const)
	/// <param name="index">Node id</param>
	/// <returns>const RendermanNode&</returns>
	const RendermanNode& toRendermanNode(uint32_t index) const
	{
		debug_assertion(index < getNumRendermanNodes(), "RendermanNode index out of bound");
		return nodes[index];
	}

	/// <summary>Get renderman node</summary>
	/// <param name="index">Node id</param>
	/// <returns>RendermanNode&</returns>
	RendermanNode& toRendermanNode(uint32_t index)
	{
		debug_assertion(index < getNumRendermanNodes(), "RendermanNode index out of bound");
		return nodes[index];
	}

	/// <summary>Get the commands necessary to render this SubpassModel. All calls are forwarded to the nodes of this
	/// model. Optimizes bind pipeline etc. calls between nodes. Assumes correctly begun render passes, subpasses etc.</summary>
	/// <param name="cmdBuffer">A command buffer to record the commands into</param>
	/// <param name="swapIdx">The current swap chain (framebuffer image) index to record commands for.</param>
	void recordRenderingCommands(pvrvk::CommandBufferBase cmdBuffer, uint16_t swapIdx);

	/// <summary>Navigate(in the Rendering structure) to the RendermanModel object that is used by this</summary>
	/// <returns>A reference to the Renderman Model object that this object belongs to</returns>
	RendermanModel& backToModel();

	/// <summary>Navigate to the root RenderManager object</summary>
	/// <returns>A reference to the RenderManager</returns>
	RenderManager& backToRenderManager();

	/// <summary>Navigate (in the Rendering structure) to the Renderman Subpass this object belongs to</summary>
	/// <returns>A reference to the Renderman Subpass this object belongs to</returns>
	RendermanSubpassGroup& backToRendermanSubpassGroup();

	/// <summary>Navigate (in the Rendering structure) to the Renderman subpass this object belongs to</summary>
	/// <returns>A reference to the Renderman subpass Pass this object belongs to</returns>
	RendermanSubpass& backToRendermanSubpass();

	/// <summary>Navigate (in the Rendering structure) to the Renderman Pass this object belongs to</summary>
	/// <returns>A reference to the Renderman Pass this object belongs to</returns>
	RendermanPass& backToRendermanPass();

	/// <summary>Navigate (in the Rendering structure) to the Renderman Effect this object belongs to</summary>
	/// <returns>A reference to the Renderman Effect this object belongs to</returns>
	RendermanEffect& backToRendermanEffect();

	/// <summary>Generates a list of semantics that the pipeline requires, but are changed per-node. Necessary to use updateAutomaticSemantics afterwards.</summary>
	void createAutomaticSemantics()
	{
		for (auto& node : nodes) { node.createAutomaticSemantics(); }
	}
};

/// <summary>Part of RendermanStructure. This class is a cooked EffectPipeline, exactly mirroring the PFX
/// pipelines. It is affected on creation time by the meshes that use it (for the Vertex Input configuration) but
/// after that it is used for rendering directly when traversing the scene.</summary>
struct RendermanPipeline
{
	struct RendermanSubpassGroup* subpassGroup_; //!< The Subpass Group this pipeline belongs to
	std::vector<RendermanSubpassMaterial*> subpassMaterials; //!< Pointers to the Subpass Materials that this pipeline makes use of
	pvrvk::GraphicsPipeline apiPipeline; //!< The Vulkan Pipeline object
	effectvk::PipelineDef* pipelineInfo; //!< Additional info on the pipeline

	Multi<pvrvk::DescriptorSet> fixedDescSet[4]; //!< Storage for the Fixed descriptor sets. A set is fixed if it contains no members with semantics.
	bool descSetIsFixed[4]; //!< If it is "fixed", it means that it is set by the PFX and no members of it are exported through semantics
	bool descSetIsMultibuffered[4]; //!< If it is "multibuffered", it means that it points to different buffers based on the swapchain index
	bool descSetExists[4]; //!< If it does not "exist", do not do anything for it...

	StringHash name; //!< The name of the pipeline object
	std::map<StringHash, RendermanBufferBinding> bufferBindings; //!< The bindings of the buffers (references to the buffer objects)
	std::map<StringHash, utils::StructuredBufferView*> bufferSemantics; //!< The corresponding buffer objects.
	std::map<StringHash, BufferEntrySemantic> bufferEntrySemantics; //!< Connection of buffer entries to semantics
	std::map<StringHash, UniformSemantic> uniformSemantics; //!< Connection of uniforms to semantics

	/// <summary>Automatic Model Entry semantics generated for this pipeline (Node scope semantics can be found in nodes).</summary>
	std::vector<AutomaticModelBufferEntrySemantic> automaticModelBufferEntrySemantics;
	/// <summary>Automatic Model /Uniform semantics generated for this pipeline (Node scope semantics can be found in nodes).</summary>
	std::vector<AutomaticModelUniformSemantic> automaticModelUniformSemantics;

	/// <summary>Navigate (in the Rendering structure) to the Renderman Subpass this object belongs to</summary>
	/// <returns>A reference to the Renderman Subpass this object belongs to</returns>
	RendermanSubpassGroup& backToSubpassGroup();

	/// <summary>Navigate (in the Rendering structure) to the Renderman Pass this object belongs to</summary>
	/// <returns>A reference to the Renderman Pass this object belongs to</returns>
	RendermanSubpass& backToSubpass();

	/// <summary>Navigate (in the Rendering structure) to the Renderman Effect this object belongs to</summary>
	/// <returns>A reference to the Renderman Effect this object belongs to</returns>
	RendermanEffect& backToRendermanEffect();

	/// <summary>Generate Uniform updating commands (setUniformPtr) for all the uniform semantics of this pipeline,
	/// including Effect and Model semantics, but excluding node semantics. The commands will read the values from the
	/// built-in Semantics objects.</summary>
	/// <param name="cmdBuffer">A command buffer to record the commands to</param>
	void recordUpdateAllUniformSemantics(pvrvk::CommandBufferBase cmdBuffer);

	/// <summary>Generate Uniform updating commands (setUniformPtr) for all Effect scope uniform semantics The commands
	/// will read the values from the built-in Semantics objects.</summary>
	/// <param name="cmdBuffer">A command buffer to record the commands to</param>
	void recordUpdateAllUniformEffectSemantics(pvrvk::CommandBufferBase cmdBuffer);

	/// <summary>Generate Uniform updating commands (setUniformPtr) for all Model scope uniform semantics The commands will
	/// read the values from the built-in Semantics objects.</summary>
	/// <param name="cmdBuffer">A command buffer to record the commands to</param>
	void recordUpdateAllUniformModelSemantics(pvrvk::CommandBufferBase cmdBuffer);

	/// <summary>Generate Uniform updating commands (setUniformPtr) for a specific Node The commands will read the values
	/// from the built-in Semantics objects.</summary>
	/// <param name="cmdBuffer">A command buffer to record the commands to</param>
	/// <param name="node">The node to record commands for</param>
	void recordUpdateAllUniformNodeSemantics(pvrvk::CommandBufferBase cmdBuffer, RendermanNode& node);

	/// <summary>Generate Uniform update commands (setUniformPtr) for a specific Model semantic The commands will read the
	/// values from the built-in Semantics objects.</summary>
	/// <param name="cmdBuffer">A command buffer to record the commands to</param>
	/// <param name="semantic">The Model semantic to update</param>
	/// <returns>true on success</returns>
	bool recordUpdateUniformCommandsModelSemantic(pvrvk::CommandBufferBase cmdBuffer, const StringHash& semantic);

	/// <summary>Generate Uniform update commands (setUniformPtr) for a specific Effect semantic The commands will read the
	/// values from the built-in Semantics objects.</summary>
	/// <param name="cmdBuffer">A command buffer to record the commands to</param>
	/// <param name="semantic">The effect semantic to update</param>
	/// <returns>Return true on success</returns>
	bool recordUpdateUniformCommandsEffectSemantic(pvrvk::CommandBufferBase cmdBuffer, const StringHash& semantic);

	/// <summary>Generate Uniform update commands (setUniformPtr) for a specific Node semantic</summary>
	/// <param name="cmdBuffer">A command buffer to record the commands to</param>
	/// <param name="semantic">The effect semantic to update</param>
	/// <param name="node">The node whose semantics to update</param>
	/// <returns>Return true on success</returns>
	bool recordUpdateUniformCommandsNodeSemantic(pvrvk::CommandBufferBase cmdBuffer, const StringHash& semantic, RendermanNode& node);

	/// <summary>Update the value of a Model Uniform semantic (the updated value will be read when the corresponding
	/// recorded update command is executed).</summary>
	/// <param name="semantic">The Model semantic to update</param>
	/// <param name="value">The new value, contained in a TypedMem object.</param>
	/// <returns>true on success, false if the semantic is not found</returns>
	bool updateUniformModelSemantic(const StringHash& semantic, const TypedMem& value);

	/// <summary>Update the value of an Effect Uniform semantic (the updated value will be read when the corresponding
	/// recorded update command is executed).</summary>
	/// <param name="semantic">The Effect semantic to update</param>
	/// <param name="value">The new value, contained in a TypedMem object.</param>
	/// <returns>true on success, false if the semantic is not found</returns>
	bool updateUniformEffectSemantic(const StringHash& semantic, const TypedMem& value);

	/// <summary>Update the value of a per-Node Uniform semantic (the updated value will be read when the corresponding
	/// recorded update command is executed).</summary>
	/// <param name="semantic">The Node semantic to update</param>
	/// <param name="value">The new value, contained in a TypedMem object.</param>
	/// <param name="node">The Node for which to update the uniform</param>
	/// <returns>true on success, false if the semantic is not found</returns>
	bool updateUniformNodeSemantic(const StringHash& semantic, const TypedMem& value, RendermanNode& node);

	/// <summary>Update the value of a per-Model pvrvk::Buffer Entry semantic. The value is updated immediately in the
	/// corresponding buffer.</summary>
	/// <param name="semantic">The Model semantic to update</param>
	/// <param name="value">The new value to set</param>
	/// <param name="dynamicSlice">(Optional) In the case of a Dynamic buffer, the "dynamic slice" or "dynamic
	/// client id" is the index in the "slice" of the buffer. Default 0.</param>
	/// <returns>Return true on success, false if the semantic is not found.</returns>
	bool updateBufferEntryModelSemantic(const StringHash& semantic, const FreeValue& value, uint32_t dynamicSlice = 0);

	/// <summary>Update the value of multiple per-Effect pvrvk::Buffer Entry semantics. The values are updated immediately
	/// in the corresponding buffer.</summary>
	/// <param name="semantics">The Effect semantic to update</param>
	/// <param name="values">The new values to set</param>
	/// <param name="numSemantics">The number of semantics to set.</param>
	/// <param name="dynamicSlice">(Optional) In the case of a Dynamic buffer, the "dynamic slice", or "dynamic client id"
	/// is the index of the "slice" of the buffer.Default 0.</param>
	/// <returns>Return true on success, false if the semantic is not found.</returns>
	bool updateBufferEntryModelSemantics(const StringHash* semantics, const FreeValue* values, uint32_t numSemantics, uint32_t dynamicSlice);

	/// <summary>Update buffer entry effect semantic</summary>
	/// <param name="semantic">Effect semantic to update</param>
	/// <param name="value">New value</param>
	/// <param name="swapid">swapchain id</param>
	/// <param name="dynamicClientId"></param>
	/// <returns> Return true on success</returns>
	bool updateBufferEntryEffectSemantic(const StringHash& semantic, const FreeValue& value, uint32_t swapid, uint32_t dynamicClientId = 0);

	/// <summary>Update the value of a per-Effect or Per-Model pvrvk::Buffer Entry semantic. The value is updated immediately in
	/// the corresponding buffer.</summary>
	/// <param name="semantic">The Effect or Model semantic to update</param>
	/// <param name="value">The new value to set</param>
	/// <param name="swapid">The current swapchain index</param>
	/// <param name="dynamicClientId">(Optional) In the case of a Dynamic buffer, the "dynamic client id" is the index of the
	/// "slice" of the buffer. Built-in functionality does NOT use this parameter for either Effect or Model
	/// semantics. Default 0.</param>
	/// <returns>Return true on success, false if the semantic is not found.</returns>
	bool updateBufferEntrySemantic(const StringHash& semantic, const FreeValue& value, uint32_t swapid, uint32_t dynamicClientId = 0);

	/// <summary>Update the value of a per-Node pvrvk::Buffer Entry semantic. The value is updated immediately in the
	/// corresponding buffer. The dynamic client id of the buffer (i.e. the Offset into the dynamic buffer) is
	/// automatically retrieved from the Node.</summary>
	/// <param name="semantic">The Node semantic to update</param>
	/// <param name="value">The new value to set</param>
	/// <param name="swapid">The current swapchain index</param>
	/// <param name="node">The RendermanNode for which to set the value.</param>
	/// <returns>Return true on success, false if the semantic is not found.</returns>
	bool updateBufferEntryNodeSemantic(const StringHash& semantic, const FreeValue& value, uint32_t swapid, RendermanNode& node);

	/// <summary>Update the values of a per-Node pvrvk::Buffer Entry semantics. The values is updated immediately in the
	/// corresponding buffer. The dynamic client id of the buffer (i.e. the Offset into the dynamic buffer) is
	/// automatically retrieved from the Node.</summary>
	/// <param name="semantics">The Node semantics to update. C-style array</param>
	/// <param name="values">The new values to set. C-style array</param>
	/// <param name="numSemantics">The number of semantics in <paramref name="values"/> and
	/// <paramref name="semantics"/></param>
	/// <param name="swapid">The current swapchain index</param>
	/// <param name="node">The RendermanNode for which to set the value.</param>
	/// <returns>Return true on success, false if the semantic is not found.</returns>
	bool updateBufferEntryNodeSemantics(const StringHash* semantics, const FreeValue* values, uint32_t numSemantics, uint32_t swapid, RendermanNode& node)
	{
		bool result = true;
		for (uint32_t i = 0; i < numSemantics; ++i) { result = result && updateBufferEntryNodeSemantic(semantics[i], values[i], swapid, node); }
		return result;
	}

	/// <summary>Update the value of multiple per-Effect pvrvk::Buffer Entry semantic. The values is updated immediately in the
	/// corresponding buffer. The dynamic client id of the buffer (i.e. the Offset into the dynamic buffer) is
	/// automatically retrieved from the Node.</summary>
	/// <param name="semantics">A c-style array of Semantic names. Must point to at least
	/// <paramref name="numSemantics"/>elements</param>
	/// <param name="values">A c-style array of the Values to set. Must point to at least
	/// <paramref name="numSemantics"/>elements</param>
	/// <param name="numSemantics">The number of semantics to set</param>
	/// <param name="swapid">The current swapchain index</param>
	/// <param name="dynamicClientId">(Optional) In the case of a Dynamic buffer, the "dynamic client id" is the index of the
	/// "slice" of the buffer. Built-in functionality does NOT use this parameter for either Effect or Model
	/// semantics. Default 0.</param>
	/// <returns>Return true on success, false if the semantic is not found.</returns>
	bool updateBufferEntryEffectSemantics(const StringHash* semantics, const FreeValue* values, uint32_t numSemantics, uint32_t swapid, uint32_t dynamicClientId = 0);

	/// <summary>Generates a list of all the semantics that this pipeline requires, that are defined per-Model (e.g. V/VP
	/// matrices, light positions etc). Then, searches the connected asset Model for these semantics, and creates
	/// connections between them so that when updateAutomaticSemantics is called, the new values are updated in the
	/// semantics so that they can be read (with setUniformPtr, or updating buffers etc.).</summary>
	/// <param name="useMainModelId">The model ID to use to read the values from. Default O (the first model).</param>
	/// <returns>true if successful, false on any error</returns>
	bool createAutomaticModelSemantics(uint32_t useMainModelId = 0);

	/// <summary>Update the value of all automatic per-Model semantics. The values are updated immediately in the
	/// corresponding buffer, and where the Uniform values are located, but Uniform values will only be visible in
	/// rendering when the recorded update uniform commands are executed (i.e. the commands generated by
	/// recordUpdateUniformCommandsXXXXX)</summary>
	/// <param name="swapIdx">The swapchain index to generate commands for (ignored for Uniforms)</param>
	/// <returns>Return true on success, false on any error</returns>
	bool updateAutomaticModelSemantics(uint32_t swapIdx);
};

/// <summary>Part of RendermanStructure. This class groups the Renderman pipeline and Models of a single RendermanSubpass.
/// This grouping is to be able to separate and order different parts of rendering and different objects that might potentially
/// have the same conditions, inside the same subpass (for example, a user might want to draw different objects with a
/// different effect although the objects have the same conditions.</summary>
struct RendermanSubpassGroup
{
	StringHash name; //!< The name (identifier) of this SubpassGroup.
	RendermanSubpass* subpass_; //!< The parent subpass of this group
	std::deque<RendermanPipeline> pipelines; //!< The pipelines contained in this group
	std::deque<RendermanSubpassGroupModel> subpassGroupModels; //!< Connections between this SubpassGroup and Models that have nodes that will be rendered in this group
	std::deque<RendermanModel*> allModels; //!< A pointer to the store of all the models, to facilitate retrieval.

	/// <summary>Navigate to the parent subpass of this object</summary>
	/// <returns>The parent subpass of this object</returns>
	const RendermanSubpass& backToSubpass() const;

	/// <summary>Get number of subpass group models</summary>
	/// <returns>uint32_t</returns>
	uint32_t getNumSubpassGroupModels() const { return static_cast<uint32_t>(subpassGroupModels.size()); }

	/// <summary>Get subpass group model</summary>(const)
	/// <param name="model">Model id</param>
	/// <returns>const RendermanSubpassGroupModel&</returns>
	const RendermanSubpassGroupModel& toSubpassGroupModel(uint32_t model) const
	{
		debug_assertion(model < subpassGroupModels.size(), "Model index out of bound");
		return subpassGroupModels[model];
	}

	/// <summary>Get subpass group model</summary>
	/// <param name="model">Model id</param>
	/// <returns>const RendermanSubpassGroupModel&</returns>
	RendermanSubpassGroupModel& toSubpassGroupModel(uint32_t model)
	{
		debug_assertion(model < subpassGroupModels.size(), "Model index out of bound");
		return subpassGroupModels[model];
	}

	/// <summary>Get renderman pipeline</summary>
	/// <param name="pipeline">Pipeline id</param>
	/// <returns>RendermanPipeline&</returns>
	RendermanPipeline& toRendermanPipeline(uint32_t pipeline)
	{
		debug_assertion(pipeline < pipelines.size(), "Pipeline index out of bound");
		return pipelines[pipeline];
	}

	/// <summary>Get renderman pipeline (const)</summary>
	/// <param name="pipeline">Pipeline id</param>
	/// <returns>const RendermanPipeline&</returns>
	const RendermanPipeline& toRendermanPipeline(uint32_t pipeline) const
	{
		debug_assertion(pipeline < pipelines.size(), "Pipeline index out of bound");
		return pipelines[pipeline];
	}

	/// <summary>Get Renderman subpass which owns this object</summary>
	/// <returns>RendermanSubpass&</returns>
	RendermanSubpass& backToSubpass();

	/// <summary>Record rendering commands for this subpass</summary>
	/// <param name="cmdBuffer">Recording Commandbuffer</param>
	/// <param name="swapIdx">pvrvk::Swapchain index</param>
	void recordRenderingCommands(pvrvk::CommandBufferBase cmdBuffer, uint16_t swapIdx);

	/// <summary>Generates a list of semantics that the pipeline requires, but are changed per-node.
	/// Necessary to use updateAutomaticSemantics afterwards.</summary>
	void createAutomaticSemantics()
	{
		for (RendermanSubpassGroupModel& subpassModel : subpassGroupModels)
		{
			for (RendermanNode& node : subpassModel.nodes) { node.createAutomaticSemantics(); }
		}
		for (RendermanPipeline& pipe : pipelines) { pipe.createAutomaticModelSemantics(); }
	}

	/// <summary>Updates all the automatic semantics for this model. Call every frame, after
	/// setting the new frame to the Models. If you have generated automatic semantics with a
	/// previous createAutomaticSemantics call, it will go through every single Automatic
	/// Semantic generated and update the value, which means that for each Semantic declared
	/// in any loaded effect/pass/subpass/...etc., it will go through the objects that have
	/// a matching semantic, retrieve its new value, and update it in the necessary uniform/
	/// memory object/ etc, without any user intervention.</summary>
	/// <param name="swapidx">The swap index of the current frame (used to know which index
	/// to update for multibuffered objects</param>
	void updateAutomaticSemantics(uint32_t swapidx)
	{
		for (RendermanSubpassGroupModel& subpassModel : subpassGroupModels)
		{
			for (RendermanPipeline& pipe : pipelines) { pipe.updateAutomaticModelSemantics(swapidx); }
			for (RendermanNode& node : subpassModel.nodes) { node.updateAutomaticSemantics(swapidx); }
		}
	}
};

/// <summary>Part of RendermanStructure. This struct groups the RenderPass subpass group.</summary>
struct RendermanSubpass
{
	RendermanPass* renderingPass_; //!< The parent Render Pass of this object
	std::deque<RendermanSubpassGroup> groups; //!< The children Subpass Groups this subpass has
	/// <summary>Return the RendermanPass to which this object belongs (const).</summary>
	/// <returns>The RendermanPass to which this object belongs (const).</returns>
	const RendermanPass& backToRendermanPass() const { return *renderingPass_; }

	/// <summary>Return the RendermanPass to which this object belongs.</summary>
	/// <returns>The RendermanPass to which this object belongs (const).</returns>
	RendermanPass& backToRendermanPass() { return *renderingPass_; }

	/// <summary>Return the RendermanEffect to which this object belongs. 2 lvls:Pass->Effect.</summary>
	/// <returns>The RendermanEffect to which this object belongs (const).</returns>
	const RendermanEffect& backToRendermanEffect() const;

	/// <summary>Return the RendermanEffect to which this object belongs. 2 lvls:Pass->Effect.</summary>
	/// <returns>The RendermanEffect to which this object belongs.</returns>
	RendermanEffect& backToRendermanEffect();

	/// <summary>Return the RenderManager to which this object belongs</summary>
	/// <returns>The RendermanEffect to which this object belongs</returns>
	const RenderManager& backToRenderManager() const;

	/// <summary>Get subpass group(const)</summary>
	/// <param name="index">Subpass group index</param>
	/// <returns>The subpass group with the specified index<returns>
	const RendermanSubpassGroup& toSubpassGroup(uint32_t index) const
	{
		debug_assertion(index < groups.size(), "Subpass group index out of bound");
		return groups[index];
	}

	/// <summary>Get SubpassGroup</summary>
	/// <param name="index">Subpass groups index</param>
	/// <returns>The subpass group with the specified index<returns>
	RendermanSubpassGroup& toSubpassGroup(uint32_t index)
	{
		debug_assertion(index < groups.size(), "Subpass group index out of bound");
		return groups[index];
	}

	/// <summary>Return the RendermanManager to which this object belongs. 3 lvls:Pass->Effect->RenderManager.</summary>
	/// <returns>The RendermanManager to which this object belongs.</returns>
	RenderManager& backToRenderManager();

	/// <summary>Get the commands necessary to render this entire Subpass (for each node, bind pipeline, descriptor
	/// sets, draw commands etc.) Calls are forwarded to the respective rendering nodes. Assumes correctly begun
	/// render passes, and (if necessary) any nextSubpass calls already recorded.</summary>
	/// <param name="cmdBuffer">A command buffer to record the commands into.</param>
	/// <param name="swapIdx">The current swap chain (framebuffer image) index to record commands for.</param>
	/// <remarks>If you need to create a SecondaryCommandBuffer for this subpass, use this overload.</remarks>
	void recordRenderingCommands(pvrvk::CommandBufferBase cmdBuffer, uint16_t swapIdx)
	{
		for (auto& group : groups) { group.recordRenderingCommands(cmdBuffer, swapIdx); }
	}

	/// <summary>Get the commands necessary to render this entire Subpass (for each node, bind pipeline, descriptor
	/// sets, draw commands etc.) into a Primary command buffer (not secondary command buffer) Allows to configure if
	/// the nextSubpass commands will be recorded, and if the</summary>
	/// <param name="cmdBuffer">A command buffer to record the commands into.</param>
	/// <param name="swapIdx">The current swap chain (framebuffer image) index to record commands for.</param>
	/// <param name="beginWithNextSubpassCommand">Record a nextSubpassInline() command at the beginning of this function.</param>
	/// <remarks>This overload can only be used if you need to create a Primary command buffer. Use the other overload
	/// of this function to create secondary command buffers.</remarks>
	void recordRenderingCommands(pvrvk::CommandBuffer& cmdBuffer, uint16_t swapIdx, bool beginWithNextSubpassCommand)
	{
		if (beginWithNextSubpassCommand) { cmdBuffer->nextSubpass(pvrvk::SubpassContents::e_INLINE); }
		pvrvk::CommandBufferBase base(cmdBuffer);
		recordRenderingCommands(base, swapIdx);
	}

	/// <summary>Generates all semantic list connections for all subobjects of this subpass (pipelines, nodes) by
	/// recursively calling createAutomaticSemantics on nodes and pipelines of this subpass. This function must have
	/// been called to be able to call updateAutomaticSemantics afterwards.</summary>
	void createAutomaticSemantics()
	{
		for (auto& group : groups) { group.createAutomaticSemantics(); }
	}

	/// <summary>Iterates all the nodes semantics per-pipeline and per-model, per-node, and updates their values to their
	/// specific per-node values. createAutomaticSemantics must have been called before.</summary>
	/// <param name="swapidx">swapchain index</param>
	void updateAutomaticSemantics(uint32_t swapidx)
	{
		for (auto& group : groups) { group.updateAutomaticSemantics(swapidx); }
	}

	/// <summary>Return number groups in this subpass</summary>
	/// <returns>uint32_t</returns>
	uint32_t getNumSubpassGroups() const { return static_cast<uint32_t>(groups.size()); }
};

/// <summary>Part of RendermanStructure. This class contains the different subpasses, exactly mirroring the PFX pass.</summary>
struct RendermanPass
{
	pvrvk::Framebuffer framebuffer[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)]; //!< The Framebuffer object chain this pass renders to
	RendermanEffect* renderEffect_; //!< The parent Effect this pass belongs to
	std::deque<RendermanSubpass> subpasses; //!< The subpasses this pass contains

	/// <summary>Get renderman effect which own this object (const)</summary>
	/// <returns>const RendermanEffect&</returns>
	const RendermanEffect& backToEffect() const { return *renderEffect_; }

	/// <summary>Get renderman effect which own this object</summary>
	/// <returns>RendermanEffect&</returns>
	RendermanEffect& backToEffect() { return *renderEffect_; }

	/// <summary>Get the commands necessary to render this entire Pass (for each subpass, for each node, bind
	/// pipeline, descriptor sets, draw commands etc.) into a Primary command buffer (secondary command buffers cannot
	/// be used at this level) Allows to configure if the begin/end renderpass and/or updateUniform commands will be
	/// recorded</summary>
	/// <param name="cmdBuffer">A Primary command buffer to record the commands into.</param>
	/// <param name="swapIdx">The current swap chain (framebuffer image) index to record commands for.</param>
	/// <param name="beginEndRendermanPass">If set to true, record a beginRenderPass() at the beginning and
	/// endRenderPass() at the end of this function. If loadOp is "clear". If the loadop is "clear", the first Model's
	/// clear color will be used.</param>
	void recordRenderingCommands(pvrvk::CommandBuffer& cmdBuffer, uint16_t swapIdx, bool beginEndRendermanPass);

	/// <summary>Get the commands necessary to render this entire Pass (for each subpass, for each node, bind
	/// pipeline, descriptor sets, draw commands etc.) into a Primary command buffer (secondary command buffers cannot
	/// be used at this level) Will call begin/end renderpass commands at start and finish, and allows to explicitly
	/// specify the clear color.</summary>
	/// <param name="cmdBuffer">A Primary command buffer to record the commands into.</param>
	/// <param name="swapIdx">The current swap chain (framebuffer image) index to record commands for.</param>
	/// <param name="clearColor">The color to which to clear the framebuffer. Ignored if loadop is not clear.</param>
	void recordRenderingCommandsWithClearColor(pvrvk::CommandBuffer& cmdBuffer, uint16_t swapIdx, const pvrvk::ClearValue& clearColor = pvrvk::ClearValue())
	{
		const pvrvk::ClearValue clearValue(clearColor);
		recordRenderingCommands_(cmdBuffer, swapIdx, &clearValue, 1);
	}

	/// <summary>Generates all semantic list connections for all subobjects of this pass (subpasses->pipelines, nodes) by
	/// recursively calling createAutomaticSemantics on subpasses'pipelines of this subpass. This function must have
	/// been called to be able to call updateAutomaticSemantics afterwards.</summary>
	void createAutomaticSemantics()
	{
		for (RendermanSubpass& subpass : subpasses) { subpass.createAutomaticSemantics(); }
	}

	/// <summary>Iterates all the nodes semantics per-effect, per-pass, per-subpass, per-model, per-node, and updates their
	/// values to the ones retrieved by the Model (model,mesh,node,material) retrieved values.
	/// createAutomaticSemantics needs to have been called before.</summary>
	/// <param name="swapidx">The current swapchain index (used to select which buffer to update in case of
	/// multibuffering. Multi-buffers and non multi-buffers are handled automatically, so always pass the current
	/// value).</param>
	void updateAutomaticSemantics(uint32_t swapidx)
	{
		for (RendermanSubpass& subpass : subpasses) { subpass.updateAutomaticSemantics(swapidx); }
	}

	/// <summary>Get the framebuffer for this pass (const)</summary>
	/// <param name="swapIndex">pvrvk::Swapchain id</param>
	/// <returns>const pvrvk::Framebuffer&</returns>
	const pvrvk::Framebuffer& getFramebuffer(uint32_t swapIndex) const { return framebuffer[swapIndex]; }

	/// <summary>Get the framebuffer for this pass</summary>
	/// <param name="swapIndex">pvrvk::Swapchain id</param>
	/// <returns>pvrvk::Framebuffer&</returns>
	pvrvk::Framebuffer getFramebuffer(uint32_t swapIndex) { return framebuffer[swapIndex]; }

	/// <summary>Navigate to a subpass object of this pass (const)</summary>
	/// <param name="subpass">The Subpass index (its order of appearance in the pass)</param>
	/// <returns>The subpass index. There is always at least one subpass per pass.</returns>
	/// <remarks>It is undefined behaviour to pass an index that does not exist.</remarks>
	const RendermanSubpass& toRendermanSubpass(uint32_t subpass) const
	{
		assertion(subpass < subpasses.size(), "Subpass index out of bound");
		return subpasses[subpass];
	}

	/// <summary>Navigate to a subpass object of this pass (const)</summary>
	/// <param name="subpass">The Subpass index (its order of appearance in the pass). It is undefined behaviour to
	/// pass an index that does not exist.</param>
	/// <returns>The Subpass.</returns>
	/// <remarks>There is always at least one subpass per pass, even if no subpasses have been defined in the PFX.It is
	/// undefined behaviour to pass an index that does not exist.</remarks>
	RendermanSubpass& toRendermanSubpass(uint32_t subpass)
	{
		assertion(subpass < subpasses.size(), "Subpass index out of bound");
		return subpasses[subpass];
	}

	/// <summaryGet number of subpasses</summary>
	/// <returns>uint32_t</returns>
	uint32_t getNumSubpass() const { return static_cast<uint32_t>(subpasses.size()); }

private:
	void recordRenderingCommands_(pvrvk::CommandBuffer& cmdBuffer, uint16_t swapIdx, const pvrvk::ClearValue* clearValues, uint32_t numClearValues);
};

/// <summary>Part of RendermanStructure. This class contains the different passes, exactly mirroring the PFX
/// effect. Contains the original EffectApi.</summary>
struct RendermanEffect
{
	RenderManager* manager_; //!< The manager this effect belongs to
	std::deque<RendermanPass> passes; //!< The passes this effect contains (Rendering tree structure entry point>
	std::deque<RendermanBufferDefinition> bufferDefinitions; //!< All buffers referenced in this effect

	std::map<StringHash, utils::StructuredBufferView*> structuredBufferViewSemantics; //!< All buffers that are (also?) referred to, as a whole, with Semantics
	std::map<StringHash, pvrvk::Buffer*> bufferSemantics; //!< All buffers that are (also?) referred to, as a whole, with Semantics
	std::map<StringHash, BufferEntrySemantic> bufferEntrySemantics; //!< All semantics of "entries" in a buffer.
	std::map<StringHash, UniformSemantic> uniformSemantics; //!< All semantics of uniforms
	bool isUpdating[4]; //!< Flag that a specified Swap Index has began updating.
	effectvk::EffectApi effect; //!< The EffectApi object used

	/// <summary>Constructor</summary>
	RendermanEffect() { memset(isUpdating, 0, sizeof(isUpdating)); }

	/// <summary>Return the RenderManager which owns this object (const)</summary>
	/// <returns>The root RenderManager object</returns>
	const RenderManager& backToRenderManager() const;

	/// <summary>Return the RenderManager which owns this object (const)</summary>
	/// <returns>The root RenderManager object</returns>
	RenderManager& backToRenderManager();

	/// <summary>Signify that there will be a batch of (buffer) updates, so that any possible buffers that are updated are
	/// only mapped and unmapped once. If you do NOT call this method before updating, the buffers will be mapped and
	/// unmapped with every single operation.</summary>
	/// <param name="swapChainIndex">The swapchain index to be updated.</param>
	/// <remarks>Unless really doing a one-off operation, always call this function before calling any of the
	/// updateAutomaticSemantics and similar operations</remarks>
	void beginBufferUpdates(uint32_t swapChainIndex) { isUpdating[swapChainIndex] = true; }

	/// <summary>Signify that the updates for the specified swapchain index have finished. Any and all mapped buffers will
	/// now be unmapped (once). If you do not call this function after you have called beginBufferUpdates, the data in
	/// your buffers are undefined.</summary>
	/// <param name="swapChainIndex">The swapchain index to be updated.</param>
	void endBufferUpdates(uint32_t swapChainIndex);

	/// <summary>Get the commands necessary to render this entire Effect (for pass, subpass, node: bind pipeline,
	/// descriptor sets, draw commands etc.) into a Primary command buffer (secondary command buffers cannot be used
	/// at this level)</summary>
	/// <param name="cmdBuffer">A Primary command buffer to record the commands into.</param>
	/// <param name="swapIdx">The current swap chain (framebuffer image) index to record commands for.</param>
	/// <param name="beginEnderRenderPasses">If set to False, the beginRenderPass and endRenderPass commands will not
	/// be recorded</param>
	void recordRenderingCommands(pvrvk::CommandBuffer& cmdBuffer, uint16_t swapIdx, bool beginEnderRenderPasses);

	/// <summary>Generates all semantic list connections for all subobjects of this effect (passes->subpasses->pipelines,
	/// nodes) by recursively calling createAutomaticSemantics on all passes. This function must have been called to
	/// be able to call updateAutomaticSemantics afterwards.</summary>
	void createAutomaticSemantics()
	{
		for (RendermanPass& pass : passes) { pass.createAutomaticSemantics(); }
	}

	/// <summary>Iterates all the nodes semantics per-effect, per-pass, per-subpass, per-model, per-node, and updates their
	/// values to their updated per-node values. CreateAutomaticSemantics must have been called otherwise no semantics
	/// will have been generated.</summary>
	/// <param name="swapidx">The current swap chain (framebuffer image) index to record commands for.</param>
	void updateAutomaticSemantics(uint32_t swapidx)
	{
		bool wasUpdating = isUpdating[swapidx];
		if (!wasUpdating) // Optimization - avoid multiple map/unmap. But only if the user has not taken care of it.
		{ beginBufferUpdates(swapidx); }
		for (RendermanPass& pass : passes) { pass.updateAutomaticSemantics(swapidx); }
		if (!wasUpdating) // If it was not mapped, unmap it. Otherwise leave it alone...
		{ endBufferUpdates(swapidx); }
	}

	/// <summary>Navigate to a RendermanPass object of this effect by the pass ID</summary>
	/// <param name="toPass">The Pass index (its order of appearance in the pass)</param>
	/// <returns>The RendermanPass object with id <paramref name="toPass"/>.
	/// There is always at least one subpass per pass.</returns>
	/// <remarks>It is undefined behaviour to pass an index that does not exist.</remarks>
	RendermanPass& toRendermanPass(uint32_t toPass) { return passes[toPass]; }

	/// <summary>Navigate to a RendermanPass object of this effect by the pass ID</summary>
	/// <param name="toPass">The Pass index (its order of appearance in the pass)</param>
	/// <returns>The RendermanPass object with id <paramref name="toPass"/>.
	/// There is always at least one subpass per pass.</returns>
	/// <remarks>It is undefined behaviour to pass an index that does not exist.</remarks>
	const RendermanPass& toRendermanPass(uint32_t toPass) const { return passes[toPass]; }

	/// <summary>Update buffer entry effect semantic</summary>
	/// <param name="semantic">Effect semantic to update</param>
	/// <param name="value">New value</param>
	/// <param name="swapid">swapchain id</param>
	/// <param name="dynamicClientId"></param>
	/// <returns> Return true on success</returns>
	bool updateBufferEntryEffectSemantic(const StringHash& semantic, const FreeValue& value, uint32_t swapid, uint32_t dynamicClientId = 0)
	{
		auto it = bufferEntrySemantics.find(semantic);
		if (it == bufferEntrySemantics.end()) { return false; }

		auto& sem = it->second;

		sem.structuredBufferView->getElement(sem.entryIndex, dynamicClientId, swapid).setValue(value);
		if ((sem.buffer[0]->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{ sem.buffer[0]->getDeviceMemory()->flushRange(sem.structuredBufferView->getDynamicSliceOffset(swapid), sem.structuredBufferView->getDynamicSliceSize()); }
		return true;
	}
};

/// <summary>Internal class containing the Render Manager tree structure representation of a scene</summary>
struct RendermanStructure
{
	std::deque<RendermanEffect> effects; //!< The root of the tree: The list of effects contained.
};

// The RendermanStructure. This class contains all the different effects that have been added.
// RendermanEffect[]
//	map<StringHash, StructuredMemoryView> effectBuffers
//	EffectApi
//	RendermanPass[]
//		RendermanSubpass[]
//			RendermanModel*[]
//			RendermanPipeline[]
//				pvrvk::GraphicsPipeline
//				effectvk::PipelineDef*
//				RendermanSubpassModel[]
//					map<stringHash, PerBoneBatchBuffers*>
//					map<stringHash, PerModelBuffers*>
//					RendermanSubpassMaterial[]
//					RendermanModel*
//					RendermanNode[]
//						assets::Node*
//						RendermanMesh*
//						RendermanSubpassMaterial*
//						dynamicOffsets
//					RendermanMaterialEffect[]
//						RendermanSubpassModel*
//						RendermanMaterial*
//						DescriptorSets

// RendermanModel[]
//	RendermanMaterial[]
//		assetMaterial
//		textures[]
//	RendermanMesh[]
//		assetMesh
//		vbos[]
//		ibo[]
//		indexType

/// <summary>The RenderManager is a rendering automation class, with class responsibilities such as: - Putting
/// together PFX files (Effects) with POD models (Models) to render - Creating Graphics Pipelines, Descriptor Sets,
/// VBOs, IBOs, UBOs, etc. - Creating and configuring render - to - texture targets - Automatically generate
/// command buffers for rendering - Automatically update textures/uniforms/buffers in the rendering api with info
/// provided by the the model (textures, matrices etc)</summary>
/// <remarks>Basic use: Create a RenderManager object Add Effects to it (usually one) : addEffect(...) Add Models to
/// specific parts of the Effect (Normally, a model is added to be rendered by a specific subpass
/// (addModelToSubpass), but shortcut methods are provided to add it to entire renderpasses, or even all
/// renderpasses) Cook the RenderManager : buildRenderObjects(...) Get rendering commands in command buffers :
/// recordRenderingCommands(...) (For complete automation) : createAutomaticSemantics(...) For each frame:
/// updateAutomaticSemantics(...) submitCommandBuffer(..) Semantics are "slots" where pieces of information can be
/// put to renders. For example, a "DIFFUSETEXTURE" semantic may exist where a texture must be added in order to
/// function as the diffuse texture for a shader, or a "MVP" semantic may exist where a matrix must be uploaded for
/// the vertex transformation. Automatic semantics are "connections" where this information will be automatically
/// retrieved from the Model (the scene object). It is important to realize that semantics exist on different parts
/// of the object, and are expected to be updated with different rates. A) Effect - things like, for example, the
/// clear color, that may be common among all objects that use an effect. B) Model - similar to the effect, it is
/// common for an entire model. Might be the Projection matrix or an ambient color. C) Node - things that are
/// specific to an object. Commonly exist on dynamic buffers. Things like the MVP matrix or Textures. PFX Bonebatch
/// scope items also end up in nodes (as one node is generated per bonebatch). Some things to look out for: The
/// final "renderable" is the Node. Each nodes carries enough information (either directly or through pointers to
/// "outer" object to render itself. One Node is created for each bonebatch of a node of a model There are many
/// different intermediate objects to avoid duplications, and different parts of storage items around the objects.
/// The distinct phases that can be discerned are: - Setup (adding Effect(s) and Model(s)) - Object generation
/// (buildRenderObjects()) - Command generation (recordCommandBuffers()) - Memory updates (updateSemantics,
/// updateAutomaticSemantics)</remarks>
class RenderManager
{
	friend class RenderManagerNodeIterator;

public:
	typedef RendermanNode Renderable; //!< A typedef to make more explicit the fact that the Node is the entity that we render.

	/// <summary>A special iterator class that is used to iterate through ALL renderable objects (nodes) of the
	/// entire render manager. Unidirectional, sequential. Provides methods to know when the pass, subpass or pipeline
	/// changed with the last advance. The effect of iterating with this class is identical to iterating for each
	/// pass, each subpass, each subpassmodel, each node.</summary>
	struct RendermanNodeIterator
	{
		friend class RenderManager;

	private:
		RenderManager& mgr;
		RendermanNode* cached;
		uint32_t nodeId;
		uint32_t subpassModelId;
		uint32_t subpassId;
		uint16_t subpassGroupId;
		uint16_t passId;
		uint32_t effectId;
		bool passChanged_;
		bool subpassChanged_;
		bool pipelineChanged_;
		bool subpassGroupChanged_;

		RendermanNodeIterator(RenderManager& mgr, bool begin) : mgr(mgr), nodeId(0), subpassModelId(0), subpassId(0), subpassGroupId(0), passId(0)
		{
			effectId = begin ? 0 : static_cast<uint32_t>(mgr.renderObjects().effects.size());
			cached = begin ? &mgr.toSubpassGroupModel(0, 0, 0, 0, 0).nodes[0] : nullptr;
		}

		void advanceNode(RendermanEffect& eff, RendermanPass& pass, RendermanSubpass& spass, RendermanSubpassGroup& group, RendermanSubpassGroupModel& spmodel)
		{
			pvrvk::GraphicsPipeline old_pipeline = cached->pipelineMaterial_->pipeline_->apiPipeline;
			subpassChanged_ = false;
			passChanged_ = false;
			if (++nodeId == spmodel.nodes.size())
			{
				nodeId = 0;
				advanceModelEffect(eff, pass, spass, group);
			}
			else
			{
				cached = &spmodel.nodes[nodeId];
			}
			pipelineChanged_ = (old_pipeline == cached->pipelineMaterial_->pipeline_->apiPipeline);
		}

		void advanceModelEffect(RendermanEffect& eff, RendermanPass& pass, RendermanSubpass& spass, RendermanSubpassGroup& group)
		{
			if (++subpassModelId == group.getNumSubpassGroupModels())
			{
				subpassModelId = 0;
				advanceSubpassGroup(eff, pass, spass);
			}
			else
			{
				cached = &group.toSubpassGroupModel(subpassModelId).toRendermanNode(0);
			}
		}
		void advanceSubpass(RendermanEffect& eff, RendermanPass& pass)
		{
			subpassChanged_ = true;
			if (++subpassId == pass.subpasses.size())
			{
				subpassId = 0;
				advancePass(eff);
			}
			else
			{
				cached = &pass.toRendermanSubpass(subpassId).toSubpassGroup(subpassGroupId).toSubpassGroupModel(0).toRendermanNode(0);
			}
		}

		void advanceSubpassGroup(RendermanEffect& effect, RendermanPass& pass, RendermanSubpass& subpass)
		{
			subpassGroupChanged_ = true;
			if (++subpassGroupId == subpass.groups.size())
			{
				subpassGroupId = 0;
				advanceSubpass(effect, pass);
			}
			else
			{
				debug_assertion(subpass.toSubpassGroup(subpassGroupId).getNumSubpassGroupModels() != 0 &&
						subpass.toSubpassGroup(subpassGroupId).toSubpassGroupModel(0).getNumRendermanNodes() != 0,
					"Subpassgroup must have atleast one model and a model node");

				cached = &subpass.toSubpassGroup(subpassGroupId).toSubpassGroupModel(0).toRendermanNode(0);
			}
		}

		void advancePass(RendermanEffect& eff)
		{
			passChanged_ = true;
			if (++passId == eff.passes.size())
			{
				passId = 0;
				advanceEffect();
			}
			cached = &eff.toRendermanPass(passId).toRendermanSubpass(0).toSubpassGroup(0).toSubpassGroupModel(0).toRendermanNode(0);
		}
		void advanceEffect()
		{
			if (++effectId == mgr.renderObjects().effects.size()) { cached = nullptr; }
			else
			{
				cached = &mgr.renderObjects().effects[effectId].toRendermanPass(0).toRendermanSubpass(0).toSubpassGroup(0).toSubpassGroupModel(0).toRendermanNode(0);
			}
		}

	public:
		/// <summary>Return true if we moved to a new pass during the last call to operator ++</summary>
		/// <returns>True if the pass "just" changed (i.e. the last call to operator ++ changed the pass), otherwise false</returns>
		bool passChanged() const { return passChanged_; }

		/// <summary>Return true if we moved to a new subpass during the last call to operator ++</summary>
		/// <returns>True if the subpass "just" changed (i.e. the last call to operator ++ changed the subpass), otherwise false</returns>
		bool subpassChanged() const { return subpassChanged_; }

		/// <summary>Return true if the current node is being rendered with a different pipeline than the previous</summary>
		/// <returns>True if the pipeline "just" changed (i.e. the last call to operator ++ changed the pipeline), otherwise false</returns>
		bool pipelineChanged() const { return pipelineChanged_; }

		/// <summary>Equality operator. Returns true if points to the exact same node in the tree hierarchy. rhs must be
		/// an operator from the same Render Manager</summary>
		/// <param name="rhs">Right hand side. If the node comes from a different RenderManager, the behaviour is undefined</param>
		/// <returns>True if the iterators point to the same node, otherwise false</returns>
		bool operator==(RendermanNodeIterator& rhs) const
		{
			return (effectId == rhs.effectId) && (passId == rhs.passId) && (subpassId == rhs.subpassId) && (subpassModelId == rhs.subpassModelId) && (nodeId == rhs.nodeId);
		}

		/// <summary>Inequality operator. Returns true if points to the exact same node in the tree hierarchy. rhs must be
		/// an operator from the same Render Manager</summary>
		/// <param name="rhs">Right hand side. If the node comes from a different RenderManager, the behaviour is undefined</param>
		/// <returns>True if the iterators do not point to the same node, otherwise false</returns>
		bool operator!=(RendermanNodeIterator& rhs) const
		{
			return (effectId != rhs.effectId) || (passId != rhs.passId) || (subpassId != rhs.subpassId) || (subpassModelId != rhs.subpassModelId) || (nodeId != rhs.nodeId);
		}

		/// <summary>Dereferencing operator. Returning current node</summary>
		/// <returns>Current Node</summary>
		RendermanNode& operator*() { return *cached; }

		/// <summary>Dereferencing operator. Returning current node</summary>
		/// <returns>Current Node</summary>
		RendermanNode* operator->() { return cached; }

		/// <summary>Advance operator (postfix). Moves to the next node, but returns the old current iterator</summary>
		/// <returns>An iterator pointing to the current node at the time of the call</summary>
		RendermanNodeIterator operator++(int)
		{
			RendermanNodeIterator cpy(*this);
			++(*this);
			return cpy;
		}

		/// <summary>Advance operator (prefix). Moves to the next node</summary>
		/// <returns>This, now advanced by one position, iterator</summary>
		RendermanNodeIterator& operator++()
		{
			auto& effect = mgr.toEffect(effectId);
			auto& pass = effect.toRendermanPass(passId);
			auto& subpass = pass.toRendermanSubpass(subpassId);
			auto& subpassGroup = subpass.toSubpassGroup(subpassGroupId);
			auto& spmodel = subpassGroup.toSubpassGroupModel(subpassModelId);
			advanceNode(effect, pass, subpass, subpassGroup, spmodel);
			return *this;
		}
	};

	/// <summary>This class is a dummy container that provides begin() and end() methods to iterate through all nodes
	/// of the RenderManager. Extremely useful to use in C++11 range based for: for (auto& node :
	/// renderManager.renderables())</summary>
	struct RenderManagerNodeIteratorAdapter
	{
		/// <summary>Returns an iterator pointing to the first RendermanNode element</summary>
		/// <returns>RendermanNodeIterator</returns>
		RendermanNodeIterator begin() const { return RendermanNodeIterator(mgr, true); }

		/// <summary>Returns an iterator pointing to the first RendermanNode element</summary>
		/// <returns>RendermanNodeIterator</returns>
		RendermanNodeIterator end() const { return RendermanNodeIterator(mgr, false); }

	private:
		friend class RenderManager;
		RenderManager& mgr;
		RenderManagerNodeIteratorAdapter(RenderManager& mgr) : mgr(mgr) {}
	};

private:
	typedef std::deque<RendermanModel> RendermanModelStorage;
	// effect / pass / subpass
	pvrvk::DeviceWeakPtr _device;
	RendermanStructure _renderStructure;
	RendermanModelStorage _modelStorage; // STORAGE: Deque, so that we can insert elements without invalidating pointers.
	pvrvk::Swapchain _swapchain;
	pvrvk::DescriptorPool _descPool;
	std::map<assets::Mesh*, std::vector<AttributeLayout>*> meshAttributeLayout; // points to finalPipeAttributeLayouts
	IAssetProvider* _assetProvider;
	pvr::utils::vma::Allocator _vmaAllocator;

	/// <summary>Generate the RenderManager, create the structure, add all rendering effects, create the API objects, and
	/// in general, cook everything. Call AFTER any calls to addEffect(...) and addModel...(...). Call BEFORE any
	/// calls to createAutomaticSemantics(...), update semantics etc.
	/// Calling this function. This function record & submit texture uploading commands</summary>
	/// <returns>True if successful.</returns>
	void buildRenderObjects_(pvrvk::CommandBuffer& texUploadCmdBuffer);

public:
	/// <summary>Constructor. Creates an empty rendermanager. In order to use it, you need to addEffect() and addModel() to
	/// populate it, then buildRenderObjects(), then createAutomaticSemantics(), generate</summary>
	RenderManager() {}

	/// <summary>Get the Asset Provider object that was set when initializing this RenderManager</summary>
	/// <returns>The Asset Provider object that was set when initializing this RenderManager</returns>
	IAssetProvider& getAssetProvider() { return *_assetProvider; }

	/// <summary>Initialize this RenderManager, prepare it for configuration. Call this once, before calling any
	/// other functions of the RenderManager</summary>
	/// <param name="assetProvider">The Asset Provider object that will be used to load any referenced textures
	/// and shaders</param>
	/// <param name="swapchain">The swapchain object for on-screen rendering</param>
	/// <param name="pool">The descriptor pool from which all descriptor sets will be allocated</param>
	/// <returns>True if successful, false if any error occured during init</returns>
	bool init(IAssetProvider& assetProvider, const pvrvk::Swapchain& swapchain, const pvrvk::DescriptorPool& pool)
	{
		_assetProvider = &assetProvider;
		_swapchain = swapchain;
		_device = swapchain->getDevice();
		_descPool = pool;
		if (!pool)
		{
			assertion(false, "Rendermanager - Invalid pvrvk::DescriptorPool");
			return false;
		}

		pvrvk::Device device = getDevice().lock();
		_vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(device));

		return true;
	}

	/// <summary>Get the swapchain object with which this render manager was initialized</summary>
	/// <returns>The swapchain object with which this render manager was initialized</returns>
	const pvrvk::Swapchain& getSwapchain() const { return _swapchain; }

	/// <summary>This method provides a class that functions as a "virtual" node container. Its sole purpose is
	/// providing begin() and end() methods that iterate through all nodes of the RenderManager. Extremely useful to
	/// use in C++11 range based for: for (auto& node : renderManager.renderables())</summary>
	/// <returns> A lightweight object used to provide the begin() and end() functions for all the renderable objects.
	/// The begin/end iterator pair will iterate every single rendering node, each time advancing one node</returns>
	RenderManagerNodeIteratorAdapter renderables() { return RenderManagerNodeIteratorAdapter(*this); }

	/// <summary>Navigate the structure of the Rendermanager. Goes to the Effect object with index 'effect'</summary>
	/// <param name="effect">The index of the effect to navigate to. The index is the order with which it was added to
	/// the RenderManager.</param>
	/// <returns>The RendermanEffect object with index 'effect'.</returns>
	RendermanEffect& toEffect(uint32_t effect) { return _renderStructure.effects[effect]; }

	/// <summary>Navigate the structure of the Rendermanager. Goes to the Pass object with index 'pass' in effect with
	/// index 'effect'.</summary>
	/// <param name="effect">The index of the effect the object belongs to. The index is the order with which it was
	/// added to the RenderManager.</param>
	/// <param name="pass">The index of the pass within 'effect' that the object belongs to. The index is the order of
	/// the pass in the effect.</param>
	/// <returns>The RendermanPass object with index 'pass' in effect 'effect'.</returns>
	RendermanPass& toPass(uint32_t effect, uint32_t pass) { return toEffect(effect).toRendermanPass(pass); }

	/// <summary>Navigate the structure of the Rendermanager. Goes to the Subpass object #'subpass' in pass #'pass' in
	/// effect #'effect'.</summary>
	/// <param name="effect">The index of the effect the object belongs to. The index is the order with which it was
	/// added to the RenderManager.</param>
	/// <param name="pass">The index of the pass within 'effect' that the object belongs to. The index is the order of
	/// the pass in the effect.</param>
	/// <param name="subpass">The index of the subpass within 'pass' that the object belongs to. The index is the
	/// order of the subpass in the pass.</param>
	/// <returns>The RendermanSubpass object with index 'subpass' in pass 'pass' of effect 'effect'.</returns>
	RendermanSubpass& toSubpass(uint32_t effect, uint32_t pass, uint32_t subpass) { return toPass(effect, pass).toRendermanSubpass(subpass); }

	/// <summary>Navigate the structure of the Rendermanager. Goes to the SubpassGroup object  #'subpassGroup' in
	/// subpass #'subpass' in pass #'pass' in effect #'effect'.</summary>
	/// <param name="effect">The index of the effect the object belongs to. The index is the order with which it was
	/// added to the RenderManager.</param>
	/// <param name="pass">The index of the pass within 'effect' that the object belongs to. The index is the order of
	/// the pass in the effect.</param>
	/// <param name="subpass">The index of the subpass within 'pass' that the object belongs to. The index is the
	/// order of the subpass in the pass.</param>
	/// <param name="subpassGroup">The index of the Subpass Group within 'subpass' . The index is the
	/// order of the subpassGroup in the subpass.</param>
	/// <returns>The RendermanSubpassGroup object with index 'subpassGroup' in subpass 'subpass' in pass 'pass' of
	/// effect 'effect'.</returns>
	RendermanSubpassGroup& toSubpassGroup(uint32_t effect, uint32_t pass, uint32_t subpass, uint32_t subpassGroup)
	{
		return toSubpass(effect, pass, subpass).toSubpassGroup(subpassGroup);
	}

	/// <summary>Navigate the structure of the Rendermanager. Goes to the Pipeline object #'pipeline' in subpass object
	/// #'subpass' in pass #'pass' in effect #'effect'.</summary>
	/// <param name="effect">The index of the effect the object belongs to. The index is the order with which it was
	/// added to the RenderManager.</param>
	/// <param name="pass">The index of the pass within 'effect' that the object belongs to. The index is the order of
	/// the pass in the effect.</param>
	/// <param name="subpass">The index of the subpass within 'pass' that the object belongs to. The index is the
	/// order of the subpass in the pass.</param>
	/// <param name="pipeline">The index of the pipeline within 'subpass'. The index is the order of the pipeline in
	/// the pass.</param>
	/// <param name="subpassGroup">The index of the group within 'subpass' that the object belongs to. The index is the
	/// order of the subpassGroup in the subpass.</param>
	/// <returns>The RendermanPipeline object with index 'pipeline' in subpass 'subpass' in pass 'pass' of effect
	/// 'effect'.</returns>
	RendermanPipeline& toPipeline(uint32_t effect, uint32_t pass, uint32_t subpass, uint32_t subpassGroup, uint32_t pipeline)
	{
		return toSubpassGroup(effect, pass, subpass, subpassGroup).toRendermanPipeline(pipeline);
	}

	/// <summary>Navigate the structure of the Rendermanager. Goes to a SubpassModel object. A SubpassModel object is the
	/// data held for a Model when it is added to a specific Subpass.</summary>
	/// <param name="effect">The index of the effect the object belongs to. The index is the order with which it was
	/// added to the RenderManager.</param>
	/// <param name="pass">The index of the pass within 'effect' that the object belongs to. The index is the order of
	/// the pass in the effect.</param>
	/// <param name="subpass">The index of the subpass within 'pass' that the object belongs to. The index is the
	/// order of the subpass in the pass.</param>
	/// <param name="subpassGroup">The index of the subpassGroup within 'subpass' that the object belongs to. The index is the
	/// order of the subpassGroup in the subpass.</param>
	/// <param name="model">The index of the model within 'subpass'. The index is the order in which this model was
	/// added to Subpass.</param>
	/// <returns>The RendermanPipeline object with index 'pipeline' in subpass 'subpass' in pass 'pass' of effect
	/// 'effect'.</returns>
	RendermanSubpassGroupModel& toSubpassGroupModel(uint32_t effect, uint32_t pass, uint32_t subpass, uint32_t subpassGroup, uint32_t model)
	{
		return toSubpassGroup(effect, pass, subpass, subpassGroup).toSubpassGroupModel(model);
	}

	/// <summary>Navigate the structure of the Rendermanager. Goes to the Model object with index 'model'.</summary>
	/// <param name="model">The index of the model. The index is the order with which it was added to the
	/// RenderManager.</param>
	/// <returns>The RendermanModel object with index 'model' in the RenderManager'.</returns>
	RendermanModel& toModel(uint32_t model) { return _modelStorage[model]; }

	/// <summary>Navigate the structure of the Rendermanager. Goes to the Mesh object #'mesh' of the model #'model'.</summary>
	/// <param name="model">The index of the rendermodel this mesh belongs to. The index is the order with which it
	/// was added to the RenderManager.</param>
	/// <param name="mesh">The index of the rendermesh in the model. The index of the mesh is the same as it was in
	/// the pvr::assets::Model file.</param>
	/// <returns>The RendermanMesh object with index 'mesh' in the model #'model'.</returns>
	RendermanMesh& toRendermanMesh(uint32_t model, uint32_t mesh) { return _modelStorage[model].meshes[mesh]; }

	/// <summary>Navigate the structure of the Rendermanager. Access a Mesh through its effect (as opposed to through its
	/// model object. The purpose of this function is to find the object while navigating a subpass.</summary>
	/// <param name="effect">The index of the effect in the RenderManager.</param>
	/// <param name="pass">The index of the pass in the Effect.</param>
	/// <param name="subpass">The index of the subpass in the Pass.</param>
	/// <param name="subpassGroup">The index of the subpassGroup in the Subpass.</param>
	/// <param name="model">The index of the model in the Subpass. Caution- this is not the same as the index of the
	/// model in the RenderManager.</param>
	/// <param name="mesh">The index of the mesh in the model. This is the same as the index of the pvr::assets::Mesh
	/// in the pvr::assets::Model.</param>
	/// <returns>The selected RendermanMesh item.</returns>
	RendermanMesh& toRendermanMeshByEffect(uint32_t effect, uint32_t pass, uint32_t subpass, uint32_t subpassGroup, uint32_t model, uint32_t mesh)
	{
		return _renderStructure.effects[effect].passes[pass].subpasses[subpass].groups[subpassGroup].allModels[model]->meshes[mesh];
	}

	/// <summary>Get a reference to the entire render structure of the RenderManager. Raw.</summary>
	/// <returns>A reference to the data structure.</returns>
	RendermanStructure& renderObjects() { return _renderStructure; }

	/// <summary>Get a reference to the storage of Models in the RenderManager. Raw.</summary>
	/// <returns>A reference to the Models.</returns>
	RendermanModelStorage& renderModels() { return _modelStorage; }

	/// <summary>Get the context that this RenderManager uses.</summary>
	/// <returns>The context that this RenderManager uses.</returns>
	pvrvk::DeviceWeakPtr& getDevice() { return _device; }

	/// <summary>Get the context that this RenderManager uses.</summary>
	/// <returns>The context that this RenderManager uses.</returns>
	const pvrvk::DeviceWeakPtr& getDevice() const { return _device; }

	/// <summary>Get the allocator that this RenderManager uses.</summary>
	/// <returns>The allocator that this RenderManager uses.</returns>
	utils::vma::Allocator& getAllocator() { return _vmaAllocator; }

	/// <summary>Get the allocator that this RenderManager uses.</summary>
	/// <returns>The allocator that this RenderManager uses.</returns>
	const utils::vma::Allocator& getAllocator() const { return _vmaAllocator; }

	/// <summary>Add a model for rendering. This method is a shortcut for adding a model to ALL renderpasses, ALL
	/// subpasses.</summary>
	/// <param name="model">A pvr::assets::Model handle to add for rendering. Default 0.</param>
	/// <param name="effect">The Effect to whose subpasses the model will be added.</param>
	/// <returns>The order of the model within the RenderManager (the index to use for getModel).</returns>
	int32_t addModelForAllPasses(const assets::ModelHandle& model, uint16_t effect = 0)
	{
		int32_t index = -1;
		for (std::size_t pass = 0; pass != _renderStructure.effects[effect].passes.size(); ++pass) { index = addModelForAllSubpasses(model, static_cast<uint16_t>(pass), effect); }
		return index;
	}

	/// <summary>Add a model for rendering. This method is a shortcut for adding a model to ALL subpasses of a
	/// specific renderpass.</summary>
	/// <param name="model">A pvr::assets::Model handle to add for rendering.</param>
	/// <param name="effect">The Effect to which the pass to render to belongs. Default 0.</param>
	/// <param name="pass">The Pass of the Effect to whose subpasses the model will be added.</param>
	/// <returns>The order of the model within the RenderManager (the index to use for getModel).</returns>
	int32_t addModelForAllSubpasses(const assets::ModelHandle& model, uint16_t pass, uint16_t effect = 0)
	{
		int32_t index = -1;
		for (uint32_t subpass = 0; subpass != _renderStructure.effects[effect].passes[pass].subpasses.size(); ++subpass)
		{ index = addModelForAllSubpassGroups(model, pass, static_cast<uint16_t>(subpass), effect); }
		return index;
	}

	/// <summary>Add a model for rendering. This method is a shortcut for adding a model to ALL subpass groups
	/// of a specific subpass.</summary>
	/// <param name="model">A pvr::assets::Model handle to add for rendering.</param>
	/// <param name="effect">The Effect to which the pass to render to belongs. Default 0.</param>
	/// <param name="pass">The Pass of the Effect to whose subpasses the model will be added.</param>
	/// <param name="subpass">The Subpasspass of the Effect to whose subpass groups the model will be added.</param>
	/// <returns>The order of the model within the RenderManager (the index to use for getModel).</returns>
	int32_t addModelForAllSubpassGroups(const assets::ModelHandle& model, uint16_t pass, uint16_t subpass, uint16_t effect = 0)
	{
		int32_t index = -1;
		for (uint32_t subpassGroup = 0; subpassGroup < toSubpass(effect, pass, subpass).getNumSubpassGroups(); ++subpassGroup)
		{ index = addModelForSubpassGroup(model, pass, subpass, subpassGroup, effect); }
		return index;
	}

	/// <summary>Add a model for rendering. Adds a model for rendering to a specific subpass.</summary>
	/// <param name="model">A pvr::assets::Model handle to add for rendering.</param>
	/// <param name="effect">The Effect to which the subpass to render to belongs. Default 0.</param>
	/// <param name="pass">The Pass of the Effect to whose subpass the model will be added.</param>
	/// <param name="subpassGroup">The SubpassGroup to which this model will be added.</param>
	/// <param name="subpass">The Subpass to which this model will be rendered.</param>
	/// <returns>The order of the model within the RenderManager (the index to use for getModel).</returns>efault 0.
	int32_t addModelForSubpassGroup(const assets::ModelHandle& model, uint32_t pass, uint32_t subpass, uint32_t subpassGroup, uint32_t effect = 0)
	{
		struct apimodelcomparator
		{
			const assets::ModelHandle& model;
			apimodelcomparator(const assets::ModelHandle& model) : model(model) {}

			bool operator()(const RendermanModel& rhs) { return rhs.assetModel == model; }
		};
		int32_t index;
		auto it = std::find_if(_modelStorage.begin(), _modelStorage.end(), apimodelcomparator(model));

		RendermanModel* apimodel = nullptr;
		// check if the model is already processed
		// if not then make a ney entry else return an index to it.
		if (it == _modelStorage.end())
		{
			index = static_cast<int32_t>(_modelStorage.size());
			_modelStorage.emplace_back(RendermanModel());
			apimodel = &_modelStorage.back();
			apimodel->mgr_ = this;
			apimodel->assetModel = model;
			apimodel->meshes.resize(model->getNumMeshes());
			apimodel->materials.resize(model->getNumMaterials());
			for (uint32_t meshid = 0; meshid < model->getNumMeshes(); ++meshid)
			{
				apimodel->meshes[meshid].assetMesh = assets::getMeshHandle(model, meshid);
				apimodel->meshes[meshid].renderModel_ = apimodel;
				apimodel->meshes[meshid].assetMeshId = meshid;
			}
			for (uint32_t materialid = 0; materialid < model->getNumMaterials(); ++materialid)
			{
				apimodel->materials[materialid].assetMaterial = assets::getMaterialHandle(model, materialid);
				apimodel->materials[materialid].renderModel_ = apimodel;
				apimodel->materials[materialid].assetMaterialId = materialid;
			}
		}
		else
		{
			index = static_cast<int32_t>(it - _modelStorage.begin());
			apimodel = &*it;
		}

		_renderStructure.effects[effect].passes[pass].subpasses[subpass].groups[subpassGroup].allModels.emplace_back(apimodel);
		return index;
	}

	/// <summary>Add an effect to the RenderManager. Must be called before models are added to this effect.</summary>
	/// <param name="effect">A new effect to be added</param>
	/// <param name="cmdBuffer">The commandBuffer to use for uploading commands. Must be submitted by the calee</param>
	/// <returns>The order of the Effect within the RenderManager (the index to use for toEffect()). Return -1 if
	/// error.</returns>
	uint32_t addEffect(const effect::Effect& effect, pvrvk::CommandBuffer& cmdBuffer)
	{
		debug_assertion(cmdBuffer != nullptr, "RenderManager::addEffect - Invalid pvrvk::CommandBuffer");
		pvrvk::Device device = cmdBuffer->getDevice();
		// Validate
		if (!device)
		{
			assertion(false, "RenderManager: Invalid pvrvk::Device");
			return static_cast<uint32_t>(-1);
		}
		this->_device = device;
		effectvk::EffectApi effectapi = std::make_shared<pvr::effectvk::impl::Effect_>(this->_device);
		effectapi->init(effect, _swapchain, cmdBuffer, getAssetProvider(), _vmaAllocator, _vmaAllocator);

		_renderStructure.effects.resize(_renderStructure.effects.size() + 1);
		auto& new_effect = _renderStructure.effects.back();
		new_effect.effect = effectapi;
		new_effect.manager_ = this;

		new_effect.passes.resize(effectapi->getNumPasses());

		for (uint32_t passId = 0; passId < effectapi->getNumPasses(); ++passId)
		{
			RendermanPass& pass = new_effect.passes[passId];
			pass.subpasses.resize(effectapi->getPass(passId).subpasses.size());
			for (uint32_t i = 0; i < ARRAY_SIZE(pass.framebuffer); ++i) { pass.framebuffer[i] = effectapi->getPass(passId).framebuffers[i]; }
			for (uint32_t subpassId = 0; subpassId < effectapi->getPass(passId).subpasses.size(); ++subpassId)
			{ pass.subpasses[subpassId].groups.resize(effectapi->getPass(passId).subpasses[subpassId].groups.size()); }
		}

		return static_cast<uint32_t>(_renderStructure.effects.size() - 1);
	}

	/// <summary>Generate the RenderManager, create the structure, add all rendering effects, create the API objects, and
	/// in general, cook everything. Call AFTER any calls to addEffect(...) and addModel...(...). Call BEFORE any
	/// calls to createAutomaticSemantics(...), update semantics etc.
	/// Calling this function. This function record & submit texture uploading commands and wait for the uploading finishes</summary>
	/// <param name="texUploadCmdPool">A pvrvk::CommandPool to use to allocate command buffers for the uploading commands. Must be
	/// created with the queue family id of <paramRef name="submitQueue"/></param>
	/// <param name="submitQueue">The CommandQueue where the uploading commands will be submitted</param>
	/// <returns>True if successful.</returns>
	void buildRenderObjects(pvrvk::CommandPool& texUploadCmdPool, pvrvk::Queue submitQueue)
	{
		pvrvk::CommandBuffer cmdBuffer = texUploadCmdPool->allocateCommandBuffer();
		pvrvk::Fence fence = submitQueue->getDevice()->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		cmdBuffer->begin();
		buildRenderObjects_(cmdBuffer);
		// submit the commandbuffer
		cmdBuffer->end();
		pvrvk::SubmitInfo submitInfo;
		submitInfo.commandBuffers = &cmdBuffer;
		submitQueue->submit(&submitInfo, 1, fence);
		fence->wait();
	}

	/// <summary>Generate the RenderManager, create the structure, add all rendering effects, create the API objects, and
	/// in general, cook everything. Call AFTER any calls to addEffect(...) and addModel...(...). Call BEFORE any
	/// calls to createAutomaticSemantics(...), update semantics etc.
	/// This overload of the function will provide all the upload commands in a command buffer that will NOT have
	/// been executed yet.</summary>
	/// <param name="texUploadCmdBuffer">The command buffer where the Upload commands will be recorded</param>
	void buildRenderObjects(pvrvk::CommandBuffer& texUploadCmdBuffer) { buildRenderObjects_(texUploadCmdBuffer); }

	/// <summary>Create rendering commands for all objects of all effects,passes,subpasses etc... added to the
	/// RenderManager. Will iterate the entire render structure generating any necessary binding/drawing commands into
	/// <paramref name="cmdBuffer."/></summary>
	/// <param name="cmdBuffer">A command buffer to record the commands into</param>
	/// <param name="swapIdx">The current swap chain (framebuffer image) index to record commands for.</param>
	/// <param name="beginEndRenderPass">If set to true, record a beginRenderPass() at the beginning and
	/// endRenderPass() at the end of this</param>
	/// <remarks>If finer granularity of rendering commands is required, navigate the RenderStructure's objects
	/// (potentially using the RenderNodeIterator (call renderables()) and record rendering commands from them.</remarks>
	void recordAllRenderingCommands(pvrvk::CommandBuffer& cmdBuffer, uint16_t swapIdx, bool beginEndRenderPass = true);

	/// <summary>Return number of effects this render manager owns</summary>
	/// <returns>Number of effects</returns>
	size_t getNumEffects() const { return _renderStructure.effects.size(); }

	/// <summary>For each object of this RenderManager, generates a list of all the semantics that it requires (as
	/// defined in its Effect's pipeline objecT). Then, searches any assets (model, mesh, material, node etc.) for
	/// these semantics, and creates connections between them so that when updateAutomaticSemantics is called, the
	/// updated values are updated in the semantics so that they can be read (with setUniformPtr, or updating buffers
	/// etc.).</summary>
	void createAutomaticSemantics()
	{
		for (RendermanEffect& effect : _renderStructure.effects)
		{
			for (RendermanPass& pass : effect.passes)
			{
				for (RendermanSubpass& subpass : pass.subpasses)
				{
					for (RendermanSubpassGroup& subpassGroup : subpass.groups)
					{
						for (RendermanSubpassGroupModel& subpassModel : subpassGroup.subpassGroupModels) { subpassModel.createAutomaticSemantics(); }
						for (RendermanPipeline& pipe : subpassGroup.pipelines) { pipe.createAutomaticModelSemantics(); }
					}
				}
			}
		}
	}

	/// <summary>Iterates all the nodes semantics per-effect, per-pass, per-subpass, per-model, per-node, and updates their
	/// values to their new, updated values. Needs to have called createAutomaticSemantics before.</summary>
	/// <param name="swapidx">swapchain index</param>
	void updateAutomaticSemantics(uint32_t swapidx)
	{
		for (RendermanEffect& effect : _renderStructure.effects) { effect.updateAutomaticSemantics(swapidx); }
	}
};

// This command is used to update uniform semantics directly into the TypedMem objects
inline bool recordUpdateUniformSemanticToExternalMemory(pvrvk::CommandBufferBase cmdBuffer, int32_t uniformLocation, TypedMem& value_ptr)
{
	throw pvr::UnsupportedOperationError();
	(void)cmdBuffer;
	(void)uniformLocation;
	(void)value_ptr;
	return true;
}

inline void RendermanEffect::endBufferUpdates(uint32_t swapChainIndex)
{
	if (isUpdating[swapChainIndex])
	{
		for (auto& buffer : bufferDefinitions)
		{
			auto& buf = buffer.buffer;
			auto& apibuf = *buf;
			if (apibuf.getDeviceMemory()->isMapped())
			{
				// flush the buffer if necessary
				if ((apibuf.getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
				{ apibuf.getDeviceMemory()->flushRange(apibuf.getDeviceMemory()->getMappedOffset(), apibuf.getDeviceMemory()->getMappedSize()); }
			}
		}
	}
	isUpdating[swapChainIndex] = false;
}

// RendermanPipeline inline definition
inline RendermanSubpassGroup& RendermanPipeline::backToSubpassGroup() { return *subpassGroup_; }

inline RendermanSubpass& RendermanPipeline::backToSubpass() { return backToSubpassGroup().backToSubpass(); }

inline RendermanEffect& RendermanPipeline::backToRendermanEffect() { return backToSubpass().backToRendermanEffect(); }

inline void RendermanPipeline::recordUpdateAllUniformSemantics(pvrvk::CommandBufferBase cmdBuffer)
{
	for (auto& sem : uniformSemantics) { recordUpdateUniformSemanticToExternalMemory(cmdBuffer, sem.second.uniformLocation, sem.second.memory); }
	for (auto& sem : backToRendermanEffect().uniformSemantics) { recordUpdateUniformSemanticToExternalMemory(cmdBuffer, sem.second.uniformLocation, sem.second.memory); }
}

inline void RendermanPipeline::recordUpdateAllUniformModelSemantics(pvrvk::CommandBufferBase cmdBuffer)
{
	for (auto& sem : uniformSemantics) { recordUpdateUniformSemanticToExternalMemory(cmdBuffer, sem.second.uniformLocation, sem.second.memory); }
}

inline void RendermanPipeline::recordUpdateAllUniformEffectSemantics(pvrvk::CommandBufferBase cmdBuffer)
{
	for (auto& sem : backToRendermanEffect().uniformSemantics) { recordUpdateUniformSemanticToExternalMemory(cmdBuffer, sem.second.uniformLocation, sem.second.memory); }
}

inline void RendermanPipeline::recordUpdateAllUniformNodeSemantics(pvrvk::CommandBufferBase cmdBuffer, RendermanNode& node)
{
	for (auto& sem : node.uniformSemantics) { recordUpdateUniformSemanticToExternalMemory(cmdBuffer, sem.second.uniformLocation, sem.second.memory); }
}

inline bool RendermanPipeline::recordUpdateUniformCommandsModelSemantic(pvrvk::CommandBufferBase cmdBuffer, const StringHash& semantic)
{
	auto it = uniformSemantics.find(semantic);
	if (it == uniformSemantics.end() || it->second.uniformLocation == -1) { return false; }
	return recordUpdateUniformSemanticToExternalMemory(cmdBuffer, it->second.uniformLocation, it->second.memory);
}

inline bool RendermanPipeline::recordUpdateUniformCommandsEffectSemantic(pvrvk::CommandBufferBase cmdBuffer, const StringHash& semantic)
{
	auto& cont = backToRendermanEffect().uniformSemantics;
	auto it = cont.find(semantic);
	if (it == cont.end() || it->second.uniformLocation == -1) { return false; }
	return recordUpdateUniformSemanticToExternalMemory(cmdBuffer, it->second.uniformLocation, it->second.memory);
}

inline bool RendermanPipeline::recordUpdateUniformCommandsNodeSemantic(pvrvk::CommandBufferBase cmdBuffer, const StringHash& semantic, RendermanNode& node)
{
	auto& cont = node.uniformSemantics;
	auto it = cont.find(semantic);
	if (it == cont.end() || it->second.uniformLocation == -1) { return false; }
	return recordUpdateUniformSemanticToExternalMemory(cmdBuffer, it->second.uniformLocation, it->second.memory);
}

inline bool RendermanPipeline::updateBufferEntryModelSemantic(const StringHash& semantic, const FreeValue& value, uint32_t dynamicSlice)
{
	auto it = bufferEntrySemantics.find(semantic);
	if (it == bufferEntrySemantics.end())
	{
		Log(LogLevel::Debug, "RendermanPipeline::updateBufferEntryModelSemantic - Semantic '%s' not found", semantic.c_str());
		return false;
	}
	auto& sem = it->second;
	auto buffer = *sem.buffer;

	sem.structuredBufferView->getElement(sem.entryIndex, dynamicSlice).setValue(value);
	if ((buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{ buffer->getDeviceMemory()->flushRange(buffer->getDeviceMemory()->getMappedOffset(), buffer->getDeviceMemory()->getMappedSize()); }
	return true;
}

inline bool RendermanPipeline::updateBufferEntryModelSemantics(const StringHash* semantics, const FreeValue* value, uint32_t numSemantics, uint32_t dynamicSlice)
{
	for (uint32_t i = 0; i < numSemantics; ++i)
	{
		if (!updateBufferEntryModelSemantic(semantics[i], value[i], dynamicSlice)) { return false; }
	}
	return true;
}

inline bool RendermanPipeline::updateBufferEntryEffectSemantic(const StringHash& semantic, const FreeValue& value, uint32_t swapid, uint32_t dynamicClientId)
{
	return backToRendermanEffect().updateBufferEntryEffectSemantic(semantic, value, swapid, dynamicClientId);
}

inline bool RendermanPipeline::updateBufferEntryNodeSemantic(const StringHash& semantic, const FreeValue& value, uint32_t swapid, RendermanNode& node)
{
	auto& effect = backToRendermanEffect();
	auto it = bufferEntrySemantics.find(semantic);
	if (it == bufferEntrySemantics.end())
	{
		auto& cont = effect.bufferEntrySemantics;
		it = cont.find(semantic);
		if (it == cont.end()) { return false; }
	}
	BufferEntrySemantic& sem = it->second;

	uint32_t dynamicSliceId = static_cast<uint32_t>(-1);

	for (uint32_t i = 0; i < node.dynamicBuffer[sem.setId].size(); ++i)
	{
		if (&node.dynamicBuffer[sem.setId][i]->structuredBufferView == sem.structuredBufferView)
		{
			dynamicSliceId = node.dynamicSliceId[sem.setId][swapid][i];
			break;
		}
	}

	sem.structuredBufferView->getElement(sem.entryIndex, 0, dynamicSliceId).setValue(value);
	return true;
}

inline bool RendermanPipeline::updateBufferEntryEffectSemantics(const StringHash* semantics, const FreeValue* value, uint32_t numSemantics, uint32_t swapid, uint32_t dynamicClientId)
{
	for (uint32_t i = 0; i < numSemantics; ++i)
	{
		auto& cont = backToRendermanEffect().bufferEntrySemantics;
		auto it = cont.find(semantics[i]);
		if (it == cont.end()) { continue; }
		auto& sem = it->second;
		sem.structuredBufferView->getElement(sem.entryIndex, dynamicClientId, swapid).setValue(value[i]);
	}
	return true;
}

inline bool RendermanPipeline::updateBufferEntrySemantic(const StringHash& semantic, const FreeValue& value, uint32_t swapid, uint32_t dynamicClientId)
{
	auto& effect = backToRendermanEffect();
	auto it = bufferEntrySemantics.find(semantic);
	if (it == bufferEntrySemantics.end())
	{
		auto& cont = effect.bufferEntrySemantics;
		it = cont.find(semantic);
		if (it == cont.end()) { return false; }
	}
	auto& sem = it->second;

	sem.structuredBufferView->getElement(sem.entryIndex, dynamicClientId, swapid).setValue(value);
	return true;
}

inline bool RendermanPipeline::updateUniformModelSemantic(const StringHash& semantic, const TypedMem& value)
{
	auto it = uniformSemantics.find(semantic);
	if (it == uniformSemantics.end()) { return false; }
	auto& sem = it->second;
	debug_assertion(value.isDataCompatible(it->second.memory), "updateUniformModelSemantic: Semantic not found in pipeline");
	if (!value.isDataCompatible(it->second.memory)) { return false; }
	sem.memory = value;
	return true;
}

inline bool RendermanPipeline::updateUniformEffectSemantic(const StringHash& semantic, const TypedMem& value)
{
	auto& cont = subpassGroup_->backToSubpass().backToRendermanPass().backToEffect().uniformSemantics;
	auto it = cont.find(semantic);
	if (it == cont.end())
	{
		debug_assertion(false, strings::createFormatted("updateUniformModelSemantic: Semantic [%s] not found in pipeline", semantic.c_str()));
		return false;
	}
	auto& sem = it->second;
	debug_assertion(value.isDataCompatible(it->second.memory),
		strings::createFormatted("updateUniformModelSemantic: Semantic value passed for semantic [%s] type incompatible with uniform type found.       "
								 "Passed: Datatype id [%d], ArrayElements [%d]   Required: Passed: Datatype id [%d], ArrayElements [%d]",
			semantic.c_str(), value.dataType(), value.arrayElements(), sem.memory.dataType(), sem.memory.arrayElements()));
	if (!value.isDataCompatible(it->second.memory)) { return false; }
	sem.memory = value;
	return true;
}

inline bool RendermanPipeline::updateUniformNodeSemantic(const StringHash& semantic, const TypedMem& value, RendermanNode& node)
{
	auto it = node.uniformSemantics.find(semantic);
	if (it == node.uniformSemantics.end())
	{
		debug_assertion(false, strings::createFormatted("updateUniformNodeSemantic: Semantic [%s] not found in pipeline for node id [%d]", semantic.c_str(), node.assetNodeId));
		return false;
	}
	auto& sem = it->second;
	debug_assertion(value.isDataCompatible(it->second.memory),
		strings::createFormatted("updateUniformNodeSemantic: Semantic value passed for semantic [%s] type incompatible with uniform type found in node [%d].       "
								 "Passed: Datatype id [%d], ArrayElements [%d]   Required: Passed: Datatype id [%d], ArrayElements [%d]",
			semantic.c_str(), node.assetNodeId, value.dataType(), value.arrayElements(), sem.memory.dataType(), sem.memory.arrayElements()));
	if (!value.isDataCompatible(it->second.memory)) { return false; }
	sem.memory = value;
	return true;
}

inline bool RendermanPipeline::createAutomaticModelSemantics(uint32_t useMainModelId)
{
	{
		automaticModelBufferEntrySemantics.clear();

		auto& model = backToRendermanEffect().manager_->renderModels()[useMainModelId];

		auto& cont = backToRendermanEffect().bufferEntrySemantics;

		for (auto& reqsem : cont)
		{
			ModelSemanticSetter setter = model.getModelSemanticSetter(reqsem.first);
			if (setter == nullptr) { Log(LogLevel::Information, "Automatic Model semantic [%s] not found.", reqsem.first.c_str()); }
			else
			{
				Log(LogLevel::Information, "Automatic Model semantic [%s] found! Creating automatic connection with model [%d]", reqsem.first.c_str(), useMainModelId);
				automaticModelBufferEntrySemantics.resize(automaticModelBufferEntrySemantics.size() + 1);
				AutomaticModelBufferEntrySemantic& autosem = automaticModelBufferEntrySemantics.back();
				autosem.model = &model;
				autosem.buffer = reqsem.second.buffer;
				autosem.entryIndex = reqsem.second.entryIndex;
				autosem.semanticSetFunc = setter;
				autosem.semantic = &reqsem.first;
				autosem.structuredBufferView = reqsem.second.structuredBufferView;
			}
		}
	}
	{
		automaticModelUniformSemantics.clear();

		auto& model = backToRendermanEffect().manager_->renderModels()[useMainModelId];

		auto& cont = uniformSemantics;

		for (auto& reqsem : cont)
		{
			ModelSemanticSetter setter = model.getModelSemanticSetter(reqsem.first);
			if (setter == nullptr) { Log(LogLevel::Information, "Automatic Model semantic [%s] not found.", reqsem.first.c_str()); }
			else
			{
				Log(LogLevel::Information, "Automatic Model semantic [%s] found! Creating automatic connection with model [%d]", reqsem.first.c_str(), useMainModelId);
				automaticModelUniformSemantics.resize(automaticModelUniformSemantics.size() + 1);
				AutomaticModelUniformSemantic& autosem = automaticModelUniformSemantics.back();
				autosem.model = &model;
				autosem.semanticSetFunc = setter;
				autosem.semantic = &reqsem.first;
				autosem.memory = &reqsem.second.memory;
			}
		}
	}
	return true;
}

inline bool RendermanPipeline::updateAutomaticModelSemantics(uint32_t swapidx)
{
#ifdef PVR_RENDERMANAGER_DEBUG_SEMANTIC_UPDATES
	Log(LogLevel::Information, "\n************** RendermanPipeline::updateAutomaticModelSemantics ****************");
#endif
	TypedMem val;
	const uint32_t numSwapchains = this->backToRendermanEffect().backToRenderManager().getSwapchain()->getSwapchainLength();
	for (auto& sem : automaticModelBufferEntrySemantics)
	{
		const uint32_t numSlicesPerSwapchains = sem.structuredBufferView->getNumDynamicSlices() / numSwapchains;
		debug_assertion(numSlicesPerSwapchains == 1, "");
		const uint64_t offset = swapidx * numSlicesPerSwapchains * sem.structuredBufferView->getDynamicSliceSize();
		const uint32_t slizeIndex = static_cast<uint32_t>(offset / sem.structuredBufferView->getDynamicSliceSize());

#ifdef PVR_RENDERMANAGER_DEBUG_SEMANTIC_UPDATES
		Log(LogLevel::Information, "slice index %d, semantic entry index %d", sem.structuredBufferView->getMappedDynamicSlice(), sem.entryIndex);
#endif
		sem.semanticSetFunc(val, *sem.model);
		sem.structuredBufferView->getElement(sem.entryIndex, 0, slizeIndex).setValue(val);
	}
	for (auto& sem : automaticModelUniformSemantics)
	{
		sem.semanticSetFunc(val, *sem.model);
		(const TypedMem&)(*sem.memory) = val; // const is to avoid the "allocate" call in the TypedMem
	}
	return true;
}

inline void RendermanNode::updateAutomaticSemantics(uint32_t swapidx)
{
#ifdef PVR_RENDERMANAGER_DEBUG_SEMANTIC_UPDATES
	Log(LogLevel::Information, "\n\n*********** RendermanNode::updateAutomaticSemantics **************");
	Log(LogLevel::Information, "Swapindex %d", swapidx);
#endif
	TypedMem val; // Keep it here to avoid tons of different memory allocations...
	for (auto& sem : automaticEntrySemantics)
	{
		uint32_t dynamicSlizeId = sem.bufferDynamicSlice[swapidx];
#ifdef PVR_RENDERMANAGER_DEBUG_BUFFERS
		Log(LogLevel::Debug, "dynamicSlizeId: %d, semantic entryIndex: %d  Semantic: %s Offset: %d", dynamicSlizeId, sem.entryIndex, sem.semantic->c_str(),
			sem.structuredBufferView->getElement(sem.entryIndex, 0, dynamicSlizeId).getOffset());
#endif
		sem.semanticSetFunc(val, *this);

		auto ele = sem.structuredBufferView->getElement(sem.entryIndex, 0, dynamicSlizeId);
		ele.setArrayValuesStartingFromThis(val);
	}
	for (auto& sem : automaticUniformSemantics)
	{
		sem.semanticSetFunc(val, *this);
		(const TypedMem&)(*sem.memory) = val; // const is to avoid the "allocate" call in the TypedMem
	}
}

inline void RendermanNode::createAutomaticSemantics()
{
	automaticEntrySemantics.clear();
	const uint32_t numSwapchains = this->toRendermanPipeline().backToRendermanEffect().backToRenderManager().getSwapchain()->getSwapchainLength();
	for (auto& reqsem : this->toRendermanPipeline().bufferEntrySemantics)
	{
		NodeSemanticSetter setter = getNodeSemanticSetter(reqsem.first);
		if (setter == nullptr) { Log(LogLevel::Information, "Renderman: Automatic node semantic [%s] not found.", reqsem.first.c_str()); }
		else
		{
			Log(LogLevel::Information, "Renderman: Automatic node semantic [%s] found! Creating automatic connection:", reqsem.first.c_str());
			automaticEntrySemantics.resize(automaticEntrySemantics.size() + 1);
			AutomaticNodeBufferEntrySemantic& autosem = automaticEntrySemantics.back();
			autosem.buffer = reqsem.second.buffer;
			autosem.dynamicOffsetNodeId = reqsem.second.dynamicOffsetNodeId;
			autosem.entryIndex = reqsem.second.entryIndex;
			autosem.setId = reqsem.second.setId;
			autosem.semanticSetFunc = setter;
			autosem.structuredBufferView = reqsem.second.structuredBufferView;
			autosem.semantic = &reqsem.first;
			memset(autosem.bufferDynamicOffset, 0, sizeof(autosem.bufferDynamicOffset));
			memset(autosem.bufferDynamicSlice, 0, sizeof(autosem.bufferDynamicSlice));
			uint32_t i = 0;
			for (; i < this->dynamicBuffer[autosem.setId].size(); ++i)
			{
				if (&dynamicBuffer[autosem.setId][i]->buffer == autosem.buffer)
				{
					for (uint32_t j = 0; j < numSwapchains; ++j)
					{
						autosem.bufferDynamicOffset[j] = this->dynamicOffset[autosem.setId][j][i];
						autosem.bufferDynamicSlice[j] = this->dynamicSliceId[autosem.setId][j][i];
					}
					break;
				}
			}

			debug_assertion(i != this->dynamicBuffer[autosem.setId].size(), "");
		}
	}
	for (auto& reqsem : uniformSemantics)
	{
		NodeSemanticSetter setter = getNodeSemanticSetter(reqsem.first);
		if (setter == nullptr) { Log(LogLevel::Information, "Automatic node semantic [%s] not found.", reqsem.first.c_str()); }
		else
		{
			Log(LogLevel::Information, "Automatic node semantic [%s] found! Creating automatic connection:", reqsem.first.c_str());
			automaticUniformSemantics.resize(automaticUniformSemantics.size() + 1);
			AutomaticNodeUniformSemantic& autosem = automaticUniformSemantics.back();
			autosem.semanticSetFunc = setter;
			autosem.semantic = &reqsem.first;
			autosem.memory = &reqsem.second.memory;
		}
	}
	std::sort(automaticEntrySemantics.begin(), automaticEntrySemantics.end(),
		[](const AutomaticNodeBufferEntrySemantic& a, const AutomaticNodeBufferEntrySemantic& b) { return a.entryIndex < b.entryIndex; });
}

//                                      RendermanSubpassMaterial inline definition
inline const RendermanModel& RendermanSubpassMaterial::backToModel() const { return *backToSubpassGroupModel().renderModel_; }
inline RendermanModel& RendermanSubpassMaterial::backToModel() { return *backToSubpassGroupModel().renderModel_; }
inline RendermanSubpassGroup& RendermanSubpassMaterial::backToSubpassGroup() { return *backToSubpassGroupModel().renderSubpassGroup_; }
inline const RendermanSubpassGroup& RendermanSubpassMaterial::backToSubpassGroup() const { return *backToSubpassGroupModel().renderSubpassGroup_; }

//                                      RendermanSubpassMesh inline definition
inline const RendermanModel& RendermanSubpassMesh::backToModel() const { return *backToSubpassGroupModel().renderModel_; }
inline RendermanModel& RendermanSubpassMesh::backToModel() { return *backToSubpassGroupModel().renderModel_; }
inline const RendermanSubpassGroup& RendermanSubpassMesh::backToSubpassGroup() const { return *backToSubpassGroupModel().renderSubpassGroup_; }
inline RendermanSubpassGroup& RendermanSubpassMesh::backToSubpassGroup() { return *backToSubpassGroupModel().renderSubpassGroup_; }

//                                      RendermanSubpassModel inline definition
inline RendermanModel& RendermanSubpassGroupModel::backToModel() { return *renderModel_; }
inline RenderManager& RendermanSubpassGroupModel::backToRenderManager() { return *renderModel_->mgr_; }
inline RendermanSubpassGroup& RendermanSubpassGroupModel::backToRendermanSubpassGroup() { return *renderSubpassGroup_; }
inline RendermanSubpass& RendermanSubpassGroupModel::backToRendermanSubpass() { return renderSubpassGroup_->backToSubpass(); }
inline RendermanPass& RendermanSubpassGroupModel::backToRendermanPass() { return renderSubpassGroup_->backToSubpass().backToRendermanPass(); }
inline RendermanEffect& RendermanSubpassGroupModel::backToRendermanEffect() { return renderSubpassGroup_->backToSubpass().backToRendermanEffect(); }

//                                      RendermanMaterial inline definition
inline const RendermanModel& RendermanMaterial::backToRendermanModel() const { return *renderModel_; }
inline const RendermanModel& RendermanMaterial::backToRendermanModel() { return *renderModel_; }
inline const RenderManager& RendermanMaterial::backToRenderManager() const { return *backToRendermanModel().mgr_; }
inline RenderManager& RendermanMaterial::backToRenderManager() { return *backToRendermanModel().mgr_; }

//                                      RendermanMesh inline definition
inline const RenderManager& RendermanMesh::backToRenderManager() const { return *backToRendermanModel().mgr_; }
inline RenderManager& RendermanMesh::backToRenderManager() { return *backToRendermanModel().mgr_; }

//                                      RendermanMaterialSubpassPipeline inline definition
inline const RendermanPipeline& RendermanMaterialSubpassPipeline::toPipeline() const { return *pipeline_; }
inline RendermanPipeline& RendermanMaterialSubpassPipeline::toPipeline() { return *pipeline_; }

//                                      RendermanSubpass inline definition
inline const RendermanEffect& RendermanSubpass::backToRendermanEffect() const { return *renderingPass_->renderEffect_; }
inline RendermanEffect& RendermanSubpass::backToRendermanEffect() { return *renderingPass_->renderEffect_; }
inline const RenderManager& RendermanSubpass::backToRenderManager() const { return *backToRendermanEffect().manager_; }
inline RenderManager& RendermanSubpass::backToRenderManager() { return *backToRendermanEffect().manager_; }

//                                      RendermanEffect inline definition
inline const RenderManager& RendermanEffect::backToRenderManager() const { return *manager_; }
inline RenderManager& RendermanEffect::backToRenderManager() { return *manager_; }

inline const RendermanSubpass& RendermanSubpassGroup::backToSubpass() const { return *subpass_; }
inline RendermanSubpass& RendermanSubpassGroup::backToSubpass() { return *subpass_; }
} // namespace utils
} // namespace pvr
