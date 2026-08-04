//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Builder](index.md)/[importTexture](import-texture.md)

# importTexture

[main]\
open fun [importTexture](import-texture.md)(id: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [Texture.Builder](index.md)

Specify a native texture to import as a Filament texture. 

 The texture id is backend-specific: 

- OpenGL: GLuint texture ID

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| id | a backend specific texture identifier |
