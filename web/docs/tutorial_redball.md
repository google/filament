
This tutorial will describe how to create the **redball** demo, introducing you to materials and
textures.

For starters, create a text file called `redball.html` and copy over the HTML that we used in the
[previous tutorial]. Change the last script tag from `triangle.js` to `redball.js`.

Next you'll need to get a couple command-line tools: `matc` and `cmgen`. You can find these in the
appropriate [Filament release](//github.com/google/filament/releases). You should choose the
archive that corresponds to your development machine rather than the one for web, and the version
that matches the `unpkg.com/filament@x.x.x` url in the script tag of `redball.html` (you may check
out the last available release of [filament on npm](https://www.npmjs.com/package/filament)).

## Define plastic material

The `matc` tool consumes a text file containing a high-level description of a PBR material, and
produces a binary material package that contains shader code and associated metadata. For more
information, see the official document describing the [Filament Material System].

Let's try out `matc`. Create the following file in your favorite text editor and call it
`plastic.mat`.

```text
material {
    name : Lit,
    shadingModel : lit,
    parameters : [
        { type : float3, name : baseColor },
        { type : float,  name : roughness },
        { type : float,  name : clearCoat },
        { type : float,  name : clearCoatRoughness }
    ],
}

fragment {
    void material(inout MaterialInputs material) {
        prepareMaterial(material);
        material.baseColor.rgb = materialParams.baseColor;
        material.roughness = materialParams.roughness;
        material.clearCoat = materialParams.clearCoat;
        material.clearCoatRoughness = materialParams.clearCoatRoughness;
    }
}
```

Next, invoke `matc` as follows.

```bash
matc -a opengl -p mobile -o plastic.filamat plastic.mat
```

You should now have a material archive in your working directory, which we'll use later in the
tutorial.

## Bake environment map

Next we'll use Filament's `cmgen` tool to consume a HDR environment map in latlong format, and
produce two cubemap files: a mipmapped IBL and a blurry skybox.

Download [pillars_2k.hdr], then invoke the following command in your terminal.

```bash
cmgen -x pillars_2k --format=ktx --size=256 --extract-blur=0.1 pillars_2k.hdr
```

You should now have a `pillars_2k` folder containing a couple KTX files for the IBL and skybox, as
well as a text file with spherical harmonics coefficients. You can discard the text file because the
IBL KTX contains these coefficients in its metadata.

## Create JavaScript

Next, create `redball.js` with the following content.

```js {fragment="root"}
const environ = 'pillars_2k';
const ibl_url = `${environ}/${environ}_ibl.ktx`;
const sky_url = `${environ}/${environ}_skybox.ktx`;
const filamat_url = 'plastic.filamat'

Filament.init([ filamat_url, ibl_url, sky_url ], () => {
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
    // TODO: create lights
    // TODO: create IBL
    // TODO: create skybox

    this.swapChain = engine.createSwapChain();
    this.renderer = engine.createRenderer();
    this.camera = engine.createCamera(Filament.EntityManager.get().create());
    this.view = engine.createView();
    this.view.setCamera(this.camera);
    this.view.setScene(scene);
    this.resize();
    this.render = this.render.bind(this);
    this.resize = this.resize.bind(this);
    window.addEventListener('resize', this.resize);
    window.requestAnimationFrame(this.render);
  }

  render() {
    const eye = [0, 0, 4], center = [0, 0, 0], up = [0, 1, 0];
    const radians = Date.now() / 10000;
    vec3.rotateY(eye, eye, center, radians);
    this.camera.lookAt(eye, center, up);
    this.renderer.render(this.swapChain, this.view);
    window.requestAnimationFrame(this.render);
  }

  resize() {
    const dpr = window.devicePixelRatio;
    const width = this.canvas.width = window.innerWidth * dpr;
    const height = this.canvas.height = window.innerHeight * dpr;
    this.view.setViewport([0, 0, width, height]);
    this.camera.setProjectionFov(45, width / height, 1.0, 10.0, Fov.VERTICAL);
  }
}
```

The above boilerplate should be familiar to you from the previous tutorial, although it loads in a
new set of assets. We also added some animation to the camera.

Next let's create a material instance from the package that we built at the beginning the tutorial.
Replace the **create material** comment with the following snippet.

```js {fragment="create material"}
const material = engine.createMaterial(filamat_url);
const matinstance = material.createInstance();

const red = [0.8, 0.0, 0.0];
matinstance.setColor3Parameter('baseColor', Filament.RgbType.sRGB, red);
matinstance.setFloatParameter('roughness', 0.5);
matinstance.setFloatParameter('clearCoat', 1.0);
matinstance.setFloatParameter('clearCoatRoughness', 0.3);
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

vb.setBufferAt(engine, 0, icosphere.vertices);
vb.setBufferAt(engine, 1, icosphere.tangents);
ib.setBuffer(engine, icosphere.triangles);

Filament.RenderableManager.Builder(1)
  .boundingBox({ center: [-1, -1, -1], halfExtent: [1, 1, 1] })
  .material(0, matinstance)
  .geometry(0, PrimitiveType.TRIANGLES, vb, ib)
  .build(engine, renderable);
```

At this point, the app is rendering a sphere, but it is black so it doesn't show up. To prove that
the sphere is there, you can try changing the background color to blue via `setClearColor`, like we
did in the first tutorial.

## Add lighting

In this section we will create some directional light sources, as well as an image-based light (IBL)
defined by one of the KTX files we built at the start of the demo. First, replace the **create
lights** comment with the following snippet.

```js {fragment="create lights"}
const sunlight = Filament.EntityManager.get().create();
scene.addEntity(sunlight);
Filament.LightManager.Builder(LightType.SUN)
  .color([0.98, 0.92, 0.89])
  .intensity(110000.0)
  .direction([0.6, -1.0, -0.8])
  .sunAngularRadius(1.9)
  .sunHaloSize(10.0)
  .sunHaloFalloff(80.0)
  .build(engine, sunlight);

const backlight = Filament.EntityManager.get().create();
scene.addEntity(backlight);
Filament.LightManager.Builder(LightType.DIRECTIONAL)
        .direction([-1, 0, 1])
        .intensity(50000.0)
        .build(engine, backlight);
```

The `SUN` light source is similar to the `DIRECTIONAL` light source, but has some extra
parameters because Filament will automatically draw a disk into the skybox.

Next we need to create an `IndirectLight` object from the KTX IBL. One way of doing this is the
following (don't type this out, there's an easier way).

```js
const format = Filament.PixelDataFormat.RGB;
const datatype = Filament.PixelDataType.UINT_10F_11F_11F_REV;

// Create a Texture object for the mipmapped cubemap.
const ibl_package = Filament.Buffer(Filament.assets[ibl_url]);
const iblktx = new Filament.Ktx1Bundle(ibl_package);

const ibltex = Filament.Texture.Builder()
  .width(iblktx.info().pixelWidth)
  .height(iblktx.info().pixelHeight)
  .levels(iblktx.getNumMipLevels())
  .sampler(Filament.Texture$Sampler.SAMPLER_CUBEMAP)
  .format(Filament.Texture$InternalFormat.RGBA8)
  .build(engine);

for (let level = 0; level < iblktx.getNumMipLevels(); ++level) {
  const uint8array = iblktx.getCubeBlob(level).getBytes();
  const pixelbuffer = Filament.PixelBuffer(uint8array, format, datatype);
  ibltex.setImageCube(engine, level, pixelbuffer);
}

// Parse the spherical harmonics metadata.
const shstring = iblktx.getMetadata('sh');
const shfloats = shstring.split(/\s/, 9 * 3).map(parseFloat);

// Build the IBL object and insert it into the scene.
const indirectLight = Filament.IndirectLight.Builder()
  .reflections(ibltex)
  .irradianceSh(3, shfloats)
  .intensity(50000.0)
  .build(engine);

scene.setIndirectLight(indirectLight);
```

Filament provides a JavaScript utility to make this simpler,
simply replace the **create IBL** comment with the following snippet.

```js {fragment="create IBL"}
const indirectLight = engine.createIblFromKtx1(ibl_url);
indirectLight.setIntensity(50000);
scene.setIndirectLight(indirectLight);
```

## Add background

At this point you can run the demo and you should see a red plastic ball against a black background.
Without a skybox, the reflections on the ball are not representative of its surroundings.
Here's one way to create a texture for the skybox:

```js
const sky_package = Filament.Buffer(Filament.assets[sky_url]);
const skyktx = new Filament.Ktx1Bundle(sky_package);
const skytex = Filament.Texture.Builder()
  .width(skyktx.info().pixelWidth)
  .height(skyktx.info().pixelHeight)
  .levels(1)
  .sampler(Filament.Texture$Sampler.SAMPLER_CUBEMAP)
  .format(Filament.Texture$InternalFormat.RGBA8)
  .build(engine);

const uint8array = skyktx.getCubeBlob(0).getBytes();
const pixelbuffer = Filament.PixelBuffer(uint8array, format, datatype);
skytex.setImageCube(engine, 0, pixelbuffer);
```

Filament provides a Javascript utility to make this easier.
Replace **create skybox** with the following.

```js {fragment="create skybox"}
const skybox = engine.createSkyFromKtx1(sky_url);
scene.setSkybox(skybox);
```

That's it, we now have a shiny red ball floating in an environment! The complete JavaScript file is
available [here](tutorial_redball.js).

In the [next tutorial], we'll take a closer look at textures and interaction.

[pillars_2k.hdr]:
//github.com/google/filament/blob/main/third_party/environments/pillars_2k.hdr

[next tutorial]: tutorial_suzanne.html
[previous tutorial]: tutorial_triangle.html
[Filament release]: //github.com/google/filament/releases
[Filament Material System]: https://google.github.io/filament/Materials.md.html
