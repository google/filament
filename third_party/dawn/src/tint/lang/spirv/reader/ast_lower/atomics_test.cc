// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/ast_lower/atomics.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"
#include "src/tint/lang/wgsl/reader/parser/parser.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::reader {
namespace {

class AtomicsTest : public ast::transform::TransformTest {
  public:
    ast::transform::Output Run(std::string in) {
        auto file = std::make_unique<Source::File>("test", std::move(in));
        auto parser = wgsl::reader::Parser(file.get());
        parser.Parse();

        auto& b = parser.builder();

        wgsl::BuiltinFn two_params[] = {
            wgsl::BuiltinFn::kAtomicExchange, wgsl::BuiltinFn::kAtomicAdd,
            wgsl::BuiltinFn::kAtomicSub,      wgsl::BuiltinFn::kAtomicMin,
            wgsl::BuiltinFn::kAtomicMax,      wgsl::BuiltinFn::kAtomicAnd,
            wgsl::BuiltinFn::kAtomicOr,       wgsl::BuiltinFn::kAtomicXor,
        };
        for (auto& a : two_params) {
            b.Func(std::string{"stub_"} + wgsl::str(a) + "_u32",
                   tint::Vector{
                       b.Param("p0", b.ty.u32()),
                       b.Param("p1", b.ty.u32()),
                   },
                   b.ty.u32(),
                   tint::Vector{
                       b.Return(0_u),
                   },
                   tint::Vector{
                       b.ASTNodes().Create<Atomics::Stub>(b.ID(), b.AllocateNodeID(), a),
                   });
            b.Func(std::string{"stub_"} + wgsl::str(a) + "_i32",
                   tint::Vector{
                       b.Param("p0", b.ty.i32()),
                       b.Param("p1", b.ty.i32()),
                   },
                   b.ty.i32(),
                   tint::Vector{
                       b.Return(0_i),
                   },
                   tint::Vector{
                       b.ASTNodes().Create<Atomics::Stub>(b.ID(), b.AllocateNodeID(), a),
                   });
        }

        b.Func("stub_atomicLoad_u32",
               tint::Vector{
                   b.Param("p0", b.ty.u32()),
               },
               b.ty.u32(),
               tint::Vector{
                   b.Return(0_u),
               },
               tint::Vector{
                   b.ASTNodes().Create<Atomics::Stub>(b.ID(), b.AllocateNodeID(),
                                                      wgsl::BuiltinFn::kAtomicLoad),
               });
        b.Func("stub_atomicLoad_i32",
               tint::Vector{
                   b.Param("p0", b.ty.i32()),
               },
               b.ty.i32(),
               tint::Vector{
                   b.Return(0_i),
               },
               tint::Vector{
                   b.ASTNodes().Create<Atomics::Stub>(b.ID(), b.AllocateNodeID(),
                                                      wgsl::BuiltinFn::kAtomicLoad),
               });

        b.Func("stub_atomicStore_u32",
               tint::Vector{
                   b.Param("p0", b.ty.u32()),
                   b.Param("p1", b.ty.u32()),
               },
               b.ty.void_(), tint::Empty,
               tint::Vector{
                   b.ASTNodes().Create<Atomics::Stub>(b.ID(), b.AllocateNodeID(),
                                                      wgsl::BuiltinFn::kAtomicStore),
               });
        b.Func("stub_atomicStore_i32",
               tint::Vector{
                   b.Param("p0", b.ty.i32()),
                   b.Param("p1", b.ty.i32()),
               },
               b.ty.void_(), tint::Empty,
               tint::Vector{
                   b.ASTNodes().Create<Atomics::Stub>(b.ID(), b.AllocateNodeID(),
                                                      wgsl::BuiltinFn::kAtomicStore),
               });

        b.Func("stub_atomic_compare_exchange_weak_u32",
               tint::Vector{
                   b.Param("p0", b.ty.u32()),
                   b.Param("p1", b.ty.u32()),
                   b.Param("p2", b.ty.u32()),
               },
               b.ty.u32(),
               tint::Vector{
                   b.Return(0_u),
               },
               tint::Vector{
                   b.ASTNodes().Create<Atomics::Stub>(b.ID(), b.AllocateNodeID(),
                                                      wgsl::BuiltinFn::kAtomicCompareExchangeWeak),
               });
        b.Func("stub_atomic_compare_exchange_weak_i32",
               tint::Vector{b.Param("p0", b.ty.i32()), b.Param("p1", b.ty.i32()),
                            b.Param("p2", b.ty.i32())},
               b.ty.i32(),
               tint::Vector{
                   b.Return(0_i),
               },
               tint::Vector{
                   b.ASTNodes().Create<Atomics::Stub>(b.ID(), b.AllocateNodeID(),
                                                      wgsl::BuiltinFn::kAtomicCompareExchangeWeak),
               });

        // Keep this pointer alive after Transform() returns
        files_.emplace_back(std::move(file));

        return ast::transform::TransformTest::Run<Atomics>(resolver::Resolve(b));
    }

