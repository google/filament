/**
 * Utilities to improve the performance of the CTS, by caching data that is
 * expensive to build using a two-level cache (in-memory, pre-computed file).
 */

import { assert } from '../util/util.js';

interface DataStore {
  load(path: string): Promise<Uint8Array>;
}

/** Logger is a basic debug logger function */
export type Logger = (s: string) => void;

/**
 * DataCacheNode represents a single cache entry in the LRU DataCache.
 * DataCacheNode is a doubly linked list, so that least-recently-used entries can be removed, and
 * cache hits can move the node to the front of the list.
 */
class DataCacheNode {
  public constructor(path: string, data: unknown) {
    this.path = path;
    this.data = data;
  }

  /** insertAfter() re-inserts this node in the doubly-linked list after `prev` */
  public insertAfter(prev: DataCacheNode) {
    this.unlink();
    this.next = prev.next;
    this.prev = prev;
    prev.next = this;
    if (this.next) {
      this.next.prev = this;
    }
  }

  /** unlink() removes this node from the doubly-linked list */
  public unlink() {
    const prev = this.prev;
    const next = this.next;
    if (prev) {
      prev.next = next;
    }
    if (next) {
      next.prev = prev;
    }
    this.prev = null;
    this.next = null;
  }

  public readonly path: string; // The file path this node represents
  public readonly data: unknown; // The deserialized data for this node
  public prev: DataCacheNode | null = null; // The previous node in the doubly-linked list
  public next: DataCacheNode | null = null; // The next node in the doubly-linked list
}

/** DataCache is an interface to a LRU-cached data store used to hold data cached by path */
export class DataCache {
  public constructor() {
    this.lruHeadNode.next = this.lruTailNode;
    this.lruTailNode.prev = this.lruHeadNode;
  }

  /** setDataStore() sets the backing data store used by the data cache */
  public setStore(dataStore: DataStore) {
    this.dataStore = dataStore;
  }

  /** setDebugLogger() sets the verbose logger */
  public setDebugLogger(logger: Logger) {
    this.debugLogger = logger;
  }

  /**
   * fetch() retrieves cacheable data from the data cache, first checking the
   * in-memory cache, then the data store (if specified), then resorting to
   * building the data and storing it in the cache.
   */
  public async fetch<Data>(cacheable: Cacheable<Data>): Promise<Data> {
    {
      // First check the in-memory cache
      const node = this.cache.get(cacheable.path);
      if (node !== undefined) {
        this.log('in-memory cache hit');
        node.insertAfter(this.lruHeadNode);
        return Promise.resolve(node.data as Data);
      }
    }
    this.log('in-memory cache miss');
    // In in-memory cache miss.
    // Next, try the data store.
    if (this.dataStore !== null && !this.unavailableFiles.has(cacheable.path)) {
      let serialized: Uint8Array | undefined;
      try {
        serialized = await this.dataStore.load(cacheable.path);
        this.log('loaded serialized');
      } catch (err) {
        // not found in data store
        this.log(`failed to load (${cacheable.path}): ${err}`);
        this.unavailableFiles.add(cacheable.path);
      }
      if (serialized !== undefined) {
        this.log(`deserializing`);
        const data = cacheable.deserialize(serialized);
        this.addToCache(cacheable.path, data);
        return data;
      }
    }
    // Not found anywhere. Build the data, and cache for future lookup.
    this.log(`cache: building (${cacheable.path})`);
    const data = await cacheable.build();
    this.addToCache(cacheable.path, data);
    return data;
  }

  /**
   * addToCache() creates a new node for `path` and `data`, inserting the new node at the front of
   * the doubly-linked list. If the number of entries in the cache exceeds this.maxCount, then the
   * least recently used entry is evicted
   * @param path the file path for the data
   * @param data the deserialized data
   */
  private addToCache(path: string, data: unknown) {
    if (this.cache.size >= this.maxCount) {
      const toEvict = this.lruTailNode.prev;
      assert(toEvict !== null);
      toEvict.unlink();
      this.cache.delete(toEvict.path);
      this.log(`evicting ${toEvict.path}`);
    }
    const node = new DataCacheNode(path, data);
    node.insertAfter(this.lruHeadNode);
    this.cache.set(path, node);
    this.log(`added ${path}. new count: ${this.cache.size}`);
  }

  private log(msg: string) {
    if (this.debugLogger !== null) {
      this.debugLogger(`DataCache: ${msg}`);
    }
  }

  // Max number of entries in the cache before LRU entries are evicted.
  private readonly maxCount = 4;

  private cache = new Map<string, DataCacheNode>();
  private lruHeadNode = new DataCacheNode('', null); // placeholder node (no path or data)
  private lruTailNode = new DataCacheNode('', null); // placeholder node (no path or data)
  private unavailableFiles = new Set<string>();
  private dataStore: DataStore | null = null;
  private debugLogger: Logger | null = null;
}

/** The data cache */
export const dataCache = new DataCache();

/** true if the current process is building the cache */
let isBuildingDataCache = false;

/** @returns true if the data cache is currently being built */
export function getIsBuildingDataCache() {
  return isBuildingDataCache;
}

/** Sets whether the data cache is currently being built */
export function setIsBuildingDataCache(value = true) {
  isBuildingDataCache = value;
}

/**
 * Cacheable is the interface to something that can be stored into the
 * DataCache.
 * The 'npm run gen_cache' tool will look for module-scope variables of this
 * interface, with the name `d`.
 */
export interface Cacheable<Data> {
  /** the globally unique path for the cacheable data */
  readonly path: string;

  /**
   * build() builds the cacheable data.
   * This is assumed to be an expensive operation and will only happen if the
   * cache does not already contain the built data.
   */
  build(): Promise<Data>;

  /**
   * serialize() encodes `data` to a binary representation so that it can be stored in a cache file.
   */
  serialize(data: Data): Uint8Array;

  /**
   * deserialize() is the inverse of serialize(), decoding the binary representation back to a Data
   * object.
   */
  deserialize(binary: Uint8Array): Data;
}
