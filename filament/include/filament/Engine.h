/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef TNT_FILAMENT_ENGINE_H
#define TNT_FILAMENT_ENGINE_H

#include <backend/Platform.h>

#include <utils/compiler.h>

namespace utils {
class Entity;
class EntityManager;
class JobSystem;
} // namespace utils

namespace filament {

class BufferObject;
class Camera;
class ColorGrading;
class DebugRegistry;
class Fence;
class IndexBuffer;
class SkinningBuffer;
class IndirectLight;
class Material;
class MaterialInstance;
class MorphTargetBuffer;
class Renderer;
class RenderTarget;
class Scene;
class Skybox;
class Stream;
class SwapChain;
class Texture;
class VertexBuffer;
class View;

class LightManager;
class RenderableManager;
class TransformManager;

#ifndef FILAMENT_PER_RENDER_PASS_ARENA_SIZE_IN_MB
#    define FILAMENT_PER_RENDER_PASS_ARENA_SIZE_IN_MB 3
#endif

#ifndef FILAMENT_PER_FRAME_COMMANDS_SIZE_IN_MB
#    define FILAMENT_PER_FRAME_COMMANDS_SIZE_IN_MB 2
#endif

#ifndef FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB
#    define FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB 1
#endif

#ifndef FILAMENT_COMMAND_BUFFER_SIZE_IN_MB
#    define FILAMENT_COMMAND_BUFFER_SIZE_IN_MB (FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB * 3)
#endif

/**
 * Engine is filament's main entry-point.
 *
 * An Engine instance main function is to keep track of all resources created by the user and
 * manage the rendering thread as well as the hardware renderer.
 *
 * To use filament, an Engine instance must be created first:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * #include <filament/Engine.h>
 * using namespace filament;
 *
 * Engine* engine = Engine::create();
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Engine essentially represents (or is associated to) a hardware context
 * (e.g. an OpenGL ES context).
 *
 * Rendering typically happens in an operating system's window (which can be full screen), such
 * window is managed by a filament.Renderer.
 *
 * A typical filament render loop looks like this:
 *
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * #include <filament/Engine.h>
 * #include <filament/Renderer.h>
 * #include <filament/Scene.h>
 * #include <filament/View.h>
 * using namespace filament;
 *
 * Engine* engine       = Engine::create();
 * SwapChain* swapChain = engine->createSwapChain(nativeWindow);
 * Renderer* renderer   = engine->createRenderer();
 * Scene* scene         = engine->createScene();
 * View* view           = engine->createView();
 *
 * view->setScene(scene);
 *
 * do {
 *     // typically we wait for VSYNC and user input events
 *     if (renderer->beginFrame(swapChain)) {
 *         renderer->render(view);
 *         renderer->endFrame();
 *     }
 * } while (!quit);
 *
 * engine->destroy(view);
 * engine->destroy(scene);
 * engine->destroy(renderer);
 * engine->destroy(swapChain);
 * Engine::destroy(&engine); // clears engine*
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Resource Tracking
 * =================
 *
 *  Each Engine instance keeps track of all objects created by the user, such as vertex and index
 *  buffers, lights, cameras, etc...
 *  The user is expected to free those resources, however, leaked resources are freed when the
 *  engine instance is destroyed and a warning is emitted in the console.
 *
 * Thread safety
 * =============
 *
 * An Engine instance is not thread-safe. The implementation makes no attempt to synchronize
 * calls to an Engine instance methods.
 * If multi-threading is needed, synchronization must be external.
 *
 * Multi-threading
 * ===============
 *
 * When created, the Engine instance starts a render thread as well as multiple worker threads,
 * these threads have an elevated priority appropriate for rendering, based on the platform's
 * best practices. The number of worker threads depends on the platform and is automatically
 * chosen for best performance.
 *
 * On platforms with asymmetric cores (e.g. ARM's Big.Little), Engine makes some educated guesses
 * as to which cores to use for the render thread and worker threads. For example, it'll try to
 * keep an OpenGL ES thread on a Big core.
 *
 * Swap Chains
 * ===========
 *
 * A swap chain represents an Operating System's *native* renderable surface. Typically it's a window
 * or a view. Because a SwapChain is initialized from a native object, it is given to filament
 * as a `void*`, which must be of the proper type for each platform filament is running on.
 *
 * @see SwapChain
 *
 *
 * @see Renderer
 */
class UTILS_PUBLIC Engine {
public:
    using Platform = backend::Platform;
    using Backend = backend::Backend;
    using DriverConfig = backend::Platform::DriverConfig;

