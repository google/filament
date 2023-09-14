
This tutorial will describe how to create the **suzanne** demo, introducing you to compressed
textures, mipmap generation, asynchronous texture loading, and trackball rotation.

Much like the [previous tutorial], you'll need to use the command-line tools that can be found in
the appropriate [Filament release] for your development machine. In addition to `matc` and `cmgen`,
we'll also be using `filamesh` and `mipgen`.

## Create filamesh file

Filament does not have an asset loading system, but it does provide a binary mesh format
called `filamesh` for simple use cases. Let's create a compressed filamesh file for suzanne by
converting [this OBJ file]:

```bash
filamesh --compress monkey.obj suzanne.filamesh
```

## Create mipmapped textures

Next, let's create mipmapped KTX files using filament's `mipgen` tool. We'll create compressed and
non-compressed variants for each texture, since not all platforms support the same compression
formats. First copy over the PNG files from the [monkey folder], then do:

```bash
# Create mipmaps for base color
mipgen albedo.png albedo.ktx2
mipgen --compression=uastc albedo.png albedo.ktx2

# Create mipmaps for the normal map and a compressed variant.
mipgen --strip-alpha --kernel=NORMALS --linear normal.png normal.ktx
mipgen --strip-alpha --kernel=NORMALS --linear --compression=uastc_normals \
    normal.png normal.ktx2

# Create mipmaps for the single-component roughness map and a compressed variant.
mipgen --grayscale roughness.png roughness.ktx
mipgen --grayscale --compression=uastc roughness.png roughness.ktx2

# Create mipmaps for the single-component metallic map and a compressed variant.
mipgen --grayscale metallic.png metallic.ktx
mipgen --grayscale --compression=uastc metallic.png metallic.ktx2

# Create mipmaps for the single-component occlusion map and a compressed variant.
mipgen --grayscale ao.png ao.ktx
mipgen --grayscale --compression=uastc ao.png ao.ktx2
```

For more information on mipgen's arguments and supported formats, do `mipgen --help`.

In a production setting, you'd want to invoke these commands with a script or build system.

## Bake environment map

Much like the [previous tutorial] we need to use Filament's `cmgen` tool to produce cubemap files.

Download [venetian_crossroads_2k.hdr], then invoke the following commands in your terminal.

```bash
cmgen -x . --format=ktx --size=64 --extract-blur=0.1 venetian_crossroads_2k.hdr
cd venetian* ; mv venetian*_ibl.ktx venetian_crossroads_2k_skybox_tiny.ktx ; cd -

cmgen -x . --format=ktx --size=256 --extract-blur=0.1 venetian_crossroads_2k.hdr
cmgen -x . --format=ktx --size=256 --extract-blur=0.1 venetian_crossroads_2k.hdr
cmgen -x . --format=ktx --size=256 --extract-blur=0.1 venetian_crossroads_2k.hdr
```

## Define textured material

You might recall the `filamat` file we generated in the previous tutorial for red plastic. For this
demo, we'll create a material that uses textures for several parameters.

Create the following text file and call it `textured.mat`. Note that our material definition now
requires a `uv0` attribute.

```text
material {
    name : textured,
    requires : [ uv0 ],
    shadingModel : lit,
    parameters : [
        { type : sampler2d, name : albedo },
        { type : sampler2d, name : roughness },
        { type : sampler2d, name : metallic },
        { type : float, name : clearCoat },
        { type : sampler2d, name : normal },
        { type : sampler2d, name : ao }
    ],
}

fragment {
    void material(inout MaterialInputs material) {
        material.normal = texture(materialParams_normal, getUV0()).xyz * 2.0 - 1.0;
        prepareMaterial(material);
        material.baseColor = texture(materialParams_albedo, getUV0());
        material.roughness = texture(materialParams_roughness, getUV0()).r;
        material.metallic = texture(materialParams_metallic, getUV0()).r;
        material.clearCoat = materialParams.clearCoat;
        material.ambientOcclusion = texture(materialParams_ao, getUV0()).r;
    }
}
```

