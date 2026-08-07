#!/usr/bin/env python3
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

"""Report public C++ APIs that the Java/JNI bindings expose but the JS bindings do not.

The web (embind) surface is maintained by hand, so it drifts from the C++ headers silently: a
method that is never registered simply does not exist in JavaScript, and nothing fails at build
time. This tool makes that drift visible.

How it works
------------
Three surfaces are parsed and diffed:

  C++    filament/include/filament/*.h and libs/gltfio/include/gltfio/*.h -- the API that exists.
  web    web/filament-js/{jsbindings,jsbindings_generated,jsenums,jsenums_generated}.cpp plus
         extensions.js -- the API JavaScript can actually reach. filament.d.ts is excluded on
         purpose; see collect_web().
  Java   android/**/src/main/java/.../*.java -- the API Android exposes.

A C++ method is reported when it is absent from the web surface *and* present on the Java one.
Requiring the Java side keeps the report actionable: it filters out APIs that no binding layer
has ever exposed, and it means acting on a finding never makes the web surface wider than
Android's.

Deliberate omissions live in EXCLUSIONS below, each with the reason. Anything not excluded and
not bound is reported, and the tool exits 1, so this can gate CI.

Usage
-----
    ./tools/web-binding-audit/run.py            # report unexplained gaps, exit 1 if any
    ./tools/web-binding-audit/run.py --all      # also list the deliberate exclusions
    ./tools/web-binding-audit/run.py --verbose  # include the C++ declaration for each gap

Known limitations
-----------------
Matching is by *name*, not by signature, so a method bound with fewer parameters than C++
declares (an overload gap) is reported as covered. setMorphWeights was such a case: bound, but
without its offset argument. Signature-level checking would need a real C++ parser; until then,
treat a clean run as "no missing names" rather than "no missing functionality".
"""

import argparse
import os
import re
import sys
from collections import defaultdict

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))

HEADER_DIRS = [
    ("filament/include/filament", ""),
    ("libs/gltfio/include/gltfio", "gltfio$"),
]

WEB_SOURCES = [
    "web/filament-js/jsbindings.cpp",
    "web/filament-js/jsbindings_generated.cpp",
    "web/filament-js/jsenums.cpp",
    "web/filament-js/jsenums_generated.cpp",
]

JAVA_DIRS = [
    "android/filament-android/src/main/java/com/google/android/filament",
    "android/gltfio-android/src/main/java/com/google/android/filament/gltfio",
    "android/filament-utils-android/src/main/java/com/google/android/filament/utils",
]

# Classes that are not part of the JS surface by design.
SKIPPED_CLASSES = {
    # CRTP/infrastructure helpers, not real API.
    "BuilderNameMixin", "FilamentAPI",
    # Math and geometry value types, bound as value_array/value_object rather than classes.
    "Aabb", "Aabb$Corners", "Box", "Viewport", "Color", "ColorSpace", "Gamut", "TransferFunction",
    # Iterator plumbing.
    "TransformManager$children_iterator", "TransformManager$children_range",
    # Frame pacing and history: driven by requestAnimationFrame on the web.
    "FrameHistoryStream", "FrameHistoryStream$NewFramesRange", "FrameHistoryStream$Result",
    "FramePacer", "FramePacer$Builder", "FramePipelineEstimator", "Sync",
    "gltfio$AssetConfigurationExtended",
}

# A C++ name that covers many overloads is usually bound as a family of typed entry points
# (setParameter -> setBoolParameter, setInt2Parameter, ...). Rather than hand-listing those names,
# derive the family from the *Java* bindings, which spell every overload out with concrete types:
#
#   public void setParameter(String name, int x, int y)  ->  an int2 overload must be reachable
#
# Each Java overload is turned into a type token (Int2) and checked against the web surface under
# the naming conventions the bindings actually use, infix (setInt2Parameter) or suffix
# (setParameterInt2). A family that covers only some overloads then reports the specific ones that
# are missing, instead of hiding behind a comment claiming the family is complete.
JAVA_TYPE_TOKENS = {
    "boolean": "Bool", "float": "Float", "int": "Int", "double": "Double", "long": "Long",
}

