let assets = {};
let assets_ready = false;
let context_ready = false;

// This is usually (not always) called before the wasm has finished JIT compiling.
async function load(promises) {
    for (let name in promises) {
        assets[name] = await promises[name];
    }
    assets_ready = true;
    maybe_launch();
}

// This is called as soon as the wasm has finished JIT compilation. We also create the WebGL 2.0
// context here.
Module.postRun = function() {
    let canvas = document.getElementById('filament-canvas');
    let ctx = GL.createContext(canvas, {
        majorVersion: 2,
        minorVersion: 0,
        antialias: false,
        depth: false
    });
    GL.makeContextCurrent(ctx);
    context_ready = true;
    maybe_launch();
}

// This is called twice: once after assets have loaded, and once after WASM compilation. We try to
// be robust with either order, launching the C++ code only after both requirements are fulfilled.
function maybe_launch() {
    if (assets_ready && context_ready) {
        _launch();
        canvas_resize();
        window.addEventListener("resize", canvas_resize);
        canvas_render();    
    }
}

function canvas_render() {
    _render();
    window.requestAnimationFrame(canvas_render);
}

function canvas_resize() {
    let canvas = document.getElementById('filament-canvas');
    let pr = window.devicePixelRatio;
    let w = window.innerWidth;
    let h = window.innerHeight;
    canvas.width = (w * pr) | 0;
    canvas.height = (h * pr) | 0;
    _resize((w * pr) | 0, (h * pr) | 0 );
}

function load_file(url) {
    let promise = new Promise((success, failure) => {
        fetch(url).then(resp => {
            resp.arrayBuffer().then(filedata => success({
                kind: 'file',
                name: url,
                data: filedata
            }));
        });
    });
    return promise;
}

let context2d = document.createElement('canvas').getContext('2d');

// This function does PNG decoding (or JPEG, or whatever), which allows us to avoid including a
// texture decoder (such as stb_image) in the WebAssembly module. It always returns RGBA8 data,
// regardless of how it is stored in the image file.
function load_texture(url) {
    let promise = new Promise((success, failure) => {
        let img = new Image();
        img.src = url;
        img.onerror = failure;
        img.onload = () => {
            context2d.canvas.width = img.width;
            context2d.canvas.height = img.height;
            context2d.width = img.width;
            context2d.height = img.height;
            context2d.drawImage(img, 0, 0);
            let imgdata = context2d.getImageData(0, 0, img.width, img.height).data.buffer;
            success({
                kind: 'image',
                name: url,
                data: new Uint32Array(imgdata),
                width: img.width,
                height: img.height
            });
        };
    });
    return promise;
}
