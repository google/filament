//
// Copyright 2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/util.h"

#include <limits>

#include "common/span.h"
#include "common/utilities.h"
#include "compiler/preprocessor/numeric_lex.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/SymbolTable.h"

bool atoi_clamp(const char *str, unsigned int *value)
{
    bool success = angle::pp::numeric_lex_int(str, value);
    if (!success)
        *value = std::numeric_limits<unsigned int>::max();
    return success;
}

namespace sh
{

namespace
{
// [primarySize-1][secondarySize-1] is the GL type with a basic type of float.
constexpr GLenum kFloatGLType[4][4] = {
    // float1xS only makes sense for S == 1
    {
        GL_FLOAT,
        GL_NONE,
        GL_NONE,
        GL_NONE,
    },
    // float2xS is vec2 for S == 1, and mat2xS o.w.
    {
        GL_FLOAT_VEC2,
        GL_FLOAT_MAT2,
        GL_FLOAT_MAT2x3,
        GL_FLOAT_MAT2x4,
    },
    // float3xS is vec3 for S == 1, and mat3xS o.w.
    {
        GL_FLOAT_VEC3,
        GL_FLOAT_MAT3x2,
        GL_FLOAT_MAT3,
        GL_FLOAT_MAT3x4,
    },
    // float4xS is vec4 for S == 1, and mat4xS o.w.
    {
        GL_FLOAT_VEC4,
        GL_FLOAT_MAT4x2,
        GL_FLOAT_MAT4x3,
        GL_FLOAT_MAT4,
    },
};
// [primarySize-1] is the GL type with a basic type of int.
constexpr GLenum kIntGLType[4] = {GL_INT, GL_INT_VEC2, GL_INT_VEC3, GL_INT_VEC4};
// [primarySize-1] is the GL type with a basic type of uint.
constexpr GLenum kUIntGLType[4] = {GL_UNSIGNED_INT, GL_UNSIGNED_INT_VEC2, GL_UNSIGNED_INT_VEC3,
                                   GL_UNSIGNED_INT_VEC4};
// [primarySize-1] is the GL type with a basic type of bool.
constexpr GLenum kBoolGLType[4] = {GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3, GL_BOOL_VEC4};

bool IsInterpolationIn(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqSmoothIn:
        case EvqFlatIn:
        case EvqNoPerspectiveIn:
        case EvqCentroidIn:
        case EvqSampleIn:
        case EvqNoPerspectiveCentroidIn:
        case EvqNoPerspectiveSampleIn:
            return true;
        default:
            return false;
    }
}

bool IsInterpolationOut(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqSmoothOut:
        case EvqFlatOut:
        case EvqNoPerspectiveOut:
        case EvqCentroidOut:
        case EvqSampleOut:
        case EvqNoPerspectiveCentroidOut:
        case EvqNoPerspectiveSampleOut:
            return true;
        default:
            return false;
    }
}
}  // anonymous namespace

