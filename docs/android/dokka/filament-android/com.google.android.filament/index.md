//[filament-android](../../index.md)/[com.google.android.filament](index.md)

# Package-level declarations

## Types

| Name | Summary |
|---|---|
| [ACESLegacyToneMapper](-a-c-e-s-legacy-tone-mapper/index.md) | [main]<br>open class [ACESLegacyToneMapper](-a-c-e-s-legacy-tone-mapper/index.md) : [ToneMapper](-tone-mapper/index.md)<br>ACES tone mapping operator, modified to match the perceived brightness of FilmicToneMapper. |
| [ACESToneMapper](-a-c-e-s-tone-mapper/index.md) | [main]<br>open class [ACESToneMapper](-a-c-e-s-tone-mapper/index.md) : [ToneMapper](-tone-mapper/index.md)<br>ACES tone mapping operator. |
| [AgxToneMapper](-agx-tone-mapper/index.md) | [main]<br>open class [AgxToneMapper](-agx-tone-mapper/index.md) : [ToneMapper](-tone-mapper/index.md)<br>AgX tone mapping operator. |
| [Box](-box/index.md) | [main]<br>open class [Box](-box/index.md)<br>An axis aligned 3D box represented by its center and half-extent. |
| [BufferObject](-buffer-object/index.md) | [main]<br>open class [BufferObject](-buffer-object/index.md)<br>A generic GPU buffer containing data. |
| [Camera](-camera/index.md) | [main]<br>open class [Camera](-camera/index.md)<br>Camera represents the eye(s) through which the scene is viewed. |
| [ColorGrading](-color-grading/index.md) | [main]<br>open class [ColorGrading](-color-grading/index.md)<br>ColorGrading is used to transform (either to modify or correct) the colors of the HDR buffer rendered by Filament. |
| [Colors](-colors/index.md) | [main]<br>open class [Colors](-colors/index.md)<br>Utilities to manipulate and convert colors |
| [DisplayRangeToneMapper](-display-range-tone-mapper/index.md) | [main]<br>open class [DisplayRangeToneMapper](-display-range-tone-mapper/index.md) : [ToneMapper](-tone-mapper/index.md)<br>A tone mapper that converts the input HDR RGB color into one of 16 debug colors that represent the pixel's exposure. |
| [Engine](-engine/index.md) | [main]<br>open class [Engine](-engine/index.md)<br>Engine is filament's main entry-point. |
| [Entity](-entity/index.md) | [main]<br>@[Retention](https://developer.android.com/reference/kotlin/java/lang/annotation/Retention.html)(value = [CLASS](https://developer.android.com/reference/kotlin/java/lang/annotation/RetentionPolicy.html#CLASS))<br>@[Target](https://developer.android.com/reference/kotlin/java/lang/annotation/Target.html)(value = [])<br>annotation class [Entity](-entity/index.md) |
| [EntityInstance](-entity-instance/index.md) | [main]<br>@[Retention](https://developer.android.com/reference/kotlin/java/lang/annotation/Retention.html)(value = [CLASS](https://developer.android.com/reference/kotlin/java/lang/annotation/RetentionPolicy.html#CLASS))<br>@[Target](https://developer.android.com/reference/kotlin/java/lang/annotation/Target.html)(value = [])<br>annotation class [EntityInstance](-entity-instance/index.md) |
| [EntityManager](-entity-manager/index.md) | [main]<br>open class [EntityManager](-entity-manager/index.md)<br>Thread-safe coordinator managing the lifecycle, indices, and generation tracking for all Entity instances. |
| [Exposure](-exposure/index.md) | [main]<br>open class [Exposure](-exposure/index.md)<br>A series of utilities to compute exposure, exposure value at ISO 100 (EV100), luminance and illuminance using a physically-based camera model. |
| [Fence](-fence/index.md) | [main]<br>open class [Fence](-fence/index.md)<br>Fence is used to synchronize the application main thread with filament's rendering thread. |
| [Filament](-filament/index.md) | [main]<br>open class [Filament](-filament/index.md) |
| [FilmicToneMapper](-filmic-tone-mapper/index.md) | [main]<br>open class [FilmicToneMapper](-filmic-tone-mapper/index.md) : [ToneMapper](-tone-mapper/index.md)<br>&quot;Filmic&quot; tone mapping operator. |
| [FramePacer](-frame-pacer/index.md) | [main]<br>open class [FramePacer](-frame-pacer/index.md)<br>FramePacer Coordinates frame scheduling and presentation timestamps across multi-threaded rendering architectures. |
| [Frustum](-frustum/index.md) | [main]<br>open class [Frustum](-frustum/index.md)<br>A frustum defined by six planes |
| [GenericToneMapper](-generic-tone-mapper/index.md) | [main]<br>open class [GenericToneMapper](-generic-tone-mapper/index.md) : [ToneMapper](-tone-mapper/index.md)<br>Generic tone mapping operator that gives control over the tone mapping curve. |
| [GT7ToneMapper](-g-t7-tone-mapper/index.md) | [main]<br>open class [GT7ToneMapper](-g-t7-tone-mapper/index.md) : [ToneMapper](-tone-mapper/index.md)<br>Gran Turismo 7 tone mapping operator. |
| [IndexBuffer](-index-buffer/index.md) | [main]<br>open class [IndexBuffer](-index-buffer/index.md)<br>A buffer containing vertex indices into a VertexBuffer. |
| [IndirectLight](-indirect-light/index.md) | [main]<br>open class [IndirectLight](-indirect-light/index.md)<br>IndirectLight is used to simulate environment lighting, a form of global illumination. |
| [InstanceBuffer](-instance-buffer/index.md) | [main]<br>open class [InstanceBuffer](-instance-buffer/index.md)<br>InstanceBuffer holds draw (GPU) instance transforms. |
| [LightManager](-light-manager/index.md) | [main]<br>open class [LightManager](-light-manager/index.md)<br>LightManager allows to create a light source in the scene, such as a sun or street lights. |
| [LinearToneMapper](-linear-tone-mapper/index.md) | [main]<br>open class [LinearToneMapper](-linear-tone-mapper/index.md) : [ToneMapper](-tone-mapper/index.md)<br>Linear tone mapping operator that returns the input color but clamped to the 0..1 range. |
| [Material](-material/index.md) | [main]<br>open class [Material](-material/index.md)<br>A Material defines the visual appearance of a surface. |
| [MaterialInstance](-material-instance/index.md) | [main]<br>open class [MaterialInstance](-material-instance/index.md)<br>A MaterialInstance represents a specific instance of a Material. |
| [MathUtils](-math-utils/index.md) | [main]<br>class [MathUtils](-math-utils/index.md) |
| [MorphTargetBuffer](-morph-target-buffer/index.md) | [main]<br>open class [MorphTargetBuffer](-morph-target-buffer/index.md)<br>A container for vertex morphing data that supports both automatic and manual morphing. |
| [NativeSurface](-native-surface/index.md) | [main]<br>open class [NativeSurface](-native-surface/index.md) |
| [PBRNeutralToneMapper](-p-b-r-neutral-tone-mapper/index.md) | [main]<br>open class [PBRNeutralToneMapper](-p-b-r-neutral-tone-mapper/index.md) : [ToneMapper](-tone-mapper/index.md)<br>Khronos PBR Neutral tone mapping operator. |
| [PixelBufferDescriptor](-pixel-buffer-descriptor/index.md) | [main]<br>open class [PixelBufferDescriptor](-pixel-buffer-descriptor/index.md)<br>A descriptor to an image in main memory, typically used to transfer image data from the CPU to the GPU. |
| [RenderableManager](-renderable-manager/index.md) | [main]<br>open class [RenderableManager](-renderable-manager/index.md)<br>Factory and manager for \em renderables, which are entities that can be drawn. |
| [Renderer](-renderer/index.md) | [main]<br>open class [Renderer](-renderer/index.md)<br>A Renderer instance represents an operating system's window. |
| [RenderTarget](-render-target/index.md) | [main]<br>open class [RenderTarget](-render-target/index.md)<br>An offscreen render target that can be associated with a View and contains weak references to a set of attached Texture objects. |
| [Scene](-scene/index.md) | [main]<br>open class [Scene](-scene/index.md)<br>A Scene is a flat container of Renderable and Light instances. |
| [SkinningBuffer](-skinning-buffer/index.md) | [main]<br>open class [SkinningBuffer](-skinning-buffer/index.md)<br>SkinningBuffer is used to hold skinning data (bones). |
| [Skybox](-skybox/index.md) | [main]<br>open class [Skybox](-skybox/index.md)<br>Skybox When added to a Scene, the Skybox fills all untouched pixels. |
| [Stream](-stream/index.md) | [main]<br>open class [Stream](-stream/index.md)<br>Stream is used to attach a video stream to a Filament `Texture`. |
| [SurfaceOrientation](-surface-orientation/index.md) | [main]<br>open class [SurfaceOrientation](-surface-orientation/index.md)<br>Helper used to populate `TANGENTS` buffers. |
| [SwapChain](-swap-chain/index.md) | [main]<br>open class [SwapChain](-swap-chain/index.md)<br>A swap chain represents an Operating System's *native* renderable surface. |
| [SwapChainFlags](-swap-chain-flags/index.md) | [main]<br>class [SwapChainFlags](-swap-chain-flags/index.md)<br>Flags that a `SwapChain` can be created with to control behavior. |
| [Texture](-texture/index.md) | [main]<br>open class [Texture](-texture/index.md)<br>Texture The Texture class supports:<br>- 2D textures - 3D textures - Cube maps - mip mapping<br>Creation and destruction<br>A Texture object is created using the Texture::Builder and destroyed by calling Engine::destroy(const Texture*). |
| [TextureSampler](-texture-sampler/index.md) | [main]<br>open class [TextureSampler](-texture-sampler/index.md)<br>TextureSampler defines how a texture is accessed. |
| [ToneMapper](-tone-mapper/index.md) | [main]<br>open class [ToneMapper](-tone-mapper/index.md)<br>Interface for tone mapping operators. |
| [TransformManager](-transform-manager/index.md) | [main]<br>open class [TransformManager](-transform-manager/index.md)<br>TransformManager is used to add transform components to entities. |
| [VertexBuffer](-vertex-buffer/index.md) | [main]<br>open class [VertexBuffer](-vertex-buffer/index.md)<br>Holds a set of buffers that define the geometry of a Renderable. |
| [View](-view/index.md) | [main]<br>open class [View](-view/index.md)<br>A View encompasses all the state needed for rendering a Scene. |
| [Viewport](-viewport/index.md) | [main]<br>open class [Viewport](-viewport/index.md)<br>Viewport describes a view port in pixel coordinates A view port is represented by its left-bottom coordinate, width and height in pixels. |
