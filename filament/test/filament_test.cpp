/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <iostream>
#include <random>

#include <gtest/gtest.h>

#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat3.h>
#include <math/mat4.h>
#include <math/scalar.h>

#include <filament/Box.h>
#include <filament/Camera.h>
#include <filament/Color.h>
#include <filament/Frustum.h>
#include <filament/Material.h>
#include <filament/Engine.h>

#include <private/filament/UniformInterfaceBlock.h>
#include <private/filament/UibGenerator.h>

#include "details/Allocators.h"
#include "details/Material.h"
#include "details/Camera.h"
#include "details/Froxelizer.h"
#include "details/Engine.h"
#include "components/RenderableManager.h"
#include "components/TransformManager.h"
#include "UniformBuffer.h"

using namespace filament;
using namespace filament::math;
using namespace utils;

static bool isGray(float3 v) {
    return v.r == v.g && v.g == v.b;
}

static bool almostEqualUlps(float a, float b, int maxUlps) {
    if (a == b) return true;
    int intDiff = abs(*reinterpret_cast<int32_t*>(&a) - *reinterpret_cast<int32_t*>(&b));
    return intDiff <= maxUlps;
}

static bool vec3eq(float3 a, float3 b) {
    return  almostEqualUlps(a.x, b.x, 1) &&
            almostEqualUlps(a.y, b.y, 1) &&
            almostEqualUlps(a.z, b.z, 1);
}

TEST(FilamentTest, AabbMath) {
    constexpr Aabb aabb = {{4, 5, 6}, {12, 14, 11}};

    const mat4f m(mat3f::rotation(F_PI_2, float3 {0, 0, 1}), float3 {-4, -5, -6});
    const Aabb result = aabb.transform(m);

    // Compare Arvo's method (above) with the naive method (below).
    const float3 a = (m * float4(aabb.min.x, aabb.min.y, aabb.min.z, 1.0)).xyz;
    const float3 b = (m * float4(aabb.min.x, aabb.min.y, aabb.max.z, 1.0)).xyz;
    const float3 c = (m * float4(aabb.min.x, aabb.max.y, aabb.min.z, 1.0)).xyz;
    const float3 d = (m * float4(aabb.min.x, aabb.max.y, aabb.max.z, 1.0)).xyz;
    const float3 e = (m * float4(aabb.max.x, aabb.min.y, aabb.min.z, 1.0)).xyz;
    const float3 f = (m * float4(aabb.max.x, aabb.min.y, aabb.max.z, 1.0)).xyz;
    const float3 g = (m * float4(aabb.max.x, aabb.max.y, aabb.min.z, 1.0)).xyz;
    const float3 h = (m * float4(aabb.max.x, aabb.max.y, aabb.max.z, 1.0)).xyz;
    const Aabb expected {
        .min = min(min(min(min(min(min(min(a, b), c), d), e), f), g), h),
        .max = max(max(max(max(max(max(max(a, b), c), d), e), f), g), h),
    };

    EXPECT_PRED2(vec3eq, result.min, expected.min);
    EXPECT_PRED2(vec3eq, result.max, expected.max);
}

TEST(FilamentTest, SkinningMath) {

    struct Bone {
        quatf q;
        float4 t;
        float4 s;
    };

    auto makeBone = [&](mat4f m) -> Bone {
        // figure out the scales
        float4 s = { length(m[0]), length(m[1]), length(m[2]), 0.0f };
        if (dot(cross(m[0].xyz, m[1].xyz), m[2].xyz) < 0) {
            s[2] = -s[2];
        }

        // compute the inverse scales
        float4 is = { 1.0f/s.x, 1.0f/s.y, 1.0f/s.z, 0.0f };

        // normalize the matrix
        m[0] *= is[0];
        m[1] *= is[1];
        m[2] *= is[2];


        Bone bone;
        bone.s = s;
        bone.q = m.toQuaternion();
        bone.t = m[3];
        return bone;
    };

    auto applyBone = [](Bone const& bone, float3 v) -> float3 {
        float4 q = bone.q.xyzw;
        float3 t = bone.t.xyz;
        float3 s = bone.s.xyz;

        // apply the non-uniform scales
        v *= s;

        // apply the rigid transform (valid only for unit quaternions)
        v += 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);

        // apply the translation
        v += t;

        return v;
    };

    auto check = [&](mat4f m, float3 v) {
        float3 expect = (m * v).xyz;
        float3 actual = applyBone(makeBone(m), v);
        static constexpr float value_eps = 40 * std::numeric_limits<float>::epsilon();
        EXPECT_NEAR(expect.x, actual.x, value_eps);
        EXPECT_NEAR(expect.y, actual.y, value_eps);
        EXPECT_NEAR(expect.z, actual.z, value_eps);
    };

    mat4f m;
    float3 v = {1, 2, 3};

    m = mat4f::translation(float3{1, 2, 3});
    check(m, v);

    m = mat4f::scaling(float3{1, 2, 3});
    check(m, v);

    m = mat4f::scaling(float3{1, 2, 3}) * mat4f::translation(float3{1, 2, 3});
    check(m, v);

    m = mat4f::translation(float3{1, 2, 3}) * mat4f::scaling(float3{1, 2, 3});
    check(m, v);

    m = mat4f::translation(float3{1, 2, 3}) * mat4f::scaling(float3{1, -4, 1});
    check(m, v);


    std::default_random_engine generator(82828); // NOLINT
    std::uniform_real_distribution<float> distribution(-4, 4);
    std::uniform_real_distribution<float> dangle(-2.0 * F_PI, 2.0 * F_PI);
    auto rand_gen = std::bind(distribution, generator);

    for (size_t i = 0; i < 100; ++i) {
        m =
                mat4f::translation(float3{rand_gen(), rand_gen(), rand_gen()}) *
                mat4f::rotation(dangle(generator), normalize(float3{rand_gen(), rand_gen(), rand_gen()})) *
                mat4f::scaling(float3{rand_gen(), rand_gen(), rand_gen()});
        check(m, v);
    }
}