    /**
     * Config is used to define the memory footprint used by the engine, such as the
     * command buffer size. Config can be used to customize engine requirements based 
     * on the applications needs.
     *
     *    .perRenderPassArenaSizeMB (default: 3 MiB)
     *   +--------------------------+
     *   |                          |
     *   | .perFrameCommandsSizeMB  |
     *   |    (default 2 MiB)       |
     *   |                          |
     *   +--------------------------+
     *   |  (froxel, etc...)        |
     *   +--------------------------+
     *
     *
     *      .commandBufferSizeMB (default 3MiB)
     *   +--------------------------+
     *   | .minCommandBufferSizeMB  |
     *   +--------------------------+
     *   | .minCommandBufferSizeMB  |
     *   +--------------------------+
     *   | .minCommandBufferSizeMB  |
     *   +--------------------------+
     *   :                          :
     *   :                          :
     *
     */
    struct Config {
        /**
         * Size in MiB of the low-level command buffer arena.
         *
         * Each new command buffer is allocated from here. If this buffer is too small the program
         * might terminate or rendering errors might occur.
         *
         * This is typically set to minCommandBufferSizeMB * 3, so that up to 3 frames can be
         * batched-up at once.
         *
         * This value affects the application's memory usage.
         */
        uint32_t commandBufferSizeMB = FILAMENT_COMMAND_BUFFER_SIZE_IN_MB;


        /**
         * Size in MiB of the per-frame data arena.
         *
         * This is the main arena used for allocations when preparing a frame.
         * e.g.: Froxel data and high-level commands are allocated from this arena.
         *
         * If this size is too small, the program will abort on debug builds and have undefined
         * behavior otherwise.
         *
         * This value affects the application's memory usage.
         */
        uint32_t perRenderPassArenaSizeMB = FILAMENT_PER_RENDER_PASS_ARENA_SIZE_IN_MB;


        /**
         * Size in MiB of the backend's handle arena.
         *
         * Backends will fallback to slower heap-based allocations when running out of space and
         * log this condition.
         *
         * If 0, then the default value for the given platform is used
         *
         * This value affects the application's memory usage.
         */
        uint32_t driverHandleArenaSizeMB = 0;


        /**
         * Minimum size in MiB of a low-level command buffer.
         *
         * This is how much space is guaranteed to be available for low-level commands when a new
         * buffer is allocated. If this is too small, the engine might have to stall to wait for
         * more space to become available, this situation is logged.
         *
         * This value does not affect the application's memory usage.
         */
        uint32_t minCommandBufferSizeMB = FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB;


        /**
         * Size in MiB of the per-frame high level command buffer.
         *
         * This buffer is related to the number of draw calls achievable within a frame, if it is
         * too small, the program will abort on debug builds and have undefined behavior otherwise.
         *
         * It is allocated from the 'per-render-pass arena' above. Make sure that at least 1 MiB is
         * left in the per-render-pass arena when deciding the size of this buffer.
         *
         * This value does not affect the application's memory usage.
         */
        uint32_t perFrameCommandsSizeMB = FILAMENT_PER_FRAME_COMMANDS_SIZE_IN_MB;
    };

