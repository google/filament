// tocio WASM smoke test (node). Parses a config, builds a processor, applies
// it to a float buffer on the CPU, and emits GLSL + Metal shaders.
//
// Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
// SPDX-License-Identifier: BSD-3-Clause
import factory from '../../../build/tocio.mjs';

const cfg_text = [
  'ocio_profile_version: 2',
  'colorspaces:',
  '  - !<ColorSpace>',
  '    name: lin',
  '    isdata: false',
  '  - !<ColorSpace>',
  '    name: scaled',
  '    isdata: false',
  '    from_reference: !<MatrixTransform> {matrix: [2,0,0,0, 0,2,0,0, 0,0,2,0, 0,0,0,1]}',
  '',
].join('\n');

const M = await factory();

function cstr(s) {
  const n = M.lengthBytesUTF8(s) + 1;
  const p = M._malloc(n);
  M.stringToUTF8(s, p, n);
  return p;
}

let fail = 0;
function check(cond, msg) {
  if (cond) console.log('  ok:', msg);
  else { console.log('  FAIL:', msg); fail++; }
}

const pCfg = cstr(cfg_text);
const cfg = M._tocw_parse(pCfg, M.lengthBytesUTF8(cfg_text));
M._free(pCfg);
check(cfg !== 0, 'parse config');
check(M._tocw_num_colorspaces(cfg) === 2, '2 colorspaces');

const pSrc = cstr('lin'), pDst = cstr('scaled');
const ops = M._tocw_processor(cfg, pSrc, pDst);
M._free(pSrc); M._free(pDst);
check(ops !== 0, 'build processor lin->scaled');

// apply to 2 RGBA pixels
const npix = 2, ch = 4;
const buf = M._malloc(npix * ch * 4);
const f32 = new Float32Array(npix * ch);
for (let i = 0; i < f32.length; ++i) f32[i] = (i % 4 === 3) ? 1.0 : 0.3;
M.HEAPF32.set(f32, buf >> 2);
const rc = M._tocw_apply(ops, buf, npix, ch);
check(rc === 0, 'apply ok');
const out = M.HEAPF32.subarray(buf >> 2, (buf >> 2) + npix * ch);
check(Math.abs(out[0] - 0.6) < 1e-5, `scaled to 0.6 (got ${out[0].toFixed(4)})`);
M._free(buf);

// emit GLSL (WebGL2 / ES3.0)
const pGlsl = M._tocw_emit_glsl(ops, 0);
check(pGlsl !== 0, 'emit GLSL');
const glsl = M.UTF8ToString(pGlsl);
check(glsl.includes('OCIOMain') && glsl.includes('#version 300 es'),
      'GLSL has OCIOMain + ES3.0 version');
M._tocw_free_str(pGlsl);

// emit Metal (macOS/iOS GPU)
const pMetal = M._tocw_emit_metal(ops);
check(pMetal !== 0, 'emit Metal');
const metal = M.UTF8ToString(pMetal);
check(metal.includes('#include <metal_stdlib>') &&
      metal.includes('float4 OCIOMain(float4') && metal.includes('float4x4('),
      'Metal has metal_stdlib + OCIOMain + float4x4 matrix');
M._tocw_free_str(pMetal);

M._tocw_free_ops(ops);
M._tocw_free_config(cfg);

console.log(fail ? `\n${fail} failed` : '\nall tocio WASM smoke checks passed');
process.exit(fail ? 1 : 0);