  private:
    std::vector<std::unique_ptr<Source::File>> files_;
};

TEST_F(AtomicsTest, StripUnusedBuiltins) {
    auto* src = R"(
fn f() {
}
)";

    auto* expect = src;

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ArrayOfU32) {
    auto* src = R"(
var<workgroup> wg : array<u32, 4>;

fn f() {
  stub_atomicStore_u32(wg[1], 1u);
}
)";

    auto* expect = R"(
var<workgroup> wg : array<atomic<u32>, 4u>;

fn f() {
  atomicStore(&(wg[1]), 1u);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ArrayOfU32_ViaDerefPointerIndex) {
    auto* src = R"(
var<workgroup> wg : array<u32, 4>;

fn f() {
  let p = &wg;
  stub_atomicStore_u32((*p)[1], 1u);
}
)";

    auto* expect = R"(
var<workgroup> wg : array<atomic<u32>, 4u>;

fn f() {
  let p = &(wg);
  atomicStore(&((*(p))[1]), 1u);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ArrayOfU32_ViaPointerIndex) {
    auto* src = R"(
var<workgroup> wg : array<u32, 4>;

fn f() {
  let p = &wg;
  stub_atomicStore_u32(p[1], 1u);
}
)";

    auto* expect = R"(
var<workgroup> wg : array<atomic<u32>, 4u>;

fn f() {
  let p = &(wg);
  atomicStore(&(p[1]), 1u);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ArraysOfU32) {
    auto* src = R"(
var<workgroup> wg : array<array<array<u32, 1>, 2>, 3>;

fn f() {
  stub_atomicStore_u32(wg[2][1][0], 1u);
}
)";

    auto* expect = R"(
var<workgroup> wg : array<array<array<atomic<u32>, 1u>, 2u>, 3u>;

fn f() {
  atomicStore(&(wg[2][1][0]), 1u);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, AliasedArraysOfU32) {
    auto* src = R"(
alias A0 = u32;

alias A1 = array<A0, 1>;

alias A2 = array<A1, 2>;

alias A3 = array<A2, 3>;

var<workgroup> wg : A3;

fn f() {
  stub_atomicStore_u32(wg[2][1][0], 1u);
}
)";

    auto* expect = R"(
alias A0 = u32;

alias A1 = array<A0, 1>;

alias A2 = array<A1, 2>;

alias A3 = array<A2, 3>;

var<workgroup> wg : array<array<array<atomic<u32>, 1u>, 2u>, 3u>;

fn f() {
  atomicStore(&(wg[2][1][0]), 1u);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, FlatStructSingleAtomic) {
    auto* src = R"(
struct S {
  a : u32,
}

var<workgroup> wg : S;

fn f() {
  stub_atomicStore_u32(wg.a, 1u);
}
)";

    auto* expect = R"(
struct S_atomic {
  a : atomic<u32>,
}

struct S {
  a : u32,
}

var<workgroup> wg : S_atomic;

fn f() {
  atomicStore(&(wg.a), 1u);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, FlatStructSingleAtomic_ViaDerefPointerDot) {
    auto* src = R"(
struct S {
  a : u32,
}

var<workgroup> wg : S;

fn f() {
  let p = &wg;
  stub_atomicStore_u32((*p).a, 1u);
}
)";

    auto* expect = R"(
struct S_atomic {
  a : atomic<u32>,
}

struct S {
  a : u32,
}

var<workgroup> wg : S_atomic;

fn f() {
  let p = &(wg);
  atomicStore(&((*(p)).a), 1u);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, FlatStructSingleAtomic_ViaPointerDot) {
    auto* src = R"(
struct S {
  a : u32,
}

var<workgroup> wg : S;

fn f() {
  let p = &wg;
  stub_atomicStore_u32(p.a, 1u);
}
)";

    auto* expect = R"(
struct S_atomic {
  a : atomic<u32>,
}

struct S {
  a : u32,
}

var<workgroup> wg : S_atomic;

