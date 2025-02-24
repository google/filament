/*!
\brief Contains the class representing an entire Scene, or Model.
\file PVRAssets/Model.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRAssets/model/Camera.h"
#include "PVRAssets/model/Animation.h"
#include "PVRAssets/model/Light.h"
#include "PVRAssets/model/Mesh.h"

/// <summary>Main namespace of the PowerVR Framework.</summary>
namespace pvr {
class Stream;
/// <summary>Main namespace of the PVRAssets library.</summary>
namespace assets {

class Model;
class Mesh;
class Camera;
class Light;
/// <summary>A reference counted wrapper for a Model</summary>
typedef std::shared_ptr<Model> ModelHandle;
/// <summary>A reference counted wrapper for a Mesh.</summary>
typedef std::shared_ptr<Mesh> MeshHandle;
/// <summary>A reference counted wrapper for a Camera</summary>
typedef std::shared_ptr<Camera> CameraHandle;
/// <summary>A reference counted wrapper for a Light</summary>
typedef std::shared_ptr<Light> LightHandle;

/// <summary>The skeleton structure encapsulates all that which is required to define a skeleton including name, a set of bones and matrices for transformations.</summary>
struct Skeleton
{
	/// <summary>The name of the skeleton.</summary>
	std::string name;
	/// <summary>A list of bones in the skeleton.</summary>
	std::vector<uint32_t> bones;
	/// <summary>Matrices used for animating the skeleton.</summary>
	std::vector<glm::mat4> invBindMatrices;
};

/// <summary>Enumerates the model formats directly supported by the Framework.</summary>
enum class ModelFileFormat
{
	UNKNOWN = 0,
	POD,
	GLTF,
};

/// <summary>The Model class represents an entire Scene, or Model. It is mainly a Node structure, allowing various
/// different kinds of data to be stored in the Nodes. The class contains a tree-like structure of Nodes. Each Node
/// can be a Mesh node (containing a Mesh), Camera node or Light node. The tree-structure assumes transformational
/// hierarchy (as usual). Transformations are expressed through Animation objects (a static transform is an
/// animation with a single frame) There is an implicit order in the nodes - First in the array the Mesh nodes will
/// be laid out, then Camera and Light nodes.</summary>
class Model
{
public:
	/// <summary>Brings the Mesh class name into this class.</summary>
	typedef assets::Mesh Mesh;

	/// <summary>The Node represents a Mesh, Camera or Light. A Node has its own parenting, material, animation and
	/// custom user data. The tree-structure assumes transformational hierarchy (as usual), so parent transformations
	/// should be applied to children. Transformations are expressed through Animation objects (a static transform is
	/// an animation with a single frame). Note: Node ID and MeshID can sometimes be confusing: They will always be
	/// the same (when a MeshID makes sense) because Meshes are always laid out first in the internal list of Nodes.</summary>
	class Node
	{
	public:
		/// <summary>Raw internal structure of the Node.</summary>
		struct InternalData
		{
			StringHash name; //!< Name of object
			uint32_t objectIndex; //!< Index into mesh, light or camera array, depending on which object list contains this Node
			uint32_t materialIndex; //!< Index of material used on this mesh
			uint32_t parentIndex; //!< Index into Node array; recursively apply ancestor's transforms after this instance's.
			UInt8Buffer userData; //!< Optionally, user data

			/// <summary>Transformation flags.</summary>
			enum TransformFlags
			{
				Identity = 0, //!< Transformation Identity
				Scale = 0x01, //!< Transformation has Scale
				Rotate = 0x02, //!< Transformation has Rotation
				Translate = 0x04, //!< Transformation has Translation
				SRT = Scale | Rotate | Translate, //!< SRT
				Matrix = 64, //!< has matrix
			};

			// CONTAINS
			float frameTransform[16]; //!< contains interpolated SRT or Matrix for an frame else single Matrix for non animated node. Rotations are stored as quaternion in the format xyzw

			// NODE'S LOCAL SPACE SRT
			glm::vec3 scale; //!< Node's local space scale
			glm::quat rotation; //!< Node's local space rotation
			glm::vec3 translation; //!< Node's local space translation

			uint32_t transformFlags; //!< keep an flag whether the transformation data is stored as SRT or Matrix or Identity.
			int32_t skin; //!< Skin index for mesh nodes

			bool hasAnimation; //!< Has animation data

			/// <summary>Get current frame scale animation</summary>
			/// <returns>Returns scale</returns>
			glm::vec3& getFrameScaleAnimation() { return *(glm::vec3*)frameTransform; }

			/// <summary>Get current frame rotation animation</summary>
			/// <returns>Returns rotation</returns>
			glm::quat& getFrameRotationAnimation() { return *(glm::quat*)(&frameTransform[3]); }

			/// <summary>Get current frame translation animation</summary>
			/// <returns>Returns translation</returns>
			glm::vec3& getFrameTranslationAnimation() { return *(glm::vec3*)(&frameTransform[7]); }

			/// <summary>Get current frame scale animation (const)</summary>
			/// <returns>Returns scale</returns>
			const glm::vec3& getFrameScaleAnimation() const { return *(glm::vec3*)frameTransform; }

			/// <summary>Get current frame rotation animation (const)</summary>
			/// <returns>Returns rotation</returns>
			const glm::quat& getFrameRotationAnimation() const { return *(glm::quat*)(&frameTransform[3]); }

			/// <summary>Get current frame translation animation (const)</summary>
			/// <returns>Returns rotation</returns>
			const glm::vec3& getFrameTranslationAnimation() const { return *(glm::vec3*)(&frameTransform[7]); }

			/// <summary>Get local space scale</summary>
			/// <returns>Returns local space scale<returns>
			glm::vec3& getScale() { return scale; }

			/// <summary>Get local space rotation</summary>
			/// <returns>Returns local space rotation<returns>
			glm::quat& getRotate() { return rotation; }

			/// <summary>Get local space translation</summary>
			/// <returns>Returns local space translation<returns>
			glm::vec3& getTranslation() { return translation; }

			/// <summary>Get local space scale</summary>
			/// <returns>Returns local space scale<returns>
			const glm::vec3& getScale() const { return scale; }

			/// <summary>Get local space rotation</summary>
			/// <returns>Returns local space rotation<returns>
			const glm::quat& getRotate() const { return rotation; }

			/// <summary>Get local space translation</summary>
			/// <returns>Returns local space translation<returns>
			const glm::vec3& getTranslation() const { return translation; }

			/// <summary>Default constructor</summary>
			InternalData()
				: objectIndex(static_cast<uint32_t>(-1)), materialIndex(static_cast<uint32_t>(-1)), parentIndex(static_cast<uint32_t>(-1)), scale(1.0f), translation(0.0f)
			{
				transformFlags = TransformFlags::Identity;
				hasAnimation = false;
			}
		};

	public:
		/// <summary>Get which Mesh, Camera or Light this object refers to.</summary>
		/// <returns>The index of the Mesh, Camera or Light array of this node (depending on its type)</returns>
		uint32_t getObjectId() const { return _data.objectIndex; }

		/// <summary>Get this Node's name.</summary>
		/// <returns>The name of this object</returns>
		const StringHash& getName() const { return _data.name; }

		/// <summary>Get this Node's parent id.</summary>
		/// <returns>The ID of this Node's parent Node.</returns>
		uint32_t getParentID() const { return _data.parentIndex; }

		/// <summary>Get this Node's material id.</summary>
		/// <returns>The ID of this Node's Material</returns>
		uint32_t getMaterialIndex() const { return _data.materialIndex; }

		/// <summary>Associate a material with this node (Assign a material id to this node)</summary>
		/// <param name="materialId">The material ID to associate with</param>
		void setMaterialIndex(uint32_t materialId) { _data.materialIndex = materialId; }

		/// <summary>Get this Node's user data.</summary>
		/// <returns>The user data of this Node</returns>
		const uint8_t* getUserData() const { return _data.userData.data(); }

		/// <summary>Get the size of this Node's user data.</summary>
		/// <returns>Return The size in bytes of the user data of this Node</returns>
		uint32_t getUserDataSize() const { return static_cast<uint32_t>(_data.userData.size()); }

		/// <summary>Set mesh id. Must correlate with the actual position of this node in the data.</summary>
		/// <param name="index">The id to set the index of this node.</param>
		void setIndex(uint32_t index) { _data.objectIndex = index; }

		/// <summary>Set the name of this node.</summary>
		/// <param name="name">The std::string to set this node's name to.</param>
		void setName(const StringHash& name) { _data.name = name; }

		/// <summary>Set the parent of this node.</summary>
		/// <param name="parentID">the ID of this node's parent</param>
		void setParentID(uint32_t parentID) { _data.parentIndex = parentID; }

		/// <summary>Set the user data of this node. A bit copy of the data will be made.</summary>
		/// <param name="size">The size, in bytes, of the data</param>
		/// <param name="data">A pointer from which (size) data will be copied.</param>
		void setUserData(uint32_t size, const char* data)
		{
			_data.userData.resize(size);
			memcpy(_data.userData.data(), data, size);
		}

		/// <summary>Get a reference to the internal data of this object. Handle with care.</summary>
		/// <returns>Return a reference to the internal data of this object.</returns>
		InternalData& getInternalData() { return _data; }

		/// <summary>Get a const reference to the data object carrying all internal data of this model.</summary>
		/// <returns>Return a reference to the internal data of this object.</returns>
		const InternalData& getInternalData() const { return _data; }

	private:
		InternalData _data;
	};

	/// <summary>Internal class which stores Texture information of the model (name).</summary>
	class Texture
	{
	public:
		/// <summary>Default Constructor</summary>
		Texture() {}

		/// <summary>Constructor</summary>
		/// <param name="name">Texture name</param>
		Texture(const StringHash&& name) : _name(std::move(name)) {}

		/// <summary>Get the name of the texture.</summary>
		/// <returns>Return the texture name</returns>
		const pvr::StringHash& getName() const { return _name; }

		/// <summary>Get a reference to the name of the texture.</summary>
		/// <returns>Return reference to the texture name</returns>
		pvr::StringHash& getName() { return _name; }

		/// <summary>Set the name of the texture.</summary>
		/// <param name="name">The std::string to set this texture name to.</param>
		void setName(const StringHash& name) { this->_name = name; }

	private:
		pvr::StringHash _name;
	};

	/// <summary>Class which stores model material info.</summary>
	class Material
	{
	public:
		/// <summary>Constructor. Non initializing</summary>
		Material() : _defaultSemantics(*this) {}

		/// <summary>A blend function</summary>
		enum BlendFunction
		{
			BlendFuncZero = 0, //!< BlendFunction (Zero)
			BlendFuncOne, //!< BlendFunction (One)
			BlendFuncFactor, //!< BlendFunction (Factor)
			BlendFuncOneMinusBlendFactor, //!< BlendFunction (One Minus Blend Factor)

			BlendFuncSrcColor = 0x0300, //!< BlendFunction (source Color)
			BlendFuncOneMinusSrcColor, //!< BlendFunction (One Minus Source Color)
			BlendFuncSrcAlpha, //!< BlendFunction (Source Alpha)
			BlendFuncOneMinusSrcAlpha, //!< BlendFunction (One Minus Source Alpha)
			BlendFuncDstAlpha, //!< BlendFunction (Destination Alpha)
			BlendFuncOneMinusDstAlpha, //!< BlendFunction (One Minus Destination Alpha)
			BlendFuncDstColor, //!< BlendFunction (Destination Alpha)
			BlendFuncOneMinusDstColor, //!< BlendFunction (One Minus Destination Color)
			BlendFuncSrcAlphaSaturate, //!< BlendFunction (Source Alpha Saturate)

			BlendFuncConstantColor = 0x8001, //!< BlendFunction (Constant Color)
			BlendFuncOneMinusConstantColor, //!< BlendFunction (One Minus Constant Color)
			BlendFuncConstantAlpha, //!< BlendFunction (Constant Alpha)
			BlendFuncOneMinusConstantAlpha //!< BlendFunction (One Minus Constant Alpha)
		};

		/// <summary>A blend operation</summary>
		enum BlendOperation
		{
			BlendOpAdd = 0x8006, //!< Blend Operation (Add)
			BlendOpMin, //!< Blend Operation (Min)
			BlendOpMax, //!< Blend Operation (Max)
			BlendOpSubtract = 0x800A, //!< Blend Operation (Subtract)
			BlendOpReverseSubtract //!< Blend Operation (Reverse Subtract)
		};

		//"BLENDFUNCTION", BlendFunction
		//"BLENDOP", BlendOperation
		//"DIFFUSETEXTURE",
		//"SPECULARTEXTURE",
		//"NORMALTEXTURE",
		//"EMISSIVETEXTURE",
		//"GLOSSINESSTEXTURE",
		//"OPACITYTEXTURE",
		//"REFLECTIONTEXTURE",
		//"REFRACTIONTEXTURE",
		//"OPACITY",
		//"AMBIENTCOLOR",
		//"DIFFUSECOLOR",
		//"SPECULARCOLOR",
		//"SHININESS",
		//"EFFECTFILE",
		//"EFFECTNAME",
		//"BLENDFUNCSRCCOLOR"
		//"BLENDFUNCSRCALPHA"
		//"BLENDFUNCDSTCOLOR"
		//"BLENDFUNCDSTALPHA"
		//"BLENDCOLOR"
		//"BLENDFACTOR"
		//"FLAGS"
		//"USERDATA"

		//--------  PBR COMMON SEMANTICS ---------
		//"METALLICITY"
		//"ROUGHNESS"
		//"METALLICITYTEXTURE"
		//"ROUGHNESSTEXTURE"

		//-------- PBR POD SEMANTICS -------------
		//"IOR"
		//"FRESENEL"
		//"SSSCATERING"
		//"SSCATERINGDEPTH"
		//"SSCATERINGCOLOR"
		//"EMISSIONLUMINANCE"
		//"EMISSIONKELVIN"
		//"ANISTROPHY"

		//-------- PBR GLTF SEMANTICS ------------
		// "METALLICBASECOLOR"
		// "EMISSIVECOLOR"
		// "ALPHACUTOFF"
		// "DOUBLESIDED"
		// "NORMALTEXTURE"
		// "OCCLUSIONTEXTURE"

		//------- PBR SEMANTICS ----------
		//"REFLECTIVITY"
		//"EMISSION"

		/// <summary>Raw internal structure of the Material.</summary>
		struct InternalData
		{
			std::map<StringHash, FreeValue> materialSemantics; //!< storage for the per-material semantics
			std::map<StringHash, uint32_t> textureIndices; //!< Map of texture (semantic) names to indexes

			StringHash name; //!< Name of the material
			StringHash effectFile; //!< Effect filename if using an effect
			StringHash effectName; //!< Effect name (in the filename) if using an effect

			UInt8Buffer userData; //!< Raw user data
		};

		/// <summary>Base class for Physically-based-rendering(PBR) semantics</summary>
		class PBRSemantics
		{
		public:
			/// <summary>Destructor</summary>
			virtual ~PBRSemantics() {}

			/// <summary>Constructor</summary>
			/// <param name="mat">Material</param>
			PBRSemantics(Material& mat) : _material(mat) {}

			/// <summary>Get occlusion texture index</summary>
			/// <returns> Occlusion texture index</returns>
			uint32_t getOcclusionTextureIndex() const { return _material.getTextureIndex("OCCLUSIONTEXTURE"); }

			/// <summary>Set occlusion texture index</summary>
			/// <param name="index"> Occlusion texture index</param>
			void setOcclusionTextureIndex(uint32_t index) { _material.setTextureIndex("OCCLUSIONTEXTURE", index); }

			/// <summary>Get normal texture index</summary>
			/// <returns> normal texture index</returns>
			uint32_t getNormalTextureIndex() const { return _material.getTextureIndex("NORMALTEXTURE"); }

			/// <summary>Set normal texture index</summary>
			/// <param name="index"> normal texture index</param>
			void setNormalTextureIndex(uint32_t index) { _material.setTextureIndex("NORMALTEXTURE", index); }

			/// <summary>Get the RGB components of the emissive color of the material. These values are linear.
			/// If an emissiveTexture is specified, this value is multiplied with the texel values.</summary>
			/// <returns> emissive color</returns>
			glm::vec3 getEmissiveColor() const { return _material.getMaterialAttributeWithDefault("EMISSIVECOLOR", glm::vec3(0.f)); }

			/// <summary>Set the RGB components of the emissive color of the material. These values are linear.
			/// If an emissiveTexture is specified, this value is multiplied with the texel values.</summary>
			/// <param name="color">  emissive color</param>
			void setEmissiveColor(const glm::vec3& color)
			{
				FreeValue value;
				value.setValue(color);
				_material.setMaterialAttribute("EMISSIVECOLOR", value);
			}

			/// <summary>Set the emissive texture index</summary>
			/// <param name="index">  emissive texture index</param>
			void setEmissiveTextureIndex(uint32_t index) { _material.setTextureIndex("EMISSIVETEXTURE", index); }

			/// <summary>Get the emissive texture index</summary>
			/// <returns> Returns emissive texture index</returns>
			uint32_t getEmissiveTextureIndex() const { return _material.getTextureIndex("EMISSIVETEXTURE"); }

			/// <summary>Set the roughness texture index</summary>
			/// <param name="index">  roughness texture index</param>
			void setRoughnessTextureIndex(uint32_t index) { _material.setTextureIndex("ROUGHNESSTEXTURE", index); }

			/// <summary>Get the roughness texture index</summary>
			/// <returns> Returns roughness texture index</returns>
			uint32_t getRoughnessTextureIndex() { return _material.getTextureIndex("ROUGHNESSTEXTURE"); }

			/// <summary>Set the metallicity texture index</summary>
			/// <param name="index">  metallicity texture index</param>
			void setMetallicityTextureIndex(uint32_t index) { _material.setTextureIndex("METALLICITYTEXTURE", index); }

			/// <summary>Get the metallicity texture index</summary>
			/// <returns> Returns metallicity texture index</returns>
			uint32_t getMetallicityTextureIndex() { return _material.getTextureIndex("METALLICITYTEXTURE"); }

		protected:
			Material& _material; //!< Material used
		};

		/// <summary>POD Metallic roughness semantics</summary>
		class PODMetallicRoughnessSemantics : public PBRSemantics
		{
		public:
			/// <summary>Constructor</summary>
			/// <param name="mat">Material</param>
			PODMetallicRoughnessSemantics(Material& mat) : PBRSemantics(mat) {}

			/// <summary>set emission luminance</summary>
			/// <param name="luminance">luminance</param>
			void setEmissionLuminance(float luminance) { _material.setMaterialAttribute("EMISSIONLUMINANCE", FreeValue(luminance)); }

			/// <summary>The Physical Material supports an emissive component, additive light on top of other shading.
			/// Emission identity is defined by the weight and color multiplied by the luminence, and also tinted by the Kelvin color temperature (where 6500=white).</summary>
			/// <returns>Returns emission luminance</returns>
			float getEmissionLuminance() const { return _material.getMaterialAttributeWithDefault("EMISSIONLUMINANCE", 0.f); }

			/// <summary>Set emission kelvin</summary>
			/// <param name="kelvin">Kelvin</param>
			void setEmissionKelvin(float kelvin) { _material.setMaterialAttribute("EMISSIONKELVIN", kelvin); }

			/// <summary>Get emission kelvin</summary>
			/// <returns>Returns emission kelvin</returns>
			float getEmissionKelvin() const { return _material.getMaterialAttributeWithDefault("EMISSIONKELVIN", 1.f); }
		};

		/// <summary>Specifies the alpha mode used</summary>
		enum class GLTFAlphaMode
		{
			Opaque, //< The alpha value is ignored and the rendered output is fully opaque.
			Mask, //< The rendered output is either fully opaque or fully transparent depending on the alpha value and the specified alpha cutoff value.
			Blend, //<The alpha value is used to composite the source and destination areas. The rendered output is combined with the background using the normal painting operation
				   //(i.e. the Porter and Duff over operator).
		};

		/// <summary>This class provides accessor for gltf metallic roughness semantics</summary>
		class GLTFMetallicRoughnessSemantics : public PBRSemantics
		{
		public:
			/// <summary>Constructor</summary>
			/// <param name="material">Material</param>
			GLTFMetallicRoughnessSemantics(Material& material) : PBRSemantics(material) {}

			/// <summary>Set the base color of the material. The base color has two different interpretations depending on the value of metalness.
			/// When the material is a metal, the base color is the specific measured reflectance value at normal incidence (F0). For a non-metal the base
			/// color represents the reflected diffuse color of the material. In this model it is not possible to specify a F0 value for non-metals,
			/// and a linear value of 4% (0.04) is used.</summary>
			/// <param name="color">base color</param>
			void setBaseColor(const glm::vec4& color)
			{
				FreeValue value;
				value.setValue(color);
				_material.setMaterialAttribute("METALLICBASECOLOR", value);
			}

			/// <summary>Get the base color of the material.
			/// The base color has two different interpretations depending on the value of metalness.
			/// When the material is a metal, the base color is the specific measured reflectance value at normal incidence (F0).
			/// For a non-metal the base color represents the reflected diffuse color of the material.
			/// In this model it is not possible to specify a F0 value for non-metals, and a linear value of 4% (0.04) is used.</summary>
			/// <returns>base color</returns>
			glm::vec4 getBaseColor() const { return _material.getMaterialAttributeWithDefault<glm::vec4>("METALLICBASECOLOR", glm::vec4(1.f)); }

			/// <summary>Set the base color texture. This texture contains RGB(A) components in sRGB color space.
			/// The first three components (RGB) specify the base color of the material.
			/// If the fourth component (A) is present, it represents the alpha coverage of the material.
			/// Otherwise, an alpha of 1.0 is assumed. The alphaMode property specifies how alpha is interpreted.
			/// The stored texels must not be premultiplied.</summary>
			/// <param name="index">base color texture index</param>
			void setBaseColorTextureIndex(uint32_t index) { _material.setTextureIndex("DIFFUSETEXTURE", index); }

			/// <summary>Get the base color texture. This texture contains RGB(A) components in sRGB color space.
			/// The first three components (RGB) specify the base color of the material.
			/// If the fourth component (A) is present, it represents the alpha coverage of the material.
			/// Otherwise, an alpha of 1.0 is assumed. The alphaMode property specifies how alpha is interpreted.
			/// The stored texels must not be premultiplied.</summary>
			/// <returns>base color texture index</returns>
			uint32_t getBaseColorTextureIndex() const { return _material.getTextureIndex("DIFFUSETEXTURE"); }

			/// <summary>Set the metalness of the material. A value of 1.0 means the material is a metal.
			/// A value of 0.0 means the material is a dielectric. Values in between are for blending between metals and dielectrics
			/// such as dirty metallic surfaces. This value is linear. If a metallicRoughnessTexture is specified,
			/// this value is multiplied with the metallic texel values.</summary>
			/// <param name="metallic">metallic factor</param>
			void setMetallicity(float metallic)
			{
				FreeValue value;
				value.setValue(metallic);
				_material.setMaterialAttribute("METALLICITY", value);
			}

			/// <summary>Set the metalness of the material. A value of 1.0 means the material is a metal.
			/// A value of 0.0 means the material is a dielectric. Values in between are for blending between metals and dielectrics
			/// such as dirty metallic surfaces. This value is linear. If a metallicRoughnessTexture is specified,
			/// this value is multiplied with the metallic texel values.</summary>
			/// <returns>metallic factor</returns>
			float getMetallicity() const { return _material.getMaterialAttributeWithDefault<float>("METALLICITY", 0); }

			/// <summary>Set the roughness of the material. A value of 1.0 means the material is completely rough.
			/// A value of 0.0 means the material is completely smooth. This value is linear.
			/// If a metallicRoughnessTexture is specified, this value is multiplied with the roughness texel values.</summary>
			/// <param name="roughness">roughness factor</param>
			void setRoughness(float roughness)
			{
				FreeValue value;
				value.setValue(roughness);
				_material.setMaterialAttribute("ROUGHNESS", value);
			}

			/// <summary>Get the roughness of the material. A value of 1.0 means the material is completely rough.
			/// A value of 0.0 means the material is completely smooth. This value is linear.
			/// If a metallicRoughnessTexture is specified, this value is multiplied with the roughness texel values.</summary>
			/// <returns>roughness factor</returns>
			float getRoughness() const { return _material.getMaterialAttributeWithDefault<float>("ROUGHNESS", 0); }

			/// <summary>Get the alpha cutoff value of the material.
			/// Specifies the cutoff threshold when in MASK mode. If the alpha value is greater than or equal to this
			/// value then it is rendered as fully opaque, otherwise, it is rendered as fully transparent.
			/// A value greater than 1.0 will render the entire material as fully transparent. This value is ignored for other modes.</summary>
			/// <returns>The alpha cutoff value of the material.</returns>
			float getAlphaCutOff() const { return _material.getMaterialAttributeWithDefault("ALPHACUTOFF", 0.5f); }

			/// <summary>Get the alpha cutoff value of the material.
			/// Specifies the cutoff threshold when in MASK mode. If the alpha value is greater than or equal to this
			/// value then it is rendered as fully opaque, otherwise, it is rendered as fully transparent.
			/// A value greater than 1.0 will render the entire material as fully transparent. This value is ignored for other modes.</summary>
			/// <param name="cutoff">cutoff factor</param>
			void setAlphaCutOff(float cutoff)
			{
				FreeValue val;
				val.setValue(cutoff);
				_material.setMaterialAttribute("ALPHACUTOFF", val);
			}

			/// <summary>Return whether the material is double sided. When this value is false, back-face culling is enabled.
			/// When this value is true, back-face culling is disabled and double sided lighting is enabled.
			/// The back-face must have its normals reversed before the lighting equation is evaluated.</summary>
			/// <returns>Return true whether is double sided</returns>
			bool isDoubleSided() const { return static_cast<bool>(_material.getMaterialAttributeWithDefault("DOUBLESIDED", 1)); }

			/// <summary>Set the material is double sided or not. When this value is false, back-face culling is enabled.
			/// When this value is true, back-face culling is disabled and double sided lighting is enabled.
			/// The back-face must have its normals reversed before the lighting equation is evaluated.</summary>
			/// <param name="doubleSided">set the material is double sided or not</param>
			void setDoubleSided(bool doubleSided)
			{
				FreeValue val;
				val.setValue(static_cast<uint32_t>(doubleSided));
				_material.setMaterialAttribute("DOUBLESIDED", val);
			}

			/// <summary>Get the material's alpha rendering mode enumeration specifying the interpretation of the alpha
			/// value of the main factor and texture.</summary>
			/// <returns>Returns alpha mode</returns>
			GLTFAlphaMode getAlphaMode()
			{
				return static_cast<GLTFAlphaMode>(_material.getMaterialAttributeWithDefault("ALPHAMODE", static_cast<uint32_t>(GLTFAlphaMode::Opaque)));
			}

			/// <summary>Set the material's alpha rendering mode enumeration specifying the interpretation of the alpha
			/// value of the main factor and texture.</summary>
			/// <param name="alphaMode">set the alpha mode</param>
			void setAlphaMode(GLTFAlphaMode alphaMode) const
			{
				FreeValue val;
				val.setValue(static_cast<uint32_t>(alphaMode));
				_material.setMaterialAttribute("ALPHAMODE", val);
			}
		};

		/// <summary>This class is provided for convenient compile-time access of the semantic values.</summary>
		class DefaultMaterialSemantics
		{
		public:
			/// <summary>Constructor from a parent material (in order to initialize the material reference).</summary>
			/// <param name="material"> The material that will be the parent of this object.</param>
			DefaultMaterialSemantics(const Material& material) : material(&material) {}

			/// <summary>Get material ambient (semantic "AMBIENT")</summary>
			/// <returns>Material ambient</returns>
			glm::vec3 getAmbient() const { return material->getMaterialAttributeWithDefault<glm::vec3>("AMBIENT", glm::vec3(0.f, 0.f, 0.f)); }

			/// <summary>Get material diffuse (semantic "DIFFUSE")</summary>
			/// <returns>Material diffuse</returns>
			glm::vec3 getDiffuse() const { return material->getMaterialAttributeWithDefault<glm::vec3>("DIFFUSE", glm::vec3(1.f, 1.f, 1.f)); }

			/// <summary>Get material specular (semantic "SPECULAR")</summary>
			/// <returns>Material specular</returns>
			glm::vec3 getSpecular() const { return material->getMaterialAttributeWithDefault<glm::vec3>("SPECULAR", glm::vec3(0.f, 0.f, 0.f)); }

			/// <summary>Get material shininess (semantic "SHININESS")</summary>
			/// <returns>Material shininess</returns>
			float getShininess() const { return material->getMaterialAttributeWithDefault<float>("SHININESS", 0.f); }

			/// <summary>Get the diffuse color texture's index (semantic "DIFFUSETEXTURE", return-1 if not exist)</summary>
			/// <returns>Return the diffuse texture index, if exists, otherwise return -1</returns>
			uint32_t getDiffuseTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("DIFFUSETEXTURE");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Return the ambient color texture's index (semantic "AMBIENTTEXTURE", return-1 if not exist)</summary>
			/// <returns>Return the Ambient texture index, if exists</returns>
			uint32_t getAmbientTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("AMBIENTTEXTURE");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Get the specular color texture's index (semantic "SPECULARCOLORTEXTURE", return-1 if not exist)</summary>
			/// <returns>Return the specular color texture index</returns>
			uint32_t getSpecularColorTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("SPECULARCOLORTEXTURE");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Get the specular level texture's index (semantic "SPECULARLEVELTEXTURE", return-1 if not exist)</summary>
			/// <returns>Return the specular level texture index</returns>
			uint32_t getSpecularLevelTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("SPECULARLEVELTEXTURE");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Get bumpmap texture index (semantic "NORMALTEXTURE", return-1 if not exist)</summary>
			/// <returns>Return the bumpmap texture index</returns>
			uint32_t getBumpMapTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("NORMALTEXTURE");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Get emissive texture's index  (semantic "EMISSIVETEXTURE", return-1 if not exist)</summary>
			/// <returns>Return the emissive texture index</returns>
			uint32_t getEmissiveTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("EMISSIVETEXTURE");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Get glossiness texture's index  (semantic "GLOSSINESSTEXTURE", return-1 if not exist)</summary>
			/// <returns>Return the glossiness texture index</returns>
			uint32_t getGlossinessTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("GLOSSINESSTEXTURE");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Get opacity texture's index  (semantic "OPACITYTEXTURE", return-1 if not exist)</summary>
			/// <returns>Return the opacity texture index</returns>
			uint32_t getOpacityTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("OPACITYTEXTURE");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Get reflection texture's index (semantic "REFLECTIONTEXTURE", return-1 if not exist)</summary>
			/// <returns>Return the reflection texture index</returns>
			uint32_t getReflectionTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("REFLECTIONTEXTURE");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Return refraction texture's index (semantic "REFRACTIONTEXTURE", return-1 if not exist)</summary>
			/// <returns>Return the refraction texture index</returns>
			uint32_t getRefractionTextureIndex() const
			{
				static const StringHash diffuseTexSemantic("REFRACTIONTEXTURE");
				return material->getTextureIndex(diffuseTexSemantic);
			}

			/// <summary>Get this material opacity (semantic "OPACITY")</summary>
			/// <returns>Return the material opacity</returns>
			float getOpacity() const { return material->getMaterialAttributeWithDefault<float>("OPACITY", 1.f); }

			/// <summary>Get the blend function for Source Color (semantic "BLENDSRCCOLOR")</summary>
			/// <returns>Return source color blend function</returns>
			BlendFunction getBlendSrcRGB() const { return material->getMaterialAttributeWithDefault<BlendFunction>("BLENDSRCCOLOR", BlendFunction::BlendFuncOne); }

			/// <summary>Get the blend function for Source Alpha (semantic "BLENDSRCALPHA")</summary>
			/// <returns>Return source alpha blend function</returns>
			BlendFunction getBlendSrcA() const { return material->getMaterialAttributeWithDefault<BlendFunction>("BLENDSRCALPHA", BlendFunction::BlendFuncOne); }

			/// <summary>Get the blend function for Destination Color (semantic "BLENDDSTCOLOR")</summary>
			/// <returns>Return destination color blend function</returns>
			BlendFunction getBlendDstRGB() const { return material->getMaterialAttributeWithDefault<BlendFunction>("BLENDDSTCOLOR", BlendFunction::BlendFuncZero); }

			/// <summary>Get the blend function for Destination Alpha (semantic "BLENDDSTALPHA")</summary>
			/// <returns>Return destination alpha blend function</returns>
			BlendFunction getBlendDstA() const { return material->getMaterialAttributeWithDefault<BlendFunction>("BLENDDSTALPHA", BlendFunction::BlendFuncZero); }

			/// <summary>Get the blend operation for Color (semantic "BLENDOPCOLOR")</summary>
			/// <returns>Return the color's blend operator</returns>
			BlendOperation getBlendOpRGB() const { return material->getMaterialAttributeWithDefault<BlendOperation>("BLENDOPCOLOR", BlendOperation::BlendOpAdd); }

			/// <summary>Return the blend operation for Alpha (semantic "BLENDOPALPHA")</summary>
			/// <returns>Return the alpha's blend operator</returns>
			BlendOperation getBlendOpA() const { return material->getMaterialAttributeWithDefault<BlendOperation>("BLENDOPALPHA", BlendOperation::BlendOpAdd); }

			/// <summary>Get the blend color (semantic "BLENDCOLOR")</summary>
			/// <returns>Return blend color</returns>
			glm::vec4 getBlendColor() const { return material->getMaterialAttributeWithDefault<glm::vec4>("BLENDCOLOR", glm::vec4(0.f, 0.f, 0.f, 0.f)); }

			/// <summary>Return the blend factor (semantic "BLENDFACTOR")</summary>
			/// <returns>Return blend factor</returns>
			glm::vec4 getBlendFactor() const { return material->getMaterialAttributeWithDefault<glm::vec4>("BLENDFACTOR", glm::vec4(0.f, 0.f, 0.f, 0.f)); }

		private:
			const Material* material;
		};

	public:
		/// <summary>Get a Default Semantics adapter for this object. This is just a convenience object to access
		/// the default semantics through compile time functions.</summary>
		/// <returns>The default semantics for this object.</returns>
		const DefaultMaterialSemantics& defaultSemantics() const { return _defaultSemantics; }

		/// <summary>Set a material attribute by Semantic Name. Any semantic name may be passed, but some of them
		/// may be additionally accessed through Default Semantics.</summary>
		/// <param name="semantic">The semantic to set</param>
		/// <param name="value">The value to set</param>
		void setMaterialAttribute(const StringHash& semantic, const FreeValue& value) { _data.materialSemantics[semantic] = value; }

		/// <summary>Retrieve a material attribute by Semantic Name. If it does not exist, NULL will be returned.</summary>
		/// <param name="semantic">The semantic to retrieve</param>
		/// <returns> A pointer to the value of the semantic with name <paramRef name="semantic"/>. If the semantic
		/// does not exist, returns null</returns>
		const FreeValue* getMaterialAttribute(const StringHash& semantic) const
		{
			auto it = _data.materialSemantics.find(semantic);
			if (it != _data.materialSemantics.end()) { return &it->second; }
			return nullptr;
		}

		/// <summary>Retrieve a material attribute value, by Semantic Name, as a specific type.
		/// If it does not exist, the default value will be returned.</summary>
		/// <typeparam name="Type">The type that the value will be interpreded as</param>
		/// <param name="semantic">The semantic name of the value to retrieve</param>
		/// <param name="defaultAttrib">The default value. This will be returned if semantic does not exist</param>
		/// <returns> The value of the semantic with name <paramRef name="semantic"/>. If the semantic
		/// does not exist, returns the default value</returns>
		template<typename Type>
		const Type getMaterialAttributeWithDefault(const StringHash& semantic, const Type& defaultAttrib) const
		{
			auto* val = getMaterialAttribute(semantic);
			if (val) { return val->interpretValueAs<Type>(); }
			return defaultAttrib;
		}

		/// <summary>Retrieve a material attribute value, by Semantic Name, as a specific type.
		/// If it does not exist, null will be returned.</summary>
		/// <typeparam name="Type">The type that the value will be interpreded as</param>
		/// <param name="semantic">The semantic name of the value to retrieve</param>
		/// <returns> A pointer to the value of the semantic with name <paramRef name="semantic"/>. If the semantic
		/// does not exist, returns Null</returns>
		template<typename Type>
		const Type* getMaterialAttributeAs(const StringHash& semantic) const
		{
			auto* val = getMaterialAttribute(semantic);
			if (val) { return &val->interpretValueAs<Type>(); }
			return NULL;
		}

		/// <summary>Query if a semantic exists (Either texture or attribute)</summary>
		/// <param name="semantic">The semantic name to check.</param>
		/// <returns>True if either a texture or a material attribute with the specified
		/// semantic exists, otherwise false</returns>
		bool hasSemantic(const StringHash& semantic) const { return hasMaterialTexture(semantic) || hasMaterialAttribute(semantic); }

		/// <summary>Check if a material texture with the specified semantic exists.</summary>
		/// <param name="semantic">The semantic of the material texture to check.</param>
		/// <returns>True if the material texture exists, otherwise false</returns>
		bool hasMaterialTexture(const StringHash& semantic) const { return getTextureIndex(semantic) != static_cast<uint32_t>(-1); }
		/// <summary>Check if a material attribute with the specified semantic exists.</summary>
		/// <param name="semantic">The semantic of the material attribute to check.</param>
		/// <returns>True if the material attribute exists, otherwise false</returns>
		bool hasMaterialAttribute(const StringHash& semantic) const { return getMaterialAttribute(semantic) != NULL; }

		/// <summary>Set material effect name.</summary>
		/// <param name="name">Material effect name</param>
		void setEffectName(const StringHash& name) { _data.effectName = name; }

		/// <summary>Set material effect file name.</summary>
		/// <param name="name">Effect file name</param>
		void setEffectFile(const StringHash& name) { _data.effectFile = name; }

		/// <summary>Find a texture with the specified semantic. If it exists, returns its index otherwise -1.</summary>
		/// <param name="semantic">The semantic of the texture to retrieve.</param>
		/// <returns>If the index with this semantic exists, return its index. Otherwise, return -1.</returns>
		uint32_t getTextureIndex(const StringHash& semantic) const
		{
			auto it = _data.textureIndices.find(semantic);
			return (it == _data.textureIndices.end()) ? -1 : it->second;
		}

		/// <summary>Set texture with the specified semantic and index.</summary>
		/// <param name="semantic">The semantic of the texture.</param>
		/// <param name="index">texture index</param>
		void setTextureIndex(const StringHash& semantic, uint32_t index) { _data.textureIndices[semantic] = index; }

		/// <summary>Get this material name.</summary>
		/// <returns>return the material name</returns>
		const StringHash& getName() const { return this->_data.name; }

		/// <summary>Get this material effect file name.</summary>
		/// <returns>Retuurn Material effect file name</returns>
		const StringHash& getEffectFile() const { return _data.effectFile; }

		/// <summary>Get this material effect name.</summary>
		/// <returns>Return material effect name</returns>
		const StringHash& getEffectName() const { return _data.effectName; }

		/// <summary>Return a reference to the material's internal data structure. Handle with care.</summary>
		/// <returns>Return reference to the internal data</returns>
		InternalData& getInternalData() { return _data; }

	private:
		UInt8Buffer userData;
		InternalData _data;
		DefaultMaterialSemantics _defaultSemantics;
	};
	/// <summary>Struct containing the internal data of the Model.</summary>
	struct InternalData
	{
		std::map<StringHash, FreeValue> semantics; //!< Store of the semantics

		float clearColor[3]; //!< Background color
		float ambientColor[3]; //!< Ambient color

		std::vector<Mesh> meshes; //!< Mesh array. Any given mesh can be referenced by multiple Nodes.
		std::vector<Camera> cameras; //!< Camera array. Any given mesh can be referenced by multiple Nodes.
		std::vector<Light> lights; //!< Light array. Any given mesh can be referenced by multiple Nodes.
		std::vector<Texture> textures; //!< Textures in this Model
		std::vector<Material> materials; //!< Materials in this Model
		std::vector<Node> nodes; //!< Nodes array. The nodes are sorted thus: First Mesh Nodes, then Light Nodes, then Camera nodes.
		std::vector<Skeleton> skeletons; //!< Skeleton array

		std::vector<AnimationData> animationsData; //!< Animation data
		std::vector<AnimationInstance> animationInstances; //!< Animation instance data
		uint32_t numMeshNodes; //!< Number of items in the nodes array which are Meshes
		uint32_t numLightNodes; //!< Number of items in the nodes array which are Meshes
		uint32_t numCameraNodes; //!< Number of items in the nodes array which are Meshes

		uint32_t numFrames; //!< Number of frames of animation
		float currentFrame; //!< Current frame in the animation
		float FPS; //!< The frames per second the animation should be played at

		UInt8Buffer userData; //!< Custom raw data stored by the user

		float units; //!< Unit scaling
		uint32_t flags; //!< Flags
		std::shared_ptr<void> userDataPtr; //!< Can be used to store any kind of data that the user wraps in a std::shared_ptr resource

		/// <summary>Constructor. Initializing to empty.</summary>
		InternalData() : numMeshNodes(0), numLightNodes(0), numCameraNodes(0), numFrames(0), currentFrame(0), FPS(30), units(1), flags(0)
		{
			memset(clearColor, 0, sizeof(clearColor));
			memset(ambientColor, 0, sizeof(ambientColor));
		}
	};

private:
	std::vector<float> cachedFrame; //!< Cache indicating the frames at which the matrix cache was filled
	std::vector<glm::mat4x4> worldMatrixFrameN; //!< Cache of world matrices for the frame described in fCachedFrame
	std::vector<glm::mat4x4> worldMatrixFrameZero; //!< Cache of frame 0 matrices
	InternalData _data; //!< A set of internal data relating to the model
public:
	/// <summary>Return the value of a Model-wide semantic as a FreeValue, null if it does not exist.</summary>
	/// <param name="semantic">The semantic name to retrieve</param>
	/// <returns>A pointer to a FreeValue containing the value of the semantic. If the semantic does not exist,
	/// return NULL</returns>
	const FreeValue* getModelSemantic(const StringHash& semantic) const
	{
		auto it = _data.semantics.find(semantic);
		if (it == _data.semantics.end()) { return NULL; }
		return &it->second;
	}

	/// <summary>Get a pointer to the UserData of this model, if such data exist.</summary>
	/// <returns>A pointer to the UserData, as a Reference Counted Void pointer. Cast to appropriate (ref counted)type</returns>
	const std::shared_ptr<void>& getUserDataPtr() const { return this->_data.userDataPtr; }

	/// <summary>Get a pointer to the UserData of this model.</summary>
	/// <returns>A pointer to the UserData, as a Reference Counted Void pointer. Cast to appropriate (ref counted)type</returns>
	std::shared_ptr<void> getUserDataPtr() { return this->_data.userDataPtr; }

	/// <summary>Set the UserData of this model (wrap the data into a std::shared_ptr and cast to Ref Counted void pointer.</summary>
	/// <param name="ptr">The UserData. Must be wrapped in an appropriate std::shared_ptr, and then cast into a std::shared_ptr to void</param>
	void setUserDataPtr(const std::shared_ptr<void>& ptr) { _data.userDataPtr = ptr; }

public:
	/// <summary>Free the vertex data (Vertex attribute values, Vertex Index values) of all meshes to free memory.
	/// Usually called after VBOs/IBOs have been created. Any other data of the Mesh are unaffected.</summary>
	void releaseVertexData()
	{
		for (uint32_t i = 0; i < getNumMeshes(); ++i) { releaseVertexData(i); }
	}

	/// <summary>Free the vertex data (Vertex attribute values, Vertex Index values) of a single mesh to free memory.
	/// Usually called after VBOs/IBOs have been created. Any other data of the Mesh are unaffected.</summary>
	/// <param name="meshId">The meshId of the mesh whose vertex data to free</param>
	void releaseVertexData(uint32_t meshId)
	{
		Mesh& mesh = getMesh(meshId);
		for (uint32_t i = 0; i < mesh.getNumDataElements(); ++i) { mesh.removeData(i); }
		mesh.getFaces().setData(0, 0);
	}

	/// <summary>Return the world-space position of a light. Corresponds to the Model's current frame of animation.</summary>
	/// <param name="lightId">The node for which to return the world matrix.</param>
	/// <returns>Return The world matrix of (nodeId).</returns>
	glm::vec3 getLightPosition(uint32_t lightId) const;

	/// <summary>Get number of animation data</summary>
	/// <returns> Return number of animation data</returns>
	size_t getNumAnimationData() const { return _data.animationsData.size(); }

	/// <summary> Get animation data</summary>
	/// <param name="index"> animation data index</param>
	/// <returns>Return animation data</returns>
	const AnimationData& getAnimationData(uint32_t index) const { return _data.animationsData[index]; }

	/// <summary>Get Animation Data</summary>
	/// <param name="name">Animation data name</param>
	/// <returns>Returns animation data</returns>
	const AnimationData* getAnimationData(const char* name) const
	{
		const auto& it =
			std::find_if(_data.animationsData.begin(), _data.animationsData.end(), [&](const AnimationData& anim) { return strcmp(name, anim.getAnimationName().c_str()) == 0; });
		if (it != _data.animationsData.end()) { return &(*it); }
		return nullptr;
	}

	/// <summary>Get animation instance</summary>
	/// <param name="index">Animation instance index</param>
	/// <returns>Returns animation instance</returns>
	const AnimationInstance& getAnimationInstance(uint32_t index) const { return _data.animationInstances[index]; }

	/// <summary>Get animation instance</summary>
	/// <param name="index">Animation instance index</param>
	/// <returns>Returns animation instance</returns>
	AnimationInstance& getAnimationInstance(uint32_t index) { return _data.animationInstances[index]; }

	/// <summary>Get number of animation instances</summary>
	/// <returns>Return number of animation instances</returns>
	size_t getNumAnimationInstances() const { return _data.animationInstances.size(); }

	/// <summary>Add new animation instance</summary>
	/// <param name="animationInstance">Animation instance to add</param>
	/// <returns>Return animation instance id</returns>
	size_t addAnimationInstance(const AnimationInstance& animationInstance)
	{
		_data.animationInstances.emplace_back(animationInstance);
		return _data.animationInstances.size() - 1;
	}

	/// <summary>Return the model-to-world matrix of a node. Corresponds to the Model's current frame of animation. This
	/// version will store a copy of the matrix in an internal cache so that repeated calls for it will use the cached
	/// copy of it. Will also store the cached versions of all parents of this node, or use cached versions of them if
	/// they exist. Use this if you have long hierarchies and/or repeated calls per frame.</summary>
	/// <param name="nodeId">The node for which to return the world matrix.</param>
	/// <returns>Return The world matrix of (nodeId).</returns>
	glm::mat4x4 getWorldMatrix(uint32_t nodeId) const;

	/// <summary>Return the model-to-world matrix of a node. Corresponds to the Model's current frame of animation. This
	/// version will not use caching and will recalculate the matrix. Faster if the matrix is only used a few times.</summary>
	/// <param name="nodeId">The node for which to return the world matrix</param>
	/// <returns>return The world matrix of (nodeId)</returns>
	glm::mat4x4 getWorldMatrixNoCache(uint32_t nodeId) const;

	/// <summary>Return the model-to-world matrix of a specified bone. Corresponds to the Model's current frame of
	/// animation. This version will use caching.</summary>
	/// <param name="skinNodeID">The node for which to return the world matrix</param>
	/// <param name="boneId">The bone for which to return the world matrix</param>
	/// <returns>Return The world matrix of (nodeId, boneID)</returns>
	glm::mat4x4 getBoneWorldMatrix(uint32_t skinNodeID, uint32_t boneId) const;

	/// <summary>Transform a custom matrix with a node's parent's transformation. Allows a custom matrix to be applied to a
	/// node, while honoring the hierarchical transformations applied by its parent hierarchy.</summary>
	/// <param name="nodeId">The node whose parents will be applied to the transformation.</param>
	/// <param name="localMatrix">The matrix to transform</param>
	/// <returns>Return The localMatrix transformation, modified by the hierarchical transformations of the node.</returns>
	/// <remarks>This function can be used to implement custom procedural animation/kinematics schemes, in which case
	/// some nodes may need to have their animations customly defined, but must still honor their parents'
	/// transformations.</remarks>
	glm::mat4x4 toWorldMatrix(uint32_t nodeId, const glm::mat4& localMatrix) const
	{
		uint32_t parentID = _data.nodes[nodeId].getParentID();
		if (parentID == static_cast<uint32_t>(-1)) { return localMatrix; }
		else
		{
			glm::mat4 parentWorld = getWorldMatrix(parentID);
			return parentWorld * localMatrix;
		}
	}

	/// <summary>Get skeleton</summary>
	/// <param name="skeletonIndex">Skeleton index</param>
	/// <returns>Return skeleton</returns>
	const Skeleton& getSkeleton(uint32_t skeletonIndex) const { return _data.skeletons[skeletonIndex]; }

	/// <summary>Get number of skeletons</summary>
	/// <returns>Return number of skeleton</returns>
	size_t getNumSkeletons() const { return _data.skeletons.size(); }

	/// <summary>Get the clear color (background) (float array R,G,B,A).</summary>
	/// <returns>The clear color (float array R,G,B,A).</returns>
	const float* getBackgroundColor() const { return _data.clearColor; }

	/// <summary>Get the number of distinct camera objects. May be different than the actual number of Camera
	/// Instances (Nodes).</summary>
	/// <returns>Return The number of distinct camera objects.</returns>
	uint32_t getNumCameras() const { return static_cast<uint32_t>(_data.cameras.size()); }

	/// <summary>Get the number of Camera nodes in this model</summary>
	/// <returns>Return The number of Camera instances (Nodes) in this Model.</returns>
	uint32_t getNumCameraNodes() const { return getNumCameras(); /* Will be changed at a future revision */ }

	/// <summary>Get a Camera from this model</summary>
	/// <param name="cameraIndex">The index of the camera. Valid values (0 to getNumCameras()-1)</param>
	/// <returns>Return the camera</returns>
	const Camera& getCamera(uint32_t cameraIndex) const
	{
		assertion(cameraIndex < getNumCameras(), "Invalid camera index");
		return _data.cameras[cameraIndex];
	}
	/// <summary>Get a Camera from this model</summary>
	/// <param name="cameraIndex">The index of the camera. Valid values (0 to getNumCameras()-1)</param>
	/// <returns>Return the camera</returns>
	Camera& getCamera(uint32_t cameraIndex)
	{
		assertion(cameraIndex < getNumCameras(), "Invalid camera index");
		return _data.cameras[cameraIndex];
	}

	/// <summary>Get a specific CameraNode.</summary>
	/// <param name="cameraNodeIndex">The Index of a Camera Node. It is not the same as the NodeID. Valid values: (0
	/// .. getNumCameraNodes()-1)</param>
	/// <returns>Return The Camera Node</returns>
	const Node& getCameraNode(uint32_t cameraNodeIndex) const
	{
		assertion(cameraNodeIndex < getNumCameraNodes(), "Invalid camera node index");
		// Camera nodes are after the mesh and light nodes in the array
		return getNode(getNodeIdFromCameraId(cameraNodeIndex));
	}

	/// <summary>Get a specific CameraNode.</summary>
	/// <param name="cameraNodeIndex">The Index of a Camera Node. It is not the same as the NodeID. Valid values: (0
	/// .. getNumCameraNodes()-1)</param>
	/// <returns>Return The Camera Node</returns>
	Node& getCameraNode(uint32_t cameraNodeIndex)
	{
		assertion(cameraNodeIndex < getNumCameraNodes(), "Invalid camera node index");
		// Camera nodes are after the mesh and light nodes in the array
		return getNode(getNodeIdFromCameraId(cameraNodeIndex));
	}

	/// <summary>Get number of animation instances</summary>
	/// <returns> Returns number of animation instances<returns>
	size_t getNumAnimations() const { return _data.animationsData.size(); }

	/// <summary>Get the (global) Node Index of a specific CameraNode.</summary>
	/// <param name="cameraNodeIndex">The Index of a Camera Node that will be used to calculate the NodeID. Valid
	/// values: (0 to getNumCameraNodes()-1)</param>
	/// <returns>Return The Node index of the specified camera node. Normally, it is the same as getNumMeshes +
	/// getNumLights + cameraNodeIndex</returns>
	uint32_t getNodeIdFromCameraId(uint32_t cameraNodeIndex) const
	{
		// Camera nodes are after the mesh and light nodes in the array
		assertion(cameraNodeIndex < getNumCameraNodes(), "Invalid camera node index");
		return getNumMeshes() + getNumLights() + cameraNodeIndex;
	}

	/// <summary>Get the number of distinct Light objects. May be different than the actual number of Light Instances
	/// (Nodes).</summary>
	/// <returns>Return The number of distinct Light objects in this Model.</returns>
	uint32_t getNumLights() const { return static_cast<uint32_t>(_data.lights.size()); }

	/// <summary>Get the number of Light nodes.</summary>
	/// <returns>Return The number of Light instances (Nodes) in this Model.</returns>
	uint32_t getNumLightNodes() const { return getNumLights(); /* Will be changed at a future revision */ }

	/// <summary>Get the light object with the specific Light Index.</summary>
	/// <param name="lightIndex">The index of the light. Valid values (0..getNumLights()-1)</param>
	/// <returns>Return the light</returns>
	const Light& getLight(uint32_t lightIndex) const
	{
		assertion(lightIndex < getNumLights(), "Invalid light index");
		return _data.lights[lightIndex];
	}
	/// <summary>Get the light object with the specific Light Index.</summary>
	/// <param name="lightIndex">The index of the light. Valid values (0..getNumLights()-1)</param>
	/// <returns>Return the light</returns>
	Light& getLight(uint32_t lightIndex)
	{
		assertion(lightIndex < getNumLights(), "Invalid light index");
		return _data.lights[lightIndex];
	}

	/// <summary>Get a specific Light Node.</summary>
	/// <param name="lightNodeIndex">The Index of the Light Node. It is not the same as the NodeID. Valid values: (0
	/// to getNumLightNodes()-1)</param>
	/// <returns>Return the light node</returns>
	const Node& getLightNode(uint32_t lightNodeIndex) const
	{
		assertion(lightNodeIndex < getNumLights(), "Invalid light node index");
		// Light nodes are after the mesh nodes in the array
		return getNode(getNodeIdFromLightNodeId(lightNodeIndex));
	}

	/// <summary>Get the GLOBAL index of a specific Light Node.</summary>
	/// <param name="lightNodeIndex">The Index of the Light Node. It is not the same as the NodeID. Valid values: (0
	/// to getNumLightNodes()-1)</param>
	/// <returns>Return the Node index of the same index. It is the same as getNumMeshNodes() + lightNodeIndex.</returns>
	uint32_t getNodeIdFromLightNodeId(uint32_t lightNodeIndex) const
	{
		assertion(lightNodeIndex < getNumLightNodes(), "Invalid light node index");
		// Light nodes are after the mesh nodes in the array
		return getNumMeshNodes() + lightNodeIndex;
	}

	/// <summary>Get the number of distinct Mesh objects. Unless each Mesh appears at exactly one Node, may be
	/// different than the actual number of Mesh instances.</summary>
	/// <returns>Return The number of different Mesh objects in this Model.</returns>
	uint32_t getNumMeshes() const { return static_cast<uint32_t>(_data.meshes.size()); }

	/// <summary>Get the number of Mesh nodes.</summary>
	/// <returns>Return The number of Mesh instances (Nodes) in this Model.</returns>
	uint32_t getNumMeshNodes() const { return _data.numMeshNodes; }

	/// <summary>Get the Mesh object with the specific Mesh Index. Constant overload.</summary>
	/// <param name="meshIndex">The index of the Mesh. Valid values (0..getNumMeshes()-1)</param>
	/// <returns>The mesh with id <paramref name="meshIndex."/>Const ref.</returns>
	const Mesh& getMesh(uint32_t meshIndex) const { return _data.meshes[meshIndex]; }

	/// <summary>Allocate memory for animation data</summary>
	/// <param name="numAnimation">Number of animation data to allocate</param>
	void allocateAnimationsData(uint32_t numAnimation) { _data.animationsData.resize(numAnimation); }

	/// <summary>Allocate memory for animation instances</summary>
	/// <param name="numAnimation">Number of animation instances to allocate</param>
	void allocateAnimationInstances(uint32_t numAnimation) { _data.animationInstances.resize(numAnimation); }

	/// <summary>Get the Mesh object with the specific Mesh Index.</summary>
	/// <param name="index">The index of the Mesh. Valid values (0..getNumMeshes()-1)</param>
	/// <returns>Return the mesh from this model</returns>
	Mesh& getMesh(uint32_t index)
	{
		assertion(index < getNumMeshes(), "Invalid mesh index");
		return _data.meshes[index];
	}

	/// <summary>Get a specific Mesh Node.</summary>
	/// <param name="meshIndex">The Index of the Mesh Node. For meshes, it is the same as the NodeID. Valid values: (0
	/// to getNumMeshNodes()-1)</param>
	/// <returns>Return the Mesh Node from this model</returns>
	const Node& getMeshNode(uint32_t meshIndex) const
	{
		assertion(meshIndex < getNumMeshNodes(), "Invalid mesh index");
		// Mesh nodes are at the start of the array
		return getNode(meshIndex);
	}

	/// <summary>Get a specific Mesh Node.</summary>
	/// <param name="meshIndex">The Index of the Mesh Node. For meshes, it is the same as the NodeID. Valid values: (0
	/// to getNumMeshNodes()-1)</param>
	/// <returns>Return he Mesh Node from this model</returns>
	Node& getMeshNode(uint32_t meshIndex)
	{
		assertion(meshIndex < getNumMeshNodes(), "Invalid mesh index");
		// Mesh nodes are at the start of the array
		return getNode(meshIndex);
	}

	/// <summary>Connect mesh to a mesh node (i.e. set the node's mesh to the mesh</summary>
	/// <param name="meshId">The mesh id</param>
	/// <param name="meshNodeId">The mesh node id to connect to</param>
	void connectMeshWithMeshNode(uint32_t meshId, uint32_t meshNodeId) { getMeshNode(meshNodeId).setIndex(meshId); }

	/// <summary>Connect mesh to number of mesh nodes</summary>
	/// <param name="meshId">The mesh id</param>
	/// <param name="beginMeshNodeId">Begin mesh node id (inclusive)</param>
	/// <param name="endMeshNodeId">End mesh node id (inclusive)</param>
	void connectMeshWithMeshNodes(uint32_t meshId, uint32_t beginMeshNodeId, uint32_t endMeshNodeId)
	{
		for (uint32_t i = beginMeshNodeId; i <= endMeshNodeId; ++i) { connectMeshWithMeshNode(meshId, i); }
	}

	/// <summary>Assign material id to number of mesh nodes</summary>
	/// <param name="materialIndex">Material id</param>
	/// <param name="beginMeshNodeId">Begin mesh node id (inclusive)</param>
	/// <param name="endMeshNodeId">end mesh node id (inclusive)</param>
	void assignMaterialToMeshNodes(uint32_t materialIndex, uint32_t beginMeshNodeId, uint32_t endMeshNodeId)
	{
		for (uint32_t i = beginMeshNodeId; i <= endMeshNodeId; ++i) { getMeshNode(i).setMaterialIndex(materialIndex); }
	}

	/// <summary>Get the (global) Node Index of a specific MeshNode. This function is provided for completion, as
	/// NodeID == MeshNodeID</summary>
	/// <param name="meshNodeIndex">The Index of a Mesh Node that will be used to calculate the NodeID. Valid values:
	/// (0 to getNumMeshNodes()-1)</param>
	/// <returns>Return the Node index of the specified Mesh node. This function just returns the meshNodeIndex (but is
	/// harmless and inlined).</returns>
	uint32_t getNodeIdForMeshNodeId(uint32_t meshNodeIndex) const
	{
		debug_assertion(meshNodeIndex < getNumMeshNodes(), "invalid mesh node index");
		// Camera nodes are after the mesh and light nodes in the array
		return meshNodeIndex;
	}

	/// <summary>Get an iterator to the beginning of the meshes.</summary>
	/// <returns>Return an iterator</returns>
	std::vector<Mesh>::iterator beginMeshes() { return _data.meshes.begin(); }

	/// <summary>Get an iterator past the end of the meshes.</summary>
	/// <returns>Iterator to one past the last item of the mesh array</returns>
	std::vector<Mesh>::iterator endMeshes() { return _data.meshes.end(); }

	/// <summary>Get a const_iterator to the beginning of the meshes.</summary>
	/// <returns>Iterator to the start of the mesh array</returns>
	std::vector<Mesh>::const_iterator beginMeshes() const { return _data.meshes.begin(); }

	/// <summary>Get a const_iterator past the end of the meshes.</summary>
	/// <returns>Iterator to one past the last item of the mesh array</returns>
	std::vector<Mesh>::const_iterator endMeshes() const { return _data.meshes.end(); }

	/// <summary>Get the total number of nodes (Meshes, Cameras, Lights, others (helpers etc)).</summary>
	/// <returns>Return number of nodes in this model</returns>
	uint32_t getNumNodes() const { return static_cast<uint32_t>(_data.nodes.size()); }

	/// <summary>Get the node with the specified index.</summary>
	/// <param name="index">The index of the node to get</param>
	/// <returns>Return The Node from this scene</returns>
	const Node& getNode(uint32_t index) const { return _data.nodes[index]; }

	/// <summary>Get the node with the specified index.</summary>
	/// <param name="index">The index of the node to get</param>
	/// <returns>Return The Node from this scene</returns>
	Node& getNode(uint32_t index) { return _data.nodes[index]; }

	/// <summary>Get the number of distinct Textures in the scene.</summary>
	/// <returns>Return number of distinct textures</returns>
	uint32_t getNumTextures() const { return static_cast<uint32_t>(_data.textures.size()); }

	/// <summary>Get the texture with the specified index.</summary>
	/// <param name="index">The index of the texture to get</param>
	/// <returns>Return a texture from this scene</returns>
	const Texture& getTexture(uint32_t index) const { return _data.textures[index]; }

	/// <summary>Get the number of distinct Materials in the scene.</summary>
	/// <returns>Return number of materials in this scene</returns>
	uint32_t getNumMaterials() const { return static_cast<uint32_t>(_data.materials.size()); }

	/// <summary>Get the material with the specified index.</summary>
	/// <param name="index">The index of material to get</param>
	/// <returns>Return a material from this scene</returns>
	const Material& getMaterial(uint32_t index) const { return _data.materials[index]; }

	/// <summary>Add a material to this model, and gets its (just created) material id</summary>
	/// <param name="material">The the material to add to the materials of this model</param>
	/// <returns>The material id generated for the new material.</returns>
	uint32_t addMaterial(const Material& material)
	{
		_data.materials.emplace_back(material);
		return static_cast<uint32_t>(_data.materials.size() - 1);
	}

	/// <summary>Get the material with the specified index.</summary>
	/// <param name="index">The index of material to get</param>
	/// <returns>Return a material from this scene</returns>
	Material& getMaterial(uint32_t index) { return _data.materials[index]; }

	/// <summary>Get the total number of frames in the scene. The total number of usable animated frames is limited to
	/// exclude (numFrames - 1) but include any partial number up to (numFrames - 1). Example: If there are 100 frames
	/// of animation, the highest frame number allowed is 98, since that will blend between frames 98 and 99. (99
	/// being of course the 100th frame.)</summary>
	/// <returns>Return the number of frames in this model</returns>
	uint32_t getNumFrames() const { return _data.numFrames ? _data.numFrames : 1; }

	/// <summary>Get the current frame of the scene.</summary>
	/// <returns>Return the current frame</returns>
	float getCurrentFrame();

	/// <summary>Set the expected FPS of the animation.</summary>
	/// <param name="fps">FPS of the animation</param>
	void setFPS(float fps) { _data.FPS = fps; }

	/// <summary>Get the FPS this animation was created for.</summary>
	/// <returns>Get the expected FPS of the animation.</returns>
	float getFPS() const { return _data.FPS; }

	/// <summary>Set custom user data.</summary>
	/// <param name="size">The size, in bytes, of the data.</param>
	/// <param name="data">Pointer to the raw data. (size) bytes will be copied as-is from this pointer.</param>
	void setUserData(uint32_t size, const char* data);

	/// <summary>Only used for custom model creation. Allocate an number of cameras.</summary>
	/// <param name="count">Number of camera to allocate in this scene</param>
	void allocCameras(uint32_t count);

	/// <summary>Only used for custom model creation. Allocate a number of lights.</summary>
	/// <param name="count">number of lights to allocate in this scene</param>
	void allocLights(uint32_t count);

	/// <summary>Only used for custom model creation. Allocate a number of meshes.</summary>
	/// <param name="count">number of meshes to allocate in this scene</param>
	void allocMeshes(uint32_t count);

	/// <summary>Only used for custom model creation. Allocate a number of nodes.</summary>
	/// <param name="count">number of nodes to allocate in this scene</param>
	void allocNodes(uint32_t count);

	/// <summary>Get a reference to the internal data of this Model. Handle with care.</summary>
	/// <returns>Return internal data</returns>
	InternalData& getInternalData() { return _data; }

	/// <summary>Get the properties of a camera. This is additional info on the class (remarks or documentation).</summary>
	/// <param name="cameraIdx">The index of the camera.</param>
	/// <param name="fov">Camera field of view.</param>
	/// <param name="from">Camera position in world.</param>
	/// <param name="to">Camera target point in world.</param>
	/// <param name="up">Camera tilt up (roll) vector in world.</param>
	/// <param name="frameTimeInMs">The time in milli seconds to retrieve camera properties for.</param>
	/// <remarks>If cameraIdx >= number of cameras, an error will be logged and this function will have no effect.</remarks>
	void getCameraProperties(uint32_t cameraIdx, float& fov, glm::vec3& from, glm::vec3& to, glm::vec3& up, float frameTimeInMs = 0.f) const;

	/// <summary>Get the properties of a camera.</summary>
	/// <param name="cameraIdx">The index of the camera.</param>
	/// <param name="fov">Camera field of view in world.</param>
	/// <param name="from">Camera position in world.</param>
	/// <param name="to">Camera target point in world.</param>
	/// <param name="up">Camera tilt up (roll) vector in world.</param>
	/// <param name="nearClip">Camera near clipping plane distance</param>
	/// <param name="farClip">Camera far clipping plane distance</param>
	/// <param name="frameTimeInMs">The time in milli seconds to retrieve camera properties for.</param>
	/// <remarks>If cameraIdx >= number of cameras, an error will be logged and this function will have no effect.</remarks>
	void getCameraProperties(uint32_t cameraIdx, float& fov, glm::vec3& from, glm::vec3& to, glm::vec3& up, float& nearClip, float& farClip, float frameTimeInMs = 0.f) const;

	/// <summary>Get the direction of a spot or directional light.</summary>
	/// <param name="lightIdx">index of the light.</param>
	/// <param name="direction">The direction of the light.</param>
	/// <remarks>If lightIdx >= number of lights, an error will be logged and this function will have no effect.</remarks>
	void getLightDirection(uint32_t lightIdx, glm::vec3& direction) const;

	/// <summary>Get the position of a point or spot light.</summary>
	/// <param name="lightIdx">light index.</param>
	/// <param name="position">The position of the light.</param>
	/// <returns>False if <paramref name="lightIdx"/>does not exist</returns>
	/// <remarks>If lightIdx >= number of lights, an error will be logged and this function will have no effect.</remarks>
	void getLightPosition(uint32_t lightIdx, glm::vec3& position) const;

	/// <summary>Get the position of a point or spot light.</summary>
	/// <param name="lightIdx">light index.</param>
	/// <param name="position">The position of the light.</param>
	/// <remarks>If lightIdx >= number of lights, an error will be logged and this function will have no effect.</remarks>
	void getLightPosition(uint32_t lightIdx, glm::vec4& position) const;

	/// <summary>Free the resources of this model.</summary>
	void destroy() { _data = InternalData(); }

	/// <summary>Allocate the specified number of mesh nodes.</summary>
	/// <param name="no">The number of mesh nodes to allocate</param>
	void allocMeshNodes(uint32_t no);

	/// <summary>Add new texture</summary>
	/// <param name="tex"> Texture to add</param>
	/// <returns>returns texture index</returns>
	int32_t addTexture(const Texture& tex)
	{
		_data.textures.emplace_back(tex);
		return static_cast<int32_t>(_data.textures.size()) - 1;
	}
};

