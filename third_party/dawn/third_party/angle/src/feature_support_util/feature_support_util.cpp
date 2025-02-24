//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// feature_support_util.cpp: Helps client APIs make decisions based on rules
// data files.  For example, the Android EGL loader uses this library to
// determine whether to use ANGLE or a native GLES driver.

#include "feature_support_util.h"
#include <json/json.h>
#include <string.h>
#include "common/platform.h"
#if defined(ANGLE_PLATFORM_ANDROID)
#    include <android/log.h>
#    include <unistd.h>
#endif
#include <fstream>
#include <ios>
#include <list>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>
#include "../gpu_info_util/SystemInfo.h"

namespace angle
{

#if defined(ANGLE_PLATFORM_ANDROID)
// Define ANGLE_FEATURE_UTIL_LOG_VERBOSE if you want VERBOSE to output
// ANGLE_FEATURE_UTIL_LOG_VERBOSE is automatically defined when is_debug = true

#    define ERR(...) __android_log_print(ANDROID_LOG_ERROR, "ANGLE", __VA_ARGS__)
#    define WARN(...) __android_log_print(ANDROID_LOG_WARN, "ANGLE", __VA_ARGS__)
#    define INFO(...) __android_log_print(ANDROID_LOG_INFO, "ANGLE", __VA_ARGS__)
#    define DEBUG(...) __android_log_print(ANDROID_LOG_DEBUG, "ANGLE", __VA_ARGS__)
#    ifdef ANGLE_FEATURE_UTIL_LOG_VERBOSE
#        define VERBOSE(...) __android_log_print(ANDROID_LOG_VERBOSE, "ANGLE", __VA_ARGS__)
#    else
#        define VERBOSE(...) ((void)0)
#    endif
#else  // defined(ANDROID)
#    define ERR(...) printf(__VA_ARGS__)
#    define WARN(...) printf(__VA_ARGS__)
#    define INFO(...) printf(__VA_ARGS__)
#    define DEBUG(...) printf(__VA_ARGS__)
// Uncomment for debugging.
// #    define VERBOSE(...) printf(__VA_ARGS__)
#    define VERBOSE(...)
#endif  // defined(ANDROID)

// JSON values are generally composed of either:
//  - Objects, which are a set of comma-separated string:value pairs (note the recursive nature)
//  - Arrays, which are a set of comma-separated values.
// We'll call the string in a string:value pair the "identifier".  These identifiers are defined
// below, as follows:

// The JSON identifier for the top-level set of rules.  This is an object, the value of which is an
// array of rules.  The rules will be processed in order.  If a rule matches, the rule's version of
// the answer (true or false) becomes the new answer.  After all rules are processed, the
// most-recent answer is the final answer.
constexpr char kJsonRules[] = "Rules";
// The JSON identifier for a given rule.  A rule is an object, the first string:value pair is this
// identifier (i.e. "Rule") as the string and the value is a user-firendly description of the rule:
constexpr char kJsonRule[] = "Rule";
// Within a rule, the JSON identifier for the answer--whether or not to use ANGLE.  The value is a
// boolean (i.e. true or false).
constexpr char kJsonUseANGLE[] = "UseANGLE";

// Within a rule, the JSON identifier for describing one or more applications.  The value is an
// array of objects, each object of which can specify attributes of an application.
constexpr char kJsonApplications[] = "Applications";
// Within an object that describes the attributes of an application, the JSON identifier for the
// name of the application (e.g. "com.google.maps").  The value is a string.  If any other
// attributes will be specified, this must be the first attribute specified in the object.
constexpr char kJsonAppName[] = "AppName";

// Within a rule, the JSON identifier for describing one or more devices.  The value is an
// array of objects, each object of which can specify attributes of a device.
constexpr char kJsonDevices[] = "Devices";
// Within an object that describes the attributes of a device, the JSON identifier for the
// manufacturer of the device.  The value is a string.  If any other non-GPU attributes will be
// specified, this must be the first attribute specified in the object.
constexpr char kJsonManufacturer[] = "Manufacturer";
// Within an object that describes the attributes of a device, the JSON identifier for the
// model of the device.  The value is a string.
constexpr char kJsonModel[] = "Model";

// Within an object that describes the attributes of a device, the JSON identifier for describing
// one or more GPUs/drivers used in the device.  The value is an
// array of objects, each object of which can specify attributes of a GPU and its driver.
constexpr char kJsonGPUs[] = "GPUs";
// Within an object that describes the attributes of a GPU and driver, the JSON identifier for the
// vendor of the device/driver.  The value is a string.  If any other attributes will be specified,
// this must be the first attribute specified in the object.
constexpr char kJsonVendor[] = "Vendor";
// Within an object that describes the attributes of a GPU and driver, the JSON identifier for the
// deviceId of the device.  The value is an unsigned integer.  If the driver version will be
// specified, this must preceded the version attributes specified in the object.
constexpr char kJsonDeviceId[] = "DeviceId";

// Within an object that describes the attributes of either an application or a GPU, the JSON
// identifier for the major version of that application or GPU driver.  The value is a positive
// integer number.  Not specifying a major version implies a wildcard for all values of a version.
constexpr char kJsonVerMajor[] = "VerMajor";
// Within an object that describes the attributes of either an application or a GPU, the JSON
// identifier for the minor version of that application or GPU driver.  The value is a positive
// integer number.  In order to specify a minor version, it must be specified immediately after the
// major number associated with it.  Not specifying a minor version implies a wildcard for the
// minor, subminor, and patch values of a version.
constexpr char kJsonVerMinor[] = "VerMinor";
// Within an object that describes the attributes of either an application or a GPU, the JSON
// identifier for the subminor version of that application or GPU driver.  The value is a positive
// integer number.  In order to specify a subminor version, it must be specified immediately after
// the minor number associated with it.  Not specifying a subminor version implies a wildcard for
// the subminor and patch values of a version.
constexpr char kJsonVerSubMinor[] = "VerSubMinor";
// Within an object that describes the attributes of either an application or a GPU, the JSON
// identifier for the patch version of that application or GPU driver.  The value is a positive
// integer number.  In order to specify a patch version, it must be specified immediately after the
// subminor number associated with it.  Not specifying a patch version implies a wildcard for the
// patch value of a version.
constexpr char kJsonVerPatch[] = "VerPatch";

// This encapsulates a std::string.  The default constructor (not given a string) assumes that this
// is a wildcard (i.e. will match all other StringPart objects).
class StringPart
{
  public:
    StringPart() = default;
    explicit StringPart(const std::string part) : mPart(part), mWildcard(false) {}
    ~StringPart() = default;

