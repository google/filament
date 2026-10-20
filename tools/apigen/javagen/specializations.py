#!/usr/bin/env python3
#
# Copyright (C) 2026 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Class-specific Java and JNI C++ specializations for Filament API bindings.

This module centralizes handcrafted Java source snippets, legacy compatibility aliases,
polymorphic subclass hierarchies, reflection helpers, and JNI C++ bridge templates that
supplement or override standard IR-driven code generation in :mod:`javagen.java_emitter`
and :mod:`javagen.jni_emitter`.

Specialized Subsystems:
-----------------------
1. **AttributeBitset Conversion (`ATTRIBUTE_BITSET_JAVA_HELPER`)**:
   - Pre-caches `VertexBuffer.VertexAttribute.values()` in `sVertexAttributeValues` to avoid
     defensive array allocations and converts a 32-bit `AttributeBitset` bitmask into an
     unmodifiable `EnumSet<VertexBuffer.VertexAttribute>`.
2. **ToneMapper Polymorphic Hierarchy (`TONE_MAPPER_*`, `get_tone_mapper_jni`)**:
   - Provides backward-compatible static nested subclass aliases (`Linear`, `ACES`, `ACESLegacy`,
     `Filmic`, `PBRNeutral`, `GT7`, `Agx`, `Generic`) on `ToneMapper` and a `finalize()` destructor
     bridge (`nDestroyToneMapper`) for heap-allocated C++ `ToneMapper` instances.
3. **View Picking & Legacy Enums (`VIEW_*`, `get_view_pick_jni`)**:
   - Supplies deprecated `ToneMapping` enum/methods, `TargetBufferFlags` `EnumSet` helpers, and
     the asynchronous `View.pick(...)` query pipeline backed by `InternalOnPickCallback` and
     cached JNI field IDs (`JniState`).
4. **Texture PixelBufferDescriptor Alias (`PIXEL_BUFFER_DESCRIPTOR_ALIAS_JAVA`)**:
   - Emits the `@Deprecated` nested `Texture.PixelBufferDescriptor` subclass delegating to
     top-level `com.google.android.filament.PixelBufferDescriptor`.
5. **Engine Lifecycle, Config & Builder (`ENGINE_*`, `get_engine_*`)**:
   - Defines `Engine.Config` arena size constants and default field initializers, custom
     `Engine.create(...)` / `createSwapChain(...)` / `destroy*(...)` lifecycle methods,
     `Engine.Builder`, and their corresponding JNI C++ implementations.
6. **Material Reflection & Default Instance (`MATERIAL_*`, `get_material_get_parameters_jni`)**:
   - Implements `Material.getDefaultInstance()` returning the cached `mDefaultInstance` field
     and `Material.getParameters(...)` populating `ParameterInfo[]` via JNI reflection.
7. **FramePacer VsyncTick Convenience Overloads (`get_vsync_tick_convenience_java`)**:
   - Synthesizes overloaded `setupFrame(...)` convenience methods unpacking `VsyncTick` fields.
