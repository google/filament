//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)

# Engine

open class [Engine](index.md)

Engine is filament's main entry-point. 

 An Engine instance main function is to keep track of all resources created by the user and manage the rendering thread as well as the hardware renderer. 

 To use filament, an Engine instance must be created first: 

```kotlin
import com.google.android.filament.*

Engine engine = Engine.create();

```

 Engine essentially represents (or is associated to) a hardware context (e.g. an OpenGL ES context). 

 Rendering typically happens in an operating system's window (which can be full screen), such window is managed by a [Renderer](../-renderer/index.md). 

 A typical filament render loop looks like this: 

```kotlin
import com.google.android.filament.*

Engine engine        = Engine.create();
SwapChain swapChain  = engine.createSwapChain(nativeWindow);
Renderer renderer    = engine.createRenderer();
Scene scene          = engine.createScene();
View view            = engine.createView();

view.setScene(scene);

do {
    // typically we wait for VSYNC and user input events
    if (renderer.beginFrame(swapChain)) {
        renderer.render(view);
        renderer.endFrame();
    }
} while (!quit);

engine.destroyView(view);
engine.destroyScene(scene);
engine.destroyRenderer(renderer);
engine.destroySwapChain(swapChain);
engine.destroy();

```

# Resource Tracking

 Each `Engine` instance keeps track of all objects created by the user, such as vertex and index buffers, lights, cameras, etc... The user is expected to free those resources, however, leaked resources are freed when the engine instance is destroyed and a warning is emitted in the console. 

# Thread safety

 An `Engine` instance is not thread-safe. The implementation makes no attempt to synchronize calls to an `Engine` instance methods. If multi-threading is needed, synchronization must be external. 

# Multi-threading

 When created, the `Engine` instance starts a render thread as well as multiple worker threads, these threads have an elevated priority appropriate for rendering, based on the platform's best practices. The number of worker threads depends on the platform and is automatically chosen for best performance. 

 On platforms with asymmetric cores (e.g. ARM's Big.Little), `Engine` makes some educated guesses as to which cores to use for the render thread and worker threads. For example, it'll try to keep an OpenGL ES thread on a Big core. 

# Swap Chains

 A swap chain represents an Operating System's **native** renderable surface. Typically it's a window or a view. Because a [SwapChain](../-swap-chain/index.md) is initialized from a native object, it is given to filament as an `Object`, which must be of the proper type for each platform filament is running on. 

#### See also

| |
|---|
| [SwapChain](../-swap-chain/index.md) |
| [Renderer](../-renderer/index.md) |

## Types

| Name | Summary |
|---|---|
| [Backend](-backend/index.md) | [main]<br>enum [Backend](-backend/index.md)<br>Denotes a backend |
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md)<br>Constructs `Engine` objects using a builder pattern. |
| [Config](-config/index.md) | [main]<br>open class [Config](-config/index.md)<br>Parameters for customizing the initialization of [Engine](index.md). |
| [FeatureLevel](-feature-level/index.md) | [main]<br>enum [FeatureLevel](-feature-level/index.md)<br>Defines the backend's feature levels. |
| [FeatureState](-feature-state/index.md) | [main]<br>enum [FeatureState](-feature-state/index.md)<br>Three-state feature state. |
| [GpuContextPriority](-gpu-context-priority/index.md) | [main]<br>enum [GpuContextPriority](-gpu-context-priority/index.md)<br>This controls the priority level for GPU work scheduling, which helps prioritize the submitted GPU work and enables preemption. |
| [StereoscopicType](-stereoscopic-type/index.md) | [main]<br>enum [StereoscopicType](-stereoscopic-type/index.md)<br>The type of technique for stereoscopic rendering. |

## Functions

