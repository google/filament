//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <cctype>

#include "compiler/translator/InfoSink.h"
#include "compiler/translator/Name.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/msl/AstHelpers.h"
#include "compiler/translator/msl/ProgramPrelude.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/util.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

namespace
{

class ProgramPrelude : public TIntermTraverser
{
    using LineTag       = unsigned;
    using FuncEmitter   = void (*)(ProgramPrelude &, const TFunction &);
    using FuncToEmitter = std::map<Name, FuncEmitter>;

  public:
    ProgramPrelude(TInfoSinkBase &out, const ProgramPreludeConfig &ppc)
        : TIntermTraverser(true, false, false), mOut(out)
    {
        mOut << "#include <metal_stdlib>\n\n";
        ALWAYS_INLINE();
        int_clamp();

        switch (ppc.shaderType)
        {
            case MetalShaderType::None:
                ASSERT(0 && "ppc.shaderType should not be ShaderTypeNone");
                break;
            case MetalShaderType::Vertex:
                transform_feedback_guard();
                break;
            case MetalShaderType::Fragment:
                functionConstants();
                mOut << "constant bool " << mtl::kSampleMaskWriteEnabledConstName << " = "
                     << mtl::kMultisampledRenderingConstName;
                if (ppc.usesDerivatives)
                {
                    mOut << " || " << mtl::kWriteHelperSampleMaskConstName;
                }
                mOut << ";\n";
                break;
            case MetalShaderType::Compute:
                ASSERT(0 && "compute shaders not currently supported");
                break;
            default:
                break;
        }

        mOut << "#pragma clang diagnostic ignored \"-Wunused-value\"\n";
    }

  private:
    bool emitGuard(LineTag lineTag)
    {
        if (mEmitted.find(lineTag) != mEmitted.end())
        {
            return false;
        }
        mEmitted.insert(lineTag);
        return true;
    }

    static FuncToEmitter BuildFuncToEmitter();

    void visitOperator(TOperator op, const TFunction *func, const TType *argType0);

    void visitOperator(TOperator op,
                       const TFunction *func,
                       const TType *argType0,
                       const TType *argType1);

    void visitOperator(TOperator op,
                       const TFunction *func,
                       const TType *argType0,
                       const TType *argType1,
                       const TType *argType2);

    void visitVariable(const Name &name, const TType &type);
    void visitVariable(const TVariable &var);
    void visitStructure(const TStructure &s);

    bool visitBinary(Visit, TIntermBinary *node) override;
    bool visitUnary(Visit, TIntermUnary *node) override;
    bool visitAggregate(Visit, TIntermAggregate *node) override;
    bool visitDeclaration(Visit, TIntermDeclaration *node) override;
    void visitSymbol(TIntermSymbol *node) override;

  private:
    void ALWAYS_INLINE();

    void transform_feedback_guard();

    void enable_if();
    void addressof();
    void distanceScalar();
    void faceforwardScalar();
    void reflectScalar();
    void refractScalar();
    void degrees();
    void radians();
    void mod();
    void mixBool();
    void postIncrementMatrix();
    void preIncrementMatrix();
    void postDecrementMatrix();
    void preDecrementMatrix();
    void negateMatrix();
    void matmulAssign();
    void int_clamp();
    void addMatrixScalarAssign();
    void subMatrixScalarAssign();
    void addMatrixScalar();
    void addScalarMatrix();
    void subMatrixScalar();
    void subScalarMatrix();
    void divMatrixScalar();
    void divMatrixScalarAssign();
    void divScalarMatrix();
    void componentWiseDivide();
    void componentWiseDivideAssign();
    void componentWiseMultiply();
    void outerProduct();
    void inverse2();
    void inverse3();
    void inverse4();
    void equalScalar();
    void equalVector();
    void equalMatrix();
    void notEqualVector();
    void notEqualStruct();
    void notEqualStructArray();
    void notEqualMatrix();
    void equalArray();
    void equalStructArray();
    void notEqualArray();
    void signInt();
    void pack_half_2x16();
    void unpack_half_2x16();
    void vectorElemRef();
    void swizzleRef();
    void out();
    void inout();
    void flattenArray();
    void castVector();
    void castMatrix();
    void functionConstants();
    void textureEnv();
    void texelFetch_2D();
    void texelFetch_3D();
    void texelFetch_2DArray();
    void texelFetch_2DMS();
    void texelFetchOffset_2D();
    void texelFetchOffset_3D();
    void texelFetchOffset_2DArray();
    void texture_2D();
    void texture_3D();
    void texture_Cube();
    void texture_2DArray();
    void texture_2DShadow();
    void texture_CubeShadow();
    void texture_2DArrayShadow();
    void textureBias_2D();
    void textureBias_3D();
    void textureBias_Cube();
    void textureBias_2DArray();
    void textureBias_2DShadow();
    void textureBias_CubeShadow();
    void textureBias_2DArrayShadow();
    void texture2D();
    void texture2DBias();
    void texture2DGradEXT();
    void texture2DLod();
    void texture2DLodEXT();
    void texture2DProj();
    void texture2DProjBias();
    void texture2DProjGradEXT();
    void texture2DProjLod();
    void texture2DProjLodEXT();
    void texture3D();
    void texture3DBias();
    void texture3DLod();
    void texture3DProj();
    void texture3DProjBias();
    void texture3DProjLod();
    void textureCube();
    void textureCubeBias();
    void textureCubeGradEXT();
    void textureCubeLod();
    void textureCubeLodEXT();
    void textureGrad_2D();
    void textureGrad_3D();
    void textureGrad_Cube();
    void textureGrad_2DArray();
    void textureGrad_2DShadow();
    void textureGrad_CubeShadow();
    void textureGrad_2DArrayShadow();
    void textureGradOffset_2D();
    void textureGradOffset_3D();
    void textureGradOffset_2DArray();
    void textureGradOffset_2DShadow();
    void textureGradOffset_2DArrayShadow();
    void textureLod_2D();
    void textureLod_3D();
    void textureLod_Cube();
    void textureLod_2DArray();
    void textureLod_2DShadow();
    void textureLod_CubeShadow();
    void textureLod_2DArrayShadow();
    void textureLodOffset_2D();
    void textureLodOffset_3D();
    void textureLodOffset_2DArray();
    void textureLodOffset_2DShadow();
    void textureLodOffset_2DArrayShadow();
    void textureOffset_2D();
    void textureOffset_3D();
    void textureOffset_2DArray();
    void textureOffset_2DShadow();
    void textureOffset_2DArrayShadow();
    void textureOffsetBias_2D();
    void textureOffsetBias_3D();
    void textureOffsetBias_2DArray();
    void textureOffsetBias_2DShadow();
    void textureOffsetBias_2DArrayShadow();
    void textureProj_2D_float3();
    void textureProj_2D_float4();
    void textureProj_2DShadow();
    void textureProj_3D();
    void textureProjBias_2D_float3();
    void textureProjBias_2D_float4();
    void textureProjBias_2DShadow();
    void textureProjBias_3D();
    void textureProjGrad_2D_float3();
    void textureProjGrad_2D_float4();
    void textureProjGrad_2DShadow();
    void textureProjGrad_3D();
    void textureProjGradOffset_2D_float3();
    void textureProjGradOffset_2D_float4();
    void textureProjGradOffset_2DShadow();
    void textureProjGradOffset_3D();
    void textureProjLod_2D_float3();
    void textureProjLod_2D_float4();
    void textureProjLod_2DShadow();
    void textureProjLod_3D();
    void textureProjLodOffset_2D_float3();
    void textureProjLodOffset_2D_float4();
    void textureProjLodOffset_2DShadow();
    void textureProjLodOffset_3D();
    void textureProjOffset_2D_float3();
    void textureProjOffset_2D_float4();
    void textureProjOffset_2DShadow();
    void textureProjOffset_3D();
    void textureProjOffsetBias_2D_float3();
    void textureProjOffsetBias_2D_float4();
    void textureProjOffsetBias_2DShadow();
    void textureProjOffsetBias_3D();
    void textureSize_2D();
    void textureSize_3D();
    void textureSize_2DArray();
    void textureSize_2DArrayShadow();
    void textureSize_2DMS();
    void imageLoad();
    void imageStore();
    void memoryBarrierImage();
    void interpolateAtCenter();
    void interpolateAtCentroid();
    void interpolateAtSample();
    void interpolateAtOffset();

  private:
    TInfoSinkBase &mOut;
    std::unordered_set<LineTag> mEmitted;
    std::unordered_set<const TSymbol *> mHandled;
    const FuncToEmitter mFuncToEmitter = BuildFuncToEmitter();
};

}  // anonymous namespace

////////////////////////////////////////////////////////////////////////////////

#define PROGRAM_PRELUDE_DECLARE(name, code, ...)                \
    void ProgramPrelude::name()                                 \
    {                                                           \
        ASSERT(code[0] == '\n');                                \
        if (emitGuard(__LINE__))                                \
        {                                                       \
            __VA_ARGS__; /* dependencies */                     \
            mOut << (static_cast<const char *>(code "\n") + 1); \
        }                                                       \
    }

////////////////////////////////////////////////////////////////////////////////

PROGRAM_PRELUDE_DECLARE(transform_feedback_guard, R"(
#if TRANSFORM_FEEDBACK_ENABLED
    #define __VERTEX_OUT(args) void
#else
    #define __VERTEX_OUT(args) args
#endif
)")

PROGRAM_PRELUDE_DECLARE(ALWAYS_INLINE, R"(
#define ANGLE_ALWAYS_INLINE __attribute__((always_inline))
)")

PROGRAM_PRELUDE_DECLARE(enable_if, R"(
template <bool B, typename T = void>
struct ANGLE_enable_if {};
template <typename T>
struct ANGLE_enable_if<true, T>
{
    using type = T;
};
template <bool B>
using ANGLE_enable_if_t = typename ANGLE_enable_if<B>::type;
)")

PROGRAM_PRELUDE_DECLARE(addressof,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE thread T * ANGLE_addressof(thread T &ref)
{
    return &ref;
}
)")

PROGRAM_PRELUDE_DECLARE(distanceScalar,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE T ANGLE_distance_scalar(T x, T y)
{
    return metal::abs(x - y);
}
)")

PROGRAM_PRELUDE_DECLARE(faceforwardScalar,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE T ANGLE_faceforward_scalar(T n, T i, T nref)
{
    return nref * i < T(0) ? n : -n;
}
)")

PROGRAM_PRELUDE_DECLARE(reflectScalar,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE T ANGLE_reflect_scalar(T i, T n)
{
    return i - T(2) * (n * i) * n;
}
)")

PROGRAM_PRELUDE_DECLARE(refractScalar,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE T ANGLE_refract_scalar(T i, T n, T eta)
{
    auto dotNI = n * i;
    auto k = T(1) - eta * eta * (T(1) - dotNI * dotNI);
    if (k < T(0))
    {
        return T(0);
    }
    else
    {
        return eta * i - (eta * dotNI + metal::sqrt(k)) * n;
    }
}
)")

PROGRAM_PRELUDE_DECLARE(signInt,
                        R"(
ANGLE_ALWAYS_INLINE int ANGLE_sign_int(int x)
{
    return (0 < x) - (x < 0);
}
template <int N>
ANGLE_ALWAYS_INLINE metal::vec<int, N> ANGLE_sign_int(metal::vec<int, N> x)
{
    metal::vec<int, N> s;
    for (int i = 0; i < N; ++i)
    {
        s[i] = ANGLE_sign_int(x[i]);
    }
    return s;
}
)")

PROGRAM_PRELUDE_DECLARE(int_clamp,
                        R"(
ANGLE_ALWAYS_INLINE int ANGLE_int_clamp(int value, int minValue, int maxValue)
{
    return ((value < minValue) ?  minValue : ((value > maxValue) ? maxValue : value));
};
)")

PROGRAM_PRELUDE_DECLARE(degrees, R"(
template <typename T>
ANGLE_ALWAYS_INLINE T ANGLE_degrees(T x)
{
    return static_cast<T>(57.29577951308232) * x;
}
)")

PROGRAM_PRELUDE_DECLARE(radians, R"(
template <typename T>
ANGLE_ALWAYS_INLINE T ANGLE_radians(T x)
{
    return static_cast<T>(1.7453292519943295e-2) * x;
}
)")

PROGRAM_PRELUDE_DECLARE(mod,
                        R"(
template <typename X, typename Y>
ANGLE_ALWAYS_INLINE X ANGLE_mod(X x, Y y)
{
    return x - y * metal::floor(x / y);
}
)")

