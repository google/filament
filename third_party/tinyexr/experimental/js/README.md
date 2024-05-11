# Experimental JavaScript port of TinyEXR

Using Emscripten.

## How to compile

edit `compile_to_js.sh`, then

```
./compile_to_js.sh
```

## How to run test

Requires node.js

```
$ node test.js
```

## How to run a browser example

Edit EXR file in `index.html`
Copy a EXR file to this directory(default = asakusa.exr).
(NOTE: Chrome does not allow reading a file from parent path(e.g. `../../asakusa.exr`).

Launch http server, e.g.,

```
$ python3 -m http.server 
```

Open `http://localhost:8000` with an browser.

## TODO

* [x] Write HTML5 Canvas drawing JS code from Float32Array(
* [ ] Pollish JS API.
