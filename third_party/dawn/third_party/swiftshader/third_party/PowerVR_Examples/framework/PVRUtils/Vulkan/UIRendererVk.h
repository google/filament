/*!
\brief Contains implementations of functions for the classes in UIRenderer.h
\file PVRUtils/Vulkan/UIRendererVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRUtils/Vulkan/SpriteVk.h"
#include "PVRUtils/Vulkan/HelperVk.h"
#include "PVRVk/CommandBufferVk.h"
#include "PVRVk/RenderPassVk.h"
#include "PVRVk/ApiObjectsVk.h"
#include "PVRUtils/Vulkan/MemoryAllocator.h"

namespace pvr {
namespace ui {
/// <summary>Manages and render the sprites.</summary>
class UIRenderer
{
public:
	/// <summary>Information used for uploading required info to the shaders (matrices, attributes etc).</summary>
	struct ProgramData
	{
		/// <summary>Uniform index information.</summary>
		enum Uniform
		{
			UniformMVPmtx,
			UniformFontTexture,
			UniformColor,
			UniformAlphaMode,
			UniformUVmtx,
			NumUniform
		};
		/// <summary>Attribute index information.</summary>
		enum Attribute
		{
			AttributeVertex,
			AttributeUV,
			NumAttribute
		};
		/// <summary>An array of uniforms used by the UIRenderer.</summary>
		int32_t uniforms[NumUniform];
		/// <summary>An array of attributes used by the UIRenderer.</summary>
		int32_t attributes[NumAttribute];
	};

	/// <summary>Retrieves the Font index buffer.</summary>
	/// <returns>The pvrvk::Buffer corresponding to the Font index buffer.</returns>
	const pvrvk::Buffer& getFontIbo()
	{
		if (!_fontIbo)
		{
			// create the FontIBO
			std::vector<uint16_t> fontFaces;
			fontFaces.resize(impl::Font_::FontElement);

			for (uint32_t i = 0; i < impl::Font_::MaxRenderableLetters; ++i)
			{
				fontFaces[i * 6] = static_cast<uint16_t>(0 + i * 4);
				fontFaces[i * 6 + 1] = static_cast<uint16_t>(3 + i * 4);
				fontFaces[i * 6 + 2] = static_cast<uint16_t>(1 + i * 4);

				fontFaces[i * 6 + 3] = static_cast<uint16_t>(3 + i * 4);
				fontFaces[i * 6 + 4] = static_cast<uint16_t>(0 + i * 4);
				fontFaces[i * 6 + 5] = static_cast<uint16_t>(2 + i * 4);
			}

			_fontIbo = utils::createBuffer(getDevice().lock(), pvrvk::BufferCreateInfo(sizeof(fontFaces[0]) * impl::Font_::FontElement, pvrvk::BufferUsageFlags::e_INDEX_BUFFER_BIT),
				pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, _vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
			pvr::utils::updateHostVisibleBuffer(_fontIbo, &fontFaces[0], 0, static_cast<uint32_t>(sizeof(fontFaces[0]) * fontFaces.size()), true);
		}
		return _fontIbo;
	}

	/// <summary>Retrieves the Image vertex buffer.</summary>
	/// <returns>The pvrvk::Buffer corresponding to the Image vertex buffer.</returns>
	const pvrvk::Buffer& getImageVbo()
	{
		if (!_imageVbo)
		{
			// create the image vbo
			const float verts[] = {
				/*    Position  */
				-1.f, 1.f, 0.f, 1.0f, 0.0f, 1.0f, // upper left
				-1.f, -1.f, 0.f, 1.0f, 0.f, 0.0f, // lower left
				1.f, 1.f, 0.f, 1.0f, 1.f, 1.f, // upper right
				-1.f, -1.f, 0.f, 1.0f, 0.f, 0.0f, // lower left
				1.f, -1.f, 0.f, 1.0f, 1.f, 0.0f, // lower right
				1.f, 1.f, 0.f, 1.0f, 1.f, 1.f, // upper right
			};
			_imageVbo = utils::createBuffer(getDevice().lock(), pvrvk::BufferCreateInfo(sizeof(verts), pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT),
				pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, _vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
			pvr::utils::updateHostVisibleBuffer(_imageVbo, static_cast<const void*>(verts), 0, sizeof(verts), true);
		}
		return _imageVbo;
	}
	/// <summary>Constructor. Does not produce a ready-to-use object, use the init function before use.</summary>
	UIRenderer() : _screenRotation(.0f), _numSprites(0) {}

	/// <summary>Move Constructor. Does not produce a ready-to-use object, use the init function before use.</summary>
	/// <param name="rhs">Another UIRenderer to initiialise from.</param>
	UIRenderer(UIRenderer&& rhs)
		: _renderpass(std::move(rhs._renderpass)), _subpass(std::move(rhs._subpass)), _programData(std::move(rhs._programData)), _defaultFont(std::move(rhs._defaultFont)),
		  _sdkLogo(std::move(rhs._sdkLogo)), _defaultTitle(std::move(rhs._defaultTitle)), _defaultDescription(std::move(rhs._defaultDescription)),
		  _defaultControls(std::move(rhs._defaultControls)), _device(std::move(rhs._device)), _pipelineLayout(std::move(rhs._pipelineLayout)), _pipeline(std::move(rhs._pipeline)),
		  _texDescLayout(std::move(rhs._texDescLayout)), _uboMvpDescLayout(std::move(rhs._uboMvpDescLayout)), _uboMaterialLayout(std::move(rhs._uboMaterialLayout)),
		  _samplerBilinear(std::move(rhs._samplerBilinear)), _samplerTrilinear(std::move(rhs._samplerTrilinear)), _descPool(std::move(rhs._descPool)),
		  _activeCommandBuffer(std::move(rhs._activeCommandBuffer)), _mustEndCommandBuffer(std::move(rhs._mustEndCommandBuffer)), _fontIbo(std::move(rhs._fontIbo)),
		  _imageVbo(std::move(rhs._imageVbo)), _screenDimensions(std::move(rhs._screenDimensions)), _screenRotation(std::move(rhs._screenRotation)),
		  _groupId(std::move(rhs._groupId)), _uboMvp(std::move(rhs._uboMvp)), _uboMaterial(std::move(rhs._uboMaterial)), _numSprites(std::move(rhs._numSprites)),
		  _sprites(std::move(rhs._sprites)), _textElements(std::move(rhs._textElements)), _fonts(std::move(rhs._fonts))
	{
		updateResourceOwnsership();
	}

	/// <summary>Assignment Operator overload. Does not produce a ready-to-use object, use the init function before use.</summary>
	/// <param name="rhs">Another UIRenderer to initiialise from.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	UIRenderer& operator=(UIRenderer&& rhs)
	{
		if (this == &rhs) { return *this; }
		_renderpass = std::move(rhs._renderpass);
		_subpass = std::move(rhs._subpass);
		_programData = std::move(rhs._programData);
		_defaultFont = std::move(rhs._defaultFont);
		_sdkLogo = std::move(rhs._sdkLogo);
		_defaultTitle = std::move(rhs._defaultTitle);
		_defaultDescription = std::move(rhs._defaultDescription);
		_defaultControls = std::move(rhs._defaultControls);
		_device = std::move(rhs._device);
		_pipelineLayout = std::move(rhs._pipelineLayout);
		_pipeline = std::move(rhs._pipeline);
		_texDescLayout = std::move(rhs._texDescLayout);
		_uboMvpDescLayout = std::move(rhs._uboMvpDescLayout);
		_uboMaterialLayout = std::move(rhs._uboMaterialLayout);
		_samplerBilinear = std::move(rhs._samplerBilinear);
		_samplerTrilinear = std::move(rhs._samplerTrilinear);
		_descPool = std::move(rhs._descPool);
		_activeCommandBuffer = std::move(rhs._activeCommandBuffer);
		_mustEndCommandBuffer = std::move(rhs._mustEndCommandBuffer);
		_fontIbo = std::move(rhs._fontIbo);
		_imageVbo = std::move(rhs._imageVbo);
		_screenDimensions = std::move(rhs._screenDimensions);
		_screenRotation = std::move(rhs._screenRotation);
		_groupId = std::move(rhs._groupId);
		_uboMvp = std::move(rhs._uboMvp);
		_uboMaterial = std::move(rhs._uboMaterial);
		_numSprites = std::move(rhs._numSprites);
		updateResourceOwnsership();
		return *this;
	}

	//!\cond NO_DOXYGEN
	UIRenderer& operator=(const UIRenderer& rhs) = delete;
	UIRenderer(UIRenderer& rhs) = delete;
	//!\endcond

	/// <summary>Return thedevice the UIRenderer was initialized with. If the UIrenderer was not initialized,
	/// behaviour is undefined.</summary>
	/// <returns>The pvrvk::Device which was used to initialise the UIRenderer.</returns>
	pvrvk::DeviceWeakPtr& getDevice() { return _device; }

	/// <summary>Return the device the UIRenderer was initialized with. If the UIrenderer was not initialized,
	/// behaviour is undefined.</summary>
	/// <returns>The pvrvk::Device which was used to initialise the UIRenderer.</returns>
	const pvrvk::DeviceWeakPtr& getDevice() const { return _device; }

	/// <summary>Returns the ProgramData used by this UIRenderer.</summary>
	/// <returns>The ProgramData structure used by the UIRenderer.</returns>
	const ProgramData& getProgramData() { return _programData; }

	/// <summary>Returns the GraphicsPipeline object used by this UIRenderer.</summary>
	/// <returns>The graphics pipeline being used by the UIRenderer.</returns>
	pvrvk::GraphicsPipeline getPipeline() { return _pipeline; }

	/// <summary>Returns the VMA allocator object used by this UIRenderer.</summary>
	/// <returns>The VMA allocator being used by the UIRenderer.</returns>
	pvr::utils::vma::Allocator& getMemoryAllocator() { return _vmaAllocator; }

	/// <summary>Returns the VMA allocator object used by this UIRenderer.</summary>
	/// <returns>The VMA allocator being used by the UIRenderer.</returns>
	const pvr::utils::vma::Allocator& getMemoryAllocator() const { return _vmaAllocator; }

	/// <summary>Check that we have called beginRendering() and not called endRendering. See the beginRendering() method.</summary>
	/// <returns>True if the command buffer is currently recording.</returns>
	bool isRendering() { return _activeCommandBuffer->isRecording(); }

	/// <summary>Initialize the UIRenderer with a graphics context.
	/// MUST BE called exactly once before use, after a valid
	/// graphics context is available (usually, during initView).
	/// Initialising creates its Default Text Font and PowerVR SDK logo. Therefore
	/// the calle must handle the texture uploads via assetLoader.</summary>
	/// <param name="width">The width of the screen used for rendering.</param>
	/// <param name="height">The height of the screen used for rendering</param>
	/// <param name="fullscreen">Indicates whether the rendering is occuring in full screen mode.</param>
	/// <param name="renderpass">A renderpass to use for this UIRenderer</param>
	/// <param name="subpass">The subpass to use for this UIRenderer</param>
	/// <param name="isFrameBufferSrgb">Specifies whether the render target is sRGB format. If not then a gamma correction is performed</param>
	/// <param name="commandPool">The pvrvk::CommandPool object to use for allocating command buffers</param>
	/// <param name="queue">The pvrvk::Queue object to use for submitting command buffers</param>
	/// <param name="createDefaultLogo">Specifies whether a default logo should be initialised</param>
	/// <param name="createDefaultTitle">Specifies whether a default title should be initialised</param>
	/// <param name="createDefaultFont">Specifies whether a default font should be initialised</param>
	/// <param name="maxNumInstances"> maximum number of sprite instances to be allocated from this uirenderer.
	/// it must be atleast maxNumSprites becasue each sprites is an instance on its own.</param>
	/// <param name="maxNumSprites"> maximum number of renderable sprites (Text and Images)
	/// to be allocated from this uirenderer</param>
	void init(uint32_t width, uint32_t height, bool fullscreen, const pvrvk::RenderPass& renderpass, uint32_t subpass, bool isFrameBufferSrgb, pvrvk::CommandPool& commandPool,
		pvrvk::Queue& queue, bool createDefaultLogo = true, bool createDefaultTitle = true, bool createDefaultFont = true, uint32_t maxNumInstances = 64, uint32_t maxNumSprites = 64);

	/// <summary>Destructor for the UIRenderer which will release all resources currently in use.</summary>
	~UIRenderer()
	{
		_defaultFont.reset();
		_defaultTitle.reset();
		_defaultDescription.reset();
		_defaultControls.reset();
		_sdkLogo.reset();
		_uboMaterial.reset();
		_uboMvp.reset();
		_texDescLayout.reset();
		_uboMvpDescLayout.reset();
		_uboMaterialLayout.reset();
		_pipelineLayout.reset();
		_pipeline.reset();
		_pipelineCache.reset();
		_samplerBilinear.reset();
		_samplerTrilinear.reset();
		_activeCommandBuffer.reset();
		_fontIbo.reset();
		_imageVbo.reset();
		_sprites.clear();
		_fonts.clear();
		_textElements.clear();
		_screenRotation = .0f;
		_numSprites = 0;
		_renderpass.reset();
		_vmaAllocator.reset();
		_descPool.reset();
		_device.reset();
	}

	/// <summary>Create a Text sprite. Initialize with std::string. Uses default font.</summary>
	/// <param name="text">std::string object that this Text object will be initialized with</param>
	/// <param name="maxLength">The maximum length of characters for the text element.</param>
	/// <returns>Text framework object.</returns>
	TextElement createTextElement(const std::string& text, uint32_t maxLength) { return createTextElement(text, _defaultFont, maxLength); }

	/// <summary>Create Text sprite from std::string.</summary>
	/// <param name="text">String object that this Text object will be initialized with</param>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.</param>
	/// <param name="maxLength">The maximum length of characters for the text element.</param>
	/// <returns>Text framework object.</returns>
	TextElement createTextElement(const std::string& text, const Font& font, uint32_t maxLength);

	/// <summary>Create a Text Element sprite from a pvr::ui::Font. A default string will be used</summary>
	/// <param name="font">The font that the text element will be using. The font must belong to the same UIrenderer object.</param>
	/// <param name="maxLength">The maximum length of characters for the text element.</param>
	/// <returns>Text framework object.</returns>
	TextElement createTextElement(const Font& font, uint32_t maxLength) { return createTextElement(std::string(""), font, maxLength); }

	/// <summary>Create Text sprite from wide std::wstring. Uses the Default Font.</summary>
	/// <param name="text">Wide std::string that this Text object will be initialized with. Will use the Default Font.</param>
	/// <param name="maxLength">The maximum length of characters for the text element.</param>
	/// <returns>Text framework object.</returns>
	TextElement createTextElement(const std::wstring& text, uint32_t maxLength) { return createTextElement(text, _defaultFont, maxLength); }

	/// <summary>Create Text sprite from wide std::string.</summary>
	/// <param name="text">text to be rendered.</param>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.</param>
	/// <param name="maxLength">The maximum length of characters for the text element.</param>
	/// <returns>Text framework object.</returns>
	TextElement createTextElement(const std::wstring& text, const Font& font, uint32_t maxLength);

	/// <summary>Create a Text sprite from a TextElement.</summary>
	/// <param name="textElement">text element to initialise a Text framework object from.</param>
	/// <returns>Text framework object</returns>
	Text createText(const TextElement& textElement);

	/// <summary>Create an empty Text sprite. Initialize with std::string. Uses default font.</summary>
	/// <param name="maxLength">The maximum length of characters for the text element.</param>
	/// <returns>Text framework object.</returns>
	Text createText(uint32_t maxLength = 255) { return createText(createTextElement("", maxLength)); }

	/// <summary>Create a Text sprite. Initialize with std::string. Uses default font.</summary>
	/// <param name="text">std::string object that this Text object will be initialized with</param>
	/// <param name="maxLength">The maximum length of characters for the text element.</param>
	/// <returns>Text framework object.</returns>
	Text createText(const std::string& text, uint32_t maxLength = 0) { return createText(createTextElement(text, maxLength)); }

	/// <summary>Create Text sprite from std::string.</summary>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.</param>
	/// <param name="text">String object that this Text object will be initialized with</param>
	/// <param name="maxLength">The maximum length of characters for the text element.</param>
	/// <returns>Text framework object.</returns>
	Text createText(const Font& font, const std::string& text, uint32_t maxLength = 0) { return createText(createTextElement(text, font, maxLength)); }

	/// <summary>Create Text sprite from std::string.</summary>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.</param>
	/// <param name="maxLength">The maximum length of characters for the text element.</param>
	/// <returns>Text framework object.</returns>
	Text createText(const Font& font, uint32_t maxLength = 255) { return createText(createTextElement("", font, maxLength)); }

	/// <summary>Create Text sprite from wide std::string. Uses the Default Font.</summary>
	/// <param name="text">Wide std::string that this Text object will be initialized with. Will use the Default Font.</param>
	/// <param name="maxLength">The maximum length of characters for the text element.</param>
	/// <returns>Text framework object.</returns>
	Text createText(const std::wstring& text, uint32_t maxLength = 0) { return createText(createTextElement(text, maxLength)); }

	/// <summary>Create Text sprite from wide std::string.</summary>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.</param>
	/// <param name="text">text to be rendered.</param>
	/// <returns>Text framework object.</returns>
	Text createText(const Font& font, const std::wstring& text) { return createText(createTextElement(text, font, static_cast<uint32_t>(text.length()))); }

	/// <summary>Create Text sprite from wide std::string.</summary>
	/// <param name="font">The font that the text will be using. The font must belong to the same UIrenderer object.</param>
	/// <param name="text">text to be rendered.</param>
	/// <param name="maxLength">The maximum length of characters for the text element.</param>
	/// <returns>Text framework object.</returns>
	Text createText(const Font& font, const std::wstring& text, uint32_t maxLength = 0) { return createText(createTextElement(text, font, maxLength)); }

	/// <summary>Get the X dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Width.</summary>
	/// <returns>Render width of the rectangle the UIRenderer is using for rendering.</returns>
	float getRenderingDimX() const { return _screenDimensions.x; }

	/// <summary>Get the Y dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Height.</summary>
	/// <returns>Render height of the rectangle the UIRenderer is using for rendering.</returns>
	float getRenderingDimY() const { return _screenDimensions.y; }

	/// <summary>Get the rendering dimensions of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Width and Screen Height.</summary>
	/// <returns>Render width and height of the rectangle the UIRenderer is using for rendering.</returns>
	glm::vec2 getRenderingDim() const { return _screenDimensions; }

	/// <summary>Get the viewport of the rectangle the UIRenderer is rendering to. Initial value is the Screen Width and Screen Height.</summary>
	/// <returns>Viewport width and height of the rectangle the UIRenderer is using for rendering.</returns>
	pvrvk::Rect2D getViewport() const
	{
		return pvrvk::Rect2D(pvrvk::Offset2D(0, 0), pvrvk::Extent2D(static_cast<int32_t>(getRenderingDimX()), static_cast<uint32_t>(getRenderingDimY())));
	}

	/// <summary>Set the X dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Width.</summary>
	/// <param name="value">The new rendering width.</param>
	void setRenderingDimX(float value) { _screenDimensions.x = value; }

	/// <summary>Set the Y dimension of the rectangle the UIRenderer is rendering to in order to scale UI elements.
	/// Initial value is the Screen Height.</summary>
	/// <param name="value">The new rendering height.</param>
	void setRenderingDimY(float value) { _screenDimensions.y = value; }

	/// <summary>Create a font from a given texture (use PVRTexTool to create the font texture from a font file).</summary>
	/// <param name="image">An ImageView object of the font texture file, which will be used directly.</param>
	/// <param name="textureHeader">A pvr::TextureHeader of the same object. Necessary for the texture metadata.</param>
	/// <param name="sampler">(Optional) A specific sampler object to use for this font.</param>
	/// <returns>A new Font object. Null object if failed.</returns>
	/// <remarks>Use PVRTexTool to create textures suitable for Font use. You can then use any of the createFont
	/// function overloads to create a pvr::ui::Font object to render your sprites. This overload requires that you
	/// have already created a Texture2D object from the file and is suitable for sharing a texture between different
	/// UIRenderer objects.</remarks>
	Font createFont(const pvrvk::ImageView& image, const TextureHeader& textureHeader, const pvrvk::Sampler& sampler = pvrvk::Sampler());

	/// <summary>Create a pvr::ui::Image from a pvrvk::ImageView image.
	/// NOTE: Creating new image requires re-recording the commandbuffer</summary>
	/// <param name="image">A TextureView object of the texture file. Must be 2D. It will be used
	/// directly.</param>
	/// <param name="sampler">(Optional) A specific sampler object to use for this font.</param>
	/// <returns>A new Image object of the specified texture. Null object if failed.</returns>
	Image createImage(const pvrvk::ImageView& image, const pvrvk::Sampler& sampler = pvrvk::Sampler());

	/// <summary>Create a pvr::ui::Image from a Texture Atlas asset.
	/// NOTE: Creating new image requires re-recording the commandbuffer</summary>
	/// <param name="image">A pvrvk::ImageView object. Will be internally used to create an pvrvk::Texture2D to
	/// use.</param>
	/// <param name="uv">Texture UV coordinate</param>
	/// <param name="sampler">A sampler used for this Image (Optional)</param>
	/// <returns>A new Image object of the specified texture. Null object if failed.</returns>
	Image createImageFromAtlas(const pvrvk::ImageView& image, const pvrvk::Rect2Df& uv, const pvrvk::Sampler& sampler = pvrvk::Sampler());

	/// <summary>Create a pvr::ui::MatrixGroup.</summary>
	/// <returns>A new Group to display different sprites together. Null object if failed.</returns>
	MatrixGroup createMatrixGroup();

	/// <summary>Create a pvr::ui::PixelGroup.</summary>
	/// <returns>A new Group to display different sprites together. Null object if failed.</returns>
	PixelGroup createPixelGroup();

	/// <summary>Begin rendering to a specific CommandBuffer. Must be called to render sprites. DO NOT update sprites
	/// after calling this function before calling endRendering.</summary>
	/// <param name="commandBuffer">The SecondaryCommandBuffer object where all the rendering commands will be put
	/// into.</param>
	/// <remarks>THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	/// UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group as many
	/// rendering commands as possible between begin and end, to avoid needless state changes.</remarks>
	void beginRendering(pvrvk::SecondaryCommandBuffer& commandBuffer) { beginRendering(commandBuffer, pvrvk::Framebuffer(), true); }

	/// <summary>Begin rendering to a specific CommandBuffer, using a specific Framebuffer and optionally a renderpass.
	/// Must be called to render sprites. DO NOT update sprites after calling this function before calling endRendering.</summary>
	/// <param name="commandBuffer">The SecondaryCommandBuffer object where all the rendering commands will be put into.</param>
	/// <param name="framebuffer">A framebuffer object which will be used to begin the command buffer.</param>
	/// <param name="useRenderPass">Specifies whether a RenderPass should be used to begin the command buffer.</param>
	/// <remarks>THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	/// UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group as many
	/// rendering commands as possible between begin and end, to avoid needless state changes.</remarks>
	void beginRendering(pvrvk::SecondaryCommandBuffer& commandBuffer, const pvrvk::Framebuffer& framebuffer, bool useRenderPass = false)
	{
		if (!commandBuffer->isRecording())
		{
			if (useRenderPass) { commandBuffer->begin(_renderpass, _subpass); }
			else
			{
				commandBuffer->begin(framebuffer, _subpass);
			}
			_mustEndCommandBuffer = true;
		}
		else
		{
			_mustEndCommandBuffer = false;
		}
		pvr::utils::beginCommandBufferDebugLabel(commandBuffer, pvrvk::DebugUtilsLabel("PVRUtilsVk::UIRenderer::Rendering"));
		commandBuffer->bindPipeline(getPipeline()); // bind the uirenderer pipeline
		_activeCommandBuffer = commandBuffer;
	}

	/// <summary>Begin rendering to a specific CommandBuffer. Must be called to render sprites. DO NOT update sprites after
	/// calling this function before calling endRendering.</summary>
	/// <param name="commandBuffer">The CommandBuffer object where all the rendering commands will be put into.</param>
	/// <remarks>THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	/// UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group as many
	/// rendering commands as possible between begin and end, to avoid needless state changes.</remarks>
	void beginRendering(pvrvk::CommandBuffer& commandBuffer)
	{
		debug_assertion(commandBuffer->isRecording(),
			"UIRenderer: If a Primary command buffer is passed to the UIRenderer,"
			" it must be in the Recording state");
		pvr::utils::beginCommandBufferDebugLabel(commandBuffer, pvrvk::DebugUtilsLabel("PVRUtilsVk::UIRenderer::Rendering"));
		_mustEndCommandBuffer = false;
		commandBuffer->bindPipeline(getPipeline()); // bind the uirenderer pipeline
		_activeCommandBuffer = commandBuffer;
	}

	/// <summary>Begin rendering to a specific CommandBuffer, with a custom user-provided GraphicsPipeline.</summary>
	/// <param name="commandBuffer">The SecondaryCommandBuffer object where all the rendering commands will be put into.</param>
	/// <param name="pipe">The GraphicsPipeline to use for rendering.</param>
	/// <remarks>THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	/// UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group as many
	/// rendering commands as possible between beginRendering and endRendering, to avoid needless state changes. Use
	/// this overload to render with a custom GraphicsPipeline.</remarks>
	void beginRendering(pvrvk::SecondaryCommandBuffer commandBuffer, pvrvk::GraphicsPipeline& pipe) { beginRendering(commandBuffer, pipe, pvrvk::Framebuffer(), true); }

	/// <summary>Begin rendering to a specific CommandBuffer, with a custom user-provided GraphicsPipeline.</summary>
	/// <param name="commandBuffer">The SecondaryCommandBuffer object where all the rendering commands will be put into.</param>
	/// <param name="pipe">The GraphicsPipeline to use for rendering.</param>
	/// <param name="framebuffer">A framebuffer object which will be used to begin the command buffer.</param>
	/// <param name="useRenderPass">Specifies whether a RenderPass should be used to begin the command buffer.</param>
	/// <remarks>THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	/// UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group as many
	/// rendering commands as possible between beginRendering and endRendering, to avoid needless state changes. Use
	/// this overload to render with a custom GraphicsPipeline.</remarks>
	void beginRendering(pvrvk::SecondaryCommandBuffer commandBuffer, pvrvk::GraphicsPipeline& pipe, const pvrvk::Framebuffer& framebuffer, bool useRenderPass = false)
	{
		if (!commandBuffer->isRecording())
		{
			if (useRenderPass) { commandBuffer->begin(_renderpass, _subpass); }
			else
			{
				commandBuffer->begin(framebuffer, _subpass);
			}
			_mustEndCommandBuffer = true;
		}
		else
		{
			_mustEndCommandBuffer = false;
		}
		pvr::utils::beginCommandBufferDebugLabel(commandBuffer, pvrvk::DebugUtilsLabel("PVRUtilsVk::UIRenderer::Rendering"));
		commandBuffer->bindPipeline(pipe);
		_activeCommandBuffer = commandBuffer;
	}

	/// <summary>Begin rendering to a specific CommandBuffer, with a custom user-provided GraphicsPipeline.</summary>
	/// <param name="commandBuffer">The CommandBuffer object where all the rendering commands will be put into.</param>
	/// <param name="pipe">The GraphicsPipeline to use for rendering.</param>
	/// <remarks>THIS METHOD OR ITS OVERLOAD MUST BE CALLED BEFORE RENDERING ANY SPRITES THAT BELONG TO A SPECIFIC
	/// UIRenderer. The sequence must always be beginRendering, render ..., endRendering. Always try to group as many
	/// rendering commands as possible between beginRendering and endRendering, to avoid needless state changes. Use
	/// this overload to render with a custom GraphicsPipeline.</remarks>
	void beginRendering(pvrvk::CommandBuffer commandBuffer, pvrvk::GraphicsPipeline& pipe)
	{
		debug_assertion(commandBuffer->isRecording(),
			"UIRenderer: If a Primary command buffer is passed to the UIRenderer,"
			" it must be in the Recording state");
		pvr::utils::beginCommandBufferDebugLabel(commandBuffer, pvrvk::DebugUtilsLabel("PVRUtilsVk::UIRenderer::Rendering"));
		_mustEndCommandBuffer = false;
		commandBuffer->bindPipeline(pipe);
		_activeCommandBuffer = commandBuffer;
	}

	/// <summary>End rendering. Always call this method before submitting the commandBuffer passed to the UIRenderer.</summary>
	/// <remarks>This method must be called after you finish rendering sprites (after a call to beginRendering). The
	/// sequence must always be beginRendering, render ..., endRendering. Try to group as many of the rendering
	/// commands (preferably all) between beginRendering and endRendering.</remarks>
	void endRendering()
	{
		if (_activeCommandBuffer)
		{
			pvr::utils::endCommandBufferDebugLabel(_activeCommandBuffer);
			if (_mustEndCommandBuffer)
			{
				_mustEndCommandBuffer = false;
				_activeCommandBuffer->end();
			}
			_activeCommandBuffer.reset();
		}
	}

	/// <summary>Get the CommandBuffer that is being used to currently render.</summary>
	/// <returns>If between a beginRendering and endRendering, the CommandBuffer used at beginRendering. Otherwise,
	/// null.</returns>
	pvrvk::CommandBufferBase& getActiveCommandBuffer() { return _activeCommandBuffer; }

	/// <summary>The UIRenderer has a built-in default pvr::ui::Font that can always be used when the UIRenderer is
	/// initialized. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The default font. Constant overload.</returns>
	const Font& getDefaultFont() const { return _defaultFont; }

	/// <summary>The UIRenderer has a built-in default pvr::ui::Font that can always be used when the UIRenderer is
	/// initialized. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>A pvr::ui::Font object of the default font.</returns>
	Font& getDefaultFont() { return _defaultFont; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Image of the PowerVR SDK logo that can always be used when the
	/// UIRenderer is initialized. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The PowerVR SDK pvr::ui::Image. Constant overload.</returns>
	const Image& getSdkLogo() const { return _sdkLogo; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Image of the PowerVR SDK logo that can always be used when the
	/// UIRenderer is initialized. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The PowerVR SDK pvr::ui::Image.</returns>
	Image& getSdkLogo() { return _sdkLogo; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for used as a title (top-left,
	/// large) for convenience. Set the text of this sprite and use it as normal. Can be resized and repositioned at
	/// will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default title pvr::ui::Text. Originally empty (use setText on it). Constant overload.</returns>
	const Text& getDefaultTitle() const { return _defaultTitle; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for used as a title (top-left,
	/// large) for convenience. Set the text of this sprite and use it as normal. Can be resized and repositioned at
	/// will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default title pvr::ui::Text. Originally empty (use setText on it).</returns>
	Text& getDefaultTitle() { return _defaultTitle; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for use as a description (subtitle)
	/// (top-left, below DefaultTitle, small)) for convenience. Set the text of this sprite and use it as normal. Can
	/// be resized and repositioned at will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default descritption pvr::ui::Text. Originally empty (use setText on it). Constant overload.</returns>
	const Text& getDefaultDescription() const { return _defaultDescription; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for used as a description (subtitle
	/// (top-left, below DefaultTitle, small)) for convenience. Set the text of this sprite and use it as normal. Can
	/// be resized and repositioned at will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default descritption pvr::ui::Text. Originally empty (use setText on it).</returns>
	Text& getDefaultDescription() { return _defaultDescription; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for use as a controls display
	/// (bottom-left, small text) for convenience. You can set the text of this sprite and use it as normal. Can be
	/// resized and repositioned at will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default Controls pvr::ui::Text. Originally empty (use setText on it). Constant overload.</returns>
	const Text& getDefaultControls() const { return _defaultControls; }

	/// <summary>The UIRenderer has a built-in pvr::ui::Text positioned and sized for use as a controls display
	/// (bottom-left, small text) for convenience. You can set the text of this sprite and use it as normal. Can be
	/// resized and repositioned at will. Used throughout the PowerVR SDK Examples.</summary>
	/// <returns>The Default Controls pvr::ui::Text. Originally empty (use setText on it).</returns>
	Text& getDefaultControls() { return _defaultControls; }

	/// <summary>Return the PipelineLayout object of the internal Pipeline object used by this
	/// UIRenderer.</summary>
	/// <returns>The PipelineLayout.</returns>
	pvrvk::PipelineLayout getPipelineLayout() { return _pipelineLayout; }

	/// <summary>Returns the projection matrix</summary>
	/// <returns>The UIRenderer projection matrix</returns>
	glm::mat4 getProjection() const { return pvr::math::ortho(Api::Vulkan, 0.0, getRenderingDimX(), 0.0f, getRenderingDimY()); }

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const pvrvk::DescriptorSetLayout&</returns>
	void rotateScreen90degreeCCW()
	{
		_screenRotation += glm::pi<float>() * .5f;
		std::swap(_screenDimensions.x, _screenDimensions.y);
	}

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const pvrvk::DescriptorSetLayout&</returns>
	void rotateScreen90degreeCW()
	{
		_screenRotation -= glm::pi<float>() * .5f;
		std::swap(_screenDimensions.x, _screenDimensions.y);
	}

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const pvrvk::DescriptorSetLayout&</returns>
	glm::mat4 getScreenRotation() const { return glm::rotate(_screenRotation, glm::vec3(0.0f, 0.0f, 1.f)); }

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const pvrvk::DescriptorSetLayout&</returns>
	const pvrvk::DescriptorSetLayout& getTexDescriptorSetLayout() const { return _texDescLayout; }

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const pvrvk::DescriptorSetLayout&</returns>
	const pvrvk::DescriptorSetLayout& getUboDescSetLayout() const { return _uboMvpDescLayout; }

	/// <summary>Returns maximum renderable sprites (Text and Images)</summary>
	/// <returns>The maximum number of renderable sprites</returns>
	uint32_t getMaxRenderableSprites() const { return _uboMaterial._numArrayId; }

	/// <summary>Return maximum number of instances supported (including sprites and groups)</summary>
	/// <returns>The maximum number of instances</returns>
	uint32_t getMaxInstances() const { return _uboMvp._numArrayId; }

	/// <summary>return the number of available renderable sprites (Image and Text)</summary>
	/// <returns>The number of remaining sprite slots</returns>
	uint32_t getNumAvailableSprites() const { return _uboMaterial.getNumAvailableBufferArrays(); }

	/// <summary>return the number of availble instance</summary>
	/// <returns>The number of remaining instance slots</returns>
	uint32_t getNumAvailableInstances() const { return _uboMvp.getNumAvailableBufferArrays(); }

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const pvrvk::DescriptorSetLayout&</returns>
	pvrvk::DescriptorPool& getDescriptorPool() { return _descPool; }

	/// <summary>Return the bilinear sampler used by the UIRenderer</summary>
	/// <returns>The bilinear sampler used by the UIRenderer</returns>
	pvrvk::Sampler& getSamplerBilinear() { return _samplerBilinear; }

	/// <summary>Return the trilinear sampler used by the UIRenderer</summary>
	/// <returns>The trilinear sampler used by the UIRenderer</returns>
	pvrvk::Sampler& getSamplerTrilinear() { return _samplerTrilinear; }

private:
	void updateResourceOwnsership()
	{
		std::for_each(_sprites.begin(), _sprites.end(), [this](SpriteWeakRef& sprite) { sprite.lock()->setUIRenderer(this); });

		std::for_each(_fonts.begin(), _fonts.end(), [this](FontWeakRef& font) { font.lock()->setUIRenderer(this); });

		std::for_each(_textElements.begin(), _textElements.end(), [this](TextElementWeakRef& textElement) { textElement.lock()->setUIRenderer(this); });
	}

	friend class pvr::ui::impl::Image_;
	friend class pvr::ui::impl::Text_;
	friend class pvr::ui::impl::Group_;
	friend class pvr::ui::impl::Sprite_;
	friend class pvr::ui::impl::Font_;

	/// <summary>return the default DescriptorSetLayout. ONLY to be used by the Sprites</summary>
	/// <returns>const pvrvk::DescriptorSetLayout&</returns>
	uint64_t generateGroupId() { return _groupId++; }

	struct UboMvp
	{
		friend class ::pvr::ui::UIRenderer;
		UboMvp() : _freeArrayId(0) {}
		void init(pvrvk::Device& device, pvrvk::DescriptorSetLayout& descLayout, pvrvk::DescriptorPool& pool, UIRenderer& uirenderer);
		void initLayout(pvrvk::Device& device, uint32_t numElements);

		void reset()
		{
			_buffer.reset();
			_uboDescSetSet.reset();
		}

		void updateMvp(uint32_t bufferArrayId, const glm::mat4x4& mvp);

		int32_t getNewBufferSlice()
		{
			if (_freeArrayIds.size())
			{
				const uint32_t id = _freeArrayIds.back();
				_freeArrayIds.pop_back();
				return static_cast<int32_t>(id);
			}
			return (_freeArrayId < _numArrayId ? static_cast<int32_t>(_freeArrayId++) : -1);
		}

		void releaseBufferSlice(uint32_t id)
		{
			debug_assertion(id < _numArrayId, "Invalid id");
			_freeArrayIds.emplace_back(id);
		}

		void bindUboDynamic(pvrvk::CommandBufferBase& cb, const pvrvk::PipelineLayout& pipelayout, uint32_t mvpBufferSlice)
		{
			uint32_t dynamicOffsets[] = { static_cast<uint32_t>(_structuredBufferView.getDynamicSliceOffset(mvpBufferSlice)) };
			cb->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelayout, 1, _uboDescSetSet, dynamicOffsets, ARRAY_SIZE(dynamicOffsets));
		}

		uint32_t getNumAvailableBufferArrays() const { return static_cast<uint32_t>((_numArrayId - _freeArrayId) + _freeArrayIds.size()); }

	private:
		uint32_t _freeArrayId;
		utils::StructuredBufferView _structuredBufferView;
		pvrvk::Buffer _buffer;
		pvr::utils::vma::Allocation _memAllocation;
		pvrvk::DescriptorSet _uboDescSetSet;
		uint32_t _numArrayId;
		std::vector<uint32_t> _freeArrayIds;
	};

	struct UboMaterial
	{
	public:
		friend class ::pvr::ui::UIRenderer;
		UboMaterial() : _freeArrayId(0) {}

		void reset()
		{
			_buffer.reset();
			_uboDescSetSet.reset();
		}

		void init(pvrvk::Device& device, pvrvk::DescriptorSetLayout& descLayout, pvrvk::DescriptorPool& pool, UIRenderer& uirenderer);
		void initLayout(pvrvk::Device& device, uint32_t numArrayId);

		void updateMaterial(uint32_t arrayIndex, const glm::vec4& color, int32_t alphaMode, const glm::mat4& uv);

		int32_t getNewBufferArray()
		{
			if (_freeArrayIds.size())
			{
				const uint32_t id = _freeArrayIds.back();
				_freeArrayIds.pop_back();
				return static_cast<int32_t>(id);
			}
			return (_freeArrayId < _numArrayId ? static_cast<int32_t>(_freeArrayId++) : -1);
		}

		void releaseBufferArray(uint32_t id)
		{
			debug_assertion(id < _numArrayId, "Invalid id");
			_freeArrayIds.emplace_back(id);
		}

		void bindUboDynamic(pvrvk::CommandBufferBase& cb, const pvrvk::PipelineLayout& pipelayout, uint32_t bufferSlice)
		{
			uint32_t dynamicOffsets[] = { static_cast<uint32_t>(_structuredBufferView.getDynamicSliceOffset(bufferSlice)) };
			cb->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelayout, 2, _uboDescSetSet, dynamicOffsets, ARRAY_SIZE(dynamicOffsets));
		}

		uint32_t getNumAvailableBufferArrays() const { return static_cast<uint32_t>(((_numArrayId - _freeArrayId) + _freeArrayIds.size())); }

	private:
		pvrvk::DescriptorSet _uboDescSetSet;
		uint32_t _freeArrayId;
		uint32_t _numArrayId;
		utils::StructuredBufferView _structuredBufferView;
		pvrvk::Buffer _buffer;
		std::vector<uint32_t> _freeArrayIds;
	};

	UboMvp& getUbo() { return _uboMvp; }

	UboMaterial& getMaterial() { return _uboMaterial; }

	void setUpUboPoolLayouts(uint32_t numInstances, uint32_t numSprites);
	void setUpUboPools(uint32_t numInstances, uint32_t numSprites);

	void initCreateDefaultFont(pvrvk::CommandBuffer& cmdBuffer);
	void initCreateDefaultSdkLogo(pvrvk::CommandBuffer& cmdBuffer);
	void initCreateDefaultSampler();
	void initCreateDefaultTitle();
	void initCreatePipeline(bool isFramebufferSrgb);
	void initCreateDescriptorSetLayout();

	pvr::utils::vma::Allocator _vmaAllocator;
	std::vector<SpriteWeakRef> _sprites;
	std::vector<TextElementWeakRef> _textElements;
	std::vector<FontWeakRef> _fonts;

	pvrvk::RenderPass _renderpass;
	uint32_t _subpass;
	ProgramData _programData;
	Font _defaultFont;
	Image _sdkLogo;
	Text _defaultTitle;
	Text _defaultDescription;
	Text _defaultControls;
	pvrvk::DeviceWeakPtr _device;

	pvrvk::PipelineLayout _pipelineLayout;
	pvrvk::GraphicsPipeline _pipeline;
	pvrvk::PipelineCache _pipelineCache;
	pvrvk::DescriptorSetLayout _texDescLayout;
	pvrvk::DescriptorSetLayout _uboMvpDescLayout;
	pvrvk::DescriptorSetLayout _uboMaterialLayout;
	pvrvk::Sampler _samplerBilinear;
	pvrvk::Sampler _samplerTrilinear;
	pvrvk::DescriptorPool _descPool;
	pvrvk::CommandBufferBase _activeCommandBuffer;
	bool _mustEndCommandBuffer;
	pvrvk::Buffer _fontIbo;
	pvrvk::Buffer _imageVbo;
	glm::vec2 _screenDimensions;
	float _screenRotation;
	uint64_t _groupId = 1;
	UboMvp _uboMvp;
	UboMaterial _uboMaterial;
	uint32_t _numSprites;

	// Methods and Members related to MoltenVK support.
#ifdef VK_USE_PLATFORM_MACOS_MVK

	MVKConfiguration mvkConfig;
	size_t sizeOfMVK = 0;
	bool isFullImageViewSwizzleMVK = false;

#endif
};
} // namespace ui
} // namespace pvr
