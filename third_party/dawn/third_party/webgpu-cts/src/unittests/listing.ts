/* eslint-disable import/no-restricted-paths */
import { TestSuiteListing } from '../common/internal/test_suite_listing.js';
import { makeListing } from '../common/tools/crawl.js';

export const listing: Promise<TestSuiteListing> = makeListing(__filename);