    static StringPart FromJson(const Json::Value &parent, const char *key)
    {
        if (parent.isMember(key) && parent[key].isString())
        {
            return StringPart(parent[key].asString());
        }
        return {};
    }

    bool match(const StringPart &toCheck) const
    {
        return (mWildcard || toCheck.mWildcard || (toCheck.mPart == mPart));
    }

  public:
    std::string mPart;
    bool mWildcard = true;
};

// This encapsulates a 32-bit unsigned integer.  The default constructor (not given a number)
// assumes that this is a wildcard (i.e. will match all other IntegerPart objects).
class IntegerPart
{
  public:
    IntegerPart() = default;
    explicit IntegerPart(uint32_t part) : mPart(part), mWildcard(false) {}
    ~IntegerPart() = default;

    static IntegerPart FromJson(const Json::Value &parent, const char *key)
    {
        if (parent.isMember(key) && parent[key].isInt())
        {
            return IntegerPart(parent[key].asInt());
        }
        return {};
    }

    bool match(const IntegerPart &toCheck) const
    {
        return (mWildcard || toCheck.mWildcard || (toCheck.mPart == mPart));
    }

  public:
    uint32_t mPart = 0;
    bool mWildcard = true;
};

// This encapsulates a list of other classes, each of which will have a match() and logItem()
// method.  The common constructor (given a type, but not any list items) assumes that this is
// a wildcard (i.e. will match all other ListOf<t> objects).
template <class T>
class ListOf
{
  public:
    explicit ListOf(const std::string listType) : mWildcard(true), mListType(listType) {}
    ~ListOf() { mList.clear(); }
    void addItem(T &&toAdd)
    {
        mList.push_back(std::move(toAdd));
        mWildcard = false;
    }
    bool match(const T &toCheck) const
    {
        VERBOSE("\t\t Matching ListOf<%s> against item:\n", mListType.c_str());
        if (mWildcard || toCheck.mWildcard)
        {
            VERBOSE("\t\t\t Successful match due to wildcard.\n");
            return true;
        }
        for (const T &it : mList)
        {
            if (it.match(toCheck))
            {
                VERBOSE("\t\t\t Successful match due to list item match.\n");
                return true;
            }
        }
        VERBOSE("\t\t\t Failed to match.\n");
        return false;
    }
    bool match(const ListOf<T> &toCheck) const
    {
        VERBOSE("\t\t Matching ListOf<%s>:\n", mListType.c_str());
        if (mWildcard || toCheck.mWildcard)
        {
            VERBOSE("\t\t\t Successful match due to wildcard.\n");
            return true;
        }
        // If we make it to here, both this and toCheck have at least one item in their mList.
        for (const T &it : toCheck.mList)
        {
            if (match(it))
            {
                VERBOSE("\t\t\t Successful match due to list item match.\n");
                return true;
            }
        }
        VERBOSE("\t\t\t Failed to match list.\n");
        return false;
    }
    void logListOf(const std::string prefix, const std::string name) const
    {
        if (mWildcard)
        {
            VERBOSE("%sListOf%s is wildcarded to always match\n", prefix.c_str(), name.c_str());
        }
        else
        {
            VERBOSE("%sListOf%s has %d item(s):\n", prefix.c_str(), name.c_str(),
                    static_cast<int>(mList.size()));
            for (auto &it : mList)
            {
                it.logItem();
            }
        }
    }

