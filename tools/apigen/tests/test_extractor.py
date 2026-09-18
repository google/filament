#!/usr/bin/env python3
#
# Copyright (C) 2026 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import unittest
import sys
import os
import tempfile
from pathlib import Path

# Add parent directory to path to import extractor
sys.path.append(str(Path(__file__).parent.parent))

import extractor
from extractor import Extractor

class TestExtractor(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Initialize Clang once
        cls.cindex = extractor.setup_clang()

    def parse_source(self, code, suffix=".h"):
        """Helper to parse C++ code string into an Extractor instance."""
        # Create a temporary file
        with tempfile.NamedTemporaryFile(suffix=suffix, mode="w", delete=False) as f:
            f.write(code)
            fname = f.name

        try:
            index = self.cindex.Index.create()
            # We need to include potential system headers or at least basic types
            # For unit tests, we can often get away with minimal arguments
            # or by mocking everything in the snippet.
            args = ["-x", "c++", "-std=c++17"]
            tu = index.parse(fname, args=args)
            
            # Check for errors
            for diag in tu.diagnostics:
                if diag.severity >= self.cindex.Diagnostic.Error:
                    print(f"Diagnostic: {diag.spelling}")



            ex = Extractor(fname)
            ex.extract(tu.cursor)
            return ex
        finally:
            if os.path.exists(fname):
                os.remove(fname)

    def test_basic_class(self):
        code = """
        class MyClass {
            public:
                void methodA();
                int fieldB;
        };
        """
        ex = self.parse_source(code)
        classes = ex.output["classes"]
        self.assertEqual(len(classes), 1)
        self.assertEqual(classes[0]["name"], "MyClass")
        
        methods = classes[0]["methods"]
        self.assertEqual(len(methods), 1)
        self.assertEqual(methods[0]["name"], "methodA")
        
        fields = classes[0]["fields"]
        self.assertEqual(len(fields), 1)
        self.assertEqual(fields[0]["name"], "fieldB")

    def test_specialization_trait_scanning(self):
        # This tests the "scan_tokens_for_traits" fallback logic
        # We deliberately omit <type_traits> so AST might fail to resolve enable_if_t fully if it was real code,
        # but our snippet uses a fake enable_if_t to emulate the pattern found in MaterialInstance.h
        code = """
        namespace std {
            template<bool B, class T = void> struct enable_if {};
            template<bool B, class T = void> using enable_if_t = typename enable_if<B,T>::type;
        }
        
        template<typename T> struct is_supported_parameter_t {};
        
        class MaterialInstance {
        public:
            template <typename T, typename = is_supported_parameter_t<T>>
            void setParameter(const char* name, T value);
        };
        """
        # We need to manually inject the trait definition into the extractor's known traits
        # because our simple scanner mimics finding it in a complex header chain.
        # However, the scanner *should* find it if we provide the "using" definition.
        
        # Let's try to simulate exactly what broke before:
        # A trait defined via 'using' that gets used in a template default argument.
        
        code_with_trait = """
        template<typename T> struct is_supported { static constexpr bool value = true; };
        
        // The pattern we match in scan_tokens_for_traits:
        // using NAME = std::enable_if_t< ... >
        
        // Mocking the tokens to match the regex/state machine
        namespace std { 
            template<bool B, class T = void> struct enable_if { using type = T; };
            template<bool B, class T = void> using enable_if_t = typename enable_if<B,T>::type;
            
            template<class A, class B> struct is_same { static constexpr bool value = false; };
            // Define is_same_v as a variable template to match usage
            template<class A, class B> constexpr bool is_same_v = is_same<A,B>::value;
        }
        
        class MaterialInstance {
        public:
            template <typename T>
            using is_supported_parameter_t = std::enable_if_t<
                std::is_same_v<float, T> ||
                std::is_same_v<int, T>
            >;
            
            template <typename T, typename = is_supported_parameter_t<T>>
            void setParameter(const char* name, T value);
        };
        """
        
        
        ex = self.parse_source(code_with_trait)
        
        # Check if trait was found
        self.assertIn("is_supported_parameter_t", ex.traits)
        self.assertIn("float", ex.traits["is_supported_parameter_t"])
        self.assertIn("int", ex.traits["is_supported_parameter_t"])
        
        # Check if setParameter has specializations
        cls = next(c for c in ex.output["classes"] if c["name"] == "MaterialInstance")
        
        method = cls["methods"][0]
        self.assertEqual(method["name"], "setParameter")
        self.assertTrue(len(method["specializations"]) > 0)
        
        # Verify specific specialization
        specs = method["specializations"]
        types = [s["T"] for s in specs]
        self.assertIn("float", types)
        self.assertIn("int", types)


    def test_nested_builder(self):
        """Verify that nested Builder classes are extracted (Regression test for Texture::Builder)."""
        code = """
        class Texture {
        public:
            class Builder {
            public:
                void build();
            };
        };
        """
        ex = self.parse_source(code)
        classes = ex.output["classes"]
        
        # We expect "Texture" class
        texture = next((c for c in classes if c["name"] == "Texture"), None)
        self.assertIsNotNone(texture)
        
        # Check if Builder is in the classes list
        builder = next((c for c in classes if c["name"] == "Builder"), None)
        self.assertIsNotNone(builder, "Nested Builder class incorrectly missing from output")
        
        # Verify qualified name
        # print(f"DEBUG: Builder Qual Name: {builder['qualified_name']}")
        self.assertIn("Texture::Builder", builder["qualified_name"])
        
    def test_aliases(self):
        """Verify that 'using' aliases are extracted (Regression test for Texture types)."""
        code = """
        namespace backend {
            enum class TextureFormat { RED, RGBA };
        }
        class Texture {
        public:
            using InternalFormat = backend::TextureFormat;
        };
        """
        ex = self.parse_source(code)
        classes = ex.output["classes"]
        texture = next(c for c in classes if c["name"] == "Texture")
        
        # Check for aliases
        aliases = texture.get("aliases", [])
        self.assertTrue(len(aliases) > 0, "Aliases list is empty")
        
        internal_format = next((a for a in aliases if a["name"] == "InternalFormat"), None)
        self.assertIsNotNone(internal_format, "InternalFormat alias missing")
        self.assertEqual(internal_format["type"]["cpp_name"], "backend::TextureFormat")

    def test_enum_param(self):
        """Verify that enum parameters retain their enum type (Regression test for Texture::Sampler)."""
        code = """
        namespace backend {
            enum class SamplerType : unsigned char { LINEAR, NEAREST };
        }
        class Texture {
        public:
            using Sampler = backend::SamplerType;
            static void getMaxTextureSize(Sampler type);
        };
        """
        ex = self.parse_source(code)
        classes = ex.output["classes"]
        texture = next(c for c in classes if c["name"] == "Texture")
        method = next(m for m in texture["methods"] if m["name"] == "getMaxTextureSize")
        
        arg_type = method["arguments"][0]["type"]
        
        self.assertIn(arg_type["cpp_name"], ["Sampler", "Texture::Sampler", "backend::SamplerType"])
        self.assertEqual(arg_type["category"], "enum", "Enum param category degraded to primitive/unknown")

    def test_reference_category(self):
        """Verify that references to objects have correct category (not unknown)."""
        code = """
        class MyObj {};
        class Tester {
        public:
            void takeObj(MyObj& o);
            void takeObjMove(MyObj&& o);
        };
        """
        ex = self.parse_source(code)
        classes = ex.output["classes"]
        tester = next(c for c in classes if c["name"] == "Tester")
        
        takeObj = next(m for m in tester["methods"] if m["name"] == "takeObj")
        self.assertEqual(takeObj["arguments"][0]["type"]["category"], "object")
        
        takeObjMove = next(m for m in tester["methods"] if m["name"] == "takeObjMove")
        self.assertEqual(takeObjMove["arguments"][0]["type"]["category"], "object")

    def test_fqn_desugaring(self):
        """Verify that typedefs are desugared to their fully qualified underlying types."""
        code = """
        namespace utils {
            class Entity {};
        }
        class Foo {
        public:
            using Thing = utils::Entity;
            void setThing(Thing t);
        };
        """
        ex = self.parse_source(code)
        foo = next(c for c in ex.output["classes"] if c["name"] == "Foo")
        method = next(m for m in foo["methods"] if m["name"] == "setThing")
        arg_type = method["arguments"][0]["type"]
        self.assertEqual(arg_type["cpp_name"], "Thing")
        self.assertEqual(arg_type["qualified_name"], "utils::Entity")

    def test_sized_types_preservation(self):
        """Verify that fixed-width integer types preserve their sized names in qualified_name."""
        code = """
        typedef unsigned int uint32_t;
        typedef signed char int8_t;
        typedef unsigned long size_t;
        using Id = uint32_t;
        class Bar {
        public:
            void testSized(uint32_t a, int8_t b, size_t c, Id d);
        };
        """
        ex = self.parse_source(code)
        bar = next(c for c in ex.output["classes"] if c["name"] == "Bar")
        method = next(m for m in bar["methods"] if m["name"] == "testSized")
        args = method["arguments"]
        self.assertEqual(args[0]["type"]["qualified_name"], "uint32_t")
        self.assertEqual(args[1]["type"]["qualified_name"], "int8_t")
        self.assertEqual(args[2]["type"]["qualified_name"], "size_t")
        self.assertEqual(args[3]["type"]["cpp_name"], "Id")
        self.assertEqual(args[3]["type"]["qualified_name"], "uint32_t")

    def test_is_noexcept(self):
        """Verify that noexcept specifications are correctly extracted."""
        code = """
        class Tester {
        public:
            void fNoexcept() noexcept;
            void fThrowing();
            void fNoexceptTrue() noexcept(true);
            void fNoexceptFalse() noexcept(false);
            void fThrowSpec() throw();
        };
        """
        ex = self.parse_source(code)
        tester = next(c for c in ex.output["classes"] if c["name"] == "Tester")
        methods = {m["name"]: m["is_noexcept"] for m in tester["methods"]}
        self.assertTrue(methods["fNoexcept"])
        self.assertFalse(methods["fThrowing"])
        self.assertTrue(methods["fNoexceptTrue"])
        self.assertFalse(methods["fNoexceptFalse"])
        self.assertTrue(methods["fThrowSpec"])

    def test_nested_enums_and_static_methods(self):
        """Verify that nested class enums and static method flags are correctly extracted."""
        code = """
        class CameraTest {
        public:
            enum class Projection : int {
                PERSPECTIVE,
                ORTHO
            };
            static void projection(Projection p);
            void setProjection(Projection p);
        };
        """
        ex = self.parse_source(code)
        camera = next(c for c in ex.output["classes"] if c["name"] == "CameraTest")
        self.assertEqual(len(camera.get("enums", [])), 1)
        enum_obj = camera["enums"][0]
        self.assertEqual(enum_obj["name"], "Projection")
        self.assertEqual(len(enum_obj["entries"]), 2)
        self.assertEqual(enum_obj["entries"][0]["name"], "PERSPECTIVE")
        self.assertEqual(enum_obj["entries"][1]["name"], "ORTHO")

        proj_static = next(m for m in camera["methods"] if m["name"] == "projection")
        self.assertTrue(proj_static["is_static"])

        proj_instance = next(m for m in camera["methods"] if m["name"] == "setProjection")
        self.assertFalse(proj_instance["is_static"])

    def test_overloads_and_default_arguments(self):
        """Verify that overloads and default arguments are properly extracted."""
        code = """
        class Foo {
        public:
            void setIntensity(float intensity);
            void setIntensity(float watts, float efficiency);
            void configure(int a, float b = 1.0f);
        };
        """
        ex = self.parse_source(code)
        foo = next(c for c in ex.output["classes"] if c["name"] == "Foo")
        methods = foo["methods"]
        self.assertEqual(len(methods), 3)
        
        cfg = next(m for m in methods if m["name"] == "configure")
        self.assertEqual(len(cfg["arguments"]), 2)
        self.assertIsNone(cfg["arguments"][0].get("default_value"))
        self.assertIsNotNone(cfg["arguments"][1].get("default_value"))
        self.assertEqual(cfg["arguments"][1]["default_value"], "1.0f")

    def test_phase1_string_type_extraction(self):
        """Verify extraction and categorization of string types."""
        code = """
        namespace std {
            template<typename CharT> class basic_string_view {};
            using string_view = basic_string_view<char>;
            class string {};
        }
        namespace utils {
            class CString {};
            class StaticString {};
            class ImmutableCString {};
        }
        class StringExtractorTest {
        public:
            void takeView(std::string_view v);
            void takeCString(const utils::CString& s);
            void takeStaticString(utils::StaticString s);
            utils::ImmutableCString getImmutable() const;
            const char* getRaw() const;
        };
        """
        ex = self.parse_source(code)
        cls = next(c for c in ex.output["classes"] if c["name"] == "StringExtractorTest")
        methods = {m["name"]: m for m in cls["methods"]}

        self.assertEqual(methods["takeView"]["arguments"][0]["type"]["category"], "string")
        self.assertEqual(methods["takeCString"]["arguments"][0]["type"]["category"], "string")
        self.assertEqual(methods["takeStaticString"]["arguments"][0]["type"]["category"], "string")
        self.assertEqual(methods["getImmutable"]["return_type"]["category"], "string")
        self.assertEqual(methods["getRaw"]["return_type"]["category"], "string")

    def test_phase1_chrono_extraction(self):
        """Verify extraction and categorization of chrono duration and time_point types."""
        code = """
        namespace std {
            namespace chrono {
                template<class Rep, class Period = void> class duration {};
                using nanoseconds = duration<long long>;
                using milliseconds = duration<long long>;
                using seconds = duration<long long>;
                template<class Clock, class Duration = void> class time_point {};
                class steady_clock {};
            }
        }
        class ChronoExtractorTest {
        public:
            void setNanos(std::chrono::nanoseconds ns);
            void setMillis(std::chrono::milliseconds ms);
            std::chrono::time_point<std::chrono::steady_clock> getTimestamp() const;
        };
        """
        ex = self.parse_source(code)
        cls = next(c for c in ex.output["classes"] if c["name"] == "ChronoExtractorTest")
        methods = {m["name"]: m for m in cls["methods"]}

        self.assertEqual(methods["setNanos"]["arguments"][0]["type"]["category"], "chrono")
        self.assertEqual(methods["setMillis"]["arguments"][0]["type"]["category"], "chrono")
        self.assertEqual(methods["getTimestamp"]["return_type"]["category"], "chrono")

    def test_phase1_tribool_and_optional_extraction(self):
        """Verify extraction and categorization of utils::tribool and std::optional."""
        code = """
        namespace utils {
            struct tribool {};
        }
        namespace std {
            template<typename T> class optional {};
        }
        class TriboolExtractorTest {
        public:
            void setTri(utils::tribool mode);
            utils::tribool getTri() const;
            void setOpt(std::optional<bool> flag);
            std::optional<bool> getOpt() const;
        };
        """
        ex = self.parse_source(code)
        cls = next(c for c in ex.output["classes"] if c["name"] == "TriboolExtractorTest")
        methods = {m["name"]: m for m in cls["methods"]}

        self.assertEqual(methods["setTri"]["arguments"][0]["type"]["category"], "tribool")
        self.assertEqual(methods["getTri"]["return_type"]["category"], "tribool")
        self.assertEqual(methods["setOpt"]["arguments"][0]["type"]["category"], "optional")
        self.assertEqual(methods["getOpt"]["return_type"]["category"], "optional")

    def test_phase1_bitset_extraction(self):
        """Verify extraction of utils::bitset."""
        code = """
        typedef unsigned int uint32_t;
        namespace utils {
            template<typename T> struct bitset {};
        }
        class BitsetExtractorTest {
        public:
            void setFlags(utils::bitset<uint32_t> flags);
            utils::bitset<uint32_t> getFlags() const;
        };
        """
        ex = self.parse_source(code)
        cls = next(c for c in ex.output["classes"] if c["name"] == "BitsetExtractorTest")
        methods = {m["name"]: m for m in cls["methods"]}

        self.assertIn("bitset", methods["setFlags"]["arguments"][0]["type"]["cpp_name"])
        self.assertIn("bitset", methods["getFlags"]["return_type"]["cpp_name"])

    def test_latex_formula_conversion(self):
        """Verify that LaTeX math formulas in Doxygen comments are converted to clean Unicode / HTML."""
        code = """
        namespace filament {
        namespace Exposure {
            /**
             * Returns the exposure value for luminance (in @f$ \\frac{cd}{m^2} @f$).
             * Formula: @f$ \\frac{1}{\\pi \\cdot K} @f$.
             */
            float ev100FromLuminance(float luminance) noexcept;
        }
        }
        """
        ex = self.parse_source(code)
        cls = next(c for c in ex.output["classes"] if c["name"] == "Exposure")
        method = next(m for m in cls["methods"] if m["name"] == "ev100FromLuminance")
        doc_brief = method["doc"]["brief"]
        doc_details = method["doc"]["details"]
        self.assertIn("cd / m<sup>2</sup>", doc_brief)
        self.assertIn("1 / (π · K)", doc_details)
        self.assertNotIn("@f$", doc_brief)
        self.assertNotIn("@f$", doc_details)
        self.assertNotIn("\\frac", doc_brief)
        self.assertNotIn("\\frac", doc_details)

    def test_doxygen_see_and_block_tags(self):
        """Verify that \\see, @see, \\sa, and block tags are extracted correctly into meta."""
        code = """
        namespace filament {
        class Foo {
        public:
            /**
             * Primary method documentation.
             *
             * @param x An integer parameter.
             * \\see Builder::geometry()
             * \\see otherMethod(int, float)
             * \\warning Do not call concurrently.
             */
            void doSomething(int x) noexcept;
        };
        }
        """
        ex = self.parse_source(code)
        cls = next(c for c in ex.output["classes"] if c["name"] == "Foo")
        method = next(m for m in cls["methods"] if m["name"] == "doSomething")
        doc = method["doc"]
        self.assertEqual(doc["brief"], "Primary method documentation.")
        self.assertEqual(doc["params"].get("x"), "An integer parameter.")
        self.assertIn("see", doc["meta"])
        self.assertEqual(doc["meta"]["see"], ["Builder::geometry()", "otherMethod(int, float)"])
        self.assertIn("warning", doc["meta"])
        self.assertEqual(doc["meta"]["warning"], "Do not call concurrently.")

    def test_multiline_comma_separated_see_tags(self):
        """Verify that multi-line comma-separated @see targets are cleanly parsed without parameter leakage."""
        code = """
        namespace filament {
        class Bar {
        public:
            /**
             * Enables or disables shadow mapping. Enabled by default.
             *
             * @param enabled true enables shadow mapping, false disables it.
             *
             * @see LightManager::Builder::castShadows(),
             *      RenderableManager::Builder::receiveShadows(),
             *      RenderableManager::Builder::castShadows(),
             */
            void setShadowingEnabled(bool enabled) noexcept;
        };
        }
        """
        ex = self.parse_source(code)
        cls = next(c for c in ex.output["classes"] if c["name"] == "Bar")
        method = next(m for m in cls["methods"] if m["name"] == "setShadowingEnabled")
        doc = method["doc"]
        self.assertEqual(doc["brief"], "Enables or disables shadow mapping.")
        self.assertEqual(doc["details"], "Enabled by default.")
        self.assertEqual(doc["params"].get("enabled"), "true enables shadow mapping, false disables it.")
        self.assertEqual(doc["meta"]["see"], [
            "LightManager::Builder::castShadows()",
            "RenderableManager::Builder::receiveShadows()",
            "RenderableManager::Builder::castShadows()"
        ])

    def test_directional_param_documentation(self):
        """Verify that @param[in], @param[out], and @param[in,out] are extracted correctly."""
        code = """
        #define UTILS_PUBLIC
        namespace filament {
        class UTILS_PUBLIC DirectionalDoc {
        public:
            /**
             * Directional parameter method.
             * @param[in] inVal Input value.
             * @param[out] outVal Output buffer.
             * @param[in,out] inOutVal In-out parameter.
             */
            void process(int inVal, int* outVal, int* inOutVal) noexcept;
        };
        }
        """
        ex = self.parse_source(code)
        cls = next(c for c in ex.output["classes"] if c["name"] == "DirectionalDoc")
        method = next(m for m in cls["methods"] if m["name"] == "process")
        doc = method["doc"]
        self.assertEqual(doc["params"].get("inVal"), "Input value.")
        self.assertEqual(doc["params"].get("outVal"), "Output buffer.")
        self.assertEqual(doc["params"].get("inOutVal"), "In-out parameter.")

    def test_namespace_function_extraction(self):
        """Verify that C++ namespaces containing functions are extracted as utility classes."""
        code = """
        namespace filament {
        namespace MathUtils {
            /**
             * Adds two numbers.
             */
            int add(int a, int b) noexcept;
        }
        }
        """
        ex = self.parse_source(code)
        cls = next(c for c in ex.output["classes"] if c["name"] == "MathUtils")
        self.assertEqual(cls["category"], "utility")
        self.assertTrue(cls.get("is_namespace"))
        self.assertTrue(cls.get("is_utility"))
        self.assertEqual(len(cls["methods"]), 1)
        self.assertTrue(cls["methods"][0]["is_static"])
        self.assertEqual(cls["methods"][0]["name"], "add")

    def test_alias_and_all_static_utility_extraction(self):
        """Verify that typedefs/using aliases and all-static classes are extracted properly."""
        code = """
        namespace filament {
            using LinearColor = float[3];
            typedef float LinearColorA[4];

            class Color {
            public:
                static float toLinear(float v);
                static float toSRGB(float v);
            };
        }
        """
        ex = self.parse_source(code)
        aliases = ex.output.get("aliases", [])
        self.assertTrue(any(a["name"] == "LinearColor" for a in aliases))
        self.assertTrue(any(a["name"] == "LinearColorA" for a in aliases))

        cls = next(c for c in ex.output["classes"] if c["name"] == "Color")
        self.assertEqual(cls["category"], "utility")
        self.assertTrue(cls.get("is_utility"))
        self.assertEqual(len(cls["methods"]), 2)
        self.assertTrue(all(m["is_static"] for m in cls["methods"]))

    def test_bitfield_auto_deduction(self):
        """Verify automatic deduction of bitfield structs and wrapper classes without annotations."""
        code = """
        struct DirectBitfield {
            int a : 1;
            int b : 1;
        };

        class WrapperBitfield {
        public:
            WrapperBitfield() noexcept;
        private:
            DirectBitfield mParams{};
        };

        struct SmallBitfield {
            short a : 4;
            short b : 4;
        };

        struct BigBitfield {
            long long a : 35;
            long long b : 10;
        };

        struct Exceeds64Bits {
            long long a : 40;
            long long b : 40;
        };
        """
        ex = self.parse_source(code)
        
        # 1. Direct bitfield struct
        direct = next(c for c in ex.output["classes"] if c["name"] == "DirectBitfield")
        self.assertEqual(direct["category"], "bitfield")
        self.assertEqual(direct["archetype"], "bitfield")
        self.assertEqual(direct["bitfield_primitive"], "int")
        self.assertFalse(direct.get("is_aggregate", False))
        self.assertTrue(direct["fields"][0]["is_bitfield"])
        self.assertEqual(direct["fields"][0]["bitfield_width"], 1)

        # 2. Wrapper bitfield class
        wrapper = next(c for c in ex.output["classes"] if c["name"] == "WrapperBitfield")
        self.assertEqual(wrapper["category"], "bitfield")
        self.assertEqual(wrapper["archetype"], "bitfield")
        self.assertEqual(wrapper["bitfield_primitive"], "int")

        # 3. Dynamic sizing tiers
        small = next(c for c in ex.output["classes"] if c["name"] == "SmallBitfield")
        self.assertEqual(small["bitfield_primitive"], "short")

        big = next(c for c in ex.output["classes"] if c["name"] == "BigBitfield")
        self.assertEqual(big["bitfield_primitive"], "long")

        # 4. Fallback for > 64 bits
        large = next(c for c in ex.output["classes"] if c["name"] == "Exceeds64Bits")
        self.assertNotEqual(large.get("archetype"), "bitfield")

    def test_apigen_namespaced_attributes(self):
        """Verify that apigen:skip and apigen:alternate_name annotations are properly handled."""
        code = """
        #define UTILS_NOAPIGEN [[clang::annotate("apigen:skip")]]
        #define UTILS_APIGEN_ALTERNATE_NAME(name) [[clang::annotate("apigen:alternate_name:" #name)]]

        class MyAnnotatedClass {
        public:
            UTILS_NOAPIGEN void skippedMethod();
            UTILS_APIGEN_ALTERNATE_NAME(renamedMethod) void normalMethod();
        };
        """
        ex = self.parse_source(code)
        cls = next(c for c in ex.output["classes"] if c["name"] == "MyAnnotatedClass")
        skipped = next(m for m in cls["methods"] if m["name"] == "skippedMethod")
        self.assertIn("apigen:skip", skipped["attributes"])

        normal = next(m for m in cls["methods"] if m["name"] == "normalMethod")
        self.assertIn("apigen:alternate_name:renamedMethod", normal["attributes"])

    def test_doxygen_list_formatting(self):
        """Verify that list items without prior double newlines are separated into details."""
        code = """
        class MyDocClass {
        public:
            /**
             * Creates a default sampler.
             * The default parameters are:
             * - filterMag : NEAREST
             * - filterMin : NEAREST
             */
            MyDocClass();
        };
        """
        ex = self.parse_source(code)
        cls = next(c for c in ex.output["classes"] if c["name"] == "MyDocClass")
        ctor = cls["methods"][0]
        self.assertEqual(ctor["doc"]["brief"], "Creates a default sampler.")
        self.assertIn("- filterMag : NEAREST", ctor["doc"]["details"])
        self.assertIn("- filterMin : NEAREST", ctor["doc"]["details"])

    def test_builder_extraction(self):
        """Verify that nested Builder classes are extracted, linked to parent, and copy/move ctors filtered."""
        code = """
        template<typename T> struct BuilderBase {};
        class Light {
            struct BuilderDetails;
        public:
            class Builder : public BuilderBase<BuilderDetails> {
            public:
                Builder() noexcept;
                Builder(Builder const& rhs) noexcept;
                Builder(Builder&& rhs) noexcept;
                ~Builder() noexcept;
                Builder& operator=(Builder const& rhs) noexcept;
                Builder& operator=(Builder&& rhs) noexcept;

                Builder& intensity(float val) noexcept;
                Light* build();
            };
        };
        """
        ex = self.parse_source(code)
        classes = {c["name"]: c for c in ex.output["classes"]}
        self.assertIn("Light", classes)
        self.assertIn("Builder", classes)

        builder = classes["Builder"]
        self.assertEqual(builder.get("parent_class"), "Light")
        self.assertTrue(builder.get("is_nested"))
        self.assertTrue(builder.get("is_builder"))
        self.assertEqual(builder.get("category"), "builder")
        self.assertEqual(builder.get("archetype"), "builder")

        # Check constructor filtering: only default constructor should remain (copy/move filtered)
        ctors = [m for m in builder["methods"] if m.get("is_constructor")]
        self.assertEqual(len(ctors), 1)
        self.assertEqual(len(ctors[0]["arguments"]), 0)

        # Check methods
        method_names = [m["name"] for m in builder["methods"] if not m.get("is_constructor")]
        self.assertIn("intensity", method_names)
        self.assertIn("build", method_names)
        self.assertFalse(any(m.startswith("operator") for m in method_names))

    def test_nested_aggregate_struct_extraction(self):
        """Verify that nested aggregate structs and multi-level sub-structs are properly extracted."""
        code = """
        class OuterManager {
        public:
            struct Options {
                struct NestedSub {
                    bool enabled = true;
                    float scale = 1.0f;
                };

                int count = 10;
                NestedSub sub;
            };

            void configure(const Options& options) noexcept;
        };
        """
        ex = self.parse_source(code)
        classes = {c["name"]: c for c in ex.output["classes"]}
        self.assertIn("OuterManager", classes)
        self.assertIn("Options", classes)
        self.assertIn("NestedSub", classes)

        opts = classes["Options"]
        self.assertEqual(opts.get("parent_class"), "OuterManager")
        self.assertTrue(opts.get("is_nested"))
        self.assertTrue(opts.get("is_aggregate"))

        sub = classes["NestedSub"]
        self.assertEqual(sub.get("parent_class"), "Options")
        self.assertTrue(sub.get("is_nested"))
        self.assertTrue(sub.get("is_aggregate"))

    def test_fixed_size_array_field_extraction(self):
        """Verify that fixed-size C array fields are extracted with correct array type signature."""
        code = """
        struct ArrayStruct {
            float positions[3];
            int indices[4];
        };
        """
        ex = self.parse_source(code)
        classes = {c["name"]: c for c in ex.output["classes"]}
        self.assertIn("ArrayStruct", classes)
        cls = classes["ArrayStruct"]
        self.assertTrue(cls.get("is_aggregate"))

        fields = {f["name"]: f for f in cls["fields"]}
        self.assertIn("positions", fields)
        self.assertIn("indices", fields)
        self.assertEqual(fields["positions"]["type"]["cpp_name"], "float[3]")
        self.assertEqual(fields["indices"]["type"]["cpp_name"], "int[4]")

    def test_valued_and_aliased_enum_extraction(self):
        """Verify that negative, non-sequential, and aliased enum constants are correctly extracted with values."""
        code = """
        class EnumContainer {
        public:
            enum class FrameStatus : int {
                SKIPPED_SPURIOUS = -2,
                SKIPPED_STALE = -1,
                ACCEPTED = 0
            };

            enum class AttachmentPoint : unsigned char {
                COLOR = 0,
                DEPTH = 1,
                COLOR0 = 0,
                COLOR1 = 3
            };
        };
        """
        ex = self.parse_source(code)
        classes = {c["name"]: c for c in ex.output["classes"]}
        self.assertIn("EnumContainer", classes)
        container = classes["EnumContainer"]
        enums = {e["name"]: e for e in container.get("enums", [])}
        self.assertIn("FrameStatus", enums)
        self.assertIn("AttachmentPoint", enums)

        fs_entries = {entry["name"]: entry["value"] for entry in enums["FrameStatus"]["entries"]}
        self.assertEqual(fs_entries["SKIPPED_SPURIOUS"], -2)
        self.assertEqual(fs_entries["SKIPPED_STALE"], -1)
        self.assertEqual(fs_entries["ACCEPTED"], 0)

        ap_entries = {entry["name"]: entry["value"] for entry in enums["AttachmentPoint"]["entries"]}
        self.assertEqual(ap_entries["COLOR"], 0)
        self.assertEqual(ap_entries["DEPTH"], 1)
        self.assertEqual(ap_entries["COLOR0"], 0)
        self.assertEqual(ap_entries["COLOR1"], 3)

    def test_base_struct_field_and_method_inheritance(self):
        """Verify that derived structs inherit fields and methods from non-polymorphic base structs."""
        code = """
        typedef int int32_t;
        typedef unsigned int uint32_t;
        namespace backend {
            struct Viewport {
                int32_t left;
                int32_t bottom;
                uint32_t width;
                uint32_t height;
                int32_t right() const noexcept { return left + int32_t(width); }
                int32_t top() const noexcept { return bottom + int32_t(height); }
            };
        }
        namespace filament {
            class Viewport : public backend::Viewport {
            public:
                Viewport() noexcept : backend::Viewport{} {}
                Viewport(int32_t left, int32_t bottom, uint32_t width, uint32_t height) noexcept
                    : backend::Viewport{left, bottom, width, height} {}
                bool empty() const noexcept { return !width || !height; }
            };
        }
        """
        ex = self.parse_source(code)
        classes = {c["name"]: c for c in ex.output["classes"]}
        self.assertIn("Viewport", classes)
        vp = classes["Viewport"]
        self.assertTrue(vp.get("is_aggregate"))
        self.assertEqual(vp.get("type", {}).get("category"), "struct")

        field_names = [f["name"] for f in vp.get("fields", [])]
        self.assertEqual(field_names, ["left", "bottom", "width", "height"])

        method_names = [m["name"] for m in vp.get("methods", [])]
        self.assertIn("Viewport", method_names)
        self.assertIn("empty", method_names)
        self.assertIn("right", method_names)
        self.assertIn("top", method_names)

    def test_pixel_buffer_descriptor_extraction(self):
        """Verify extraction of methods with backend::PixelBufferDescriptor&& move-reference arguments."""
        code = """
        typedef unsigned int uint32_t;
        namespace filament {
            class RenderTarget;
            namespace backend {
                class PixelBufferDescriptor;
            }
            class RendererTest {
            public:
                void readPixels(uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
                        backend::PixelBufferDescriptor&& buffer);
                void readPixels(RenderTarget* renderTarget,
                        uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
                        backend::PixelBufferDescriptor&& buffer);
            };
        }
        """
        ex = self.parse_source(code)
        classes = {c["name"]: c for c in ex.output["classes"]}
        self.assertIn("RendererTest", classes)
        methods = classes["RendererTest"]["methods"]
        self.assertEqual(len(methods), 2)
        for m in methods:
            self.assertEqual(m["name"], "readPixels")
            buf_arg = m["arguments"][-1]
            self.assertEqual(buf_arg["name"], "buffer")
            self.assertTrue(buf_arg["type"]["is_move_reference"])
            self.assertIn("PixelBufferDescriptor", buf_arg["type"]["qualified_name"])

    def test_referenced_classes_discovery(self):
        """Verify that referenced aggregate structs and external classes are discovered and populated in referenced_classes."""
        h_path = Path(__file__).parent / "java" / "21_external_aggregates.h"
        if not h_path.exists():
            return
        clang_args, all_includes = extractor.get_clang_args(str(h_path))
        index = extractor.clang_cindex.Index.create()
        tu = index.parse(str(h_path), args=clang_args, options=extractor.clang_cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
        ex = Extractor(str(h_path), include_paths=all_includes)
        ex.extract(tu.cursor)

        ref_classes = {c["name"]: c for c in ex.output.get("referenced_classes", [])}
        self.assertIn("Viewport", ref_classes)
        self.assertIn("Box", ref_classes)

        vp = ref_classes["Viewport"]
        self.assertTrue(vp.get("is_aggregate"))
        self.assertEqual([f["name"] for f in vp.get("fields", [])], ["left", "bottom", "width", "height"])

        box = ref_classes["Box"]
        self.assertTrue(box.get("is_aggregate"))
        self.assertEqual([f["name"] for f in box.get("fields", [])], ["center", "halfExtent"])

        methods = {m["name"]: m for m in ex.output["classes"][0]["methods"]}
        self.assertIn("getBoundingBox", methods)
        self.assertEqual(methods["getBoundingBox"]["return_type"]["cpp_name"], "Box")

    def test_lazy_header_discovery_for_forward_declarations(self):
        """Verify that forward-declared external types (like Viewport in Renderer.h) are lazily discovered from include search paths."""
        renderer_header = Path(__file__).parent.parent.parent.parent / "filament" / "include" / "filament" / "Renderer.h"
        if not renderer_header.exists():
            return
        clang_args, all_includes = extractor.get_clang_args(str(renderer_header))
        index = extractor.clang_cindex.Index.create()
        tu = index.parse(str(renderer_header), args=clang_args, options=extractor.clang_cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
        ex = Extractor(str(renderer_header), include_paths=all_includes)
        ex.extract(tu.cursor)

        ref_classes = {c["name"]: c for c in ex.output.get("referenced_classes", [])}
        self.assertIn("Viewport", ref_classes)
        vp = ref_classes["Viewport"]
        self.assertTrue(vp.get("is_aggregate"))
        self.assertEqual([f["name"] for f in vp.get("fields", [])], ["left", "bottom", "width", "height"])

    def test_texture_bindings_extraction(self):
        """Verify that texture features (aliases, builder mixins, alternate names, noapigen) are extracted."""
        h_path = Path(__file__).parent / "java" / "22_texture_bindings.h"
        if not h_path.exists():
            return
        clang_args, all_includes = extractor.get_clang_args(str(h_path))
        index = extractor.clang_cindex.Index.create()
        tu = index.parse(str(h_path), args=clang_args, options=extractor.clang_cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
        ex = Extractor(str(h_path), include_paths=all_includes)
        ex.extract(tu.cursor)

        classes = {c["name"]: c for c in ex.output["classes"]}
        self.assertIn("TextureBindingsTest", classes)
        cls_def = classes["TextureBindingsTest"]

        method_names = [m["name"] for m in cls_def["methods"]]
        self.assertIn("isTextureFormatSupported", method_names)
        self.assertIn("computeTextureDataSize", method_names)
        self.assertIn("getWidth", method_names)
        self.assertIn("setImage", method_names)
        self.assertIn("setExternalImage", method_names)
        self.assertIn("generateMipmaps", method_names)
        self.assertIn("isCreationComplete", method_names)

        # Ensure NOAPIGEN methods have the skip attribute
        set_image_async = next((m for m in cls_def["methods"] if m["name"] == "setImageAsync"), None)
        self.assertIsNotNone(set_image_async)
        self.assertIn("filament:apigen:skip", set_image_async["attributes"])

        # Check Builder
        builder = next((c for c in ex.output["classes"] if c["name"] == "Builder"), None)
        self.assertIsNotNone(builder)
        builder_methods = {m["name"]: m for m in builder["methods"]}
        self.assertIn("import", builder_methods)
        self.assertIn("filament:apigen:alternate_name:importTexture", builder_methods["import"]["attributes"])
        self.assertIn("usage", builder_methods)
        self.assertIn("swizzle", builder_methods)
        self.assertIn("async", builder_methods)
        self.assertIn("filament:apigen:skip", builder_methods["async"]["attributes"])

    def test_bone_buffer_bindings_extraction(self):
        """Verify that bone buffer methods with alternate names and size_param are extracted."""
        h_path = Path(__file__).parent / "java" / "23_bone_buffer_bindings.h"
        if not h_path.exists():
            return
        clang_args, all_includes = extractor.get_clang_args(str(h_path))
        index = extractor.clang_cindex.Index.create()
        tu = index.parse(str(h_path), args=clang_args, options=extractor.clang_cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
        ex = Extractor(str(h_path), include_paths=all_includes)
        ex.extract(tu.cursor)

        classes = {c["name"]: c for c in ex.output["classes"]}
        self.assertIn("BoneBufferBindingsTest", classes)
        cls_def = classes["BoneBufferBindingsTest"]

        set_bones_methods = [m for m in cls_def["methods"] if m["name"] == "setBones"]
        self.assertEqual(len(set_bones_methods), 2)

        # Check alternate names
        alt_names = {attr.split(":")[-1] for m in set_bones_methods for attr in m["attributes"] if "alternate_name" in attr}
        self.assertIn("setBonesAsQuaternions", alt_names)
        self.assertIn("setBonesAsMatrices", alt_names)

        # Check Slice on transforms argument
        for m in set_bones_methods:
            transforms_arg = next(a for a in m["arguments"] if a["name"] == "transforms")
            t_str = transforms_arg["type"].get("qualified_name") or transforms_arg["type"].get("cpp_name", "")
            self.assertIn("Slice", t_str)

    def test_trait_aliases_and_return_types(self):
        """Verify that trait aliases across classes and return-type templates are resolved."""
        code = """
        namespace std {
            template<bool B, class T = void> struct enable_if { using type = T; };
            template<bool B, class T = void> using enable_if_t = typename enable_if<B,T>::type;
            template<class A, class B> struct is_same { static constexpr bool value = false; };
            template<class A, class B> constexpr bool is_same_v = is_same<A,B>::value;
        }

        class Provider {
        public:
            template <typename T>
            using source_trait_t = std::enable_if_t<
                std::is_same_v<float, T> ||
                std::is_same_v<int, T> ||
                std::is_same_v<bool, T>
            >;
        };

        class Consumer {
        public:
            template <typename T>
            using alias_trait_t = Provider::source_trait_t<T>;

            template <typename T, typename = alias_trait_t<T>>
            void setValue(const char* name, T val);

            template <typename T, typename = alias_trait_t<T>>
            T getValue(const char* name) const;
        };
        """
        ex = self.parse_source(code)
        self.assertIn("source_trait_t", ex.traits)
        self.assertIn("alias_trait_t", ex.traits)
        self.assertEqual(len(ex.traits["alias_trait_t"]), 3)

        cls = next(c for c in ex.output["classes"] if c["name"] == "Consumer")
        set_val = next(m for m in cls["methods"] if m["name"] == "setValue")
        get_val = next(m for m in cls["methods"] if m["name"] == "getValue")

        self.assertEqual(len(set_val["specializations"]), 3)
        self.assertEqual(len(get_val["specializations"]), 3)

        val_arg = next(a for a in set_val["arguments"] if a["name"] == "val")
        self.assertTrue(val_arg["type"].get("is_template_param"))
        self.assertEqual(val_arg["type"].get("template_param_name"), "T")

        self.assertTrue(get_val["return_type"].get("is_template_param"))
        self.assertEqual(get_val["return_type"].get("template_param_name"), "T")

    def test_template_trait_specializations_extraction(self):
        """Verify that template trait specializations are extracted from 24_template_trait_specializations.h."""
        h_path = Path(__file__).parent / "java" / "24_template_trait_specializations.h"
        if not h_path.exists():
            return
        clang_args, all_includes = extractor.get_clang_args(str(h_path))
        index = extractor.clang_cindex.Index.create()
        tu = index.parse(str(h_path), args=clang_args, options=extractor.clang_cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
        ex = Extractor(str(h_path), include_paths=all_includes)
        ex.extract(tu.cursor)

        classes = {c["name"]: c for c in ex.output["classes"]}
        self.assertIn("TemplateTraitSpecializationsTest", classes)
        cls_def = classes["TemplateTraitSpecializationsTest"]

        methods = {m["name"]: m for m in cls_def["methods"]}
        self.assertIn("setParameter", methods)
        self.assertIn("getParameter", methods)

        set_param = methods["setParameter"]
        self.assertEqual(len(set_param["specializations"]), 4)
        spec_types = [s["T"] for s in set_param["specializations"]]
        self.assertEqual(spec_types, ["float", "int32_t", "bool", "math::float3"])

        get_param = methods["getParameter"]
        self.assertEqual(len(get_param["specializations"]), 4)
        self.assertTrue(get_param["return_type"].get("is_template_param"))
        self.assertEqual(get_param["return_type"].get("template_param_name"), "T")

        # Builder verification
        builder = next((c for c in ex.output["classes"] if c["name"] == "Builder"), None)
        self.assertIsNotNone(builder)
        builder_methods = {m["name"]: m for m in builder["methods"]}
        self.assertIn("parameter", builder_methods)
        self.assertEqual(len(builder_methods["parameter"]["specializations"]), 4)

    def test_tagged_array_buffers_extraction(self):
        """Verify that tagged array buffer attributes and specializations are extracted from 25_tagged_array_buffers.h."""
        h_path = Path(__file__).parent / "java" / "25_tagged_array_buffers.h"
        if not h_path.exists():
            return
        clang_args, all_includes = extractor.get_clang_args(str(h_path))
        index = extractor.clang_cindex.Index.create()
        tu = index.parse(str(h_path), args=clang_args, options=extractor.clang_cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
        ex = Extractor(str(h_path), include_paths=all_includes)
        ex.extract(tu.cursor)

        classes = {c["name"]: c for c in ex.output["classes"]}
        self.assertIn("TaggedArrayBuffersTest", classes)
        cls_def = classes["TaggedArrayBuffersTest"]

        methods = {m["name"]: m for m in cls_def["methods"]}
        self.assertIn("setBuffer", methods)

        set_buffer = methods["setBuffer"]
        self.assertEqual(len(set_buffer["arguments"]), 2)
        values_arg = next(a for a in set_buffer["arguments"] if a["name"] == "values")
        self.assertIn("filament:apigen:tagged_array", values_arg["attributes"])
        t_str = values_arg["type"].get("qualified_name") or values_arg["type"].get("cpp_name", "")
        self.assertIn("Slice", t_str)

        self.assertEqual(len(set_buffer["specializations"]), 8)
        spec_types = [s["T"] for s in set_buffer["specializations"]]
        self.assertEqual(spec_types, [
            "float",
            "math::float2",
            "math::float4",
            "math::mat4f",
            "int32_t",
            "math::int4",
            "bool",
            "math::bool4"
        ])

    def test_anonymous_union_field_extraction(self):
        """Verify that fields inside anonymous unions are extracted into the struct."""
        code = """
        #define UTILS_PUBLIC
        namespace filament {
        class UTILS_PUBLIC TestUnion {
        public:
            struct ParameterInfo {
                const char* name;
                bool isSampler;
                union {
                    int type;
                    float samplerType;
                };
                int count;
            };
        };
        }
        """
        ex = self.parse_source(code)
        classes = {c["name"]: c for c in ex.output["classes"]}
        self.assertIn("TestUnion", classes)
        self.assertIn("ParameterInfo", classes)
        pinfo = classes["ParameterInfo"]
        fields = {f["name"]: f for f in pinfo["fields"]}
        self.assertIn("name", fields)
        self.assertIn("isSampler", fields)
        self.assertIn("type", fields)
        self.assertIn("samplerType", fields)
        self.assertIn("count", fields)

    def test_pojo_options_struct_auto_deduction(self):
        """Verify that structs declared in Options.h headers are deduced as POJO structs."""
        options_code = """
        namespace filament {
        struct SuperResolutionOptions {
            float scale;
            bool enabled;
            struct SubParams {
                int mode;
            };
        };
        }
        """
        ex = self.parse_source(options_code, suffix="Options.h")
        classes = {c["name"]: c for c in ex.output["classes"]}
        self.assertIn("SuperResolutionOptions", classes)
        sro = classes["SuperResolutionOptions"]
        self.assertTrue(sro.get("is_pojo_struct"))
        self.assertEqual(sro.get("archetype"), "pojo_struct")

        # Non-options file should NOT be deduced as POJO
        standard_code = """
        namespace filament {
        struct RegularData {
            float x;
            float y;
        };
        }
        """
        ex_std = self.parse_source(standard_code, suffix="RegularData.h")
        classes_std = {c["name"]: c for c in ex_std.output["classes"]}
        self.assertIn("RegularData", classes_std)
        reg = classes_std["RegularData"]
        self.assertFalse(reg.get("is_pojo_struct", False))
        self.assertNotEqual(reg.get("archetype"), "pojo_struct")

    def test_doxygen_html_tag_preservation(self):
        """Verify that HTML tags like <p>, <b>, <code> are preserved and not stripped by doxygen parsing."""
        comment = """/**
         * <p>This structure can be used to specify the minimum scale factor.</p>
         */"""
        doc = extractor.parse_doxygen(comment)
        self.assertTrue(doc["brief"].startswith("<p>"))
        self.assertIn("<p>This structure can be used", doc["brief"])

        # Also test trailing member comment with //!<
        comment_trailing = "//!< <p>A member comment with paragraph tag.</p>"
        doc_trailing = extractor.parse_doxygen(comment_trailing)
        self.assertTrue(doc_trailing["brief"].startswith("<p>"))

if __name__ == '__main__':
    unittest.main()


