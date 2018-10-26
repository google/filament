Filament.init([
  'parquet.filamat',
  'shader_ball.filamesh',
  'venetian_crossroads_2k_ibl.ktx',
  'venetian_crossroads_2k_skybox.ktx',
  'floor_ao_roughness_metallic.png',
  'floor_basecolor.png',
  'floor_normal.png',
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
    this.canvas = document.getElementsByTagName('canvas')[0];
    const engine = this.engine = Filament.Engine.create(this.canvas);
    this.scene = engine.createScene();

    const sunlight = Filament.EntityManager.get().create();
    Filament.LightManager.Builder(LightType.SUN)
            .color([0.98, 0.92, 0.89])
            .intensity(100000.0)
            .direction([0.6, -1.0, -0.8])
            .castShadows(true)
            .sunAngularRadius(1.9)
            .sunHaloSize(10.0)
            .sunHaloFalloff(80.0)
            .build(engine, sunlight);
    this.scene.addEntity(sunlight);

    const ibldata = 'venetian_crossroads_2k_ibl.ktx';
    const indirectLight = this.ibl = engine.createIblFromKtx(ibldata);
    this.scene.setIndirectLight(indirectLight);

    const radians = 1.0;
    indirectLight.setRotation(mat3.fromRotation(mat3.create(), radians, [0, 1, 0]))
    indirectLight.setIntensity(10000);

    const skydata = 'venetian_crossroads_2k_skybox.ktx';
    const skybox = engine.createSkyFromKtx(skydata);
    this.scene.setSkybox(skybox);

    const material = engine.createMaterial('parquet.filamat');
    const matinstance = material.createInstance();

    const sampler = new Filament.TextureSampler(
        Filament.MinFilter.LINEAR_MIPMAP_LINEAR,
        Filament.MagFilter.LINEAR,
        Filament.WrapMode.CLAMP_TO_EDGE);

    const ao = engine.createTextureFromPng('floor_ao_roughness_metallic.png');
    const basecolor = engine.createTextureFromPng('floor_basecolor.png', {'srgb': true});
    const normal = engine.createTextureFromPng('floor_normal.png');
    matinstance.setTextureParameter('aoRoughnessMetallic', ao, sampler)
    matinstance.setTextureParameter('baseColor', basecolor, sampler)
    matinstance.setTextureParameter('normal', normal, sampler)

    const mesh = engine.loadFilamesh('shader_ball.filamesh', matinstance);
    this.shaderball = mesh.renderable;
    this.scene.addEntity(mesh.renderable);

    this.swapChain = engine.createSwapChain();
    this.renderer = engine.createRenderer();
    this.camera = engine.createCamera();
    this.view = engine.createView();
    this.view.setCamera(this.camera);
    this.view.setScene(this.scene);
    this.resize();
    this.render = this.render.bind(this);
    this.resize = this.resize.bind(this);
    window.addEventListener('resize', this.resize);
    window.requestAnimationFrame(this.render);
  }

  render() {

    const radians = Date.now() / 1000;
    const transform = mat4.fromRotation(mat4.create(), radians, [0, 1, 0]);
    const tcm = this.engine.getTransformManager();
    tcm.setTransform(tcm.getInstance(this.shaderball), transform);

    this.renderer.render(this.swapChain, this.view);
    window.requestAnimationFrame(this.render);
  }

  resize() {
    const dpr = window.devicePixelRatio;
    const width = this.canvas.width = window.innerWidth * dpr;
    const height = this.canvas.height = window.innerHeight * dpr;
    this.view.setViewport([0, 0, width, height]);
    const eye = [0, 1.8, 5], center = [0, 1, -1], up = [0, 1, 0];
    this.camera.lookAt(eye, center, up);
    const aspect = width / height;
    this.camera.setProjectionFov(45, aspect, 1.0, 10.0, aspect < 1 ? Fov.HORIZONTAL : Fov.VERTICAL);
  }
}
