# Copyright (C) 2026 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Browser-driven renderer for test/renderdiff.

The desktop renderer spawns gltf_viewer as a subprocess. The web renderer runs the *same*
program, compiled to WebAssembly, inside a pinned headless Chromium, and drives it through the
globalThis.filamentApp contract of the generated sample page (samples/wasm_shell.html.in):

  1. serve the WASM bundle over loopback HTTP,
  2. open gltf_viewer.html with ?autorun=0,
  3. stage the batch spec and the model into MEMFS,
  4. call main() with the same argv shape the desktop renderer uses,
  5. wait for the app to signal that its main loop has stopped,
  6. read the TIFF back out of MEMFS onto the host filesystem.

Determinism comes from pinning the browser (see rendering_requirements.txt) and forcing
SwiftShader, so the goldens do not depend on the host GPU. Web goldens are kept separate from
desktop goldens -- the renderer spec is 'web-webgl' / 'web-webgpu' -- because the rasterizer,
the shader compiler and the precision guarantees all differ from desktop.

This module is imported lazily by RendererFactory so that a desktop-only run does not need
playwright installed.
"""

import concurrent.futures
import functools
import http.server
import json
import os
import queue
import shutil
import socketserver
import struct
import threading
import uuid

from renderers import BaseRenderer, GltfRenderTestCase, default_concurrent_renders
from results import RESULT_OK, RESULT_FAILED
from utils import important_print

# Flags that remove sources of frame-to-frame and machine-to-machine variation. Without these
# the same build produces different pixels on different hosts, which makes goldens worthless.
DETERMINISM_FLAGS = [
    '--force-device-scale-factor=1',
    '--disable-gpu-sandbox',
    '--hide-scrollbars',
    '--disable-lcd-text',
    '--disable-background-timer-throttling',
    '--disable-renderer-backgrounding',
    '--disable-partial-raster',
    '--disable-skia-runtime-opts',
    '--deterministic-mode',
]

# Software rasterization, per backend. Rendering on whatever GPU the runner happens to have
# would make the goldens unreproducible.
BACKEND_FLAGS = {
    'webgl': [
        '--use-angle=swiftshader',
        '--enable-unsafe-swiftshader',
    ],
    'webgpu': [
        '--enable-unsafe-webgpu',
        '--use-webgpu-adapter=swiftshader',
        '--enable-features=Vulkan',
        # Without these the Linux runner cannot allocate the canvas swap-chain image ("Could not
        # find SharedImageBackingFactory ... WebgpuSwapChainTexture"), Chromium drops the WebGPU
        # instance, and every mapAsync() fails with "A valid external Instance reference no longer
        # exists". Putting the compositor's GL and Chromium's own Vulkan on SwiftShader as well
        # gives the shared image a backing. Harmless on macOS, which renders the same with them.
        '--use-angle=swiftshader',
        '--use-vulkan=swiftshader',
    ],
}

# gltf_viewer's -a flag takes a Filament backend name. renderdiff's backend name identifies the
# *web* API instead, because that is what the goldens are keyed on: 'webgl' pixels and
# 'desktop-opengl' pixels are not interchangeable even though both are the opengl backend.
FILAMENT_API = {
    'webgl': 'opengl',
    'webgpu': 'webgpu',
}

# The browser build is part of the golden contract: a different Chromium can rasterize the
# same draw calls differently even under SwiftShader. rendering_requirements.txt pins
# playwright, which transitively pins the browser, but that only binds an environment that
# was actually installed from that file -- a developer with a stale or hand-installed
# playwright would otherwise just see every web golden mismatch with no explanation.
# Asserting it here converts that into one clear error.
#
# Bumping playwright means bumping this and regenerating the web goldens.
EXPECTED_BROWSER_VERSION = '153.0.8010.12'


# Compiling and instantiating a ~90MB debug .wasm is slow, and so is software rasterization.
MODULE_READY_TIMEOUT_MS = 180000
RENDER_TIMEOUT_MS = 600000
# Bounds each asset fetch during staging. Enforced inside the page: page.evaluate() itself
# cannot be interrupted from the driver, so a stuck fetch would otherwise hang the run.
STAGING_TIMEOUT_MS = 120000


# Synthetic origin-relative paths served directly by _LocalServer so assets and rendered TIFFs
# stream as raw loopback TCP bytes rather than base64-encoded CDP JSON messages.
ASSET_ROUTE_PREFIX = '/__renderdiff_asset__/'
OUTPUT_ROUTE_PREFIX = '/__renderdiff_out__/'
HOST_PAGE_PATH = '/__renderdiff_host__.html'

_HOST_PAGE_HTML = b"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <style>
    html, body { margin: 0; padding: 0; overflow: hidden; background: #1a1a1a; }
    iframe { width: 1024px; height: 768px; border: 0; display: block; }
  </style>
</head>
<body></body>
</html>
"""

