# Filament AI Context Pack & Quick Reference

Use this document to provide expert context about the Filament rendering engine to LLMs (Gemini, Claude, ChatGPT). Copy and paste this file directly into the model's context or attach it to your project workspace.

---

## 1. Filament Engine Core Architecture

Filament is a real-time, physically-based rendering (PBR) engine. It has a strict thread separation model: a main thread where the user modifies scene state, and a render command queue thread that communicates with GPU APIs.

### The Core Rendering Pipeline
To render anything in Filament, you must establish these five core entities:
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
Filament uses an Entity Component System (ECS) model for scene graph objects (cameras, lights, renderable meshes).
*   **Entities**: Lightweight identifiers.
    *   *Create*: `utils::Entity entity = utils::EntityManager::get().create();`
    *   *Destroy ID*: `utils::EntityManager::get().destroy(entity);`
*   **Components**: Attached to Entities using builders.
    *   *Create Renderable*: Use `RenderableManager::Builder` on an entity:
        ```cpp
        RenderableManager::Builder(1)
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vb, ib)
            .material(0, matInstance)
            .build(*engine, entity);
        ```
    *   *Create Camera*: Use `engine->createCamera(entity)` to attach a Camera component to the entity.
*   **Destruction Sequence (CRITICAL)**:
    1. First, call `engine->destroy(entity)` (or `engine->destroyCameraComponent(entity)` for cameras). This cleans up all Filament-managed component memory associated with the entity.
    2. Then, call `utils::EntityManager::get().destroy(entity)` to free the entity ID.
    ```cpp
    // Correct Cleanup
    engine->destroy(myRenderableEntity);
    utils::EntityManager::get().destroy(myRenderableEntity);
    ```

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
    utils::Entity cameraEntity = utils::EntityManager::get().create();
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
    utils::EntityManager::get().destroy(cameraEntity);

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
