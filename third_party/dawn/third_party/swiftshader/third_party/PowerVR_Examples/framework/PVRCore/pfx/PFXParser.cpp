/*!
\brief Implementation of methods of the PFXReader class.
\file PVRCore/pfx/PFXParser.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

//!\cond NO_DOXYGEN
#include "PVRCore/pfx/PFXParser.h"
#include "PVRCore/strings/StringFunctions.h"
#include "PVRCore/stream/FileStream.h"
#include "PVRCore/stream/BufferStream.h"
#include "PVRCore/texture/Texture.h"
#include "PVRCore/Log.h"
#include "pugixml.hpp"
#include <set>

namespace pvr {
namespace pfx {
namespace {
ImageDataFormat getFormat(const pugi::xml_attribute& attr)
{
	struct ImageFormat
	{
		StringHash name;
		ImageDataFormat fmt;
	};
	static const ImageFormat bufferFormats[] = {
		ImageFormat{ StringHash("r8_unorm"), ImageDataFormat(PixelFormat::R_8(), VariableType::UnsignedByteNorm, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r8_uint"), ImageDataFormat(PixelFormat::R_8(), VariableType::UnsignedByte, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r8_sint"), ImageDataFormat(PixelFormat::R_8(), VariableType::SignedByte, ColorSpace::lRGB) },

		ImageFormat{ StringHash("r8g8_unorm"), ImageDataFormat(PixelFormat::RG_88(), VariableType::UnsignedByteNorm, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r8g8_uint"), ImageDataFormat(PixelFormat::RG_88(), VariableType::UnsignedByte, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r8g8_sint"), ImageDataFormat(PixelFormat::RG_88(), VariableType::SignedByte, ColorSpace::lRGB) },

		ImageFormat{ StringHash("r8g8b8a8_unorm"), ImageDataFormat(PixelFormat::RGBA_8888(), VariableType::UnsignedByteNorm, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r8g8b8a8_uint"), ImageDataFormat(PixelFormat::RGBA_8888(), VariableType::UnsignedByte, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r8g8b8a8_sint"), ImageDataFormat(PixelFormat::RGBA_8888(), VariableType::SignedByte, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r8g8b8a8_unorm_srgb"), ImageDataFormat(PixelFormat::RGBA_8888(), VariableType::UnsignedByteNorm, ColorSpace::sRGB) },

		ImageFormat{ StringHash("b8g8r8a8_unorm"), ImageDataFormat(PixelFormat::BGRA_8888(), VariableType::UnsignedByteNorm, ColorSpace::lRGB) },
		ImageFormat{ StringHash("b8g8r8a8_unorm_srgb"), ImageDataFormat(PixelFormat::BGRA_8888(), VariableType::UnsignedByteNorm, ColorSpace::sRGB) },

		ImageFormat{ StringHash("a8b8g8r8_unorm"), ImageDataFormat(PixelFormat::ABGR_8888(), VariableType::UnsignedByteNorm, ColorSpace::lRGB) },
		ImageFormat{ StringHash("a8b8g8r8_uint"), ImageDataFormat(PixelFormat::ABGR_8888(), VariableType::UnsignedByte, ColorSpace::lRGB) },
		ImageFormat{ StringHash("a8b8g8r8_sint"), ImageDataFormat(PixelFormat::ABGR_8888(), VariableType::SignedByteNorm, ColorSpace::lRGB) },
		ImageFormat{ StringHash("a8b8g8r8_unorm_srgb"), ImageDataFormat(PixelFormat::ABGR_8888(), VariableType::UnsignedByteNorm, ColorSpace::sRGB) },

		ImageFormat{ StringHash("r16_uint"), ImageDataFormat(PixelFormat::R_16(), VariableType::UnsignedShort, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r16_sint"), ImageDataFormat(PixelFormat::R_16(), VariableType::SignedShort, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r16_sfloat"), ImageDataFormat(PixelFormat::R_16(), VariableType::SignedFloat, ColorSpace::lRGB) },

		ImageFormat{ StringHash("r16g16_uint"), ImageDataFormat(PixelFormat::RG_1616(), VariableType::UnsignedShort, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r16g16_sint"), ImageDataFormat(PixelFormat::RG_1616(), VariableType::SignedShort, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r16g16_sfloat"), ImageDataFormat(PixelFormat::RG_1616(), VariableType::SignedFloat, ColorSpace::lRGB) },

		ImageFormat{ StringHash("r16g16b16a16_uint"), ImageDataFormat(PixelFormat::RGBA_16161616(), VariableType::UnsignedShort, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r16g16b16a16_sint"), ImageDataFormat(PixelFormat::RGBA_16161616(), VariableType::SignedShort, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r16g16b16a16_sfloat"), ImageDataFormat(PixelFormat::RGBA_16161616(), VariableType::SignedFloat, ColorSpace::lRGB) },

		ImageFormat{ StringHash("r32_uint"), ImageDataFormat(PixelFormat::R_32(), VariableType::UnsignedInteger, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r32_sint"), ImageDataFormat(PixelFormat::R_32(), VariableType::SignedInteger, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r32_sfloat"), ImageDataFormat(PixelFormat::R_32(), VariableType::SignedFloat, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r32g32_uint"), ImageDataFormat(PixelFormat::RG_3232(), VariableType::UnsignedInteger, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r32g32_sint"), ImageDataFormat(PixelFormat::RG_3232(), VariableType::SignedInteger, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r32g32_sfloat"), ImageDataFormat(PixelFormat::RG_3232(), VariableType::SignedFloat, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r32g32b32a32_uint"), ImageDataFormat(PixelFormat::RGBA_32323232(), VariableType::UnsignedInteger, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r32g32b32a32_sint"), ImageDataFormat(PixelFormat::RGBA_32323232(), VariableType::SignedInteger, ColorSpace::lRGB) },
		ImageFormat{ StringHash("r32g32b32a32_sfloat"), ImageDataFormat(PixelFormat::RGBA_32323232(), VariableType::SignedFloat, ColorSpace::lRGB) },

		ImageFormat{ StringHash("d16"), ImageDataFormat(PixelFormat::Depth16(), VariableType::UnsignedShort, ColorSpace::lRGB) },
		ImageFormat{ StringHash("d24"), ImageDataFormat(PixelFormat::Depth24(), VariableType::UnsignedInteger, ColorSpace::lRGB) },
		ImageFormat{ StringHash("d24s8"), ImageDataFormat(PixelFormat::Depth24Stencil8(), VariableType::UnsignedInteger, ColorSpace::lRGB) },
		ImageFormat{ StringHash("d32"), ImageDataFormat(PixelFormat::Depth32(), VariableType::UnsignedInteger, ColorSpace::lRGB) },
	};

	ImageDataFormat retval;
	if (attr && attr.value())
	{
		const StringHash fmtStr(strings::toLower(attr.value()));

		for (const ImageFormat buffFmt : bufferFormats)
		{
			if (buffFmt.name == fmtStr)
			{
				retval = buffFmt.fmt;
				break;
			}
		}
		if (retval.format.getPixelTypeId() == 0)
		{
			Log(LogLevel::Warning,
				"PfxParser: 'format' attribute of <texture> "
				"element was provided, but the format %s not recognized. Defaulting to RGBA8888.",
				attr.value());
			retval = ImageDataFormat();
		}
	}
	return retval;
}

std::map<std::string, GpuDatatypes> datatype_strings;

inline bool static_call_only_once_initialize_map()
{
	datatype_strings.clear();
	datatype_strings["mat2"] = GpuDatatypes::mat2x2;
	datatype_strings["mat2x2"] = GpuDatatypes::mat2x2;
	datatype_strings["mat2x3"] = GpuDatatypes::mat2x3;
	datatype_strings["mat2x4"] = GpuDatatypes::mat2x4;
	datatype_strings["mat3"] = GpuDatatypes::mat3x3;
	datatype_strings["mat3x2"] = GpuDatatypes::mat3x2;
	datatype_strings["mat3x3"] = GpuDatatypes::mat3x3;
	datatype_strings["mat3x4"] = GpuDatatypes::mat3x4;
	datatype_strings["mat4"] = GpuDatatypes::mat4x4;
	datatype_strings["mat4x2"] = GpuDatatypes::mat4x2;
	datatype_strings["mat4x3"] = GpuDatatypes::mat4x3;
	datatype_strings["mat4x4"] = GpuDatatypes::mat4x4;
	datatype_strings["vec2"] = GpuDatatypes::vec2;
	datatype_strings["vec3"] = GpuDatatypes::vec3;
	datatype_strings["vec4"] = GpuDatatypes::vec4;
	datatype_strings["ivec2"] = GpuDatatypes::ivec2;
	datatype_strings["ivec3"] = GpuDatatypes::ivec3;
	datatype_strings["ivec4"] = GpuDatatypes::ivec4;
	datatype_strings["uvec2"] = GpuDatatypes::uvec2;
	datatype_strings["uvec3"] = GpuDatatypes::uvec3;
	datatype_strings["uvec4"] = GpuDatatypes::uvec4;
	datatype_strings["bvec2"] = GpuDatatypes::bvec2;
	datatype_strings["bvec3"] = GpuDatatypes::bvec3;
	datatype_strings["bvec4"] = GpuDatatypes::bvec4;
	datatype_strings["float"] = GpuDatatypes::Float;
	datatype_strings["float"] = GpuDatatypes::Float;
	datatype_strings["int"] = GpuDatatypes::Integer;
	datatype_strings["int8_t"] = GpuDatatypes::Integer;
	datatype_strings["int16_t"] = GpuDatatypes::Integer;
	datatype_strings["int32_t"] = GpuDatatypes::Integer;
	datatype_strings["uint"] = GpuDatatypes::uinteger;
	datatype_strings["uint8_t"] = GpuDatatypes::uinteger;
	datatype_strings["uint16_t"] = GpuDatatypes::uinteger;
	datatype_strings["uint32_t"] = GpuDatatypes::uinteger;
	datatype_strings["bool"] = GpuDatatypes::boolean;

	return true;
}

inline GpuDatatypes dataTypeFromString(const std::string& mystr)
{
	// bypass the warning
	static bool initialize_map = static_call_only_once_initialize_map();
	(void)initialize_map;
	auto it = datatype_strings.find(strings::toLower(mystr));

	if (it == datatype_strings.end()) { Log(LogLevel::Warning, "Unrecognized datatype [%s] reading PFX file", mystr.c_str()); }
	return it == datatype_strings.end() ? GpuDatatypes::none : it->second;
}

const std::string uniform_str("uniform");
const std::string storage_str("storage");
const std::string uniformdynamic_str("uniformdynamic");
const std::string storagedynamic_str("storagedynamic");
const std::string dynamicuniform_str("dynamicuniform");
const std::string dynamicstorage_str("dynamicstorage");

inline DescriptorType bufferDescriptorTypeFromString(const std::string& mystr)
{
	const std::string str = strings::toLower(mystr);
	if (str == uniform_str) { return DescriptorType::UniformBuffer; }
	else if (str == storage_str)
	{
		return DescriptorType::StorageBuffer;
	}
	else if (str == uniformdynamic_str)
	{
		return DescriptorType::UniformBufferDynamic;
	}
	else if (str == storagedynamic_str)
	{
		return DescriptorType::StorageBufferDynamic;
	}
	else if (str == dynamicuniform_str)
	{
		return DescriptorType::UniformBufferDynamic;
	}
	else if (str == dynamicstorage_str)
	{
		return DescriptorType::StorageBufferDynamic;
	}
	return DescriptorType::UniformBuffer;
}

const std::string nearest_str("nearest");
const std::string linear_str("linear");
const std::string cubic_str("cubic");
const std::string none_str("none");
inline Filter filterFromAttribute(const pugi::xml_attribute& attr, Filter default_value)
{
	Filter ret = default_value;
	if (!attr.empty())
	{
		const std::string value = strings::toLower(attr.value());
		if (value == nearest_str) { ret = Filter::Nearest; }
		else if (value == linear_str)
		{
			ret = Filter::Linear;
		}
		else if (value == cubic_str)
		{
			ret = Filter::Cubic;
		}
		else if (value == none_str)
		{
			ret = Filter::None;
		}
	}
	return ret;
}

inline SamplerMipmapMode mipMapModeFromAttribute(const pugi::xml_attribute& attr, SamplerMipmapMode default_value)
{
	SamplerMipmapMode ret = default_value;
	if (!attr.empty())
	{
		const std::string value = strings::toLower(attr.value());
		if (value == nearest_str) { ret = SamplerMipmapMode::Nearest; }
		else if (value == linear_str)
		{
			ret = SamplerMipmapMode::Linear;
		}
	}
	return ret;
}

const std::string clamp_str("clamp");
const std::string repeat_str("repeat");
inline SamplerAddressMode wrapFromAttribute(const pugi::xml_attribute& attr, SamplerAddressMode default_value)
{
	SamplerAddressMode ret = default_value;
	if (!attr.empty())
	{
		const std::string value = strings::toLower(attr.value());
		if (value == clamp_str) { ret = SamplerAddressMode::ClampToEdge; }
		else if (value == repeat_str)
		{
			ret = SamplerAddressMode::Repeat;
		}
	}
	return ret;
}

const std::string requiresUniformSemantic_str("requiresuniformsemantic");
const std::string requiresUniformSemanticNotPresent_str("requiresuniformsemanticnotpresent");
const std::string requiresUniformSemanticPresent_str("requiresuniformsemanticpresent");
const std::string requiresAttributeSemantic_str("requiresattributesemantic");
const std::string requiresAttributeSemanticPresent_str("requiresattributesemanticpresent");
const std::string requiresAttributeSemanticNotPresent_str("requiresattributesemanticnotpresent");

inline effect::PipelineCondition::ConditionType conditionFromAttribute(const pugi::xml_attribute& attr)
{
	effect::PipelineCondition::ConditionType ret = effect::PipelineCondition::Always;
	if (!attr.empty())
	{
		const std::string value = strings::toLower(attr.value());
		if (value == requiresUniformSemantic_str) { ret = effect::PipelineCondition::UniformRequired; }
		else if (value == requiresUniformSemanticNotPresent_str)
		{
			ret = effect::PipelineCondition::UniformRequiredNo;
		}
		else if (value == requiresAttributeSemantic_str)
		{
			ret = effect::PipelineCondition::AttributeRequired;
		}
		else if (value == requiresAttributeSemanticNotPresent_str)
		{
			ret = effect::PipelineCondition::AttributeRequiredNo;
		}
		else if (value == requiresUniformSemanticPresent_str)
		{
			ret = effect::PipelineCondition::UniformRequired;
		}
		else if (value == requiresAttributeSemanticPresent_str)
		{
			ret = effect::PipelineCondition::AttributeRequired;
		}
	}
	return ret;
}

const std::string vertex_str("vertex");
const std::string fragment_str("fragment");
const std::string geometry_str("geometry");
const std::string tessControl_str("tesscontrol");
const std::string tessellationControl_str("tessellationcontrol");
const std::string tessEvaluation_str("tessevaluation");
const std::string tessellationEvaluation_str("tessellationevaluation");
inline ShaderType shaderTypeFromString(pugi::xml_attribute& attr)
{
	ShaderType ret = ShaderType::UnknownShader;
	if (!attr.empty())
	{
		const std::string value = strings::toLower(attr.value());
		if (value == vertex_str) { ret = ShaderType::VertexShader; }
		else if (value == fragment_str)
		{
			ret = ShaderType::FragmentShader;
		}
		else if (value == geometry_str)
		{
			ret = ShaderType::GeometryShader;
		}
		else if (value == tessControl_str)
		{
			ret = ShaderType::TessControlShader;
		}
		else if (value == tessellationControl_str)
		{
			ret = ShaderType::TessControlShader;
		}
		else if (value == tessEvaluation_str)
		{
			ret = ShaderType::TessEvaluationShader;
		}
		else if (value == tessellationEvaluation_str)
		{
			ret = ShaderType::TessEvaluationShader;
		}
	}
	return ret;
}

const std::string model_str("model");
const std::string node_str("node");
const std::string effect_str("effect");
const std::string bonebatch_str("bonebatch");
const std::string automatic_str("automatic");
const std::string auto_str("auto");
inline VariableScope scopeFromString(const pugi::xml_attribute& attr)
{
	VariableScope ret = VariableScope::Effect;
	if (!attr.empty())
	{
		const std::string value = strings::toLower(attr.value());
		if (value == automatic_str || value == auto_str) { ret = VariableScope::Automatic; }
		else if (value == effect_str)
		{
			ret = VariableScope::Effect;
		}
		else if (value == model_str)
		{
			ret = VariableScope::Model;
		}
		else if (value == node_str)
		{
			ret = VariableScope::Node;
		}
		else if (value == bonebatch_str)
		{
			ret = VariableScope::BoneBatch;
		}
		else
		{
			Log("PFXParser: Type '%s' for buffer or uniform scope was not recognized. Valid values: 'model', 'node', 'effect'", attr.value());
		}
	}
	return ret;
}

const std::string blend_factor_str[] = { "zero", "one", "srccolor", "oneminussrccolor", "dstcolor", "oneminusdstcolor", "srcalpha", "oneminussrcalpha", "dstalpha",
	"oneminusdstalpha", "constantcolor", "oneminusconstantcolor", "constantalpha", "oneminusconstantalpha", "src1color", "oneminussrc1color", "src1alpha", "oneminussrc1alpha" };

static_assert(ARRAY_SIZE(blend_factor_str) == static_cast<uint32_t>(BlendFactor::NumBlendFactor), "Number blendfactor strings must be same as the BlendFactor::NumBlendFactor");

inline BlendFactor blendFactorFromString(const char* val, BlendFactor defaultBlend)
{
	BlendFactor ret = defaultBlend;
	const std::string value = strings::toLower(val);
	for (uint32_t i = 0; i < static_cast<uint32_t>(BlendFactor::NumBlendFactor); ++i)
	{
		if (value == blend_factor_str[i])
		{
			ret = BlendFactor(i);
			break;
		}
	}
	return ret;
}

// BlendOps
const std::string blend_op_str[] = { "add", "subtract", "reversesubtract", "min", "max" };

static_assert(ARRAY_SIZE(blend_op_str) == static_cast<uint32_t>(BlendOp::NumBlendFunc), "Number blendop strings must be same as the BlendOp::NumBlendFunc");

inline BlendOp blendOpFromString(const pugi::xml_attribute& attr)
{
	BlendOp ret = BlendOp::Default;
	const std::string value = strings::toLower(attr.value());
	if (!value.empty())
	{
		if (value == blend_op_str[0]) { ret = BlendOp::Add; }
		else if (value == blend_op_str[1])
		{
			ret = BlendOp::Subtract;
		}
		else if (value == blend_op_str[2])
		{
			ret = BlendOp::ReverseSubtract;
		}
		else if (value == blend_op_str[3])
		{
			ret = BlendOp::Min;
		}
		else if (value == blend_op_str[4])
		{
			ret = BlendOp::Max;
		}
		else
		{
			Log("PFXParser: Type '%s' for BlendOp as not recognized. using the default %s", attr.value(), blend_op_str[static_cast<uint32_t>(ret)].c_str());
		}
	}
	return ret;
}

ColorChannelFlags blendChannelWriteMaskFromString(const pugi::xml_attribute& attr)
{
	if (strlen(attr.value()) == 0) { return ColorChannelFlags::All; }
	const std::string value(strings::toLower(attr.value()));

	if (value == "none") { return ColorChannelFlags::None; }

	ColorChannelFlags bits = ColorChannelFlags(0);
	if (value.find_first_of('r') != std::string::npos) { bits |= ColorChannelFlags::R; }
	if (value.find_first_of('g') != std::string::npos) { bits |= ColorChannelFlags::G; }
	if (value.find_first_of('b') != std::string::npos) { bits |= ColorChannelFlags::B; }
	if (value.find_first_of('a') != std::string::npos) { bits |= ColorChannelFlags::A; }
	return bits;
}

const char* comparison_mode_str[] = {
	"never",
	"less",
	"equal",
	"lequal",
	"greater",
	"notequal",
	"gequal",
	"always",
};
static_assert(ARRAY_SIZE(comparison_mode_str) == static_cast<uint32_t>(CompareOp::NumComparisonMode),
	"Number comparison_mode_str strings must be same as the ComparisonMode::NumComparisonMode");

inline CompareOp comparisonModeFromString(const char* value, CompareOp dflt)
{
	const std::string val(strings::toLower(value));
	CompareOp rtn = dflt;
	for (uint32_t i = 0; i < static_cast<uint32_t>(CompareOp::NumComparisonMode); ++i)
	{
		if (strcmp(val.c_str(), comparison_mode_str[i]) == 0)
		{
			rtn = CompareOp(i);
			break;
		}
	}
	return rtn;
}

void addTextures(effect::Effect& effect, pugi::xml_named_node_iterator begin, pugi::xml_named_node_iterator end)
{
	for (auto it = begin; it != end; ++it)
	{
		effect::TextureDefinition tex;
		tex.name = it->attribute("name") ? it->attribute("name").value() : StringHash();
		tex.path = it->attribute("path") ? it->attribute("path").value() : StringHash();
		tex.height = it->attribute("height") ? static_cast<uint32_t>(it->attribute("height").as_int()) : 0u;
		tex.width = it->attribute("width") ? static_cast<uint32_t>(it->attribute("width").as_int()) : 0;
		tex.format = getFormat(it->attribute("format"));
		effect.addTexture(std::move(tex));
	}
}

void addEntryToBuffer(effect::BufferDefinition& buffer, pugi::xml_node& entry_node)
{
	effect::BufferDefinition::Entry entry;
	entry.arrayElements = 1;
	if (entry_node.attribute("arrayElements")) { entry.arrayElements = entry_node.attribute("arrayElements").as_int(); }
	entry.semantic = entry_node.attribute("semantic").value();
	entry.dataType = dataTypeFromString(entry_node.attribute("dataType").value());
	buffer.entries.emplace_back(std::move(entry));
}

void addBuffers(effect::Effect& effect, pugi::xml_named_node_iterator begin, pugi::xml_named_node_iterator end)
{
	for (auto it = begin; it != end; ++it)
	{
		effect::BufferDefinition buff;
		buff.name = it->attribute("name") ? it->attribute("name").value() : StringHash();

		buff.scope = it->attribute("scope") ? scopeFromString(it->attribute("scope")) : VariableScope::Effect;

		buff.multibuffering = it->attribute("multibuffering").as_bool();

		for (auto child = it->children().begin(); child != it->children().end(); ++child) { addEntryToBuffer(buff, *child); }
		effect.addBuffer(std::move(buff));
	}
}

std::unique_ptr<Stream> getStream(const std::string& filename, const IAssetProvider* assetProvider)
{
	if (assetProvider) { return assetProvider->getAssetStream(filename); }
	else
	{
		return std::make_unique<FileStream>(filename, "r");
	}
}

void addFileCodeSourceToVector(std::vector<char>& shaderSource, const char* filename, const IAssetProvider* assetProvider)
{
	std::unique_ptr<Stream> str = getStream(filename, assetProvider);
	if (!str->isReadable()) { throw FileIOError(filename, "PfxParser: - File not found"); }
	str->readIntoBuffer(shaderSource);
}

void addShaderCodeToVectors(const StringHash& /*name*/, ShaderType shaderType, std::map<StringHash, std::pair<ShaderType, std::vector<char> /**/> /**/>& versionedShaders,
	const pugi::xml_node& node, const StringHash& apiVersion, bool isFile, bool addToAll, const IAssetProvider* assetProvider)
{
	// The next two lines will select either running the for-loop just once for the value matching versionedShaders,
	// or for all values in versionedShaders (i.e. was "apiVersion" nothing?)
	// Precondition: addToAll is true, or apiVersion exists in versionedShaders.
	const char* node_value = isFile ? NULL : node.child_value();
	std::vector<char>& rawData_vector = versionedShaders[apiVersion].second;
	versionedShaders[apiVersion].first = shaderType;

	size_t initial_size = rawData_vector.size();
	size_t value_size = node_value ? strlen(node_value) : 0;

	if (isFile)
	{
		if (node.attribute("path"))
		{
			// Append the data to the "main" node's vector
			addFileCodeSourceToVector(rawData_vector, node.attribute("path").value(), assetProvider);
		}
		else
		{
			Log(LogLevel::Warning,
				"PfxParser: Found <file> element in <shader>, but no 'path' attribute."
				" Skipping. Syntax should be <file path=\"pathname...\".");
		}
	}
	else
	{
		// Append the data to the node's vector
		rawData_vector.resize(rawData_vector.size() + value_size);
		memcpy(rawData_vector.data() + initial_size, node_value, value_size);
	}

	// Append to everything if requested
	if (addToAll)
	{
		for (auto it = versionedShaders.begin(); it != versionedShaders.end(); ++it)
		{
			if (&it->second.second == &rawData_vector) { continue; } // Skip yourself!
			// Append the additional data to the node's vector
			it->second.second.resize(it->second.second.size() + value_size);
			memcpy(it->second.second.data() + (it->second.second.size() - value_size), rawData_vector.data() + initial_size, value_size);
		}
	}
}