typedef Model::Material Material; ///< Export the Material into the pvr::assets namespace
typedef Model::Node Node; ///< Export the Node into the pvr::assets namespace
typedef Model::Mesh::VertexAttributeData VertexAttributeData; ///< Export the VertexAttributeData into the pvr::assets namespace

/// <summary>A ref counted handle to a Node. Created by calling getNodeHandle. Uses
///.the Model's reference count</summary>
typedef std::shared_ptr<Node> NodeHandle;
/// <summary>A ref counted handle to a Material. Created by calling getMaterialHandle. Uses
///.the Model's reference count</summary>
typedef std::shared_ptr<Material> MaterialHandle;

/// <summary>Create a Reference Counted Handle to a Mesh from a Model. The handle provided
/// works as any other std::shared_ptr smart pointer, and uses the "shared ref count"
/// feature that allows the created MeshHandle to use the Model's reference count (e.g. if
/// the Mesh is copied, the Model's reference count increases, and if all references to the
/// Model are released, the Model will be kept alive by this reference.</summary>
/// <param name="model">The model to whom the mesh we will create the handle for belongs</param>
/// <param name="meshId">The ID of the meshId inside <paramRef name="model"/></param>
/// <returns>A MeshHandle to the Mesh. It shares the ref counting of
/// <paramRef name="model"/></returns
inline MeshHandle getMeshHandle(ModelHandle model, uint32_t meshId) { return std::shared_ptr<Mesh>(model, &model->getMesh(meshId)); }

