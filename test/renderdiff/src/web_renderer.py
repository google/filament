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

import base64
import functools
import http.server
import json
import os
import socketserver
import struct
import threading

from renderers import BaseRenderer, GltfRenderTestCase
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


# Synthetic origin-relative path used to hand host files to the page without copying them into
# the served directory. Intercepted with page.route().
ASSET_ROUTE_PREFIX = '/__renderdiff_asset__/'

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
  """Serves the WASM bundle directory over loopback on an ephemeral port."""

  def __init__(self, directory: str):

    class Handler(http.server.SimpleHTTPRequestHandler):

      def log_message(self, fmt, *args):
        pass  # The default handler spams stderr with one line per request.

    handler = functools.partial(Handler, directory=directory)

    class Server(socketserver.TCPServer):
      allow_reuse_address = True
      daemon_threads = True

    self._httpd = Server(('127.0.0.1', 0), handler)
    self.port = self._httpd.server_address[1]
    self._thread = threading.Thread(target=self._httpd.serve_forever, daemon=True)
    self._thread.start()

  @property
  def base_url(self) -> str:
    return f'http://127.0.0.1:{self.port}'

  def shutdown(self):
    self._httpd.shutdown()
    self._httpd.server_close()


class WebGltfRenderTestCase(GltfRenderTestCase):
  """A gltf test rendered in a browser. Only the execution differs from the desktop case."""

  def _execute_render(self, renderer, env, working_dir, out_tif_name):
    return renderer.render_in_browser(self, out_tif_name)


