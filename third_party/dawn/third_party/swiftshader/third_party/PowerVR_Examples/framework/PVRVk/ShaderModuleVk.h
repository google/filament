/*!
\brief The PVRVk ShaderModule class.
\file PVRVk/ShaderModuleVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
/// <summary>ShaderModule creation descriptor.</summary>
struct ShaderModuleCreateInfo
{
public:
	/// <summary>Default Constructor</summary>
	/// <param name="flags">A set of pvrvk::ShaderModuleCreateFlags defining how the ShaderModule will be created</param>
	inline ShaderModuleCreateInfo(ShaderModuleCreateFlags flags = ShaderModuleCreateFlags::e_NONE) : _flags(flags) {}

	/// <summary>Constructor</summary>
	/// <param name="shaderSources">Points to code that is used to create the shader module</param>
	/// <param name="flags">A set of pvrvk::ShaderModuleCreateFlags defining how the ShaderModule will be created</param>
	inline ShaderModuleCreateInfo(const std::vector<uint32_t>& shaderSources, ShaderModuleCreateFlags flags = ShaderModuleCreateFlags::e_NONE)
		: _flags(flags), _shaderSources(shaderSources)
	{
		if (_shaderSources.empty()) { throw ErrorValidationFailedEXT("Attempted to create ShaderModuleCreateInfo with empty shader source."); }
	}

	/// <summary>Constructor</summary>
	/// <param name="shaderSources">Points to code that is used to create the shader module</param>
	/// <param name="shaderSourcesSize">The size of the shader sources in bytes</param>
	/// <param name="flags">A set of pvrvk::ShaderModuleCreateFlags defining how the ShaderModule will be created</param>
	inline ShaderModuleCreateInfo(const uint32_t* shaderSources, uint32_t shaderSourcesSize, ShaderModuleCreateFlags flags = ShaderModuleCreateFlags::e_NONE)
		: _flags(flags), _shaderSources(shaderSources, shaderSources + shaderSourcesSize)
	{
		if (_shaderSources.empty()) { throw ErrorValidationFailedEXT("Attempted to create ShaderModuleCreateInfo with empty shader source."); }
	}

	/// <summary>Get the ShaderModule creation flags</summary>
	/// <returns>The set of ShaderModule creation flags</returns>
	inline ShaderModuleCreateFlags getFlags() const { return _flags; }
	/// <summary>Set the ShaderModule creation flags</summary>
	/// <param name="flags">The ShaderModule creation flags</param>
	inline void setFlags(ShaderModuleCreateFlags flags) { this->_flags = flags; }
	/// <summary>Get the size of the shader sources</summary>
	/// <returns>The size of shader sources</returns>
	inline uint32_t getCodeSize() const { return static_cast<uint32_t>(_shaderSources.size() * 4.0); }
	/// <summary>Get the shader sources</summary>
	/// <returns>The set of shader sources</returns>
	inline const std::vector<uint32_t>& getShaderSources() const { return _shaderSources; }
	/// <summary>Set the shader sources to use for creating a shader module</summary>
	/// <param name="shaderSources">Shader sources to use for creating a shader module</param>
	inline void setShaderSources(const std::vector<uint32_t>& shaderSources) { this->_shaderSources = shaderSources; }

private:
	/// <summary>Flags to use for creating the ShaderModule</summary>
	ShaderModuleCreateFlags _flags;
	/// <summary>The shader module sources</summary>
	std::vector<uint32_t> _shaderSources;
};
namespace impl {
/// <summary>Vulkan shader module wrapper</summary>
class ShaderModule_ : public PVRVkDeviceObjectBase<VkShaderModule, ObjectType::e_SHADER_MODULE>, public DeviceObjectDebugUtils<ShaderModule_>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class ShaderModule_;
	};

	static ShaderModule constructShared(const DeviceWeakPtr& device, const ShaderModuleCreateInfo& createInfo)
	{
		return std::make_shared<ShaderModule_>(make_shared_enabler{}, device, createInfo);
	}

	/// <summary>Creation information used when creating the shader module.</summary>
	ShaderModuleCreateInfo _createInfo;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(ShaderModule_)
	ShaderModule_(make_shared_enabler, const DeviceWeakPtr& device, const ShaderModuleCreateInfo& createInfo);

	~ShaderModule_();
	//!\endcond

	/// <summary>Get the ShaderModule creation flags</summary>
	/// <returns>The set of ShaderModule creation flags</returns>
	inline ShaderModuleCreateFlags getFlags() const { return _createInfo.getFlags(); }
	/// <summary>Get the size of the shader sources</summary>
	/// <returns>The size of shader sources</returns>
	inline uint32_t getCodeSize() const { return _createInfo.getCodeSize(); }
	/// <summary>Get the shader sources</summary>
	/// <returns>The set of shader sources</returns>
	inline const std::vector<uint32_t>& getShaderSources() const { return _createInfo.getShaderSources(); }
	/// <summary>Get this shader modules's create flags</summary>
	/// <returns>ShaderModuleCreateInfo</returns>
	ShaderModuleCreateInfo getCreateInfo() const { return _createInfo; }
};
} // namespace impl
} // namespace pvrvk
