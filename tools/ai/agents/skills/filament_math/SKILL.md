---
name: filament-math
description: >
  Enforce correct mathematical primitives and naming conventions in Filament code.
  Use this skill when performing coordinate transformations, vector math, or projection setups.
---

# Filament Mathematical Primitives and Naming Conventions

Filament provides its own set of math primitives optimized for graphics. Avoid importing external library formats (like GLM, Eigen, or raw DirectX math structures) unless explicitly requested.

## 1. Approved Math Types
All math constructs must reside in the `filament::math` namespace:

*   **Vectors**:
    *   `math::float2` (2D float vectors)
    *   `math::float3` (3D float vectors)
    *   `math::float4` (4D float vectors)
    *   `math::half3` / `math::half4` (half-precision float vectors)
    *   `math::int2` / `math::int3` / `math::int4` (integer vectors)
*   **Matrices**:
    *   `math::mat3f` (3x3 float matrix)
    *   `math::mat4f` (4x4 float matrix)
*   **Rotations**:
    *   `math::quatf` (quaternion rotation)

## 2. Naming and Manipulations
*   Use standard operators (`+`, `-`, `*`, `/`) between math components.
*   Avoid raw float pointers or C-style array indices. Use the built-in constructors or indexing operators:
    ```cpp
    // Correct
    math::float3 position(0.0f, 1.0f, 2.0f);
    float x = position.x; // or position[0]
    ```
*   When transforming vectors via matrices, ensure proper dimensional multiplication order:
    ```cpp
    math::mat4f transform = ...;
    math::float4 homogeneousVec(position, 1.0f);
    math::float4 transformed = transform * homogeneousVec;
    ```