"""

from typing import Dict, List

# Java helper methods emitted when a class returns a C++ `AttributeBitset` bitmask
# (e.g. `Material.getRequiredAttributes()`), converting the integer bitmask into an
# unmodifiable `Set<VertexBuffer.VertexAttribute>` without per-call `.values()` allocation.
ATTRIBUTE_BITSET_JAVA_HELPER: List[str] = [
    "    private static final VertexBuffer.VertexAttribute[] sVertexAttributeValues =",
    "            VertexBuffer.VertexAttribute.values();",
    "",
    "    private static Set<VertexBuffer.VertexAttribute> getAttributes(int bitSet) {",
    "        Set<VertexBuffer.VertexAttribute> set = EnumSet.noneOf(VertexBuffer.VertexAttribute.class);",
    "        VertexBuffer.VertexAttribute[] values = sVertexAttributeValues;",
    "        for (int i = 0, n = values.length; i < n; i++) {",
    "            if ((bitSet & (1 << i)) != 0) {",
    "                set.add(values[i]);",
    "            }",
    "        }",
    "        return Collections.unmodifiableSet(set);",
    "    }",
    "",
]

# Backward-compatible static inner classes on `ToneMapper` extending the standalone
# top-level `*ToneMapper` subclasses (`LinearToneMapper`, `ACESToneMapper`, `AgxToneMapper`, etc.).
TONE_MAPPER_SUBCLASSES_JAVA: List[str] = [
    "    public static class Linear extends LinearToneMapper {}",
    "    public static class ACES extends ACESToneMapper {}",
    "    public static class ACESLegacy extends ACESLegacyToneMapper {}",
    "    public static class Filmic extends FilmicToneMapper {}",
    "    public static class PBRNeutral extends PBRNeutralToneMapper {}",
    "    public static class GT7 extends GT7ToneMapper {}",
    "    public static class Agx extends AgxToneMapper {",
    "        public static class AgxLook {",
    "            public static final AgxToneMapper.AgxLook NONE = AgxToneMapper.AgxLook.NONE;",
    "            public static final AgxToneMapper.AgxLook PUNCHY = AgxToneMapper.AgxLook.PUNCHY;",
    "            public static final AgxToneMapper.AgxLook GOLDEN = AgxToneMapper.AgxLook.GOLDEN;",
    "        }",
    "        public Agx() {",
    "            super();",
    "        }",
    "        public Agx(AgxToneMapper.AgxLook look) {",
    "            super(look);",
    "        }",
    "    }",
    "    public static class Generic extends GenericToneMapper {",
    "        public Generic() {",
    "            super();",
    "        }",
    "        public Generic(float contrast, float midGrayIn, float midGrayOut, float hdrMax) {",
    "            super(contrast, midGrayIn, midGrayOut, hdrMax);",
    "        }",
    "    }",
    "",
]

# Java `finalize()` override on `ToneMapper` to ensure heap-allocated C++ `ToneMapper`
# instances (which are not owned by `Engine`) are freed when garbage-collected.
TONE_MAPPER_FINALIZE_JAVA: List[str] = [
    "    @Override",
    "    protected void finalize() throws Throwable {",
    "        try {",
    "            super.finalize();",
    "        } finally {",
    "            nDestroyToneMapper(mNativeObject);",
    "        }",
    "    }",
    "",
]

# Private static native declaration for destroying a heap-allocated `ToneMapper`.
TONE_MAPPER_NATIVE_DECL_JAVA = (
    "    private static native void nDestroyToneMapper(long nativeObject);"
)


def get_tone_mapper_jni(jni_package: str) -> List[str]:
    """Return JNI C++ source lines for `ToneMapper.nDestroyToneMapper`.

    Unlike Engine-managed resources, `ToneMapper` subclasses are heap-allocated directly
    via `new` in their JNI constructors and destroyed via `delete` when finalized.

    Args:
        jni_package: Mangled JNI package prefix (e.g. `'com_google_android_filament'`).

    Returns:
        List of C++ source lines defining `Java_<pkg>_ToneMapper_nDestroyToneMapper`.
    """
    return [
        'extern "C" JNIEXPORT void JNICALL',
        f"Java_{jni_package}_ToneMapper_nDestroyToneMapper(JNIEnv *env, jclass clazz, jlong toneMapper_) {{",
        "    ToneMapper* toneMapper = (ToneMapper*) toneMapper_;",
        "    delete toneMapper;",
        "}",
        "",
    ]


# Legacy nested enums emitted inside `View`:
# - `@Deprecated ToneMapping` enum retained for backward binary/source compatibility.
# - `TargetBufferFlags` enum with precomputed `EnumSet` constants (`NONE`, `ALL_COLOR`,
#   `DEPTH_STENCIL`, `ALL`) and a bitmask reduction helper `flags(EnumSet<TargetBufferFlags>)`.
VIEW_ENUMS_JAVA: List[str] = [
    "    /**",
    "     * List of available tone-mapping operators",
    "     *",
    "     * @deprecated Use ColorGrading instead",
    "     */",
    "    @Deprecated",
    "    public enum ToneMapping {",
    "        /**",
    "         * Equivalent to disabling tone-mapping.",
    "         */",
    "        LINEAR,",
    "",
    "        /**",
    "         * The Academy Color Encoding System (ACES).",
    "         */",
    "        ACES",
    "    }",
    "",
    "    /**",
    "     * Used to select buffers.",
    "     */",
    "    public enum TargetBufferFlags {",
    "        /**",
    "         * Color 0 buffer selected.",
    "         */",
    "        COLOR0(0x1),",
    "        /**",
    "         * Color 1 buffer selected.",
    "         */",
    "        COLOR1(0x2),",
    "        /**",
    "         * Color 2 buffer selected.",
    "         */",
    "        COLOR2(0x4),",
    "        /**",
    "         * Color 3 buffer selected.",
    "         */",
    "        COLOR3(0x8),",
    "        /**",
    "         * Depth buffer selected.",
    "         */",
    "        DEPTH(0x10),",
    "        /**",
    "         * Stencil buffer selected.",
    "         */",
    "        STENCIL(0x20);",
    "",
    "        /*",
    "         * No buffer selected",
    "         */",
    "        public static EnumSet<TargetBufferFlags> NONE = EnumSet.noneOf(TargetBufferFlags.class);",
    "",
    "        /*",
    "         * All color buffers selected",
    "         */",
    "        public static EnumSet<TargetBufferFlags> ALL_COLOR =",
    "                EnumSet.of(COLOR0, COLOR1, COLOR2, COLOR3);",
    "        /**",
    "         * Depth and stencil buffer selected.",
    "         */",
    "        public static EnumSet<TargetBufferFlags> DEPTH_STENCIL = EnumSet.of(DEPTH, STENCIL);",
    "        /**",
    "         * All buffers are selected.",
    "         */",
    "        public static EnumSet<TargetBufferFlags> ALL = EnumSet.range(COLOR0, STENCIL);",
    "",
    "        private int mFlags;",
    "",
    "        TargetBufferFlags(int flags) {",
    "            mFlags = flags;",
    "        }",
    "",
    "        static int flags(EnumSet<TargetBufferFlags> flags) {",
    "            int result = 0;",
    "            for (TargetBufferFlags flag : flags) {",
    "                result |= flag.mFlags;",
    "            }",
    "            return result;",
    "        }",
    "    }",
    "",
]

# Handcrafted `View` methods and inner classes:
# - Deprecated `setToneMapping` / `getToneMapping` stubs.
# - `OnPickCallback` public listener interface and `View.pick(...)` entry point.
# - `InternalOnPickCallback` runnable populated from C++ via JNI field reflection before
#   dispatching `PickingQueryResult` on the user-provided `Handler` or `Executor`.
VIEW_METHODS_AND_PICK_JAVA: List[str] = [
    "    /**",
    "     * @deprecated Use {@link #setColorGrading(ColorGrading)}",
    "     */",
    "    @Deprecated",
    "    public void setToneMapping(@NonNull ToneMapping type) {",
    "    }",
    "",
    "    /**",
    "     * @deprecated Use {@link #getColorGrading()}. This always returns {@link ToneMapping#ACES}",
    "     */",
    "    @Deprecated",
    "    @NonNull",
    "    public ToneMapping getToneMapping() {",
    "        return ToneMapping.ACES;",
    "    }",
    "",
    "    /**",
    "     * An interface to implement a custom class to receive results of picking queries.",
    "     */",
    "    public interface OnPickCallback {",
    "        /**",
    "         * onPick() is called by the specified Handler in {@link View#pick} when the picking query",
    "         * result is available.",
    "         * @param result An instance of {@link PickingQueryResult}.",
    "         */",
    "        void onPick(@NonNull PickingQueryResult result);",
    "    }",
    "",
    "    /**",
    "     * Creates a picking query. Multiple queries can be created (e.g.: multi-touch).",
    "     * Picking queries are all executed when {@link Renderer#render} is called on this View.",
    "     * The provided callback is guaranteed to be called at some point in the future.",
    "     *",
    "     * <p>Typically it takes a couple frames to receive the result of a picking query.</p>",
    "     *",
    "     * @param x        Horizontal coordinate to query in the viewport with origin on the left.",
    "     * @param y        Vertical coordinate to query on the viewport with origin at the bottom.",
    "     * @param handler  An {@link java.util.concurrent.Executor Executor}.",
    "     *                 On Android this can also be a {@link android.os.Handler Handler}.",
    "     * @param callback User callback executed by <code>handler</code> when the picking query",
    "     *                 result is available.",
    "     */",
    "    public void pick(int x, int y,",
    "            @Nullable Object handler, @Nullable OnPickCallback callback) {",
    "        InternalOnPickCallback internalCallback = new InternalOnPickCallback(callback);",
    "        nPick(getNativeObject(), x, y, handler, internalCallback);",
    "    }",
    "",
    '    @UsedByNative("View.cpp")',
    "    private static class InternalOnPickCallback implements Runnable {",
    "        private final OnPickCallback mUserCallback;",
    "        private final PickingQueryResult mPickingQueryResult = new PickingQueryResult();",
    "",
    '        @UsedByNative("View.cpp")',
    "        @Entity",
    "        int mRenderable;",
    "",
    '        @UsedByNative("View.cpp")',
    "        float mDepth;",
    "",
    '        @UsedByNative("View.cpp")',
    "        float mFragCoordsX;",
    '        @UsedByNative("View.cpp")',
    "        float mFragCoordsY;",
    '        @UsedByNative("View.cpp")',
    "        float mFragCoordsZ;",
    "",
    "        public InternalOnPickCallback(OnPickCallback mUserCallback) {",
    "            this.mUserCallback = mUserCallback;",
    "        }",
    "",
    "        @Override",
    "        public void run() {",
    "            mPickingQueryResult.renderable = mRenderable;",
    "            mPickingQueryResult.depth = mDepth;",
    "            mPickingQueryResult.fragCoords[0] = mFragCoordsX;",
    "            mPickingQueryResult.fragCoords[1] = mFragCoordsY;",
    "            mPickingQueryResult.fragCoords[2] = mFragCoordsZ;",
    "            if (mUserCallback != null) {",
    "                mUserCallback.onPick(mPickingQueryResult);",
    "            }",
    "        }",
    "    }",
    "",
]

# Private static native declaration for `View.nPick(...)`.
VIEW_PICK_NATIVE_DECL_JAVA = (
    "    private static native void nPick(long nativeView, int x, int y, "
    "@Nullable Object handler, @NonNull InternalOnPickCallback internalCallback);"
)


def get_view_pick_jni(jni_func_name: str) -> str:
    """Return the JNI C++ implementation for `View.nPick`.

    Caches `View$InternalOnPickCallback` field IDs in a function-local `static const JniState`
    struct on first invocation, wraps `handler` and `internalCallback` in a `JniCallback`,
    and forwards the C++ `View::PickingQueryResult` fields into the Java callback runnable
    before posting it back to the Java thread via `JniCallback::postToJavaAndDestroy`.

    Args:
        jni_func_name: Fully mangled JNI function symbol for `View.nPick`.

    Returns:
        Formatted C++ JNI function definition string.
    """
    return f'''extern "C" JNIEXPORT void JNICALL
{jni_func_name}(JNIEnv* env, jclass clazz,
        jlong nativeView,
        jint x, jint y, jobject handler, jobject internalCallback) {{

    static const struct JniState {{
        jclass internalOnPickCallbackClass;
        jfieldID renderableFieldId;
        jfieldID depthFieldId;
        jfieldID fragCoordXFieldId;
        jfieldID fragCoordYFieldId;
        jfieldID fragCoordZFieldId;
        explicit JniState(JNIEnv* env) noexcept {{
            internalOnPickCallbackClass = env->FindClass("com/google/android/filament/View$InternalOnPickCallback");
            renderableFieldId = env->GetFieldID(internalOnPickCallbackClass, "mRenderable", "I");
            depthFieldId = env->GetFieldID(internalOnPickCallbackClass, "mDepth", "F");
            fragCoordXFieldId = env->GetFieldID(internalOnPickCallbackClass, "mFragCoordsX", "F");
            fragCoordYFieldId = env->GetFieldID(internalOnPickCallbackClass, "mFragCoordsY", "F");
            fragCoordZFieldId = env->GetFieldID(internalOnPickCallbackClass, "mFragCoordsZ", "F");
        }}
    }} jniState(env);

    View* view = (View*) nativeView;
    JniCallback *callback = JniCallback::make(env, handler, internalCallback);
    view->pick(x, y, [callback](View::PickingQueryResult const& result) {{
        jobject obj = callback->getCallbackObject();
        JNIEnv* env = filament::VirtualMachineEnv::get().getEnvironment();
        env->SetIntField(obj, jniState.renderableFieldId, (jint)result.renderable.getId());
        env->SetFloatField(obj, jniState.depthFieldId, result.depth);
        env->SetFloatField(obj, jniState.fragCoordXFieldId, result.fragCoords.x);
        env->SetFloatField(obj, jniState.fragCoordYFieldId, result.fragCoords.y);
        env->SetFloatField(obj, jniState.fragCoordZFieldId, result.fragCoords.z);
        JniCallback::postToJavaAndDestroy(callback);
    }}, callback->getHandler());
}}'''


# Deprecated `Texture.PixelBufferDescriptor` static inner class extending top-level
# `com.google.android.filament.PixelBufferDescriptor` for backward compatibility.
PIXEL_BUFFER_DESCRIPTOR_ALIAS_JAVA: List[str] = [
    "    /**",
    "     * @deprecated Use {@link com.google.android.filament.PixelBufferDescriptor} instead.",
    "     */",
    "    @Deprecated",
    "    public static class PixelBufferDescriptor extends com.google.android.filament.PixelBufferDescriptor {",
    "        public PixelBufferDescriptor(@NonNull Buffer storage,",
    "                @NonNull Texture.Format format, @NonNull Texture.Type type,",
    "                @IntRange(from = 1, to = 8) int alignment,",
    "                @IntRange(from = 0) int left, @IntRange(from = 0) int top,",
    "                @IntRange(from = 0) int stride,",
    "                @Nullable Object handler, @Nullable Runnable callback) {",
    "            super(storage, format, type, alignment, left, top, stride, handler, callback);",
    "        }",
    "",
    "        public PixelBufferDescriptor(@NonNull Buffer storage,",
    "                @NonNull Texture.Format format, @NonNull Texture.Type type) {",
    "            super(storage, format, type);",
    "        }",
    "",
    "        public PixelBufferDescriptor(@NonNull Buffer storage,",
    "                @NonNull Texture.Format format, @NonNull Texture.Type type,",
    "                @IntRange(from = 1, to = 8) int alignment) {",
    "            super(storage, format, type, alignment);",
    "        }",
    "",
    "        public PixelBufferDescriptor(@NonNull Buffer storage,",
    "                @NonNull Texture.Format format, @NonNull Texture.Type type,",
    "                @IntRange(from = 1, to = 8) int alignment,",
    "                @IntRange(from = 0) int left, @IntRange(from = 0) int top) {",
    "            super(storage, format, type, alignment, left, top);",
    "        }",
    "",
    "        public PixelBufferDescriptor(@NonNull Buffer storage,",
    "                @NonNull Texture.Format format, @NonNull Texture.Type type,",
    "                @Nullable Object handler, @Nullable Runnable callback) {",
    "            this(storage, format, type, 1, 0, 0, 0, handler, callback);",
    "        }",
    "",
    "        public PixelBufferDescriptor(@NonNull Buffer storage,",
    "                @NonNull Texture.Format format, @NonNull Texture.Type type,",
    "                @IntRange(from = 1, to = 8) int alignment,",
    "                @Nullable Object handler, @Nullable Runnable callback) {",
    "            this(storage, format, type, alignment, 0, 0, 0, handler, callback);",
    "        }",
    "",
    "        public PixelBufferDescriptor(@NonNull ByteBuffer storage,",
    "                @NonNull Texture.CompressedType compressedFormat,",
    "                @IntRange(from = 0) int compressedSizeInBytes,",
    "                @Nullable Object handler, @Nullable Runnable callback) {",
    "            this(storage, compressedFormat, compressedSizeInBytes);",
    "            setCallback(handler, callback);",
    "        }",
    "",
    "        public PixelBufferDescriptor(@NonNull ByteBuffer storage,",
    "                @NonNull Texture.CompressedType compressedFormat,",
    "                @IntRange(from = 0) int compressedSizeInBytes) {",
    "            super(storage, compressedFormat, compressedSizeInBytes);",
    "        }",
    "    }",
    "",
]


def get_engine_config_constants_java(indent: str) -> List[str]:
    """Return constant declarations emitted at the top of `Engine.Config`.

    Mirrors the C++ preprocessor `#define` arena size constants in `<filament/Engine.h>`
    (`FILAMENT_PER_RENDER_PASS_ARENA_SIZE_IN_MB`, `FILAMENT_COMMAND_BUFFER_SIZE_IN_MB`, etc.)
    as well as the `SINGLE_THREADED` sentinel value (`0xFFFFFFFFL`) for `jobSystemThreadCount`.

    Args:
        indent: Leading indentation string for the `Engine.Config` inner class.

    Returns:
        List of indented Java constant declaration lines.
    """
    return [
        f"{indent}    // #defines in Engine.h",
        f"{indent}    private static final long FILAMENT_PER_RENDER_PASS_ARENA_SIZE_IN_MB = 3;",
        f"{indent}    private static final long FILAMENT_PER_FRAME_COMMANDS_SIZE_IN_MB = 2;",
        f"{indent}    private static final long FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB = 1;",
        f"{indent}    private static final long FILAMENT_COMMAND_BUFFER_SIZE_IN_MB =",
        f"{indent}            FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB * 3;",
        f"{indent}",
        f"{indent}    /**",
        f"{indent}     * Special value for jobSystemThreadCount, forcing the JobSystem to be single-threaded.",
        f"{indent}     */",
        f"{indent}    public static final long SINGLE_THREADED = 0xFFFFFFFFL;",
        f"{indent}",
    ]


# Default Java initializer expressions for `Engine.Config` POJO fields, matching the
# C++ member initializers in `filament::Engine::Config`.
ENGINE_CONFIG_DEFAULTS: Dict[str, str] = {
    "commandBufferSizeMB": "FILAMENT_COMMAND_BUFFER_SIZE_IN_MB",
    "perRenderPassArenaSizeMB": "FILAMENT_PER_RENDER_PASS_ARENA_SIZE_IN_MB",
    "driverHandleArenaSizeMB": "0",
    "minCommandBufferSizeMB": "FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB",
    "perFrameCommandsSizeMB": "FILAMENT_PER_FRAME_COMMANDS_SIZE_IN_MB",
    "jobSystemThreadCount": "0",
    "metalUploadBufferSizeBytes": "512 * 1024",
    "metalDisablePanicOnDrawableFailure": "false",
    "disableParallelShaderCompile": "false",
    "stereoscopicType": "StereoscopicType.NONE",
    "stereoscopicEyeCount": "2",
    "resourceAllocatorCacheSizeMB": "64",
    "resourceAllocatorCacheMaxAge": "1",
    "disableHandleUseAfterFreeCheck": "false",
    "preferredShaderLanguage": "ShaderLanguage.DEFAULT",
    "forceGLES2Context": "false",
    "assertNativeWindowIsValid": "false",
    "gpuContextPriority": "GpuContextPriority.DEFAULT",
    "sharedUboInitialSizeInBytes": "256 * 64",
    "asynchronousMode": "AsynchronousMode.NONE",
    "materialCacheCapacity": "0",
    "programCacheCapacity": "0",
    "enableMultipleDirectionalLights": "false",
}


def get_vsync_tick_convenience_java(indent_str: str, body_indent_str: str) -> str:
    """Return overloaded `FramePacer.setupFrame(...)` convenience methods for `VsyncTick`.

    Generates three progressively simpler `setupFrame` overloads that omit `VsyncTick`
     fields (`latchTimeNanos`, `hardwareTimelines`, `vsyncPeriodNanos` defaulting to 60Hz
    `16666666L`) and delegate to `nSetupFrame` or the 4-argument overload.

    Args:
        indent_str: Method declaration indentation string (e.g. `'    '`).
        body_indent_str: Method body indentation string (e.g. `'        '`).

    Returns:
        Formatted Java source block containing the three `setupFrame` convenience overloads.
    """
    return "\n".join([
        f"{indent_str}public FrameStatus setupFrame(long frameTimeNanos, long vsyncPeriodNanos, @Nullable long[] hardwareTimelines, int timelineCount) {{",
        f"{body_indent_str}return FrameStatus.from(nSetupFrame(getNativeObject(), frameTimeNanos, vsyncPeriodNanos, 0, hardwareTimelines, timelineCount));",
        f"{indent_str}}}",
        "",
        f"{indent_str}public FrameStatus setupFrame(long frameTimeNanos, long vsyncPeriodNanos) {{",
        f"{body_indent_str}return setupFrame(frameTimeNanos, vsyncPeriodNanos, null, 0);",
        f"{indent_str}}}",
        "",
        f"{indent_str}public FrameStatus setupFrame(long frameTimeNanos) {{",
        f"{body_indent_str}return setupFrame(frameTimeNanos, 16666666L);",
        f"{indent_str}}}",
    ])


# Java body for `Material.getDefaultInstance()` returning the cached `mDefaultInstance`
# wrapper initialized in `Material`'s constructor rather than allocating a new wrapper per call.
MATERIAL_GET_DEFAULT_INSTANCE_BODY_JAVA: List[str] = [
    "    @NonNull",
    "    public MaterialInstance getDefaultInstance() {",
    "        return mDefaultInstance;",
    "    }",
]

# Java overloads for `Material.getParameters(...)` allocating and populating an array
# of `Material.ParameterInfo` POJOs via `nGetParameters`.
MATERIAL_GET_PARAMETERS_BODY_JAVA: List[str] = [
    "    @NonNull",
    "    public ParameterInfo[] getParameters() {",
    "        int count = getParameterCount();",
    "        ParameterInfo[] parameters = new ParameterInfo[count];",
    "        if (count > 0) {",
    "            for (int i = 0; i < count; i++) {",
    "                parameters[i] = new ParameterInfo();",
    "            }",
    "            nGetParameters(getNativeObject(), parameters, count);",
    "        }",
    "        return parameters;",
    "    }",
    "",
    "    @IntRange(from = 0)",
    "    public int getParameters(@NonNull ParameterInfo[] parameters, @IntRange(from = 0) int count) {",
    "        return nGetParameters(getNativeObject(), parameters, count);",
    "    }",
]

# Private static native declaration for `Material.nGetParameters(...)`.
MATERIAL_GET_PARAMETERS_NATIVE_DECL_JAVA = (
    "    private static native int nGetParameters(long nativeMaterial, "
    "@NonNull ParameterInfo[] parameters, @IntRange(from = 0) int count);"
)


def get_material_get_parameters_jni(jni_func_name: str) -> str:
    """Return the JNI C++ implementation for `Material.nGetParameters`.

    Queries `Material::getParameters(ParameterInfo*, size_t)` into a temporary `std::vector`,
    resolves the Java `Material$ParameterInfo` fields and enum `.values()` arrays (`ParameterType`,
    `SamplerType`, `SubpassType`, `Precision`), and populates each element of the Java
    `ParameterInfo[]` array in-place while cleaning up all local JNI references.

    Args:
        jni_func_name: Fully mangled JNI function symbol for `Material.nGetParameters`.

    Returns:
        Formatted C++ JNI function definition string.
    """
    return "\n".join([
        'extern "C" JNIEXPORT jint JNICALL',
        f"{jni_func_name}(JNIEnv *env, jclass clazz, jlong nativeMaterial, jobjectArray parameters, jint count) {{",
        "    Material const * const that = (Material const *) nativeMaterial;",
        "    if (!parameters || count <= 0) {",
        "        return 0;",
        "    }",
        "    std::vector<Material::ParameterInfo> info(count);",
        "    size_t const received = that->getParameters(info.data(), (size_t)count);",
        "",
        '    jclass const infoClass = env->FindClass("com/google/android/filament/Material$ParameterInfo");',
        '    jmethodID const infoCtor = env->GetMethodID(infoClass, "<init>", "()V");',
        '    jfieldID const nameField = env->GetFieldID(infoClass, "name", "Ljava/lang/String;");',
        '    jfieldID const isSamplerField = env->GetFieldID(infoClass, "isSampler", "Z");',
        '    jfieldID const isSubpassField = env->GetFieldID(infoClass, "isSubpass", "Z");',
        '    jfieldID const typeField = env->GetFieldID(infoClass, "type", "Lcom/google/android/filament/Material$ParameterType;");',
        '    jfieldID const samplerTypeField = env->GetFieldID(infoClass, "samplerType", "Lcom/google/android/filament/Material$SamplerType;");',
        '    jfieldID const subpassTypeField = env->GetFieldID(infoClass, "subpassType", "Lcom/google/android/filament/Material$SubpassType;");',
        '    jfieldID const countField = env->GetFieldID(infoClass, "count", "I");',
        '    jfieldID const precisionField = env->GetFieldID(infoClass, "precision", "Lcom/google/android/filament/Material$Precision;");',
        "",
        '    jclass const pTypeClass = env->FindClass("com/google/android/filament/Material$ParameterType");',
        '    jmethodID const pTypeValues = env->GetStaticMethodID(pTypeClass, "values", "()[Lcom/google/android/filament/Material$ParameterType;");',
        "    jobjectArray const pTypeArray = (jobjectArray)env->CallStaticObjectMethod(pTypeClass, pTypeValues);",
        "",
        '    jclass const sTypeClass = env->FindClass("com/google/android/filament/Material$SamplerType");',
        '    jmethodID const sTypeValues = env->GetStaticMethodID(sTypeClass, "values", "()[Lcom/google/android/filament/Material$SamplerType;");',
        "    jobjectArray const sTypeArray = (jobjectArray)env->CallStaticObjectMethod(sTypeClass, sTypeValues);",
        "",
        '    jclass const spTypeClass = env->FindClass("com/google/android/filament/Material$SubpassType");',
        '    jmethodID const spTypeValues = env->GetStaticMethodID(spTypeClass, "values", "()[Lcom/google/android/filament/Material$SubpassType;");',
        "    jobjectArray const spTypeArray = (jobjectArray)env->CallStaticObjectMethod(spTypeClass, spTypeValues);",
        "",
        '    jclass const precClass = env->FindClass("com/google/android/filament/Material$Precision");',
        '    jmethodID const precValues = env->GetStaticMethodID(precClass, "values", "()[Lcom/google/android/filament/Material$Precision;");',
        "    jobjectArray const precArray = (jobjectArray)env->CallStaticObjectMethod(precClass, precValues);",
        "",
        "    for (size_t i = 0; i < received; ++i) {",
        "        jobject elem = env->GetObjectArrayElement(parameters, (jsize)i);",
        "        if (!elem) {",
        "            elem = env->NewObject(infoClass, infoCtor);",
        "            env->SetObjectArrayElement(parameters, (jsize)i, elem);",
        "        }",
        "        if (elem) {",
        "            jstring const jname = env->NewStringUTF(info[i].name);",
        "            env->SetObjectField(elem, nameField, jname);",
        "            env->DeleteLocalRef(jname);",
        "",
        "            env->SetBooleanField(elem, isSamplerField, (jboolean)info[i].isSampler);",
        "            env->SetBooleanField(elem, isSubpassField, (jboolean)info[i].isSubpass);",
        "            env->SetIntField(elem, countField, (jint)info[i].count);",
        "",
        "            if (info[i].isSampler) {",
        "                jobject const sTypeObj = env->GetObjectArrayElement(sTypeArray, (jsize)info[i].samplerType);",
        "                env->SetObjectField(elem, samplerTypeField, sTypeObj);",
        "                env->DeleteLocalRef(sTypeObj);",
        "                env->SetObjectField(elem, typeField, nullptr);",
        "                env->SetObjectField(elem, subpassTypeField, nullptr);",
        "            } else if (info[i].isSubpass) {",
        "                jobject const spTypeObj = env->GetObjectArrayElement(spTypeArray, (jsize)info[i].subpassType);",
        "                env->SetObjectField(elem, subpassTypeField, spTypeObj);",
        "                env->DeleteLocalRef(spTypeObj);",
        "                env->SetObjectField(elem, typeField, nullptr);",
        "                env->SetObjectField(elem, samplerTypeField, nullptr);",
        "            } else {",
        "                jobject const pTypeObj = env->GetObjectArrayElement(pTypeArray, (jsize)info[i].type);",
        "                env->SetObjectField(elem, typeField, pTypeObj);",
        "                env->DeleteLocalRef(pTypeObj);",
        "                env->SetObjectField(elem, samplerTypeField, nullptr);",
        "                env->SetObjectField(elem, subpassTypeField, nullptr);",
        "            }",
        "",
        "            jobject const precObj = env->GetObjectArrayElement(precArray, (jsize)info[i].precision);",
        "            env->SetObjectField(elem, precisionField, precObj);",
        "            env->DeleteLocalRef(precObj);",
        "",
        "            env->DeleteLocalRef(elem);",
        "        }",
        "    }",
        "    env->DeleteLocalRef(precArray);",
        "    env->DeleteLocalRef(spTypeArray);",
        "    env->DeleteLocalRef(sTypeArray);",
        "    env->DeleteLocalRef(pTypeArray);",
        "",
        "    return (jint)received;",
        "}",
    ])


# Engine method names intercepted during IR method iteration because their Java and JNI
# implementations are provided in bulk by `ENGINE_METHODS_JAVA` / `get_engine_jni_methods`.
ENGINE_INTERCEPTED_METHODS = ("destroy", "createSwapChain")

# Handcrafted `Engine` Java constructor, static `create(...)` factories, `SwapChain`
# creation overloads (validating the platform surface via `Platform.get().validateSurface`),
# typed and overloaded `destroy*(...)` resource lifecycle methods (asserting successful
# destruction and clearing the Java wrapper's native handle), and `assertDestroy`.
ENGINE_METHODS_JAVA = """    private Engine(long nativeEngine, @Nullable Config config) {
        mNativeObject = nativeEngine;
        mConfig = config;
    }

    @NonNull
    public static Engine create() {
        return new Builder().build();
    }

    @NonNull
    public static Engine create(@NonNull Backend backend) {
        return new Builder()
            .backend(backend)
            .build();
    }

    @NonNull
    public static Engine create(@NonNull Object sharedContext) {
        return new Builder()
            .sharedContext(sharedContext)
            .build();
    }

    public void destroy() {
        nDestroyEngine(getNativeObject());
        clearNativeObject();
    }

    @NonNull
    public SwapChain createSwapChain(@NonNull Object surface) {
        return createSwapChain(surface, SwapChainFlags.CONFIG_DEFAULT);
    }

    @NonNull
    public SwapChain createSwapChain(@NonNull Object surface, long flags) {
        if (Platform.get().validateSurface(surface)) {
            long nativeSwapChain = nCreateSwapChain(getNativeObject(), surface, flags);
            if (nativeSwapChain == 0) throw new IllegalStateException("Couldn't create SwapChain");
            return new SwapChain(nativeSwapChain, surface);
        }
        throw new IllegalArgumentException("Invalid surface " + surface);
    }

    @NonNull
    public SwapChain createSwapChain(int width, int height, long flags) {
        if (width >= 0 && height >= 0) {
            long nativeSwapChain =
                nCreateSwapChainHeadless(getNativeObject(), width, height, flags);
            if (nativeSwapChain == 0) throw new IllegalStateException("Couldn't create SwapChain");
            return new SwapChain(nativeSwapChain, null);
        }
        throw new IllegalArgumentException("Invalid parameters");
    }

    @NonNull
    public SwapChain createSwapChain(int width, int height) {
        return createSwapChain(width, height, SwapChainFlags.CONFIG_DEFAULT);
    }

    @NonNull
    public SwapChain createSwapChainFromNativeSurface(@NonNull NativeSurface surface, long flags) {
        long nativeSwapChain =
                nCreateSwapChainFromRawPointer(getNativeObject(), surface.getNativeObject(), flags);
        if (nativeSwapChain == 0) throw new IllegalStateException("Couldn't create SwapChain");
        return new SwapChain(nativeSwapChain, surface);
    }


    public void destroySwapChain(@NonNull SwapChain swapChain) {
        assertDestroy(nDestroySwapChain(getNativeObject(), swapChain.getNativeObject()));
        swapChain.clearNativeObject();
    }

    public void destroyView(@NonNull View view) {
        assertDestroy(nDestroyView(getNativeObject(), view.getNativeObject()));
        view.clearNativeObject();
    }

    public void destroyRenderer(@NonNull Renderer renderer) {
        assertDestroy(nDestroyRenderer(getNativeObject(), renderer.getNativeObject()));
        renderer.clearNativeObject();
    }

    public void destroyScene(@NonNull Scene scene) {
        assertDestroy(nDestroyScene(getNativeObject(), scene.getNativeObject()));
        scene.clearNativeObject();
    }

    public void destroyStream(@NonNull Stream stream) {
        assertDestroy(nDestroyStream(getNativeObject(), stream.getNativeObject()));
        stream.clearNativeObject();
    }

    public void destroyFence(@NonNull Fence fence) {
        assertDestroy(nDestroyFence(getNativeObject(), fence.getNativeObject()));
        fence.clearNativeObject();
    }

    public void destroyFramePacer(@NonNull FramePacer framePacer) {
        assertDestroy(nDestroyFramePacer(getNativeObject(), framePacer.getNativeObject()));
        framePacer.clearNativeObject();
    }

    public void destroyBufferObject(@NonNull BufferObject bufferObject) {
        assertDestroy(nDestroyBufferObject(getNativeObject(), bufferObject.getNativeObject()));
        bufferObject.clearNativeObject();
    }

    public void destroyIndexBuffer(@NonNull IndexBuffer indexBuffer) {
        assertDestroy(nDestroyIndexBuffer(getNativeObject(), indexBuffer.getNativeObject()));
        indexBuffer.clearNativeObject();
    }

    public void destroyInstanceBuffer(@NonNull InstanceBuffer instanceBuffer) {
        assertDestroy(nDestroyInstanceBuffer(getNativeObject(), instanceBuffer.getNativeObject()));
        instanceBuffer.clearNativeObject();
    }

    public void destroyVertexBuffer(@NonNull VertexBuffer vertexBuffer) {
        assertDestroy(nDestroyVertexBuffer(getNativeObject(), vertexBuffer.getNativeObject()));
        vertexBuffer.clearNativeObject();
    }

    public void destroySkinningBuffer(@NonNull SkinningBuffer skinningBuffer) {
        assertDestroy(nDestroySkinningBuffer(getNativeObject(), skinningBuffer.getNativeObject()));
        skinningBuffer.clearNativeObject();
    }

    public void destroyMorphTargetBuffer(@NonNull MorphTargetBuffer morphTargetBuffer) {
        assertDestroy(nDestroyMorphTargetBuffer(getNativeObject(), morphTargetBuffer.getNativeObject()));
        morphTargetBuffer.clearNativeObject();
    }

    public void destroyIndirectLight(@NonNull IndirectLight ibl) {
        assertDestroy(nDestroyIndirectLight(getNativeObject(), ibl.getNativeObject()));
        ibl.clearNativeObject();
    }

    public void destroyMaterial(@NonNull Material material) {
        assertDestroy(nDestroyMaterial(getNativeObject(), material.getNativeObject()));
        material.clearNativeObject();
    }

    public void destroyMaterialInstance(@NonNull MaterialInstance materialInstance) {
        assertDestroy(nDestroyMaterialInstance(getNativeObject(), materialInstance.getNativeObject()));
        materialInstance.clearNativeObject();
    }

    public void destroySkybox(@NonNull Skybox skybox) {
        assertDestroy(nDestroySkybox(getNativeObject(), skybox.getNativeObject()));
        skybox.clearNativeObject();
    }

    public void destroyColorGrading(@NonNull ColorGrading colorGrading) {
        assertDestroy(nDestroyColorGrading(getNativeObject(), colorGrading.getNativeObject()));
        colorGrading.clearNativeObject();
    }

    public void destroyTexture(@NonNull Texture texture) {
        assertDestroy(nDestroyTexture(getNativeObject(), texture.getNativeObject()));
        texture.clearNativeObject();
    }

    public void destroyRenderTarget(@NonNull RenderTarget target) {
        assertDestroy(nDestroyRenderTarget(getNativeObject(), target.getNativeObject()));
        target.clearNativeObject();
    }

    public void destroyEntity(@Entity int entity) {
        nDestroyEntity(getNativeObject(), entity);
    }

    public void destroy(@NonNull SwapChain swapChain) { destroySwapChain(swapChain); }
    public void destroy(@NonNull View view) { destroyView(view); }
    public void destroy(@NonNull Renderer renderer) { destroyRenderer(renderer); }
    public void destroy(@NonNull Scene scene) { destroyScene(scene); }
    public void destroy(@NonNull Stream stream) { destroyStream(stream); }
    public void destroy(@NonNull Fence fence) { destroyFence(fence); }
    public void destroy(@NonNull FramePacer framePacer) { destroyFramePacer(framePacer); }
    public void destroy(@NonNull BufferObject bufferObject) { destroyBufferObject(bufferObject); }
    public void destroy(@NonNull IndexBuffer indexBuffer) { destroyIndexBuffer(indexBuffer); }
    public void destroy(@NonNull InstanceBuffer instanceBuffer) { destroyInstanceBuffer(instanceBuffer); }
    public void destroy(@NonNull VertexBuffer vertexBuffer) { destroyVertexBuffer(vertexBuffer); }
    public void destroy(@NonNull SkinningBuffer skinningBuffer) { destroySkinningBuffer(skinningBuffer); }
    public void destroy(@NonNull MorphTargetBuffer morphTargetBuffer) { destroyMorphTargetBuffer(morphTargetBuffer); }
    public void destroy(@NonNull IndirectLight ibl) { destroyIndirectLight(ibl); }
    public void destroy(@NonNull Material material) { destroyMaterial(material); }
    public void destroy(@NonNull MaterialInstance materialInstance) { destroyMaterialInstance(materialInstance); }
    public void destroy(@NonNull Skybox skybox) { destroySkybox(skybox); }
    public void destroy(@NonNull ColorGrading colorGrading) { destroyColorGrading(colorGrading); }
    public void destroy(@NonNull Texture texture) { destroyTexture(texture); }
    public void destroy(@NonNull RenderTarget target) { destroyRenderTarget(target); }
    public void destroy(@Entity int entity) { destroyEntity(entity); }

    private static void assertDestroy(boolean success) {
        if (!success) {
            throw new IllegalStateException("Object could not be destroyed (or already destroyed)");
        }
    }"""

# Private static native declarations corresponding to `ENGINE_METHODS_JAVA` and `ENGINE_BUILDER_JAVA`.
ENGINE_NATIVE_DECLS_JAVA: List[str] = [
    "    private static native void nDestroyEngine(long nativeEngine);",
    "    private static native long nCreateSwapChain(long nativeEngine, Object nativeWindow, long flags);",
    "    private static native long nCreateSwapChainHeadless(long nativeEngine, int width, int height, long flags);",
    "    private static native long nCreateSwapChainFromRawPointer(long nativeEngine, long pointer, long flags);",
    "    private static native boolean nDestroyBufferObject(long nativeEngine, long nativeBufferObject);",
    "    private static native boolean nDestroyColorGrading(long nativeEngine, long nativeColorGrading);",
    "    private static native boolean nDestroyFence(long nativeEngine, long nativeFence);",
    "    private static native boolean nDestroyFramePacer(long nativeEngine, long nativeFramePacer);",
    "    private static native boolean nDestroyIndexBuffer(long nativeEngine, long nativeIndexBuffer);",
    "    private static native boolean nDestroyIndirectLight(long nativeEngine, long nativeIndirectLight);",
    "    private static native boolean nDestroyInstanceBuffer(long nativeEngine, long nativeInstanceBuffer);",
    "    private static native boolean nDestroyMaterial(long nativeEngine, long nativeMaterial);",
    "    private static native boolean nDestroyMaterialInstance(long nativeEngine, long nativeMaterialInstance);",
    "    private static native boolean nDestroyMorphTargetBuffer(long nativeEngine, long nativeMorphTargetBuffer);",
    "    private static native boolean nDestroyRenderTarget(long nativeEngine, long nativeTarget);",
    "    private static native boolean nDestroyRenderer(long nativeEngine, long nativeRenderer);",
    "    private static native boolean nDestroyScene(long nativeEngine, long nativeScene);",
    "    private static native boolean nDestroySkinningBuffer(long nativeEngine, long nativeSkinningBuffer);",
    "    private static native boolean nDestroySkybox(long nativeEngine, long nativeSkybox);",
    "    private static native boolean nDestroyStream(long nativeEngine, long nativeStream);",
    "    private static native boolean nDestroySwapChain(long nativeEngine, long nativeSwapChain);",
    "    private static native boolean nDestroyTexture(long nativeEngine, long nativeTexture);",
    "    private static native boolean nDestroyVertexBuffer(long nativeEngine, long nativeVertexBuffer);",
    "    private static native boolean nDestroyView(long nativeEngine, long nativeView);",
    "    private static native void nDestroyEntity(long nativeEngine, int entity);",
    "    private static native long nCreateBuilder();",
    "    private static native void nDestroyBuilder(long nativeBuilder);",
    "    private static native void nSetBuilderBackend(long nativeBuilder, long backend);",
    "    private static native void nSetBuilderConfig(long nativeBuilder, long commandBufferSizeMB,",
    "            long perRenderPassArenaSizeMB, long driverHandleArenaSizeMB,",
    "            long minCommandBufferSizeMB, long perFrameCommandsSizeMB, long jobSystemThreadCount,",
    "            boolean disableParallelShaderCompile, int stereoscopicType, long stereoscopicEyeCount,",
    "            long resourceAllocatorCacheSizeMB, long resourceAllocatorCacheMaxAge,",
    "            boolean disableHandleUseAfterFreeCheck,",
    "            int preferredShaderLanguage,",
    "            boolean forceGLES2Context, boolean assertNativeWindowIsValid,",
    "            int gpuContextPriority,",
    "            long sharedUboInitialSizeInBytes,",
    "            boolean enableMultipleDirectionalLights);",
    "    private static native void nSetBuilderFeatureLevel(long nativeBuilder, int ordinal);",
    "    private static native void nSetBuilderSharedContext(long nativeBuilder, long sharedContext);",
    "    private static native void nSetBuilderPaused(long nativeBuilder, boolean paused);",
    "    private static native void nSetBuilderFeature(long nativeBuilder, String name, boolean value);",
    "    private static native void nSetBuilderColorGrading(long nativeBuilder, long nativeColorGradingBuilder);",
    "    private static native long nBuilderBuild(long nativeBuilder);",
]

# Handcrafted `Engine.Builder` inner class because `Engine.Builder` unpacks the 18-field
# `Engine.Config` POJO into `nSetBuilderConfig(...)`, validates platform `sharedContext`
# handles via `Platform.get().validateSharedContext(...)`, and retains `mConfig` on the
# constructed `Engine` instance.
ENGINE_BUILDER_JAVA = """    /**
     * Use <code>Builder</code> to construct an <code>Engine</code> object instance.
     */
    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"})
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;
        private Config mConfig;

        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        /**
         * Sets the {@link Backend} for the Engine.
         *
         * @param backend Driver backend to use
         * @return A reference to this Builder for chaining calls.
         */
        public Builder backend(Backend backend) {
            nSetBuilderBackend(mNativeBuilder, backend.ordinal());
            return this;
        }

        /**
         * Sets a sharedContext for the Engine.
         *
         * @param sharedContext  A platform-dependant OpenGL context used as a shared context
         *                       when creating filament's internal context. On Android this parameter
         *                       <b>must be</b> an instance of {@link android.opengl.EGLContext}.
         * @return A reference to this Builder for chaining calls.
         */
        public Builder sharedContext(Object sharedContext) {
            if (Platform.get().validateSharedContext(sharedContext)) {
                nSetBuilderSharedContext(mNativeBuilder,
                        Platform.get().getSharedContextNativeHandle(sharedContext));
                return this;
            }
            throw new IllegalArgumentException("Invalid shared context " + sharedContext);
        }

        /**
         * Configure the Engine with custom parameters.
         *
         * @param config A {@link Config} object
         * @return A reference to this Builder for chaining calls.
         */
        public Builder config(Config config) {
            mConfig = config;
            nSetBuilderConfig(mNativeBuilder, config.commandBufferSizeMB,
                    config.perRenderPassArenaSizeMB, config.driverHandleArenaSizeMB,
                    config.minCommandBufferSizeMB, config.perFrameCommandsSizeMB,
                    config.jobSystemThreadCount, config.disableParallelShaderCompile,
                    config.stereoscopicType.ordinal(), config.stereoscopicEyeCount,
                    config.resourceAllocatorCacheSizeMB, config.resourceAllocatorCacheMaxAge,
                    config.disableHandleUseAfterFreeCheck,
                    config.preferredShaderLanguage.ordinal(),
                    config.forceGLES2Context, config.assertNativeWindowIsValid,
                    config.gpuContextPriority.ordinal(),
                    config.sharedUboInitialSizeInBytes,
                    config.enableMultipleDirectionalLights);
            return this;
        }

        /**
         * Sets the initial featureLevel for the Engine.
         *
         * @param featureLevel The feature level at which initialize Filament.
         * @return A reference to this Builder for chaining calls.
         */
        public Builder featureLevel(FeatureLevel featureLevel) {
            nSetBuilderFeatureLevel(mNativeBuilder, featureLevel.ordinal());
            return this;
        }

        /**
         * Sets the initial paused state of the rendering thread.
         *
         * @param paused Whether to start the rendering thread paused.
         * @return A reference to this Builder for chaining calls.
         */
        public Builder paused(boolean paused) {
            nSetBuilderPaused(mNativeBuilder, paused);
            return this;
        }

        /**
         * Set a feature flag value. This is the only way to set constant feature flags.
         * @param name feature name
         * @param value true to enable, false to disable
         * @return A reference to this Builder for chaining calls.
         */
        public Builder feature(@NonNull String name, boolean value) {
            nSetBuilderFeature(mNativeBuilder, name, value);
            return this;
        }

        /**
         * Sets the builder used to create the default ColorGrading object.
         * @param colorGrading Builder used to create the default color grading.
         * @return A reference to this Builder for chaining calls.
         */
        @NonNull
        public Builder colorGrading(@NonNull ColorGrading.Builder colorGrading) {
            nSetBuilderColorGrading(mNativeBuilder, colorGrading.getNativeBuilder());
            return this;
        }

        /**
         * Creates an instance of Engine
         *
         * @return A newly created <code>Engine</code>, or <code>null</code> if the GPU driver couldn't
         *         be initialized, for instance if it doesn't support the right version of OpenGL or
         *         OpenGL ES.
         *
         * @throws Error if there isn't enough memory to allocate the command buffer.
         */
        public Engine build() {
            long nativeEngine = nBuilderBuild(mNativeBuilder);
            if (nativeEngine == 0) throw new IllegalStateException("Couldn't create Engine");
            return new Engine(nativeEngine, mConfig);
        }

        private static class BuilderFinalizer {
            private final long mNativeObject;

            BuilderFinalizer(long nativeObject) {
                mNativeObject = nativeObject;
            }

            @Override
            public void finalize() {
                try {
                    super.finalize();
                } catch (Throwable t) { // Ignore
                } finally {
                    nDestroyBuilder(mNativeObject);
                }
            }
        }
    }"""


def get_engine_jni_methods(jni_package: str) -> str:
    """Return JNI C++ implementations for `Engine` lifecycle, `SwapChain`, and `Builder` methods.

    Synthesizes:
    - `nDestroyEngine` and `nDestroy<Resource>` bridges wrapped in `wrapJni` so any C++
      precondition panic on the calling thread is converted into a Java exception.
    - `nCreateSwapChain` (resolving platform `ANativeWindow` via `getNativeWindow`),
      `nCreateSwapChainHeadless`, and `nCreateSwapChainFromRawPointer`.
    - `Engine::Builder` JNI bridges (`nCreateBuilder`, `nDestroyBuilder`, `nSetBuilderConfig`,
      `nBuilderBuild` wrapped in `wrapJniBackend`, etc.).

    Args:
        jni_package: Mangled JNI package prefix (e.g. `'com_google_android_filament'`).

    Returns:
        Formatted C++ source block containing all specialized `Engine` JNI functions.
    """
    # Generate typed Engine::destroy(T*) bridges for all 20 Engine-managed resource types.
    types = [
        "BufferObject", "ColorGrading", "Fence", "FramePacer",
        "IndexBuffer", "IndirectLight", "InstanceBuffer", "Material",
        "MaterialInstance", "MorphTargetBuffer", "RenderTarget",
        "Renderer", "Scene", "SkinningBuffer", "Skybox", "Stream",
        "SwapChain", "Texture", "VertexBuffer", "View",
    ]
    destroy_funcs = [
        f"""extern "C" JNIEXPORT jboolean JNICALL
