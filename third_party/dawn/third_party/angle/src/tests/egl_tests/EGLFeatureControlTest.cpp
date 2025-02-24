//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Tests the eglQueryStringiANGLE and eglQueryDisplayAttribANGLE functions exposed by the
// extension EGL_ANGLE_feature_control.

#include <gtest/gtest.h>
#include <optional>

#include "common/string_utils.h"
#include "libANGLE/Display.h"
#include "test_utils/ANGLETest.h"

using namespace angle;

class EGLFeatureControlTest : public ANGLETest<>
{
  public:
    void testSetUp() override { mDisplay = EGL_NO_DISPLAY; }

    void testTearDown() override
    {
        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglTerminate(mDisplay);
        }
    }

  protected:
    EGLDisplay mDisplay;

    bool initTest()
    {
        // http://anglebug.com/42262291 This test sporadically times out on Win10/Intel
        if (IsWindows() && IsIntel())
            return false;

        EGLAttrib dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(), EGL_NONE};
        mDisplay              = eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE,
                                                      reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        EXPECT_NE(mDisplay, EGL_NO_DISPLAY);

        EXPECT_EQ(eglInitialize(mDisplay, nullptr, nullptr), static_cast<EGLBoolean>(EGL_TRUE));

        EXPECT_TRUE(IsEGLClientExtensionEnabled("EGL_ANGLE_feature_control"));

        return true;
    }

    using FeatureNameModifier = std::function<std::string(const std::string &)>;
    void testOverrideFeatures(FeatureNameModifier modifyName);
};

// Ensure eglQueryStringiANGLE generates EGL_BAD_DISPLAY if the display passed in is invalid.
TEST_P(EGLFeatureControlTest, InvalidDisplay)
{
    ANGLE_SKIP_TEST_IF(!initTest());
    EXPECT_EQ(nullptr, eglQueryStringiANGLE(EGL_NO_DISPLAY, EGL_FEATURE_NAME_ANGLE, 0));
    EXPECT_EGL_ERROR(EGL_BAD_DISPLAY);
}

// Ensure eglQueryStringiANGLE generates EGL_BAD_PARAMETER if the index is negative.
TEST_P(EGLFeatureControlTest, NegativeIndex)
{
    ANGLE_SKIP_TEST_IF(!initTest());
    EXPECT_EQ(nullptr, eglQueryStringiANGLE(mDisplay, EGL_FEATURE_NAME_ANGLE, -1));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
}

// Ensure eglQueryStringiANGLE generates EGL_BAD_PARAMETER if the index is out of bounds.
TEST_P(EGLFeatureControlTest, IndexOutOfBounds)
{
    ANGLE_SKIP_TEST_IF(!initTest());
    egl::Display *display = static_cast<egl::Display *>(mDisplay);
    EXPECT_EQ(nullptr, eglQueryStringiANGLE(mDisplay, EGL_FEATURE_NAME_ANGLE,
                                            display->getFeatures().size()));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
}

// Ensure eglQueryStringiANGLE generates EGL_BAD_PARAMETER if the name is not one of the valid
// options specified in EGL_ANGLE_feature_control.
TEST_P(EGLFeatureControlTest, InvalidName)
{
    ANGLE_SKIP_TEST_IF(!initTest());
    EXPECT_EQ(nullptr, eglQueryStringiANGLE(mDisplay, 100, 0));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
}