void addShaders(effect::Effect& theEffect, pugi::xml_named_node_iterator begin, pugi::xml_named_node_iterator end, const IAssetProvider* assetProvider)
{
	// For each shader element, we will create one per version...
	for (auto shader = begin; shader != end; ++shader)
	{
		std::map<StringHash, std::pair<ShaderType, std::vector<char> /**/> /**/> versionedShaders;
		StringHash shaderName;

		ShaderType shaderType = ShaderType::UnknownShader;
		// Get its name
		for (auto it2 = shader->attributes_begin(); it2 != shader->attributes_end(); ++it2)
		{
			if (std::string(it2->name()) == std::string("name")) { shaderName = it2->value(); }
			if (std::string(it2->name()) == std::string("type")) { shaderType = shaderTypeFromString(*it2); }
		}
		if (shaderType == ShaderType::UnknownShader)
		{
			Log("PFXReader: Shader with name [%s] was defined without the [type] attribute, or value was unrecognised.", shaderName.c_str());
			continue;
		}
		if (shaderName.empty())
		{
			Log("PFXReader: <shader> element did not have a [name] attribute, and will be skipped as it will not be possible to be referenced by other elements.");
			continue;
		}

		// Generate a list of api versions iterating every child element of the shader.
		// Should be either <file> or <code>, and may or may not contain an apiversion attribute.
		// For now, we create the list of apiversions.
		for (auto child = shader->children().begin(); child != shader->children().end(); ++child)
		{
			const auto& apiVersionAttr = child->attribute("apiVersion");
			if (apiVersionAttr) { versionedShaders[apiVersionAttr.value()]; }
			else
			{
				versionedShaders[StringHash()];
			}
		}
		// All valid values have now been added to the vector. Now we concatenate all of them that are
		// either global or belong to the same apiversion.
		for (auto child = shader->children().begin(); child != shader->children().end(); ++child)
		{
			const auto& apiVersionAttr = child->attribute("apiVersion");
			bool isFile = std::string(child->name()) == std::string("file"), isCode = std::string(child->name()) == std::string("code");

			if (isFile || isCode) // NoCode!
			{
				addShaderCodeToVectors(shaderName, shaderType, versionedShaders, *child, apiVersionAttr ? StringHash(apiVersionAttr.value()) : StringHash(), isFile,
					apiVersionAttr.empty(), assetProvider);
			}
			else
			{
				Log(LogLevel::Warning, "PfxParser: Found node that was neither <code> nor <file> while parsing a <shader>. Skipping.");
			}
		}
		// One last bit! Actually add them to the effect. Note - they are character arrays without null-terminators...
		for (auto entry = versionedShaders.begin(); entry != versionedShaders.end(); ++entry)
		{ theEffect.addShader(entry->first, effect::Shader(StringHash(shaderName), entry->second.first, std::string(entry->second.second.begin(), entry->second.second.end()))); }
	}
}

