import { assert } from '../util/util.js';

/** Metadata about tests (that can't be derived at runtime). */
export type TestMetadata = {
  /**
   * Estimated average time-per-subcase, in milliseconds.
   * This is used to determine chunking granularity when exporting to WPT with
   * chunking enabled (like out-wpt/cts-chunked2sec.https.html).
   */
  subcaseMS: number;
};

export type TestMetadataListing = {
  [testQuery: string]: TestMetadata;
};

export function loadMetadataForSuite(suiteDir: string): TestMetadataListing | null {
  assert(typeof require !== 'undefined', 'loadMetadataForSuite is only implemented on Node');
  /* eslint-disable-next-line n/no-restricted-require */
  const fs = require('fs');

  const metadataFile = `${suiteDir}/listing_meta.json`;
  if (!fs.existsSync(metadataFile)) {
    return null;
  }

  const metadata: TestMetadataListing = JSON.parse(fs.readFileSync(metadataFile, 'utf8'));
  return metadata;
}
