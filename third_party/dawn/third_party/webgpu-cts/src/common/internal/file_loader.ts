import { IterableTestGroup } from '../internal/test_group.js';
import { assert } from '../util/util.js';

import { parseQuery } from './query/parseQuery.js';
import { TestQuery } from './query/query.js';
import { TestSuiteListing } from './test_suite_listing.js';
import { loadTreeForQuery, TestTree, TestTreeLeaf } from './tree.js';

// A listing file, e.g. either of:
// - `src/webgpu/listing.ts` (which is dynamically computed, has a Promise<TestSuiteListing>)
// - `out/webgpu/listing.js` (which is pre-baked, has a TestSuiteListing)
interface ListingFile {
  listing: Promise<TestSuiteListing> | TestSuiteListing;
}

// A .spec.ts file, as imported.
export interface SpecFile {
  readonly description: string;
  readonly g: IterableTestGroup;
}

export interface ImportInfo {
  url: string;
}

interface TestFileLoaderEventMap {
  import: MessageEvent<ImportInfo>;
  imported: MessageEvent<ImportInfo>;
  finish: MessageEvent<void>;
}

// Override the types for addEventListener/removeEventListener so the callbacks can be used as
// strongly-typed.
/* eslint-disable-next-line @typescript-eslint/no-unsafe-declaration-merging */
export interface TestFileLoader extends EventTarget {
  addEventListener<K extends keyof TestFileLoaderEventMap>(
    type: K,
    listener: (this: TestFileLoader, ev: TestFileLoaderEventMap[K]) => void,
    options?: boolean | AddEventListenerOptions
  ): void;
  addEventListener(
    type: string,
    listener: EventListenerOrEventListenerObject,
    options?: boolean | AddEventListenerOptions
  ): void;
  removeEventListener<K extends keyof TestFileLoaderEventMap>(
    type: K,
    listener: (this: TestFileLoader, ev: TestFileLoaderEventMap[K]) => void,
    options?: boolean | EventListenerOptions
  ): void;
  removeEventListener(
    type: string,
    listener: EventListenerOrEventListenerObject,
    options?: boolean | EventListenerOptions
  ): void;
}

// Base class for DefaultTestFileLoader and FakeTestFileLoader.
/* eslint-disable-next-line @typescript-eslint/no-unsafe-declaration-merging */
export abstract class TestFileLoader extends EventTarget {
  abstract listing(suite: string): Promise<TestSuiteListing>;
  protected abstract import(path: string): Promise<SpecFile>;

  async importSpecFile(suite: string, path: string[]): Promise<SpecFile> {
    const url = `${suite}/${path.join('/')}.spec.js`;
    this.dispatchEvent(new MessageEvent<ImportInfo>('import', { data: { url } }));
    const ret = await this.import(url);
    this.dispatchEvent(new MessageEvent<ImportInfo>('imported', { data: { url } }));
    return ret;
  }

  async loadTree(
    query: TestQuery,
    {
      subqueriesToExpand = [],
      fullyExpandSubtrees = [],
      maxChunkTime = Infinity,
    }: { subqueriesToExpand?: string[]; fullyExpandSubtrees?: string[]; maxChunkTime?: number } = {}
  ): Promise<TestTree> {
    const tree = await loadTreeForQuery(this, query, {
      subqueriesToExpand: subqueriesToExpand.map(s => {
        const q = parseQuery(s);
        assert(q.level >= 2, () => `subqueriesToExpand entries should not be multi-file:\n  ${q}`);
        return q;
      }),
      fullyExpandSubtrees: fullyExpandSubtrees.map(s => parseQuery(s)),
      maxChunkTime,
    });
    this.dispatchEvent(new MessageEvent<void>('finish'));
    return tree;
  }

  async loadCases(query: TestQuery): Promise<IterableIterator<TestTreeLeaf>> {
    const tree = await this.loadTree(query);
    return tree.iterateLeaves();
  }
}

export class DefaultTestFileLoader extends TestFileLoader {
  async listing(suite: string): Promise<TestSuiteListing> {
    return ((await import(`../../${suite}/listing.js`)) as ListingFile).listing;
  }

  import(path: string): Promise<SpecFile> {
    return import(`../../${path}`);
  }
}
