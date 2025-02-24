// Implements the standalone test runner (see also: /standalone/index.html).

/* eslint no-console: "off" */

import { dataCache } from '../framework/data_cache.js';
import { getResourcePath, setBaseResourcePath } from '../framework/resources.js';
import { globalTestConfig } from '../framework/test_config.js';
import { DefaultTestFileLoader } from '../internal/file_loader.js';
import { Logger } from '../internal/logging/logger.js';
import { LiveTestCaseResult } from '../internal/logging/result.js';
import { parseQuery } from '../internal/query/parseQuery.js';
import { TestQueryLevel } from '../internal/query/query.js';
import { TestTreeNode, TestSubtree, TestTreeLeaf, TestTree } from '../internal/tree.js';
import { setDefaultRequestAdapterOptions } from '../util/navigator_gpu.js';
import { ErrorWithExtra, unreachable } from '../util/util.js';

import {
  kCTSOptionsInfo,
  parseSearchParamLikeWithOptions,
  CTSOptions,
  OptionInfo,
  OptionsInfos,
  camelCaseToSnakeCase,
} from './helper/options.js';
import { TestDedicatedWorker, TestSharedWorker, TestServiceWorker } from './helper/test_worker.js';

const rootQuerySpec = 'webgpu:*';
let promptBeforeReload = false;
let isFullCTS = false;

globalTestConfig.frameworkDebugLog = console.log;

window.onbeforeunload = () => {
  // Prompt user before reloading if there are any results
  return promptBeforeReload ? false : undefined;
};

const kOpenTestLinkAltText = 'Open';

type StandaloneOptions = CTSOptions & { runnow: OptionInfo };

const kStandaloneOptionsInfos: OptionsInfos<StandaloneOptions> = {
  ...kCTSOptionsInfo,
  runnow: { description: 'run immediately on load' },
};

const { queries: qs, options } = parseSearchParamLikeWithOptions(
  kStandaloneOptionsInfos,
  window.location.search || rootQuerySpec
);
const { runnow, powerPreference, compatibility, forceFallbackAdapter } = options;
globalTestConfig.enableDebugLogs = options.debug;
globalTestConfig.unrollConstEvalLoops = options.unrollConstEvalLoops;
globalTestConfig.compatibility = compatibility;
globalTestConfig.enforceDefaultLimits = options.enforceDefaultLimits;
globalTestConfig.logToWebSocket = options.logToWebSocket;

const logger = new Logger();

setBaseResourcePath('../out/resources');

const testWorker =
  options.worker === null
    ? null
    : options.worker === 'dedicated'
    ? new TestDedicatedWorker(options)
    : options.worker === 'shared'
    ? new TestSharedWorker(options)
    : options.worker === 'service'
    ? new TestServiceWorker(options)
    : unreachable();

const autoCloseOnPass = document.getElementById('autoCloseOnPass') as HTMLInputElement;
const resultsVis = document.getElementById('resultsVis')!;
const progressElem = document.getElementById('progress')!;
const progressTestNameElem = progressElem.querySelector('.progress-test-name')!;
const stopButtonElem = progressElem.querySelector('button')!;
let runDepth = 0;
let stopRequested = false;

stopButtonElem.addEventListener('click', () => {
  stopRequested = true;
});

if (powerPreference || compatibility || forceFallbackAdapter) {
  setDefaultRequestAdapterOptions({
    ...(powerPreference && { powerPreference }),
    // MAINTENANCE_TODO: remove compatibilityMode once no longer needed.
    ...(compatibility && { compatibilityMode: true, featureLevel: 'compatibility' }),
    ...(forceFallbackAdapter && { forceFallbackAdapter: true }),
  });
}

dataCache.setStore({
  load: async (path: string) => {
    const response = await fetch(getResourcePath(`cache/${path}`));
    if (!response.ok) {
      return Promise.reject(response.statusText);
    }
    return new Uint8Array(await response.arrayBuffer());
  },
});

interface SubtreeResult {
  pass: number;
  fail: number;
  warn: number;
  skip: number;
  total: number;
  timems: number;
}

function emptySubtreeResult() {
  return { pass: 0, fail: 0, warn: 0, skip: 0, total: 0, timems: 0 };
}

function mergeSubtreeResults(...results: SubtreeResult[]) {
  const target = emptySubtreeResult();
  for (const result of results) {
    target.pass += result.pass;
    target.fail += result.fail;
    target.warn += result.warn;
    target.skip += result.skip;
    target.total += result.total;
    target.timems += result.timems;
  }
  return target;
}

