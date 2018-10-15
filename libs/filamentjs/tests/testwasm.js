class App {
  constructor() {
    const BAKED_COLOR_PACKAGE = Filament.Buffer(Filament.assets['bakedColor.filamat']);
    const SANDBOX_LIT_PACKAGE = Filament.Buffer(Filament.assets['sandboxLit.filamat']);
    const TEXTURED_LIT_PACKAGE = Filament.Buffer(Filament.assets['texturedLit.filamat']);
    const ALBEDO_KTX = Filament.Buffer(Filament.assets['albedo.ktx']);

    this.canvas = document.getElementsByTagName('canvas')[0];
    const engine = this.engine = Filament.Engine.create(this.canvas);
    this.scene = engine.createScene();
    this.triangle = Filament.EntityManager.get()
      .create();
    this.scene.addEntity(this.triangle);
    const TRIANGLE_POSITIONS = Filament.Buffer(new Float32Array([
      1, 0,
      Math.cos(Math.PI * 2 / 3), Math.sin(Math.PI * 2 / 3),
      Math.cos(Math.PI * 4 / 3), Math.sin(Math.PI * 4 / 3),
    ]));
    const TRIANGLE_COLORS = Filament.Buffer(new Uint32Array([
      0xffff0000,
      0xff00ff00,
      0xff0000ff,
    ]));
    const VertexAttribute = Filament.VertexAttribute;
    const AttributeType = Filament.VertexBuffer$AttributeType;
    this.vb = Filament.VertexBuffer.Builder()
      .vertexCount(3)
      .bufferCount(2)
      .attribute(VertexAttribute.POSITION, 0, AttributeType.FLOAT2, 0, 8)
      .attribute(VertexAttribute.COLOR, 1, AttributeType.UBYTE4, 0, 4)
      .normalized(VertexAttribute.COLOR)
      .build(engine);
    this.vb.setBufferAt(engine, 0, TRIANGLE_POSITIONS);
    this.vb.setBufferAt(engine, 1, TRIANGLE_COLORS);
    this.ib = Filament.IndexBuffer.Builder()
      .indexCount(3)
      .bufferType(Filament.IndexBuffer$IndexType.USHORT)
      .build(engine);
    this.ib.setBuffer(engine, Filament.Buffer(new Uint16Array([0, 1, 2])));

    const sandboxMaterial = engine.createMaterial(SANDBOX_LIT_PACKAGE);
    const sandboxInstance = sandboxMaterial.createInstance();
    sandboxInstance.setFloatParameter("metallic", 1.0)
    sandboxInstance.setFloat3Parameter("baseColor", [1.0, 2.0, 3.0]);

    const ktx = Window.ktx = new Filament.KtxBundle(ALBEDO_KTX)
    const nmips = ktx.getNumMipLevels();
    const ta = ktx.getBlob([0, 0, 0]).getBytes();
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
      tex.setImage(engine, level, pbd)
    }

    const texturedMaterial = engine.createMaterial(TEXTURED_LIT_PACKAGE);
    const texturedInstance = texturedMaterial.createInstance();
    const sampler = new Filament.TextureSampler(Filament.MinFilter.LINEAR_MIPMAP_LINEAR,
      Filament.MagFilter.LINEAR, Filament.WrapMode.REPEAT);
    texturedInstance.setTextureParameter("albedo", tex, sampler);

    const mat = engine.createMaterial(BAKED_COLOR_PACKAGE);
    const matinst = mat.getDefaultInstance();
    Filament.RenderableManager.Builder(1)
      .boundingBox([
        [-1, -1, -1],
        [1, 1, 1]
      ])
      .material(0, matinst)
      .geometry(0, Filament.RenderableManager$PrimitiveType.TRIANGLES, this.vb, this.ib)
      .build(engine, this.triangle);

    this.swapChain = engine.createSwapChain();
    this.renderer = engine.createRenderer();
    this.camera = engine.createCamera();
    this.view = engine.createView();
    this.view.setCamera(this.camera);
    this.view.setScene(this.scene);
    this.view.setClearColor([0.1, 0.2, 0.3, 1.0]); // blue-green background
    this.resize(); // adjust the initial viewport
    this.render = this.render.bind(this);
    this.resize = this.resize.bind(this);
    window.addEventListener("resize", this.resize);
    window.requestAnimationFrame(this.render);
  }

  render() {
    // Test setting transforms.
    const transform = [
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1];
    const tcm = this.engine.getTransformManager();
    tcm.setTransform(tcm.getInstance(this.triangle), transform);

    // Render the frame.
    if (this.renderer.beginFrame(this.swapChain)) {
      this.renderer.render(this.view);
      this.renderer.endFrame();
    }
    this.engine.execute();
    window.requestAnimationFrame(this.render);
  }

  resize() {
    const dpr = window.devicePixelRatio;
    const width = this.canvas.width = window.innerWidth * dpr;
    const height = this.canvas.height = window.innerHeight * dpr;
    this.view.setViewport([0, 0, width, height]);
    const aspect = width / height;
    const Projection = Filament.Camera$Projection;
    this.camera.setProjection(Projection.ORTHO, -aspect, aspect, -1, 1, 0, 1);
  }
}

Filament.init([
  'bakedColor.filamat',
  'sandboxLit.filamat',
  'texturedLit.filamat',
  'albedo.ktx',
], () => { Window.app = new App() });