    bool mWildcard;

  private:
    std::string mListType;
    std::vector<T> mList;
};

// This encapsulates up-to four 32-bit unsigned integers, that represent a potentially-complex
// version number.  The default constructor (not given any numbers) assumes that this is a wildcard
// (i.e. will match all other Version objects).  Each part of a Version is stored in an IntegerPart
// class, and so may be wildcarded as well.
class Version
{
  public:
    Version(uint32_t major, uint32_t minor, uint32_t subminor, uint32_t patch)
        : mMajor(major), mMinor(minor), mSubminor(subminor), mPatch(patch)
    {}

    Version()                           = default;
    Version(const Version &)            = default;
    Version(Version &&)                 = default;
    Version &operator=(const Version &) = default;
    Version &operator=(Version &&)      = default;
    ~Version()                          = default;

    static Version FromJson(const Json::Value &jObject)
    {
        Version version;
        version.mMajor = IntegerPart::FromJson(jObject, kJsonVerMajor);
        if (version.mMajor.mWildcard)
        {
            return version;
        }
        // Revision fields are only checked if their parent version field
        // is set.
        version.mMinor = IntegerPart::FromJson(jObject, kJsonVerMinor);
        if (version.mMinor.mWildcard)
        {
            return version;
        }

        version.mSubminor = IntegerPart::FromJson(jObject, kJsonVerSubMinor);
        if (version.mSubminor.mWildcard)
        {
            return version;
        }

        version.mPatch = IntegerPart::FromJson(jObject, kJsonVerPatch);
        return version;
    }

    bool match(const Version &toCheck) const
    {
        VERBOSE("\t\t\t Matching Version %s against %s\n", getString().c_str(),
                toCheck.getString().c_str());
        return (isWildcard() || toCheck.isWildcard() ||
                (mMajor.match(toCheck.mMajor) && mMinor.match(toCheck.mMinor) &&
                 mSubminor.match(toCheck.mSubminor) && mPatch.match(toCheck.mPatch)));
    }

    std::string getString() const
    {
        if (mMajor.mWildcard)
        {
            return "*";
        }

        std::ostringstream ss;
        ss << mMajor.mPart;
        // Must at least have a major version:
        if (!mMinor.mWildcard)
        {
            ss << "." << mMinor.mPart;
            if (!mSubminor.mWildcard)
            {
                ss << "." << mSubminor.mPart;

                if (!mPatch.mWildcard)
                {
                    ss << "." << mPatch.mPart;
                }
            }
        }
        if (mPatch.mWildcard)
        {
            ss << ".*";
        }
        return ss.str();
    }

    bool isWildcard() const { return mMajor.mWildcard; }

  public:
    IntegerPart mMajor;
    IntegerPart mMinor;
    IntegerPart mSubminor;
    IntegerPart mPatch;
};

// This encapsulates an application, and potentially the application's Version.  The default
// constructor (not given any values) assumes that this is a wildcard (i.e. will match all
// other Application objects).  Each part of an Application is stored in a class that may
// also be wildcarded.
class Application
{
  public:
    Application(StringPart name, Version version = {})
        : mName(name), mVersion(version), mWildcard(false)
    {}
    Application()  = default;
    ~Application() = default;

