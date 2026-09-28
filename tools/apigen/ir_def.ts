/**
 * Universal C++ Binding Generator - Intermediate Representation (IR)
 * 
 * This file defines the JSON schema output by the C++ Extractor.
 * Target Generators (Java, Rust, JS) consume this structure to produce bindings.
 */

// ----------------------------------------------------------------------------
// Root Object
// ----------------------------------------------------------------------------

export interface API {
  /** Global configuration and context */
  meta: {
    generator_version: string;
    source_file: string;
  };

  /** Dependency Graph: Files required for this API to compile/link */
  includes: {
    /** Local header files (e.g. "my_game/entity.h") - useful for Java package mapping */
    user: string[];
    /** System header files (e.g. "<vector>") - useful for standard library mapping */
    system: string[];
  };

  /** Top-level definitions */
  aliases: Alias[];
  enums: Enum[];
  classes: Class[];
  functions: Function[];

  /**
   * Lazily discovered referenced external types and aggregate structs
   * (e.g. Viewport, Box, PixelBufferDescriptor).
   */
  referenced_classes: Class[];
}

// ----------------------------------------------------------------------------
// Common Entities
// ----------------------------------------------------------------------------

export interface Entity {
  /** Unqualified name (e.g. "Entity") */
  name: string;
  /** Fully qualified C++ name (e.g. "mygame::Entity") */
  qualified_name: string;
  /** Doxygen-parsed documentation */
  doc: Documentation;
  /** File and line number in the source C++ */
  location: SourceLocation;
  /** 
   * Backend attributes and annotations.
   * e.g. ["filament:apigen:skip", "filament:apigen:retained",
   *       "filament:apigen:bitfield", "filament:apigen:flags", "filament:apigen:alternate_name:importTexture",
   *       "filament:apigen:tagged_array", "filament:apigen:used_by_native"]
   * Populated via `[[clang::annotate("filament:apigen:...")]]` / `UTILS_NOAPIGEN` / etc.
   */
  attributes: string[];
}

export interface SourceLocation {
  file: string;
  line: number;
}

export interface Documentation {
  /**
   * First paragraph or sentence of the doc comment.
   * Format: CommonMark (normalized from Doxygen)
   */
  brief: string;
  /**
   * Detailed description (all subsequent paragraphs, code blocks, tables).
   * Format: CommonMark (normalized from Doxygen)
   */
  details: string;
  /** Mapping of argument names to descriptions (from @param) */
  params: Record<string, string>;
  /** Description of the return value (from @return) */
  returns: string;
  /** Custom and block tags (e.g. @see is string[], @warning, @note, @deprecated, @since) */
  meta: Record<string, string | string[]>;
}

// ----------------------------------------------------------------------------
// Type System
// ----------------------------------------------------------------------------

export interface Type {
  /** 
   * The raw C++ type string, canonicalized.
   * e.g. "const char*", "std::vector<int>", "size_t", "math::float3", "float[3]"
   */
  cpp_name: string;
  /**
   * The fully qualified name (e.g. "filament::math::details::TVec3<float>").
   * Used for strict type linking, canonical desugaring, and imports.
   */
  qualified_name: string;

  /** 
   * High-level classification for easier mapping across target generators.
   */
  category: TypeCategory;

  /** If true, this type is const-qualified (e.g. const int, const Box&) */
  is_const: boolean;
  /** If true, this is a pointer (e.g. int*, Camera*) */
  is_pointer: boolean;
  /** If true, this is an l-value reference (e.g. int&, const Box&) */
  is_reference: boolean;
  /** If true, this is an r-value reference (e.g. int&&, PixelBufferDescriptor&&) */
  is_move_reference: boolean;

  /** Nullability contract for pointers/references and return types */
  nullability: Nullability;

  /** If true, this type is or references a template parameter (e.g. T, const T*) */
  is_template_param?: boolean;
  /** Name of the template parameter when is_template_param is true (e.g. "T") */
  template_param_name?: string;
}

export type Nullability =
  | "unspecified" // Default C++ behavior
  | "nullable"    // _Nullable / UTILS_NULLABLE
  | "nonnull";    // _Nonnull / UTILS_NONNULL

export type TypeCategory =
  | "void"
  | "primitive"   // Fixed-width/sized ints (int32_t, uint8_t, size_t), float, double, bool
  | "string"      // const char*, std::string, std::string_view, utils::CString, StaticString, ImmutableCString
  | "chrono"      // std::chrono::duration, std::chrono::time_point
  | "tribool"     // utils::tribool
  | "optional"    // std::optional
  | "enum"        // User-defined enum
  | "object"      // User-defined class/struct/handle
  | "struct"      // Aggregate / POD struct (e.g. Box, Viewport)
  | "container"   // std::vector, std::array, FixedCapacityVector
  | "stdfunc"     // std::function, utils::Invocable
  | "unknown";    // Fallback / array types (e.g. float[3])

// ----------------------------------------------------------------------------
// Classes, Structs & Archetypes
// ----------------------------------------------------------------------------