type SetCheckedRecursively = () => void;
type GenerateSubtreeHTML = (parent: HTMLElement) => SetCheckedRecursively;
type RunSubtree = () => Promise<SubtreeResult>;

interface VisualizedSubtree {
  generateSubtreeHTML: GenerateSubtreeHTML;
  runSubtree: RunSubtree;
}

// DOM generation

function memoize<T>(fn: () => T): () => T {
  let value: T | undefined;
  return () => {
    if (value === undefined) {
      value = fn();
    }
    return value;
  };
}

function makeTreeNodeHTML(tree: TestTreeNode, parentLevel: TestQueryLevel): VisualizedSubtree {
  let subtree: VisualizedSubtree;

  if ('children' in tree) {
    subtree = makeSubtreeHTML(tree, parentLevel);
  } else {
    subtree = makeCaseHTML(tree);
  }

  const generateMyHTML = (parentElement: HTMLElement) => {
    const div = $('<div>').appendTo(parentElement)[0];
    return subtree.generateSubtreeHTML(div);
  };
  return { runSubtree: subtree.runSubtree, generateSubtreeHTML: generateMyHTML };
}

function makeCaseHTML(t: TestTreeLeaf): VisualizedSubtree {
  // Becomes set once the case has been run once.
  let caseResult: LiveTestCaseResult | undefined;

  // Becomes set once the DOM for this case exists.
  let clearRenderedResult: (() => void) | undefined;
  let updateRenderedResult: (() => void) | undefined;

  const name = t.query.toString();
  const runSubtree = async () => {
    if (clearRenderedResult) clearRenderedResult();

    const result: SubtreeResult = emptySubtreeResult();
    progressTestNameElem.textContent = name;

    const [rec, res] = logger.record(name);
    caseResult = res;
    if (testWorker) {
      await testWorker.run(rec, name);
    } else {
      await t.run(rec);
    }

    result.total++;
    result.timems += caseResult.timems;
    switch (caseResult.status) {
      case 'pass':
        result.pass++;
        break;
      case 'fail':
        result.fail++;
        break;
      case 'skip':
        result.skip++;
        break;
      case 'warn':
        result.warn++;
        break;
      default:
        unreachable();
    }

    if (updateRenderedResult) updateRenderedResult();

    return result;
  };

  const generateSubtreeHTML = (div: HTMLElement) => {
    div.classList.add('testcase');

    const caselogs = $('<div>').addClass('testcaselogs').hide();
    const [casehead, setChecked] = makeTreeNodeHeaderHTML(t, runSubtree, 2, checked => {
      checked ? caselogs.show() : caselogs.hide();
    });
    const casetime = $('<div>').addClass('testcasetime').html('ms').appendTo(casehead);
    div.appendChild(casehead);
    div.appendChild(caselogs[0]);

    clearRenderedResult = () => {
      div.removeAttribute('data-status');
      casetime.text('ms');
      caselogs.empty();
    };

    updateRenderedResult = () => {
      if (caseResult) {
        div.setAttribute('data-status', caseResult.status);

        casetime.text(caseResult.timems.toFixed(4) + ' ms');

        if (caseResult.logs) {
          caselogs.empty();
          // Show exceptions at the top since they are often unexpected can point out an error in the test itself vs the WebGPU implementation.
          caseResult.logs
            .filter(l => l.name === 'EXCEPTION')
            .forEach(l => {
              $('<pre>').addClass('testcaselogtext').text(l.toJSON()).appendTo(caselogs);
            });
          for (const l of caseResult.logs) {
            const caselog = $('<div>').addClass('testcaselog').appendTo(caselogs);
            $('<button>')
              .addClass('testcaselogbtn')
              .attr('alt', 'Log stack to console')
              .attr('title', 'Log stack to console')
              .appendTo(caselog)
              .on('click', () => {
                consoleLogError(l);
              });
            $('<pre>').addClass('testcaselogtext').appendTo(caselog).text(l.toJSON());
          }
        }
      }
    };

    updateRenderedResult();

    return setChecked;
  };

  return { runSubtree, generateSubtreeHTML };
}

