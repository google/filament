/*!
\brief Demonstrates how to efficiently render UI and sprites using UIRenderer.
\file VulkanExampleUI.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRShell/PVRShell.h"
#include "PVRVk/ApiObjectsVk.h"
#include "PVRUtils/PVRUtilsVk.h"

enum
{
	NullQuadPix = 4,
	VirtualWidth = 640,
	VirtualHeight = 480,
	UiDisplayTime = 5, // Display each page for 5 seconds
	UiDisplayTimeInMs = UiDisplayTime * 1000,
	NumClocks = 22
};
static const float LowerContainerHeight = .3f;

namespace Sprites {
enum Enum
{
	Clockface,
	Hand,
	Battery,
	Web,
	Newmail,
	Network,
	Calendar,
	WeatherSunCloudBig,
	WeatherSunCloud,
	WeatherRain,
	WeatherStorm,
	ContainerCorner,
	ContainerVertical,
	ContainerHorizontal,
	ContainerFiller,
	VerticalBar,
	Text1,
	Text2,
	TextLorem,
	TextWeather,
	TextFriday,
	TextSaturday,
	TextSunday,
	TextMonday,
	ClockfaceSmall,
	HandSmall,
	WindowBottom,
	WindowBottomCorner,
	WindowSide,
	WindowTop,
	WindowTopLeft,
	WindowTopRight,
	Count,
	None = 0xFFFF
};
}
// Ancillary textures
namespace Ancillary {
enum Enum
{
	Topbar = Sprites::Count,
	Background = Sprites::Count + 1,
	Count = 2
};
}

const pvr::StringHash SpritesFileNames[Sprites::Count + Ancillary::Count] = {
	"clock-face.pvr", // Clockface
	"hand.pvr", // Hand
	"battery.pvr", // Battery
	"internet-web-browser.pvr", // Web
	"mail-message-new.pvr", // Newmail
	"network-wireless.pvr", // Network
	"office-calendar.pvr", // Calendar

	"weather-sun-cloud-big.pvr", // Weather_SUNCLOUD_BIG
	"weather-sun-cloud.pvr", // Weather_SUNCLOUD
	"weather-rain.pvr", // Weather_RAIN
	"weather-storm.pvr", // Weather_STORM

	"container-corner.pvr", // Container_CORNER
	"container-vertical.pvr", // Container_VERT
	"container-horizontal.pvr", // Container_HORI
	"container-filler.pvr", // container_FILLER
	"vertical-bar.pvr",
	"text1.pvr", // Text1
	"text2.pvr", // Text2
	"loremipsum.pvr",
	"text-weather.pvr", // Text_WEATHER
	"text-fri.pvr", // Fri
	"text-sat.pvr", // Sat
	"text-sun.pvr", // Sun
	"text-mon.pvr", // Mon

	"clock-face-small.pvr", // ClockfaceSmall
	"hand-small.pvr", // Hand_SMALL

	"window-bottom.pvr", // Window_BOTTOM
	"window-bottomcorner.pvr", // Window_BOTTOMCORNER
	"window-side.pvr", // Window_SIDE
	"window-top.pvr", // Window_TOP
	"window-topleft.pvr", // Window_TOPLEFT
	"window-topright.pvr", // Window_TOPRIGHT

	"topbar.pvr", // Topbar
	"background.pvr", // Background
};

// Displayed pages
namespace DisplayPage {
enum Enum
{
	Clocks,
	Weather,
	Window,

	Count,
	Default = Clocks
};
}

// Display option. Toggled with keyboard.
namespace DisplayOption {
enum Enum
{
	UI,
	Count,
	Default = UI
};
}

// Display _state
namespace DisplayState {
enum Enum
{
	Element,
	Transition,
	Default = Element
};
}

const char* const FragShaderFileName = "ColShader.fsh.spv"; // ColorShader
const char* const VertShaderFileName = "ColShader.vsh.spv"; // ColorShader

// Group shader programs and their uniform locations together

struct SpriteDesc
{
	pvrvk::ImageView imageView;
	uint32_t uiWidth;
	uint32_t uiHeight;
	uint32_t uiSrcX;
	uint32_t uiSrcY;
	bool bHasAlpha;
	void release() { imageView.reset(); }
};

struct Vertex
{
	glm::vec4 vVert;
};

struct SpriteClock
{
	pvr::ui::PixelGroup group; // root group
	pvr::ui::PixelGroup hand; // hand group contains hand sprite
	pvr::ui::Image clock; // clock sprite
	glm::vec2 scale;
};

struct SpriteContainer
{
	pvr::ui::PixelGroup group;
	pvrvk::Rect2Df size;
};

struct PageClock
{
	pvr::ui::MatrixGroup group[(uint8_t)pvrvk::FrameworkCaps::MaxSwapChains]; // root group
	void update(uint32_t swapchain, float frameTime, const glm::mat4& trans);
	std::vector<SpriteClock> clocks;
	SpriteContainer container;
	glm::mat4 _projMtx;
};

struct PageWeather
{
	pvr::ui::MatrixGroup group[(uint8_t)pvrvk::FrameworkCaps::MaxSwapChains];
	void update(uint32_t swapchain, const glm::mat4& transMtx);
	glm::mat4 _projMtx;
	SpriteContainer containerTop, containerBottom;
};

struct PageWindow
{
	pvr::ui::MatrixGroup group[(uint8_t)pvrvk::FrameworkCaps::MaxSwapChains];
	pvr::utils::StructuredBufferView renderQuadUboBufferView;
	pvrvk::Buffer renderQuadUboBuffer;
	pvrvk::DescriptorSet renderQuadUboDesc[4];
	void update(glm::mat4& proj, uint32_t swapchain, float width, float height, const glm::mat4& trans);
	pvrvk::Rect2D renderArea;
};

/// <summary>Update the clock page.</summary>
/// <param name="screenWidth"> Screen width. </param>
/// <param name="screenHeight"> Screen height. </param>
/// <param name="frameTime"> Current frame. </param>
/// <param name="trans"> Transformation matrix. </param>
void PageClock::update(uint32_t swapchain, float frameTime, const glm::mat4& trans)
{
	// to do render the container
	static float handRotate = 0.0f;
	handRotate -= frameTime * 0.001f;
	const float clockHandScale(.22f);
	uint32_t i = 0;
	// right groups
	glm::vec2 clockOrigin(container.size.getOffset().getX() + container.size.getExtent().getWidth(), container.size.getOffset().getY() + container.size.getExtent().getHeight());
	const glm::vec2 smallClockDim(clocks[0].group->getDimensions() * clocks[0].scale);
	glm::vec2 clockOffset(0, 0);
	uint32_t clockIndex = 1;
	for (; i < clocks.size() / 2; i += 2)
	{
		// the first two small clock (left & right) at the top closer.
		if (i < 2)
		{
			clocks[i].hand->setRotation(handRotate + clockIndex)->setScale(glm::vec2(clockHandScale));
			clocks[i].group->setAnchor(pvr::ui::Anchor::TopRight, clockOrigin);
			clocks[i].group->setPixelOffset(-smallClockDim.x * 2, 0);
			++clockIndex;

			clocks[i + 1].hand->setRotation(handRotate + clockIndex)->setScale(glm::vec2(clockHandScale));
			clocks[i + 1].group->setAnchor(pvr::ui::Anchor::TopLeft, glm::vec2(container.size.getOffset().getX(), clockOrigin.y));
			clocks[i + 1].group->setPixelOffset(smallClockDim.x * 2, 0);
			++clockIndex;
			continue;
		}

		clocks[i].hand->setRotation(handRotate + clockIndex)->setScale(glm::vec2(clockHandScale));
		clocks[i].group->setAnchor(pvr::ui::Anchor::TopRight, clockOrigin);
		clocks[i].group->setPixelOffset(0, clockOffset.y);
		++clockIndex;

		clocks[i + 1].hand->setRotation(handRotate + clockIndex)->setScale(glm::vec2(clockHandScale));
		clocks[i + 1].group->setAnchor(pvr::ui::Anchor::TopRight, clockOrigin);

		clocks[i + 1].group->setPixelOffset(-smallClockDim.x, clockOffset.y);

		clockOffset.y -= smallClockDim.y;
		++clockIndex;
	}

	// left group
	clockOrigin = glm::vec2(container.size.getOffset().getX(), container.size.getOffset().getY() + container.size.getExtent().getHeight());
	clockOffset.y = 0;
	for (; i < clocks.size() - 1; i += 2)
	{
		clocks[i].hand->setRotation(handRotate + clockIndex)->setScale(glm::vec2(clockHandScale));
		clocks[i].group->setAnchor(pvr::ui::Anchor::TopLeft, clockOrigin);
		clocks[i].group->setPixelOffset(0, clockOffset.y);
		++clockIndex;

		clocks[i + 1].hand->setRotation(handRotate + clockIndex)->setScale(glm::vec2(clockHandScale));
		clocks[i + 1].group->setAnchor(pvr::ui::Anchor::TopLeft, clockOrigin);
		clocks[i + 1].group->setPixelOffset(smallClockDim.x, clockOffset.y);
		clockOffset.y -= smallClockDim.y;
		++clockIndex;
	}
	// render the center clocks
	clocks[i].hand->setRotation(handRotate);
	clocks[i].group->setAnchor(pvr::ui::Anchor::Center, glm::vec2(0))->setPixelOffset(0, 30);
	group[swapchain]->setScaleRotateTranslate(trans); // _transform the entire group
	group[swapchain]->commitUpdates();
}

/// <summary>Update the window page</summary>
/// <param name="screenWidth">Screen width.</param>
/// <param name="screenHeight">Screen height.</param>
/// <param name="trans">Transformation matrix.</param>
void PageWindow::update(glm::mat4& proj, uint32_t swapchain, float width, float height, const glm::mat4& trans)
{
	glm::vec2 offset(width * .5f, height * .5f); // center it on the screen
	// offset the render area center to aligned with the center of the screen
	offset -= glm::vec2(renderArea.getExtent().getWidth(), renderArea.getExtent().getHeight()) * glm::vec2(.5f, .5f);

	glm::mat4 worldTrans = glm::translate(glm::vec3(offset, 0.0f)) * trans;
	group[swapchain]->setScaleRotateTranslate(worldTrans);
	group[swapchain]->commitUpdates();

	// update the render quad ubo
	glm::mat4 scale = glm::scale(glm::vec3(glm::vec2(renderArea.getExtent().getWidth(), renderArea.getExtent().getHeight()) / glm::vec2(width, height), 1.f));
	glm::mat4x4 mvp = proj * worldTrans * scale;
	renderQuadUboBufferView.getElement(0, 0, swapchain).setValue(mvp);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(renderQuadUboBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{ renderQuadUboBuffer->getDeviceMemory()->flushRange(renderQuadUboBufferView.getDynamicSliceOffset(swapchain), renderQuadUboBufferView.getDynamicSliceSize()); }
}

/// <summary>Update the weather page.</summary>
/// <param name="swapchain">The current swapchain.</param>
/// <param name="transMtx">Transformation matrix</param>
void PageWeather::update(uint32_t swapchain, const glm::mat4& transMtx)
{
	group[swapchain]->setScaleRotateTranslate(transMtx);
	group[swapchain]->commitUpdates();
}

class VulkanExampleUI;

/// Constants
const char* const DisplayOpts[DisplayOption::Count] = {
	"Displaying Interface", // Ui
};

#ifdef DISPLAY_SPRITE_ALPHA
const char* const SpriteShaderDefines[] = {
	"DISPLAY_SPRITE_ALPHA",
};
#else
const char** const SpriteShaderDefines = NULL;
#endif

static const uint32_t DimDefault = 0xABCD;
static const uint32_t DimCentre = 0xABCE;
static const float ByteToFloat = 1.0f / 255.0f;

static const char* const TextLoremIpsum = "Stencil Clipped text: \n\nLorem ipsum dolor sit amet, consectetuer adipiscing elit.\nDonec molestie. "
										  "Sed aliquam sem ut arcu.\nPhasellus sollicitudin. Vestibulum condimentum facilisis nulla.\nIn "
										  "hac habitasse platea dictumst. Nulla nonummy. Cras quis libero.\nCras venenatis. Aliquam posuere "
										  "lobortis pede. Nullam fringilla urna id leo.\nPraesent aliquet pretium erat. Praesent non odio. "
										  "Pellentesque a magna a\nmauris vulputate lacinia. Aenean viverra. Class aptent taciti sociosqu "
										  "ad litora\ntorquent per conubia nostra, per inceptos hymenaeos. Aliquam\nlacus. Mauris magna eros, "
										  "semper a, tempor et, rutrum et, tortor.";

class Area
{
private:
	int32_t x;
	int32_t y;
	int32_t w;
	int32_t h;
	int32_t size;
	bool isFilled;

	Area* right;
	Area* left;

private:
	void setSize(int32_t iWidth, int32_t iHeight);

public:
	Area(int32_t iWidth, int32_t iHeight);
	Area();

	Area* insert(int32_t iWidth, int32_t iHeight);
	bool deleteArea();

	int32_t getX() const;
	int32_t getY() const;
};

class SpriteCompare
{
public:
	bool operator()(const SpriteDesc& pSpriteDescA, const SpriteDesc& pSpriteDescB)
	{
		uint32_t uiASize = pSpriteDescA.uiWidth * pSpriteDescA.uiHeight;
		uint32_t uiBSize = pSpriteDescB.uiWidth * pSpriteDescB.uiHeight;
		return (uiASize > uiBSize);
	}
};

struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;
	pvrvk::Queue queue;
	pvr::utils::vma::Allocator vmaAllocator;
	pvrvk::Swapchain swapchain;
	pvrvk::CommandPool commandPool;
	pvrvk::DescriptorPool descriptorPool;
	pvrvk::Semaphore imageAcquiredSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore presentationSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameResourcesFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	pvrvk::GraphicsPipeline renderQuadPipe;
	pvrvk::GraphicsPipeline renderWindowTextPipe;

	// Shader handles
	pvrvk::ShaderModule vertexShader;
	pvrvk::ShaderModule fragmentShader;

	pvrvk::DescriptorSetLayout texLayout;
	pvrvk::DescriptorSetLayout uboLayoutVert;
	pvrvk::DescriptorSetLayout uboLayoutFrag;

	pvrvk::Sampler samplerNearest;
	pvrvk::Sampler samplerBilinear;

	// UIRenderer used to display text
	pvr::ui::UIRenderer uiRenderer;

	PageClock pageClock;
	PageWeather pageWeather;
	PageWindow pageWindow;
	SpriteContainer containerTop;
	pvrvk::Buffer quadVbo;

	pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;
	pvr::Multi<pvrvk::ImageView> depthStencil;
	pvr::Multi<pvrvk::CommandBuffer> cmdBuffers;

	pvr::Multi<pvrvk::SecondaryCommandBuffer> cmdBufferTitleDesc;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> cmdBufferBaseUI;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> commandBufferClockPage;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> cmdBufferWeatherpage;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> cmdBufferWindow;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> cmdBufferRenderUI;

	SpriteDesc spritesDesc[Sprites::Count + Ancillary::Count];

	pvr::ui::Text textLorem;
	pvr::ui::Image sprites[Sprites::Count + Ancillary::Count];

	pvr::ui::PixelGroup groupBaseUI;

	pvrvk::PipelineCache pipelineCache;

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

class VulkanExampleUI : public pvr::Shell
{
private:
	std::unique_ptr<DeviceResources> _deviceResources;

	uint32_t _frameId;

	// Transforms
	float _wndRotate;
	glm::mat4 _transform;
	glm::mat4 _projMtx;

	// Display options
	int32_t _displayOption;
	DisplayState::Enum _state;
	float _transitionPerc;
	DisplayPage::Enum _currentPage;
	DisplayPage::Enum _lastPage;
	int32_t _cycleDir;
	uint64_t _currTime;

	// Time
	float _wndRotPerc;
	uint64_t _prevTransTime;
	uint64_t _prevTime;
	bool _swipe;
	glm::vec2 _screenScale;

	void createFullScreenQuad(pvrvk::CommandBuffer& uploadCmd)
	{
		uint32_t width = getWidth();
		uint32_t height = getHeight();
		Vertex vVerts[4] = {
			{ glm::vec4(0, height, 0, 1) }, // top left
			{ glm::vec4(0, 0, 0, 1) }, // bottom left
			{ glm::vec4(width, height, 0, 1) }, // top right
			{ glm::vec4(width, 0, 0, 1) } // bottom right
		};
		_deviceResources->quadVbo = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(sizeof(vVerts), pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		bool isBufferHostVisible = (_deviceResources->quadVbo->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT) != 0;

		if (isBufferHostVisible) { pvr::utils::updateHostVisibleBuffer(_deviceResources->quadVbo, &vVerts[0], 0, static_cast<uint32_t>(sizeof(vVerts[0]) * 4), true); }
		else
		{
			pvr::utils::updateBufferUsingStagingBuffer(
				_deviceResources->device, _deviceResources->quadVbo, uploadCmd, &vVerts[0], 0, static_cast<uint32_t>(sizeof(vVerts[0]) * 4), _deviceResources->vmaAllocator);
		}
	}

	void updateTitleAndDesc(DisplayOption::Enum displayOption)
	{
		switch (displayOption)
		{
		case DisplayOption::UI:
			_deviceResources->uiRenderer.getDefaultDescription()->setText("Displaying Interface");
			_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
			break;
		default: break;
		}
		for (uint32_t i = 0, len = _deviceResources->swapchain->getSwapchainLength(); i < len; ++i)
		{
			_deviceResources->cmdBufferTitleDesc[i]->begin(_deviceResources->onScreenFramebuffer[i], 0);
			_deviceResources->uiRenderer.beginRendering(_deviceResources->cmdBufferTitleDesc[i]);
			_deviceResources->uiRenderer.getDefaultTitle()->render();
			_deviceResources->uiRenderer.getDefaultDescription()->render();
			_deviceResources->uiRenderer.getSdkLogo()->render();
			_deviceResources->uiRenderer.endRendering();
			_deviceResources->cmdBufferTitleDesc[i]->end();
		}
	}

private:
	void drawScreenAlignedQuad(const pvrvk::GraphicsPipeline& pipe, pvrvk::DescriptorSet& ubo, pvrvk::CommandBufferBase cmdBuffers);
	void renderUI(uint32_t swapchain);
	void renderPage(DisplayPage::Enum Page, const glm::mat4& mTransform, uint32_t swapchain);
	void createPipelines();
	void createBaseUI();
	void createPageWeather();
	void createPageWindow();
	void swipeLeft();
	void swipeRight();
	void eventMappedInput(pvr::SimplifiedInput action);
	float getVirtualWidth() { return static_cast<float>(isRotated() ? this->getHeight() : this->getWidth()); }
	float getVirtualHeight() { return static_cast<float>(isRotated() ? this->getWidth() : this->getHeight()); }
	float toDeviceX(float fVal) { return ((fVal / VirtualWidth) * getVirtualWidth()); }
	float toDeviceY(float fVal) { return ((fVal / VirtualHeight) * getVirtualHeight()); }
	inline bool isRotated() { return this->isScreenRotated(); }
	void createSamplersAndDescriptorSet();
	void createSpriteContainer(pvrvk::Rect2Df const& rect, uint32_t numSubContainer, float lowerContainerHeight, SpriteContainer& outContainer);
	void createPageClock();
	void createClockSprite(SpriteClock& outClock, Sprites::Enum sprite);
	void recordSecondaryCommandBuffers(uint32_t swapchain);
	void loadSprites(pvrvk::CommandBuffer& uploadCmd);

public:
	VulkanExampleUI();
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
};

VulkanExampleUI::VulkanExampleUI()
	: _wndRotate(0.0f), _displayOption(DisplayOption::Default), _state(DisplayState::Default), _transitionPerc(0.0f), _currentPage(DisplayPage::Default),
	  _lastPage(DisplayPage::Default), _cycleDir(1), _wndRotPerc(0.0f), _prevTransTime(0), _prevTime(0), _currTime(static_cast<uint64_t>(-1))
{}

/// <summary>Create Window page.</summary>
void VulkanExampleUI::createPageWindow()
{
	// create the window page
	_deviceResources->textLorem = _deviceResources->uiRenderer.createText(TextLoremIpsum);
	_deviceResources->textLorem->setScale(glm::vec2(.5f));
	_deviceResources->textLorem->setColor(0.0f, 0.0f, 0.0f, 1.0f);
	_deviceResources->textLorem->setAnchor(pvr::ui::Anchor::BottomLeft, glm::vec2(-1.0f, -1.0f));
	_deviceResources->pageWindow.renderArea = pvrvk::Rect2D(0, 0, 390, 250);
	_deviceResources->pageWindow.renderArea.setOffset(pvrvk::Offset2D(static_cast<int32_t>(_deviceResources->pageWindow.renderArea.getOffset().getX() * _screenScale.x),
		static_cast<int32_t>(_deviceResources->pageWindow.renderArea.getOffset().getY() * _screenScale.y)));
	_deviceResources->pageWindow.renderArea.setExtent(pvrvk::Extent2D(static_cast<uint32_t>(_deviceResources->pageWindow.renderArea.getExtent().getWidth() * _screenScale.x),
		static_cast<uint32_t>(_deviceResources->pageWindow.renderArea.getExtent().getHeight() * _screenScale.y)));
	for (uint32_t i = 0, len = _deviceResources->swapchain->getSwapchainLength(); i < len; ++i)
	{
		_deviceResources->pageWindow.group[i] = _deviceResources->uiRenderer.createMatrixGroup();
		_deviceResources->pageWindow.group[i]->setViewProjection(_projMtx);
		_deviceResources->pageWindow.group[i]->add(_deviceResources->textLorem);
		_deviceResources->pageWindow.group[i]->commitUpdates();
	}
}

/// <summary>Create sprite container.</summary>
/// <param name="rect">Container rectangle.</param>
/// <param name="numSubContainer">Number of lower sub containers.</param>
/// <param name="lowerContainerHeight">lower container height.</param>
/// <param name="outContainer">Returned Sprite container.</param>
void VulkanExampleUI::createSpriteContainer(pvrvk::Rect2Df const& rect, uint32_t numSubContainer, float lowerContainerHeight, SpriteContainer& outContainer)
{
	outContainer.size = rect;
	outContainer.group = _deviceResources->uiRenderer.createPixelGroup();

	// calculate the border of the container
	const float borderX = _deviceResources->sprites[Sprites::ContainerHorizontal]->getWidth() / _deviceResources->uiRenderer.getRenderingDimX() * 2.f;

	const float borderY = _deviceResources->sprites[Sprites::ContainerCorner]->getHeight() / _deviceResources->uiRenderer.getRenderingDimY() * 2.f;

	pvrvk::Rect2Df rectVerticleLeft(rect.getOffset().getX(), rect.getOffset().getY() + borderY, borderX, rect.getExtent().getHeight() - borderY * 2);
	pvrvk::Rect2Df rectVerticleRight(
		rect.getOffset().getX() + rect.getExtent().getWidth(), rect.getOffset().getY() + borderY, rect.getExtent().getWidth(), rect.getExtent().getHeight() - borderY * 2);
	pvrvk::Rect2Df rectTopHorizontal(rect.getOffset().getX() + borderX, rect.getOffset().getY() + rect.getExtent().getHeight() - borderY, rect.getExtent().getWidth() - borderX * 2,
		rect.getExtent().getHeight());
	pvrvk::Rect2Df rectBottomHorizontal(rect.getOffset().getX() + borderX, rect.getOffset().getY(), rect.getExtent().getWidth() - borderX * 2, rect.getOffset().getY() + borderY);

	// align the sprites to lower left so they will be aligned with their group
	_deviceResources->sprites[Sprites::ContainerCorner]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.0f);
	_deviceResources->sprites[Sprites::ContainerVertical]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.0f);
	_deviceResources->sprites[Sprites::ContainerHorizontal]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.f, -1.f);

	// add the filler
	{
		pvr::ui::PixelGroup filler = _deviceResources->uiRenderer.createPixelGroup();
		filler->add(_deviceResources->sprites[Sprites::ContainerFiller]);
		_deviceResources->sprites[Sprites::ContainerFiller]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.f, -1.f);
		filler->setAnchor(pvr::ui::Anchor::BottomLeft, rect.getOffset().getX() + borderX, rect.getOffset().getY() + borderY);

		filler->setScale(glm::vec2(.5f * (rect.getExtent().getWidth() - borderX * 2 /*minus the left and right borders*/) * _deviceResources->uiRenderer.getRenderingDimX() /
				_deviceResources->sprites[Sprites::ContainerFiller]->getWidth(),
			.501f * (rect.getExtent().getHeight() - borderY * 2 /*minus Top and Bottom borders*/) * _deviceResources->uiRenderer.getRenderingDimY() /
				_deviceResources->sprites[Sprites::ContainerFiller]->getHeight()));

		outContainer.group->add(filler);
		outContainer.group->setSize(glm::vec2(_deviceResources->uiRenderer.getRenderingDimX(), _deviceResources->uiRenderer.getRenderingDimY()));
	}

	// Top Left Corner
	{
		pvr::ui::PixelGroup newGroup = _deviceResources->uiRenderer.createPixelGroup();
		// place the centre at the
		newGroup->add(_deviceResources->sprites[Sprites::ContainerCorner]);
		newGroup->setAnchor(pvr::ui::Anchor::BottomRight, rectTopHorizontal.getOffset().getX(), rectTopHorizontal.getOffset().getY());
		outContainer.group->add(newGroup);
	}

	// Top Right Corner
	{
		pvr::ui::PixelGroup newGroup = _deviceResources->uiRenderer.createPixelGroup();
		newGroup->add(_deviceResources->sprites[Sprites::ContainerCorner]);
		// flip the x coordinate by negative scale
		newGroup->setAnchor(pvr::ui::Anchor::BottomRight, rectTopHorizontal.getOffset().getX() + rectTopHorizontal.getExtent().getWidth(), rectTopHorizontal.getOffset().getY())
			->setScale(glm::vec2(-1.f, 1.0f));
		outContainer.group->add(newGroup);
	}

	// bottom left Corner
	{
		pvr::ui::PixelGroup newGroup = _deviceResources->uiRenderer.createPixelGroup();
		newGroup->add(_deviceResources->sprites[Sprites::ContainerCorner]);
		// flip the y coordinates
		newGroup->setAnchor(pvr::ui::Anchor::BottomRight, rectBottomHorizontal.getOffset().getX(), rectBottomHorizontal.getExtent().getHeight())->setScale(glm::vec2(1.f, -1.0f));
		outContainer.group->add(newGroup);
	}

	// Bottom right Corner
	{
		pvr::ui::PixelGroup newGroup = _deviceResources->uiRenderer.createPixelGroup();
		newGroup->add(_deviceResources->sprites[Sprites::ContainerCorner]);
		// flip the x and y coordinates
		newGroup
			->setAnchor(pvr::ui::Anchor::BottomRight, rectBottomHorizontal.getOffset().getX() + rectBottomHorizontal.getExtent().getWidth(), rectBottomHorizontal.getExtent().getHeight())
			->setScale(glm::vec2(-1.f, -1.0f));
		outContainer.group->add(newGroup);
	}

	// Horizontal Up
	{
		// calculate the width of the sprite
		float width =
			(rectTopHorizontal.getExtent().getWidth() * .5f * _deviceResources->uiRenderer.getRenderingDimX() / _deviceResources->sprites[Sprites::ContainerVertical]->getWidth());
		pvr::ui::PixelGroup horizontal = _deviceResources->uiRenderer.createPixelGroup();
		horizontal->add(_deviceResources->sprites[Sprites::ContainerVertical]);
		horizontal->setAnchor(pvr::ui::Anchor::BottomLeft, rectTopHorizontal.getOffset().getX(), rectTopHorizontal.getOffset().getY());
		horizontal->setScale(glm::vec2(width, 1.f));
		outContainer.group->add(horizontal);
	}

	// Horizontal Down
	{
		// calculate the width of the sprite
		float width =
			(rectBottomHorizontal.getExtent().getWidth() * .5f * _deviceResources->uiRenderer.getRenderingDimX() / _deviceResources->sprites[Sprites::ContainerVertical]->getWidth());
		pvr::ui::PixelGroup horizontal = _deviceResources->uiRenderer.createPixelGroup();
		horizontal->add(_deviceResources->sprites[Sprites::ContainerVertical]);
		horizontal->setAnchor(pvr::ui::Anchor::TopLeft, rectBottomHorizontal.getOffset().getX(), rectBottomHorizontal.getOffset().getY());
		horizontal->setScale(glm::vec2(width, -1.f));
		outContainer.group->add(horizontal);
	}

	// Vertical Left
	{
		// calculate the height of the sprite
		float height =
			(rectVerticleLeft.getExtent().getHeight() * .501f * _deviceResources->uiRenderer.getRenderingDimY() / _deviceResources->sprites[Sprites::ContainerHorizontal]->getHeight());
		pvr::ui::PixelGroup verticle = _deviceResources->uiRenderer.createPixelGroup();
		verticle->add(_deviceResources->sprites[Sprites::ContainerHorizontal]);
		verticle->setScale(glm::vec2(1, height))->setAnchor(pvr::ui::Anchor::BottomLeft, rectVerticleLeft.getOffset().getX(), rectVerticleLeft.getOffset().getY())->setPixelOffset(0, 0);
		outContainer.group->add(verticle);
	}

	// Vertical Right
	{
		// calculate the height of the sprite
		float height = (rectVerticleRight.getExtent().getHeight() * .501f * _deviceResources->uiRenderer.getRenderingDimY() /
			_deviceResources->sprites[Sprites::ContainerHorizontal]->getHeight());
		pvr::ui::PixelGroup vertical = _deviceResources->uiRenderer.createPixelGroup();
		vertical->add(_deviceResources->sprites[Sprites::ContainerHorizontal]);
		vertical->setScale(glm::vec2(-1, height))->setAnchor(pvr::ui::Anchor::BottomLeft, rectVerticleRight.getOffset().getX(), rectVerticleRight.getOffset().getY());
		outContainer.group->add(vertical);
	}

	float width = 1.f / _deviceResources->uiRenderer.getRenderingDimX() * _deviceResources->sprites[Sprites::ContainerHorizontal]->getWidth();
	float height = (outContainer.size.getExtent().getHeight() - outContainer.size.getOffset().getY()) * .5f;

	// calculate the each container size
	float containerWidth = rect.getExtent().getWidth() / numSubContainer;
	float borderWidth = 1.f / _deviceResources->uiRenderer.getRenderingDimX() * _deviceResources->sprites[Sprites::VerticalBar]->getWidth();
	pvrvk::Rect2Df subRect(rect.getOffset().getX(), rect.getOffset().getY(), rect.getOffset().getX() + containerWidth, rect.getOffset().getY() + lowerContainerHeight);
	height = .5f * (subRect.getExtent().getHeight() - subRect.getOffset().getY()) * _deviceResources->uiRenderer.getRenderingDimY() /
		_deviceResources->sprites[Sprites::VerticalBar]->getHeight();
	// create the lower containers

	// Horizontal Split
	{
		// half it here because the scaling happen at the centre
		width = (rect.getExtent().getWidth() * .5f * _deviceResources->uiRenderer.getRenderingDimX() / _deviceResources->sprites[Sprites::VerticalBar]->getHeight());
		width -= .25; // reduce the width by quarter of a pixel so they fit well between the container
		pvr::ui::PixelGroup horizontal = _deviceResources->uiRenderer.createPixelGroup();
		horizontal->add(_deviceResources->sprites[Sprites::VerticalBar]);
		horizontal->setScale(glm::vec2(1.f, width))
			->setAnchor(pvr::ui::Anchor::BottomLeft, rect.getOffset().getX() + (2 / _deviceResources->uiRenderer.getRenderingDimX()) /*offset it by 2 pixel*/,
				subRect.getExtent().getHeight());
		horizontal->setRotation(glm::pi<float>() * -.5f); // rotate y 90 degree
		outContainer.group->add(horizontal);
	}

	for (uint32_t i = 0; i < numSubContainer - 1; ++i)
	{
		pvr::ui::PixelGroup groupVertical = _deviceResources->uiRenderer.createPixelGroup();
		_deviceResources->sprites[Sprites::VerticalBar]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.f, -1.f);
		groupVertical->add(_deviceResources->sprites[Sprites::VerticalBar]);
		groupVertical->setAnchor(pvr::ui::Anchor::BottomLeft, subRect.getExtent().getWidth(), subRect.getOffset().getY())->setScale(glm::vec2(1, height));
		outContainer.group->add(groupVertical);
		subRect.setOffset(pvrvk::Offset2Df(subRect.getOffset().getX() + containerWidth - borderWidth, subRect.getOffset().getY()));
		subRect.setExtent(pvrvk::Extent2Df(subRect.getExtent().getWidth() + containerWidth, subRect.getExtent().getHeight()));
	}
	_deviceResources->containerTop = outContainer;
}

