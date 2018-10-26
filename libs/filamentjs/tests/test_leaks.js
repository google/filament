Filament.init([ 'bakedColor.filamat' ], () => {
  window.VertexAttribute = Filament.VertexAttribute;
  window.AttributeType = Filament.VertexBuffer$AttributeType;
  window.PrimitiveType = Filament.RenderableManager$PrimitiveType;
  window.IndexType = Filament.IndexBuffer$IndexType;
  window.Fov = Filament.Camera$Fov;
  window.LightType = Filament.LightManager$Type;
  const canvas = document.getElementsByTagName('canvas')[0];
  window.app = new App(canvas);
});

class App {
  constructor(canvas) {
    const BAKED_COLOR_PACKAGE = Filament.Buffer(Filament.assets['bakedColor.filamat']);
    this.canvas = canvas;
    this.engine = Filament.Engine.create(this.canvas);
    this.mi = this.engine.createMaterial(BAKED_COLOR_PACKAGE).getDefaultInstance();
    const engine = this.engine;
    this.polygon = Filament.EntityManager.get().create();
    this.createPolygon(this.polygon, 3, this.mi);
    this.scene = engine.createScene();
    this.scene.addEntity(this.polygon);
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
  }

  render() {
    this.animate();
    this.renderer.render(this.swapChain, this.view);
    window.requestAnimationFrame(this.render);
  }

  resize() {
    const width = this.canvas.width;
    const height = this.canvas.height;
    this.view.setViewport([0, 0, width, height]);
    const eye = [0, 0, 3], center = [0, 0, -1], up = [0, 1, 0];
    this.camera.lookAt(eye, center, up);
    const aspect = width / height;
    this.camera.setProjectionFov(45, aspect, 1.0, 10.0, aspect < 1 ? Fov.HORIZONTAL : Fov.VERTICAL);
  }

  createPolygon(polygon, nsides, minstance) {
    const engine = this.engine;
    const nverts = nsides + 1;
    const ntris = nsides;
    const dtheta = Math.PI * 2 / nsides;
    // Create typed arrays.
    const indices = new Uint16Array(3 * ntris);
    const positions = new Float32Array(2 * nverts);
    const colors = new Uint32Array(nverts);
    const palette = new Uint32Array([0xffff0000, 0xff00ff00, 0xff0000ff]);
    for (let i = 1, j = 0, theta = 0; i < nverts; i++, theta += dtheta) {
      positions[i * 2] = Math.cos(theta);
      positions[i * 2 + 1] = Math.sin(theta);
      colors[i] = palette[i % 3];
      indices[j++] = 0;
      indices[j++] = i;
      indices[j++] = 1 + (i % (nverts - 1));
    }
    // Create vertex buffer.
    const vbuilder = Filament.VertexBuffer.Builder()
      .vertexCount(nverts)
      .bufferCount(2)
      .attribute(VertexAttribute.POSITION, 0, AttributeType.FLOAT2, 0, 8)
      .attribute(VertexAttribute.COLOR, 1, AttributeType.UBYTE4, 0, 4)
      .normalized(VertexAttribute.COLOR);
    const vb = vbuilder.build(engine);
    vbuilder.delete();
    const pbuf = Filament.Buffer(positions);
    const cbuf = Filament.Buffer(colors);
    vb.setBufferAt(engine, 0, pbuf);
    vb.setBufferAt(engine, 1, cbuf);
    pbuf.delete();
    cbuf.delete();
    // Create index buffer.
    const ibuilder = Filament.IndexBuffer.Builder()
      .indexCount(indices.length)
      .bufferType(IndexType.USHORT);
    const ib = ibuilder.build(engine);
    ibuilder.delete();
    const ibuf = Filament.Buffer(indices);
    ib.setBuffer(engine, ibuf);
    ibuf.delete();
    // Create renderable component.
    const rbuilder = Filament.RenderableManager.Builder(1)
      .boundingBox([ [-1, -1, -1], [1, 1, 1] ])
      .material(0, minstance)
      .geometry(0, PrimitiveType.TRIANGLES, vb, ib);
    rbuilder.build(engine, polygon);
    rbuilder.delete();
    vb.delete();
    ib.delete();
  }

  animate() {
    // Recreate the polygon every couple frames, changing the number of sides.
    this.frame = (this.frame | 0) + 1;
    if (this.frame > 2) {
      this.frame = 0;
      this.nsides = (this.nsides | 0) + 1;
      this.nsides = this.nsides % 10;
      // Destroy and recreate the renderable component.
      this.engine.destroyEntity(this.polygon);
      this.createPolygon(this.polygon, 3 + this.nsides, this.mi);
    }
    // Rotate the polygon around the Z axis.
    const radians = Date.now() / 1000;
    const transform = mat4.fromRotation(mat4.create(), radians, [0, 0, 1]);
    const tcm = this.engine.getTransformManager();
    const inst = tcm.getInstance(this.polygon);
    tcm.setTransform(inst, transform);
    inst.delete();
  }
}