    static bool FromJson(const Json::Value &jObject, Application *out)
    {
        // If an application is listed, the application's name is required:
        auto name = StringPart::FromJson(jObject, kJsonAppName);
        if (name.mWildcard)
        {
            return false;
        }
        auto version = Version::FromJson(jObject);
        *out         = Application{std::move(name), std::move(version)};
        return true;
    }

    bool match(const Application &toCheck) const
    {
        return (mWildcard || toCheck.mWildcard ||
                (toCheck.mName.match(mName) && toCheck.mVersion.match(mVersion)));
    }
    void logItem() const
    {
        if (mWildcard)
        {
            VERBOSE("      Wildcard (i.e. will match all applications)\n");
        }
        else if (!mVersion.isWildcard())
        {
            VERBOSE("      Application \"%s\" (version: %s)\n", mName.mPart.c_str(),
                    mVersion.getString().c_str());
        }
        else
        {
            VERBOSE("      Application \"%s\"\n", mName.mPart.c_str());
        }
    }

  public:
    StringPart mName;
    Version mVersion;
    bool mWildcard = true;
};

// This encapsulates a GPU and its driver.  The default constructor (not given any values) assumes
// that this is a wildcard (i.e. will match all other GPU objects).  Each part of a GPU is stored
// in a class that may also be wildcarded.
class GPU
{
  public:
    GPU(StringPart vendor, IntegerPart deviceId, Version version)
        : mVendor(std::move(vendor)),
          mDeviceId(std::move(deviceId)),
          mVersion(version),
          mWildcard(false)
    {}
    GPU(std::string vendor, uint32_t deviceId, Version version)
        : GPU(StringPart(std::move(vendor)), IntegerPart(deviceId), std::move(version))
    {}
    GPU()  = default;
    ~GPU() = default;
    bool match(const GPU &toCheck) const
    {
        VERBOSE("\t\t Matching %s \n\t\t  against %s\n", toString().c_str(),
                toCheck.toString().c_str());
        return (mWildcard || toCheck.mWildcard ||
                (toCheck.mVendor.match(mVendor) && toCheck.mDeviceId.match(mDeviceId) &&
                 toCheck.mVersion.match(mVersion)));
    }

    // Returns true if out is set to a valid GPU instance.
    static bool CreateGpuFromJson(const Json::Value &jObject, GPU *out)
    {
        // If a GPU is listed, the vendor name is required:
        auto vendor = StringPart::FromJson(jObject, kJsonVendor);
        if (vendor.mWildcard)
        {
            WARN("Asked to parse a GPU, but no vendor found.\n");
            return false;
        }

        auto deviceId = IntegerPart::FromJson(jObject, kJsonDeviceId);
        auto version  = Version::FromJson(jObject);
        *out          = GPU{std::move(vendor), std::move(deviceId), std::move(version)};
        return true;
    }

    std::string toString() const
    {
        if (mWildcard)
        {
            return std::string("Wildcard (i.e. will match all GPUs)");
        }

        std::ostringstream ss;
        ss << "GPU vendor: " << mVendor.mPart;
        if (!mDeviceId.mWildcard)
        {
            ss << ", deviceId: " << std::hex << mDeviceId.mPart;
        }
        ss << ", version: " << mVersion.getString();
        return ss.str();
    }

    void logItem() const { VERBOSE("\t     %s\n", toString().c_str()); }

  public:
    StringPart mVendor;
    IntegerPart mDeviceId;
    Version mVersion;
    bool mWildcard = true;
};

// This encapsulates a device, and potentially the device's model and/or a list of GPUs/drivers
// associated with the Device.  The default constructor (not given any values) assumes that this is
// a wildcard (i.e. will match all other Device objects).  Each part of a Device is stored in a
// class that may also be wildcarded.
class Device
{
  public:
    Device(StringPart manufacturer, StringPart model)
        : mManufacturer(std::move(manufacturer)),
          mModel(std::move(model)),
          mGpuList("GPU"),
          mWildcard(false)
    {}
    Device() : mGpuList("GPU") {}
    ~Device() = default;