fn f() {
  let p = &(wg);
  atomicStore(&(p.a), 1u);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, FlatStructMultipleAtomic) {
    auto* src = R"(
struct S {
  a : u32,
  b : i32,
}

var<workgroup> wg : S;

fn f1() {
  stub_atomicStore_u32(wg.a, 1u);
}

fn f2() {
  stub_atomicStore_i32(wg.b, 2i);
}
)";

    auto* expect = R"(
struct S_atomic {
  a : atomic<u32>,
  b : atomic<i32>,
}

struct S {
  a : u32,
  b : i32,
}

var<workgroup> wg : S_atomic;

fn f1() {
  atomicStore(&(wg.a), 1u);
}

fn f2() {
  atomicStore(&(wg.b), 2i);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, NestedStruct) {
    auto* src = R"(
struct S0 {
  a : u32,
  b : i32,
  c : u32,
}

struct S1 {
  a : i32,
  b : u32,
  c : S0,
}

struct S2 {
  a : i32,
  b : S1,
  c : u32,
}

var<workgroup> wg : S2;

fn f() {
  stub_atomicStore_u32(wg.b.c.a, 1u);
}
)";

    auto* expect = R"(
struct S0_atomic {
  a : atomic<u32>,
  b : i32,
  c : u32,
}

struct S0 {
  a : u32,
  b : i32,
  c : u32,
}

struct S1_atomic {
  a : i32,
  b : u32,
  c : S0_atomic,
}

struct S1 {
  a : i32,
  b : u32,
  c : S0,
}

struct S2_atomic {
  a : i32,
  b : S1_atomic,
  c : u32,
}

struct S2 {
  a : i32,
  b : S1,
  c : u32,
}

var<workgroup> wg : S2_atomic;

fn f() {
  atomicStore(&(wg.b.c.a), 1u);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ArrayOfStruct) {
    auto* src = R"(
struct S {
  a : u32,
  b : i32,
  c : u32,
}

@group(0) @binding(1) var<storage, read_write> arr : array<S>;

fn f() {
  stub_atomicStore_i32(arr[4].b, 1i);
}
)";

    auto* expect = R"(
struct S_atomic {
  a : u32,
  b : atomic<i32>,
  c : u32,
}

struct S {
  a : u32,
  b : i32,
  c : u32,
}

@group(0) @binding(1) var<storage, read_write> arr : array<S_atomic>;

fn f() {
  atomicStore(&(arr[4].b), 1i);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ArrayOfStruct_ViaDerefPointerIndex) {
    auto* src = R"(
struct S {
  a : u32,
  b : i32,
  c : u32,
}

@group(0) @binding(1) var<storage, read_write> arr : array<S>;

fn f() {
  let p = &arr;
  stub_atomicStore_i32((*p)[4].b, 1i);
}
)";

    auto* expect = R"(
struct S_atomic {
  a : u32,
  b : atomic<i32>,
  c : u32,
}

struct S {
  a : u32,
  b : i32,
  c : u32,
}

@group(0) @binding(1) var<storage, read_write> arr : array<S_atomic>;

fn f() {
  let p = &(arr);
  atomicStore(&((*(p))[4].b), 1i);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ArrayOfStruct_ViaPointerIndex) {
    auto* src = R"(
struct S {
  a : u32,
  b : i32,
  c : u32,
}

@group(0) @binding(1) var<storage, read_write> arr : array<S>;

fn f() {
  let p = &arr;
  stub_atomicStore_i32(p[4].b, 1i);
}
)";

    auto* expect = R"(
struct S_atomic {
  a : u32,
  b : atomic<i32>,
  c : u32,
}

struct S {
  a : u32,
  b : i32,
  c : u32,
}

@group(0) @binding(1) var<storage, read_write> arr : array<S_atomic>;

fn f() {
  let p = &(arr);
  atomicStore(&(p[4].b), 1i);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, StructOfArray) {
    auto* src = R"(
struct S {
  a : array<i32>,
}

@group(0) @binding(1) var<storage, read_write> s : S;

fn f() {
  stub_atomicStore_i32(s.a[4], 1i);
}
)";

    auto* expect = R"(
struct S_atomic {
  a : array<atomic<i32>>,
}

struct S {
  a : array<i32>,
}

@group(0) @binding(1) var<storage, read_write> s : S_atomic;

fn f() {
  atomicStore(&(s.a[4]), 1i);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ViaPtrLet) {
    auto* src = R"(
struct S {
  i : i32,
}

@group(0) @binding(1) var<storage, read_write> s : S;

fn f() {
  let p0 = &(s);
  let p1 : ptr<storage, i32, read_write> = &((*(p0)).i);
  stub_atomicStore_i32(*p1, 1i);
}
)";

    auto* expect =
        R"(
