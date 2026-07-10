---
name: filament-lifetime
description: >
  Enforce correct Filament object creation and destruction lifecycles.
  Use this skill when initializing rendering pipelines or allocating/deallocating GPU buffers and entities.
---

# Filament object creation and destruction lifecycles

Filament manages its own native allocations. To prevent segmentation faults and memory leaks, follow these rules:

## 1. ECS Entities & Components
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