TEST(FilamentTest, TransformManager) {
    filament::FTransformManager tcm;
    EntityManager& em = EntityManager::get();
    std::array<Entity, 3> entities;
    em.create(entities.size(), entities.data());

    // test component creation
    tcm.create(entities[0]);
    EXPECT_TRUE(tcm.hasComponent(entities[0]));
    TransformManager::Instance parent = tcm.getInstance(entities[0]);
    EXPECT_TRUE(bool(parent));

    // test component creation with parent
    tcm.create(entities[1], parent, mat4f{});
    EXPECT_TRUE(tcm.hasComponent(entities[1]));
    TransformManager::Instance child = tcm.getInstance(entities[1]);
    EXPECT_TRUE(bool(child));

    // test default values
    EXPECT_EQ(tcm.getTransform(parent), mat4f{ float4{ 1 }});
    EXPECT_EQ(tcm.getWorldTransform(parent), mat4f{ float4{ 1 }});
    EXPECT_EQ(tcm.getTransform(child), mat4f{ float4{ 1 }});
    EXPECT_EQ(tcm.getWorldTransform(child), mat4f{ float4{ 1 }});

    // test setting a transform
    tcm.setTransform(parent, mat4f{ float4{ 2 }});

    // test local and world transform propagation
    EXPECT_EQ(tcm.getTransform(parent), mat4f{ float4{ 2 }});
    EXPECT_EQ(tcm.getWorldTransform(parent), mat4f{ float4{ 2 }});
    EXPECT_EQ(tcm.getTransform(child), mat4f{ float4{ 1 }});
    EXPECT_EQ(tcm.getWorldTransform(child), mat4f{ float4{ 2 }});

    // test local transaction
    tcm.openLocalTransformTransaction();
    tcm.setTransform(parent, mat4f{ float4{ 4 }});

    // check the transforms ARE NOT propagated
    EXPECT_EQ(tcm.getTransform(parent), mat4f{ float4{ 4 }});
    EXPECT_EQ(tcm.getWorldTransform(parent), mat4f{ float4{ 2 }});
    EXPECT_EQ(tcm.getTransform(child), mat4f{ float4{ 1 }});
    EXPECT_EQ(tcm.getWorldTransform(child), mat4f{ float4{ 2 }});

    tcm.commitLocalTransformTransaction();
    // test propagation after closing the transaction
    EXPECT_EQ(tcm.getTransform(parent), mat4f{ float4{ 4 }});
    EXPECT_EQ(tcm.getWorldTransform(parent), mat4f{ float4{ 4 }});
    EXPECT_EQ(tcm.getTransform(child), mat4f{ float4{ 1 }});
    EXPECT_EQ(tcm.getWorldTransform(child), mat4f{ float4{ 4 }});

    //
    // test out-of-order parent/child
    //

    tcm.create(entities[2]);
    EXPECT_TRUE(tcm.hasComponent(entities[2]));
    TransformManager::Instance newParent = tcm.getInstance(entities[2]);
    ASSERT_LT(child, newParent);

    // test reparenting
    tcm.setParent(child, newParent);

    // make sure child/parent are out of order (i.e.: setParent() doesn't invalidate instances)
    EXPECT_LT(tcm.getInstance(entities[1]), tcm.getInstance(entities[2]));

    // local transaction reorders parent/child
    tcm.openLocalTransformTransaction();
    tcm.setTransform(newParent, mat4f{ float4{ 8 }});
    tcm.commitLocalTransformTransaction();

    // local transaction invalidates Instances
    parent = tcm.getInstance(entities[0]);
    child = tcm.getInstance(entities[1]);
    newParent = tcm.getInstance(entities[2]);

    // check parent / child order is correct
    EXPECT_GT(child, newParent);

    // check transform propagation
    EXPECT_EQ(tcm.getTransform(newParent), mat4f{ float4{ 8 }});
    EXPECT_EQ(tcm.getWorldTransform(newParent), mat4f{ float4{ 8 }});
    EXPECT_EQ(tcm.getTransform(child), mat4f{ float4{ 1 }});
    EXPECT_EQ(tcm.getWorldTransform(child), mat4f{ float4{ 8 }});

    // check children iterators
    size_t c = 0;
    auto first = tcm.getChildrenBegin(newParent);
    auto last = tcm.getChildrenEnd(newParent);
    while (first != last) {
        ++first;
        c++;
    }

    EXPECT_EQ(tcm.getChildrenEnd(parent)++, tcm.getChildrenEnd(parent));
    EXPECT_EQ(tcm.getChildrenBegin(parent), tcm.getChildrenEnd(parent));
    EXPECT_EQ(c, tcm.getChildCount(newParent));
}