// For each valid name and index in the feature description arrays, query the values and ensure
// that no error is generated, and that the values match the correct values frim ANGLE's display's
// FeatureList.
TEST_P(EGLFeatureControlTest, QueryAll)
{
    ANGLE_SKIP_TEST_IF(!initTest());
    egl::Display *display       = static_cast<egl::Display *>(mDisplay);
    angle::FeatureList features = display->getFeatures();
    for (size_t i = 0; i < features.size(); i++)
    {
        EXPECT_STREQ(features[i]->name, eglQueryStringiANGLE(mDisplay, EGL_FEATURE_NAME_ANGLE, i));
        EXPECT_STREQ(FeatureCategoryToString(features[i]->category),
                     eglQueryStringiANGLE(mDisplay, EGL_FEATURE_CATEGORY_ANGLE, i));
        EXPECT_STREQ(FeatureStatusToString(features[i]->enabled),
                     eglQueryStringiANGLE(mDisplay, EGL_FEATURE_STATUS_ANGLE, i));
        ASSERT_EGL_SUCCESS();
    }
}

// Ensure eglQueryDisplayAttribANGLE returns the correct number of features when queried with
// attribute EGL_FEATURE_COUNT_ANGLE
TEST_P(EGLFeatureControlTest, FeatureCount)
{
    ANGLE_SKIP_TEST_IF(!initTest());
    egl::Display *display = static_cast<egl::Display *>(mDisplay);
    EGLAttrib value       = -1;
    EXPECT_EQ(static_cast<EGLBoolean>(EGL_TRUE),
              eglQueryDisplayAttribANGLE(mDisplay, EGL_FEATURE_COUNT_ANGLE, &value));
    EXPECT_EQ(display->getFeatures().size(), static_cast<size_t>(value));
    ASSERT_EGL_SUCCESS();
}

void EGLFeatureControlTest::testOverrideFeatures(FeatureNameModifier modifyName)
{
    ANGLE_SKIP_TEST_IF(!initTest());
    egl::Display *display       = static_cast<egl::Display *>(mDisplay);
    angle::FeatureList features = display->getFeatures();

    // Build lists of features to enable/disabled. Toggle features we know are ok to toggle based
    // from this list.
    std::vector<const char *> enabled;
    std::vector<const char *> disabled;
    std::vector<std::string> modifiedNameStorage;
    std::vector<bool> shouldBe;
    std::vector<std::string> testedFeatures = {
        // Safe to toggle on GL
        angle::GetFeatureName(angle::Feature::AddAndTrueToLoopCondition),
        angle::GetFeatureName(angle::Feature::ClampFragDepth),
        // Safe to toggle on GL and Vulkan
        angle::GetFeatureName(angle::Feature::ClampPointSize),
        // Safe to toggle on D3D
        angle::GetFeatureName(angle::Feature::ZeroMaxLodWorkaround),
        angle::GetFeatureName(angle::Feature::ExpandIntegerPowExpressions),
        angle::GetFeatureName(angle::Feature::RewriteUnaryMinusOperator),
    };

    modifiedNameStorage.reserve(features.size());
    shouldBe.reserve(features.size());

    for (size_t i = 0; i < features.size(); i++)
    {
        modifiedNameStorage.push_back(modifyName(features[i]->name));

        bool toggle = std::find(testedFeatures.begin(), testedFeatures.end(),
                                std::string(features[i]->name)) != testedFeatures.end();
        if (features[i]->enabled ^ toggle)
        {
            enabled.push_back(modifiedNameStorage[i].c_str());
        }
        else
        {
            disabled.push_back(modifiedNameStorage[i].c_str());
        }
        // Save what we expect the feature status will be when checking later.
        shouldBe.push_back(features[i]->enabled ^ toggle);
    }
    disabled.push_back(0);
    enabled.push_back(0);

    // Terminate the old display (we just used it to collect features)
    eglTerminate(mDisplay);

    // Create a new display with these overridden features.
    EGLAttrib dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE,
                             GetParam().getRenderer(),
                             EGL_FEATURE_OVERRIDES_ENABLED_ANGLE,
                             reinterpret_cast<EGLAttrib>(enabled.data()),
                             EGL_FEATURE_OVERRIDES_DISABLED_ANGLE,
                             reinterpret_cast<EGLAttrib>(disabled.data()),
                             EGL_NONE};
    mDisplay              = eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE,
                                                  reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(mDisplay, EGL_NO_DISPLAY);
    ASSERT_EQ(eglInitialize(mDisplay, nullptr, nullptr), EGLBoolean(EGL_TRUE));

    // Check that all features have the correct status (even the ones we toggled).
    for (size_t i = 0; i < features.size(); i++)
    {
        EXPECT_STREQ(FeatureStatusToString(shouldBe[i]),
                     eglQueryStringiANGLE(mDisplay, EGL_FEATURE_STATUS_ANGLE, i))
            << modifiedNameStorage[i];
    }
}

