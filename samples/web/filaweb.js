let assets = {};
let assets_ready = false;
let context_ready = false;
let previous_mouse_buttons = 0;
let queued_mouse_events = [];

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
        depth: false,
        alpha: false
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
        let canvas = document.getElementById('filament-canvas');
        canvas.addEventListener("wheel", canvas_mouse);
        canvas.addEventListener("pointermove", canvas_mouse);
        canvas.addEventListener("pointerdown", canvas_mouse);
        canvas.addEventListener("pointerup", canvas_mouse);
        canvas_render();
    }
}

// Update a tiny queue of pointer events. We don't send them to WASM immediately because ImGui
// detects click events by looking for three consecutive frames of down-up-down.
function canvas_mouse(evt) {
    if (evt.pointerType == 'touch' && !evt.isPrimary) {
        return;
    }
    let args = [evt.clientX, evt.clientY, evt.deltaX || 0, evt.deltaY || 0, evt.buttons];
    if (evt.buttons != previous_mouse_buttons || queued_mouse_events.length == 0) {
        queued_mouse_events.push(args);
    } else {
        // Clobber the previous event if the button state doesn't change, there's no point in
        // creating latency.
        queued_mouse_events[queued_mouse_events.length - 1] = args;
    }
    previous_mouse_buttons = evt.buttons;
}

function canvas_render() {
    if (queued_mouse_events.length > 0) {
        let args = queued_mouse_events.splice(0, 1)[0];
        _mouse(args[0], args[1], args[2], args[3], args[4]);
    }
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
    _resize((w * pr) | 0, (h * pr) | 0, pr);
}

function load_rawfile(url) {
    let promise = new Promise((success, failure) => {
        fetch(url).then(resp => {
            resp.arrayBuffer().then(filedata => success({
                kind: 'rawfile',
                url: url,
                data: new Uint8Array(filedata)
            }));
        });
    });
    return promise;
}

let load_texture = load_rawfile;

function load_cubemap(name, suffix) {
    let urlprefix = name + '/';
    let promises = {};
    promises['ibl'] = load_rawfile(urlprefix + name + '_ibl' + suffix);
    promises['skybox'] = load_rawfile(urlprefix + name + '_skybox' + suffix);
    let numberRemaining = Object.keys(promises).length;
    var promise = new Promise((success) => {
        for (let key in promises) {
            promises[key].then((result) => {
                assets[urlprefix + key] = result;
                if (--numberRemaining == 0) {
                    success({
                        kind: 'cubemap',
                        name: urlprefix,
                    });
                }
            });
        }
    });
    return promise;
}