/// <summary> Code in initApplication() will be called by pvr::Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns> Return true if no error occurred. </returns>
pvr::Result VulkanExampleUI::initApplication()
{
	setStencilBitsPerPixel(8);

	// initialise current and previous times to avoid saturating the variable used for rotating the window text
	_currTime = this->getTime();
	_prevTime = this->getTime();
	_frameId = 0;

	return pvr::Result::Success;
}

/// <summary>create the weather page.</summary>
void VulkanExampleUI::createPageWeather()
{
	// background
	pvr::ui::PixelGroup backGround = _deviceResources->uiRenderer.createPixelGroup();
	backGround->add(_deviceResources->sprites[Ancillary::Background]);

	// create the weather page
	std::vector<pvr::ui::Sprite> groupsList;

	SpriteContainer container;
	createSpriteContainer(_deviceResources->pageClock.container.size, 4, LowerContainerHeight, container);
	_deviceResources->pageWeather.containerTop = container;
	groupsList.push_back(container.group);
	pvr::ui::PixelGroup group = _deviceResources->uiRenderer.createPixelGroup();

	// align the sprite with its parent group
	_deviceResources->sprites[Sprites::TextWeather]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.f);
	group->setScale(_screenScale);
	group->add(_deviceResources->sprites[Sprites::TextWeather]);
	const glm::vec2& containerHalfSize =
		glm::vec2(_deviceResources->pageWeather.containerTop.size.getExtent().getWidth(), _deviceResources->pageWeather.containerTop.size.getExtent().getHeight()) * .5f;
	group
		->setAnchor(pvr::ui::Anchor::CenterLeft, _deviceResources->pageWeather.containerTop.size.getOffset().getX(),
			_deviceResources->pageWeather.containerTop.size.getOffset().getY() + (_deviceResources->pageWeather.containerTop.size.getExtent().getHeight() / 2.0f))
		->setPixelOffset(10, 40);
	groupsList.push_back(group);

	// add the Weather
	group = _deviceResources->uiRenderer.createPixelGroup();
	group->add(_deviceResources->sprites[Sprites::WeatherSunCloudBig]);
	// align the sprite with its parent group
	_deviceResources->sprites[Sprites::WeatherSunCloudBig]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.f);
	group
		->setAnchor(pvr::ui::Anchor::Center, _deviceResources->pageWeather.containerTop.size.getOffset().getX() + containerHalfSize.x,
			_deviceResources->pageWeather.containerTop.size.getOffset().getY() + containerHalfSize.y)
		->setPixelOffset(0, 40);
	group->setScale(_screenScale);
	groupsList.push_back(group);

	// create the bottom 4 groups
	const Sprites::Enum sprites[] = { Sprites::WeatherSunCloud, Sprites::TextFriday, Sprites::WeatherSunCloud, Sprites::TextSaturday, Sprites::WeatherRain, Sprites::TextSunday,
		Sprites::WeatherStorm, Sprites::TextMonday };

	const float width = _deviceResources->pageWeather.containerTop.size.getExtent().getWidth() / 4.f;
	float tempOffsetX = _deviceResources->pageWeather.containerTop.size.getOffset().getX() + (width * .5f);
	;

	for (uint32_t i = 0; i < 8; i += 2)
	{
		Sprites::Enum weather = sprites[i];
		Sprites::Enum text = sprites[i + 1];

		group = _deviceResources->uiRenderer.createPixelGroup();
		// align the sprite with its parent group
		this->_deviceResources->sprites[weather]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.f);
		group->add(this->_deviceResources->sprites[weather]);
		group->setAnchor(pvr::ui::Anchor::BottomCenter, tempOffsetX, _deviceResources->pageWeather.containerTop.size.getOffset().getY());
		group->setScale(_screenScale);
		groupsList.push_back(group);

		// add the text
		group = _deviceResources->uiRenderer.createPixelGroup();
		// align the text with its parent group
		this->_deviceResources->sprites[text]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.0f, -1.f);
		group->add(this->_deviceResources->sprites[text]);
		group->setAnchor(pvr::ui::Anchor::TopCenter, tempOffsetX, _deviceResources->pageWeather.containerTop.size.getOffset().getY() + LowerContainerHeight)->setPixelOffset(0, -5);

		group->setScale(_screenScale);
		groupsList.push_back(group);
		tempOffsetX = tempOffsetX + width;
	}

	for (uint32_t i = 0, len = _deviceResources->swapchain->getSwapchainLength(); i < len; ++i)
	{
		_deviceResources->pageWeather.group[i] = _deviceResources->uiRenderer.createMatrixGroup();
		_deviceResources->pageWeather.group[i]->add(groupsList.data(), static_cast<uint32_t>(groupsList.size()));
		_deviceResources->pageWeather.group[i]->setViewProjection(_projMtx);
		_deviceResources->pageWeather.group[i]->commitUpdates();
	}
}

