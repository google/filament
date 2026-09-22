//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Builder](index.md)/[importTexture](import-texture.md)

# importTexture

[main]\
open fun [importTexture](import-texture.md)(id: Long): [Texture.Builder](index.md)

Specify a native texture to import as a Filament texture. 

The texture id is backend-specific:

- OpenGL: GLuint texture ID
- Metal: idWith Metal, the id

```kotlin

 id <MTLTexture> metalTexture = ...
 filamentTexture->import((intptr_t) CFBridgingRetain(metalTexture));
 // free to release metalTexture

 // after using texture:
 engine->destroy(filamentTexture);   // metalTexture is released

```

This method should be used as a last resort. This API is subject to change or removal.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| id | a backend specific texture identifier |
