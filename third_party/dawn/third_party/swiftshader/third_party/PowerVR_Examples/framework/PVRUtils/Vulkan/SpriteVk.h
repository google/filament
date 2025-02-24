/*!
\brief Contains the Sprite classes and framework objects used by the UIRenderer (Sprite, Text, Image, Font, Group).
\file PVRUtils/Vulkan/SpriteVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/PVRVk.h"
#include "PVRUtils/StructuredMemory.h"
#include "PVRCore/math/AxisAlignedBox.h"
#include "PVRCore/strings/StringFunctions.h"
#include "PVRCore/texture/Texture.h"

//!\cond NO_DOXYGEN
#define NUM_BITS_GROUP_ID 8
//!\endcond

/// <summary>Main PowerVR Namespace</summary>
namespace pvr {
//!\cond NO_DOXYGEN
class Texture;
//!\endcond
/// <summary>Main namespace for the PVRUtils Library. Contains the UIRenderer and the several Sprite classes.
///</summary>
namespace ui {
//!\cond NO_DOXYGEN
class UIRenderer;
namespace impl {
class Font_;
class Text_;
class TextElement_;
class Image_;
class Group_;
class MatrixGroup_;
class PixelGroup_;
class Sprite_;
} // namespace impl
//!\endcond

/// <summary>A simple std::runtime_error wrapper for UIRenderer error codes.</summary>
class UIRendererError : public std::runtime_error
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="error">The error message to log.</param>
	explicit UIRendererError(const std::string& error) : std::runtime_error("UIRenderer Error | " + error) {}
};

/// <summary>A simple extension of a UIRendererError used to throwing errors due to reaching the maximum number of instances supported by a UIRenderer.</summary>
class UIRendererInstanceMaxError : public UIRendererError
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="error">The error message to log.</param>
	explicit UIRendererInstanceMaxError(const std::string& error) : UIRendererError("Maximum number of instances supported by this UIRenderer reached | " + error) {}
};

/// <summary>A Reference Counted Framework Object wrapping the Group_ class. Groups several sprites to apply
/// some transformation to them and render them all together.</summary>
typedef std::shared_ptr<impl::Group_> Group;

/// <summary>A Reference Counted Framework Object wrapping the MatrixGroup_ class. Groups several sprites to apply
/// some matrix transformation to them and render them all together.</summary>
typedef std::shared_ptr<impl::MatrixGroup_> MatrixGroup;

/// <summary>A Reference Counted Framework Object wrapping the PixelGroup_ class. Groups several sprites to apply
/// intuitive 2D operations and layouts to them.</summary>
typedef std::shared_ptr<impl::PixelGroup_> PixelGroup;

/// <summary>A Reference Counted Framework Object wrapping the Sprite_ interface. Represents anything you can
/// use with the UIRenderer (Font, Text, Image, Group).</summary>
typedef std::shared_ptr<impl::Sprite_> Sprite;

/// <summary>A weak reference Counted Framework Object wrapping the Sprite_ interface. Represents anything you can
/// use with the UIRenderer (Font, Text, Image, Group).</summary>
typedef std::weak_ptr<impl::Sprite_> SpriteWeakRef;

/// <summary>A Reference Counted Framework Object wrapping the Text_ interface.</summary>
typedef std::shared_ptr<impl::Text_> Text;

/// <summary>A weak reference Counted Framework Object wrapping the Text_ interface.</summary>
typedef std::weak_ptr<impl::Text_> TextWeakRef;

/// <summary>A Reference Counted Framework Object wrapping the Font_ class. Is an Image object augmented by font
/// metadata. Is used by the Text class.</summary>
typedef std::shared_ptr<impl::Font_> Font;

/// <summary>A weak reference Counted Framework Object wrapping the Font_ class. Is an Image object augmented by font
/// metadata. Is used by the Text class.</summary>
typedef std::weak_ptr<impl::Font_> FontWeakRef;

/// <summary>A Reference Counted Framework Object wrapping the TextElement_ class. The TextElement is a Sprite and contains a
/// std::string of characters to be displayed with the Font that it uses.</summary>
typedef std::shared_ptr<impl::TextElement_> TextElement;

/// <summary>A weak reference Counted Framework Object wrapping the TextElement_ class. The TextElement is a Sprite and contains a
/// std::string of characters to be displayed with the Font that it uses.</summary>
typedef std::weak_ptr<impl::TextElement_> TextElementWeakRef;

/// <summary>A Reference Counted Framework Object wrapping the Image_ class. The Image is a Sprite and contains
/// a 2D texture that can be displayed.</summary>
typedef std::shared_ptr<impl::Image_> Image;

/// <summary>A weak Reference Counted Framework Object wrapping the Image_ class. The Image is a Sprite and contains
/// a 2D texture that can be displayed.</summary>
typedef std::weak_ptr<impl::Image_> ImageWeakRef;

/// <summary>An Enumeration of all the Anchor points that can be used to position a Sprite. An anchor point is
/// the point to which all positioning will be relative to. Use this to facilitate the laying out of UIs.</summary>
enum class Anchor
{
	TopLeft,
	TopCenter,
	TopRight,
	CenterLeft,
	Center,
	CenterRight,
	BottomLeft,
	BottomCenter,
	BottomRight
};

/// <summary>Contains the implementation of the raw UIRenderer classes. In order to use the library, use the PowerVR
/// Framework objects found in the pvr::ui namespace</summary>
namespace impl {
/// <summary>Base sprite class. Use through the Sprite framework object. Represents something that can be rendered
/// with the UIRenderer. Texts, Images, Groups are all sprites (Fonts too although it would only be used as a
/// sprite to display the entire font's glyphs).</summary>
class Sprite_
{
private:
	friend class pvr::ui::UIRenderer;
	friend class pvr::ui::impl::Group_;
	friend class pvr::ui::impl::Font_;
	friend class pvr::ui::impl::PixelGroup_;
	friend class pvr::ui::impl::MatrixGroup_;
	friend class pvr::ui::impl::Image_;
	friend class pvr::ui::impl::TextElement_;
	friend class pvr::ui::impl::Text_;

	/// <summary>Do not call directly. CommitUpdates will call this function. If writing new sprite classes,
	/// implement this function to calculate the mvp matrix of the sprite from any other possible representation that
	/// the sprite's data contains into its own _cachedMatrix member.</summary>
	virtual void calculateMvp(uint64_t parentIds, glm::mat4 const& srt, const glm::mat4& viewProj, pvrvk::Rect2D const& viewport) const = 0;

	virtual void onRender(pvrvk::CommandBufferBase& commands, uint64_t) { (void)commands; }

	/// <summary>A function to call when adding a particular instance.</summary>
	/// <param name="instance">The instance id of the parent.</param>
	virtual void onAddInstance(uint64_t parentId) = 0;

	/// <summary>A function to call when removing a particular instance.</summary>
	/// <param name="instance">The instance id of the parent.</param>
	virtual void onRemoveInstance(uint64_t parentId) { (void)parentId; }

	/// <summary>Constructor for a sprite.</summary>
	/// <param name="uiRenderer">The UIRenderer to use for rendering this sprite.</param>
	explicit Sprite_(UIRenderer& uiRenderer);

protected:
	/// <summary>Setter for the current UIRenderer being used for the Sprite.</summary>
	/// <param name="uiRenderer">The new UIRenderer to use for rendering this sprite.</param>
	void setUIRenderer(UIRenderer* uiRenderer) { _uiRenderer = uiRenderer; }

	/// <summary>Bounding rectangle of the sprite.</summary>
	mutable math::AxisAlignedBox _boundingRect;

	/// <summary>Modulation color (multiplicative).</summary>
	mutable glm::vec4 _color;

	/// <summary>Set the shader to render alpha-only.</summary>
	mutable int32_t _alphaMode;

	/// <summary>UIRenderer this sprite belongs to.</summary>
	UIRenderer* _uiRenderer;

	/// <summary>The cached transformation matrix.</summary>
	mutable glm::mat4 _cachedMatrix;

	/// <summary>View projection matrix.</summary>
	glm::mat4 _viewProj;

	/// <summary>Determines whether the sprite name should be updated.</summary>
	/// <returns>Returns true if the sprite name is dirty and the cached string must be updated.</returns>
	bool isSpriteNameDirty() const { return true; }

	/// <summary>A cached sprite name.</summary>
	std::string _spriteName;

public:
	/// <summary>Call this function after changing the sprite in any way, in order to update its internal
	/// information. This function should be called before any rendering commands are submitted and
	/// before calling functions such as getDimensions, in order to actually process all the changes
	/// to the sprite.</summary>
	virtual void commitUpdates() const;

	/// <summary>Virtual Descructor for a Sprite.</summary>
	virtual ~Sprite_() {}

	/// <summary>Get the Sprite's bounding box. If the sprite has changed, the value returned is only valid after
	/// calling the commitUpdates function</summary>
	/// <returns>The Sprite's bounding box.</returns>
	glm::vec2 getDimensions() const { return glm::vec2(_boundingRect.getSize()); }

	/// <summary>Render is the normal function to call to render a sprite. Before calling this function, call
	/// beginRendering on the uiRenderer this sprite belongs to to set up the commandBuffer to render to. In general
	/// try to group as many render commands as possible between the beginRendering and endRendering. This overload
	/// does not apply any transformations to the sprite.</summary>
	void render();

	/// <summary>Use this to use this sprite as Alpha channel only, setting its color to 1,1,1,a. Otherwise, an Alpha
	/// texture would render black. Always use this setting to render Fonts that have been generated with PVRTexTool
	/// as Alpha textures.</summary>
	/// <param name="isAlphaOnly">Pass "true" to flush all color channels to 1.0 and keep the alpha channel. Pass false
	/// (default state) to render with texture colors unchanged.</param>
	void setAlphaRenderingMode(bool isAlphaOnly) const { _alphaMode = isAlphaOnly; }

	/// <summary>Set a modulation (multiplicative) color to the sprite, as a vector of normalised 32 bit float values.
	/// Range of values must be 0..1</summary>
	/// <param name="color">A glm::vec4 that contains color and alpha values in the range of 0..1. Initial value
	/// (1.0,1.0,1.0,1.0)</param>
	void setColor(glm::vec4 color) const { _color = color; }

	/// <summary>Set a modulation (multiplicative) color to the sprite, as bytes (0..255).</summary>
	/// <param name="r">Red channel. Initial value 255.</param>
	/// <param name="g">Green channel. Initial value 255.</param>
	/// <param name="b">Blue channel. Initial value 255.</param>
	/// <param name="a">Alpha channel. Initial value 255.</param>
	void setColor(uint32_t r, uint32_t g, uint32_t b, uint32_t a) const
	{
		_color[0] = static_cast<float>(r / 255.f);
		_color[1] = static_cast<float>(g / 255.f);
		_color[2] = static_cast<float>(b / 255.f);
		_color[3] = static_cast<float>(a / 255.f);
	}

	/// <summary>Set a modulation (multiplicative) color to the sprite, as normalised floating point values. Values
	/// must be in the range of 0..1</summary>
	/// <param name="r">Red channel. Initial value 1.</param>
	/// <param name="g">Green channel. Initial value 1.</param>
	/// <param name="b">Blue channel. Initial value 1.</param>
	/// <param name="a">Alpha channel. Initial value 1.</param>
	void setColor(float r, float g, float b, float a) const
	{
		_color.r = r;
		_color.g = g;
		_color.b = b;
		_color.a = a;
	}

	/// <summary>Set a modulation (multiplicative) color to the sprite, as bytes packed into an integer.</summary>
	/// <param name="rgba">8 bit groups, least significant bits: Red, then Green, then Blue, then most significant
	/// bits is Alpha.</param>
	void setColor(uint32_t rgba) const
	{
		_color[0] = static_cast<float>(rgba & 0xFF) / 255.f;
		_color[1] = static_cast<float>(rgba >> 8 & 0xFF) / 255.f;
		_color[2] = static_cast<float>(rgba >> 16 & 0xFF) / 255.f;
		_color[3] = static_cast<float>(rgba >> 24 & 0xFF) / 255.f;
	}

	/// <summary>Get the modulation (multiplicative) color of the sprite, as a glm::vec4.</summary>
	/// <returns>The sprites's modulation color. Values are normalised in the range of 0..1</returns>
	const glm::vec4& getColor() const { return _color; }

	/// <summary>Get the sprite name.</summary>
	/// <returns>The sprites's name</returns>
	virtual const std::string& getSpriteName()
	{
		if (isSpriteNameDirty()) { _spriteName = "Sprite"; }
		return _spriteName;
	}

	/// <summary>This setting queries if this is set to render as Alpha channel only, (setting its color to 1,1,1,a).
	/// Otherwise, an Alpha texture would render black. This setting is typically used to render Text with Fonts that
	/// have been generated with PVRTexTool as Alpha textures.</summary>
	/// <returns>"true" if set to render as Alpha, false otherwise.</returns>
	bool getAlphaRenderingMode() const { return _alphaMode == 1; }

	/// <summary>Get the sprite's own transformation matrix. Does not contain hierarchical transformations from groups
	/// etc. This function is valid only after any changes to the sprite have been commited with commitUpdates as it
	/// is normally calculated in commitUpdates.</summary>
	/// <returns>The sprite's final transformation matrix. If the sprite is rendered on its own, this is the matrix
	/// that will be uploaded to the shader.</returns>
	const glm::mat4& getMatrix() const { return _cachedMatrix; }

	/// <summary>Get the Sprite's bounding box. If the sprite has changed, the value returned is only valid after
	/// calling the commitUpdates function</summary>
	/// <returns>The Sprite's bounding box.</returns>
	math::AxisAlignedBox const& getBoundingBox() const { return _boundingRect; }

	/// <summary>Retrieves the groups scaled dimension based on each childs current scaling factor.</summary>
	/// <returns>The groups scaled dimension.</returns>
	virtual glm::vec2 getScaledDimension() const = 0;
};

/// <summary>A component that can be positioned in 2D using 2d position, scale, rotation and anchored using its
/// center or corners.</summary>
class I2dComponent
{
private:
	friend class pvr::ui::UIRenderer;
	friend class pvr::ui::impl::PixelGroup_;
	friend class pvr::ui::impl::Text_;
	friend class pvr::ui::impl::Image_;

	I2dComponent()
		: _anchor(Anchor::Center), _position(0.f, 0.f), _scale(1.f, 1.f), _rotation(0.f), _isPositioningDirty(true), _pixelOffset(0, 0), _uv(0.0f, 0.0f, 1.0, 1.0f), _isUVDirty(true)
	{}

protected:
	/// <summary>The position in the sprite relative to which all positioning calculations are done.</summary>
	mutable Anchor _anchor;

	/// <summary>Position of the sprite relative to its UIRenderer area.</summary>
	mutable glm::vec2 _position;

	/// <summary>Scale of the sprite. A scale of 1 means natural size (1:1 mapping of sprite to screen pixels).</summary>
	mutable glm::vec2 _scale;

	/// <summary>Rotation of the sprite, in radians.</summary>
	mutable float _rotation;

	/// <summary>Used to avoid unnecessary expensive calculations if commitUpdate is called unnecessarily.</summary>
	mutable bool _isPositioningDirty;

	/// <summary>The pixel offset used by this I2dComponent.</summary>
	mutable glm::vec2 _pixelOffset;

	/// <summary>UV coordinates for the I2dComponent.</summary>
	mutable pvrvk::Rect2Df _uv;

	/// <summary>Used to avoid unnecessary expensive calculations if commitUpdate is called unnecessarily.</summary>
	mutable bool _isUVDirty;

public:
	/// <summary>Virtual Descructor for a I2dComponent.</summary>
	virtual ~I2dComponent() {}

	/// <summary>Set the anchor and position ("centerpoint") of this component. The anchor is the point around which
	/// all operations (e.g. scales, rotations) will happen.</summary>
	/// <param name="anchor">The anchor point</param>
	/// <param name="ndcPos">The normalized device coordinates (-1..1) where the anchor should be in its group</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	I2dComponent const* setAnchor(Anchor anchor, const glm::vec2& ndcPos)
	{
		setAnchor(anchor, ndcPos.x, ndcPos.y);
		return this;
	}

	/// <summary>Set the anchor and position ("centerpoint") of this component. The anchor is the point around which
	/// all operations (e.g. scales, rotations) will happen.</summary>
	/// <param name="anchor">The anchor point</param>
	/// <param name="ndcPosX">The normalized (-1..1) horizontal coordinate where the anchor should be in its group</param>
	/// <param name="ndcPosY">The normalized (-1..1) vertical coordinate where the anchor should be in its group</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	I2dComponent const* setAnchor(Anchor anchor, float ndcPosX = -1.f, float ndcPosY = -1.f) const
	{
		_anchor = anchor;
		_position.x = ndcPosX;
		_position.y = ndcPosY;
		_isPositioningDirty = true;
		return this;
	}

	/// <summary>Set the pixel offset of this object. Pixel offset is applied after every other calculation, so it
	/// always moves the final (transformed) sprite by the specified number of pixels in each direction.</summary>
	/// <param name="offsetX">Number of pixels to move the sprite right (negative for left)</param>
	/// <param name="offsetY">Number of pixels to move the sprite up (negative for down)</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	I2dComponent const* setPixelOffset(float offsetX, float offsetY) const
	{
		_pixelOffset.x = offsetX, _pixelOffset.y = offsetY;
		_isPositioningDirty = true;
		return this;
	}

	/// <summary>Set the pixel offset of this object. Pixel offset is applied after every other calculation, so it
	/// always moves the final (transformed) sprite by the specified number of pixels in each direction.</summary>
	/// <param name="offset">Number of pixels to move the sprite right/up (negative for down/left)</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	I2dComponent const* setPixelOffset(glm::vec2 offset) const { return setPixelOffset(offset.x, offset.y); }

	/// <summary>Set the scale of this object.</summary>
	/// <param name="scale">The scale of this object. (1,1) is natural scale.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	I2dComponent const* setScale(glm::vec2 const& scale) const
	{
		_scale = scale;
		_isPositioningDirty = true;
		return this;
	}

	/// <summary>Set the scale of this object.</summary>
	/// <param name="scaleX">The scale of this object in the X direction. 1 is natural scale.</param>
	/// <param name="scaleY">The scale of this object in the Y direction. 1 is natural scale.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	I2dComponent const* setScale(float scaleX, float scaleY) const
	{
		_scale = glm::vec2(scaleX, scaleY);
		_isPositioningDirty = true;
		return this;
	}

	/// <summary>Set the rotation of this object on the screen (in fact, its parent group's) plane.</summary>
	/// <param name="radians">The Counter Clockwise rotation of this object, in radians, around its Z axis.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	I2dComponent const* setRotation(float radians) const
	{
		_rotation = radians;
		_isPositioningDirty = true;
		return this;
	}

	/// <summary>Set the uv coordinates for this object.</summary>
	/// <param name="uv">The UV coordinate to use for this object.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	I2dComponent const* setUV(const pvrvk::Rect2Df& uv) const
	{
		_uv = uv;
		_isUVDirty = true;
		return this;
	}
};

/// <summary>Use this class through the reference counted Framework Object pvr::ui::Image. Represents a 2D Image (aka
/// Texture). Can be used like all Sprites and additionally contains methods required for working with Images.</summary>
class Image_ : public Sprite_, public I2dComponent
{
private:
	friend class pvr::ui::UIRenderer;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class Image_;
	};

	static Image constructShared(UIRenderer& uiRenderer, const pvrvk::ImageView& imageView, uint32_t width, uint32_t height, const pvrvk::Sampler& sampler)
	{
		return std::make_shared<Image_>(make_shared_enabler{}, uiRenderer, imageView, width, height, sampler);
	}

	/// <summary>Function that will be automatically called by the uiRenderer. Do not call.</summary>
	void calculateMvp(uint64_t parentIds, const glm::mat4& srt, const glm::mat4& viewProj, const pvrvk::Rect2D& viewport) const;

	/// <summary>Function that will be automatically called by the uiRenderer. Do not call.</summary>
	void onRender(pvrvk::CommandBufferBase& commands, uint64_t parentId);

	void updateTextureDescriptorSet() const;

	void updateUbo(uint64_t parentIds) const;

	void onRemoveInstance(uint64_t parentid);

	void onAddInstance(uint64_t parentId);

protected:
	/// <summary>Holds data specific to rendering a particular sprite in particular its model view project matrix.</summary>
	struct MvpUboData
	{
		/// <summary>Model view project matrix.</summary>
		glm::mat4 mvp;
		/// <summary>A buffer array identifier.</summary>
		mutable int32_t bufferArrayId;

		/// <summary>Default constructor for the MvpUboData structure.</summary>
		MvpUboData() : bufferArrayId(-1) {}
	};

	/// <summary>Holds data specific to rendering a particular sprite in particular its color and alpha mode.</summary>
	struct MaterialUboData
	{
		/// <summary>Rendering color.</summary>
		glm::vec4 color;
		/// <summary>Specifies whether alpha rendering mode is being used.</summary>
		uint32_t isAlphaMode;

		/// <summary>A buffer array identifier.</summary>
		mutable int32_t bufferArrayId;

		/// <summary>Default constructor for the MaterialUboData structure.</summary>
		MaterialUboData() : bufferArrayId(-1) {}
	};

	/// <summary>Holds data specific to rendering a particular sprite in particular its color and alpha mode.</summary>
	MaterialUboData _materialData;

	/// <summary>The descriptor set containing the texture of this object.</summary>
	mutable pvrvk::DescriptorSet _texDescSet;

	/// <summary>Width of the image.</summary>
	uint32_t _texW;

	/// <summary>Height of the image.</summary>
	uint32_t _texH;

	/// <summary>The texture object of this image.</summary>
	pvrvk::ImageView _imageView;

	/// <summary>The sampler used by this image.</summary>
	pvrvk::Sampler _sampler;

	/// <summary>A map containing the model view projection data for all paths for this Image.</summary>
	mutable std::map<uint64_t, MvpUboData> _mvpData;

	/// <summary>Used to avoid unnecessary expensive calculations if commitUpdate is called unnecessarily.</summary>
	mutable bool _isTextureDirty;

public:
	//!\cond NO_DOXYGEN
	/// <summary>Internal</summary>
	Image_(make_shared_enabler, UIRenderer& uiRenderer, const pvrvk::ImageView& imageView, uint32_t width, uint32_t height, const pvrvk::Sampler& sampler);
	//!\endcond

	/// <summary>Virtual Descructor for a Image_.</summary>
	virtual ~Image_() { onRemoveInstance(0); }

	/// <summary>Get the width of this image width in pixels.</summary>
	/// <returns>Image width in pixels.</returns>
	uint32_t getWidth() const { return _texW; }

	/// <summary>Get the height of this image width in pixels.</summary>
	/// <returns>Image width in pixels.</returns>
	uint32_t getHeight() const { return _texH; }

	/// <summary>Retrieve the Texture2D object that this Image wraps.</summary>
	/// <returns>The Texture2D object that this Image wraps.</returns>
	const pvrvk::ImageView& getImageView() const { return _imageView; }

	/// <summary>Retrieve the Texture2D object that this Image wraps.</summary>
	/// <returns>The Texture2D object that this Image wraps.</returns>
	pvrvk::ImageView& getImageView() { return _imageView; }

	/// <summary>Retrieve the Sampler that this Image will use for sampling the texture. Const overload.</summary>
	/// <returns>The Sampler that this Image will use for sampling the texture. Const overload.</returns>
	const pvrvk::Sampler& getSampler() const { return _sampler; }

	/// <summary>Retrieve the Sampler that this Image will use for sampling the texture.</summary>
	/// <returns>The Sampler that this Image will use for sampling the texture.</returns>
	pvrvk::Sampler& getSampler() { return _sampler; }

	/// <summary>Retrieve the descriptorSet containing this Image's texture.</summary>
	/// <returns>The descriptorSet containing this Image's texture.</returns>
	const pvrvk::DescriptorSet& getTexDescriptorSet() const
	{
		updateTextureDescriptorSet();
		return _texDescSet;
	}

	/// <summary>Get the size of this texture after applying scale</summary>
	/// <returns>The size of this texture after applying scale</returns>
	glm::vec2 getScaledDimension() const { return getDimensions() * _scale; }

	/// <summary>Get the sprite name.</summary>
	/// <returns>The sprites's name</returns>
	const std::string& getSpriteName()
	{
		if (isSpriteNameDirty()) { _spriteName = getImageView()->getObjectName(); }
		return _spriteName;
	}
};

/// <summary>Use this class through the reference counted Framework Object pvr::ui::Font. Is an Image_ containing font
/// characters along with the metadata necessary for rendering text with them. Although it can be used like an
/// Image_, this does not make some sense since it would just display the characters as a texture atlas. Text
/// objects will contain a reference to a Font to render with.</summary>
class Font_
{
public:
	/// <summary>struct containing the UV's corresponding to the UV coordinates of a character of a Font.</summary>
	struct CharacterUV
	{
		/// <summary>The ul coordinate.</summary>
		float ul;
		/// <summary>The vt coordinate.</summary>
		float vt;
		/// <summary>The ur coordinate.</summary>
		float ur;
		/// <summary>The vb coordinate.</summary>
		float vb;
	};

	/// <summary>struct representing the metrics of a character of a Font.</summary>
	struct CharMetrics
	{
		/// <summary>Prefix offset.</summary>
		int16_t xOff;
		/// <summary>The width of the character.</summary>
		uint16_t characterWidth;
	};

	/// <summary>Enumeration values useful for text rendering. PVRTexTool uses these values when creating fonts.</summary>
	enum
	{
		InvalidChar = 0xFDFDFDFD,
		FontHeader = 0xFCFC0050,
		FontCharList = 0xFCFC0051,
		FontRects = 0xFCFC0052,
		FontMetrics = 0xFCFC0053,
		FontYoffset = 0xFCFC0054,
		FontKerning = 0xFCFC0055,
		MaxRenderableLetters = 0xFFFF >> 2,
		FontElement = MaxRenderableLetters * 6,
	};

private:
	friend class pvr::ui::UIRenderer;

	static int32_t characterCompFunc(const void* a, const void* b);
	static int32_t kerningCompFunc(const void* a, const void* b);

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class Font_;
	};

	static Font constructShared(UIRenderer& uiRenderer, const pvrvk::ImageView& tex2D, const TextureHeader& textureHeader, const pvrvk::Sampler& sampler)
	{
		return std::make_shared<Font_>(make_shared_enabler{}, uiRenderer, tex2D, textureHeader, sampler);
	}

	void setUIRenderer(UIRenderer* uiRenderer) { _uiRenderer = uiRenderer; }

	struct Header // 12 bytes
	{
		uint8_t version; /*!< Version of Font_. */
		uint8_t spaceWidth; /*!< The width of the 'Space' character. */
		int16_t numCharacters; /*!< Total number of characters contained in this file. */
		int16_t numKerningPairs; /*!< Number of characters which kern against each other. */
		int16_t ascent; /*!< The height of the character, in pixels, from the base line. */
		int16_t lineSpace; /*!< The base line to base line dimension, in pixels. */
		int16_t borderWidth; /*!< px Border around each character. */
	} _header;