/// <summary>Create clock sprite.</summary>
/// <param name="outClock">Returned clock.</param>
/// <param name="sprite">Clock Sprite to create.</param>
void VulkanExampleUI::createClockSprite(SpriteClock& outClock, Sprites::Enum sprite)
{
	// create a group of clock and hand so they can be transformed
	outClock.group = _deviceResources->uiRenderer.createPixelGroup();
	outClock.clock = _deviceResources->sprites[sprite];
	outClock.hand = _deviceResources->uiRenderer.createPixelGroup();

	outClock.hand->add(_deviceResources->sprites[Sprites::Hand]);
	outClock.group->add(outClock.clock);
	outClock.group->add(outClock.hand);

	// set the size of the parent group
	outClock.group->setSize(outClock.clock->getDimensions());

	// centre the clock's to the centre of the parent group
	outClock.clock->setAnchor(pvr::ui::Anchor::Center, 0.0f, 0.0f);

	// centre the hand group so that it can be rotated at the centre of the clock
	outClock.hand->setSize(_deviceResources->sprites[Sprites::Hand]->getDimensions())->setAnchor(pvr::ui::Anchor::BottomCenter, .0f, .0f);
	// centre the clock hand bottom centre and offset it by few pixel so they can be rotated at that point
	_deviceResources->sprites[Sprites::Hand]->setAnchor(pvr::ui::Anchor::BottomCenter, glm::vec2(0.0f, -1.f))->setPixelOffset(0, -10);
}

