---
name: filament-lifetime
description: >
  Enforce correct Filament object creation and destruction lifecycles.
  Use this skill when initializing rendering pipelines or allocating/deallocating GPU buffers and entities.
---

# Filament object creation and destruction lifecycles

Filament manages its own native allocations. To prevent segmentation faults and memory leaks, follow these rules:

## 1. ECS Entities & Components
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

## 2. Engine-Managed Native Objects
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