struct S_atomic {
  i : atomic<i32>,
}

struct S {
  i : i32,
}

@group(0) @binding(1) var<storage, read_write> s : S_atomic;

fn f() {
  let p0 = &(s);
  let p1 : ptr<storage, atomic<i32>, read_write> = &((*(p0)).i);
  atomicStore(&(*(p1)), 1i);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, StructIsolatedMixedUsage) {
    auto* src = R"(
struct S {
  i : i32,
}

@group(0) @binding(1) var<storage, read_write> s : S;

fn f() {
  stub_atomicStore_i32(s.i, 1i);
}

fn another_usage() {
  var s : S;
  let x : i32 = s.i;
  s.i = 3i;
}
)";

    auto* expect =
        R"(
struct S_atomic {
  i : atomic<i32>,
}

struct S {
  i : i32,
}

@group(0) @binding(1) var<storage, read_write> s : S_atomic;

fn f() {
  atomicStore(&(s.i), 1i);
}

fn another_usage() {
  var s : S;
  let x : i32 = s.i;
  s.i = 3i;
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, AtomicLoad) {
    auto* src = R"(
var<workgroup> wg_u32 : u32;
var<workgroup> wg_i32 : i32;
@group(0) @binding(0) var<storage, read_write> sg_u32 : u32;
@group(0) @binding(1) var<storage, read_write> sg_i32 : i32;

fn f() {
  {let r = stub_atomicLoad_u32(wg_u32);}
  {let r = stub_atomicLoad_i32(wg_i32);}
  {let r = stub_atomicLoad_u32(sg_u32);}
  {let r = stub_atomicLoad_i32(sg_i32);}
}
)";

    auto* expect =
        R"(
var<workgroup> wg_u32 : atomic<u32>;

var<workgroup> wg_i32 : atomic<i32>;

@group(0) @binding(0) var<storage, read_write> sg_u32 : atomic<u32>;

@group(0) @binding(1) var<storage, read_write> sg_i32 : atomic<i32>;

fn f() {
  {
    let r = atomicLoad(&(wg_u32));
  }
  {
    let r = atomicLoad(&(wg_i32));
  }
  {
    let r = atomicLoad(&(sg_u32));
  }
  {
    let r = atomicLoad(&(sg_i32));
  }
}
)";

    auto got = Run(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, AtomicExchange) {
    auto* src = R"(
var<workgroup> wg_u32 : u32;
var<workgroup> wg_i32 : i32;
@group(0) @binding(0) var<storage, read_write> sg_u32 : u32;
@group(0) @binding(1) var<storage, read_write> sg_i32 : i32;

fn f() {
  {let r = stub_atomicExchange_u32(wg_u32, 123u);}
  {let r = stub_atomicExchange_i32(wg_i32, 123i);}
  {let r = stub_atomicExchange_u32(sg_u32, 123u);}
  {let r = stub_atomicExchange_i32(sg_i32, 123i);}
}
)";

    auto* expect =
        R"(
var<workgroup> wg_u32 : atomic<u32>;

var<workgroup> wg_i32 : atomic<i32>;

@group(0) @binding(0) var<storage, read_write> sg_u32 : atomic<u32>;

@group(0) @binding(1) var<storage, read_write> sg_i32 : atomic<i32>;

fn f() {
  {
    let r = atomicExchange(&(wg_u32), 123u);
  }
  {
    let r = atomicExchange(&(wg_i32), 123i);
  }
  {
    let r = atomicExchange(&(sg_u32), 123u);
  }
  {
    let r = atomicExchange(&(sg_i32), 123i);
  }
}
)";

    auto got = Run(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, AtomicAdd) {
    auto* src = R"(
var<workgroup> wg_u32 : u32;
var<workgroup> wg_i32 : i32;
@group(0) @binding(0) var<storage, read_write> sg_u32 : u32;
@group(0) @binding(1) var<storage, read_write> sg_i32 : i32;

fn f() {
  {let r = stub_atomicAdd_u32(wg_u32, 123u);}
  {let r = stub_atomicAdd_i32(wg_i32, 123i);}
  {let r = stub_atomicAdd_u32(sg_u32, 123u);}
  {let r = stub_atomicAdd_i32(sg_i32, 123i);}
}
)";

    auto* expect =
        R"(
var<workgroup> wg_u32 : atomic<u32>;

var<workgroup> wg_i32 : atomic<i32>;

@group(0) @binding(0) var<storage, read_write> sg_u32 : atomic<u32>;