void addPipelineAttribute(effect::Effect&, const StringHash&, effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	effect::AttributeSemantic semantic;
	semantic.dataType = dataTypeFromString(attribute_element.attribute("dataType").value());
	semantic.location = static_cast<uint8_t>(attribute_element.attribute("location").as_int());
	semantic.semantic = attribute_element.attribute("semantic").value();
	semantic.variableName = attribute_element.attribute("variable").value();
	semantic.vboBinding = static_cast<uint8_t>(attribute_element.attribute("vboBinding").as_int());
	pipeline.attributes.emplace_back(std::move(semantic));
}

void addPipelineUniform(effect::Effect&, const StringHash&, effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	effect::UniformSemantic semantic;
	semantic.dataType = dataTypeFromString(attribute_element.attribute("dataType").value());
	semantic.arrayElements = attribute_element.attribute("arrayElements").as_int();
	if (semantic.arrayElements == 0) { semantic.arrayElements = 1; }
	semantic.semantic = attribute_element.attribute("semantic").value();
	semantic.variableName = attribute_element.attribute("variable").value();
	semantic.scope = scopeFromString(attribute_element.attribute("scope"));
	semantic.set = static_cast<uint8_t>(attribute_element.attribute("set").as_int());
	semantic.binding = static_cast<uint8_t>(attribute_element.attribute("binding").as_int());
	pipeline.uniforms.emplace_back(semantic);
}