float NumericLexFloat32OutOfRangeToInfinity(const std::string &str)
{
    // Parses a decimal string using scientific notation into a floating point number.
    // Out-of-range values are converted to infinity. Values that are too small to be
    // represented are converted to zero.

    // The mantissa in decimal scientific notation. The magnitude of the mantissa integer does not
    // matter.
    unsigned int decimalMantissa = 0;
    size_t i                     = 0;
    bool decimalPointSeen        = false;
    bool nonZeroSeenInMantissa   = false;

    // The exponent offset reflects the position of the decimal point.
    int exponentOffset = -1;

    // This is just a counter for how many decimal digits are written to decimalMantissa.
    int mantissaDecimalDigits = 0;

    while (i < str.length())
    {
        const char c = str[i];
        if (c == 'e' || c == 'E')
        {
            break;
        }
        if (c == '.')
        {
            decimalPointSeen = true;
            ++i;
            continue;
        }

        unsigned int digit = static_cast<unsigned int>(c - '0');
        ASSERT(digit < 10u);
        if (digit != 0u)
        {
            nonZeroSeenInMantissa = true;
        }
        if (nonZeroSeenInMantissa)
        {
            // Add bits to the mantissa until space runs out in 32-bit int. This should be
            // enough precision to make the resulting binary mantissa accurate to 1 ULP.
            if (decimalMantissa <= (std::numeric_limits<unsigned int>::max() - 9u) / 10u)
            {
                decimalMantissa = decimalMantissa * 10u + digit;
                ++mantissaDecimalDigits;
            }
            if (!decimalPointSeen)
            {
                ++exponentOffset;
            }
        }
        else if (decimalPointSeen)
        {
            --exponentOffset;
        }
        ++i;
    }
    if (decimalMantissa == 0)
    {
        return 0.0f;
    }
    int exponent = 0;
    if (i < str.length())
    {
        ASSERT(str[i] == 'e' || str[i] == 'E');
        ++i;
        bool exponentOutOfRange = false;
        bool negativeExponent   = false;
        if (str[i] == '-')
        {
            negativeExponent = true;
            ++i;
        }
        else if (str[i] == '+')
        {
            ++i;
        }
        while (i < str.length())
        {
            const char c       = str[i];
            unsigned int digit = static_cast<unsigned int>(c - '0');
            ASSERT(digit < 10u);
            if (exponent <= (std::numeric_limits<int>::max() - 9) / 10)
            {
                exponent = exponent * 10 + digit;
            }
            else
            {
                exponentOutOfRange = true;
            }
            ++i;
        }
        if (negativeExponent)
        {
            exponent = -exponent;
        }
        if (exponentOutOfRange)
        {
            if (negativeExponent)
            {
                return 0.0f;
            }
            else
            {
                return std::numeric_limits<float>::infinity();
            }
        }
    }
    // Do the calculation in 64-bit to avoid overflow.
    long long exponentLong =
        static_cast<long long>(exponent) + static_cast<long long>(exponentOffset);
    if (exponentLong > std::numeric_limits<float>::max_exponent10)
    {
        return std::numeric_limits<float>::infinity();
    }
    // In 32-bit float, min_exponent10 is -37 but min() is
    // 1.1754943E-38. 10^-37 may be the "minimum negative integer such
    // that 10 raised to that power is a normalized float", but being
    // constrained to powers of ten it's above min() (which is 2^-126).
    // Values below min() are flushed to zero near the end of this
    // function anyway so (AFAICT) this comparison is only done to ensure
    // that the exponent will not make the pow() call (below) overflow.
    // Comparing against -38 (min_exponent10 - 1) will do the trick.
    else if (exponentLong < std::numeric_limits<float>::min_exponent10 - 1)
    {
        return 0.0f;
    }
    // The exponent is in range, so we need to actually evaluate the float.
    exponent     = static_cast<int>(exponentLong);
    double value = decimalMantissa;

    // Calculate the exponent offset to normalize the mantissa.
    int normalizationExponentOffset = 1 - mantissaDecimalDigits;
    // Apply the exponent.
    value *= std::pow(10.0, static_cast<double>(exponent + normalizationExponentOffset));
    if (value > static_cast<double>(std::numeric_limits<float>::max()))
    {
        return std::numeric_limits<float>::infinity();
    }
    if (static_cast<float>(value) < std::numeric_limits<float>::min())
    {
        return 0.0f;
    }
    return static_cast<float>(value);
}

bool strtof_clamp(const std::string &str, float *value)
{
    // Custom float parsing that can handle the following corner cases:
    //   1. The decimal mantissa is very small but the exponent is very large, putting the resulting
    //   number inside the float range.
    //   2. The decimal mantissa is very large but the exponent is very small, putting the resulting
    //   number inside the float range.
    //   3. The value is out-of-range and should be evaluated as infinity.
    //   4. The value is too small and should be evaluated as zero.
    // See ESSL 3.00.6 section 4.1.4 for the relevant specification.
    *value = NumericLexFloat32OutOfRangeToInfinity(str);
    return !gl::isInf(*value);
}

