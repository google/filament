// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "gmock/gmock.h"

#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/msl/writer/helper_test.h"
#include "src/tint/utils/text/string.h"

namespace tint::msl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(MslWriterTest, EmitType_Array) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, ty.array<bool, 4>()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + MetalArray() + R"(
void foo() {
  tint_array<bool, 4> a = {};
}
)");
}

TEST_F(MslWriterTest, EmitType_ArrayOfArray) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, ty.array(ty.array<bool, 4>(), 5)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + MetalArray() + R"(
void foo() {
  tint_array<tint_array<bool, 4>, 5> a = {};
}
)");
}

TEST_F(MslWriterTest, EmitType_ArrayOfArrayOfArray) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a",
              ty.ptr(core::AddressSpace::kFunction, ty.array(ty.array(ty.array<bool, 4>(), 5), 6)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + MetalArray() + R"(
void foo() {
  tint_array<tint_array<tint_array<bool, 4>, 5>, 6> a = {};
}
)");
}

TEST_F(MslWriterTest, EmitType_RuntimeArray) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, ty.array<bool, 0>()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + MetalArray() + R"(
void foo() {
  tint_array<bool, 1> a = {};
}
)");
}

TEST_F(MslWriterTest, EmitType_Bool) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, ty.bool_()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  bool a = false;
}
)");
}

TEST_F(MslWriterTest, EmitType_F32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, ty.f32()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float a = 0.0f;
}
)");
}

TEST_F(MslWriterTest, EmitType_F16) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, ty.f16()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  half a = 0.0h;
}
)");
}

TEST_F(MslWriterTest, EmitType_I32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, ty.i32()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  int a = 0;
}
)");
}

TEST_F(MslWriterTest, EmitType_Matrix_F32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, ty.mat2x3<f32>()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float2x3 a = float2x3(0.0f);
}
)");
}

TEST_F(MslWriterTest, EmitType_Matrix_F16) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, ty.mat2x3<f16>()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  half2x3 a = half2x3(0.0h);
}
)");
}
TEST_F(MslWriterTest, EmitType_U32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, ty.u32()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  uint a = 0u;
}
)");
}

TEST_F(MslWriterTest, EmitType_U64) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, ty.u64()));
        b.Return(func);
    });

    // Use `Print()` as u64 types are only support after certain transforms have run.
    ASSERT_TRUE(Print()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  ulong a = 0u;
}
)");
}

TEST_F(MslWriterTest, EmitType_Atomic_U32) {
    auto* func = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("a", ty.ptr(core::AddressSpace::kWorkgroup, ty.atomic<u32>()));
    func->SetParams({param});
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo(threadgroup atomic_uint* const a) {
}
)");
}

TEST_F(MslWriterTest, EmitType_Atomic_I32) {
    auto* func = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("a", ty.ptr(core::AddressSpace::kWorkgroup, ty.atomic<i32>()));
    func->SetParams({param});
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo(threadgroup atomic_int* const a) {
}
)");
}

TEST_F(MslWriterTest, EmitType_Vector) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, ty.vec3<f32>()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float3 a = 0.0f;
}
)");
}

TEST_F(MslWriterTest, EmitType_VectorPacked) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, ty.packed_vec(ty.f32(), 3)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  packed_float3 a = 0.0f;
}
)");
}

TEST_F(MslWriterTest, EmitType_Void) {
    // Tested via the function return type.
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
}
)");
}

TEST_F(MslWriterTest, EmitType_Pointer_Workgroup) {
    auto* func = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("param", ty.ptr<workgroup, f32>());
    func->SetParams({param});
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo(threadgroup float* const param) {
}
)");
}

TEST_F(MslWriterTest, EmitType_Pointer_Const) {
    auto* func = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("param", ty.ptr<storage, i32, read>());
    func->SetParams({param});
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo(const device int* const param) {
}
)");
}

TEST_F(MslWriterTest, EmitType_Struct) {
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("a"), ty.i32()},
                                                  {mod.symbols.Register("b"), ty.f32()},
                                              });
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, s));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct S {
  int a;
  float b;
};

void foo() {
  S a = {};
}
)");
}

TEST_F(MslWriterTest, EmitType_Struct_Dedup) {
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("a"), ty.i32()},
                                                  {mod.symbols.Register("b"), ty.f32()},
                                              });
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, s));
        b.Var("b", ty.ptr(core::AddressSpace::kFunction, s));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct S {
  int a;
  float b;
};

void foo() {
  S a = {};
  S b = {};
}
)");
}