@group(0) @binding(1) var<storage, read_write> sg_i32 : atomic<i32>;

fn f() {
  {
    let r = atomicAdd(&(wg_u32), 123u);
  }
  {
    let r = atomicAdd(&(wg_i32), 123i);
  }
  {
    let r = atomicAdd(&(sg_u32), 123u);
  }
  {
    let r = atomicAdd(&(sg_i32), 123i);
  }
}
)";

    auto got = Run(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, AtomicSub) {
    auto* src = R"(
var<workgroup> wg_u32 : u32;
var<workgroup> wg_i32 : i32;
@group(0) @binding(0) var<storage, read_write> sg_u32 : u32;
@group(0) @binding(1) var<storage, read_write> sg_i32 : i32;

fn f() {
  {let r = stub_atomicSub_u32(wg_u32, 123u);}
  {let r = stub_atomicSub_i32(wg_i32, 123i);}
  {let r = stub_atomicSub_u32(sg_u32, 123u);}
  {let r = stub_atomicSub_i32(sg_i32, 123i);}
}
)";

    auto* expect =
        R"(
var<workgroup> wg_u32 : atomic<u32>;

var<workgroup> wg_i32 : atomic<i32>;

@group(0) @binding(0) var<storage, read_write> sg_u32 : atomic<u32>;

@group(0) @binding(1) var<storage, read_write> sg_i32 : atomic<i32>;

fn f() {
  {
    let r = atomicSub(&(wg_u32), 123u);
  }
  {
    let r = atomicSub(&(wg_i32), 123i);
  }
  {
    let r = atomicSub(&(sg_u32), 123u);
  }
  {
    let r = atomicSub(&(sg_i32), 123i);
  }
}
)";

    auto got = Run(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, AtomicMin) {
    auto* src = R"(
var<workgroup> wg_u32 : u32;
var<workgroup> wg_i32 : i32;
@group(0) @binding(0) var<storage, read_write> sg_u32 : u32;
@group(0) @binding(1) var<storage, read_write> sg_i32 : i32;

fn f() {
  {let r = stub_atomicMin_u32(wg_u32, 123u);}
  {let r = stub_atomicMin_i32(wg_i32, 123i);}
  {let r = stub_atomicMin_u32(sg_u32, 123u);}
  {let r = stub_atomicMin_i32(sg_i32, 123i);}
}
)";

    auto* expect =
        R"(
var<workgroup> wg_u32 : atomic<u32>;

var<workgroup> wg_i32 : atomic<i32>;

@group(0) @binding(0) var<storage, read_write> sg_u32 : atomic<u32>;

@group(0) @binding(1) var<storage, read_write> sg_i32 : atomic<i32>;

fn f() {
  {
    let r = atomicMin(&(wg_u32), 123u);
  }
  {
    let r = atomicMin(&(wg_i32), 123i);
  }
  {
    let r = atomicMin(&(sg_u32), 123u);
  }
  {
    let r = atomicMin(&(sg_i32), 123i);
  }
}
)";

    auto got = Run(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, AtomicMax) {
    auto* src = R"(
var<workgroup> wg_u32 : u32;
var<workgroup> wg_i32 : i32;
@group(0) @binding(0) var<storage, read_write> sg_u32 : u32;
@group(0) @binding(1) var<storage, read_write> sg_i32 : i32;

fn f() {
  {let r = stub_atomicMax_u32(wg_u32, 123u);}
  {let r = stub_atomicMax_i32(wg_i32, 123i);}
  {let r = stub_atomicMax_u32(sg_u32, 123u);}
  {let r = stub_atomicMax_i32(sg_i32, 123i);}
}
)";

    auto* expect =
        R"(
var<workgroup> wg_u32 : atomic<u32>;

var<workgroup> wg_i32 : atomic<i32>;

@group(0) @binding(0) var<storage, read_write> sg_u32 : atomic<u32>;

@group(0) @binding(1) var<storage, read_write> sg_i32 : atomic<i32>;

fn f() {
  {
    let r = atomicMax(&(wg_u32), 123u);
  }
  {
    let r = atomicMax(&(wg_i32), 123i);
  }
  {
    let r = atomicMax(&(sg_u32), 123u);
  }
  {
    let r = atomicMax(&(sg_i32), 123i);
  }
}
)";

    auto got = Run(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, AtomicAnd) {
    auto* src = R"(
var<workgroup> wg_u32 : u32;
var<workgroup> wg_i32 : i32;
@group(0) @binding(0) var<storage, read_write> sg_u32 : u32;
@group(0) @binding(1) var<storage, read_write> sg_i32 : i32;

fn f() {
  {let r = stub_atomicAnd_u32(wg_u32, 123u);}
  {let r = stub_atomicAnd_i32(wg_i32, 123i);}
  {let r = stub_atomicAnd_u32(sg_u32, 123u);}
  {let r = stub_atomicAnd_i32(sg_i32, 123i);}
}
)";

    auto* expect =
        R"(