GLenum GLVariableType(const TType &type)
{
    switch (type.getBasicType())
    {
        case EbtFloat:
            ASSERT(type.getNominalSize() >= 1 && type.getNominalSize() <= 4);
            ASSERT(type.getSecondarySize() >= 1 && type.getSecondarySize() <= 4);

            return kFloatGLType[type.getNominalSize() - 1][type.getSecondarySize() - 1];

        case EbtInt:
            ASSERT(type.getNominalSize() >= 1 && type.getNominalSize() <= 4);
            ASSERT(type.getSecondarySize() == 1);

            return kIntGLType[type.getNominalSize() - 1];

        case EbtUInt:
            ASSERT(type.getNominalSize() >= 1 && type.getNominalSize() <= 4);
            ASSERT(type.getSecondarySize() == 1);

            return kUIntGLType[type.getNominalSize() - 1];

        case EbtBool:
            ASSERT(type.getNominalSize() >= 1 && type.getNominalSize() <= 4);
            ASSERT(type.getSecondarySize() == 1);

            return kBoolGLType[type.getNominalSize() - 1];

        case EbtYuvCscStandardEXT:
            return GL_UNSIGNED_INT;

        case EbtSampler2D:
            return GL_SAMPLER_2D;
        case EbtSampler3D:
            return GL_SAMPLER_3D;
        case EbtSamplerCube:
            return GL_SAMPLER_CUBE;
        case EbtSamplerExternalOES:
            return GL_SAMPLER_EXTERNAL_OES;
        case EbtSamplerExternal2DY2YEXT:
            return GL_SAMPLER_EXTERNAL_2D_Y2Y_EXT;
        case EbtSampler2DRect:
            return GL_SAMPLER_2D_RECT_ANGLE;
        case EbtSampler2DArray:
            return GL_SAMPLER_2D_ARRAY;
        case EbtSampler2DMS:
            return GL_SAMPLER_2D_MULTISAMPLE;
        case EbtSampler2DMSArray:
            return GL_SAMPLER_2D_MULTISAMPLE_ARRAY;
        case EbtSamplerCubeArray:
            return GL_SAMPLER_CUBE_MAP_ARRAY;
        case EbtSamplerBuffer:
            return GL_SAMPLER_BUFFER;
        case EbtISampler2D:
            return GL_INT_SAMPLER_2D;
        case EbtISampler3D:
            return GL_INT_SAMPLER_3D;
        case EbtISamplerCube:
            return GL_INT_SAMPLER_CUBE;
        case EbtISampler2DArray:
            return GL_INT_SAMPLER_2D_ARRAY;
        case EbtISampler2DMS:
            return GL_INT_SAMPLER_2D_MULTISAMPLE;
        case EbtISampler2DMSArray:
            return GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY;
        case EbtISamplerCubeArray:
            return GL_INT_SAMPLER_CUBE_MAP_ARRAY;
        case EbtISamplerBuffer:
            return GL_INT_SAMPLER_BUFFER;
        case EbtUSampler2D:
            return GL_UNSIGNED_INT_SAMPLER_2D;
        case EbtUSampler3D:
            return GL_UNSIGNED_INT_SAMPLER_3D;
        case EbtUSamplerCube:
            return GL_UNSIGNED_INT_SAMPLER_CUBE;
        case EbtUSampler2DArray:
            return GL_UNSIGNED_INT_SAMPLER_2D_ARRAY;
        case EbtUSampler2DMS:
            return GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE;
        case EbtUSampler2DMSArray:
            return GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY;
        case EbtUSamplerCubeArray:
            return GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY;
        case EbtUSamplerBuffer:
            return GL_UNSIGNED_INT_SAMPLER_BUFFER;
        case EbtSampler2DShadow:
            return GL_SAMPLER_2D_SHADOW;
        case EbtSamplerCubeShadow:
            return GL_SAMPLER_CUBE_SHADOW;
        case EbtSampler2DArrayShadow:
            return GL_SAMPLER_2D_ARRAY_SHADOW;
        case EbtSamplerCubeArrayShadow:
            return GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW;
        case EbtImage2D:
            return GL_IMAGE_2D;
        case EbtIImage2D:
            return GL_INT_IMAGE_2D;
        case EbtUImage2D:
            return GL_UNSIGNED_INT_IMAGE_2D;
        case EbtImage2DArray:
            return GL_IMAGE_2D_ARRAY;
        case EbtIImage2DArray:
            return GL_INT_IMAGE_2D_ARRAY;
        case EbtUImage2DArray:
            return GL_UNSIGNED_INT_IMAGE_2D_ARRAY;
        case EbtImage3D:
            return GL_IMAGE_3D;
        case EbtIImage3D:
            return GL_INT_IMAGE_3D;
        case EbtUImage3D:
            return GL_UNSIGNED_INT_IMAGE_3D;
        case EbtImageCube:
            return GL_IMAGE_CUBE;
        case EbtIImageCube:
            return GL_INT_IMAGE_CUBE;
        case EbtUImageCube:
            return GL_UNSIGNED_INT_IMAGE_CUBE;
        case EbtImageCubeArray:
            return GL_IMAGE_CUBE_MAP_ARRAY;
        case EbtIImageCubeArray:
            return GL_INT_IMAGE_CUBE_MAP_ARRAY;
        case EbtUImageCubeArray:
            return GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY;
        case EbtImageBuffer:
            return GL_IMAGE_BUFFER;
        case EbtIImageBuffer:
            return GL_INT_IMAGE_BUFFER;
        case EbtUImageBuffer:
            return GL_UNSIGNED_INT_IMAGE_BUFFER;
        case EbtAtomicCounter:
            return GL_UNSIGNED_INT_ATOMIC_COUNTER;
        case EbtSamplerVideoWEBGL:
            return GL_SAMPLER_VIDEO_IMAGE_WEBGL;
        case EbtPixelLocalANGLE:
        case EbtIPixelLocalANGLE:
        case EbtUPixelLocalANGLE:
            // TODO(anglebug.com/40096838): For now, we can expect PLS handles to be rewritten to
            // images before anyone calls into here.
            [[fallthrough]];
        default:
            UNREACHABLE();
            return GL_NONE;
    }
}