void addPipelineShader(effect::Effect& effect, const StringHash& apiName, effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	effect::Shader shader;
	shader.name = attribute_element.attribute("name").value();
	auto it = effect.versionedShaders[apiName].find(shader.name);
	if (it != effect.versionedShaders[apiName].end()) { pipeline.shaders.emplace_back(&(it->second)); }
	else
	{
		if (!apiName.empty())
		{
			Log(LogLevel::Warning, "PFXParser: Could not find shader with name [%s] referenced in pipeline [%s] for api [%s]", shader.name.c_str(), pipeline.name.c_str(),
				apiName.c_str());
		}
		else
		{
			Log(LogLevel::Warning, "PFXParser: Could not find shader with name [%s] referenced in pipeline [%s] for api unspecified.", shader.name.c_str(), pipeline.name.c_str());
		}
	}
}

void addPipelineBuffer(effect::Effect& effect, const StringHash&, effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	StringHash name = attribute_element.attribute("name").value();

	auto it = effect.buffers.find(name);
	if (it != effect.buffers.end())
	{
		effect::BufferRef ref;
		ref.binding = static_cast<int8_t>(attribute_element.attribute("binding").as_int());
		ref.set = static_cast<int8_t>(attribute_element.attribute("set").as_int());
		ref.semantic = attribute_element.attribute("semantic").value();
		ref.type = bufferDescriptorTypeFromString(attribute_element.attribute("type").value());
		ref.bufferName = name;
		it->second.allSupportedBindings = it->second.allSupportedBindings | descriptorTypeToBufferUsage(ref.type);
		it->second.isDynamic = pvr::isDescriptorTypeDynamic(ref.type);
		pipeline.buffers.emplace_back(ref);
	}
	else
	{
		Log("PfxParser::read: Could not find buffer definition [%s] referenced in pipeline [%d]", pipeline.name.c_str(), name.c_str());
	}
}