void FormatMSLField(StringStream& out,
                    const char* addr,
                    const char* type,
                    size_t array_count,
                    const char* name) {
    out << "  /* " << std::string(addr) << " */ ";
    if (array_count == 0) {
        out << type << " ";
    } else {
        out << "tint_array<" << type << ", " << std::to_string(array_count) << "> ";
    }
    out << name << ";\n";
}

#define CHECK_TYPE_SIZE_AND_ALIGN(TYPE, SIZE, ALIGN)      \
    static_assert(sizeof(TYPE) == SIZE, "Bad type size"); \
    static_assert(alignof(TYPE) == ALIGN, "Bad type alignment")

// Declare C++ types that match the size and alignment of the types of the same
// name in MSL.
#define DECLARE_TYPE(NAME, SIZE, ALIGN) \
    struct alignas(ALIGN) NAME {        \
        uint8_t _[SIZE];                \
    };                                  \
    CHECK_TYPE_SIZE_AND_ALIGN(NAME, SIZE, ALIGN)

// Size and alignments taken from the MSL spec:
// https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
DECLARE_TYPE(float2, 8, 8);
DECLARE_TYPE(float3, 12, 4);
DECLARE_TYPE(float4, 16, 16);
DECLARE_TYPE(float2x2, 16, 8);
DECLARE_TYPE(float2x3, 32, 16);
DECLARE_TYPE(float2x4, 32, 16);
DECLARE_TYPE(float3x2, 24, 8);
DECLARE_TYPE(float3x3, 48, 16);
DECLARE_TYPE(float3x4, 48, 16);
DECLARE_TYPE(float4x2, 32, 8);
DECLARE_TYPE(float4x3, 64, 16);
DECLARE_TYPE(float4x4, 64, 16);
DECLARE_TYPE(half2, 4, 4);
DECLARE_TYPE(packed_half3, 6, 2);
DECLARE_TYPE(half4, 8, 8);
DECLARE_TYPE(half2x2, 8, 4);
DECLARE_TYPE(half2x3, 16, 8);
DECLARE_TYPE(half2x4, 16, 8);
DECLARE_TYPE(half3x2, 12, 4);
DECLARE_TYPE(half3x3, 24, 8);
DECLARE_TYPE(half3x4, 24, 8);
DECLARE_TYPE(half4x2, 16, 4);
DECLARE_TYPE(half4x3, 32, 8);
DECLARE_TYPE(half4x4, 32, 8);
using uint = unsigned int;

struct MemberData {
    Symbol name;
    const core::type::Type* type;
    uint32_t size = 0;
    uint32_t align = 0;
};
core::type::Struct* MkStruct(core::ir::Module& mod,
                             core::type::Manager& ty,
                             std::string_view name,
                             VectorRef<MemberData> data) {
    Vector<const core::type::StructMember*, 26> members;
    uint32_t align = 0;
    uint32_t size = 0;
    for (uint32_t i = 0; i < data.Length(); ++i) {
        auto& d = data[i];

        uint32_t mem_align = d.align == 0 ? d.type->Align() : d.align;
        uint32_t mem_size = d.size == 0 ? d.type->Size() : d.size;

        uint32_t offset = tint::RoundUp(mem_align, size);
        members.Push(ty.Get<core::type::StructMember>(d.name, d.type, i, offset, mem_align,
                                                      mem_size, core::IOAttributes{}));

        align = std::max(align, mem_align);
        size = offset + mem_size;
    }

    return ty.Get<core::type::Struct>(mod.symbols.New(name), std::move(members), align,
                                      tint::RoundUp(align, size), size);
}