GLenum GLVariablePrecision(const TType &type)
{
    if (type.getBasicType() == EbtFloat)
    {
        switch (type.getPrecision())
        {
            case EbpHigh:
                return GL_HIGH_FLOAT;
            case EbpMedium:
                return GL_MEDIUM_FLOAT;
            case EbpLow:
                return GL_LOW_FLOAT;
            default:
                UNREACHABLE();
        }
    }
    else if (type.getBasicType() == EbtInt || type.getBasicType() == EbtUInt)
    {
        switch (type.getPrecision())
        {
            case EbpHigh:
                return GL_HIGH_INT;
            case EbpMedium:
                return GL_MEDIUM_INT;
            case EbpLow:
                return GL_LOW_INT;
            default:
                UNREACHABLE();
        }
    }

    // Other types (boolean, sampler) don't have a precision
    return GL_NONE;
}

ImmutableString ArrayString(const TType &type)
{
    if (!type.isArray())
        return ImmutableString("");

    const angle::Span<const unsigned int> &arraySizes = type.getArraySizes();
    constexpr const size_t kMaxDecimalDigitsPerSize = 10u;
    ImmutableStringBuilder arrayString(arraySizes.size() * (kMaxDecimalDigitsPerSize + 2u));
    for (auto arraySizeIter = arraySizes.rbegin(); arraySizeIter != arraySizes.rend();
         ++arraySizeIter)
    {
        arrayString << "[";
        if (*arraySizeIter > 0)
        {
            arrayString << *arraySizeIter;
        }
        arrayString << "]";
    }
    return arrayString;
}

