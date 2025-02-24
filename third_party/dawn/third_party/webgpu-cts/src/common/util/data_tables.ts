import { ResolveType, ZipKeysWithValues } from './types.js';

export type valueof<K> = K[keyof K];

export function keysOf<T extends string>(obj: { [k in T]: unknown }): readonly T[] {
  return Object.keys(obj) as unknown[] as T[];
}

export function numericKeysOf<T>(obj: object): readonly T[] {
  return Object.keys(obj).map(n => Number(n)) as unknown[] as T[];
}

/**
 * @returns a new Record from `objects`, using the string returned by Object.toString() as the keys
 * and the objects as the values.
 */
export function objectsToRecord<T extends Object>(objects: readonly T[]): Record<string, T> {
  const record = {};
  return objects.reduce((obj, type) => {
    return {
      ...obj,
      [type.toString()]: type,
    };
  }, record);
}

/**
 * Creates an info lookup object from a more nicely-formatted table. See below for examples.
 *
 * Note: Using `as const` on the arguments to this function is necessary to infer the correct type.
 */
export function makeTable<
  Members extends readonly string[],
  Defaults extends readonly unknown[],
  Table extends { readonly [k: string]: readonly unknown[] },
>(
  members: Members,
  defaults: Defaults,
  table: Table
): {
  readonly [k in keyof Table]: ResolveType<ZipKeysWithValues<Members, Table[k], Defaults>>;
} {
  const result: { [k: string]: { [m: string]: unknown } } = {};
  for (const [k, v] of Object.entries<readonly unknown[]>(table)) {
    const item: { [m: string]: unknown } = {};
    for (let i = 0; i < members.length; ++i) {
      item[members[i]] = v[i] ?? defaults[i];
    }
    result[k] = item;
  }
  /* eslint-disable-next-line @typescript-eslint/no-explicit-any */
  return result as any;
}

/**
 * Creates an info lookup object from a more nicely-formatted table.
 *
 * Note: Using `as const` on the arguments to this function is necessary to infer the correct type.
 *
 * Example:
 *
 * ```
 * const t = makeTableWithDefaults(
 *   { c: 'default' },       // columnRenames
 *   ['a', 'default', 'd'],  // columnsKept
 *   ['a', 'b', 'c', 'd'],   // columns
 *   [123, 456, 789, 1011],  // defaults
 *   {                       // table
 *     foo: [1, 2, 3, 4],
 *     bar: [5,  ,  , 8],
 *     moo: [ , 9,10,  ],
 *   }
 * );
 *
 * // t = {
 * //   foo: { a:   1, default:   3, d:    4 },
 * //   bar: { a:   5, default: 789, d:    8 },
 * //   moo: { a: 123, default:  10, d: 1011 },
 * // };
 * ```
 *
 * MAINTENANCE_TODO: `ZipKeysWithValues<Members, Table[k], Defaults>` is incorrect
 * because Members no longer maps to Table[k]. It's not clear if this is even possible to fix
 * because it requires mapping, not zipping. Maybe passing in a index mapping
 * would fix it (which is gross) but if you have columnsKept as [0, 2, 3] then maybe it would
 * be possible to generate the correct type? I don't think we can generate the map at compile time
 * so we'd have to hand code it. Other ideas, don't generate kLimitsInfoCore and kLimitsInfoCompat
 * where they are keys of infos. Instead, generate kLimitsInfoCoreDefaults, kLimitsInfoCoreMaximums,
 * kLimitsInfoCoreClasses where each is just a `{[k: string]: type}`. Could zip those after or,
 * maybe that suggests passing in the hard coded indices would work.
 *
 * @param columnRenames the name of the column in the table that will be assigned to the 'default' property of each entry.
 * @param columnsKept the names of properties you want in the generated lookup table. This must be a subset of the columns of the tables except for the name 'default' which is looked from the previous argument.
 * @param columns the names of the columns of the name
 * @param defaults the default value by column for any element in a row of the table that is undefined
 * @param table named table rows.
 */
export function makeTableRenameAndFilter<
  Members extends readonly string[],
  DataMembers extends readonly string[],
  Defaults extends readonly unknown[],
  Table extends { readonly [k: string]: readonly unknown[] },
>(
  columnRenames: { [key: string]: string },
  columnsKept: Members,
  columns: DataMembers,
  defaults: Defaults,
  table: Table
): {
  readonly [k in keyof Table]: ResolveType<ZipKeysWithValues<Members, Table[k], Defaults>>;
} {
  const result: { [k: string]: { [m: string]: unknown } } = {};
  const keyToIndex = new Map<string, number>(
    columnsKept.map(name => {
      const remappedName = columnRenames[name] === undefined ? name : columnRenames[name];
      return [name, columns.indexOf(remappedName)];
    })
  );
  for (const [k, v] of Object.entries<readonly unknown[]>(table)) {
    const item: { [m: string]: unknown } = {};
    for (const member of columnsKept) {
      const ndx = keyToIndex.get(member)!;
      item[member] = v[ndx] ?? defaults[ndx];
    }
    result[k] = item;
  }
  /* eslint-disable-next-line @typescript-eslint/no-explicit-any */
  return result as any;
}