TEST_F(MslWriterTest, EmitType_Struct_Layout_NumericTypes) {
    // Note: Skip vec3 and matCx3 types here as they have need special treatment.
    Vector<MemberData, 26> data = {{mod.symbols.Register("a"), ty.i32(), 32},        //
                                   {mod.symbols.Register("b"), ty.f32(), 128, 128},  //
                                   {mod.symbols.Register("c"), ty.vec2<f32>()},      //
                                   {mod.symbols.Register("d"), ty.u32()},            //
                                   {mod.symbols.Register("e"), ty.vec4<f32>()},      //
                                   {mod.symbols.Register("f"), ty.u32()},            //
                                   {mod.symbols.Register("g"), ty.mat2x2<f32>()},    //
                                   {mod.symbols.Register("h"), ty.u32()},            //
                                   {mod.symbols.Register("i"), ty.mat2x4<f32>()},    //
                                   {mod.symbols.Register("j"), ty.u32()},            //
                                   {mod.symbols.Register("k"), ty.mat3x2<f32>()},    //
                                   {mod.symbols.Register("l"), ty.u32()},            //
                                   {mod.symbols.Register("m"), ty.mat3x4<f32>()},    //
                                   {mod.symbols.Register("n"), ty.u32()},            //
                                   {mod.symbols.Register("o"), ty.mat4x2<f32>()},    //
                                   {mod.symbols.Register("p"), ty.u32()},            //
                                   {mod.symbols.Register("q"), ty.mat4x4<f32>()},    //
                                   {mod.symbols.Register("r"), ty.f32()}};

    auto* s = MkStruct(mod, ty, "S", data);

    // ALL_FIELDS() calls the macro FIELD(ADDR, TYPE, ARRAY_COUNT, NAME)
    // for each field of the structure s.
#define ALL_FIELDS()                       \
    FIELD(0x0000, int, 0, a)               \
    FIELD(0x0004, int8_t, 124, tint_pad)   \
    FIELD(0x0080, float, 0, b)             \
    FIELD(0x0084, int8_t, 124, tint_pad_1) \
    FIELD(0x0100, float2, 0, c)            \
    FIELD(0x0108, uint, 0, d)              \
    FIELD(0x010c, int8_t, 4, tint_pad_2)   \
    FIELD(0x0110, float4, 0, e)            \
    FIELD(0x0120, uint, 0, f)              \
    FIELD(0x0124, int8_t, 4, tint_pad_3)   \
    FIELD(0x0128, float2x2, 0, g)          \
    FIELD(0x0138, uint, 0, h)              \
    FIELD(0x013c, int8_t, 4, tint_pad_4)   \
    FIELD(0x0140, float2x4, 0, i)          \
    FIELD(0x0160, uint, 0, j)              \
    FIELD(0x0164, int8_t, 4, tint_pad_5)   \
    FIELD(0x0168, float3x2, 0, k)          \
    FIELD(0x0180, uint, 0, l)              \
    FIELD(0x0184, int8_t, 12, tint_pad_6)  \
    FIELD(0x0190, float3x4, 0, m)          \
    FIELD(0x01c0, uint, 0, n)              \
    FIELD(0x01c4, int8_t, 4, tint_pad_7)   \
    FIELD(0x01c8, float4x2, 0, o)          \
    FIELD(0x01e8, uint, 0, p)              \
    FIELD(0x01ec, int8_t, 4, tint_pad_8)   \
    FIELD(0x01f0, float4x4, 0, q)          \
    FIELD(0x0230, float, 0, r)             \
    FIELD(0x0234, int8_t, 76, tint_pad_9)

    // Check that the generated string is as expected.
    StringStream expect;
    expect << "struct S {\n";
#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) \
    FormatMSLField(expect, #ADDR, #TYPE, ARRAY_COUNT, #NAME);
    ALL_FIELDS()
#undef FIELD
    expect << R"(};
)";

    auto* var = b.Var("a", ty.ptr(core::AddressSpace::kStorage, s));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Load(var);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_THAT(output_.msl, testing::HasSubstr(expect.str())) << output_.msl;

    // 1.4 Metal and C++14
    // The Metal programming language is a C++14-based Specification with
    // extensions and restrictions. Refer to the C++14 Specification (also
    // known as the ISO/IEC JTC1/SC22/WG21 N4431 Language Specification) for a
    // detailed description of the language grammar.
    //
    // Tint is written in C++14, so use the compiler to verify the generated
    // layout is as expected for C++14 / MSL.
    {
        struct S {
#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) std::array<TYPE, ARRAY_COUNT ? ARRAY_COUNT : 1> NAME;
            ALL_FIELDS()
#undef FIELD
        };

#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) \
    EXPECT_EQ(ADDR, static_cast<int>(offsetof(S, NAME))) << "Field " << #NAME;
        ALL_FIELDS()
#undef FIELD
    }
#undef ALL_FIELDS
}