/// <summary>Create a Reference Counted Handle to a Material from a Model. The handle provided
/// works as any other std::shared_ptr smart pointer, and uses the "shared ref count"
/// feature that allows the created MaterialHandle to use the Model's reference count (e.g. if
/// the Material is copied, the Model's reference count increases, and if all references to the
/// Model are released, the Model will be kept alive by this reference.</summary>
/// <param name="model">The model to whom the material we will create the handle for belongs</param>
/// <param name="materialId">The ID of the material inside <paramRef name="model"/></param>
/// <returns>A MaterialHandle to the Material. It shares the ref counting of
/// <paramRef name="model"/></returns
inline MaterialHandle getMaterialHandle(ModelHandle model, uint32_t materialId) { return std::shared_ptr<Material>(model, &model->getMaterial(materialId)); }

/// <summary>Create a Reference Counted Handle to a Light from a Model. The handle provided
/// works as any other std::shared_ptr smart pointer, and uses the "shared ref count"
/// feature that allows the created LightHandle to use the Model's reference count (e.g. if
/// the Light is copied, the Model's reference count increases, and if all references to the
/// Model are released, the Model will be kept alive by this reference.</summary>
/// <param name="model">The model to whom the Light we will create the handle for belongs</param>
/// <param name="lightId">The ID of the Light inside <paramRef name="model"/></param>
/// <returns>A LightHandle to the Light. It shares the ref counting of
/// <paramRef name="model"/></returns>
inline LightHandle getLightHandle(ModelHandle model, uint32_t lightId) { return std::shared_ptr<Light>(model, &model->getLight(lightId)); }