    /**
     * Creates an instance of Engine
     *
     * @param backend           Which driver backend to use.
     *
     * @param platform          A pointer to an object that implements Platform. If this is
     *                          provided, then this object is used to create the hardware context
     *                          and expose platform features to it.
     *
     *                          If not provided (or nullptr is used), an appropriate Platform
     *                          is created automatically.
     *
     *                          All methods of this interface are called from filament's
     *                          render thread, which is different from the main thread.
     *
     *                          The lifetime of \p platform must exceed the lifetime of
     *                          the Engine object.
     *
     *  @param sharedGLContext  A platform-dependant OpenGL context used as a shared context
     *                          when creating filament's internal context.
     *                          Setting this parameter will force filament to use the OpenGL
     *                          implementation (instead of Vulkan for instance).
     *
     * @param config            A pointer to optional parameters to specify memory size
     *                          configuration options.  If nullptr, then defaults used.
     *
     * @return A pointer to the newly created Engine, or nullptr if the Engine couldn't be created.
     *
     * nullptr if the GPU driver couldn't be initialized, for instance if it doesn't
     * support the right version of OpenGL or OpenGL ES.
     *
     * @exception utils::PostConditionPanic can be thrown if there isn't enough memory to
     * allocate the command buffer. If exceptions are disabled, this condition if fatal and
     * this function will abort.
     *
     * \remark
     * This method is thread-safe.
     */
    static Engine* create(Backend backend = Backend::DEFAULT,
            Platform* platform = nullptr, void* sharedGLContext = nullptr,
            const Config* config = nullptr);

#if UTILS_HAS_THREADING
    /**
     * A callback used with Engine::createAsync() called once the engine is initialized and it is
     * safe to call Engine::getEngine(token). This callback is invoked from an arbitrary worker
     * thread. Engine::getEngine() CANNOT be called from that thread, instead it must be called
     * from the same thread than Engine::createAsync() was called from.
     *
     * @param user   User provided parameter given in createAsync().
     *
     * @param token  An opaque token used to call Engine::getEngine().
     */
    using CreateCallback = void(void* user, void* token);

    /**
     * Creates an instance of Engine asynchronously
     *
     * @param callback          Callback called once the engine is initialized and it is safe to
     *                          call Engine::getEngine.
     *
     * @param user              A user provided pointer that is given back to callback unmodified.
     *
     * @param backend           Which driver backend to use.
     *
     * @param platform          A pointer to an object that implements Platform. If this is
     *                          provided, then this object is used to create the hardware context
     *                          and expose platform features to it.
     *
     *                          If not provided (or nullptr is used), an appropriate Platform
     *                          is created automatically.
     *
     *                          All methods of this interface are called from filament's
     *                          render thread, which is different from the main thread.
     *
     *                          The lifetime of \p platform must exceed the lifetime of
     *                          the Engine object.
     *
     *  @param sharedGLContext  A platform-dependant OpenGL context used as a shared context
     *                          when creating filament's internal context.
     *                          Setting this parameter will force filament to use the OpenGL
     *                          implementation (instead of Vulkan for instance).
     * 
     * @param config            A pointer to optional parameters to specify memory size
     *                          configuration options
     */
    static void createAsync(CreateCallback callback, void* user,
            Backend backend = Backend::DEFAULT,
            Platform* platform = nullptr, void* sharedGLContext = nullptr,
            const Config* config = nullptr);

    /**
     * Retrieve an Engine* from createAsync(). This must be called from the same thread than
     * Engine::createAsync() was called from.
     *
     * @param token An opaque token given in the createAsync() callback function.
     *
     * @return A pointer to the newly created Engine, or nullptr if the Engine couldn't be created.
     *
     * @exception utils::PostConditionPanic can be thrown if there isn't enough memory to
     * allocate the command buffer. If exceptions are disabled, this condition if fatal and
     * this function will abort.
     */
    static Engine* getEngine(void* token);
#endif

    /**
     * Destroy the Engine instance and all associated resources.
     *
     * Engine.destroy() should be called last and after all other resources have been destroyed,
     * it ensures all filament resources are freed.
     *
     * Destroy performs the following tasks:
     * 1. Destroy all internal software and hardware resources.
     * 2. Free all user allocated resources that are not already destroyed and logs a warning.
     *    This indicates a "leak" in the user's code.
     * 3. Terminate the rendering engine's thread.
     *
     * @param engine A pointer to the filament.Engine* to be destroyed.
     *               \p engine is cleared upon return.
     *
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * #include <filament/Engine.h>
     * using namespace filament;
     *
     * Engine* engine = Engine::create();
     * Engine::destroy(&engine); // clears engine*
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *
     * \remark
     * This method is thread-safe.
     */
    static void destroy(Engine** engine);

