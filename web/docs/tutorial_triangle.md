## Start your project

First, create a text file called `triangle.html` and fill it with the following HTML. This creates
a mobile-friendly page with a full-screen canvas.

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <title>Filament Tutorial</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,user-scalable=no,initial-scale=1">
    <style>
        body { margin: 0; overflow: hidden; }
        canvas { touch-action: none; width: 100%; height: 100%; }
    </style>
</head>
<body>
    <canvas></canvas>
    <script src="filament.js"></script>
    <script src="//unpkg.com/gl-matrix@2.8.1"></script>
    <script src="triangle.js"></script>
</body>
</html>
```

The above HTML loads three JavaScript files:
- `filament.js` does a couple things:
   - Downloads assets and compiles the Filament WASM module.
   - Contains high-level utilities, e.g. to simplify loading KTX textures from JavaScript.
- `gl-matrix-min.js` is a small library that provides vector math functionality.
- `triangle.js` will contain your application code.

Go ahead and create `triangle.js` with the following content.

```js {fragment="root"}
class App {
  constructor() {
    // TODO: create entities
    this.render = this.render.bind(this);
    this.resize = this.resize.bind(this);
    window.addEventListener('resize', this.resize);
    window.requestAnimationFrame(this.render);
  }
  render() {
    // TODO: render scene
    window.requestAnimationFrame(this.render);
  }
  resize() {
    // TODO: adjust viewport and canvas
  }
}

Filament.init(['triangle.filamat'], () => { window.app = new App() } );
```

The two calls to `bind()` allow us to pass instance methods as callbacks for animation and resize
events.

`Filament.init()` consumes two things: a list of asset URLs and a callback.

The callback will be triggered only after all assets finish downloading and the Filament module has
become ready. In our callback, we simply instantiated the `App` object, since we'll do most of the
work in its constructor. We also set the app instance into a `Window` property to make it accessible
from the developer console.

Go ahead and download [triangle.filamat](triangle.filamat) and place it in your project folder.
This is a *material package*, which is a binary file that contains shaders and other bits of data
that define a PBR material. We'll learn more about material packages in the next tutorial.

## Spawn a local server

Because of CORS restrictions, your web app cannot fetch the material package directly from the
file system. One way around this is to create a temporary server using Python or node:

```bash
python3 -m http.server     # Python 3
python -m SimpleHTTPServer # Python 2.7
npx http-server -p 8000    # nodejs
```

To see if this works, navigate to [http://localhost:8000](http://localhost:8000) and check if you
can load the page without any errors appearing in the developer console.

Take care not to use Python's simple server in production since it does not serve WebAssembly files
with the correct MIME type.

## Create the Engine and Scene

We now have a basic skeleton that can respond to paint and resize events. Let's start adding
Filament objects to the app. Insert the following code into the top of the app constructor.

```js {fragment="create entities"}
this.canvas = document.getElementsByTagName('canvas')[0];
const engine = this.engine = Filament.Engine.create(this.canvas);
```

The above snippet creates the `Engine` by passing it a canvas DOM object. The engine needs the
canvas in order to create a WebGL 2.0 context in its contructor.

The engine is a factory for many Filament entities, including `Scene`, which is a flat container of
entities. Let's go ahead and create a scene, then add a blank entity called `triangle` into the
scene.

```js {fragment="create entities"}
this.scene = engine.createScene();
this.triangle = Filament.EntityManager.get().create();
this.scene.addEntity(this.triangle);
```

Filament uses an [Entity-Component System](//en.wikipedia.org/wiki/Entity-component-system).
The triangle entity in the above snippet does not yet have an associated component. Later in the
tutorial we will make it into a *renderable*. Renderables are entities that have associated draw
calls.

## Construct typed arrays

Next we'll create two typed arrays: a positions array with XY coordinates for each vertex, and a
colors array with a 32-bit word for each vertex.

```js {fragment="create entities"}
const TRIANGLE_POSITIONS = new Float32Array([
    1, 0,
    Math.cos(Math.PI * 2 / 3), Math.sin(Math.PI * 2 / 3),
    Math.cos(Math.PI * 4 / 3), Math.sin(Math.PI * 4 / 3),
]);