var<workgroup> wg_u32 : atomic<u32>;

var<workgroup> wg_i32 : atomic<i32>;

@group(0) @binding(0) var<storage, read_write> sg_u32 : atomic<u32>;

@group(0) @binding(1) var<storage, read_write> sg_i32 : atomic<i32>;

fn f() {
  {
    let r = atomicAnd(&(wg_u32), 123u);
  }
  {
    let r = atomicAnd(&(wg_i32), 123i);
  }
  {
    let r = atomicAnd(&(sg_u32), 123u);
  }
  {
    let r = atomicAnd(&(sg_i32), 123i);
  }
}
)";

    auto got = Run(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, AtomicOr) {
    auto* src = R"(
var<workgroup> wg_u32 : u32;
var<workgroup> wg_i32 : i32;
@group(0) @binding(0) var<storage, read_write> sg_u32 : u32;
@group(0) @binding(1) var<storage, read_write> sg_i32 : i32;

fn f() {
  {let r = stub_atomicOr_u32(wg_u32, 123u);}
  {let r = stub_atomicOr_i32(wg_i32, 123i);}
  {let r = stub_atomicOr_u32(sg_u32, 123u);}
  {let r = stub_atomicOr_i32(sg_i32, 123i);}
}
)";

    auto* expect =
        R"(
var<workgroup> wg_u32 : atomic<u32>;

var<workgroup> wg_i32 : atomic<i32>;

@group(0) @binding(0) var<storage, read_write> sg_u32 : atomic<u32>;

@group(0) @binding(1) var<storage, read_write> sg_i32 : atomic<i32>;

fn f() {
  {
    let r = atomicOr(&(wg_u32), 123u);
  }
  {
    let r = atomicOr(&(wg_i32), 123i);
  }
  {
    let r = atomicOr(&(sg_u32), 123u);
  }
  {
    let r = atomicOr(&(sg_i32), 123i);
  }
}
)";

    auto got = Run(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, AtomicXor) {
    auto* src = R"(
var<workgroup> wg_u32 : u32;
var<workgroup> wg_i32 : i32;
@group(0) @binding(0) var<storage, read_write> sg_u32 : u32;
@group(0) @binding(1) var<storage, read_write> sg_i32 : i32;

fn f() {
  {let r = stub_atomicXor_u32(wg_u32, 123u);}
  {let r = stub_atomicXor_i32(wg_i32, 123i);}
  {let r = stub_atomicXor_u32(sg_u32, 123u);}
  {let r = stub_atomicXor_i32(sg_i32, 123i);}
}
)";

    auto* expect =
        R"(
var<workgroup> wg_u32 : atomic<u32>;

var<workgroup> wg_i32 : atomic<i32>;

@group(0) @binding(0) var<storage, read_write> sg_u32 : atomic<u32>;

@group(0) @binding(1) var<storage, read_write> sg_i32 : atomic<i32>;

fn f() {
  {
    let r = atomicXor(&(wg_u32), 123u);
  }
  {
    let r = atomicXor(&(wg_i32), 123i);
  }
  {
    let r = atomicXor(&(sg_u32), 123u);
  }
  {
    let r = atomicXor(&(sg_i32), 123i);
  }
}
)";

    auto got = Run(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, AtomicCompareExchangeWeak) {
    auto* src = R"(
var<workgroup> wg_u32 : u32;
var<workgroup> wg_i32 : i32;
@group(0) @binding(0) var<storage, read_write> sg_u32 : u32;
@group(0) @binding(1) var<storage, read_write> sg_i32 : i32;

fn f() {
  {let r = stub_atomic_compare_exchange_weak_u32(wg_u32, 123u, 456u);}
  {let r = stub_atomic_compare_exchange_weak_i32(wg_i32, 123i, 456i);}
  {let r = stub_atomic_compare_exchange_weak_u32(sg_u32, 123u, 456u);}
  {let r = stub_atomic_compare_exchange_weak_i32(sg_i32, 123i, 456i);}
}
)";

    auto* expect =
        R"(
var<workgroup> wg_u32 : atomic<u32>;

var<workgroup> wg_i32 : atomic<i32>;

