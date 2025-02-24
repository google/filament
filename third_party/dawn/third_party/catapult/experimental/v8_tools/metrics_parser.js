'use strict';

//  Method for parsing the metric string. It creates collections with
//  all types of browsersOptions, subprocessesOptions,
// probes, components and sizes.
//  Sizes are splitted in two because they are linked with the type of
//  probe: from Chrome or from OS.
//  Components are stored in a hierarchy made from maps.
function parseAllMetrics(metricNames) {
  let browsersOptions = [];
  let subprocessesOptions = [];
  const componentsTree = new Map();
  const sizeMap = new Map();
  for (let element of metricNames) {
    if (element.includes('process_count') ||
      element.includes('dump_count')) {
      continue;
    }
    if (element.startsWith('memory')) {
      element = element.split(':');
      browsersOptions.push(element[1]);
      subprocessesOptions.push(element[2]);
      if (sizeMap.has(element[3])) {
        const array = sizeMap.get(element[3]);
        const sizeElem = element[element.length - 1];
        if (array.indexOf(sizeElem) === -1) {
          array.push(element[element.length - 1]);
          sizeMap.set(element[3], array);
        }
      } else {
        sizeMap.set(element[3], [element[element.length - 1]]);
      }
      let map = componentsTree;
      for (let i = 3; i < element.length - 1; i++) {
        if (!map.has(element[i])) {
          const newMap = new Map();
          map.set(element[i], newMap);
          map = newMap;
        } else {
          map = map.get(element[i]);
        }
      }
    }
  }

  browsersOptions = _.uniq(browsersOptions);
  subprocessesOptions = _.uniq(subprocessesOptions);

  return {
    browsers: browsersOptions,
    subprocesses: subprocessesOptions,
    components: componentsTree,
    sizes: sizeMap,
    names: metricNames
  };
}