export type ClassArchetype =
  | "handle"         // Managed engine object holding mNativeObject (e.g. Camera, Scene)
  | "bitfield"       // Packed scalar bitfield value object (e.g. TextureSampler)
  | "inline_buffer"  // Fixed inline array buffer value class (e.g. Frustum)
  | "aggregate"      // Pure data-only aggregate struct with exploded primitive fields (e.g. Box, Viewport)
  | "utility"        // Pure static utility functions namespace (e.g. Colors, Exposure)
  | "builder"        // Fluent nested builder class (e.g. IndirectLight::Builder)
  | "pojo_struct";   // Public mutable options/record struct (e.g. DynamicResolutionOptions, PickingQueryResult)

export type ClassCategory =
  | "object"
  | "struct"
  | "utility"
  | "builder"
  | "bitfield";

export interface Class extends Entity {
  /** Class type info */
  type: Type;

  /** Parent class fully qualified names or spelling */
  bases: string[];

  fields: Field[];
  methods: Function[];
  /** 
   * Internal aliases (using/typedef) exposed by this class. 
   * e.g. "using Instance = utils::EntityInstance<LightManager>;"
   */
  aliases: Alias[];

  /**
   * Static constexpr class constants (VAR_DECL).
   * e.g. "static constexpr uint64_t FENCE_WAIT_FOR_EVER = ...;"
   */
  constants: Constant[];

  /**
   * Enums declared or aliased inside this class scope.
   */
  enums: Enum[];

  /** True if nested within an outer class or struct */
  is_nested?: boolean;

  /** Name of the enclosing class if nested (e.g. "LightManager", "ShadowOptions") */
  parent_class?: string;

  /** True if pure aggregate POD struct (non-polymorphic, all public fields) */
  is_aggregate?: boolean;

  /** True if pure static utility class (no fields, all static methods) */
  is_utility?: boolean;

  /** True if builder class (named Builder or deriving from BuilderBase) */
  is_builder?: boolean;

  /** True if synthesized from a C++ namespace with functions */
  is_namespace?: boolean;

  /** True if POJO struct with public mutable fields (e.g. DynamicResolutionOptions, PickingQueryResult) */
  is_pojo_struct?: boolean;

  /** High-level behavioral archetype driving generator binding patterns */
  archetype?: ClassArchetype;

  /** High-level category classification */
  category?: ClassCategory;

  /** Sized byte length of bitfield backing storage (<= 8) */
  bitfield_size?: number;

  /** Dynamic primitive type backing bitfield archetype ("short", "int", "long") */
  bitfield_primitive?: "short" | "int" | "long";
}

export interface Field extends Entity {
  type: Type;
  /** Raw C++ default value string if present (e.g. "1024", "false", "{ 0.f , 0.f , 0.f }") */
  default_value: string | null;
  /** True if field is a bitfield declaration */
  is_bitfield?: boolean;
  /** Width in bits if field is a bitfield */
  bitfield_width?: number | null;
}

export interface Constant extends Entity {
  type: Type;
  /** Evaluated/cleaned constant value expression (e.g. "-1", "1024", "0x1") */
  value: string;
  /** Raw token expression from C++ source before alias/expression normalization */
  raw_value?: string;
}

export interface Enum extends Entity {
  /** The underlying integer type (e.g. "uint32_t", "int") */
  underlying_type: string;
  entries: EnumEntry[];
  /** True if annotated with apigen:flags or apigen:bitmask */
  is_flags?: boolean;
}

export interface EnumEntry {
  name: string;
  value: number;
  doc: Documentation;
}

export interface Alias extends Entity {
  /** The underlying type being aliased */
  type: Type;
}

// ----------------------------------------------------------------------------
// Functions, Methods & Templated APIs
// ----------------------------------------------------------------------------

export interface Function extends Entity {
  return_type: Type;
  arguments: Argument[];

  /** True if static method */
  is_static: boolean;
  /** True if const method (only valid for member functions) */
  is_const: boolean;
  /** True if method is marked noexcept, throw(), or noexcept(true) */
  is_noexcept: boolean;
  /** True if method is a constructor (CONSTRUCTOR cursor) */
  is_constructor?: boolean;

  /** 
   * Template Support (Compressed Representation)
   * If this is a generic function, this list will be non-empty.
   */
  template_parameters: TemplateParameter[];

  /**
   * SFINAE or Concept output.
   * e.g. "std::enable_if_t<is_supported_parameter_t<T>>"
   */
  constraint: string | null;

  /**
   * Optional concrete types to instantiate for backends that don't support generics (Java).
   * Populated from SFINAE traits or generator configuration.
   * e.g. [ { "T": "int" }, { "T": "float" } ]
   */
  specializations: Record<string, string>[];
}

export interface Argument {
  name: string;
  type: Type;
  /** Raw C++ default value string if present (e.g. "nullptr", "1.0f", "0") */
  default_value: string | null;
  /**
   * Argument-specific attributes.
   * e.g. ["apigen:size_param:count"] from [[clang::annotate("apigen:size_param:count")]]
   */
  attributes: string[];
}

export interface TemplateParameter {
  name: string; // e.g. "T"
  type: string; // "typename", "class", or non-type parameter type (e.g. "int")
}


