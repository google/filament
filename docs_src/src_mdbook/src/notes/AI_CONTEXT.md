# Filament AI Context Pack & Quick Reference

Use this document to provide expert context about the Filament rendering engine to LLMs (Gemini, Claude, ChatGPT). Copy and paste this file directly into the model's context or attach it to your project workspace.

---

## Key Directives (TL;DR)
1.  **ECS Entity Component Lifecycle**: First call `engine->destroy(entity)` to strip components, then call `engine->getEntityManager().destroy(entity)` to free the entity ID.
2.  **Engine-Managed Objects**: Never use standard C++ `delete` on pointers returned by `Engine`. Always destroy them via `engine->destroy(ptr)`.
3.  **Math Types**: Always use `filament::math` types (vectors, matrices, quaternions) instead of external types like GLM.
4.  **Vertex Attributes**: Align attributes in your `VertexBuffer` builders with custom material definitions (.mat).

---

## 1. Filament Engine Core Architecture

Filament is a real-time, physically-based rendering (PBR) engine. It has a strict thread separation model: a main thread where the user modifies scene state, and a render command queue thread that communicates with GPU APIs.

### The Core Rendering Pipeline
To render anything in Filament, you must establish these six core entities:
1.  **Engine**: The main context and factory. Keeps track of all GPU resources and worker threads.
2.  **SwapChain**: Represents the native platform window or render target surface.
3.  **Renderer**: Manages frame command submissions (`beginFrame`, `render`, `endFrame`).
4.  **View**: Integrates the viewport, camera, and scene. Controls post-processing effects, lighting settings, and rendering features.
5.  **Scene**: A container for entities with renderable or light components.
6.  **Camera**: Defines the projection matrix and transform.

---

## 2. Object Lifecycle and Destruction Rules (CRITICAL)

Filament manages its own native allocations. To prevent segmentation faults and memory leaks, follow these rules:

### A. ECS Entities & Components
#### Getting the Entity Manager
*   **Preferred Method**: Retrieve the `EntityManager` directly from the `Engine` instance:
    `utils::EntityManager& em = engine->getEntityManager();`
*   **Alternative (Avoid if engine is available)**: Only use the global singleton if the `Engine` pointer is inaccessible:
    `utils::EntityManager& em = utils::EntityManager::get();`

#### Entities
*   **Create**: Create Entity IDs using the entity manager:
    `utils::Entity entity = engine->getEntityManager().create();`
*   **Destroy**: Free the Entity ID from the entity manager:
    `engine->getEntityManager().destroy(entity);`

#### Components
*   **Create**: Attach components to an existing entity using manager builders:
    *   *Renderables/Lights*: Use `RenderableManager::Builder` or `LightManager::Builder` on the entity:
        ```cpp
        RenderableManager::Builder(1)
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vb, ib)
            .material(0, matInstance)
            .build(*engine, entity);
        ```
    *   *Cameras*: Attach a Camera component to the entity:
        `Camera* camera = engine->createCamera(entity);`
*   **Destroy (CRITICAL)**:
    1.  **To clean up the entire entity**:
        First, destroy all Filament-managed components on the entity:
        `engine->destroy(entity);`
        Second, free the Entity ID:
        `engine->getEntityManager().destroy(entity);`
        ```cpp
        // Correct Entity Cleanup
        engine->destroy(myEntity);
        engine->getEntityManager().destroy(myEntity);
        ```
    2.  **To remove a single component from the entity**:
        You can destroy specific components individually without deleting the entity:
        *   *Cameras*: `engine->destroyCameraComponent(entity);`
        *   *Transforms*: `engine->getTransformManager().destroy(entity);`
        *   *Renderables*: `engine->getRenderableManager().destroy(entity);`

### B. Engine-Managed Native Objects
Native GPU objects are created using the `Engine` factory or nested `Builder` patterns.
*   **NO `delete`**: Never call standard C++ `delete` on pointers returned by `Engine`.
*   **Direct Factories**: Instantiated directly from the `Engine` instance.
    *   *Objects*: `View`, `Scene`, `Renderer`, `SwapChain`.
    *   *Pattern*:
        ```cpp
        View* view = engine->createView();
        // ...
        engine->destroy(view);
        ```
*   **Builder Pattern**: Instantiated using a nested `Builder` class, passing the engine instance to `build(*engine)`.
    *   *Objects*: `VertexBuffer`, `IndexBuffer`, `Texture`, `Material`, `IndirectLight`, `Skybox`, `RenderTarget`.
    *   *Pattern*:
        ```cpp
        VertexBuffer* vb = VertexBuffer::Builder()
            .vertexCount(count)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
            .build(*engine);
        // ...
        engine->destroy(vb);
        ```

---

## 3. Vertex Buffers and Materials

*   **Attribute Alignment**: Ensure your `VertexBuffer::Builder` setup matches the attributes declared in the custom material shader (`.mat`).
*   **Attribute Offsets**: Do not hardcode magic numbers. Compute offsets dynamically using `sizeof`:
    ```cpp
    struct Vertex {
        math::float3 position;
        math::float2 uv;
    };
    VertexBuffer* vb = VertexBuffer::Builder()
        .vertexCount(count)
        .bufferCount(1)
        .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3, 0, sizeof(Vertex))
        .attribute(VertexAttribute::TEXCOORDS, 0, VertexBuffer::AttributeType::FLOAT2, offsetof(Vertex, uv), sizeof(Vertex))
        .build(*engine);

    // ...
    // When done, destroy the VertexBuffer:
    engine->destroy(vb);
    ```

---

## 4. Minimal C++ Boilerplate Example

Here is the correct, boilerplate-complete structure of a Filament application:

```cpp
#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/View.h>
#include <filament/Camera.h>
#include <filament/SwapChain.h>
#include <utils/EntityManager.h>

using namespace filament;

void runRenderLoop(void* nativeWindow) {
    // 1. Initialize Engine and Core Pipeline Objects
    Engine* engine = Engine::create();
    SwapChain* swapChain = engine->createSwapChain(nativeWindow);
    Renderer* renderer = engine->createRenderer();
    Scene* scene = engine->createScene();
    View* view = engine->createView();

    // 2. Set up ECS Camera
    utils::Entity cameraEntity = engine->getEntityManager().create();
    Camera* camera = engine->createCamera(cameraEntity);
    
    // Configure View
    view->setScene(scene);
    view->setCamera(camera);

    // 3. Render Loop
    bool quit = false;
    while (!quit) {
        if (renderer->beginFrame(swapChain)) {
            renderer->render(view);
            renderer->endFrame();
        }
    }

    // 4. Proper Destruction Order (Reverse of Creation)
    engine->destroyCameraComponent(cameraEntity);
    engine->getEntityManager().destroy(cameraEntity);

    engine->destroy(view);
    engine->destroy(scene);
    engine->destroy(renderer);
    engine->destroy(swapChain);
    
    // Destroy Engine context last
    Engine::destroy(&engine);
}
```

---

## 5. Filament Math & Types
*   Always use Filament's mathematical library under the `filament::math` namespace.
*   **Vector types**: `math::float2`, `math::float3`, `math::float4`.
*   **Matrix types**: `math::mat3f`, `math::mat4f`.
*   **Rotation types**: `math::quatf`.