PROGRAM_PRELUDE_DECLARE(mixBool,
                        R"(
template <typename T, int N>
ANGLE_ALWAYS_INLINE metal::vec<T,N> ANGLE_mix_bool(metal::vec<T, N> a, metal::vec<T, N> b, metal::vec<bool, N> c)
{
    return metal::mix(a, b, static_cast<metal::vec<T,N>>(c));
}
)")

PROGRAM_PRELUDE_DECLARE(pack_half_2x16,
                        R"(
ANGLE_ALWAYS_INLINE uint32_t ANGLE_pack_half_2x16(float2 v)
{
    return as_type<uint32_t>(half2(v));
}
)")

PROGRAM_PRELUDE_DECLARE(unpack_half_2x16,
                        R"(
ANGLE_ALWAYS_INLINE float2 ANGLE_unpack_half_2x16(uint32_t x)
{
    return float2(as_type<half2>(x));
}
)")

PROGRAM_PRELUDE_DECLARE(matmulAssign, R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE thread metal::matrix<T, Cols, Rows> &operator*=(thread metal::matrix<T, Cols, Rows> &a, metal::matrix<T, Cols, Cols> b)
{
    a = a * b;
    return a;
}
)")

PROGRAM_PRELUDE_DECLARE(postIncrementMatrix,
                        R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE metal::matrix<T, Cols, Rows> operator++(thread metal::matrix<T, Cols, Rows> &a, int)
{
    auto b = a;
    a += T(1);
    return b;
}
)",
                        addMatrixScalarAssign())

PROGRAM_PRELUDE_DECLARE(preIncrementMatrix,
                        R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE thread metal::matrix<T, Cols, Rows> &operator++(thread metal::matrix<T, Cols, Rows> &a)
{
    a += T(1);
    return a;
}
)",
                        addMatrixScalarAssign())

PROGRAM_PRELUDE_DECLARE(postDecrementMatrix,
                        R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE metal::matrix<T, Cols, Rows> operator--(thread metal::matrix<T, Cols, Rows> &a, int)
{
    auto b = a;
    a -= T(1);
    return b;
}
)",
                        subMatrixScalarAssign())

PROGRAM_PRELUDE_DECLARE(preDecrementMatrix,
                        R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE thread metal::matrix<T, Cols, Rows> &operator--(thread metal::matrix<T, Cols, Rows> &a)
{
    a -= T(1);
    return a;
}
)",
                        subMatrixScalarAssign())

PROGRAM_PRELUDE_DECLARE(negateMatrix,
                        R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE metal::matrix<T, Cols, Rows> operator-(metal::matrix<T, Cols, Rows> m)
{
    for (size_t col = 0; col < Cols; ++col)
    {
        thread auto &mCol = m[col];
        mCol = -mCol;
    }
    return m;
}
)")

PROGRAM_PRELUDE_DECLARE(addMatrixScalarAssign, R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE thread metal::matrix<T, Cols, Rows> &operator+=(thread metal::matrix<T, Cols, Rows> &m, T x)
{
    for (size_t col = 0; col < Cols; ++col)
    {
        m[col] += x;
    }
    return m;
}
)")

PROGRAM_PRELUDE_DECLARE(addMatrixScalar,
                        R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE metal::matrix<T, Cols, Rows> operator+(metal::matrix<T, Cols, Rows> m, T x)
{
    m += x;
    return m;
}
)",
                        addMatrixScalarAssign())

PROGRAM_PRELUDE_DECLARE(addScalarMatrix,
                        R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE metal::matrix<T, Cols, Rows> operator+(T x, metal::matrix<T, Cols, Rows> m)
{
    for (size_t col = 0; col < Cols; ++col)
    {
        m[col] = x + m[col];
    }
    return m;
}
)")

PROGRAM_PRELUDE_DECLARE(subMatrixScalarAssign,
                        R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE thread metal::matrix<T, Cols, Rows> &operator-=(thread metal::matrix<T, Cols, Rows> &m, T x)
{
    for (size_t col = 0; col < Cols; ++col)
    {
        m[col] -= x;
    }
    return m;
}
)")

PROGRAM_PRELUDE_DECLARE(subMatrixScalar,
                        R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE metal::matrix<T, Cols, Rows> operator-(metal::matrix<T, Cols, Rows> m, T x)
{
    m -= x;
    return m;
}
)",
                        subMatrixScalarAssign())

PROGRAM_PRELUDE_DECLARE(subScalarMatrix,
                        R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE metal::matrix<T, Cols, Rows> operator-(T x, metal::matrix<T, Cols, Rows> m)
{
    for (size_t col = 0; col < Cols; ++col)
    {
        m[col] = x - m[col];
    }
    return m;
}
)")

PROGRAM_PRELUDE_DECLARE(divMatrixScalarAssign,
                        R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE thread metal::matrix<T, Cols, Rows> &operator/=(thread metal::matrix<T, Cols, Rows> &m, T x)
{
    for (size_t col = 0; col < Cols; ++col)
    {
        m[col] /= x;
    }
    return m;
}
)")

PROGRAM_PRELUDE_DECLARE(divMatrixScalar,
                        R"(
#if __METAL_VERSION__ <= 220
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE metal::matrix<T, Cols, Rows> operator/(metal::matrix<T, Cols, Rows> m, T x)
{
    m /= x;
    return m;
}
#endif
)",
                        divMatrixScalarAssign())

PROGRAM_PRELUDE_DECLARE(divScalarMatrix,
                        R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE metal::matrix<T, Cols, Rows> operator/(T x, metal::matrix<T, Cols, Rows> m)
{
    for (size_t col = 0; col < Cols; ++col)
    {
        m[col] = x / m[col];
    }
    return m;
}
)")

PROGRAM_PRELUDE_DECLARE(componentWiseDivide, R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE metal::matrix<T, Cols, Rows> operator/(metal::matrix<T, Cols, Rows> a, metal::matrix<T, Cols, Rows> b)
{
    for (size_t col = 0; col < Cols; ++col)
    {
        a[col] /= b[col];
    }
    return a;
}
)")

PROGRAM_PRELUDE_DECLARE(componentWiseDivideAssign,
                        R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE thread metal::matrix<T, Cols, Rows> &operator/=(thread metal::matrix<T, Cols, Rows> &a, metal::matrix<T, Cols, Rows> b)
{
    a = a / b;
    return a;
}
)",
                        componentWiseDivide())

PROGRAM_PRELUDE_DECLARE(componentWiseMultiply, R"(
template <typename T, int Cols, int Rows>
ANGLE_ALWAYS_INLINE metal::matrix<T, Cols, Rows> ANGLE_componentWiseMultiply(metal::matrix<T, Cols, Rows> a, metal::matrix<T, Cols, Rows> b)
{
    for (size_t col = 0; col < Cols; ++col)
    {
        a[col] *= b[col];
    }
    return a;
}
)")

PROGRAM_PRELUDE_DECLARE(outerProduct, R"(
template <typename T, int M, int N>
ANGLE_ALWAYS_INLINE metal::matrix<T, N, M> ANGLE_outerProduct(metal::vec<T, M> u, metal::vec<T, N> v)
{
    metal::matrix<T, N, M> o;
    for (size_t n = 0; n < N; ++n)
    {
        o[n] = u * v[n];
    }
    return o;
}
)")

PROGRAM_PRELUDE_DECLARE(inverse2, R"(
template <typename T>
ANGLE_ALWAYS_INLINE metal::matrix<T, 2, 2> ANGLE_inverse(metal::matrix<T, 2, 2> m)
{
    metal::matrix<T, 2, 2> adj;
    adj[0][0] =  m[1][1];
    adj[0][1] = -m[0][1];
    adj[1][0] = -m[1][0];
    adj[1][1] =  m[0][0];
    T det = (adj[0][0] * m[0][0]) + (adj[0][1] * m[1][0]);
    return adj * (T(1) / det);
}
)")

PROGRAM_PRELUDE_DECLARE(inverse3, R"(
template <typename T>
ANGLE_ALWAYS_INLINE metal::matrix<T, 3, 3> ANGLE_inverse(metal::matrix<T, 3, 3> m)
{
    T a = m[1][1] * m[2][2] - m[2][1] * m[1][2];
    T b = m[1][0] * m[2][2];
    T c = m[1][2] * m[2][0];
    T d = m[1][0] * m[2][1];
    T det = m[0][0] * (a) -
            m[0][1] * (b - c) +
            m[0][2] * (d - m[1][1] * m[2][0]);
    det = T(1) / det;
    metal::matrix<T, 3, 3> minv;
    minv[0][0] = (a) * det;
    minv[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * det;
    minv[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * det;
    minv[1][0] = (c - b) * det;
    minv[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * det;
    minv[1][2] = (m[1][0] * m[0][2] - m[0][0] * m[1][2]) * det;
    minv[2][0] = (d - m[2][0] * m[1][1]) * det;
    minv[2][1] = (m[2][0] * m[0][1] - m[0][0] * m[2][1]) * det;
    minv[2][2] = (m[0][0] * m[1][1] - m[1][0] * m[0][1]) * det;
    return minv;
}
)")

PROGRAM_PRELUDE_DECLARE(inverse4, R"(
template <typename T>
ANGLE_ALWAYS_INLINE metal::matrix<T, 4, 4> ANGLE_inverse(metal::matrix<T, 4, 4> m)
{
    T A2323 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
    T A1323 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
    T A1223 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
    T A0323 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
    T A0223 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
    T A0123 = m[2][0] * m[3][1] - m[2][1] * m[3][0];
    T A2313 = m[1][2] * m[3][3] - m[1][3] * m[3][2];
    T A1313 = m[1][1] * m[3][3] - m[1][3] * m[3][1];
    T A1213 = m[1][1] * m[3][2] - m[1][2] * m[3][1];
    T A2312 = m[1][2] * m[2][3] - m[1][3] * m[2][2];
    T A1312 = m[1][1] * m[2][3] - m[1][3] * m[2][1];
    T A1212 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
    T A0313 = m[1][0] * m[3][3] - m[1][3] * m[3][0];
    T A0213 = m[1][0] * m[3][2] - m[1][2] * m[3][0];
    T A0312 = m[1][0] * m[2][3] - m[1][3] * m[2][0];
    T A0212 = m[1][0] * m[2][2] - m[1][2] * m[2][0];
    T A0113 = m[1][0] * m[3][1] - m[1][1] * m[3][0];
    T A0112 = m[1][0] * m[2][1] - m[1][1] * m[2][0];
    T a = m[1][1] * A2323 - m[1][2] * A1323 + m[1][3] * A1223;
    T b = m[1][0] * A2323 - m[1][2] * A0323 + m[1][3] * A0223;
    T c = m[1][0] * A1323 - m[1][1] * A0323 + m[1][3] * A0123;
    T d = m[1][0] * A1223 - m[1][1] * A0223 + m[1][2] * A0123;
    T det = m[0][0] * ( a )
          - m[0][1] * ( b )
          + m[0][2] * ( c )
          - m[0][3] * ( d );
    det = T(1) / det;
    metal::matrix<T, 4, 4> im;
    im[0][0] = det *   ( a );
    im[0][1] = det * - ( m[0][1] * A2323 - m[0][2] * A1323 + m[0][3] * A1223 );
    im[0][2] = det *   ( m[0][1] * A2313 - m[0][2] * A1313 + m[0][3] * A1213 );
    im[0][3] = det * - ( m[0][1] * A2312 - m[0][2] * A1312 + m[0][3] * A1212 );
    im[1][0] = det * - ( b );
    im[1][1] = det *   ( m[0][0] * A2323 - m[0][2] * A0323 + m[0][3] * A0223 );
    im[1][2] = det * - ( m[0][0] * A2313 - m[0][2] * A0313 + m[0][3] * A0213 );
    im[1][3] = det *   ( m[0][0] * A2312 - m[0][2] * A0312 + m[0][3] * A0212 );
    im[2][0] = det *   ( c );
    im[2][1] = det * - ( m[0][0] * A1323 - m[0][1] * A0323 + m[0][3] * A0123 );
    im[2][2] = det *   ( m[0][0] * A1313 - m[0][1] * A0313 + m[0][3] * A0113 );
    im[2][3] = det * - ( m[0][0] * A1312 - m[0][1] * A0312 + m[0][3] * A0112 );
    im[3][0] = det * - ( d );
    im[3][1] = det *   ( m[0][0] * A1223 - m[0][1] * A0223 + m[0][2] * A0123 );
    im[3][2] = det * - ( m[0][0] * A1213 - m[0][1] * A0213 + m[0][2] * A0113 );
    im[3][3] = det *   ( m[0][0] * A1212 - m[0][1] * A0212 + m[0][2] * A0112 );
    return im;
}
)")

PROGRAM_PRELUDE_DECLARE(equalArray,
                        R"(
template <typename T, size_t N>
ANGLE_ALWAYS_INLINE bool ANGLE_equal(metal::array<T, N> u, metal::array<T, N> v)
{
    for(size_t i = 0; i < N; i++)
        if (!ANGLE_equal(u[i], v[i])) return false;
    return true;
}
)",
                        equalScalar(),
                        equalVector(),
                        equalMatrix())

PROGRAM_PRELUDE_DECLARE(equalStructArray,
                        R"(
template <typename T, size_t N>
ANGLE_ALWAYS_INLINE bool ANGLE_equalStructArray(metal::array<T, N> u, metal::array<T, N> v)
{
    for(size_t i = 0; i < N; i++)
    {
        if (!ANGLE_equal(u[i], v[i]))
            return false;
    }
    return true;
}
)")

PROGRAM_PRELUDE_DECLARE(notEqualArray,
                        R"(
template <typename T, size_t N>
ANGLE_ALWAYS_INLINE bool ANGLE_notEqual(metal::array<T, N> u, metal::array<T, N> v)
{
    return !ANGLE_equal(u,v);
}
)",
                        equalArray())

PROGRAM_PRELUDE_DECLARE(equalScalar,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE bool ANGLE_equal(T u, T v)
{
    return u == v;
}
)")

PROGRAM_PRELUDE_DECLARE(equalVector,
                        R"(
template <typename T, int N>
ANGLE_ALWAYS_INLINE bool ANGLE_equal(metal::vec<T, N> u, metal::vec<T, N> v)
{
    return metal::all(u == v);
}
)")

PROGRAM_PRELUDE_DECLARE(equalMatrix,
                        R"(
template <typename T, int C, int R>
ANGLE_ALWAYS_INLINE bool ANGLE_equal(metal::matrix<T, C, R> a, metal::matrix<T, C, R> b)
{
    for (int c = 0; c < C; ++c)
    {
        if (!ANGLE_equal(a[c], b[c]))
        {
            return false;
        }
    }
    return true;
}
)",
                        equalVector())