/// <summary>Create clock page</summary>
void VulkanExampleUI::createPageClock()
{
	SpriteContainer container;
	const uint32_t numClocksInColumn = 5;
	float containerHeight = _deviceResources->sprites[Sprites::ClockfaceSmall]->getDimensions().y * numClocksInColumn / getHeight();
	containerHeight += LowerContainerHeight * .5f; // add the lower container height as well
	float containerWidth = _deviceResources->sprites[Sprites::ClockfaceSmall]->getDimensions().x * 4;
	containerWidth += _deviceResources->sprites[Sprites::Clockface]->getDimensions().x;
	containerWidth /= getWidth();

	pvrvk::Rect2Df containerRect(-containerWidth, -containerHeight, containerWidth * 2.f, containerHeight * 2.f);
	createSpriteContainer(containerRect, 2, LowerContainerHeight, container);
	_deviceResources->pageClock.container = container;

	pvr::ui::Sprite groupSprites[NumClocks + 3];
	uint32_t i = 0;
	for (; i < NumClocks; ++i)
	{
		SpriteClock clock;
		createClockSprite(clock, Sprites::ClockfaceSmall);
		clock.group->setScale(_screenScale);
		clock.scale = _screenScale;
		// add the clock group in to page group
		groupSprites[i] = clock.group;
		_deviceResources->pageClock.clocks.push_back(clock); // add the clock
	}

	// add the centre clock
	// group the hands
	SpriteClock clockCenter;
	createClockSprite(clockCenter, Sprites::Clockface);
	clockCenter.group->setScale(_screenScale);
	groupSprites[i++] = clockCenter.group;
	_deviceResources->pageClock.clocks.push_back(clockCenter);

	_deviceResources->sprites[Sprites::Text1]
		->setAnchor(pvr::ui::Anchor::BottomLeft, glm::vec2(_deviceResources->pageClock.container.size.getOffset().getX(), _deviceResources->pageClock.container.size.getOffset().getY()))
		->setPixelOffset(0, 10);
	_deviceResources->sprites[Sprites::Text1]->setScale(_screenScale);
	groupSprites[i++] = _deviceResources->sprites[Sprites::Text1];

	_deviceResources->sprites[Sprites::Text2]
		->setAnchor(pvr::ui::Anchor::BottomRight,
			glm::vec2(_deviceResources->pageClock.container.size.getExtent().getWidth() + _deviceResources->pageClock.container.size.getOffset().getX() - 0.05f,
				_deviceResources->pageClock.container.size.getOffset().getY()))
		->setPixelOffset(0, 10);
	_deviceResources->sprites[Sprites::Text2]->setScale(_screenScale);
	groupSprites[i++] = _deviceResources->sprites[Sprites::Text2];

	for (uint32_t swapchainIndex = 0, len = _deviceResources->swapchain->getSwapchainLength(); swapchainIndex < len; ++swapchainIndex)
	{
		_deviceResources->pageClock.group[swapchainIndex] = _deviceResources->uiRenderer.createMatrixGroup();
		pvr::ui::MatrixGroup groupBorder = _deviceResources->uiRenderer.createMatrixGroup();
		groupBorder->add(_deviceResources->sprites[Sprites::ContainerVertical]);
		groupBorder->setScaleRotateTranslate(glm::translate(glm::vec3(0.0f, -0.45f, 0.0f)) * glm::scale(glm::vec3(.65f, .055f, .2f)));
		_deviceResources->pageClock.group[swapchainIndex]->add(_deviceResources->containerTop.group);
		_deviceResources->pageClock.group[swapchainIndex]->add(groupSprites, ARRAY_SIZE(groupSprites));
		_deviceResources->pageClock.group[swapchainIndex]->setViewProjection(_projMtx);
		_deviceResources->pageClock.group[swapchainIndex]->commitUpdates();
	}
}