# Methods whose family should be derived, keyed by "Class.method". The value is the Java class file
# to read the overloads from (usually the same class).
DERIVED_FAMILIES = {
    "MaterialInstance.setParameter": "MaterialInstance",
    "MaterialInstance.setConstant": "MaterialInstance",
    "LightManager$Builder.intensity": "LightManager",
    "IndirectLight$Builder.irradiance": "IndirectLight",
}

# Overload shapes that the derivation cannot name on its own, because the web binding uses a name
# unrelated to the Java parameter types (textures, colour spaces, matrices).
IRREGULAR_FAMILY_MEMBERS = {
    "MaterialInstance.setParameter": ["setMat3Parameter", "setMat4Parameter",
                                      "setTextureParameter", "setColor3Parameter",
                                      "setColor4Parameter"],
    "LightManager$Builder.intensity": ["intensity", "intensityEnergy"],
    "IndirectLight$Builder.irradiance": ["irradianceTex", "irradianceSh"],
}


def java_overload_tokens(java_src, method):
    """Turn `method(String name, int x, int y)` declarations into type tokens like "Int2"."""
    tokens = set()
    pattern = re.compile(r"public\s+void\s+" + re.escape(method) + r"\s*\(([^)]*)\)")
    for params in pattern.findall(java_src):
        params = params.replace("@NonNull", " ").replace("@Nullable", " ")
        parts = [p.strip() for p in params.split(",") if p.strip()]
        # Drop the leading `String name` argument.
        if parts and parts[0].split()[0] == "String":
            parts = parts[1:]
        if not parts:
            continue
        types = [p.split()[0] for p in parts]
        if len(set(types)) != 1 or types[0] not in JAVA_TYPE_TOKENS:
            continue  # irregular shape; covered by IRREGULAR_FAMILY_MEMBERS
        token = JAVA_TYPE_TOKENS[types[0]]
        tokens.add(token + (str(len(types)) if len(types) > 1 else ""))
    return tokens


def candidate_names(method, token):
    """The naming conventions the bindings use: infix after the verb, or suffix."""
    for verb in ("set", "get", "is", "has"):
        if method.startswith(verb):
            rest = method[len(verb):]
            return {f"{verb}{token}{rest}", f"{method}{token}"}
    return {f"{method}{token}"}

