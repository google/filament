Filament.init(['nonlit.filamat'], () => {
  window.VertexAttribute = Filament.VertexAttribute;
  window.AttributeType = Filament.VertexBuffer$AttributeType;
  window.Projection = Filament.Camera$Projection;
  window.app = new App(
      document.getElementsByTagName('canvas')[0],
      document.getElementById('frame-time-counter'));
});

const NUMBER_OF_TRIANGLES = 100;
// probably 60 fps; record 1 seconds ish worth of frames.
const NUMBER_OF_FRAMES_TO_RECORD_FPS = 1 * 60;

class App {
  constructor(canvas, frameTimeCounter) {
    this.canvas = canvas;
    this.frameTimeCounter = frameTimeCounter;

    this.lastFrameTime = null;
    this.frameDeltas = [];
    for (let i = 0; i < NUMBER_OF_FRAMES_TO_RECORD_FPS; ++i) {
      this.frameDeltas.push(0);
    }
    this.frameDeltasIndex = 0;
    this.frameDeltasSum = 0;
    this.frameTimeIsValid = false;

    const engine = this.engine = Filament.Engine.create(this.canvas);
    this.scene = engine.createScene();
    this.triangles = [];
    for (let i = 0; i < NUMBER_OF_TRIANGLES; ++i) {
      const entity = Filament.EntityManager.get().create();
      this.triangles.push({entity: entity});
      this.scene.addEntity(entity);
    }

    const TRIANGLE_POSITIONS = new Float32Array([
      1,
      0,
      Math.cos(Math.PI * 2 / 3),
      Math.sin(Math.PI * 2 / 3),
      Math.cos(Math.PI * 4 / 3),
      Math.sin(Math.PI * 4 / 3),
    ]);

    const TRIANGLE_COLORS =
        new Uint32Array([0xffff0000, 0xff00ff00, 0xff0000ff]);

    this.vb =
        Filament.VertexBuffer.Builder()
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

    this.ib.setBuffer(engine, new Uint16Array([0, 1, 2]));

    this.swapChain = engine.createSwapChain();
    this.renderer = engine.createRenderer();
    this.camera = engine.createCamera(Filament.EntityManager.get().create());

    this.view = engine.createView();
    this.view.setSampleCount(4);
    this.view.setCamera(this.camera);
    this.view.setScene(this.scene);

    this.renderer.setClearOptions(
        {clearColor: [0.0, 0.1, 0.2, 1.0], clear: true});

    this.resize();
    this.render = this.render.bind(this);
    this.resize = this.resize.bind(this);
    window.addEventListener('resize', this.resize);
    window.requestAnimationFrame(this.render);
  }

  render() {
    const frameTime = Date.now();
    if (this.lastFrameTime) {
      const delta = frameTime - this.lastFrameTime;
      this.frameDeltasSum =
          this.frameDeltasSum - this.frameDeltas[this.frameDeltasIndex] + delta;
      this.frameDeltas[this.frameDeltasIndex] = delta;
      this.frameDeltasIndex =
          (this.frameDeltasIndex + 1) % NUMBER_OF_FRAMES_TO_RECORD_FPS;
      if (this.frameDeltasIndex == 0) {
        this.frameTimeIsValid = true;
      }
    }
    this.lastFrameTime = frameTime;

    if (this.frameTimeIsValid) {
      const averageFrameDelta =
          this.frameDeltasSum * 1.0 / NUMBER_OF_FRAMES_TO_RECORD_FPS;
      this.frameTimeCounter.innerHTML = averageFrameDelta.toFixed(2) + ' ms';
    }

    this.triangles.forEach(triangle => {
      if (triangle.mat) {
        this.engine.destroyMaterial(triangle.mat);
      }
      triangle.mat = this.engine.createMaterial('nonlit.filamat');
      const matinst = triangle.mat.getDefaultInstance();
      Filament.RenderableManager.Builder(1)
          .boundingBox({center: [-1, -1, -1], halfExtent: [1, 1, 1]})
          .material(0, matinst)
          .geometry(
              0, Filament.RenderableManager$PrimitiveType.TRIANGLES, this.vb,
              this.ib)
          .build(this.engine, triangle.entity);
    });

    const radians = Date.now() / 1000;
    const transform = mat4.fromRotation(mat4.create(), radians, [0, 0, 1]);
    const tcm = this.engine.getTransformManager();
    const inst = tcm.getInstance(this.triangles[0].entity);
    tcm.setTransform(inst, transform);
    inst.delete();
    this.renderer.render(this.swapChain, this.view);
    window.requestAnimationFrame(this.render);
  }

  resize() {
    const dpr = window.devicePixelRatio;
    const width = this.canvas.width = window.innerWidth * dpr;
    const height = this.canvas.height = window.innerHeight * dpr;
    this.view.setViewport([0, 0, width, height]);
    const aspect = width / height;
    this.camera.setProjection(Projection.ORTHO, -aspect, aspect, -1, 1, 0, 1);
  }
}