PROGRAM_PRELUDE_DECLARE(notEqualMatrix,
                        R"(
template <typename T, int C, int R>
ANGLE_ALWAYS_INLINE bool ANGLE_notEqual(metal::matrix<T, C, R> u, metal::matrix<T, C, R> v)
{
    return !ANGLE_equal(u, v);
}
)",
                        equalMatrix())

PROGRAM_PRELUDE_DECLARE(notEqualVector,
                        R"(
template <typename T, int N>
ANGLE_ALWAYS_INLINE bool ANGLE_notEqual(metal::vec<T, N> u, metal::vec<T, N> v)
{
    return !ANGLE_equal(u, v);
}
)",
                        equalVector())

PROGRAM_PRELUDE_DECLARE(notEqualStruct,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE bool ANGLE_notEqualStruct(thread const T &a, thread const T &b)
{
    return !ANGLE_equal(a, b);
}
template <typename T>
ANGLE_ALWAYS_INLINE bool ANGLE_notEqualStruct(constant const T &a, thread const T &b)
{
    return !ANGLE_equal(a, b);
}
template <typename T>
ANGLE_ALWAYS_INLINE bool ANGLE_notEqualStruct(thread const T &a, constant const T &b)
{
    return !ANGLE_equal(a, b);
}
template <typename T>
ANGLE_ALWAYS_INLINE bool ANGLE_notEqualStruct(constant const T &a, constant const T &b)
{
    return !ANGLE_equal(a, b);
}
)",
                        equalVector(),
                        equalMatrix())

PROGRAM_PRELUDE_DECLARE(notEqualStructArray,
                        R"(
template <typename T, size_t N>
ANGLE_ALWAYS_INLINE bool ANGLE_notEqualStructArray(metal::array<T, N> u, metal::array<T, N> v)
{
    for(size_t i = 0; i < N; i++)
    {
        if (ANGLE_notEqualStruct(u[i], v[i]))
            return true;
    }
    return false;
}
)",
                        notEqualStruct())

PROGRAM_PRELUDE_DECLARE(vectorElemRef,
                        R"(
template <typename T, int N>
struct ANGLE_VectorElemRef
{
    thread metal::vec<T, N> &mVec;
    T mRef;
    const int mIndex;
    ~ANGLE_VectorElemRef() { mVec[mIndex] = mRef; }
    ANGLE_VectorElemRef(thread metal::vec<T, N> &vec, int index)
        : mVec(vec), mRef(vec[index]), mIndex(index)
    {}
    operator thread T &() { return mRef; }
};
template <typename T, int N>
ANGLE_ALWAYS_INLINE ANGLE_VectorElemRef<T, N> ANGLE_elem_ref(thread metal::vec<T, N> &vec, int index)
{
    return ANGLE_VectorElemRef<T, N>(vec, metal::clamp(index, 0, N - 1));
}
)")

PROGRAM_PRELUDE_DECLARE(swizzleRef,
                        R"(
template <typename T, int VN, int SN>
struct ANGLE_SwizzleRef
{
    thread metal::vec<T, VN> &mVec;
    metal::vec<T, SN> mRef;
    int mIndices[SN];
    ~ANGLE_SwizzleRef()
    {
        for (int i = 0; i < SN; ++i)
        {
            const int j = mIndices[i];
            mVec[j] = mRef[i];
        }
    }
    ANGLE_SwizzleRef(thread metal::vec<T, VN> &vec, thread const int *indices)
        : mVec(vec)
    {
        for (int i = 0; i < SN; ++i)
        {
            const int j = indices[i];
            mIndices[i] = j;
            mRef[i] = mVec[j];
        }
    }
    operator thread metal::vec<T, SN> &() { return mRef; }
};
template <typename T, int N>
ANGLE_ALWAYS_INLINE ANGLE_VectorElemRef<T, N> ANGLE_swizzle_ref(thread metal::vec<T, N> &vec, int i0)
{
    return ANGLE_VectorElemRef<T, N>(vec, i0);
}
template <typename T, int N>
ANGLE_ALWAYS_INLINE ANGLE_SwizzleRef<T, N, 2> ANGLE_swizzle_ref(thread metal::vec<T, N> &vec, int i0, int i1)
{
    const int is[] = { i0, i1 };
    return ANGLE_SwizzleRef<T, N, 2>(vec, is);
}
template <typename T, int N>
ANGLE_ALWAYS_INLINE ANGLE_SwizzleRef<T, N, 3> ANGLE_swizzle_ref(thread metal::vec<T, N> &vec, int i0, int i1, int i2)
{
    const int is[] = { i0, i1, i2 };
    return ANGLE_SwizzleRef<T, N, 3>(vec, is);
}
template <typename T, int N>
ANGLE_ALWAYS_INLINE ANGLE_SwizzleRef<T, N, 4> ANGLE_swizzle_ref(thread metal::vec<T, N> &vec, int i0, int i1, int i2, int i3)
{
    const int is[] = { i0, i1, i2, i3 };
    return ANGLE_SwizzleRef<T, N, 4>(vec, is);
}
)",
                        vectorElemRef())

PROGRAM_PRELUDE_DECLARE(out, R"(
template <typename T>
struct ANGLE_Out
{
    T mTemp;
    thread T &mDest;
    ~ANGLE_Out() { mDest = mTemp; }
    ANGLE_Out(thread T &dest)
        : mTemp(dest), mDest(dest)
    {}
    operator thread T &() { return mTemp; }
};
template <typename T>
ANGLE_ALWAYS_INLINE ANGLE_Out<T> ANGLE_out(thread T &dest)
{
    return ANGLE_Out<T>(dest);
}
)")

PROGRAM_PRELUDE_DECLARE(inout, R"(
template <typename T>
struct ANGLE_InOut
{
    T mTemp;
    thread T &mDest;
    ~ANGLE_InOut() { mDest = mTemp; }
    ANGLE_InOut(thread T &dest)
        : mTemp(dest), mDest(dest)
    {}
    operator thread T &() { return mTemp; }
};
template <typename T>
ANGLE_ALWAYS_INLINE ANGLE_InOut<T> ANGLE_inout(thread T &dest)
{
    return ANGLE_InOut<T>(dest);
}
)")

PROGRAM_PRELUDE_DECLARE(flattenArray, R"(
template <typename T>
struct ANGLE_flatten_impl
{
    static ANGLE_ALWAYS_INLINE thread T *exec(thread T &x)
    {
        return &x;
    }
};
template <typename T, size_t N>
struct ANGLE_flatten_impl<metal::array<T, N>>
{
    static ANGLE_ALWAYS_INLINE auto exec(thread metal::array<T, N> &arr) -> T
    {
        return ANGLE_flatten_impl<T>::exec(arr[0]);
    }
};
template <typename T, size_t N>
ANGLE_ALWAYS_INLINE auto ANGLE_flatten(thread metal::array<T, N> &arr) -> T
{
    return ANGLE_flatten_impl<T>::exec(arr[0]);
}
)")

PROGRAM_PRELUDE_DECLARE(castVector, R"(
template <typename T, int N1, int N2>
struct ANGLE_castVector {};
template <typename T, int N>
struct ANGLE_castVector<T, N, N>
{
    static ANGLE_ALWAYS_INLINE metal::vec<T, N> exec(metal::vec<T, N> const v)
    {
        return v;
    }
};
template <typename T>
struct ANGLE_castVector<T, 2, 3>
{
    static ANGLE_ALWAYS_INLINE metal::vec<T, 2> exec(metal::vec<T, 3> const v)
    {
        return v.xy;
    }
};
template <typename T>
struct ANGLE_castVector<T, 2, 4>
{
    static ANGLE_ALWAYS_INLINE metal::vec<T, 2> exec(metal::vec<T, 4> const v)
    {
        return v.xy;
    }
};
template <typename T>
struct ANGLE_castVector<T, 3, 4>
{
    static ANGLE_ALWAYS_INLINE metal::vec<T, 3> exec(metal::vec<T, 4> const v)
    {
        return as_type<metal::vec<T, 3>>(v);
    }
};
template <int N1, int N2, typename T>
ANGLE_ALWAYS_INLINE metal::vec<T, N1> ANGLE_cast(metal::vec<T, N2> const v)
{
    return ANGLE_castVector<T, N1, N2>::exec(v);
}
)")

PROGRAM_PRELUDE_DECLARE(castMatrix,
                        R"(
template <typename T, int C1, int R1, int C2, int R2, typename Enable = void>
struct ANGLE_castMatrix
{
    static ANGLE_ALWAYS_INLINE metal::matrix<T, C1, R1> exec(metal::matrix<T, C2, R2> const m2)
    {
        metal::matrix<T, C1, R1> m1;
        const int MinC = C1 <= C2 ? C1 : C2;
        const int MinR = R1 <= R2 ? R1 : R2;
        for (int c = 0; c < MinC; ++c)
        {
            for (int r = 0; r < MinR; ++r)
            {
                m1[c][r] = m2[c][r];
            }
            for (int r = R2; r < R1; ++r)
            {
                m1[c][r] = c == r ? T(1) : T(0);
            }
        }
        for (int c = C2; c < C1; ++c)
        {
            for (int r = 0; r < R1; ++r)
            {
                m1[c][r] = c == r ? T(1) : T(0);
            }
        }
        return m1;
    }
};
template <typename T, int C1, int R1, int C2, int R2>
struct ANGLE_castMatrix<T, C1, R1, C2, R2, ANGLE_enable_if_t<(C1 <= C2 && R1 <= R2)>>
{
    static ANGLE_ALWAYS_INLINE metal::matrix<T, C1, R1> exec(metal::matrix<T, C2, R2> const m2)
    {
        metal::matrix<T, C1, R1> m1;
        for (size_t c = 0; c < C1; ++c)
        {
            m1[c] = ANGLE_cast<R1>(m2[c]);
        }
        return m1;
    }
};
template <int C1, int R1, int C2, int R2, typename T>
ANGLE_ALWAYS_INLINE metal::matrix<T, C1, R1> ANGLE_cast(metal::matrix<T, C2, R2> const m)
{
    return ANGLE_castMatrix<T, C1, R1, C2, R2>::exec(m);
};
)",
                        enable_if(),
                        castVector())

PROGRAM_PRELUDE_DECLARE(textureEnv,
                        R"(
template <typename T>
struct ANGLE_TextureEnv
{
    thread T *texture;
    thread metal::sampler *sampler;
};
)")