function makeSubtreeHTML(n: TestSubtree, parentLevel: TestQueryLevel): VisualizedSubtree {
  let subtreeResult: SubtreeResult = emptySubtreeResult();
  // Becomes set once the DOM for this case exists.
  let clearRenderedResult: (() => void) | undefined;
  let updateRenderedResult: (() => void) | undefined;

  const { runSubtree, generateSubtreeHTML } = makeSubtreeChildrenHTML(
    n.children.values(),
    n.query.level
  );

  const runMySubtree = async () => {
    if (runDepth === 0) {
      stopRequested = false;
      progressElem.style.display = '';
      // only prompt if this is the full CTS and we started from the root.
      if (isFullCTS && n.query.filePathParts.length === 0) {
        promptBeforeReload = true;
      }
    }
    if (stopRequested) {
      const result = emptySubtreeResult();
      result.skip = 1;
      result.total = 1;
      return result;
    }

    ++runDepth;

    if (clearRenderedResult) clearRenderedResult();
    subtreeResult = await runSubtree();
    if (updateRenderedResult) updateRenderedResult();

    --runDepth;
    if (runDepth === 0) {
      progressElem.style.display = 'none';
    }

    return subtreeResult;
  };

  const generateMyHTML = (div: HTMLElement) => {
    const subtreeHTML = $('<div>').addClass('subtreechildren');
    const generateSubtree = memoize(() => generateSubtreeHTML(subtreeHTML[0]));

    // Hide subtree - it's not generated yet.
    subtreeHTML.hide();
    const [header, setChecked] = makeTreeNodeHeaderHTML(n, runMySubtree, parentLevel, checked => {
      if (checked) {
        // Make sure the subtree is generated and then show it.
        generateSubtree();
        subtreeHTML.show();
      } else {
        subtreeHTML.hide();
      }
    });

    div.classList.add('subtree');
    div.classList.add(['', 'multifile', 'multitest', 'multicase'][n.query.level]);
    div.appendChild(header);
    div.appendChild(subtreeHTML[0]);

    clearRenderedResult = () => {
      div.removeAttribute('data-status');
    };

    updateRenderedResult = () => {
      let status = '';
      if (subtreeResult.pass > 0) {
        status += 'pass';
      }
      if (subtreeResult.fail > 0) {
        status += 'fail';
      }
      if (subtreeResult.skip === subtreeResult.total && subtreeResult.total > 0) {
        status += 'skip';
      }
      div.setAttribute('data-status', status);
      if (autoCloseOnPass.checked && status === 'pass') {
        div.firstElementChild!.removeAttribute('open');
      }
    };

    updateRenderedResult();

    return () => {
      setChecked();
      const setChildrenChecked = generateSubtree();
      setChildrenChecked();
    };
  };

  return { runSubtree: runMySubtree, generateSubtreeHTML: generateMyHTML };
}

function makeSubtreeChildrenHTML(
  children: Iterable<TestTreeNode>,
  parentLevel: TestQueryLevel
): VisualizedSubtree {
  const childFns = Array.from(children, subtree => makeTreeNodeHTML(subtree, parentLevel));

  const runMySubtree = async () => {
    const results: SubtreeResult[] = [];
    for (const { runSubtree } of childFns) {
      if (stopRequested) {
        break;
      }
      results.push(await runSubtree());
    }
    return mergeSubtreeResults(...results);
  };
  const generateMyHTML = (div: HTMLElement) => {
    const setChildrenChecked = Array.from(childFns, ({ generateSubtreeHTML }) =>
      generateSubtreeHTML(div)
    );

    return () => {
      for (const setChildChecked of setChildrenChecked) {
        setChildChecked();
      }
    };
  };

  return { runSubtree: runMySubtree, generateSubtreeHTML: generateMyHTML };
}

function consoleLogError(e: Error | ErrorWithExtra | undefined) {
  if (e === undefined) return;
  /* eslint-disable-next-line @typescript-eslint/no-explicit-any */
  (globalThis as any)._stack = e;
  console.log('_stack =', e);
  if ('extra' in e && e.extra !== undefined) {
    console.log('_stack.extra =', e.extra);
  }
}

