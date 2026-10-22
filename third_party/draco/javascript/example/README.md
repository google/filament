# Example

## Quick Start

To start using Draco, you could try the example `webgl_loader_draco.html`. The
code shows a simple example of integration of threejs and Draco Javascript or
WebAssembly decoder. From the example, you should be able to load an encoded
Draco mesh file and visualize it through threejs's fancy 3D tools.

How to run the example code:

1. Clone this project to a working directory, e.g. draco/.

2. From the project's root directory, start a local http server. E.g, with
Python, you could run "python -m SimpleHTTPServer".

3. Load javascript/example/webgl_loader_draco.html. You should be able to see a
bunny rendered by threejs using Draco loader.

## Advanced Example

To use more advanced features of Draco loader, please look at the example
`webgl_loader_draco_advanced.html`. In this example, you could:

1. Switch between Javascript decoder and WebAssembly decoder if it's supported
by the browser.

2. Load your own encoded Draco file (.drc) by clicking on "Choose File" and
select a file.

## Static Loading Javascript Decoder

In the previous examples, `DRACOLoader.js` will dynamically load Javascript
decoder or WASM decoder depending on the support of the browser. To avoid
dynamically loading the decoder, e.g. to use with JS bundler like webpack, you
could directly include `draco_decoder.js` and set the decoder type to `js`. For
example:

Include Javascript decoder:

```html
<script src="../draco_decoder.js"></script>
```

Create DracoLoader by setting the decoder type:

```js
// (Optional) Change decoder source directory (defaults to
// 'https://www.gstatic.com/draco/versioned/decoders/1.5.7/'). It is recommended
// to always pull your Draco JavaScript and WASM decoders from this URL. Users
// will benefit from having the Draco decoder in cache as more sites start using
// the static URL.
THREE.DRACOLoader.setDecoderPath('./path/to/decoder/');

// (Optional) Force non-WebAssembly JS decoder (without this line, WebAssembly
// is the default if supported).
THREE.DRACOLoader.setDecoderConfig({type: 'js'});

// (Optional) Pre-fetch decoder source files (defaults to load on demand).
THREE.DRACOLoader.getDecoderModule();

var dracoLoader = new THREE.DRACOLoader();

dracoLoader.load( 'model.drc', function ( geometry ) {

  scene.add( new THREE.Mesh( geometry ) );

  // (Optional) Release the cached decoder module.
  THREE.DRACOLoader.releaseDecoderModule();

} );
```