ImmutableString GetTypeName(const TType &type, ShHashFunction64 hashFunction, NameMap *nameMap)
{
    if (type.getBasicType() == EbtStruct)
        return HashName(type.getStruct(), hashFunction, nameMap);
    else
        return ImmutableString(type.getBuiltInTypeNameString());
}

bool IsVaryingOut(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqVaryingOut:
        case EvqVertexOut:
        case EvqGeometryOut:
        case EvqTessControlOut:
        case EvqTessEvaluationOut:
        case EvqPatchOut:
            return true;

        default:
            break;
    }

    return IsInterpolationOut(qualifier);
}

bool IsVaryingIn(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqVaryingIn:
        case EvqFragmentIn:
        case EvqGeometryIn:
        case EvqTessControlIn:
        case EvqTessEvaluationIn:
        case EvqPatchIn:
            return true;

        default:
            break;
    }

    return IsInterpolationIn(qualifier);
}

bool IsVarying(TQualifier qualifier)
{
    return IsVaryingIn(qualifier) || IsVaryingOut(qualifier);
}

bool IsMatrixGLType(GLenum type)
{
    switch (type)
    {
        case GL_FLOAT_MAT2:
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT4:
        case GL_FLOAT_MAT2x3:
        case GL_FLOAT_MAT2x4:
        case GL_FLOAT_MAT3x2:
        case GL_FLOAT_MAT3x4:
        case GL_FLOAT_MAT4x2:
        case GL_FLOAT_MAT4x3:
            return true;
        default:
            return false;
    }
}

bool IsGeometryShaderInput(GLenum shaderType, TQualifier qualifier)
{
    return (qualifier == EvqGeometryIn) ||
           ((shaderType == GL_GEOMETRY_SHADER_EXT) && IsInterpolationIn(qualifier));
}

bool IsTessellationControlShaderInput(GLenum shaderType, TQualifier qualifier)
{
    return qualifier == EvqTessControlIn ||
           ((shaderType == GL_TESS_CONTROL_SHADER) && IsInterpolationIn(qualifier));
}

bool IsTessellationControlShaderOutput(GLenum shaderType, TQualifier qualifier)
{
    return qualifier == EvqTessControlOut ||
           ((shaderType == GL_TESS_CONTROL_SHADER) && IsInterpolationOut(qualifier));
}

bool IsTessellationEvaluationShaderInput(GLenum shaderType, TQualifier qualifier)
{
    return qualifier == EvqTessEvaluationIn ||
           ((shaderType == GL_TESS_EVALUATION_SHADER) && IsInterpolationIn(qualifier));
}

InterpolationType GetInterpolationType(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqFlatIn:
        case EvqFlatOut:
        // The auxiliary storage qualifier patch is not used for interpolation
        // it is a compile-time error to use interpolation qualifiers with patch
        case EvqPatchIn:
        case EvqPatchOut:
            return INTERPOLATION_FLAT;

        case EvqNoPerspectiveIn:
        case EvqNoPerspectiveOut:
            return INTERPOLATION_NOPERSPECTIVE;

        case EvqNoPerspectiveCentroidIn:
        case EvqNoPerspectiveCentroidOut:
            return INTERPOLATION_NOPERSPECTIVE_CENTROID;

        case EvqNoPerspectiveSampleIn:
        case EvqNoPerspectiveSampleOut:
            return INTERPOLATION_NOPERSPECTIVE_SAMPLE;

        case EvqSmoothIn:
        case EvqSmoothOut:
        case EvqVertexOut:
        case EvqFragmentIn:
        case EvqVaryingIn:
        case EvqVaryingOut:
        case EvqGeometryIn:
        case EvqGeometryOut:
        case EvqTessControlIn:
        case EvqTessControlOut:
        case EvqTessEvaluationIn:
        case EvqTessEvaluationOut:
            return INTERPOLATION_SMOOTH;

        case EvqCentroidIn:
        case EvqCentroidOut:
            return INTERPOLATION_CENTROID;

        case EvqSampleIn:
        case EvqSampleOut:
            return INTERPOLATION_SAMPLE;
        default:
            UNREACHABLE();
            return INTERPOLATION_SMOOTH;
    }
}

