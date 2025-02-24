/*!
\brief Contains the implementations for the PowerVR RenderManager and other Rendering helpers
\file PVRUtils/Vulkan/RenderManagerVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN

#include "PVRPfx/RenderManagerVk.h"
#include "PVRVk/PipelineLayoutVk.h"
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/SwapchainVk.h"
#include "PVRVk/QueueVk.h"
#include "PVRVk/CommandBufferVk.h"
#include "PVRVk/HeadersVk.h"
#include "PVRUtils/Vulkan/HelperVk.h"
#include "PVRCore/strings/StringHash.h"
#include "PVRCore/math/MathUtils.h"
#include <algorithm>
namespace pvr {
namespace utils {
using namespace pvrvk;
typedef std::vector<utils::AttributeLayout> AttributeConfiguration;

struct PipelineSet
{
	std::vector<StringHash> pipelines;
	bool operator<(const PipelineSet& rhs) const { return std::lexicographical_compare(pipelines.begin(), pipelines.end(), rhs.pipelines.begin(), rhs.pipelines.end()); }
	PipelineSet() {}
	PipelineSet(std::set<StringHash>& set)
	{
		pipelines.resize(static_cast<uint32_t>(set.size()));
		uint32_t count = 0;
		for (auto& setitem : set) { pipelines[count++] = setitem; }
	}
};

////////// PIPELINE SELECTION ///////////
namespace {
inline std::pair<StringHash, effectvk::PipelineDef*> selectPipelineForSubpassGroupMeshMaterial(
	effectvk::EffectApi& effect, const effectvk::SubpassGroup& pipes, const assets::Mesh& mesh, const assets::Material& material)
{
	bool incompatible = true;
	auto cond_pipe_it = pipes.pipelines.begin();
	// This loop will break if a compatible pipeline was found.
	for (; cond_pipe_it != pipes.pipelines.end(); ++cond_pipe_it)
	{
		incompatible = false; // Start with the assumption that the pipeline is compatible
		// This loop will break if a compatible pipeline was found.
		for (auto condition = cond_pipe_it->conditions.begin(); condition != cond_pipe_it->conditions.end() && !incompatible; ++condition)
		{
			// This loop will complete if the pipeline is incompatible, and the outer loop will move to the next pipeline.
			// Otherwise, if no incompatible conditions are found, it will break while "incompatible=false", which will also break the outer loop
			// and cond_pipe_it will be the first compatible pipeline.
			switch (condition->type)
			{
			case effect::PipelineCondition::AttributeRequired: incompatible = (mesh.getVertexAttributeByName(condition->value) == 0); break;
			case effect::PipelineCondition::AttributeRequiredNo: incompatible = (mesh.getVertexAttributeByName(condition->value) != 0); break;
			case effect::PipelineCondition::UniformRequired: incompatible = !material.hasSemantic(condition->value); break;
			case effect::PipelineCondition::UniformRequiredNo: incompatible = material.hasSemantic(condition->value); break;
			default: Log(LogLevel::Warning, "Unsupported effect::PipelineCondition used.");
			}
		}
		if (!incompatible) { break; }
	}

	if (!incompatible) // cond_pipe_it!=pass.subpasses[subpass].pipelines.end()
	{
		effectvk::PipelineDef* pipeDef = effect->getPipelineDefinition(cond_pipe_it->pipeline);
		pipeDef->createParam.inputAssembler.setPrimitiveTopology(convertToPVRVk(mesh.getPrimitiveType()));
		return std::make_pair(cond_pipe_it->pipeline, pipeDef);
	}

	Log("failed to find a compatible pipeline for a mesh with material %s", material.getName().c_str());
	return std::make_pair(StringHash(), (effectvk::PipelineDef*)nullptr);
}

inline std::pair<StringHash, const effectvk::PipelineDef*> selectPipelineForSubpassGroupMeshMaterial(
	effectvk::EffectApi& effect, uint32_t passId, uint32_t subpassId, uint32_t subpassGroupId, const assets::Mesh& mesh, const assets::Material& material)
{
	auto& pass = effect->getPass(passId);
	return selectPipelineForSubpassGroupMeshMaterial(effect, pass.subpasses[subpassId].groups[subpassGroupId], mesh, material);
}
} // namespace

////////// ATTRIBUTES AND VBO MANAGEMENT
namespace {
// A reswizzler is the function we will call to read Vertex data with a
// specific layout from a piece of memory, into another piece of memory, with a different layout.
typedef void (*Reswizzler)(
	uint8_t* to, uint8_t* from, uint32_t toOffset, uint32_t fromOffset, uint32_t toWidth, uint32_t fromWidth, uint32_t tostride, uint32_t fromstride, uint32_t items);

template<typename Fromtype, typename Totype>
void attribToAttrib(uint8_t* to, uint8_t* from, uint32_t toOffset, uint32_t fromOffset, uint32_t toWidth, uint32_t fromWidth, uint32_t tostride, uint32_t fromstride, uint32_t items)
{
	uint_fast16_t width = std::min(fromWidth, toWidth);
	for (uint_fast32_t item = 0; item < items; ++item)
	{
		unsigned char* tmpTo = to + toOffset + item * tostride;
		unsigned char* tmpFrom = from + fromOffset + item * fromstride;
		uint32_t vec = 0;
		for (; vec < width; ++vec)
		{
			Fromtype from_value = *reinterpret_cast<Fromtype*>(tmpFrom + vec * sizeof(Fromtype));
			Totype to_value = (Totype)from_value;
			*reinterpret_cast<Totype*>(tmpTo + vec * sizeof(Totype)) = to_value;
		}

		for (; vec < 3 && vec < toWidth; ++vec) { *reinterpret_cast<Totype*>(tmpTo + vec * sizeof(Totype)) = (Totype)0; }
		for (; vec < toWidth; ++vec) { *reinterpret_cast<Totype*>(tmpTo + vec * sizeof(Totype)) = (Totype)1; }
	}
}

struct Remapper
{
	void* to;
	void* from;
	uint32_t tooffset, fromoffset, towidth, fromwidth, tostride, fromstride;
};

template<typename Fromtype, typename Totype>
inline void attribToAttrib(Remapper remap, size_t numitems)
{
	attribToAttrib<Fromtype, Totype>(static_cast<unsigned char*>(remap.to), static_cast<unsigned char*>(remap.from), remap.tooffset, remap.fromoffset, remap.towidth,
		remap.fromwidth, remap.tostride, remap.fromstride, numitems);
}

Reswizzler selectReswizzler(DataType fromType, DataType toType)
{
	switch (fromType)
	{
	case DataType::Float32:
		switch (toType)
		{
		case DataType::Float32: return &(attribToAttrib<float, float>);
		case DataType::Int32:
		case DataType::UInt32: return &(attribToAttrib<float, int32_t>);
		case DataType::Int16:
		case DataType::UInt16: return &(attribToAttrib<float, int16_t>);
		case DataType::Int8:
		case DataType::UInt8: return &(attribToAttrib<float, int8_t>);

		default: assertion(false, "Unsupported POD Vertex Datatype"); break;
		}
	case DataType::Int32:
	case DataType::UInt32:
		switch (toType)
		{
		case DataType::Float32: return &(attribToAttrib<int32_t, float>);
		case DataType::Int32:
		case DataType::UInt32: return &(attribToAttrib<int32_t, int32_t>);
		case DataType::Int16:
		case DataType::UInt16: return &(attribToAttrib<int32_t, int16_t>);
		case DataType::Int8:
		case DataType::UInt8: return &(attribToAttrib<int32_t, int8_t>); break;

		default: assertion(false, "Unsupported POD Vertex Datatype"); break;
		}
	case DataType::Int16:
	case DataType::UInt16:
		switch (toType)
		{
		case DataType::Float32: return &(attribToAttrib<int16_t, float>);
		case DataType::Int32:
		case DataType::UInt32: return &(attribToAttrib<int16_t, int32_t>);
		case DataType::Int16:
		case DataType::UInt16: return &(attribToAttrib<int16_t, int16_t>);
		case DataType::Int8:
		case DataType::UInt8: return &(attribToAttrib<int16_t, int8_t>);

		default: assertion(false, "Unsupported POD Vertex Datatype"); break;
		}
	case DataType::Int8:
	case DataType::UInt8:
		switch (toType)
		{
		case DataType::Float32: return &(attribToAttrib<int8_t, float>);
		case DataType::Int32:
		case DataType::UInt32: return &(attribToAttrib<int8_t, int32_t>);
		case DataType::Int16:
		case DataType::UInt16: return &(attribToAttrib<int8_t, int16_t>);
		case DataType::Int8:
		case DataType::UInt8: return &(attribToAttrib<int8_t, int8_t>);

		default: assertion(false, "Unsupported POD Vertex Datatype"); break;
		}
		break;

	default: assertion(false, "Unsupported POD Vertex Datatype"); break;
	}
	return NULL;
}

inline void populateVbos(AttributeConfiguration& attribConfig, std::vector<Buffer>& vbos, assets::Mesh& mesh)
{
	Reswizzler reswizzler;
	uint32_t numVertices = mesh.getNumVertices();

	std::vector<uint8_t> ptrs[16];
	for (uint32_t i = 0; i < vbos.size(); ++i) { ptrs[i].resize(!vbos[i] ? (size_t)0 : (size_t)vbos[i]->getSize()); }

	for (uint32_t binding = 0; binding < attribConfig.size(); ++binding)
	{
		for (uint32_t attribute = 0; attribute < attribConfig[binding].size(); ++attribute)
		{
			auto& attrib = attribConfig[binding][attribute];
			const auto& mattrib = mesh.getVertexAttributeByName(attrib.semantic);

			if (mattrib == NULL) { continue; }
			uint32_t mbinding = mattrib->getDataIndex();
			DataType mdatatype = mattrib->getVertexLayout().dataType;
			uint32_t mwidth = mattrib->getVertexLayout().width;
			uint8_t* mptr = mesh.getData(mbinding);

			uint8_t* ptr = ptrs[binding].data();

			reswizzler = selectReswizzler(mdatatype, attrib.datatype);

			reswizzler(ptr, mptr, attrib.offset, mattrib->getOffset(), attrib.width, mwidth, attribConfig[binding].stride, mesh.getStride(mbinding), numVertices);
		}
	}

	for (uint32_t i = 0; i < vbos.size(); ++i)
	{
		if (vbos[i]) { pvr::utils::updateHostVisibleBuffer(vbos[i], ptrs[i].data(), 0, vbos[i]->getSize(), true); }
	}
}

inline void createVbos(utils::RenderManager& renderman, const std::map<assets::Mesh*, AttributeConfiguration*>& meshAttribConfig)
{
	Device device = renderman.getDevice().lock();

	auto& apiModels = renderman.renderModels();

	for (uint32_t model_id = 0; model_id < apiModels.size(); ++model_id)
	{
		for (uint32_t mesh_id = 0; mesh_id < apiModels[model_id].assetModel->getNumMeshes(); ++mesh_id)
		{
			utils::RendermanMesh& apimesh = apiModels[model_id].meshes[mesh_id];
			assets::Mesh& mesh = *apimesh.assetMesh;

			const auto& found = meshAttribConfig.find(&mesh);
			if (found == meshAttribConfig.end())
			{
				Log("Renderman: Failed to create a vbo for the mesh id %d, model id %d", mesh_id, model_id);
				continue;
			}

			auto& attribConfig = *found->second;

			if (mesh.getFaces().getDataSize() > 0)
			{
				apimesh.ibo = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(mesh.getFaces().getDataSize(), pvrvk::BufferUsageFlags::e_INDEX_BUFFER_BIT),
					pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
					pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
					renderman.getAllocator(), pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
				apimesh.indexType = mesh.getFaces().getDataType();

				assertion(apimesh.ibo != nullptr, strings::createFormatted("RenderManager: Could not create IBO for mesh [%d] of model [%d]", mesh_id, model_id));
				pvr::utils::updateHostVisibleBuffer(apimesh.ibo, static_cast<const void*>(mesh.getFaces().getData()), 0, mesh.getFaces().getDataSize(), true);
			}

			uint32_t size = static_cast<uint32_t>(attribConfig.size());
			size = (mesh.getVertexData().empty() ? 0 : size); // make sure the mesh has a vertex data.
			apimesh.vbos.resize(size);
			for (uint32_t vbo_id = 0; vbo_id < size; ++vbo_id)
			{
				if (attribConfig[vbo_id].size() == 0) { continue; } // empty binding
				uint32_t vboSize = attribConfig[vbo_id].stride * mesh.getNumVertices();

				apimesh.vbos[vbo_id] =
					pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(vboSize, pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
						pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
						renderman.getAllocator(), pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
				populateVbos(attribConfig, apimesh.vbos, mesh);
				assertion(apimesh.vbos[vbo_id] != nullptr, strings::createFormatted("RenderManager: Could not create VBO[%d] for mesh [%d] of model [%d]", mesh_id, model_id));
			}
		}
	}
}

inline void addVertexAttributesToVboLayout(std::vector<StringHash>& inner, const std::vector<StringHash>& outer)
{
	for (auto it_outer = outer.begin(); it_outer < outer.end(); ++it_outer)
	{
		bool found = false;
		for (auto it_inner = inner.begin(); !found && it_inner < inner.end(); ++it_inner)
		{
			if (*it_outer == *it_inner) { found = true; }
		}
		if (!found) { inner.emplace_back(std::move(*it_outer)); }
	}
}

inline std::vector<StringHash> getVertexBindingsForPipeline(const effectvk::EffectApi& effect, const StringHash& pipelineName)
{
	std::vector<StringHash> retval;
	auto& pipe = effect->getEffectAsset().versionedPipelines.find(effect->getApiString())->second.find(pipelineName)->second;
	for (auto it = pipe.attributes.begin(); it != pipe.attributes.end(); ++it)
	{
		StringHash vn = it->semantic;
		retval.emplace_back(vn);
	}
	return retval;
}

inline std::vector<StringHash> getAllActiveVertexAttributesForMeshAndEffect(effectvk::EffectApi& effect, const assets::Mesh& mesh, const assets::Material& material)
{
	// TO OPTIMIZE: This function should be called per SUBPASS, not for the whole effect
	std::vector<StringHash> attributes;

	for (uint32_t pass = 0; pass < effect->getNumPasses(); ++pass)
	{
		for (uint32_t subpass = 0; subpass < effect->getPass(pass).subpasses.size(); ++subpass)
		{
			for (uint32_t subpassGroup = 0; subpassGroup < effect->getPass(pass).subpasses[subpass].groups.size(); ++subpassGroup)
			{
				std::pair<StringHash, const effectvk::PipelineDef*> pipe = selectPipelineForSubpassGroupMeshMaterial(effect, pass, subpass, subpassGroup, mesh, material);
				addVertexAttributesToVboLayout(attributes, getVertexBindingsForPipeline(effect, pipe.first));
			}
		}
	}
	return attributes;
}

inline AttributeConfiguration getVertexBindingsForPipeNoStride(const effectvk::EffectApi& effect, const StringHash& pipelineName)
{
	AttributeConfiguration retval;
	auto& pipe = effect->getEffectAsset().versionedPipelines.find(effect->getApiString())->second.find(pipelineName)->second;

	// Assuming up to 32 (!!!) VBO bindings...
	for (auto it = pipe.attributes.begin(); it != pipe.attributes.end(); ++it)
	{
		uint16_t binding = it->vboBinding;
		if (binding >= retval.size()) { retval.resize(it->vboBinding + 1u); }
		uint32_t width = getNumMatrixColumns(it->dataType) * getNumVecElements(it->dataType);
		DataType datatype = DataType::None;
		if (retval[binding].size() <= it->location) { retval[binding].resize(it->location + 1); }
		retval[binding][it->location] = utils::Attribute(it->semantic, datatype, static_cast<uint16_t>(width), static_cast<uint16_t>(retval[binding].stride), it->variableName);
	}
	return retval;
}

inline AttributeConfiguration getVertexBindingsForPipe(const effectvk::EffectApi& effect, const StringHash& pipelineName)
{
	AttributeConfiguration retval;
	auto& pipe = effect->getEffectAsset().versionedPipelines.find(effect->getApiString())->second.find(pipelineName)->second;
	uint32_t count[32] = {};

	// Assuming up to 32 (!!!) VBO bindings...
	for (auto it = pipe.attributes.begin(); it != pipe.attributes.end(); ++it)
	{
		uint16_t binding = it->vboBinding;
		if (binding >= retval.size()) { retval.resize(it->vboBinding + 1); }

		uint32_t width = getNumMatrixColumns(it->dataType) * getNumVecElements(it->dataType);
		DataType datatype = toDataType(it->dataType);
		retval[binding].resize(count[binding] + 1);
		// retval[binding].stride temporarily contains the offset! Since we do packing,
		// the "stride" is the offset of the last one...
		retval[binding][count[binding]++] = utils::Attribute(it->semantic, datatype, static_cast<uint16_t>(width), static_cast<uint16_t>(retval[binding].stride), it->variableName);
		retval[binding].stride += width * dataTypeSize(datatype); // 4 - we only support float, int32_t;

#pragma warning TODO_MUST_CHANGE_DATATYPE_FROM_THE_MESH_ALWAYS_ASSUMING_32_BIT_MUST_CHANGE
	}
	return retval;
}

inline utils::Attribute mergeAttribute(utils::Attribute one, const utils::Attribute& two)
{
	// Assume semantic1 = semantic2
	assertion(one.semantic == two.semantic,
		"RenderManager: Error processing effects. "
		"Attempted to merge attributes with different semantics");

	one.datatype = std::min(one.datatype, two.datatype);
	one.width = std::max(one.width, two.width);
	return one;
}

inline void fixVertexLayoutDatatypes(utils::Attribute& one, const assets::VertexAttributeData& two)
{
	assertion(one.semantic == two.getSemantic(),
		"RenderManager: Error processing effects. "
		"Attempted to merge attributes with different semantics");
	one.datatype = (one.datatype == pvr::DataType::None ? two.getVertexLayout().dataType : std::min(one.datatype, two.getVertexLayout().dataType));
}

inline void mergeAttributeLayouts(utils::AttributeLayout& inout_inner, utils::AttributeLayout& willBeDestroyed_outer)
{
	std::vector<utils::Attribute> inner(inout_inner.begin(), inout_inner.end());
	uint32_t inner_initial_size = static_cast<uint32_t>(inner.size()); // No point in checking the ones we just added - skip the end of the list.
	for (auto it_outer = willBeDestroyed_outer.begin(); it_outer < willBeDestroyed_outer.end(); ++it_outer)
	{
		bool found = false;
		for (uint32_t inner_idx = 0; !found && inner_idx < inner_initial_size; ++inner_idx)
		{
			auto& it_inner = inner[inner_idx];
			if (it_outer->semantic == it_inner.semantic)
			{
				// Found in the inner list.
				it_inner = mergeAttribute(it_inner, *it_outer);
				found = true;
			}
		}
		if (!found)
		{
			inner.emplace_back(std::move(*it_outer)); // destructive >:D
		}
	}
	inout_inner.assign(inner.begin(), inner.end());
}

inline void calcOffsetsAndStride(AttributeConfiguration& config)
{
	// Assuming up to 32 (!!!) VBO bindings...
	for (auto& layout : config)
	{
		layout.stride = 0;
		for (auto& attrib : layout)
		{
			attrib.offset = static_cast<uint16_t>(layout.stride);
			layout.stride += attrib.width * dataTypeSize(attrib.datatype);
		}
	}
}

inline void createAttributeConfigurations(RenderManager& renderman, std::map<PipelineSet, AttributeConfiguration>& pipeSets,
	std::map<StringHash, AttributeConfiguration*>& pipeToAttribMapping, std::map<assets::Mesh*, AttributeConfiguration*>& meshAttribConfig, bool datatypesFromModel)
{
	RendermanStructure& renderstruct = renderman.renderObjects();
	for (auto&& renderman_effect : renderstruct.effects)
	{
		auto&& effect = renderman_effect.effect;
		// A pipeline combination
		for (auto pipeset = pipeSets.begin(); pipeset != pipeSets.end(); ++pipeset)
		{
			// A pipeline combination
			AttributeConfiguration& finalLayout = pipeset->second; // DO NOT CLEAR! It may already contain attributes from another run...

			for (auto pipe = pipeset->first.pipelines.begin(); pipe != pipeset->first.pipelines.end(); ++pipe)
			{
				auto pipe2bindings = getVertexBindingsForPipeNoStride(effect, *pipe);

				for (uint32_t binding = 0; binding < pipe2bindings.size(); ++binding)
					if (pipe2bindings.size() >= binding)
					{
						if (binding >= finalLayout.size()) { finalLayout.resize(static_cast<uint32_t>(binding) + 1); }

						mergeAttributeLayouts(finalLayout[binding], pipe2bindings[binding]);
					}

				// Make sure the pipeline knows where to find its attribute
				pipeToAttribMapping[*pipe] = &pipeset->second;
			}
		}

		if (datatypesFromModel)
		{
			// Now, fix the attribute configurations by selecting the widest datatype provided by any of the models.
			auto& apiModels = renderman.renderModels();

			for (uint32_t model_id = 0; model_id < apiModels.size(); ++model_id)
			{
				for (uint32_t mesh_id = 0; mesh_id < apiModels[model_id].assetModel->getNumMeshes(); ++mesh_id)
				{
					assets::Mesh& mesh = *apiModels[model_id].meshes[mesh_id].assetMesh;
					const auto& found = meshAttribConfig.find(&mesh);
					if (found == meshAttribConfig.end()) { continue; }
					auto& attribConfig = *found->second;

					for (uint32_t binding = 0; binding < attribConfig.size(); ++binding)
					{
						for (uint32_t attribute = 0; attribute < attribConfig[binding].size(); ++attribute)
						{
							auto& attrib = attribConfig[binding][attribute];
							const auto mattrib = mesh.getVertexAttributeByName(attrib.semantic);

							if (mattrib == NULL) { continue; }
							else
							{
								fixVertexLayoutDatatypes(attrib, *mattrib);
							}
						}
					}
				}
			}
		}

		// ALL DONE - Fix the offsets and strides...
		for (auto pipeset = pipeSets.begin(); pipeset != pipeSets.end(); ++pipeset)
		{
			AttributeConfiguration& finalLayout = pipeset->second; // DO NOT CLEAR! It may already contain attributes from another run...
			for (auto binding = finalLayout.begin(); binding != finalLayout.end(); ++binding)
			{
				binding->stride = 0;
				for (auto vertex = binding->begin(); vertex != binding->end(); ++vertex)
				{
					vertex->offset = static_cast<uint16_t>(binding->stride);
					binding->stride += dataTypeSize(vertex->datatype) * vertex->width;
				}
			}
		}
	}
}
} // namespace

////////// SEMANTICS - BUFFER ENTRIES - UNIFORMS ////////////
namespace {
inline void addSemanticLists(
	RendermanBufferBinding& buff, std::map<StringHash, StructuredBufferView*>& bufferDefinitions, std::map<StringHash, BufferEntrySemantic>& bufferEntries, bool checkDuplicates)
{
	if (!buff.semantic.empty())
	{
		auto it = bufferDefinitions.find(buff.semantic);
		if (checkDuplicates && it != bufferDefinitions.end())
		{
			debug_assertion(false,
				strings::createFormatted("DUPLICATE BUFFER SEMANTIC DETECTED: Buff: [%s] Semantic [%s]", buff.bufferDefinition->name.c_str(), buff.semantic.c_str()).c_str());
		}
		if (it == bufferDefinitions.end()) { bufferDefinitions[buff.semantic] = &buff.bufferDefinition->structuredBufferView; }
	}
	auto& structuredBufferView = buff.bufferDefinition->structuredBufferView;
	uint32_t numElements = buff.bufferDefinition->memoryDescription.getNumChildren();

	for (uint32_t i = 0; i < numElements; ++i)
	{
		const StructuredMemoryDescription& memDesc = buff.bufferDefinition->memoryDescription.getElement(i);
		if (memDesc.getName().empty()) { continue; }
		auto it = bufferEntries.find(memDesc.getName());
		if (checkDuplicates && it != bufferEntries.end())
		{
			debug_assertion(false,
				strings::createFormatted("DUPLICATE BUFFER ENTRY "
										 "SEMANTIC DETECTED: Buff: [%s] Entry Semantic[%s]",
					buff.bufferDefinition->name.c_str(), it->first.c_str()));
		}
		// add the buffer entry if not exist
		if (it == bufferEntries.end())
		{
			auto& newSemanticEntry = bufferEntries[memDesc.getName()];
			newSemanticEntry.buffer = &buff.bufferDefinition->buffer;
			newSemanticEntry.entryIndex = static_cast<uint16_t>(i);
			newSemanticEntry.setId = buff.set;
			newSemanticEntry.structuredBufferView = &structuredBufferView;
		}
	}
}

inline void addUniformSemanticLists(std::map<StringHash, effectvk::UniformSemantic>& effectlist, std::map<StringHash, UniformSemantic>& newlist, bool checkDuplicates, VariableScope scope)
{
	for (auto& uniform : effectlist)
	{
		if (uniform.second.scope == scope)
		{
			auto& newUniform = newlist[uniform.first];
			newUniform.uniformLocation = uniform.second.arrayElements;
			newUniform.variablename = uniform.second.variableName;
			newUniform.memory.allocate(uniform.second.dataType, uniform.second.arrayElements);
		}
	}
}
} // namespace

/////////  PIPELINES /////////////
#pragma warning TODO_MAKE_DIFFERENT_PIPE_BASED_ON_PRIMITIVE_TOPOLOGY

inline void createPipelines(RenderManager& renderman, const std::map<StringHash, AttributeConfiguration*>& vertexConfigs)
{
	std::map<StringHash, GraphicsPipeline> pipelineApis;

	RendermanStructure& renderstruct = renderman.renderObjects();
	for (auto&& renderman_effect : renderstruct.effects)
	{
		auto&& effect = renderman_effect.effect;
		// Here we fix the input assembly based on the collected data.
		for (auto pipeline = vertexConfigs.begin(); pipeline != vertexConfigs.end(); ++pipeline)
		{
			// per-pipe config
			auto pipedef = effect->getPipelineDefinition(pipeline->first);
			if (pipedef == nullptr) { continue; }
			// COPY
			GraphicsPipelineCreateInfo pipecp = pipedef->createParam;

			// Each VBO
			auto& attributeConfig = *pipeline->second;
			if (pipedef->attributes.size())
			{
				for (uint16_t binding = 0; binding < attributeConfig.size(); ++binding)
				{
					auto& vbo = attributeConfig[binding];
					if (vbo.size() == 0) { continue; } // Empty VBO binding
					const VertexInputBindingDescription* inputBindingInfo = pipecp.vertexInput.getInputBinding(binding);

					pipecp.vertexInput.addInputBinding(
						VertexInputBindingDescription(binding, vbo.stride, (inputBindingInfo ? inputBindingInfo->getInputRate() : pvrvk::VertexInputRate::e_VERTEX)));
					for (uint16_t vertexId = 0; vertexId < vbo.size(); ++vertexId)
					{
						for (uint32_t i = 0; i < pipedef->attributes.size(); ++i) // make sure it matches with the pipeline attribute
						{
							auto& vertex = vbo[vertexId];
							if (pipedef->attributes[i].semantic == vertex.semantic && vertexId == pipedef->attributes[i].location && binding == pipedef->attributes[i].vboBinding)
							{
								pipecp.vertexInput.addInputAttribute(VertexInputAttributeDescription(
									vertexId, binding, convertToPVRVkVertexInputFormat(vertex.datatype, static_cast<uint8_t>(vertex.width)), vertex.offset));
							}
						}
					}
				}
			}

			// add viewport and scissor if not provided.
			if (pipecp.viewport.getNumViewportScissors() == 0)
			{
				const pvrvk::Extent2D& screendDim = renderman.getSwapchain()->getDimension();
				pipecp.viewport.setViewportAndScissor(0, Viewport(0, 0, static_cast<float>(screendDim.getWidth()), static_cast<float>(screendDim.getHeight())),
					pvrvk::Rect2D(pvrvk::Offset2D(0, 0), pvrvk::Extent2D(screendDim.getWidth(), screendDim.getHeight())));
			}
			pvrvk::Device device = effect->getDevice().lock();
			GraphicsPipeline pipelineApi = device->createGraphicsPipeline(pipecp, effect->getPipelineCache());

			assertion(pipelineApis.find(pipeline->first) == pipelineApis.end() || !pipelineApis.find(pipeline->first)->second);

			pipelineApis[pipeline->first] = pipelineApi;
		}
	}

	// Map the newly created pipelines to the Rendering Structure
	// Now that the pipelines are created, we can set the Uniform Locations

	for (auto&& renderman_effect : renderstruct.effects)
	{
		for (auto&& pass_effect : renderman_effect.passes)
		{
			for (auto&& subpass_effect : pass_effect.subpasses)
			{
				for (auto&& subpassGroup_effect : subpass_effect.groups)
				{
					for (auto&& pipeline_effect : subpassGroup_effect.pipelines) { pipeline_effect.apiPipeline = pipelineApis.find(pipeline_effect.name)->second; }
				}
			}
		}
	}
}

inline void fixDynamicOffsets(RenderManager& renderman)
{
	for (auto& effect : renderman.renderObjects().effects)
	{
		uint32_t passId = 0;
		for (auto& pass : effect.passes)
		{
#if defined(PVR_RENDERMANAGER_DEBUG_BUFFERS) || defined(PVR_RENDERMANAGER_DEBUG_RENDERING_COMMANDS)
			Log(LogLevel::Information, "Pass Id: %d", passId);
#endif
			uint32_t subpassId = 0;
			for (auto& subpass : pass.subpasses)
			{
#if defined(PVR_RENDERMANAGER_DEBUG_BUFFERS) || defined(PVR_RENDERMANAGER_DEBUG_RENDERING_COMMANDS)
				Log(LogLevel::Information, "\tSubpass Id: %d", subpassId);
#endif
				uint32_t subpassGroupId = 0;
				for (auto& subpassGroup : subpass.groups)
				{
#if defined(PVR_RENDERMANAGER_DEBUG_BUFFERS) || defined(PVR_RENDERMANAGER_DEBUG_RENDERING_COMMANDS)
					Log(LogLevel::Information, "\t\tSubpassGroup Id: %d", subpassGroupId);
#endif
					uint32_t modelEffectId = 0;
					for (auto& subpassGroupModels : subpassGroup.subpassGroupModels)
					{
#if defined(PVR_RENDERMANAGER_DEBUG_BUFFERS) || defined(PVR_RENDERMANAGER_DEBUG_RENDERING_COMMANDS)
						Log(LogLevel::Information, "\t\t\tSubpassGroupModelId Id: %d", modelEffectId);
#endif
						uint32_t nodeId = 0;
						for (auto& node : subpassGroupModels.nodes)
						{
#if defined(PVR_RENDERMANAGER_DEBUG_BUFFERS) || defined(PVR_RENDERMANAGER_DEBUG_RENDERING_COMMANDS)
							Log(LogLevel::Information, "\t\t\t\tNode Id: %d, Pipeline: %s", nodeId, node.pipelineMaterial_->pipeline_->name.c_str());
#endif
							for (uint32_t setid = 0; setid < FrameworkCaps::MaxDescriptorSetBindings; ++setid)
							{
#if defined(PVR_RENDERMANAGER_DEBUG_BUFFERS) || defined(PVR_RENDERMANAGER_DEBUG_RENDERING_COMMANDS)
								Log(LogLevel::Information, "\t\t\t\t\tSetId Id: %d", setid);
#endif
								for (uint32_t dynamicClient = 0; dynamicClient < node.dynamicClientId[setid].size(); ++dynamicClient) // "dynamic client" is basically the buffer index.
								{
#if defined(PVR_RENDERMANAGER_DEBUG_BUFFERS) || defined(PVR_RENDERMANAGER_DEBUG_RENDERING_COMMANDS)
									Log(LogLevel::Information, "\t\t\t\t\t\tDynamicClient(bufferId): %d", dynamicClient);
#endif
									// IGNORE THESE CHANGES WHEN SUBMITTING
									RendermanBufferDefinition* bufferDef = node.dynamicBuffer[setid][dynamicClient];
									const uint32_t dynamicClientId = node.dynamicClientId[setid][dynamicClient];
#if defined(PVR_RENDERMANAGER_DEBUG_BUFFERS) || defined(PVR_RENDERMANAGER_DEBUG_RENDERING_COMMANDS)
									Log(LogLevel::Information, "\t\t\t\t\t\t\tDynamic client id: %d", dynamicClientId);
#endif
									for (uint32_t swapId = 0; swapId < pvrvk::FrameworkCaps::MaxSwapChains; ++swapId)
									{
#if defined(PVR_RENDERMANAGER_DEBUG_BUFFERS) || defined(PVR_RENDERMANAGER_DEBUG_RENDERING_COMMANDS)
										Log(LogLevel::Information, "\t\t\t\t\t\t\tswapId: %d", swapId);
#endif
										const uint32_t dynamicSliceId = ((bufferDef->numMultiBuffers > 1) ? swapId : 0) * bufferDef->numDynamicClients + dynamicClientId;
										// What is the dynamic offset that should be used by this node
										const uint32_t offset =
											static_cast<uint32_t>(node.dynamicBuffer[setid][dynamicClient]->structuredBufferView.getDynamicSliceOffset(dynamicSliceId));
										node.dynamicOffset[setid][swapId][dynamicClient] = offset;
										node.dynamicSliceId[setid][swapId][dynamicClient] = dynamicSliceId;
#if defined(PVR_RENDERMANAGER_DEBUG_BUFFERS) || defined(PVR_RENDERMANAGER_DEBUG_RENDERING_COMMANDS)
										Log(LogLevel::Information, "\t\t\t\t\t\t\t\tDynamic slice ID: %d", dynamicSliceId);
										Log(LogLevel::Information, "\t\t\t\t\t\t\t\tBuffer: %#010x %s", node.dynamicBuffer[setid][dynamicClient]->buffer->getVkHandle(),
											node.dynamicBuffer[setid][dynamicClient]->name.c_str());
										Log(LogLevel::Information, "\t\t\t\t\t\t\t\tOffset: %d", offset);
#endif
									}
								}
							}
							++nodeId;
						}
						++modelEffectId;
					}
					++subpassGroupId;
				}
				++subpassId;
			}
			++passId;
		}
	}
}

inline bool createBuffers(RenderManager& renderman)
{
	Device device = renderman.getDevice().lock();
	debug_assertion(device != nullptr, "Rendermanager - Invalid Device");
	RendermanStructure& renderstruct = renderman.renderObjects();

	for (auto& renderman_effect : renderstruct.effects)
	{
		for (RendermanBufferDefinition& bufdef : renderman_effect.bufferDefinitions) // The buffer is created per model.
		{
			// Skip if another pipeline has created them
			debug_assertion(bufdef.memoryDescription.getNumChildren() != 0,
				strings::createFormatted("RenderManager::createAll() "
										 "Creating descriptor sets : Buffer entry list for buffer"
										 " [%d] was empty",
					bufdef.name.c_str()));

			bufdef.structuredBufferView.initDynamic(bufdef.memoryDescription, bufdef.numMultiBuffers * std::max<uint32_t>(1, bufdef.numDynamicClients), bufdef.allSupportedBindings,
				static_cast<uint32_t>(device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()),
				static_cast<uint32_t>(device->getPhysicalDevice()->getProperties().getLimits().getMinStorageBufferOffsetAlignment()));

#ifdef PVR_RENDERMANAGER_DEBUG_BUFFERS
			Log(LogLevel::Information, "*** StructureMemView init:");
			Log(LogLevel::Information, "\tname %s", bufdef.name.c_str());
			Log(LogLevel::Information, "\tnumMultiBuffer %d", bufdef.numMultiBuffers);
			Log(LogLevel::Information, "\tnumDynamicClients %d", bufdef.numDynamicClients);
			Log(LogLevel::Information, "\tbuffer size %d", bufdef.structuredBufferView.getSize());
#endif
			bufdef.buffer = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(bufdef.structuredBufferView.getSize(), pvr::utils::convertToPVRVk(bufdef.allSupportedBindings)),
				pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
				pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
				renderman.getAllocator(), pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

			bufdef.structuredBufferView.pointToMappedMemory(bufdef.buffer->getDeviceMemory()->getMappedData());
		}
	}
	fixDynamicOffsets(renderman);
	return true;
}

inline void createDescriptorSets(
	RenderManager& renderman, const std::map<assets::Mesh*, AttributeConfiguration*>& meshAttribConfig, DescriptorPool& pool, uint32_t swapchainLength, CommandBuffer cmdBuffer)
{
	(void)meshAttribConfig;
	Device device = renderman.getDevice().lock();
	debug_assertion(device != nullptr, "Rendermanager - Invalid Device");
	RendermanStructure& renderstruct = renderman.renderObjects();
	std::vector<pvrvk::WriteDescriptorSet> descSetWrites;
	for (auto& renderman_effect : renderstruct.effects)
	{
		for (auto& pass : renderman_effect.passes)
		{
			for (auto& subpass : pass.subpasses)
			{
				for (auto& subpassGroup : subpass.groups)
				{
					// ROUND 2: Create and update the descriptor sets.
					for (auto& modeleffect : subpassGroup.subpassGroupModels)
					{
						for (auto& materialeffect : modeleffect.materialEffects)
						{
							for (auto& materialpipeline : materialeffect.materialSubpassPipelines)
							{
								if (materialpipeline.pipeline_ == nullptr || !materialpipeline.pipeline_->apiPipeline) { continue; }
								auto& pipeline = *materialpipeline.pipeline_;
								auto& pipelayout = pipeline.apiPipeline->getPipelineLayout();
								auto& pipedef = *pipeline.pipelineInfo;
								int16_t set_max = -1;
								for (auto& item : pipedef.textureSamplersByTexName) { set_max = std::max(static_cast<int16_t>(item.second.set), set_max); }
								for (auto& item : pipedef.textureSamplersByTexSemantic) { set_max = std::max(static_cast<int16_t>(item.second.set), set_max); }
								for (auto& item : pipedef.modelScopeBuffers) { set_max = std::max(static_cast<int16_t>(item.second.set), set_max); }
								for (auto& item : pipedef.effectScopeBuffers) { set_max = std::max(static_cast<int16_t>(item.second.set), set_max); }
								for (auto& item : pipedef.nodeScopeBuffers) { set_max = std::max(static_cast<int16_t>(item.second.set), set_max); }
								for (auto& item : pipedef.inputAttachments[0]) { set_max = std::max(static_cast<int16_t>(item.second.set), set_max); }
								for (uint16_t set_id = 0; set_id < set_max + 1; ++set_id)
								{
									if (pipedef.descSetExists[set_id])
									{
										if (pipedef.descSetIsFixed[set_id]) // Descriptor set is "fixed", meaning it is shared by all renderables in the pipeline.
										{ materialpipeline.sets[set_id] = pipedef.fixedDescSet[set_id]; }
										else // Otherwise, create the descriptor set for this pipeline/material combination
										{
											const DescriptorSetLayout& descsetlayout = pipelayout->getDescriptorSetLayout(set_id);
											debug_assertion(descsetlayout != nullptr,
												strings::createFormatted("RenderManager::createAll() Creating descriptor sets: Descriptor set layout was referenced but was NULL: "
																		 "Pipeline[%s] Set[%d].",
													pipeline.name.c_str(), set_id));

											uint32_t swaplength = (pipedef.descSetIsMultibuffered[set_id] ? swapchainLength : 1);
											for (uint32_t swapchain = 0; swapchain < swaplength; ++swapchain)
											{ materialpipeline.sets[set_id][swapchain] = pool->allocateDescriptorSet(descsetlayout); }
										}
									}
								}

								// POPULATE THE INPUT  ATTCHMENTS
								for (uint32_t swapindex = 0; swapindex < swapchainLength; ++swapindex)
								{
									for (auto& inputEntry : pipedef.inputAttachments[swapindex])
									{
										const effectvk::InputAttachmentInfo& input = inputEntry.second;
										descSetWrites.emplace_back(
											pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_INPUT_ATTACHMENT, materialpipeline.sets[input.set][swapindex], input.binding)
												.setImageInfo(0, pvrvk::DescriptorImageInfo(input.tex, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
									}
								}

								// POPULATE THE TEXTURES
								for (auto& tex : pipedef.textureSamplersByTexSemantic)
								{
									// Get texture name by texture semantic
									// Load the texture -> caching/unduplication
									// Add it to the set.
									// IGNORE UNDUPLICATION FOR NOW
									if (pipedef.descSetIsFixed[tex.second.set])
									{
										debug_assertion(!pipedef.descSetIsFixed[tex.second.set],
											strings::createFormatted("RenderManager::createAll() Creating descriptor sets: Descriptor set "
																	 "was referenced but a semantic, but was marked FIXED"
																	 " Set[ % d] TextureSemantic[ % s].",
												pipeline.name.c_str(), tex.second.set, tex.first.c_str()));
									}
									int32_t texIndex;
									if ((texIndex = materialeffect.material->assetMaterial->getTextureIndex(tex.first)) != -1)
									{
										const StringHash& texturePath = modeleffect.renderModel_->assetModel->getTexture(texIndex).getName();
										auto imgView = utils::loadAndUploadImageAndView(device, texturePath.c_str(), true, cmdBuffer, renderman.getAssetProvider(),
											pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, renderman.getAllocator(),
											renderman.getAllocator());

										uint32_t swaplength = pipedef.descSetIsMultibuffered[tex.second.set] ? swapchainLength : 1;

										for (uint32_t swapindex = 0; swapindex < swaplength; ++swapindex)
										{
											descSetWrites.emplace_back(
												pvrvk::WriteDescriptorSet(
													pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, materialpipeline.sets[tex.second.set][swapindex], tex.second.binding)
													.setImageInfo(0, pvrvk::DescriptorImageInfo(imgView, pipedef.textureSamplersByTexSemantic.find(tex.first)->second.sampler)));
										}
									}
									else
									{
										Log(LogLevel::Information,
											"RenderManager: Texture semantic [%s] was not found in model material [%s]. "
											"The texture will need to be populated by the application",
											tex.first.c_str(), materialeffect.material->assetMaterial->getName().c_str());
#pragma warning populate_non_automatic_semantics
									}
								}

								// POPULATE THE BUFFERS
								for (auto& bufentry : pipeline.bufferBindings)
								{
									auto& buf = bufentry.second;
									auto& bufdef = *buf.bufferDefinition;
									if (bufdef.structuredBufferView.getNumElements() == 0)
									{
										assertion(false,
											strings::createFormatted("RenderManager::createAll() Creating descriptor sets : Buffer entry list for"
																	 " buffer [%s] was empty",
												bufdef.name.c_str()));
									}
									uint32_t swaplength = pipedef.descSetIsMultibuffered[buf.set] ? swapchainLength : 1;

									for (uint32_t swapindex = 0; swapindex < swaplength; ++swapindex)
									{
										debug_assertion(buf.type >= pvrvk::DescriptorType::e_UNIFORM_BUFFER && buf.type <= pvrvk::DescriptorType::e_STORAGE_BUFFER_DYNAMIC,
											"Invalid buffer type");

										descSetWrites.emplace_back(
											pvrvk::WriteDescriptorSet(buf.type, materialpipeline.sets[buf.set][swapindex], buf.binding)
												.setBufferInfo(0, pvrvk::DescriptorBufferInfo(bufdef.buffer, 0, bufdef.structuredBufferView.getDynamicSliceSize())));
									}
								}
							}
						}
					}
				}
			}
		}
	}
	device->updateDescriptorSets(descSetWrites.data(), static_cast<uint32_t>(descSetWrites.size()), nullptr, 0);
}

/////// FUNCTIONS TO ADD OBJECTS TO THE RENDER DATA STRUCTURE /////////
namespace {
inline RendermanBufferBinding makeRendermanBufferBinding(const StringHash& name, const effectvk::BufferRef& ref, std::deque<RendermanBufferDefinition>& definitions)
{
	RendermanBufferBinding bufferbinding;
	bufferbinding.binding = ref.binding;
	bufferbinding.set = ref.set;
	bufferbinding.type = convertToPVRVk(ref.type);
	bufferbinding.semantic = ref.semantic;

	struct BufferNameEquals
	{
		const StringHash& name;
		BufferNameEquals(const StringHash& name) : name(name) {}
		bool operator()(const RendermanBufferDefinition& rhs) const { return name == rhs.name; }
	};

	bufferbinding.bufferDefinition = &*std::find_if(definitions.begin(), definitions.end(), BufferNameEquals(name));
	return bufferbinding;
}

size_t addRendermanPipelineIfNotExists(RendermanSubpassGroup& subpassGroup, effectvk::PipelineDef* pipedef, RendermanEffect& renderEffect, bool& isnew)
{
	// Comparator for the next find
	struct cmppipe
	{
		effectvk::PipelineDef* ptr;
		cmppipe(effectvk::PipelineDef* ptr) : ptr(ptr) {}
		bool operator()(const RendermanPipeline& rhs) { return rhs.pipelineInfo == ptr; }
	};

	auto& container = subpassGroup.pipelines;

	auto found_pipe = std::find_if(container.begin(), container.end(), cmppipe(pipedef));
	size_t pipe_index = 0;
	if (found_pipe != container.end())
	{
		pipe_index = found_pipe - container.begin();
		isnew = false;
	}
	else
	{
		isnew = true;
		pipe_index = container.size();

		container.resize(container.size() + 1);
		RendermanPipeline& newEntry = container.back();
		newEntry.subpassGroup_ = &subpassGroup;
		newEntry.pipelineInfo = pipedef;

		addUniformSemanticLists(pipedef->uniforms, newEntry.uniformSemantics, true, VariableScope::Effect);
		addUniformSemanticLists(pipedef->uniforms, newEntry.uniformSemantics, true, VariableScope::Model);

		// Bomes are in Model Scopebuffers as they are global in .pod.
		for (auto& buffer : pipedef->modelScopeBuffers)
		{
			auto& tmp = newEntry.bufferBindings[buffer.first];
			tmp = makeRendermanBufferBinding(buffer.first, buffer.second, renderEffect.bufferDefinitions);

			addSemanticLists(tmp, newEntry.bufferSemantics, newEntry.bufferEntrySemantics, true);
		}
		for (auto& buffer : pipedef->nodeScopeBuffers)
		{
			auto& tmp = newEntry.bufferBindings[buffer.first];
			tmp = makeRendermanBufferBinding(buffer.first, buffer.second, renderEffect.bufferDefinitions);
			addSemanticLists(tmp, newEntry.bufferSemantics, newEntry.bufferEntrySemantics, true);
		}
		for (auto& buffer : pipedef->batchScopeBuffers)
		{
			auto& tmp = newEntry.bufferBindings[buffer.first];
			tmp = makeRendermanBufferBinding(buffer.first, buffer.second, renderEffect.bufferDefinitions);
			addSemanticLists(tmp, newEntry.bufferSemantics, newEntry.bufferEntrySemantics, true);
		}
		// Effects are added to the model
		for (auto& buffer : pipedef->effectScopeBuffers)
		{
			auto& tmp = newEntry.bufferBindings[buffer.first];
			// if (buffer.second.)
			tmp = makeRendermanBufferBinding(buffer.first, buffer.second, renderEffect.bufferDefinitions);
			// IT is expected to have duplicates, as multiple pipelines may attempt to add the same buffer,
			// in which case we must avoid duplication.
			addSemanticLists(tmp, renderEffect.structuredBufferViewSemantics, renderEffect.bufferEntrySemantics, false);
		}
	}
	return pipe_index;
}

template<typename Container>
inline size_t addRendermanMaterialEffectIfNotExists(Container& model, RendermanMaterial* material, bool& isnew)
{
	// Comparator for the next find
	struct cmppipe
	{
		RendermanMaterial* ptr;
		cmppipe(RendermanMaterial* ptr) : ptr(ptr) {}
		bool operator()(const RendermanSubpassMaterial& rhs) { return rhs.material == ptr; }
	};

	auto found_item = std::find_if(model.materialEffects.begin(), model.materialEffects.end(), cmppipe(material));
	size_t item_index = 0;
	if (found_item != model.materialEffects.end())
	{
		isnew = false;
		item_index = found_item - model.materialEffects.begin();
	}
	else
	{
		isnew = true;
		item_index = model.materialEffects.size();
		RendermanSubpassMaterial newEntry;
		newEntry.material = material;
		newEntry.modelSubpass_ = &model;
		model.materialEffects.emplace_back(newEntry);
	}
	return item_index;
}

template<typename Container>
inline size_t addRendermanMeshEffectIfNotExists(Container& model, RendermanMesh* mesh, bool& isnew)
{
	// Comparator for the next find
	struct cmppipe
	{
		RendermanMesh* ptr;
		cmppipe(RendermanMesh* ptr) : ptr(ptr) {}
		bool operator()(const RendermanSubpassMesh& rhs) { return rhs.rendermesh_ == ptr; }
	};

	auto found_item = std::find_if(model.subpassMeshes.begin(), model.subpassMeshes.end(), cmppipe(mesh));
	size_t item_index = 0;
	if (found_item != model.subpassMeshes.end())
	{
		isnew = false;
		item_index = found_item - model.subpassMeshes.begin();
	}
	else
	{
		isnew = true;
		item_index = model.subpassMeshes.size();
		RendermanSubpassMesh newEntry;
		newEntry.rendermesh_ = mesh;
		newEntry.modelSubpass_ = &model;
		model.subpassMeshes.emplace_back(newEntry);
	}
	return item_index;
}

template<typename Container>
inline size_t addRendermanModelEffectIfNotExists(Container& container, RendermanModel* model, RendermanSubpassGroup* subpassGroup, bool& isnew)
{
	// Comparator for the next find
	struct cmppipe
	{
		RendermanModel* ptr;
		cmppipe(RendermanModel* ptr) : ptr(ptr) {}
		bool operator()(const RendermanSubpassGroupModel& rhs) { return rhs.renderModel_ == ptr; }
	};

	auto found_item = std::find_if(container.begin(), container.end(), cmppipe(model));
	size_t item_index = 0;
	if (found_item != container.end())
	{
		isnew = false;
		item_index = found_item - container.begin();
	}
	else
	{
		isnew = true;
		item_index = container.size();
		container.emplace_back(RendermanSubpassGroupModel());
		container.back().renderModel_ = model;
		container.back().renderSubpassGroup_ = subpassGroup;
	}
	return item_index;
}

inline size_t connectMaterialEffectWithPipeline(RendermanSubpassMaterial& rms, RendermanPipeline& pipe)
{
	struct cmp
	{
		RendermanPipeline* pipe;
		cmp(RendermanPipeline& pipe) : pipe(&pipe) {}
		bool operator()(RendermanMaterialSubpassPipeline& rmpipe) { return rmpipe.pipeline_ == pipe; }
	};
	auto found = std::find_if(rms.materialSubpassPipelines.begin(), rms.materialSubpassPipelines.end(), cmp(pipe));
	size_t pipe_index = found - rms.materialSubpassPipelines.begin();
	if (found == rms.materialSubpassPipelines.end())
	{
		assertion(std::find(pipe.subpassMaterials.begin(), pipe.subpassMaterials.end(), &rms) == pipe.subpassMaterials.end());
		RendermanMaterialSubpassPipeline rmep;
		rmep.pipeline_ = &pipe;
		rmep.materialSubpass_ = &rms;
		pipe.subpassMaterials.emplace_back(&rms);
		rms.materialSubpassPipelines.emplace_back(rmep);
	}
	else
	{
		assertion(std::find(pipe.subpassMaterials.begin(), pipe.subpassMaterials.end(), &rms) != pipe.subpassMaterials.end());
	}
	return pipe_index;
}

inline void addBufferDefinitions(RendermanEffect& renderEffect, effectvk::EffectApi& effect)
{
	for (auto& buffer : effect->getBuffers())
	{
		renderEffect.bufferDefinitions.resize(renderEffect.bufferDefinitions.size() + 1);
		RendermanBufferDefinition& def = renderEffect.bufferDefinitions.back();
		def.allSupportedBindings = buffer.second.allSupportedBindings;
		def.isDynamic = buffer.second.isDynamic;
		def.structuredBufferView = buffer.second.bufferView;
		def.buffer = buffer.second.buffer;
		def.name = buffer.first;
		def.numMultiBuffers = buffer.second.numBuffers;
		def.memoryDescription = buffer.second.memoryDescription;
		def.scope = buffer.second.scope;
	}
}

void addNodeDynamicClientToBuffers(RendermanSubpassGroupModel&, RendermanNode& node, RendermanPipeline& pipeline)
{
	// Model buffer references are sorted by pipeline. No worries there.
	uint32_t current_buffer[FrameworkCaps::MaxDescriptorSetBindings] = { 0, 0, 0, 0 }; // Number of buffers bound for each descriptor set
	for (auto buffer_it = pipeline.bufferBindings.begin(); buffer_it != pipeline.bufferBindings.end(); ++buffer_it)
	{
		auto& buffer = buffer_it->second;
		if ((buffer.bufferDefinition->scope == VariableScope::Node || buffer.bufferDefinition->scope == VariableScope::BoneBatch ||
				buffer.bufferDefinition->scope == VariableScope::Effect) &&
			(buffer.type == pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC || buffer.type == pvrvk::DescriptorType::e_STORAGE_BUFFER_DYNAMIC))
		{ ++current_buffer[buffer.set]; }
	}

	for (uint32_t set = 0; set < FrameworkCaps::MaxDescriptorSetBindings; ++set)
	{
		node.dynamicClientId[set].resize(current_buffer[set]);
		node.dynamicBuffer[set].resize(current_buffer[set]);
		for (uint32_t i = 0; i < pvrvk::FrameworkCaps::MaxSwapChains; ++i)
		{
			node.dynamicOffset[set][i].resize(current_buffer[set]);
			node.dynamicSliceId[set][i].resize(current_buffer[set]);
		}
		current_buffer[set] = 0;
	}

	std::vector<RendermanBufferBinding> sortedBuffer;
	for (auto buffer_it = pipeline.bufferBindings.begin(); buffer_it != pipeline.bufferBindings.end(); ++buffer_it) { sortedBuffer.emplace_back(buffer_it->second); }

	// sort the buffer based on set and binding
	std::sort(sortedBuffer.begin(), sortedBuffer.end(), [&](const RendermanBufferBinding& a, const RendermanBufferBinding& b) {
		if (a.set < b.set || (a.set == b.set && a.binding < b.binding)) { return true; }
		return false;
	});

	for (auto buffer_it = sortedBuffer.begin(); buffer_it != sortedBuffer.end(); ++buffer_it)
	{
		auto& buffer = *buffer_it;
		if ((buffer.type == pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC || buffer.type == pvrvk::DescriptorType::e_STORAGE_BUFFER_DYNAMIC))
		{
			if (buffer.bufferDefinition->scope == VariableScope::Node || buffer.bufferDefinition->scope == VariableScope::BoneBatch)
			{
				uint32_t clientId = 0;
				if (buffer.bufferDefinition->scope == VariableScope::Node) { clientId = buffer.bufferDefinition->numDynamicClients++; }
				else if (buffer.bufferDefinition->scope == VariableScope::BoneBatch)
				{
					clientId = node.batchId;
					buffer.bufferDefinition->numDynamicClients = std::max(node.batchId + 1, buffer.bufferDefinition->numDynamicClients);
				}

				node.dynamicClientId[buffer.set][current_buffer[buffer.set]] = clientId;
				node.dynamicBuffer[buffer.set][current_buffer[buffer.set]] = buffer.bufferDefinition;
				++current_buffer[buffer.set];
			}
			else if (buffer.bufferDefinition->scope == VariableScope::Effect)
			{
				buffer.bufferDefinition->numDynamicClients = 1;

				node.dynamicClientId[buffer.set][current_buffer[buffer.set]] = 0;
				node.dynamicBuffer[buffer.set][current_buffer[buffer.set]] = buffer.bufferDefinition;
				++current_buffer[buffer.set];
			}
			else
			{
				debug_assertion(0, "DYNAMIC BUFFER IS NOT OF NODE, BATCH OR EFFECT SCOPE");
			}
		}
	}
}

inline void prepareDataStructures(RenderManager& renderman, std::map<assets::Mesh*, AttributeConfiguration*>& meshAttributeLayout, std::map<PipelineSet, AttributeConfiguration>& pipeSets)
{
	RendermanStructure& renderStructure = renderman.renderObjects();
	// PREPARE DATA STRUCTURES
	// We need to generate the entire rendering graph.
	// The structure is effect/pass/subpass/pipeline/model/node (with offshoots the materials, meshes etc)

	// Most importantly, we need to keep a list of all pipelines used for each MESH (not Node), so that we
	// can optimize the VBO layout. [pipeSets and meshAttributeLayout objects]

	// Use this in order to determine which pipelines need to use common vertex layouts
	std::map<assets::Mesh*, std::set<StringHash> /**/> setOfAllPipesUsedPerMesh;

	// Fix the size of the renderables
	for (std::deque<RendermanEffect>::iterator effect_it = renderStructure.effects.begin(); effect_it != renderStructure.effects.end(); ++effect_it)
	{
		// PHASE 1: Select combinations
		// a) What are all the distinct combinations of pipelines used? (Use to create the VBO layouts)

		effectvk::EffectApi& effectapi = effect_it->effect;

		addBufferDefinitions(*effect_it, effectapi);
		for (uint8_t passId = 0; passId < effectapi->getNumPasses(); ++passId)
		{
			effectvk::Pass& effectpass = effectapi->getPass(passId);
			RendermanPass& renderingpass = effect_it->passes[passId];
			renderingpass.renderEffect_ = &*effect_it;

			for (uint8_t subpassId = 0; subpassId < effectpass.subpasses.size(); ++subpassId)
			{
				effectvk::Subpass& effectsubpass = effectpass.subpasses[subpassId];
				RendermanSubpass& rendersubpass = renderingpass.subpasses[subpassId];
				rendersubpass.renderingPass_ = &renderingpass; // keep a pointer to the parent
				for (uint8_t subpassGroupid = 0; subpassGroupid < effectsubpass.groups.size(); ++subpassGroupid)
				{
					effectvk::SubpassGroup& effectSubpassGroup = effectsubpass.groups[subpassGroupid];
					RendermanSubpassGroup& renderSubpassGroup = rendersubpass.groups[subpassGroupid];
					renderSubpassGroup.name = effectSubpassGroup.name;
					renderSubpassGroup.subpass_ = &rendersubpass;
					// CREATE THE RENDERNODE BY SELECTING THE PIPELINE FOR EACH MODEL NODE
					for (uint32_t modelId = 0; modelId < renderSubpassGroup.allModels.size(); ++modelId)
					{
						RendermanModel& rendermodel = *renderSubpassGroup.allModels[modelId];

						for (uint32_t nodeId = 0; nodeId < rendermodel.assetModel->getNumMeshNodes(); ++nodeId)
						{
							// There may be more than one models for this pipeline

							auto& assetnode = rendermodel.assetModel->getMeshNode(nodeId);
							auto& assetmesh = rendermodel.assetModel->getMesh(assetnode.getObjectId());
							auto& assetmaterial = rendermodel.assetModel->getMaterial(assetnode.getMaterialIndex());

							std::pair<StringHash, effectvk::PipelineDef*> pipe = selectPipelineForSubpassGroupMeshMaterial(effectapi, effectSubpassGroup, assetmesh, assetmaterial);

							if (!pipe.second) { continue; }
							// Add this pipeline as one used by this mesh. Needed? YES, because meshes are NOT
							// arranged with their pipes - nodes are. We need this so that if a mesh is rendered
							// by different pipes (Think of a scene with a glass sphere and a wood sphere...)
							// we are keeping the attributes needed by all of them for the VBOs.
							setOfAllPipesUsedPerMesh[&assetmesh].insert(pipe.first);

							// Now, finish with the structure - add all items needed.
							// The structure is populated for now, not finalized. The actual API objects are
							// created later. For now, only the connections between objects are necessary.
							bool isNew = true;
							bool isPipeNew = true;

							size_t pipe_index = addRendermanPipelineIfNotExists(renderSubpassGroup, pipe.second, *effect_it, isPipeNew);

							RendermanPipeline& renderpipe = renderSubpassGroup.pipelines[pipe_index];

							renderpipe.name = pipe.first;
							renderpipe.pipelineInfo = pipe.second;

							size_t model_index = addRendermanModelEffectIfNotExists(renderSubpassGroup.subpassGroupModels, &rendermodel, &renderSubpassGroup, isNew);

							RendermanSubpassGroupModel& rendermodeleffect = renderSubpassGroup.subpassGroupModels[model_index];
							RendermanMaterial& rendermaterial = rendermodel.materials[assetnode.getMaterialIndex()];

							RendermanMesh& rendermesh = rendermodel.meshes[assetnode.getObjectId()];

							size_t rendermateffect_index = addRendermanMaterialEffectIfNotExists(rendermodeleffect, &rendermaterial, isNew);
							RendermanSubpassMaterial& rendermaterialeffect = rendermodeleffect.materialEffects[rendermateffect_index];

							size_t rendermateffectpipe_index = connectMaterialEffectWithPipeline(rendermaterialeffect, renderpipe);
							RendermanMaterialSubpassPipeline& rendermaterialeffectpipe = rendermaterialeffect.materialSubpassPipelines[rendermateffectpipe_index];

							size_t rendermesheffect_index = addRendermanMeshEffectIfNotExists(rendermodeleffect, &rendermesh, isNew);
							RendermanSubpassMesh& rendermesheffect = rendermodeleffect.subpassMeshes[rendermesheffect_index];

							// keep a pointer to the pipeline used by this mesh.
							rendermesheffect.usedByPipelines.insert(&renderpipe);

							for (uint32_t batch_id = 0; batch_id < std::max(assetmesh.getNumBoneBatches(), 1u); ++batch_id)
							{
								rendermodeleffect.nodes.emplace_back(RendermanNode());

								RendermanNode& node = rendermodeleffect.nodes.back();
								node.assetNode = assets::getNodeHandle(rendermodel.assetModel, nodeId);
								node.assetNodeId = nodeId;
								node.pipelineMaterial_ = &rendermaterialeffectpipe;
								node.subpassMesh_ = &rendermesheffect;
								node.batchId = batch_id;

								addNodeDynamicClientToBuffers(rendermodeleffect, node, renderpipe);
								addUniformSemanticLists(pipe.second->uniforms, node.uniformSemantics, true, VariableScope::Node);
								addUniformSemanticLists(pipe.second->uniforms, node.uniformSemantics, true, VariableScope::BoneBatch);
							}
						}
					}
				}
			}
		}
	}
	createBuffers(renderman);

	// CAUTION: Attribute configurations are NOT created yet.
	// Pipesets exists to remove the duplication that will happen due to the mapping.
	// So, using the "pipeSet" as a key, we will check each and every pipe combination to ensure
	// that no duplicates remain, and then we map based on the pointer to the mesh.
	// In case you are wondering, the second part either inserts or ignores the key (i.e. the set
	// of pipelines we are looking for), and returns the address to it in the set. So we remove
	// all duplication this way.
	for (auto it = setOfAllPipesUsedPerMesh.begin(); it != setOfAllPipesUsedPerMesh.end(); ++it) { meshAttributeLayout[it->first] = &pipeSets[it->second]; }
}
} // namespace