// Submit a list of features to override when creating the display with eglGetPlatformDisplay, and
// ensure that the features are correctly overridden.
TEST_P(EGLFeatureControlTest, OverrideFeatures)
{
    testOverrideFeatures([](const std::string &featureName) { return featureName; });
}

// Similar to OverrideFeatures, but ensures that camelCase variants of the name match as well.
TEST_P(EGLFeatureControlTest, OverrideFeaturesCamelCase)
{
    testOverrideFeatures(
        [](const std::string &featureName) { return angle::ToCamelCase(featureName); });
}

// Similar to OverrideFeatures, but ensures wildcard matching works
TEST_P(EGLFeatureControlTest, OverrideFeaturesWildcard)
{
    for (int j = 0; j < 2; j++)
    {
        const bool testEnableOverride = (j != 0);

        ANGLE_SKIP_TEST_IF(!initTest());

        egl::Display *display       = static_cast<egl::Display *>(mDisplay);
        angle::FeatureList features = display->getFeatures();

        // Note that we don't use the broader 'prefer_*' here because
        // prefer_monolithic_pipelines_over_libraries may affect other feature
        // flags.
        std::vector<const char *> featuresToOverride = {"prefer_d*", nullptr};

        std::vector<std::string> featureNameStorage;
        std::vector<bool> shouldBe;

        shouldBe.reserve(features.size());
        featureNameStorage.reserve(features.size());

        for (size_t i = 0; i < features.size(); i++)
        {
            std::string featureName = std::string(features[i]->name);
            std::transform(featureName.begin(), featureName.end(), featureName.begin(),
                           [](unsigned char c) { return std::tolower(c); });

            const bool featureMatch = strncmp(featureName.c_str(), "preferd", 7) == 0;

            std::optional<bool> overrideState;
            if (featureMatch)
            {
                overrideState = testEnableOverride;
            }

            // Save what we expect the feature status will be when checking later.
            shouldBe.push_back(overrideState.value_or(features[i]->enabled));
            featureNameStorage.push_back(features[i]->name);
        }

        // Terminate the old display (we just used it to collect features)
        eglTerminate(mDisplay);
        mDisplay = nullptr;

        // Create a new display with these overridden features.
        EGLAttrib dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(),
                                 testEnableOverride ? EGL_FEATURE_OVERRIDES_ENABLED_ANGLE
                                                    : EGL_FEATURE_OVERRIDES_DISABLED_ANGLE,
                                 reinterpret_cast<EGLAttrib>(featuresToOverride.data()), EGL_NONE};
        mDisplay              = eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE,
                                                      reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        ASSERT_EGL_SUCCESS();
        ASSERT_NE(mDisplay, EGL_NO_DISPLAY);
        ASSERT_EQ(eglInitialize(mDisplay, nullptr, nullptr), EGLBoolean(EGL_TRUE));

        // Check that all features have the correct status (even the ones we toggled).
        for (size_t i = 0; i < features.size(); i++)
        {
            EXPECT_STREQ(FeatureStatusToString(shouldBe[i]),
                         eglQueryStringiANGLE(mDisplay, EGL_FEATURE_STATUS_ANGLE, i))
                << featureNameStorage[i];
        }

        // Clean up display for next iteration.
        eglTerminate(mDisplay);
        mDisplay = nullptr;
    }
}