    static Device FromJson(const Json::Value &jObject)
    {
        auto manufacturer = StringPart::FromJson(jObject, kJsonManufacturer);
        if (!manufacturer.mWildcard)
        {
            // We don't let a model be specified without also specifying a manufacturer:
            auto model = StringPart::FromJson(jObject, kJsonModel);
            return Device(std::move(manufacturer), std::move(model));
        }
        // This case is not treated as an error because a rule may wish to only call out one or
        // more GPUs, but not any specific manufacturer (e.g. for any manufacturer's device
        // that uses a GPU from Vendor-A, with DeviceID-Foo, and with driver version 1.2.3.4):
        return Device();
    }

    void addGPU(GPU &&gpu) { mGpuList.addItem(std::move(gpu)); }
    bool match(const Device &toCheck) const
    {
        // GPU lists must always match, even when wildcards are used.
        VERBOSE("\t   Checking ListOf<GPU>:\n");
        if (!mGpuList.match(toCheck.mGpuList))
        {
            VERBOSE("\t Failed to match due to mismatched GPU list.\n");
            return false;
        }
        if (mWildcard || toCheck.mWildcard)
        {
            VERBOSE("\t  Matching due to wildcard.\n");
            return true;
        }
        if (toCheck.mManufacturer.match(mManufacturer) && toCheck.mModel.match(mModel))
        {
            VERBOSE("\t  Matching due to manufacturer and model match.\n");
            return true;
        }
        return false;
    }
    void logItem() const
    {
        if (mWildcard)
        {
            if (mGpuList.mWildcard)
            {
                VERBOSE("      Wildcard (i.e. will match all devices)\n");
                return;
            }
            else
            {
                VERBOSE(
                    "      Device with any manufacturer and model"
                    ", and with the following GPUs:\n");
            }
        }
        else
        {
            if (!mModel.mWildcard)
            {
                VERBOSE(
                    "      Device manufacturer: \"%s\" and model \"%s\""
                    ", and with the following GPUs:\n",
                    mManufacturer.mPart.c_str(), mModel.mPart.c_str());
            }
            else
            {
                VERBOSE(
                    "      Device manufacturer: \"%s\""
                    ", and with the following GPUs:\n",
                    mManufacturer.mPart.c_str());
            }
        }
        mGpuList.logListOf("        ", "GPUs");
    }

  public:
    StringPart mManufacturer;
    StringPart mModel;
    ListOf<GPU> mGpuList;
    bool mWildcard = true;
};

// This encapsulates a particular scenario to check against the rules.  A Scenario is similar to a
// Rule, except that a Rule has an answer and potentially many wildcards, and a Scenario is the
// fully-specified combination of an Application and a Device that is proposed to be run with
// ANGLE.  It is compared with the list of Rules.
class Scenario
{
  public:
    Scenario(const char *appName, const char *deviceMfr, const char *deviceModel)
        : mApplication(Application(StringPart(appName))),
          mDevice(Device(StringPart(deviceMfr), StringPart(deviceModel)))
    {}
    ~Scenario() = default;
    void logScenario()
    {
        VERBOSE("  Scenario to compare against the rules:\n");
        VERBOSE("    Application:\n");
        mApplication.logItem();
        VERBOSE("    Device:\n");
        mDevice.logItem();
    }

  public:
    Application mApplication;
    Device mDevice;
};

// This encapsulates a Rule that provides an answer based on whether a particular Scenario matches
// the Rule.  A Rule always has an answer, but can potentially wildcard every item in it (i.e.
// match every scenario).
class Rule
{
  public:
    Rule(const std::string description, bool useANGLE)
        : mDescription(description),
          mAppList("Application"),
          mDevList("Device"),
          mUseANGLE(useANGLE)
    {}
    ~Rule() = default;
    void addApp(Application &&app) { mAppList.addItem(std::move(app)); }
    void addDevice(Device &&dev) { mDevList.addItem(std::move(dev)); }
    bool match(const Scenario &toCheck) const
    {
        VERBOSE("    Matching rule \"%s\" against scenario:\n", mDescription.c_str());
        if (!mAppList.match(toCheck.mApplication))
        {
            VERBOSE("\tFailed to match rule due to mismatched application.\n");
            return false;
        }
        if (!mDevList.match(toCheck.mDevice))
        {
            VERBOSE("\tFailed to match rule due to mismatched device.\n");
            return false;
        }
        VERBOSE("\tSuccessfully matched rule.");
        return true;
    }
    bool getUseANGLE() const { return mUseANGLE; }
    void logRule() const
    {
        VERBOSE("  Rule: \"%s\" %s ANGLE\n", mDescription.c_str(),
                mUseANGLE ? "enables" : "disables");
        mAppList.logListOf("    ", "Applications");
        mDevList.logListOf("    ", "Devices");
    }