TEST_F(MslWriterTest, EmitType_Struct_Layout_Structures) {
    // inner_x: size(1024), align(512)
    Vector<MemberData, 2> inner_x_data = {{{mod.symbols.Register("a"), ty.i32()},  //
                                           {mod.symbols.Register("b"), ty.f32(), 0, 512}}};
    auto* inner_x = MkStruct(mod, ty, "inner_x", inner_x_data);

    // inner_y: size(516), align(4)
    Vector<MemberData, 2> inner_y_data = {{mod.symbols.Register("a"), ty.i32(), 512},
                                          {mod.symbols.Register("b"), ty.f32()}};

    auto* inner_y = MkStruct(mod, ty, "inner_y", inner_y_data);

    auto* s = ty.Struct(mod.symbols.New("S"), {{mod.symbols.Register("a"), ty.i32()},
                                               {mod.symbols.Register("b"), inner_x},
                                               {mod.symbols.Register("c"), ty.f32()},
                                               {mod.symbols.Register("d"), inner_y},
                                               {mod.symbols.Register("e"), ty.f32()}});

// ALL_FIELDS() calls the macro FIELD(ADDR, TYPE, ARRAY_COUNT, NAME)
// for each field of the structure s.
#define ALL_FIELDS()                     \
    FIELD(0x0000, int, 0, a)             \
    FIELD(0x0004, int8_t, 508, tint_pad) \
    FIELD(0x0200, inner_x, 0, b)         \
    FIELD(0x0600, float, 0, c)           \
    FIELD(0x0604, inner_y, 0, d)         \
    FIELD(0x0808, float, 0, e)           \
    FIELD(0x080c, int8_t, 500, tint_pad_4)

    // Check that the generated string is as expected.
    StringStream expect;
    expect << R"(
struct inner_x {
  /* 0x0000 */ int a;
  /* 0x0004 */ tint_array<int8_t, 508> tint_pad_1;
  /* 0x0200 */ float b;
  /* 0x0204 */ tint_array<int8_t, 508> tint_pad_2;
};

struct inner_y {
  /* 0x0000 */ int a;
  /* 0x0004 */ tint_array<int8_t, 508> tint_pad_3;
  /* 0x0200 */ float b;
};

)";

    expect << "struct S {\n";
#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) \
    FormatMSLField(expect, #ADDR, #TYPE, ARRAY_COUNT, #NAME);
    ALL_FIELDS()
#undef FIELD
    expect << R"(};
)";

    auto* var = b.Var("a", ty.ptr(core::AddressSpace::kStorage, s));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Load(var);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_THAT(output_.msl, testing::HasSubstr(expect.str())) << output_.msl;

    // 1.4 Metal and C++14
    // The Metal programming language is a C++14-based Specification with
    // extensions and restrictions. Refer to the C++14 Specification (also
    // known as the ISO/IEC JTC1/SC22/WG21 N4431 Language Specification) for a
    // detailed description of the language grammar.
    //
    // Tint is written in C++14, so use the compiler to verify the generated
    // layout is as expected for C++14 / MSL.
    {
        struct inner_x {
            uint32_t a;
            alignas(512) float b;
        };
        CHECK_TYPE_SIZE_AND_ALIGN(inner_x, 1024, 512);

        struct inner_y {
            uint32_t a[128];
            float b;
        };
        CHECK_TYPE_SIZE_AND_ALIGN(inner_y, 516, 4);

        struct S {
#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) std::array<TYPE, ARRAY_COUNT ? ARRAY_COUNT : 1> NAME;
            ALL_FIELDS()
#undef FIELD
        };

#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) \
    EXPECT_EQ(ADDR, static_cast<int>(offsetof(S, NAME))) << "Field " << #NAME;
        ALL_FIELDS()
#undef FIELD
    }

#undef ALL_FIELDS
}

TEST_F(MslWriterTest, EmitType_Struct_Layout_ArrayDefaultStride) {
    // inner: size(1024), align(512)
    Vector<MemberData, 2> inner_data = {{mod.symbols.Register("a"), ty.i32()},
                                        {mod.symbols.Register("b"), ty.f32(), 0, 512}};

    auto* inner = MkStruct(mod, ty, "inner", inner_data);

    // array_x: size(28), align(4)
    auto array_x = ty.array<f32, 7>();

    // array_y: size(4096), align(512)
    auto array_y = ty.array(inner, 4_u);

    // array_z: size(4), align(4)
    auto array_z = ty.array<f32>();

    auto* s = ty.Struct(mod.symbols.New("S"), {{mod.symbols.Register("a"), ty.i32()},
                                               {mod.symbols.Register("b"), array_x},
                                               {mod.symbols.Register("c"), ty.f32()},
                                               {mod.symbols.Register("d"), array_y},
                                               {mod.symbols.Register("e"), ty.f32()},
                                               {mod.symbols.Register("f"), array_z}});

    // ALL_FIELDS() calls the macro FIELD(ADDR, TYPE, ARRAY_COUNT, NAME)
    // for each field of the structure s.
#define ALL_FIELDS()                     \
    FIELD(0x0000, int, 0, a)             \
    FIELD(0x0004, float, 7, b)           \
    FIELD(0x0020, float, 0, c)           \
    FIELD(0x0024, int8_t, 476, tint_pad) \
    FIELD(0x0200, inner, 4, d)           \
    FIELD(0x1200, float, 0, e)           \
    FIELD(0x1204, float, 1, f)           \
    FIELD(0x1208, int8_t, 504, tint_pad_3)

    // Check that the generated string is as expected.
    StringStream expect;

    expect << R"(
struct inner {
  /* 0x0000 */ int a;
  /* 0x0004 */ tint_array<int8_t, 508> tint_pad_1;
  /* 0x0200 */ float b;
  /* 0x0204 */ tint_array<int8_t, 508> tint_pad_2;
};

)";

    expect << "struct S {\n";
