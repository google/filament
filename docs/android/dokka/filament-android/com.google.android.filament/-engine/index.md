//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)

# Engine

open class [Engine](index.md)

Engine is filament's main entry-point. 

An Engine instance main function is to keep track of all resources created by the user and manage the rendering thread as well as the hardware renderer.

To use filament, an Engine instance must be created first:

```kotlin

#include <filament/Engine.h>
using namespace filament;

Engine* engine = Engine::create();

```

Engine essentially represents (or is associated to) a hardware context (e.g. an OpenGL ES context).

Rendering typically happens in an operating system's window (which can be full screen), such window is managed by a filament.Renderer.

A typical filament render loop looks like this:

```kotlin

#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/View.h>
using namespace filament;

Engine* engine       = Engine::create();
SwapChain* swapChain = engine->createSwapChain(nativeWindow);
Renderer* renderer   = engine->createRenderer();
Scene* scene         = engine->createScene();
View* view           = engine->createView();

view->setScene(scene);

do {
    // typically we wait for VSYNC and user input events
    if (renderer->beginFrame(swapChain)) {
        renderer->render(view);
        renderer->endFrame();
    }
} while (!quit);

engine->destroy(view);
engine->destroy(scene);
engine->destroy(renderer);
engine->destroy(swapChain);
Engine::destroy(&engine); // clears engine*

```

# Resource Tracking

Each Engine instance keeps track of all objects created by the user, such as vertex and index buffers, lights, cameras, etc... The user is expected to free those resources, however, leaked resources are freed when the engine instance is destroyed and a warning is emitted in the console.

# Thread safety

An Engine instance is not thread-safe. The implementation makes no attempt to synchronize calls to an Engine instance methods. If multi-threading is needed, synchronization must be external.

# Multi-threading

When created, the Engine instance starts a render thread as well as multiple worker threads, these threads have an elevated priority appropriate for rendering, based on the platform's best practices. The number of worker threads depends on the platform and is automatically chosen for best performance.

On platforms with asymmetric cores (e.g. ARM's Big.Little), Engine makes some educated guesses as to which cores to use for the render thread and worker threads. For example, it'll try to keep an OpenGL ES thread on a Big core.

# Swap Chains

A swap chain represents an Operating System's *native* renderable surface. Typically it's a window or a view. Because a SwapChain is initialized from a native object, it is given to filament as a `void*`, which must be of the proper type for each platform filament is running on.

#### See also

| |
|---|
| [SwapChain](../-swap-chain/index.md) |
| [Renderer](../-renderer/index.md) |

## Types

| Name | Summary |
|---|---|
| [AsynchronousMode](-asynchronous-mode/index.md) | [main]<br>enum [AsynchronousMode](-asynchronous-mode/index.md) |
| [Backend](-backend/index.md) | [main]<br>enum [Backend](-backend/index.md)<br>Selects which driver a particular Engine should use. |
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md)<br>Use `Builder` to construct an `Engine` object instance. |
| [Config](-config/index.md) | [main]<br>open class [Config](-config/index.md)<br>Config is used to define the memory footprint used by the engine, such as the command buffer size. |
| [FeatureLevel](-feature-level/index.md) | [main]<br>enum [FeatureLevel](-feature-level/index.md)<br>Defines the backend's feature levels. |
| [FeatureState](-feature-state/index.md) | [main]<br>enum [FeatureState](-feature-state/index.md)<br>Three-state feature state. |
| [GpuContextPriority](-gpu-context-priority/index.md) | [main]<br>enum [GpuContextPriority](-gpu-context-priority/index.md)<br>This controls the priority level for GPU work scheduling, which helps prioritize the submitted GPU work and enables preemption. |
| [StereoscopicType](-stereoscopic-type/index.md) | [main]<br>enum [StereoscopicType](-stereoscopic-type/index.md) |

## Functions