Java_{jni_package}_Engine_nDestroy{t}(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeObject) {{
    Engine* engine = (Engine*) nativeEngine;
    {t}* obj = ({t}*) nativeObject;
    return wrapJni<jboolean>(env, [=]() {{
        return engine->destroy(obj);
    }});
}}"""
        for t in types
    ]
    destroy_code = "\n\n".join(destroy_funcs)

    return f"""extern "C" JNIEXPORT void JNICALL
Java_{jni_package}_Engine_nDestroyEngine(JNIEnv *env, jclass, jlong nativeEngine) {{
    wrapJni(env, [=]() {{
        Engine* engine = (Engine*) nativeEngine;
        Engine::destroy(&engine);
    }});
}}

extern "C" {{
extern void* getNativeWindow(JNIEnv* env, jclass, jobject surface);
}}

extern "C" JNIEXPORT jlong JNICALL
Java_{jni_package}_Engine_nCreateSwapChain(JNIEnv* env,
        jclass klass, jlong nativeEngine, jobject surface, jlong flags) {{
    Engine* engine = (Engine*) nativeEngine;
    void* win = getNativeWindow(env, klass, surface);
    return (jlong) engine->createSwapChain(win, (uint64_t) flags);
}}

extern "C" JNIEXPORT jlong JNICALL
Java_{jni_package}_Engine_nCreateSwapChainHeadless(JNIEnv*,
        jclass, jlong nativeEngine, jint width, jint height, jlong flags) {{
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) engine->createSwapChain(width, height, (uint64_t) flags);
}}