# Methods intentionally left unbound, keyed by "Class.method" or "Class.*" -> reason.
# Each entry below was checked against the implementation, not just the header.
EXCLUSIONS = {
    # FFence::wait opens with FILAMENT_CHECK_PRECONDITION(UTILS_HAS_THREADING || timeout == 0),
    # so on the single-threaded wasm build any non-zero timeout aborts. FFence::waitAndDestroy has
    # no UTILS_HAS_THREADING guard of its own: it calls wait(mode, FENCE_WAIT_FOR_EVER) directly,
    # which trips that precondition. It is unusable on the web and has no timeout parameter to fix
    # it with; the bound Fence._wait with a timeout of 0 is the supported path.
    #
    # Not to be confused with FEngine::flushAndWait, which *does* guard on
    # `if constexpr (UTILS_HAS_THREADING)` and calls execute() instead of fencing. That one is safe
    # on the web and is bound.
    "Fence.waitAndDestroy": "calls Fence::wait with FENCE_WAIT_FOR_EVER, which requires threads",
    "Engine.isValid": "bound as the per-type isValidX overloads instead",
    "IndirectLight$Builder.irradiance": "bound as irradianceSh and irradianceTex",
    "IndirectLight$Builder.radiance": "bound as radianceSh",
    "gltfio$ResourceLoader.addTextureProvider": "bound as addStbProvider/addKtx2Provider/addWebpProvider",

    # Threading and engine lifecycle the browser owns.
    "Engine.setPaused": "render-thread pausing is meaningless without a render thread",
    "Engine.isPaused": "render-thread pausing is meaningless without a render thread",
    "Engine.getJobSystem": "returns utils::JobSystem&, an internal type with no JS representation",
    "Engine.compile": "CompilerPriorityQueue scheduling has no effect without a backend thread",
    "Engine.getFeatureFlag": "returns std::optional<bool>; needs a marshalling decision first",
    "Engine.setFeatureFlag": "paired with getFeatureFlag; experimental debug surface",
    "Engine.hasFeatureFlag": "paired with getFeatureFlag; experimental debug surface",
    "Engine$Builder.*": "the web constructs the Engine through Engine.create",

    # No web equivalent for external/native video streams or native window handles.
    "Stream.*": "no web equivalent for external/native video streams",
    "Stream$Builder.*": "no web equivalent for external/native video streams",
    "Texture.setExternalImage": "requires Stream",
    "Texture.setExternalStream": "requires Stream",
    "SwapChain.*": "native-window handles, frame callbacks, protected content and frame-rate "
                   "control have no web equivalent; the canvas owns presentation",

    # Material::Builder is not registered; the web creates materials through Engine.createMaterial,
    # whose options object already carries uboBatching. The other two Builder options are simply
    # not plumbed through that entry point yet.
    "Material$Builder.uboBatching": "already reachable via Engine.createMaterial's options object",
    "Material$Builder.*": "Material::Builder is not bound; extend Engine.createMaterial's options "
                          "object to reach these",
    "Material.compile": "CompilerPriorityQueue scheduling has no effect without a backend thread",
    "MaterialInstance.compile": "CompilerPriorityQueue scheduling has no effect without a backend thread",

    # Blocked on marshalling, not on cost. gltfio::MaterialKey is a bitfield-packed POD, and
    # embind's value_object binds members by pointer-to-member, which cannot be formed for a
    # bitfield. Needs per-field accessors or a plain-object converter first.
    "gltfio$MaterialProvider.*": "blocked on marshalling the bitfield-packed gltfio::MaterialKey",

    # Frame pacing and vsync, owned by the browser via requestAnimationFrame.
    "Renderer.setDisplayInfo": "the browser owns vsync scheduling",
    "Renderer.setFrameRateOptions": "the browser owns vsync scheduling",
    "Renderer.setFrameScheduleTime": "the browser owns vsync scheduling",
    "Renderer.getFrameInfoHistory": "returns FixedCapacityVector<FrameInfo>, an unbound type",
    "Renderer.getMaxFrameHistorySize": "paired with getFrameInfoHistory",
    "Renderer.hasGpuFallenBehind": "paired with getFrameInfoHistory",

    # Templated or callback-shaped APIs covered by a hand-written typed family instead.
    "Material.setDefaultParameter": "templated; would need one binding per parameter type",
    "Scene.forEach": "needs a JS callback marshalling layer",
}

# Method names that are never bound directly on any class.
SKIPPED_METHODS = {"getEngine", "operator", "begin", "end", "size", "empty", "data", "getName",
                   "name"}


def read(path):
    with open(os.path.join(ROOT, path), encoding="utf-8", errors="replace") as f:
        return f.read()


# --------------------------------------------------------------------------------------------
# C++ headers
# --------------------------------------------------------------------------------------------

DECL_RE = re.compile(
    r"^\s*(?!return|if|for|while|else|using|typedef|friend|static_assert)"
    r"(?:UTILS_PUBLIC\s+)?"
    r"(?:static\s+|virtual\s+|inline\s+|constexpr\s+)*"
    r"(?P<ret>[A-Za-z_][\w:<>,\s\*&\[\]]*?)\s+"
    r"(?P<name>[a-zA-Z_]\w*)\s*\(")
OPEN_RE = re.compile(r"^\s*(?:class|struct)\s+(?:UTILS_PUBLIC\s+)?(\w+)\b(?![^{;]*;)")
ACCESS_RE = re.compile(r"^\s*(public|protected|private)\s*:")


