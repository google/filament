/** A range of indices expressed as `{ begin, count }`. */
export interface BeginCountRange {
  begin: number;
  count: number;
}

/* A range of indices, expressed as `{ begin, end }`. */
export interface BeginEndRange {
  begin: number;
  end: number;
}

function endOfRange(r: BeginEndRange | BeginCountRange): number {
  return 'count' in r ? r.begin + r.count : r.end;
}

function* rangeAsIterator(r: BeginEndRange | BeginCountRange): Generator<number> {
  for (let i = r.begin; i < endOfRange(r); ++i) {
    yield i;
  }
}

/**
 * Represents a range of subresources of a single-plane texture:
 * a min/max mip level and min/max array layer.
 */
export class SubresourceRange {
  readonly mipRange: BeginEndRange;
  readonly layerRange: BeginEndRange;

  constructor(subresources: {
    mipRange: BeginEndRange | BeginCountRange;
    layerRange: BeginEndRange | BeginCountRange;
  }) {
    this.mipRange = {
      begin: subresources.mipRange.begin,
      end: endOfRange(subresources.mipRange),
    };
    this.layerRange = {
      begin: subresources.layerRange.begin,
      end: endOfRange(subresources.layerRange),
    };
  }

  /**
   * Iterates over the "rectangle" of `{ level, layer }` pairs represented by the range.
   */
  *each(): Generator<{ level: number; layer: number }> {
    for (let level = this.mipRange.begin; level < this.mipRange.end; ++level) {
      for (let layer = this.layerRange.begin; layer < this.layerRange.end; ++layer) {
        yield { level, layer };
      }
    }
  }

  /**
   * Iterates over the mip levels represented by the range, each level including an iterator
   * over the array layers at that level.
   */
  *mipLevels(): Generator<{ level: number; layers: Generator<number> }> {
    for (let level = this.mipRange.begin; level < this.mipRange.end; ++level) {
      yield {
        level,
        layers: rangeAsIterator(this.layerRange),
      };
    }
  }
}