#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) \
    FormatMSLField(expect, #ADDR, #TYPE, ARRAY_COUNT, #NAME);
    ALL_FIELDS()
#undef FIELD
    expect << R"(};
)";

    auto* var = b.Var("a", ty.ptr(core::AddressSpace::kStorage, s));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Load(var);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_THAT(output_.msl, testing::HasSubstr(expect.str())) << output_.msl;

    // 1.4 Metal and C++14
    // The Metal programming language is a C++14-based Specification with
    // extensions and restrictions. Refer to the C++14 Specification (also
    // known as the ISO/IEC JTC1/SC22/WG21 N4431 Language Specification) for a
    // detailed description of the language grammar.
    //
    // Tint is written in C++14, so use the compiler to verify the generated
    // layout is as expected for C++14 / MSL.
    {
        struct inner {
            uint32_t a;
            alignas(512) float b;
        };
        CHECK_TYPE_SIZE_AND_ALIGN(inner, 1024, 512);

        // array_x: size(28), align(4)
        using array_x = std::array<float, 7>;
        CHECK_TYPE_SIZE_AND_ALIGN(array_x, 28, 4);

        // array_y: size(4096), align(512)
        using array_y = std::array<inner, 4>;
        CHECK_TYPE_SIZE_AND_ALIGN(array_y, 4096, 512);

        // array_z: size(4), align(4)
        using array_z = std::array<float, 1>;
        CHECK_TYPE_SIZE_AND_ALIGN(array_z, 4, 4);

        struct S {
#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) std::array<TYPE, ARRAY_COUNT ? ARRAY_COUNT : 1> NAME;
            ALL_FIELDS()
#undef FIELD
        };

#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) \
    EXPECT_EQ(ADDR, static_cast<int>(offsetof(S, NAME))) << "Field " << #NAME;
        ALL_FIELDS()
#undef FIELD
    }

#undef ALL_FIELDS
}

TEST_F(MslWriterTest, EmitType_Struct_Layout_Vec3) {
    // array: size(64), align(16)
    auto array = ty.array<vec3<f32>, 4>();

    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("a"), ty.i32()},
                                                  {mod.symbols.Register("b"), ty.vec3<u32>()},
                                                  {mod.symbols.Register("c"), ty.vec3<f32>()},
                                                  {mod.symbols.Register("d"), ty.f32()},
                                                  {mod.symbols.Register("e"), ty.mat2x3<f32>()},
                                                  {mod.symbols.Register("f"), ty.mat3x3<f32>()},
                                                  {mod.symbols.Register("g"), ty.mat4x3<f32>()},
                                                  {mod.symbols.Register("h"), ty.f32()},
                                                  {mod.symbols.Register("i"), array},
                                                  {mod.symbols.Register("j"), ty.i32()},
                                              });

    auto expect = MetalHeader() + MetalArray() + R"(
struct tint_packed_vec3_f32_array_element {
  /* 0x0000 */ packed_float3 packed;
  /* 0x000c */ tint_array<int8_t, 4> tint_pad;
};

struct S {
  int a;
  uint3 b;
  float3 c;
  float d;
  float2x3 e;
  float3x3 f;
  float4x3 g;
  float h;
  tint_array<float3, 4> i;
  int j;
};

struct S_packed_vec3 {
  /* 0x0000 */ int a;
  /* 0x0004 */ tint_array<int8_t, 12> tint_pad_1;
  /* 0x0010 */ packed_uint3 b;
  /* 0x001c */ tint_array<int8_t, 4> tint_pad_2;
  /* 0x0020 */ packed_float3 c;
  /* 0x002c */ float d;
  /* 0x0030 */ tint_array<tint_packed_vec3_f32_array_element, 2> e;
  /* 0x0050 */ tint_array<tint_packed_vec3_f32_array_element, 3> f;
  /* 0x0080 */ tint_array<tint_packed_vec3_f32_array_element, 4> g;
  /* 0x00c0 */ float h;
  /* 0x00c4 */ tint_array<int8_t, 12> tint_pad_3;
  /* 0x00d0 */ tint_array<tint_packed_vec3_f32_array_element, 4> i;
  /* 0x0110 */ int j;
  /* 0x0114 */ tint_array<int8_t, 12> tint_pad_4;
};

