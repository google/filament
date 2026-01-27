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
        "antiAliasing": "FXAA",
        "taa": {
            "enabled": false,
            "filterWidth": 1.0,
            "feedback": 0.04
        },
        "msaa": {
            "enabled": false,
            "sampleCount": 4,
            "customResolve": false
        },
        "dsr": {
            "enabled": false,
            "minScale": [0.25, 0.25],
            "maxScale": [1.0, 1.0],
            "sharpness": 0.9,
            "enabled": false,
            "homogeneousScaling": false,
            "quality": "MEDIUM"
        },
        "colorGrading": {
            "enabled": true,
            "quality": "MEDIUM",
            "toneMapping": "ACES_LEGACY",
            "genericToneMapper": {
                "contrast": 1.0,
                "midGrayIn": 1.0,
                "midGrayOut": 1.0,
                "hdrMax": 16.0
            },
            "luminanceScaling": false,
            "gamutMapping": false,
            "exposure": 0,
            "nightAdaptation": 0,
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
    JsonSerializer serializer;
    Settings settings1;
    ASSERT_TRUE(serializer.readJson(JSON_TEST_DEFAULTS, strlen(JSON_TEST_DEFAULTS), &settings1));

    ASSERT_TRUE(settings1.view.bloom.threshold);

    Settings settings2;
    ASSERT_TRUE(serializer.readJson("{}", strlen("{}"), &settings2));
    ASSERT_FALSE(serializer.readJson("{ badly_formed }", strlen("{ badly_formed }"), &settings2));

    Settings settings3;
    ASSERT_EQ(serializer.writeJson(settings2), serializer.writeJson(settings3));
}

TEST_F(ViewSettingsTest, JsonTestSerialization) {
    auto canParse = [](bool parseResult, std::string json) {
        if (parseResult) {
            return testing::AssertionSuccess() << "Settings can be serialized.";
        } else {
            return testing::AssertionFailure() << "JSON has errors:\n" << json.c_str() << std::endl;
        }
    };

    JsonSerializer serializer;
    Settings outSettings = {};
    std::string jsonstr = serializer.writeJson(outSettings);
    Settings inSettings = {};
    bool result = serializer.readJson(jsonstr.c_str(), jsonstr.size(), &inSettings);
    ASSERT_TRUE(canParse(result, jsonstr));
}

TEST_F(ViewSettingsTest, JsonTestMaterial) {
    JsonSerializer serializer;
    Settings settings;
    std::string js = "{" + std::string(JSON_TEST_MATERIAL) + "}";
    ASSERT_TRUE(serializer.readJson(js.c_str(), js.size(), &settings));
    std::string serialized = serializer.writeJson(settings);
    ASSERT_PRED_FORMAT2(testing::IsSubstring, "\"baz\": [1, 2, 3]", serialized);
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

static const char* JSON_TEST_CAMERA = R"TXT(
{
    "camera": {
        "aperture": 4.0,
        "shutterSpeed": 200,
        "sensitivity": 400,
        "focalLength": 50,
        "focusDistance": 5.0,
        "projection": "ORTHO",
        "near": 0.5,
        "far": 1000.0,
        "scaling": [0.5, 0.6],
        "shift": [0.1, 0.2]
    }
}
)TXT";

TEST_F(ViewSettingsTest, JsonTestCamera) {
    JsonSerializer serializer;
    Settings settings;
    ASSERT_TRUE(serializer.readJson(JSON_TEST_CAMERA, strlen(JSON_TEST_CAMERA), &settings));

    EXPECT_FLOAT_EQ(settings.camera.aperture, 4.0f);
    EXPECT_FLOAT_EQ(settings.camera.shutterSpeed, 200.0f);
    EXPECT_FLOAT_EQ(settings.camera.sensitivity, 400.0f);
    EXPECT_FLOAT_EQ(settings.camera.focalLength, 50.0f);
    EXPECT_FLOAT_EQ(settings.camera.focusDistance, 5.0f);
    EXPECT_EQ(settings.camera.projection, CameraProjection::ORTHO);
    EXPECT_FLOAT_EQ(settings.camera.near, 0.5f);
    EXPECT_FLOAT_EQ(settings.camera.far, 1000.0f);
    EXPECT_FLOAT_EQ(settings.camera.scaling.x, 0.5f);
    EXPECT_FLOAT_EQ(settings.camera.scaling.y, 0.6f);
    EXPECT_FLOAT_EQ(settings.camera.shift.x, 0.1f);
    EXPECT_FLOAT_EQ(settings.camera.shift.y, 0.2f);
}

static const char* JSON_TEST_LIGHTS = R"TXT(
{
    "lighting": {
        "enableSunlight": false,
        "lights": [
            {
                "type": "POINT",
                "position": [1.0, 2.0, 3.0],
                "color": [1.0, 0.0, 0.0],
                "intensity": 1000.0,
                "falloff": 5.0,
                "castShadows": true
            },
            {
                "type": "SPOT",
                "direction": [0.0, -1.0, 0.0],
                "spotInner": 0.5,
                "spotOuter": 0.8
            }
        ]
    }
}
)TXT";

TEST_F(ViewSettingsTest, JsonTestLights) {
    JsonSerializer serializer;
    Settings settings;
    ASSERT_TRUE(serializer.readJson(JSON_TEST_LIGHTS, strlen(JSON_TEST_LIGHTS), &settings));

    EXPECT_FALSE(settings.lighting.enableSunlight);
    ASSERT_EQ(settings.lighting.lights.size(), 2);

    const auto& light0 = settings.lighting.lights[0];
    EXPECT_EQ(light0.type, LightManager::Type::POINT);
    EXPECT_FLOAT_EQ(light0.position.x, 1.0f);
    EXPECT_FLOAT_EQ(light0.position.y, 2.0f);
    EXPECT_FLOAT_EQ(light0.position.z, 3.0f);
    EXPECT_FLOAT_EQ(light0.color.r, 1.0f);
    EXPECT_FLOAT_EQ(light0.intensity, 1000.0f);
    EXPECT_FLOAT_EQ(light0.falloff, 5.0f);
    EXPECT_TRUE(light0.castShadows);

    const auto& light1 = settings.lighting.lights[1];
    EXPECT_EQ(light1.type, LightManager::Type::SPOT);
    EXPECT_FLOAT_EQ(light1.direction.y, -1.0f);
    EXPECT_FLOAT_EQ(light1.spotInner, 0.5f);
    EXPECT_FLOAT_EQ(light1.spotOuter, 0.8f);
}
