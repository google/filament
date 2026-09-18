# APIGen Binding Migration Hardcoding & Special Case Audit

This document provides a comprehensive audit of hardcoded rules, special casing, and class-specific logic across APIGen migrations, explaining the rationale for remaining legacy special cases and defining declarative, annotation-driven solutions to eliminate them.

---

## 1. View Migration Hardcoding Audit

### Summary of View Special Cases

| Location | Identifier / Check | Purpose / Rationale |
| :--- | :--- | :--- |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L529) | `self.name == "View"` | Imports `java.util.EnumSet` required by the legacy deprecated `TargetBufferFlags` enum bitmask helpers (`NONE`, `ALL_COLOR`, `DEPTH_STENCIL`, `ALL`). |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L654) | `self.name == "View"` | Imports `com.google.android.filament.proguard.UsedByNative` to prevent ProGuard/R8 obfuscation of the private nested `InternalOnPickCallback` class invoked from C++ JNI reflection. |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L875) | `self.name == "View"` | Synthesizes legacy deprecated enums `ToneMapping` and `TargetBufferFlags` that were removed from modern C++ `View.h` (superseded by `ColorGrading` and `RenderTarget`) but must remain in `View.java` to prevent breaking existing Android client applications. |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L1223) | `self.name == "View"` | Emits legacy deprecated stubs `setToneMapping` and `getToneMapping`, and defines public asynchronous `pick(...)` overload, `OnPickCallback` interface, and `InternalOnPickCallback` runnable class. |
| [`tools/apigen/javagen/jni_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/jni_emitter.py#L1001) | `self.name == "View"` | Injects required C++ headers: `<filament/Options.h>`, `<filament/Color.h>`, `<filament/Viewport.h>`, `<common/CallbackUtils.h>`, and `<private/backend/VirtualMachineEnv.h>`. |
| [`tools/apigen/javagen/jni_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/jni_emitter.py#L1094) | `self.name == "View"` | Injects custom `Java_com_google_android_filament_View_nPick` JNI bridge implementation, which initializes cached Java reflection field IDs for `InternalOnPickCallback` and invokes `view->pick(...)` asynchronously with `JniCallback::make`. |

---

## 2. `Engine` Remaining Special Cases

| Location | Identifier / Check | Purpose / Rationale |
| :--- | :--- | :--- |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L2337) | `"destroy"` in `intercepted_engine_methods` | Intercepts Engine's destructor to emit instance method `public void destroy()` calling `nDestroyEngine(getNativeObject())` and `clearNativeObject()`, accommodating C++ `Engine::destroy(Engine**)` taking a pointer-to-pointer. |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L2339) | `"createSwapChain"` in `intercepted_engine_methods` | Intercepts `createSwapChain` because the windowed overload requires platform surface unwrapping (`Platform.get().validateSurface(surface)`) and `new SwapChain(nativeSwapChain, surface)`, suppressing both overloads. |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L3840) | `self.name == "Engine" and name == "getFeatureFlag"` | Injects runtime existence validation: `if (!hasFeatureFlag(name)) throw new IllegalArgumentException("Feature flag " + name + " does not exist");`. |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L5053) | `createSwapChain(Object, ...)` | Handwritten windowed SwapChain factory unwrapping Android `Surface` into native window pointer. |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L5063) | `createSwapChain(int, int, ...)` | Handwritten headless SwapChain factory calling `nCreateSwapChainHeadless` and returning `new SwapChain(nativeSwapChain, null)`. |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L5079) | `createSwapChainFromNativeSurface(...)` | Handwritten helper for `NativeSurface` wrapper. |
| [`tools/apigen/javagen/jni_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/jni_emitter.py#L1173) | `"destroy"` & `"createSwapChain"` in `intercepted_engine_methods` | Suppresses automatic JNI function generation for intercepted Engine lifecycle and swapchain methods. |
| [`tools/apigen/javagen/jni_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/jni_emitter.py#L2538-L2557) | `nCreateSwapChain*` and `nDestroyEngine` | Custom JNI bridges calling `getNativeWindow(env, klass, surface)`, `engine->createSwapChain(width, height, flags)`, and `Engine::destroy(&engine)`. |

---

## 3. `Material` & `MaterialInstance` Remaining Special Cases

| Location | Identifier / Check | Purpose / Rationale |
| :--- | :--- | :--- |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L832) | `self.name == "Material"` | Emits `private final MaterialInstance mDefaultInstance;` storage field. |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L1084) | `self.name == "Material"` | Initializes `mDefaultInstance = new MaterialInstance(nGetDefaultInstance(nativeObject), this);` in constructor. |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L2410) | `self.name == "Material" and name == "getDefaultInstance"` | Returns cached `mDefaultInstance` rather than generating a native JNI call. |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L2427) | `self.name == "Material" and name == "getParameters"` | Emits handwritten Java method `public ParameterInfo[] getParameters()`. |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L1078) | `self.name == "MaterialInstance"` | Synthesizes package-private fallback constructor `MaterialInstance(long handle) { this(handle, null); }` for unparented handles instantiated via `wrap(long)`. |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L3791) | `self.name == "MaterialInstance" and name == "duplicate"` | Passes `other.getMaterial()` to `new MaterialInstance(nativeMaterialInstance, other.getMaterial())`. |
| [`tools/apigen/javagen/jni_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/jni_emitter.py#L991-L1055) | `self.name == "Material" and name == "getParameters"` | Injects 70-line custom JNI bridge caching `ParameterInfo` reflection field IDs, calling `that->getParameters(...)`, unpacking discriminant union fields, and returning a `jobjectArray`. |

---

## 4. `ToneMapper` Remaining Special Cases

| Location | Identifier / Check | Purpose / Rationale |
| :--- | :--- | :--- |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L796) | `self.name == "ToneMapper"` | Emits static nested class aliases (`ToneMapper.Linear`, `ToneMapper.ACES`, etc.) forwarding to top-level classes for Java discoverability. |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L1228) | `self.name == "ToneMapper"` | Emits public `destroy()` method calling `nDestroyToneMapper(getNativeObject())` and `clearNativeObject()`. |
| [`tools/apigen/javagen/jni_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/jni_emitter.py#L1059) | `self.name == "ToneMapper"` | Emits custom JNI bridge `Java_com_google_android_filament_ToneMapper_nDestroyToneMapper` calling `delete (ToneMapper*) nativeHandle;`. |

---

## 5. Single-Site Miscellaneous Checks

| Location | Identifier / Check | Purpose / Rationale |
| :--- | :--- | :--- |
| [`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L1501) | `self.name == "PickingQueryResult" and fname == "fragCoords"` | Supplies fallback default array initializer `{0.0f, 0.0f, 0.0f}` for legacy compatibility. |
