//
//  Copyright 2019 The ANGLE Project Authors. All rights reserved.
//  Use of this source code is governed by a BSD-style license that can be
//  found in the LICENSE file.
//
//  glcTestPackageEntry_override.cpp:
//    Overrides for dEQP's OpenGL Conformance Test Package Entry Points.
//

#include "glcConfigPackage.hpp"

#include "es2cTestPackage.hpp"
#include "tes2TestPackage.hpp"

#include "es3cTestPackage.hpp"
#include "tes3TestPackage.hpp"

#include "es31cTestPackage.hpp"
#include "esextcTestPackage.hpp"
#include "tes31TestPackage.hpp"

#include "es32cTestPackage.hpp"

#include "gl3cTestPackages.hpp"
#include "gl4cTestPackages.hpp"

#include "glcNoDefaultContextPackage.hpp"
#include "glcSingleConfigTestPackage.hpp"

namespace glcts
{

// static tcu::TestPackage* createConfigPackage(tcu::TestContext& testCtx)
// {
//     return new glcts::ConfigPackage(testCtx, "CTS-Configs");
// }

static tcu::TestPackage *createES2Package(tcu::TestContext &testCtx)
{
    return new es2cts::TestPackage(testCtx, "KHR-GLES2");
}

static tcu::TestPackage *createES30Package(tcu::TestContext &testCtx)
{
    return new es3cts::ES30TestPackage(testCtx, "KHR-GLES3");
}

static tcu::TestPackage *createES31Package(tcu::TestContext &testCtx)
{
    return new es31cts::ES31TestPackage(testCtx, "KHR-GLES31");
}
static tcu::TestPackage *createESEXTPackage(tcu::TestContext &testCtx)
{
    return new esextcts::ESEXTTestPackage(testCtx, "KHR-GLESEXT");
}

static tcu::TestPackage *createES32Package(tcu::TestContext &testCtx)
{
    return new es32cts::ES32TestPackage(testCtx, "KHR-GLES32");
}

static tcu::TestPackage *createNoDefaultCustomContextPackage(tcu::TestContext &testCtx)
{
    return new glcts::NoDefaultContextPackage(testCtx, "KHR-NoContext");
}

static tcu::TestPackage *createSingleConfigES32TestPackage(tcu::TestContext &testCtx)
{
    return new glcts::SingleConfigES32TestPackage(testCtx, "KHR-Single-GLES32");
}

// static tcu::TestPackage* createGL30Package(tcu::TestContext& testCtx)
// {
//     return new gl3cts::GL30TestPackage(testCtx, "KHR-GL30");
// }
// static tcu::TestPackage* createGL31Package(tcu::TestContext& testCtx)
// {
//     return new gl3cts::GL31TestPackage(testCtx, "KHR-GL31");
// }
// static tcu::TestPackage* createGL32Package(tcu::TestContext& testCtx)
// {
//     return new gl3cts::GL32TestPackage(testCtx, "KHR-GL32");
// }
// static tcu::TestPackage* createGL33Package(tcu::TestContext& testCtx)
// {
//     return new gl3cts::GL33TestPackage(testCtx, "KHR-GL33");
// }

// static tcu::TestPackage* createGL40Package(tcu::TestContext& testCtx)
// {
//     return new gl4cts::GL40TestPackage(testCtx, "KHR-GL40");
// }
// static tcu::TestPackage* createGL41Package(tcu::TestContext& testCtx)
// {
//     return new gl4cts::GL41TestPackage(testCtx, "KHR-GL41");
// }
// static tcu::TestPackage* createGL42Package(tcu::TestContext& testCtx)
// {
//     return new gl4cts::GL42TestPackage(testCtx, "KHR-GL42");
// }
// static tcu::TestPackage* createGL43Package(tcu::TestContext& testCtx)
// {
//     return new gl4cts::GL43TestPackage(testCtx, "KHR-GL43");
// }
// static tcu::TestPackage* createGL44Package(tcu::TestContext& testCtx)
// {
//     return new gl4cts::GL44TestPackage(testCtx, "KHR-GL44");
// }
// static tcu::TestPackage* createGL45Package(tcu::TestContext& testCtx)
// {
//     return new gl4cts::GL45TestPackage(testCtx, "KHR-GL45");
// }
// static tcu::TestPackage *createGL46Package(tcu::TestContext &testCtx)
// {
//     return new gl4cts::GL46TestPackage(testCtx, "KHR-GL46");
// }

void registerPackages(void)
{
    tcu::TestPackageRegistry *registry = tcu::TestPackageRegistry::getSingleton();

    // registry->registerPackage("CTS-Configs", createConfigPackage);

    registry->registerPackage("KHR-GLES2", createES2Package);

    registry->registerPackage("KHR-GLES3", createES30Package);
    registry->registerPackage("KHR-GLES31", createES31Package);
    registry->registerPackage("KHR-GLESEXT", createESEXTPackage);

    registry->registerPackage("KHR-GLES32", createES32Package);

    registry->registerPackage("KHR-NoContext", createNoDefaultCustomContextPackage);
    registry->registerPackage("KHR-Single-GLES32", createSingleConfigES32TestPackage);

    // registry->registerPackage("KHR-GL30", createGL30Package);
    // registry->registerPackage("KHR-GL31", createGL31Package);
    // registry->registerPackage("KHR-GL32", createGL32Package);
    // registry->registerPackage("KHR-GL33", createGL33Package);

    // registry->registerPackage("KHR-GL40", createGL40Package);
    // registry->registerPackage("KHR-GL41", createGL41Package);
    // registry->registerPackage("KHR-GL42", createGL42Package);
    // registry->registerPackage("KHR-GL43", createGL43Package);
    // registry->registerPackage("KHR-GL44", createGL44Package);
    // registry->registerPackage("KHR-GL45", createGL45Package);
    // registry->registerPackage("KHR-GL46", createGL46Package);
}
}  // namespace glcts

class RegisterCTSPackages
{
  public:
    RegisterCTSPackages(void) { glcts::registerPackages(); }
};

RegisterCTSPackages g_registerCTS;
