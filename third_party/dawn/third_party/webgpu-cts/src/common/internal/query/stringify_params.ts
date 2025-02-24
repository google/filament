import { TestParams } from '../../framework/fixture.js';
import { assert } from '../../util/util.js';
import { JSONWithUndefined, badParamValueChars, paramKeyIsPublic } from '../params_utils.js';

import { stringifyParamValue, stringifyParamValueUniquely } from './json_param_value.js';
import { kParamKVSeparator, kParamSeparator, kWildcard } from './separators.js';

export function stringifyPublicParams(p: TestParams, addWildcard = false): string {
  const parts = Object.keys(p)
    .filter(k => paramKeyIsPublic(k))
    .map(k => stringifySingleParam(k, p[k]));

  if (addWildcard) parts.push(kWildcard);

  return parts.join(kParamSeparator);
}

/**
 * An _approximately_ unique string representing a CaseParams value.
 */
export function stringifyPublicParamsUniquely(p: TestParams): string {
  const keys = Object.keys(p).sort();
  return keys
    .filter(k => paramKeyIsPublic(k))
    .map(k => stringifySingleParamUniquely(k, p[k]))
    .join(kParamSeparator);
}

export function stringifySingleParam(k: string, v: JSONWithUndefined) {
  return `${k}${kParamKVSeparator}${stringifySingleParamValue(v)}`;
}

function stringifySingleParamUniquely(k: string, v: JSONWithUndefined) {
  return `${k}${kParamKVSeparator}${stringifyParamValueUniquely(v)}`;
}

function stringifySingleParamValue(v: JSONWithUndefined): string {
  const s = stringifyParamValue(v);
  assert(
    !badParamValueChars.test(s),
    `JSON.stringified param value must not match ${badParamValueChars} - was ${s}`
  );
  return s;
}
