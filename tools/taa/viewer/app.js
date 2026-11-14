/* Copyright (C) 2026 The Android Open Source Project
 * SPDX-License-Identifier: Apache-2.0
 *
 * Security note: All DOM updates strictly use createElement / textContent / replaceChildren.
 * No innerHTML, outerHTML, insertAdjacentHTML, document.write, eval, or blocking dialogs are used.
 */

(function () {
  "use strict";

  const REF_SOURCE_ID = "__ref__";
  const RUN_COLORS = [
    "#4da3ff",
    "#ff8a3d",
    "#3ddc97",
    "#c77dff",
    "#ffd166",
    "#ff5c77",
    "#2ec4b6",
    "#f78fb3",
  ];
  const REF_COLOR = "#94a3b8";
  const ZOOM_STOPS = [0.5, 1, 2, 3, 4, 6, 8, 12, 16, 24, 32];

  // Exact IEC 61966-2-1 sRGB -> Linear LUT matching taa_metrics.cpp
  const SRGB_TO_LINEAR = new Float32Array(256);
  for (let i = 0; i < 256; i++) {
    const c = i / 255.0;
    SRGB_TO_LINEAR[i] = c <= 0.04045 ? c / 12.92 : Math.pow((c + 0.055) / 1.055, 2.4);
  }

  // Perceptual 256-entry Inferno colormap LUT (RGB)
  const INFERNO_STOPS = [
    [0.0, [20, 11, 53]],
    [0.25, [87, 16, 110]],
    [0.5, [188, 55, 84]],
    [0.75, [249, 142, 9]],
    [1.0, [252, 255, 164]],
  ];
  const INFERNO_LUT = new Uint8Array(256 * 3);
  for (let i = 0; i < 256; i++) {
    const t = i / 255.0;
    let s0 = INFERNO_STOPS[0];
    let s1 = INFERNO_STOPS[INFERNO_STOPS.length - 1];
    for (let j = 0; j < INFERNO_STOPS.length - 1; j++) {
      if (t >= INFERNO_STOPS[j][0] && t <= INFERNO_STOPS[j + 1][0]) {
        s0 = INFERNO_STOPS[j];
        s1 = INFERNO_STOPS[j + 1];
        break;
      }
    }
    const span = Math.max(1e-6, s1[0] - s0[0]);
    const u = (t - s0[0]) / span;
    INFERNO_LUT[i * 3 + 0] = Math.round(s0[1][0] + u * (s1[1][0] - s0[1][0]));
    INFERNO_LUT[i * 3 + 1] = Math.round(s0[1][1] + u * (s1[1][1] - s0[1][1]));
    INFERNO_LUT[i * 3 + 2] = Math.round(s0[1][2] + u * (s1[1][2] - s0[1][2]));
  }

  // Safe DOM helper
  function el(tag, props, ...children) {
    const node = document.createElement(tag);
    if (props) {
      for (const [k, v] of Object.entries(props)) {
        if (v === undefined || v === null) continue;
        if (k === "className") {
          node.className = v;
        } else if (k === "textContent") {
          node.textContent = String(v);
        } else if (k === "dataset") {
          for (const [dk, dv] of Object.entries(v)) {
            node.dataset[dk] = String(dv);
          }
        } else if (k.startsWith("on") && typeof v === "function") {
          node.addEventListener(k.slice(2).toLowerCase(), v);
        } else {
          node.setAttribute(k, String(v));
        }
      }
    }
    for (const child of children) {
      if (child === null || child === undefined || child === false) continue;
      if (typeof child === "string" || typeof child === "number") {
        node.appendChild(document.createTextNode(String(child)));
      } else {
        node.appendChild(child);
      }
    }
    return node;
  }

  function makeSwatchCanvas(color, size) {
    const c = document.createElement("canvas");
    c.width = size;
    c.height = size;
    c.className = size <= 9 ? "run-swatch" : "swatch-canvas";
    const ctx = c.getContext("2d");
    ctx.fillStyle = color;
    ctx.fillRect(0, 0, size, size);
    return c;
  }

  // Application State
  const state = {
    manifest: null,
    runs: [],
    runsById: new Map(),
    enabledRunIds: new Set(),
    showReferenceInGrid: true,

    selectedScenarioKey: null,
    categoryFilter: "all",
    scenarioSort: "default",

    layout: "grid", // 'grid' | 'split' | 'flip'
    splitSourceA: null,
    splitSourceB: null,
    splitVertical: true,
    splitRatio: 0.5,
    flipSource: null,
    flipAltSource: null,

    layer: "capture", // 'capture' | 'heatmap' | 'signed' | 'flicker' | 'diff_png'
    compareRefId: REF_SOURCE_ID,
    errorMode: "linear_rgb", // 'linear_rgb' | 'linear_luma' | 'srgb_lsb'
    flickerMode: "residual", // 'residual' | 'raw'
    threshold: 0.02, // in linear [0..1] (or converted for LSB)
    gain: 8,
    maskMode: "off", // 'off' | 'outline' | 'gate'
    showHotspots: false,

    frameIndex: 0,
    frameCount: 1,
    playing: false,
    playDirection: 1,
    loopMode: "loop",
    fps: 8,
    lastTickMs: 0,

    chartMetric: "edge_rmse",

    zoomMode: 0, // 0 = Fit, >0 = explicit texel scale
    centerX: 0.5, // normalized [0..1] image focus center
    centerY: 0.5,

    hoverPixel: null, // {x, y}
    pinnedPixel: null, // {x, y}
    loupeSourceId: null,

    // Per-scenario decoded RGBA cache
    scenarioCacheKey: null,
    loadGeneration: 0,
    imageWidth: 512,
    imageHeight: 512,
    // Map<sourceId, { captures: Map<k, ImageEntry>, diffs: Map<k, ImageEntry> }>
    frameBuffers: new Map(),
    referenceBuffers: new Map(), // Map<k, ImageEntry>
    maskBuffer: null, // Uint8Array(W*H) 0 or 255
    flickerCache: new Map(),
    hotspotCache: new Map(),
    processedCanvasCache: new Map(),

    tileViews: [], // [{ sourceId, sourceIdB, canvas, badgeEl }]
  };

  // Offscreen scratch canvas for decoding PNG -> ImageData
  const decodeCanvas = document.createElement("canvas");
  const decodeCtx = decodeCanvas.getContext("2d", { willReadFrequently: true });

  function getRunColor(sourceId) {
    if (sourceId === REF_SOURCE_ID) return REF_COLOR;
    const idx = state.runs.findIndex((r) => r.id === sourceId);
    return idx >= 0 ? RUN_COLORS[idx % RUN_COLORS.length] : "#4da3ff";
  }

  function getSourceLabel(sourceId) {
    if (sourceId === REF_SOURCE_ID) return "Ground Truth Ref";
    const r = state.runsById.get(sourceId);
    return r ? r.label : String(sourceId);
  }

  function getScenarioEntry(run, scenarioKey) {
    if (!run || !run.scenarios) return null;
    return run.scenarios[scenarioKey] || null;
  }

  function getAllSourceOptions() {
    const list = [{ id: REF_SOURCE_ID, label: "Ground Truth Reference" }];
    for (const r of state.runs) {
      list.push({ id: r.id, label: r.label });
    }
    return list;
  }

  // Image loading & decoding
  function loadImageElement(url) {
    return new Promise((resolve, reject) => {
      const img = new Image();
      img.decoding = "async";
      img.onload = () => resolve(img);
      img.onerror = () => reject(new Error("Failed to load image"));
      img.src = url;
    });
  }

  async function decodeUrlToEntry(url) {
    const img = await loadImageElement(url);
    const w = img.naturalWidth || img.width;
    const h = img.naturalHeight || img.height;
    if (decodeCanvas.width !== w || decodeCanvas.height !== h) {
      decodeCanvas.width = w;
      decodeCanvas.height = h;
    }
    decodeCtx.clearRect(0, 0, w, h);
    decodeCtx.drawImage(img, 0, 0);
    const imageData = decodeCtx.getImageData(0, 0, w, h);
    return { img, width: w, height: h, rgba: imageData.data };
  }

  function clearDerivedCaches() {
    state.processedCanvasCache.clear();
  }

  async function loadScenarioAssets(scenarioKey) {
    const gen = ++state.loadGeneration;
    state.scenarioCacheKey = scenarioKey;
    state.frameBuffers.clear();
    state.referenceBuffers.clear();
    state.maskBuffer = null;
    state.flickerCache.clear();
    state.hotspotCache.clear();
    clearDerivedCaches();

    // Determine frame count from series & assets across all runs
    let maxFrames = 0;
    for (const r of state.runs) {
      const sc = getScenarioEntry(r, scenarioKey);
      if (!sc) continue;
      if (sc.assets && Array.isArray(sc.assets.frame_indices) && sc.assets.frame_indices.length > 0) {
        const lastIdx = sc.assets.frame_indices[sc.assets.frame_indices.length - 1];
        maxFrames = Math.max(maxFrames, lastIdx + 1);
      } else if (sc.assets && sc.assets.perf_capture) {
        maxFrames = Math.max(maxFrames, 1);
      }
      if (sc.series) {
        for (const [sName, arr] of Object.entries(sc.series)) {
          if (sName === "frame_gpu_ms") continue;
          if (Array.isArray(arr)) maxFrames = Math.max(maxFrames, arr.length);
        }
      }
    }
    if (maxFrames === 0) {
      // Check if perf-only scenario
      for (const r of state.runs) {
        const sc = getScenarioEntry(r, scenarioKey);
        if (sc && sc.series && Array.isArray(sc.series.frame_gpu_ms)) {
          maxFrames = Math.max(maxFrames, sc.series.frame_gpu_ms.length);
        }
      }
    }
    state.frameCount = Math.max(1, maxFrames);
    if (state.frameIndex >= state.frameCount) {
      state.frameIndex = 0;
    }
    syncFrameControls();

    const loadingEl = document.getElementById("loading-indicator");
    loadingEl.classList.remove("hidden");

    const tasks = [];
    let refSourceAssets = null;
    for (const r of state.runs) {
      const sc = getScenarioEntry(r, scenarioKey);
      if (!sc || !sc.assets) continue;
      if (!refSourceAssets && (Object.keys(sc.assets.references || {}).length > 0 || sc.assets.perf_aliased_reference)) {
        refSourceAssets = sc.assets;
      }
      const runEntry = { captures: new Map(), diffs: new Map() };
      state.frameBuffers.set(r.id, runEntry);

      const capMap = sc.assets.captures || {};
      const capKeys = Object.keys(capMap);
      if (capKeys.length > 0) {
        for (const kStr of capKeys) {
          const k = Number(kStr);
          const url = capMap[kStr];
          tasks.push(
            decodeUrlToEntry(url)
              .then((decoded) => {
                if (gen !== state.loadGeneration) return;
                state.imageWidth = decoded.width;
                state.imageHeight = decoded.height;
                runEntry.captures.set(k, decoded);
              })
              .catch(() => {})
          );
        }
      } else if (sc.assets.perf_capture) {
        tasks.push(
          decodeUrlToEntry(sc.assets.perf_capture)
            .then((decoded) => {
              if (gen !== state.loadGeneration) return;
              state.imageWidth = decoded.width;
              state.imageHeight = decoded.height;
              runEntry.captures.set(0, decoded);
            })
            .catch(() => {})
        );
      }

      const diffMap = sc.assets.diffs || {};
      for (const [kStr, url] of Object.entries(diffMap)) {
        const k = Number(kStr);
        tasks.push(
          decodeUrlToEntry(url)
            .then((decoded) => {
              if (gen !== state.loadGeneration) return;
              runEntry.diffs.set(k, decoded);
            })
            .catch(() => {})
        );
      }
    }

    if (refSourceAssets) {
      const refMap = refSourceAssets.references || {};
      const refKeys = Object.keys(refMap);
      if (refKeys.length > 0) {
        for (const kStr of refKeys) {
          const k = Number(kStr);
          tasks.push(
            decodeUrlToEntry(refMap[kStr])
              .then((decoded) => {
                if (gen !== state.loadGeneration) return;
                state.imageWidth = decoded.width;
                state.imageHeight = decoded.height;
                state.referenceBuffers.set(k, decoded);
              })
              .catch(() => {})
          );
        }
      } else if (refSourceAssets.perf_aliased_reference) {
        tasks.push(
          decodeUrlToEntry(refSourceAssets.perf_aliased_reference)
            .then((decoded) => {
              if (gen !== state.loadGeneration) return;
              state.referenceBuffers.set(0, decoded);
            })
            .catch(() => {})
        );
      }

      if (refSourceAssets.mask) {
        tasks.push(
          decodeUrlToEntry(refSourceAssets.mask)
            .then((decoded) => {
              if (gen !== state.loadGeneration) return;
              const n = decoded.width * decoded.height;
              const mask = new Uint8Array(n);
              for (let i = 0; i < n; i++) {
                mask[i] = decoded.rgba[i * 4] > 127 ? 255 : 0;
              }
              state.maskBuffer = mask;
            })
            .catch(() => {})
        );
      }
    }

    await Promise.all(tasks);
    if (gen !== state.loadGeneration) return;

    loadingEl.classList.add("hidden");
    state.flickerCache.clear();
    state.hotspotCache.clear();
    clearDerivedCaches();
    rebuildTiles();
    renderAll();
  }

  function getCaptureEntry(sourceId, frameIdx) {
    if (sourceId === REF_SOURCE_ID) {
      if (state.referenceBuffers.has(frameIdx)) {
        return state.referenceBuffers.get(frameIdx);
      }
      if (state.referenceBuffers.has(0)) {
        return state.referenceBuffers.get(0);
      }
      return null;
    }
    const runBuf = state.frameBuffers.get(sourceId);
    if (!runBuf) return null;
    if (runBuf.captures.has(frameIdx)) return runBuf.captures.get(frameIdx);
    if (runBuf.captures.has(0)) return runBuf.captures.get(0);
    return null;
  }

  function getDiffEntry(sourceId, frameIdx) {
    const runBuf = state.frameBuffers.get(sourceId);
    if (!runBuf) return null;
    return runBuf.diffs.get(frameIdx) || runBuf.diffs.get(0) || null;
  }

  function linearLumaAt(rgba, offset) {
    const r = SRGB_TO_LINEAR[rgba[offset]];
    const g = SRGB_TO_LINEAR[rgba[offset + 1]];
    const b = SRGB_TO_LINEAR[rgba[offset + 2]];
    return 0.2126 * r + 0.7152 * g + 0.0722 * b;
  }

  function pixelRmseLinear(rgbaA, rgbaB, offset) {
    const dr = SRGB_TO_LINEAR[rgbaA[offset]] - SRGB_TO_LINEAR[rgbaB[offset]];
    const dg = SRGB_TO_LINEAR[rgbaA[offset + 1]] - SRGB_TO_LINEAR[rgbaB[offset + 1]];
    const db = SRGB_TO_LINEAR[rgbaA[offset + 2]] - SRGB_TO_LINEAR[rgbaB[offset + 2]];
    return Math.sqrt((dr * dr + dg * dg + db * db) / 3.0);
  }

  // Per-pixel temporal standard deviation across frames 0..N-1
  function getFlickerMap(sourceId, mode) {
    const cacheKey = `${sourceId}|${mode}`;
    if (state.flickerCache.has(cacheKey)) {
      return state.flickerCache.get(cacheKey);
    }
    const runBuf = state.frameBuffers.get(sourceId);
    if (!runBuf || runBuf.captures.size < 2) {
      return null;
    }
    const frameKeys = Array.from(runBuf.captures.keys()).sort((a, b) => a - b);
    const first = runBuf.captures.get(frameKeys[0]);
    const w = first.width;
    const h = first.height;
    const nPix = w * h;
    const kCount = frameKeys.length;

    const mean = new Float32Array(nPix);
    const m2 = new Float32Array(nPix);

    for (let idx = 0; idx < kCount; idx++) {
      const k = frameKeys[idx];
      const cap = runBuf.captures.get(k);
      const ref = mode === "residual" ? getCaptureEntry(REF_SOURCE_ID, k) : null;
      const capRgba = cap.rgba;
      const refRgba = ref ? ref.rgba : null;
      const count = idx + 1;
      for (let p = 0; p < nPix; p++) {
        const off = p * 4;
        let val = linearLumaAt(capRgba, off);
        if (refRgba) {
          val -= linearLumaAt(refRgba, off);
        }
        const delta = val - mean[p];
        mean[p] += delta / count;
        const delta2 = val - mean[p];
        m2[p] += delta * delta2;
      }
    }

    const sigma = new Float32Array(nPix);
    const sorted = new Float32Array(nPix);
    for (let p = 0; p < nPix; p++) {
      const s = Math.sqrt(m2[p] / kCount);
      sigma[p] = s;
      sorted[p] = s;
    }
    sorted.sort();
    const top1Start = Math.max(0, Math.floor(nPix * 0.99));
    let sqSum = 0;
    let topCount = 0;
    for (let i = top1Start; i < nPix; i++) {
      sqSum += sorted[i] * sorted[i];
      topCount++;
    }
    const top1Rms = topCount > 0 ? Math.sqrt(sqSum / topCount) : 0;
    const res = { width: w, height: h, sigma, top1Rms };
    state.flickerCache.set(cacheKey, res);
    return res;
  }

  // Exact 10x10 grid (8x8 inner core) P95 hotspot computation matching taa_metrics.cpp
  function sortedPercentile(sortedArr, q) {
    const n = sortedArr.length;
    if (n === 0) return 0;
    if (n === 1) return sortedArr[0];
    const pos = q * (n - 1);
    const lo = Math.floor(pos);
    const hi = Math.min(n - 1, lo + 1);
    const frac = pos - lo;
    return sortedArr[lo] * (1 - frac) + sortedArr[hi] * frac;
  }

  function getHotspotCells(sourceId, frameIdx) {
    if (sourceId === REF_SOURCE_ID) return null;
    const cacheKey = `${sourceId}|${frameIdx}|${state.compareRefId}`;
    if (state.hotspotCache.has(cacheKey)) {
      return state.hotspotCache.get(cacheKey);
    }
    const cap = getCaptureEntry(sourceId, frameIdx);
    const ref = getCaptureEntry(state.compareRefId, frameIdx);
    if (!cap || !ref || cap.width !== ref.width || cap.height !== ref.height) {
      return null;
    }
    const w = cap.width;
    const h = cap.height;
    const mask = state.maskBuffer;
    const cells = [];

    for (let gr = 1; gr <= 8; gr++) {
      const y0 = Math.floor((gr * h) / 10);
      const y1 = Math.floor(((gr + 1) * h) / 10);
      for (let gc = 1; gc <= 8; gc++) {
        const x0 = Math.floor((gc * w) / 10);
        const x1 = Math.floor(((gc + 1) * w) / 10);
        const vals = [];
        for (let y = y0; y < y1; y++) {
          for (let x = x0; x < x1; x++) {
            const p = y * w + x;
            const off = p * 4;
            const isFg = (mask && mask[p] > 0) || linearLumaAt(ref.rgba, off) > 1e-4;
            if (!isFg) continue;
            vals.push(pixelRmseLinear(cap.rgba, ref.rgba, off));
          }
        }
        if (vals.length >= 10) {
          vals.sort((a, b) => a - b);
          const p95 = sortedPercentile(vals, 0.95);
          cells.push({ gr, gc, x0, y0, x1, y1, p95 });
        }
      }
    }
    cells.sort((a, b) => b.p95 - a.p95);
    const top10 = cells.slice(0, 10);
    state.hotspotCache.set(cacheKey, top10);
    return top10;
  }

  // Synthesize displayable offscreen canvas for (sourceId, frameIdx) under current layer settings
  function getProcessedSourceFrame(sourceId, frameIdx) {
    const cap = getCaptureEntry(sourceId, frameIdx);
    if (!cap) return null;

    if (state.layer === "capture" || sourceId === state.compareRefId) {
      if (state.layer === "diff_png" && sourceId !== REF_SOURCE_ID) {
        const diffEnt = getDiffEntry(sourceId, frameIdx);
        if (diffEnt) return { canvas: diffEnt.img, statsText: "diff PNG" };
      }
      if (state.layer === "capture") {
        return { canvas: cap.img, statsText: "" };
      }
      if (sourceId === REF_SOURCE_ID && state.layer !== "capture") {
        return { canvas: cap.img, statsText: "reference" };
      }
    }

    if (state.layer === "diff_png") {
      const diffEnt = getDiffEntry(sourceId, frameIdx);
      return diffEnt ? { canvas: diffEnt.img, statsText: "diff PNG" } : { canvas: cap.img, statsText: "no diff PNG" };
    }

    const cacheKey = [
      sourceId,
      frameIdx,
      state.layer,
      state.compareRefId,
      state.errorMode,
      state.flickerMode,
      state.threshold.toFixed(4),
      state.gain,
      state.maskMode,
    ].join("|");

    if (state.processedCanvasCache.has(cacheKey)) {
      return state.processedCanvasCache.get(cacheKey);
    }

    const w = cap.width;
    const h = cap.height;
    const nPix = w * h;
    const outCanvas = document.createElement("canvas");
    outCanvas.width = w;
    outCanvas.height = h;
    const outCtx = outCanvas.getContext("2d");
    const outImg = outCtx.createImageData(w, h);
    const dst = outImg.data;
    const src = cap.rgba;
    const mask = state.maskBuffer;
    const gateMask = state.maskMode === "gate" && mask !== null;

    let statsText = "";

    if (state.layer === "flicker") {
      const fMap = getFlickerMap(sourceId, state.flickerMode);
      if (!fMap) {
        return { canvas: cap.img, statsText: "need >=2 frames" };
      }
      const tau = state.threshold;
      const scale = state.gain * 8.0;
      let exceed = 0;
      let denom = 0;
      for (let p = 0; p < nPix; p++) {
        const off = p * 4;
        const allowed = !gateMask || mask[p] > 0;
        if (allowed) denom++;
        const s = allowed ? fMap.sigma[p] : 0;
        if (s > tau) {
          exceed++;
          const norm = Math.min(1.0, (s - tau) * scale);
          const ci = Math.min(255, Math.max(0, Math.round(norm * 255))) * 3;
          dst[off] = INFERNO_LUT[ci];
          dst[off + 1] = INFERNO_LUT[ci + 1];
          dst[off + 2] = INFERNO_LUT[ci + 2];
          dst[off + 3] = 255;
        } else {
          const dim = Math.round((0.2126 * src[off] + 0.7152 * src[off + 1] + 0.0722 * src[off + 2]) * 0.26);
          dst[off] = dim;
          dst[off + 1] = dim;
          dst[off + 2] = dim;
          dst[off + 3] = 255;
        }
      }
      const pct = denom > 0 ? ((exceed / denom) * 100).toFixed(1) : "0.0";
      statsText = `top1% ${fMap.top1Rms.toFixed(4)} | ${pct}% > tau`;
    } else {
      const ref = getCaptureEntry(state.compareRefId, frameIdx);
      if (!ref || ref.width !== w || ref.height !== h) {
        return { canvas: cap.img, statsText: "no ref frame" };
      }
      const refRgba = ref.rgba;
      const tau = state.threshold;
      const gain = state.gain;
      let exceed = 0;
      let denom = 0;

      if (state.layer === "heatmap") {
        for (let p = 0; p < nPix; p++) {
          const off = p * 4;
          const allowed = !gateMask || mask[p] > 0;
          if (allowed) denom++;
          let err = 0;
          if (allowed) {
            if (state.errorMode === "linear_rgb") {
              err = pixelRmseLinear(src, refRgba, off);
            } else if (state.errorMode === "linear_luma") {
              err = Math.abs(linearLumaAt(src, off) - linearLumaAt(refRgba, off));
            } else {
              const m = Math.max(
                Math.abs(src[off] - refRgba[off]),
                Math.abs(src[off + 1] - refRgba[off + 1]),
                Math.abs(src[off + 2] - refRgba[off + 2])
              );
              err = m / 255.0;
            }
          }
          if (err > tau) {
            exceed++;
            const norm = Math.min(1.0, (err - tau) * gain);
            const ci = Math.min(255, Math.max(0, Math.round(norm * 255))) * 3;
            dst[off] = INFERNO_LUT[ci];
            dst[off + 1] = INFERNO_LUT[ci + 1];
            dst[off + 2] = INFERNO_LUT[ci + 2];
            dst[off + 3] = 255;
          } else {
            const dim = Math.round((0.2126 * src[off] + 0.7152 * src[off + 1] + 0.0722 * src[off + 2]) * 0.26);
            dst[off] = dim;
            dst[off + 1] = dim;
            dst[off + 2] = dim;
            dst[off + 3] = 255;
          }
        }
        const pct = denom > 0 ? ((exceed / denom) * 100).toFixed(1) : "0.0";
        statsText = `${pct}% px > tau`;
      } else if (state.layer === "signed") {
        for (let p = 0; p < nPix; p++) {
          const off = p * 4;
          const allowed = !gateMask || mask[p] > 0;
          if (allowed) denom++;
          const dy = allowed ? linearLumaAt(src, off) - linearLumaAt(refRgba, off) : 0;
          if (Math.abs(dy) > tau) {
            exceed++;
            const mag = Math.min(1.0, (Math.abs(dy) - tau) * gain);
            if (dy > 0) {
              // Overshoot / bright ghosting -> warm red-orange-yellow
              dst[off] = Math.round(90 + 165 * mag);
              dst[off + 1] = Math.round(24 + 175 * Math.pow(mag, 1.6));
              dst[off + 2] = Math.round(28 * (1 - mag));
            } else {
              // Darkening / energy loss -> cyan-blue
              dst[off] = Math.round(20 * (1 - mag));
              dst[off + 1] = Math.round(70 + 165 * Math.pow(mag, 1.3));
              dst[off + 2] = Math.round(120 + 135 * mag);
            }
            dst[off + 3] = 255;
          } else {
            const dim = Math.round((0.2126 * src[off] + 0.7152 * src[off + 1] + 0.0722 * src[off + 2]) * 0.26);
            dst[off] = dim;
            dst[off + 1] = dim;
            dst[off + 2] = dim;
            dst[off + 3] = 255;
          }
        }
        const pct = denom > 0 ? ((exceed / denom) * 100).toFixed(1) : "0.0";
        statsText = `${pct}% |dY| > tau`;
      }
    }

    outCtx.putImageData(outImg, 0, 0);
    // Bound cache size
    if (state.processedCanvasCache.size > 96) {
      const oldest = state.processedCanvasCache.keys().next().value;
      state.processedCanvasCache.delete(oldest);
    }
    const entry = { canvas: outCanvas, statsText };
    state.processedCanvasCache.set(cacheKey, entry);
    return entry;
  }

  // Compute viewport transform mapping image [0..W, 0..H] into canvas CSS pixels [0..cw, 0..ch]
  function computeViewRect(cw, ch, imgW, imgH) {
    const fitScale = Math.min(cw / imgW, ch / imgH) * 0.98;
    const scale = state.zoomMode === 0 ? fitScale : state.zoomMode;
    const drawW = imgW * scale;
    const drawH = imgH * scale;
    let ox = cw * 0.5 - state.centerX * drawW;
    let oy = ch * 0.5 - state.centerY * drawH;
    if (state.zoomMode === 0) {
      ox = (cw - drawW) * 0.5;
      oy = (ch - drawH) * 0.5;
    }
    return { ox, oy, drawW, drawH, scale };
  }

  function drawTileOverlays(ctx, rect, sourceId, cw, ch) {
    const imgW = state.imageWidth;
    const imgH = state.imageHeight;

    // Silhouette mask outline overlay
    if (state.maskMode === "outline" && state.maskBuffer) {
      const mask = state.maskBuffer;
      ctx.fillStyle = "rgba(61, 220, 151, 0.65)";
      const step = rect.scale < 1 ? 2 : 1;
      for (let y = 1; y < imgH - 1; y += step) {
        for (let x = 1; x < imgW - 1; x += step) {
          const p = y * imgW + x;
          if (mask[p] === 0) continue;
          if (mask[p - 1] === 0 || mask[p + 1] === 0 || mask[p - imgW] === 0 || mask[p + imgW] === 0) {
            const sx = rect.ox + x * rect.scale;
            const sy = rect.oy + y * rect.scale;
            ctx.fillRect(sx, sy, Math.max(1, rect.scale), Math.max(1, rect.scale));
          }
        }
      }
    }

    // 8x8 inner-core P95 hotspot grid overlay
    if (state.showHotspots && sourceId !== REF_SOURCE_ID) {
      const top10 = getHotspotCells(sourceId, state.frameIndex);
      // Draw faint 8x8 inner grid boundary
      const innerX = rect.ox + Math.floor(imgW / 10) * rect.scale;
      const innerY = rect.oy + Math.floor(imgH / 10) * rect.scale;
      const innerW = (Math.floor((9 * imgW) / 10) - Math.floor(imgW / 10)) * rect.scale;
      const innerH = (Math.floor((9 * imgH) / 10) - Math.floor(imgH / 10)) * rect.scale;
      ctx.strokeStyle = "rgba(168, 182, 204, 0.25)";
      ctx.lineWidth = 1;
      ctx.strokeRect(innerX, innerY, innerW, innerH);

      if (top10) {
        ctx.font = "10px ui-monospace, SFMono-Regular, Menlo, monospace";
        for (let idx = top10.length - 1; idx >= 0; idx--) {
          const cell = top10[idx];
          const cx = rect.ox + cell.x0 * rect.scale;
          const cy = rect.oy + cell.y0 * rect.scale;
          const cellW = (cell.x1 - cell.x0) * rect.scale;
          const cellH = (cell.y1 - cell.y0) * rect.scale;
          const isPeak = idx === 0;
          ctx.strokeStyle = isPeak ? "#ff5c77" : "#ffd166";
          ctx.lineWidth = isPeak ? 2 : 1.25;
          ctx.strokeRect(cx, cy, cellW, cellH);
          if (cellW >= 34 && cellH >= 16) {
            ctx.fillStyle = "rgba(11, 14, 20, 0.78)";
            ctx.fillRect(cx + 1, cy + 1, Math.min(cellW - 2, 44), 13);
            ctx.fillStyle = isPeak ? "#ff5c77" : "#ffd166";
            ctx.fillText(`#${idx + 1} ${cell.p95.toFixed(3)}`, cx + 3, cy + 11);
          }
        }
      }
    }

    // Active / pinned pixel crosshair across all synchronized tiles
    const targetPx = state.pinnedPixel || state.hoverPixel;
    if (targetPx) {
      const px = rect.ox + (targetPx.x + 0.5) * rect.scale;
      const py = rect.oy + (targetPx.y + 0.5) * rect.scale;
      const color = state.pinnedPixel ? "#ffd166" : "rgba(77, 163, 255, 0.85)";
      ctx.strokeStyle = color;
      ctx.lineWidth = 1;
      if (rect.scale >= 4) {
        ctx.strokeRect(
          rect.ox + targetPx.x * rect.scale,
          rect.oy + targetPx.y * rect.scale,
          rect.scale,
          rect.scale
        );
      } else {
        ctx.beginPath();
        ctx.moveTo(px - 8, py);
        ctx.lineTo(px - 2, py);
        ctx.moveTo(px + 2, py);
        ctx.lineTo(px + 8, py);
        ctx.moveTo(px, py - 8);
        ctx.lineTo(px, py - 2);
        ctx.moveTo(px, py + 2);
        ctx.lineTo(px, py + 8);
        ctx.stroke();
      }
      // Subtle edge ticks on viewport border
      ctx.fillStyle = color;
      ctx.fillRect(px - 1, 0, 2, 4);
      ctx.fillRect(0, py - 1, 4, 2);
    }
  }

  function renderTile(tile) {
    const canvas = tile.canvas;
    const dpr = window.devicePixelRatio || 1;
    const cssW = canvas.clientWidth;
    const cssH = canvas.clientHeight;
    if (cssW <= 0 || cssH <= 0) return;
    const targetW = Math.round(cssW * dpr);
    const targetH = Math.round(cssH * dpr);
    if (canvas.width !== targetW || canvas.height !== targetH) {
      canvas.width = targetW;
      canvas.height = targetH;
    }

    const ctx = canvas.getContext("2d");
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.fillStyle = "#07090d";
    ctx.fillRect(0, 0, cssW, cssH);
    ctx.imageSmoothingEnabled = false;

    const imgW = state.imageWidth;
    const imgH = state.imageHeight;
    const rect = computeViewRect(cssW, cssH, imgW, imgH);

    if (tile.mode === "split") {
      const frameA = getProcessedSourceFrame(tile.sourceId, state.frameIndex);
      const frameB = getProcessedSourceFrame(tile.sourceIdB, state.frameIndex);
      if (!frameA && !frameB) {
        drawMissingFrameMessage(ctx, cssW, cssH);
        tile.badgeEl.textContent = "no frame dump";
        return;
      }

      const splitPos = state.splitVertical ? cssW * state.splitRatio : cssH * state.splitRatio;

      if (frameA) {
        ctx.save();
        ctx.beginPath();
        if (state.splitVertical) {
          ctx.rect(0, 0, splitPos, cssH);
        } else {
          ctx.rect(0, 0, cssW, splitPos);
        }
        ctx.clip();
        ctx.drawImage(frameA.canvas, rect.ox, rect.oy, rect.drawW, rect.drawH);
        drawTileOverlays(ctx, rect, tile.sourceId, cssW, cssH);
        ctx.restore();
      }

      if (frameB) {
        ctx.save();
        ctx.beginPath();
        if (state.splitVertical) {
          ctx.rect(splitPos, 0, cssW - splitPos, cssH);
        } else {
          ctx.rect(0, splitPos, cssW, cssH - splitPos);
        }
        ctx.clip();
        ctx.drawImage(frameB.canvas, rect.ox, rect.oy, rect.drawW, rect.drawH);
        drawTileOverlays(ctx, rect, tile.sourceIdB, cssW, cssH);
        ctx.restore();
      }

      // Draw split divider line + handle
      ctx.strokeStyle = "#4da3ff";
      ctx.lineWidth = 1.5;
      ctx.beginPath();
      if (state.splitVertical) {
        ctx.moveTo(splitPos, 0);
        ctx.lineTo(splitPos, cssH);
      } else {
        ctx.moveTo(0, splitPos);
        ctx.lineTo(cssW, splitPos);
      }
      ctx.stroke();

      // Corner labels
      drawCornerPill(ctx, 8, 8, `A: ${getSourceLabel(tile.sourceId)}`, getRunColor(tile.sourceId));
      const bLabel = `B: ${getSourceLabel(tile.sourceIdB)}`;
      const bWidth = ctx.measureText(bLabel).width + 20;
      drawCornerPill(
        ctx,
        state.splitVertical ? Math.max(splitPos + 8, cssW - bWidth - 8) : 8,
        state.splitVertical ? 8 : Math.max(splitPos + 8, cssH - 28),
        bLabel,
        getRunColor(tile.sourceIdB)
      );

      const statParts = [];
      if (frameA && frameA.statsText) statParts.push(`A: ${frameA.statsText}`);
      if (frameB && frameB.statsText) statParts.push(`B: ${frameB.statsText}`);
      tile.badgeEl.textContent = statParts.join("  |  ") || `${imgW}x${imgH}`;
      return;
    }

    const frame = getProcessedSourceFrame(tile.sourceId, state.frameIndex);
    if (!frame) {
      drawMissingFrameMessage(ctx, cssW, cssH);
      tile.badgeEl.textContent = "metrics only (run with -d to inspect frames)";
      return;
    }

    ctx.drawImage(frame.canvas, rect.ox, rect.oy, rect.drawW, rect.drawH);
    drawTileOverlays(ctx, rect, tile.sourceId, cssW, cssH);

    // Attach scenario frame metric readout to tile badge
    const scMetric = getFrameScalarReadout(tile.sourceId, state.frameIndex);
    const parts = [];
    if (frame.statsText) parts.push(frame.statsText);
    if (scMetric) parts.push(scMetric);
    tile.badgeEl.textContent = parts.join("  |  ") || `${imgW}x${imgH}`;
  }

  function drawCornerPill(ctx, x, y, text, dotColor) {
    ctx.font = "600 11px -apple-system, BlinkMacSystemFont, sans-serif";
    const w = ctx.measureText(text).width + 22;
    ctx.fillStyle = "rgba(11, 14, 20, 0.82)";
    ctx.fillRect(x, y, w, 20);
    ctx.strokeStyle = "#283347";
    ctx.lineWidth = 1;
    ctx.strokeRect(x, y, w, 20);
    ctx.fillStyle = dotColor;
    ctx.beginPath();
    ctx.arc(x + 9, y + 10, 3.5, 0, Math.PI * 2);
    ctx.fill();
    ctx.fillStyle = "#e6edf7";
    ctx.fillText(text, x + 17, y + 14);
  }

  function drawMissingFrameMessage(ctx, cw, ch) {
    ctx.fillStyle = "#6f7f99";
    ctx.font = "12px ui-monospace, SFMono-Regular, Menlo, monospace";
    ctx.textAlign = "center";
    ctx.fillText("No frame PNGs dumped for this scenario (pass -d <dir> to taa_harness)", cw * 0.5, ch * 0.5);
    ctx.textAlign = "left";
  }

  function getFrameScalarReadout(sourceId, frameIdx) {
    if (sourceId === REF_SOURCE_ID) return "ground truth";
    const run = state.runsById.get(sourceId);
    const sc = getScenarioEntry(run, state.selectedScenarioKey);
    if (!sc || !sc.series) return "";
    const arr = sc.series[state.chartMetric];
    if (Array.isArray(arr) && frameIdx < arr.length && typeof arr[frameIdx] === "number") {
      return `${state.chartMetric}: ${arr[frameIdx].toFixed(4)}`;
    }
    return "";
  }

  // Build / rebuild synchronized viewport tiles
  function rebuildTiles() {
    const container = document.getElementById("tiles-container");
    state.tileViews = [];

    const sources = [];
    if (state.layout === "grid") {
      if (state.showReferenceInGrid && state.referenceBuffers.size > 0) {
        sources.push({ mode: "single", sourceId: REF_SOURCE_ID });
      }
      for (const r of state.runs) {
        if (state.enabledRunIds.has(r.id)) {
          sources.push({ mode: "single", sourceId: r.id });
        }
      }
      if (sources.length === 0 && state.runs.length > 0) {
        sources.push({ mode: "single", sourceId: state.runs[0].id });
      }
    } else if (state.layout === "split") {
      sources.push({
        mode: "split",
        sourceId: state.splitSourceA || state.runs[0]?.id || REF_SOURCE_ID,
        sourceIdB: state.splitSourceB || state.runs[1]?.id || REF_SOURCE_ID,
      });
    } else {
      sources.push({
        mode: "single",
        sourceId: state.flipSource || state.runs[0]?.id || REF_SOURCE_ID,
      });
    }

    container.classList.remove("tiles-cols-1", "tiles-cols-2", "tiles-cols-3");
    if (sources.length <= 1) {
      container.classList.add("tiles-cols-1");
    } else if (sources.length === 2 || sources.length === 4) {
      container.classList.add("tiles-cols-2");
    } else {
      container.classList.add("tiles-cols-3");
    }

    const nodes = [];
    for (const spec of sources) {
      const badgeEl = el("span", { className: "tile-badge", textContent: "" });
      const titleText =
        spec.mode === "split"
          ? `${getSourceLabel(spec.sourceId)}  vs  ${getSourceLabel(spec.sourceIdB)}`
          : getSourceLabel(spec.sourceId);
      const titleNode = el(
        "div",
        { className: "tile-title" },
        makeSwatchCanvas(getRunColor(spec.sourceId), 9),
        el("span", { textContent: titleText })
      );
      const header = el("div", { className: "tile-header" }, titleNode, badgeEl);
      const canvas = el("canvas", { className: "tile-canvas" });
      const wrap = el("div", { className: "tile-canvas-wrap" }, canvas);
      const tileNode = el("div", { className: "tile" }, header, wrap);

      const tileObj = {
        mode: spec.mode,
        sourceId: spec.sourceId,
        sourceIdB: spec.sourceIdB || null,
        canvas,
        badgeEl,
      };
      attachCanvasInteractions(tileObj);
      state.tileViews.push(tileObj);
      nodes.push(tileNode);
    }

    container.replaceChildren(...nodes);
  }

  function clientEventToImageCoords(canvas, clientX, clientY) {
    const bounds = canvas.getBoundingClientRect();
    const lx = clientX - bounds.left;
    const ly = clientY - bounds.top;
    const rect = computeViewRect(bounds.width, bounds.height, state.imageWidth, state.imageHeight);
    const ix = Math.floor((lx - rect.ox) / rect.scale);
    const iy = Math.floor((ly - rect.oy) / rect.scale);
    return { lx, ly, bounds, rect, ix, iy };
  }

  function attachCanvasInteractions(tileObj) {
    const canvas = tileObj.canvas;
    let draggingMode = null; // 'pan' | 'split'
    let moved = false;
    let startX = 0;
    let startY = 0;
    let startCenterX = 0.5;
    let startCenterY = 0.5;

    canvas.addEventListener("pointerdown", (ev) => {
      if (ev.button !== 0) return;
      canvas.setPointerCapture(ev.pointerId);
      const info = clientEventToImageCoords(canvas, ev.clientX, ev.clientY);
      moved = false;
      startX = ev.clientX;
      startY = ev.clientY;
      startCenterX = state.centerX;
      startCenterY = state.centerY;

      if (tileObj.mode === "split") {
        const splitPos = state.splitVertical
          ? info.bounds.width * state.splitRatio
          : info.bounds.height * state.splitRatio;
        const coord = state.splitVertical ? info.lx : info.ly;
        if (Math.abs(coord - splitPos) <= 10) {
          draggingMode = "split";
          return;
        }
      }
      draggingMode = "pan";
    });

    canvas.addEventListener("pointermove", (ev) => {
      const info = clientEventToImageCoords(canvas, ev.clientX, ev.clientY);
      if (info.ix >= 0 && info.ix < state.imageWidth && info.iy >= 0 && info.iy < state.imageHeight) {
        state.hoverPixel = { x: info.ix, y: info.iy };
      } else {
        state.hoverPixel = null;
      }

      if (draggingMode === "split") {
        moved = true;
        const raw = state.splitVertical
          ? info.lx / Math.max(1, info.bounds.width)
          : info.ly / Math.max(1, info.bounds.height);
        state.splitRatio = Math.min(0.97, Math.max(0.03, raw));
        renderAllViewports();
        return;
      }

      if (draggingMode === "pan") {
        const dx = ev.clientX - startX;
        const dy = ev.clientY - startY;
        if (Math.hypot(dx, dy) > 3) {
          moved = true;
          if (state.zoomMode === 0) {
            state.zoomMode = Math.max(1, Math.round(info.rect.scale * 10) / 10);
            syncZoomButtons();
          }
          state.centerX = Math.min(
            1.2,
            Math.max(-0.2, startCenterX - dx / Math.max(1, state.imageWidth * state.zoomMode))
          );
          state.centerY = Math.min(
            1.2,
            Math.max(-0.2, startCenterY - dy / Math.max(1, state.imageHeight * state.zoomMode))
          );
        }
      }

      renderAllViewports();
      renderInspector();
    });

    canvas.addEventListener("pointerup", (ev) => {
      if (draggingMode && !moved) {
        const info = clientEventToImageCoords(canvas, ev.clientX, ev.clientY);
        if (info.ix >= 0 && info.ix < state.imageWidth && info.iy >= 0 && info.iy < state.imageHeight) {
          if (
            state.pinnedPixel &&
            state.pinnedPixel.x === info.ix &&
            state.pinnedPixel.y === info.iy
          ) {
            state.pinnedPixel = null;
          } else {
            state.pinnedPixel = { x: info.ix, y: info.iy };
          }
        }
      }
      draggingMode = null;
      renderAllViewports();
      renderInspector();
    });

    canvas.addEventListener("pointerleave", () => {
      if (!draggingMode) {
        state.hoverPixel = null;
        renderAllViewports();
        renderInspector();
      }
    });

    canvas.addEventListener(
      "wheel",
      (ev) => {
        ev.preventDefault();
        const info = clientEventToImageCoords(canvas, ev.clientX, ev.clientY);
        const curScale = info.rect.scale;
        const normX = (info.lx - info.rect.ox) / Math.max(1, info.rect.drawW);
        const normY = (info.ly - info.rect.oy) / Math.max(1, info.rect.drawH);

        let nextZoom = curScale;
        if (ev.deltaY < 0) {
          nextZoom = ZOOM_STOPS.find((z) => z > curScale * 1.05) || ZOOM_STOPS[ZOOM_STOPS.length - 1];
        } else {
          for (let i = ZOOM_STOPS.length - 1; i >= 0; i--) {
            if (ZOOM_STOPS[i] < curScale * 0.95) {
              nextZoom = ZOOM_STOPS[i];
              break;
            }
          }
        }
        state.zoomMode = nextZoom;
        // Keep cursor anchored on the same image texel
        const newDrawW = state.imageWidth * nextZoom;
        const newDrawH = state.imageHeight * nextZoom;
        state.centerX = Math.min(1.2, Math.max(-0.2, normX + (info.bounds.width * 0.5 - info.lx) / newDrawW));
        state.centerY = Math.min(1.2, Math.max(-0.2, normY + (info.bounds.height * 0.5 - info.ly) / newDrawH));
        syncZoomButtons();
        renderAllViewports();
      },
      { passive: false }
    );
  }

  // Timeline Multi-Engine Per-Frame Chart
  function renderTimelineChart() {
    const canvas = document.getElementById("timeline-chart");
    const dpr = window.devicePixelRatio || 1;
    const cw = canvas.clientWidth;
    const ch = canvas.clientHeight;
    if (cw <= 0 || ch <= 0) return;
    if (canvas.width !== Math.round(cw * dpr) || canvas.height !== Math.round(ch * dpr)) {
      canvas.width = Math.round(cw * dpr);
      canvas.height = Math.round(ch * dpr);
    }
    const ctx = canvas.getContext("2d");
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.fillStyle = "#0b0e14";
    ctx.fillRect(0, 0, cw, ch);

    const padL = 52;
    const padR = 14;
    const padT = 16;
    const padB = 20;
    const plotW = Math.max(10, cw - padL - padR);
    const plotH = Math.max(10, ch - padT - padB);

    const metric = state.chartMetric;
    const seriesList = [];
    let maxLen = state.frameCount;
    let minVal = Infinity;
    let maxVal = -Infinity;

    for (const r of state.runs) {
      if (!state.enabledRunIds.has(r.id) && state.runs.length > 1) continue;
      const sc = getScenarioEntry(r, state.selectedScenarioKey);
      if (!sc || !sc.series || !Array.isArray(sc.series[metric])) continue;
      const arr = sc.series[metric];
      maxLen = Math.max(maxLen, arr.length);
      for (const v of arr) {
        if (typeof v === "number" && Number.isFinite(v)) {
          minVal = Math.min(minVal, v);
          maxVal = Math.max(maxVal, v);
        }
      }
      seriesList.push({ run: r, values: arr, color: getRunColor(r.id) });
    }

    if (seriesList.length === 0 || !Number.isFinite(minVal)) {
      ctx.fillStyle = "#6f7f99";
      ctx.font = "11px ui-monospace, SFMono-Regular, Menlo, monospace";
      ctx.fillText(`No per-frame '${metric}' series recorded for this scenario`, padL, ch * 0.5);
      return;
    }

    if (Math.abs(maxVal - minVal) < 1e-6) {
      minVal = Math.max(0, minVal - 0.01);
      maxVal = maxVal + 0.01;
    } else {
      const pad = (maxVal - minVal) * 0.1;
      minVal = Math.max(0, minVal - pad * 0.5);
      maxVal += pad;
    }

    const xFor = (k) => padL + (maxLen <= 1 ? 0 : (k / (maxLen - 1)) * plotW);
    const yFor = (v) => padT + (1 - (v - minVal) / (maxVal - minVal)) * plotH;

    // Motion vs Static Hold shaded halves (for non-static quality regimes)
    const scMeta = state.manifest.scenarios.find((s) => s.key === state.selectedScenarioKey);
    const isMovingScenario =
      scMeta &&
      scMeta.category !== "Static" &&
      scMeta.regime !== "static" &&
      metric !== "frame_gpu_ms" &&
      maxLen >= 4;
    if (isMovingScenario) {
      const midK = Math.floor(maxLen / 2);
      const midX = xFor(midK - 0.5);
      ctx.fillStyle = "rgba(77, 163, 255, 0.05)";
      ctx.fillRect(padL, padT, Math.max(0, midX - padL), plotH);
      ctx.strokeStyle = "rgba(168, 182, 204, 0.25)";
      ctx.setLineDash([3, 3]);
      ctx.beginPath();
      ctx.moveTo(midX, padT);
      ctx.lineTo(midX, padT + plotH);
      ctx.stroke();
      ctx.setLineDash([]);
      ctx.fillStyle = "#6f7f99";
      ctx.font = "9px ui-monospace, SFMono-Regular, Menlo, monospace";
      ctx.fillText("Active Motion", padL + 4, padT + 10);
      ctx.fillText("Static Recovery", midX + 5, padT + 10);
    }

    // Highlight 4-frame worst regression window if comparing >=2 runs on edge_rmse
    if (metric === "edge_rmse" && state.manifest.localizations) {
      for (const r of state.runs.slice(1)) {
        if (!state.enabledRunIds.has(r.id)) continue;
        const loc = state.manifest.localizations[r.id]?.[state.selectedScenarioKey]?.regression;
        if (loc) {
          const wx0 = xFor(loc.t_start);
          const wx1 = xFor(loc.t_end);
          ctx.fillStyle = "rgba(255, 92, 119, 0.13)";
          ctx.fillRect(wx0, padT, Math.max(2, wx1 - wx0), plotH);
          ctx.fillStyle = "#ff5c77";
          ctx.font = "9px ui-monospace, SFMono-Regular, Menlo, monospace";
          ctx.fillText(`t*=${loc.peak_frame}`, xFor(loc.peak_frame) - 10, padT + plotH - 3);
          break;
        }
      }
    }

    // Y-axis gridlines & labels
    ctx.strokeStyle = "#1c2433";
    ctx.lineWidth = 1;
    ctx.fillStyle = "#6f7f99";
    ctx.font = "10px ui-monospace, SFMono-Regular, Menlo, monospace";
    for (let g = 0; g <= 2; g++) {
      const u = g / 2;
      const y = padT + u * plotH;
      const val = maxVal - u * (maxVal - minVal);
      ctx.beginPath();
      ctx.moveTo(padL, y);
      ctx.lineTo(padL + plotW, y);
      ctx.stroke();
      ctx.fillText(val.toFixed(3), 4, y + 3);
    }

    // Curves
    for (const s of seriesList) {
      ctx.strokeStyle = s.color;
      ctx.lineWidth = 1.75;
      ctx.beginPath();
      let started = false;
      for (let k = 0; k < s.values.length; k++) {
        const v = s.values[k];
        if (typeof v !== "number") continue;
        const x = xFor(k);
        const y = yFor(v);
        if (!started) {
          ctx.moveTo(x, y);
          started = true;
        } else {
          ctx.lineTo(x, y);
        }
      }
      ctx.stroke();

      for (let k = 0; k < s.values.length; k++) {
        const v = s.values[k];
        if (typeof v !== "number") continue;
        ctx.fillStyle = s.color;
        ctx.beginPath();
        ctx.arc(xFor(k), yFor(v), k === state.frameIndex ? 3.5 : 2, 0, Math.PI * 2);
        ctx.fill();
      }
    }

    // Playhead vertical cursor
    if (state.frameIndex < maxLen) {
      const px = xFor(state.frameIndex);
      ctx.strokeStyle = "#e6edf7";
      ctx.lineWidth = 1.25;
      ctx.beginPath();
      ctx.moveTo(px, padT - 4);
      ctx.lineTo(px, padT + plotH + 2);
      ctx.stroke();
    }

    // X-axis frame ticks
    ctx.fillStyle = "#6f7f99";
    const tickStep = maxLen > 32 ? Math.ceil(maxLen / 16) : 1;
    for (let k = 0; k < maxLen; k += tickStep) {
      ctx.fillText(String(k), xFor(k) - 4, ch - 4);
    }
  }

  // Right Inspector: 16x16 Loupe, Per-Engine RGB/Luma Table, and Pinned Pixel Temporal Waveform
  function renderInspector() {
    const loupeCanvas = document.getElementById("loupe-canvas");
    const lCtx = loupeCanvas.getContext("2d");
    lCtx.imageSmoothingEnabled = false;
    lCtx.fillStyle = "#05070a";
    lCtx.fillRect(0, 0, loupeCanvas.width, loupeCanvas.height);

    const coordsEl = document.getElementById("loupe-coords");
    const pinBadge = document.getElementById("loupe-pin-badge");
    const unpinBtn = document.getElementById("btn-unpin");
    const tableHost = document.getElementById("pixel-values-table");

    const target = state.pinnedPixel || state.hoverPixel;
    if (state.pinnedPixel) {
      pinBadge.textContent = `Pinned (${state.pinnedPixel.x}, ${state.pinnedPixel.y})`;
      pinBadge.classList.add("pinned");
      unpinBtn.classList.remove("hidden");
    } else {
      pinBadge.textContent = "Hover viewport (click to pin)";
      pinBadge.classList.remove("pinned");
      unpinBtn.classList.add("hidden");
    }

    if (!target) {
      coordsEl.textContent = "(x: --, y: --)";
      tableHost.replaceChildren(el("div", { className: "mono-subtle", textContent: "Move cursor over image to inspect texels." }));
      renderPixelWaveform(null);
      return;
    }

    coordsEl.textContent = `(${target.x}, ${target.y})`;

    const loupeSource = state.loupeSourceId || state.runs[0]?.id || REF_SOURCE_ID;
    const loupeFrame = getProcessedSourceFrame(loupeSource, state.frameIndex);
    if (loupeFrame) {
      const patch = 16;
      const half = Math.floor(patch / 2);
      const sx = target.x - half;
      const sy = target.y - half;
      const cellPx = loupeCanvas.width / patch;
      lCtx.drawImage(
        loupeFrame.canvas,
        sx,
        sy,
        patch,
        patch,
        0,
        0,
        loupeCanvas.width,
        loupeCanvas.height
      );
      // Subtle texel grid
      lCtx.strokeStyle = "rgba(255, 255, 255, 0.08)";
      lCtx.lineWidth = 1;
      for (let i = 1; i < patch; i++) {
        const p = i * cellPx;
        lCtx.beginPath();
        lCtx.moveTo(p, 0);
        lCtx.lineTo(p, loupeCanvas.height);
        lCtx.moveTo(0, p);
        lCtx.lineTo(loupeCanvas.width, p);
        lCtx.stroke();
      }
      // Highlight center inspected texel
      lCtx.strokeStyle = "#ffd166";
      lCtx.lineWidth = 1.5;
      lCtx.strokeRect(half * cellPx, half * cellPx, cellPx, cellPx);
    }

    // Per-engine RGB / Linear Y / Error readout at (target.x, target.y)
    const refCap = getCaptureEntry(REF_SOURCE_ID, state.frameIndex);
    const pixIdx = (target.y * state.imageWidth + target.x) * 4;
    const rows = [];
    const sourcesToShow = [];
    if (refCap) sourcesToShow.push(REF_SOURCE_ID);
    for (const r of state.runs) sourcesToShow.push(r.id);

    for (const sid of sourcesToShow) {
      const cap = getCaptureEntry(sid, state.frameIndex);
      if (!cap || pixIdx + 3 >= cap.rgba.length) continue;
      const R = cap.rgba[pixIdx];
      const G = cap.rgba[pixIdx + 1];
      const B = cap.rgba[pixIdx + 2];
      const Y = linearLumaAt(cap.rgba, pixIdx);
      const deltaStr =
        sid === REF_SOURCE_ID || !refCap
          ? "--"
          : pixelRmseLinear(cap.rgba, refCap.rgba, pixIdx).toFixed(4);
      const nameCell = el(
        "td",
        null,
        makeSwatchCanvas(`rgb(${R},${G},${B})`, 10),
        el("span", { textContent: getSourceLabel(sid) })
      );
      rows.push(
        el(
          "tr",
          null,
          nameCell,
          el("td", { textContent: `${R},${G},${B}` }),
          el("td", { textContent: Y.toFixed(3) }),
          el("td", { textContent: deltaStr })
        )
      );
    }

    const tbl = el(
      "table",
      { className: "kv-table" },
      el(
        "thead",
        null,
        el(
          "tr",
          null,
          el("th", { textContent: "Source" }),
          el("th", { textContent: "sRGB" }),
          el("th", { textContent: "Lin Y" }),
          el("th", { textContent: "dRMSE" })
        )
      ),
      el("tbody", null, ...rows)
    );
    tableHost.replaceChildren(tbl);

    renderPixelWaveform(target);
  }

  function renderPixelWaveform(target) {
    const canvas = document.getElementById("waveform-canvas");
    const ctx = canvas.getContext("2d");
    const cw = canvas.width;
    const ch = canvas.height;
    ctx.fillStyle = "#0b0e14";
    ctx.fillRect(0, 0, cw, ch);

    if (!target || state.frameCount < 2) {
      ctx.fillStyle = "#6f7f99";
      ctx.font = "10px ui-monospace, SFMono-Regular, Menlo, monospace";
      ctx.fillText(
        state.frameCount < 2 ? "Single-frame scenario" : "Hover or click a pixel to plot Y(t)",
        12,
        ch * 0.5
      );
      return;
    }

    const padL = 34;
    const padR = 8;
    const padT = 8;
    const padB = 14;
    const plotW = cw - padL - padR;
    const plotH = ch - padT - padB;
    const nFrames = state.frameCount;
    const pixOff = (target.y * state.imageWidth + target.x) * 4;

    const traces = [];
    let minY = Infinity;
    let maxY = -Infinity;

    if (state.referenceBuffers.size > 0) {
      const vals = [];
      for (let k = 0; k < nFrames; k++) {
        const ent = getCaptureEntry(REF_SOURCE_ID, k);
        const y = ent && pixOff + 3 < ent.rgba.length ? linearLumaAt(ent.rgba, pixOff) : null;
        if (y !== null) {
          minY = Math.min(minY, y);
          maxY = Math.max(maxY, y);
        }
        vals.push(y);
      }
      traces.push({ color: REF_COLOR, dashed: true, vals });
    }

    for (const r of state.runs) {
      if (!state.enabledRunIds.has(r.id) && state.runs.length > 1) continue;
      const vals = [];
      let any = false;
      for (let k = 0; k < nFrames; k++) {
        const ent = getCaptureEntry(r.id, k);
        const y = ent && pixOff + 3 < ent.rgba.length ? linearLumaAt(ent.rgba, pixOff) : null;
        if (y !== null) {
          any = true;
          minY = Math.min(minY, y);
          maxY = Math.max(maxY, y);
        }
        vals.push(y);
      }
      if (any) traces.push({ color: getRunColor(r.id), dashed: false, vals });
    }

    if (!Number.isFinite(minY)) return;
    if (Math.abs(maxY - minY) < 0.01) {
      minY = Math.max(0, minY - 0.01);
      maxY = Math.min(1, maxY + 0.01);
    } else {
      const pad = (maxY - minY) * 0.12;
      minY = Math.max(0, minY - pad);
      maxY = Math.min(1.05, maxY + pad);
    }

    const xFor = (k) => padL + (k / Math.max(1, nFrames - 1)) * plotW;
    const yFor = (v) => padT + (1 - (v - minY) / Math.max(1e-5, maxY - minY)) * plotH;

    ctx.fillStyle = "#6f7f99";
    ctx.font = "9px ui-monospace, SFMono-Regular, Menlo, monospace";
    ctx.fillText(maxY.toFixed(2), 2, padT + 6);
    ctx.fillText(minY.toFixed(2), 2, padT + plotH);

    for (const tr of traces) {
      ctx.strokeStyle = tr.color;
      ctx.lineWidth = tr.dashed ? 1.25 : 1.6;
      ctx.setLineDash(tr.dashed ? [3, 3] : []);
      ctx.beginPath();
      let started = false;
      for (let k = 0; k < tr.vals.length; k++) {
        const v = tr.vals[k];
        if (v === null) continue;
        const x = xFor(k);
        const y = yFor(v);
        if (!started) {
          ctx.moveTo(x, y);
          started = true;
        } else {
          ctx.lineTo(x, y);
        }
      }
      ctx.stroke();
    }
    ctx.setLineDash([]);

    const px = xFor(state.frameIndex);
    ctx.strokeStyle = "#ffd166";
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(px, padT);
    ctx.lineTo(px, padT + plotH);
    ctx.stroke();
  }

  function renderScenarioMetricsTable() {
    const host = document.getElementById("scenario-metrics-table");
    const metricRows = [
      ["edge_rmse_mean", "Edge RMSE", false],
      ["rmse_mean", "Whole RMSE", false],
      ["gmsd_mean", "GMSD (3x3)", false],
      ["hotspot_top10_rmse", "Hotspot Top10", false],
      ["hotspot_peak_rmse", "Hotspot Peak", false],
      ["edge_ghost_peak", "Ghost Peak", false],
      ["wake_ghosting_rms", "Wake Ghosting", false],
      ["edge_tail_error", "Tail Error", false],
      ["edge_recovery_frames", "Recovery Frms", false],
      ["detail_transfer_mean", "Detail Xfer", true],
      ["spurious_hf_mean", "Spurious HF", false],
      ["residual_flicker_top1pct_rms", "Flicker Top1%", false],
      ["subpixel_ring_flicker_rms", "Star Ring RMS", false],
      ["upscaler_gpu_ms_p50", "Upscaler P50 ms", false],
      ["frame_gpu_ms_p50", "Frame P50 ms", false],
    ];

    const headerCells = [el("th", { textContent: "Metric" })];
    for (const r of state.runs) {
      headerCells.push(el("th", { textContent: r.label.slice(0, 12), title: r.label }));
    }

    const bodyRows = [];
    for (const [mKey, mLabel, higherBetter] of metricRows) {
      const vals = state.runs.map((r) => {
        const sc = getScenarioEntry(r, state.selectedScenarioKey);
        return sc && sc.scalars && typeof sc.scalars[mKey] === "number" ? sc.scalars[mKey] : null;
      });
      if (vals.every((v) => v === null)) continue;

      const baseVal = vals[0];
      const cells = [el("td", { textContent: mLabel, title: mKey })];
      vals.forEach((v, idx) => {
        if (v === null) {
          cells.push(el("td", { textContent: "--" }));
          return;
        }
        let cls = "";
        if (idx > 0 && baseVal !== null && mKey !== "edge_recovery_frames") {
          const diff = v - baseVal;
          if (Math.abs(diff) > 1e-4) {
            const improved = higherBetter ? diff > 0 : diff < 0;
            cls = improved ? "delta-good" : "delta-bad";
          }
        }
        const formatted = mKey === "edge_recovery_frames" ? String(Math.round(v)) : v.toFixed(4);
        cells.push(el("td", { className: cls, textContent: formatted }));
      });
      bodyRows.push(el("tr", null, ...cells));
    }

    if (bodyRows.length === 0) {
      host.replaceChildren(el("div", { className: "mono-subtle", textContent: "No scalar metrics in this scenario." }));
      return;
    }

    host.replaceChildren(
      el(
        "table",
        { className: "kv-table" },
        el("thead", null, el("tr", null, ...headerCells)),
        el("tbody", null, ...bodyRows)
      )
    );
  }

  // Top Run Pills, Warnings & 4-Axis Qualification Scorecard
  function renderRunPillsAndScorecard() {
    const pillsHost = document.getElementById("run-pills");
    const pills = [];

    const refPill = el(
      "button",
      {
        type: "button",
        className: `run-pill ${state.showReferenceInGrid ? "active" : ""}`,
        title: "Toggle Ground Truth Reference tile in Grid mode",
        onClick: () => {
          state.showReferenceInGrid = !state.showReferenceInGrid;
          renderRunPillsAndScorecard();
          rebuildTiles();
          renderAllViewports();
        },
      },
      makeSwatchCanvas(REF_COLOR, 9),
      el("span", { textContent: "Reference" })
    );
    pills.push(refPill);

    state.runs.forEach((r, idx) => {
      const active = state.enabledRunIds.has(r.id);
      const pill = el(
        "button",
        {
          type: "button",
          className: `run-pill ${active ? "active" : ""}`,
          title: `Toggle [${idx + 1}] ${r.label}`,
          onClick: () => {
            if (state.enabledRunIds.has(r.id) && state.enabledRunIds.size > 1) {
              state.enabledRunIds.delete(r.id);
            } else {
              state.enabledRunIds.add(r.id);
            }
            renderRunPillsAndScorecard();
            rebuildTiles();
            renderAll();
          },
        },
        makeSwatchCanvas(getRunColor(r.id), 9),
        el("span", { textContent: `[${idx + 1}] ${r.label}` })
      );
      pills.push(pill);
    });
    pillsHost.replaceChildren(...pills);

    // Warning banner (schedule mismatches + hotspot rank reversals)
    const banner = document.getElementById("warning-banner");
    const warnings = [...(state.manifest.schedule_warnings || [])];
    if (state.manifest.hotspot_warnings) {
      for (const [rid, wList] of Object.entries(state.manifest.hotspot_warnings)) {
        const lbl = getSourceLabel(rid);
        for (const w of wList) warnings.push(`[${lbl}] ${w}`);
      }
    }
    if (warnings.length > 0) {
      banner.textContent = warnings.join("   |   ");
      banner.classList.remove("hidden");
    } else {
      banner.classList.add("hidden");
    }

    // 4-Axis Scorecard Drawer
    const drawer = document.getElementById("scorecard-drawer");
    const hasAnySummary = state.runs.some(
      (r) => Array.isArray(r.benchmark_summaries) && r.benchmark_summaries.length > 0
    );
    if (!hasAnySummary) {
      drawer.replaceChildren(
        el("div", {
          className: "mono-subtle",
          textContent: "Single-model run loaded (pass -b / --perf to populate 4-axis qualification scorecard).",
        })
      );
      return;
    }

    const cols = [
      ["spatial_quality_db", "Spatial (dB)", true, 2],
      ["spatial_gmsd", "GMSD", false, 4],
      ["spatial_detail_transfer_pct", "Detail %", true, 1],
      ["spatial_hotspot_top10_rmse", "Sp.Hotspot", false, 4],
      ["temporal_stability_db", "Temporal (dB)", true, 2],
      ["temporal_flicker_rms", "Flicker 1%", false, 4],
      ["motion_robustness_db", "Motion (dB)", true, 2],
      ["motion_wake_ghosting_rms", "Wake Ghost", false, 4],
      ["motion_hotspot_top10_rmse", "Mo.Hotspot", false, 4],
      ["perf_upscaler_gpu_ms", "Upscaler ms", false, 3],
      ["perf_frame_gpu_ms", "Frame ms", false, 3],
    ];

    const ths = [el("th", { textContent: "Run (Upscale)" })];
    for (const [, label] of cols) ths.push(el("th", { textContent: label }));
    ths.push(el("th", { textContent: "CTS" }));

    const baseSum = state.runs[0]?.benchmark_summaries?.[0] || null;
    const trs = [];
    for (const r of state.runs) {
      for (const s of r.benchmark_summaries || []) {
        const tds = [
          el(
            "td",
            null,
            makeSwatchCanvas(getRunColor(r.id), 9),
            el("span", { textContent: ` ${r.label} (${s.upscaling}x)` })
          ),
        ];
        for (const [key, , higherBetter, digits] of cols) {
          const val = s[key];
          if (typeof val !== "number" || val === 0) {
            tds.push(el("td", { textContent: "--" }));
            continue;
          }
          let cls = "";
          if (baseSum && s !== baseSum && typeof baseSum[key] === "number" && baseSum[key] !== 0) {
            const d = val - baseSum[key];
            if (Math.abs(d) > 1e-4) {
              cls = (higherBetter ? d > 0 : d < 0) ? "delta-good" : "delta-bad";
            }
          }
          tds.push(el("td", { className: cls, textContent: val.toFixed(digits) }));
        }
        tds.push(
          el("td", {
            className: s.cts_valid ? "delta-good" : "delta-neutral",
            textContent: s.cts_valid ? "PASS" : "--",
          })
        );
        trs.push(el("tr", null, ...tds));
      }
    }

    drawer.replaceChildren(
      el(
        "table",
        { className: "scorecard-table" },
        el("thead", null, el("tr", null, ...ths)),
        el("tbody", null, ...trs)
      )
    );
  }

  // Left Scenario List Sidebar
  function renderScenarioSidebar() {
    const listEl = document.getElementById("scenario-list");
    const baseRun = state.runs[0] || null;
    const candRun = state.runs[1] || state.runs[0] || null;

    let items = state.manifest.scenarios.filter((sc) => {
      if (state.categoryFilter === "all") return true;
      if (state.categoryFilter === "Static") return sc.category === "Static" || sc.regime === "static";
      if (state.categoryFilter === "Motion") {
        return (
          sc.category === "Drift" ||
          sc.category === "Tracked Motion" ||
          sc.category === "Unmodeled Motion" ||
          sc.regime === "camera" ||
          sc.regime === "object"
        );
      }
      if (state.categoryFilter === "Perf") return sc.category === "Perf";
      return true;
    });

    function metricForSort(scKey) {
      const bSc = getScenarioEntry(baseRun, scKey);
      const cSc = getScenarioEntry(candRun, scKey);
      if (state.scenarioSort === "delta_edge") {
        const bv = bSc?.scalars?.edge_rmse_mean ?? 0;
        const cv = cSc?.scalars?.edge_rmse_mean ?? bv;
        return cv - bv;
      }
      if (state.scenarioSort === "ghost_peak") {
        return cSc?.scalars?.edge_ghost_peak ?? 0;
      }
      if (state.scenarioSort === "flicker") {
        return cSc?.scalars?.residual_flicker_top1pct_rms ?? 0;
      }
      return 0;
    }

    if (state.scenarioSort !== "default") {
      items = [...items].sort((a, b) => metricForSort(b.key) - metricForSort(a.key));
    }

    const cards = items.map((sc) => {
      const bSc = getScenarioEntry(baseRun, sc.key);
      const cSc = getScenarioEntry(candRun, sc.key);
      let badgeText = "";
      let badgeCls = "delta-neutral";

      if (state.runs.length >= 2 && bSc?.scalars?.edge_rmse_mean !== undefined && cSc?.scalars?.edge_rmse_mean !== undefined) {
        const d = cSc.scalars.edge_rmse_mean - bSc.scalars.edge_rmse_mean;
        badgeText = `${d >= 0 ? "+" : ""}${d.toFixed(4)}`;
        if (Math.abs(d) > 0.0005) badgeCls = d < 0 ? "delta-good" : "delta-bad";
      } else if (bSc?.scalars?.edge_rmse_mean !== undefined) {
        badgeText = `eRMSE ${bSc.scalars.edge_rmse_mean.toFixed(3)}`;
      } else if (bSc?.scalars?.upscaler_gpu_ms_p50 !== undefined) {
        badgeText = `${bSc.scalars.upscaler_gpu_ms_p50.toFixed(2)} ms`;
      }

      return el(
        "button",
        {
          type: "button",
          className: `scenario-card ${sc.key === state.selectedScenarioKey ? "active" : ""}`,
          onClick: () => selectScenario(sc.key),
        },
        el(
          "div",
          { className: "sc-row-top" },
          el("span", { className: "sc-name", textContent: sc.regime, title: sc.key }),
          el("span", { className: "sc-upscale", textContent: `${sc.upscaling}x` })
        ),
        el(
          "div",
          { className: "sc-row-bottom" },
          el("span", { textContent: sc.category }),
          el("span", { className: badgeCls, textContent: badgeText })
        )
      );
    });

    listEl.replaceChildren(...cards);
  }

  function populateSourceSelects() {
    const opts = getAllSourceOptions();
    const ids = ["select-split-a", "select-split-b", "select-flip-source", "select-compare-ref", "select-loupe-source"];
    for (const domId of ids) {
      const sel = document.getElementById(domId);
      sel.replaceChildren(
        ...opts.map((o) => el("option", { value: o.id, textContent: o.label }))
      );
    }
    state.splitSourceA = state.runs[0]?.id || REF_SOURCE_ID;
    state.splitSourceB = state.runs[1]?.id || REF_SOURCE_ID;
    state.flipSource = state.runs[0]?.id || REF_SOURCE_ID;
    state.flipAltSource = state.runs[1]?.id || REF_SOURCE_ID;
    state.compareRefId = REF_SOURCE_ID;
    state.loupeSourceId = state.runs[0]?.id || REF_SOURCE_ID;

    document.getElementById("select-split-a").value = state.splitSourceA;
    document.getElementById("select-split-b").value = state.splitSourceB;
    document.getElementById("select-flip-source").value = state.flipSource;
    document.getElementById("select-compare-ref").value = state.compareRefId;
    document.getElementById("select-loupe-source").value = state.loupeSourceId;
  }

  function syncToolbarVisibility() {
    document.getElementById("split-controls").classList.toggle("hidden", state.layout !== "split");
    document.getElementById("flip-controls").classList.toggle("hidden", state.layout !== "flip");

    const needsHeatmapBar = state.layer === "heatmap" || state.layer === "signed" || state.layer === "flicker";
    document.getElementById("heatmap-controls").classList.toggle("hidden", !needsHeatmapBar);
    document.getElementById("select-error-mode").classList.toggle("hidden", state.layer !== "heatmap");
    document.getElementById("select-flicker-mode").classList.toggle("hidden", state.layer !== "flicker");

    for (const btn of document.querySelectorAll("#layout-buttons .seg-btn")) {
      btn.classList.toggle("active", btn.dataset.layout === state.layout);
    }
    document.getElementById("btn-hotspots").classList.toggle("active", state.showHotspots);

    const tauEl = document.getElementById("readout-threshold");
    if (state.layer === "heatmap" && state.errorMode === "srgb_lsb") {
      tauEl.textContent = `${Math.round(state.threshold * 255)} LSB`;
    } else {
      tauEl.textContent = state.threshold.toFixed(3);
    }
    document.getElementById("readout-gain").textContent = `${state.gain}x`;
  }

  function syncZoomButtons() {
    for (const btn of document.querySelectorAll("#zoom-buttons .seg-btn")) {
      btn.classList.toggle("active", Number(btn.dataset.zoom) === state.zoomMode);
    }
  }

  function syncFrameControls() {
    const slider = document.getElementById("slider-frame");
    slider.max = String(Math.max(0, state.frameCount - 1));
    slider.value = String(state.frameIndex);

    const scMeta = state.manifest?.scenarios?.find((s) => s.key === state.selectedScenarioKey);
    let phase = "";
    if (scMeta && scMeta.category !== "Static" && scMeta.regime !== "static" && state.frameCount >= 4) {
      const mid = Math.floor(state.frameCount / 2);
      phase = state.frameIndex < mid ? " [Motion]" : " [Recovery]";
    }
    const pad2 = (n) => String(n).padStart(2, "0");
    document.getElementById("frame-status").textContent = `Frame ${pad2(state.frameIndex)} / ${pad2(
      Math.max(0, state.frameCount - 1)
    )}${phase}`;
    document.getElementById("btn-play").textContent = state.playing ? "Pause" : "Play";
  }

  function setFrame(index) {
    const clamped = Math.max(0, Math.min(state.frameCount - 1, index));
    if (clamped === state.frameIndex) return;
    state.frameIndex = clamped;
    syncFrameControls();
    renderAllViewports();
    renderTimelineChart();
    renderInspector();
  }

  function stepFrame(delta) {
    if (state.frameCount <= 1) return;
    const next = (state.frameIndex + delta + state.frameCount) % state.frameCount;
    setFrame(next);
  }

  function selectScenario(scenarioKey) {
    state.selectedScenarioKey = scenarioKey;
    renderScenarioSidebar();
    renderScenarioMetricsTable();
    loadScenarioAssets(scenarioKey);
  }

  function renderAllViewports() {
    for (const t of state.tileViews) {
      renderTile(t);
    }
  }

  function renderAll() {
    syncToolbarVisibility();
    syncFrameControls();
    renderAllViewports();
    renderTimelineChart();
    renderInspector();
    renderScenarioMetricsTable();
  }

  // Playback RAF loop
  function tickPlayback(nowMs) {
    if (state.playing && state.frameCount > 1) {
      const interval = 1000 / Math.max(1, state.fps);
      if (nowMs - state.lastTickMs >= interval) {
        state.lastTickMs = nowMs;
        if (state.loopMode === "pingpong") {
          let next = state.frameIndex + state.playDirection;
          if (next >= state.frameCount) {
            state.playDirection = -1;
            next = Math.max(0, state.frameCount - 2);
          } else if (next < 0) {
            state.playDirection = 1;
            next = Math.min(state.frameCount - 1, 1);
          }
          setFrame(next);
        } else {
          setFrame((state.frameIndex + 1) % state.frameCount);
        }
      }
    }
    requestAnimationFrame(tickPlayback);
  }

  function bindUiControls() {
    document.getElementById("btn-scorecard").addEventListener("click", () => {
      const drawer = document.getElementById("scorecard-drawer");
      drawer.classList.toggle("hidden");
      document.getElementById("btn-scorecard").classList.toggle("active", !drawer.classList.contains("hidden"));
      setTimeout(() => renderAll(), 0);
    });

    const modal = document.getElementById("shortcuts-modal");
    document.getElementById("btn-shortcuts").addEventListener("click", () => modal.classList.remove("hidden"));
    document.getElementById("btn-close-shortcuts").addEventListener("click", () => modal.classList.add("hidden"));
    modal.addEventListener("click", (ev) => {
      if (ev.target === modal) modal.classList.add("hidden");
    });

    for (const btn of document.querySelectorAll("#layout-buttons .seg-btn")) {
      btn.addEventListener("click", () => {
        state.layout = btn.dataset.layout;
        syncToolbarVisibility();
        rebuildTiles();
        renderAllViewports();
      });
    }

    document.getElementById("select-split-a").addEventListener("change", (ev) => {
      state.splitSourceA = ev.target.value;
      rebuildTiles();
      renderAllViewports();
    });
    document.getElementById("select-split-b").addEventListener("change", (ev) => {
      state.splitSourceB = ev.target.value;
      rebuildTiles();
      renderAllViewports();
    });
    document.getElementById("btn-split-axis").addEventListener("click", (ev) => {
      state.splitVertical = !state.splitVertical;
      ev.target.textContent = state.splitVertical ? "Vert" : "Horiz";
      renderAllViewports();
    });

    document.getElementById("select-flip-source").addEventListener("change", (ev) => {
      state.flipAltSource = state.flipSource;
      state.flipSource = ev.target.value;
      rebuildTiles();
      renderAllViewports();
    });
    document.getElementById("btn-flip-ab").addEventListener("click", () => swapAbSources());

    document.getElementById("select-layer").addEventListener("change", (ev) => {
      state.layer = ev.target.value;
      clearDerivedCaches();
      renderAll();
    });
    document.getElementById("select-compare-ref").addEventListener("change", (ev) => {
      state.compareRefId = ev.target.value;
      state.hotspotCache.clear();
      clearDerivedCaches();
      renderAll();
    });
    document.getElementById("select-error-mode").addEventListener("change", (ev) => {
      state.errorMode = ev.target.value;
      clearDerivedCaches();
      renderAll();
    });
    document.getElementById("select-flicker-mode").addEventListener("change", (ev) => {
      state.flickerMode = ev.target.value;
      clearDerivedCaches();
      renderAll();
    });

    document.getElementById("slider-threshold").addEventListener("input", (ev) => {
      state.threshold = Number(ev.target.value);
      clearDerivedCaches();
      syncToolbarVisibility();
      renderAllViewports();
      renderInspector();
    });
    document.getElementById("btn-msaa-floor").addEventListener("click", () => {
      let floorVal = 0.02;
      for (const r of state.runs) {
        const sc = getScenarioEntry(r, state.selectedScenarioKey);
        if (sc?.scalars?.msaa_vs_reference_edge_rmse) {
          floorVal = sc.scalars.msaa_vs_reference_edge_rmse;
          break;
        }
      }
      state.threshold = Math.min(0.15, Math.max(0, floorVal));
      document.getElementById("slider-threshold").value = String(state.threshold);
      clearDerivedCaches();
      syncToolbarVisibility();
      renderAllViewports();
      renderInspector();
    });

    document.getElementById("slider-gain").addEventListener("input", (ev) => {
      state.gain = Number(ev.target.value);
      clearDerivedCaches();
      syncToolbarVisibility();
      renderAllViewports();
      renderInspector();
    });

    document.getElementById("select-mask").addEventListener("change", (ev) => {
      state.maskMode = ev.target.value;
      clearDerivedCaches();
      renderAllViewports();
      renderInspector();
    });
    document.getElementById("btn-hotspots").addEventListener("click", () => {
      state.showHotspots = !state.showHotspots;
      syncToolbarVisibility();
      renderAllViewports();
    });

    for (const btn of document.querySelectorAll("#zoom-buttons .seg-btn")) {
      btn.addEventListener("click", () => {
        state.zoomMode = Number(btn.dataset.zoom);
        if (state.zoomMode === 0) {
          state.centerX = 0.5;
          state.centerY = 0.5;
        }
        syncZoomButtons();
        renderAllViewports();
      });
    }
    document.getElementById("btn-reset-view").addEventListener("click", () => {
      state.zoomMode = 0;
      state.centerX = 0.5;
      state.centerY = 0.5;
      state.splitRatio = 0.5;
      syncZoomButtons();
      renderAllViewports();
    });

    for (const pill of document.querySelectorAll("#category-filters .pill-btn")) {
      pill.addEventListener("click", () => {
        state.categoryFilter = pill.dataset.cat;
        for (const p of document.querySelectorAll("#category-filters .pill-btn")) {
          p.classList.toggle("active", p === pill);
        }
        renderScenarioSidebar();
      });
    }
    document.getElementById("select-scenario-sort").addEventListener("change", (ev) => {
      state.scenarioSort = ev.target.value;
      renderScenarioSidebar();
    });

    // Transport controls
    document.getElementById("btn-first").addEventListener("click", () => setFrame(0));
    document.getElementById("btn-prev").addEventListener("click", () => stepFrame(-1));
    document.getElementById("btn-play").addEventListener("click", () => {
      state.playing = !state.playing;
      state.lastTickMs = performance.now();
      syncFrameControls();
    });
    document.getElementById("btn-next").addEventListener("click", () => stepFrame(1));
    document.getElementById("btn-last").addEventListener("click", () => setFrame(state.frameCount - 1));
    document.getElementById("slider-frame").addEventListener("input", (ev) => {
      state.playing = false;
      setFrame(Number(ev.target.value));
    });
    document.getElementById("select-loop-mode").addEventListener("change", (ev) => {
      state.loopMode = ev.target.value;
    });
    document.getElementById("select-fps").addEventListener("change", (ev) => {
      state.fps = Number(ev.target.value);
    });
    document.getElementById("select-chart-metric").addEventListener("change", (ev) => {
      state.chartMetric = ev.target.value;
      renderTimelineChart();
      renderAllViewports();
    });

    // Timeline chart click & drag scrubbing
    const chartCanvas = document.getElementById("timeline-chart");
    let scrubbingChart = false;
    const scrubFromChartEvent = (ev) => {
      const b = chartCanvas.getBoundingClientRect();
      const padL = 52;
      const padR = 14;
      const plotW = Math.max(10, b.width - padL - padR);
      const u = Math.min(1, Math.max(0, (ev.clientX - b.left - padL) / plotW));
      const k = Math.round(u * Math.max(0, state.frameCount - 1));
      state.playing = false;
      setFrame(k);
    };
    chartCanvas.addEventListener("pointerdown", (ev) => {
      scrubbingChart = true;
      chartCanvas.setPointerCapture(ev.pointerId);
      scrubFromChartEvent(ev);
    });
    chartCanvas.addEventListener("pointermove", (ev) => {
      if (scrubbingChart) scrubFromChartEvent(ev);
    });
    chartCanvas.addEventListener("pointerup", () => {
      scrubbingChart = false;
    });

    document.getElementById("select-loupe-source").addEventListener("change", (ev) => {
      state.loupeSourceId = ev.target.value;
      renderInspector();
    });
    document.getElementById("btn-unpin").addEventListener("click", () => {
      state.pinnedPixel = null;
      renderAllViewports();
      renderInspector();
    });

    window.addEventListener("resize", () => renderAll());
    window.addEventListener("keydown", handleKeyDown);
  }

  function swapAbSources() {
    if (state.layout === "split") {
      const tmp = state.splitSourceA;
      state.splitSourceA = state.splitSourceB;
      state.splitSourceB = tmp;
      document.getElementById("select-split-a").value = state.splitSourceA;
      document.getElementById("select-split-b").value = state.splitSourceB;
      rebuildTiles();
      renderAllViewports();
    } else {
      if (state.layout !== "flip") {
        state.layout = "flip";
        syncToolbarVisibility();
      }
      const next = state.flipAltSource || state.runs[1]?.id || REF_SOURCE_ID;
      state.flipAltSource = state.flipSource;
      state.flipSource = next;
      document.getElementById("select-flip-source").value = state.flipSource;
      rebuildTiles();
      renderAllViewports();
    }
  }

  function handleKeyDown(ev) {
    const tag = ev.target?.tagName;
    if (tag === "INPUT" || tag === "SELECT" || tag === "TEXTAREA") {
      if (ev.key !== "Escape") return;
    }

    if (ev.key === "Escape") {
      document.getElementById("shortcuts-modal").classList.add("hidden");
      return;
    }
    if (ev.key === "?") {
      document.getElementById("shortcuts-modal").classList.toggle("hidden");
      return;
    }
    if (ev.key === " ") {
      ev.preventDefault();
      state.playing = !state.playing;
      state.lastTickMs = performance.now();
      syncFrameControls();
      return;
    }
    if (ev.key === "ArrowLeft") {
      ev.preventDefault();
      state.playing = false;
      stepFrame(-1);
      return;
    }
    if (ev.key === "ArrowRight") {
      ev.preventDefault();
      state.playing = false;
      stepFrame(1);
      return;
    }
    if (ev.key === "Home") {
      ev.preventDefault();
      setFrame(0);
      return;
    }
    if (ev.key === "End") {
      ev.preventDefault();
      setFrame(state.frameCount - 1);
      return;
    }
    if (ev.key === "0") {
      state.zoomMode = 0;
      state.centerX = 0.5;
      state.centerY = 0.5;
      syncZoomButtons();
      renderAllViewports();
      return;
    }

    const lower = ev.key.toLowerCase();
    if (lower === "g") {
      state.layout = "grid";
      syncToolbarVisibility();
      rebuildTiles();
      renderAllViewports();
    } else if (lower === "s") {
      state.layout = "split";
      syncToolbarVisibility();
      rebuildTiles();
      renderAllViewports();
    } else if (lower === "f") {
      state.layout = "flip";
      syncToolbarVisibility();
      rebuildTiles();
      renderAllViewports();
    } else if (lower === "b") {
      swapAbSources();
    } else if (lower === "r") {
      state.layout = "flip";
      state.flipAltSource = state.flipSource;
      state.flipSource = REF_SOURCE_ID;
      document.getElementById("select-flip-source").value = REF_SOURCE_ID;
      syncToolbarVisibility();
      rebuildTiles();
      renderAllViewports();
    } else if (lower === "e") {
      state.layer = state.layer === "heatmap" ? "capture" : "heatmap";
      document.getElementById("select-layer").value = state.layer;
      clearDerivedCaches();
      renderAll();
    } else if (lower === "h") {
      state.showHotspots = !state.showHotspots;
      syncToolbarVisibility();
      renderAllViewports();
    } else if (lower === "m") {
      const order = ["off", "outline", "gate"];
      state.maskMode = order[(order.indexOf(state.maskMode) + 1) % order.length];
      document.getElementById("select-mask").value = state.maskMode;
      clearDerivedCaches();
      renderAllViewports();
      renderInspector();
    } else if (ev.key >= "1" && ev.key <= "9") {
      const idx = Number(ev.key) - 1;
      if (idx < state.runs.length) {
        const chosen = state.runs[idx].id;
        if (state.layout === "flip") {
          state.flipAltSource = state.flipSource;
          state.flipSource = chosen;
          document.getElementById("select-flip-source").value = chosen;
          state.loupeSourceId = chosen;
          document.getElementById("select-loupe-source").value = chosen;
          rebuildTiles();
          renderAllViewports();
          renderInspector();
        } else {
          if (state.enabledRunIds.has(chosen) && state.enabledRunIds.size > 1) {
            state.enabledRunIds.delete(chosen);
          } else {
            state.enabledRunIds.add(chosen);
          }
          renderRunPillsAndScorecard();
          rebuildTiles();
          renderAll();
        }
      }
    }
  }

  async function init() {
    bindUiControls();
    try {
      const resp = await fetch("/api/manifest", { cache: "no-store" });
      if (!resp.ok) {
        throw new Error(`HTTP ${resp.status}`);
      }
      const manifest = await resp.json();
      state.manifest = manifest;
      state.runs = manifest.runs || [];
      for (const r of state.runs) {
        state.runsById.set(r.id, r);
        state.enabledRunIds.add(r.id);
      }
      // If >=3 runs loaded, default to hiding Reference tile in Grid so engines fit comfortably
      if (state.runs.length >= 3) {
        state.showReferenceInGrid = false;
      }

      populateSourceSelects();
      renderRunPillsAndScorecard();

      // Auto-expand scorecard if benchmark_summaries exist across >=2 runs
      if (
        state.runs.length >= 2 &&
        state.runs.some((r) => Array.isArray(r.benchmark_summaries) && r.benchmark_summaries.length > 0)
      ) {
        document.getElementById("scorecard-drawer").classList.remove("hidden");
        document.getElementById("btn-scorecard").classList.add("active");
      }

      const firstScenario = manifest.scenarios?.[0]?.key || null;
      if (firstScenario) {
        selectScenario(firstScenario);
      } else {
        document.getElementById("tiles-container").replaceChildren(
          el("div", { className: "empty-state", textContent: "No scenarios found in loaded runs." })
        );
      }
    } catch (_err) {
      document.getElementById("tiles-container").replaceChildren(
        el("div", { className: "empty-state", textContent: "Failed to load /api/manifest from taa_viewer.py." })
      );
    }

    requestAnimationFrame(tickPlayback);
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", init);
  } else {
    init();
  }
})();