void addPipelineInputAttachment(effect::Effect&, const StringHash&, effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	effect::InputAttachmentRef ref;
	ref.binding = static_cast<int8_t>(attribute_element.attribute("binding").as_int());
	ref.set = static_cast<int8_t>(attribute_element.attribute("set").as_int(-1));
	ref.targetIndex = static_cast<int8_t>(attribute_element.attribute("targetIndex").as_int(-1));
	pipeline.inputAttachments.emplace_back(ref);
}

void addPipelineTexture(effect::Effect& effect, const StringHash&, effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	StringHash name = attribute_element.attribute("name").value();
	if (effect.textures.find(name) != effect.textures.end()) { return; }

	effect::TextureReference ref;
	ref.binding = static_cast<int8_t>(attribute_element.attribute("binding").as_int());
	ref.set = static_cast<int8_t>(attribute_element.attribute("set").as_int(-1));
	ref.semantic = attribute_element.attribute("semantic").value();
	ref.samplerFilter = packSamplerFilter(filterFromAttribute(attribute_element.attribute("minification"), Filter::Nearest),
		filterFromAttribute(attribute_element.attribute("magnification"), Filter::Nearest), mipMapModeFromAttribute(attribute_element.attribute("mipmap"), SamplerMipmapMode::Nearest));
	ref.wrapR = wrapFromAttribute(attribute_element.attribute("wrap_r"), SamplerAddressMode::ClampToEdge);
	ref.wrapS = wrapFromAttribute(attribute_element.attribute("wrap_s"), SamplerAddressMode::ClampToEdge);
	ref.wrapT = wrapFromAttribute(attribute_element.attribute("wrap_t"), SamplerAddressMode::ClampToEdge);
	ref.wrapR = wrapFromAttribute(attribute_element.attribute("wrap_u"), ref.wrapR);
	ref.wrapS = wrapFromAttribute(attribute_element.attribute("wrap_v"), ref.wrapS);
	ref.wrapT = wrapFromAttribute(attribute_element.attribute("wrap_w"), ref.wrapT);
	ref.wrapR = wrapFromAttribute(attribute_element.attribute("wrap_x"), ref.wrapR);
	ref.wrapS = wrapFromAttribute(attribute_element.attribute("wrap_y"), ref.wrapS);
	ref.wrapT = wrapFromAttribute(attribute_element.attribute("wrap_z"), ref.wrapT);
	ref.variableName = attribute_element.attribute("variable").value();
	ref.textureName = name;
	pipeline.textures.emplace_back(ref);
}

