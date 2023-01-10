const environ = 'pillars_2k';
const ibl_url = `${environ}/${environ}_ibl.ktx`;
const sky_url = `${environ}/${environ}_skybox.ktx`;
const filamat_url = 'plastic.filamat'
Filament.init([filamat_url, ibl_url, sky_url], () => {
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
});
class App {
  constructor(canvas) {
    this.canvas = canvas;
    const engine = this.engine = Filament.Engine.create(canvas);
    const scene = engine.createScene();
    const material = engine.createMaterial(filamat_url);
    const matinstance = material.createInstance();
    const red = [0.8, 0.0, 0.0];
    matinstance.setColor3Parameter('baseColor', Filament.RgbType.sRGB, red);
    matinstance.setFloatParameter('roughness', 0.5);
    matinstance.setFloatParameter('clearCoat', 1.0);
    matinstance.setFloatParameter('clearCoatRoughness', 0.3);
    const renderable = Filament.EntityManager.get()
      .create();
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
      .boundingBox({
        center: [-1, -1, -1],
        halfExtent: [1, 1, 1]
      })
      .material(0, matinstance)
      .geometry(0, PrimitiveType.TRIANGLES, vb, ib)
      .build(engine, renderable);
    const sunlight = Filament.EntityManager.get()
      .create();
    scene.addEntity(sunlight);
    Filament.LightManager.Builder(LightType.SUN)
      .color([0.98, 0.92, 0.89])
      .intensity(110000.0)
      .direction([0.6, -1.0, -0.8])
      .sunAngularRadius(1.9)
      .sunHaloSize(10.0)
      .sunHaloFalloff(80.0)
      .build(engine, sunlight);
    const backlight = Filament.EntityManager.get()
      .create();
    scene.addEntity(backlight);
    Filament.LightManager.Builder(LightType.DIRECTIONAL)
      .direction([-1, 0, 1])
      .intensity(50000.0)
      .build(engine, backlight);
    const indirectLight = engine.createIblFromKtx1(ibl_url);
    indirectLight.setIntensity(50000);
    scene.setIndirectLight(indirectLight);
    const skybox = engine.createSkyFromKtx1(sky_url);
    scene.setSkybox(skybox);
    this.swapChain = engine.createSwapChain();
    this.renderer = engine.createRenderer();
    this.camera = engine.createCamera(Filament.EntityManager.get()
      .create());
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
    const eye = [0, 0, 4],
      center = [0, 0, 0],
      up = [0, 1, 0];
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
