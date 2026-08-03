//[filament-android](../../index.md)/[com.google.android.filament](index.md)

# Package-level declarations

## Types

| Name | Summary |
|---|---|
| [Box](-box/index.md) | [main]<br>open class [Box](-box/index.md)<br>An axis-aligned 3D box represented by its center and half-extent. |
| [BufferObject](-buffer-object/index.md) | [main]<br>open class [BufferObject](-buffer-object/index.md)<br>A generic GPU buffer containing data. |
| [Camera](-camera/index.md) | [main]<br>open class [Camera](-camera/index.md)<br>Camera represents the eye through which the scene is viewed. |
| [ColorGrading](-color-grading/index.md) | [main]<br>open class [ColorGrading](-color-grading/index.md)<br>`ColorGrading` is used to transform (either to modify or correct) the colors of the HDR buffer rendered by Filament. |
| [Colors](-colors/index.md) | [main]<br>open class [Colors](-colors/index.md)<br>Utilities to manipulate and convert colors. |
| [Engine](-engine/index.md) | [main]<br>open class [Engine](-engine/index.md)<br>Engine is filament's main entry-point. |
| [Entity](-entity/index.md) | [main]<br>@[Retention](https://developer.android.com/reference/kotlin/java/lang/annotation/Retention.html)(value = [CLASS](https://developer.android.com/reference/kotlin/java/lang/annotation/RetentionPolicy.html#CLASS))<br>@[Target](https://developer.android.com/reference/kotlin/java/lang/annotation/Target.html)(value = [])<br>annotation class [Entity](-entity/index.md) |
| [EntityInstance](-entity-instance/index.md) | [main]<br>@[Retention](https://developer.android.com/reference/kotlin/java/lang/annotation/Retention.html)(value = [CLASS](https://developer.android.com/reference/kotlin/java/lang/annotation/RetentionPolicy.html#CLASS))<br>@[Target](https://developer.android.com/reference/kotlin/java/lang/annotation/Target.html)(value = [])<br>annotation class [EntityInstance](-entity-instance/index.md) |
| [EntityManager](-entity-manager/index.md) | [main]<br>open class [EntityManager](-entity-manager/index.md) |
| [Fence](-fence/index.md) | [main]<br>open class [Fence](-fence/index.md) |
| [Filament](-filament/index.md) | [main]<br>open class [Filament](-filament/index.md) |
| [IndexBuffer](-index-buffer/index.md) | [main]<br>open class [IndexBuffer](-index-buffer/index.md)<br>A buffer containing vertex indices into a `VertexBuffer`. |
| [IndirectLight](-indirect-light/index.md) | [main]<br>open class [IndirectLight](-indirect-light/index.md)<br>`IndirectLight` is used to simulate environment lighting, a form of global illumination. |
| [LightManager](-light-manager/index.md) | [main]<br>open class [LightManager](-light-manager/index.md)<br>LightManager allows you to create a light source in the scene, such as a sun or street lights. |
| [Material](-material/index.md) | [main]<br>open class [Material](-material/index.md)<br>A Filament Material defines the visual appearance of an object. |
| [MaterialInstance](-material-instance/index.md) | [main]<br>open class [MaterialInstance](-material-instance/index.md) |
| [MathUtils](-math-utils/index.md) | [main]<br>class [MathUtils](-math-utils/index.md) |
| [MorphTargetBuffer](-morph-target-buffer/index.md) | [main]<br>open class [MorphTargetBuffer](-morph-target-buffer/index.md) |
| [NativeSurface](-native-surface/index.md) | [main]<br>open class [NativeSurface](-native-surface/index.md) |
| [RenderableManager](-renderable-manager/index.md) | [main]<br>open class [RenderableManager](-renderable-manager/index.md)<br>Factory and manager for renderables, which are entities that can be drawn. |
| [Renderer](-renderer/index.md) | [main]<br>open class [Renderer](-renderer/index.md)<br>A `Renderer` instance represents an operating system's window. |
| [RenderTarget](-render-target/index.md) | [main]<br>open class [RenderTarget](-render-target/index.md)<br>An offscreen render target that can be associated with a [View](-view/index.md) and contains weak references to a set of attached [Texture](-texture/index.md) objects. |
| [Scene](-scene/index.md) | [main]<br>open class [Scene](-scene/index.md)<br>A `Scene` is a flat container of [RenderableManager](-renderable-manager/index.md) and [LightManager](-light-manager/index.md) components. |
| [SkinningBuffer](-skinning-buffer/index.md) | [main]<br>open class [SkinningBuffer](-skinning-buffer/index.md) |
| [Skybox](-skybox/index.md) | [main]<br>open class [Skybox](-skybox/index.md)<br>Skybox When added to a [Scene](-scene/index.md), the `Skybox` fills all untouched pixels. |
| [Stream](-stream/index.md) | [main]<br>open class [Stream](-stream/index.md)<br>`Stream` is used to attach a native video stream to a filament [Texture](-texture/index.md). |
| [SurfaceOrientation](-surface-orientation/index.md) | [main]<br>open class [SurfaceOrientation](-surface-orientation/index.md)<br>Helper used to populate `TANGENTS` buffers. |
| [SwapChain](-swap-chain/index.md) | [main]<br>open class [SwapChain](-swap-chain/index.md)<br>A `SwapChain` represents an Operating System's **native** renderable surface. |
| [SwapChainFlags](-swap-chain-flags/index.md) | [main]<br>class [SwapChainFlags](-swap-chain-flags/index.md)<br>Flags that a `SwapChain` can be created with to control behavior. |
| [Texture](-texture/index.md) | [main]<br>open class [Texture](-texture/index.md)<br>Texture The `Texture` class supports:<br>- 2D textures - 3D textures - Cube maps - mip mapping<br>Usage example<br> A `Texture` object is created using the [Texture.Builder](-texture/-builder/index.md) and destroyed by calling [destroyTexture](-engine/destroy-texture.md). |
| [TextureSampler](-texture-sampler/index.md) | [main]<br>open class [TextureSampler](-texture-sampler/index.md)<br>`TextureSampler` defines how a texture is accessed. |
| [ToneMapper](-tone-mapper/index.md) | [main]<br>open class [ToneMapper](-tone-mapper/index.md)<br>Interface for tone mapping operators. |
| [TransformManager](-transform-manager/index.md) | [main]<br>open class [TransformManager](-transform-manager/index.md)<br>`TransformManager` is used to add transform components to entities. |
| [VertexBuffer](-vertex-buffer/index.md) | [main]<br>open class [VertexBuffer](-vertex-buffer/index.md)<br>Holds a set of buffers that define the geometry of a `Renderable`. |
| [View](-view/index.md) | [main]<br>open class [View](-view/index.md)<br>Encompasses all the state needed for rendering a [Scene](-scene/index.md). |
| [Viewport](-viewport/index.md) | [main]<br>open class [Viewport](-viewport/index.md)<br>Specifies a rectangular region within a render target in terms of pixel coordinates. |