PROGRAM_PRELUDE_DECLARE(functionConstants,
                        R"(
#define ANGLE_SAMPLE_COMPARE_GRADIENT_INDEX   0
#define ANGLE_RASTERIZATION_DISCARD_INDEX     1
#define ANGLE_MULTISAMPLED_RENDERING_INDEX    2
#define ANGLE_DEPTH_WRITE_ENABLED_INDEX       3
#define ANGLE_EMULATE_ALPHA_TO_COVERAGE_INDEX 4
#define ANGLE_WRITE_HELPER_SAMPLE_MASK_INDEX  5

constant bool ANGLEUseSampleCompareGradient [[function_constant(ANGLE_SAMPLE_COMPARE_GRADIENT_INDEX)]];
constant bool ANGLERasterizerDisabled       [[function_constant(ANGLE_RASTERIZATION_DISCARD_INDEX)]];
constant bool ANGLEMultisampledRendering    [[function_constant(ANGLE_MULTISAMPLED_RENDERING_INDEX)]];
constant bool ANGLEDepthWriteEnabled        [[function_constant(ANGLE_DEPTH_WRITE_ENABLED_INDEX)]];
constant bool ANGLEEmulateAlphaToCoverage   [[function_constant(ANGLE_EMULATE_ALPHA_TO_COVERAGE_INDEX)]];
constant bool ANGLEWriteHelperSampleMask    [[function_constant(ANGLE_WRITE_HELPER_SAMPLE_MASK_INDEX)]];

#define ANGLE_ALPHA0
)")

