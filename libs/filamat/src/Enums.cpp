/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "filamat/Enums.h"

#include "filamat/MaterialBuilder.h"

namespace filamat {

std::unordered_map<std::string, Property> Enums::mStringToProperty = {
        { "baseColor",           Property::BASE_COLOR },
        { "roughness",           Property::ROUGHNESS },
        { "metallic",            Property::METALLIC },
        { "reflectance",         Property::REFLECTANCE },
        { "ambientOcclusion",    Property::AMBIENT_OCCLUSION },
        { "clearCoat",           Property::CLEAR_COAT },
        { "clearCoatRoughness",  Property::CLEAR_COAT_ROUGHNESS },
        { "clearCoatNormal",     Property::CLEAR_COAT_NORMAL },
        { "anisotropy",          Property::ANISOTROPY },
        { "anisotropyDirection", Property::ANISOTROPY_DIRECTION },
        { "thickness",           Property::THICKNESS },
        { "subsurfacePower",     Property::SUBSURFACE_POWER },
        { "subsurfaceColor",     Property::SUBSURFACE_COLOR },
        { "sheenColor",          Property::SHEEN_COLOR },
        { "glossiness",          Property::GLOSSINESS },
        { "specularColor",       Property::SPECULAR_COLOR },
        { "emissive",            Property::EMISSIVE },
        { "normal",              Property::NORMAL },
        { "postLightingColor",   Property::POST_LIGHTING_COLOR },
};

template <>
std::unordered_map<std::string, Property>& Enums::getMap<Property>() noexcept {
    return mStringToProperty;
};

std::unordered_map<std::string, UniformType> Enums::mStringToUniformType = {
        { "bool",     UniformType::BOOL },
        { "bool2",    UniformType::BOOL2 },
        { "bool3",    UniformType::BOOL3 },
        { "bool4",    UniformType::BOOL4 },
        { "float",    UniformType::FLOAT },
        { "float2",   UniformType::FLOAT2 },
        { "float3",   UniformType::FLOAT3 },
        { "float4",   UniformType::FLOAT4 },
        { "int",      UniformType::INT },
        { "int2",     UniformType::INT2 },
        { "int3",     UniformType::INT3 },
        { "int4",     UniformType::INT4 },
        { "uint",     UniformType::UINT },
        { "uint2",    UniformType::UINT2 },
        { "uint3",    UniformType::UINT3 },
        { "uint4",    UniformType::UINT4 },
        { "mat3",     UniformType::MAT3 },
        { "mat4",     UniformType::MAT4 },
        { "float3x3", UniformType::MAT3 },
        { "float4x4", UniformType::MAT4 }
};

template <>
std::unordered_map<std::string, UniformType>& Enums::getMap<UniformType>() noexcept {
    return mStringToUniformType;
};

std::unordered_map<std::string, SamplerType> Enums::mStringToSamplerType = {
        { "sampler2d",       SamplerType::SAMPLER_2D },
        { "samplerCubemap",  SamplerType::SAMPLER_CUBEMAP },
        { "samplerExternal", SamplerType::SAMPLER_EXTERNAL },
};

template <>
std::unordered_map<std::string, SamplerType>& Enums::getMap<SamplerType>() noexcept {
    return mStringToSamplerType;
};

std::unordered_map<std::string, SamplerPrecision> Enums::mStringToSamplerPrecision = {
        { "default", SamplerPrecision::DEFAULT },
        { "low",     SamplerPrecision::LOW },
        { "medium",  SamplerPrecision::MEDIUM },
        { "high",    SamplerPrecision::HIGH },
};

template <>
std::unordered_map<std::string, SamplerPrecision>& Enums::getMap<SamplerPrecision>() noexcept {
    return mStringToSamplerPrecision;
};

std::unordered_map<std::string, SamplerFormat> Enums::mStringToSamplerFormat = {
        { "int",    SamplerFormat::INT },
        { "uint",   SamplerFormat::UINT },
        { "float",  SamplerFormat::FLOAT },
        { "shadow", SamplerFormat::SHADOW },
};

template <>
std::unordered_map<std::string, SamplerFormat>& Enums::getMap<SamplerFormat>() noexcept {
    return mStringToSamplerFormat;
};

} // namespace filamat
