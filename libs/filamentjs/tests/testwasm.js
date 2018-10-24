Filament.init([
  'bakedColor.filamat',
  'sandboxLit.filamat',
  'texturedLit.filamat',
  'albedo.ktx',
  'pillars_2k_ibl.ktx',
  'pillars_2k_skybox.ktx',
], () => {
  // Declare convenient aliases to Filament enums at global scope.
  window.VertexAttribute = Filament.VertexAttribute;
  window.AttributeType = Filament.VertexBuffer$AttributeType;
  window.PrimitiveType = Filament.RenderableManager$PrimitiveType;
  window.IndexType = Filament.IndexBuffer$IndexType;
  window.Fov = Filament.Camera$Fov;
  window.LightType = Filament.LightManager$Type;

  // Create the app and store its instance in the global scope to aid debugging.
  window.app = new App();
});

class App {
  constructor() {

    const BAKED_COLOR_PACKAGE = Filament.Buffer(Filament.assets['bakedColor.filamat']);
    const SANDBOX_LIT_PACKAGE = Filament.Buffer(Filament.assets['sandboxLit.filamat']);
    const TEXTURED_LIT_PACKAGE = Filament.Buffer(Filament.assets['texturedLit.filamat']);
    const ALBEDO_KTX = Filament.Buffer(Filament.assets['albedo.ktx']);
    const IBL_KTX = Filament.Buffer(Filament.assets['pillars_2k_ibl.ktx']);
    const SKYBOX_KTX = Filament.Buffer(Filament.assets['pillars_2k_skybox.ktx']);
    const TRIANGLE_POSITIONS = Filament.Buffer(new Float32Array([ 1, 0, -.5, .86, -.5, -.86 ]));
    const TRIANGLE_COLORS = Filament.Buffer(new Uint32Array([0xffff0000, 0xff00ff00, 0xff0000ff]));

    let vb, ib;

    this.canvas = document.getElementsByTagName('canvas')[0];
    const engine = this.engine = Filament.Engine.create(this.canvas);
    this.scene = engine.createScene();

    ////////////////////////////////////////////////////////////////////////////////////////////////

    this.triangle = Filament.EntityManager.get().create();

    vb = Filament.VertexBuffer.Builder()
      .vertexCount(3)
      .bufferCount(2)
      .attribute(VertexAttribute.POSITION, 0, AttributeType.FLOAT2, 0, 8)
      .attribute(VertexAttribute.COLOR, 1, AttributeType.UBYTE4, 0, 4)
      .normalized(VertexAttribute.COLOR)
      .build(engine);
    vb.setBufferAt(engine, 0, TRIANGLE_POSITIONS);
    vb.setBufferAt(engine, 1, TRIANGLE_COLORS);

    ib = Filament.IndexBuffer.Builder()
      .indexCount(3)
      .bufferType(IndexType.USHORT)
      .build(engine);
    ib.setBuffer(engine, Filament.Buffer(new Uint16Array([0, 1, 2])));

    Filament.RenderableManager.Builder(1)
      .boundingBox([ [-1, -1, -1], [1, 1, 1] ])
      .material(0, engine.createMaterial(BAKED_COLOR_PACKAGE).getDefaultInstance())
      .geometry(0, PrimitiveType.TRIANGLES, vb, ib)
      .build(engine, this.triangle);

    ////////////////////////////////////////////////////////////////////////////////////////////////

    this.sphere = new Filament.IcoSphere(5);
    const sphere = Filament.EntityManager.get().create();

    vb = Filament.VertexBuffer.Builder()
      .vertexCount(this.sphere.vertices.length / 3)
      .bufferCount(2)
      .attribute(VertexAttribute.POSITION, 0, AttributeType.FLOAT3, 0, 0)
      .attribute(VertexAttribute.TANGENTS, 1, AttributeType.SHORT4, 0, 0)
      .normalized(VertexAttribute.TANGENTS)
      .build(engine);
    vb.setBufferAt(engine, 0, Filament.Buffer(this.sphere.vertices));
    vb.setBufferAt(engine, 1, Filament.Buffer(this.sphere.tangents));

    ib = Filament.IndexBuffer.Builder()
      .indexCount(this.sphere.triangles.length)
      .bufferType(IndexType.USHORT)
      .build(engine);
    ib.setBuffer(engine, Filament.Buffer(this.sphere.triangles));

    const sandboxMaterial = engine.createMaterial(SANDBOX_LIT_PACKAGE);
    const sandboxMatInstance = sandboxMaterial.createInstance();
    Filament.RenderableManager.Builder(1)
      .boundingBox([ [-1, -1, -1], [1, 1, 1] ])
      .material(0, sandboxMatInstance)
      .geometry(0, PrimitiveType.TRIANGLES, vb, ib)
      .build(engine, sphere);

    ////////////////////////////////////////////////////////////////////////////////////////////////

    this.scene.addEntity(sphere);
    this.sphereEntity = sphere;

    const sunlight = Filament.EntityManager.get().create();
    Filament.LightManager.Builder(LightType.SUN)
            .color([0.98, 0.92, 0.89])
            .intensity(110000.0)
            .direction([0.6, -1.0, -0.8])
            .castShadows(true)
            .sunAngularRadius(1.9)
            .sunHaloSize(10.0)
            .sunHaloFalloff(80.0)
            .build(engine, sunlight);
    this.scene.addEntity(sunlight);

    sandboxMatInstance.setColorParameter("baseColor", Filament.RgbType.sRGB, [0.8, 0.0, 0.0]);
    sandboxMatInstance.setFloatParameter("roughness", 0.5);
    sandboxMatInstance.setFloatParameter("metallic", 0.0);
    sandboxMatInstance.setFloatParameter("reflectance", 0.3);
    sandboxMatInstance.setFloatParameter("clearCoat", 0.7);
    sandboxMatInstance.setFloatParameter("clearCoatRoughness", 0.3);
    sandboxMatInstance.setFloatParameter("anisotropy", 0.0);

    this.swapChain = engine.createSwapChain();
    this.renderer = engine.createRenderer();
    this.camera = engine.createCamera();
    this.view = engine.createView();
    this.view.setCamera(this.camera);
    this.view.setScene(this.scene);
    this.resize();
    this.render = this.render.bind(this);
    this.resize = this.resize.bind(this);
    window.addEventListener("resize", this.resize);
    window.requestAnimationFrame(this.render);

    const sandboxInstance = sandboxMaterial.createInstance();
    sandboxInstance.setFloatParameter("metallic", 1.0)
    sandboxInstance.setFloat3Parameter("baseColor", [1.0, 2.0, 3.0]);

    const format = Filament.PixelDataFormat.RGBM;
    const dtype = Filament.PixelDataType.UBYTE;

    const iblktx = new Filament.KtxBundle(IBL_KTX);
    const iblnmips = iblktx.getNumMipLevels();
    const ibltex = Filament.Texture.Builder()
      .width(iblktx.info().pixelWidth)
      .height(iblktx.info().pixelHeight)
      .levels(iblnmips)
      .sampler(Filament.Texture$Sampler.SAMPLER_CUBEMAP)
      .format(Filament.Texture$InternalFormat.RGBA8)
      .rgbm(true)
      .build(engine);
    for (let level = 0; level < iblnmips; ++level) {
      const ta = iblktx.getCubeBlob(level).getBytes();
      ibltex.setImageCube(engine, level, Filament.PixelBuffer(ta, format, dtype));
    }
    const shstring = iblktx.getMetadata("sh");
    const shfloats = shstring.split(/\s/, 9 * 3).map(parseFloat);
    const indirectLight = Filament.IndirectLight.Builder()
      .reflections(ibltex)
      .irradianceSh(3, shfloats)
      .intensity(30000.0)
      .build(engine);
    indirectLight.setIntensity(50000);
    this.scene.setIndirectLight(indirectLight);

    const skyktx = new Filament.KtxBundle(SKYBOX_KTX);
    const skytex = Filament.Texture.Builder()
      .width(skyktx.info().pixelWidth)
      .height(skyktx.info().pixelHeight)
      .levels(1)
      .sampler(Filament.Texture$Sampler.SAMPLER_CUBEMAP)
      .format(Filament.Texture$InternalFormat.RGBA8)
      .rgbm(true)
      .build(engine);
    const skyta = skyktx.getCubeBlob(0).getBytes();
    skytex.setImageCube(engine, 0, Filament.PixelBuffer(skyta, format, dtype));
    const skybox = Filament.Skybox.Builder()
      .environment(skytex)
      .build(engine);
    this.scene.setSkybox(skybox);

    // Test normal 2D textures.
    (() => {
      const ktx = new Filament.KtxBundle(ALBEDO_KTX);
      const nmips = ktx.getNumMipLevels();
      console.assert(11 == nmips);
      console.assert(1024 == ktx.info().pixelWidth)
      console.assert(1024 == ktx.info().pixelHeight)

      const tex = Filament.Texture.Builder()
        .width(ktx.info().pixelWidth)
        .height(ktx.info().pixelHeight)
        .levels(nmips)
        .sampler(Filament.Texture$Sampler.SAMPLER_2D)
        .format(Filament.Texture$InternalFormat.SRGB8)
        .build(engine);

      const format = Filament.PixelDataFormat.RGB;
      const dtype = Filament.PixelDataType.UBYTE;
      for (let level = 0; level < nmips; ++level) {
        const ta = ktx.getBlob([level, 0, 0]).getBytes();
        const pbd = Filament.PixelBuffer(ta, format, dtype);
        tex.setImage(engine, level, pbd);
      }

      const texturedMaterial = engine.createMaterial(TEXTURED_LIT_PACKAGE);
      const texturedInstance = texturedMaterial.createInstance();
      const sampler = new Filament.TextureSampler(Filament.MinFilter.LINEAR_MIPMAP_LINEAR,
        Filament.MagFilter.LINEAR, Filament.WrapMode.REPEAT);
      texturedInstance.setTextureParameter("albedo", tex, sampler);
    })();
  }

  render() {
    const transform = mat4.fromTranslation(mat4.create(), [0, 0, -4]);
    const tcm = this.engine.getTransformManager();
    tcm.setTransform(tcm.getInstance(this.sphereEntity), transform);
    this.renderer.render(this.swapChain, this.view);
    window.requestAnimationFrame(this.render);
  }

  resize() {
    const dpr = window.devicePixelRatio;
    const width = this.canvas.width = window.innerWidth * dpr;
    const height = this.canvas.height = window.innerHeight * dpr;
    this.view.setViewport([0, 0, width, height]);
    this.camera.setExposure(16.0, 1 / 125.0, 100.0);
    const eye = [0, 0, 0], center = [0, 0, -1], up = [0, 1, 0];
    this.camera.lookAt(eye, center, up);
    const aspect = width / height;
    this.camera.setProjectionFov(45, aspect, 1.0, 10.0, aspect < 1 ? Fov.HORIZONTAL : Fov.VERTICAL);
  }
}
