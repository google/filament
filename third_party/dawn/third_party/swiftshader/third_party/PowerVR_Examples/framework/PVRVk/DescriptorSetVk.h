/*!
\brief The DescriptorSet class, representing a "directory" of shader-accessible objects
like Buffers and Images
\file PVRVk/DescriptorSetVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"
#include "PVRVk/PipelineConfigVk.h"
#include "PVRVk/BufferVk.h"

namespace pvrvk {
/// <summary>Contains all information required to create a Descriptor Set Layout. This is the number of Textures,
/// Samplers, Uniform Buffer Objects, and Shader Storage Buffer Objects bound for any shader stage.</summary>
struct DescriptorSetLayoutCreateInfo
{
public:
	struct DescriptorSetLayoutBinding
	{
		uint16_t binding;
		pvrvk::DescriptorType descriptorType;
		uint16_t descriptorCount;
		pvrvk::ShaderStageFlags stageFlags;
		Sampler immutableSampler;
		DescriptorSetLayoutBinding() : descriptorCount(1), stageFlags(pvrvk::ShaderStageFlags::e_ALL), descriptorType(pvrvk::DescriptorType::e_MAX_ENUM) {}
		DescriptorSetLayoutBinding(uint16_t bindIndex, pvrvk::DescriptorType descType, uint16_t descriptorCount = 1,
			pvrvk::ShaderStageFlags stageFlags = pvrvk::ShaderStageFlags::e_ALL, const Sampler& immutableSampler = Sampler())
			: binding(bindIndex), descriptorType(descType), descriptorCount(descriptorCount), stageFlags(stageFlags), immutableSampler(immutableSampler)
		{}

		bool operator==(const DescriptorSetLayoutBinding& rhs) const
		{
			return binding == rhs.binding && descriptorType == rhs.descriptorType && descriptorCount == rhs.descriptorCount && stageFlags == rhs.stageFlags;
		}

		bool operator!=(const DescriptorSetLayoutBinding& rhs) const { return !(*this == rhs); }
	};

private:
	std::vector<DescriptorSetLayoutBinding> descLayoutInfo;

public:
	DescriptorSetLayoutCreateInfo() = default;
	DescriptorSetLayoutCreateInfo(std::initializer_list<DescriptorSetLayoutBinding> bindingList)
	{
		for (auto& it : bindingList) { setBinding(it); }
	}

	DescriptorSetLayoutCreateInfo& setBinding(const DescriptorSetLayoutBinding& layoutBinding)
	{
		auto it =
			std::find_if(descLayoutInfo.begin(), descLayoutInfo.end(), [&layoutBinding](const DescriptorSetLayoutBinding& info) { return info.binding == layoutBinding.binding; });
		if (it != descLayoutInfo.end()) { (*it) = layoutBinding; }
		else
		{
			descLayoutInfo.emplace_back(layoutBinding);
		}
		return *this;
	}

	/// <summary>Set the buffer binding of Descriptor Objects in the specified shader stages.</summary>
	/// <param name="binding">The index to which the binding will be added</param>
	/// <param name="descriptorType">The type of descriptor</param>
	/// <param name="descriptorCount">The number of descriptors to add starting at that index</param>
	/// <param name="stageFlags">The shader stages for which the number of bindings is set to (count)</param>
	/// <param name="immutableSampler">If an immutable sampler is set, pass it here. See vulkan spec</param>
	/// <returns>This object (allows chaining of calls)</returns>
	DescriptorSetLayoutCreateInfo& setBinding(uint16_t binding, pvrvk::DescriptorType descriptorType, uint16_t descriptorCount = 1,
		pvrvk::ShaderStageFlags stageFlags = pvrvk::ShaderStageFlags::e_ALL, Sampler immutableSampler = Sampler())
	{
		return setBinding(DescriptorSetLayoutBinding(binding, descriptorType, descriptorCount, stageFlags, immutableSampler));
	}

	/// <summary>Clear all entries</summary>
	/// <returns>Return this for chaining</returns>
	DescriptorSetLayoutCreateInfo& clear()
	{
		descLayoutInfo.clear();
		return *this;
	}

	/// <summary>Return the number of images in this object</summary>
	/// <returns>the number of images in this object</returns>
	uint16_t getNumBindings() const { return static_cast<uint16_t>(descLayoutInfo.size()); }

	/// <summary>Equality operator. Does deep comparison of the contents.</summary>
	/// <param name="rhs">The right-hand side argument of the operator.</param>
	/// <returns>True if the layouts have identical bindings</returns>
	bool operator==(const DescriptorSetLayoutCreateInfo& rhs) const
	{
		if (getNumBindings() != rhs.getNumBindings()) { return false; }
		for (uint32_t i = 0; i < getNumBindings(); ++i)
		{
			if (descLayoutInfo[i] != rhs.descLayoutInfo[i]) { return false; }
		}
		return true;
	}

	/// <summary>Get descriptor binding</summary>
	/// <param name="bindingId">Binding Index</param>
	/// <returns>The binding layout object (DescriptorBindingLayout)</returns>
	const DescriptorSetLayoutBinding* getBinding(uint16_t bindingId) const
	{
		auto it = std::find_if(descLayoutInfo.begin(), descLayoutInfo.end(), [&](const DescriptorSetLayoutBinding& info) { return info.binding == bindingId; });
		if (it != descLayoutInfo.end()) { return &(*it); }
		return nullptr;
	}

	/// <summary>Get all layout bindings</summary>
	/// <returns>const DescriptorSetLayoutBinding*</returns>
	const DescriptorSetLayoutBinding* getAllBindings() const { return descLayoutInfo.data(); }

private:
	friend class ::pvrvk::impl::DescriptorSetLayout_;
	friend struct ::pvrvk::WriteDescriptorSet;
};

namespace impl {
/// <summary>Constructor. . Vulkan implementation of a DescriptorSet.</summary>
class DescriptorSetLayout_ : public PVRVkDeviceObjectBase<VkDescriptorSetLayout, ObjectType::e_DESCRIPTOR_SET_LAYOUT>, public DeviceObjectDebugUtils<DescriptorSetLayout_>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() = default;
		friend class DescriptorSetLayout_;
	};

	static DescriptorSetLayout constructShared(const DeviceWeakPtr& device, const DescriptorSetLayoutCreateInfo& createInfo)
	{
		return std::make_shared<DescriptorSetLayout_>(make_shared_enabler{}, device, createInfo);
	}

	DescriptorSetLayoutCreateInfo _createInfo;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(DescriptorSetLayout_)
	~DescriptorSetLayout_();
	DescriptorSetLayout_(make_shared_enabler, const DeviceWeakPtr& device, const DescriptorSetLayoutCreateInfo& createInfo);
	//!\endcond

	/// <summary>Get the DescriptorSetCreateInfo object that was used to create this layout.</summary>
	/// <returns>The DescriptorSetCreateInfo object that was used to create this layout.</returns>
	const DescriptorSetLayoutCreateInfo& getCreateInfo() const { return _createInfo; }

	/// <summary>Clear the descriptor set layout create param list.</summary>
	void clearCreateInfo() { _createInfo.clear(); }
};

/// <summary>Internal class</summary>
template<typename T, uint32_t ArraySize>
class DescriptorStore
{
	//!\cond NO_DOXYGEN
public:
	DescriptorStore() : _ptr(_tArray), _numItems(0) {}

	// Copy Constructor
	DescriptorStore(const DescriptorStore& descStore) : _numItems(descStore._numItems), _tVector(descStore._tVector)
	{
		// Copy _tArray elements
		std::copy(std::begin(descStore._tArray), std::end(descStore._tArray), _tArray);
		// Point towards _tVector.data() if _tVector is being used else point towards _tArray
		_ptr = !_tVector.empty() ? _tVector.data() : _tArray;
	}

	// Destructor
	~DescriptorStore() = default;

	// Add swap functionality
	friend void swap(DescriptorStore& first, DescriptorStore& second)
	{
		using std::swap;

		// IF second._ptr is pointing to the array then point to MY array
		// If second._ptr is pointing to the vector then point to seconds vector
		T* tempPtr1 = second._ptr == second._tArray ? first._tArray : second._tVector.data();
		first._ptr = tempPtr1;

		// IF first._ptr is pointing to the array then point to MY array
		// If first._ptr is pointing to the vector then point to firsts vector
		T* tempPtr2 = first._ptr == first._tArray ? second._tArray : first._tVector.data();
		second._ptr = tempPtr2;

		swap(first._numItems, second._numItems);
		swap(first._tVector, second._tVector);
		swap(first._tArray, second._tArray);
	}

	// Assignment Operator
	DescriptorStore& operator=(DescriptorStore other)
	{
		swap(*this, other);
		return *this;
	}

	// Move Assignment Operator
	DescriptorStore& operator=(DescriptorStore&& other)
	{
		swap(other);
		return *this;
	}

	// Move constructor
	DescriptorStore(DescriptorStore&& other) noexcept : DescriptorStore() { swap(*this, other); }

	void clear()
	{
		// Reset each of the items
		std::fill(std::begin(_tArray), std::end(_tArray), T());
		_tVector.clear();
		_ptr = _tArray;
		_numItems = 0;
	}

	void set(uint32_t index, const T& obj)
	{
		// Move to overflow when the number of items reaches the limit of the limited storage array
		if (_tVector.empty() && index >= ArraySize)
		{
			// Move the limited storage array into the vector
			moveToOverFlow();
		}
		// Determine whether we should grow the container
		if (!_tVector.empty() && index >= _tVector.size())
		{
			// Grow by ArraySize to reduce the number of times we call resize
			_tVector.resize(index + ArraySize);
			_ptr = _tVector.data();
		}

		_numItems = std::max(_numItems, index + 1);

		_ptr[index] = obj;

		// Ensure _ptr is pointing at either _tArray or _tVector.data() depending on the index currently being set
		assert(index < ArraySize ? _ptr == _tArray : true && "Pointer must be pointing at _tArray");
		assert(index >= ArraySize ? _ptr == _tVector.data() : true && "Pointer must be pointing at _tVector.data()");
	}

	uint32_t size() const { return _numItems; }

	const T* begin() const { return _ptr; }
	const T* end() const { return _ptr + _numItems; }

	const T& operator[](uint32_t index) const { return get(index); }

	const T& get(uint32_t index) const { return _ptr[index]; }

private:
	void moveToOverFlow()
	{
		_tVector.reserve(ArraySize * 2);
		_tVector.assign(std::begin(_tArray), std::end(_tArray));
		// The pointer now points to the head of the vector
		_ptr = _tVector.data();
	}

	T _tArray[ArraySize];
	std::vector<T> _tVector;
	T* _ptr;
	uint32_t _numItems;
	//!\endcond
};
} // namespace impl

/// <summary>Descriptor Pool create parameter.</summary>
struct DescriptorPoolCreateInfo
{
private:
	std::array<pvrvk::DescriptorPoolSize, static_cast<uint32_t>(pvrvk::DescriptorType::e_RANGE_SIZE)> _descriptorPoolSizes;
	uint16_t _numDescriptors;
	uint16_t _maxSets;

public:
	/// <summary>Constructor</summary>
	DescriptorPoolCreateInfo() : _numDescriptors(0), _maxSets(200) {}

	/// <summary>Constructor</summary>
	/// <param name="maxSets">The maximum number of descriptor sets which can be allocated by this descriptor pool</param>
	/// <param name="combinedImageSamplers">The maximum number of combined image samplers descriptor types which can be used in descriptor sets allocated by this descriptor
	/// pool.</param> <param name="inputAttachments">The maximum number of input attachment descriptor types which can be used in descriptor sets allocated by this descriptor
	/// pool.</param> <param name="staticUbos">The maximum number of uniform buffer descriptor types which can be used in descriptor sets allocated by this descriptor
	/// pool.</param> <param name="dynamicUbos">The maximum number of dynamic uniform buffers descriptor types which can be used in descriptor sets allocated by this descriptor
	/// pool.</param> <param name="staticSsbos">The maximum number of storage buffer descriptor types which can be used in descriptor sets allocated by this descriptor
	/// pool.</param> <param name="dynamicSsbos">The maximum number of dynamic storage buffer descriptor types which can be used in descriptor sets allocated by this descriptor
	/// pool.</param>
	explicit DescriptorPoolCreateInfo(uint16_t maxSets, uint16_t combinedImageSamplers = 32, uint16_t inputAttachments = 0, uint16_t staticUbos = 32, uint16_t dynamicUbos = 32,
		uint16_t staticSsbos = 0, uint16_t dynamicSsbos = 0)
		: _numDescriptors(0), _maxSets(maxSets)
	{
		if (combinedImageSamplers != 0) { addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, combinedImageSamplers); }
		if (inputAttachments != 0) { addDescriptorInfo(pvrvk::DescriptorType::e_INPUT_ATTACHMENT, inputAttachments); }
		if (staticUbos != 0) { addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, staticUbos); }
		if (dynamicUbos != 0) { addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, dynamicUbos); }
		if (staticSsbos != 0) { addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_BUFFER, staticSsbos); }
		if (dynamicSsbos != 0) { addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_BUFFER_DYNAMIC, dynamicSsbos); }
	}

	explicit DescriptorPoolCreateInfo(std::initializer_list<pvrvk::DescriptorPoolSize> descriptorPoolSizes, uint16_t maxSets = 200) : _numDescriptors(0), _maxSets(maxSets)
	{
		for (auto& it : descriptorPoolSizes) { addDescriptorInfo(it); }
	}

	DescriptorPoolCreateInfo& addDescriptorInfo(const pvrvk::DescriptorPoolSize& descriptorPoolSize)
	{
		_descriptorPoolSizes[_numDescriptors] = descriptorPoolSize;
		_numDescriptors++;
		return *this;
	}

	/// <summary>Add the maximum number of the specified descriptor types that the pool will contain.</summary>
	/// <param name="descType">Descriptor type</param>
	/// <param name="count">Maximum number of descriptors of (type)</param>
	/// <returns>this (allow chaining)</returns>
	DescriptorPoolCreateInfo& addDescriptorInfo(pvrvk::DescriptorType descType, uint16_t count) { return addDescriptorInfo(pvrvk::DescriptorPoolSize(descType, count)); }

	/// <summary>Set the maximum number of descriptor sets.</summary>
	/// <param name="maxSets">The maximum number of descriptor sets</param>
	/// <returns>this (allow chaining)</returns>
	DescriptorPoolCreateInfo& setMaxDescriptorSets(uint16_t maxSets)
	{
		this->_maxSets = maxSets;
		return *this;
	}

	/// <summary>Get the number of allocations of a descriptor type is supported on this pool (const).</summary>
	/// <param name="descType">DescriptorType</param>
	/// <returns>Number of allocations.</returns>
	uint32_t getNumDescriptorTypes(pvrvk::DescriptorType descType) const
	{
		for (uint16_t i = 0; i < _numDescriptors; i++)
		{
			if (_descriptorPoolSizes[i].getType() == descType) { return _descriptorPoolSizes[i].getDescriptorCount(); }
		}
		return 0;
	}

	/// <summary>Get maximum sets supported on this pool.</summary>
	/// <returns>uint32_t</returns>
	uint32_t getMaxDescriptorSets() const { return _maxSets; }
};

/// <summary>This class contains all the information necessary to populate a Descriptor Set with the actual API
/// objects. Use with the method update of the DescriptorSet. Populate this object with actual Descriptor objects
/// (UBOs, textures etc).</summary>
struct DescriptorImageInfo
{
	Sampler sampler; //!< Sampler handle, and is used in descriptor updates for types pvrvk::DescriptorType::e_SAMPLER and pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER if the
					 //!< binding being updated does not use immutable samplers
	ImageView imageView; //!< Image view handle, and is used in descriptor updates for types pvrvk::DescriptorType::e_SAMPLED_IMAGE, pvrvk::DescriptorType::e_STORAGE_IMAGE,
						 //!< pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, and pvrvk::DescriptorType::e_INPUT_ATTACHMENT
	pvrvk::ImageLayout imageLayout; //!< Layout that the image subresources accessible from imageView will be in at the time this descriptor is accessed. imageLayout is used in
									//!< descriptor updates for types pvrvk::DescriptorType::e_SAMPLED_IMAGE, pvrvk::DescriptorType::e_STORAGE_IMAGE,
									//!< pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, and pvrvk::DescriptorType::e_INPUT_ATTACHMENT
	/// <summary>Constructor. Initalizes to pvrvk::ImageLayout::e_UNDEFINED</summary>
	DescriptorImageInfo() : imageLayout(pvrvk::ImageLayout::e_UNDEFINED) {}

	/// <summary>Constructor from a sampler object.</summary>
	/// <param name="sampler">Sampler handle, and is used in descriptor updates for types
	/// pvrvk::DescriptorType::e_SAMPLER and pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER if the binding being
	/// updated does not use immutable samplers</param>
	explicit DescriptorImageInfo(const Sampler& sampler) : sampler(sampler), imageLayout(pvrvk::ImageLayout::e_UNDEFINED) {}

	/// <summary>Constructor from all elements</summary>
	/// <param name="imageView">Image view handle, and is used in descriptor updates for types
	/// pvrvk::DescriptorType::e_SAMPLED_IMAGE, pvrvk::DescriptorType::e_STORAGE_IMAGE,
	/// pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, and pvrvk::DescriptorType::e_INPUT_ATTACHMENT</param>
	/// <param name="sampler">Sampler handle, and is used in descriptor updates for types
	/// pvrvk::DescriptorType::e_SAMPLER and pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER if the binding
	/// being updated does not use immutable samplers</param>
	/// <param name="imageLayout">Layout that the image subresources accessible from imageView
	/// will be in at the time this descriptor is accessed. imageLayout is used in descriptor
	/// updates for types pvrvk::DescriptorType::e_SAMPLED_IMAGE, pvrvk::DescriptorType::e_STORAGE_IMAGE,
	/// pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, and pvrvk::DescriptorType::e_INPUT_ATTACHMENT</param>
	DescriptorImageInfo(const ImageView& imageView, const Sampler& sampler, pvrvk::ImageLayout imageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)
		: sampler(sampler), imageView(imageView), imageLayout(imageLayout)
	{}

	/// <summary>Constructor from all elements except sampler</summary>
	/// <param name="imageView">Image view handle, and is used in descriptor updates for types
	/// pvrvk::DescriptorType::e_SAMPLED_IMAGE, pvrvk::DescriptorType::e_STORAGE_IMAGE,
	/// pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, and pvrvk::DescriptorType::e_INPUT_ATTACHMENT</param>
	/// <param name="imageLayout">Layout that the image subresources accessible from imageView
	/// will be in at the time this descriptor is accessed. imageLayout is used in descriptor
	/// updates for types pvrvk::DescriptorType::e_SAMPLED_IMAGE, pvrvk::DescriptorType::e_STORAGE_IMAGE,
	/// pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, and pvrvk::DescriptorType::e_INPUT_ATTACHMENT</param>
	DescriptorImageInfo(const ImageView& imageView, pvrvk::ImageLayout imageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)
		: imageView(imageView), imageLayout(imageLayout)
	{}
};
/// <summary>A struct describing a descriptor buffer binding</summary>
struct DescriptorBufferInfo
{
	Buffer buffer; //!< The buffer object
	VkDeviceSize offset; //!< The offset into the buffer
	VkDeviceSize range; //!< The range of the buffer
	/// <summary>Constructor. Zero initialize.</summary>
	DescriptorBufferInfo() : offset(0), range(0) {}
	/// <summary>Constructor. Individual elements.</summary>
	/// <param name="buffer">The referenced buffer</param>
	/// <param name="offset">An offset into the buffer, if the buffer is piecemally bound</param>
	/// <param name="range">A range of the buffer, if the buffer is piecemally bound</param>
	DescriptorBufferInfo(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range) : buffer(buffer), offset(offset), range(range) {}
};

/// <summary>Contains information for an initialization/update of a descriptor set. This class
/// contains both the update AND the descriptor set to be updated, so that multiple descriptor
/// sets can be updated in a single call to Device::updateDescriptorSets</summary>
struct WriteDescriptorSet
{
	/// <summary>Constructor. Undefined values</summary>
	WriteDescriptorSet() : _infos() {}

	/// <summary>Constructor. Initializes with a specified descriptor into a set</summary>
	/// <param name="descType">The descriptor type of this write</param>
	/// <param name="descSet">The descriptor set which to update</param>
	/// <param name="dstBinding">The binding to update</param>
	/// <param name="dstArrayElement">If the destination is an array, the array index to update</param>
	WriteDescriptorSet(pvrvk::DescriptorType descType, DescriptorSet descSet, uint32_t dstBinding = 0, uint32_t dstArrayElement = 0)
		: _descType(descType), _descSet(descSet), _dstBinding(dstBinding), _dstArrayElement(dstArrayElement), _infos()
	{
		set(descType, descSet, dstBinding, dstArrayElement);
	}

	/// <summary>Set the descriptor type</summary>
	/// <param name="descType">The descriptor type</param>
	/// <returns>This object (allow chaining)</returns>
	WriteDescriptorSet& setDescriptorType(pvrvk::DescriptorType descType)
	{
		_descType = descType;
		if ((_descType >= pvrvk::DescriptorType::e_SAMPLER && _descType <= pvrvk::DescriptorType::e_STORAGE_IMAGE) || _descType == pvrvk::DescriptorType::e_INPUT_ATTACHMENT)
		{ _infoType = InfoType::ImageInfo; }
		else if (_descType >= pvrvk::DescriptorType::e_UNIFORM_BUFFER && _descType <= pvrvk::DescriptorType::e_STORAGE_BUFFER_DYNAMIC)
		{
			_infoType = InfoType::BufferInfo;
		}
		else if (_descType == pvrvk::DescriptorType::e_UNIFORM_TEXEL_BUFFER || _descType == pvrvk::DescriptorType::e_STORAGE_TEXEL_BUFFER)
		{
			_infoType = InfoType::TexelBufferView;
		}
		else
		{
			assert(false && "Cannot resolve Info type from descriptor type");
		}
		return *this;
	}

	/// <summary>Set the descriptor set</summary>
	/// <param name="descriptorSet">The descriptor set that will be updated</param>
	/// <returns>This object (allow chaining)</returns>
	WriteDescriptorSet& setDescriptorSet(DescriptorSet& descriptorSet)
	{
		_descSet = descriptorSet;
		return *this;
	}

	/// <summary>Set the destination binding</summary>
	/// <param name="binding">The binding into the descriptor set</param>
	/// <returns>This object (allow chaining)</returns>
	WriteDescriptorSet& setDestBinding(uint32_t binding)
	{
		_dstBinding = binding;
		return *this;
	}

	/// <summary>Set destination array element</summary>
	/// <param name="arrayElement">Array element.</param>
	/// <returns>This object (allow chaining)</returns>
	WriteDescriptorSet& setDestArrayElement(uint32_t arrayElement)
	{
		_dstArrayElement = arrayElement;
		return *this;
	}

	/// <summary>Sets all the data of this write</summary>
	/// <param name="newDescType">The new descriptor type</param>
	/// <param name="descSet">The new descriptor set</param>
	/// <param name="dstBinding">The new destination Binding in spir-v</param>
	/// <param name="dstArrayElement">If the target descriptor is an array,
	/// the index to update.</param>
	/// <returns>This object (allow chaining)</returns>
	WriteDescriptorSet& set(pvrvk::DescriptorType newDescType, const DescriptorSet& descSet, uint32_t dstBinding = 0, uint32_t dstArrayElement = 0)
	{
		setDescriptorType(newDescType);
		_descSet = descSet;
		_dstBinding = dstBinding;
		_dstArrayElement = dstArrayElement;
		_infos.clear();
		return *this;
	}

	/// <summary>If the target descriptor is an image, set the image info image info</summary>
	/// <param name="arrayIndex">The target array index</param>
	/// <param name="imageInfo">The image info to set</param>
	/// <returns>This object (allow chaining)</returns>
	WriteDescriptorSet& setImageInfo(uint32_t arrayIndex, const DescriptorImageInfo& imageInfo)
	{
#ifdef DEBUG
		// VALIDATE DESCRIPTOR TYPE
		assert(((_descType >= pvrvk::DescriptorType::e_SAMPLER) && (_descType <= pvrvk::DescriptorType::e_STORAGE_IMAGE)) || (_descType == pvrvk::DescriptorType::e_INPUT_ATTACHMENT));
		if (_descType == pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER) { assert(imageInfo.sampler && imageInfo.imageView && "Sampler and ImageView must be valid"); }
#endif
		DescriptorInfos info;
		info.imageInfo = imageInfo;
		_infos.set(arrayIndex, info);
		return *this;
	}

	/// <summary>If the target descriptor is a buffer, sets the buffer info</summary>
	/// <param name="arrayIndex">If an array, the target array index</param>
	/// <param name="bufferInfo">The buffer info to set</param>
	/// <returns>This object(allow chaining)</returns>
	WriteDescriptorSet& setBufferInfo(uint32_t arrayIndex, const DescriptorBufferInfo& bufferInfo)
	{
#ifdef DEBUG
		assert(_descType >= pvrvk::DescriptorType::e_UNIFORM_BUFFER && _descType <= pvrvk::DescriptorType::e_STORAGE_BUFFER_DYNAMIC);
		assert(bufferInfo.buffer && "Buffer must be valid");
#endif
		DescriptorInfos info;
		info.bufferInfo = bufferInfo;
		_infos.set(arrayIndex, info);
		return *this;
	}

	/// <summary>If the target descriptor is a Texel buffer, set the Texel Buffer info</summary>
	/// <param name="arrayIndex">If an array, the target array index</param>
	/// <param name="bufferView">The Texel Buffer view to set</param>
	/// <returns>This object(allow chaining)</returns>
	WriteDescriptorSet& setTexelBufferInfo(uint32_t arrayIndex, const BufferView& bufferView)
	{
#ifdef DEBUG
		assert(_descType >= pvrvk::DescriptorType::e_UNIFORM_TEXEL_BUFFER && _descType <= pvrvk::DescriptorType::e_STORAGE_TEXEL_BUFFER);
		assert(bufferView && "Texel BufferView must be valid");
#endif
		DescriptorInfos info;
		info.texelBuffer = bufferView;
		_infos.set(arrayIndex, info);
		return *this;
	}

	/// <summary>Clear all info of this object</summary>
	/// <returns>This object(allow chaining)</returns>
	WriteDescriptorSet& clearAllInfos()
	{
		_infos.clear();
		return *this;
	}

	/// <summary>Get the number of descriptors being updated</summary>
	/// <returns>The the number of descriptors</returns>
	uint32_t getNumDescriptors() const { return _infos.size(); }

	/// <summary>Get descriptor type</summary>
	/// <returns>The descriptor type</returns>
	pvrvk::DescriptorType getDescriptorType() const { return _descType; }

	/// <summary>Get descriptor set to update</summary>
	/// <returns>The descriptor set</returns>
	DescriptorSet getDescriptorSet() { return _descSet; }

	/// <summary>Get the descriptor set to update</summary>
	/// <returns>The descriptor set</returns>
	const DescriptorSet& getDescriptorSet() const { return _descSet; }

	/// <summary>If an array, get the destination array element</summary>
	/// <returns>The destination array element</returns>
	uint32_t getDestArrayElement() const { return _dstArrayElement; }

	/// <summary>Get the destination binding indiex</summary>
	/// <returns>The destination binding index</returns>
	uint32_t getDestBinding() const { return _dstBinding; }

private:
	friend class ::pvrvk::impl::Device_;

	pvrvk::DescriptorType _descType;
	DescriptorSet _descSet;
	uint32_t _dstBinding;
	uint32_t _dstArrayElement;
	struct DescriptorInfos
	{
		DescriptorImageInfo imageInfo;
		DescriptorBufferInfo bufferInfo;
		BufferView texelBuffer;

		DescriptorInfos() = default;
		bool isValid() const { return imageInfo.imageView || imageInfo.sampler || bufferInfo.buffer || texelBuffer; }
	};

	impl::DescriptorStore<DescriptorInfos, 4> _infos;

	enum InfoType
	{
		ImageInfo,
		BufferInfo,
		TexelBufferView
	};
	InfoType _infoType;

	// CALL THIS ONE FROM THE DEVICE - CPU SIDE KEEPING ALIVE OF THE DESCRIPTORS IN THIS SET
	void updateKeepAliveIntoDestinationDescriptorSet() const;
};

/// <summary>Copy descriptor set</summary>
struct CopyDescriptorSet
{
	DescriptorSet srcSet; //!< Source descriptor set to copy from
	uint32_t srcBinding; //!< source binding to copy
	uint32_t srcArrayElement; //!< source array element to copy from
	DescriptorSet dstSet; //!< Destination descriptor set
	uint32_t dstBinding; //!< Destination binding to copy in to.
	uint32_t dstArrayElement; //!< Destination array element to copy into
	uint32_t descriptorCount; //!< Number of descriptor to copy
};

namespace impl {
/// <summary>A descriptor pool - an object used to allocate (and recycle) Descriptor Sets.</summary>
class DescriptorPool_ : public PVRVkDeviceObjectBase<VkDescriptorPool, ObjectType::e_DESCRIPTOR_POOL>,
						public DeviceObjectDebugUtils<DescriptorPool_>,
						public std::enable_shared_from_this<DescriptorPool_>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() = default;
		friend class DescriptorPool_;
	};

	static DescriptorPool constructShared(const DeviceWeakPtr& device, const DescriptorPoolCreateInfo& createInfo)
	{
		return std::make_shared<DescriptorPool_>(make_shared_enabler{}, device, createInfo);
	}

	DescriptorPoolCreateInfo _createInfo;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(DescriptorPool_)
	~DescriptorPool_();
	DescriptorPool_(make_shared_enabler, const DeviceWeakPtr& device, const DescriptorPoolCreateInfo& createInfo);
	//!\endcond

	/// <summary>Allocate descriptor set</summary>
	/// <param name="layout">Descriptor set layout</param>
	/// <returns>Return DescriptorSet else null if fails.</returns>
	DescriptorSet allocateDescriptorSet(const DescriptorSetLayout& layout);

	/// <summary>Return the descriptor pool create info from which this descriptor pool was allocated</summary>
	/// <returns>The descriptor pool create info</returns>
	const DescriptorPoolCreateInfo& getCreateInfo() const { return _createInfo; }
};

/// <summary>Vulkan implementation of a DescriptorSet.</summary>
class DescriptorSet_ : public PVRVkDeviceObjectBase<VkDescriptorSet, ObjectType::e_DESCRIPTOR_SET>, public DeviceObjectDebugUtils<DescriptorSet_>
{
private:
	friend struct ::pvrvk::WriteDescriptorSet;
	friend class DescriptorPool_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() = default;
		friend class DescriptorSet_;
	};

	static DescriptorSet constructShared(const DescriptorSetLayout& descSetLayout, DescriptorPool& pool)
	{
		return std::make_shared<DescriptorSet_>(make_shared_enabler{}, descSetLayout, pool);
	}

	mutable std::vector<std::vector<std::shared_ptr<void> /**/> /**/> _keepAlive;
	DescriptorSetLayout _descSetLayout;
	DescriptorPool _descPool;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(DescriptorSet_)

	DescriptorSet_(make_shared_enabler, const DescriptorSetLayout& descSetLayout, DescriptorPool& pool)
		: PVRVkDeviceObjectBase(pool->getDevice()), DeviceObjectDebugUtils(), _descSetLayout(descSetLayout), _descPool(pool)
	{
		// Create the Vulkan VkDescriptorSetAllocateInfo structure
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = static_cast<VkStructureType>(pvrvk::StructureType::e_DESCRIPTOR_SET_ALLOCATE_INFO);
		allocInfo.pSetLayouts = &getDescriptorSetLayout()->getVkHandle();
		allocInfo.descriptorSetCount = 1;
		allocInfo.descriptorPool = getDescriptorPool()->getVkHandle();

		// For appropriate smart reference counting we need to keep alive the bindings
		const auto& allBindings = _descSetLayout->getCreateInfo().getAllBindings();
		uint16_t maxBinding = 0;
		uint32_t i = 0, size = _descSetLayout->getCreateInfo().getNumBindings();
		// Loop through the descriptor set bindings and determine the maximum binding
		for (; i < size; ++i) { maxBinding = std::max(allBindings[i].binding, maxBinding); }
		// Use the maximum binding + 1 to resize the keepAlive array
		_keepAlive.resize(maxBinding + 1u);
		// Now use the descriptor count for each descriptor binding to determine the total number of entries
		for (i = 0; i < size; ++i)
		{
			auto& entry = allBindings[i];
			auto& aliveEntry = _keepAlive[entry.binding];
			aliveEntry.resize(entry.descriptorCount);
		}
		vkThrowIfFailed(getDevice()->getVkBindings().vkAllocateDescriptorSets(_descSetLayout->getDevice()->getVkHandle(), &allocInfo, &_vkHandle), "Allocate Descriptor Set failed");
	}

	~DescriptorSet_()
	{
		_keepAlive.clear();
		if (getVkHandle() != VK_NULL_HANDLE)
		{
			if (getDescriptorPool()->getDevice())
			{
				getDevice()->getVkBindings().vkFreeDescriptorSets(getDescriptorPool()->getDevice()->getVkHandle(), getDescriptorPool()->getVkHandle(), 1, &getVkHandle());
				_vkHandle = VK_NULL_HANDLE;
			}
			else
			{
				reportDestroyedAfterDevice();
			}
			_descSetLayout.reset();
		}
	}
	//!\endcond

	/// <summary>Return the layout of this DescriptorSet.</summary>
	/// <returns>This DescriptorSet's DescriptorSetLayout</returns>
	const DescriptorSetLayout& getDescriptorSetLayout() const { return _descSetLayout; }

	/// <summary>Return the descriptor pool from which this descriptor set was allocated</summary>
	/// <returns>The descriptor pool</returns>
	const DescriptorPool& getDescriptorPool() const { return _descPool; }

	/// <summary>Return the descriptor pool from which this descriptor set was allocated</summary>
	/// <returns>The descriptor pool</returns>
	DescriptorPool& getDescriptorPool() { return _descPool; }
};

} // namespace impl

