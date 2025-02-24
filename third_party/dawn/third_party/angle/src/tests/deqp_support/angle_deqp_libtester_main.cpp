//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// deqp_libtester_main.cpp: Entry point for tester DLL.

#include <cstdio>
#include <iostream>

#include "angle_deqp_libtester.h"
#include "common/angleutils.h"
#include "common/system_utils.h"
#include "deMath.h"
#include "deUniquePtr.hpp"
#include "platform/PlatformMethods.h"
#include "tcuANGLEPlatform.h"
#include "tcuApp.hpp"
#include "tcuCommandLine.hpp"
#include "tcuDefs.hpp"
#include "tcuPlatform.hpp"
#include "tcuRandomOrderExecutor.h"
#include "tcuResource.hpp"
#include "tcuTestLog.hpp"
#include "util/OSWindow.h"

namespace
{

tcu::Platform *g_platform            = nullptr;
tcu::CommandLine *g_cmdLine          = nullptr;
tcu::DirArchive *g_archive           = nullptr;
tcu::TestLog *g_log                  = nullptr;
tcu::TestContext *g_testCtx          = nullptr;
tcu::TestPackageRoot *g_root         = nullptr;
tcu::RandomOrderExecutor *g_executor = nullptr;

std::string GetLogFileName(std::string deqpDataDir)
{
#if (DE_OS == DE_OS_ANDROID)
    // On Android executable dir is not writable, so use data dir instead
    return std::string("/data/data/com.android.angle.test/") + g_cmdLine->getLogFileName();
#else
    return g_cmdLine->getLogFileName();
#endif
}

}  // anonymous namespace

ANGLE_LIBTESTER_EXPORT bool deqp_libtester_init_platform(int argc,
                                                         const char *argv[],
                                                         void *logErrorFunc,
                                                         const dEQPOptions &options)
{
    try
    {
#if (DE_OS != DE_OS_WIN32)
        // Set stdout to line-buffered mode (will be fully buffered by default if stdout is pipe).
        setvbuf(stdout, DE_NULL, _IOLBF, 4 * 1024);
#endif
        g_platform = CreateANGLEPlatform(reinterpret_cast<angle::LogErrorFunc>(logErrorFunc),
                                         options.preRotation, options.driverOption);

        if (!deSetRoundingMode(DE_ROUNDINGMODE_TO_NEAREST_EVEN))
        {
            std::cout << "Failed to set floating point rounding mode." << std::endl;
            return false;
        }

        constexpr size_t kMaxDataDirLen = 1000;
        char deqpDataDir[kMaxDataDirLen];
        if (!angle::FindTestDataPath(ANGLE_DEQP_DATA_DIR, deqpDataDir, kMaxDataDirLen))
        {
            std::cout << "Failed to find dEQP data directory: " << ANGLE_DEQP_DATA_DIR << std::endl;
            return false;
        }

        g_cmdLine = new tcu::CommandLine(argc, argv);
        g_archive = new tcu::DirArchive(deqpDataDir);
        g_log     = new tcu::TestLog(GetLogFileName(deqpDataDir).c_str(), g_cmdLine->getLogFlags());
        g_testCtx = new tcu::TestContext(*g_platform, *g_archive, *g_log, *g_cmdLine, DE_NULL);
        g_root    = new tcu::TestPackageRoot(*g_testCtx, tcu::TestPackageRegistry::getSingleton());
        g_executor =
            new tcu::RandomOrderExecutor(*g_root, *g_testCtx, options.enableRenderDocCapture);
    }
    catch (const std::exception &e)
    {
        tcu::die("%s", e.what());
        return false;
    }

    return true;
}

// Exported to the tester app.
ANGLE_LIBTESTER_EXPORT int deqp_libtester_main(int argc, const char *argv[])
{
    if (!deqp_libtester_init_platform(argc, argv, nullptr, {}))
    {
        tcu::die("Could not initialize the dEQP platform");
    }

    try
    {
        de::UniquePtr<tcu::App> app(new tcu::App(*g_platform, *g_archive, *g_log, *g_cmdLine));

        // Main loop.
        for (;;)
        {
            if (!app->iterate())
                break;
        }
    }
    catch (const std::exception &e)
    {
        deqp_libtester_shutdown_platform();
        tcu::die("%s", e.what());
    }

    deqp_libtester_shutdown_platform();
    return 0;
}

ANGLE_LIBTESTER_EXPORT void deqp_libtester_shutdown_platform()
{
    delete g_executor;
    g_executor = nullptr;
    delete g_root;
    g_root = nullptr;
    delete g_testCtx;
    g_testCtx = nullptr;
    delete g_log;
    g_log = nullptr;
    delete g_archive;
    g_archive = nullptr;
    delete g_cmdLine;
    g_cmdLine = nullptr;
    delete g_platform;
    g_platform = nullptr;
}

ANGLE_LIBTESTER_EXPORT dEQPTestResult deqp_libtester_run(const char *caseName)
{
    const char *emptyString = "";
    if (g_platform == nullptr)
    {
        tcu::die("Failed to initialize platform.");
    }

    try
    {
        // Poll platform events
        const bool platformOk = g_platform->processEvents();

        if (platformOk)
        {
            const tcu::TestStatus &result = g_executor->execute(caseName);
            switch (result.getCode())
            {
                case QP_TEST_RESULT_PASS:
                    return dEQPTestResult::Pass;
                case QP_TEST_RESULT_NOT_SUPPORTED:
                    std::cout << "Not supported! " << result.getDescription() << std::endl;
                    return dEQPTestResult::NotSupported;
                case QP_TEST_RESULT_QUALITY_WARNING:
                    std::cout << "Quality warning! " << result.getDescription() << std::endl;
                    return dEQPTestResult::Pass;
                case QP_TEST_RESULT_COMPATIBILITY_WARNING:
                    std::cout << "Compatiblity warning! " << result.getDescription() << std::endl;
                    return dEQPTestResult::Pass;
                default:
                    return dEQPTestResult::Fail;
            }
        }
        else
        {
            std::cout << "Aborted test!" << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Exception running test: " << e.what() << std::endl;
        return dEQPTestResult::Exception;
    }

    return dEQPTestResult::Fail;
}