| Name | Summary |
|---|---|
| [cancelAsyncCall](cancel-async-call.md) | [main]<br>open fun [cancelAsyncCall](cancel-async-call.md)(id: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Cancel the pending asynchronous call pointed to by `id`, which is retrieved whenever you invoke a non-blocking version of method on an object, such as `Texture::setImageAsync` or `BufferObject::setBufferAsync`. |
| [compile](compile.md) | [main]<br>open fun [compile](compile.md)(priority: [Material.CompilerPriorityQueue](../-material/-compiler-priority-queue/index.md), material: [Material](../-material/index.md), view: [View](../-view/index.md), shadowReceiver: [Engine.FeatureState](-feature-state/index.md), skinning: [Engine.FeatureState](-feature-state/index.md), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))<br>Asynchronously ensures that the variants of the specified Material needed to render it in the provided View are compiled. |
| [create](create.md) | [main]<br>open fun [create](create.md)(): [Engine](index.md)<br>open fun [create](create.md)(backend: [Engine.Backend](-backend/index.md)): [Engine](index.md)<br>open fun [create](create.md)(sharedContext: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html)): [Engine](index.md) |
| [createCamera](create-camera.md) | [main]<br>open fun [createCamera](create-camera.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Camera](../-camera/index.md)<br>Creates a Camera component. |
| [createFence](create-fence.md) | [main]<br>open fun [createFence](create-fence.md)(): [Fence](../-fence/index.md)<br>Creates a Fence. |
| [createRenderer](create-renderer.md) | [main]<br>open fun [createRenderer](create-renderer.md)(): [Renderer](../-renderer/index.md)<br>Creates a renderer associated to this engine. |
| [createScene](create-scene.md) | [main]<br>open fun [createScene](create-scene.md)(): [Scene](../-scene/index.md)<br>Creates a Scene. |
| [createSwapChain](create-swap-chain.md) | [main]<br>open fun [createSwapChain](create-swap-chain.md)(surface: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html)): [SwapChain](../-swap-chain/index.md)<br>open fun [createSwapChain](create-swap-chain.md)(width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [SwapChain](../-swap-chain/index.md)<br>open fun [createSwapChain](create-swap-chain.md)(surface: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), flags: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [SwapChain](../-swap-chain/index.md)<br>open fun [createSwapChain](create-swap-chain.md)(width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), flags: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [SwapChain](../-swap-chain/index.md) |
| [createSwapChainFromNativeSurface](create-swap-chain-from-native-surface.md) | [main]<br>open fun [createSwapChainFromNativeSurface](create-swap-chain-from-native-surface.md)(surface: [NativeSurface](../-native-surface/index.md), flags: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [SwapChain](../-swap-chain/index.md) |
| [createView](create-view.md) | [main]<br>open fun [createView](create-view.md)(): [View](../-view/index.md)<br>Creates a View. |
| [destroy](destroy.md) | [main]<br>open fun [destroy](destroy.md)()<br>open fun [destroy](destroy.md)(bufferObject: [BufferObject](../-buffer-object/index.md))<br>open fun [destroy](destroy.md)(colorGrading: [ColorGrading](../-color-grading/index.md))<br>open fun [destroy](destroy.md)(fence: [Fence](../-fence/index.md))<br>open fun [destroy](destroy.md)(framePacer: [FramePacer](../-frame-pacer/index.md))<br>open fun [destroy](destroy.md)(indexBuffer: [IndexBuffer](../-index-buffer/index.md))<br>open fun [destroy](destroy.md)(ibl: [IndirectLight](../-indirect-light/index.md))<br>open fun [destroy](destroy.md)(instanceBuffer: [InstanceBuffer](../-instance-buffer/index.md))<br>open fun [destroy](destroy.md)(material: [Material](../-material/index.md))<br>open fun [destroy](destroy.md)(materialInstance: [MaterialInstance](../-material-instance/index.md))<br>open fun [destroy](destroy.md)(morphTargetBuffer: [MorphTargetBuffer](../-morph-target-buffer/index.md))<br>open fun [destroy](destroy.md)(target: [RenderTarget](../-render-target/index.md))<br>open fun [destroy](destroy.md)(renderer: [Renderer](../-renderer/index.md))<br>open fun [destroy](destroy.md)(scene: [Scene](../-scene/index.md))<br>open fun [destroy](destroy.md)(skinningBuffer: [SkinningBuffer](../-skinning-buffer/index.md))<br>open fun [destroy](destroy.md)(skybox: [Skybox](../-skybox/index.md))<br>open fun [destroy](destroy.md)(stream: [Stream](../-stream/index.md))<br>open fun [destroy](destroy.md)(swapChain: [SwapChain](../-swap-chain/index.md))<br>open fun [destroy](destroy.md)(texture: [Texture](../-texture/index.md))<br>open fun [destroy](destroy.md)(vertexBuffer: [VertexBuffer](../-vertex-buffer/index.md))<br>open fun [destroy](destroy.md)(view: [View](../-view/index.md))<br>open fun [destroy](destroy.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)) |
| [destroyBufferObject](destroy-buffer-object.md) | [main]<br>open fun [destroyBufferObject](destroy-buffer-object.md)(bufferObject: [BufferObject](../-buffer-object/index.md)) |
| [destroyCameraComponent](destroy-camera-component.md) | [main]<br>open fun [destroyCameraComponent](destroy-camera-component.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Destroys the Camera component associated with the given entity. |
| [destroyColorGrading](destroy-color-grading.md) | [main]<br>open fun [destroyColorGrading](destroy-color-grading.md)(colorGrading: [ColorGrading](../-color-grading/index.md)) |
| [destroyEntity](destroy-entity.md) | [main]<br>open fun [destroyEntity](destroy-entity.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)) |
| [destroyFence](destroy-fence.md) | [main]<br>open fun [destroyFence](destroy-fence.md)(fence: [Fence](../-fence/index.md)) |
| [destroyFramePacer](destroy-frame-pacer.md) | [main]<br>open fun [destroyFramePacer](destroy-frame-pacer.md)(framePacer: [FramePacer](../-frame-pacer/index.md)) |
| [destroyIndexBuffer](destroy-index-buffer.md) | [main]<br>open fun [destroyIndexBuffer](destroy-index-buffer.md)(indexBuffer: [IndexBuffer](../-index-buffer/index.md)) |
| [destroyIndirectLight](destroy-indirect-light.md) | [main]<br>open fun [destroyIndirectLight](destroy-indirect-light.md)(ibl: [IndirectLight](../-indirect-light/index.md)) |
| [destroyInstanceBuffer](destroy-instance-buffer.md) | [main]<br>open fun [destroyInstanceBuffer](destroy-instance-buffer.md)(instanceBuffer: [InstanceBuffer](../-instance-buffer/index.md)) |
| [destroyMaterial](destroy-material.md) | [main]<br>open fun [destroyMaterial](destroy-material.md)(material: [Material](../-material/index.md)) |
| [destroyMaterialInstance](destroy-material-instance.md) | [main]<br>open fun [destroyMaterialInstance](destroy-material-instance.md)(materialInstance: [MaterialInstance](../-material-instance/index.md)) |
| [destroyMorphTargetBuffer](destroy-morph-target-buffer.md) | [main]<br>open fun [destroyMorphTargetBuffer](destroy-morph-target-buffer.md)(morphTargetBuffer: [MorphTargetBuffer](../-morph-target-buffer/index.md)) |
| [destroyRenderer](destroy-renderer.md) | [main]<br>open fun [destroyRenderer](destroy-renderer.md)(renderer: [Renderer](../-renderer/index.md)) |
| [destroyRenderTarget](destroy-render-target.md) | [main]<br>open fun [destroyRenderTarget](destroy-render-target.md)(target: [RenderTarget](../-render-target/index.md)) |
| [destroyScene](destroy-scene.md) | [main]<br>open fun [destroyScene](destroy-scene.md)(scene: [Scene](../-scene/index.md)) |
| [destroySkinningBuffer](destroy-skinning-buffer.md) | [main]<br>open fun [destroySkinningBuffer](destroy-skinning-buffer.md)(skinningBuffer: [SkinningBuffer](../-skinning-buffer/index.md)) |
| [destroySkybox](destroy-skybox.md) | [main]<br>open fun [destroySkybox](destroy-skybox.md)(skybox: [Skybox](../-skybox/index.md)) |
| [destroyStream](destroy-stream.md) | [main]<br>open fun [destroyStream](destroy-stream.md)(stream: [Stream](../-stream/index.md)) |
| [destroySwapChain](destroy-swap-chain.md) | [main]<br>open fun [destroySwapChain](destroy-swap-chain.md)(swapChain: [SwapChain](../-swap-chain/index.md)) |
| [destroyTexture](destroy-texture.md) | [main]<br>open fun [destroyTexture](destroy-texture.md)(texture: [Texture](../-texture/index.md)) |
| [destroyVertexBuffer](destroy-vertex-buffer.md) | [main]<br>open fun [destroyVertexBuffer](destroy-vertex-buffer.md)(vertexBuffer: [VertexBuffer](../-vertex-buffer/index.md)) |
| [destroyView](destroy-view.md) | [main]<br>open fun [destroyView](destroy-view.md)(view: [View](../-view/index.md)) |
| [enableAccurateTranslations](enable-accurate-translations.md) | [main]<br>open fun [enableAccurateTranslations](enable-accurate-translations.md)()<br>Helper to enable accurate translations. |
| [flush](flush.md) | [main]<br>open fun [flush](flush.md)()<br>Kicks the hardware thread (e.g. |
| [flushAndWait](flush-and-wait.md) | [main]<br>open fun [flushAndWait](flush-and-wait.md)()<br>open fun [flushAndWait](flush-and-wait.md)(timeout: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Kicks the hardware thread (e.g. |
| [getActiveFeatureLevel](get-active-feature-level.md) | [main]<br>open fun [getActiveFeatureLevel](get-active-feature-level.md)(): [Engine.FeatureLevel](-feature-level/index.md)<br>Returns the currently active feature level. |
| [getBackend](get-backend.md) | [main]<br>open fun [getBackend](get-backend.md)(): [Engine.Backend](-backend/index.md)<br>Returns the resolved backend. |
| [getBufferObjectCount](get-buffer-object-count.md) | [main]<br>open fun [getBufferObjectCount](get-buffer-object-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Retrieve the count of each resource tracked by Engine. |
| [getCameraComponent](get-camera-component.md) | [main]<br>open fun [getCameraComponent](get-camera-component.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Camera](../-camera/index.md)<br>Returns the Camera component of the given entity. |
| [getColorGradingCount](get-color-grading-count.md) | [main]<br>open fun [getColorGradingCount](get-color-grading-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getConfig](get-config.md) | [main]<br>open fun [getConfig](get-config.md)(): [Engine.Config](-config/index.md)<br>Retrieves the configuration settings of this Engine. |
| [getEntityManager](get-entity-manager.md) | [main]<br>open fun [getEntityManager](get-entity-manager.md)(): [EntityManager](../-entity-manager/index.md) |
| [getFeatureFlag](get-feature-flag.md) | [main]<br>open fun [getFeatureFlag](get-feature-flag.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Retrieves the value of any feature flag. |
| [getIndexBufferCount](get-index-buffer-count.md) | [main]<br>open fun [getIndexBufferCount](get-index-buffer-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getIndirectLightCount](get-indirect-light-count.md) | [main]<br>open fun [getIndirectLightCount](get-indirect-light-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getInstanceBufferCount](get-instance-buffer-count.md) | [main]<br>open fun [getInstanceBufferCount](get-instance-buffer-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getLightManager](get-light-manager.md) | [main]<br>open fun [getLightManager](get-light-manager.md)(): [LightManager](../-light-manager/index.md) |
| [getMaterialCount](get-material-count.md) | [main]<br>open fun [getMaterialCount](get-material-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getMaxAutomaticInstances](get-max-automatic-instances.md) | [main]<br>open fun [getMaxAutomaticInstances](get-max-automatic-instances.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Queries the maximum number of GPU instances that Filament creates when automatic instancing is enabled. |
| [getMaxStereoscopicEyes](get-max-stereoscopic-eyes.md) | [main]<br>open fun [getMaxStereoscopicEyes](get-max-stereoscopic-eyes.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the maximum number of stereoscopic eyes supported by Filament. |
| [getMorphTargetBufferCount](get-morph-target-buffer-count.md) | [main]<br>open fun [getMorphTargetBufferCount](get-morph-target-buffer-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getRenderableManager](get-renderable-manager.md) | [main]<br>open fun [getRenderableManager](get-renderable-manager.md)(): [RenderableManager](../-renderable-manager/index.md) |
| [getRenderTargetCount](get-render-target-count.md) | [main]<br>open fun [getRenderTargetCount](get-render-target-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getSceneCount](get-scene-count.md) | [main]<br>open fun [getSceneCount](get-scene-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getSkinningBufferCount](get-skinning-buffer-count.md) | [main]<br>open fun [getSkinningBufferCount](get-skinning-buffer-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getSkyboxeCount](get-skyboxe-count.md) | [main]<br>open fun [getSkyboxeCount](get-skyboxe-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getSteadyClockTimeNano](get-steady-clock-time-nano.md) | [main]<br>open fun [getSteadyClockTimeNano](get-steady-clock-time-nano.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Get the current time. |
| [getStreamCount](get-stream-count.md) | [main]<br>open fun [getStreamCount](get-stream-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getSupportedFeatureLevel](get-supported-feature-level.md) | [main]<br>open fun [getSupportedFeatureLevel](get-supported-feature-level.md)(): [Engine.FeatureLevel](-feature-level/index.md)<br>Query the feature level supported by the selected backend. |
| [getSwapChainCount](get-swap-chain-count.md) | [main]<br>open fun [getSwapChainCount](get-swap-chain-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getTextureCount](get-texture-count.md) | [main]<br>open fun [getTextureCount](get-texture-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getTransformManager](get-transform-manager.md) | [main]<br>open fun [getTransformManager](get-transform-manager.md)(): [TransformManager](../-transform-manager/index.md) |
| [getVertexBufferCount](get-vertex-buffer-count.md) | [main]<br>open fun [getVertexBufferCount](get-vertex-buffer-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getViewCount](get-view-count.md) | [main]<br>open fun [getViewCount](get-view-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [hasFeatureFlag](has-feature-flag.md) | [main]<br>open fun [hasFeatureFlag](has-feature-flag.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Check if a feature flag exists |
| [hasUnrecoverableFailure](has-unrecoverable-failure.md) | [main]<br>open fun [hasUnrecoverableFailure](has-unrecoverable-failure.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the engine has encountered an unrecoverable failure. |
| [isAsynchronousModeEnabled](is-asynchronous-mode-enabled.md) | [main]<br>open fun [isAsynchronousModeEnabled](is-asynchronous-mode-enabled.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Checks if the engine is set up for asynchronous operation. |
| [isAutomaticInstancingEnabled](is-automatic-instancing-enabled.md) | [main]<br>open fun [isAutomaticInstancingEnabled](is-automatic-instancing-enabled.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [isPaused](is-paused.md) | [main]<br>open fun [isPaused](is-paused.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Get paused state of rendering thread. |
| [isStereoSupported](is-stereo-supported.md) | [main]<br>open fun [isStereoSupported](is-stereo-supported.md)(stereoscopicType: [Engine.StereoscopicType](-stereoscopic-type/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Queries the device and platform for support of the given stereoscopic type. |
| [setActiveFeatureLevel](set-active-feature-level.md) | [main]<br>open fun [setActiveFeatureLevel](set-active-feature-level.md)(featureLevel: [Engine.FeatureLevel](-feature-level/index.md)): [Engine.FeatureLevel](-feature-level/index.md)<br>Activate all features of a given feature level. |
| [setAutomaticInstancingEnabled](set-automatic-instancing-enabled.md) | [main]<br>open fun [setAutomaticInstancingEnabled](set-automatic-instancing-enabled.md)(enable: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))<br>Enables or disables automatic instancing of render primitives. |
| [setFeatureFlag](set-feature-flag.md) | [main]<br>open fun [setFeatureFlag](set-feature-flag.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), value: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Set the value of a non-constant feature flag. |
| [setPaused](set-paused.md) | [main]<br>open fun [setPaused](set-paused.md)(paused: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))<br>Pause or resume rendering thread. |
| [unprotected](unprotected.md) | [main]<br>open fun [unprotected](unprotected.md)()<br>Switch the command queue to unprotected mode. |