//////////////// SEMANTICS //////////////// SEMANTICS //////////////// SEMANTICS ////////////////
namespace {

static const StringHash VIEWMATRIX_STR("VIEWMATRIX");
static const StringHash VIEWPROJECTIONMATRIX_STR("VIEWPROJECTIONMATRIX");

// clang-format off
#define CAMERA(idxchar, idx) \
    case HashCompileTime<'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'A', 'T', 'R', 'I', 'X', idxchar>::value:\
    case HashCompileTime<'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'T', 'X', idxchar>::value:\
    case HashCompileTime<'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', idxchar>::value:\
    case HashCompileTime<'P', 'E', 'R', 'S', 'P', 'E', 'C', 'T', 'I', 'V', 'E', 'M', 'A', 'T', 'R', 'I', 'X', idxchar>::value:\
    case HashCompileTime<'P', 'E', 'R', 'S', 'P', 'E', 'C', 'T', 'I', 'V', 'E', 'M', 'T', 'X', idxchar>::value:\
    case HashCompileTime<'P', 'E', 'R', 'S', 'P', 'E', 'C', 'T', 'I', 'V', 'E', idxchar>::value:\
{ return &getPerspectiveMatrix##idx; } break;\
    case HashCompileTime<'V', 'I', 'E', 'W', 'M', 'A', 'T', 'R', 'I', 'X', idxchar>::value:\
    case HashCompileTime<'V', 'I', 'E', 'W', 'M', 'T', 'X', idxchar>::value:\
    case HashCompileTime<'V', 'I', 'E', 'W', idxchar>::value:\
{ return &getViewMatrix##idx; } break;\
    case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'A', 'T', 'R', 'I', 'X', idxchar>::value:\
    case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'T', 'X', idxchar>::value:\
    case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', idxchar>::value:\
    case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'M', 'A', 'T', 'R', 'I', 'X', idxchar>::value:\
    case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'M', 'T', 'X', idxchar>::value:\
    case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', idxchar>::value:\
    case HashCompileTime<'V', 'P', 'M', 'A', 'T', 'R', 'I', 'X', idxchar>::value:\
{ return &getViewProjectionMatrix##idx; } break;\

#define LIGHT0_9(idxchar,idx) \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'P', 'O', 'S', 'I', 'T', 'I', 'O', 'N', idxchar>::value: \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'P', 'O', 'S', idxchar>::value: { return getLightPosition##idx; } break; \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'D', 'I', 'R', 'E', 'C', 'T', 'I', 'O', 'N', idxchar>::value: \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'D', 'I', 'R', idxchar>::value: { return getLightDirection##idx; } break; \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'C', 'O', 'L', 'O', 'R', idxchar>::value: \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'C', 'O', 'L', 'O', 'U', 'R', idxchar>::value: {return getLightColor##idx; }break; \

#define LIGHT10_99(idxchar0,idxchar1,idx) \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'P', 'O', 'S', 'I', 'T', 'I', 'O', 'N', idxchar0, idxchar1>::value: \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'P', 'O', 'S', idxchar0, idxchar1>::value: return getLightPosition##idx; break; \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'D', 'I', 'R', 'E', 'C', 'T', 'I', 'O', 'N', idxchar0, idxchar1>::value: \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'D', 'I', 'R', idxchar0, idxchar1>::value: return getLightDirection##idx; break; \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'C', 'O', 'L', 'O', 'R', idxchar0, idxchar1>::value: \
    case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'C', 'O', 'L', 'O', 'U', 'R', idxchar0, idxchar1>::value: return getLightColor##idx; break; \

#define BONEMTX0_9(idxchar, idx) \
    case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X', idxchar>::value:\
    case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'T', 'X', idxchar>::value:\
    case HashCompileTime<'B', 'O', 'N', 'E', idxchar>::value: return &getBoneMatrix##idx; break;\
    case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X', 'I', 'T', idxchar> ::value:\
    case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'T', 'X', 'I', 'T', idxchar> ::value:\
    case HashCompileTime<'B', 'O', 'N', 'E', 'I', 'T', idxchar> ::value: return &getBoneMatrixIT##idx;break;\

#define BONEMTX10_99(idxchar0,idxchar1, idx) \
    case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X', idxchar0, idxchar1>::value: \
    case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'T', 'X', idxchar0, idxchar1>::value: \
    case HashCompileTime<'B', 'O', 'N', 'E', idxchar0, idxchar1>::value: return &getBoneMatrix##idx; break;\
    case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X', 'I', 'T', idxchar0, idxchar1> ::value: \
    case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'T', 'X', 'I', 'T', idxchar0, idxchar1> ::value: \
    case HashCompileTime<'B', 'O', 'N', 'E', 'I', 'T', idxchar0, idxchar1> ::value: return &getBoneMatrixIT##idx;break;\
// clang-format on
}

//////////// FUNCTIONS THAT WILL READ SEMANTIC INFORMATION FROM THE MODEL ///////
/////////   (NodeSemanticSetters / ModelSemanticSetters ////////////
namespace {

inline bool getPerspectiveMatrix(
  TypedMem& memory, const RendermanModel& rmodel, uint32_t cameraIndex)
{
	const assets::Model& model = *rmodel.assetModel;
	const auto screenDim = rmodel.backToRenderManager().getSwapchain()->getDimension();
	if (model.getNumCameraNodes() <= cameraIndex) { return false; }
	glm::vec3 from, to, up(0.0f, 1.0f, 0.0f);
	float fov, nearClip, farClip;
	model.getCameraProperties(cameraIndex, fov, from, to, up, nearClip, farClip); // vTo is calculated from the rotation
	memory.setValue(pvr::math::perspectiveFov(Api::Vulkan, fov,
	                static_cast<float>(screenDim.getWidth()), static_cast<float>(screenDim.getHeight()), nearClip, farClip));
	return true;
}

inline bool getViewMatrix(
  TypedMem& memory, const RendermanModel& rmodel, uint32_t cameraIndex)
{
	const assets::Model& model = *rmodel.assetModel;

	if (model.getNumCameraNodes() <= cameraIndex) { return false; }
	glm::vec3 from, to, up(0.0f, 1.0f, 0.0f);
	float fov, nearClip, farClip;

	model.getCameraProperties(cameraIndex, fov, from, to, up, nearClip, farClip); // vTo is calculated from the rotation

	//We build the model view matrix from the camera position, target and an up vector.
	//For this we use glm::lookAt().
	memory.setValue(glm::lookAt(from, to, up));
	return true;
}
inline bool getViewProjectionMatrix(
  TypedMem&  memory, const RendermanModel& rmodel, uint32_t cameraIndex)
{
	const assets::Model& model = *rmodel.assetModel;
	const auto screenDim = rmodel.backToRenderManager().getSwapchain()->getDimension();
	if (model.getNumCameraNodes() <= cameraIndex) { return false; }
	glm::vec3 from, to, up(0.0f, 1.0f, 0.0f);
	glm::mat4x4 view;
	float fov, nearClip, farClip;

	model.getCameraProperties(cameraIndex, fov, from, to, up, nearClip, farClip); // vTo is calculated from the rotation

	//We build the model view matrix from the camera position, target and an up vector.
	//For this we use glm::lookAt().
	view = glm::lookAt(from, to, up);

	// Set up the View * Projection Matrix
	bool isRotated = screenDim.getHeight() > screenDim.getWidth();
	if (isRotated)
	{
		memory.setValue(
		  pvr::math::perspective(Api::Vulkan, fov,
		                         static_cast<float>(screenDim.getHeight()) / screenDim.getWidth(), nearClip, farClip,
		                         glm::pi<float>() * .5f) * view);
	}
	else
	{
		memory.setValue(
		  pvr::math::perspective(Api::Vulkan, fov,
		                         static_cast<float>(screenDim.getWidth()) / screenDim.getHeight(), nearClip, farClip) * view);
	}

	return true;
}

inline bool getLightPosition(TypedMem& mem,
                             const RendermanModel& rmodel, int32_t lightNodeId)
{
	const assets::Model& model = *rmodel.assetModel;

	if (model.getNumLightNodes() <= static_cast<uint32_t>(lightNodeId)) { return false; }
	mem.setValue(model.getLightPosition(lightNodeId));
	return true;
}

inline bool getLightDirection(TypedMem& mem,
                              const RendermanModel& rmodel, int32_t lightNodeId)
{
	const assets::Model& model = *rmodel.assetModel;

	if (model.getNumLightNodes() <= static_cast<uint32_t>(lightNodeId)) { return false; }
	mem.allocate(GpuDatatypes::vec3);
	model.getLightDirection(lightNodeId, mem.interpretValueAs<glm::vec3>());
	return true;
}

inline bool getLightColor(TypedMem& mem,
                           const RendermanModel& rmodel, int32_t lightNodeId)
{
	const assets::Model& model = *rmodel.assetModel;

	if (model.getNumLightNodes() <= static_cast<uint32_t>(lightNodeId)) { return false; }
	mem.setValue(model.getLight(lightNodeId).getColor());
	return true;
}

inline bool getBoneMatrix(TypedMem& mem, const RendermanNode& node, uint32_t boneid)
{
	auto& rmesh = node.toRendermanMesh();
	auto& assetmesh = *rmesh.assetMesh;

    int32_t skeletonId = assetmesh.getSkeletonId();
    debug_assertion(assetmesh.getMeshInfo().isSkinned && skeletonId >= 0, "Must be skinned mesh");
  
   const pvr::assets::Skeleton& skeleton =   rmesh.backToRendermanModel().assetModel->getSkeleton(skeletonId);
	debug_assertion((skeleton.bones.size() > node.batchId && assetmesh.getNumBones() > boneid), "OUT OF BOUNDS");
    
    node.toRendermanMesh().backToRendermanModel().assetModel->getSkeleton(skeletonId); 
    mem.setValue(rmesh.renderModel_->assetModel->getBoneWorldMatrix(rmesh.assetMeshId, boneid));
	return true;
}

inline bool getBoneMatrixIT(TypedMem& mem, const RendermanNode& node, uint32_t boneid)
{
	auto& rmesh = node.toRendermanMesh();
	auto& assetmesh = *rmesh.assetMesh;
    int32_t skeletonId = assetmesh.getSkeletonId();
    debug_assertion(assetmesh.getMeshInfo().isSkinned &&  skeletonId >= 0, "Must be skinned mesh");
	mem.setValue(glm::inverseTranspose(glm::mat3(rmesh.renderModel_->assetModel->getBoneWorldMatrix(rmesh.assetMeshId, boneid))));
	return true;
}

inline bool getBoneMatrices(TypedMem& mem, const RendermanNode& node)
{
	auto& rmesh = node.toRendermanMesh();
	auto& assetmesh = *rmesh.assetMesh;
    int32_t skeletonId = assetmesh.getSkeletonId();

    debug_assertion(assetmesh.getMeshInfo().isSkinned && skeletonId >= 0, "Must be skinned mesh");
   const  pvr::assets::Skeleton& skeleton = rmesh.backToRendermanModel().assetModel->getSkeleton(assetmesh.getSkeletonId());    
	mem.allocate(GpuDatatypes::mat4x4, static_cast<uint32_t>(skeleton.bones.size()));
	debug_assertion(assetmesh.getMeshInfo().isSkinned && (assetmesh.getNumBoneBatches() > node.batchId), "OUT OF BOUNDS");
	for (uint32_t boneid = mem.arrayElements(); boneid > 0;/*done in the loop*/)
	{
		--boneid;
		mem.setValue(rmesh.renderModel_->assetModel->getBoneWorldMatrix(rmesh.assetMeshId, boneid), boneid);
	}
	return true;
}

inline bool getBoneMatricesIT(TypedMem& mem, const RendermanNode& node)
{
	auto& rmesh = node.toRendermanMesh();
	auto& assetmesh = *rmesh.assetMesh;
    int32_t skeletonId = assetmesh.getSkeletonId();
    
    debug_assertion(assetmesh.getMeshInfo().isSkinned && skeletonId >= 0, "Must be skinned mesh");
    const  pvr::assets::Skeleton& skeleton = rmesh.backToRendermanModel().assetModel->getSkeleton(assetmesh.getSkeletonId());    
    
	mem.allocate(GpuDatatypes::mat3x3, static_cast<uint32_t>(skeleton.bones.size()));
	for (uint32_t boneid = mem.arrayElements(); boneid > 0;/*done in the loop*/)
	{
		--boneid;
		mem.setValue(glm::inverseTranspose(glm::mat3(rmesh.renderModel_->assetModel->getBoneWorldMatrix(rmesh.assetMeshId, boneid))), boneid);
	}
	return true;
}

inline bool getNumBones(TypedMem& mem, const RendermanNode& node)
{
	mem.setValue(node.toRendermanMesh().assetMesh->getNumBones());
	return true;
}

// clang-format off
#define BONEFUNC(idx) bool getBoneMatrix##idx(TypedMem& mem, const RendermanNode& node) { return getBoneMatrix(mem, node, idx); }\
    bool getBoneMatrixIT##idx(TypedMem& mem, const RendermanNode& node) { return getBoneMatrixIT(mem, node, idx); }

BONEFUNC(0)BONEFUNC(1)BONEFUNC(2)BONEFUNC(3)BONEFUNC(4)BONEFUNC(5)BONEFUNC(6)BONEFUNC(7)BONEFUNC(8)BONEFUNC(9)
BONEFUNC(10)BONEFUNC(11)BONEFUNC(12)BONEFUNC(13)BONEFUNC(14)BONEFUNC(15)BONEFUNC(16)BONEFUNC(17)BONEFUNC(18)BONEFUNC(19)
BONEFUNC(20)BONEFUNC(21)BONEFUNC(22)BONEFUNC(23)BONEFUNC(24)BONEFUNC(25)BONEFUNC(26)BONEFUNC(27)BONEFUNC(28)BONEFUNC(29)
BONEFUNC(30)BONEFUNC(31)BONEFUNC(32)BONEFUNC(33)BONEFUNC(34)BONEFUNC(35)BONEFUNC(36)BONEFUNC(37)BONEFUNC(38)BONEFUNC(39)
BONEFUNC(40)BONEFUNC(41)BONEFUNC(42)BONEFUNC(43)BONEFUNC(44)BONEFUNC(45)BONEFUNC(46)BONEFUNC(47)BONEFUNC(48)BONEFUNC(49)
BONEFUNC(50)BONEFUNC(51)BONEFUNC(52)BONEFUNC(53)BONEFUNC(54)BONEFUNC(55)BONEFUNC(56)BONEFUNC(57)BONEFUNC(58)BONEFUNC(59)
BONEFUNC(60)BONEFUNC(61)BONEFUNC(62)BONEFUNC(63)BONEFUNC(64)BONEFUNC(65)BONEFUNC(66)BONEFUNC(67)BONEFUNC(68)BONEFUNC(69)
BONEFUNC(70)BONEFUNC(71)BONEFUNC(72)BONEFUNC(73)BONEFUNC(74)BONEFUNC(75)BONEFUNC(76)BONEFUNC(77)BONEFUNC(78)BONEFUNC(79)
BONEFUNC(80)BONEFUNC(81)BONEFUNC(82)BONEFUNC(83)BONEFUNC(84)BONEFUNC(85)BONEFUNC(86)BONEFUNC(87)BONEFUNC(88)BONEFUNC(89)
BONEFUNC(90)BONEFUNC(91)BONEFUNC(92)BONEFUNC(93)BONEFUNC(94)BONEFUNC(95)BONEFUNC(96)BONEFUNC(97)BONEFUNC(98)BONEFUNC(99)

#define LIGHTFUNC(idx) \
    bool getLightPosition##idx(TypedMem& mem, const RendermanModel& model) { return getLightPosition(mem, model, idx); }\
    bool getLightDirection##idx(TypedMem& mem, const RendermanModel& model) { return getLightDirection(mem, model, idx); }\
    bool getLightColor##idx(TypedMem& mem, const RendermanModel& model) { return getLightColor(mem, model, idx); }\

LIGHTFUNC(0)LIGHTFUNC(1)LIGHTFUNC(2)LIGHTFUNC(3)LIGHTFUNC(4)LIGHTFUNC(5)LIGHTFUNC(6)LIGHTFUNC(7)LIGHTFUNC(8)LIGHTFUNC(9)
LIGHTFUNC(10)LIGHTFUNC(11)LIGHTFUNC(12)LIGHTFUNC(13)LIGHTFUNC(14)LIGHTFUNC(15)LIGHTFUNC(16)LIGHTFUNC(17)LIGHTFUNC(18)LIGHTFUNC(19)
LIGHTFUNC(20)LIGHTFUNC(21)LIGHTFUNC(22)LIGHTFUNC(23)LIGHTFUNC(24)LIGHTFUNC(25)LIGHTFUNC(26)LIGHTFUNC(27)LIGHTFUNC(28)LIGHTFUNC(29)
LIGHTFUNC(30)LIGHTFUNC(31)LIGHTFUNC(32)LIGHTFUNC(33)LIGHTFUNC(34)LIGHTFUNC(35)LIGHTFUNC(36)LIGHTFUNC(37)LIGHTFUNC(38)LIGHTFUNC(39)
LIGHTFUNC(40)LIGHTFUNC(41)LIGHTFUNC(42)LIGHTFUNC(43)LIGHTFUNC(44)LIGHTFUNC(45)LIGHTFUNC(46)LIGHTFUNC(47)LIGHTFUNC(48)LIGHTFUNC(49)
LIGHTFUNC(50)LIGHTFUNC(51)LIGHTFUNC(52)LIGHTFUNC(53)LIGHTFUNC(54)LIGHTFUNC(55)LIGHTFUNC(56)LIGHTFUNC(57)LIGHTFUNC(58)LIGHTFUNC(59)
LIGHTFUNC(60)LIGHTFUNC(61)LIGHTFUNC(62)LIGHTFUNC(63)LIGHTFUNC(64)LIGHTFUNC(65)LIGHTFUNC(66)LIGHTFUNC(67)LIGHTFUNC(68)LIGHTFUNC(69)
LIGHTFUNC(70)LIGHTFUNC(71)LIGHTFUNC(72)LIGHTFUNC(73)LIGHTFUNC(74)LIGHTFUNC(75)LIGHTFUNC(76)LIGHTFUNC(77)LIGHTFUNC(78)LIGHTFUNC(79)
LIGHTFUNC(80)LIGHTFUNC(81)LIGHTFUNC(82)LIGHTFUNC(83)LIGHTFUNC(84)LIGHTFUNC(85)LIGHTFUNC(86)LIGHTFUNC(87)LIGHTFUNC(88)LIGHTFUNC(89)
LIGHTFUNC(90)LIGHTFUNC(91)LIGHTFUNC(92)LIGHTFUNC(93)LIGHTFUNC(94)LIGHTFUNC(95)LIGHTFUNC(96)LIGHTFUNC(97)LIGHTFUNC(98)LIGHTFUNC(99)

#define CAMFUNC(idx) \
    bool getPerspectiveMatrix##idx(TypedMem& mem, const RendermanModel& model) \
    { \
        return getPerspectiveMatrix(mem, model, idx); \
    }\
    bool getViewMatrix##idx(TypedMem& mem, const RendermanModel& model) \
    { \
        return getViewMatrix(mem, model, idx);\
    }\
    bool getViewProjectionMatrix##idx(TypedMem& mem, const RendermanModel& model) \
    {\
        return getViewProjectionMatrix(mem, model, idx);\
    }

CAMFUNC(0)CAMFUNC(1)CAMFUNC(2)CAMFUNC(3)CAMFUNC(4)CAMFUNC(5)CAMFUNC(6)CAMFUNC(7)CAMFUNC(8)CAMFUNC(9)
	// clang-format on

	inline bool getWorldMatrix(TypedMem& mem, const RendermanNode& node)
{
	mem.setValue(node.toRendermanMesh().renderModel_->assetModel->getWorldMatrix(node.assetNodeId));
	return true;
}

inline bool getWorldMatrixIT(TypedMem& mem, const RendermanNode& node)
{
	mem.setValue(glm::inverseTranspose(glm::mat3(node.toRendermanMesh().renderModel_->assetModel->getWorldMatrix(node.assetNodeId))));
	return true;
}

inline bool getModelViewMatrix(TypedMem& mem, const RendermanNode& node)
{
	getWorldMatrix(mem, node);
	TypedMem viewmtx;
	node.toRendermanMesh().renderModel_->getModelSemantic(VIEWMATRIX_STR, viewmtx);
	mem.interpretValueAs<glm::mat4>() = viewmtx.interpretValueAs<glm::mat4>() * mem.interpretValueAs<glm::mat4>();
	return true;
}

inline bool getModelViewProjectionMatrix(TypedMem& mem, const RendermanNode& node)
{
	getWorldMatrix(mem, node);
	TypedMem viewprojmtx;
	if (node.subpassMesh_->rendermesh_->renderModel_->getModelSemantic(VIEWPROJECTIONMATRIX_STR, viewprojmtx))
	{ mem.interpretValueAs<glm::mat4>() = viewprojmtx.interpretValueAs<glm::mat4>() * mem.interpretValueAs<glm::mat4>(); }
	return true;
}
} // namespace

/////////// MEMBER FUNCTIONS OF THE RENDERMANAGER ///////////////

// RENDERNODE

bool RendermanNode::getNodeSemantic(const StringHash& semantic, TypedMem& mem) const { return getNodeSemanticSetter(semantic)(mem, *this); }

NodeSemanticSetter RendermanNode::getNodeSemanticSetter(const StringHash& semantic) const
{
	switch (semantic.getHash())
	{
	case HashCompileTime<'W', 'O', 'R', 'L', 'D', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'W', 'O', 'R', 'L', 'D', 'M', 'T', 'X'>::value:
	case HashCompileTime<'W', 'O', 'R', 'L', 'D'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'M', 'T', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'W', 'O', 'R', 'L', 'D', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'W', 'O', 'R', 'L', 'D', 'M', 'T', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'W', 'O', 'R', 'L', 'D'>::value: {
		return &getWorldMatrix;
	}
	break;
	case HashCompileTime<'W', 'O', 'R', 'L', 'D', 'I', 'T', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'W', 'O', 'R', 'L', 'D', 'I', 'T', 'M', 'T', 'X'>::value:
	case HashCompileTime<'W', 'O', 'R', 'L', 'D', 'I', 'T'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'I', 'T', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'I', 'T', 'M', 'T', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'I', 'T'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'W', 'O', 'R', 'L', 'D', 'I', 'T', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'W', 'O', 'R', 'L', 'D', 'I', 'T', 'M', 'T', 'X'>::value:
	case HashCompileTime<'W', 'O', 'R', 'L', 'D', 'M', 'A', 'T', 'R', 'I', 'X', 'I', 'T'>::value:
	case HashCompileTime<'W', 'O', 'R', 'L', 'D', 'M', 'T', 'X', 'I', 'T'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'M', 'A', 'T', 'R', 'I', 'X', 'I', 'T'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'M', 'T', 'X', 'I', 'T'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'W', 'O', 'R', 'L', 'D', 'M', 'A', 'T', 'R', 'I', 'X', 'I', 'T'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'W', 'O', 'R', 'L', 'D', 'M', 'T', 'X', 'I', 'T'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'W', 'O', 'R', 'L', 'D', 'I', 'T'>::value: {
		return &getWorldMatrixIT;
	}
	break;
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'V', 'I', 'E', 'W', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'V', 'I', 'E', 'W', 'M', 'T', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'V', 'I', 'E', 'W'>::value:
	case HashCompileTime<'M', 'V', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'M', 'V', 'M', 'T', 'X'>::value:
	case HashCompileTime<'M', 'V'>::value: {
		return &getModelViewMatrix;
	}
	break;
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'T', 'X'>::value:
	case HashCompileTime<'M', 'O', 'D', 'E', 'L', 'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N'>::value:
	case HashCompileTime<'M', 'V', 'P', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'M', 'V', 'P', 'M', 'T', 'X'>::value:
	case HashCompileTime<'M', 'V', 'P'>::value: {
		return &getModelViewProjectionMatrix;
	}
	break;
	case HashCompileTime<'B', 'O', 'N', 'E', 'C', 'O', 'U', 'N', 'T'>::value:
	case HashCompileTime<'N', 'U', 'M', 'B', 'O', 'N', 'E', 'S'>::value: {
		return &getNumBones;
	}
	break;
	// clang-format off
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'C', 'E', 'S'>::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X', 'A', 'R', 'R', 'A', 'Y'>::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X'>::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'T', 'X'>::value: \
	case HashCompileTime<'B', 'O', 'N', 'E'>::value: return &getBoneMatrices; break; \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'C', 'E', 'S', 'I', 'T'> ::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'C', 'E', 'S', 'A', 'R', 'R', 'A', 'Y', 'I', 'T'> ::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X', 'A', 'R', 'R', 'A', 'Y', 'I', 'T'> ::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'C', 'E', 'S', 'I', 'T', 'A', 'R', 'R', 'A', 'Y'> ::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X', 'I', 'T', 'A', 'R', 'R', 'A', 'Y'> ::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'A', 'T', 'R', 'I', 'X', 'I', 'T'> ::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'M', 'T', 'X', 'I', 'T'> ::value: \
	case HashCompileTime<'B', 'O', 'N', 'E', 'I', 'T'> ::value: return &getBoneMatricesIT; break; \
		BONEMTX0_9('0', 0)BONEMTX0_9('1', 1)BONEMTX0_9('2', 2)BONEMTX0_9('3', 3)BONEMTX0_9('4', 4)BONEMTX0_9('5', 5)BONEMTX0_9('6', 6)BONEMTX0_9('7', 7)BONEMTX0_9('8', 8)BONEMTX0_9('9', 9)
		BONEMTX10_99('1', '0', 10)BONEMTX10_99('1', '1', 11)BONEMTX10_99('1', '2', 12)BONEMTX10_99('1', '3', 13)BONEMTX10_99('1', '4', 14)BONEMTX10_99('1', '5', 15)BONEMTX10_99('1', '6', 16)BONEMTX10_99('1', '7', 17)BONEMTX10_99('1', '8', 18)BONEMTX10_99('1', '9', 19)
		BONEMTX10_99('2', '0', 20)BONEMTX10_99('2', '1', 21)BONEMTX10_99('2', '2', 22)BONEMTX10_99('2', '3', 23)BONEMTX10_99('2', '4', 24)BONEMTX10_99('2', '5', 25)BONEMTX10_99('2', '6', 26)BONEMTX10_99('2', '7', 27)BONEMTX10_99('2', '8', 28)BONEMTX10_99('2', '9', 29)
		BONEMTX10_99('3', '0', 30)BONEMTX10_99('3', '1', 31)BONEMTX10_99('3', '2', 32)BONEMTX10_99('3', '3', 33)BONEMTX10_99('3', '4', 34)BONEMTX10_99('3', '5', 35)BONEMTX10_99('3', '6', 36)BONEMTX10_99('3', '7', 37)BONEMTX10_99('3', '8', 38)BONEMTX10_99('3', '9', 39)
		BONEMTX10_99('4', '0', 40)BONEMTX10_99('4', '1', 41)BONEMTX10_99('4', '2', 42)BONEMTX10_99('4', '3', 43)BONEMTX10_99('4', '4', 44)BONEMTX10_99('4', '5', 45)BONEMTX10_99('4', '6', 46)BONEMTX10_99('4', '7', 47)BONEMTX10_99('4', '8', 48)BONEMTX10_99('4', '9', 49)
		BONEMTX10_99('5', '0', 50)BONEMTX10_99('5', '1', 51)BONEMTX10_99('5', '2', 52)BONEMTX10_99('5', '3', 53)BONEMTX10_99('5', '4', 54)BONEMTX10_99('5', '5', 55)BONEMTX10_99('5', '6', 56)BONEMTX10_99('5', '7', 57)BONEMTX10_99('5', '8', 58)BONEMTX10_99('5', '9', 59)
		BONEMTX10_99('6', '0', 60)BONEMTX10_99('6', '1', 61)BONEMTX10_99('6', '2', 62)BONEMTX10_99('6', '3', 63)BONEMTX10_99('6', '4', 64)BONEMTX10_99('6', '5', 65)BONEMTX10_99('6', '6', 66)BONEMTX10_99('6', '7', 67)BONEMTX10_99('6', '8', 68)BONEMTX10_99('6', '9', 69)
		BONEMTX10_99('7', '0', 70)BONEMTX10_99('7', '1', 71)BONEMTX10_99('7', '2', 72)BONEMTX10_99('7', '3', 73)BONEMTX10_99('7', '4', 74)BONEMTX10_99('7', '5', 75)BONEMTX10_99('7', '6', 76)BONEMTX10_99('7', '7', 77)BONEMTX10_99('7', '8', 78)BONEMTX10_99('7', '9', 79)
		BONEMTX10_99('8', '0', 80)BONEMTX10_99('8', '1', 81)BONEMTX10_99('8', '2', 82)BONEMTX10_99('8', '3', 83)BONEMTX10_99('8', '4', 84)BONEMTX10_99('8', '5', 85)BONEMTX10_99('8', '6', 86)BONEMTX10_99('8', '7', 87)BONEMTX10_99('8', '8', 88)BONEMTX10_99('8', '9', 89)
		BONEMTX10_99('9', '0', 90)BONEMTX10_99('9', '1', 91)BONEMTX10_99('9', '2', 92)BONEMTX10_99('9', '3', 93)BONEMTX10_99('9', '4', 94)BONEMTX10_99('9', '5', 95)BONEMTX10_99('9', '6', 96)BONEMTX10_99('9', '7', 97)BONEMTX10_99('9', '8', 98)BONEMTX10_99('9', '9', 99)
		// clang-format on
	}
	return NULL;
}

bool RendermanNode::updateNodeValueSemantic(const StringHash& semantic, const FreeValue& value, uint32_t swapid)
{
	return toRendermanPipeline().updateBufferEntryNodeSemantic(semantic, value, swapid, *this);
}

// RENDERMODEL

ModelSemanticSetter RendermanModel::getModelSemanticSetter(const StringHash& semantic) const
{
	switch (semantic.getHash())
	{
	case HashCompileTime<'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'T', 'X'>::value:
	case HashCompileTime<'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N'>::value:
	case HashCompileTime<'P', 'E', 'R', 'S', 'P', 'E', 'C', 'T', 'I', 'V', 'E', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'P', 'E', 'R', 'S', 'P', 'E', 'C', 'T', 'I', 'V', 'E', 'M', 'T', 'X'>::value:
	case HashCompileTime<'P', 'E', 'R', 'S', 'P', 'E', 'C', 'T', 'I', 'V', 'E'>::value: return &getPerspectiveMatrix0; break;
	case HashCompileTime<'V', 'I', 'E', 'W', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'V', 'I', 'E', 'W', 'M', 'T', 'X'>::value:
	case HashCompileTime<'V', 'I', 'E', 'W'>::value: return &getViewMatrix0; break;
	case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N', 'M', 'T', 'X'>::value:
	case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'E', 'C', 'T', 'I', 'O', 'N'>::value:
	case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
	case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J', 'M', 'T', 'X'>::value:
	case HashCompileTime<'V', 'I', 'E', 'W', 'P', 'R', 'O', 'J'>::value:
	case HashCompileTime<'V', 'P', 'M', 'A', 'T', 'R', 'I', 'X'>::value:
		return &getViewProjectionMatrix0;
		break;
		// Expands to definitions like the above, but suffixed with a character: VIEWPROJECTION0,VIEWPROJECTION1,VIEWMATRIX0,VIEWMATRIX1 etc, each referencing a different camera.
		CAMERA('0', 0) CAMERA('1', 1) CAMERA('2', 2) CAMERA('3', 3) CAMERA('4', 4) CAMERA('5', 5) CAMERA('6', 6) CAMERA('7', 7) CAMERA('8', 8) CAMERA('9', 9) break;
	case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'P', 'O', 'S', 'I', 'T', 'I', 'O', 'N'>::value:
	case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'P', 'O', 'S'>::value: return &getLightPosition0; break;
	case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'D', 'I', 'R', 'E', 'C', 'T', 'I', 'O', 'N'>::value:
	case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'D', 'I', 'R'>::value: return &getLightDirection0; break;
	case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'C', 'O', 'L', 'O', 'R'>::value:
	case HashCompileTime<'L', 'I', 'G', 'H', 'T', 'C', 'O', 'L', 'O', 'U', 'R'>::value:
		return &getLightColor0;
		break;
		LIGHT0_9('0', 0)
		LIGHT0_9('1', 1) LIGHT0_9('2', 2) LIGHT0_9('3', 3) LIGHT0_9('4', 4) LIGHT0_9('5', 5) LIGHT0_9('6', 6) LIGHT0_9('7', 7) LIGHT0_9('8', 8) LIGHT0_9('9', 9) break;
	default: break;
	}
	return NULL;
}

bool RendermanModel::getModelSemantic(const StringHash& semantic, TypedMem& memory) const
{
	auto setter = getModelSemanticSetter(semantic);
	return setter ? setter(memory, *this) : false;
}

/////////// RENDERING COMMANDS - (various classes of the RenderManager) ///////////

void RenderManager::recordAllRenderingCommands(CommandBuffer& cmdBuffer, uint16_t swapIdx, bool recordBeginEndRenderPass)
{
	for (auto& effect : _renderStructure.effects) { effect.recordRenderingCommands(cmdBuffer, swapIdx, recordBeginEndRenderPass); }
}

void RendermanEffect::recordRenderingCommands(CommandBuffer& cmdBuffer, uint16_t swapIdx, bool beginEndRenderPass)
{
	for (auto& pass : passes) { pass.recordRenderingCommands(cmdBuffer, swapIdx, beginEndRenderPass); }
}

void RendermanPass::recordRenderingCommands(CommandBuffer& cmdBuffer, uint16_t swapIdx, bool beginEndRendermanPass)
{
	if (beginEndRendermanPass)
	{
		// use the clear color from the model if found, else use the default
		ClearValue clearColor(.0f, .0f, .0f, 1.0f);
		for (uint32_t i = 0; i < subpasses.size(); ++i)
		{
			for (uint32_t j = 0; j < subpasses[i].groups.size(); ++j)
			{
				for (RendermanModel* model : subpasses[i].groups[j].allModels)
				{
					if (model)
					{
						memcpy(&clearColor, model->assetModel->getInternalData().clearColor, sizeof(model->assetModel->getInternalData().clearColor));
						i = j = static_cast<uint32_t>(-1); // break the outer loop as well.
						break;
					}
				}
			}
		}

		recordRenderingCommandsWithClearColor(cmdBuffer, swapIdx, clearColor);
	}
	else
	{
		recordRenderingCommands_(cmdBuffer, swapIdx, nullptr, 0);
	}
}

void RendermanPass::recordRenderingCommands_(CommandBuffer& cmdBuffer, uint16_t swapIdx, const ClearValue* clearValues, uint32_t numClearValues)
{
	if (clearValues)
	{
		cmdBuffer->beginRenderPass(framebuffer[swapIdx], framebuffer[swapIdx]->getRenderPass(),
			pvrvk::Rect2D(pvrvk::Offset2D(0, 0), pvrvk::Extent2D(framebuffer[swapIdx]->getDimensions().getWidth(), framebuffer[swapIdx]->getDimensions().getHeight())), true,
			clearValues, numClearValues);
	}
	bool first = true;
	for (auto& subpass : subpasses)
	{
		subpass.recordRenderingCommands(cmdBuffer, swapIdx, !first);
		first = false;
	}
	if (clearValues) { cmdBuffer->endRenderPass(); }
}

void RendermanSubpassGroup::recordRenderingCommands(CommandBufferBase cmdBuffer, uint16_t swapIdx)
{
	for (auto& spmodels : subpassGroupModels) { spmodels.recordRenderingCommands(cmdBuffer, swapIdx); }
}

void RendermanSubpassGroupModel::recordRenderingCommands(CommandBufferBase cmdBuffer, uint16_t swapIdx)
{
	DescriptorSet prev_sets[4] = {};

	bool bindSets[FrameworkCaps::MaxDescriptorSetBindings] = { true, true, true, true };

	const uint32_t* dynamicOffsets[FrameworkCaps::MaxDescriptorSetBindings][pvrvk::FrameworkCaps::MaxSwapChains] = { { 0 } };

	GraphicsPipeline prev_pipeline = nullptr;
	uint32_t nodeId = 0;
	for (auto& node : nodes)
	{
		auto& renderpipeline = *node.pipelineMaterial_->pipeline_;
		GraphicsPipeline& pipeline = renderpipeline.apiPipeline;

		bool bindPipeline = (!prev_pipeline || pipeline != prev_pipeline);
		prev_pipeline = pipeline;

		for (uint32_t setid = 0; setid < FrameworkCaps::MaxDescriptorSetBindings; ++setid)
		{
			if (!renderpipeline.pipelineInfo->descSetExists[setid])
			{
				bindSets[setid] = false;
				continue;
			}
			uint32_t setswapid = renderpipeline.pipelineInfo->descSetIsMultibuffered[setid] ? swapIdx : 0;
			const std::vector<uint32_t>& nodeDynamicOffsets = node.getDynamicOffsets(setid, swapIdx);
			bindSets[setid] = (bindPipeline || node.pipelineMaterial_->sets[setid][setswapid] != prev_sets[setid] || nodeDynamicOffsets.data() != dynamicOffsets[setid][swapIdx]);

			if (bindSets[setid])
			{
				prev_sets[setid] = node.pipelineMaterial_->sets[setid][setswapid];
				dynamicOffsets[setid][swapIdx] = nodeDynamicOffsets.data();
			}
		}
#ifdef PVR_RENDERMANAGER_DEBUG_RENDERING_COMMANDS
		Log(LogLevel::Information, "RendermanSubpassGroupModel::recordRenderingCommands nodeid: %d, pipeline name: %s", nodeId, renderpipeline.name.c_str());
#endif
		++nodeId;
		node.recordRenderingCommands(cmdBuffer, swapIdx, bindPipeline, bindSets);
	}
}

void RendermanNode::recordRenderingCommands(
	CommandBufferBase cmdBuffer, uint16_t swapidx, bool recordBindPipeline, bool* recordBindDescriptorSets, bool recordBindVboIbo, bool recordDrawCalls)
{
	auto& pipe = toRendermanPipeline();
	auto& rmesh = toRendermanMesh();
	if (!pipe.apiPipeline) { return; }
	if (recordBindPipeline) { cmdBuffer->bindPipeline(pipe.apiPipeline); }

	for (uint32_t setid = 0; setid < FrameworkCaps::MaxDescriptorSetBindings; ++setid)
	{
		if (!recordBindDescriptorSets || recordBindDescriptorSets[setid])
		{
			if (!pipe.pipelineInfo->descSetExists[setid]) { continue; }
			uint32_t setswapid = pipe.pipelineInfo->descSetIsMultibuffered[setid] ? swapidx : 0;
			const std::vector<uint32_t>& dynamicOffset2 = getDynamicOffsets(setid, setswapid);

#ifdef PVR_RENDERMANAGER_DEBUG_RENDERING_COMMANDS
			Log(LogLevel::Information, "RendermanNode bindDescriptorSet");
			Log(LogLevel::Information, "\tsetId %d", setid);
			Log(LogLevel::Information, "\tsetswapId %d", setswapid);
			for (uint32_t offsetId = 0; offsetId < dynamicOffset2.size(); ++offsetId) { Log(LogLevel::Information, "\toffset %d: %d", offsetId, dynamicOffset2[offsetId]); }
#endif

			cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipe.apiPipeline->getPipelineLayout(), setid, pipelineMaterial_->sets[setid][setswapid],
				dynamicOffset2.data(), static_cast<uint32_t>(dynamicOffset2.size()));

			setswapid = pipe.pipelineInfo->descSetIsMultibuffered[setid] ? swapidx : 0;
		}
	}

	if (recordBindVboIbo)
	{
		if (rmesh.vbos.size() > 0) { cmdBuffer->bindVertexBuffer(rmesh.vbos[0], 0, 0); }
		if (rmesh.ibo) { cmdBuffer->bindIndexBuffer(rmesh.ibo, 0, convertToPVRVk(rmesh.indexType)); }
	}

	if (recordDrawCalls)
	{
		pvr::assets::Mesh& mesh = *rmesh.assetMesh;
		if (rmesh.ibo) { cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3); }
		else
		{
			cmdBuffer->draw(0, mesh.getNumVertices());
		}
	}
}

////////// RENDERING COMMANDS ///////// RENDERING COMMANDS ///////// RENDERING COMMANDS /////////

void RenderManager::buildRenderObjects_(CommandBuffer& texUploadCmdBuffer)
{
	// Distinct combinations of pipelines used for each mesh -> Used for the attribute layouts.
	std::map<PipelineSet, AttributeConfiguration> pipeSets;

	std::map<StringHash, AttributeConfiguration*> pipeToAttribMapping;

	// PREPARE DATA STRUCTURES
	// Creation of the renderables will have to be 2 passes: We will need to go through the entire list
	// ones to spot any duplicates etc. between pipelines, attribute layouts etc.
	// And then we will need to go through the list another time to actually create the pipelines and
	// map them to the meshes.

	prepareDataStructures(*this, meshAttributeLayout, pipeSets);

	// FOR EACH AND EVERY DISTINCT COMBINATION OF PIPELINES, create the attribute layouts. Do the merging, mapping etc.
	createAttributeConfigurations(*this, pipeSets, pipeToAttribMapping, meshAttributeLayout, true);

	// PHASE 3: Create the pipelines. We could not do that in the previous phase as we did not have the complete
	// picture of which meshes render with what pipelines, in order to generate the correct input assembly.
	createPipelines(*this, pipeToAttribMapping);

	// PHASE 4: Create the VBOs. Same. We also remap the actual data.
	createVbos(*this, meshAttributeLayout);

	// PHASE 5: Create all the descriptor sets, populate them with the UBOs/SSBOs, and the textures
	createDescriptorSets(*this, meshAttributeLayout, _descPool, _swapchain->getSwapchainLength(), texUploadCmdBuffer);
}
} // namespace utils
} // namespace pvr
//!\endcond
