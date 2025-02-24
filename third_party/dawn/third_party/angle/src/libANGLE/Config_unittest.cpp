//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "libANGLE/AttributeMap.h"
#include "libANGLE/Config.h"

// Create a generic, valid EGL config that can be modified to test sorting and
// filtering routines
static egl::Config GenerateGenericConfig()
{
    egl::Config config;

    config.bufferSize          = 24;
    config.redSize             = 8;
    config.greenSize           = 8;
    config.blueSize            = 8;
    config.luminanceSize       = 0;
    config.alphaSize           = 8;
    config.alphaMaskSize       = 0;
    config.bindToTextureRGB    = EGL_TRUE;
    config.bindToTextureRGBA   = EGL_TRUE;
    config.colorBufferType     = EGL_RGB_BUFFER;
    config.configCaveat        = EGL_NONE;
    config.configID            = 0;
    config.conformant          = EGL_OPENGL_ES2_BIT;
    config.depthSize           = 24;
    config.level               = 0;
    config.matchNativePixmap   = EGL_NONE;
    config.maxPBufferWidth     = 1024;
    config.maxPBufferHeight    = 1024;
    config.maxPBufferPixels    = config.maxPBufferWidth * config.maxPBufferWidth;
    config.maxSwapInterval     = 0;
    config.minSwapInterval     = 4;
    config.nativeRenderable    = EGL_OPENGL_ES2_BIT;
    config.nativeVisualID      = 0;
    config.nativeVisualType    = 0;
    config.renderableType      = EGL_FALSE;
    config.sampleBuffers       = 0;
    config.samples             = 0;
    config.stencilSize         = 8;
    config.surfaceType         = EGL_PBUFFER_BIT | EGL_WINDOW_BIT | EGL_SWAP_BEHAVIOR_PRESERVED_BIT;
    config.transparentType     = EGL_NONE;
    config.transparentRedValue = 0;
    config.transparentGreenValue = 0;
    config.transparentBlueValue  = 0;

    return config;
}

static std::vector<egl::Config> GenerateUniqueConfigs(size_t count)
{
    std::vector<egl::Config> configs;

    for (size_t i = 0; i < count; i++)
    {
        egl::Config config = GenerateGenericConfig();
        config.samples     = static_cast<EGLint>(i);
        configs.push_back(config);
    }

    return configs;
}

// Add unique configs to a ConfigSet and expect that the size of the
// set is equal to the number of configs added.
TEST(ConfigSetTest, Size)
{
    egl::ConfigSet set;

    std::vector<egl::Config> uniqueConfigs = GenerateUniqueConfigs(16);
    for (size_t i = 0; i < uniqueConfigs.size(); i++)
    {
        set.add(uniqueConfigs[i]);
        EXPECT_EQ(set.size(), i + 1);
    }
}

// [EGL 1.5] section 3.4:
// EGL_CONFIG_ID is a unique integer identifying different EGLConfigs. Configuration IDs
// must be small positive integers starting at 1 and ID assignment should be compact;
// that is, if there are N EGLConfigs defined by the EGL implementation, their
// configuration IDs should be in the range [1, N].
TEST(ConfigSetTest, IDs)
{
    egl::ConfigSet set;

    std::set<EGLint> ids;

    std::vector<egl::Config> uniqueConfigs = GenerateUniqueConfigs(16);
    for (size_t i = 0; i < uniqueConfigs.size(); i++)
    {
        EGLint id = set.add(uniqueConfigs[i]);

        // Check that the config that was inserted has the ID that was returned
        // by ConfigSet::add
        EXPECT_EQ(id, set.get(id).configID);

        ids.insert(id);
    }

    // Verify configCount unique IDs
    EXPECT_EQ(ids.size(), set.size());

    // Check that there are no gaps and the IDs are in the range [1, N].
    EXPECT_EQ(*std::min_element(ids.begin(), ids.end()), 1);
    EXPECT_EQ(*std::max_element(ids.begin(), ids.end()), static_cast<EGLint>(set.size()));
}

