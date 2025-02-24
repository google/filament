const listOfTestModules = require('./listOfTestModules');
const buildDirs = listOfTestModules.dirs;
const buildFiles = listOfTestModules.files;

function isWildcard (filter) {
  if (filter.includes('*')) return true;
  return false;
}

function filterBy (wildcard, item) {
  return new RegExp('^' + wildcard.replace(/\*/g, '.*') + '$').test(item);
}

/**
 * @param filterCondition
 * matches all given wildcards with available test modules to generate an elaborate filter condition
 */
function matchWildCards (filterCondition) {
  const conditions = filterCondition.split(' ').length ? filterCondition.split(' ') : [filterCondition];
  const matches = [];

  for (const filter of conditions) {
    if (isWildcard(filter)) {
      const matchedDirs = Object.keys(buildDirs).filter(e => filterBy(filter, e));
      if (matchedDirs.length) {
        matches.push(matchedDirs.join(' '));
      }
      const matchedModules = Object.keys(buildFiles).filter(e => filterBy(filter, e));
      if (matchedModules.length) { matches.push(matchedModules.join(' ')); }
    } else {
      matches.push(filter);
    }
  }

  return matches.join(' ');
}

module.exports.matchWildCards = matchWildCards;

/**
 * Test cases
 * @fires only when run directly from terminal
 * eg: node matchModules
 */
if (require.main === module) {
  const assert = require('assert');

  assert.strictEqual(matchWildCards('typed*ex'), 'typed*ex');
  assert.strictEqual(matchWildCards('typed*ex*'), 'typed_threadsafe_function_existing_tsfn');
  assert.strictEqual(matchWildCards('async*'), 'async_context async_progress_queue_worker async_progress_worker async_worker async_worker_persistent');
  assert.strictEqual(matchWildCards('typed*func'), 'typed*func');
  assert.strictEqual(matchWildCards('typed*func*'), 'typed_threadsafe_function');
  assert.strictEqual(matchWildCards('typed*function'), 'typed_threadsafe_function');
  assert.strictEqual(matchWildCards('object*inh'), 'object*inh');
  assert.strictEqual(matchWildCards('object*inh*'), 'objectwrap_multiple_inheritance');
  assert.strictEqual(matchWildCards('*remove*'), 'objectwrap_removewrap');
  assert.strictEqual(matchWildCards('*function'), 'threadsafe_function typed_threadsafe_function');
  assert.strictEqual(matchWildCards('**function'), 'threadsafe_function typed_threadsafe_function');
  assert.strictEqual(matchWildCards('a*w*p*'), 'async_worker_persistent');
  assert.strictEqual(matchWildCards('fun*ref'), 'fun*ref');
  assert.strictEqual(matchWildCards('fun*ref*'), 'function_reference');
  assert.strictEqual(matchWildCards('*reference'), 'function_reference object_reference reference');

  console.log('ALL tests passed');
}