#pragma pack(push, 4) // force 4byte alignment
	struct KerningPair
	{
		uint64_t pair; /*!< Shifted and OR'd pair for 32bit characters */
		int32_t offset; /*!< Kerning offset (in pixels) */
	};
#pragma pack(pop)
	std::vector<uint32_t> _characters;
	std::vector<KerningPair> _kerningPairs;
	std::vector<CharMetrics> _charMetrics;
	std::vector<CharacterUV> _characterUVs;
	std::vector<pvrvk::Rect2D> _rects;
	std::vector<int32_t> _yOffsets;
	pvrvk::ImageView _imageView;
	glm::uvec2 _dim;
	uint32_t _alphaRenderingMode;
	pvrvk::DescriptorSet _texDescSet;
	UIRenderer* _uiRenderer;

public:
	//!\cond NO_DOXYGEN
	/// <summary>Constructor. Do not use - use the UIRenderer::createFont.</summary>
	Font_(make_shared_enabler, UIRenderer& uiRenderer, const pvrvk::ImageView& tex2D, const TextureHeader& textureHeader, const pvrvk::Sampler& sampler);
	//!\endcond

	/// <summary>Load the font data from the font texture.</summary>
	/// <param name="texture">The pvr::Texture texture to load font data from.</param>
	/// <returns>Returns 'True' if the font was loaded successfully.</returns>
	void loadFontData(const Texture& texture);

	/// <summary>Find the index of a character inside the internal font character list. Only useful for custom font
	/// use.</summary>
	/// <param name="character">The value of a character. Accepts ASCII through to UTF32 characters.</param>
	/// <returns>The index of the character inside the internal font list.</returns>
	uint32_t findCharacter(uint32_t character) const;

	/// <summary>Apply kerning to two characters (give the offset required by the specific pair).</summary>
	/// <param name="charA">The first (left) character of the pair.</param>
	/// <param name="charB">The second (right) character of the pair.</param>
	/// <param name="offset">Output parameter, the offset that must be applied to the second character due to kerning.</param>
	void applyKerning(uint32_t charA, uint32_t charB, float& offset);

	/// <summary>Get the character metrix of this font</summary>
	/// <param name="index">The internal index of the character. Use findCharacter to get the index of a specific known character.</param>
	/// <returns>A CharMetrics object representing the character metrics of the character with that index.</returns>
	const CharMetrics& getCharMetrics(uint32_t index) const { return _charMetrics[index]; }

	/// <summary>Get the UVs of the characters of this font</summary>
	/// <param name="index">The internal index of the character. Use findCharacter to get the index of a specific
	/// known character.</param>
	/// <returns>A CharMetrics object representing the character metrics of the character with that index.</returns>
	const CharacterUV& getCharacterUV(uint32_t index) const { return _characterUVs[index]; }

	/// <summary>Get the rectangle for a specific character</summary>
	/// <param name="index">The internal index of the character. Use findCharacter to get the index of a specific
	/// known character.</param>
	/// <returns>The rectangle where this character exists in the font texture.</returns>
	const pvrvk::Rect2D& getRectangle(uint32_t index) const { return _rects[index]; }

	/// <summary>Get the spacing between baseline to baseline of this font, in pixels.</summary>
	/// <returns>The spacing between baseline to baseline of this font, in pixels.</returns>
	int16_t getFontLineSpacing() const { return _header.lineSpace; }

	/// <summary>Get the distance between baseline to Ascent of this font, in pixels.</summary>
	/// <returns>The distance from Baseline to Ascent of this font, in pixels.</returns>
	int16_t getAscent() const { return _header.ascent; }

	/// <summary>Get the width, in pixels, of the Space character.</summary>
	/// <returns>The width, in pixels, of the Space character.</returns>
	uint8_t getSpaceWidth() const { return _header.spaceWidth; }

	/// <summary>Get the Y offset of the font.</summary>
	/// <param name="index">The internal index of the character. Use findCharacter to get the index of a specific known character.</param>
	/// <returns>The Y offset of the font.</returns>
	int32_t getYOffset(uint32_t index) const { return _yOffsets[index]; }

	/// <summary>Get whether the font uses alpha rendering mode.</summary>
	/// <returns>True if alpha rendering mode is being used. False otherwise.</returns>
	bool isAlphaRendering() const { return _alphaRenderingMode != 0; }

	/// <summary>Retrieve the descriptorSet containing this Font's texture.</summary>
	/// <returns>The descriptorSet containing this Font's texture.</returns>
	const pvrvk::DescriptorSet& getTexDescriptorSet() const { return _texDescSet; }

	/// <summary>Retrieve the image view containing this font's texture.</summary>
	/// <returns>This Font's texture.</returns>
	const pvrvk::ImageView& getImageView() const { return _imageView; }
};

