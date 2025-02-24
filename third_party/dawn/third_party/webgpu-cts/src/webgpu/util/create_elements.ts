import { Fixture } from '../../common/framework/fixture.js';
import { unreachable } from '../../common/util/util.js';

// TESTING_TODO: This should expand to more canvas types (which will enhance a bunch of tests):
// - canvas element not in dom
// - canvas element in dom
// - offscreen canvas from transferControlToOffscreen from canvas not in dom
// - offscreen canvas from transferControlToOffscreen from canvas in dom
// - offscreen canvas from new OffscreenCanvas
export const kAllCanvasTypes = ['onscreen', 'offscreen'] as const;
export type CanvasType = (typeof kAllCanvasTypes)[number];

type CanvasForCanvasType<T extends CanvasType> = {
  onscreen: HTMLCanvasElement;
  offscreen: OffscreenCanvas;
}[T];

/** Valid contextId for HTMLCanvasElement/OffscreenCanvas,
 *  spec: https://html.spec.whatwg.org/multipage/canvas.html#dom-canvas-getcontext
 */
export const kValidCanvasContextIds = [
  '2d',
  'bitmaprenderer',
  'webgl',
  'webgl2',
  'webgpu',
] as const;
export type CanvasContext = (typeof kValidCanvasContextIds)[number];

/** Create HTMLCanvas/OffscreenCanvas. */
export function createCanvas<T extends CanvasType>(
  test: Fixture,
  canvasType: T,
  width: number,
  height: number
): CanvasForCanvasType<T> {
  if (canvasType === 'onscreen') {
    if (typeof document !== 'undefined') {
      return createOnscreenCanvas(test, width, height) as CanvasForCanvasType<T>;
    } else {
      test.skip('Cannot create HTMLCanvasElement');
    }
  } else if (canvasType === 'offscreen') {
    if (typeof OffscreenCanvas !== 'undefined') {
      return createOffscreenCanvas(test, width, height) as CanvasForCanvasType<T>;
    } else {
      test.skip('Cannot create an OffscreenCanvas');
    }
  } else {
    unreachable();
  }
}

/** Create HTMLCanvasElement. */
export function createOnscreenCanvas(
  test: Fixture,
  width: number,
  height: number
): HTMLCanvasElement {
  let canvas: HTMLCanvasElement;
  if (typeof document !== 'undefined') {
    canvas = document.createElement('canvas');
    canvas.width = width;
    canvas.height = height;
  } else {
    test.skip('Cannot create HTMLCanvasElement');
  }
  return canvas;
}

/** Create OffscreenCanvas. */
export function createOffscreenCanvas(
  test: Fixture,
  width: number,
  height: number
): OffscreenCanvas {
  if (typeof OffscreenCanvas === 'undefined') {
    test.skip('OffscreenCanvas is not supported');
  }

  return new OffscreenCanvas(width, height);
}