struct tint_module_vars_struct {
  device S_packed_vec3* a;
};

tint_array<float3, 4> tint_load_array_packed_vec3(device tint_array<tint_packed_vec3_f32_array_element, 4>* const from) {
  return tint_array<float3, 4>{float3((*from)[0u].packed), float3((*from)[1u].packed), float3((*from)[2u].packed), float3((*from)[3u].packed)};
}

S tint_load_struct_packed_vec3(device S_packed_vec3* const from) {
  int const v = (*from).a;
  uint3 const v_1 = uint3((*from).b);
  float3 const v_2 = float3((*from).c);
  float const v_3 = (*from).d;
  tint_array<tint_packed_vec3_f32_array_element, 2> const v_4 = (*from).e;
  float2x3 const v_5 = float2x3(float3(v_4[0u].packed), float3(v_4[1u].packed));
  tint_array<tint_packed_vec3_f32_array_element, 3> const v_6 = (*from).f;
  float3x3 const v_7 = float3x3(float3(v_6[0u].packed), float3(v_6[1u].packed), float3(v_6[2u].packed));
  tint_array<tint_packed_vec3_f32_array_element, 4> const v_8 = (*from).g;
  float4x3 const v_9 = float4x3(float3(v_8[0u].packed), float3(v_8[1u].packed), float3(v_8[2u].packed), float3(v_8[3u].packed));
  float const v_10 = (*from).h;
  tint_array<float3, 4> const v_11 = tint_load_array_packed_vec3((&(*from).i));
  return S{.a=v, .b=v_1, .c=v_2, .d=v_3, .e=v_5, .f=v_7, .g=v_9, .h=v_10, .i=v_11, .j=(*from).j};
}

kernel void foo(device S_packed_vec3* a [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.a=a};
  tint_load_struct_packed_vec3(tint_module_vars.a);
}
)";

    auto* var = b.Var("a", ty.ptr(core::AddressSpace::kStorage, s));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Load(var);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, expect);
}