/// <summary>UIRenderer vertex format.</summary>
struct Vertex
{
	/// <summary>x position.</summary>
	float x;
	/// <summary>y position.</summary>
	float y;
	/// <summary>z position.</summary>
	float z;
	/// <summary>w coordinate.</summary>
	float rhw;

	/// <summary>texture u coordinate.</summary>
	float tu;
	/// <summary>texture v coordinate.</summary>
	float tv;

	/// <summary>Setter for a Vertex's data</summary>
	/// <param name="inX">The x position.</param>
	/// <param name="inY">The y position.</param>
	/// <param name="inZ">The z position.</param>
	/// <param name="inRhw">The w coordinate.</param>
	/// <param name="inU">The texture u coordinate.</param>
	/// <param name="inV">The texture v coordinate.</param>
	void setData(float inX, float inY, float inZ, float inRhw, float inU, float inV)
	{
		this->x = inX;
		this->y = inY;
		this->z = inZ;
		this->rhw = inRhw;
		this->tu = inU;
		this->tv = inV;
	}
};

/// <summary>The TextElement class should be used through the reference counted Framework Object pvr::ui::TextElement. The TextElement_ class handles the
/// implementation specifics for creating, managing and rendering text elements to the screen including buffer creation, updates and deletion
/// as well as the rendering of the text element.</summary>
class TextElement_
{
private:
	friend class pvr::ui::impl::Text_;
	friend class pvr::ui::UIRenderer;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class TextElement_;
	};

	static TextElement constructShared(UIRenderer& uiRenderer, const Font& font, uint32_t maxTextLength = 255)
	{
		return std::make_shared<TextElement_>(make_shared_enabler{}, uiRenderer, font, maxTextLength);
	}

	static TextElement constructShared(UIRenderer& uiRenderer, const std::wstring& str, const Font& font, uint32_t maxTextLength = 0)
	{
		return std::make_shared<TextElement_>(make_shared_enabler{}, uiRenderer, str, font, maxTextLength);
	}

	static TextElement constructShared(UIRenderer& uiRenderer, const std::string& str, const Font& font, uint32_t maxTextLength = 0)
	{
		return std::make_shared<TextElement_>(make_shared_enabler{}, uiRenderer, str, font, maxTextLength);
	}

	bool updateText() const
	{
		if (_isTextDirty)
		{
			regenerateText();
			updateVbo();
			_isTextDirty = false;
			return true;
		}
		return false;
	}

	void setUIRenderer(UIRenderer* uiRenderer) { _uiRenderer = uiRenderer; }

	void createBuffers();
	void regenerateText() const;
	void updateVbo() const;
	/// <summary>Function that will be automatically called by the uiRenderer. Do not call.</summary>
	void onRender(pvrvk::CommandBufferBase& commands);

	/// <summary>Function that will be automatically called by the uiRenderer. Do not call.</summary>
	uint32_t updateVertices(float fZPos, float xPos, float yPos, const std::vector<uint32_t>& text, Vertex* const pVertices) const;

	bool _isUtf8;
	mutable bool _isTextDirty;
	mutable Font _font;
	mutable pvrvk::Buffer _vbo;
	mutable pvrvk::Buffer _drawIndirectBuffer;
	mutable uint32_t _maxLength;
	mutable std::string _textStr;
	mutable std::wstring _textWStr;
	mutable std::vector<uint32_t> _utf32;
	mutable std::vector<Vertex> _vertices;
	mutable int32_t _numCachedVerts;
	mutable math::AxisAlignedBox _boundingRect; //< Bounding rectangle of the sprite
	UIRenderer* _uiRenderer;