# TIFF tags we care about.
_TAG_IMAGE_WIDTH = 256
_TAG_IMAGE_LENGTH = 257
_TAG_COMPRESSION = 259
_TAG_STRIP_OFFSETS = 273
_TAG_SAMPLES_PER_PIXEL = 277
_TAG_STRIP_BYTE_COUNTS = 279

_TIFF_TYPE_SIZES = {1: 1, 2: 1, 3: 2, 4: 4, 5: 8}


def _read_tiff_tags(data: bytes) -> dict:
  """Parses the first IFD of a TIFF and returns {tag: [values]}.

  Deliberately hand-rolled: the renderdiff python has no image dependencies (compare.py shells
  out to a native differ) and adding Pillow/numpy just to sanity-check a header is not worth it.
  """
  if len(data) < 8:
    raise ValueError(f'truncated TIFF: {len(data)} bytes')
  if data[:2] == b'II':
    endian = '<'
  elif data[:2] == b'MM':
    endian = '>'
  else:
    raise ValueError(f'not a TIFF: byte order mark is {data[:2]!r}')

  magic, = struct.unpack_from(endian + 'H', data, 2)
  if magic != 42:
    raise ValueError(f'not a TIFF: magic is {magic}')

  ifd_offset, = struct.unpack_from(endian + 'I', data, 4)
  entry_count, = struct.unpack_from(endian + 'H', data, ifd_offset)

  tags = {}
  for i in range(entry_count):
    entry = ifd_offset + 2 + i * 12
    tag, field_type, count = struct.unpack_from(endian + 'HHI', data, entry)
    size = _TIFF_TYPE_SIZES.get(field_type)
    if size is None or field_type == 5:
      continue
    total = size * count
    # Values of 4 bytes or fewer are stored inline in the entry itself.
    offset = entry + 8
    if total > 4:
      offset, = struct.unpack_from(endian + 'I', data, entry + 8)
    code = {1: 'B', 2: 'B', 3: 'H', 4: 'I'}[field_type]
    tags[tag] = list(struct.unpack_from(f'{endian}{count}{code}', data, offset))
  return tags