// For smart pointer reference counting we need to keep alive the descriptor set binding entries to do this we place them in an array kept alive by the DescriptorSet itself
// This means that the caller application can let resources go out scope and the Descriptor set "keepAlive" array will keep them alive as long as needed
inline void WriteDescriptorSet::updateKeepAliveIntoDestinationDescriptorSet() const
{
	// Get the keep alive entry for the current binding
	auto& keepAlive = getDescriptorSet()->_keepAlive[this->_dstBinding];

	// Handle BufferInfo entries
	if (_infoType == InfoType::BufferInfo)
	{
		for (uint32_t i = 0; i < _infos.size(); ++i)
		{
			// Ensure the into entry is valid
			if (_infos[i].isValid()) { keepAlive[i] = _infos[i].bufferInfo.buffer; }
		}
	}
	// Handle ImageInfo entries
	else if (_infoType == InfoType::ImageInfo)
	{
		for (uint32_t i = 0; i < _infos.size(); ++i)
		{
			// Ensure the into entry is valid
			if (_infos[i].isValid())
			{
				auto newpair = std::make_shared<std::pair<Sampler, ImageView> /**/>();
				newpair->first = _infos[i].imageInfo.sampler;
				newpair->second = _infos[i].imageInfo.imageView;

				keepAlive[i] = newpair;
			}
		}
	}
	// Handle TexelBufferView entries
	else if (_infoType == InfoType::TexelBufferView)
	{
		for (uint32_t i = 0; i < _infos.size(); ++i)
		{
			// Ensure the into entry is valid
			if (_infos[i].isValid()) { keepAlive[i] = _infos[i].texelBuffer; }
		}
	}
}
} // namespace pvrvk