class WebRenderer(BaseRenderer):
  """Renders renderdiff tests with the WASM build of gltf_viewer inside headless Chromium."""

  # The size the web goldens are stored at.
  RENDER_WIDTH = 512
  RENDER_HEIGHT = 512

  # ViewerGui sizes its sidebar from its contents (ViewerGui.cpp sets mSidebarWidth from
  # ImGui::GetWindowWidth()), not from ViewerGui::DEFAULT_SIDEBAR_WIDTH, so the value is
  # emergent rather than declared. FilamentApp2 then renders the scene into the window minus
  # the sidebar, which is why the window we ask for is wider than the image we want. Measured
  # at --force-device-scale-factor=1. If ImGui styling ever changes this drifts -- which is
  # precisely why _validate_tif() asserts the dimensions of the TIFF that actually came out.
  SIDEBAR_WIDTH = 410

  # A correctly sized but empty render must not pass as a golden.
  MIN_DISTINCT_COLORS = 16

  def __init__(self, platform: str, backend: str, executable: str, num_threads: int = None,
               test_filter: str = None):
    super().__init__(platform, backend, executable)
    # Rendering is sequential: each test needs its own wasm instance (a second callMain() on a
    # module that already built an Engine is not supported), and SwiftShader saturates the CPU
    # anyway. num_threads is accepted so the CLI is uniform with the desktop renderer.
    self.num_threads = num_threads
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
    self._playwright = None
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

  def _start(self):
    from playwright.sync_api import sync_playwright

    self._server = _LocalServer(self.bundle_dir)
    self._playwright = sync_playwright().start()
    # channel='chromium' selects the full browser. Playwright's default headless build is
    # chrome-headless-shell, which has no WebGPU support at all.
    self._browser = self._playwright.chromium.launch(
        channel='chromium',
        headless=True,
        args=DETERMINISM_FLAGS + BACKEND_FLAGS[self.backend])
    self._check_browser_version()

  def _check_browser_version(self):
    version = self._browser.version
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

  def _stop(self):
    if self._browser:
      self._browser.close()
      self._browser = None
    if self._playwright:
      self._playwright.stop()
      self._playwright = None
    if self._server:
      self._server.shutdown()
      self._server = None

  def _check_rasterizer(self, page):
    """Confirms we are on SwiftShader. Recorded once, then asserted."""
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
          return ext ? gl.getParameter(ext.UNMASKED_RENDERER_WEBGL)
                     : gl.getParameter(gl.RENDERER);
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
    page = self._browser.new_page(viewport={'width': 1024, 'height': 768})

    console = []
    page.on('console', lambda msg: console.append(f'[{msg.type}] {msg.text}'))
    page.on('pageerror', lambda err: console.append(f'[pageerror] {err}'))

    try:
      manifest = self._asset_manifest(case.model_path)
      by_index = {str(i): host for i, (_, host) in enumerate(manifest)}

      def serve_asset(route):
        # This must always fulfill. An unhandled exception here would leave the page's fetch()
        # pending forever, and page.evaluate() has no timeout of its own.
        index = route.request.url.rsplit('/', 1)[-1]
        host = by_index.get(index)
        try:
          if host is None:
            raise FileNotFoundError(f'no asset with index {index}')
          with open(host, 'rb') as f:
            body = f.read()
        except OSError as e:
          route.fulfill(status=500, body=str(e))
          return
        route.fulfill(status=200, body=body,
                      headers={'Content-Type': 'application/octet-stream'})

      page.route(f'**{ASSET_ROUTE_PREFIX}*', serve_asset)

      page.goto(f'{self._server.base_url}/gltf_viewer.html?autorun=0', wait_until='load')
      page.wait_for_function(
          "() => globalThis.filamentApp && (filamentApp.status === 'ready' || filamentApp.error)",
          timeout=MODULE_READY_TIMEOUT_MS)
      error = page.evaluate('() => filamentApp.error')
      if error is not None:
        return self._fail(out_name, console, f"page failed before running: {error}")

      self._check_rasterizer(page)

      with open(case.test_json_path) as f:
        spec_text = f.read()

      page.evaluate(
          """async ({spec, files, timeoutMs}) => {
              const fs = filamentApp.module.FS;
              const stage = (path, bytes) => {
                fs.mkdirTree(path.substring(0, path.lastIndexOf('/')));
                fs.writeFile(path, bytes);
              };
              fs.mkdirTree('/work');
              fs.chdir('/work');
              stage('/work/spec.json', new TextEncoder().encode(spec));
              for (const [memfsPath, url] of files) {
                // AbortController, because a hung fetch would hang page.evaluate() and there
                // is no way to interrupt it from the driver side.
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
          }""",
          {
              'spec': spec_text,
              'files': [[memfs, f'{ASSET_ROUTE_PREFIX}{i}']
                        for i, (memfs, _) in enumerate(manifest)],
              'timeoutMs': STAGING_TIMEOUT_MS,
          })

      model_memfs = manifest[0][0] if case.model_path.lower().endswith('.glb') else \
          f'/work/{os.path.basename(case.model_path)}'

      argv = [
          '-a', FILAMENT_API[self.backend],
          '--batch=/work/spec.json',
          f'--window-size={self.RENDER_WIDTH + self.SIDEBAR_WIDTH}x{self.RENDER_HEIGHT}',
          model_memfs,
      ]
      print(f'{out_name}: argv={argv}')
      page.evaluate('(argv) => filamentApp.run(argv)', argv)

      try:
        page.wait_for_function('() => filamentApp.exited || filamentApp.error !== null',
                               timeout=RENDER_TIMEOUT_MS)
      except Exception:
        status = page.evaluate('() => filamentApp.status')
        return self._fail(out_name, console,
                          f"timed out after {RENDER_TIMEOUT_MS}ms in status '{status}'")

      status = page.evaluate('() => filamentApp.status')
      error = page.evaluate('() => filamentApp.error')
      if error is not None:
        return self._fail(out_name, console, f"final status '{status}': {error}")

      listing = page.evaluate(
          "() => filamentApp.module.FS.readdir('/work').filter(n => n[0] !== '.')")

      tif_name = f'{case.test_name}0.tif'
      json_name = f'{case.test_name}0.json'
      if tif_name not in listing:
        return self._fail(out_name, console,
                          f'{tif_name} was not produced; /work contains {listing}')

      tif = self._read_memfs(page, f'/work/{tif_name}')
      problem = self._validate_tif(tif)
      if problem:
        return self._fail(out_name, console, problem)

      with open(out_tif_name, 'wb') as f:
        f.write(tif)

      if json_name in listing:
        settings = self._read_memfs(page, f'/work/{json_name}')
        with open(os.path.join(case.output_dir, f'{out_name}.json'), 'wb') as f:
          f.write(settings)

      important_print(f'{out_name} rendering succeeded.')
      return 0, RESULT_OK
    except RasterizerMismatch:
      # Fatal for every test, not just this one.
      raise
    except Exception as e:
      # One broken model or a flaky page should cost us that test, not the whole suite.
      return self._fail(out_name, console, f'{type(e).__name__}: {e}')
    finally:
      page.close()
      self._write_console_log(case, out_name, console)

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

  @staticmethod
  def _read_memfs(page, path: str) -> bytes:
    """Reads a MEMFS file as base64.

    Returning the raw Uint8Array would marshal a megabyte of pixels as a JSON array of numbers,
    which is an order of magnitude slower than base64.
    """
    encoded = page.evaluate(
        """(path) => {
            const bytes = filamentApp.module.FS.readFile(path, {encoding: 'binary'});
            let binary = '';
            const chunk = 0x8000;
            for (let i = 0; i < bytes.length; i += chunk) {
              binary += String.fromCharCode.apply(null, bytes.subarray(i, i + chunk));
            }
            return btoa(binary);
        }""", path)
    return base64.b64decode(encoded)

  def _validate_tif(self, tif: bytes):
    """Returns a failure message, or None if the image is usable as a golden."""
    try:
      tags = _read_tiff_tags(tif)
    except ValueError as e:
      return str(e)

    width = tags.get(_TAG_IMAGE_WIDTH, [0])[0]
    height = tags.get(_TAG_IMAGE_LENGTH, [0])[0]
    if (width, height) != (self.RENDER_WIDTH, self.RENDER_HEIGHT):
      # Almost certainly SIDEBAR_WIDTH drifting. Refusing the image is the whole point: silently
      # accepting a differently sized render would rewrite every golden on the next update.
      return (f'expected a {self.RENDER_WIDTH}x{self.RENDER_HEIGHT} image but got '
              f'{width}x{height}; SIDEBAR_WIDTH ({self.SIDEBAR_WIDTH}) is probably stale')

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

  def run_tests(self, test_config, output_dir: str) -> list:
    cases = self._collect_test_cases(test_config, output_dir)
    if not cases:
      return []

    results = []
    self._start()
    try:
      for case in cases:
        results.append(case.render(self))
    finally:
      self._stop()
    return results