Next, invoke `matc` as follows.

```bash
matc -a opengl -p mobile -o textured.filamat textured.mat
```

You should now have a material archive in your working directory. For the suzanne asset, the normal
map adds scratches, the albedo map paints the eyes white, and so on. For more information on
materials, consult the official document describing the [Filament Material System].

## Create app skeleton

Create a text file called `suzanne.html` and copy over the HTML that we used in the [previous
tutorial]. Change the last script tag from `redball.js` to `suzanne.js`. Next, create `suzanne.js`
with the following content.

```js {fragment="root"}
// TODO: declare asset URLs

Filament.init([ filamat_url, filamesh_url, sky_small_url, ibl_url ], () => {
    window.app = new App(document.getElementsByTagName('canvas')[0]);
});

class App {
    constructor(canvas) {
        this.canvas = canvas;
        this.engine = Filament.Engine.create(canvas);
        this.scene = this.engine.createScene();

        const material = this.engine.createMaterial(filamat_url);
        this.matinstance = material.createInstance();

        const filamesh = this.engine.loadFilamesh(filamesh_url, this.matinstance);
        this.suzanne = filamesh.renderable;

        // TODO: create sky box and IBL
        // TODO: initialize gltumble
        // TODO: fetch larger assets

        this.swapChain = this.engine.createSwapChain();
        this.renderer = this.engine.createRenderer();
        this.camera = this.engine.createCamera(Filament.EntityManager.get().create());
        this.view = this.engine.createView();
        this.view.setCamera(this.camera);
        this.view.setScene(this.scene);
        this.render = this.render.bind(this);
        this.resize = this.resize.bind(this);
        window.addEventListener('resize', this.resize);

        const eye = [0, 0, 4], center = [0, 0, 0], up = [0, 1, 0];
        this.camera.lookAt(eye, center, up);

        this.resize();
        window.requestAnimationFrame(this.render);
    }

    render() {
        // TODO: apply gltumble matrix
        this.renderer.render(this.swapChain, this.view);
        window.requestAnimationFrame(this.render);
    }

    resize() {
        const dpr = window.devicePixelRatio;
        const width = this.canvas.width = window.innerWidth * dpr;
        const height = this.canvas.height = window.innerHeight * dpr;
        this.view.setViewport([0, 0, width, height]);

        const aspect = width / height;
        const Fov = Filament.Camera$Fov, fov = aspect < 1 ? Fov.HORIZONTAL : Fov.VERTICAL;
        this.camera.setProjectionFov(45, aspect, 1.0, 10.0, fov);
    }
}
```

Our app will only require a subset of assets to be present for `App` construction. We'll download
the other assets after construction. By using a progressive loading strategy, we can reduce the
perceived load time.

Next we need to supply the URLs for various assets. This is actually a bit tricky, because different
clients have different capabilities for compressed textures.

To help you download only the texture assets that you need, Filament provides a
`getSupportedFormatSuffix` function. This takes a space-separated list of desired format types
(`etc`, `s3tc`, or `astc`) that the app developer knows is available from the server. The function
performs an intersection of the *desired* set with the *supported* set, then returns an appropriate
string -- which might be empty.

In our case, we know that our web server will have `astc` and `s3tc` variants for albedo, and `etc`
variants for the other textures. The uncompressed variants (empty string) are always available as a
last resort. Go ahead and replace the **declare asset URLs** comment with the following snippet.

```js {fragment="declare asset URLs"}
const albedo_suffix = Filament.getSupportedFormatSuffix('astc s3tc_srgb');
const texture_suffix = Filament.getSupportedFormatSuffix('etc');

const environ = 'venetian_crossroads_2k'
const ibl_url = `${environ}/${environ}_ibl.ktx`;
const sky_small_url = `${environ}/${environ}_skybox_tiny.ktx`;
const sky_large_url = `${environ}/${environ}_skybox.ktx`;
const albedo_url = `albedo${albedo_suffix}.ktx`;
const ao_url = `ao${texture_suffix}.ktx`;
const metallic_url = `metallic${texture_suffix}.ktx`;
const normal_url = `normal${texture_suffix}.ktx`;
const roughness_url = `roughness${texture_suffix}.ktx`;
const filamat_url = 'textured.filamat';
const filamesh_url = 'suzanne.filamesh';
```