public:
	//!\cond NO_DOXYGEN
	/// <summary>Constructor for a Text element. Do not use - use the UIRenderer::createTextElement</summary>
	/// <param name="uiRenderer">The UIRenderer to use when creating the Font.</param>
	/// <param name="font">The font to use for the text element.</param>
	/// <param name="maxTextLength">The maximum number of characters for the text element.</param>
	TextElement_(make_shared_enabler, UIRenderer& uiRenderer, const Font& font, uint32_t maxTextLength)
		: _isTextDirty(true), _font(font), _maxLength(maxTextLength), _uiRenderer(&uiRenderer)
	{
		_maxLength = _maxLength ? _maxLength : 255;
		createBuffers();
		updateText();
	}

	/// <summary>Constructor for a Text element. Do not use - use the UIRenderer::createTextElement</summary>
	/// <param name="uiRenderer">The UIRenderer to use when creating the Font.</param>
	/// <param name="str">The Text string to use for the text element.</param>
	/// <param name="font">The font to use for the text element.</param>
	/// <param name="maxTextLength">The maximum number of characters for the text element. If less than strlen(str),
	/// it is implicitly set to strlen(<paramref name="str"/>)</param>
	TextElement_(make_shared_enabler, UIRenderer& uiRenderer, const std::string& str, const Font& font, uint32_t maxTextLength = 0)
		: _isTextDirty(true), _font(font), _maxLength(std::max<uint32_t>(static_cast<uint32_t>(str.length()), maxTextLength)), _uiRenderer(&uiRenderer)
	{
		_maxLength = _maxLength ? _maxLength : 255;
		createBuffers();
		setText(str);
		updateText();
	}

	/// <summary>Constructor for a Text element. Do not use - use the UIRenderer::createTextElement</summary>
	/// <param name="uiRenderer">The UIRenderer to use when creating the Font.</param>
	/// <param name="str">The Text string to use for the text element.</param>
	/// <param name="font">The font to use for the text element.</param>
	/// <param name="maxTextLength">The maximum number of characters for the text element. If less than strlen(str),
	/// it is implicitly set to strlen(<paramref name="str"/>)</param>
	TextElement_(make_shared_enabler, UIRenderer& uiRenderer, const std::wstring& str, const Font& font, uint32_t maxTextLength = 0)
		: _isTextDirty(true), _font(font), _maxLength(std::max<uint32_t>(static_cast<uint32_t>(str.length()), maxTextLength)), _uiRenderer(&uiRenderer)
	{
		_maxLength = _maxLength ? _maxLength : 255;
		createBuffers();
		setText(str);
		updateText();
	}
	//!\endcond

	/// <summary>The maximum number of letters supported by a TextElement</summary>
	enum
	{
		MaxLetters = 5120
	};

	/// <summary>Get the Sprite's bounding box dimensions. If the sprite has changed, the value returned is only valid after
	/// calling the commitUpdates function</summary>
	/// <returns>The Sprite's bounding box dimensions.</returns>
	glm::vec2 getDimensions() const { return glm::vec2(_boundingRect.getSize()); }

	/// <summary>Get the Sprite's bounding box. If the sprite has changed, the value returned is only valid after
	/// calling the commitUpdates function</summary>
	/// <returns>The Sprite's bounding box.</returns>
	math::AxisAlignedBox const& getBoundingBox() const { return _boundingRect; }

	/// <summary>Sets the text element text from a std::string</summary>
	/// <param name="str">The new text value to use for the text element.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	TextElement_& setText(const std::string& str);

	/// <summary>Sets the text element text from a std::string</summary>
	/// <param name="str">The new text value to use for the text element.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	TextElement_& setText(std::string&& str);

	/// <summary>Sets the text element text from a std::wstring</summary>
	/// <param name="str">The new text value to use for the text element.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	TextElement_& setText(const std::wstring& str);

	/// <summary>Sets the text element text from a std::wstring</summary>
	/// <param name="str">The new text value to use for the text element.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	TextElement_& setText(std::wstring&& str);

	/// <summary>Gets the text element current text string</summary>
	/// <returns>The text element current text string</returns>
	const std::string& getString() const { return _textStr; }

	/// <summary>Gets the text element current text wstring</summary>
	/// <returns>The text element current text wstring</returns>
	const std::wstring& getWString() const { return _textWStr; }

	/// <summary>Gets the text element current font</summary>
	/// <returns>The text element current font</returns>
	const Font& getFont() const { return _font; }
};