extern "C" JNIEXPORT jlong JNICALL
Java_{jni_package}_Engine_nCreateSwapChainFromRawPointer(JNIEnv*,
        jclass, jlong nativeEngine, jlong pointer, jlong flags) {{
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) engine->createSwapChain((void*) pointer, (uint64_t) flags);
}}


extern "C" JNIEXPORT void JNICALL
Java_{jni_package}_Engine_nDestroyEntity(JNIEnv*, jclass,
        jlong nativeEngine, jint entity_) {{
    Engine* engine = (Engine*) nativeEngine;
    Entity& entity = *reinterpret_cast<Entity*>(&entity_);
    engine->destroy(entity);
}}

{destroy_code}

extern "C" JNIEXPORT jlong JNICALL
Java_{jni_package}_Engine_nCreateBuilder(JNIEnv*, jclass) {{
    return (jlong) new Engine::Builder{{}};
}}

extern "C" JNIEXPORT void JNICALL
Java_{jni_package}_Engine_nDestroyBuilder(JNIEnv*, jclass, jlong nativeBuilder) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    delete builder;
}}

extern "C" JNIEXPORT void JNICALL
Java_{jni_package}_Engine_nSetBuilderBackend(JNIEnv*, jclass,
        jlong nativeBuilder, jlong backend) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    builder->backend((Engine::Backend) backend);
}}