// Test case to verify filtering of egl::ConfigSet based on bit size attributes
// (e.g., EGL_RED_SIZE, EGL_GREEN_SIZE, etc.). The test generates configurations
// with varying bit sizes for each attribute, filters by the attribute, and
// checks that the number of filtered results matches the expected count.
TEST(ConfigSetTest, FilteringBitSizes)
{
    egl::ConfigSet set;

    struct VariableConfigBitSize
    {
        EGLint Name;
        EGLint(egl::Config::*ConfigMember);
    };

    VariableConfigBitSize testMembers[] = {
        {EGL_RED_SIZE, &egl::Config::redSize},     {EGL_GREEN_SIZE, &egl::Config::greenSize},
        {EGL_BLUE_SIZE, &egl::Config::blueSize},   {EGL_ALPHA_SIZE, &egl::Config::alphaSize},
        {EGL_DEPTH_SIZE, &egl::Config::depthSize}, {EGL_STENCIL_SIZE, &egl::Config::stencilSize},
    };

    // Generate configsPerType configs with varying bit sizes of each type
    size_t configsPerType = 4;
    for (size_t i = 0; i < ArraySize(testMembers); i++)
    {
        for (size_t j = 0; j < configsPerType; j++)
        {
            egl::Config config = GenerateGenericConfig();

            // Set all the other tested members of this config to 0
            for (size_t k = 0; k < ArraySize(testMembers); k++)
            {
                config.*(testMembers[k].ConfigMember) = 0;
            }

            // Set the tested member of this config to i so it ranges from
            // [1, configsPerType]
            config.*(testMembers[i].ConfigMember) = static_cast<EGLint>(j) + 1;

            set.add(config);
        }
    }

    // for each tested member, filter by it's type and verify that the correct number
    // of results are returned
    for (size_t i = 0; i < ArraySize(testMembers); i++)
    {
        // Start with a filter of 1 to not grab the other members
        for (EGLint j = 0; j < static_cast<EGLint>(configsPerType); j++)
        {
            egl::AttributeMap filter;
            filter.insert(testMembers[i].Name, j + 1);

            std::vector<const egl::Config *> filteredConfigs = set.filter(filter);

            EXPECT_EQ(filteredConfigs.size(), configsPerType - j);
        }
    }
}

// Verify the sorting, [EGL 1.5] section 3.4.1.2 pg 30:
// [configs are sorted] by larger total number of color bits (for an RGB
// color buffer this is the sum of EGL_RED_SIZE, EGL_GREEN_SIZE, EGL_BLUE_SIZE,
// and EGL_ALPHA_SIZE; for a luminance color buffer, the sum of EGL_LUMINANCE_SIZE
// and EGL_ALPHA_SIZE).If the requested number of bits in attrib list for a
// particular color component is 0 or EGL_DONT_CARE, then the number of bits
// for that component is not considered.
TEST(ConfigSetTest, SortingBitSizes)
{
    egl::ConfigSet set;
    size_t testConfigCount = 64;
    for (size_t i = 0; i < testConfigCount; i++)
    {
        egl::Config config = GenerateGenericConfig();

        // Give random-ish bit sizes to the config
        config.redSize   = (i * 2) % 3;
        config.greenSize = (i + 5) % 7;
        config.blueSize  = (i + 7) % 11;
        config.alphaSize = (i + 13) % 17;

        set.add(config);
    }

    egl::AttributeMap greaterThan1BitFilter;
    greaterThan1BitFilter.insert(EGL_RED_SIZE, 1);
    greaterThan1BitFilter.insert(EGL_GREEN_SIZE, 1);
    greaterThan1BitFilter.insert(EGL_BLUE_SIZE, 1);
    greaterThan1BitFilter.insert(EGL_ALPHA_SIZE, 1);

    std::vector<const egl::Config *> filteredConfigs = set.filter(greaterThan1BitFilter);
    for (size_t i = 1; i < filteredConfigs.size(); i++)
    {
        const egl::Config &prevConfig = *filteredConfigs[i - 1];
        size_t prevBitCount =
            prevConfig.redSize + prevConfig.greenSize + prevConfig.blueSize + prevConfig.alphaSize;

        const egl::Config &curConfig = *filteredConfigs[i];
        size_t curBitCount =
            curConfig.redSize + curConfig.greenSize + curConfig.blueSize + curConfig.alphaSize;

        EXPECT_GE(prevBitCount, curBitCount);
    }
}