void addPipelineBlending(effect::Effect&, const StringHash&, effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	pipeline.blending.blendEnable = attribute_element.attribute("enabled").as_bool();
	pipeline.blending.srcBlendColor = blendFactorFromString(attribute_element.attribute("srcColorFactor").as_string(), BlendFactor::DefaultSrcRgba);
	pipeline.blending.srcBlendAlpha = blendFactorFromString(attribute_element.attribute("srcAlphaFactor").as_string(), BlendFactor::DefaultSrcRgba);
	pipeline.blending.dstBlendColor = blendFactorFromString(attribute_element.attribute("dstColorFactor").as_string(), BlendFactor::DefaultDestRgba);
	pipeline.blending.dstBlendAlpha = blendFactorFromString(attribute_element.attribute("dstAlphaFactor").as_string(), BlendFactor::DefaultDestRgba);
	pipeline.blending.blendOpColor = blendOpFromString(attribute_element.attribute("colorBlendOp"));
	pipeline.blending.blendOpAlpha = blendOpFromString(attribute_element.attribute("alphaBlendOp"));
	pipeline.blending.channelWriteMask = blendChannelWriteMaskFromString(attribute_element.attribute("writeMask"));
}

inline StencilOp stencilOpFromString(const std::string& str, StencilOp dflt)
{
	if (str == "keep") { return StencilOp::Keep; }
	else if (str == "zero")
	{
		return StencilOp::Zero;
	}
	else if (str == "replace")
	{
		return StencilOp::Replace;
	}
	else if (str == "incrementclamp")
	{
		return StencilOp::IncrementClamp;
	}
	else if (str == "decrementclamp")
	{
		return StencilOp::DecrementClamp;
	}
	else if (str == "invert")
	{
		return StencilOp::Invert;
	}
	else if (str == "incrementwrap")
	{
		return StencilOp::IncrementWrap;
	}
	else if (str == "decrementwrap")
	{
		return StencilOp::DecrementWrap;
	}
	else
	{
		return dflt;
	}
}

void addPipelineDepthStencil(effect::Effect&, const StringHash&, effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	//--- Depth
	pipeline.depthCmpFunc = comparisonModeFromString(attribute_element.attribute("depthFunc").as_string(""), CompareOp::DefaultDepthFunc);

	pipeline.enableDepthTest = attribute_element.attribute("depthTest").as_bool(false);
	pipeline.enableDepthWrite = attribute_element.attribute("depthWrite").as_bool(true);

	pipeline.enableStencilTest = attribute_element.attribute("stencilTest").as_bool(true);

	//---- Stencil, check for common
	pipeline.stencilFront.opDepthFail = stencilOpFromString(strings::toLower(attribute_element.attribute("stencilOpDepthFail").as_string("")), StencilOp::Keep);

	pipeline.stencilFront.opDepthPass = stencilOpFromString(strings::toLower(attribute_element.attribute("stencilOpDepthPass").as_string("")), StencilOp::Keep);

	pipeline.stencilFront.opStencilFail = stencilOpFromString(strings::toLower(attribute_element.attribute("stencilOpStencilFail").as_string("")), StencilOp::Keep);

	pipeline.stencilFront.compareMask = attribute_element.attribute("stencilCompareMask").as_uint(0xff);
	pipeline.stencilFront.writeMask = attribute_element.attribute("stencilWriteMask").as_uint(0xff);
	pipeline.stencilFront.reference = attribute_element.attribute("stencilReference").as_uint(0);
	pipeline.stencilFront.compareOp = comparisonModeFromString(attribute_element.attribute("stencilFunc").as_string(""), CompareOp::DefaultStencilFunc);

	pipeline.stencilBack = pipeline.stencilFront;

	//---- Stencil, now check for explicit case, overwrite if necessary
	// stencil front
	pipeline.stencilFront.opDepthFail = stencilOpFromString(strings::toLower(attribute_element.attribute("stencilOpDepthFailFront").as_string("")), pipeline.stencilFront.opDepthFail);

	pipeline.stencilFront.opDepthPass = stencilOpFromString(strings::toLower(attribute_element.attribute("stencilOpDepthPassFront").as_string("")), pipeline.stencilFront.opDepthPass);

	pipeline.stencilFront.opStencilFail =
		stencilOpFromString(strings::toLower(attribute_element.attribute("stencilOpStencilFailFront").as_string("")), pipeline.stencilFront.opStencilFail);

	pipeline.stencilFront.compareMask = attribute_element.attribute("stencilCompareMaskFront").as_uint(pipeline.stencilFront.compareMask);
	pipeline.stencilFront.writeMask = attribute_element.attribute("stencilWriteMaskFront").as_uint(pipeline.stencilFront.writeMask);
	pipeline.stencilFront.reference = attribute_element.attribute("stencilReferenceFront").as_uint(pipeline.stencilFront.reference);
	pipeline.stencilFront.compareOp = comparisonModeFromString(attribute_element.attribute("stencilFunc").as_string(""), pipeline.stencilFront.compareOp);

	// stencil back
	pipeline.stencilBack.opDepthFail = stencilOpFromString(strings::toLower(attribute_element.attribute("stencilOpDepthFailBack").as_string("")), pipeline.stencilBack.opDepthFail);

	pipeline.stencilBack.opDepthPass = stencilOpFromString(strings::toLower(attribute_element.attribute("stencilOpDepthPassBack").as_string("")), pipeline.stencilBack.opDepthPass);

	pipeline.stencilBack.opStencilFail =
		stencilOpFromString(strings::toLower(attribute_element.attribute("stencilOpStencilFailBack").as_string("")), pipeline.stencilBack.opStencilFail);

	pipeline.stencilBack.compareMask = attribute_element.attribute("stencilCompareMaskBack").as_uint(pipeline.stencilBack.compareMask);
	pipeline.stencilBack.writeMask = attribute_element.attribute("stencilWriteMaskBack").as_uint(pipeline.stencilBack.writeMask);
	pipeline.stencilBack.reference = attribute_element.attribute("stencilReferenceBack").as_uint(pipeline.stencilBack.reference);
	pipeline.stencilBack.compareOp = comparisonModeFromString(attribute_element.attribute("stencilFunc").as_string(""), pipeline.stencilBack.compareOp);
}

inline Face faceFromString(const std::string& str, Face defaultFace)
{
	if (str.empty()) { return defaultFace; }
	else if ("none" == str)
	{
		return Face::None;
	}
	else if ("front" == str)
	{
		return Face::Front;
	}
	else if ("back" == str)
	{
		return Face::Back;
	}
	else if ("frontback" == str || "front_and_back" == str || "frontandback" == str)
	{
		return Face::FrontAndBack;
	}
	return defaultFace;
}

inline StepRate stepRateFromString(const char* str, StepRate defaultStepRate)
{
	const std::string str_l(strings::toLower(str));
	if (str_l == "vertex") { return StepRate::Vertex; }
	else if (str_l == "instance")
	{
		return StepRate::Instance;
	}
	return defaultStepRate;
}

inline PolygonWindingOrder polygonWindingOrderFromString(const std::string& str)
{
	if (str == "cw" || str == "clockwise") { return PolygonWindingOrder::FrontFaceCW; }
	else if (str == "ccw" || str == "counterclockwise")
	{
		return PolygonWindingOrder::FrontFaceCCW;
	}
	return PolygonWindingOrder::FrontFaceCCW;
}