/// <summary>Use this class through the reference counted Framework Object pvr::ui::Text. Represents some text that can
/// be rendered as a normal Sprite_ and additionally contains the necessary text manipulation functions.</summary>
class Text_ : public Sprite_, public I2dComponent
{
private:
	friend class pvr::ui::UIRenderer;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class Text_;
	};

	static Text constructShared(UIRenderer& uiRenderer, const TextElement& textElement) { return std::make_shared<Text_>(make_shared_enabler{}, uiRenderer, textElement); }

	void onRemoveInstance(uint64_t parentId);

	struct MvpUboData
	{
		glm::mat4 mvp; // model-view-projection
		mutable int32_t bufferArrayId;
		MvpUboData() : bufferArrayId(-1) {}
	};

	struct MaterialUboData
	{
		glm::vec4 color;
		uint32_t isAlphaMode;
		mutable int32_t bufferArrayId;
		MaterialUboData() : bufferArrayId(-1) {}
	} _materialData;

	const pvrvk::DescriptorSet& getTexDescriptorSet() const { return getFont()->getTexDescriptorSet(); }
	void onAddInstance(uint64_t parentId);

	void calculateMvp(uint64_t parentIds, glm::mat4 const& srt, const glm::mat4& viewProj, pvrvk::Rect2D const& viewport) const;

	void onRender(pvrvk::CommandBufferBase& commands, uint64_t parentId);

	void updateUbo(uint64_t parentId) const;
	mutable TextElement _textElement;
	mutable std::map<uint64_t, MvpUboData> _mvpData;

	std::string _imageViewObjectName;
	std::string _vboObjectName;
	std::string _drawIndirectBufferObjectName;

