#include "gles_conformance_tests.h"
#include "GTFMain.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <vector>

ANGLE_FORMAT_PRINTF(1, 2)
static std::vector<char> FormatArg(const char *fmt, ...)
{
    va_list vararg;
    va_start(vararg, fmt);
    int len = vsnprintf(nullptr, 0, fmt, vararg);
    va_end(vararg);

    std::vector<char> buf(len + 1);

    va_start(vararg, fmt);
    vsnprintf(buf.data(), buf.size(), fmt, vararg);
    va_end(vararg);

    return buf;
}

static std::string GetExecutableDirectory()
{
    std::vector<char> executableFileBuf(MAX_PATH);
    DWORD executablePathLen =
        GetModuleFileNameA(nullptr, executableFileBuf.data(), executableFileBuf.size());
    if (executablePathLen == 0)
    {
        return false;
    }

    std::string executableLocation = executableFileBuf.data();
    size_t lastPathSepLoc          = executableLocation.find_last_of("\\/");
    if (lastPathSepLoc != std::string::npos)
    {
        executableLocation = executableLocation.substr(0, lastPathSepLoc);
    }
    else
    {
        executableLocation = "";
    }

    return executableLocation;
}

void RunConformanceTest(const std::string &testPath, EGLNativeDisplayType nativeDisplay)
{
    std::vector<char *> args;

    // Empty first argument for the program name
    args.push_back("");

    std::vector<char> widthArg = FormatArg("-width=%u", 64);
    args.push_back(widthArg.data());

    std::vector<char> heightArg = FormatArg("-height=%u", 64);
    args.push_back(heightArg.data());

    std::vector<char> displayArg = FormatArg("-d=%llu", nativeDisplay);
    args.push_back(displayArg.data());

    std::vector<char> runArg = FormatArg("-run=%s/conformance_tests/%s",
                                         GetExecutableDirectory().c_str(), testPath.c_str());
    args.push_back(runArg.data());

    // Redirect cout
    std::streambuf *oldCoutStreamBuf = std::cout.rdbuf();
    std::ostringstream strCout;
    std::cout.rdbuf(strCout.rdbuf());

    if (GTFMain(args.size(), args.data()) != 0)
    {
        FAIL() << "GTFMain failed.";
    }

    // Restore old cout
    std::cout.rdbuf(oldCoutStreamBuf);
    std::string log = strCout.str();

    // Look for failures
    size_t offset                  = 0;
    std::string offsetSearchString = "failure = ";
    while ((offset = log.find("failure = ", offset)) != std::string::npos)
    {
        offset += offsetSearchString.length();

        size_t failureCount = atoll(log.c_str() + offset);
        EXPECT_EQ(0, failureCount) << log;
    }
}