TEST(FilamentTest, UniformInterfaceBlock) {

    UniformInterfaceBlock::Builder b;

    b.name("TestUniformInterfaceBlock");
    b.add("a_float_0", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_float_1", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_float_2", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_float_3", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_vec4_0",  1, UniformInterfaceBlock::Type::FLOAT4);
    b.add("a_float_4", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_float_5", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_float_6", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_vec3_0",  1, UniformInterfaceBlock::Type::FLOAT3);
    b.add("a_float_7", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_float[3]",3, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_float_8", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_mat3_0",  1, UniformInterfaceBlock::Type::MAT3);
    b.add("a_mat4_0",  1, UniformInterfaceBlock::Type::MAT4);
    b.add("a_mat3[3]", 3, UniformInterfaceBlock::Type::MAT3);


    UniformInterfaceBlock ib(b.build());
    auto const& info = ib.getUniformInfoList();

    // test that 4 floats are packed together
    EXPECT_EQ(0, info[0].offset);
    EXPECT_EQ(1, info[1].offset);
    EXPECT_EQ(2, info[2].offset);
    EXPECT_EQ(3, info[3].offset);

    // test the double4 is where it should be
    EXPECT_EQ(4, info[4].offset);

    // check 3 following floats are packed right after the double4
    EXPECT_EQ(8, info[5].offset);
    EXPECT_EQ(9, info[6].offset);
    EXPECT_EQ(10, info[7].offset);

    // check that the following double3 is aligned to the next double4 boundary
    EXPECT_EQ(12, info[8].offset);

    // check that the following float is just behind the double3
    EXPECT_EQ(15, info[9].offset);

    // check that arrays are aligned on double4 and have a stride of double4
    EXPECT_EQ(16, info[10].offset);
    EXPECT_EQ(4, info[10].stride);
    EXPECT_EQ(3, info[10].size);

    // check the base offset of the member following the array is rounded up to the next multiple of the base alignment.
    EXPECT_EQ(28, info[11].offset);

    // check mat3 alignment is double4
    EXPECT_EQ(32, info[12].offset);
    EXPECT_EQ(12, info[12].stride);

    // check following mat4 is 3*double4 away
    EXPECT_EQ(44, info[13].offset);
    EXPECT_EQ(16, info[13].stride);

    // arrays of matrices
    EXPECT_EQ(60, info[14].offset);
    EXPECT_EQ(12, info[14].stride);
    EXPECT_EQ(3, info[14].size);
}

TEST(FilamentTest, UniformBuffer) {

    struct ubo {
                    float   f0;
                    float   f1;
                    float   f2;
                    float   f3;
        alignas(16) float4 v0;
                    float   f4;
                    float   f5;
                    float   f6;
        alignas(16) float3 v1;     // double3 are aligned to 4 floats
                    float   f7;
                    struct {
                        alignas(16) float v;    // arrays entries are always aligned to 4 floats
                    } u[3];
                    float   f8;
        alignas(16) float4 m0[3];  // mat3 are like vec4f[3]
        alignas(16) mat4f   m1;
    };

    auto CHECK = [](ubo const* data) {
        EXPECT_EQ(1.0f, data->f0);
        EXPECT_EQ(3.0f, data->f1);
        EXPECT_EQ(5.0f, data->f2);
        EXPECT_EQ(7.0f, data->f3);
        EXPECT_EQ((float4{ -1.1f, -1.2f, 3.14f, sqrtf(2)}), data->v0);
        EXPECT_EQ(11.0f, data->f4);
        EXPECT_EQ(13.0f, data->f5);
        EXPECT_EQ(17.0f, data->f6);
        EXPECT_EQ((float3{ 1, 2, 3}), data->v1);
        EXPECT_EQ(19.0f, data->f7);
        EXPECT_EQ(-3.0f, data->u[0].v);
        EXPECT_EQ(-5.0f, data->u[1].v);
        EXPECT_EQ(-7.0f, data->u[2].v);
        EXPECT_EQ(23.0f, data->f8);
        EXPECT_EQ((mat4f{100, 200, 300, 0, 400, 500, 600, 0, 700, 800, 900, 0, 0, 0, 0, 1}), data->m1);
    };

    auto CHECK2 = [](std::vector<UniformInterfaceBlock::UniformInfo> const& info) {
        EXPECT_EQ(offsetof(ubo, f0)/4, info[0].offset);
        EXPECT_EQ(offsetof(ubo, f1)/4, info[1].offset);
        EXPECT_EQ(offsetof(ubo, f2)/4, info[2].offset);
        EXPECT_EQ(offsetof(ubo, f3)/4, info[3].offset);
        EXPECT_EQ(offsetof(ubo, v0)/4, info[4].offset);
        EXPECT_EQ(offsetof(ubo, f4)/4, info[5].offset);
        EXPECT_EQ(offsetof(ubo, f5)/4, info[6].offset);
        EXPECT_EQ(offsetof(ubo, f6)/4, info[7].offset);
        EXPECT_EQ(offsetof(ubo, v1)/4, info[8].offset);
        EXPECT_EQ(offsetof(ubo, f7)/4, info[9].offset);
        EXPECT_EQ(offsetof(ubo,  u)/4, info[10].offset);
        EXPECT_EQ(offsetof(ubo, f8)/4, info[11].offset);
        EXPECT_EQ(offsetof(ubo, m0)/4, info[12].offset);
        EXPECT_EQ(offsetof(ubo, m1)/4, info[13].offset);
    };

    UniformInterfaceBlock::Builder b;
    b.name("TestUniformBuffer");
    b.add("a_float_0", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_float_1", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_float_2", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_float_3", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_vec4_0",  1, UniformInterfaceBlock::Type::FLOAT4);
    b.add("a_float_4", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_float_5", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_float_6", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_vec3_0",  1, UniformInterfaceBlock::Type::FLOAT3);
    b.add("a_float_7", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_float[3]",3, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_float_8", 1, UniformInterfaceBlock::Type::FLOAT);
    b.add("a_mat3_0",  1, UniformInterfaceBlock::Type::MAT3);
    b.add("a_mat4_0",  1, UniformInterfaceBlock::Type::MAT4);
    UniformInterfaceBlock ib(b.build());

    CHECK2(ib.getUniformInfoList());

    EXPECT_EQ(sizeof(ubo), ib.getSize());

    UniformBuffer buffer(sizeof(ubo));
    ubo const* data = static_cast<ubo const*>(buffer.getBuffer());

    buffer.setUniform(offsetof(ubo, f0), 1.0f);
    buffer.setUniform(offsetof(ubo, f1), 3.0f);
    buffer.setUniform(offsetof(ubo, f2), 5.0f);
    buffer.setUniform(offsetof(ubo, f3), 7.0f);
    buffer.setUniform(offsetof(ubo, v0), float4{ -1.1f, -1.2f, 3.14f, sqrtf(2) });
    buffer.setUniform(offsetof(ubo, f4), 11.0f);
    buffer.setUniform(offsetof(ubo, f5), 13.0f);
    buffer.setUniform(offsetof(ubo, f6), 17.0f);
    buffer.setUniform(offsetof(ubo, v1), float3{ 1, 2, 3 });
    buffer.setUniform(offsetof(ubo, f7), 19.0f);
    buffer.setUniform(offsetof(ubo, u[0].v), -3.0f);
    buffer.setUniform(offsetof(ubo, u[1].v), -5.0f);
    buffer.setUniform(offsetof(ubo, u[2].v), -7.0f);
    buffer.setUniform(offsetof(ubo, f8), 23.0f);
    buffer.setUniform(offsetof(ubo, m0), mat3f{10,20,30, 40,50,60, 70,80,90 });
    buffer.setUniform(offsetof(ubo, m1), mat4f{100,200,300,0, 400,500,600,0, 700,800,900,0, 0,0,0,1 });

    CHECK(data);

    ubo copy(*data);
    CHECK(data);
    CHECK(&copy);

    ubo move(std::move(copy));
    CHECK(&move);

    copy = *data;
    CHECK(data);
    CHECK(&copy);

    move = std::move(copy);
    CHECK(&move);
}

TEST(FilamentTest, UniformBufferSize1) {
    UniformInterfaceBlock::Builder b;
    b.name("UniformBufferSize1");
    b.add("f4a", 1, UniformInterfaceBlock::Type::FLOAT4); // offset = 0
    b.add("f4b", 1, UniformInterfaceBlock::Type::FLOAT4); // offset = 16
    b.add("f1a", 1, UniformInterfaceBlock::Type::FLOAT);  // offset = 32
    b.add("f1b", 1, UniformInterfaceBlock::Type::FLOAT);  // offset = 36
    UniformInterfaceBlock uib(b.build());
    UniformBuffer buffer(uib.getSize());

    float4 f4(1.0f);
    ssize_t f4_offset = uib.getUniformOffset("f4a", 0);
    buffer.setUniformArray(f4_offset, &f4, 1);

    float f1(1.0f);
    ssize_t f1_offset = uib.getUniformOffset("f1b", 0);
    buffer.setUniformArray(f1_offset, &f1, 1);

    buffer.invalidate();
}

TEST(FilamentTest, UniformBufferSize2) {
    UniformInterfaceBlock::Builder b;
    b.name("UniformBufferSize2");
    b.add("f4a", 1, UniformInterfaceBlock::Type::FLOAT4); // offset = 0
    b.add("f4b", 1, UniformInterfaceBlock::Type::FLOAT4); // offset = 16
    b.add("f1a", 1, UniformInterfaceBlock::Type::FLOAT);  // offset = 32
    b.add("f2a", 1, UniformInterfaceBlock::Type::FLOAT2); // offset = 36
    UniformInterfaceBlock uib(b.build());
    UniformBuffer buffer(uib.getSize());

    float4 f4(1.0f);
    ssize_t f4_offset = uib.getUniformOffset("f4a", 0);
    buffer.setUniformArray(f4_offset, &f4, 1);

    float2 f2(1.0f);
    ssize_t f2_offset = uib.getUniformOffset("f2a", 0);
    buffer.setUniformArray(f2_offset, &f2, 1);

    buffer.invalidate();
}

TEST(FilamentTest, BoxCulling) {
    Frustum frustum(mat4f::frustum(-1, 1, -1, 1, 1, 100));

    // a cube centered in 0 of size 1
    Box box = { 0, 0.5f };

    // box fully inside
    EXPECT_TRUE( frustum.intersects(box.translateTo({ 0,  0, -10})) );

    // box clipped by the near or far plane
    EXPECT_TRUE( frustum.intersects(box.translateTo({ 0,  0,   -1})) );
    EXPECT_TRUE( frustum.intersects(box.translateTo({ 0,  0, -100})) );

    // box clipped by one or several planes of the frustum for any z, but still visible
    EXPECT_TRUE( frustum.intersects(box.translateTo({ -10,   0, -10 })) );
    EXPECT_TRUE( frustum.intersects(box.translateTo({  10,   0, -10 })) );
    EXPECT_TRUE( frustum.intersects(box.translateTo({   0, -10, -10 })) );
    EXPECT_TRUE( frustum.intersects(box.translateTo({   0,  10, -10 })) );
    EXPECT_TRUE( frustum.intersects(box.translateTo({ -10, -10, -10 })) );
    EXPECT_TRUE( frustum.intersects(box.translateTo({  10,  10, -10 })) );
    EXPECT_TRUE( frustum.intersects(box.translateTo({  10, -10, -10 })) );
    EXPECT_TRUE( frustum.intersects(box.translateTo({ -10,  10, -10 })) );
    EXPECT_TRUE( frustum.intersects(box.translateTo({ -10,  10, -10 })) );
    EXPECT_TRUE( frustum.intersects(box.translateTo({  10, -10, -10 })) );

    // box outside frustum planes
    EXPECT_FALSE( frustum.intersects(box.translateTo({ 0,     0,    0})) );
    EXPECT_FALSE( frustum.intersects(box.translateTo({ 0,     0, -101})) );
    EXPECT_FALSE( frustum.intersects(box.translateTo({-1.51,  0, -0.5})) );

    // slightly inside the frustum
    EXPECT_TRUE( frustum.intersects(box.translateTo({-1.49,  0, -0.5})) );
    EXPECT_TRUE( frustum.intersects(box.translateTo({-100,   0, -100})) );

    // expected false classification (the box is not visible, but its classified as visible)
    EXPECT_TRUE( frustum.intersects(box.translateTo({-100.51, 0, -100})) );
    EXPECT_TRUE( frustum.intersects(box.translateTo({-100.99, 0, -100})) );
    EXPECT_FALSE(frustum.intersects(box.translateTo({-101.01, 0, -100})) ); // good again

    // A box that entirely contain the frustum
    EXPECT_TRUE( frustum.intersects( { 0, 200 }) );
}

TEST(FilamentTest, SphereCulling) {
    Frustum frustum(mat4f::frustum(-1, 1, -1, 1, 1, 100));

    // a sphere centered in 0 of size 1
    float4 sphere = { 0, 0, 0, 0.5f };

    // sphere fully inside
    EXPECT_TRUE( frustum.intersects(sphere + float4{ 0, 0, -10, 0}) );

    // sphere clipped by the near or far plane
    EXPECT_TRUE( frustum.intersects(sphere + float4{ 0, 0, -1, 0}) );
    EXPECT_TRUE( frustum.intersects(sphere + float4{ 0, 0, -100, 0}) );

    // sphere clipped by one or several planes of the frustum for any z, but still visible
    EXPECT_TRUE( frustum.intersects(sphere + float4{ -10, 0, -10, 0 }) );
    EXPECT_TRUE( frustum.intersects(sphere + float4{ 10, 0, -10, 0 }) );
    EXPECT_TRUE( frustum.intersects(sphere + float4{ 0, -10, -10, 0 }) );
    EXPECT_TRUE( frustum.intersects(sphere + float4{ 0, 10, -10, 0 }) );
    EXPECT_TRUE( frustum.intersects(sphere + float4{ -10, -10, -10, 0 }) );
    EXPECT_TRUE( frustum.intersects(sphere + float4{ 10, 10, -10, 0 }) );
    EXPECT_TRUE( frustum.intersects(sphere + float4{ 10, -10, -10, 0 }) );
    EXPECT_TRUE( frustum.intersects(sphere + float4{ -10, 10, -10, 0 }) );
    EXPECT_TRUE( frustum.intersects(sphere + float4{ -10, 10, -10, 0 }) );
    EXPECT_TRUE( frustum.intersects(sphere + float4{ 10, -10, -10, 0 }) );

    // sphere outside frustum planes
    EXPECT_FALSE( frustum.intersects(sphere + float4{ 0, 0, 0, 0}) );
    EXPECT_FALSE( frustum.intersects(sphere + float4{ 0, 0, -101, 0}) );
    EXPECT_FALSE( frustum.intersects(sphere + float4{ -1.51, 0, -0.5, 0}) );

    // slightly inside the frustum
    EXPECT_TRUE( frustum.intersects(sphere + float4{ -100, 0, -100, 0}) );

    // A sphere that entirely contain the frustum
    EXPECT_TRUE(frustum.intersects({ 0, 200 }));
}

TEST(FilamentTest, ColorConversion) {
    // Linear to Gamma
    // 0.0 stays 0.0
    EXPECT_PRED2(vec3eq, (sRGBColor{0.0f, 0.0f, 0.0f}), Color::toSRGB<FAST>(LinearColor{0.0f}));
    // 1.0 stays 1.0
    EXPECT_PRED2(vec3eq, (sRGBColor{1.0f, 0.0f, 0.0f}), Color::toSRGB<FAST>({1.0f, 0.0f, 0.0f}));

    // 0.0 stays 0.0
    EXPECT_PRED2(vec3eq, (sRGBColor{0.0f, 0.0f, 0.0f}),
            Color::toSRGB<ACCURATE>(LinearColor{0.0f}));
    // 1.0 stays 1.0
    EXPECT_PRED2(vec3eq, (sRGBColor{1.0f, 0.0f, 0.0f}),
            Color::toSRGB<ACCURATE>({1.0f, 0.0f, 0.0f}));

    EXPECT_LT((sRGBColor{0.5f, 0.0f, 0.0f}.x), Color::toSRGB<FAST>({0.5f, 0.0f, 0.0f}).x);

    EXPECT_LT((sRGBColor{0.5f, 0.0f, 0.0f}.x), Color::toSRGB<ACCURATE>({0.5f, 0.0f, 0.0f}).x);

    EXPECT_PRED1(isGray, Color::toSRGB<FAST>(LinearColor{0.5f}));
    EXPECT_PRED1(isGray, Color::toSRGB<ACCURATE>(LinearColor{0.5f}));

    // Gamma to Linear
    // 0.0 stays 0.0
    EXPECT_PRED2(vec3eq, (LinearColor{0.0f, 0.0f, 0.0f}), Color::toLinear<FAST>(sRGBColor{0.0f}));
    // 1.0 stays 1.0
    EXPECT_PRED2(vec3eq, (LinearColor{1.0f, 0.0f, 0.0f}), Color::toLinear<FAST>({1.0f, 0.0f, 0.0f}));

    // 0.0 stays 0.0
    EXPECT_PRED2(vec3eq, (LinearColor{0.0f, 0.0f, 0.0f}), Color::toLinear<ACCURATE>(sRGBColor{0.0f}));
    // 1.0 stays 1.0
    EXPECT_PRED2(vec3eq, (LinearColor{1.0f, 0.0f, 0.0f}), Color::toLinear<ACCURATE>({1.0f, 0.0f, 0.0f}));


    EXPECT_GT((LinearColor{0.5f, 0.0f, 0.0f}.x), Color::toLinear<FAST>({0.5f, 0.0f, 0.0f}).x);

    EXPECT_GT((LinearColor{0.5f, 0.0f, 0.0f}.x), Color::toLinear<ACCURATE>({0.5f, 0.0f, 0.0f}).x);

    EXPECT_PRED1(isGray, Color::toLinear<FAST>(sRGBColor{0.5f}));
    EXPECT_PRED1(isGray, Color::toLinear<ACCURATE>(sRGBColor{0.5f}));
}


TEST(FilamentTest, FroxelData) {
    using namespace filament;

    FEngine* engine = FEngine::create();

    LinearAllocatorArena arena("FRenderer: per-frame allocator", FEngine::CONFIG_PER_RENDER_PASS_ARENA_SIZE);
    utils::ArenaScope<LinearAllocatorArena> scope(arena);


    // view-port size is chosen so that we fit exactly a integer # of froxels horizontally
    // (unfortunately there is no way to guarantee it as it depends on the max # of froxel
    // used by the engine). We do this to infer the value of the left and right most planes
    // to check if they're computed correctly.
    Viewport vp(0, 0, 1280, 640);
    mat4f p = mat4f::perspective(90, 1.0f, 0.1, 100, mat4f::Fov::HORIZONTAL);

    Froxelizer froxelData(*engine);
    froxelData.setOptions(5, 100);
    froxelData.prepare(engine->getDriverApi(), scope, vp, p, 0.1, 100);

    Froxel f = froxelData.getFroxelAt(0,0,0);

    // 45-deg plane, with normal pointing outward to the left
    EXPECT_FLOAT_EQ(-F_SQRT2/2, f.planes[Froxel::LEFT].x);
    EXPECT_FLOAT_EQ(         0, f.planes[Froxel::LEFT].y);
    EXPECT_FLOAT_EQ( F_SQRT2/2, f.planes[Froxel::LEFT].z);

    // the right side of froxel 1 is near 45-deg plane pointing outward to the right
    EXPECT_TRUE(f.planes[Froxel::RIGHT].x > 0);
    EXPECT_FLOAT_EQ(0, f.planes[Froxel::RIGHT].y);
    EXPECT_TRUE(f.planes[Froxel::RIGHT].z < 0);

    // right side of last horizontal froxel is 45-deg plane pointing outward to the right
    Froxel g = froxelData.getFroxelAt(froxelData.getFroxelCountX()-1,0,0);
    EXPECT_FLOAT_EQ(F_SQRT2/2, g.planes[Froxel::RIGHT].x);
    EXPECT_FLOAT_EQ(        0, g.planes[Froxel::RIGHT].y);
    EXPECT_FLOAT_EQ(F_SQRT2/2, g.planes[Froxel::RIGHT].z);

    // first froxel near plane facing us
    EXPECT_FLOAT_EQ(        0, f.planes[Froxel::NEAR].x);
    EXPECT_FLOAT_EQ(        0, f.planes[Froxel::NEAR].y);
    EXPECT_FLOAT_EQ(        1, f.planes[Froxel::NEAR].z);

    // first froxel far plane away from us
    EXPECT_FLOAT_EQ(        0, f.planes[Froxel::FAR].x);
    EXPECT_FLOAT_EQ(        0, f.planes[Froxel::FAR].y);
    EXPECT_FLOAT_EQ(       -1, f.planes[Froxel::FAR].z);

    // first froxel near plane distance always 0
    EXPECT_FLOAT_EQ(        0, f.planes[Froxel::NEAR].w);

    // first froxel far plane distance always zLightNear
    EXPECT_FLOAT_EQ(        5,-f.planes[Froxel::FAR].w);

    Froxel l = froxelData.getFroxelAt(0, 0, froxelData.getFroxelCountZ()-1);

    // farthest froxel far plane distance always zLightFar
    EXPECT_FLOAT_EQ(        100,-l.planes[Froxel::FAR].w);

    // create a dummy point light that can be referenced in LightSoa
    Entity e = engine->getEntityManager().create();
    LightManager::Builder(LightManager::Type::POINT).build(*engine, e);
    LightManager::Instance instance = engine->getLightManager().getInstance(e);

    FScene::LightSoa lights;
    lights.push_back({}, {}, {}, {}, {}, {});   // first one is always skipped
    lights.push_back(float4{ 0, 0, -5, 1 }, {}, instance, 1, {}, {});

    {
        froxelData.froxelizeLights(*engine, {}, lights);
        auto const& froxelBuffer = froxelData.getFroxelBufferUser();
        auto const& recordBuffer = froxelData.getRecordBufferUser();
        // light straddles the "light near" plane
        size_t pointCount = 0;
        for (const auto& entry : froxelBuffer) {
            EXPECT_LE(entry.count, 1);
            pointCount += entry.count;
        }
        EXPECT_GT(pointCount, 0);
    }

    {
        // light doesn't cross any froxel near or far plane
        lights.elementAt<FScene::POSITION_RADIUS>(1) = float4{ 0, 0, -3, 1 };

        auto pos = lights.elementAt<FScene::POSITION_RADIUS>(1);
        EXPECT_TRUE(pos == float4( 0, 0, -3, 1 ));

        froxelData.froxelizeLights(*engine, {}, lights);
        auto const& froxelBuffer = froxelData.getFroxelBufferUser();
        auto const& recordBuffer = froxelData.getRecordBufferUser();
        size_t pointCount = 0;
        for (const auto& entry : froxelBuffer) {
            EXPECT_LE(entry.count, 1);
            pointCount += entry.count;
        }
        EXPECT_GT(pointCount, 0);
    }

    froxelData.terminate(engine->getDriverApi());

    Engine::destroy((Engine **)&engine);
}

TEST(FilamentTest, Bones) {

    struct Shader {
        static mat3f normal(PerRenderableUibBone const& bone) noexcept {
            quatf q = bone.q;
            float3 is = bone.ns.xyz;
            return mat3f(mat3(q) * mat3::scaling(is));
        }
        static  mat4f vertice(PerRenderableUibBone const& bone) noexcept {
            quatf q = bone.q;
            float3 t = bone.t.xyz;
            float3 s = bone.s.xyz;
            return mat4f(mat4::translation(t) * mat4(q) * mat4::scaling(s));
        }
        static float3 normal(float3 n, PerRenderableUibBone const& bone) noexcept {
            quatf q = bone.q;
            float3 is = bone.ns.xyz;
            // apply the inverse of the non-uniform scales
            n *= is;
            // apply the rigid transform
            n += 2.0 * cross(q.xyz, cross(q.xyz, n) + q.w * n);
            return n;
        }
        static  float3 vertice(float3 v, PerRenderableUibBone const& bone) noexcept {
            quatf q = bone.q;
            float3 t = bone.t.xyz;
            float3 s = bone.s.xyz;
            // apply the non-uniform scales
            v *= s;
            // apply the rigid transform
            v += 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
            // apply the translation
            v += t;
            return v;
        }
    };

    struct Test {
        static inline double epsilon(double x, double y) {
            double maxXYOne = std::max({ 1.0, std::fabs(x), std::fabs(y) });
            return 1e-5 * maxXYOne;
        }

        static void expect_eq(mat4f e, mat4f a) noexcept {
            for (size_t j = 0; j < 4; j++) {
                for (size_t i = 0; i < 4; i++) {
                    EXPECT_NEAR(e[i][j], a[i][j], epsilon(e[i][j], a[i][j]));
                }
            }
        }
        static void expect_eq(mat3f e, mat3f a) noexcept {
            for (size_t j = 0; j < 3; j++) {
                for (size_t i = 0; i < 3; i++) {
                    EXPECT_NEAR(e[i][j], a[i][j], epsilon(e[i][j], a[i][j]));
                }
            }
        }
        static void expect_eq(float3 e, float3 a) noexcept {
            for (size_t i = 0; i < 3; i++) {
                EXPECT_NEAR(e[i], a[i], epsilon(e[i], a[i]));
            }
        }

        static void check(mat4f const& m) noexcept {
            PerRenderableUibBone b;
            FRenderableManager::makeBone(&b, m);

            expect_eq(Shader::vertice(b), m);

            mat3f n = transpose(inverse(m.upperLeft()));
            n *= mat3f(1.0f / std::sqrt(max(float3{length2(n[0]), length2(n[1]), length2(n[2])})));
            expect_eq(Shader::normal(b), n);
        }

        static void check(mat4f const& m, float3 const& v) noexcept {
            PerRenderableUibBone b;
            FRenderableManager::makeBone(&b, m);

            expect_eq((m * v).xyz, Shader::vertice(v, b));

            mat3f n = transpose(inverse(m.upperLeft()));
            n *= mat3f(1.0f / std::sqrt(max(float3{length2(n[0]), length2(n[1]), length2(n[2])})));
            expect_eq(n * normalize(v), Shader::normal(normalize(v), b));

            float3 normal = n * normalize(v);
            EXPECT_LE(max(abs(normal)), 1.0);
        }
    };

    Test::check(mat4f{});
    Test::check(mat4f::translation(float3{ 1, 2, 3 }));

    Test::check(mat4f::scaling(float3{ 2, 2, 2 }));

    Test::check(mat4f::scaling(float3{ 4, 2, 3 }));
    Test::check(mat4f::scaling(float3{ 4, -2, -3 }));
    Test::check(mat4f::scaling(float3{ -4, 2, -3 }));
    Test::check(mat4f::scaling(float3{ -4, -2, 3 }));

    Test::check(mat4f::scaling(float3{ -4, -2, -3 }));
    Test::check(mat4f::scaling(float3{ -4, 2, 3 }));
    Test::check(mat4f::scaling(float3{ 4, -2, 3 }));
    Test::check(mat4f::scaling(float3{ 4, 2, -3 }));

    Test::check(mat4f::rotation(F_PI_2, float3{ 0, 0, 1 }));
    Test::check(mat4f::rotation(F_PI_2, float3{ 0, 1, 0 }));
    Test::check(mat4f::rotation(F_PI_2, float3{ 1, 0, 0 }));
    Test::check(mat4f::rotation(F_PI_2, float3{ 0, 1, 1 }));
    Test::check(mat4f::rotation(F_PI_2, float3{ 1, 0, 1 }));
    Test::check(mat4f::rotation(F_PI_2, float3{ 1, 1, 0 }));
    Test::check(mat4f::rotation(-F_PI_2, float3{ 0, 0, 1 }));
    Test::check(mat4f::rotation(-F_PI_2, float3{ 0, 1, 0 }));
    Test::check(mat4f::rotation(-F_PI_2, float3{ 1, 0, 0 }));
    Test::check(mat4f::rotation(-F_PI_2, float3{ 0, 1, 1 }));
    Test::check(mat4f::rotation(-F_PI_2, float3{ 1, 0, 1 }));
    Test::check(mat4f::rotation(-F_PI_2, float3{ 1, 1, 0 }));

    mat4f m = mat4f::translation(float3{ 1, 2, 3 }) *
                                                    mat4f::rotation(-F_PI_2, float3{ 1, 1, 0 }) *
                                                    mat4f::scaling(float3{ -2, 3, 0.04 });

    Test::check(m);

    std::default_random_engine generator(82828);
    std::uniform_real_distribution<float> distribution(-100.0f, 100.0f);
    auto rand_gen = std::bind(distribution, generator);

    for (size_t i = 0; i < 100; ++i) {
        float3 p(rand_gen(), rand_gen(), rand_gen());
        Test::check(m, p);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