function makeTreeNodeHeaderHTML(
  n: TestTreeNode,
  runSubtree: RunSubtree,
  parentLevel: TestQueryLevel,
  onChange: (checked: boolean) => void
): [HTMLElement, SetCheckedRecursively] {
  const isLeaf = 'run' in n;
  const div = $('<details>').addClass('nodeheader');
  const header = $('<summary>').appendTo(div);

  // prevent toggling if user is selecting text from an input element
  {
    let lastNodeName = '';
    div.on('pointerdown', event => {
      lastNodeName = event.target.nodeName;
    });
    div.on('click', event => {
      if (lastNodeName === 'INPUT') {
        event.preventDefault();
      }
    });
  }

  const setChecked = () => {
    div.prop('open', true); // (does not fire onChange)
    onChange(true);
  };

  const href = createSearchQuery([n.query.toString()]);
  if (onChange) {
    div.on('toggle', function (this) {
      onChange((this as HTMLDetailsElement).open);
    });

    // Expand the shallower parts of the tree at load.
    // Also expand completely within subtrees that are at the same query level
    // (e.g. s:f:t,* and s:f:t,t,*).
    if (n.query.level <= lastQueryLevelToExpand || n.query.level === parentLevel) {
      setChecked();
    }
  }
  const runtext = isLeaf ? 'Run case' : 'Run subtree';
  $('<button>')
    .addClass(isLeaf ? 'leafrun' : 'subtreerun')
    .attr('alt', runtext)
    .attr('title', runtext)
    .on('click', async () => {
      if (runDepth > 0) {
        showInfo('tests are already running');
        return;
      }
      showInfo('');
      console.log(`Starting run for ${n.query}`);
      // turn off all run buttons
      $('#resultsVis').addClass('disable-run');
      const startTime = performance.now();
      await runSubtree();
      const dt = performance.now() - startTime;
      const dtMinutes = dt / 1000 / 60;
      // turn on all run buttons
      $('#resultsVis').removeClass('disable-run');
      console.log(`Finished run: ${dt.toFixed(1)} ms = ${dtMinutes.toFixed(1)} min`);
    })
    .appendTo(header);
  $('<a>')
    .addClass('nodelink')
    .attr('href', href)
    .attr('alt', kOpenTestLinkAltText)
    .attr('title', kOpenTestLinkAltText)
    .appendTo(header);
  $('<button>')
    .addClass('copybtn')
    .attr('alt', 'copy query')
    .attr('title', 'copy query')
    .on('click', () => {
      void navigator.clipboard.writeText(n.query.toString());
    })
    .appendTo(header);
  if ('testCreationStack' in n && n.testCreationStack) {
    $('<button>')
      .addClass('testcaselogbtn')
      .attr('alt', 'Log test creation stack to console')
      .attr('title', 'Log test creation stack to console')
      .appendTo(header)
      .on('click', () => {
        consoleLogError(n.testCreationStack);
      });
  }
  const nodetitle = $('<div>').addClass('nodetitle').appendTo(header);
  const nodecolumns = $('<span>').addClass('nodecolumns').appendTo(nodetitle);
  {
    $('<input>')
      .attr('type', 'text')
      .attr('title', n.query.toString())
      .prop('readonly', true)
      .addClass('nodequery')
      .on('click', event => {
        (event.target as HTMLInputElement).select();
      })
      .val(n.query.toString())
      .appendTo(nodecolumns);
    if (n.subtreeCounts) {
      $('<span>')
        .attr('title', '(Nodes with TODOs) / (Total test count)')
        .text(TestTree.countsToString(n))
        .appendTo(nodecolumns);
    }
  }
  if ('description' in n && n.description) {
    nodetitle.append('&nbsp;');
    $('<pre>') //
      .addClass('nodedescription')
      .text(n.description)
      .appendTo(header);
  }
  return [div[0], setChecked];
}

// Collapse s:f:t:* or s:f:t:c by default.
let lastQueryLevelToExpand: TestQueryLevel = 2;

/**
 * Takes an array of string, ParamValue and returns an array of pairs
 * of [key, value] where value is a string. Converts boolean to '0' or '1'.
 */
function keyValueToPairs([k, v]: [string, boolean | string | null]): [string, string][] {
  const key = camelCaseToSnakeCase(k);
  if (typeof v === 'boolean') {
    return [[key, v ? '1' : '0']];
  } else if (Array.isArray(v)) {
    return v.map(v => [key, v]);
  } else {
    return [[key, v!.toString()]];
  }
}

/**
 * Converts key value pairs to a search string.
 * Keys will appear in order in the search string.
 * Values can be undefined, null, boolean, string, or string[]
 * If the value is falsy the key will not appear in the search string.
 * If the value is an array the key will appear multiple times.
 *
 * @param params Some object with key value pairs.
 * @returns a search string.
 */
function prepareParams(params: Record<string, boolean | string | null>): string {
  const pairsArrays = Object.entries(params)
    .filter(([, v]) => !(v === false || v === null || v === '0'))
    .map(keyValueToPairs);
  const pairs = pairsArrays.flat();
  return new URLSearchParams(pairs).toString();
}

// This is just a cast in one place.
export function optionsToRecord(options: CTSOptions) {
  return options as unknown as Record<string, boolean | string | null>;
}

/**
 * Given a search query, generates a search parameter string
 * @param queries array of queries
 * @param params an optional existing search
 * @returns a search query string
 */
