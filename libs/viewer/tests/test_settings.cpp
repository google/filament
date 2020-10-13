/*
 * Copyright 2020 The Android Open Source Project
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

#include <viewer/AutomationSpec.h>
#include <viewer/Settings.h>

#include <gtest/gtest.h>

using namespace filament::viewer;

class ViewSettingsTest : public testing::Test {};

static const char* JSON_TEST_DEFAULTS = R"TXT(
{
    "view": {
        "sampleCount": 1,
        "antiAliasing": "FXAA",
        "taa": {
            "enabled": false,
            "filterWidth": 1.0,
            "feedback": 0.04
        },
        "colorGrading": {
            "enabled": true,
            "quality": "MEDIUM",
            "toneMapping": "ACES_LEGACY",
            "temperature": 0,
            "tint": 0,
            "outRed": [1.0, 0.0, 0.0],
            "outGreen": [0.0, 1.0, 0.0],
            "outBlue": [0.0, 0.0, 1.0],
            "shadows": [1.0, 1.0, 1.0, 0.0],
            "midtones": [1.0, 1.0, 1.0, 0.0],
            "highlights": [1.0, 1.0, 1.0, 0.0],
            "ranges": [0.0, 0.333, 0.550, 1.0],
            "contrast": 1.0,
            "vibrance": 1.0,
            "saturation": 1.0,
            "slope": [1.0, 1.0, 1.0],
            "offset": [0.0, 0.0, 0.0],
            "power": [1.0, 1.0, 1.0],
            "gamma": [1.0, 1.0, 1.0],
            "midPoint": [1.0, 1.0, 1.0],
            "scale": [1.0, 1.0, 1.0],
            "linkedCurves": false
        },
        "ssao": {
            "enabled": false,
            "radius": 0.3,
            "power": 1.0,
            "bias": 0.0005,
            "resolution": 0.5,
            "intensity": 1.0,
            "quality": "LOW",
            "upsampling": "LOW",
            "minHorizonAngleRad": 0.0,
            "ssct": {
                "enabled": false,
                "lightConeRad": 1.0,
                "shadowDistance": 0.3,
                "contactDistanceMax": 1.0,
                "intensity": 0.8,
                "lightDirection": [0, -1, 0],
                "depthBias": 0.01,
                "depthSlopeBias": 0.01,
                "sampleCount": 4
            }
        },
        "bloom": {
            "enabled": false,
            "strength": 0.10,
            "resolution": 360,
            "anamorphism": 1.0,
            "levels": 6,
            "blendMode": "ADD",
            "threshold": true,
            "highlight": 1000.0
        },
        "fog": {
            "enabled": false,
            "distance": 0.0,
            "maximumOpacity": 1.0,
            "height": 0.0,
            "heightFalloff": 1.0,
            "color": [0.5, 0.5, 0.5],
            "density": 0.1,
            "inScatteringStart": 0.0,
            "inScatteringSize": -1.0,
            "fogColorFromIbl": false
        },
        "dof": {
            "enabled": false,
            "focusDistance": 10.0,
            "cocScale": 1.0,
            "maxApertureDiameter": 0.01
        },
        "vignette": {
            "enabled": false,
            "midPoint": 0.5,
            "roundness": 0.5,
            "feather": 0.5,
            "color": [0, 0, 0, 1]
        },
        "dithering": "TEMPORAL",
        "renderQuality": {
            "hdrColorBuffer": "HIGH"
        },
        "dynamicLighting": {
            "zLightNear": 5,
            "zLightFar": 100,
        },
        "shadowType": "PCF",
        "vsmShadowOptions": {
            "anisotropy": 1
        },
        "postProcessingEnabled": true
    }
}
)TXT";

static const char* JSON_TEST_MATERIAL = R"TXT(
"material": {
  "scalar": { "foo": 5.0, "bar": 2.0 },
  "float3": { "baz": [1, 2, 3] }
})TXT";

static const char* JSON_TEST_AUTOMATION = R"TXT([{
    "name": "test_72_cases",
    "base": { "view.bloom.strength": 0.5 },
    "permute": {
        "view.bloom.enabled": [false, true],
        "material.scalar.metallicFactor": [0.0, 0.2, 0.4, 0.6, 0.8, 1.0],
        "material.scalar.roughnessFactor": [0.0, 0.2, 0.4, 0.6, 0.8, 1.0]
    }
}])TXT";

TEST_F(ViewSettingsTest, JsonTestDefaults) {
    Settings settings1 = {0};
    ASSERT_TRUE(readJson(JSON_TEST_DEFAULTS, strlen(JSON_TEST_DEFAULTS), &settings1));

    ASSERT_TRUE(settings1.view.bloom.threshold);

    Settings settings2;
    ASSERT_TRUE(readJson("{}", strlen("{}"), &settings2));
    ASSERT_FALSE(readJson("{ badly_formed }", strlen("{ badly_formed }"), &settings2));

    Settings settings3;
    ASSERT_EQ(writeJson(settings2), writeJson(settings3));
}

TEST_F(ViewSettingsTest, JsonTestMaterial) {
    Settings settings = {0};
    std::string js = "{" + std::string(JSON_TEST_MATERIAL) + "}";
    ASSERT_TRUE(readJson(js.c_str(), js.size(), &settings));
    std::string serialized = writeJson(settings);
    ASSERT_PRED_FORMAT2(testing::IsSubstring, "\"baz\": [1, 2, 3]", serialized);
}

TEST_F(ViewSettingsTest, DefaultAutomationSpec) {
    AutomationSpec* specs = AutomationSpec::generateDefaultTestCases();
    ASSERT_TRUE(specs);
    ASSERT_EQ(specs->size(), 65);

    Settings settings;

    ASSERT_TRUE(specs->get(0, &settings));
    ASSERT_FALSE(settings.view.postProcessingEnabled);
    ASSERT_EQ(settings.view.dithering, Dithering::TEMPORAL);

    ASSERT_TRUE(specs->get(1, &settings));
    ASSERT_TRUE(settings.view.postProcessingEnabled);

    ASSERT_TRUE(specs->get(64, &settings));
    ASSERT_FALSE(specs->get(65, &settings));

    delete specs;
}

TEST_F(ViewSettingsTest, CustomAutomationSpec) {
    AutomationSpec* spec = AutomationSpec::generate(JSON_TEST_AUTOMATION,
            strlen(JSON_TEST_AUTOMATION));
    ASSERT_TRUE(spec);
    ASSERT_EQ(spec->size(), 72);
    delete spec;
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