    /**
     * Destroy the Engine instance and all associated resources.
     *
     * Engine.destroy() should be called last and after all other resources have been destroyed,
     * it ensures all filament resources are freed.
     *
     * Destroy performs the following tasks:
     * 1. Destroy all internal software and hardware resources.
     * 2. Free all user allocated resources that are not already destroyed and logs a warning.
     *    This indicates a "leak" in the user's code.
     * 3. Terminate the rendering engine's thread.
     *
     * @param engine A pointer to the filament.Engine to be destroyed.
     *
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * #include <filament/Engine.h>
     * using namespace filament;
     *
     * Engine* engine = Engine::create();
     * Engine::destroy(engine);
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *
     * \remark
     * This method is thread-safe.
     */
    static void destroy(Engine* engine);

    using FeatureLevel = backend::FeatureLevel;


    /**
     * Query the feature level supported by the selected backend.
     *
     * A specific feature level needs to be set before the corresponding features can be used.
     *
     * @return FeatureLevel supported the selected backend.
     * @see setActiveFeatureLevel
     */
    FeatureLevel getSupportedFeatureLevel() const noexcept;

    /**
     * Activate all features of a given feature level. By default FeatureLevel::FEATURE_LEVEL_1 is
     * active. The selected feature level must not be higher than the value returned by
     * getActiveFeatureLevel() and it's not possible lower the active feature level.
     *
     * @param featureLevel the feature level to activate. If featureLevel is lower than
     *                     getActiveFeatureLevel(), the current (higher) feature level is kept.
     *                     If featureLevel is higher than getSupportedFeatureLevel(), an exception
     *                     is thrown, or the program is terminated if exceptions are disabled.
     *
     * @return the active feature level.
     *
     * @see getSupportedFeatureLevel
     * @see getActiveFeatureLevel
     */
    FeatureLevel setActiveFeatureLevel(FeatureLevel featureLevel);

    /**
     * Returns the currently active feature level.
     * @return currently active feature level
     * @see getSupportedFeatureLevel
     * @see setActiveFeatureLevel
     */
    FeatureLevel getActiveFeatureLevel() const noexcept;


    /**
     * @return EntityManager used by filament
     */
    utils::EntityManager& getEntityManager() noexcept;

    /**
     * @return RenderableManager reference
     */
    RenderableManager& getRenderableManager() noexcept;

    /**
     * @return LightManager reference
     */
    LightManager& getLightManager() noexcept;

    /**
     * @return TransformManager reference
     */
    TransformManager& getTransformManager() noexcept;

    /**
     * Helper to enable accurate translations.
     * If you need this Engine to handle a very large world space, one way to achieve this
     * automatically is to enable accurate translations in the TransformManager. This helper
     * provides a convenient way of doing that.
     * This is typically called once just after creating the Engine.
     */
    void enableAccurateTranslations() noexcept;

    /**
     * Enables or disables automatic instancing of render primitives. Instancing of render
     * primitives can greatly reduce CPU overhead but requires the instanced primitives to be
     * identical (i.e. use the same geometry) and use the same MaterialInstance. If it is known
     * that the scene doesn't contain any identical primitives, automatic instancing can have some
     * overhead and it is then best to disable it.
     *
     * Disabled by default.
     *
     * @param enable true to enable, false to disable automatic instancing.
     *
     * @see RenderableManager
     * @see MaterialInstance
     */
    void setAutomaticInstancingEnabled(bool enable) noexcept;

    /**
     * @return true if automatic instancing is enabled, false otherwise.
     * @see setAutomaticInstancingEnabled
     */
    bool isAutomaticInstancingEnabled() const noexcept;

    /**
     * Creates a SwapChain from the given Operating System's native window handle.
     *
     * @param nativeWindow An opaque native window handle. e.g.: on Android this is an
     *                     `ANativeWindow*`.
     * @param flags One or more configuration flags as defined in `SwapChain`.
     *
     * @return A pointer to the newly created SwapChain or nullptr if it couldn't be created.
     *
     * @see Renderer.beginFrame()
     */
    SwapChain* createSwapChain(void* nativeWindow, uint64_t flags = 0) noexcept;