def _tiff_distinct_colors(data: bytes, tags: dict, budget: int = 20000):
  """Counts distinct pixel values over a strided sample of an uncompressed TIFF.

  Returns None if the image is compressed, in which case the caller falls back to a cruder
  check. Filament's TIFF encoder writes uncompressed data today, so the fallback is defensive.
  """
  if tags.get(_TAG_COMPRESSION, [1])[0] != 1:
    return None

  offsets = tags.get(_TAG_STRIP_OFFSETS)
  counts = tags.get(_TAG_STRIP_BYTE_COUNTS)
  samples = tags.get(_TAG_SAMPLES_PER_PIXEL, [1])[0]
  if not offsets or not counts or samples < 1:
    return None

  pixels = bytearray()
  for offset, count in zip(offsets, counts):
    pixels += data[offset:offset + count]

  total = len(pixels) // samples
  if total == 0:
    return None
  stride = max(1, total // budget)
  return len({bytes(pixels[i * samples:(i + 1) * samples]) for i in range(0, total, stride)})


class RasterizerMismatch(RuntimeError):
  """Raised when the browser is not using the expected software rasterizer.

  This is fatal for the whole run rather than a per-test failure: every image produced would be
  unreproducible, so there is nothing to be gained by carrying on.
  """


class _LocalServer:
  """Serves the WASM bundle, staged model assets, and rendered outputs over threaded loopback HTTP."""

  def __init__(self, directory: str):
    outer = self
    self._lock = threading.Lock()
    self._assets = {}
    self._uploads = {}

    class Handler(http.server.SimpleHTTPRequestHandler):
      extensions_map = {
          **http.server.SimpleHTTPRequestHandler.extensions_map,
          '.wasm': 'application/wasm',
          '.js': 'text/javascript',
          '.data': 'application/octet-stream',
      }

      def log_message(self, fmt, *args):
        pass  # The default handler spams stderr with one line per request.

      def end_headers(self):
        # Allow each worker's Chromium HTTP cache to reuse gltf_viewer.js/.data/.html across
        # iframe test runs instead of re-reading them per test.
        if not self.path.startswith((ASSET_ROUTE_PREFIX, OUTPUT_ROUTE_PREFIX)):
          self.send_header('Cache-Control', 'public, max-age=3600')
        super().end_headers()

      def do_GET(self):
        path = self.path.split('?', 1)[0]
        if path == HOST_PAGE_PATH:
          self.send_response(200)
          self.send_header('Content-Type', 'text/html; charset=utf-8')
          self.send_header('Content-Length', str(len(_HOST_PAGE_HTML)))
          self.end_headers()
          self.wfile.write(_HOST_PAGE_HTML)
          return

        if path.startswith(ASSET_ROUTE_PREFIX):
          rest = path[len(ASSET_ROUTE_PREFIX):]
          parts = rest.split('/', 1)
          host_path = outer.lookup_asset(parts[0], parts[1] if len(parts) == 2 else '')
          if host_path is None:
            self.send_error(404, f'unknown asset {rest}')
            return
          try:
            size = os.path.getsize(host_path)
            with open(host_path, 'rb') as f:
              self.send_response(200)
              self.send_header('Content-Type', 'application/octet-stream')
              self.send_header('Content-Length', str(size))
              self.end_headers()
              shutil.copyfileobj(f, self.wfile)
          except OSError as e:
            self.send_error(500, str(e))
          return

        super().do_GET()

      def do_PUT(self):
        path = self.path.split('?', 1)[0]
        if not path.startswith(OUTPUT_ROUTE_PREFIX):
          self.send_error(404)
          return
        rest = path[len(OUTPUT_ROUTE_PREFIX):]
        parts = rest.split('/', 1)
        if len(parts) != 2:
          self.send_error(400)
          return
        length = int(self.headers.get('Content-Length', '0'))
        body = self.rfile.read(length) if length > 0 else b''
        outer.store_upload(parts[0], parts[1], body)
        self.send_response(200)
        self.send_header('Content-Length', '0')
        self.end_headers()

    handler = functools.partial(Handler, directory=directory)

    class Server(socketserver.ThreadingTCPServer):
      allow_reuse_address = True
      daemon_threads = True

    self._httpd = Server(('127.0.0.1', 0), handler)
    self.port = self._httpd.server_address[1]
    self._thread = threading.Thread(target=self._httpd.serve_forever, daemon=True)
    self._thread.start()

  @property
  def base_url(self) -> str:
    return f'http://127.0.0.1:{self.port}'

  def register_assets(self, token: str, by_index: dict):
    with self._lock:
      self._assets[token] = by_index

  def unregister_token(self, token: str):
    with self._lock:
      self._assets.pop(token, None)
      stale_keys = [k for k in self._uploads if k[0] == token]
      for k in stale_keys:
        self._uploads.pop(k, None)

  def lookup_asset(self, token: str, index: str):
    with self._lock:
      by_index = self._assets.get(token)
      return by_index.get(index) if by_index else None

  def store_upload(self, token: str, name: str, data: bytes):
    with self._lock:
      self._uploads[(token, name)] = data

  def pop_upload(self, token: str, name: str):
    with self._lock:
      return self._uploads.pop((token, name), None)

  def shutdown(self):
    self._httpd.shutdown()
    self._httpd.server_close()


class WebGltfRenderTestCase(GltfRenderTestCase):
  """A gltf test rendered in a browser. Only the execution differs from the desktop case."""

  def _execute_render(self, renderer, env, working_dir, out_tif_name):
    return renderer.render_in_browser(self, out_tif_name)


class WebRenderer(BaseRenderer):
  """Renders renderdiff tests with the WASM build of gltf_viewer inside headless Chromium."""

  # The window gltf_viewer is asked for, the counterpart of the fixed default window the desktop
  # renderer gets. FilamentApp2 renders the scene into the window minus the sidebar width that
  # gltf_viewer passes to ViewerGui, so the captured image is narrower than this. The harness
  # deliberately does not predict that width: the image is whatever size gltf_viewer produces,
  # exactly as on desktop, and diffimg reports a size mismatch against the golden if the
  # sidebar ever changes. 952 keeps the current web goldens at 512x512.
  WINDOW_WIDTH = 952
  WINDOW_HEIGHT = 512

  # A correctly sized but empty render must not pass as a golden.
  MIN_DISTINCT_COLORS = 16

  def __init__(self, platform: str, backend: str, executable: str, num_threads: int = None,
               test_filter: str = None):
    super().__init__(platform, backend, executable)
    # Each test runs in an isolated iframe inside one of N headless Chromium workers. Every worker
    # compiles gltf_viewer.wasm once on its host page and hands the compiled WebAssembly.Module to
    # each test iframe via instantiateWasm, overlapping single-threaded WASM asset decoding and
    # scene setup with SwiftShader rasterization across the runner's cores.
    self.concurrent_renders = num_threads if num_threads else default_concurrent_renders()
    self.test_filter = test_filter

    if backend not in FILAMENT_API:
      raise ValueError(
          f"backend '{backend}' is not a web backend; expected one of {sorted(FILAMENT_API)}")

    # For the web renderer, --executable points at the directory holding the emscripten output
    # rather than at a binary.
    self.bundle_dir = os.path.abspath(executable)
    if os.path.isfile(self.bundle_dir):
      self.bundle_dir = os.path.dirname(self.bundle_dir)
    for required in ('gltf_viewer.html', 'gltf_viewer.js', 'gltf_viewer.wasm'):
      path = os.path.join(self.bundle_dir, required)
      if not os.path.exists(path):
        raise FileNotFoundError(
            f'{path} not found. --executable must point at the WASM sample bundle, e.g. '
            f'out/cmake-wasm-debug/samples')

    self._server = None
    self._browser = None
    self._tls = threading.local()
    self._rasterizer_lock = threading.Lock()
    self.rasterizer = None

  def get_env(self) -> dict:
    return os.environ.copy()

  def _make_gltf_test_case(self, test, named_output_dir, model, model_path, test_json_path):
    return WebGltfRenderTestCase(
        test_name=test.name,
        backend=self.backend,
        output_dir=named_output_dir,
        model=model,
        model_path=model_path,
        test_json_path=test_json_path)

  def _make_sample_test_case(self, test, named_output_dir):
    raise NotImplementedError(
        f"test '{test.name}' is a sample_test, which the web renderer does not support yet. "
        f"Remove '{self.platform}-{self.backend}' from its renderers list.")

  # ---------------------------------------------------------------- browser lifecycle

  def _check_browser_version(self, browser=None):
    target = browser if browser is not None else self._browser
    version = target.version
    if version == EXPECTED_BROWSER_VERSION:
      return
    if os.environ.get('RDIFF_ALLOW_ANY_BROWSER') == '1':
      important_print(f'WARNING: browser is {version}, expected {EXPECTED_BROWSER_VERSION}. '
                      f'Renders may not match the goldens.')
      return
    raise RasterizerMismatch(
        f'browser is Chromium {version} but the web goldens were produced with '
        f'{EXPECTED_BROWSER_VERSION}. Reinstall the pinned browser with\n'
        f'    python3 -m pip install -r test/renderdiff/src/rendering_requirements.txt\n'
        f'    python3 -m playwright install chromium\n'
        f'or set RDIFF_ALLOW_ANY_BROWSER=1 to render anyway (expect golden mismatches). '
        f'If the pin itself is being updated, change EXPECTED_BROWSER_VERSION in '
        f'web_renderer.py and regenerate the web goldens.')

  def _check_rasterizer(self, page):
    """Confirms we are on SwiftShader. Recorded once across workers, then asserted."""
    with self._rasterizer_lock:
      if self.rasterizer is not None:
        return

      if self.backend == 'webgpu':
        self.rasterizer = page.evaluate("""async () => {
            if (!navigator.gpu) { return 'no navigator.gpu'; }
            const adapter = await navigator.gpu.requestAdapter();
            if (!adapter) { return 'no adapter'; }
            const info = adapter.info || {};
            return [info.vendor, info.architecture, info.device, info.description]
                .filter(Boolean).join(' / ');
        }""")
      else:
        self.rasterizer = page.evaluate("""() => {
            const gl = document.createElement('canvas').getContext('webgl2');
            if (!gl) { return 'no webgl2 context'; }
            const ext = gl.getExtension('WEBGL_debug_renderer_info');
            const renderer = ext ? gl.getParameter(ext.UNMASKED_RENDERER_WEBGL)
                                 : gl.getParameter(gl.RENDERER);
            const lose = gl.getExtension('WEBGL_lose_context');
            if (lose) { lose.loseContext(); }
            return renderer;
        }""")

      important_print(f'rasterizer: {self.rasterizer}')
      if 'swiftshader' not in (self.rasterizer or '').lower():
        # Hardware rasterization produces pixels that will not match the goldens, and worse, will
        # not match the next machine's either. Fail loudly rather than write junk goldens.
        if os.environ.get('RDIFF_ALLOW_ANY_RASTERIZER') != '1':
          raise RasterizerMismatch(
              f"expected a SwiftShader rasterizer but got '{self.rasterizer}'. Web goldens are "
              f'only reproducible under software rasterization. Set '
              f'RDIFF_ALLOW_ANY_RASTERIZER=1 to override for local experiments.')

  def _init_host_page(self, browser, console: list):
    """Creates a worker host page and compiles gltf_viewer.wasm once for iframe reuse."""
    page = browser.new_page(viewport={'width': 1024, 'height': 768})
    page.on('console', lambda msg: console.append(f'[{msg.type}] {msg.text}'))
    page.on('pageerror', lambda err: console.append(f'[pageerror] {err}'))
    page.goto(f'{self._server.base_url}{HOST_PAGE_PATH}', wait_until='load')
    self._check_rasterizer(page)
    page.evaluate(
        """async (timeoutMs) => {
            const abort = new AbortController();
            const timer = setTimeout(() => abort.abort(), timeoutMs);
            try {
              const response = await fetch('/gltf_viewer.wasm', {signal: abort.signal});
              if (!response.ok) {
                throw new Error(`fetching gltf_viewer.wasm: HTTP ${response.status}`);
              }
              globalThis.__renderdiffCompiledWasm =
                  await WebAssembly.compileStreaming(Promise.resolve(response));
            } finally {
              clearTimeout(timer);
            }
        }""",
        MODULE_READY_TIMEOUT_MS)
    return page

  # ---------------------------------------------------------------- rendering

  def _asset_manifest(self, model_path: str) -> list:
    """Host files to stage into MEMFS, as (memfs_path, host_path) pairs.

    A .glb is self-contained. A .gltf references buffers and textures by relative URI, so the
    whole containing directory has to come along.
    """
    model_path = os.path.abspath(model_path)
    name = os.path.basename(model_path)
    if model_path.lower().endswith('.glb'):
      return [(f'/work/{name}', model_path)]

    root = os.path.dirname(model_path)
    manifest = []
    for dirpath, _, filenames in os.walk(root):
      for filename in filenames:
        host = os.path.join(dirpath, filename)
        rel = os.path.relpath(host, root)
        manifest.append((f'/work/{rel}', host))
    return manifest

  def render_in_browser(self, case, out_tif_name: str) -> tuple:
    out_name = case.get_out_name(self)
    browser = self._tls.browser
    console = self._tls.console
    console.clear()

    if self._tls.page is None or self._tls.page.is_closed():
      self._tls.page = self._init_host_page(browser, console)
      console.clear()
    page = self._tls.page

    token = uuid.uuid4().hex
    manifest = self._asset_manifest(case.model_path)
    by_index = {str(i): host for i, (_, host) in enumerate(manifest)}
    self._server.register_assets(token, by_index)

    try:
      page.evaluate(
          """() => new Promise((resolve, reject) => {
              const prev = document.getElementById('runner');
              if (prev) { prev.remove(); }
              const frame = document.createElement('iframe');
              frame.id = 'runner';
              frame.onload = () => resolve();
              frame.onerror = () => reject(new Error('failed to load gltf_viewer.html iframe'));
              frame.src = '/gltf_viewer.html?autorun=0';
              document.body.appendChild(frame);
          })""")

      frame_el = page.query_selector('#runner')
      frame = frame_el.content_frame() if frame_el else None
      if frame is None:
        return self._fail(out_name, console, 'runner iframe was not attached')

      frame.wait_for_function(
          "() => globalThis.filamentApp && (filamentApp.status === 'ready' || filamentApp.error)",
          timeout=MODULE_READY_TIMEOUT_MS)
      error = frame.evaluate('() => filamentApp.error')
      if error is not None:
        return self._fail(out_name, console, f"page failed before running: {error}")

      with open(case.test_json_path) as f:
        spec_text = f.read()

      frame.evaluate(
          """async ({spec, files, timeoutMs}) => {
              const fs = filamentApp.module.FS;
              const stage = (path, bytes) => {
                fs.mkdirTree(path.substring(0, path.lastIndexOf('/')));
                fs.writeFile(path, bytes);
              };
              fs.mkdirTree('/work');
              fs.chdir('/work');
              stage('/work/spec.json', new TextEncoder().encode(spec));
              // Fetch up to 4 model resources concurrently over loopback.
              let cursor = 0;
              const worker = async () => {
                while (cursor < files.length) {
                  const [memfsPath, url] = files[cursor++];
                  const abort = new AbortController();
                  const timer = setTimeout(() => abort.abort(), timeoutMs);
                  try {
                    const response = await fetch(url, {signal: abort.signal});
                    if (!response.ok) {
                      throw new Error(`staging ${memfsPath}: HTTP ${response.status}`);
                    }
                    stage(memfsPath, new Uint8Array(await response.arrayBuffer()));
                  } finally {
                    clearTimeout(timer);
                  }
                }
              };
              await Promise.all(Array.from({length: Math.min(4, files.length)}, () => worker()));
          }""",
          {
              'spec': spec_text,
              'files': [[memfs, f'{ASSET_ROUTE_PREFIX}{token}/{i}']
                        for i, (memfs, _) in enumerate(manifest)],
              'timeoutMs': STAGING_TIMEOUT_MS,
          })

      model_memfs = manifest[0][0] if case.model_path.lower().endswith('.glb') else \
          f'/work/{os.path.basename(case.model_path)}'

      argv = [
          '-a', FILAMENT_API[self.backend],
          '--batch=/work/spec.json',
          f'--window-size={self.WINDOW_WIDTH}x{self.WINDOW_HEIGHT}',
          model_memfs,
      ]
      print(f'{out_name}: argv={argv}')
      frame.evaluate('(argv) => filamentApp.run(argv)', argv)

      try:
        frame.wait_for_function('() => filamentApp.exited || filamentApp.error !== null',
                                timeout=RENDER_TIMEOUT_MS)
      except Exception:
        status = frame.evaluate('() => filamentApp.status')
        return self._fail(out_name, console,
                          f"timed out after {RENDER_TIMEOUT_MS}ms in status '{status}'")

      status = frame.evaluate('() => filamentApp.status')
      error = frame.evaluate('() => filamentApp.error')
      if error is not None:
        return self._fail(out_name, console, f"final status '{status}': {error}")

      listing = frame.evaluate(
          "() => filamentApp.module.FS.readdir('/work').filter(n => n[0] !== '.')")

      tif_name = f'{case.test_name}0.tif'
      json_name = f'{case.test_name}0.json'
      if tif_name not in listing:
        return self._fail(out_name, console,
                          f'{tif_name} was not produced; /work contains {listing}')

      tif = self._read_memfs(frame, token, 'tif', f'/work/{tif_name}')
      problem = self._validate_tif(tif)
      if problem:
        return self._fail(out_name, console, problem)

      with open(out_tif_name, 'wb') as f:
        f.write(tif)

      if json_name in listing:
        settings = self._read_memfs(frame, token, 'json', f'/work/{json_name}')
        with open(os.path.join(case.output_dir, f'{out_name}.json'), 'wb') as f:
          f.write(settings)

      important_print(f'{out_name} rendering succeeded.')
      return 0, RESULT_OK
    except RasterizerMismatch:
      # Fatal for every test, not just this one.
      raise
    except Exception as e:
      # If the renderer process crashed, force the next test on this worker to rebuild the host
      # page rather than trying to reuse a dead target.
      try:
        page.close()
      except Exception:
        pass
      self._tls.page = None
      return self._fail(out_name, console, f'{type(e).__name__}: {e}')
    finally:
      if self._tls.page is not None and not self._tls.page.is_closed():
        try:
          # Explicitly release the iframe's GPU device / WebGL context before detaching the DOM
          # node so Chromium never accumulates live contexts while waiting on garbage collection.
          self._tls.page.evaluate(
              """() => {
                  const f = document.getElementById('runner');
                  if (!f) { return; }
                  try {
                    const win = f.contentWindow;
                    const app = win && win.filamentApp;
                    if (app && app.module && app.module.preinitializedWebGPUDevice) {
                      app.module.preinitializedWebGPUDevice.destroy();
                      app.module.preinitializedWebGPUDevice = null;
                    }
                    const canvas = win && win.document && win.document.getElementById('canvas');
                    if (canvas) {
                      const gl = canvas.getContext('webgl2');
                      if (gl) {
                        const lose = gl.getExtension('WEBGL_lose_context');
                        if (lose) { lose.loseContext(); }
                      }
                      canvas.width = 1;
                      canvas.height = 1;
                    }
                  } catch (_) {}
                  f.src = 'about:blank';
                  f.remove();
              }""")
        except Exception:
          try:
            self._tls.page.close()
          except Exception:
            pass
          self._tls.page = None
      self._server.unregister_token(token)
      self._write_console_log(case, out_name, list(console))

  @staticmethod
  def _write_console_log(case, out_name: str, console: list):
    """Saves the whole browser console next to the render, whether or not the render succeeded.

    A failure prints only the tail of the console to the log, and the CI log is several thousand
    lines long by then. The file travels with the rest of the renderdiff output in the uploaded
    artifact, which is where someone debugging a web failure will be looking. It is written for
    successful renders too, because warnings on a passing render are often the first sign of the
    failure that comes next. Neither compare.py nor update_golden.py reads '.log' files.
    """
    try:
      with open(os.path.join(case.output_dir, f'{out_name}.console.log'), 'w') as f:
        f.write('\n'.join(console))
        if console:
          f.write('\n')
    except OSError as e:
      print(f'{out_name}: could not write the browser console log: {e}')

  def _read_memfs(self, frame, token: str, slot: str, path: str) -> bytes:
    """Uploads a MEMFS file as raw bytes to _LocalServer over loopback HTTP PUT."""
    frame.evaluate(
        """async ({path, url}) => {
            const bytes = filamentApp.module.FS.readFile(path, {encoding: 'binary'});
            const response = await fetch(url, {method: 'PUT', body: bytes});
            if (!response.ok) {
              throw new Error(`uploading ${path}: HTTP ${response.status}`);
            }
        }""",
        {'path': path, 'url': f'{OUTPUT_ROUTE_PREFIX}{token}/{slot}'})
    data = self._server.pop_upload(token, slot)
    if data is None:
      raise RuntimeError(f'upload for {path} missing from server')
    return data

  def _validate_tif(self, tif: bytes):
    """Returns a failure message, or None if the image is usable as a golden."""
    try:
      tags = _read_tiff_tags(tif)
    except ValueError as e:
      return str(e)

    # The dimensions are not checked here. They are a property of the golden, and diffimg
    # already fails a comparison whose sizes differ.
    width = tags.get(_TAG_IMAGE_WIDTH, [0])[0]
    height = tags.get(_TAG_IMAGE_LENGTH, [0])[0]
    if width == 0 or height == 0:
      return f'image has no pixels ({width}x{height})'

    colors = _tiff_distinct_colors(tif, tags)
    if colors is None:
      # Compressed, or a layout we do not parse. An empty frame would compress to almost
      # nothing, so size is a usable proxy.
      if len(tif) < 4096:
        return f'image is suspiciously small ({len(tif)} bytes)'
      return None
    if colors < self.MIN_DISTINCT_COLORS:
      return (f'image is trivial ({colors} distinct colors); the scene probably never rendered')
    return None

  @staticmethod
  def _fail(out_name: str, console: list, message: str) -> tuple:
    tail = '\n'.join(f'  {line}' for line in console[-30:])
    important_print(f'{out_name} rendering failed: {message}')
    if tail:
      print(f'--- browser console (tail) ---\n{tail}')
    return 1, RESULT_FAILED

  # ---------------------------------------------------------------- driver

  def _worker_loop(self, case_queue: queue.Queue, abort_event: threading.Event) -> list:
    from playwright.sync_api import sync_playwright

    worker_results = []
    with sync_playwright() as pw:
      # channel='chromium' selects the full browser. Playwright's default headless build is
      # chrome-headless-shell, which has no WebGPU support at all.
      browser = pw.chromium.launch(
          channel='chromium',
          headless=True,
          args=DETERMINISM_FLAGS + BACKEND_FLAGS[self.backend])
      try:
        self._check_browser_version(browser)
        self._tls.browser = browser
        self._tls.console = []
        self._tls.page = self._init_host_page(browser, self._tls.console)

        while not abort_event.is_set():
          try:
            case = case_queue.get_nowait()
          except queue.Empty:
            break
          worker_results.append(case.render(self))
      except RasterizerMismatch:
        abort_event.set()
        raise
      finally:
        if getattr(self._tls, 'page', None) is not None:
          try:
            self._tls.page.close()
          except Exception:
            pass
          self._tls.page = None
        self._tls.browser = None
        browser.close()
    return worker_results

  def run_tests(self, test_config, output_dir: str) -> list:
    cases = self._collect_test_cases(test_config, output_dir)
    if not cases:
      return []

    num_workers = max(1, min(self.concurrent_renders, len(cases)))
    print(f'Rendering {self.platform}-{self.backend} with up to '
          f'{num_workers} concurrent browser workers')

    case_queue = queue.Queue()
    # Start larger models first so a single heavy asset does not trail at the end of the queue.
    ordered_cases = sorted(
        cases,
        key=lambda c: os.path.getsize(c.model_path) if os.path.exists(c.model_path) else 0,
        reverse=True)
    for case in ordered_cases:
      case_queue.put(case)

    abort_event = threading.Event()
    results = []
    self._server = _LocalServer(self.bundle_dir)
    try:
      with concurrent.futures.ThreadPoolExecutor(max_workers=num_workers) as executor:
        futures = [
            executor.submit(self._worker_loop, case_queue, abort_event)
            for _ in range(num_workers)
        ]
        for future in concurrent.futures.as_completed(futures):
          results.extend(future.result())
    finally:
      self._server.shutdown()
      self._server = None
    return results