TEST_F(MslWriterTest, AttemptTintPadSymbolCollision) {
    Vector<MemberData, 26> data = {// uses symbols tint_pad_[0..9] and tint_pad_[20..35]
                                   {mod.symbols.Register("tint_pad_2"), ty.i32(), 32},         //
                                   {mod.symbols.Register("tint_pad_20"), ty.f32(), 128, 128},  //
                                   {mod.symbols.Register("tint_pad_33"), ty.vec2<f32>()},      //
                                   {mod.symbols.Register("tint_pad_1"), ty.u32()},             //
                                   {mod.symbols.Register("tint_pad_3"), ty.vec3<f32>()},       //
                                   {mod.symbols.Register("tint_pad_7"), ty.u32()},             //
                                   {mod.symbols.Register("tint_pad_25"), ty.vec4<f32>()},      //
                                   {mod.symbols.Register("tint_pad_5"), ty.u32()},             //
                                   {mod.symbols.Register("tint_pad_27"), ty.mat2x2<f32>()},    //
                                   {mod.symbols.Register("tint_pad_24"), ty.u32()},            //
                                   {mod.symbols.Register("tint_pad_23"), ty.mat2x3<f32>()},    //
                                   {mod.symbols.Register("tint_pad"), ty.u32()},               //
                                   {mod.symbols.Register("tint_pad_8"), ty.mat2x4<f32>()},     //
                                   {mod.symbols.Register("tint_pad_26"), ty.u32()},            //
                                   {mod.symbols.Register("tint_pad_29"), ty.mat3x2<f32>()},    //
                                   {mod.symbols.Register("tint_pad_6"), ty.u32()},             //
                                   {mod.symbols.Register("tint_pad_22"), ty.mat3x3<f32>()},    //
                                   {mod.symbols.Register("tint_pad_32"), ty.u32()},            //
                                   {mod.symbols.Register("tint_pad_34"), ty.mat3x4<f32>()},    //
                                   {mod.symbols.Register("tint_pad_35"), ty.u32()},            //
                                   {mod.symbols.Register("tint_pad_30"), ty.mat4x2<f32>()},    //
                                   {mod.symbols.Register("tint_pad_9"), ty.u32()},             //
                                   {mod.symbols.Register("tint_pad_31"), ty.mat4x3<f32>()},    //
                                   {mod.symbols.Register("tint_pad_28"), ty.u32()},            //
                                   {mod.symbols.Register("tint_pad_4"), ty.mat4x4<f32>()},     //
                                   {mod.symbols.Register("tint_pad_21"), ty.f32()}};

    auto* s = MkStruct(mod, ty, "S", data);

    auto expect = R"(
struct S_packed_vec3 {
  /* 0x0000 */ int tint_pad_2;
  /* 0x0004 */ tint_array<int8_t, 124> tint_pad_10;
  /* 0x0080 */ float tint_pad_20;
  /* 0x0084 */ tint_array<int8_t, 124> tint_pad_11;
  /* 0x0100 */ float2 tint_pad_33;
  /* 0x0108 */ uint tint_pad_1;
  /* 0x010c */ tint_array<int8_t, 4> tint_pad_12;
  /* 0x0110 */ packed_float3 tint_pad_3;
  /* 0x011c */ uint tint_pad_7;
  /* 0x0120 */ float4 tint_pad_25;
  /* 0x0130 */ uint tint_pad_5;
  /* 0x0134 */ tint_array<int8_t, 4> tint_pad_13;
  /* 0x0138 */ float2x2 tint_pad_27;
  /* 0x0148 */ uint tint_pad_24;
  /* 0x014c */ tint_array<int8_t, 4> tint_pad_14;
  /* 0x0150 */ tint_array<tint_packed_vec3_f32_array_element, 2> tint_pad_23;
  /* 0x0170 */ uint tint_pad;
  /* 0x0174 */ tint_array<int8_t, 12> tint_pad_16;
  /* 0x0180 */ float2x4 tint_pad_8;
  /* 0x01a0 */ uint tint_pad_26;
  /* 0x01a4 */ tint_array<int8_t, 4> tint_pad_17;
  /* 0x01a8 */ float3x2 tint_pad_29;
  /* 0x01c0 */ uint tint_pad_6;
  /* 0x01c4 */ tint_array<int8_t, 12> tint_pad_18;
  /* 0x01d0 */ tint_array<tint_packed_vec3_f32_array_element, 3> tint_pad_22;
  /* 0x0200 */ uint tint_pad_32;
  /* 0x0204 */ tint_array<int8_t, 12> tint_pad_19;
  /* 0x0210 */ float3x4 tint_pad_34;
  /* 0x0240 */ uint tint_pad_35;
  /* 0x0244 */ tint_array<int8_t, 4> tint_pad_36;
  /* 0x0248 */ float4x2 tint_pad_30;
  /* 0x0268 */ uint tint_pad_9;
  /* 0x026c */ tint_array<int8_t, 4> tint_pad_37;
  /* 0x0270 */ tint_array<tint_packed_vec3_f32_array_element, 4> tint_pad_31;
  /* 0x02b0 */ uint tint_pad_28;
  /* 0x02b4 */ tint_array<int8_t, 12> tint_pad_38;
  /* 0x02c0 */ float4x4 tint_pad_4;
  /* 0x0300 */ float tint_pad_21;
  /* 0x0304 */ tint_array<int8_t, 124> tint_pad_39;
};
)";

    auto* var = b.Var("a", ty.ptr(core::AddressSpace::kStorage, s));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Load(var);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_THAT(output_.msl, testing::HasSubstr(expect)) << output_.msl;
}

TEST_F(MslWriterTest, EmitType_Sampler) {
    auto* func = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("a", ty.sampler());
    func->SetParams({param});
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo(sampler a) {
}
)");
}

TEST_F(MslWriterTest, EmitType_SamplerComparison) {
    auto* func = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("a", ty.comparison_sampler());
    func->SetParams({param});
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo(sampler a) {
}
)");
}

struct MslDepthTextureData {
    core::type::TextureDimension dim;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, MslDepthTextureData data) {
    StringStream str;
    str << data.dim;
    out << str.str();
    return out;
}
using MslWriterDepthTexturesTest = MslWriterTestWithParam<MslDepthTextureData>;
TEST_P(MslWriterDepthTexturesTest, Emit) {
    auto params = GetParam();

    auto* t = ty.Get<core::type::DepthTexture>(params.dim);
    auto* func = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("a", t);
    func->SetParams({param});
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo()" + params.result +
                               R"( a) {
}
)");
}
INSTANTIATE_TEST_SUITE_P(
    MslWriterTest,
    MslWriterDepthTexturesTest,
    testing::Values(MslDepthTextureData{core::type::TextureDimension::k2d,
                                        "depth2d<float, access::sample>"},
                    MslDepthTextureData{core::type::TextureDimension::k2dArray,
                                        "depth2d_array<float, access::sample>"},
                    MslDepthTextureData{core::type::TextureDimension::kCube,
                                        "depthcube<float, access::sample>"},
                    MslDepthTextureData{core::type::TextureDimension::kCubeArray,
                                        "depthcube_array<float, access::sample>"}));