extern "C" JNIEXPORT void JNICALL
Java_{jni_package}_Engine_nSetBuilderConfig(JNIEnv*, jclass,
        jlong nativeBuilder, jlong commandBufferSizeMB, jlong perRenderPassArenaSizeMB,
        jlong driverHandleArenaSizeMB, jlong minCommandBufferSizeMB, jlong perFrameCommandsSizeMB,
        jlong jobSystemThreadCount, jboolean disableParallelShaderCompile,
        jint stereoscopicType, jlong stereoscopicEyeCount,
        jlong resourceAllocatorCacheSizeMB, jlong resourceAllocatorCacheMaxAge,
        jboolean disableHandleUseAfterFreeCheck,
        jint preferredShaderLanguage,
        jboolean forceGLES2Context, jboolean assertNativeWindowIsValid,
        jint gpuContextPriority,
        jlong sharedUboInitialSizeInBytes,
        jboolean enableMultipleDirectionalLights) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    Engine::Config config = {{
            .commandBufferSizeMB = (uint32_t) commandBufferSizeMB,
            .perRenderPassArenaSizeMB = (uint32_t) perRenderPassArenaSizeMB,
            .driverHandleArenaSizeMB = (uint32_t) driverHandleArenaSizeMB,
            .minCommandBufferSizeMB = (uint32_t) minCommandBufferSizeMB,
            .perFrameCommandsSizeMB = (uint32_t) perFrameCommandsSizeMB,
            .jobSystemThreadCount = (uint32_t) jobSystemThreadCount,
            .disableParallelShaderCompile = (bool) disableParallelShaderCompile,
            .stereoscopicType = (Engine::StereoscopicType) stereoscopicType,
            .stereoscopicEyeCount = (uint8_t) stereoscopicEyeCount,
            .resourceAllocatorCacheSizeMB = (uint32_t) resourceAllocatorCacheSizeMB,
            .resourceAllocatorCacheMaxAge = (uint8_t) resourceAllocatorCacheMaxAge,
            .disableHandleUseAfterFreeCheck = (bool) disableHandleUseAfterFreeCheck,
            .preferredShaderLanguage = (Engine::Config::ShaderLanguage) preferredShaderLanguage,
            .forceGLES2Context = (bool) forceGLES2Context,
            .assertNativeWindowIsValid = (bool) assertNativeWindowIsValid,
            .gpuContextPriority = (Engine::GpuContextPriority) gpuContextPriority,
            .sharedUboInitialSizeInBytes = (uint32_t) sharedUboInitialSizeInBytes,
            .enableMultipleDirectionalLights = (bool) enableMultipleDirectionalLights,
    }};
    builder->config(&config);
}}