public:
	//!\cond NO_DOXYGEN
	/// <summary>Constructor. Do not use - use UIRenderer::createText</summary>
	Text_(make_shared_enabler, UIRenderer& uiRenderer, const TextElement& textElement);
	//!\endcond

	/// <summary>Virtual Descructor for a Text_.</summary>
	virtual ~Text_() { onRemoveInstance(0); }

	/// <summary>Gets the text objects current font</summary>
	/// <returns>The text objects current font</returns>
	const Font getFont() const { return getTextElement()->getFont(); }

	/// <summary>Gets the text objects current text element</summary>
	/// <returns>The text objects current text element</returns>
	TextElement getTextElement() { return _textElement; }

	/// <summary>Gets the text objects current text element</summary>
	/// <returns>The text objects current text element</returns>
	const TextElement getTextElement() const { return _textElement; }

	/// <summary>Get the size of this texture after applying scale</summary>
	/// <returns>The size of this texture after applying scale</returns>
	glm::vec2 getScaledDimension() const { return getDimensions() * _scale; }

	/// <summary>Sets the text element text from a std::string</summary>
	/// <param name="str">The new text value to use for the text element.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	Text_& setText(const std::string& str)
	{
		getTextElement()->setText(str);
		return *this;
	}

	/// <summary>Sets the text element text from a std::string</summary>
	/// <param name="str">The new text value to use for the text element.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	Text_& setText(std::string&& str)
	{
		getTextElement()->setText(std::forward<std::string>(str));
		return *this;
	}

	/// <summary>Sets the text element text from a std::wstring</summary>
	/// <param name="str">The new text value to use for the text element.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	Text_& setText(const std::wstring& str)
	{
		getTextElement()->setText(str);
		return *this;
	}

	/// <summary>Sets the text element text from a std::wstring</summary>
	/// <param name="str">The new text value to use for the text element.</param>
	/// <returns>this object (allow chaining commands with ->)</returns>
	Text_& setText(std::wstring&& str)
	{
		getTextElement()->setText(std::forward<std::wstring>(str));
		return *this;
	}

	/// <summary>Determines whether the sprite name should be updated.</summary>
	/// <returns>Returns true if the sprite name is dirty and the cached string must be updated.</returns>
	bool isSpriteNameDirty() const
	{
		return !(_textElement->getFont()->getImageView()->getObjectName() == _imageViewObjectName && _textElement->_vbo->getObjectName() == _vboObjectName);
	}

	/// <summary>Get the sprite name.</summary>
	/// <returns>The sprites's name</returns>
	const std::string& getSpriteName()
	{
		if (isSpriteNameDirty())
		{
			_imageViewObjectName = _textElement->getFont()->getImageView()->getObjectName();
			_vboObjectName = _textElement->_vbo->getObjectName();
			_drawIndirectBufferObjectName = _textElement->_drawIndirectBuffer->getObjectName();
			_spriteName = "ImageView: " + _imageViewObjectName;
			_spriteName += ", Vbo: " + _vboObjectName;
			_spriteName += ", Indirect Buffer: " + _drawIndirectBufferObjectName;
		}
		return _spriteName;
	}
};

