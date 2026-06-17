# Configuring Gemini in Android Studio for Filament

If you are developing Android apps with Filament using the AI capabilities in **Android Studio (Gemini / Studio Bot)**, follow this guide to configure the assistant for accurate Filament code generation.

---

## 1. Custom Prompt Instructions in Android Studio

Android Studio allows you to set custom system prompts or context guidelines for its built-in Gemini assistant. Copy and paste the block below into your Gemini configurations to teach it Filament-specific guidelines.

### Copy-Paste Instruction Template:
```text
You are an expert assistant for Android application development. When writing graphics and rendering code using the Filament engine (com.google.android.filament), strictly adhere to these rules:

1. Lifecycle & Resource Management:
   - Never let Java objects wrapping native Filament entities garbage-collect without calling their explicit destroy/release methods.
   - For Engine-managed native objects (Engine, View, Scene, Renderer, SwapChain, Material, VertexBuffer, IndexBuffer, Texture), you must release them using `engine.destroy(object)` before releasing the Engine instance.
   - For ECS-managed objects (Entities created via EntityManager), strip components from the entity using `engine.destroy(entity)` before destroying the entity ID using `EntityManager.get().destroy(entity)`.
   - The Filament Engine itself must be released last using `Engine.destroy(engine)`.

2. Surface & View Setup:
   - When using Filament inside a TextureView or SurfaceView, ensure SwapChain is created using the native surface holder (or surface texture) only after it becomes valid.
   - Always destroy the SwapChain when the surface is destroyed, and recreate it when a new surface is available.

3. Math Types:
   - Use Filament's Android math classes or standard floats. Do not mix third-party math packages unless explicitly requested.
```

---

## 2. Dynamic Context Sharing in Android Studio

To ensure Gemini in Android Studio leverages project-level documentation:
1. **Reference files in prompt**: You can directly drag or reference `docs/AI_CONTEXT.md` into the Gemini chat window or tag it to instruct the model to use it as context.
2. **Javadoc integration**: Android Studio AIs dynamically index APIs using their attached Javadocs. Filament's JSR-305 nullability annotations (`@NonNull`, `@Nullable`) and detailed library Javadocs in the Filament AAR will guide the AI to generate safe, null-checked API calls.