extern "C" JNIEXPORT void JNICALL
Java_{jni_package}_Engine_nSetBuilderFeatureLevel(JNIEnv*, jclass,
        jlong nativeBuilder, jint ordinal) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    builder->featureLevel((Engine::FeatureLevel) ordinal);
}}

extern "C" JNIEXPORT void JNICALL
Java_{jni_package}_Engine_nSetBuilderSharedContext(JNIEnv*, jclass,
        jlong nativeBuilder, jlong sharedContext) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    builder->sharedContext((void*) sharedContext);
}}

extern "C" JNIEXPORT void JNICALL
Java_{jni_package}_Engine_nSetBuilderPaused(JNIEnv*, jclass,
        jlong nativeBuilder, jboolean paused) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    builder->paused((bool) paused);
}}

extern "C" JNIEXPORT void JNICALL
Java_{jni_package}_Engine_nSetBuilderFeature(JNIEnv *env, jclass,
        jlong nativeBuilder, jstring name_, jboolean value) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    const char *name = env->GetStringUTFChars(name_, 0);
    builder->feature(name, (bool) value);
    env->ReleaseStringUTFChars(name_, name);
}}

extern "C" JNIEXPORT void JNICALL
Java_{jni_package}_Engine_nSetBuilderColorGrading(JNIEnv *env, jclass,
        jlong nativeBuilder, jlong nativeColorGradingBuilder) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    ColorGrading::Builder const* colorGradingBuilder = (ColorGrading::Builder const*) nativeColorGradingBuilder;
    builder->colorGrading(*colorGradingBuilder);
}}

extern "C" JNIEXPORT jlong JNICALL
Java_{jni_package}_Engine_nBuilderBuild(JNIEnv *env, jclass, jlong nativeBuilder) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    return wrapJniBackend<jlong>(env, [=]() {{
        return (jlong) builder->build();
    }});
}}"""

