import { Cacheable, dataCache } from '../../../../common/framework/data_cache.js';
import { unreachable } from '../../../../common/util/util.js';
import BinaryStream from '../../../util/binary_stream.js';
import { deserializeComparator, serializeComparator } from '../../../util/compare.js';
import {
  MatrixValue,
  Value,
  VectorValue,
  deserializeValue,
  isScalarValue,
  serializeValue,
} from '../../../util/conversion.js';
import {
  FPInterval,
  deserializeFPInterval,
  serializeFPInterval,
} from '../../../util/floating_point.js';
import { flatten2DArray, unflatten2DArray } from '../../../util/math.js';

import { Case } from './case.js';
import { Expectation, isComparator } from './expectation.js';

enum SerializedExpectationKind {
  Value,
  Interval,
  Interval1DArray,
  Interval2DArray,
  Array,
  Comparator,
}

/** serializeExpectation() serializes an Expectation to a BinaryStream */
export function serializeExpectation(s: BinaryStream, e: Expectation) {
  if (isScalarValue(e) || e instanceof VectorValue || e instanceof MatrixValue) {
    s.writeU8(SerializedExpectationKind.Value);
    serializeValue(s, e);
    return;
  }
  if (e instanceof FPInterval) {
    s.writeU8(SerializedExpectationKind.Interval);
    serializeFPInterval(s, e);
    return;
  }
  if (e instanceof Array) {
    if (e[0] instanceof Array) {
      e = e as FPInterval[][];
      const cols = e.length;
      const rows = e[0].length;
      s.writeU8(SerializedExpectationKind.Interval2DArray);
      s.writeU16(cols);
      s.writeU16(rows);
      s.writeArray(flatten2DArray(e), serializeFPInterval);
    } else {
      e = e as FPInterval[];
      s.writeU8(SerializedExpectationKind.Interval1DArray);
      s.writeArray(e, serializeFPInterval);
    }
    return;
  }
  if (isComparator(e)) {
    s.writeU8(SerializedExpectationKind.Comparator);
    serializeComparator(s, e);
    return;
  }
  unreachable(`cannot serialize Expectation ${e}`);
}

/** deserializeExpectation() deserializes an Expectation from a BinaryStream */
export function deserializeExpectation(s: BinaryStream): Expectation {
  const kind = s.readU8();
  switch (kind) {
    case SerializedExpectationKind.Value: {
      return deserializeValue(s);
    }
    case SerializedExpectationKind.Interval: {
      return deserializeFPInterval(s);
    }
    case SerializedExpectationKind.Interval1DArray: {
      return s.readArray(deserializeFPInterval);
    }
    case SerializedExpectationKind.Interval2DArray: {
      const cols = s.readU16();
      const rows = s.readU16();
      return unflatten2DArray(s.readArray(deserializeFPInterval), cols, rows);
    }
    case SerializedExpectationKind.Comparator: {
      return deserializeComparator(s);
    }
    default: {
      unreachable(`invalid serialized expectation kind: ${kind}`);
    }
  }
}

/** serializeCase() serializes a Case to a BinaryStream */
export function serializeCase(s: BinaryStream, c: Case) {
  s.writeCond(c.input instanceof Array, {
    if_true: () => {
      // c.input is array
      s.writeArray(c.input as Value[], serializeValue);
    },
    if_false: () => {
      // c.input is not array
      serializeValue(s, c.input as Value);
    },
  });
  serializeExpectation(s, c.expected);
}

/** deserializeCase() deserializes a Case from a BinaryStream */
export function deserializeCase(s: BinaryStream): Case {
  const input = s.readCond({
    if_true: () => {
      // c.input is array
      return s.readArray(deserializeValue);
    },
    if_false: () => {
      // c.input is not array
      return deserializeValue(s);
    },
  });
  const expected = deserializeExpectation(s);
  return { input, expected };
}

/** CaseListBuilder is a function that builds a list of cases, Case[] */
export type CaseListBuilder = () => Case[];

/**
 * CaseCache is a cache of Case[].
 * CaseCache implements the Cacheable interface, so the cases can be pre-built
 * and stored in the data cache, reducing computation costs at CTS runtime.
 */
export class CaseCache implements Cacheable<Record<string, Case[]>> {
  /**
   * Constructor
   * @param name the name of the cache. This must be globally unique.
   * @param builders a Record of case-list name to case-list builder.
   */
  constructor(name: string, builders: Record<string, CaseListBuilder>) {
    this.path = `webgpu/shader/execution/${name}.bin`;
    this.builders = builders;
  }

  /** get() returns the list of cases with the given name */
  public async get(name: string): Promise<Case[]> {
    const data = await dataCache.fetch(this);
    return data[name];
  }

  /**
   * build() implements the Cacheable.build interface.
   * @returns the data.
   */
  build(): Promise<Record<string, Case[]>> {
    const built: Record<string, Case[]> = {};
    for (const name in this.builders) {
      const cases = this.builders[name]();
      built[name] = cases;
    }
    return Promise.resolve(built);
  }

  /**
   * serialize() implements the Cacheable.serialize interface.
   * @returns the serialized data.
   */
  serialize(data: Record<string, Case[]>): Uint8Array {
    const maxSize = 32 << 20; // 32MB - max size for a file
    const stream = new BinaryStream(new ArrayBuffer(maxSize));
    stream.writeU32(Object.keys(data).length);
    for (const name in data) {
      stream.writeString(name);
      stream.writeArray(data[name], serializeCase);
    }
    return stream.buffer();
  }

  /**
   * deserialize() implements the Cacheable.deserialize interface.
   * @returns the deserialize data.
   */
  deserialize(array: Uint8Array): Record<string, Case[]> {
    const s = new BinaryStream(array.buffer);
    const casesByName: Record<string, Case[]> = {};
    const numRecords = s.readU32();
    for (let i = 0; i < numRecords; i++) {
      const name = s.readString();
      const cases = s.readArray(deserializeCase);
      casesByName[name] = cases;
    }
    return casesByName;
  }

  public readonly path: string;
  private readonly builders: Record<string, CaseListBuilder>;
}

export function makeCaseCache(name: string, builders: Record<string, CaseListBuilder>): CaseCache {
  return new CaseCache(name, builders);
}
