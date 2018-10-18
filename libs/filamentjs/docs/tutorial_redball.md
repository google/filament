## Create materials and textures

TODO: Describe how to use `matc` and `cmgen` to create `plastic.filamat` and the two `pillars_2k`
KTX files.

## Start your project

Create a text file called `redball.html` and fill it with the same HTML you used in the [previous
tutorial](tutorial_triangle.html) but change the last script tag from `triangle.js` to
`redball.js`.

Next, create `redball.js` with the following content.

```js {fragment="root"}
Filament.loadMathExtensions();

Filament.init([ 'plastic.filamat', 'pillars_2k_ibl.ktx', 'pillars_2k_skybox.ktx' ], () => {
  // Create some global aliases to enums for convenience.
  window.VertexAttribute = Filament.VertexAttribute;
  window.AttributeType = Filament.VertexBuffer$AttributeType;
  window.PrimitiveType = Filament.RenderableManager$PrimitiveType;
  window.IndexType = Filament.IndexBuffer$IndexType;
  window.Fov = Filament.Camera$Fov;
  window.LightType = Filament.LightManager$Type;

  // Obtain the canvas DOM object and pass it to the App.
  const canvas = document.getElementsByTagName('canvas')[0];
  window.app = new App(canvas);
} );

class App {
  constructor(canvas) {
    this.canvas = canvas;
    const engine = this.engine = Filament.Engine.create(canvas);
    const scene = engine.createScene();

    // TODO: create material
    // TODO: create sphere
    // TODO: create sunlight
    // TODO: create IBL
    // TODO: create skybox

    this.swapChain = engine.createSwapChain();
    this.renderer = engine.createRenderer();
    this.camera = engine.createCamera();
    this.view = engine.createView();
    this.view.setCamera(this.camera);
    this.view.setScene(scene);
    this.resize();
    this.render = this.render.bind(this);
    this.resize = this.resize.bind(this);
    window.addEventListener("resize", this.resize);
    window.requestAnimationFrame(this.render);
  }

  render() {
    if (this.renderer.beginFrame(this.swapChain)) {
      this.renderer.render(this.view);
      this.renderer.endFrame();
    }
    this.engine.execute();
    window.requestAnimationFrame(this.render);
  }

  resize() {
    // Adjust the canvas resolution and Filament viewport.
    const dpr = window.devicePixelRatio;
    const width = this.canvas.width = window.innerWidth * dpr;
    const height = this.canvas.height = window.innerHeight * dpr;
    this.view.setViewport([0, 0, width, height]);

    // Adjust the camera frustum.
    const eye = [0, 0, 0], center = [0, 0, -1], up = [0, 1, 0];
    this.camera.lookAt(eye, center, up);
    this.camera.setProjectionFov(45, width / height, 1.0, 10.0, Fov.VERTICAL);
  }
}
```

The above boilerplate should be familiar to you from the previous tutorial, although it loads in a
new set of assets and the camera uses a perspective projection.

Next let's create a material instance from the package that we built at the beginning the tutorial.
Replace the **create material** todo with the following snippet.

```js {fragment="create material"}
const material_package = Filament.Buffer(Filament.assets['plastic.filamat']);
const material = engine.createMaterial(material_package);
const matinstance = material.createInstance();

const red = [0.8, 0.0, 0.0];
matinstance.setColorParameter("baseColor", Filament.RgbType.sRGB, red);
matinstance.setFloatParameter("roughness", 0.5);
matinstance.setFloatParameter("reflectance", 0.5);
matinstance.setFloatParameter("clearCoat", 1.0);
matinstance.setFloatParameter("clearCoatRoughness", 0.3);
```

The next step is to create a renderable for the sphere. To help with this, we'll use the `IcoSphere`
utility class, whose constructor takes a LOD. Its job is to subdivide an icosadedron, producing
three arrays:

- `icosphere.vertices` Float32Array of XYZ coordinates.
- `icosphere.tangents` Uint16Array (interpreted as half-floats) encoding the surface orientation
  as quaternions.
- `icosphere.triangles` Uint16Array with triangle indices.

Let's go ahead use these arrays to build the vertex buffer and index buffer. Replace **create
sphere** with the following snippet.

```js {fragment="create sphere"}
const renderable = Filament.EntityManager.get().create();
scene.addEntity(renderable);

const icosphere = new Filament.IcoSphere(5);

const vb = Filament.VertexBuffer.Builder()
  .vertexCount(icosphere.vertices.length / 3)
  .bufferCount(2)
  .attribute(VertexAttribute.POSITION, 0, AttributeType.FLOAT3, 0, 0)
  .attribute(VertexAttribute.TANGENTS, 1, AttributeType.SHORT4, 0, 0)
  .normalized(VertexAttribute.TANGENTS)
  .build(engine);

const ib = Filament.IndexBuffer.Builder()
  .indexCount(icosphere.triangles.length)
  .bufferType(IndexType.USHORT)
  .build(engine);

vb.setBufferAt(engine, 0, Filament.Buffer(icosphere.vertices));
vb.setBufferAt(engine, 1, Filament.Buffer(icosphere.tangents));
ib.setBuffer(engine, Filament.Buffer(icosphere.triangles));

Filament.RenderableManager.Builder(1)
  .boundingBox([ [-1, -1, -1], [1, 1, 1] ])
  .material(0, matinstance)
  .geometry(0, PrimitiveType.TRIANGLES, vb, ib)
  .build(engine, renderable);

const transform = mat4.fromTranslation(mat4.create(), [0, 0, -4]);
const tcm = this.engine.getTransformManager();
tcm.setTransform(tcm.getInstance(renderable), transform);
```