/// <summary>Abstract container for sprites. See MatrixGroup or PixelGroup. A group contains references to a number of
/// sprites, allowing hierarchical transformations to be applied to them</summary>
/// <remarks>A very complex transformation using the "id" member is used to optimize the group transformations,
/// as each child needs to hold the transformations</remarks>
class Group_ : public Sprite_
{
private:
	friend class pvr::ui::impl::PixelGroup_;
	friend class pvr::ui::impl::MatrixGroup_;

	/// <summary>Internal function that is triggered when all item transformations have been applied and the final matrices can be calculated.</summary>
	/// <param name="parentId">The groups parent's id.</param>
	/// <param name="srt">A scale rotate translate matrix.</param>
	/// <param name="viewProjection">A view projection matrix.</param>
	/// <param name="viewport">The viewport currently in use.</param>
	void calculateMvp(uint64_t parentIds, glm::mat4 const& srt, const glm::mat4& viewProj, pvrvk::Rect2D const& viewport) const
	{
		glm::mat4 tmpMatrix = srt * _cachedMatrix;
		// My cached matrix should always be up-to-date unless overridden. No effect.
		for (ChildContainer::iterator it = _children.begin(); it != _children.end(); ++it) { (*it)->calculateMvp(packId(parentIds, _id), tmpMatrix, viewProj, viewport); }
	}

	/// <summary>Internal function that UIRenderer calls to render. Do not call directly.</summary>
	virtual void onRender(pvrvk::CommandBufferBase& commandBuffer, uint64_t parentId)
	{
		for (ChildContainer::iterator it = _children.begin(); it != _children.end(); ++it) { (*it)->onRender(commandBuffer, packId(parentId, _id)); }
	}

	/// <summary>Constructor. Internal use. The parameter groupid is an implementation detail used to implement and optimize
	/// group behaviour, and cannot be trivially determined.</summary>
	Group_(UIRenderer& uiRenderer, uint64_t groupid) : Sprite_(uiRenderer), _id(groupid) {}

	struct SpriteEntryEquals
	{
		Sprite sprite;
		SpriteEntryEquals(const Sprite& sprite) : sprite(sprite) {}
		bool operator()(const Sprite& rhs) { return sprite == rhs; }
	};

	void onRemoveInstance(uint64_t parentId)
	{
		for (uint32_t i = 0; i < _children.size(); ++i) { _children[i]->onRemoveInstance(packId(parentId, _id)); }
	}