## Create skybox and IBL

Next, let's create the low-resolution skybox and IBL in the `App` constructor.

```js {fragment="create sky box and IBL"}
this.skybox = this.engine.createSkyFromKtx1(sky_small_url);
this.scene.setSkybox(this.skybox);
this.indirectLight = this.engine.createIblFromKtx1(ibl_url);
this.indirectLight.setIntensity(100000);
this.scene.setIndirectLight(this.indirectLight);
```

This allows users to see a reasonable background fairly quickly, before larger assets have finished
loading in.

## Fetch assets asychronously

Next we'll invoke the `Filament.fetch` function from within the app constructor. This function is
very similar to `Filament.init`. It takes a list of asset URLs and a callback function that triggers
when the assets have finished downloading.

In our callback, we'll make several `setTextureParameter` calls on the material instance, then we'll
recreate the skybox using a higher-resolution texture. As a last step we unhide the renderable that
was created in the app constructor.

```js {fragment="fetch larger assets"}
Filament.fetch([sky_large_url, albedo_url, roughness_url, metallic_url, normal_url, ao_url], () => {
    const albedo = this.engine.createTextureFromKtx2(albedo_url, {srgb: true});
    const roughness = this.engine.createTextureFromKtx2(roughness_url);
    const metallic = this.engine.createTextureFromKtx2(metallic_url);
    const normal = this.engine.createTextureFromKtx2(normal_url);
    const ao = this.engine.createTextureFromKtx2(ao_url);

    const sampler = new Filament.TextureSampler(
        Filament.MinFilter.LINEAR_MIPMAP_LINEAR,
        Filament.MagFilter.LINEAR,
        Filament.WrapMode.CLAMP_TO_EDGE);

    this.matinstance.setTextureParameter('albedo', albedo, sampler);
    this.matinstance.setTextureParameter('roughness', roughness, sampler);
    this.matinstance.setTextureParameter('metallic', metallic, sampler);
    this.matinstance.setTextureParameter('normal', normal, sampler);
    this.matinstance.setTextureParameter('ao', ao, sampler);

    // Replace low-res skybox with high-res skybox.
    this.engine.destroySkybox(this.skybox);
    this.skybox = this.engine.createSkyFromKtx1(sky_large_url);
    this.scene.setSkybox(this.skybox);

    this.scene.addEntity(this.suzanne);
});
```

## Introduce trackball rotation

Add the following script tag to your HTML file. This imports a small third-party library that
listens for drag events and computes a rotation matrix.

```html
<script src="//unpkg.com/gltumble"></script>
```

Next, replace the **initialize gltumble** and **apply gltumble matrix** comments with the following
two code snippets.

```js {fragment="initialize gltumble"}
this.trackball = new Trackball(canvas, {startSpin: 0.035});
```

```js {fragment="apply gltumble matrix"}
const tcm = this.engine.getTransformManager();
const inst = tcm.getInstance(this.suzanne);
tcm.setTransform(inst, this.trackball.getMatrix());
inst.delete();
```

That's it, we now have a fast-loading interactive demo. The complete JavaScript file is available
[here](tutorial_suzanne.js).

[Filament release]: //github.com/google/filament/releases
[previous tutorial]: tutorial_redball.html
[Filament Material System]: https://google.github.io/filament/Materials.md.html
[this OBJ file]: https://github.com/google/filament/blob/main/assets/models/monkey/monkey.obj
[monkey folder]: https://github.com/google/filament/blob/main/assets/models/monkey

[venetian_crossroads_2k.hdr]:
//github.com/google/filament/blob/main/third_party/environments/venetian_crossroads_2k.hdr