TEST_F(MslWriterTest, EmitType_DepthMultisampledTexture) {
    auto* t = ty.Get<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d);
    auto* func = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("a", t);
    func->SetParams({param});
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo(depth2d_ms<float, access::read> a) {
}
)");
}

struct MslTextureData {
    core::type::TextureDimension dim;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, MslTextureData data) {
    StringStream str;
    str << data.dim;
    out << str.str();
    return out;
}
using MslWriterSampledtexturesTest = MslWriterTestWithParam<MslTextureData>;
TEST_P(MslWriterSampledtexturesTest, Emit) {
    auto params = GetParam();

    auto* t = ty.Get<core::type::SampledTexture>(params.dim, ty.f32());
    auto* func = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("a", t);
    func->SetParams({param});
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo()" + params.result +
                               R"( a) {
}
)");
}
INSTANTIATE_TEST_SUITE_P(
    MslWriterTest,
    MslWriterSampledtexturesTest,
    testing::Values(
        MslTextureData{core::type::TextureDimension::k1d, "texture1d<float, access::sample>"},
        MslTextureData{core::type::TextureDimension::k2d, "texture2d<float, access::sample>"},
        MslTextureData{core::type::TextureDimension::k2dArray,
                       "texture2d_array<float, access::sample>"},
        MslTextureData{core::type::TextureDimension::k3d, "texture3d<float, access::sample>"},
        MslTextureData{core::type::TextureDimension::kCube, "texturecube<float, access::sample>"},
        MslTextureData{core::type::TextureDimension::kCubeArray,
                       "texturecube_array<float, access::sample>"}));

TEST_F(MslWriterTest, EmitType_MultisampledTexture) {
    auto* ms = ty.Get<core::type::MultisampledTexture>(core::type::TextureDimension::k2d, ty.u32());
    auto* func = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("a", ms);
    func->SetParams({param});
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo(texture2d_ms<uint, access::read> a) {
}
)");
}

struct MslStorageTextureData {
    core::type::TextureDimension dim;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, MslStorageTextureData data) {
    StringStream str;
    str << data.dim;
    return out << str.str();
}
using MslWriterStorageTexturesTest = MslWriterTestWithParam<MslStorageTextureData>;
TEST_P(MslWriterStorageTexturesTest, Emit) {
    auto params = GetParam();

    auto* f32 = ty.f32();
    auto s = ty.Get<core::type::StorageTexture>(params.dim, core::TexelFormat::kR32Float,
                                                core::Access::kWrite, f32);
    auto* func = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("a", s);
    func->SetParams({param});
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo()" + params.result +
                               R"( a) {
}
)");
}
INSTANTIATE_TEST_SUITE_P(MslWriterTest,
                         MslWriterStorageTexturesTest,
                         testing::Values(MslStorageTextureData{core::type::TextureDimension::k1d,
                                                               "texture1d<float, access::write>"},
                                         MslStorageTextureData{core::type::TextureDimension::k2d,
                                                               "texture2d<float, access::write>"},
                                         MslStorageTextureData{
                                             core::type::TextureDimension::k2dArray,
                                             "texture2d_array<float, access::write>"},
                                         MslStorageTextureData{core::type::TextureDimension::k3d,
                                                               "texture3d<float, access::write>"}));

// Metal only supports f{16, 32} at (8x8). Bfloat is also supported but isn't in WGSL.
TEST_F(MslWriterTest, EmitType_SubgroupMatrixLeft) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, ty.subgroup_matrix_left(ty.f32(), 8, 8)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  simdgroup_float8x8 a = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
}
)");
}

// Metal only supports f{16, 32} at (8x8). Bfloat is also supported but isn't in WGSL.
TEST_F(MslWriterTest, EmitType_SubgroupMatrixRight) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, ty.subgroup_matrix_right(ty.f16(), 8, 8)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  simdgroup_half8x8 a = make_filled_simdgroup_matrix<half, 8, 8>(0.0h);
}
)");
}

// Metal only supports f{16, 32} at (8x8). Bfloat is also supported but isn't in WGSL.
TEST_F(MslWriterTest, EmitType_SubgroupMatrixResult) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a",
              ty.ptr(core::AddressSpace::kFunction, ty.subgroup_matrix_result(ty.f32(), 8, 8)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  simdgroup_float8x8 a = make_filled_simdgroup_matrix<float, 8, 8>(0.0f);
}
)");
}

}  // namespace
}  // namespace tint::msl::writer