const TRIANGLE_COLORS = new Uint32Array([0xffff0000, 0xff00ff00, 0xff0000ff]);
```

Next we'll use the positions and colors buffers to create a single `VertexBuffer` object.

```js {fragment="create entities"}
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
```

The above snippet first creates aliases for two enum types, then constructs the vertex buffer using
its `Builder` method. After that, it pushes two buffer objects into the appropriate slots using
`setBufferAt`.

In the Filament API, the above builder pattern is often used for constructing objects in lieu of
long argument lists. The daisy chain of function calls allows the client code to be somewhat
self-documenting.

Our app sets up two buffer slots in the vertex buffer, and each slot is associated with a single
attribute. Alternatively, we could have interleaved or concatenated these attributes into a single
buffer slot.

Next we'll construct an index buffer. The index buffer for our triangle is trivial: it simply holds
the integers 0,1,2.

```js {fragment="create entities"}
this.ib = Filament.IndexBuffer.Builder()
    .indexCount(3)
    .bufferType(Filament.IndexBuffer$IndexType.USHORT)
    .build(engine);

this.ib.setBuffer(engine, new Uint16Array([0, 1, 2]));
```

Note that constructing an index buffer is similar to constructing a vertex buffer, but it only has
one buffer slot, and it can only contain two types of data (USHORT or UINT).

## Finish up initialization

Next let's construct an actual `Material` from the material package that was downloaded (the
material is an object; the package is just a binary blob), then extract the default
`MaterialInstance` from the material object. Material instances have concrete values for their
parameters, and they can be bound to renderables. We'll learn more about material instances in the
next tutorial.

After extracting the material instance, we can finally create a renderable component for the
triangle by setting up a bounding box and passing in the vertex and index buffers.

```js {fragment="create entities"}
const mat = engine.createMaterial('triangle.filamat');
const matinst = mat.getDefaultInstance();
Filament.RenderableManager.Builder(1)
    .boundingBox({ center: [-1, -1, -1], halfExtent: [1, 1, 1] })
    .material(0, matinst)
    .geometry(0, Filament.RenderableManager$PrimitiveType.TRIANGLES, this.vb, this.ib)
    .build(engine, this.triangle);
```

Next let's wrap up the initialization routine by creating the swap chain, renderer, camera, and
view.

```js {fragment="create entities"}
this.swapChain = engine.createSwapChain();
this.renderer = engine.createRenderer();
this.camera = engine.createCamera(Filament.EntityManager.get().create());
this.view = engine.createView();
this.view.setCamera(this.camera);
this.view.setScene(this.scene);

// Set up a blue-green background:
this.renderer.setClearOptions({clearColor: [0.0, 0.1, 0.2, 1.0], clear: true});

// Adjust the initial viewport:
this.resize();
```

At this point, we're done creating all Filament entities, and the code should run without errors.
However the canvas is still blank!

## Render and resize handlers

Recall that our App class has a skeletal render method, which the browser calls every time it needs
to repaint. Often this is 60 times a second.

```js
render() {
    // TODO: render scene
    window.requestAnimationFrame(this.render);
}
```

Let's flesh this out by rotating the triangle and invoking the Filament renderer. Add the following
code to the top of the render method.

```js {fragment="render scene"}
// Rotate the triangle.
const radians = Date.now() / 1000;
const transform = mat4.fromRotation(mat4.create(), radians, [0, 0, 1]);
const tcm = this.engine.getTransformManager();
const inst = tcm.getInstance(this.triangle);
tcm.setTransform(inst, transform);
inst.delete();

// Render the frame.
this.renderer.render(this.swapChain, this.view);
```

The first half of our render method obtains the transform component of the triangle entity and uses
gl-matrix to generate a rotation matrix.

The second half of our render method invokes the Filament renderer on the view, and tells the
Filament engine to execute its internal command buffer. The Filament renderer can tell the app
that it wants to skip a frame, hence the `if` statement.

One last step. Add the following code to the resize method. This adjusts the resolution of the
rendering surface when the window size changes, taking `devicePixelRatio` into account for high-DPI
displays. It also adjusts the camera frustum accordingly.

```js {fragment="adjust viewport and canvas"}
const dpr = window.devicePixelRatio;
const width = this.canvas.width = window.innerWidth * dpr;
const height = this.canvas.height = window.innerHeight * dpr;
this.view.setViewport([0, 0, width, height]);

const aspect = width / height;
const Projection = Filament.Camera$Projection;
this.camera.setProjection(Projection.ORTHO, -aspect, aspect, -1, 1, 0, 1);
```

You should now have a spinning triangle! The completed JavaScript is available
[here](tutorial_triangle.js).

In the [next tutorial], we'll take a closer look at Filament materials and 3D rendering.

[next tutorial]: tutorial_redball.html
