/*!
\brief CreateInfo object helpers / proxies for the Graphics and Compute pipelines.
\file PVRVk/PipelineConfigVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/TypesVk.h"
#include "PVRVk/ForwardDecObjectsVk.h"
namespace pvrvk {
//!\cond NO_DOXYGEN
struct VertexAttributeInfoCmp_BindingLess_IndexLess
{
	bool operator()(const VertexInputAttributeDescription& lhs, const VertexInputAttributeDescription& rhs) const
	{
		return lhs.getBinding() < rhs.getBinding() || (lhs.getBinding() == rhs.getBinding() && lhs.getLocation() < rhs.getLocation());
	}
};

struct VertexAttributeInfoPred_BindingEquals
{
	uint16_t binding;
	VertexAttributeInfoPred_BindingEquals(uint16_t binding) : binding(binding) {}
	bool operator()(const VertexInputAttributeDescription& nfo) const { return nfo.getBinding() == binding; }
};

struct VertexBindingInfoCmp_BindingLess
{
	bool operator()(const VertexInputBindingDescription& lhs, const VertexInputBindingDescription& rhs) const { return lhs.getBinding() < rhs.getBinding(); }
};

struct VertexBindingInfoPred_BindingLess
{
	bool operator()(uint16_t lhs, const VertexInputBindingDescription& rhs) const { return lhs < rhs.getBinding(); }
};

struct VertexBindingInfoPred_BindingEqual
{
	uint16_t binding;
	VertexBindingInfoPred_BindingEqual(uint16_t binding) : binding(binding) {}
	bool operator()(const VertexInputBindingDescription& nfo) const { return nfo.getBinding() == binding; }
};
//!\endcond

/// <summary>Contains parameters needed to set depth stencil states to a pipeline create params. This object can be
/// added to a PipelineCreateInfo to set a depth-stencil state to values other than their defaults.</summary>
/// <remarks>--- Defaults: --- depthWrite:enabled, depthTest:enabled, DepthComparison:Less, Stencil Text:
/// disabled, All stencil ops:Keep,</remarks>
struct PipelineDepthStencilStateCreateInfo
{
private:
	// stencil
	bool _depthTest;
	bool _depthWrite;
	bool _stencilTestEnable;
	bool _depthBoundTest;
	bool _enableDepthStencilState;
	float _minDepth;
	float _maxDepth;

	StencilOpState _stencilFront;
	StencilOpState _stencilBack;

	pvrvk::CompareOp _depthCmpOp;

public:
	/// <summary>Set all Depth and Stencil parameters.</summary>
	/// <param name="depthWrite">Enable/ Disable depth write</param>
	/// <param name="depthTest"> Enable/ Disable depth test</param>
	/// <param name="depthCompareFunc">Depth compare op</param>
	/// <param name="stencilTest">Enable/ Disable stencil test</param>
	/// <param name="depthBoundTest">Enable/ Disable depth bound test</param>
	/// <param name="stencilFront">Stencil front state</param>
	/// <param name="stencilBack">Stencil back state</param>
	/// <param name="minDepth">Min depth value</param>
	/// <param name="maxDepth">Max depth value</param>
	explicit PipelineDepthStencilStateCreateInfo(bool depthWrite = true, bool depthTest = false, pvrvk::CompareOp depthCompareFunc = pvrvk::CompareOp::e_LESS,
		bool stencilTest = false, bool depthBoundTest = false, const StencilOpState& stencilFront = StencilOpState(), const StencilOpState& stencilBack = StencilOpState(),
		float minDepth = 0.f, float maxDepth = 1.f)
		: _depthTest(depthTest), _depthWrite(depthWrite), _stencilTestEnable(stencilTest), _depthBoundTest(depthBoundTest), _enableDepthStencilState(true), _minDepth(minDepth),
		  _maxDepth(maxDepth), _stencilFront(stencilFront), _stencilBack(stencilBack), _depthCmpOp(depthCompareFunc)
	{}

	/// <summary>Return true if depth test is enable</summary>
	/// <returns>bool</returns>
	bool isDepthTestEnable() const { return _depthTest; }

	/// <summary>Return true if depth write is enable</summary>
	/// <returns>bool</returns>
	bool isDepthWriteEnable() const { return _depthWrite; }

	/// <summary>Return true if depth bound is enable</summary>
	/// <returns>bool</returns>
	bool isDepthBoundTestEnable() const { return _depthBoundTest; }

	/// <summary>Return true if stencil test is enable</summary>
	/// <returns>bool</returns>
	bool isStencilTestEnable() const { return _stencilTestEnable; }

	/// <summary>Return minimum depth value</summary>
	/// <returns>float</returns>
	float getMinDepth() const { return _minDepth; }

	/// <summary>Return maximum depth value</summary>
	/// <returns>float</returns>
	float getMaxDepth() const { return _maxDepth; }

	/// <summary>Return depth comparison operator</summary>
	/// <returns>pvrvk::CompareOp</returns>
	pvrvk::CompareOp getDepthComapreOp() const { return _depthCmpOp; }

	/// <summary>Return true if this state is enabled.</summary>
	/// <returns>bool</returns>
	bool isAllStatesEnabled() const { return _enableDepthStencilState; }

	/// <summary>Enable/ Disale the entire state</summary>
	/// <param name="flag">True:enable, False:disable</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineDepthStencilStateCreateInfo& enableAllStates(bool flag)
	{
		_enableDepthStencilState = flag;
		return *this;
	}

	/// <summary>Enable/disable writing into the Depth Buffer.</summary>
	/// <param name="depthWrite">True:enable, False:disable</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineDepthStencilStateCreateInfo& enableDepthWrite(bool depthWrite)
	{
		_depthWrite = depthWrite;
		return *this;
	}

	/// <summary>Enable/disable depth test (initial state: enabled)</summary>
	/// <param name="depthTest">True:enable, False:disable</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineDepthStencilStateCreateInfo& enableDepthTest(bool depthTest)
	{
		_depthTest = depthTest;
		return *this;
	}

	/// <summary>Set the depth compare function (initial state: LessEqual)</summary>
	/// <param name="compareFunc">A ComparisonMode (Less, Greater, Less etc.)</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineDepthStencilStateCreateInfo& setDepthCompareFunc(pvrvk::CompareOp compareFunc)
	{
		_depthCmpOp = compareFunc;
		return *this;
	}

	/// <summary>Enable/disable stencil test.</summary>
	/// <param name="stencilTest">True:enable, False:disable</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineDepthStencilStateCreateInfo& enableStencilTest(bool stencilTest)
	{
		_stencilTestEnable = stencilTest;
		return *this;
	}

	/// <summary>Set the stencil front state</summary>
	/// <param name="stencil">Stencil state</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineDepthStencilStateCreateInfo& setStencilFront(const StencilOpState& stencil)
	{
		_stencilFront = stencil;
		return *this;
	}

	/// <summary>Set the stencil back state</summary>
	/// <param name="stencil">Stencil state</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineDepthStencilStateCreateInfo& setStencilBack(const StencilOpState& stencil)
	{
		_stencilBack = stencil;
		return *this;
	}

	/// <summary>Set the stencil front and back state</summary>
	/// <param name="stencil">Stencil state</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineDepthStencilStateCreateInfo& setStencilFrontAndBack(StencilOpState& stencil)
	{
		_stencilFront = stencil, _stencilBack = stencil;
		return *this;
	}

	/// <summary>Return stencil front state</summary>
	/// <returns>const StencilOpState&</returns>
	const StencilOpState& getStencilFront() const { return _stencilFront; }

	/// <summary>Return stencil back state</summary>
	/// <returns>const StencilOpState&</returns>
	const StencilOpState& getStencilBack() const { return _stencilBack; }

	/// <summary>Enable/ Disable depth bound testing</summary>
	/// <param name="enabled">True:enable, False:disable</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineDepthStencilStateCreateInfo& setDepthBoundEnabled(bool enabled)
	{
		_depthBoundTest = enabled;
		return *this;
	}

	/// <summary>Set the minimum depth bound</summary>
	/// <param name="minDepth">The minimum depth bound</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineDepthStencilStateCreateInfo& setMinDepthBound(float minDepth)
	{
		_minDepth = minDepth;
		return *this;
	}

	/// <summary>Set the maximum depth bound</summary>
	/// <param name="maxDepth">The maximum depth bound</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineDepthStencilStateCreateInfo& setMaxDepthBound(float maxDepth)
	{
		_maxDepth = maxDepth;
		return *this;
	}
};

/// <summary>Contains parameters needed to configure the Vertex Input for a pipeline object. (vertex attrubutes, input
/// bindings etc.). Use by adding the buffer bindings with (setInputBinding) and then configure the attributes with
/// (addVertexAttribute). Default settings: 0 Vertext buffers, 0 vertex attributes.</summary>
struct PipelineVertexInputStateCreateInfo
{
private:
	friend class ::pvrvk::impl::GraphicsPipeline_;

	std::vector<VertexInputBindingDescription> _inputBindings;
	std::vector<VertexInputAttributeDescription> _attributes;

public:
	/// <summary>Return the input bindings</summary>
	/// <returns>const std::vector<VertexInputBindingDescription>&</returns>
	const std::vector<VertexInputBindingDescription>& getInputBindings() const { return _inputBindings; }

	/// <summary>Return the vertex attributes</summary>
	/// <returns>const std::vector<VertexInputAttributeDescription>&</returns>
	const std::vector<VertexInputAttributeDescription>& getAttributes() const { return _attributes; }

	/// <summary>Clear this object.</summary>
	/// <returns>this object (allows chained calls)</returns>
	PipelineVertexInputStateCreateInfo& clear()
	{
		_inputBindings.clear();
		_attributes.clear();
		return *this;
	}

	/// <summary>Set the vertex input buffer bindings.</summary>
	/// <param name="bindingDesc">New input binding description</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineVertexInputStateCreateInfo& addInputBinding(const VertexInputBindingDescription& bindingDesc)
	{
		internal::insertSorted_overwrite(_inputBindings, bindingDesc, VertexBindingInfoCmp_BindingLess());
		return *this;
	}

	/// <summary>Return a VertexBindingInfo for a buffer binding index, else return NULL if not found</summary>
	/// <param name="bufferBinding">Buffer binding index</param>
	/// <returns>const VertexInputBindingDescription*</returns>
	const VertexInputBindingDescription* getInputBinding(uint32_t bufferBinding) const
	{
		for (const VertexInputBindingDescription& it : _inputBindings)
		{
			if (it.getBinding() == bufferBinding) { return &it; }
		}
		return nullptr;
	}

	/// <summary>Returns an input binding at a given index.</summary>
	/// <param name="index">The index of the VertexInputBindingDescription to retrieve.</param>
	/// <returns>const VertexInputBindingDescription&</returns>
	const VertexInputBindingDescription& getInputBindingByIndex(uint32_t index) const { return _inputBindings[index]; }

	/// <summary>Add vertex layout information to a buffer binding index using a VertexAttributeInfo object.</summary>
	/// <param name="attributeInfo">Vertex attribute info.</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineVertexInputStateCreateInfo& addInputAttribute(const VertexInputAttributeDescription& attributeInfo)
	{
		internal::insertSorted_overwrite(_attributes, attributeInfo, VertexAttributeInfoCmp_BindingLess_IndexLess());
		return *this;
	}

	/// <summary>Add vertex layout information to a buffer binding index using a VertexAttributeInfo object.</summary>
	/// <param name="attributeInfo">Pointer to vertex attribute info</param>
	/// <param name="numAttributes">Number of vertex attributes</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineVertexInputStateCreateInfo& addInputAttributes(const VertexInputAttributeDescription* attributeInfo, uint32_t numAttributes)
	{
		for (uint32_t i = 0; i < numAttributes; ++i) { internal::insertSorted_overwrite(_attributes, attributeInfo[i], VertexAttributeInfoCmp_BindingLess_IndexLess()); }
		return *this;
	}
};

/// <summary>Add Input Assembler configuration to this buffer object (primitive topology, vertex restart, vertex
/// reuse etc).</summary>
/// <remarks>--- Default settings --- Primitive Topology: TriangleList, Primitive Restart: False, Vertex Reuse:
/// Disabled, Primitive Restart Index: 0xFFFFFFFF</remarks>
struct PipelineInputAssemblerStateCreateInfo
{
	friend class ::pvrvk::impl::GraphicsPipeline_;

private:
	mutable pvrvk::PrimitiveTopology _topology;
	bool _disableVertexReuse;
	bool _primitiveRestartEnable;
	uint32_t _primitiveRestartIndex;

public:
	/// <summary>Create and configure an InputAssembler configuration.</summary>
	/// <param name="topology">Primitive Topology (default: TriangleList)</param>
	/// <param name="disableVertexReuse">Disable Vertex Reuse (true:disabled false:enabled). Default:true</param>
	/// <param name="primitiveRestartEnable">(true: enabled, false: disabled) Default:false</param>
	/// <param name="primitiveRestartIndex">Primitive Restart Index. Default 0xFFFFFFFF</param>
	/// <returns>this object (allows chained calls)</returns>
	explicit PipelineInputAssemblerStateCreateInfo(pvrvk::PrimitiveTopology topology = pvrvk::PrimitiveTopology::e_TRIANGLE_LIST, bool disableVertexReuse = true,
		bool primitiveRestartEnable = false, uint32_t primitiveRestartIndex = 0xFFFFFFFF)
		: _topology(topology), _disableVertexReuse(disableVertexReuse), _primitiveRestartEnable(primitiveRestartEnable), _primitiveRestartIndex(primitiveRestartIndex)
	{}

	/// <summary>Enable/ disable primitive restart.</summary>
	/// <param name="enable">true for enable, false for disable.</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineInputAssemblerStateCreateInfo& setPrimitiveRestartEnable(bool enable)
	{
		_primitiveRestartEnable = enable;
		return *this;
	}

	/// <summary>Enable/ disable vertex reuse.</summary>
	/// <param name="disable">true for disable, false for enable.</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineInputAssemblerStateCreateInfo& setVertexReuseDisable(bool disable)
	{
		_disableVertexReuse = disable;
		return *this;
	}

	/// <summary>Set primitive topology.</summary>
	/// <param name="topology">The primitive topology to interpret the vertices as (TriangleList, Lines etc)</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineInputAssemblerStateCreateInfo& setPrimitiveTopology(pvrvk::PrimitiveTopology topology)
	{
		this->_topology = topology;
		return *this;
	}

	/// <summary>Check if Vertex Reuse is disabled</summary>
	/// <returns>True if vertex reuse is disabled, otherwise false</returns>
	bool isVertexReuseDisabled() const { return _disableVertexReuse; }

	/// <summary>Check if primitive restart is enabled</summary>
	/// <returns>True if primitive restart is enabled, otherwise false</returns>
	bool isPrimitiveRestartEnabled() const { return _primitiveRestartEnable; }

	/// <summary>Get the primitive restart index</summary>
	/// <returns>The primitive restart index</returns>
	uint32_t getPrimitiveRestartIndex() const { return _primitiveRestartIndex; }

	/// <summary>Get primitive topology</summary>
	/// <returns>Returns primitive topology.</returns>
	pvrvk::PrimitiveTopology getPrimitiveTopology() const { return _topology; }
};

/// <summary>Pipeline Color blending state configuration (alphaToCoverage, logicOp).</summary>
/// <remarks>Defaults: Enable alpha to coverage:false, Enable logic op: false, Logic Op: Set, Attachments: 0</remarks>
struct PipelineColorBlendStateCreateInfo
{
private:
	friend class ::pvrvk::impl::GraphicsPipeline_;
	PipelineColorBlendAttachmentState _attachmentStates[FrameworkCaps::MaxColorAttachments];
	uint32_t _numAttachmentStates;
	bool _alphaToCoverageEnable;
	bool _logicOpEnable;
	pvrvk::LogicOp _logicOp;
	Color _colorBlendConstants;

public:
	/// <summary>Get color blend attachment states</summary>
	/// <returns>const PipelineColorBlendAttachmentState*</returns>
	const PipelineColorBlendAttachmentState* getAttachmentStates() const { return _attachmentStates; }

	/// <summary>Create a Color Blend state object.</summary>
	/// <param name="alphaToCoverageEnable">enable/ disable alpa to coverage (default:disable)</param>
	/// <param name="colorBlendConstants">Blending constants</param>
	/// <param name="logicOpEnable">enable/disable logicOp (default:disable)</param>
	/// <param name="logicOp">Select logic operation (default:Set)</param>
	/// <param name="attachmentStates">An array of color blend attachment states (default: NULL)</param>
	/// <param name="numAttachmentStates">Number of color attachment states in array (default: 0)</param>
	PipelineColorBlendStateCreateInfo(bool alphaToCoverageEnable, bool logicOpEnable, pvrvk::LogicOp logicOp, Color colorBlendConstants,
		PipelineColorBlendAttachmentState* attachmentStates, uint32_t numAttachmentStates)
		: _numAttachmentStates(0), _alphaToCoverageEnable(alphaToCoverageEnable), _logicOpEnable(logicOpEnable), _logicOp(logicOp), _colorBlendConstants(colorBlendConstants)
	{
		assert(numAttachmentStates < FrameworkCaps::MaxColorAttachments && "Blend Attachments out of range.");
		for (uint32_t i = 0; i < numAttachmentStates; i++) { _attachmentStates[i] = attachmentStates[i]; }
		_numAttachmentStates = numAttachmentStates;
	}

	/// <summary>Create a Color Blend state object.</summary>
	/// <param name="alphaToCoverageEnable">enable/ disable alpa to coverage (default:disable</param>
	/// <param name="logicOpEnable">enable/disable logicOp (default:disable)</param>
	/// <param name="logicOp">Select logic operation (default:Set)</param>
	/// <param name="colorBlendConstants">Set color blend constants. Default (0,0,0,0)</param>
	explicit PipelineColorBlendStateCreateInfo(
		bool alphaToCoverageEnable = false, bool logicOpEnable = false, pvrvk::LogicOp logicOp = pvrvk::LogicOp::e_SET, Color colorBlendConstants = Color(0., 0., 0., 0.))
		: _numAttachmentStates(0), _alphaToCoverageEnable(alphaToCoverageEnable), _logicOpEnable(logicOpEnable), _logicOp(logicOp), _colorBlendConstants(colorBlendConstants)
	{}

	/// <summary>Set a constant for color blending</summary>
	/// <param name="blendConst">The color blend constant</param>
	/// <returns>Return this object (allows chained calls)</returns>
	PipelineColorBlendStateCreateInfo& setColorBlendConst(const Color& blendConst)
	{
		_colorBlendConstants = blendConst;
		return *this;
	}

	/// <summary>Get the constant for color blending</summary>
	/// <returns>The color blend constant</returns>
	const Color& getColorBlendConst() const { return _colorBlendConstants; }

	/// <summary>Get color-blend attachment state (const).</summary>
	/// <param name="index">attachment index</param>
	/// <returns>const PipelineColorBlendAttachmentState&</returns>
	const PipelineColorBlendAttachmentState& getAttachmentState(uint32_t index) const { return _attachmentStates[index]; }

	/// <summary>Get number of attachment states</summary>
	/// <returns>uint32_t</returns>
	uint32_t getNumAttachmentStates() const { return _numAttachmentStates; }

	/// <summary>Enable/ disable alpha to coverage.</summary>
	/// <param name="alphaToCoverageEnable">Boolean flags indicating to enable/ disable alpha coverage</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineColorBlendStateCreateInfo& setAlphaToCoverageEnable(bool alphaToCoverageEnable)
	{
		_alphaToCoverageEnable = alphaToCoverageEnable;
		return *this;
	}

	/// <summary>Enable/ disable logic op.</summary>
	/// <param name="logicOpEnable">Boolean flags indicating to enable/ disable logic op</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineColorBlendStateCreateInfo& setLogicOpEnable(bool logicOpEnable)
	{
		_logicOpEnable = logicOpEnable;
		return *this;
	}

	/// <summary>Set the logic op.</summary>
	/// <param name="logicOp">New logic op to set</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineColorBlendStateCreateInfo& setLogicOp(pvrvk::LogicOp logicOp)
	{
		_logicOp = logicOp;
		return *this;
	}

	/// <summary>Append a color attachment blend configuration (appended to the end of the attachments list).</summary>
	/// <returns>this object (allows chained calls)</returns>
	PipelineColorBlendStateCreateInfo& clearAttachments()
	{
		for (uint32_t i = 0; i < FrameworkCaps::MaxColorAttachments; i++) { _attachmentStates[i] = PipelineColorBlendAttachmentState(); }
		_numAttachmentStates = 0;
		return *this;
	}

	/// <summary>Add a color attachment state blend configuration to a specified index.</summary>
	/// <param name="index">Which index this color attachment will be</param>
	/// <param name="state">The color attachment state to add</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineColorBlendStateCreateInfo& setAttachmentState(uint32_t index, const PipelineColorBlendAttachmentState& state)
	{
		assert(index < FrameworkCaps::MaxColorAttachments && "Blend config out of range.");
		_attachmentStates[index] = state;
		if (index >= _numAttachmentStates) { _numAttachmentStates = index + 1; }
		return *this;
	}

	/// <summary>Set all color attachment states as an array. Replaces any that had already been added.</summary>
	/// <param name="state">An array of color attachment states</param>
	/// <param name="count">The number of color attachment states in (state)</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineColorBlendStateCreateInfo& setAttachmentStates(uint32_t count, PipelineColorBlendAttachmentState const* state)
	{
		assert(count < FrameworkCaps::MaxColorAttachments && "Blend config out of range.");
		for (uint32_t i = 0; i < count; i++) { _attachmentStates[i] = state[i]; }
		_numAttachmentStates = count;
		return *this;
	}

	/// <summary>Check if Alpha to Coverage is enabled</summary>
	/// <returns>True if enabled, otherwise false</returns>
	bool isAlphaToCoverageEnabled() const { return _alphaToCoverageEnable; }

	/// <summary>Check if Logic Op is enabled</summary>
	/// <returns>True if enabled, otherwise false</returns>
	bool isLogicOpEnabled() const { return _logicOpEnable; }

	/// <summary>Get the Logic Op (regardless if enabled or not)</summary>
	/// <returns>The logic op</returns>
	pvrvk::LogicOp getLogicOp() const { return _logicOp; }
};

/// <summary>Pipeline Viewport state descriptor. Sets the base configuration of all viewports.</summary>
/// <remarks>Defaults: Number of Viewports:1, Clip Origin: lower lef, Depth range: 0..1</remarks>
struct PipelineViewportStateCreateInfo
{
	friend class ::pvrvk::impl::GraphicsPipeline_;

private:
	std::pair<Rect2D, Viewport> _scissorViewports[FrameworkCaps::MaxScissorViewports];
	uint32_t _numScissorViewports;

public:
	/// <summary>Constructor.</summary>
	PipelineViewportStateCreateInfo() : _numScissorViewports(0) {}

	/// <summary>Configure the viewport with its corresponding scissor rectangle for an attachment</summary>
	/// <param name="index">The index of the attachment for which to set the viewport and scissor rectangle</param>
	/// <param name="viewport">The viewport to set for attachment <paramref name="index"/></param>
	/// <param name="scissor">The scissor rectangle of the viewport</param>
	/// <returns>return this object (allows chained calls)</returns>
	PipelineViewportStateCreateInfo& setViewportAndScissor(uint32_t index, const Viewport& viewport, const Rect2D& scissor)
	{
		assert(index < FrameworkCaps::MaxScissorViewports && "Scissor Viewport out of range.");
		_scissorViewports[index].first = scissor;
		_scissorViewports[index].second = viewport;
		_numScissorViewports++;
		return *this;
	}

	/// <summary>Clear all states</summary>
	/// <returns>Return this object (allows chained calls)</returns>
	PipelineViewportStateCreateInfo& clear()
	{
		for (uint32_t i = 0; i < FrameworkCaps::MaxScissorViewports; i++)
		{
			_scissorViewports[i].first = Rect2D();
			_scissorViewports[i].second = Viewport();
		}
		_numScissorViewports = 0;
		return *this;
	}

	/// <summary>Get the scissor rectangle for the specified attachment intex</summary>
	/// <param name="index">The index for which to return the scissor rectangle</param>
	/// <returns>Get the scissor rectangle for the specified attachment intex</returns>
	const Rect2D& getScissor(uint32_t index) const { return _scissorViewports[index].first; }

	/// <summary>Get the viewport for the specified attachment intex</summary>
	/// <param name="index">The index for which to return the scissor rectangle</param>
	/// <returns>Get the scissor rectangle for the specified attachment intex</returns>
	const Viewport& getViewport(uint32_t index) const { return _scissorViewports[index].second; }

	/// <summary>Return number of viewport and scissor</summary>
	/// <returns>uint32_t</returns>
	uint32_t getNumViewportScissors() const { return _numScissorViewports; }
};

/// <summary>Pipeline Rasterisation, clipping and culling state configuration. Culling, winding order, depth clipping,
/// raster discard, point size, fill mode, provoking vertex.</summary>
/// <remarks>Defaults: Cull face: back, Front face: CounterClockWise, Depth Clipping: true, Rasterizer Discard: false,
/// Program Point Size: false, Point Origin: Lower left, Fill Mode: Front&Back, Provoking Vertex: First</remarks>
struct PipelineRasterizationStateCreateInfo
{
private:
	pvrvk::CullModeFlags _cullFace;
	pvrvk::FrontFace _frontFaceWinding;
	bool _enableDepthClip;
	bool _enableRasterizerDiscard;
	bool _enableProgramPointSize;
	bool _enableDepthBias;
	float _depthBiasClamp;
	float _depthBiasConstantFactor;
	float _depthBiasSlopeFactor;
	pvrvk::PolygonMode _fillMode;
	float _lineWidth;
	uint32_t _rasterizationStream;
	friend class ::pvrvk::impl::GraphicsPipeline_;

public:
	/// <summary>Create a rasterization and polygon state configuration.</summary>
	/// <param name="cullFace">Face culling (default Back)</param>
	/// <param name="frontFaceWinding">The polygon winding order (default, Front face is counterclockwise)</param>
	/// <param name="enableDepthClip">Enable depth clipping. If set to false , depth Clamping happens instead of clipping
	/// (default true)</param>
	/// <param name="enableRasterizerDiscard">Enable rasterizer discard (default false)</param>
	/// <param name="enableProgramPointSize">Enable program point size (default true)</param>
	/// <param name="fillMode">Polygon fill mode (default: Front and Back)</param>
	/// <param name="lineWidth">Width of rendered lines (default One)</param>
	/// <param name="enableDepthBias">Enable depth bias (default false)</param>
	/// <param name="enableDepthBias">Enable depth bias (default false)</param>
	/// <param name="depthBiasClamp">If depth bias is enabled, the clamping value for depth bias (default 0)</param>
	/// <param name="depthBiasConstantFactor">If depth bias is enabled, the constant value by which to bias depth(default 0)</param>
	/// <param name="depthBiasSlopeFactor">If depth bias is enabled, the slope value by which to bias depth(default:0)</param>
	/// <param name="rasterizationStream">Controls the use of vertex shader based transform feedback</param>
	explicit PipelineRasterizationStateCreateInfo(pvrvk::CullModeFlags cullFace = pvrvk::CullModeFlags::e_NONE,
		pvrvk::FrontFace frontFaceWinding = pvrvk::FrontFace::e_COUNTER_CLOCKWISE, bool enableDepthClip = true, bool enableRasterizerDiscard = false,
		bool enableProgramPointSize = false, pvrvk::PolygonMode fillMode = pvrvk::PolygonMode::e_FILL, float lineWidth = 1.0f, bool enableDepthBias = false,
		float depthBiasClamp = 0.f, float depthBiasConstantFactor = 0.f, float depthBiasSlopeFactor = 0.f, uint32_t rasterizationStream = 0)
		: _cullFace(cullFace), _frontFaceWinding(frontFaceWinding), _enableDepthClip(enableDepthClip), _enableRasterizerDiscard(enableRasterizerDiscard),
		  _enableProgramPointSize(enableProgramPointSize), _enableDepthBias(enableDepthBias), _depthBiasClamp(depthBiasClamp), _depthBiasConstantFactor(depthBiasConstantFactor),
		  _depthBiasSlopeFactor(depthBiasSlopeFactor), _fillMode(fillMode), _lineWidth(lineWidth), _rasterizationStream(rasterizationStream)
	{}

	/// <summary>Set the face that will be culled (front/back/both/none).</summary>
	/// <param name="face">Cull face</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineRasterizationStateCreateInfo& setCullMode(pvrvk::CullModeFlags face)
	{
		_cullFace = face;
		return *this;
	}

	/// <summary>Set the line width</summary>
	/// <param name="lineWidth">The width of lines (in pixels) when drawing line primitives.</param>
	/// <returns>Return this object (allows chained calls)</returns>
	PipelineRasterizationStateCreateInfo& setLineWidth(float lineWidth)
	{
		_lineWidth = lineWidth;
		return *this;
	}

	/// <summary>Set the Rasterization stream</summary>
	/// <param name="rasterizationStream">The vertex stream selected for rasterization. This parameter controls usage of transform feedback if it is supported.</param>
	/// <returns>Return this object (allows chained calls)</returns>
	PipelineRasterizationStateCreateInfo& setRasterizationStream(uint32_t rasterizationStream)
	{
		_rasterizationStream = rasterizationStream;
		return *this;
	}

	/// <summary>Select between depth Clipping and depth Clamping</summary>
	/// <param name="enableDepthClip">Set to true to clip polygons at the Z-sides of the view frustum Set to false to
	/// clamp the depth to the min/max values and not clip based on depth</param>
	/// <returns>Return this object (allows chained calls)</returns>
	PipelineRasterizationStateCreateInfo& setDepthClip(bool enableDepthClip)
	{
		_enableDepthClip = enableDepthClip;
		return *this;
	}

	/// <summary>Enable depth bias (add a value to the calculated fragment depth)</summary>
	/// <param name="enableDepthBias">Set to true to enable depth bias, false to disable</param>
	/// <param name="depthBiasClamp">The maximum (or minimum) value of depth biasing</param>
	/// <param name="depthBiasConstantFactor">A constant value added to all fragment depths</param>
	/// <param name="depthBiasSlopeFactor">Depth slope factor for multiply fragment depths</param>
	/// <returns>Return this object (allows chained calls)</returns>
	PipelineRasterizationStateCreateInfo& setDepthBias(bool enableDepthBias, bool depthBiasClamp = 0.f, bool depthBiasConstantFactor = 0.f, bool depthBiasSlopeFactor = 0.f)
	{
		_enableDepthBias = enableDepthBias;
		_depthBiasClamp = depthBiasClamp;
		_depthBiasConstantFactor = depthBiasConstantFactor;
		_depthBiasSlopeFactor = depthBiasSlopeFactor;
		return *this;
	}

	/// <summary>Set which polygon winding order is considered the "front" face (The opposite order is considered back
	/// face).</summary>
	/// <param name="frontFaceWinding">The winding order that will represent front faces</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineRasterizationStateCreateInfo& setFrontFaceWinding(pvrvk::FrontFace frontFaceWinding)
	{
		_frontFaceWinding = frontFaceWinding;
		return *this;
	}

	/// <summary>Disable all phases after transform feedback (rasterization and later)</summary>
	/// <param name="enable">Set to "false" for normal rendering, "true" to enable rasterization discard</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineRasterizationStateCreateInfo& setRasterizerDiscard(bool enable)
	{
		_enableRasterizerDiscard = enable;
		return *this;
	}

	/// <summary>Enable/disable Program Point Size.</summary>
	/// <param name="enable">Set to "true" to control point size for the entire program</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineRasterizationStateCreateInfo& setProgramPointSize(bool enable)
	{
		_enableProgramPointSize = enable;
		return *this;
	}

	/// <summary>Set polygon fill mode.</summary>
	/// <param name="mode">New polygon mode to set</param>
	/// <returns>this object (allows chained calls)</returns>
	PipelineRasterizationStateCreateInfo& setPolygonMode(pvrvk::PolygonMode mode)
	{
		_fillMode = mode;
		return *this;
	}

	/// <summary>Get which of the faces (Front/Back/None/Both) will not be rendered (will be culled)</summary>
	/// <returns>The faces that will be culled ("None" means everything drawn, "Both" means nothing drawn)</returns>
	pvrvk::CullModeFlags getCullFace() const { return _cullFace; }

	/// <summary>Get which winding order is considered the FRONT face (CCW means front faces are the counterclockwise,
	/// CW clockwise)</summary>
	/// <returns>The winding order that is considered FRONT facing</returns>
	pvrvk::FrontFace getFrontFaceWinding() const { return _frontFaceWinding; }

	/// <summary>Check if depth clipping is enabled (i.e. depth clamping disabled). If disabled, polygons will not be
	/// clipped against the front and back clipping plane clipping planes: instead, the primitives' depth will be
	/// clamped to min and max depth instead.</summary>
	/// <returns>True if depth clipping is enabled (depth clamping is disabled). False if depth clamping is enabled
	/// (depth clipping disabled).</returns>
	bool isDepthClipEnabled() const { return _enableDepthClip; }

	/// <summary>Check if rasterization is skipped (so all parts of the pipeline after transform feedback)</summary>
	/// <returns>True if rasterization discard is enabled, false for normal rendering.</returns>
	bool isRasterizerDiscardEnabled() const { return _enableRasterizerDiscard; }

	/// <summary>Check if program point size is enabled</summary>
	/// <returns>True if program point size is enabled, otherwise false</returns>
	bool isProgramPointSizeEnabled() const { return _enableProgramPointSize; }

	/// <summary>Check if depth bias is enabled. (If enabled, clamp, constant factor and slope factor can be checked)</summary>
	/// <returns>True if depth bias is enabled, otherwise false.</returns>
	bool isDepthBiasEnabled() const { return _enableDepthBias; }

	/// <summary>Get the maximum(minimum) value of depth bias</summary>
	/// <returns>The maximum(minimum) value of depth bias</returns>
	float getDepthBiasClamp() const { return _depthBiasClamp; }

	/// <summary>Get the constant factor of depth bias</summary>
	/// <returns>The constant factor depth bias</returns>
	float getDepthBiasConstantFactor() const { return _depthBiasConstantFactor; }

	/// <summary>Get the slope factor of depth bias</summary>
	/// <returns>The slope factor depth bias</returns>
	float getDepthBiasSlopeFactor() const { return _depthBiasSlopeFactor; }

	/// <summary>Get the slope factor of depth bias</summary>
	/// <returns>The slope factor depth bias</returns>
	pvrvk::PolygonMode getPolygonMode() const { return _fillMode; }

	/// <summary>Get the slope factor of depth bias</summary>
	/// <returns>The slope factor depth bias</returns>
	float getLineWidth() const { return _lineWidth; }

	/// <summary>Get the vertex stream selected for rasterization</summary>
	/// <returns>The vertex stream selected for rasterization</returns>
	uint32_t getRasterizationStream() const { return _rasterizationStream; }
};

/// <summary>Pipeline Multisampling state configuration: Number of samples, alpha to coverage, alpha to one,
/// sampling mask.</summary>
/// <remarks>Defaults: No multisampling</remarks>
struct PipelineMultisampleStateCreateInfo
{
private:
	friend class ::pvrvk::impl::GraphicsPipeline_;
	bool _sampleShadingEnable;
	bool _alphaToCoverageEnable;
	bool _alphaToOneEnable;
	pvrvk::SampleCountFlags _numRasterizationSamples;
	float _minSampleShading;
	pvrvk::SampleMask _sampleMask;

public:
	/// <summary>Constructor. Create a multisampling configuration.</summary>
	/// <param name="sampleShadingEnable">Enable/disable sample shading (defalt false)</param>
	/// <param name="alphaToCoverageEnable">Enable/disable alpha-to-coverage</param>
	/// <param name="alphaToOneEnable">Enable/disable alpha-to-one</param>
	/// <param name="rasterizationSamples">The number of rasterization samples (default 1)</param>
	/// <param name="minSampleShading">The minimum sample Shading (default 0)</param>
	/// <param name="sampleMask">sampleMask (default 0)</param>
	explicit PipelineMultisampleStateCreateInfo(bool sampleShadingEnable = false, bool alphaToCoverageEnable = false, bool alphaToOneEnable = false,
		pvrvk::SampleCountFlags rasterizationSamples = pvrvk::SampleCountFlags::e_1_BIT, float minSampleShading = 0.f, pvrvk::SampleMask sampleMask = 0xffffffff)
		: _sampleShadingEnable(sampleShadingEnable), _alphaToCoverageEnable(alphaToCoverageEnable), _alphaToOneEnable(alphaToOneEnable),
		  _numRasterizationSamples(rasterizationSamples), _minSampleShading(minSampleShading), _sampleMask(sampleMask)
	{}

	/// <summary>Enable/ Disable alpha to coverage</summary>
	/// <param name="enable">True to enable Alpha to Coverage, false to Disable</param>
	/// <returns>Return this object (allows chained calls)</returns>
	PipelineMultisampleStateCreateInfo& setAlphaToCoverage(bool enable)
	{
		_alphaToCoverageEnable = enable;
		return *this;
	}

	/// <summary>Enable/ disable sampler shading (Multi Sampling Anti Aliasing).</summary>
	/// <param name="enable">true enable per-sample shading, false to disable</param>
	/// <returns>this (allow chaining)</returns>
	PipelineMultisampleStateCreateInfo& setSampleShading(bool enable)
	{
		_sampleShadingEnable = enable;
		return *this;
	}

	/// <summary>Controls whether the alpha component of the fragment's first color output is replaced with one</summary>
	/// <param name="enable">true enable alpha to one, false disable</param>
	/// <returns>this (allow chaining)</returns>
	PipelineMultisampleStateCreateInfo& setAlphaToOne(bool enable)
	{
		_alphaToOneEnable = enable;
		return *this;
	}

	/// <summary>Set the number of samples per pixel used in rasterization (Multi sample anti aliasing)</summary>
	/// <param name="numSamples">The number of samples</param>
	/// <returns>this (allow chaining)</returns>
	PipelineMultisampleStateCreateInfo& setNumRasterizationSamples(pvrvk::SampleCountFlags numSamples)
	{
		_numRasterizationSamples = numSamples;
		return *this;
	}

	/// <summary>Set minimum sample shading.</summary>
	/// <param name="minSampleShading">The number of minimum samples to shade</param>
	/// <returns>this (allow chaining)</returns>
	PipelineMultisampleStateCreateInfo& setMinSampleShading(float minSampleShading)
	{
		_minSampleShading = minSampleShading;
		return *this;
	}

	/// <summary>Set a bitmask of static coverage information that is ANDed with the coverage information generated
	/// during rasterization.</summary>
	/// <param name="mask">The sample mask. See the corresponding API spec for exact bit usage of the mask.</param>
	/// <returns>this (allow chaining)</returns>
	PipelineMultisampleStateCreateInfo& setSampleMask(pvrvk::SampleMask mask)
	{
		_sampleMask = mask;
		return *this;
	}

	/// <summary>Get the sample mask</summary>
	/// <returns>The sample mask</returns>
	const pvrvk::SampleMask& getSampleMask() const { return _sampleMask; }

	/// <summary>Return the number of rasterization (MSAA) samples</summary>
	/// <returns>The number of rasterization samples</returns>
	pvrvk::SampleCountFlags getRasterizationSamples() const { return _numRasterizationSamples; }

	/// <summary>Get the number of minimum samples</summary>
	/// <returns>The number of minimum samples</returns>
	float getMinSampleShading() const { return _minSampleShading; }

	/// <summary>Get the sample shading state</summary>
	/// <returns>true if sample shading enabled, false if disabled</returns>
	bool isSampleShadingEnabled() const { return _sampleShadingEnable; }

	/// <summary>Get alpha to coverage state</summary>
	/// <returns>True if enabled, false if disabled</returns>
	bool isAlphaToCoverageEnabled() const { return _alphaToCoverageEnable; }

	/// <summary>Get alpha to one state</summary>
	/// <returns>Return true if alpha to one is enabled, false if disabled</returns>
	bool isAlphaToOneEnabled() const { return _alphaToOneEnable; }
};

/// <summary>Create params for Pipeline Dynamic states. Enable each state that you want to be able to dynamically
/// set.</summary>
struct DynamicStatesCreateInfo
{
private:
	bool _dynamicStates[static_cast<uint32_t>(pvrvk::DynamicState::e_RANGE_SIZE)];

public:
	/// <summary>Constructor</summary>
	DynamicStatesCreateInfo() { memset(_dynamicStates, 0, sizeof(_dynamicStates)); }

	/// <summary>Check if a specific dynamic state is enabled.</summary>
	/// <param name="state">The state to check</param>
	/// <returns>true if <paramref name="state"/>is enabled, otherwise false</returns>
	bool isDynamicStateEnabled(pvrvk::DynamicState state) const { return (_dynamicStates[static_cast<uint32_t>(state)]); }

	/// <summary>Enable/disable a dynamic state</summary>
	/// <param name="state">The state to enable or disable</param>
	/// <param name="enable">True to enable, false to disable</param>
	/// <returns>Return this object(allows chained calls)</returns>
	DynamicStatesCreateInfo& setDynamicState(pvrvk::DynamicState state, bool enable)
	{
		_dynamicStates[static_cast<uint32_t>(state)] = enable;
		return *this;
	}
};

/// <summary>A representation of a Shader constant</summary>
struct ShaderConstantInfo
{
	uint32_t constantId; //!< ID of the specialization constant in SPIR-V.
	unsigned char data[64]; //!< Data, max can hold 4x4 matrix
	uint32_t sizeInBytes; //!< Data size in bytes

	/// <summary>Constructor (Zero initialization)</summary>
	ShaderConstantInfo() : constantId(0), sizeInBytes(0) { memset(data, 0, sizeof(data)); }

	/// <summary>Return if this is a valid constant info</summary>
	/// <returns>bool</returns>
	bool isValid() const { return (sizeInBytes <= 64 && sizeInBytes > 0); }

	/// <summary>Constructor. Initialise with inidividual values</summary>
	/// <param name="constantId">ID of the specialization constant in SPIR-V.</param>
	/// <param name="data">Data, max can hold 4x4 matrix</param>
	/// <param name="sizeOfData">Data size in bytes</param>
	ShaderConstantInfo(uint32_t constantId, const void* data, uint32_t sizeOfData) : constantId(constantId), sizeInBytes(sizeOfData)
	{
		memset(this->data, 0, sizeof(this->data));
		memcpy(this->data, data, sizeInBytes);
	}
};

/// <summary>Pipeline vertex ShaderModule stage create param.</summary>
struct PipelineShaderStageCreateInfo
{
	friend class ::pvrvk::impl::GraphicsPipeline_;

private:
	pvrvk::ShaderModule _shaderModule;
	std::vector<ShaderConstantInfo> _shaderConsts;
	std::string _entryPoint;

public:
	/// <summary>Constructor.</summary>
	PipelineShaderStageCreateInfo() : _entryPoint("main") {}

	/// <summary>Construct from a pvrvk::ShaderModule object</summary>
	/// <param name="shader">A vertex shader</param>
	PipelineShaderStageCreateInfo(const ShaderModule& shader) : _shaderModule(shader), _entryPoint("main") {}

	/// <summary>Get the shader of this shader stage object</summary>
	/// <returns>The shader</returns>
	const ShaderModule& getShader() const { return _shaderModule; }

	/// <summary>Return true if this state is active (contains a shader)</summary>
	/// <returns>True if valid, otherwise false</returns>
	bool isActive() const { return _shaderModule != nullptr; }

	/// <summary>Set the shader.</summary>
	/// <param name="shader">A shader</param>
	void setShader(const ShaderModule& shader) { _shaderModule = shader; }

	/// <summary>Set the shader entry point function (default: "main"). Only supported for specific APIs</summary>
	/// <param name="entryPoint">A name of a function that will be used as an entry point in the shader</param>
	void setEntryPoint(const std::string& entryPoint) { _entryPoint = entryPoint; }

	/// <summary>Get the entry point of the shader</summary>
	/// <returns>The entry point of the shader</returns>
	const std::string& getEntryPoint() const { return _entryPoint; }

	/// <summary>operator =</summary>
	/// <param name="shader">ShaderModule object</param>
	/// <returns>PipelineShaderStageCreateInfo&</returns>
	PipelineShaderStageCreateInfo& operator=(const ShaderModule& shader)
	{
		setShader(shader);
		return *this;
	}

	/// <summary>Set a shader constants to the shader</summary>
	/// <param name="index">The index of the shader constant to set (does not have to be in order)</param>
	/// <param name="shaderConst">The shader constant to set to index <paramref name="index"/></param>
	/// <returns>Return this (allow chaining)</returns>
	PipelineShaderStageCreateInfo& setShaderConstant(uint32_t index, const ShaderConstantInfo& shaderConst)
	{
		if (_shaderConsts.size() < index + 1) { _shaderConsts.resize(index + 1); }
		_shaderConsts[index] = shaderConst;
		return *this;
	}

	/// <summary>Set all shader constants.</summary>
	/// <param name="shaderConsts">A c-style array containing the shader constants</param>
	/// <param name="numConstants">The number of shader constants in <paramref name="shaderConsts"/></param>
	/// <returns>Return this (allow chaining)</returns>
	/// <remarks>Uses better memory reservation than the setShaderConstant counterpart.</remarks>
	PipelineShaderStageCreateInfo& setShaderConstants(const ShaderConstantInfo* shaderConsts, uint32_t numConstants)
	{
		_shaderConsts.resize(numConstants);
		for (uint32_t i = 0; i < numConstants; i++) { _shaderConsts[i] = shaderConsts[i]; }
		return *this;
	}

	/// <summary>Retrieve a ShaderConstant by index</summary>
	/// <param name="index">The index of the ShaderConstant to retrieve</param>
	/// <returns>The shader constant</returns>
	const ShaderConstantInfo& getShaderConstant(uint32_t index) const { return _shaderConsts[index]; }

	/// <summary>Get all shader constants</summary>
	/// <returns>Return an array of all defined shader constants</returns>
	const ShaderConstantInfo* getAllShaderConstants() const { return _shaderConsts.data(); }

	/// <summary>Get the number of shader constants</summary>
	/// <returns>The number of shader constants</returns>
	uint32_t getNumShaderConsts() const { return (uint32_t)_shaderConsts.size(); }
};

/// <summary>Creation parameters for all Tesselation shaders</summary>
struct TesselationStageCreateInfo
{
	friend class ::pvrvk::impl::GraphicsPipeline_;

private:
	ShaderModule _controlShader, _evalShader;
	uint32_t _patchControlPoints;
	std::vector<ShaderConstantInfo> _shaderConstsTessCtrl;

	std::vector<ShaderConstantInfo> _shaderConstTessEval;

	std::string _controlShaderEntryPoint;
	std::string _evalShaderEntryPoint;

public:
	/// <summary>Constructor</summary>
	TesselationStageCreateInfo() : _patchControlPoints(3), _controlShaderEntryPoint("main"), _evalShaderEntryPoint("main") {}

	/// <summary>Get the Tessellation Control shader</summary>
	/// <returns>The Tessellation Control shader</returns>
	const ShaderModule& getControlShader() const { return _controlShader; }

	/// <summary>Get the Tessellation Evaluation shader</summary>
	/// <returns>The Tessellation Evaluation shader</returns>
	const ShaderModule& getEvaluationShader() const { return _evalShader; }

	/// <summary>Check if the Tessellation Control shader has been set</summary>
	/// <returns>true if the Tessellation Control shader has been set</returns>
	bool isControlShaderActive() const { return _controlShader != nullptr; }

	/// <summary>Check if the Tessellation Evaluation shader has been set</summary>
	/// <returns>true if the Tessellation Evaluation shader has been set</returns>
	bool isEvaluationShaderActive() const { return _evalShader != nullptr; }

	/// <summary>Set the control shader.</summary>
	/// <param name="shader">A shader to set to the Tessellation Control stage</param>
	/// <returns>this (allow chaining)</returns>
	TesselationStageCreateInfo& setControlShader(const ShaderModule& shader)
	{
		_controlShader = shader;
		return *this;
	}

	/// <summary>Set control shader entry point</summary>
	/// <param name="entryPoint">Entrypoint</param>
	/// <returns>TesselationStageCreateInfo&</returns>
	TesselationStageCreateInfo& setControlShaderEntryPoint(const char* entryPoint)
	{
		_controlShaderEntryPoint.assign(entryPoint);
		return *this;
	}

	/// <summary>Set evaluation shader entry point</summary>
	/// <param name="entryPoint">Entrypoint</param>
	/// <returns>TesselationStageCreateInfo&</returns>
	TesselationStageCreateInfo& setEvaluationShaderEntryPoint(const char* entryPoint)
	{
		_evalShaderEntryPoint.assign(entryPoint);
		return *this;
	}

	/// <summary>Set the control shader.</summary>
	/// <param name="shader">A shader to set to the Tessellation Control stage</param>
	/// <returns>this (allow chaining)</returns>
	TesselationStageCreateInfo& setEvaluationShader(const ShaderModule& shader)
	{
		_evalShader = shader;
		return *this;
	}

	/// <summary>Set number of control points</summary>
	/// <param name="controlPoints">The number of control points per patch</param>
	/// <returns>this (allow chaining)</returns>
	TesselationStageCreateInfo& setNumPatchControlPoints(uint32_t controlPoints)
	{
		_patchControlPoints = controlPoints;
		return *this;
	}

	/// <summary>Get number of control points</summary>
	/// <returns>The number of patch control points</returns>
	uint32_t getNumPatchControlPoints() const { return _patchControlPoints; }

	/// <summary>Set a shader constant for the Tessellation Control shader</summary>
	/// <param name="index">Index of the constant to set</param>
	/// <param name="shaderConst">Value of the constant to set</param>
	/// <returns>Return this for chaining</returns>
	TesselationStageCreateInfo& setControlShaderConstant(uint32_t index, const ShaderConstantInfo& shaderConst)
	{
		if (_shaderConstsTessCtrl.size() < index + 1) { _shaderConstsTessCtrl.resize(index + 1); }

		_shaderConstsTessCtrl[index] = shaderConst;
		return *this;
	}

	/// <summary>Set all Tessellation Control shader constants.</summary>
	/// <param name="shaderConsts">A c-style array containing the shader constants</param>
	/// <param name="numConstants">The number of shader constants in <paramref name="shaderConsts"/></param>
	/// <returns>Return this (allow chaining)</returns>
	/// <remarks>Uses better memory reservation than the setShaderConstant counterpart.</remarks>
	TesselationStageCreateInfo& setControlShaderConstants(const ShaderConstantInfo* shaderConsts, uint32_t numConstants)
	{
		if (_shaderConstsTessCtrl.size() != numConstants) { _shaderConstsTessCtrl.resize(numConstants); }

		for (uint32_t i = 0; i < numConstants; i++) { _shaderConstsTessCtrl[i] = shaderConsts[numConstants]; }
		return *this;
	}

	/// <summary>Get a Control shader constant</summary>
	/// <param name="index">The index of the constant to get. It is undefined to retrieve a constant that does not
	/// exist.</param>
	/// <returns>The Constant to get</returns>
	const ShaderConstantInfo& getControlShaderConstant(uint32_t index) const { return _shaderConstsTessCtrl[index]; }

	/// <summary>Return all control shader constants as a c-style array</summary>
	/// <returns>C-style array of all shader constants</returns>
	const ShaderConstantInfo* getAllControlShaderConstants() const { return _shaderConstsTessCtrl.data(); }

	/// <summary>Return number of control shader constants</summary>
	/// <returns>uint32_t</returns>
	uint32_t getNumControlShaderConstants() const { return (uint32_t)_shaderConstsTessCtrl.size(); }

	/// <summary>Set a shader constant for the Tessellation Evaluation shader</summary>
	/// <param name="index">Index of the constant to set</param>
	/// <param name="shaderConst">Value of the constant to set</param>
	/// <returns>Return this for chaining</returns>
	void setEvaluationShaderConstant(uint32_t index, const ShaderConstantInfo& shaderConst)
	{
		if (_shaderConstTessEval.size() < index + 1) { _shaderConstTessEval.resize(index + 1); }
		_shaderConstTessEval[index] = shaderConst;
	}

	/// <summary>Set all Tessellation Evaluation shader constants.</summary>
	/// <param name="shaderConsts">A c-style array containing the shader constants</param>
	/// <param name="numConstants">The number of shader constants in <paramref name="shaderConsts"/></param>
	/// <returns>Return this (allow chaining)</returns>
	/// <remarks>Uses better memory reservation than the setShaderConstant counterpart.</remarks>
	TesselationStageCreateInfo& setEvaluationShaderConstants(const ShaderConstantInfo* shaderConsts, uint32_t numConstants)
	{
		if (_shaderConstTessEval.size() != numConstants) { _shaderConstTessEval.resize(numConstants); }

		for (uint32_t i = 0; i < numConstants; i++) { _shaderConstTessEval[i] = shaderConsts[numConstants]; }
		return *this;
	}

	/// <summary>Get Evaluation shader constants</summary>
	/// <param name="index">The index of the constant to retrieve. It is undefined to retrieve a constant that does
	/// not exist.</param>
	/// <returns>The ShaderConstantInfo at index <paramref name="index"/></returns>
	const ShaderConstantInfo& getEvaluationlShaderConstant(uint32_t index) const { return _shaderConstTessEval[index]; }

	/// <summary>Return all evaluationshader constants</summary>
	/// <returns>const ShaderConstantInfo*</returns>
	const ShaderConstantInfo* getAllEvaluationShaderConstants() const { return _shaderConstTessEval.data(); }

	/// <summary>Return number of evaluatinon shader constants</summary>
	/// <returns>The number of evaluatinon shader constants</returns>
	uint32_t getNumEvaluatinonShaderConstants() const { return (uint32_t)_shaderConstTessEval.size(); }

	/// <summary>Get evaluation shader entry point</summary>
	/// <returns>const char*</returns>
	const char* getEvaluationShaderEntryPoint() const { return _evalShaderEntryPoint.c_str(); }

	/// <summary>Get control shader entry point</summary>
	/// <returns>const char*</returns>
	const char* getControlShaderEntryPoint() const { return _controlShaderEntryPoint.c_str(); }
};
} // namespace pvrvk