    std::string mDescription;
    ListOf<Application> mAppList;
    ListOf<Device> mDevList;
    bool mUseANGLE;
};

// This encapsulates a list of Rules that Scenarios are matched against.  A Scenario is compared
// with each Rule, in order.  Any time a Scenario matches a Rule, the current answer is overridden
// with the answer of the matched Rule.
class RuleList
{
  public:
    RuleList() {}
    ~RuleList() { mRuleList.clear(); }

    static RuleList *ReadRulesFromJsonString(const std::string jsonFileContents)
    {
        RuleList *rules = new RuleList;

        // Open the file and start parsing it:
        Json::CharReaderBuilder builder;
        // Json::CharReaderBuilder::strictMode(&builder.settings_);
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

        Json::Value jTopLevelObject;
        std::string errorMessage;
        const bool succeeded = reader->parse(&*jsonFileContents.begin(), &*jsonFileContents.end(),
                                             &jTopLevelObject, &errorMessage);
        if (!succeeded)
        {
            VERBOSE("Failed to parse rules from json file. Error: %s\n", errorMessage.c_str());
            return nullptr;
        }

        for (const auto &jRule : jTopLevelObject[kJsonRules])
        {
            std::string ruleDescription = jRule[kJsonRule].asString();
            bool useANGLE               = jRule[kJsonUseANGLE].asBool();
            Rule newRule(std::move(ruleDescription), useANGLE);

            for (const auto &jApp : jRule[kJsonApplications])
            {
                Application app;
                if (Application::FromJson(jApp, &app))
                {
                    newRule.addApp(std::move(app));
                }
            }

            for (const auto &jDev : jRule[kJsonDevices])
            {
                Device newDev = Device::FromJson(jDev);
                for (const auto &jGPU : jDev[kJsonGPUs])
                {
                    GPU newGPU;
                    if (GPU::CreateGpuFromJson(jGPU, &newGPU))
                    {
                        newDev.addGPU(std::move(newGPU));
                    }
                }
                newRule.addDevice(std::move(newDev));
            }

            rules->addRule(std::move(newRule));
        }

        // Make sure there is at least one, default rule.  If not, add it here:
        if (rules->mRuleList.empty())
        {
            Rule defaultRule("Default Rule", false);
            rules->addRule(std::move(defaultRule));
        }
        return rules;
    }

    void addRule(Rule &&rule) { mRuleList.push_back(std::move(rule)); }
    bool getUseANGLE(const Scenario &toCheck)
    {
        // Initialize useANGLE to the system-wide default (that should be set in the default
        // rule, but just in case, set it here too):
        bool useANGLE = false;
        VERBOSE("Checking scenario against %d ANGLE-for-Android rules:\n",
                static_cast<int>(mRuleList.size()));

        for (const Rule &rule : mRuleList)
        {
            VERBOSE("  Checking Rule: \"%s\" (to see whether there's a match)\n",
                    rule.mDescription.c_str());
            if (rule.match(toCheck))
            {
                VERBOSE("  -> Rule matches.  Updating useANGLE to %s.\n",
                        rule.getUseANGLE() ? "true" : "false");
                // The ANGLE rules are ordered from least to greatest specificity, meaning that
                // the last rule with a match should dictate whether or not ANGLE should be
                // recommended for use.
                useANGLE = rule.getUseANGLE();
            }
            else
            {
                VERBOSE("  -> Rule doesn't match.\n");
            }
        }
        return useANGLE;
    }
    void logRules()
    {
        VERBOSE("Showing %d ANGLE-for-Android rules:\n", static_cast<int>(mRuleList.size()));
        for (const Rule &rule : mRuleList)
        {
            rule.logRule();
        }
    }

  public:
    std::vector<Rule> mRuleList;
};

}  // namespace angle