// a field may not have qualifer without in or out.
InterpolationType GetFieldInterpolationType(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqSmooth:
            return INTERPOLATION_SMOOTH;
        case EvqFlat:
            return INTERPOLATION_FLAT;
        case EvqNoPerspective:
            return INTERPOLATION_NOPERSPECTIVE;
        case EvqCentroid:
            return INTERPOLATION_CENTROID;
        case EvqSample:
            return INTERPOLATION_SAMPLE;
        case EvqNoPerspectiveCentroid:
            return INTERPOLATION_NOPERSPECTIVE_CENTROID;
        case EvqNoPerspectiveSample:
            return INTERPOLATION_NOPERSPECTIVE_SAMPLE;
        default:
            return GetInterpolationType(qualifier);
    }
}

TType GetShaderVariableBasicType(const sh::ShaderVariable &var)
{
    switch (var.type)
    {
        case GL_BOOL:
            return TType(EbtBool);
        case GL_BOOL_VEC2:
            return TType(EbtBool, 2);
        case GL_BOOL_VEC3:
            return TType(EbtBool, 3);
        case GL_BOOL_VEC4:
            return TType(EbtBool, 4);
        case GL_FLOAT:
            return TType(EbtFloat);
        case GL_FLOAT_VEC2:
            return TType(EbtFloat, 2);
        case GL_FLOAT_VEC3:
            return TType(EbtFloat, 3);
        case GL_FLOAT_VEC4:
            return TType(EbtFloat, 4);
        case GL_FLOAT_MAT2:
            return TType(EbtFloat, 2, 2);
        case GL_FLOAT_MAT3:
            return TType(EbtFloat, 3, 3);
        case GL_FLOAT_MAT4:
            return TType(EbtFloat, 4, 4);
        case GL_FLOAT_MAT2x3:
            return TType(EbtFloat, 2, 3);
        case GL_FLOAT_MAT2x4:
            return TType(EbtFloat, 2, 4);
        case GL_FLOAT_MAT3x2:
            return TType(EbtFloat, 3, 2);
        case GL_FLOAT_MAT3x4:
            return TType(EbtFloat, 3, 4);
        case GL_FLOAT_MAT4x2:
            return TType(EbtFloat, 4, 2);
        case GL_FLOAT_MAT4x3:
            return TType(EbtFloat, 4, 3);
        case GL_INT:
            return TType(EbtInt);
        case GL_INT_VEC2:
            return TType(EbtInt, 2);
        case GL_INT_VEC3:
            return TType(EbtInt, 3);
        case GL_INT_VEC4:
            return TType(EbtInt, 4);
        case GL_UNSIGNED_INT:
            return TType(EbtUInt);
        case GL_UNSIGNED_INT_VEC2:
            return TType(EbtUInt, 2);
        case GL_UNSIGNED_INT_VEC3:
            return TType(EbtUInt, 3);
        case GL_UNSIGNED_INT_VEC4:
            return TType(EbtUInt, 4);
        default:
            UNREACHABLE();
            return TType();
    }
}

void DeclareGlobalVariable(TIntermBlock *root, const TVariable *variable)
{
    TIntermDeclaration *declaration = new TIntermDeclaration();
    declaration->appendDeclarator(new TIntermSymbol(variable));

    TIntermSequence *globalSequence = root->getSequence();
    globalSequence->insert(globalSequence->begin(), declaration);
}

