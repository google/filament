---
name: bindings-synchronization
description: >
  Guidelines and SOP for synchronizing public C++ API changes (methods, enums, options structs) 
  to Java (Android JNI) and JavaScript/TypeScript (Web/WASM Embind).
---

# Cross-Platform Language Bindings Synchronization

When making changes to public Filament C++ APIs, AI agents **must** ensure the changes are mirrored in the Java and JavaScript/TypeScript binding layers.

---

## 1. Trigger Conditions

You must check and update the language bindings if your changes touch any public headers under `filament/include/filament/`, specifically:
- **New/Modified/Deleted Methods** on core classes (e.g., `View`, `Engine`, `Camera`, `LightManager`, `RenderableManager`).
- **Options Structs** (e.g., `AmbientOcclusionOptions`, `BloomOptions`, `FogOptions`).
  > [!IMPORTANT]
  > If you modify options structs defined in `filament/include/filament/Options.h`, **you must run the beamsplitter tool**. The tool parses `Options.h` and automatically updates Java bindings, JavaScript/TypeScript bindings, and C++ JSON serialization files.
  > 
  > Run it via:
  > ```bash
  > cd tools/beamsplitter && go run .
  > ```
- **Enums & Constants** (e.g., `BlendMode`, `QualityLevel`, `AntiAliasing` under `filament/include/filament/` or nested inside classes).

---


## 2. Java & Android JNI Synchronization

All JNI JNI-exported functions use automatic symbol resolution (dynamic mapping), meaning they are mapped by signature rather than a static registration table.

### File Mappings
- **Java Class**: `android/filament-android/src/main/java/com/google/android/filament/<ClassName>.java`
- **JNI C++ Wrapper**: `android/filament-android/src/main/cpp/<ClassName>.cpp`

### Guidelines
1. **API Consistency**: Keep method visibility `public` (unless the API is strictly internal to the Java package). Follow Java naming conventions (camelCase).
2. **Getter/Setter Completeness**: If you add a Setter (e.g., `setFoo`), always add the corresponding Getter (e.g., `isFoo` or `getFoo`).
3. **JNI Signature**:
   - Native declarations in Java must be `private static native`.
   - The corresponding C++ function in JNI must be decorated with `extern "C" JNIEXPORT <ReturnType> JNICALL Java_com_google_android_filament_<ClassName>_<NativeMethodName>`.
   - Return values from native C++ booleans should be cast to `jboolean` using `(jboolean)` or `static_cast<jboolean>()`.

---

## 3. JavaScript & TypeScript Synchronization

Filament uses Emscripten's Embind to export C++ APIs to JS.

### File Mappings
- **Embind Source**: `web/filament-js/jsbindings.cpp`
- **Enums Embind Source**: `web/filament-js/jsenums.cpp`
- **TypeScript Definition**: `web/filament-js/filament.d.ts`
- **JS Extension Wrappers**: `web/filament-js/extensions.js`

### Guidelines
1. **Embind Registration**:
   - Register member functions in `jsbindings.cpp` within the corresponding `class_<ClassName>("<JSClassName>")` block using `.function("methodName", &ClassName::methodName)`.
2. **TypeScript Declarations**:
   - Always update `filament.d.ts` to match any changes. Every bound C++ method should have a corresponding declaration inside `export class <ClassName>` to avoid IDE type errors.
3. **Complex Option Structs**:
   - If you modify or add fields to an options struct (like `BloomOptions`), make sure to:
     - Register the new fields in `jsbindings.cpp` using `.field("fieldName", &ClassName::Options::fieldName)`.
     - Update the TypeScript interface in `filament.d.ts` to include the new properties.
     - Check if `extensions.js` contains a prototype-overrides wrapper for merging the options (e.g., `Filament.View.prototype.setBloomOptions`). Update the default options object if needed.

---

## 4. Verification Check

Before concluding a task:
1. Double-check JNI signature signatures against the Java class.
2. Verify that type bindings and return types match up.
3. Verify that TS definitions in `filament.d.ts` are 100% synchronized.
