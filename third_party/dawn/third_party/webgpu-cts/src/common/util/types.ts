/** Forces a type to resolve its type definitions, to make it readable/debuggable. */
export type ResolveType<T> = T extends object
  ? T extends infer O
    ? { [K in keyof O]: ResolveType<O[K]> }
    : never
  : T;

/** Returns the type `true` iff X and Y are exactly equal */
export type TypeEqual<X, Y> = (<T>() => T extends X ? 1 : 2) extends <T>() => T extends Y ? 1 : 2
  ? true
  : false;

/* eslint-disable-next-line @typescript-eslint/no-unused-vars */
export function assertTypeTrue<_ extends true>() {}

/** `ReadonlyArray` of `ReadonlyArray`s. */
export type ROArrayArray<T> = ReadonlyArray<ReadonlyArray<T>>;
/** `ReadonlyArray` of `ReadonlyArray`s of `ReadonlyArray`s. */
export type ROArrayArrayArray<T> = ReadonlyArray<ReadonlyArray<ReadonlyArray<T>>>;

/**
 * Deep version of the Readonly<> type, with support for tuples (up to length 7).
 * <https://gist.github.com/masterkidan/7322752f569b1bba53e0426266768623>
 */
export type DeepReadonly<T> = T extends [infer A]
  ? DeepReadonlyObject<[A]>
  : T extends [infer A, infer B]
  ? DeepReadonlyObject<[A, B]>
  : T extends [infer A, infer B, infer C]
  ? DeepReadonlyObject<[A, B, C]>
  : T extends [infer A, infer B, infer C, infer D]
  ? DeepReadonlyObject<[A, B, C, D]>
  : T extends [infer A, infer B, infer C, infer D, infer E]
  ? DeepReadonlyObject<[A, B, C, D, E]>
  : T extends [infer A, infer B, infer C, infer D, infer E, infer F]
  ? DeepReadonlyObject<[A, B, C, D, E, F]>
  : T extends [infer A, infer B, infer C, infer D, infer E, infer F, infer G]
  ? DeepReadonlyObject<[A, B, C, D, E, F, G]>
  : T extends Map<infer U, infer V>
  ? ReadonlyMap<DeepReadonlyObject<U>, DeepReadonlyObject<V>>
  : T extends Set<infer U>
  ? ReadonlySet<DeepReadonlyObject<U>>
  : T extends Promise<infer U>
  ? Promise<DeepReadonlyObject<U>>
  : T extends Primitive
  ? T
  : T extends (infer A)[]
  ? DeepReadonlyArray<A>
  : DeepReadonlyObject<T>;

type Primitive = string | number | boolean | undefined | null | Function | symbol;
type DeepReadonlyArray<T> = ReadonlyArray<DeepReadonly<T>>;
type DeepReadonlyObject<T> = { readonly [P in keyof T]: DeepReadonly<T[P]> };

/**
 * Computes the intersection of a set of types, given the union of those types.
 *
 * From: https://stackoverflow.com/a/56375136
 */
export type UnionToIntersection<U> =
  /* eslint-disable-next-line @typescript-eslint/no-explicit-any */
  (U extends any ? (k: U) => void : never) extends (k: infer I) => void ? I : never;

/** "Type asserts" that `X` is a subtype of `Y`. */
type EnsureSubtype<X, Y> = X extends Y ? X : never;

type TupleHeadOr<T, Default> = T extends readonly [infer H, ...(readonly unknown[])] ? H : Default;
type TupleTailOr<T, Default> = T extends readonly [unknown, ...infer Tail] ? Tail : Default;
type TypeOr<T, Default> = T extends undefined ? Default : T;

/**
 * Zips a key tuple type and a value tuple type together into an object.
 *
 * @template Keys Keys of the resulting object.
 * @template Values Values of the resulting object. If a key corresponds to a `Values` member that
 *   is undefined or past the end, it defaults to the corresponding `Defaults` member.
 * @template Defaults Default values. If a key corresponds to a `Defaults` member that is past the
 *   end, the default falls back to `undefined`.
 */
export type ZipKeysWithValues<
  Keys extends readonly string[],
  Values extends readonly unknown[],
  Defaults extends readonly unknown[],
> =
  //
  Keys extends readonly [infer KHead, ...infer KTail]
    ? {
        readonly [k in EnsureSubtype<KHead, string>]: TypeOr<
          TupleHeadOr<Values, undefined>,
          TupleHeadOr<Defaults, undefined>
        >;
      } & ZipKeysWithValues<
        EnsureSubtype<KTail, readonly string[]>,
        TupleTailOr<Values, []>,
        TupleTailOr<Defaults, []>
      >
    : {}; // K exhausted