| Name | Summary |
|---|---|
| [compile](compile.md) | [main]<br>open fun [compile](compile.md)(priority: [Material.CompilerPriorityQueue](../-material/-compiler-priority-queue/index.md), material: [Material](../-material/index.md), view: [View](../-view/index.md), shadowReceiver: [Engine.FeatureState](-feature-state/index.md), skinning: [Engine.FeatureState](-feature-state/index.md), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))<br>Asynchronously ensures that the variants of the specified Material required to render it in the provided View are compiled. |
| [create](create.md) | [main]<br>open fun [create](create.md)(): [Engine](index.md)<br>Creates an instance of Engine using the default [Backend](-backend/index.md) This method is one of the few thread-safe methods.<br>[main]<br>open fun [create](create.md)(backend: [Engine.Backend](-backend/index.md)): [Engine](index.md)<br>Creates an instance of Engine using the specified [Backend](-backend/index.md) This method is one of the few thread-safe methods.<br>[main]<br>open fun [create](create.md)(sharedContext: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html)): [Engine](index.md)<br>Creates an instance of Engine using the [OPENGL](-backend/-o-p-e-n-g-l/index.md) and a shared OpenGL context. |
| [createCamera](create-camera.md) | [main]<br>open fun [createCamera](create-camera.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Camera](../-camera/index.md)<br>Creates and adds a [Camera](../-camera/index.md) component to a given `entity`. |
| [createFence](create-fence.md) | [main]<br>open fun [createFence](create-fence.md)(): [Fence](../-fence/index.md)<br>Creates a [Fence](../-fence/index.md). |
| [createRenderer](create-renderer.md) | [main]<br>open fun [createRenderer](create-renderer.md)(): [Renderer](../-renderer/index.md)<br>Creates a [Renderer](../-renderer/index.md). |
| [createScene](create-scene.md) | [main]<br>open fun [createScene](create-scene.md)(): [Scene](../-scene/index.md)<br>Creates a [Scene](../-scene/index.md). |
| [createSwapChain](create-swap-chain.md) | [main]<br>open fun [createSwapChain](create-swap-chain.md)(surface: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html)): [SwapChain](../-swap-chain/index.md)<br>Creates an opaque [SwapChain](../-swap-chain/index.md) from the given OS native window handle.<br>[main]<br>open fun [createSwapChain](create-swap-chain.md)(surface: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), flags: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [SwapChain](../-swap-chain/index.md)<br>Creates a [SwapChain](../-swap-chain/index.md) from the given OS native window handle.<br>[main]<br>open fun [createSwapChain](create-swap-chain.md)(width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), flags: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [SwapChain](../-swap-chain/index.md)<br>Creates a headless [SwapChain](../-swap-chain/index.md) |
| [createSwapChainFromNativeSurface](create-swap-chain-from-native-surface.md) | [main]<br>open fun [createSwapChainFromNativeSurface](create-swap-chain-from-native-surface.md)(surface: [NativeSurface](../-native-surface/index.md), flags: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [SwapChain](../-swap-chain/index.md)<br>Creates a [SwapChain](../-swap-chain/index.md) from a [NativeSurface](../-native-surface/index.md). |
| [createView](create-view.md) | [main]<br>open fun [createView](create-view.md)(): [View](../-view/index.md)<br>Creates a [View](../-view/index.md). |
| [destroy](destroy.md) | [main]<br>open fun [destroy](destroy.md)()<br>Destroy the `Engine` instance and all associated resources. |
| [destroyCameraComponent](destroy-camera-component.md) | [main]<br>open fun [destroyCameraComponent](destroy-camera-component.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Destroys the [Camera](../-camera/index.md) component associated with the given entity. |
| [destroyColorGrading](destroy-color-grading.md) | [main]<br>open fun [destroyColorGrading](destroy-color-grading.md)(colorGrading: [ColorGrading](../-color-grading/index.md))<br>Destroys a [ColorGrading](../-color-grading/index.md) and frees all its associated resources. |
| [destroyEntity](destroy-entity.md) | [main]<br>open fun [destroyEntity](destroy-entity.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Destroys all Filament-known components from this `entity`. |
| [destroyFence](destroy-fence.md) | [main]<br>open fun [destroyFence](destroy-fence.md)(fence: [Fence](../-fence/index.md))<br>Destroys a [Fence](../-fence/index.md) and frees all its associated resources. |
| [destroyIndexBuffer](destroy-index-buffer.md) | [main]<br>open fun [destroyIndexBuffer](destroy-index-buffer.md)(indexBuffer: [IndexBuffer](../-index-buffer/index.md))<br>Destroys a [IndexBuffer](../-index-buffer/index.md) and frees all its associated resources. |
| [destroyIndirectLight](destroy-indirect-light.md) | [main]<br>open fun [destroyIndirectLight](destroy-indirect-light.md)(ibl: [IndirectLight](../-indirect-light/index.md))<br>Destroys a [IndirectLight](../-indirect-light/index.md) and frees all its associated resources. |
| [destroyMaterial](destroy-material.md) | [main]<br>open fun [destroyMaterial](destroy-material.md)(material: [Material](../-material/index.md))<br>Destroys a [Material](../-material/index.md) and frees all its associated resources. |
| [destroyMaterialInstance](destroy-material-instance.md) | [main]<br>open fun [destroyMaterialInstance](destroy-material-instance.md)(materialInstance: [MaterialInstance](../-material-instance/index.md))<br>Destroys a [MaterialInstance](../-material-instance/index.md) and frees all its associated resources. |
| [destroyMorphTargetBuffer](destroy-morph-target-buffer.md) | [main]<br>open fun [destroyMorphTargetBuffer](destroy-morph-target-buffer.md)(morphTargetBuffer: [MorphTargetBuffer](../-morph-target-buffer/index.md))<br>Destroys a [MorphTargetBuffer](../-morph-target-buffer/index.md) and frees all its associated resources. |
| [destroyRenderer](destroy-renderer.md) | [main]<br>open fun [destroyRenderer](destroy-renderer.md)(renderer: [Renderer](../-renderer/index.md))<br>Destroys a [Renderer](../-renderer/index.md) and frees all its associated resources. |
| [destroyRenderTarget](destroy-render-target.md) | [main]<br>open fun [destroyRenderTarget](destroy-render-target.md)(target: [RenderTarget](../-render-target/index.md))<br>Destroys a [RenderTarget](../-render-target/index.md) and frees all its associated resources. |
| [destroyScene](destroy-scene.md) | [main]<br>open fun [destroyScene](destroy-scene.md)(scene: [Scene](../-scene/index.md))<br>Destroys a [Scene](../-scene/index.md) and frees all its associated resources. |
| [destroySkinningBuffer](destroy-skinning-buffer.md) | [main]<br>open fun [destroySkinningBuffer](destroy-skinning-buffer.md)(skinningBuffer: [SkinningBuffer](../-skinning-buffer/index.md))<br>Destroys a [SkinningBuffer](../-skinning-buffer/index.md) and frees all its associated resources. |
| [destroySkybox](destroy-skybox.md) | [main]<br>open fun [destroySkybox](destroy-skybox.md)(skybox: [Skybox](../-skybox/index.md))<br>Destroys a [Skybox](../-skybox/index.md) and frees all its associated resources. |
| [destroyStream](destroy-stream.md) | [main]<br>open fun [destroyStream](destroy-stream.md)(stream: [Stream](../-stream/index.md))<br>Destroys a [Stream](../-stream/index.md) and frees all its associated resources. |
| [destroySwapChain](destroy-swap-chain.md) | [main]<br>open fun [destroySwapChain](destroy-swap-chain.md)(swapChain: [SwapChain](../-swap-chain/index.md))<br>Destroys a [SwapChain](../-swap-chain/index.md) and frees all its associated resources. |
| [destroyTexture](destroy-texture.md) | [main]<br>open fun [destroyTexture](destroy-texture.md)(texture: [Texture](../-texture/index.md))<br>Destroys a [Texture](../-texture/index.md) and frees all its associated resources. |
| [destroyVertexBuffer](destroy-vertex-buffer.md) | [main]<br>open fun [destroyVertexBuffer](destroy-vertex-buffer.md)(vertexBuffer: [VertexBuffer](../-vertex-buffer/index.md))<br>Destroys a [VertexBuffer](../-vertex-buffer/index.md) and frees all its associated resources. |
| [destroyView](destroy-view.md) | [main]<br>open fun [destroyView](destroy-view.md)(view: [View](../-view/index.md))<br>Destroys a [View](../-view/index.md) and frees all its associated resources. |
| [enableAccurateTranslations](enable-accurate-translations.md) | [main]<br>open fun [enableAccurateTranslations](enable-accurate-translations.md)()<br>Helper to enable accurate translations. |
| [flush](flush.md) | [main]<br>open fun [flush](flush.md)()<br>Kicks the hardware thread (e.g. |
| [flushAndWait](flush-and-wait.md) | [main]<br>open fun [flushAndWait](flush-and-wait.md)()<br>Kicks the hardware thread (e.g.: the OpenGL, Vulkan or Metal thread) and blocks until all commands to this point are executed.<br>[main]<br>open fun [flushAndWait](flush-and-wait.md)(timeout: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Kicks the hardware thread (e.g. |
| [getActiveFeatureLevel](get-active-feature-level.md) | [main]<br>open fun [getActiveFeatureLevel](get-active-feature-level.md)(): [Engine.FeatureLevel](-feature-level/index.md)<br>Returns the currently active feature level. |
| [getBackend](get-backend.md) | [main]<br>open fun [getBackend](get-backend.md)(): [Engine.Backend](-backend/index.md) |
| [getCameraComponent](get-camera-component.md) | [main]<br>open fun [getCameraComponent](get-camera-component.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Camera](../-camera/index.md)<br>Returns the Camera component of the given `entity`. |
| [getConfig](get-config.md) | [main]<br>open fun [getConfig](get-config.md)(): [Engine.Config](-config/index.md)<br>Retrieves the configuration settings of this [Engine](index.md). |
| [getEntityManager](get-entity-manager.md) | [main]<br>open fun [getEntityManager](get-entity-manager.md)(): [EntityManager](../-entity-manager/index.md) |
| [getFeatureFlag](get-feature-flag.md) | [main]<br>open fun [getFeatureFlag](get-feature-flag.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Retrieves the value of any feature flag. |
| [getLightManager](get-light-manager.md) | [main]<br>open fun [getLightManager](get-light-manager.md)(): [LightManager](../-light-manager/index.md) |
| [getMaxStereoscopicEyes](get-max-stereoscopic-eyes.md) | [main]<br>open fun [getMaxStereoscopicEyes](get-max-stereoscopic-eyes.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Returns the maximum number of stereoscopic eyes supported by Filament. |
| [getNativeJobSystem](get-native-job-system.md) | [main]<br>open fun [getNativeJobSystem](get-native-job-system.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getRenderableManager](get-renderable-manager.md) | [main]<br>open fun [getRenderableManager](get-renderable-manager.md)(): [RenderableManager](../-renderable-manager/index.md) |
| [getSteadyClockTimeNano](get-steady-clock-time-nano.md) | [main]<br>open fun [getSteadyClockTimeNano](get-steady-clock-time-nano.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Get the current time. |
| [getSupportedFeatureLevel](get-supported-feature-level.md) | [main]<br>open fun [getSupportedFeatureLevel](get-supported-feature-level.md)(): [Engine.FeatureLevel](-feature-level/index.md)<br>Query the feature level supported by the selected backend. |
| [getTransformManager](get-transform-manager.md) | [main]<br>open fun [getTransformManager](get-transform-manager.md)(): [TransformManager](../-transform-manager/index.md) |
| [hasFeatureFlag](has-feature-flag.md) | [main]<br>open fun [hasFeatureFlag](has-feature-flag.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Checks if a feature flag exists |
| [hasUnrecoverableFailure](has-unrecoverable-failure.md) | [main]<br>open fun [hasUnrecoverableFailure](has-unrecoverable-failure.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the engine has encountered an unrecoverable failure. |
| [isAutomaticInstancingEnabled](is-automatic-instancing-enabled.md) | [main]<br>open fun [isAutomaticInstancingEnabled](is-automatic-instancing-enabled.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [isPaused](is-paused.md) | [main]<br>open fun [isPaused](is-paused.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Get paused state of rendering thread. |
| [isValid](is-valid.md) | [main]<br>open fun [isValid](is-valid.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [isValidColorGrading](is-valid-color-grading.md) | [main]<br>open fun [isValidColorGrading](is-valid-color-grading.md)(object: [ColorGrading](../-color-grading/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidExpensiveMaterialInstance](is-valid-expensive-material-instance.md) | [main]<br>open fun [isValidExpensiveMaterialInstance](is-valid-expensive-material-instance.md)(object: [MaterialInstance](../-material-instance/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidFence](is-valid-fence.md) | [main]<br>open fun [isValidFence](is-valid-fence.md)(object: [Fence](../-fence/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidIndexBuffer](is-valid-index-buffer.md) | [main]<br>open fun [isValidIndexBuffer](is-valid-index-buffer.md)(object: [IndexBuffer](../-index-buffer/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidIndirectLight](is-valid-indirect-light.md) | [main]<br>open fun [isValidIndirectLight](is-valid-indirect-light.md)(object: [IndirectLight](../-indirect-light/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidMaterial](is-valid-material.md) | [main]<br>open fun [isValidMaterial](is-valid-material.md)(object: [Material](../-material/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidMaterialInstance](is-valid-material-instance.md) | [main]<br>open fun [isValidMaterialInstance](is-valid-material-instance.md)(ma: [Material](../-material/index.md), mi: [MaterialInstance](../-material-instance/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidMorphTargetBuffer](is-valid-morph-target-buffer.md) | [main]<br>open fun [isValidMorphTargetBuffer](is-valid-morph-target-buffer.md)(object: [MorphTargetBuffer](../-morph-target-buffer/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidRenderer](is-valid-renderer.md) | [main]<br>open fun [isValidRenderer](is-valid-renderer.md)(object: [Renderer](../-renderer/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidRenderTarget](is-valid-render-target.md) | [main]<br>open fun [isValidRenderTarget](is-valid-render-target.md)(object: [RenderTarget](../-render-target/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidScene](is-valid-scene.md) | [main]<br>open fun [isValidScene](is-valid-scene.md)(object: [Scene](../-scene/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidSkinningBuffer](is-valid-skinning-buffer.md) | [main]<br>open fun [isValidSkinningBuffer](is-valid-skinning-buffer.md)(object: [SkinningBuffer](../-skinning-buffer/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidSkybox](is-valid-skybox.md) | [main]<br>open fun [isValidSkybox](is-valid-skybox.md)(object: [Skybox](../-skybox/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidStream](is-valid-stream.md) | [main]<br>open fun [isValidStream](is-valid-stream.md)(object: [Stream](../-stream/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidSwapChain](is-valid-swap-chain.md) | [main]<br>open fun [isValidSwapChain](is-valid-swap-chain.md)(object: [SwapChain](../-swap-chain/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidTexture](is-valid-texture.md) | [main]<br>open fun [isValidTexture](is-valid-texture.md)(object: [Texture](../-texture/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidVertexBuffer](is-valid-vertex-buffer.md) | [main]<br>open fun [isValidVertexBuffer](is-valid-vertex-buffer.md)(object: [VertexBuffer](../-vertex-buffer/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [isValidView](is-valid-view.md) | [main]<br>open fun [isValidView](is-valid-view.md)(object: [View](../-view/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the object is valid. |
| [setActiveFeatureLevel](set-active-feature-level.md) | [main]<br>open fun [setActiveFeatureLevel](set-active-feature-level.md)(featureLevel: [Engine.FeatureLevel](-feature-level/index.md)): [Engine.FeatureLevel](-feature-level/index.md)<br>Activate all features of a given feature level. |
| [setAutomaticInstancingEnabled](set-automatic-instancing-enabled.md) | [main]<br>open fun [setAutomaticInstancingEnabled](set-automatic-instancing-enabled.md)(enable: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))<br>Enables or disables automatic instancing of render primitives. |
| [setFeatureFlag](set-feature-flag.md) | [main]<br>open fun [setFeatureFlag](set-feature-flag.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), value: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Set the value of a non-constant feature flag. |
| [setPaused](set-paused.md) | [main]<br>open fun [setPaused](set-paused.md)(paused: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))<br>Pause or resume the rendering thread. |
| [unprotected](unprotected.md) | [main]<br>open fun [unprotected](unprotected.md)()<br>Switch the command queue to unprotected mode. |