extern "C" {

using namespace angle;

// This function is part of the version-2 API:
ANGLE_EXPORT bool ANGLEGetFeatureSupportUtilAPIVersion(unsigned int *versionToUse)
{
    if (!versionToUse || (*versionToUse < kFeatureVersion_LowestSupported))
    {
        // The versionToUse is either nullptr or is less than the lowest version supported, which
        // is an error.
        return false;
    }
    if (*versionToUse > kFeatureVersion_HighestSupported)
    {
        // The versionToUse is greater than the highest version supported; change it to the
        // highest version supported (caller will decide if it can use that version).
        *versionToUse = kFeatureVersion_HighestSupported;
    }
    return true;
}

// This function is part of the version-2 API:
ANGLE_EXPORT bool ANGLEAndroidParseRulesString(const char *rulesString,
                                               RulesHandle *rulesHandle,
                                               int *rulesVersion)
{
    if (!rulesString || !rulesHandle || !rulesVersion)
    {
        return false;
    }

    std::string rulesFileContents = rulesString;
    RuleList *rules               = RuleList::ReadRulesFromJsonString(rulesFileContents);
    if (!rules)
    {
        return false;
    }

    rules->logRules();

    *rulesHandle  = rules;
    *rulesVersion = 0;
    return true;
}

// This function is part of the version-2 API:
ANGLE_EXPORT bool ANGLEGetSystemInfo(SystemInfoHandle *systemInfoHandle)
{
    if (!systemInfoHandle)
    {
        return false;
    }

    // TODO (http://anglebug.com/42261721): Restore the real code
    angle::SystemInfo *systemInfo = new angle::SystemInfo;
    systemInfo->gpus.resize(1);
    GPUDeviceInfo &gpu = systemInfo->gpus[0];
    gpu.vendorId       = 0xFEFEFEFE;
    gpu.deviceId       = 0xFEEEFEEE;
    gpu.driverVendor   = "Foo";
    gpu.driverVersion  = "1.2.3.4";

    *systemInfoHandle = systemInfo;
    return true;
}

// This function is part of the version-2 API:
ANGLE_EXPORT bool ANGLEAddDeviceInfoToSystemInfo(const char *deviceMfr,
                                                 const char *deviceModel,
                                                 SystemInfoHandle systemInfoHandle)
{
    angle::SystemInfo *systemInfo = static_cast<angle::SystemInfo *>(systemInfoHandle);
    if (!deviceMfr || !deviceModel || !systemInfo)
    {
        return false;
    }

    systemInfo->machineManufacturer = deviceMfr;
    systemInfo->machineModelName    = deviceModel;
    return true;
}

// This function is part of the version-2 API:
ANGLE_EXPORT bool ANGLEShouldBeUsedForApplication(const RulesHandle rulesHandle,
                                                  int rulesVersion,
                                                  const SystemInfoHandle systemInfoHandle,
                                                  const char *appName)
{
    RuleList *rules               = static_cast<RuleList *>(rulesHandle);
    angle::SystemInfo *systemInfo = static_cast<angle::SystemInfo *>(systemInfoHandle);
    if (!rules || !systemInfo || !appName || (systemInfo->gpus.size() != 1))
    {
        return false;
    }

    Scenario scenario(appName, systemInfo->machineManufacturer.c_str(),
                      systemInfo->machineModelName.c_str());
    Version gpuDriverVersion(systemInfo->gpus[0].detailedDriverVersion.major,
                             systemInfo->gpus[0].detailedDriverVersion.minor,
                             systemInfo->gpus[0].detailedDriverVersion.subMinor,
                             systemInfo->gpus[0].detailedDriverVersion.patch);
    GPU gpuDriver(systemInfo->gpus[0].driverVendor, systemInfo->gpus[0].deviceId,
                  std::move(gpuDriverVersion));
    scenario.mDevice.addGPU(std::move(gpuDriver));
    scenario.logScenario();

    bool rtn = rules->getUseANGLE(std::move(scenario));
    VERBOSE("Application \"%s\" should %s ANGLE.\n", appName, rtn ? "use" : "NOT use");

    return rtn;
}

// This function is part of the version-2 API:
ANGLE_EXPORT void ANGLEFreeRulesHandle(const RulesHandle rulesHandle)
{
    RuleList *rules = static_cast<RuleList *>(rulesHandle);
    if (rules)
    {
        delete rules;
    }
}

// This function is part of the version-2 API:
ANGLE_EXPORT void ANGLEFreeSystemInfoHandle(const SystemInfoHandle systemInfoHandle)
{
    angle::SystemInfo *systemInfo = static_cast<angle::SystemInfo *>(systemInfoHandle);
    if (systemInfo)
    {
        delete systemInfo;
    }
}

}  // extern "C"