At this point, the app is rendering a sphere, but it is black so it doesn't show up. To prove that
the sphere is there, you can try changing the background color to blue via `setClearColor`, like we
did in the first tutorial.

The next step is to add some lighting. We'll be creating two types of light sources: a directional
light source that represents the sun, and an image-based light (IBL) defined by one of the KTX files
we built at the start of the demo. First, replace the **create sunlight** todo with the following
snippet.

```js {fragment="create sunlight"}
const sunlight = Filament.EntityManager.get().create();
scene.addEntity(sunlight);

Filament.LightManager.Builder(LightType.SUN)
  .color([0.98, 0.92, 0.89])
  .intensity(110000.0)
  .direction([0.6, -1.0, -0.8])
  .castShadows(true)
  .sunAngularRadius(1.9)
  .sunHaloSize(10.0)
  .sunHaloFalloff(80.0)
  .build(engine, sunlight);
```

We are using a light type of `SUN`, which is similar to `DIRECTIONAL`, but it has some extra
parameters because Filament will automatically draw a disk into the skybox.

Next let's create a `IndirectLight` object from the KTX IBL. One way of doing this is the following
(don't type this out, there's an easier way).

```js {fragment="create IBL"}
const format = Filament.PixelDataFormat.RGBM;
const datatype = Filament.PixelDataType.UBYTE;

// Create a Texture object for the mipmapped cubemap.
const ibl_package = Filament.Buffer(Filament.assets['pillars_2k_ibl.ktx']);
const iblktx = new Filament.KtxBundle(ibl_package);
const ibltex = Filament.Texture.Builder()
  .width(iblktx.info().pixelWidth)
  .height(iblktx.info().pixelHeight)
  .levels(iblktx.getNumMipLevels())
  .sampler(Filament.Texture$Sampler.SAMPLER_CUBEMAP)
  .format(Filament.Texture$InternalFormat.RGBA8)
  .rgbm(true)
  .build(engine);
for (let level = 0; level < iblktx.getNumMipLevels(); ++level) {
  const uint8array = iblktx.getCubeBlob(level).getBytes();
  const pixelbuffer = Filament.PixelBuffer(uint8array, format, datatype);
  ibltex.setImageCube(engine, level, pixelbuffer);
}

// Parse the spherical harmonics metadata.
const shstring = iblktx.getMetadata("sh");
const shfloats = shstring.split(/\s/, 9 * 3).map(parseFloat);

// Build the IBL object and insert it into the scene.
const indirectLight = Filament.IndirectLight.Builder()
  .reflections(ibltex)
  .irradianceSh(3, shfloats)
  .intensity(50000.0)
  .build(engine);

scene.setIndirectLight(indirectLight);
```

This is a lot of boilerplate, so Filament provides a JavaScript utilitiy to make this simpler;
simply replace the **create IBL** todo with the following snippet. *NOTE: not yet implemented.*

```js
const ibl_package = Filament.Buffer(Filament.assets['pillars_2k_ibl.ktx']);
const indirectLight = Filament.createIblFromKtx(ibl_package);
indirectLight.setIntensity(50000);
scene.setIndirectLight(indirectLight);
```

At the point you can run the demo and you should see a red plastic ball against a black background.
Without a skybox, the reflections on the ball aren't truly representative of the its surroundings.
Here's one way to create a texture for the skybox:

```js {fragment="create skybox"}
const sky_package = Filament.Buffer(Filament.assets['pillars_2k_skybox.ktx']);
const skyktx = new Filament.KtxBundle(sky_package);
const skytex = Filament.Texture.Builder()
  .width(skyktx.info().pixelWidth)
  .height(skyktx.info().pixelHeight)
  .levels(1)
  .sampler(Filament.Texture$Sampler.SAMPLER_CUBEMAP)
  .format(Filament.Texture$InternalFormat.RGBA8)
  .rgbm(true)
  .build(engine);

const uint8array = skyktx.getCubeBlob(0).getBytes();
const pixelbuffer = Filament.PixelBuffer(uint8array, format, datatype);
skytex.setImageCube(engine, 0, pixelbuffer);
```

Again, this is a lot of boilerplate, so Filament provides a Javascript utility for you. Replace
**create skybox** with the following. *NOTE: not yet implemented.*

```js
const sky_package = Filament.Buffer(Filament.assets['pillars_2k_skybox.ktx']);
const skytex = Filament.createTextureFromKtx(sky_package, {'rgbm': True});
```
```js {fragment="create skybox"}
const skybox = Filament.Skybox.Builder().environment(skytex).build(engine);
scene.setSkybox(skybox);
```

This completes the tutorial; the completed JavaScript is available [here](tutorial_redball.js).