/// <summary>Create a Reference Counted Handle to a Camera from a Model. The handle provided
/// works as any other std::shared_ptr smart pointer, and uses the "shared ref count"
/// feature that allows the created CameraHandle to use the Model's reference count (e.g. if
/// the Camera is copied, the Model's reference count increases, and if all references to the
/// Model are released, the Model will be kept alive by this reference.</summary>
/// <param name="model">The model to whom the Camera we will create the handle for belongs</param>
/// <param name="cameraId">The ID of the Camera inside <paramRef name="model"/></param>
/// <returns>A CameraHandle to the Camera. It shares the ref counting of
/// <paramRef name="model"/></returns>
inline CameraHandle getCameraHandle(ModelHandle model, uint32_t cameraId) { return std::shared_ptr<Camera>(model, &model->getCamera(cameraId)); }

/// <summary>Create a Reference Counted Handle to a Node from a Model. The handle provided
/// works as any other std::shared_ptr smart pointer, and uses the "shared ref count"
/// feature that allows the created NodeHandle to use the Model's reference count (e.g. if
/// the Node is copied, the Model's reference count increases, and if all references to the
/// Model are released, the Model will be kept alive by this reference.</summary>
/// <param name="model">The model to whom the Node we will create the handle for belongs</param>
/// <param name="nodeId">The ID of the Node inside <paramRef name="model"/></param>
/// <returns>A Node Handle to the Node. It shares the ref counting of
/// <paramRef name="model"/></returns>
inline NodeHandle getNodeHandle(ModelHandle model, uint32_t nodeId) { return std::shared_ptr<Node>(model, &model->getNode(nodeId)); }
} // namespace assets
} // namespace pvr