/// <summary>Create base UI.</summary>
void VulkanExampleUI::createBaseUI()
{
	// build the render base UI
	float offset = 0.f;
	int32_t offsetPixel = 10;
	// battery sprite
	_deviceResources->sprites[Sprites::Battery]->setAnchor(pvr::ui::Anchor::TopRight, glm::vec2(1.f, 1.0f));
	offset -= _deviceResources->sprites[Sprites::Battery]->getDimensions().x + offsetPixel;

	// web sprite
	_deviceResources->sprites[Sprites::Web]->setAnchor(pvr::ui::Anchor::TopRight, glm::vec2(1.f, 1.0))->setPixelOffset(offset, 0.f);
	offset -= _deviceResources->sprites[Sprites::Web]->getDimensions().x + offsetPixel;

	// new mail sprite
	_deviceResources->sprites[Sprites::Newmail]->setAnchor(pvr::ui::Anchor::TopRight, glm::vec2(1.f, 1.0f))->setPixelOffset(offset, 0.f);
	offset -= _deviceResources->sprites[Sprites::Newmail]->getDimensions().x + offsetPixel;
	;

	// network sprite
	_deviceResources->sprites[Sprites::Network]->setAnchor(pvr::ui::Anchor::TopRight, glm::vec2(1.f, 1.0f))->setPixelOffset(offset, 0.f);
	_deviceResources->groupBaseUI = _deviceResources->uiRenderer.createPixelGroup();

	pvr::ui::PixelGroup horizontalTopBarGrop = _deviceResources->uiRenderer.createPixelGroup();
	_deviceResources->sprites[Ancillary::Topbar]->setAnchor(pvr::ui::Anchor::BottomLeft, -1.f, -1.f);
	horizontalTopBarGrop->add(_deviceResources->sprites[Ancillary::Topbar]);
	horizontalTopBarGrop->setAnchor(pvr::ui::Anchor::TopLeft, -1.f, 1.f);
	horizontalTopBarGrop->setScale(glm::vec2(_deviceResources->uiRenderer.getRenderingDimX() * .5f, 1.0f));

	_deviceResources->groupBaseUI->add(_deviceResources->sprites[Ancillary::Background])
		->add(horizontalTopBarGrop)
		->add(_deviceResources->sprites[Sprites::Battery])
		->add(_deviceResources->sprites[Sprites::Web])
		->add(_deviceResources->sprites[Sprites::Newmail])
		->add(_deviceResources->sprites[Sprites::Network]);

	glm::vec2 scale = glm::vec2(_deviceResources->sprites[Ancillary::Background]->getWidth(), _deviceResources->sprites[Ancillary::Background]->getHeight());
	scale = 2.5f / scale;
	scale *= glm::vec2(getWidth(), getHeight());
	_deviceResources->sprites[Ancillary::Background]->setAnchor(pvr::ui::Anchor::TopLeft, -1.f, 1.f)->setScale(scale);

	_deviceResources->groupBaseUI->setSize(glm::vec2(_deviceResources->uiRenderer.getRenderingDimX(), _deviceResources->uiRenderer.getRenderingDimY()))
		->setAnchor(pvr::ui::Anchor::TopRight, glm::vec2(1.0f, 1.0f));

	_deviceResources->groupBaseUI->commitUpdates(); // update once here
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanExampleUI::initView()
{
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

	// Create the logical device
	pvr::utils::QueuePopulateInfo queueInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface };
	pvr::utils::QueueAccessInfo queueAccessInfo;
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), &queueInfo, 1, &queueAccessInfo);

	// get the queue
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{ swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT; } // Create the swapchain and depth stencil attachments

	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));
	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	// Create the command pool
	_deviceResources->commandPool =
		_deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(queueAccessInfo.familyId, pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 16)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 16)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 16));

	for (uint32_t swapchainIndex = 0, len = _deviceResources->swapchain->getSwapchainLength(); swapchainIndex < len; ++swapchainIndex)
	{
		_deviceResources->cmdBuffers[swapchainIndex] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->cmdBufferTitleDesc[swapchainIndex] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
	}
	// Initialize _deviceResources->uiRenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue, true, true, true, 1024);

	_screenScale = glm::vec2(glm::min(_deviceResources->uiRenderer.getRenderingDim().x / getWidth(), _deviceResources->uiRenderer.getRenderingDim().y / getHeight()));
	_prevTransTime = this->getTime();

	// Load the sprites
	_deviceResources->cmdBuffers[0]->begin(pvrvk::CommandBufferUsageFlags::e_ONE_TIME_SUBMIT_BIT);
	loadSprites(_deviceResources->cmdBuffers[0]);
	createFullScreenQuad(_deviceResources->cmdBuffers[0]);
	_deviceResources->cmdBuffers[0]->end();

	// Submit all the image uploads
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->cmdBuffers[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();
	_deviceResources->cmdBuffers[0]->reset(pvrvk::CommandBufferResetFlags::e_RELEASE_RESOURCES_BIT);

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	// Load the shaders
	createPipelines();

	createSamplersAndDescriptorSet();

	if (isScreenRotated())
	{
		_projMtx = pvr::math::ortho(pvr::Api::Vulkan, 0.f, static_cast<float>(_deviceResources->swapchain->getDimension().getHeight()), 0.f,
			static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()), 0.0f);
	}
	else
	{
		_projMtx = pvr::math::ortho(pvr::Api::Vulkan, 0.f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()), 0.f,
			static_cast<float>(_deviceResources->swapchain->getDimension().getHeight()), 0.0f);
	}
	_swipe = false;
	// set the default title
	_deviceResources->uiRenderer.getDefaultTitle()->setText("ExampleUI");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	// create the UI groups
	createBaseUI();
	createPageClock();
	createPageWeather();
	createPageWindow();

	for (uint32_t swapchainIndex = 0, len = _deviceResources->swapchain->getSwapchainLength(); swapchainIndex < len; ++swapchainIndex)
	{
		recordSecondaryCommandBuffers(swapchainIndex);
		_deviceResources->presentationSemaphores[swapchainIndex] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[swapchainIndex] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameResourcesFences[swapchainIndex] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
	}
	updateTitleAndDesc((DisplayOption::Enum)_displayOption);
	return pvr::Result::Success;
}