def parse_header(path, prefix):
    """Return {qualified class name: {method: declaration}} for public methods only."""
    out = defaultdict(dict)
    src = read(path)
    src = re.sub(r"/\*.*?\*/", "", src, flags=re.S)
    src = re.sub(r"//[^\n]*", "", src)

    stack, access, depth = [], ["public"], 0
    for line in src.splitlines():
        opened = OPEN_RE.match(line)
        if opened:
            stack.append((opened.group(1), depth))
            # `class` members default to private, `struct` members to public.
            access.append("private" if line.strip().startswith("class") else "public")
        access_change = ACCESS_RE.match(line)
        if access_change:
            access[-1] = access_change.group(1)
        if stack and access[-1] == "public":
            decl = DECL_RE.match(line)
            if decl and decl.group("name") not in ("operator", "if", "return"):
                cls = prefix + "$".join(name for name, _ in stack)
                out[cls][decl.group("name")] = line.strip()
        depth += line.count("{") - line.count("}")
        while stack and depth <= stack[-1][1]:
            stack.pop()
            access.pop()
    return out


def collect_headers():
    headers = defaultdict(dict)
    for rel, prefix in HEADER_DIRS:
        directory = os.path.join(ROOT, rel)
        for name in sorted(os.listdir(directory)):
            if name.endswith(".h"):
                for cls, methods in parse_header(os.path.join(rel, name), prefix).items():
                    headers[cls].update(methods)
    return headers


# --------------------------------------------------------------------------------------------
# Web (embind) surface
# --------------------------------------------------------------------------------------------

REGISTRATION_RE = re.compile(
    r'(?:class_|value_object|value_array|enum_)<[^;]*?>\s*\(\s*"([^"]+)"')
MEMBER_RE = re.compile(
    r'\.(?:function|class_function|property|field|value|element'
    r'|BUILDER_FUNCTION|EMBIND_LAMBDA|constructor)\s*\(\s*"([^"]+)"')


def collect_web():
    """Return {embind class name: {member names}}.

    Two conventions matter. Registrations chain across many lines, so the current class persists
    until the next registration opens. And entry points registered as `_foo` are re-exposed as
    `foo` by extensions.js, so the leading underscore is stripped.
    """
    web = defaultdict(set)
    for source in WEB_SOURCES:
        current = None
        for line in read(source).splitlines():
            opened = REGISTRATION_RE.search(line)
            if opened:
                current = opened.group(1)
                web.setdefault(current, set())
            for member in MEMBER_RE.finditer(line):
                if current:
                    web[current].add(member.group(1).lstrip("_"))

    extensions = read("web/filament-js/extensions.js")
    for cls, method in re.findall(
            r"Filament\.([A-Za-z0-9_$]+)\.prototype\.([A-Za-z0-9_]+)\s*=", extensions):
        web[cls].add(method)
    for cls, method in re.findall(
            r"Filament\.([A-Za-z0-9_$]+)\.([A-Za-z0-9_]+)\s*=\s*function", extensions):
        web[cls].add(method)

    # filament.d.ts is deliberately NOT consulted here. It is a declaration, not a registration:
    # a stale or aspirational entry there would mask a genuinely missing embind binding, which is
    # exactly how filament.d.ts came to declare setMorphWeights with a signature the bindings had
    # not exposed for some time. The authoritative web surface is the embind registrations plus
    # the extensions.js wrappers that re-expose them.
    return web


# --------------------------------------------------------------------------------------------
# Java surface
# --------------------------------------------------------------------------------------------

JAVA_METHOD_RE = re.compile(
    r"^[ \t]*public\s+(?:static\s+|final\s+|native\s+|synchronized\s+)*"
    r"[\w<>\[\],.@\s]+?[\s\]>]+(\w+)\s*\(", re.M)
JAVA_NATIVE_RE = re.compile(
    r"^[ \t]*(?:private|public)\s+static\s+native\s+.*?\bn(\w+)\s*\(", re.M)