void addPipelineRasterization(effect::Effect&, const StringHash&, effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	pipeline.cullFace = faceFromString(attribute_element.attribute("faceCulling").as_string(), Face::Default);
	pipeline.windingOrder = polygonWindingOrderFromString(attribute_element.attribute("frontFaceWinding").as_string("ccw"));
}

void addPipelineVertexInputBinding(effect::Effect&, const StringHash&, effect::PipelineDefinition& pipeline, pugi::xml_node& attribute_element)
{
	pipeline.vertexBinding.emplace_back(
		effect::PipelineVertexBinding(attribute_element.attribute("index").as_uint(), stepRateFromString(attribute_element.attribute("stepRate").as_string(""), StepRate::Vertex)));
}

typedef void (*pfn_add_element)(effect::Effect&, const StringHash&, effect::PipelineDefinition&, pugi::xml_node&);

void addElementsToPipelines(effect::Effect& effect, std::map<StringHash, effect::PipelineDefinition>& pipelines, pugi::xml_node& pipe_element, pfn_add_element adder)
{
	if (pipe_element.attribute("apiVersion"))
	{
		const auto& apiversion = pipe_element.attribute("apiVersion").value();
		adder(effect, apiversion, pipelines[apiversion], pipe_element);
	}
	else
	{
		for (auto versions = pipelines.begin(); versions != pipelines.end(); ++versions) { adder(effect, versions->first, versions->second, pipe_element); }
	}
}

const StringHash empty_str("");
bool processPipeline(effect::Effect& effect, pugi::xml_node& pipe_element, const StringHash& name)
{
	std::map<StringHash, effect::PipelineDefinition> pipelines;
	pipelines[empty_str].name = name;

	for (auto it = pipe_element.children().begin(); it != pipe_element.children().end(); ++it)
	{
		if (it->attribute("apiVersion")) { pipelines[it->attribute("apiVersion").value()].name = name; }
	}

	for (auto it = effect.getVersions().begin(); it != effect.getVersions().end(); ++it) { pipelines[it->c_str()].name = name; }

	// add attributes
	for (auto it = pipe_element.children("attribute").begin(); it != pipe_element.children("attribute").end(); ++it)
	{ addElementsToPipelines(effect, pipelines, *it, &addPipelineAttribute); } // add uniforms
	for (auto it = pipe_element.children("uniform").begin(); it != pipe_element.children("uniform").end(); ++it)
	{ addElementsToPipelines(effect, pipelines, *it, &addPipelineUniform); } // add shaders
	for (auto it = pipe_element.children("shader").begin(); it != pipe_element.children("shader").end(); ++it)
	{ addElementsToPipelines(effect, pipelines, *it, &addPipelineShader); } // add buffers
	for (auto it = pipe_element.children("buffer").begin(); it != pipe_element.children("buffer").end(); ++it)
	{ addElementsToPipelines(effect, pipelines, *it, &addPipelineBuffer); } // add textures
	for (auto it = pipe_element.children("texture").begin(); it != pipe_element.children("texture").end(); ++it)
	{ addElementsToPipelines(effect, pipelines, *it, &addPipelineTexture); } // add input attachments
	for (auto it = pipe_element.children("inputattachment").begin(); it != pipe_element.children("inputattachment").end(); ++it)
	{ addElementsToPipelines(effect, pipelines, *it, &addPipelineInputAttachment); } // add the blending
	for (auto it = pipe_element.children("blending").begin(); it != pipe_element.children("blending").end(); ++it)
	{
		addElementsToPipelines(effect, pipelines, *it, &addPipelineBlending);
	} // add the depth stencil
	  // add a default if depthStencil children not found

	for (auto it = pipe_element.children("depthstencil").begin(); it != pipe_element.children("depthstencil").end(); ++it)
	{
		addElementsToPipelines(effect, pipelines, *it, &addPipelineDepthStencil);
	} // add the raster states
	  // add defaults if rasterization children not found
	if (pipe_element.children("rasterization").begin() == pipe_element.children("rasterization").end())
	{ addElementsToPipelines(effect, pipelines, pipe_element, &addPipelineRasterization); }
	else
	{
		for (auto it = pipe_element.children("rasterization").begin(); it != pipe_element.children("rasterization").end(); ++it)
		{ addElementsToPipelines(effect, pipelines, *it, &addPipelineRasterization); }
	}

	// add the pipeline binding
	for (auto it = pipe_element.children("vbobinding").begin(); it != pipe_element.children("vbobinding").end(); ++it)
	{ addElementsToPipelines(effect, pipelines, *it, &addPipelineVertexInputBinding); }
	for (auto it = pipelines.begin(); it != pipelines.end(); ++it) { effect.versionedPipelines[it->first][it->second.name] = it->second; }
	return true;
}

void addPipelines(effect::Effect& effect, pugi::xml_named_node_iterator begin, pugi::xml_named_node_iterator end)
{
	// Each pipeline element...
	for (auto pipe_element = begin; pipe_element != end; ++pipe_element)
	{
		StringHash pipelineName;

		// Get its name
		for (auto it2 = pipe_element->attributes_begin(); it2 != pipe_element->attributes_end(); ++it2)
		{
			if (std::string(it2->name()) == std::string("name")) { pipelineName = it2->value(); }
		}
		processPipeline(effect, *pipe_element, pipelineName);
	}
}

void addSubpassGroup(effect::SubpassGroup& outGroup, pugi::xml_node& subpassgroup_element)
{
	uint32_t pipe_counter = 0;
	outGroup.name = subpassgroup_element.attribute("name").as_string("");
	outGroup.pipelines.resize(static_cast<uint32_t>(subpassgroup_element.select_nodes("pipeline").size()));
	pipe_counter = 0;
	for (auto pipeline = subpassgroup_element.children("pipeline").begin(); pipeline != subpassgroup_element.children("pipeline").end(); ++pipeline)
	{
		effect::PipelineReference& ref = outGroup.pipelines[pipe_counter++];
		ref.pipelineName = pipeline->attribute("name").value();
		int32_t counter = 0;
		auto condition_begin = pipeline->children("condition").begin();
		auto condition_end = pipeline->children("condition").end();
		// unfortunately no operator "-" exists for those iterators - they are not random access, so we'll traverse twice. No big deal.
		for (auto conditions = condition_begin; conditions != condition_end; ++conditions) { ++counter; }
		ref.conditions.resize(counter);
		counter = 0;
		for (auto condition = pipeline->children("condition").begin(); condition != pipeline->children("condition").end(); ++condition)
		{
			ref.conditions[counter].type = conditionFromAttribute(condition->attribute("type"));
			ref.conditions[counter++].value = condition->attribute("name").value();
		}

		counter = 0;
		auto identifiers_begin = pipeline->children("exportIdentifier").begin();
		auto identifiers_end = pipeline->children("exportIdentifier").end();
		for (auto identifier = identifiers_begin; identifier != identifiers_end; ++identifier) { ++counter; }
		ref.identifiers.resize(static_cast<size_t>(counter));
		counter = 0;
		for (auto identifier = identifiers_begin; identifier != identifiers_end; ++identifier) { ref.identifiers[counter++] = identifier->attribute("name").value(); }
	}
}

