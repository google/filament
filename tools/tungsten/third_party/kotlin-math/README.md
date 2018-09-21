# kotlin-math
Set of Kotlin APIs to make graphics math easier to write. These APIs are mostly
modeled after GLSL (OpenGL Shading Language) to make porting code to and from
shaders easier.

The various types offered by this library are only meant to be _value types_.
Most APIs are therefore exposed as top-level functions and not as methods.
For instance:

```kotlin
val v = Float3(1.0f, 3.0f, 4.0f)
val n = normalize(v)
```

## Building the project

Simply run the following command to generate `build/libs/kotlin-math.jar`:

```bash
$ ./gradlew assemble
```

## Types

Vector types:
- Float2, vector of 2 floats
- Float3, vector of 3 floats
- Float4, vector of 4 floats
- Bool2, vector of 2 booleans
- Bool3, vector of 3 booleans
- Bool4, vector of 4 booleans

Matrix types:
- Mat3, 3x3 float matrix
- Mat4, 4x4 float matrix

## Vector types

### Accessing components

Each vector type exposes its component as properties:

```kotlin
val x = myVector.x
val (x, y, z) = myVector
```

A vector can also be treated as an array:

```kotlin
val x = myVector[0]
val x = myVector[VectorComponents.X]
```

The traditional mathematical form with 1-based indexing can be used:

```kotlin
val x = myVector(1)
```

### Property aliases

To improve code readability, the vector types provide aliases for each property,
allowing you to choose the most appropriate names:

```kotlin
val (x, y, z) = myPosition
val (r, g, b) = myColor
val (s, t) = myTextureCoordinates
```

### Swizzling

Vector types also provide different ways to swizzle their components, although
in a more limited way than in GLSL. The most obvious use for swizzling is to
extract sub-vectors:

```kotlin
val position = Float3(…)
val position2d = position.xy // extract a Float2

val colorWithAlpha = Float4(…)
val rgbColor = colorWithAlpha.rgb // extract a Float3
```

The get operators allows for more complex swizzling by enabling re-ordering and
duplication of the components:

```kotlin
val colorWithAlpha = Float4(…)
val bgrColor = colorWithAlpha[
    VectorComponents.B,
    VectorComponents.G,
    VectorComponents.R
] // re-ordered 3 components sub-vector
```

### Comparing vector types

Vector comparisons follow GLSL rules:
- `==` returns true if all components are equal 
- `!=` returns true if not all components are equal

In addition you can use component-wise relational operators that return a vector
of boolean with the result of each component-wise comparison:
- `lessThan`
- `lessThanEqual`
- `greaterThan`
- `greaterThanEqual`
- `equal`
- `notEqual`

Example:

```kotlin
if (all(lessThan(v1, v2))) {
   // …
}
```

You can also use the following infix operators if you prefer the operator
syntax:
- `lt`
- `lte`
- `gt`
- `gte`
- `eq`
- `neq`

Example:

```kotlin
if (any(v1 lte v2)) {
   // …
}
```

## Matrix types

Matrices are represented as a set of column vectors. For instance, a `Mat4` can
be destructured into the right, up, forward and translation vectors:

```kotlin
val (right, up, forward, translation) = myMat4
```

Each vector can be accessed as a property or by its index:

```kotlin
forward = myMat4.forward
forward = myMat4.z
forward = myMat4[2]
forward = myMat4[MatrixColumns.Z]
```

Matrix types also offer APIs to access each element individually by specifying
the column then row:

```kotlin
v = myMat4.z[1]
v = myMat4[2, 1]
v = myMat4[MatrixColumns.Z, 1]
```

You can also use the invoke operator to access elements in row-major mode with
1-based indices to follow the traditional mathematical notation:

```kotlin
v = myMat4(2, 3) // equivalent to myMat4[2, 1]
```

## Scalar APIs

The file `Scalar.kt` contains various helper methods to use common math operations
with floats. It is intended to be used in combination with Kotlin 1.2's new float
math methods.
