/*!
\brief Contains implementations of functions for the UIRenderer class.
\file PVRUtils/Vulkan/UIRendererVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN

#include "PVRCore/stream/BufferStream.h"
#include "PVRVk/ImageVk.h"
#include "PVRVk/SamplerVk.h"
#include "PVRVk/PipelineLayoutVk.h"
#include "PVRVk/DescriptorSetVk.h"
#include "PVRUtils/Vulkan/UIRendererVk.h"
#include "PVRUtils/ArialBoldFont.h"
#include "PVRUtils/PowerVRLogo.h"
#include "PVRUtils/Vulkan/UIRendererVertShader.h"
#include "PVRUtils/Vulkan/UIRendererFragShader.h"
#include "PVRUtils/Vulkan/HelperVk.h"
using std::map;
using std::vector;
using namespace pvrvk;
const uint32_t MaxDescUbo = 200;
const uint32_t MaxCombinedImageSampler = 200;
namespace pvr {
namespace ui {
const glm::vec2 BaseScreenDim(640, 480);
namespace {
enum class MaterialBufferElement
{
	UVMtx,
	Color,
	AlphaMode
};

enum class UboDescSetBindingId
{
	MVP,
	Material
};
} // namespace

void UIRenderer::initCreatePipeline(bool isFrameBufferSrgb)
{
	GraphicsPipelineCreateInfo pipelineDesc;
	PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(_texDescLayout);

	if (_uboMvpDescLayout) { pipeLayoutInfo.addDescSetLayout(_uboMvpDescLayout); }
	if (_uboMaterialLayout) { pipeLayoutInfo.addDescSetLayout(_uboMaterialLayout); }

	Device deviceSharedPtr = _device.lock();

	_pipelineLayout = deviceSharedPtr->createPipelineLayout(pipeLayoutInfo);

	_pipelineLayout->setObjectName("PVRUtilsVk::UIRenderer::UI Graphics PipelineLayout");
	pipelineDesc.pipelineLayout = _pipelineLayout;
	// Text_ pipe
	ShaderModule vs;
	ShaderModule fs;

	vs = deviceSharedPtr->createShaderModule(ShaderModuleCreateInfo(BufferStream("", spv_UIRendererVertShader, sizeof(spv_UIRendererVertShader)).readToEnd<uint32_t>()));
	fs = deviceSharedPtr->createShaderModule(ShaderModuleCreateInfo(BufferStream("", spv_UIRendererFragShader, sizeof(spv_UIRendererFragShader)).readToEnd<uint32_t>()));

	pipelineDesc.vertexShader.setShader(vs);
	pipelineDesc.fragmentShader.setShader(fs);
	VertexInputAttributeDescription posAttrib(0, 0, pvrvk::Format::e_R32G32B32A32_SFLOAT, 0);
	VertexInputAttributeDescription texAttrib(1, 0, pvrvk::Format::e_R32G32_SFLOAT, sizeof(float) * 4);
	pipelineDesc.vertexInput.addInputBinding(VertexInputBindingDescription(0, sizeof(float) * 6, pvrvk::VertexInputRate::e_VERTEX)).addInputAttribute(posAttrib).addInputAttribute(texAttrib);

	PipelineColorBlendAttachmentState attachmentState(true, pvrvk::BlendFactor::e_SRC_ALPHA, pvrvk::BlendFactor::e_ONE_MINUS_SRC_ALPHA, pvrvk::BlendOp::e_ADD,
		pvrvk::BlendFactor::e_ZERO, pvrvk::BlendFactor::e_ONE, pvrvk::BlendOp::e_ADD,
		pvrvk::ColorComponentFlags::e_R_BIT | pvrvk::ColorComponentFlags::e_G_BIT | pvrvk::ColorComponentFlags::e_B_BIT | pvrvk::ColorComponentFlags::e_A_BIT);

	pipelineDesc.colorBlend.setAttachmentState(0, attachmentState);
	pipelineDesc.depthStencil.enableDepthTest(false).enableDepthWrite(false);
	pipelineDesc.rasterizer.setCullMode(pvrvk::CullModeFlags::e_NONE);
	pipelineDesc.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);
	pipelineDesc.viewport.setViewportAndScissor(0, Viewport(0, 0, _screenDimensions.x, _screenDimensions.y),
		Rect2D(pvrvk::Offset2D(0, 0), pvrvk::Extent2D(static_cast<uint32_t>(_screenDimensions.x), static_cast<uint32_t>(_screenDimensions.y))));
	pipelineDesc.renderPass = _renderpass;
	pipelineDesc.subpass = _subpass;
	pipelineDesc.flags = pvrvk::PipelineCreateFlags::e_ALLOW_DERIVATIVES_BIT;
	pipelineDesc.multiSample.setNumRasterizationSamples(_renderpass->getCreateInfo().getAttachmentDescription(0).getSamples());

	const int32_t shaderConst = static_cast<int32_t>(isFrameBufferSrgb);
	pipelineDesc.fragmentShader.setShaderConstant(0, pvrvk::ShaderConstantInfo(0, &shaderConst, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Integer))));

#ifdef VK_USE_PLATFORM_MACOS_MVK

	VkInstance instance = deviceSharedPtr->getPhysicalDevice()->getInstance()->getVkHandle();

	// set fullImageViewSwizzle to true and set MoltenVKConfig.
	if (!isFullImageViewSwizzleMVK)
	{
		mvkConfig.fullImageViewSwizzle = true;
		pvrvk::getVkBindings().vkSetMoltenVKConfigurationMVK(instance, &mvkConfig, &sizeOfMVK);
	}

#endif

	_pipeline = deviceSharedPtr->createGraphicsPipeline(pipelineDesc, _pipelineCache);
	_pipeline->setObjectName("PVRUtilsVk::UIRenderer::UI GraphicsPipeline");

#ifdef VK_USE_PLATFORM_MACOS_MVK
	if (!isFullImageViewSwizzleMVK)
	{
		// Reset fullImageViewSwizzle to it's previous value.
		mvkConfig.fullImageViewSwizzle = isFullImageViewSwizzleMVK;
		pvrvk::getVkBindings().vkSetMoltenVKConfigurationMVK(instance, &mvkConfig, &sizeOfMVK);
	}
#endif
}

void UIRenderer::initCreateDescriptorSetLayout()
{
	DescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, MaxCombinedImageSampler);
	descPoolInfo.setMaxDescriptorSets(MaxCombinedImageSampler);
	descPoolInfo.addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, MaxDescUbo);
	descPoolInfo.setMaxDescriptorSets((uint16_t)(descPoolInfo.getMaxDescriptorSets() + MaxDescUbo));

	Device deviceSharedPtr = _device.lock();

	_descPool = deviceSharedPtr->createDescriptorPool(descPoolInfo);
	_descPool->setObjectName("PVRUtilsVk::UIRenderer::DescriptorPool");

	DescriptorSetLayoutCreateInfo layoutInfo;

	// CombinedImagesampler Layout
	layoutInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	_texDescLayout = deviceSharedPtr->createDescriptorSetLayout(layoutInfo);

	// Mvp ubo Layout
	layoutInfo.clear().setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);
	_uboMvpDescLayout = deviceSharedPtr->createDescriptorSetLayout(layoutInfo);

	// material ubo layout
	layoutInfo.clear().setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	_uboMaterialLayout = deviceSharedPtr->createDescriptorSetLayout(layoutInfo);
}

Font UIRenderer::createFont(const ImageView& image, const TextureHeader& tex, const Sampler& sampler)
{
	Font font = pvr::ui::impl::Font_::constructShared(*this, image, tex, sampler);
	_fonts.emplace_back(font);
	return font;
}

MatrixGroup UIRenderer::createMatrixGroup()
{
	MatrixGroup group = pvr::ui::impl::MatrixGroup_::constructShared(*this, generateGroupId());
	_sprites.emplace_back(group);
	group->commitUpdates();
	return group;
}

PixelGroup UIRenderer::createPixelGroup()
{
	PixelGroup group = pvr::ui::impl::PixelGroup_::constructShared(*this, generateGroupId());
	_sprites.emplace_back(group);
	group->commitUpdates();
	return group;
}

void UIRenderer::setUpUboPoolLayouts(uint32_t numInstances, uint32_t numSprites)
{
	debug_assertion(numInstances >= numSprites,
		"Maximum number of "
		"instances must be atleast the same as maximum number of sprites");

	pvrvk::Device device = getDevice().lock();
	// mvp pool
	_uboMvp.initLayout(device, numInstances);
	// material pool
	_uboMaterial.initLayout(device, numSprites);
}

void UIRenderer::setUpUboPools(uint32_t numInstances, uint32_t numSprites)
{
	debug_assertion(numInstances >= numSprites,
		"Maximum number of "
		"instances must be atleast the same as maximum number of sprites");

	// mvp pool
	pvrvk::Device device = getDevice().lock();
	_uboMvp.init(device, _uboMvpDescLayout, getDescriptorPool(), *this);
	// material pool
	_uboMaterial.init(device, _uboMaterialLayout, getDescriptorPool(), *this);
}

Image UIRenderer::createImage(const ImageView& tex, const Sampler& sampler) { return createImageFromAtlas(tex, Rect2Df(0.0f, 0.0f, 1.0f, 1.0f), sampler); }

pvr::ui::Image UIRenderer::createImageFromAtlas(const ImageView& tex, const Rect2Df& uv, const Sampler& sampler)
{
	Image image = pvr::ui::impl::Image_::constructShared(*this, tex, tex->getImage()->getWidth(), tex->getImage()->getHeight(), sampler);
	_sprites.emplace_back(image);
	// construct the scaling matrix
	// calculate the scale factor
	// convert from texel to normalize coord
	image->setUV(uv);
	image->commitUpdates();
	return image;
}

TextElement UIRenderer::createTextElement(const std::wstring& text, const Font& font, uint32_t maxLength)
{
	(void)maxLength;
	TextElement spriteText = pvr::ui::impl::TextElement_::constructShared(*this, text, font);
	_textElements.emplace_back(spriteText);
	return spriteText;
}

TextElement UIRenderer::createTextElement(const std::string& text, const Font& font, uint32_t size)
{
	TextElement spriteText = pvr::ui::impl::TextElement_::constructShared(*this, text, font, size);
	_textElements.emplace_back(spriteText);
	return spriteText;
}

Text UIRenderer::createText(const TextElement& textElement)
{
	Text text = pvr::ui::impl::Text_::constructShared(*this, textElement);
	_sprites.emplace_back(text);
	text->commitUpdates();
	return text;
}

void UIRenderer::init(uint32_t width, uint32_t height, bool fullscreen, const RenderPass& renderpass, uint32_t subpass, bool isFrameBufferSrgb, CommandPool& commandPool,
	Queue& queue, bool createDefaultLogo, bool createDefaultTitle, bool createDefaultFont, uint32_t maxNumInstances, uint32_t maxNumSprites)
{
#ifdef VK_USE_PLATFORM_MACOS_MVK

	VkInstance instance = renderpass->getDevice()->getPhysicalDevice()->getInstance()->getVkHandle();
	sizeOfMVK = sizeof(MVKConfiguration);
	pvrvk::getVkBindings().vkGetMoltenVKConfigurationMVK(instance, &mvkConfig, &sizeOfMVK);
	isFullImageViewSwizzleMVK = mvkConfig.fullImageViewSwizzle;

#endif
	_mustEndCommandBuffer = false;
	_device = renderpass->getDevice();

	pvr::utils::beginQueueDebugLabel(queue, pvrvk::DebugUtilsLabel("PVRUtilsVk::UIRenderer::Init"));

	// create pool layouts
	setUpUboPoolLayouts(maxNumInstances, maxNumSprites);

	{
		pvrvk::Device device = getDevice().lock();
		// Create a temporary buffer to derive the memory requirements for our buffer allocator.
		// This buffer will be released at the end of this scope.
		pvrvk::Buffer tempBuffer = utils::createBuffer(device,
			pvrvk::BufferCreateInfo(_uboMvp._structuredBufferView.getDynamicSliceSize(),
				pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT | pvrvk::BufferUsageFlags::e_INDEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT),
			(pvrvk::MemoryPropertyFlags(0)));

		_vmaAllocator = utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(device, 0));
	}

	_screenDimensions = glm::vec2(width, height);
	_renderpass = renderpass;
	_subpass = subpass;
	// screen rotated?
	if (_screenDimensions.y > _screenDimensions.x && fullscreen) { rotateScreen90degreeCCW(); }

	// create the commandbuffer
	CommandBuffer cmdBuffer = commandPool->allocateCommandBuffer();
	cmdBuffer->setObjectName("PVRUtilsVk::UIRenderer::initCommandBuffer");
	cmdBuffer->begin();
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel("PVRUtilsVk::UIRenderer::Batching UIRenderer Resource Upload"));

	initCreateDescriptorSetLayout();

	_pipelineCache = _device.lock()->createPipelineCache();

	initCreatePipeline(isFrameBufferSrgb);
	setUpUboPools(maxNumInstances, maxNumSprites);
	initCreateDefaultSampler();

	if (createDefaultLogo) { initCreateDefaultSdkLogo(cmdBuffer); }
	if (createDefaultFont) { initCreateDefaultFont(cmdBuffer); }
	if (createDefaultTitle) { initCreateDefaultTitle(); }
	pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
	cmdBuffer->end();

	Fence fence = queue->getDevice()->createFence();

	SubmitInfo submitInfo;
	submitInfo.commandBuffers = &cmdBuffer;
	submitInfo.numCommandBuffers = 1;
	queue->submit(&submitInfo, 1, fence);
	fence->wait();

	pvr::utils::endQueueDebugLabel(queue);
}

void UIRenderer::initCreateDefaultSampler()
{
	pvrvk::SamplerCreateInfo samplerDesc;

	samplerDesc.wrapModeU = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerDesc.wrapModeV = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerDesc.wrapModeW = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;

	samplerDesc.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	samplerDesc.minFilter = pvrvk::Filter::e_LINEAR;
	samplerDesc.magFilter = pvrvk::Filter::e_LINEAR;
	_samplerBilinear = _device.lock()->createSampler(samplerDesc);

	_samplerBilinear->setObjectName("PVRUtilsVk::UIRenderer::Bilinear Sampler");
	samplerDesc.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	_samplerTrilinear = _device.lock()->createSampler(samplerDesc);
	_samplerTrilinear->setObjectName("PVRUtilsVk::UIRenderer::Trilinear Sampler");
}

void UIRenderer::initCreateDefaultSdkLogo(CommandBuffer& cmdBuffer)
{
	ImageView sdkLogoImage;
	std::unique_ptr<Stream> sdkLogo = std::make_unique<BufferStream>("", _PowerVR_Logo_RGBA_pvr, _PowerVR_Logo_RGBA_pvr_size);

	Texture sdkTex;
	sdkTex = textureLoad(*sdkLogo, TextureFileFormat::PVR);
	Device device = getDevice().lock();

	pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel("PVRUtilsVk::UIRenderer::initCreateDefaultSdkLogo"));
	sdkLogoImage = utils::uploadImageAndView(
		device, sdkTex, true, cmdBuffer, pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, getMemoryAllocator(), getMemoryAllocator());
	pvr::utils::endCommandBufferDebugLabel(cmdBuffer);

	_sdkLogo = createImage(sdkLogoImage, _samplerBilinear);
	_sdkLogo->setAnchor(Anchor::BottomRight, glm::vec2(.98f, -.98f));
	float scalefactor = .3f * getRenderingDim().x / BaseScreenDim.x;

	if (scalefactor > 1) { scalefactor = 1; }
	else if (scalefactor > .5)
	{
		scalefactor = .5;
	}
	else if (scalefactor > .25)
	{
		scalefactor = .25;
	}
	else if (scalefactor > .125)
	{
		scalefactor = .125;
	}
	else
	{
		scalefactor = .0625;
	}

	_sdkLogo->setScale(glm::vec2(scalefactor));
	_sdkLogo->commitUpdates();
	_sdkLogo->getImageView()->setObjectName("PVRUtilsVk::UIRenderer::SDK Logo ImageView");
}

void UIRenderer::initCreateDefaultTitle()
{
	// Default Title
	{
		_defaultTitle = createText(createTextElement("DefaultTitle", _defaultFont, 256));
		_defaultTitle->setAnchor(Anchor::TopLeft, glm::vec2(-.98f, .98f))->setScale(glm::vec2(.8, .8));
		_defaultTitle->commitUpdates();
		// set object names
		if (_defaultTitle->getTextElement()->_vbo->getVkHandle() != VK_NULL_HANDLE)
		{ _defaultTitle->getTextElement()->_vbo->setObjectName("PVRUtilsVk::UIRenderer::Default Title Vbo"); }

		if (_defaultTitle->getTextElement()->_drawIndirectBuffer->getVkHandle() != VK_NULL_HANDLE)
		{ _defaultTitle->getTextElement()->_drawIndirectBuffer->setObjectName("PVRUtilsVk::UIRenderer::Default Title Draw Indirect Buffer"); }
	}

	// Default Description
	{
		_defaultDescription = createText(createTextElement("", _defaultFont, 256));
		// set object names
		if (_defaultDescription->getTextElement()->_vbo->getVkHandle() != VK_NULL_HANDLE)
		{ _defaultDescription->getTextElement()->_vbo->setObjectName("PVRUtilsVk::UIRenderer::Default Description Vbo"); }

		if (_defaultDescription->getTextElement()->_drawIndirectBuffer->getVkHandle() != VK_NULL_HANDLE)
		{ _defaultDescription->getTextElement()->_drawIndirectBuffer->setObjectName("PVRUtilsVk::UIRenderer::Default Description Draw Indirect Buffer"); }
		_defaultDescription->setAnchor(Anchor::TopLeft, glm::vec2(-.98f, .98f - _defaultTitle->getFont()->getFontLineSpacing() / static_cast<float>(getRenderingDimY()) * 1.5f))
			->setScale(glm::vec2(.60, .60));
		_defaultDescription->commitUpdates();
	}

	// Default Controls
	{
		_defaultControls = createText(createTextElement("", _defaultFont, 256));
		// set object names
		if (_defaultControls->getTextElement()->_vbo->getVkHandle() != VK_NULL_HANDLE)
		{ _defaultControls->getTextElement()->_vbo->setObjectName("PVRUtilsVk::UIRenderer::Default Controls Vbo"); }

		if (_defaultControls->getTextElement()->_drawIndirectBuffer->getVkHandle() != VK_NULL_HANDLE)
		{ _defaultControls->getTextElement()->_drawIndirectBuffer->setObjectName("PVRUtilsVk::UIRenderer::Default Controls Draw Indirect Buffer"); }
		_defaultControls->setAnchor(Anchor::BottomLeft, glm::vec2(-.98f, -.98f))->setScale(glm::vec2(.5, .5));
		_defaultControls->commitUpdates();
	}
}

void UIRenderer::initCreateDefaultFont(CommandBuffer& cmdBuffer)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel("PVRUtilsVk::UIRenderer::initCeateDefaultFont"));
	Texture fontTex;
	std::unique_ptr<Stream> arialFontTex;
	float maxRenderDim = glm::max<float>(getRenderingDimX(), getRenderingDimY());
	// pick the right font size of this resolution.
	if (maxRenderDim <= 800) { arialFontTex = std::make_unique<BufferStream>("", _arialbd_36_r8_pvr, _arialbd_36_r8_pvr_size); }
	else if (maxRenderDim <= 1000)
	{
		arialFontTex = std::make_unique<BufferStream>("", _arialbd_46_r8_pvr, _arialbd_46_r8_pvr_size);
	}
	else
	{
		arialFontTex = std::make_unique<BufferStream>("", _arialbd_56_r8_pvr, _arialbd_56_r8_pvr_size);
	}

	fontTex = textureLoad(*arialFontTex, TextureFileFormat::PVR);

	Device device = getDevice().lock();

#ifdef VK_USE_PLATFORM_MACOS_MVK
	VkInstance instance = device->getPhysicalDevice()->getInstance()->getVkHandle();

	// set fullImageViewSwizzle to true and set MoltenVKConfig.
	if (!isFullImageViewSwizzleMVK)
	{
		mvkConfig.fullImageViewSwizzle = true;
		pvrvk::getVkBindings().vkSetMoltenVKConfigurationMVK(instance, &mvkConfig, &sizeOfMVK);
	}

#endif

	pvrvk::ImageView fontImage = device->createImageView(pvrvk::ImageViewCreateInfo(utils::uploadImage(device, fontTex, true, cmdBuffer, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
																						pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, getMemoryAllocator(), getMemoryAllocator()),
		pvrvk::ComponentMapping(pvrvk::ComponentSwizzle::e_R, pvrvk::ComponentSwizzle::e_R, pvrvk::ComponentSwizzle::e_R, pvrvk::ComponentSwizzle::e_R)));

#ifdef VK_USE_PLATFORM_MACOS_MVK
	if (!isFullImageViewSwizzleMVK)
	{
		mvkConfig.fullImageViewSwizzle = isFullImageViewSwizzleMVK;
		pvrvk::getVkBindings().vkSetMoltenVKConfigurationMVK(instance, &mvkConfig, &sizeOfMVK);
	}
#endif

	_defaultFont = createFont(fontImage, fontTex);
	fontImage->setObjectName("PVRUtilsVk::UIRenderer::Default Font ImageView");

	pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
}

void UIRenderer::UboMaterial::initLayout(Device& device, uint32_t numArrayId)
{
	_numArrayId = numArrayId;
	// create the descriptor set & buffer if not already created
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement("uv", GpuDatatypes::mat4x4);
	desc.addElement("color", GpuDatatypes::vec4);
	desc.addElement("alphaMode", GpuDatatypes::Integer);

	_structuredBufferView.initDynamic(desc, _numArrayId, pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
}

void UIRenderer::UboMaterial::init(Device& device, DescriptorSetLayout& descLayout, DescriptorPool& pool, UIRenderer& uirenderer)
{
	_buffer = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(static_cast<VkDeviceSize>(_structuredBufferView.getSize()), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT |
			pvrvk::MemoryPropertyFlags::e_HOST_CACHED_BIT,
		uirenderer._vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

	_structuredBufferView.pointToMappedMemory(_buffer->getDeviceMemory()->getMappedData());
	if (!_buffer) { Log("Failed to create UIRenderer Material buffer"); }
	_buffer->setObjectName("PVRUtilsVk::UIRenderer::UboMaterial");

	if (!_uboDescSetSet) { _uboDescSetSet = pool->allocateDescriptorSet(descLayout); }
	_uboDescSetSet->setObjectName("PVRUtilsVk::UIRenderer::UboMaterial DescriptorSet");

	WriteDescriptorSet writeDescSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _uboDescSetSet, 0, 0);
	writeDescSet.setBufferInfo(0, DescriptorBufferInfo(_buffer, 0, _structuredBufferView.getDynamicSliceSize()));
	device->updateDescriptorSets(&writeDescSet, 1, nullptr, 0);
}

void UIRenderer::UboMvp::initLayout(Device& device, uint32_t numElements)
{
	_numArrayId = numElements;
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement("mvp", GpuDatatypes::mat4x4);

	_structuredBufferView.initDynamic(desc, _numArrayId, pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
}

void UIRenderer::UboMvp::init(Device& device, DescriptorSetLayout& descLayout, DescriptorPool& pool, UIRenderer& uirenderer)
{
	_buffer = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(static_cast<VkDeviceSize>(_structuredBufferView.getSize()), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT |
			pvrvk::MemoryPropertyFlags::e_HOST_CACHED_BIT,
		uirenderer._vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

	_structuredBufferView.pointToMappedMemory(_buffer->getDeviceMemory()->getMappedData());

	if (!_uboDescSetSet) { _uboDescSetSet = pool->allocateDescriptorSet(descLayout); }
	WriteDescriptorSet writeDescSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _uboDescSetSet, 0, 0);

	writeDescSet.setBufferInfo(0, DescriptorBufferInfo(_buffer, 0, _structuredBufferView.getDynamicSliceSize()));

	device->updateDescriptorSets(&writeDescSet, 1, nullptr, 0);
}

void UIRenderer::UboMaterial::updateMaterial(uint32_t arrayIndex, const glm::vec4& color, int32_t alphaMode, const glm::mat4& uv)
{
	_structuredBufferView.getElement(static_cast<uint32_t>(MaterialBufferElement::UVMtx), 0, arrayIndex).setValue(uv);
	_structuredBufferView.getElement(static_cast<uint32_t>(MaterialBufferElement::Color), 0, arrayIndex).setValue(color);
	_structuredBufferView.getElement(static_cast<uint32_t>(MaterialBufferElement::AlphaMode), 0, arrayIndex).setValue(alphaMode);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{ _buffer->getDeviceMemory()->flushRange(_structuredBufferView.getDynamicSliceOffset(arrayIndex), _structuredBufferView.getDynamicSliceSize()); }
}

void UIRenderer::UboMvp::updateMvp(uint32_t bufferArrayId, const glm::mat4x4& mvp)
{
	_structuredBufferView.getElement(0, 0, bufferArrayId).setValue(mvp);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{ _buffer->getDeviceMemory()->flushRange(_structuredBufferView.getDynamicSliceOffset(bufferArrayId), _structuredBufferView.getDynamicSliceSize()); }
}
} // namespace ui
} // namespace pvr
//!\endcond
