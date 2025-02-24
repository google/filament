// A listing of all specs within a single suite. This is the (awaited) type of
// `groups` in '{cts,unittests}/listing.ts' and `listing` in the auto-generated
// 'out/{cts,unittests}/listing.js' files (see tools/gen_listings_and_webworkers).
export type TestSuiteListing = TestSuiteListingEntry[];

export type TestSuiteListingEntry = TestSuiteListingEntrySpec | TestSuiteListingEntryReadme;

interface TestSuiteListingEntrySpec {
  readonly file: string[];
}

interface TestSuiteListingEntryReadme {
  readonly file: string[];
  readonly readme: string;
}