void addSubpass(effect::Subpass& outSubpass, pugi::xml_node& subpass_element)
{
	//-----------------------------------------
	// render targets
	outSubpass.targets[0] = StringHash(subpass_element.attribute("target0").as_string("default"));
	outSubpass.targets[1] = StringHash(subpass_element.attribute("target1").value());
	outSubpass.targets[2] = StringHash(subpass_element.attribute("target2").value());
	outSubpass.targets[3] = StringHash(subpass_element.attribute("target3").value());

	//-----------------------------------------
	// inputs
	outSubpass.inputs[0] = StringHash(subpass_element.attribute("input0").value());
	outSubpass.inputs[1] = StringHash(subpass_element.attribute("input1").value());
	outSubpass.inputs[2] = StringHash(subpass_element.attribute("input2").value());
	outSubpass.inputs[3] = StringHash(subpass_element.attribute("input3").value());
	outSubpass.useDepthStencil = subpass_element.attribute("usesDepthStencil").as_bool(true);

	//----
	// if there is no subpassgroup then add one
	// else process all the subgroups
	if (subpass_element.children("subpassgroup").begin() == subpass_element.children("subpassgroup").end())
	{
		outSubpass.groups.resize(1);
		addSubpassGroup(outSubpass.groups[0], subpass_element);
	}
	else
	{
		outSubpass.groups.resize(subpass_element.select_nodes("subpassgroup").size());
		uint32_t groupIndex = 0;
		for (auto walk = subpass_element.children("subpassgroup").begin(); walk != subpass_element.children("subpassgroup").end(); ++walk, ++groupIndex)
		{ addSubpassGroup(outSubpass.groups[groupIndex], *walk); }
	}
}

void addPass(effect::Effect& effect, pugi::xml_node& pass_element, bool& depthStencilCreated)
{
	effect.passes.resize(effect.passes.size() + 1);
	effect::Pass& pass = effect.passes.back();

	pass.name = pass_element.attribute("name").as_string("");
	pass.targetDepthStencil = pass_element.attribute("targetDepthStencil").as_string("");
	if (pass.targetDepthStencil.empty())
	{
		pass.targetDepthStencil = "PfxDefaultDepthBuffer";
		if (!depthStencilCreated) // Create a depth buffer if needed
		{
			effect::TextureDefinition tex;
			tex.name = "PfxDefaultDepthBuffer";
			tex.path = StringHash();
			tex.height = 0;
			tex.width = 0;
			tex.format = ImageDataFormat(PixelFormat::Depth32(), VariableType::UnsignedInteger, ColorSpace::lRGB);
			effect.addTexture(std::move(tex));
			depthStencilCreated = true;
		}
	}
	// do the subpasses
	auto subpass_begin = pass_element.children("subpass").begin();
	auto subpass_end = pass_element.children("subpass").end();
	uint32_t size = static_cast<uint32_t>(pass_element.select_nodes("subpass").size());
	size = (size ? size : 1);
	pass.subpasses.resize(size);

	// if we have no supass then create one.
	if (subpass_begin == subpass_end) { addSubpass(pass.subpasses[0], pass_element); }
	else
	{
		effect::Subpass* subpass = &pass.subpasses[0];
		for (auto it = subpass_begin; it != subpass_end; ++it)
		{
			addSubpass(*subpass, *it);
			++subpass;
		}
	}
}

void addEffects(effect::Effect& effect, pugi::xml_named_node_iterator begin, pugi::xml_named_node_iterator end)
{
	bool depthStencilCreated = false;
	// Each effect element...
	for (auto effect_element = begin; effect_element != end; ++effect_element)
	{
		// Get its name
		for (auto it2 = effect_element->attributes_begin(); it2 != effect_element->attributes_end(); ++it2)
		{
			if (std::string(it2->name()) == std::string("name")) { effect.name = it2->value(); }
		}

		auto pass_begin = effect_element->children("pass").begin();
		auto pass_end = effect_element->children("pass").end();
		if (pass_begin == pass_end) // If there is only one pass, it is allowed skip the "pass" elements and put the rest straight into the pass.
		{ addPass(effect, *effect_element, depthStencilCreated); }
		else
		{
			for (auto pass = pass_begin; pass != pass_end; ++pass) { addPass(effect, *pass, depthStencilCreated); }
		}
	}
}

void findVersions(effect::Effect& effect, std::set<StringHash>& apiversions, pugi::xml_node root)
{
	for (auto it = root.children().begin(); it != root.children().end(); ++it)
	{
		if (it->attribute("apiVersion")) { apiversions.insert(it->attribute("apiVersion").value()); }
		findVersions(effect, apiversions, *it);
	}
}

void addVersions(effect::Effect& effect, pugi::xml_node root)
{
	std::set<StringHash> apiversions;
	apiversions.insert("");

	findVersions(effect, apiversions, root);

	for (auto it = apiversions.begin(); it != apiversions.end(); ++it) { effect.addVersion(*it); }
}

} // namespace

effect::Effect readPFX(const ::pvr::Stream& stream, const ::pvr::IAssetProvider* assetProvider)
{
	effect::Effect asset;
	readPFX(stream, assetProvider, asset);
	return asset;
}

void readPFX(const ::pvr::Stream& stream, const ::pvr::IAssetProvider* assetProvider, effect::Effect& asset)
{
	std::vector<char> v = stream.readToEnd<char>();

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_buffer_inplace(v.data(), v.size());

	if (result.status != pugi::xml_parse_status::status_ok || !doc || !doc.root()) { throw InvalidDataError("[PfxParser::readAsset_]: Failed to parse PFX file - not valid XML"); }
	if (!doc.root().first_child() || std::string(doc.root().first_child().name()) != std::string("pfx"))
	{ throw InvalidDataError("[PfxParser::readAsset_]: Failed to parse PFX file - root <pfx> element not found"); }
	const auto& root = doc.root().first_child();
	auto textures = root.children("texture");
	auto shaders = root.children("shader");
	auto buffers = root.children("buffer");
	auto pipelines = root.children("pipeline");
	auto effects = root.children("effect");

	//*** Load header attributes ***//
	for (auto it = root.attributes_begin(); it != root.attributes_end(); ++it) { asset.headerAttributes[it->name()] = it->value(); }

	//*** Load Textures ***//
	addVersions(asset, root); // Pre-process a list of all different version flavors.
	addTextures(asset, textures.begin(), textures.end());
	addBuffers(asset, buffers.begin(), buffers.end());
	addShaders(asset, shaders.begin(), shaders.end(), assetProvider);
	addPipelines(asset, pipelines.begin(), pipelines.end());
	addEffects(asset, effects.begin(), effects.end());
}

} // namespace pfx

} // namespace pvr
//!\endcond