    /**
     * Creates a headless SwapChain.
     *
      * @param width    Width of the drawing buffer in pixels.
      * @param height   Height of the drawing buffer in pixels.
     * @param flags     One or more configuration flags as defined in `SwapChain`.
     *
     * @return A pointer to the newly created SwapChain or nullptr if it couldn't be created.
     *
     * @see Renderer.beginFrame()
     */
    SwapChain* createSwapChain(uint32_t width, uint32_t height, uint64_t flags = 0) noexcept;

    /**
     * Creates a renderer associated to this engine.
     *
     * A Renderer is intended to map to a *window* on screen.
     *
     * @return A pointer to the newly created Renderer or nullptr if it couldn't be created.
     */
    Renderer* createRenderer() noexcept;

    /**
     * Creates a View.
     *
     * @return A pointer to the newly created View or nullptr if it couldn't be created.
     */
    View* createView() noexcept;

    /**
     * Creates a Scene.
     *
     * @return A pointer to the newly created Scene or nullptr if it couldn't be created.
     */
    Scene* createScene() noexcept;

    /**
     * Creates a Camera component.
     *
     * @param entity Entity to add the camera component to.
     * @return A pointer to the newly created Camera or nullptr if it couldn't be created.
     */
    Camera* createCamera(utils::Entity entity) noexcept;

    /**
     * Returns the Camera component of the given entity.
     *
     * @param entity An entity.
     * @return A pointer to the Camera component for this entity or nullptr if the entity didn't
     *         have a Camera component. The pointer is valid until destroyCameraComponent()
     *         is called or the entity itself is destroyed.
     */
    Camera* getCameraComponent(utils::Entity entity) noexcept;

    /**
     * Destroys the Camera component associated with the given entity.
     *
     * @param entity An entity.
     */
    void destroyCameraComponent(utils::Entity entity) noexcept;

    /**
     * Creates a Fence.
     *
     * @return A pointer to the newly created Fence or nullptr if it couldn't be created.
     */
    Fence* createFence() noexcept;

    bool destroy(const BufferObject* p);        //!< Destroys a BufferObject object.
    bool destroy(const VertexBuffer* p);        //!< Destroys an VertexBuffer object.
    bool destroy(const Fence* p);               //!< Destroys a Fence object.
    bool destroy(const IndexBuffer* p);         //!< Destroys an IndexBuffer object.
    bool destroy(const SkinningBuffer* p);      //!< Destroys a SkinningBuffer object.
    bool destroy(const MorphTargetBuffer* p);   //!< Destroys a MorphTargetBuffer object.
    bool destroy(const IndirectLight* p);       //!< Destroys an IndirectLight object.

    /**
     * Destroys a Material object
     * @param p the material object to destroy
     * @attention All MaterialInstance of the specified material must be destroyed before
     *            destroying it.
     * @exception utils::PreConditionPanic is thrown if some MaterialInstances remain.
     * no-op if exceptions are disabled and some MaterialInstances remain.
     */
    bool destroy(const Material* p);
    bool destroy(const MaterialInstance* p);    //!< Destroys a MaterialInstance object.
    bool destroy(const Renderer* p);            //!< Destroys a Renderer object.
    bool destroy(const Scene* p);               //!< Destroys a Scene object.
    bool destroy(const Skybox* p);              //!< Destroys a SkyBox object.
    bool destroy(const ColorGrading* p);        //!< Destroys a ColorGrading object.
    bool destroy(const SwapChain* p);           //!< Destroys a SwapChain object.
    bool destroy(const Stream* p);              //!< Destroys a Stream object.
    bool destroy(const Texture* p);             //!< Destroys a Texture object.
    bool destroy(const RenderTarget* p);        //!< Destroys a RenderTarget object.
    bool destroy(const View* p);                //!< Destroys a View object.
    void destroy(utils::Entity e);              //!< Destroys all filament-known components from this entity

    /**
     * Kicks the hardware thread (e.g. the OpenGL, Vulkan or Metal thread) and blocks until
     * all commands to this point are executed. Note that does guarantee that the
     * hardware is actually finished.
     *
     * <p>This is typically used right after destroying the <code>SwapChain</code>,
     * in cases where a guarantee about the <code>SwapChain</code> destruction is needed in a
     * timely fashion, such as when responding to Android's
     * <code>android.view.SurfaceHolder.Callback.surfaceDestroyed</code></p>
     */
    void flushAndWait();