// GLSL ES 1.0.17 4.6.1 The Invariant Qualifier
bool CanBeInvariantESSL1(TQualifier qualifier)
{
    return IsVaryingIn(qualifier) || IsVaryingOut(qualifier) ||
           IsBuiltinOutputVariable(qualifier) ||
           (IsBuiltinFragmentInputVariable(qualifier) && qualifier != EvqFrontFacing);
}

// GLSL ES 3.00 Revision 6, 4.6.1 The Invariant Qualifier
// GLSL ES 3.10 Revision 4, 4.8.1 The Invariant Qualifier
bool CanBeInvariantESSL3OrGreater(TQualifier qualifier)
{
    return IsVaryingOut(qualifier) || qualifier == EvqFragmentOut ||
           IsBuiltinOutputVariable(qualifier) || qualifier == EvqFragmentInOut;
}

bool IsBuiltinOutputVariable(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqPosition:
        case EvqPointSize:
        case EvqFragDepth:
        case EvqFragColor:
        case EvqSecondaryFragColorEXT:
        case EvqFragData:
        case EvqSecondaryFragDataEXT:
        case EvqClipDistance:
        case EvqCullDistance:
        case EvqLastFragData:
        case EvqLastFragColor:
        case EvqSampleMask:
            return true;
        default:
            break;
    }
    return false;
}

bool IsBuiltinFragmentInputVariable(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqFragCoord:
        case EvqPointCoord:
        case EvqFrontFacing:
        case EvqHelperInvocation:
        case EvqLastFragData:
        case EvqLastFragColor:
        case EvqLastFragDepth:
        case EvqLastFragStencil:
            return true;
        default:
            break;
    }
    return false;
}

bool IsShaderOutput(TQualifier qualifier)
{
    return IsVaryingOut(qualifier) || IsBuiltinOutputVariable(qualifier);
}

bool IsFragmentOutput(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqFragmentOut:
        case EvqFragmentInOut:
            return true;
        default:
            return false;
    }
}

bool IsOutputNULL(ShShaderOutput output)
{
    return output == SH_NULL_OUTPUT;
}

bool IsOutputESSL(ShShaderOutput output)
{
    return output == SH_ESSL_OUTPUT;
}

bool IsOutputGLSL(ShShaderOutput output)
{
    switch (output)
    {
        case SH_GLSL_130_OUTPUT:
        case SH_GLSL_140_OUTPUT:
        case SH_GLSL_150_CORE_OUTPUT:
        case SH_GLSL_330_CORE_OUTPUT:
        case SH_GLSL_400_CORE_OUTPUT:
        case SH_GLSL_410_CORE_OUTPUT:
        case SH_GLSL_420_CORE_OUTPUT:
        case SH_GLSL_430_CORE_OUTPUT:
        case SH_GLSL_440_CORE_OUTPUT:
        case SH_GLSL_450_CORE_OUTPUT:
        case SH_GLSL_COMPATIBILITY_OUTPUT:
            return true;
        default:
            break;
    }
    return false;
}
bool IsOutputHLSL(ShShaderOutput output)
{
    switch (output)
    {
        case SH_HLSL_3_0_OUTPUT:
        case SH_HLSL_4_1_OUTPUT:
            return true;
        default:
            break;
    }
    return false;
}
bool IsOutputSPIRV(ShShaderOutput output)
{
    return output == SH_SPIRV_VULKAN_OUTPUT;
}
bool IsOutputMSL(ShShaderOutput output)
{
    return output == SH_MSL_METAL_OUTPUT;
}
bool IsOutputWGSL(ShShaderOutput output)
{
    return output == SH_WGSL_OUTPUT;
}

