#include "helloxr_jetpack_ui.h"

#if defined(__ANDROID__)

#include <jni.h>

namespace helloxr {

namespace {

constexpr char const* PANEL_CLASS =
        "com.google.android.filament.openxr.ComposePanelSurface";

} // anonymous namespace

struct JetpackUiLayer::Impl {
    JNIEnv* getEnv(bool* attached) const {
        *attached = false;
        if (javaVm == nullptr) {
            return nullptr;
        }
        JNIEnv* env = nullptr;
        jint const result = javaVm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
        if (result == JNI_EDETACHED) {
            if (javaVm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
                return nullptr;
            }
            *attached = true;
        } else if (result != JNI_OK) {
            return nullptr;
        }
        return env;
    }

    jclass loadPanelClass(JNIEnv* env) const {
        jclass activityClass = env->GetObjectClass(activity);
        jmethodID getClassLoader =
                env->GetMethodID(activityClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
        jobject classLoader = env->CallObjectMethod(activity, getClassLoader);
        jclass classLoaderClass = env->FindClass("java/lang/ClassLoader");
        jmethodID loadClass = env->GetMethodID(classLoaderClass, "loadClass",
                "(Ljava/lang/String;)Ljava/lang/Class;");
        jstring name = env->NewStringUTF(PANEL_CLASS);
        jclass panelClass = reinterpret_cast<jclass>(
                env->CallObjectMethod(classLoader, loadClass, name));
        env->DeleteLocalRef(name);
        env->DeleteLocalRef(classLoaderClass);
        env->DeleteLocalRef(classLoader);
        env->DeleteLocalRef(activityClass);
        return panelClass;
    }

    bool attach(jobject surface) const {
        bool attached = false;
        JNIEnv* env = getEnv(&attached);
        if (env == nullptr || activity == nullptr) {
            return false;
        }
        jclass panelClass = loadPanelClass(env);
        if (panelClass != nullptr) {
            jmethodID attachMethod = env->GetStaticMethodID(panelClass, "attach",
                "(Landroid/app/Activity;Landroid/view/Surface;II)V");
            if (attachMethod != nullptr) {
            env->CallStaticVoidMethod(panelClass, attachMethod, activity, surface,
                PIXEL_WIDTH, PIXEL_HEIGHT);
            }
            env->DeleteLocalRef(panelClass);
        }
        bool const succeeded = !env->ExceptionCheck();
        if (!succeeded) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        if (attached) {
            javaVm->DetachCurrentThread();
        }
        return succeeded;
    }

    void detach() const {
        bool attached = false;
        JNIEnv* env = getEnv(&attached);
        if (env == nullptr || activity == nullptr) {
            return;
        }
        jclass panelClass = loadPanelClass(env);
        if (panelClass != nullptr) {
            jmethodID detachMethod = env->GetStaticMethodID(
                    panelClass, "detach", "(Landroid/app/Activity;)V");
            if (detachMethod != nullptr) {
                env->CallStaticVoidMethod(panelClass, detachMethod, activity);
            }
            env->DeleteLocalRef(panelClass);
        }
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        if (attached) {
            javaVm->DetachCurrentThread();
        }
    }

    void injectTouch(float u, float v, TouchAction action) const {
        bool attached = false;
        JNIEnv* env = getEnv(&attached);
        if (env == nullptr || activity == nullptr) {
            return;
        }
        jclass panelClass = loadPanelClass(env);
        if (panelClass != nullptr) {
            jmethodID injectMethod = env->GetStaticMethodID(panelClass, "injectTouch",
                    "(Landroid/app/Activity;FFI)V");
            if (injectMethod != nullptr) {
                env->CallStaticVoidMethod(panelClass, injectMethod, activity,
                        u * float(PIXEL_WIDTH), v * float(PIXEL_HEIGHT), int32_t(action));
            }
            env->DeleteLocalRef(panelClass);
        }
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        if (attached) {
            javaVm->DetachCurrentThread();
        }
    }

    void releaseActivity() {
        bool attached = false;
        JNIEnv* env = getEnv(&attached);
        if (env != nullptr && activity != nullptr) {
            env->DeleteGlobalRef(activity);
            activity = nullptr;
        }
        if (attached) {
            javaVm->DetachCurrentThread();
        }
    }

    JavaVM* javaVm = nullptr;
    jobject activity = nullptr;
    bool supported = false;
    bool enabled = false;
    bool depthTestEnabled = false;
    XrSwapchain swapchain = XR_NULL_HANDLE;
    XrCompositionLayerDepthTestFB depthTest = { XR_TYPE_COMPOSITION_LAYER_DEPTH_TEST_FB };
    XrCompositionLayerQuad layer = { XR_TYPE_COMPOSITION_LAYER_QUAD };
};

JetpackUiLayer::JetpackUiLayer() : mImpl(std::make_unique<Impl>()) {}

JetpackUiLayer::~JetpackUiLayer() = default;

void JetpackUiLayer::initializeJava(void* javaVm, void* activity) {
    mImpl->javaVm = static_cast<JavaVM*>(javaVm);
    bool attached = false;
    JNIEnv* env = mImpl->getEnv(&attached);
    if (env != nullptr) {
        mImpl->activity = env->NewGlobalRef(static_cast<jobject>(activity));
    }
    if (attached) {
        mImpl->javaVm->DetachCurrentThread();
    }
}

void JetpackUiLayer::requestExtensions(bool const requested,
        std::function<bool(char const*)> const& supports,
        std::vector<char const*>* extensions) {
    mImpl->supported = requested && supports(XR_KHR_ANDROID_SURFACE_SWAPCHAIN_EXTENSION_NAME);
    if (mImpl->supported) {
        extensions->push_back(XR_KHR_ANDROID_SURFACE_SWAPCHAIN_EXTENSION_NAME);
    } else if (requested) {
        XRLOG("Jetpack UI disabled: runtime does not support %s",
                XR_KHR_ANDROID_SURFACE_SWAPCHAIN_EXTENSION_NAME);
    }
}

void JetpackUiLayer::configure(uint32_t const maximumLayerCount) {
    mImpl->enabled = mImpl->supported && maximumLayerCount >= 2;
    if (mImpl->supported && !mImpl->enabled) {
        XRLOG("Jetpack UI disabled: runtime supports only %u composition layer(s)",
                maximumLayerCount);
    }
}

bool JetpackUiLayer::initialize(XrInstance instance, XrSession session, XrSpace space,
        bool const depthTestSupported) {
    if (!mImpl->enabled) {
        return true;
    }

    PFN_xrCreateSwapchainAndroidSurfaceKHR createSurfaceSwapchain = nullptr;
    XrResult result = xrGetInstanceProcAddr(instance, "xrCreateSwapchainAndroidSurfaceKHR",
            reinterpret_cast<PFN_xrVoidFunction*>(&createSurfaceSwapchain));
    if (XR_FAILED(result) || createSurfaceSwapchain == nullptr) {
        XRLOG("Jetpack UI disabled: xrCreateSwapchainAndroidSurfaceKHR unavailable");
        mImpl->enabled = false;
        return false;
    }

    XrSwapchainCreateInfo createInfo = { XR_TYPE_SWAPCHAIN_CREATE_INFO };
    createInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT |
                            XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT |
                            XR_SWAPCHAIN_USAGE_MUTABLE_FORMAT_BIT;
    createInfo.width = PIXEL_WIDTH;
    createInfo.height = PIXEL_HEIGHT;

    XRLOG("Jetpack UI: creating Android surface swapchain");
    jobject surface = nullptr;
    result = createSurfaceSwapchain(session, &createInfo, &mImpl->swapchain, &surface);
    if (XR_FAILED(result) || surface == nullptr) {
        XRLOG("Jetpack UI disabled: surface swapchain creation failed (%d)", int(result));
        mImpl->enabled = false;
        return false;
    }
    XRLOG("Jetpack UI: surface swapchain created, attaching Compose");

    mImpl->layer = { XR_TYPE_COMPOSITION_LAYER_QUAD };
    mImpl->layer.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
    mImpl->layer.space = space;
    mImpl->layer.eyeVisibility = XR_EYE_VISIBILITY_BOTH;
    mImpl->layer.subImage.swapchain = mImpl->swapchain;
    mImpl->layer.subImage.imageRect = { { 0, 0 }, { PIXEL_WIDTH, PIXEL_HEIGHT } };
    mImpl->layer.subImage.imageArrayIndex = 0;
    mImpl->layer.pose.orientation = { 0.0f, 0.0f, 0.0f, 1.0f };
    mImpl->layer.pose.position = { CENTER_X, CENTER_Y, PLANE_Z };
    mImpl->layer.size = { WIDTH_METERS, HEIGHT_METERS };

    if (depthTestSupported) {
        mImpl->depthTest = { XR_TYPE_COMPOSITION_LAYER_DEPTH_TEST_FB, nullptr, XR_TRUE,
            XR_COMPARE_OP_LESS_FB };
        mImpl->depthTestEnabled = true;
    }

    if (!mImpl->attach(surface)) {
        XRLOG("Jetpack UI disabled: could not attach Compose to the surface");
        xrDestroySwapchain(mImpl->swapchain);
        mImpl->swapchain = XR_NULL_HANDLE;
        mImpl->enabled = false;
        return false;
    }
    XRLOG("Jetpack UI quad created");
    return true;
}

void JetpackUiLayer::terminate() noexcept {
    if (mImpl->swapchain != XR_NULL_HANDLE) {
        mImpl->detach();
        xrDestroySwapchain(mImpl->swapchain);
        mImpl->swapchain = XR_NULL_HANDLE;
    }
    mImpl->releaseActivity();
    mImpl->enabled = false;
}

XrCompositionLayerBaseHeader const* JetpackUiLayer::getLayer(
        Submission* submission) const noexcept {
    if (!mImpl->enabled) {
        return nullptr;
    }
    submission->layer = mImpl->layer;
    submission->layer.next = nullptr;
    if (mImpl->depthTestEnabled) {
        submission->depthTest = mImpl->depthTest;
        submission->layer.next = &submission->depthTest;
    }
    return reinterpret_cast<XrCompositionLayerBaseHeader const*>(&submission->layer);
}

XrPosef JetpackUiLayer::getPose() const noexcept {
    return mImpl->layer.pose;
}

void JetpackUiLayer::setPose(XrPosef const& pose) noexcept {
    mImpl->layer.pose = pose;
}

bool JetpackUiLayer::isEnabled() const noexcept {
    return mImpl->enabled;
}

void JetpackUiLayer::injectTouch(float u, float v, TouchAction action) const {
    if (mImpl->enabled) {
        mImpl->injectTouch(u, v, action);
    }
}

} // namespace helloxr

#else

namespace helloxr {

struct JetpackUiLayer::Impl {};

JetpackUiLayer::JetpackUiLayer() : mImpl(std::make_unique<Impl>()) {}
JetpackUiLayer::~JetpackUiLayer() = default;
void JetpackUiLayer::initializeJava(void*, void*) {}
void JetpackUiLayer::requestExtensions(bool requested,
        std::function<bool(char const*)> const&, std::vector<char const*>*) {
    if (requested) {
        XRLOG("Jetpack UI is available only on Android");
    }
}
void JetpackUiLayer::configure(uint32_t) {}
bool JetpackUiLayer::initialize(XrInstance, XrSession, XrSpace, bool) { return true; }
void JetpackUiLayer::terminate() noexcept {}
XrCompositionLayerBaseHeader const* JetpackUiLayer::getLayer(Submission*) const noexcept {
    return nullptr;
}
XrPosef JetpackUiLayer::getPose() const noexcept { return {}; }
void JetpackUiLayer::setPose(XrPosef const&) noexcept {}
bool JetpackUiLayer::isEnabled() const noexcept { return false; }
void JetpackUiLayer::injectTouch(float, float, TouchAction) const {}

} // namespace helloxr

#endif