/*!
\brief Shows how to use the UIRenderer class to draw ASCII/UTF-8 or wide-charUnicode-compliant text in 3D.
\file VkIntroUIRenderer.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRVk/ApiObjectsVk.h"

// PVR font files
const char CentralTextFontFile[] = "arial_36.pvr";
const char CentralTitleFontFile[] = "starjout_60.pvr";
const char CentralTextFile[] = "Text.txt";

namespace FontSize {
enum Enum
{
	n_36,
	n_46,
	n_56,
	Count
};
}

const char* SubTitleFontFiles[FontSize::Count] = {
	"title_36.pvr",
	"title_46.pvr",
	"title_56.pvr",
};

const uint32_t IntroTime = 4000;
const uint32_t IntroFadeTime = 1000;
const uint32_t TitleTime = 4000;
const uint32_t TitleFadeTime = 1000;
const uint32_t TextFadeStart = 300;
const uint32_t TextFadeEnd = 500;

namespace Language {
enum Enum
{
	English,
	German,
	Norwegian,
	Bulgarian,
	Count
};
}

const wchar_t* Titles[Language::Count] = {
	L"IntroducingUIRenderer",
	L"Einf\u00FChrungUIRenderer",
	L"Innf\u00F8ringUIRenderer",
	L"\u0432\u044A\u0432\u0435\u0436\u0434\u0430\u043D\u0435UIRenderer",
};

class MultiBufferTextManager
{
	pvr::ui::Text _text[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	uint8_t _isDirty[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	uint32_t _lastUpdateText;
	uint32_t _numElement;

	enum
	{
		DirtyTextMask,
		DirtyColorMask
	};

public:
	MultiBufferTextManager() : _numElement(0) {}
	MultiBufferTextManager& addText(pvr::ui::Text text)
	{
		_text[_numElement++] = text;
		return *this;
	}

	pvr::ui::Text getText(uint32_t swapchain) { return _text[swapchain]; }

	void setText(uint32_t swapchain, const char* str)
	{
		_lastUpdateText = swapchain;
		_text[swapchain]->getTextElement()->setText(str);
		_text[swapchain]->commitUpdates();
		for (uint32_t i = 0; i < ARRAY_SIZE(_isDirty); ++i) { _isDirty[i] |= (1 << DirtyTextMask); }

		_isDirty[swapchain] &= ~(1 << DirtyTextMask);
	}

	void setText(uint32_t swapchain, const wchar_t* str)
	{
		_lastUpdateText = swapchain;
		_text[swapchain]->getTextElement()->setText(str);
		_text[swapchain]->commitUpdates();
		for (uint32_t i = 0; i < _numElement; ++i) { _isDirty[i] |= (1 << DirtyTextMask); }

		_isDirty[swapchain] &= ~(1 << DirtyTextMask);
	}

	void setColor(uint32_t swapchain, const glm::vec4& color)
	{
		for (uint32_t i = 0; i < _numElement; ++i)
		{
			_text[i]->setColor(color);
			_isDirty[i] |= (1 << DirtyColorMask);
		}

		_text[swapchain]->commitUpdates();
		_isDirty[swapchain] &= ~(1 << DirtyColorMask);
	}

	bool updateText(uint32_t swapchain)
	{
		if (_isDirty[swapchain] & 0x02)
		{
			_text[swapchain]->commitUpdates();
			_isDirty[swapchain] &= ~(1 << DirtyColorMask);
		}

		if (_isDirty[swapchain] & 0x01)
		{
			if (_text[_lastUpdateText]->getTextElement()->getString().length())
			{ _text[swapchain]->getTextElement()->setText(_text[_lastUpdateText]->getTextElement()->getString()); }
			else
			{
				_text[swapchain]->getTextElement()->setText(_text[_lastUpdateText]->getTextElement()->getWString());
			}
			_text[swapchain]->commitUpdates();
			_isDirty[swapchain] &= ~(1 << DirtyTextMask);
			return true;
		}
		return false;
	}

	void renderText(uint32_t swapchain) { _text[swapchain]->render(); }
};

struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;
	pvrvk::Swapchain swapchain;
	pvrvk::Queue queue;

	pvr::utils::vma::Allocator vmaAllocator;

	pvrvk::CommandPool commandPool;
	pvrvk::DescriptorPool descriptorPool;

	// UIRenderer used to display text
	pvr::ui::UIRenderer uiRenderer;

	MultiBufferTextManager titleText1;
	MultiBufferTextManager titleText2;

	pvr::ui::Image background;
	pvr::Multi<pvr::ui::MatrixGroup> centralTextGroup;
	std::vector<pvr::ui::Text> centralTextLines;
	pvr::ui::Text centralTitleLine1;
	pvr::ui::Text centralTitleLine2;

	pvr::Multi<pvrvk::ImageView> depthStencilImages;
	pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;

	pvrvk::Semaphore imageAcquiredSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore presentationSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameResourcesFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	pvr::Multi<pvrvk::SecondaryCommandBuffer> cmdBufferWithIntro;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> cmdBufferWithText;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> commandBufferSubtitle;
	pvr::Multi<pvrvk::CommandBuffer> primaryCommandBuffer;

	~DeviceResources()
	{
		if (device)
		{
			device->waitIdle();
			uint32_t l = swapchain->getSwapchainLength();
			for (uint32_t i = 0; i < l; ++i)
			{
				if (perFrameResourcesFences[i]) perFrameResourcesFences[i]->wait();
			}
		}
	}
};

/// <summary>Implementing the pvr::Shell functions.</summary>
class VulkanIntroducingUIRenderer : public pvr::Shell
{
	glm::mat4 _mvp;

	float _textOffset;
	float _lineSpacingNDC;
	std::vector<char> _text;
	std::vector<const char*> _textLines;
	Language::Enum _titleLang;
	int32_t _textStartY, _textEndY;

	std::unique_ptr<DeviceResources> _deviceResources;

	uint32_t _frameId;

	bool _centralTextRecorded[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	bool _centralTitleRecorded[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void generateBackgroundTexture(uint32_t screenWidth, uint32_t screenHeight, pvrvk::CommandBuffer& uploadCmd);

	void updateCentralTitle(uint64_t currentTime);
	void updateSubTitle(uint64_t currentTime, uint32_t swapchain);
	void updateCentralText();
	void recordCommandBuffers();

private:
};

/// <summary>Record the rendering commands.</summary>
void VulkanIntroducingUIRenderer::recordCommandBuffers()
{
	for (uint32_t i = 0; i < _deviceResources->onScreenFramebuffer.size(); ++i)
	{
		// commandbuffer intro
		{
			_deviceResources->cmdBufferWithIntro[i]->begin(_deviceResources->onScreenFramebuffer[i], 0);
			_deviceResources->uiRenderer.beginRendering(_deviceResources->cmdBufferWithIntro[i]);
			_deviceResources->background->render();
			// This is the difference
			_deviceResources->centralTitleLine1->render();
			_deviceResources->centralTitleLine2->render();
			_deviceResources->uiRenderer.getSdkLogo()->render();
			// Tells uiRenderer to do all the pending text rendering now
			_deviceResources->uiRenderer.endRendering();
			_deviceResources->cmdBufferWithIntro[i]->end();
		}

		// commandbuffer scrolling text
		{
			_deviceResources->cmdBufferWithText[i]->begin(_deviceResources->onScreenFramebuffer[i], 0);
			_deviceResources->uiRenderer.beginRendering(_deviceResources->cmdBufferWithText[i]);
			_deviceResources->background->render();
			_deviceResources->centralTextGroup[i]->render();
			_deviceResources->uiRenderer.getSdkLogo()->render();
			// Tells uiRenderer to do all the pending text rendering now
			_deviceResources->uiRenderer.endRendering();
			_deviceResources->cmdBufferWithText[i]->end();
		}
	}
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it(e.g.external modules, loading meshes, etc.).If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanIntroducingUIRenderer::initApplication()
{
	// Because the C++ standard states that only ASCII characters are valid in compiled code,
	// we are instead using an external resource file which contains all of the text to be
	// rendered. This allows complete control over the encoding of the resource file which
	// in this case is encoded as UTF-8.
	std::unique_ptr<pvr::Stream> textStream = getAssetStream(CentralTextFile);

	// The following code simply pulls out each line in the resource file and adds it
	// to an array so we can render each line separately. ReadIntoCharBuffer null-terminates the std::string
	// so it is safe to check for null character.
	textStream->readIntoCharBuffer(_text);
	size_t current = 0;
	while (current < _text.size())
	{
		const char* start = _text.data() + current;

		_textLines.push_back(start);
		while (current < _text.size() && _text[current] != '\0' && _text[current] != '\n' && _text[current] != '\r') { ++current; }

		if (current < _text.size() && (_text[current] == '\r')) { _text[current++] = '\0'; }
		// null-term the strings!!!
		if (current < _text.size() && (_text[current] == '\n' || _text[current] == '\0')) { _text[current++] = '\0'; }
	}

	_titleLang = Language::English;
	_frameId = 0;
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.</summary>
/// <returns>Result::Success if no error occurred</returns>.
pvr::Result VulkanIntroducingUIRenderer::quitApplication() { return pvr::Result::Success; }

/// <summary>Generates a simple background texture procedurally.</summary>
/// <param name="screenWidth"> screen dimension's width.</param>
/// <param name="screenHeight"> screen dimension's height.</param>
void VulkanIntroducingUIRenderer::generateBackgroundTexture(uint32_t screenWidth, uint32_t screenHeight, pvrvk::CommandBuffer& uploadCmd)
{
	// Generate star texture
	uint32_t width = pvr::math::makePowerOfTwoHigh(screenWidth);
	uint32_t height = pvr::math::makePowerOfTwoHigh(screenHeight);

	pvr::TextureHeader hd;
	hd.channelType = pvr::VariableType::UnsignedByteNorm;
	hd.pixelFormat = pvr::GeneratePixelType1<'l', 8>::ID;
	hd.colorSpace = pvr::ColorSpace::lRGB;
	hd.width = width;
	hd.height = height;
	pvr::Texture myTexture(hd);
	unsigned char* textureData = myTexture.getDataPointer();
	memset(textureData, 0, width * height);
	for (uint32_t j = 0; j < height; ++j)
	{
		for (uint32_t i = 0; i < width; ++i)
		{
			if (!(rand() % 200))
			{
				int brightness = rand() % 255;
				textureData[width * j + i] = (unsigned char)glm::clamp(textureData[width * j + i] + brightness, 0, 255);
			}
		}
	}
	_deviceResources->background = _deviceResources->uiRenderer.createImage(pvr::utils::uploadImageAndView(_deviceResources->device, myTexture, true, uploadCmd,
		pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator));
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change  in the rendering context. Used to initialize variables that are dependent on the
/// rendering context(e.g.textures, vertex buffers, etc.).</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanIntroducingUIRenderer::initView()
{
	// Create the empty API objects.
	_deviceResources = std::make_unique<DeviceResources>();

	// Create instance and retrieve compatible physical devices
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName());

	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable not find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// Create the surface
	pvrvk::Surface surface =
		pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay(), this->getConnection());

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface };
	pvr::utils::QueueAccessInfo queueAccessInfo;
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), &queuePopulateInfo, 1, &queueAccessInfo);

	// get the queue
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	// Create the command pool
	_deviceResources->commandPool =
		_deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(queueAccessInfo.familyId, pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{ swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT; } // Retrieve the swapchain images and create corresponding depth stencil images per swap chain
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), swapChainCreateOutput.renderPass, 0, getBackBufferColorspace() == pvr::ColorSpace::sRGB,
		_deviceResources->commandPool, _deviceResources->queue, true, true, true, 128);

	// Create the sync objects and the commandbuffer
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->commandBufferSubtitle[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->cmdBufferWithIntro[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->cmdBufferWithText[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->primaryCommandBuffer[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
	}

	_deviceResources->primaryCommandBuffer[0]->begin();
	// Generate background texture
	generateBackgroundTexture(getWidth(), getHeight(), _deviceResources->primaryCommandBuffer[0]);

	// The fonts are loaded here using a PVRTool's ResourceFile wrapper. However,
	// it is possible to load the textures in any way that provides access to a pointer
	// to memory, and the size of the file.
	pvr::ui::Font subTitleFont, centralTitleFont, centralTextFont;
	{
		pvr::Texture tmpTexture0;
		pvr::Texture tmpTexture1;
		auto texview0 = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, CentralTitleFontFile, true, _deviceResources->primaryCommandBuffer[0], *this,
			pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, &tmpTexture0, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
		auto texview1 = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, CentralTextFontFile, true, _deviceResources->primaryCommandBuffer[0], *this,
			pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, &tmpTexture1, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);

		centralTitleFont = _deviceResources->uiRenderer.createFont(texview0, tmpTexture0);

		pvrvk::SamplerCreateInfo centralTextSamplerCreateInfo = _deviceResources->uiRenderer.getSamplerBilinear()->getCreateInfo();

		if (_deviceResources->device->getPhysicalDevice()->getFeatures().getSamplerAnisotropy())
		{
			Log(LogLevel::Information, "Making use of supported Sampler Anisotropy");
			centralTextSamplerCreateInfo.enableAnisotropy = true;

			Log(LogLevel::Information, "Maximum supported Sampler Anisotropy: %f", _deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMaxSamplerAnisotropy());

			Log(LogLevel::Information, "Using anisotropyMaximum = %f as this is the minimum supported limit", 16);

			centralTextSamplerCreateInfo.anisotropyMaximum = _deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMaxSamplerAnisotropy();
		}

		pvrvk::Sampler centralTextSampler = _deviceResources->device->createSampler(centralTextSamplerCreateInfo);
		centralTextFont = _deviceResources->uiRenderer.createFont(texview1, tmpTexture1, centralTextSampler);

		// Determine which size title font to use.
		uint32_t screenShortDimension = std::min(getWidth(), getHeight());
		const char* subtitleFontFileName = NULL;
		if (screenShortDimension >= 720) { subtitleFontFileName = SubTitleFontFiles[FontSize::n_56]; }
		else if (screenShortDimension >= 640)
		{
			subtitleFontFileName = SubTitleFontFiles[FontSize::n_46];
		}
		else
		{
			subtitleFontFileName = SubTitleFontFiles[FontSize::n_36];
		}

		pvr::Texture tmpTexture2;
		auto texview2 = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, subtitleFontFileName, true, _deviceResources->primaryCommandBuffer[0], *this,
			pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, &tmpTexture2, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
		subTitleFont = _deviceResources->uiRenderer.createFont(texview2, tmpTexture2);
	}

	_deviceResources->primaryCommandBuffer[0]->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->primaryCommandBuffer[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();
	_deviceResources->primaryCommandBuffer[0]->reset(pvrvk::CommandBufferResetFlags::e_RELEASE_RESOURCES_BIT);

	_deviceResources->background->commitUpdates();

	_deviceResources->uiRenderer.getSdkLogo()->commitUpdates();
	const uint32_t swapChainLength = _deviceResources->swapchain->getSwapchainLength();
	for (uint32_t i = 0; i < swapChainLength; ++i)
	{
		pvr::ui::Text text1 = _deviceResources->uiRenderer.createText(subTitleFont, 255);
		pvr::ui::Text text2 = _deviceResources->uiRenderer.createText(subTitleFont, 255);
		text1->setAnchor(pvr::ui::Anchor::TopLeft, -.98f, .98f);
		text2->setAnchor(pvr::ui::Anchor::TopLeft, -.98f, .98f);

		_deviceResources->titleText1.addText(text1);
		_deviceResources->titleText2.addText(text2);
		_deviceResources->centralTextGroup[i] = _deviceResources->uiRenderer.createMatrixGroup();
	}

	_deviceResources->centralTextLines.push_back(_deviceResources->uiRenderer.createText(centralTextFont, _textLines[0], 255));
	for (uint32_t i = 0; i < swapChainLength; ++i) { _deviceResources->centralTextGroup[i]->add(_deviceResources->centralTextLines.back()); }
	_lineSpacingNDC = 1.6f * _deviceResources->centralTextLines[0]->getFont()->getFontLineSpacing() / static_cast<float>(_deviceResources->uiRenderer.getRenderingDimY());

	for (uint32_t i = 1; i < _textLines.size(); ++i)
	{
		pvr::ui::Text text = _deviceResources->uiRenderer.createText(centralTextFont, _textLines[i], 255);
		text->setAnchor(pvr::ui::Anchor::Center, glm::vec2(0.f, -(i * _lineSpacingNDC)));
		_deviceResources->centralTextLines.push_back(text);
		for (uint32_t j = 0; j < swapChainLength; ++j) { _deviceResources->centralTextGroup[j]->add(text); }
	}

	_deviceResources->centralTextLines[0]->setAlphaRenderingMode(true);
	_deviceResources->centralTitleLine1 = _deviceResources->uiRenderer.createText(centralTitleFont, "introducing", 50);
	_deviceResources->centralTitleLine2 = _deviceResources->uiRenderer.createText(centralTitleFont, "uirenderer", 50);

	_deviceResources->centralTitleLine1->setAnchor(pvr::ui::Anchor::BottomCenter, glm::vec2(.0f, .0f));
	_deviceResources->centralTitleLine2->setAnchor(pvr::ui::Anchor::TopCenter, glm::vec2(.0f, .0f));

	_textStartY = static_cast<int32_t>(-_deviceResources->uiRenderer.getRenderingDimY() - _deviceResources->centralTextGroup[0]->getDimensions().y);

	_textEndY = static_cast<int32_t>(_deviceResources->uiRenderer.getRenderingDimY() + _deviceResources->centralTextGroup[0]->getDimensions().y +
		_lineSpacingNDC * static_cast<float>(_deviceResources->uiRenderer.getRenderingDimY()));

	_textOffset = static_cast<float>(_textStartY);
	recordCommandBuffers();
	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanIntroducingUIRenderer::releaseView()
{
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_centralTextRecorded[i] = false;
		_centralTitleRecorded[i] = false;
	}
	_deviceResources.reset();

	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanIntroducingUIRenderer::renderFrame()
{
	// Clears the colour and depth buffer
	uint64_t currentTime = this->getTime() - this->getTimeAtInitApplication();

	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();
	bool mustRecord = ((currentTime < IntroTime && !_centralTitleRecorded[swapchainIndex]) || (currentTime >= IntroTime && !_centralTextRecorded[swapchainIndex]));

	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

	updateSubTitle(currentTime, swapchainIndex);
	// record the primary commandbuffer

	if (mustRecord)
	{
		_deviceResources->primaryCommandBuffer[swapchainIndex]->begin();
		const pvrvk::ClearValue clearValue[2] = { pvrvk::ClearValue(0.f, 0.f, 0.f, 1.f), pvrvk::ClearValue(1.f, 0u) };
		_deviceResources->primaryCommandBuffer[swapchainIndex]->beginRenderPass(_deviceResources->onScreenFramebuffer[swapchainIndex], false, clearValue, ARRAY_SIZE(clearValue));
	}

	// Render the 'IntroducingUIRenderer' title for the first n seconds.
	if (currentTime < IntroTime)
	{
		updateCentralTitle(currentTime);
		_centralTitleRecorded[swapchainIndex] = true;
		if (mustRecord) { _deviceResources->primaryCommandBuffer[swapchainIndex]->executeCommands(_deviceResources->cmdBufferWithIntro[swapchainIndex]); }
	}
	// Render the 3D text.
	else
	{
		updateCentralText();
		_centralTextRecorded[swapchainIndex] = true;
		if (mustRecord) { _deviceResources->primaryCommandBuffer[swapchainIndex]->executeCommands(_deviceResources->cmdBufferWithText[swapchainIndex]); }
	}
	_deviceResources->centralTextGroup[swapchainIndex]->commitUpdates();

	if (mustRecord)
	{
		_deviceResources->commandBufferSubtitle[swapchainIndex]->begin(_deviceResources->onScreenFramebuffer[swapchainIndex], 0);
		_deviceResources->uiRenderer.beginRendering(_deviceResources->commandBufferSubtitle[swapchainIndex]);
		_deviceResources->titleText1.renderText(swapchainIndex);
		_deviceResources->titleText2.renderText(swapchainIndex);
		_deviceResources->uiRenderer.endRendering();
		_deviceResources->commandBufferSubtitle[swapchainIndex]->end();

		_deviceResources->primaryCommandBuffer[swapchainIndex]->executeCommands(_deviceResources->commandBufferSubtitle[swapchainIndex]);
		_deviceResources->primaryCommandBuffer[swapchainIndex]->endRenderPass();
		_deviceResources->primaryCommandBuffer[swapchainIndex]->end();
	}

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->primaryCommandBuffer[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.numSignalSemaphores = 1;
	pvrvk::PipelineStageFlags waitStage = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.waitDstStageMask = &waitStage;

	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queue, _deviceResources->commandPool, _deviceResources->swapchain, swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	// PRESENT
	pvrvk::PresentInfo presentInfo;
	presentInfo.imageIndices = &swapchainIndex;
	presentInfo.numSwapchains = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numWaitSemaphores = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	_deviceResources->queue->present(presentInfo);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	return pvr::Result::Success;
}

/// <summary>Update the description sprite.</summary>
/// <param name="currentTime">Current Time.</param>
void VulkanIntroducingUIRenderer::updateSubTitle(uint64_t currentTime, uint32_t swapchain)
{
	// Fade effect
	static uint32_t prevLang = static_cast<uint32_t>(-1);
	uint32_t titleLang = static_cast<uint32_t>((currentTime / 1000) / (TitleTime / 1000)) % Language::Count;

	uint32_t nextLang = (titleLang + 1) % Language::Count;
	uint32_t modTime = static_cast<uint32_t>(currentTime) % TitleTime;
	float titlePerc = 1.0f;
	float nextPerc = 0.0f;
	if (modTime > TitleTime - TitleFadeTime)
	{
		titlePerc = 1.0f - ((modTime - (TitleTime - TitleFadeTime)) / static_cast<float>(TitleFadeTime));
		nextPerc = 1.0f - titlePerc;
	}

	const glm::vec4 titleCol(1.f, 1.f, 1.f, titlePerc);
	const glm::vec4 nextCol(1.f, 1.f, 1.f, nextPerc);

	// Here we are passing in a wide-character std::string to uiRenderer function. This allows
	// Unicode to be compiled in to std::string-constants, which this code snippet demonstrates.
	// Because we are not setting a projection or a model-view matrix the default projection
	// matrix is used.
	if (titleLang != prevLang)
	{
		_deviceResources->titleText1.setText(swapchain, Titles[titleLang]);
		_deviceResources->titleText2.setText(swapchain, Titles[nextLang]);
		prevLang = titleLang;
	}
	_deviceResources->titleText1.setColor(swapchain, titleCol);
	_deviceResources->titleText2.setColor(swapchain, nextCol);
	_deviceResources->titleText1.updateText(swapchain);
	_deviceResources->titleText2.updateText(swapchain);
}

/// <summary>Draws the title text.</summary>
/// <param name="currentTime">Current Time.</param>
void VulkanIntroducingUIRenderer::updateCentralTitle(uint64_t currentTime)
{
	// Using the MeasureText() method provided by uiRenderer, we can determine the bounding-box
	// size of a std::string of text. This can be useful for justify text centrally, as we are
	// doing here.
	float fadeAmount = 1.0f;

	// Fade in
	if (currentTime < IntroFadeTime) { fadeAmount = currentTime / static_cast<float>(IntroFadeTime); }
	// Fade out
	else if (currentTime > IntroTime - IntroFadeTime)
	{
		fadeAmount = 1.0f - ((currentTime - (IntroTime - IntroFadeTime)) / static_cast<float>(IntroFadeTime));
	}
	// Editing the text's alpha based on the fade amount.
	_deviceResources->centralTitleLine1->setColor(1.f, 1.f, 0.f, fadeAmount);
	_deviceResources->centralTitleLine2->setColor(1.f, 1.f, 0.f, fadeAmount);
	_deviceResources->centralTitleLine1->commitUpdates();
	_deviceResources->centralTitleLine2->commitUpdates();
}

/// <summary>Draws the 3D _text and scrolls in to the screen.</summary>
void VulkanIntroducingUIRenderer::updateCentralText()
{
	glm::mat4 mProjection = glm::mat4(1.0f);

	if (isScreenRotated())
	{
		mProjection = pvr::math::perspectiveFov(
			pvr::Api::Vulkan, 0.7f, float(_deviceResources->uiRenderer.getRenderingDimY()), float(_deviceResources->uiRenderer.getRenderingDimX()), 1.0f, 2000.0f);
	}
	else
	{
		mProjection = pvr::math::perspectiveFov(
			pvr::Api::Vulkan, 0.7f, float(_deviceResources->uiRenderer.getRenderingDimX()), float(_deviceResources->uiRenderer.getRenderingDimY()), 1.0f, 2000.0f);
	}

	const glm::mat4 mCamera = glm::lookAt(glm::vec3(_deviceResources->uiRenderer.getRenderingDimX() * .5f, -_deviceResources->uiRenderer.getRenderingDimY(), 700.0f),
		glm::vec3(_deviceResources->uiRenderer.getRenderingDimX() * .5f, 0, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	_mvp = mProjection * mCamera;

	// Calculate the FPS scale.
	float fFPSScale = float(getFrameTime()) * 60 / 1000;

	// Move the text. Progressively speed up.
	float fSpeedInc = 0.0f;
	if (_textOffset > 0.0f) { fSpeedInc = _textOffset / _textEndY; }
	_textOffset += (0.75f + (1.0f * fSpeedInc)) * fFPSScale;
	if (_textOffset > _textEndY) { _textOffset = static_cast<float>(_textStartY); }
	glm::mat4 trans = glm::translate(glm::vec3(0.0f, _textOffset, 0.0f));

	// uiRenderer can optionally be provided with user-defined projection and model-view matrices
	// which allow custom layout of text. Here we are proving both a projection and model-view
	// matrix. The projection matrix specified here uses perspective projection which will
	// provide the 3D effect. The model-view matrix positions the text in world space
	// providing the 'camera' position and the scrolling of the text.

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->centralTextGroup[i]->setScaleRotateTranslate(trans);
		_deviceResources->centralTextGroup[i]->setViewProjection(_mvp);
	}

	// The previous method (renderTitle()) explains the following functions in more detail
	// however put simply, we are looping the entire array of loaded text which is encoded
	// in UTF-8. uiRenderer batches this internally and the call to Flush() will render the
	// text to the frame buffer. We are also fading out the text over a certain distance.
	float pos, fade;
	for (uint32_t uiIndex = 0; uiIndex < _textLines.size(); ++uiIndex)
	{
		pos = (_textOffset - (uiIndex * 36.0f));
		fade = 1.0f;
		glm::vec4 color(1.0f, 1.0f, 0.0f, 1.0f);

		if (pos > TextFadeStart)
		{
			fade = glm::clamp(1.0f - ((pos - TextFadeStart) / (TextFadeEnd - TextFadeStart)), 0.0f, 1.0f);
			color.a *= fade;
		}
		_deviceResources->centralTextLines[uiIndex]->setColor(color);
	}
	_deviceResources->centralTextLines[0]->commitUpdates();
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanIntroducingUIRenderer>(); }