bool IsInShaderStorageBlock(TIntermTyped *node)
{
    TIntermSwizzle *swizzleNode = node->getAsSwizzleNode();
    if (swizzleNode)
    {
        return IsInShaderStorageBlock(swizzleNode->getOperand());
    }

    TIntermBinary *binaryNode = node->getAsBinaryNode();
    if (binaryNode)
    {
        switch (binaryNode->getOp())
        {
            case EOpIndexDirectInterfaceBlock:
            case EOpIndexIndirect:
            case EOpIndexDirect:
            case EOpIndexDirectStruct:
                return IsInShaderStorageBlock(binaryNode->getLeft());
            default:
                return false;
        }
    }

    const TType &type = node->getType();
    return type.getQualifier() == EvqBuffer;
}

GLenum GetImageInternalFormatType(TLayoutImageInternalFormat iifq)
{
    switch (iifq)
    {
        case EiifRGBA32F:
            return GL_RGBA32F;
        case EiifRGBA16F:
            return GL_RGBA16F;
        case EiifR32F:
            return GL_R32F;
        case EiifRGBA32UI:
            return GL_RGBA32UI;
        case EiifRGBA16UI:
            return GL_RGBA16UI;
        case EiifRGBA8UI:
            return GL_RGBA8UI;
        case EiifR32UI:
            return GL_R32UI;
        case EiifRGBA32I:
            return GL_RGBA32I;
        case EiifRGBA16I:
            return GL_RGBA16I;
        case EiifRGBA8I:
            return GL_RGBA8I;
        case EiifR32I:
            return GL_R32I;
        case EiifRGBA8:
            return GL_RGBA8;
        case EiifRGBA8_SNORM:
            return GL_RGBA8_SNORM;
        default:
            return GL_NONE;
    }
}

bool IsSpecWithFunctionBodyNewScope(ShShaderSpec shaderSpec, int shaderVersion)
{
    return (shaderVersion == 100 && !sh::IsWebGLBasedSpec(shaderSpec));
}

bool IsPrecisionApplicableToType(TBasicType type)
{
    switch (type)
    {
        case EbtInt:
        case EbtUInt:
        case EbtFloat:
            // TODO: find all types where precision is applicable; for example samplers.
            // http://anglebug.com/42264661
            return true;
        default:
            return false;
    }
}

bool IsRedeclarableBuiltIn(const ImmutableString &name)
{
    return name == "gl_ClipDistance" || name == "gl_CullDistance" || name == "gl_FragDepth" ||
           name == "gl_LastFragData" || name == "gl_LastFragColorARM" ||
           name == "gl_LastFragDepthARM" || name == "gl_LastFragStencilARM" ||
           name == "gl_PerVertex" || name == "gl_Position" || name == "gl_PointSize";
}

size_t FindFieldIndex(const TFieldList &fieldList, const char *fieldName)
{
    for (size_t fieldIndex = 0; fieldIndex < fieldList.size(); ++fieldIndex)
    {
        if (strcmp(fieldList[fieldIndex]->name().data(), fieldName) == 0)
        {
            return fieldIndex;
        }
    }
    UNREACHABLE();
    return 0;
}

Declaration ViewDeclaration(TIntermDeclaration &declNode, uint32_t index)
{
    ASSERT(declNode.getChildCount() > index);
    TIntermNode *childNode = declNode.getChildNode(index);
    ASSERT(childNode);
    TIntermSymbol *symbolNode;
    if ((symbolNode = childNode->getAsSymbolNode()))
    {
        return {*symbolNode, nullptr};
    }
    else
    {
        TIntermBinary *initNode = childNode->getAsBinaryNode();
        ASSERT(initNode);
        ASSERT(initNode->getOp() == TOperator::EOpInitialize);
        symbolNode = initNode->getLeft()->getAsSymbolNode();
        ASSERT(symbolNode);
        return {*symbolNode, initNode->getRight()};
    }
}

}  // namespace sh
