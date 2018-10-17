## Create materials and textures

TODO: Describe how to use `matc` and `mipgen` to create `simple.filamat` and the two `pillars_2k`
KTX files.

## Start your project

Create a text file called `redball.html` and fill it with the same HTML you used in the
[previous tutorial]() but change the last `<script>` src from `triangle.js` to `redball.js`.

Next, create `redball.js` with the following content.

```js {fragment="root"}
Filament.loadMathExtensions();

Filament.init([ 'redball.filamat', 'pillars_2k_ibl.ktx', 'pillars_2k_skybox.ktx' ], () => {
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

TODO: Verbiage

```js {fragment="create material"}
const material_package = Filament.Buffer(Filament.assets['redball.filamat']);
const material = engine.createMaterial(material_package);
const matinstance = material.createInstance();

const red = [0.8, 0.0, 0.0];
matinstance.setColorParameter("baseColor", Filament.RgbType.sRGB, red);
matinstance.setFloatParameter("roughness", 0.5);
matinstance.setFloatParameter("reflectance", 0.3);
matinstance.setFloatParameter("clearCoat", 0.7);
matinstance.setFloatParameter("clearCoatRoughness", 0.3);
```

TODO: Verbiage

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

TODO: Verbiage

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

TODO: Verbiage

```js {fragment="create IBL"}
const format = Filament.PixelDataFormat.RGBM;
const datatype = Filament.PixelDataType.UBYTE;

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

const shstring = iblktx.getMetadata("sh");
const shfloats = shstring.split(/\s/, 9 * 3).map(parseFloat);
const indirectLight = Filament.IndirectLight.Builder()
  .reflections(ibltex)
  .irradianceSh(3, shfloats)
  .intensity(30000.0)
  .build(engine);

indirectLight.setIntensity(50000);
scene.setIndirectLight(indirectLight);
```

TODO: Verbiage

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

const skybox = Filament.Skybox.Builder()
  .environment(skytex)
  .build(engine);

scene.setSkybox(skybox);
```

TODO: Verbiage