    /**
     * Kicks the hardware thread (e.g. the OpenGL, Vulkan or Metal thread) but does not wait
     * for commands to be either executed or the hardware finished.
     *
     * <p>This is typically used after creating a lot of objects to start draining the command
     * queue which has a limited size.</p>
      */
    void flush();

    /**
     * Drains the user callback message queue and immediately execute all pending callbacks.
     *
     * <p> Typically this should be called once per frame right after the application's vsync tick,
     * and typically just before computing parameters (e.g. object positions) for the next frame.
     * This is useful because otherwise callbacks will be executed by filament at a later time,
     * which may increase latency in certain applications.</p>
     */
    void pumpMessageQueues();

    /**
     * Returns the default Material.
     *
     * The default material is 80% white and uses the Material.Shading.LIT shading.
     *
     * @return A pointer to the default Material instance (a singleton).
     */
    const Material* getDefaultMaterial() const noexcept;

    /**
     * Returns the resolved backend.
     */
    Backend getBackend() const noexcept;

    /**
     * Returns the Platform object that belongs to this Engine.
     *
     * When Engine::create is called with no platform argument, Filament creates an appropriate
     * Platform subclass automatically. The specific subclass created depends on the backend and
     * OS. For example, when the OpenGL backend is used, the Platform object will be a descendant of
     * OpenGLPlatform.
     *
     * dynamic_cast should be used to cast the returned Platform object into a specific subclass.
     * Note that RTTI must be available to use dynamic_cast.
     *
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * Platform* platform = engine->getPlatform();
     * // static_cast also works, but more dangerous.
     * SpecificPlatform* specificPlatform = dynamic_cast<SpecificPlatform*>(platform);
     * specificPlatform->platformSpecificMethod();
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *
     * When a custom Platform is passed to Engine::create, Filament will use it instead, and this
     * method will return it.
     *
     * @return A pointer to the Platform object that was provided to Engine::create, or the
     * Filament-created one.
     */
    Platform* getPlatform() const noexcept;

    /**
     * Allocate a small amount of memory directly in the command stream. The allocated memory is
     * guaranteed to be preserved until the current command buffer is executed
     *
     * @param size       size to allocate in bytes. This should be small (e.g. < 1 KB)
     * @param alignment  alignment requested
     * @return           a pointer to the allocated buffer or nullptr if no memory was available.
     *
     * @note there is no need to destroy this buffer, it will be freed automatically when
     *       the current command buffer is executed.
     */
    void* streamAlloc(size_t size, size_t alignment = alignof(double)) noexcept;

    /**
      * Invokes one iteration of the render loop, used only on single-threaded platforms.
      *
      * This should be called every time the windowing system needs to paint (e.g. at 60 Hz).
      */
    void execute();

    /**
      * Retrieves the job system that the Engine has ownership over.
      *
      * @return JobSystem used by filament
      */
    utils::JobSystem& getJobSystem() noexcept;

#if defined(__EMSCRIPTEN__)
    /**
      * WebGL only: Tells the driver to reset any internal state tracking if necessary.
      * 
      * This is only useful when integrating an external renderer into Filament on platforms 
      * like WebGL, where share contexts do not exist. Filament keeps track of the GL
      * state it has set (like which texture is bound), and does not re-set that state if
      * it does not think it needs to. However, if an external renderer has set different
      * state in the mean time, Filament will use that new state unknowingly.
      * 
      * If you are in this situation, call this function - ideally only once per frame, 
      * immediately after calling Engine::execute().
      */
    void resetBackendState() noexcept;
#endif

    DebugRegistry& getDebugRegistry() noexcept;

protected:
    //! \privatesection
    Engine() noexcept = default;
    ~Engine() = default;

public:
    //! \privatesection
    Engine(Engine const&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine const&) = delete;
    Engine& operator=(Engine&&) = delete;
};

} // namespace filament

#endif // TNT_FILAMENT_ENGINE_H