@group(0) @binding(0) var<storage, read_write> sg_u32 : atomic<u32>;

@group(0) @binding(1) var<storage, read_write> sg_i32 : atomic<i32>;

fn f() {
  {
    let old_value = atomicCompareExchangeWeak(&(wg_u32), 123u, 456u).old_value;
    let r = old_value;
  }
  {
    let old_value_2 = atomicCompareExchangeWeak(&(wg_i32), 123i, 456i).old_value;
    let r = old_value_2;
  }
  {
    let old_value_1 = atomicCompareExchangeWeak(&(sg_u32), 123u, 456u).old_value;
    let r = old_value_1;
  }
  {
    let old_value_3 = atomicCompareExchangeWeak(&(sg_i32), 123i, 456i).old_value;
    let r = old_value_3;
  }
}
)";

    auto got = Run(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ReplaceAssignsAndDecls_Scaler) {
    auto* src = R"(
var<workgroup> wg : u32;

fn f() {
  stub_atomicAdd_u32(wg, 1u);

  wg = 0u;
  let a = wg;
  var b : u32;
  b = wg;
}
)";

    auto* expect = R"(
var<workgroup> wg : atomic<u32>;

fn f() {
  atomicAdd(&(wg), 1u);
  atomicStore(&(wg), 0u);
  let a = atomicLoad(&(wg));
  var b : u32;
  b = atomicLoad(&(wg));
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ReplaceAssignsAndDecls_Struct) {
    auto* src = R"(
struct S {
  a : u32,
}

var<workgroup> wg : S;

fn f() {
  stub_atomicAdd_u32(wg.a, 1u);

  wg.a = 0u;
  let a = wg.a;
  var b : u32;
  b = wg.a;
}
)";

    auto* expect = R"(
struct S_atomic {
  a : atomic<u32>,
}

struct S {
  a : u32,
}

var<workgroup> wg : S_atomic;

fn f() {
  atomicAdd(&(wg.a), 1u);
  atomicStore(&(wg.a), 0u);
  let a = atomicLoad(&(wg.a));
  var b : u32;
  b = atomicLoad(&(wg.a));
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ReplaceAssignsAndDecls_NestedStruct) {
    auto* src = R"(
struct S0 {
  a : u32,
}

struct S1 {
  s0 : S0
}

var<workgroup> wg : S1;

fn f() {
  stub_atomicAdd_u32(wg.s0.a, 1u);

  wg.s0.a = 0u;
  let a = wg.s0.a;
  var b : u32;
  b = wg.s0.a;
}
)";

    auto* expect = R"(
struct S0_atomic {
  a : atomic<u32>,
}

struct S0 {
  a : u32,
}

struct S1_atomic {
  s0 : S0_atomic,
}

struct S1 {
  s0 : S0,
}

var<workgroup> wg : S1_atomic;

fn f() {
  atomicAdd(&(wg.s0.a), 1u);
  atomicStore(&(wg.s0.a), 0u);
  let a = atomicLoad(&(wg.s0.a));
  var b : u32;
  b = atomicLoad(&(wg.s0.a));
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ReplaceAssignsAndDecls_StructMultipleAtomics) {
    auto* src = R"(
struct S {
  a : u32,
  b : u32,
  c : u32,
}

var<workgroup> wg : S;

fn f() {
  stub_atomicAdd_u32(wg.a, 1u);
  stub_atomicAdd_u32(wg.b, 1u);

  wg.a = 0u;
  let a = wg.a;
  var b : u32;
  b = wg.a;

  wg.b = 0u;
  let c = wg.b;
  var d : u32;
  d = wg.b;

  wg.c = 0u;
  let e = wg.c;
  var f : u32;
  f = wg.c;
}
)";

    auto* expect = R"(
struct S_atomic {
  a : atomic<u32>,
  b : atomic<u32>,
  c : u32,
}

struct S {
  a : u32,
  b : u32,
  c : u32,
}

var<workgroup> wg : S_atomic;

fn f() {
  atomicAdd(&(wg.a), 1u);
  atomicAdd(&(wg.b), 1u);
  atomicStore(&(wg.a), 0u);
  let a = atomicLoad(&(wg.a));
  var b : u32;
  b = atomicLoad(&(wg.a));
  atomicStore(&(wg.b), 0u);
  let c = atomicLoad(&(wg.b));
  var d : u32;
  d = atomicLoad(&(wg.b));
  wg.c = 0u;
  let e = wg.c;
  var f : u32;
  f = wg.c;
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ReplaceAssignsAndDecls_ArrayOfScalar) {
    auto* src = R"(
var<workgroup> wg : array<u32, 4>;

fn f() {
  stub_atomicAdd_u32(wg[1], 1u);

  wg[1] = 0u;
  let a = wg[1];
  var b : u32;
  b = wg[1];
}
)";

    auto* expect = R"(