PROGRAM_PRELUDE_DECLARE(texelFetch_2D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_texelFetch(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::int2 const coord,
    int const level)
{
    return env.texture->read(uint2(coord), uint32_t(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texelFetch_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_texelFetch(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::int3 const coord,
    int const level)
{
    return env.texture->read(uint3(coord), uint32_t(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texelFetch_2DArray,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_texelFetch(
    thread ANGLE_TextureEnv<metal::texture2d_array<T>> &env,
    metal::int3 const coord,
    int const level)
{
    return env.texture->read(uint2(coord.xy), uint32_t(coord.z), uint32_t(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texelFetch_2DMS,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_texelFetch(
    thread ANGLE_TextureEnv<metal::texture2d_ms<T>> &env,
    metal::int2 const coord,
    int const sample)
{
    return env.texture->read(uint2(coord), uint32_t(sample));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texelFetchOffset_2D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_texelFetchOffset(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::int2 const coord,
    int const level,
    metal::int2 const offset)
{
    return env.texture->read(uint2(coord + offset), uint32_t(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texelFetchOffset_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_texelFetchOffset(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::int3 const coord,
    int const level,
    metal::int3 const offset)
{
    return env.texture->read(uint3(coord + offset), uint32_t(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texelFetchOffset_2DArray,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_texelFetchOffset(
    thread ANGLE_TextureEnv<metal::texture2d_array<T>> &env,
    metal::int3 const coord,
    int const level,
    metal::int2 const offset)
{
    return env.texture->read(uint2(coord.xy + offset), uint32_t(coord.z), uint32_t(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture_2D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_texture(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float2 const coord)
{
    return env.texture->sample(*env.sampler, coord);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureBias_2D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_texture(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float2 const coord,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord, metal::bias(bias));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_texture(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::float3 const coord)
{
    return env.texture->sample(*env.sampler, coord);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureBias_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_texture(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::float3 const coord,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord, metal::bias(bias));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture_Cube,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_texture(
    thread ANGLE_TextureEnv<metal::texturecube<T>> &env,
    metal::float3 const coord)
{
    return env.texture->sample(*env.sampler, coord);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureBias_Cube,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_texture(
    thread ANGLE_TextureEnv<metal::texturecube<T>> &env,
    metal::float3 const coord,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord, metal::bias(bias));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture_2DArray,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_texture(
    thread ANGLE_TextureEnv<metal::texture2d_array<T>> &env,
    metal::float3 const coord)
{
    return env.texture->sample(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureBias_2DArray,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_texture(
    thread ANGLE_TextureEnv<metal::texture2d_array<T>> &env,
    metal::float3 const coord,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), metal::bias(bias));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture_2DShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture(
    thread ANGLE_TextureEnv<metal::depth2d<float>> &env,
    metal::float3 const coord)
{
    return env.texture->sample_compare(*env.sampler, coord.xy, coord.z);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureBias_2DShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture(
    thread ANGLE_TextureEnv<metal::depth2d<float>> &env,
    metal::float3 const coord,
    float const bias)
{
    return env.texture->sample_compare(*env.sampler, coord.xy, coord.z, metal::bias(bias));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture_2DArrayShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture(
    thread ANGLE_TextureEnv<metal::depth2d_array<float>> &env,
    metal::float4 const coord)
{
    return env.texture->sample_compare(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), coord.w);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureBias_2DArrayShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture(
    thread ANGLE_TextureEnv<metal::depth2d_array<float>> &env,
    metal::float4 const coord,
    float const bias)
{
    return env.texture->sample_compare(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), coord.w, metal::bias(bias));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture_CubeShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture(
    thread ANGLE_TextureEnv<metal::depthcube<float>> &env,
    metal::float4 const coord)
{
    return env.texture->sample_compare(*env.sampler, coord.xyz, coord.w);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureBias_CubeShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture(
    thread ANGLE_TextureEnv<metal::depthcube<float>> &env,
    metal::float4 const coord,
    float const bias)
{
    return env.texture->sample_compare(*env.sampler, coord.xyz, coord.w, metal::bias(bias));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture2D,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture2D(
    thread ANGLE_TextureEnv<metal::texture2d<float>> &env,
    metal::float2 const coord)
{
    return env.texture->sample(*env.sampler, coord);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture2DBias,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture2D(
    thread ANGLE_TextureEnv<metal::texture2d<float>> &env,
    metal::float2 const coord,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord, metal::bias(bias));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture2DGradEXT,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture2DGradEXT(
    thread ANGLE_TextureEnv<metal::texture2d<float>> &env,
    metal::float2 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy)
{
    return env.texture->sample(*env.sampler, coord, metal::gradient2d(dPdx, dPdy));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture2DLod,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture2DLod(
    thread ANGLE_TextureEnv<metal::texture2d<float>> &env,
    metal::float2 const coord,
    float const level)
{
    return env.texture->sample(*env.sampler, coord, metal::level(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture2DLodEXT,
                        R"(
#define ANGLE_texture2DLodEXT ANGLE_texture2DLod
)",
                        texture2DLod())

PROGRAM_PRELUDE_DECLARE(texture2DProj,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture2DProj(
    thread ANGLE_TextureEnv<metal::texture2d<float>> &env,
    metal::float3 const coord)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.z);
}
ANGLE_ALWAYS_INLINE auto ANGLE_texture2DProj(
    thread ANGLE_TextureEnv<metal::texture2d<float>> &env,
    metal::float4 const coord)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.w);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture2DProjBias,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture2DProj(
    thread ANGLE_TextureEnv<metal::texture2d<float>> &env,
    metal::float3 const coord,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.z, metal::bias(bias));
}
ANGLE_ALWAYS_INLINE auto ANGLE_texture2DProj(
    thread ANGLE_TextureEnv<metal::texture2d<float>> &env,
    metal::float4 const coord,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.w, metal::bias(bias));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture2DProjGradEXT,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture2DProjGradEXT(
    thread ANGLE_TextureEnv<metal::texture2d<float>> &env,
    metal::float3 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.z, metal::gradient2d(dPdx, dPdy));
}
ANGLE_ALWAYS_INLINE auto ANGLE_texture2DProjGradEXT(
    thread ANGLE_TextureEnv<metal::texture2d<float>> &env,
    metal::float4 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.w, metal::gradient2d(dPdx, dPdy));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture2DProjLod,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture2DProjLod(
    thread ANGLE_TextureEnv<metal::texture2d<float>> &env,
    metal::float3 const coord,
    float const level)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.z, metal::level(level));
}
ANGLE_ALWAYS_INLINE auto ANGLE_texture2DProjLod(
    thread ANGLE_TextureEnv<metal::texture2d<float>> &env,
    metal::float4 const coord,
    float const level)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.w, metal::level(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture2DProjLodEXT,
                        R"(
#define ANGLE_texture2DProjLodEXT ANGLE_texture2DProjLod
)",
                        texture2DProjLod())

PROGRAM_PRELUDE_DECLARE(texture3D,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture3D(
    thread ANGLE_TextureEnv<metal::texture3d<float>> &env,
    metal::float3 const coord)
{
    return env.texture->sample(*env.sampler, coord);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture3DBias,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture3D(
    thread ANGLE_TextureEnv<metal::texture3d<float>> &env,
    metal::float3 const coord,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord, metal::bias(bias));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture3DLod,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture3DLod(
    thread ANGLE_TextureEnv<metal::texture3d<float>> &env,
    metal::float3 const coord,
    float const level)
{
    return env.texture->sample(*env.sampler, coord, metal::level(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture3DProj,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture3DProj(
    thread ANGLE_TextureEnv<metal::texture3d<float>> &env,
    metal::float4 const coord)
{
    return env.texture->sample(*env.sampler, coord.xyz/coord.w);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture3DProjBias,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture3DProj(
    thread ANGLE_TextureEnv<metal::texture3d<float>> &env,
    metal::float4 const coord,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord.xyz/coord.w, metal::bias(bias));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(texture3DProjLod,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_texture3DProjLod(
    thread ANGLE_TextureEnv<metal::texture3d<float>> &env,
    metal::float4 const coord,
    float const level)
{
    return env.texture->sample(*env.sampler, coord.xyz/coord.w, metal::level(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureCube,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureCube(
    thread ANGLE_TextureEnv<metal::texturecube<float>> &env,
    metal::float3 const coord)
{
    return env.texture->sample(*env.sampler, coord);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureCubeBias,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureCube(
    thread ANGLE_TextureEnv<metal::texturecube<float>> &env,
    metal::float3 const coord,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord, metal::bias(bias));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureCubeGradEXT,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureCubeGradEXT(
    thread ANGLE_TextureEnv<metal::texturecube<float>> &env,
    metal::float3 const coord,
    metal::float3 const dPdx,
    metal::float3 const dPdy)
{
    return env.texture->sample(*env.sampler, coord, metal::gradientcube(dPdx, dPdy));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureCubeLod,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureCubeLod(
    thread ANGLE_TextureEnv<metal::texturecube<float>> &env,
    metal::float3 const coord,
    float const level)
{
    return env.texture->sample(*env.sampler, coord, metal::level(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureCubeLodEXT,
                        R"(
#define ANGLE_textureCubeLodEXT ANGLE_textureCubeLod
)",
                        textureCubeLod())

PROGRAM_PRELUDE_DECLARE(textureGrad_2D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureGrad(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float2 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy)
{
    return env.texture->sample(*env.sampler, coord, metal::gradient2d(dPdx, dPdy));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureGrad_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureGrad(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::float3 const coord,
    metal::float3 const dPdx,
    metal::float3 const dPdy)
{
    return env.texture->sample(*env.sampler, coord, metal::gradient3d(dPdx, dPdy));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureGrad_Cube,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureGrad(
    thread ANGLE_TextureEnv<metal::texturecube<T>> &env,
    metal::float3 const coord,
    metal::float3 const dPdx,
    metal::float3 const dPdy)
{
    return env.texture->sample(*env.sampler, coord, metal::gradientcube(dPdx, dPdy));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureGrad_2DArray,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureGrad(
    thread ANGLE_TextureEnv<metal::texture2d_array<T>> &env,
    metal::float3 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy)
{
    return env.texture->sample(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), metal::gradient2d(dPdx, dPdy));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureGrad_2DShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureGrad(
    thread ANGLE_TextureEnv<metal::depth2d<float>> &env,
    metal::float3 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy)
{
    if (ANGLEUseSampleCompareGradient)
    {
        return env.texture->sample_compare(*env.sampler, coord.xy, coord.z, metal::gradient2d(dPdx, dPdy));
    }
    else
    {
        const float2 dims = float2(env.texture->get_width(0), env.texture->get_height(0));
        const float lod = 0.5 * metal::log2(metal::max(metal::length_squared(dPdx * dims), metal::length_squared(dPdy * dims)));
        return env.texture->sample_compare(*env.sampler, coord.xy, coord.z, metal::level(lod));
    }
}
)",
                        functionConstants(),
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureGrad_2DArrayShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureGrad(
    thread ANGLE_TextureEnv<metal::depth2d_array<float>> &env,
    metal::float4 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy)
{
    if (ANGLEUseSampleCompareGradient)
    {
        return env.texture->sample_compare(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), coord.w, metal::gradient2d(dPdx, dPdy));
    }
    else
    {
        const float2 dims = float2(env.texture->get_width(0), env.texture->get_height(0));
        const float lod = 0.5 * metal::log2(metal::max(metal::length_squared(dPdx * dims), metal::length_squared(dPdy * dims)));
        return env.texture->sample_compare(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), coord.w, metal::level(lod));
    }
}
)",
                        functionConstants(),
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureGrad_CubeShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureGrad(
    thread ANGLE_TextureEnv<metal::depthcube<float>> &env,
    metal::float4 const coord,
    metal::float3 const dPdx,
    metal::float3 const dPdy)
{
    if (ANGLEUseSampleCompareGradient)
    {
        return env.texture->sample_compare(*env.sampler, coord.xyz, coord.w, metal::gradientcube(dPdx, dPdy));
    }
    else
    {
        const float3 coord_abs = metal::abs(coord.xyz);
        const bool z_major = coord_abs.z >= metal::max(coord_abs.x, coord_abs.y);
        const bool y_major = coord_abs.y >= metal::max(coord_abs.x, coord_abs.z);
        const float3 Q = z_major ? coord.xyz : (y_major ? coord.xzy : coord.yzx);
        const float3 dQdx = z_major ? dPdx : (y_major ? dPdx.xzy : dPdx.yzx);
        const float3 dQdy = z_major ? dPdy : (y_major ? dPdy.xzy : dPdy.yzx);
        const float4 d = (float4(dQdx.xy, dQdy.xy) - (Q.xy / Q.z).xyxy * float4(dQdx.zz, dQdy.zz)) / Q.z;
        const float dim = float(env.texture->get_width(0));
        const float lod = -1.0 + 0.5 * metal::log2(dim * dim * metal::max(metal::length_squared(d.xy), metal::length_squared(d.zw)));
        return env.texture->sample_compare(*env.sampler, coord.xyz, coord.w, metal::level(lod));
    }
}
)",
                        functionConstants(),
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureGradOffset_2D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureGradOffset(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float2 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy,
    int2 const offset)
{
    return env.texture->sample(*env.sampler, coord, metal::gradient2d(dPdx, dPdy), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureGradOffset_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureGradOffset(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::float3 const coord,
    metal::float3 const dPdx,
    metal::float3 const dPdy,
    int3 const offset)
{
    return env.texture->sample(*env.sampler, coord, metal::gradient3d(dPdx, dPdy), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureGradOffset_2DArray,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureGradOffset(
    thread ANGLE_TextureEnv<metal::texture2d_array<T>> &env,
    metal::float3 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy,
    metal::int2 const offset)
{
    return env.texture->sample(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), metal::gradient2d(dPdx, dPdy), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureGradOffset_2DShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureGradOffset(
    thread ANGLE_TextureEnv<metal::depth2d<float>> &env,
    metal::float3 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy,
    metal::int2 const offset)
{
    if (ANGLEUseSampleCompareGradient)
    {
        return env.texture->sample_compare(*env.sampler, coord.xy, coord.z, metal::gradient2d(dPdx, dPdy), offset);
    }
    else
    {
        const float2 dims = float2(env.texture->get_width(0), env.texture->get_height(0));
        const float lod = 0.5 * metal::log2(metal::max(metal::length_squared(dPdx * dims), metal::length_squared(dPdy * dims)));
        return env.texture->sample_compare(*env.sampler, coord.xy, coord.z, metal::level(lod), offset);
    }
}
)",
                        functionConstants(),
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureGradOffset_2DArrayShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureGradOffset(
    thread ANGLE_TextureEnv<metal::depth2d_array<float>> &env,
    metal::float4 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy,
    metal::int2 const offset)
{
    if (ANGLEUseSampleCompareGradient)
    {
        return env.texture->sample_compare(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), coord.w, metal::gradient2d(dPdx, dPdy), offset);
    }
    else
    {
        const float2 dims = float2(env.texture->get_width(0), env.texture->get_height(0));
        const float lod = 0.5 * metal::log2(metal::max(metal::length_squared(dPdx * dims), metal::length_squared(dPdy * dims)));
        return env.texture->sample_compare(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), coord.w, metal::level(lod), offset);
    }
}
)",
                        functionConstants(),
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureLod_2D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureLod(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float2 const coord,
    float const level)
{
    return env.texture->sample(*env.sampler, coord, metal::level(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureLod_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureLod(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::float3 const coord,
    float const level)
{
    return env.texture->sample(*env.sampler, coord, metal::level(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureLod_Cube,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureLod(
    thread ANGLE_TextureEnv<metal::texturecube<T>> &env,
    metal::float3 const coord,
    float const level)
{
    return env.texture->sample(*env.sampler, coord, metal::level(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureLod_2DShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureLod(
    thread ANGLE_TextureEnv<metal::depth2d<float>> &env,
    metal::float3 const coord,
    float const level)
{
    return env.texture->sample_compare(*env.sampler, coord.xy, coord.z, metal::level(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureLod_2DArray,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureLod(
    thread ANGLE_TextureEnv<metal::texture2d_array<T>> &env,
    metal::float3 const coord,
    float const level)
{
    return env.texture->sample(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), metal::level(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureLod_CubeShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureLod(
    thread ANGLE_TextureEnv<metal::depthcube<float>> &env,
    metal::float4 const coord,
    float const level)
{
    return env.texture->sample_compare(*env.sampler, coord.xyz, coord.w, metal::level(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureLod_2DArrayShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureLod(
    thread ANGLE_TextureEnv<metal::depth2d_array<float>> &env,
    metal::float4 const coord,
    float const level)
{
    return env.texture->sample_compare(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), coord.w, metal::level(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureLodOffset_2D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureLodOffset(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float2 const coord,
    float const level,
    metal::int2 const offset)
{
    return env.texture->sample(*env.sampler, coord, metal::level(level), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureLodOffset_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureLodOffset(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::float3 const coord,
    float const level,
    metal::int3 const offset)
{
    return env.texture->sample(*env.sampler, coord, metal::level(level), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureLodOffset_2DShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureLodOffset(
    thread ANGLE_TextureEnv<metal::depth2d<float>> &env,
    metal::float3 const coord,
    float const level,
    int2 const offset)
{
    return env.texture->sample_compare(*env.sampler, coord.xy, coord.z, metal::level(level), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureLodOffset_2DArray,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureLodOffset(
    thread ANGLE_TextureEnv<metal::texture2d_array<T>> &env,
    metal::float3 const coord,
    float const level,
    metal::int2 const offset)
{
    return env.texture->sample(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), metal::level(level), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureLodOffset_2DArrayShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureLodOffset(
    thread ANGLE_TextureEnv<metal::depth2d_array<float>> &env,
    metal::float4 const coord,
    float const level,
    metal::int2 const offset)
{
    return env.texture->sample_compare(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), coord.w, metal::level(level), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureOffset_2D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureOffset(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float2 const coord,
    metal::int2 const offset)
{
    return env.texture->sample(*env.sampler, coord, offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureOffsetBias_2D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureOffset(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float2 const coord,
    metal::int2 const offset,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord, metal::bias(bias), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureOffset_2DArray,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureOffset(
    thread ANGLE_TextureEnv<metal::texture2d_array<T>> &env,
    metal::float3 const coord,
    metal::int2 const offset)
{
    return env.texture->sample(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureOffsetBias_2DArray,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureOffset(
    thread ANGLE_TextureEnv<metal::texture2d_array<T>> &env,
    metal::float3 const coord,
    metal::int2 const offset,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), metal::bias(bias), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureOffset_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureOffset(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::float3 const coord,
    metal::int3 const offset)
{
    return env.texture->sample(*env.sampler, coord, offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureOffsetBias_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureOffset(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::float3 const coord,
    metal::int3 const offset,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord, metal::bias(bias), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureOffset_2DShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureOffset(
    thread ANGLE_TextureEnv<metal::depth2d<float>> &env,
    metal::float3 const coord,
    metal::int2 const offset)
{
    return env.texture->sample_compare(*env.sampler, coord.xy, coord.z, offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureOffsetBias_2DShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureOffset(
    thread ANGLE_TextureEnv<metal::depth2d<float>> &env,
    metal::float3 const coord,
    metal::int2 const offset,
    float const bias)
{
    return env.texture->sample_compare(*env.sampler, coord.xy, coord.z, metal::bias(bias), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureOffset_2DArrayShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureOffset(
    thread ANGLE_TextureEnv<metal::depth2d_array<float>> &env,
    metal::float4 const coord,
    metal::int2 const offset)
{
    return env.texture->sample_compare(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), coord.w, offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureOffsetBias_2DArrayShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureOffset(
    thread ANGLE_TextureEnv<metal::depth2d_array<float>> &env,
    metal::float4 const coord,
    metal::int2 const offset,
    float const bias)
{
    return env.texture->sample_compare(*env.sampler, coord.xy, uint32_t(metal::round(coord.z)), coord.w, metal::bias(bias), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProj_2D_float3,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProj(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float3 const coord)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.z);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjBias_2D_float3,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProj(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float3 const coord,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.z, metal::bias(bias));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProj_2D_float4,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProj(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float4 const coord)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.w);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjBias_2D_float4,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProj(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float4 const coord,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.w, metal::bias(bias));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProj_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProj(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::float4 const coord)
{
    return env.texture->sample(*env.sampler, coord.xyz/coord.w);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjBias_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProj(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::float4 const coord,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord.xyz/coord.w, metal::bias(bias));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProj_2DShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureProj(
    thread ANGLE_TextureEnv<metal::depth2d<float>> &env,
    metal::float4 const coord)
{
    return env.texture->sample_compare(*env.sampler, coord.xy/coord.w, coord.z/coord.w);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjBias_2DShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureProj(
    thread ANGLE_TextureEnv<metal::depth2d<float>> &env,
    metal::float4 const coord,
    float const bias)
{
    return env.texture->sample_compare(*env.sampler, coord.xy/coord.w, coord.z/coord.w, metal::bias(bias));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjGrad_2D_float3,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjGrad(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float3 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.z, metal::gradient2d(dPdx, dPdy));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjGrad_2D_float4,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjGrad(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float4 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.w, metal::gradient2d(dPdx, dPdy));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjGrad_2DShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjGrad(
    thread ANGLE_TextureEnv<metal::depth2d<float>> &env,
    metal::float4 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy)
{
    if (ANGLEUseSampleCompareGradient)
    {
        return env.texture->sample_compare(*env.sampler, coord.xy/coord.w, coord.z/coord.w, metal::gradient2d(dPdx, dPdy));
    }
    else
    {
        const float2 dims = float2(env.texture->get_width(0), env.texture->get_height(0));
        const float lod = 0.5 * metal::log2(metal::max(metal::length_squared(dPdx * dims), metal::length_squared(dPdy * dims)));
        return env.texture->sample_compare(*env.sampler, coord.xy/coord.w, coord.z/coord.w, metal::level(lod));
    }
}
)",
                        functionConstants(),
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjGrad_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjGrad(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::float4 const coord,
    metal::float3 const dPdx,
    metal::float3 const dPdy)
{
    return env.texture->sample(*env.sampler, coord.xyz/coord.w, metal::gradient3d(dPdx, dPdy));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjGradOffset_2D_float3,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjGradOffset(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float3 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy,
    int2 const offset)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.z, metal::gradient2d(dPdx, dPdy), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjGradOffset_2D_float4,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjGradOffset(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float4 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy,
    int2 const offset)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.w, metal::gradient2d(dPdx, dPdy), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjGradOffset_2DShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjGradOffset(
    thread ANGLE_TextureEnv<metal::depth2d<float>> &env,
    metal::float4 const coord,
    metal::float2 const dPdx,
    metal::float2 const dPdy,
    int2 const offset)
{
    if (ANGLEUseSampleCompareGradient)
    {
        return env.texture->sample_compare(*env.sampler, coord.xy/coord.w, coord.z/coord.w, metal::gradient2d(dPdx, dPdy), offset);
    }
    else
    {
        const float2 dims = float2(env.texture->get_width(0), env.texture->get_height(0));
        const float lod = 0.5 * metal::log2(metal::max(metal::length_squared(dPdx * dims), metal::length_squared(dPdy * dims)));
        return env.texture->sample_compare(*env.sampler, coord.xy/coord.w, coord.z/coord.w, metal::level(lod), offset);
    }
}
)",
                        functionConstants(),
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjGradOffset_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjGradOffset(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::float4 const coord,
    metal::float3 const dPdx,
    metal::float3 const dPdy,
    int3 const offset)
{
    return env.texture->sample(*env.sampler, coord.xyz/coord.w, metal::gradient3d(dPdx, dPdy), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjLod_2D_float3,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjLod(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float3 const coord,
    float const level)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.z, metal::level(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjLod_2D_float4,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjLod(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float4 const coord,
    float const level)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.w, metal::level(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjLod_2DShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjLod(
    thread ANGLE_TextureEnv<metal::depth2d<float>> &env,
    metal::float4 const coord,
    float const level)
{
    return env.texture->sample_compare(*env.sampler, coord.xy/coord.w, coord.z/coord.w, metal::level(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjLod_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjLod(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::float4 const coord,
    float const level)
{
    return env.texture->sample(*env.sampler, coord.xyz/coord.w, metal::level(level));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjLodOffset_2D_float3,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjLodOffset(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float3 const coord,
    float const level,
    int2 const offset)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.z, metal::level(level), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjLodOffset_2D_float4,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjLodOffset(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float4 const coord,
    float const level,
    int2 const offset)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.w, metal::level(level), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjLodOffset_2DShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjLodOffset(
    thread ANGLE_TextureEnv<metal::depth2d<float>> &env,
    metal::float4 const coord,
    float const level,
    int2 const offset)
{
    return env.texture->sample_compare(*env.sampler, coord.xy/coord.w, coord.z/coord.w, metal::level(level), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjLodOffset_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjLodOffset(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::float4 const coord,
    float const level,
    int3 const offset)
{
    return env.texture->sample(*env.sampler, coord.xyz/coord.w, metal::level(level), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjOffset_2D_float3,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjOffset(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float3 const coord,
    int2 const offset)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.z, offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjOffsetBias_2D_float3,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjOffset(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float3 const coord,
    int2 const offset,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.z, metal::bias(bias), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjOffset_2D_float4,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjOffset(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float4 const coord,
    int2 const offset)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.w, offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjOffsetBias_2D_float4,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjOffset(
    thread ANGLE_TextureEnv<metal::texture2d<T>> &env,
    metal::float4 const coord,
    int2 const offset,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord.xy/coord.w, metal::bias(bias), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjOffset_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjOffset(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::float4 const coord,
    int3 const offset)
{
    return env.texture->sample(*env.sampler, coord.xyz/coord.w, offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjOffsetBias_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjOffset(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    metal::float4 const coord,
    int3 const offset,
    float const bias)
{
    return env.texture->sample(*env.sampler, coord.xyz/coord.w, metal::bias(bias), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjOffset_2DShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjOffset(
    thread ANGLE_TextureEnv<metal::depth2d<float>> &env,
    metal::float4 const coord,
    int2 const offset)
{
    return env.texture->sample_compare(*env.sampler, coord.xy/coord.w, coord.z/coord.w, offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureProjOffsetBias_2DShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureProjOffset(
    thread ANGLE_TextureEnv<metal::depth2d<float>> &env,
    metal::float4 const coord,
    int2 const offset,
    float const bias)
{
    return env.texture->sample_compare(*env.sampler, coord.xy/coord.w, coord.z/coord.w, metal::bias(bias), offset);
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureSize_2D,
                        R"(
template <typename Texture>
ANGLE_ALWAYS_INLINE auto ANGLE_textureSize(
    thread ANGLE_TextureEnv<Texture> &env,
    int const level)
{
    return int2(env.texture->get_width(uint32_t(level)), env.texture->get_height(uint32_t(level)));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureSize_3D,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureSize(
    thread ANGLE_TextureEnv<metal::texture3d<T>> &env,
    int const level)
{
    return int3(env.texture->get_width(uint32_t(level)), env.texture->get_height(uint32_t(level)), env.texture->get_depth(uint32_t(level)));
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureSize_2DArray,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureSize(
    thread ANGLE_TextureEnv<metal::texture2d_array<T>> &env,
    int const level)
{
    return int3(env.texture->get_width(uint32_t(level)), env.texture->get_height(uint32_t(level)), env.texture->get_array_size());
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureSize_2DArrayShadow,
                        R"(
ANGLE_ALWAYS_INLINE auto ANGLE_textureSize(
    thread ANGLE_TextureEnv<metal::depth2d_array<float>> &env,
    int const level)
{
    return int3(env.texture->get_width(uint32_t(level)), env.texture->get_height(uint32_t(level)), env.texture->get_array_size());
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(textureSize_2DMS,
                        R"(
template <typename T>
ANGLE_ALWAYS_INLINE auto ANGLE_textureSize(
    thread ANGLE_TextureEnv<metal::texture2d_ms<T>> &env)
{
    return int2(env.texture->get_width(), env.texture->get_height());
}
)",
                        textureEnv())

PROGRAM_PRELUDE_DECLARE(imageLoad, R"(
template <typename T, metal::access Access>
ANGLE_ALWAYS_INLINE auto ANGLE_imageLoad(
    thread const metal::texture2d<T, Access> &texture,
    metal::int2 coord)
{
    return texture.read(uint2(coord));
}
)")

PROGRAM_PRELUDE_DECLARE(imageStore, R"(
template <typename T, metal::access Access>
ANGLE_ALWAYS_INLINE auto ANGLE_imageStore(
    thread const metal::texture2d<T, Access> &texture,
    metal::int2 coord,
    metal::vec<T, 4> value)
{
    return texture.write(value, uint2(coord));
}
)")

// TODO(anglebug.com/40096838): When using raster order groups and pixel local storage, which only
// accesses the pixel coordinate, we probably only need an execution barrier (mem_flags::mem_none).
PROGRAM_PRELUDE_DECLARE(memoryBarrierImage, R"(
ANGLE_ALWAYS_INLINE void ANGLE_memoryBarrierImage()
{
    simdgroup_barrier(metal::mem_flags::mem_texture);
}
)")

PROGRAM_PRELUDE_DECLARE(interpolateAtCenter,
                        R"(
template <typename T, typename P>
ANGLE_ALWAYS_INLINE T ANGLE_interpolateAtCenter(
    thread metal::interpolant<T, P> &interpolant)
{
    return interpolant.interpolate_at_center();
}
)")

PROGRAM_PRELUDE_DECLARE(interpolateAtCentroid,
                        R"(
template <typename T, typename P>
ANGLE_ALWAYS_INLINE T ANGLE_interpolateAtCentroid(
    thread metal::interpolant<T, P> &interpolant)
{
    return interpolant.interpolate_at_centroid();
}
template <typename T>
ANGLE_ALWAYS_INLINE T ANGLE_interpolateAtCentroid(T value) { return value; }
)")

PROGRAM_PRELUDE_DECLARE(interpolateAtSample,
                        R"(
template <typename T, typename P>
ANGLE_ALWAYS_INLINE T ANGLE_interpolateAtSample(
    thread metal::interpolant<T, P> &interpolant,
    int const sample)
{
    if (ANGLEMultisampledRendering)
    {
        return interpolant.interpolate_at_sample(static_cast<uint32_t>(sample));
    }
    else
    {
        return interpolant.interpolate_at_center();
    }
}
template <typename T>
ANGLE_ALWAYS_INLINE T ANGLE_interpolateAtSample(T value, int) { return value; }
)")

PROGRAM_PRELUDE_DECLARE(interpolateAtOffset,
                        R"(
template <typename T, typename P>
ANGLE_ALWAYS_INLINE T ANGLE_interpolateAtOffset(
    thread metal::interpolant<T, P> &interpolant,
    float2 const offset)
{
    return interpolant.interpolate_at_offset(metal::saturate(offset + 0.5f));
}
template <typename T>
ANGLE_ALWAYS_INLINE T ANGLE_interpolateAtOffset(T value, float2) { return value; }
)")

////////////////////////////////////////////////////////////////////////////////

// Returned Name is valid for as long as `buffer` is still alive.
// Returns false if no template args exist.
// Returns false if buffer is not large enough.
//
// Example:
//  "foo<1,2>" --> "foo<>"
static std::pair<Name, bool> MaskTemplateArgs(const Name &name, size_t bufferSize, char *buffer)
{
    const char *begin = name.rawName().data();
    const char *end   = strchr(begin, '<');
    if (!end)
    {
        return {{}, false};
    }
    size_t n = end - begin;
    if (n + 3 > bufferSize)
    {
        return {{}, false};
    }
    for (size_t i = 0; i < n; ++i)
    {
        buffer[i] = begin[i];
    }
    buffer[n + 0] = '<';
    buffer[n + 1] = '>';
    buffer[n + 2] = '\0';
    return {Name(buffer, name.symbolType()), true};
}

ProgramPrelude::FuncToEmitter ProgramPrelude::BuildFuncToEmitter()
{
#define EMIT_METHOD(method) \
    [](ProgramPrelude &pp, const TFunction &) -> void { return pp.method(); }
    FuncToEmitter map;

    auto put = [&](Name name, FuncEmitter emitter) {
        FuncEmitter &dest = map[name];
        ASSERT(!dest);
        dest = emitter;
    };

    auto putAngle = [&](const char *nameStr, FuncEmitter emitter) {
        Name name(nameStr, SymbolType::AngleInternal);
        put(name, emitter);
    };

    auto putBuiltIn = [&](const char *nameStr, FuncEmitter emitter) {
        Name name(nameStr, SymbolType::BuiltIn);
        put(name, emitter);
    };

    putAngle("addressof", EMIT_METHOD(addressof));
    putAngle("cast<>", EMIT_METHOD(castMatrix));
    putAngle("elem_ref", EMIT_METHOD(vectorElemRef));
    putAngle("flatten", EMIT_METHOD(flattenArray));
    putAngle("inout", EMIT_METHOD(inout));
    putAngle("out", EMIT_METHOD(out));
    putAngle("swizzle_ref", EMIT_METHOD(swizzleRef));

    putBuiltIn("texelFetch", [](ProgramPrelude &pp, const TFunction &func) {
        switch (func.getParam(0)->getType().getBasicType())
        {
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
                return pp.texelFetch_2D();
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
                return pp.texelFetch_3D();
            case EbtSampler2DArray:
            case EbtISampler2DArray:
            case EbtUSampler2DArray:
                return pp.texelFetch_2DArray();
            case EbtSampler2DMS:
            case EbtISampler2DMS:
            case EbtUSampler2DMS:
                return pp.texelFetch_2DMS();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("texelFetchOffset", [](ProgramPrelude &pp, const TFunction &func) {
        switch (func.getParam(0)->getType().getBasicType())
        {
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
                return pp.texelFetchOffset_2D();
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
                return pp.texelFetchOffset_3D();
            case EbtSampler2DArray:
            case EbtISampler2DArray:
            case EbtUSampler2DArray:
                return pp.texelFetchOffset_2DArray();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("texture", [](ProgramPrelude &pp, const TFunction &func) {
        const bool bias = func.getParamCount() == 3;
        switch (func.getParam(0)->getType().getBasicType())
        {
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
                return bias ? pp.textureBias_2D() : pp.texture_2D();
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
                return bias ? pp.textureBias_3D() : pp.texture_3D();
            case EbtSamplerCube:
            case EbtISamplerCube:
            case EbtUSamplerCube:
                return bias ? pp.textureBias_Cube() : pp.texture_Cube();
            case EbtSampler2DArray:
            case EbtISampler2DArray:
            case EbtUSampler2DArray:
                return bias ? pp.textureBias_2DArray() : pp.texture_2DArray();
            case EbtSampler2DShadow:
                return bias ? pp.textureBias_2DShadow() : pp.texture_2DShadow();
            case EbtSamplerCubeShadow:
                return bias ? pp.textureBias_CubeShadow() : pp.texture_CubeShadow();
            case EbtSampler2DArrayShadow:
                return bias ? pp.textureBias_2DArrayShadow() : pp.texture_2DArrayShadow();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("texture2D", [](ProgramPrelude &pp, const TFunction &func) {
        switch (func.getParamCount())
        {
            case 2:
                return pp.texture2D();
            case 3:
                return pp.texture2DBias();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("texture2DGradEXT", EMIT_METHOD(texture2DGradEXT));
    putBuiltIn("texture2DLod", EMIT_METHOD(texture2DLod));
    putBuiltIn("texture2DLodEXT", EMIT_METHOD(texture2DLodEXT));
    putBuiltIn("texture2DProj", [](ProgramPrelude &pp, const TFunction &func) {
        switch (func.getParamCount())
        {
            case 2:
                return pp.texture2DProj();
            case 3:
                return pp.texture2DProjBias();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("texture2DProjGradEXT", EMIT_METHOD(texture2DProjGradEXT));
    putBuiltIn("texture2DProjLod", EMIT_METHOD(texture2DProjLod));
    putBuiltIn("texture2DProjLodEXT", EMIT_METHOD(texture2DProjLodEXT));
    putBuiltIn("texture3D", [](ProgramPrelude &pp, const TFunction &func) {
        switch (func.getParamCount())
        {
            case 2:
                return pp.texture3D();
            case 3:
                return pp.texture3DBias();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("texture3DLod", EMIT_METHOD(texture3DLod));
    putBuiltIn("texture3DProj", [](ProgramPrelude &pp, const TFunction &func) {
        switch (func.getParamCount())
        {
            case 2:
                return pp.texture3DProj();
            case 3:
                return pp.texture3DProjBias();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("texture3DProjLod", EMIT_METHOD(texture3DProjLod));
    putBuiltIn("textureCube", [](ProgramPrelude &pp, const TFunction &func) {
        switch (func.getParamCount())
        {
            case 2:
                return pp.textureCube();
            case 3:
                return pp.textureCubeBias();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("textureCubeGradEXT", EMIT_METHOD(textureCubeGradEXT));
    putBuiltIn("textureCubeLod", EMIT_METHOD(textureCubeLod));
    putBuiltIn("textureCubeLodEXT", EMIT_METHOD(textureCubeLodEXT));
    putBuiltIn("textureGrad", [](ProgramPrelude &pp, const TFunction &func) {
        switch (func.getParam(0)->getType().getBasicType())
        {
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
                return pp.textureGrad_2D();
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
                return pp.textureGrad_3D();
            case EbtSamplerCube:
            case EbtISamplerCube:
            case EbtUSamplerCube:
                return pp.textureGrad_Cube();
            case EbtSampler2DArray:
            case EbtISampler2DArray:
            case EbtUSampler2DArray:
                return pp.textureGrad_2DArray();
            case EbtSampler2DShadow:
                return pp.textureGrad_2DShadow();
            case EbtSamplerCubeShadow:
                return pp.textureGrad_CubeShadow();
            case EbtSampler2DArrayShadow:
                return pp.textureGrad_2DArrayShadow();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("textureGradOffset", [](ProgramPrelude &pp, const TFunction &func) {
        switch (func.getParam(0)->getType().getBasicType())
        {
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
                return pp.textureGradOffset_2D();
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
                return pp.textureGradOffset_3D();
            case EbtSampler2DArray:
            case EbtISampler2DArray:
            case EbtUSampler2DArray:
                return pp.textureGradOffset_2DArray();
            case EbtSampler2DShadow:
                return pp.textureGradOffset_2DShadow();
            case EbtSampler2DArrayShadow:
                return pp.textureGradOffset_2DArrayShadow();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("textureLod", [](ProgramPrelude &pp, const TFunction &func) {
        switch (func.getParam(0)->getType().getBasicType())
        {
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
                return pp.textureLod_2D();
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
                return pp.textureLod_3D();
            case EbtSamplerCube:
            case EbtISamplerCube:
            case EbtUSamplerCube:
                return pp.textureLod_Cube();
            case EbtSampler2DArray:
            case EbtISampler2DArray:
            case EbtUSampler2DArray:
                return pp.textureLod_2DArray();
            case EbtSampler2DShadow:
                return pp.textureLod_2DShadow();
            case EbtSamplerCubeShadow:
                return pp.textureLod_CubeShadow();
            case EbtSampler2DArrayShadow:
                return pp.textureLod_2DArrayShadow();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("textureLodOffset", [](ProgramPrelude &pp, const TFunction &func) {
        switch (func.getParam(0)->getType().getBasicType())
        {
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
                return pp.textureLodOffset_2D();
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
                return pp.textureLodOffset_3D();
            case EbtSampler2DArray:
            case EbtISampler2DArray:
            case EbtUSampler2DArray:
                return pp.textureLodOffset_2DArray();
            case EbtSampler2DShadow:
                return pp.textureLodOffset_2DShadow();
            case EbtSampler2DArrayShadow:
                return pp.textureLodOffset_2DArrayShadow();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("textureOffset", [](ProgramPrelude &pp, const TFunction &func) {
        const bool bias = func.getParamCount() == 4;
        switch (func.getParam(0)->getType().getBasicType())
        {
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
                return bias ? pp.textureOffsetBias_2D() : pp.textureOffset_2D();
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
                return bias ? pp.textureOffsetBias_3D() : pp.textureOffset_3D();
            case EbtSampler2DArray:
            case EbtISampler2DArray:
            case EbtUSampler2DArray:
                return bias ? pp.textureOffsetBias_2DArray() : pp.textureOffset_2DArray();
            case EbtSampler2DShadow:
                return bias ? pp.textureOffsetBias_2DShadow() : pp.textureOffset_2DShadow();
            case EbtSampler2DArrayShadow:
                return bias ? pp.textureOffsetBias_2DArrayShadow()
                            : pp.textureOffset_2DArrayShadow();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("textureProj", [](ProgramPrelude &pp, const TFunction &func) {
        const bool bias = func.getParamCount() == 3;
        switch (func.getParam(0)->getType().getBasicType())
        {
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
                return func.getParam(1)->getType().getNominalSize() == 4
                           ? (bias ? pp.textureProjBias_2D_float4() : pp.textureProj_2D_float4())
                           : (bias ? pp.textureProjBias_2D_float3() : pp.textureProj_2D_float3());
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
                return bias ? pp.textureProjBias_3D() : pp.textureProj_3D();
            case EbtSampler2DShadow:
                return bias ? pp.textureProjBias_2DShadow() : pp.textureProj_2DShadow();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("textureProjGrad", [](ProgramPrelude &pp, const TFunction &func) {
        switch (func.getParam(0)->getType().getBasicType())
        {
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
                return func.getParam(1)->getType().getNominalSize() == 4
                           ? pp.textureProjGrad_2D_float4()
                           : pp.textureProjGrad_2D_float3();
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
                return pp.textureProjGrad_3D();
            case EbtSampler2DShadow:
                return pp.textureProjGrad_2DShadow();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("textureProjGradOffset", [](ProgramPrelude &pp, const TFunction &func) {
        switch (func.getParam(0)->getType().getBasicType())
        {
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
                return func.getParam(1)->getType().getNominalSize() == 4
                           ? pp.textureProjGradOffset_2D_float4()
                           : pp.textureProjGradOffset_2D_float3();
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
                return pp.textureProjGradOffset_3D();
            case EbtSampler2DShadow:
                return pp.textureProjGradOffset_2DShadow();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("textureProjLod", [](ProgramPrelude &pp, const TFunction &func) {
        switch (func.getParam(0)->getType().getBasicType())
        {
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
                return func.getParam(1)->getType().getNominalSize() == 4
                           ? pp.textureProjLod_2D_float4()
                           : pp.textureProjLod_2D_float3();
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
                return pp.textureProjLod_3D();
            case EbtSampler2DShadow:
                return pp.textureProjLod_2DShadow();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("textureProjLodOffset", [](ProgramPrelude &pp, const TFunction &func) {
        switch (func.getParam(0)->getType().getBasicType())
        {
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
                return func.getParam(1)->getType().getNominalSize() == 4
                           ? pp.textureProjLodOffset_2D_float4()
                           : pp.textureProjLodOffset_2D_float3();
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
                return pp.textureProjLodOffset_3D();
            case EbtSampler2DShadow:
                return pp.textureProjLodOffset_2DShadow();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("textureProjOffset", [](ProgramPrelude &pp, const TFunction &func) {
        const bool bias = func.getParamCount() == 4;
        switch (func.getParam(0)->getType().getBasicType())
        {
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
                return func.getParam(1)->getType().getNominalSize() == 4
                           ? (bias ? pp.textureProjOffsetBias_2D_float4()
                                   : pp.textureProjOffset_2D_float4())
                           : (bias ? pp.textureProjOffsetBias_2D_float3()
                                   : pp.textureProjOffset_2D_float3());
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
                return bias ? pp.textureProjOffsetBias_3D() : pp.textureProjOffset_3D();
            case EbtSampler2DShadow:
                return bias ? pp.textureProjOffsetBias_2DShadow() : pp.textureProjOffset_2DShadow();
            default:
                UNREACHABLE();
        }
    });
    putBuiltIn("textureSize", [](ProgramPrelude &pp, const TFunction &func) {
        switch (func.getParam(0)->getType().getBasicType())
        {
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
                return pp.textureSize_3D();
            case EbtSampler2DArray:
            case EbtISampler2DArray:
            case EbtUSampler2DArray:
                return pp.textureSize_2DArray();
            case EbtSampler2DArrayShadow:
                return pp.textureSize_2DArrayShadow();
            case EbtSampler2DMS:
            case EbtISampler2DMS:
            case EbtUSampler2DMS:
                return pp.textureSize_2DMS();
            default:
                // Same wrapper for 2D, 2D Shadow, Cube, and Cube Shadow
                return pp.textureSize_2D();
        }
    });
    putBuiltIn("imageLoad", EMIT_METHOD(imageLoad));
    putBuiltIn("imageStore", EMIT_METHOD(imageStore));
    putBuiltIn("memoryBarrierImage", EMIT_METHOD(memoryBarrierImage));

    putBuiltIn("interpolateAtCenter", EMIT_METHOD(interpolateAtCenter));
    putBuiltIn("interpolateAtCentroid", EMIT_METHOD(interpolateAtCentroid));
    putBuiltIn("interpolateAtSample", EMIT_METHOD(interpolateAtSample));
    putBuiltIn("interpolateAtOffset", EMIT_METHOD(interpolateAtOffset));

    return map;

#undef EMIT_METHOD
}

void ProgramPrelude::visitOperator(TOperator op, const TFunction *func, const TType *argType0)
{
    visitOperator(op, func, argType0, nullptr, nullptr);
}

void ProgramPrelude::visitOperator(TOperator op,
                                   const TFunction *func,
                                   const TType *argType0,
                                   const TType *argType1)
{
    visitOperator(op, func, argType0, argType1, nullptr);
}
void ProgramPrelude::visitOperator(TOperator op,
                                   const TFunction *func,
                                   const TType *argType0,
                                   const TType *argType1,
                                   const TType *argType2)
{
    switch (op)
    {
        case TOperator::EOpRadians:
            radians();
            break;
        case TOperator::EOpDegrees:
            degrees();
            break;
        case TOperator::EOpMod:
            mod();
            break;
        case TOperator::EOpRefract:
            if (argType0->isScalar())
            {
                refractScalar();
            }
            break;
        case TOperator::EOpDistance:
            if (argType0->isScalar())
            {
                distanceScalar();
            }
            break;
        case TOperator::EOpLength:
        case TOperator::EOpDot:
        case TOperator::EOpNormalize:
            break;
        case TOperator::EOpFaceforward:
            if (argType0->isScalar())
            {
                faceforwardScalar();
            }
            break;
        case TOperator::EOpReflect:
            if (argType0->isScalar())
            {
                reflectScalar();
            }
            break;

        case TOperator::EOpSin:
        case TOperator::EOpCos:
        case TOperator::EOpTan:
        case TOperator::EOpAsin:
        case TOperator::EOpAcos:
        case TOperator::EOpAtan:
        case TOperator::EOpSinh:
        case TOperator::EOpCosh:
        case TOperator::EOpTanh:
        case TOperator::EOpAsinh:
        case TOperator::EOpAcosh:
        case TOperator::EOpAtanh:
        case TOperator::EOpAbs:
        case TOperator::EOpFma:
        case TOperator::EOpPow:
        case TOperator::EOpExp:
        case TOperator::EOpExp2:
        case TOperator::EOpLog:
        case TOperator::EOpLog2:
        case TOperator::EOpSqrt:
        case TOperator::EOpFloor:
        case TOperator::EOpTrunc:
        case TOperator::EOpCeil:
        case TOperator::EOpFract:
        case TOperator::EOpRound:
        case TOperator::EOpRoundEven:
        case TOperator::EOpSaturate:
        case TOperator::EOpModf:
        case TOperator::EOpLdexp:
        case TOperator::EOpFrexp:
        case TOperator::EOpInversesqrt:
            break;

        case TOperator::EOpEqual:
            if (argType0->isVector() && argType1->isVector())
            {
                equalVector();
            }
            // Even if Arg0 is a vector or matrix, it could also be an array.
            if (argType0->isArray() && argType1->isArray())
            {
                equalArray();
            }
            if (argType0->getStruct() && argType1->getStruct() && argType0->isArray() &&
                argType1->isArray())
            {
                equalStructArray();
            }
            if (argType0->isMatrix() && argType1->isMatrix())
            {
                equalMatrix();
            }
            break;

        case TOperator::EOpNotEqual:
            if (argType0->isVector() && argType1->isVector())
            {
                notEqualVector();
            }
            else if (argType0->getStruct() && argType1->getStruct())
            {
                notEqualStruct();
            }
            // Same as above.
            if (argType0->isArray() && argType1->isArray())
            {
                notEqualArray();
            }
            if (argType0->getStruct() && argType1->getStruct() && argType0->isArray() &&
                argType1->isArray())
            {
                notEqualStructArray();
            }
            if (argType0->isMatrix() && argType1->isMatrix())
            {
                notEqualMatrix();
            }
            break;

        case TOperator::EOpCross:
            break;

        case TOperator::EOpSign:
            if (argType0->getBasicType() == TBasicType::EbtInt)
            {
                signInt();
            }
            break;

        case TOperator::EOpClamp:
        case TOperator::EOpMin:
        case TOperator::EOpMax:
        case TOperator::EOpStep:
        case TOperator::EOpSmoothstep:
            break;
        case TOperator::EOpMix:
            if (argType2->getBasicType() == TBasicType::EbtBool)
            {
                mixBool();
            }
            break;

        case TOperator::EOpAll:
        case TOperator::EOpAny:
        case TOperator::EOpIsnan:
        case TOperator::EOpIsinf:
        case TOperator::EOpDFdx:
        case TOperator::EOpDFdy:
        case TOperator::EOpFwidth:
        case TOperator::EOpTranspose:
        case TOperator::EOpDeterminant:
            break;

        case TOperator::EOpAdd:
            if (argType0->isMatrix() && argType1->isScalar())
            {
                addMatrixScalar();
            }
            if (argType0->isScalar() && argType1->isMatrix())
            {
                addScalarMatrix();
            }
            break;

        case TOperator::EOpAddAssign:
            if (argType0->isMatrix() && argType1->isScalar())
            {
                addMatrixScalarAssign();
            }
            break;

        case TOperator::EOpSub:
            if (argType0->isMatrix() && argType1->isScalar())
            {
                subMatrixScalar();
            }
            if (argType0->isScalar() && argType1->isMatrix())
            {
                subScalarMatrix();
            }
            break;

        case TOperator::EOpSubAssign:
            if (argType0->isMatrix() && argType1->isScalar())
            {
                subMatrixScalarAssign();
            }
            break;

        case TOperator::EOpDiv:
            if (argType0->isMatrix())
            {
                if (argType1->isMatrix())
                {
                    componentWiseDivide();
                }
                else if (argType1->isScalar())
                {
                    divMatrixScalar();
                }
            }
            if (argType0->isScalar() && argType1->isMatrix())
            {
                divScalarMatrix();
            }
            break;

        case TOperator::EOpDivAssign:
            if (argType0->isMatrix())
            {
                if (argType1->isMatrix())
                {
                    componentWiseDivideAssign();
                }
                else if (argType1->isScalar())
                {
                    divMatrixScalarAssign();
                }
            }
            break;

        case TOperator::EOpMatrixCompMult:
            if (argType0->isMatrix() && argType1->isMatrix())
            {
                componentWiseMultiply();
            }
            break;

        case TOperator::EOpOuterProduct:
            outerProduct();
            break;

        case TOperator::EOpInverse:
            switch (argType0->getCols())
            {
                case 2:
                    inverse2();
                    break;
                case 3:
                    inverse3();
                    break;
                case 4:
                    inverse4();
                    break;
                default:
                    UNREACHABLE();
            }
            break;

        case TOperator::EOpMatrixTimesMatrixAssign:
            matmulAssign();
            break;

        case TOperator::EOpPreIncrement:
            if (argType0->isMatrix())
            {
                preIncrementMatrix();
            }
            break;

        case TOperator::EOpPostIncrement:
            if (argType0->isMatrix())
            {
                postIncrementMatrix();
            }
            break;

        case TOperator::EOpPreDecrement:
            if (argType0->isMatrix())
            {
                preDecrementMatrix();
            }
            break;

        case TOperator::EOpPostDecrement:
            if (argType0->isMatrix())
            {
                postDecrementMatrix();
            }
            break;

        case TOperator::EOpNegative:
            if (argType0->isMatrix())
            {
                negateMatrix();
            }
            break;

        case TOperator::EOpComma:
        case TOperator::EOpAssign:
        case TOperator::EOpInitialize:
        case TOperator::EOpMulAssign:
        case TOperator::EOpIModAssign:
        case TOperator::EOpBitShiftLeftAssign:
        case TOperator::EOpBitShiftRightAssign:
        case TOperator::EOpBitwiseAndAssign:
        case TOperator::EOpBitwiseXorAssign:
        case TOperator::EOpBitwiseOrAssign:
        case TOperator::EOpMul:
        case TOperator::EOpIMod:
        case TOperator::EOpBitShiftLeft:
        case TOperator::EOpBitShiftRight:
        case TOperator::EOpBitwiseAnd:
        case TOperator::EOpBitwiseXor:
        case TOperator::EOpBitwiseOr:
        case TOperator::EOpLessThan:
        case TOperator::EOpGreaterThan:
        case TOperator::EOpLessThanEqual:
        case TOperator::EOpGreaterThanEqual:
        case TOperator::EOpLessThanComponentWise:
        case TOperator::EOpLessThanEqualComponentWise:
        case TOperator::EOpGreaterThanEqualComponentWise:
        case TOperator::EOpGreaterThanComponentWise:
        case TOperator::EOpLogicalOr:
        case TOperator::EOpLogicalXor:
        case TOperator::EOpLogicalAnd:
        case TOperator::EOpPositive:
        case TOperator::EOpLogicalNot:
        case TOperator::EOpNotComponentWise:
        case TOperator::EOpBitwiseNot:
        case TOperator::EOpVectorTimesScalarAssign:
        case TOperator::EOpVectorTimesMatrixAssign:
        case TOperator::EOpMatrixTimesScalarAssign:
        case TOperator::EOpVectorTimesScalar:
        case TOperator::EOpVectorTimesMatrix:
        case TOperator::EOpMatrixTimesVector:
        case TOperator::EOpMatrixTimesScalar:
        case TOperator::EOpMatrixTimesMatrix:
        case TOperator::EOpReturn:
        case TOperator::EOpBreak:
        case TOperator::EOpContinue:
        case TOperator::EOpEqualComponentWise:
        case TOperator::EOpNotEqualComponentWise:
        case TOperator::EOpIndexDirect:
        case TOperator::EOpIndexIndirect:
        case TOperator::EOpIndexDirectStruct:
        case TOperator::EOpIndexDirectInterfaceBlock:
        case TOperator::EOpFloatBitsToInt:
        case TOperator::EOpIntBitsToFloat:
        case TOperator::EOpUintBitsToFloat:
        case TOperator::EOpFloatBitsToUint:
        case TOperator::EOpNull:
        case TOperator::EOpKill:
        case TOperator::EOpPackUnorm2x16:
        case TOperator::EOpPackSnorm2x16:
        case TOperator::EOpPackUnorm4x8:
        case TOperator::EOpPackSnorm4x8:
        case TOperator::EOpUnpackSnorm2x16:
        case TOperator::EOpUnpackUnorm2x16:
        case TOperator::EOpUnpackUnorm4x8:
        case TOperator::EOpUnpackSnorm4x8:
            break;

        case TOperator::EOpPackHalf2x16:
            pack_half_2x16();
            break;
        case TOperator::EOpUnpackHalf2x16:
            unpack_half_2x16();
            break;

        case TOperator::EOpBitfieldExtract:
        case TOperator::EOpBitfieldInsert:
        case TOperator::EOpBitfieldReverse:
        case TOperator::EOpBitCount:
        case TOperator::EOpFindLSB:
        case TOperator::EOpFindMSB:
        case TOperator::EOpUaddCarry:
        case TOperator::EOpUsubBorrow:
        case TOperator::EOpUmulExtended:
        case TOperator::EOpImulExtended:
        case TOperator::EOpBarrier:
        case TOperator::EOpMemoryBarrier:
        case TOperator::EOpMemoryBarrierAtomicCounter:
        case TOperator::EOpMemoryBarrierBuffer:
        case TOperator::EOpMemoryBarrierShared:
        case TOperator::EOpGroupMemoryBarrier:
        case TOperator::EOpAtomicAdd:
        case TOperator::EOpAtomicMin:
        case TOperator::EOpAtomicMax:
        case TOperator::EOpAtomicAnd:
        case TOperator::EOpAtomicOr:
        case TOperator::EOpAtomicXor:
        case TOperator::EOpAtomicExchange:
        case TOperator::EOpAtomicCompSwap:
        case TOperator::EOpEmitVertex:
        case TOperator::EOpEndPrimitive:
        case TOperator::EOpArrayLength:
            UNIMPLEMENTED();
            break;

        case TOperator::EOpConstruct:
            ASSERT(!func);
            break;

        case TOperator::EOpCallFunctionInAST:
        case TOperator::EOpCallInternalRawFunction:
        default:
            ASSERT(func);
            if (mHandled.insert(func).second)
            {
                const Name name(*func);
                const auto end = mFuncToEmitter.end();
                auto iter      = mFuncToEmitter.find(name);
                if (iter == end)
                {
                    char buffer[32];
                    auto mask = MaskTemplateArgs(name, sizeof(buffer), buffer);
                    if (mask.second)
                    {
                        iter = mFuncToEmitter.find(mask.first);
                    }
                }
                if (iter != end)
                {
                    const auto &emitter = iter->second;
                    emitter(*this, *func);
                }
            }
            break;
    }
}

void ProgramPrelude::visitVariable(const Name &name, const TType &type)
{
    if (const TStructure *s = type.getStruct())
    {
        const Name typeName(*s);
        if (typeName.beginsWith(Name("TextureEnv<")))
        {
            textureEnv();
        }
    }
    else
    {
        if (name.rawName() == sh::mtl::kRasterizerDiscardEnabledConstName ||
            name.rawName() == sh::mtl::kDepthWriteEnabledConstName ||
            name.rawName() == sh::mtl::kEmulateAlphaToCoverageConstName)
        {
            functionConstants();
        }
    }
}

void ProgramPrelude::visitVariable(const TVariable &var)
{
    if (mHandled.insert(&var).second)
    {
        visitVariable(Name(var), var.getType());
    }
}

void ProgramPrelude::visitStructure(const TStructure &s)
{
    if (mHandled.insert(&s).second)
    {
        for (const TField *field : s.fields())
        {
            const TType &type = *field->type();
            visitVariable(Name(*field), type);
        }
    }
}

bool ProgramPrelude::visitBinary(Visit visit, TIntermBinary *node)
{
    const TType &leftType  = node->getLeft()->getType();
    const TType &rightType = node->getRight()->getType();
    visitOperator(node->getOp(), nullptr, &leftType, &rightType);
    return true;
}

bool ProgramPrelude::visitUnary(Visit visit, TIntermUnary *node)
{
    const TType &argType = node->getOperand()->getType();
    visitOperator(node->getOp(), nullptr, &argType);
    return true;
}

bool ProgramPrelude::visitAggregate(Visit visit, TIntermAggregate *node)
{
    const size_t argCount = node->getChildCount();

    auto getArgType = [node, argCount](size_t index) -> const TType & {
        ASSERT(index < argCount);
        TIntermTyped *arg = node->getChildNode(index)->getAsTyped();
        ASSERT(arg);
        return arg->getType();
    };

    const TFunction *func = node->getFunction();

    switch (node->getChildCount())
    {
        case 0:
        {
            visitOperator(node->getOp(), func, nullptr);
        }
        break;

        case 1:
        {
            const TType &argType0 = getArgType(0);
            visitOperator(node->getOp(), func, &argType0);
        }
        break;

        case 2:
        {
            const TType &argType0 = getArgType(0);
            const TType &argType1 = getArgType(1);
            visitOperator(node->getOp(), func, &argType0, &argType1);
        }
        break;

        case 3:
        {
            const TType &argType0 = getArgType(0);
            const TType &argType1 = getArgType(1);
            const TType &argType2 = getArgType(2);
            visitOperator(node->getOp(), func, &argType0, &argType1, &argType2);
        }
        break;

        default:
        {
            const TType &argType0 = getArgType(0);
            const TType &argType1 = getArgType(1);
            visitOperator(node->getOp(), func, &argType0, &argType1);
        }
        break;
    }

    return true;
}

bool ProgramPrelude::visitDeclaration(Visit, TIntermDeclaration *node)
{
    Declaration decl  = ViewDeclaration(*node);
    const TType &type = decl.symbol.getType();
    if (type.isStructSpecifier())
    {
        const TStructure *s = type.getStruct();
        ASSERT(s);
        visitStructure(*s);
    }
    return true;
}

void ProgramPrelude::visitSymbol(TIntermSymbol *node)
{
    visitVariable(node->variable());
}

bool sh::EmitProgramPrelude(TIntermBlock &root, TInfoSinkBase &out, const ProgramPreludeConfig &ppc)
{
    ProgramPrelude programPrelude(out, ppc);
    root.traverse(&programPrelude);
    return true;
}
