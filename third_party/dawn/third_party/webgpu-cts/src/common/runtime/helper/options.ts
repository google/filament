import { unreachable } from '../../util/util.js';

let windowURL: URL | undefined = undefined;
function getWindowURL() {
  if (windowURL === undefined) {
    windowURL = new URL(window.location.toString());
  }
  return windowURL;
}

/** Parse a runner option that is always boolean-typed. False if missing or '0'. */
export function optionEnabled(
  opt: string,
  searchParams: URLSearchParams = getWindowURL().searchParams
): boolean {
  const val = searchParams.get(opt);
  return val !== null && val !== '0';
}

/** Parse a runner option that is string-typed. If the option is missing, returns `null`. */
export function optionString(
  opt: string,
  searchParams: URLSearchParams = getWindowURL().searchParams
): string | null {
  return searchParams.get(opt);
}

/** Runtime modes for running tests in different types of workers. */
export type WorkerMode = 'dedicated' | 'service' | 'shared';
/** Parse a runner option for different worker modes (as in `?worker=shared`). Null if no worker. */
export function optionWorkerMode(
  opt: string,
  searchParams: URLSearchParams = getWindowURL().searchParams
): WorkerMode | null {
  const value = searchParams.get(opt);
  if (value === null || value === '0') {
    return null;
  } else if (value === 'service') {
    return 'service';
  } else if (value === 'shared') {
    return 'shared';
  } else if (value === '' || value === '1' || value === 'dedicated') {
    return 'dedicated';
  }
  unreachable('invalid worker= option value');
}

/**
 * The possible options for the tests.
 */
export interface CTSOptions {
  worker: WorkerMode | null;
  debug: boolean;
  compatibility: boolean;
  forceFallbackAdapter: boolean;
  enforceDefaultLimits: boolean;
  unrollConstEvalLoops: boolean;
  powerPreference: GPUPowerPreference | null;
  logToWebSocket: boolean;
}

export const kDefaultCTSOptions: CTSOptions = {
  worker: null,
  debug: true,
  compatibility: false,
  forceFallbackAdapter: false,
  enforceDefaultLimits: false,
  unrollConstEvalLoops: false,
  powerPreference: null,
  logToWebSocket: false,
};

/**
 * Extra per option info.
 */
export interface OptionInfo {
  description: string;
  parser?: (key: string, searchParams?: URLSearchParams) => boolean | string | null;
  selectValueDescriptions?: { value: string | null; description: string }[];
}

/**
 * Type for info for every option. This definition means adding an option
 * will generate a compile time error if no extra info is provided.
 */
export type OptionsInfos<Type> = Record<keyof Type, OptionInfo>;

/**
 * Options to the CTS.
 */
export const kCTSOptionsInfo: OptionsInfos<CTSOptions> = {
  worker: {
    description: 'run in a worker',
    parser: optionWorkerMode,
    selectValueDescriptions: [
      { value: null, description: 'no worker' },
      { value: 'dedicated', description: 'dedicated worker' },
      { value: 'shared', description: 'shared worker' },
      { value: 'service', description: 'service worker' },
    ],
  },
  debug: { description: 'show more info' },
  compatibility: { description: 'request adapters with featureLevel: "compatibility"' },
  forceFallbackAdapter: { description: 'pass forceFallbackAdapter: true to requestAdapter' },
  enforceDefaultLimits: {
    description: `force the adapter limits to the default limits.
Note: May fail on tests for low-power/high-performance`,
  },
  unrollConstEvalLoops: { description: 'unroll const eval loops in WGSL' },
  powerPreference: {
    description: 'set default powerPreference for some tests',
    parser: optionString,
    selectValueDescriptions: [
      { value: null, description: 'default' },
      { value: 'low-power', description: 'low-power' },
      { value: 'high-performance', description: 'high-performance' },
    ],
  },
  logToWebSocket: { description: 'send some logs to ws://localhost:59497/' },
};

/**
 * Converts camel case to snake case.
 * Examples:
 *    fooBar -> foo_bar
 *    parseHTMLFile -> parse_html_file
 */
export function camelCaseToSnakeCase(id: string) {
  return id
    .replace(/(.)([A-Z][a-z]+)/g, '$1_$2')
    .replace(/([a-z0-9])([A-Z])/g, '$1_$2')
    .toLowerCase();
}

/**
 * Creates a Options from search parameters.
 */
function getOptionsInfoFromSearchString<Type extends CTSOptions>(
  optionsInfos: OptionsInfos<Type>,
  searchString: string
): Type {
  const searchParams = new URLSearchParams(searchString);
  const optionValues: Record<string, boolean | string | null> = {};
  for (const [optionName, info] of Object.entries(optionsInfos)) {
    const parser = info.parser || optionEnabled;
    optionValues[optionName] = parser(camelCaseToSnakeCase(optionName), searchParams);
  }
  return optionValues as unknown as Type;
}

/**
 * Given a test query string in the form of `suite:foo,bar,moo&opt1=val1&opt2=val2
 * returns the query and the options.
 */
export function parseSearchParamLikeWithOptions<Type extends CTSOptions>(
  optionsInfos: OptionsInfos<Type>,
  query: string
): {
  queries: string[];
  options: Type;
} {
  const searchString = query.includes('q=') || query.startsWith('?') ? query : `q=${query}`;
  const queries = new URLSearchParams(searchString).getAll('q');
  const options = getOptionsInfoFromSearchString(optionsInfos, searchString);
  return { queries, options };
}

/**
 * Given a test query string in the form of `suite:foo,bar,moo&opt1=val1&opt2=val2
 * returns the query and the common options.
 */
export function parseSearchParamLikeWithCTSOptions(query: string) {
  return parseSearchParamLikeWithOptions(kCTSOptionsInfo, query);
}