/// <summary>Loads the sprites.</summary>
/// <returns>Return true if no error occurred</returns>
void VulkanExampleUI::loadSprites(pvrvk::CommandBuffer& uploadCmd)
{
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.minFilter = pvrvk::Filter::e_NEAREST;
	samplerInfo.magFilter = pvrvk::Filter::e_NEAREST;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	samplerInfo.wrapModeU = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerInfo.wrapModeV = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerInfo.wrapModeW = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
	pvrvk::Sampler samplerNearest = _deviceResources->device->createSampler(samplerInfo);

	pvr::Texture tex;
	// Load sprites and add to sprite array
	for (uint32_t i = 0; i < Sprites::Count + Ancillary::Count; i++)
	{
		// Copy some useful data out of the texture header.
		_deviceResources->spritesDesc[i].uiWidth = tex.getWidth();
		_deviceResources->spritesDesc[i].uiHeight = tex.getHeight();
		_deviceResources->spritesDesc[i].imageView = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, SpritesFileNames[i].c_str(), true, uploadCmd, *this,
			pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, &tex, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
		const uint8_t* pixelString = tex.getPixelFormat().getPixelTypeChar();

		if (tex.getPixelFormat().getPixelTypeId() == (uint64_t)pvr::CompressedPixelFormat::PVRTCI_2bpp_RGBA ||
			tex.getPixelFormat().getPixelTypeId() == (uint64_t)pvr::CompressedPixelFormat::PVRTCI_4bpp_RGBA || pixelString[0] == 'a' || pixelString[1] == 'a' ||
			pixelString[2] == 'a' || pixelString[3] == 'a')
		{ _deviceResources->spritesDesc[i].bHasAlpha = true; }
		else
		{
			_deviceResources->spritesDesc[i].bHasAlpha = false;
		}
		_deviceResources->sprites[i] = _deviceResources->uiRenderer.createImage(_deviceResources->spritesDesc[i].imageView, samplerNearest);
	}
}

/// <summary>Create nearest and bilinear sampler, and descriptor set.</summary>
/// <returns>Return true if no error occurred.</returns>
void VulkanExampleUI::createSamplersAndDescriptorSet()
{
	// create the samplers.
	pvrvk::SamplerCreateInfo samplerInfo;

	// create bilinear sampler
	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	_deviceResources->samplerBilinear = _deviceResources->device->createSampler(samplerInfo);

	// create point sampler
	samplerInfo.minFilter = pvrvk::Filter::e_NEAREST;
	samplerInfo.magFilter = pvrvk::Filter::e_NEAREST;
	_deviceResources->samplerNearest = _deviceResources->device->createSampler(samplerInfo);

	pvrvk::WriteDescriptorSet writeDescSets[pvrvk::FrameworkCaps::MaxSwapChains];

	pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
	descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	// set up the page window ubo
	auto& ubo = _deviceResources->pageWindow.renderQuadUboBufferView;

	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement("MVP", pvr::GpuDatatypes::mat4x4);
	uint32_t swapLength = _deviceResources->swapchain->getSwapchainLength();

	ubo.initDynamic(desc, swapLength, pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	_deviceResources->pageWindow.renderQuadUboBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(ubo.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

	ubo.pointToMappedMemory(_deviceResources->pageWindow.renderQuadUboBuffer->getDeviceMemory()->getMappedData());

	for (uint32_t i = 0; i < swapLength; ++i)
	{
		pvrvk::DescriptorSet& uboDesc = _deviceResources->pageWindow.renderQuadUboDesc[i];
		uboDesc = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->renderQuadPipe->getPipelineLayout()->getDescriptorSetLayout(0));

		writeDescSets[i]
			.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER, uboDesc, 0)
			.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->pageWindow.renderQuadUboBuffer, ubo.getDynamicSliceOffset(i), ubo.getDynamicSliceSize()));
	}
	_deviceResources->device->updateDescriptorSets(writeDescSets, swapLength, nullptr, 0);
}

