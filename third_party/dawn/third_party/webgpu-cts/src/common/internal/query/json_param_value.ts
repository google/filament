import { assert, sortObjectByKey, isPlainObject } from '../../util/util.js';
import { JSONWithUndefined } from '../params_utils.js';

// JSON can't represent various values and by default stores them as `null`.
// Instead, storing them as a magic string values in JSON.
const jsUndefinedMagicValue = '_undef_';
const jsNaNMagicValue = '_nan_';
const jsPositiveInfinityMagicValue = '_posinfinity_';
const jsNegativeInfinityMagicValue = '_neginfinity_';

// -0 needs to be handled separately, because -0 === +0 returns true. Not
// special casing +0/0, since it behaves intuitively. Assuming that if -0 is
// being used, the differentiation from +0 is desired.
const jsNegativeZeroMagicValue = '_negzero_';

// bigint values are not defined in JSON, so need to wrap them up as strings
const jsBigIntMagicPattern = /^(\d+)n$/;

const toStringMagicValue = new Map<unknown, string>([
  [undefined, jsUndefinedMagicValue],
  [NaN, jsNaNMagicValue],
  [Number.POSITIVE_INFINITY, jsPositiveInfinityMagicValue],
  [Number.NEGATIVE_INFINITY, jsNegativeInfinityMagicValue],
  // No -0 handling because it is special cased.
]);

const fromStringMagicValue = new Map<string, unknown>([
  [jsUndefinedMagicValue, undefined],
  [jsNaNMagicValue, NaN],
  [jsPositiveInfinityMagicValue, Number.POSITIVE_INFINITY],
  [jsNegativeInfinityMagicValue, Number.NEGATIVE_INFINITY],
  // -0 is handled in this direction because there is no comparison issue.
  [jsNegativeZeroMagicValue, -0],
]);

function stringifyFilter(_k: string, v: unknown): unknown {
  // Make sure no one actually uses a magic value as a parameter.
  if (typeof v === 'string') {
    assert(
      !fromStringMagicValue.has(v),
      `${v} is a magic value for stringification, so cannot be used`
    );

    assert(
      v !== jsNegativeZeroMagicValue,
      `${v} is a magic value for stringification, so cannot be used`
    );

    assert(
      v.match(jsBigIntMagicPattern) === null,
      `${v} matches bigint magic pattern for stringification, so cannot be used`
    );
  }

  const isObject = v !== null && typeof v === 'object' && !Array.isArray(v);
  if (isObject) {
    assert(
      isPlainObject(v),
      `value must be a plain object but it appears to be a '${
        Object.getPrototypeOf(v).constructor.name
      }`
    );
  }
  assert(typeof v !== 'function', `${v} can not be a function`);

  if (Object.is(v, -0)) {
    return jsNegativeZeroMagicValue;
  }

  if (typeof v === 'bigint') {
    return `${v}n`;
  }

  return toStringMagicValue.has(v) ? toStringMagicValue.get(v) : v;
}

export function stringifyParamValue(value: JSONWithUndefined): string {
  return JSON.stringify(value, stringifyFilter);
}

/**
 * Like stringifyParamValue but sorts dictionaries by key, for hashing.
 */
export function stringifyParamValueUniquely(value: JSONWithUndefined): string {
  return JSON.stringify(value, (k, v) => {
    if (typeof v === 'object' && v !== null) {
      return sortObjectByKey(v);
    }

    return stringifyFilter(k, v);
  });
}

// 'any' is part of the JSON.parse reviver interface, so cannot be avoided.
// eslint-disable-next-line  @typescript-eslint/no-explicit-any
function parseParamValueReviver(_k: string, v: any): any {
  if (fromStringMagicValue.has(v)) {
    return fromStringMagicValue.get(v);
  }

  if (typeof v === 'string') {
    const match: RegExpMatchArray | null = v.match(jsBigIntMagicPattern);
    if (match !== null) {
      // [0] is the entire match, and following entries are the capture groups
      return BigInt(match[1]);
    }
  }

  return v;
}

export function parseParamValue(s: string): JSONWithUndefined {
  return JSON.parse(s, parseParamValueReviver);
}