var<workgroup> wg : array<atomic<u32>, 4u>;

fn f() {
  atomicAdd(&(wg[1]), 1u);
  atomicStore(&(wg[1]), 0u);
  let a = atomicLoad(&(wg[1]));
  var b : u32;
  b = atomicLoad(&(wg[1]));
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ReplaceAssignsAndDecls_ArrayOfStruct) {
    auto* src = R"(
struct S {
  a : u32,
}

var<workgroup> wg : array<S, 4>;

fn f() {
  stub_atomicAdd_u32(wg[1].a, 1u);

  wg[1].a = 0u;
  let a = wg[1].a;
  var b : u32;
  b = wg[1].a;
}
)";

    auto* expect = R"(
struct S_atomic {
  a : atomic<u32>,
}

struct S {
  a : u32,
}

var<workgroup> wg : array<S_atomic, 4u>;

fn f() {
  atomicAdd(&(wg[1].a), 1u);
  atomicStore(&(wg[1].a), 0u);
  let a = atomicLoad(&(wg[1].a));
  var b : u32;
  b = atomicLoad(&(wg[1].a));
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ReplaceAssignsAndDecls_StructOfArray) {
    auto* src = R"(
struct S {
  a : array<u32>,
}

@group(0) @binding(1) var<storage, read_write> s : S;

fn f() {
  stub_atomicAdd_u32(s.a[4], 1u);

  s.a[4] = 0u;
  let a = s.a[4];
  var b : u32;
  b = s.a[4];
}
)";

    auto* expect = R"(
struct S_atomic {
  a : array<atomic<u32>>,
}

struct S {
  a : array<u32>,
}

@group(0) @binding(1) var<storage, read_write> s : S_atomic;

fn f() {
  atomicAdd(&(s.a[4]), 1u);
  atomicStore(&(s.a[4]), 0u);
  let a = atomicLoad(&(s.a[4]));
  var b : u32;
  b = atomicLoad(&(s.a[4]));
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ReplaceAssignsAndDecls_ViaPtrLet) {
    auto* src = R"(
struct S {
  i : u32,
}

@group(0) @binding(1) var<storage, read_write> s : S;

fn f() {
  let p0 = &(s);
  let p1 : ptr<storage, u32, read_write> = &((*(p0)).i);
  stub_atomicAdd_u32(*p1, 1u);

  *p1 = 0u;
  let a = *p1;
  var b : u32;
  b = *p1;
}
)";

    auto* expect = R"(
struct S_atomic {
  i : atomic<u32>,
}

struct S {
  i : u32,
}

@group(0) @binding(1) var<storage, read_write> s : S_atomic;

fn f() {
  let p0 = &(s);
  let p1 : ptr<storage, atomic<u32>, read_write> = &((*(p0)).i);
  atomicAdd(&(*(p1)), 1u);
  atomicStore(&(*(p1)), 0u);
  let a = atomicLoad(&(*(p1)));
  var b : u32;
  b = atomicLoad(&(*(p1)));
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ReplaceBitcastArgument_Scaler) {
    auto* src = R"(
var<workgroup> wg : u32;

fn f() {
  stub_atomicAdd_u32(wg, 1u);

  wg = 0u;
  var b : f32;
  b = bitcast<f32>(wg);
}
)";

    auto* expect = R"(
var<workgroup> wg : atomic<u32>;

fn f() {
  atomicAdd(&(wg), 1u);
  atomicStore(&(wg), 0u);
  var b : f32;
  b = bitcast<f32>(atomicLoad(&(wg)));
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AtomicsTest, ReplaceBitcastArgument_Struct) {
    auto* src = R"(
struct S {
  a : u32,
}

var<workgroup> wg : S;

fn f() {
  stub_atomicAdd_u32(wg.a, 1u);

  wg.a = 0u;
  var b : f32;
  b = bitcast<f32>(wg.a);
}
)";

    auto* expect = R"(
struct S_atomic {
  a : atomic<u32>,
}

struct S {
  a : u32,
}

var<workgroup> wg : S_atomic;

fn f() {
  atomicAdd(&(wg.a), 1u);
  atomicStore(&(wg.a), 0u);
  var b : f32;
  b = bitcast<f32>(atomicLoad(&(wg.a)));
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::spirv::reader