/// <summary>create graphics pipelines.</summary>
/// <returns>Return true if no error occurred.<returns>
void VulkanExampleUI::createPipelines()
{
	_deviceResources->texLayout = _deviceResources->device->createDescriptorSetLayout(
		pvrvk::DescriptorSetLayoutCreateInfo().setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT));

	_deviceResources->uboLayoutVert = _deviceResources->device->createDescriptorSetLayout(
		pvrvk::DescriptorSetLayoutCreateInfo().setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT));

	_deviceResources->uboLayoutFrag = _deviceResources->device->createDescriptorSetLayout(
		pvrvk::DescriptorSetLayoutCreateInfo().setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT));

	std::unique_ptr<pvr::Stream> vertSource = getAssetStream(VertShaderFileName);
	std::unique_ptr<pvr::Stream> fragSource = getAssetStream(FragShaderFileName);

	// create the vertex and fragment shaders
	_deviceResources->vertexShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(vertSource->readToEnd<uint32_t>()));
	_deviceResources->fragmentShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(fragSource->readToEnd<uint32_t>()));

	// --- render quad pipeline
	{
		pvrvk::GraphicsPipelineCreateInfo pipeInfo;
		pvrvk::PipelineColorBlendAttachmentState colorAttachmentBlendState;
		pipeInfo.pipelineLayout = _deviceResources->device->createPipelineLayout(pvrvk::PipelineLayoutCreateInfo().setDescSetLayout(0, _deviceResources->uboLayoutVert));

		pipeInfo.vertexShader = _deviceResources->vertexShader;
		pipeInfo.fragmentShader = _deviceResources->fragmentShader;

		pipeInfo.vertexInput.addInputAttribute(pvrvk::VertexInputAttributeDescription(0, 0, pvrvk::Format::e_R32G32B32A32_SFLOAT, 0));
		pipeInfo.vertexInput.addInputBinding(pvrvk::VertexInputBindingDescription(0, sizeof(Vertex)));
		pipeInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);
		pipeInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);
		pipeInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();

		//- Set stencil function to always pass, and write 0x1 in to the stencil buffer.
		//- disable depth write and depth test
		pvrvk::StencilOpState stencilState;
		stencilState.setPassOp(pvrvk::StencilOp::e_REPLACE);
		stencilState.setCompareOp(pvrvk::CompareOp::e_ALWAYS);
		stencilState.setWriteMask(0xffffffff);
		stencilState.setReference(1);
		pipeInfo.depthStencil.setStencilFrontAndBack(stencilState).enableStencilTest(true);
		pipeInfo.colorBlend.setAttachmentState(0, colorAttachmentBlendState);

		pipeInfo.depthStencil.setStencilFrontAndBack(stencilState).enableDepthTest(true).enableDepthWrite(false);

		pipeInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS_OR_EQUAL);

		pvr::utils::populateViewportStateCreateInfo(_deviceResources->onScreenFramebuffer[0], pipeInfo.viewport);

		_deviceResources->renderQuadPipe = _deviceResources->device->createGraphicsPipeline(pipeInfo, _deviceResources->pipelineCache);
	}

	// --- render window text ui pipeline
	{
		// copy the create param from the parent
		pvrvk::GraphicsPipelineCreateInfo pipeInfo = _deviceResources->uiRenderer.getPipeline()->getCreateInfo();
		pipeInfo.depthStencil.enableDepthTest(false).enableDepthWrite(false).enableStencilTest(true);
		// Set stencil compare op to equal and only render when the stencil function passes
		pvrvk::StencilOpState stencilState;
		stencilState.setCompareOp(pvrvk::CompareOp::e_EQUAL);
		stencilState.setCompareMask(0xff);
		stencilState.setReference(1);
		pipeInfo.depthStencil.setStencilFrontAndBack(stencilState);
		pvrvk::PipelineColorBlendAttachmentState colorAttachment;
		colorAttachment.setBlendEnable(true);
		colorAttachment.setSrcColorBlendFactor(pvrvk::BlendFactor::e_SRC_ALPHA);
		colorAttachment.setDstColorBlendFactor(pvrvk::BlendFactor::e_ONE_MINUS_SRC_ALPHA);
		// Setting the alpha blend factors so that we preserve the contents of the framebuffer (i.e. opaque), to avoid
		// artefacts on compositors that use the alpha value. Otherwise someone might use a different scheme to actually
		// have transparency in their window, if the windowing system/compositor supported it (e.g. Wayland).
		colorAttachment.setSrcAlphaBlendFactor(pvrvk::BlendFactor::e_ZERO);
		colorAttachment.setDstAlphaBlendFactor(pvrvk::BlendFactor::e_ONE);
		pipeInfo.colorBlend.setAttachmentState(0, colorAttachment);
		pipeInfo.basePipeline = _deviceResources->uiRenderer.getPipeline();
		pipeInfo.flags = pvrvk::PipelineCreateFlags::e_DERIVATIVE_BIT;
		_deviceResources->renderWindowTextPipe = _deviceResources->device->createGraphicsPipeline(pipeInfo, _deviceResources->pipelineCache);
	}
}

/// <summary>Renders a 2D quad with the given parameters. DstRect is the rectangle to be rendered in
/// world coordinates. SrcRect is the rectangle to be cropped from the texture in pixel coordinates.
/// NOTE: This is not an optimized function and should not be called repeatedly to draw quads to the screen at render time.</summary>
void VulkanExampleUI::drawScreenAlignedQuad(const pvrvk::GraphicsPipeline& pipe, pvrvk::DescriptorSet& ubo, pvrvk::CommandBufferBase cmdBuffers)
{
	cmdBuffers->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipe->getPipelineLayout(), 0, ubo);
	cmdBuffers->bindVertexBuffer(_deviceResources->quadVbo, 0, 0);
	cmdBuffers->draw(0, 4, 0, 1);
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.<returns>
pvr::Result VulkanExampleUI::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting
/// the program. If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result VulkanExampleUI::quitApplication() { return pvr::Result::Success; }

/// <summary>Render the page.</summary>
/// <param name="page">Page to render.</param>
/// <param name="mTransform">Transformation matrix.</param>
void VulkanExampleUI::renderPage(DisplayPage::Enum page, const glm::mat4& mTransform, uint32_t swapchain)
{
	switch (page)
	{
	case DisplayPage::Clocks:
		_deviceResources->pageClock.update(swapchain, float(getFrameTime()), mTransform);
		_deviceResources->cmdBuffers[swapchain]->executeCommands(_deviceResources->commandBufferClockPage[swapchain]);
		break;
	case DisplayPage::Weather:
		_deviceResources->pageWeather.update(swapchain, mTransform);
		_deviceResources->cmdBuffers[swapchain]->executeCommands(_deviceResources->cmdBufferWeatherpage[swapchain]);
		break;
	case DisplayPage::Window:
		_deviceResources->pageWindow.update(_projMtx, swapchain, _deviceResources->uiRenderer.getRenderingDimX(), _deviceResources->uiRenderer.getRenderingDimY(), mTransform);
		_deviceResources->cmdBuffers[swapchain]->executeCommands(_deviceResources->cmdBufferWindow[swapchain]);
		break;
	default: break;
	}
}

/// <summary>Renders the default interface.</summary>
void VulkanExampleUI::renderUI(uint32_t swapchain)
{
	pvrvk::ClearValue clearValues[] = { pvrvk::ClearValue(.3f, .3f, 0.3f, 1.f), pvrvk::ClearValue::createDefaultDepthStencilClearValue() };
	_deviceResources->cmdBuffers[swapchain]->beginRenderPass(
		_deviceResources->onScreenFramebuffer[swapchain], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), false, clearValues, ARRAY_SIZE(clearValues));

	// render the baseUI
	_deviceResources->cmdBuffers[swapchain]->executeCommands(_deviceResources->cmdBufferBaseUI[swapchain]);

	if (_state == DisplayState::Element)
	{
		// A transformation matrix
		if (_currentPage == DisplayPage::Window)
		{
			glm::mat4 vRot, vCentre, vInv;
			vRot = glm::rotate(_wndRotate, glm::vec3(0.0f, 0.0f, 1.0f));

			vCentre = glm::translate(glm::vec3(-_deviceResources->uiRenderer.getRenderingDim() * .5f, 0.0f));

			vInv = glm::inverse(vCentre);
			const glm::vec2 rotateOrigin =
				-glm::vec2(_deviceResources->pageWindow.renderArea.getExtent().getWidth(), _deviceResources->pageWindow.renderArea.getExtent().getHeight()) * glm::vec2(.5f);

			vCentre = glm::translate(glm::vec3(rotateOrigin, 0.0f));
			vInv = glm::inverse(vCentre);

			// align the group centre to the centre of the rotation, rotate and translate it back.
			_transform = vInv * vRot * vCentre;
		}
		else
		{
			_transform = glm::mat4(1.f);
		}
		// Just render the single, current page
		renderPage(_currentPage, _transform, swapchain);
	}
	else if (_state == DisplayState::Transition)
	{
		//--- Render outward group
		float fX = pvr::math::quadraticEaseIn(0.f, -_deviceResources->uiRenderer.getRenderingDimX() * _cycleDir, _transitionPerc);
		_transform = glm::translate(glm::vec3(fX, 0.0f, 0.0f));

		//  the last page
		renderPage(_lastPage, _transform, swapchain);

		// --- Render inward group
		fX = pvr::math::quadraticEaseIn(_deviceResources->uiRenderer.getRenderingDimX() * _cycleDir, 0.0f, _transitionPerc);
		_transform = glm::translate(glm::vec3(fX, 0.0f, 0.0f));

		// Render page
		renderPage(_currentPage, _transform, swapchain);
	}
	// record draw title and description commands
	_deviceResources->cmdBuffers[swapchain]->executeCommands(_deviceResources->cmdBufferTitleDesc[swapchain]);
	_deviceResources->cmdBuffers[swapchain]->endRenderPass();
}

/// <summary>Swipe left.</summary>
void VulkanExampleUI::swipeLeft()
{
	if (_currentPage == 0) { return; }
	_swipe = true;
	_cycleDir = -1;
}