function createSearchQuery(queries: string[], params?: string) {
  params = params === undefined ? prepareParams(optionsToRecord(options)) : params;
  // Add in q separately to avoid escaping punctuation marks.
  return `?${params}${params ? '&' : ''}${queries.map(q => 'q=' + q).join('&')}`;
}

/**
 * Show an info message on the page.
 * @param msg Message to show
 */
function showInfo(msg: string) {
  $('#info')[0].textContent = msg;
}

void (async () => {
  const loader = new DefaultTestFileLoader();

  // MAINTENANCE_TODO: start populating page before waiting for everything to load?
  isFullCTS = qs.length === 1 && qs[0] === rootQuerySpec;

  // Update the URL bar to match the exact current options.
  const updateURLsWithCurrentOptions = () => {
    const params = prepareParams(optionsToRecord(options));
    let url = `${window.location.origin}${window.location.pathname}`;
    url += createSearchQuery(qs, params);
    window.history.replaceState(null, '', url.toString());
    document.querySelectorAll(`a[alt=${kOpenTestLinkAltText}]`).forEach(elem => {
      const a = elem as HTMLAnchorElement;
      const qs = new URLSearchParams(a.search).getAll('q');
      a.search = createSearchQuery(qs, params);
    });
  };

  const addOptionsToPage = (
    options: StandaloneOptions,
    optionsInfos: typeof kStandaloneOptionsInfos
  ) => {
    const optionsElem = $('table#options>tbody')[0];
    const optionValues = optionsToRecord(options);

    const createCheckbox = (optionName: string) => {
      return $(`<input>`)
        .attr('type', 'checkbox')
        .prop('checked', optionValues[optionName] as boolean)
        .on('change', function () {
          optionValues[optionName] = (this as HTMLInputElement).checked;
          updateURLsWithCurrentOptions();
        });
    };

    const createSelect = (optionName: string, info: OptionInfo) => {
      const select = $('<select>').on('change', function (this: HTMLSelectElement) {
        optionValues[optionName] = JSON.parse(this.value);
        updateURLsWithCurrentOptions();
      });
      const currentValue = optionValues[optionName];
      for (const { value, description } of info.selectValueDescriptions!) {
        $('<option>')
          .text(description)
          .val(JSON.stringify(value))
          .prop('selected', value === currentValue)
          .appendTo(select);
      }
      return select;
    };

    Object.entries(optionsInfos).forEach(([optionName, info], i) => {
      const id = `option${i}`;
      const input =
        typeof optionValues[optionName] === 'boolean'
          ? createCheckbox(optionName)
          : createSelect(optionName, info);
      input.attr('id', id);
      $('<tr>')
        .append($('<td>').append(input))
        .append(
          $('<td>').append($('<label>').attr('for', id).text(camelCaseToSnakeCase(optionName)))
        )
        .append($('<td>').text(info.description))
        .appendTo(optionsElem);
    });
  };
  addOptionsToPage(options, kStandaloneOptionsInfos);

  if (qs.length !== 1) {
    showInfo('currently, there must be exactly one ?q=');
    return;
  }

  let rootQuery;
  try {
    rootQuery = parseQuery(qs[0]);
  } catch (e) {
    showInfo((e as Error).toString());
    return;
  }

  if (rootQuery.level > lastQueryLevelToExpand) {
    lastQueryLevelToExpand = rootQuery.level;
  }
  loader.addEventListener('import', ev => {
    showInfo(`loading: ${ev.data.url}`);
  });
  loader.addEventListener('imported', ev => {
    showInfo(`imported: ${ev.data.url}`);
  });
  loader.addEventListener('finish', () => {
    showInfo('');
  });

  let tree;
  try {
    tree = await loader.loadTree(rootQuery);
  } catch (err) {
    showInfo((err as Error).toString());
    return;
  }

  document.title = `${document.title} ${compatibility ? '(compat)' : ''} - ${rootQuery.toString()}`;

  tree.dissolveSingleChildTrees();

  const { runSubtree, generateSubtreeHTML } = makeSubtreeHTML(tree.root, 1);
  const setTreeCheckedRecursively = generateSubtreeHTML(resultsVis);

  document.getElementById('expandall')!.addEventListener('click', () => {
    setTreeCheckedRecursively();
  });

  document.getElementById('copyResultsJSON')!.addEventListener('click', () => {
    void navigator.clipboard.writeText(logger.asJSON(2));
  });

  if (runnow) {
    void runSubtree();
  }
})();