def collect_java():
    """Return {java class name: {method names}}, one flat bucket per file.

    Nested Builders live in the same file as their outer class, so folding per file rather than
    per class keeps Builder methods reachable without modelling Java nesting.
    """
    java, sources = defaultdict(set), {}
    for rel in JAVA_DIRS:
        directory = os.path.join(ROOT, rel)
        if not os.path.isdir(directory):
            continue
        for dirpath, _, files in os.walk(directory):
            for name in sorted(files):
                if not name.endswith(".java"):
                    continue
                src = open(os.path.join(dirpath, name), encoding="utf-8",
                           errors="replace").read()
                found = set(JAVA_METHOD_RE.findall(src))
                found |= {n[0].lower() + n[1:] for n in JAVA_NATIVE_RE.findall(src)}
                java[name[:-5]] |= found
                sources[name[:-5]] = src
    return java, sources


# --------------------------------------------------------------------------------------------
# Diff
# --------------------------------------------------------------------------------------------

def excluded_reason(cls, method):
    return EXCLUSIONS.get(f"{cls}.{method}") or EXCLUSIONS.get(f"{cls}.*")


def audit():
    headers, web = collect_headers(), collect_web()
    java, java_sources = collect_java()
    gaps, excluded = [], []

    for cls in sorted(headers):
        if cls in SKIPPED_CLASSES:
            continue
        bound = set(web.get(cls, []))
        java_bucket = java.get(cls.replace("gltfio$", "").split("$")[0], set())

        for method, declaration in sorted(headers[cls].items()):
            if method in SKIPPED_METHODS or method == cls.split("$")[-1]:
                continue
            key = f"{cls}.{method}"
            if key not in DERIVED_FAMILIES:
                if method in bound:
                    continue
                # embind commonly renames getX/setX to the property "x".
                if method[:3] in ("get", "set") and (method[3:4].lower() + method[4:]) in bound:
                    continue
                # Only report what Android already exposes.
                if method not in java_bucket and \
                        ("n" + method[0].upper() + method[1:]) not in java_bucket:
                    continue
            if key in DERIVED_FAMILIES:
                java_src = java_sources.get(DERIVED_FAMILIES[key], "")
                tokens = java_overload_tokens(java_src, method)
                missing, present = [], []
                for token in sorted(tokens):
                    names = candidate_names(method, token)
                    (present if (names & bound) else missing).append(
                            sorted(names)[0])
                for name in IRREGULAR_FAMILY_MEMBERS.get(key, []):
                    (present if name in bound else missing).append(name)
                if missing:
                    gaps.append((cls, method, declaration,
                                 f"typed family incomplete ({len(present)} of "
                                 f"{len(present) + len(missing)} bound), missing: "
                                 + ", ".join(missing)))
                else:
                    excluded.append((cls, method, declaration,
                                     f"bound as a typed family of {len(present)}, derived from "
                                     f"the Java overloads"))
                continue

            reason = excluded_reason(cls, method)
            (excluded if reason else gaps).append((cls, method, declaration, reason))
    return gaps, excluded


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--all", action="store_true",
                        help="also list deliberate exclusions and their reasons")
    parser.add_argument("--verbose", action="store_true",
                        help="print the C++ declaration for each gap")
    args = parser.parse_args()

    gaps, excluded = audit()

    if gaps:
        print(f"Unbound on the web but present in the Java bindings ({len(gaps)}):\n")
        current = None
        for cls, method, declaration, detail in gaps:
            if cls != current:
                print(f"  {cls}")
                current = cls
            print(f"    {method}")
            if detail:
                print(f"        {detail}")
            if args.verbose:
                print(f"        {declaration}")
        print("\nBind these in web/filament-js/jsbindings.cpp and declare them in filament.d.ts,")
        print("or add them to EXCLUSIONS in this script with the reason they cannot be bound.")
    else:
        print("No unexplained gaps: every Java-exposed C++ method is reachable from JavaScript.")

    if args.all:
        print(f"\nDeliberately unbound ({len(excluded)}):\n")
        current = None
        for cls, method, _, reason in excluded:
            if cls != current:
                print(f"  {cls}")
                current = cls
            print(f"    {method:<34} {reason}")

    return 1 if gaps else 0


if __name__ == "__main__":
    sys.exit(main())