/// <summary>Swipe right.</summary>
void VulkanExampleUI::swipeRight()
{
	if (_currentPage == DisplayPage::Count - 1) { return; }
	_swipe = true;
	_cycleDir = 1;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result VulkanExampleUI::renderFrame()
{
	// begin recording the command buffer
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

	_deviceResources->cmdBuffers[swapchainIndex]->begin(pvrvk::CommandBufferUsageFlags::e_ONE_TIME_SUBMIT_BIT);
	_currTime = this->getTime();
	float deltaTime = (_currTime - _prevTime) * 0.001f;
	_prevTime = _currTime;

	// Update Window rotation
	_wndRotPerc += (1.0f / UiDisplayTime) * deltaTime;
	_wndRotate = pvr::math::quadraticEaseOut(0.0f, glm::pi<float>() * 2.f, _wndRotPerc);

	// Check to see if we should transition to a new page (if we're not already)
	if ((_currTime - _prevTransTime > UiDisplayTimeInMs && _state != DisplayState::Transition) || _swipe)
	{
		// Switch to next page
		_state = DisplayState::Transition;
		_transitionPerc = 0.0f;
		_lastPage = _currentPage;

		// Cycle pages
		int32_t nextPage = _currentPage + _cycleDir;
		if (nextPage >= DisplayPage::Count || nextPage < 0)
		{
			_cycleDir *= -1; // Reverse direction
			nextPage = _currentPage + _cycleDir; // Recalculate
		}
		_currentPage = (DisplayPage::Enum)nextPage;
		_swipe = false;
	}

	// Calculate next transition amount
	if (_state == DisplayState::Transition)
	{
		_transitionPerc += 0.01666f; // 60 FPS
		if (_transitionPerc > 1.f)
		{
			_state = DisplayState::Element;
			_transitionPerc = 1.f;
			_wndRotate = 0.0f; // Reset Window rotation
			_wndRotPerc = 0.0f; // Reset Window rotation percentage
			_prevTransTime = _currTime; // Reset time
		}
	}

	renderUI(swapchainIndex);
	// record commands to draw the title and description
	_deviceResources->cmdBuffers[swapchainIndex]->end();

	// Submit
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags waitStage = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.commandBuffers = &_deviceResources->cmdBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDstStageMask = &waitStage;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queue, _deviceResources->commandPool, _deviceResources->swapchain, swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	// Present
	pvrvk::PresentInfo presentInfo;
	presentInfo.imageIndices = &swapchainIndex;
	presentInfo.numSwapchains = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numWaitSemaphores = 1;

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	_deviceResources->queue->present(presentInfo);
	return pvr::Result::Success;
}

/// <summary>Record secondary command buffer for drawing textures, clock page, weather page and Window page.</summary>
void VulkanExampleUI::recordSecondaryCommandBuffers(uint32_t swapchain)
{
	// record the base ui
	{
		_deviceResources->cmdBufferBaseUI[swapchain] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->uiRenderer.beginRendering(_deviceResources->cmdBufferBaseUI[swapchain]);
		_deviceResources->groupBaseUI->render(); // render the base GUI
		_deviceResources->uiRenderer.endRendering();
	}

	// record DrawClock commands
	{
		_deviceResources->commandBufferClockPage[swapchain] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->uiRenderer.beginRendering(_deviceResources->commandBufferClockPage[swapchain], _deviceResources->onScreenFramebuffer[swapchain]);
		_deviceResources->pageClock.group[swapchain]->render();
		_deviceResources->uiRenderer.endRendering();
	}

	// record draw weather commands
	{
		_deviceResources->cmdBufferWeatherpage[swapchain] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->cmdBufferWeatherpage[swapchain]->begin(_deviceResources->onScreenFramebuffer[swapchain]);
		_deviceResources->uiRenderer.beginRendering(_deviceResources->cmdBufferWeatherpage[swapchain], _deviceResources->onScreenFramebuffer[swapchain]);
		_deviceResources->pageWeather.group[swapchain]->render();
		_deviceResources->uiRenderer.endRendering();
		_deviceResources->cmdBufferWeatherpage[swapchain]->end();
	}

	// record draw Window commands
	{
		_deviceResources->cmdBufferWindow[swapchain] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->cmdBufferWindow[swapchain]->begin(_deviceResources->onScreenFramebuffer[swapchain], 0);

		// bind the render quad pipeline
		_deviceResources->cmdBufferWindow[swapchain]->bindPipeline(_deviceResources->renderQuadPipe);

		// draw a quad only to clear a specific region of the screen
		drawScreenAlignedQuad(_deviceResources->renderQuadPipe, _deviceResources->pageWindow.renderQuadUboDesc[swapchain], _deviceResources->cmdBufferWindow[swapchain]);

		// bind the renderWindowTextPipe pipeline and render the text
		_deviceResources->uiRenderer.beginRendering(
			_deviceResources->cmdBufferWindow[swapchain], _deviceResources->renderWindowTextPipe, _deviceResources->onScreenFramebuffer[swapchain]);
		_deviceResources->pageWindow.group[swapchain]->render();
		_deviceResources->uiRenderer.endRendering();

		_deviceResources->cmdBufferWindow[swapchain]->end();
	}
}

/// <summary>Handle input events.</summary>
/// <param name="action">Input event to handle</param>
void VulkanExampleUI::eventMappedInput(pvr::SimplifiedInput action)
{
	switch (action)
	{
	case pvr::SimplifiedInput::Right: swipeLeft(); break;
	case pvr::SimplifiedInput::Left: swipeRight(); break;
	case pvr::SimplifiedInput::ActionClose: // quit the application
		this->exitShell();
		break;
	default: break;
	}
}

/// <summary>Area Constructor.</summary>
/// <param name="width"> Area width.</param>
Area::Area(int32_t width, int32_t height) : x(0), y(0), isFilled(false), right(nullptr), left(nullptr) { setSize(width, height); }

/// <summary>Area Constructor.</summary>
Area::Area() : x(0), y(0), isFilled(false), right(NULL), left(NULL) { setSize(0, 0); }

/// <summary>Calculates an area where there's sufficient space or returns NULL if no space could be found.</summary>
/// <param name="width">Area width.</param>
/// <param name="height">Area height.</param>
/// <returns>Return a pointer to the area added, else NULL if it fails.</returns>
Area* Area::insert(int32_t width, int32_t height)
{
	// If this area has branches below it (i.e. is not a leaf) then traverse those.
	// Check the left branch first.
	if (left)
	{
		Area* tempPtr = NULL;
		tempPtr = left->insert(width, height);
		if (tempPtr != NULL) { return tempPtr; }
	}
	// Now check right
	if (right) { return right->insert(width, height); }
	// Already filled!
	if (isFilled) { return NULL; }

	// Too small
	if (size < width * height || w < width || h < height) { return NULL; }

	// Just right!
	if (size == width * height && w == width && h == height)
	{
		isFilled = true;
		return this;
	}
	// Too big. Split up.
	if (size > width * height && w >= width && h >= height)
	{
		// Initializes the children, and sets the left child's coordinates as these don't change.
		left = new Area;
		right = new Area;
		left->x = x;
		left->y = y;

		// --- Splits the current area depending on the size and position of the placed texture.
		// Splits vertically if larger free distance across the texture.
		if ((w - width) > (h - height))
		{
			left->w = width;
			left->h = h;

			right->x = x + width;
			right->y = y;
			right->w = w - width;
			right->h = h;
		}
		// Splits horizontally if larger or equal free distance downwards.
		else
		{
			left->w = w;
			left->h = height;

			right->x = x;
			right->y = y + height;
			right->w = w;
			right->h = h - height;
		}

		// Initializes the child members' size attributes.
		left->size = left->h * left->w;
		right->size = right->h * right->w;

		// Inserts the texture into the left child member.
		return left->insert(width, height);
	}
	// Catch all error return.
	return NULL;
}

/// <summary>Deletes the given area.</summary>
/// <returns>Return true on success.</returns>
bool Area::deleteArea()
{
	if (left != NULL)
	{
		if (left->left != NULL)
		{
			if (!left->deleteArea()) { return false; }
			if (!right->deleteArea()) { return false; }
		}
	}
	if (right != NULL)
	{
		if (right->left != NULL)
		{
			if (!left->deleteArea()) { return false; }
			if (!right->deleteArea()) { return false; }
		}
	}
	delete right;
	right = NULL;
	delete left;
	left = NULL;
	return true;
}

/// <summary>set the area size.</summary>
/// <param name="width">Area width.</param>
/// <param name="height">Area height.</param>
void Area::setSize(int32_t width, int32_t height)
{
	w = width;
	h = height;
	size = width * height;
}

/// <summary> Get the X position of the area.</summary>
/// <returns>Return the area's x position.</returns>
inline int32_t Area::getX() const { return x; }

/// <summary>get the Y position of the area.</summary>
/// <returns>Return the area's y position.</returns>
inline int32_t Area::getY() const { return y; }

/// <summary>This function must be implemented by the user of the shell.The user should return its pvr::Shell object defining
/// the behaviour of the application.</summary>
/// <returns>Return The demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanExampleUI>(); }