	void onAddInstance(uint64_t parentId)
	{
		for (ChildContainer::iterator it = _children.begin(); it != _children.end(); ++it) { (*it)->onAddInstance(packId(parentId, _id)); }
	}
	uint64_t packId(uint64_t parentIds, uint64_t id) const
	{
		uint64_t packed = parentIds << NUM_BITS_GROUP_ID;
		return packed | id;
	}

protected:
	/// <summary>A ChildContainer typedef is a vector of Sprites</summary>
	typedef std::vector<Sprite> ChildContainer;

	/// <summary>The child Sprites which this group maintains</summary>
	mutable ChildContainer _children;

	/// <summary>This groups id</summary>
	uint64_t _id;

public:
	/// <summary>Add a Sprite (Text, Image etc.) to this Group. All sprites in the group will be transformed together
	/// when calling Render on the group.
	/// NOTE: Adding  Sprites in to group requires re-recording the commandbuffer</summary>
	/// <param name="sprite">The Sprite to add.</param>
	/// <returns>Pointer to this object, in order to easily chan add commands.</returns>
	Group_* add(const Sprite& sprite);

	/// <summary>Adds number of Sprites (Text, Image etc.) to this Group. All sprites in the group will be transformed together
	/// when calling Render on the group.
	/// NOTE: Adding  Sprites in to group requires re-recording the commandbuffer</summary>
	/// <param name="sprites">A pointer to an array of Sprites to add.</param>
	/// <param name="numSprites">The number of sprites to add from the array pointed to by sprites.</param>
	/// <returns>Pointer to this object, in order to easily chan add commands.</returns>
	void add(const Sprite* sprites, uint32_t numSprites)
	{
		std::for_each(sprites, sprites + numSprites, [&](const Sprite& sprite) { add(sprite); });
	}

	/// <summary>Remove a Sprite from this Group. Linear search (Complexity O(n) )</summary>
	/// <param name="sprite">The Sprite to remove.</param>
	void remove(const Sprite& sprite)
	{
		ChildContainer::iterator it = std::find_if(_children.begin(), _children.end(), SpriteEntryEquals(sprite));
		if (it != _children.end())
		{
			(*it)->onRemoveInstance(_id);
			_children.erase(it);
		}
	}

	/// <summary>Remove all sprites in this group. Requires commandbuffer re-recording inorder to take affect</param>
	void removeAll()
	{
		_children.erase(_children.begin(), _children.end());
		_boundingRect.clear();
	}

	glm::vec2 getScaledDimension() const
	{
		glm::vec2 dim(0);
		for (uint32_t i = 0; i < _children.size(); ++i) { dim += _children[i]->getScaledDimension(); }
		return dim;
	}
};

/// <summary>This class is wrapped into the pvr::ui::Group reference counted Framework Object. Use to apply a
/// transformation to several Sprites and render them together (for example, layout some sprites to form a UI and
/// then apply translation or rotation effects to all of them to change the page).</summary>
class MatrixGroup_ : public Group_
{
private:
	friend class pvr::ui::UIRenderer;

	glm::mat4 _viewProj;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class MatrixGroup_;
	};

	static MatrixGroup constructShared(UIRenderer& uiRenderer, uint64_t id) { return std::make_shared<MatrixGroup_>(make_shared_enabler{}, uiRenderer, id); }

	/// <summary>Internal function that is triggered when all item transformations have been applied and the final matrices can be calculated.</summary>
	/// <param name="parentId">The groups parent's id.</param>
	/// <param name="srt">A scale rotate translate matrix.</param>
	/// <param name="viewProjection">A view projection matrix.</param>
	/// <param name="viewport">The viewport currently in use.</param>
	void calculateMvp(uint64_t parentIds, glm::mat4 const& srt, const glm::mat4& viewProj, pvrvk::Rect2D const& viewport) const
	{
		glm::mat4 tmpMatrix = srt * _cachedMatrix;
		// My cached matrix should always be up-to-date unless overridden. No effect.
		for (ChildContainer::iterator it = _children.begin(); it != _children.end(); ++it) { (*it)->calculateMvp(packId(parentIds, _id), tmpMatrix, viewProj, viewport); }
	}

public:
	//!\cond NO_DOXYGEN
	/// <summary>Constructor. Do not call - use UIRenderer::createGroup.</summary>
	MatrixGroup_(make_shared_enabler, UIRenderer& uiRenderer, uint64_t id);
	//!\endcond

	/// <summary>Set the scale/rotation/translation matrix of this group. If other transformations are added to this
	/// matrix, unexpected results may occur when rendering the sprites.</summary>
	/// <param name="srt">The scale/rotation/translation matrix of this group</param>
	void setScaleRotateTranslate(const glm::mat4& srt) { _cachedMatrix = srt; }
	/// <summary>Set the projection matrix of this group</summary>
	/// <param name="viewProj">A projection matrix which will be used to render all members of this group</param>
	void setViewProjection(const glm::mat4& viewProj) { _viewProj = viewProj; }

	/// <summary>Call this method when you are finished updating the sprites (text, matrices, positioning etc.), and
	/// BEFORE the beginRendering command, to commit any changes you have done to the sprites. This function must not
	/// be called during rendering.</summary>
	void commitUpdates() const;
};

/// <summary>This class is wrapped into the pvr::ui::Group reference counted Framework Object. Use to apply a
/// transformation to several Sprites and render them together (for example, layout some sprites to form a UI and
/// then apply translation or rotation effects to all of them to change the page).</summary>
class PixelGroup_ : public Group_, public I2dComponent
{
private:
	friend class pvr::ui::UIRenderer;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class PixelGroup_;
	};

	static PixelGroup constructShared(UIRenderer& uiRenderer, uint64_t id) { return std::make_shared<PixelGroup_>(make_shared_enabler{}, uiRenderer, id); }

	/// <summary>Internal function that UIRenderer calls to render. Do not call directly.</summary>
	void calculateMvp(uint64_t parentIds, glm::mat4 const& srt, const glm::mat4& viewProj, pvrvk::Rect2D const& viewport) const;

public:
	//!\cond NO_DOXYGEN
	/// <summary>Constructor. Do not call - use UIRenderer::createGroup.</summary>
	PixelGroup_(make_shared_enabler, UIRenderer& uiRenderer, uint64_t id) : Group_(uiRenderer, id) {}
	//!\endcond

	/// <summary>Set the size (extent) of this pixel group</summary>
	/// <param name="size">The size of this pixel group, used to position the items it contains. It DOES NOT perform
	/// clipping - items can very well be placed outside the size of the group, and they will be rendered correctly as
	/// long as they are within the screen/viewport.</param>
	/// <returns>Pointer to this item</returns>
	PixelGroup_* setSize(glm::vec2 const& size)
	{
		_boundingRect.setMinMax(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(size.x, size.y, 0.0f));
		return this;
	}
};
} // namespace impl
} // namespace ui
} // namespace pvr