// Ensure that dependent features are affected properly by overrides
TEST_P(EGLFeatureControlTest, OverrideFeaturesDependent)
{
    ANGLE_SKIP_TEST_IF(!initTest());

    egl::Display *display       = static_cast<egl::Display *>(mDisplay);
    angle::FeatureList features = display->getFeatures();

    const std::vector<const char *> featuresDisabled = {
        GetFeatureName(Feature::SupportsRenderpass2),
        GetFeatureName(Feature::SupportsImage2dViewOf3d), nullptr};

    const std::vector<const char *> featuresExpectDisabled = {
        // Features we changed
        GetFeatureName(Feature::SupportsRenderpass2),
        GetFeatureName(Feature::SupportsImage2dViewOf3d),

        // Features that must become disabled as a result of the above
        GetFeatureName(Feature::SupportsDepthStencilResolve),
        GetFeatureName(Feature::SupportsDepthStencilIndependentResolveNone),
        GetFeatureName(Feature::SupportsSampler2dViewOf3d),
        GetFeatureName(Feature::SupportsFragmentShadingRate),
    };

    // Features that could be different on some vendors
    const std::set<std::string> featuresThatCouldBeDifferent = {
        // Depends-on Feature::SupportsDepthStencilResolve
        GetFeatureName(Feature::EnableMultisampledRenderToTexture),
        // Depends-on Feature::SupportsFragmentShadingRate
        GetFeatureName(Feature::SupportsFoveatedRendering),
        // Depends-on Feature::EnableMultisampledRenderToTexture
        GetFeatureName(Feature::PreferDynamicRendering),
    };

    std::vector<std::string> featureNameStorage;
    std::vector<bool> shouldBe;

    shouldBe.reserve(features.size());
    featureNameStorage.reserve(features.size());

    for (size_t i = 0; i < features.size(); i++)
    {
        bool featureMatch = false;
        for (auto *ptr : featuresExpectDisabled)
        {
            if (strcmp(ptr, features[i]->name) == 0)
            {
                featureMatch = true;
                break;
            }
        }

        std::string featureName = std::string(features[i]->name);
        std::transform(featureName.begin(), featureName.end(), featureName.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        // Save what we expect the feature status will be when checking later.
        shouldBe.push_back(features[i]->enabled && !featureMatch);

        // Store copy of the feature name string, in case we need to print for a test failure
        featureNameStorage.push_back(features[i]->name);
    }

    // Terminate the old display (we just used it to collect features)
    eglTerminate(mDisplay);

    // Create a new display with these overridden features.
    EGLAttrib dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(),
                             EGL_FEATURE_OVERRIDES_DISABLED_ANGLE,
                             reinterpret_cast<EGLAttrib>(featuresDisabled.data()), EGL_NONE};
    mDisplay              = eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE,
                                                  reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(mDisplay, EGL_NO_DISPLAY);
    ASSERT_EQ(eglInitialize(mDisplay, nullptr, nullptr), EGLBoolean(EGL_TRUE));

    // Check that all features have the correct status (even the ones we toggled).
    for (size_t i = 0; i < features.size(); i++)
    {
        if (featuresThatCouldBeDifferent.count(featureNameStorage[i]) > 0)
        {
            // On some vendors these features could be different
            continue;
        }

        EXPECT_STREQ(FeatureStatusToString(shouldBe[i]),
                     eglQueryStringiANGLE(mDisplay, EGL_FEATURE_STATUS_ANGLE, i))
            << featureNameStorage[i];
    }
}

ANGLE_INSTANTIATE_TEST(EGLFeatureControlTest,
                       WithNoFixture(ES2_D3D9()),
                       WithNoFixture(ES2_D3D11()),
                       WithNoFixture(ES2_METAL()),
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_D3D11()),
                       WithNoFixture(ES3_METAL()),
                       WithNoFixture(ES3_OPENGL()));
