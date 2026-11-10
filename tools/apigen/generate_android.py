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

"""Build orchestrator for generating Filament Android Java and JNI C++ bindings."""

import argparse
import concurrent.futures
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import threading

REPO_ROOT = Path(__file__).parent.parent.parent.resolve()
APIGEN_DIR = REPO_ROOT / "tools/apigen"

if str(APIGEN_DIR) not in sys.path:
    sys.path.insert(0, str(APIGEN_DIR))
if str(REPO_ROOT / "tools/reorganize-headers") not in sys.path:
    sys.path.insert(0, str(REPO_ROOT / "tools/reorganize-headers"))

from javagen.orchestrator import process_file
from javagen.validator import BindingValidator
import run as reorganize_tool

EXTRACTOR = APIGEN_DIR / "extractor.py"
JAVAGEN = APIGEN_DIR / "javagen.py"
REORGANIZE_HEADERS = REPO_ROOT / "tools/reorganize-headers/run.py"

JAVA_OUT_DIR = REPO_ROOT / "android/filament-android/src/main/java/com/google/android/filament"
CPP_OUT_DIR = REPO_ROOT / "android/filament-android/src/main/cpp"

FILAMAT_JAVA_OUT_DIR = REPO_ROOT / "android/filamat-android/src/main/java/com/google/android/filament/filamat"
FILAMAT_CPP_OUT_DIR = REPO_ROOT / "android/filamat-android/src/main/cpp"

INCLUDES = [
    "-I", str(REPO_ROOT),
    "-I", str(REPO_ROOT / "filament/include"),
    "-I", str(REPO_ROOT / "libs/utils/include"),
    "-I", str(REPO_ROOT / "libs/math/include"),
    "-I", str(REPO_ROOT / "filament/backend/include"),
    "-I", str(REPO_ROOT / "libs/filabridge/include"),
    "-I", str(REPO_ROOT / "libs/filaflat/include"),
    "-I", str(REPO_ROOT / "libs/filamat/include"),
]

FILAMAT_TARGETS = {
    "MaterialBuilder",
}

TARGETS = [
    ("ACESLegacyToneMapper", REPO_ROOT / "filament/include/filament/ToneMapper.h"),
    ("ACESToneMapper", REPO_ROOT / "filament/include/filament/ToneMapper.h"),
    ("AgxToneMapper", REPO_ROOT / "filament/include/filament/ToneMapper.h"),
    ("Box", REPO_ROOT / "filament/include/filament/Box.h"),
    ("BufferObject", REPO_ROOT / "filament/include/filament/BufferObject.h"),
    ("Camera", REPO_ROOT / "filament/include/filament/Camera.h"),
    ("ColorGrading", REPO_ROOT / "filament/include/filament/ColorGrading.h"),
    ("Colors", REPO_ROOT / "filament/include/filament/Color.h"),
    ("DisplayRangeToneMapper", REPO_ROOT / "filament/include/filament/ToneMapper.h"),
    ("EntityManager", REPO_ROOT / "libs/utils/include/utils/EntityManager.h"),
    ("Engine", REPO_ROOT / "filament/include/filament/Engine.h"),
    ("Exposure", REPO_ROOT / "filament/include/filament/Exposure.h"),
    ("Fence", REPO_ROOT / "filament/include/filament/Fence.h"),
    ("FilmicToneMapper", REPO_ROOT / "filament/include/filament/ToneMapper.h"),
    ("FramePacer", REPO_ROOT / "filament/include/filament/FramePacer.h"),
    ("Frustum", REPO_ROOT / "filament/include/filament/Frustum.h"),
    ("GT7ToneMapper", REPO_ROOT / "filament/include/filament/ToneMapper.h"),
    ("GenericToneMapper", REPO_ROOT / "filament/include/filament/ToneMapper.h"),
    ("IndexBuffer", REPO_ROOT / "filament/include/filament/IndexBuffer.h"),
    ("IndirectLight", REPO_ROOT / "filament/include/filament/IndirectLight.h"),
    ("InstanceBuffer", REPO_ROOT / "filament/include/filament/InstanceBuffer.h"),
    ("LightManager", REPO_ROOT / "filament/include/filament/LightManager.h"),
    ("LinearToneMapper", REPO_ROOT / "filament/include/filament/ToneMapper.h"),
    ("Material", REPO_ROOT / "filament/include/filament/Material.h"),
    ("MaterialBuilder", REPO_ROOT / "libs/filamat/include/filamat/MaterialBuilder.h"),
    ("MaterialInstance", REPO_ROOT / "filament/include/filament/MaterialInstance.h"),
    ("MorphTargetBuffer", REPO_ROOT / "filament/include/filament/MorphTargetBuffer.h"),
    ("PBRNeutralToneMapper", REPO_ROOT / "filament/include/filament/ToneMapper.h"),
    ("RenderableManager", REPO_ROOT / "filament/include/filament/RenderableManager.h"),
    ("RenderTarget", REPO_ROOT / "filament/include/filament/RenderTarget.h"),
    ("Renderer", REPO_ROOT / "filament/include/filament/Renderer.h"),
    ("Scene", REPO_ROOT / "filament/include/filament/Scene.h"),
    ("SkinningBuffer", REPO_ROOT / "filament/include/filament/SkinningBuffer.h"),
    ("Skybox", REPO_ROOT / "filament/include/filament/Skybox.h"),
    ("Stream", REPO_ROOT / "filament/include/filament/Stream.h"),
    ("SwapChain", REPO_ROOT / "filament/include/filament/SwapChain.h"),
    ("Texture", REPO_ROOT / "filament/include/filament/Texture.h"),
    ("TextureSampler", REPO_ROOT / "filament/include/filament/TextureSampler.h"),
    ("ToneMapper", REPO_ROOT / "filament/include/filament/ToneMapper.h"),
    ("TransformManager", REPO_ROOT / "filament/include/filament/TransformManager.h"),
    ("VertexBuffer", REPO_ROOT / "filament/include/filament/VertexBuffer.h"),
    ("View", REPO_ROOT / "filament/include/filament/View.h"),
    ("Viewport", REPO_ROOT / "filament/include/filament/Viewport.h"),
]

# Synchronize in-process JavaGen invocations when sharing global KNOWN_CLASSES and _SIBLING_IR_CACHE
_GEN_LOCK = threading.Lock()


def extract_header(header_path: Path):
    """Run extractor.py on a single C++ header and return its JSON IR string."""
    if not header_path.exists():
        return False, header_path, f"Header not found: {header_path}"

    res = subprocess.run(
        [sys.executable, str(EXTRACTOR)] + INCLUDES + [str(header_path)],
        capture_output=True,
        text=True,
        cwd=str(REPO_ROOT),
    )
    if res.returncode != 0:
        return False, header_path, f"Extractor failed for {header_path.name}:\n{res.stderr}"

    return True, header_path, res.stdout


def ensure_reorganize_initialized() -> None:
    """Precompute library dependency ordering once for header reorganization."""
    if not reorganize_tool.SORTED_LIBS:
        deps = reorganize_tool.analyze_dependencies(str(REPO_ROOT))
        reorganize_tool.SORTED_LIBS = reorganize_tool.compute_topological_sort(deps)


def reorganize_cpp_file(cpp_path: Path):
    """Format and layer C++ #include directives in the generated JNI source file."""
    try:
        ensure_reorganize_initialized()
        reorganize_tool.reorganize_file(str(cpp_path))
        return True, None
    except Exception as exc:
        return False, f"Header reorganization failed for {cpp_path.name}:\n{exc}"


def generate_target(item, java_out_dir: Path, cpp_out_dir: Path):
    """Generate .java and .cpp bindings for a single target class in-process."""
    name, json_file = item
    try:
        with _GEN_LOCK:
            process_file(
                str(json_file),
                str(java_out_dir),
                str(cpp_out_dir),
                class_name=name,
            )
    except Exception as exc:
        return False, name, f"JavaGen failed for {name}:\n{exc}"

    cpp_file = cpp_out_dir / f"{name}.cpp"
    if not cpp_file.exists():
        for f in cpp_out_dir.glob("*.cpp"):
            if f.stem.lower() == name.lower():
                cpp_file = f
                break

    if cpp_file.exists():
        ok, err = reorganize_cpp_file(cpp_file)
        if not ok:
            return False, name, err

    return True, name, None


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate Android Java and JNI C++ bindings for Filament."
    )
    parser.add_argument(
        "-j",
        "--jobs",
        type=int,
        nargs="?",
        const=os.cpu_count() or 4,
        default=os.cpu_count() or 4,
        help="Number of parallel worker threads (default: %(default)s)",
    )
    parser.add_argument(
        "--java-dir",
        type=Path,
        default=JAVA_OUT_DIR,
        help="Target directory for generated Java sources (default: %(default)s)",
    )
    parser.add_argument(
        "--jni-dir",
        type=Path,
        default=CPP_OUT_DIR,
        help="Target directory for generated JNI C++ sources (default: %(default)s)",
    )
    parser.add_argument(
        "--validate",
        action="store_true",
        help="Run static parity and reflection validation after code generation",
    )
    parser.add_argument(
        "targets",
        nargs="*",
        help="Optional list of specific class names to generate (default: all targets)",
    )

    args = parser.parse_args()

    java_out_dir = args.java_dir.resolve()
    cpp_out_dir = args.jni_dir.resolve()
    use_default_dirs = (java_out_dir == JAVA_OUT_DIR.resolve() and cpp_out_dir == CPP_OUT_DIR.resolve())
    jobs = max(1, args.jobs)

    if not REORGANIZE_HEADERS.exists():
        print(
            f"Error: Header reorganization tool not found at {REORGANIZE_HEADERS}",
            file=sys.stderr,
        )
        sys.exit(1)

    selected_targets = TARGETS
    if args.targets:
        target_map = {t[0]: t for t in TARGETS}
        unknown = [t for t in args.targets if t not in target_map]
        if unknown:
            print(f"Error: Unknown target(s): {', '.join(unknown)}", file=sys.stderr)
            print(
                f"Available targets: {', '.join(sorted(target_map.keys()))}",
                file=sys.stderr,
            )
            sys.exit(1)
        selected_targets = [target_map[t] for t in args.targets]

    def get_target_dirs(name: str):
        if use_default_dirs and name in FILAMAT_TARGETS:
            return FILAMAT_JAVA_OUT_DIR, FILAMAT_CPP_OUT_DIR
        return java_out_dir, cpp_out_dir

    print(f"Generating Android bindings ({len(selected_targets)} targets, {jobs} workers)...")
    print(f"  Java output dir: {java_out_dir}")
    print(f"  JNI C++ output dir: {cpp_out_dir}")
    if use_default_dirs and any(t[0] in FILAMAT_TARGETS for t in selected_targets):
        print(f"  Filamat Java output dir: {FILAMAT_JAVA_OUT_DIR}")
        print(f"  Filamat JNI C++ output dir: {FILAMAT_CPP_OUT_DIR}")

    for name, _ in selected_targets:
        t_java_dir, t_cpp_dir = get_target_dirs(name)
        t_java_dir.mkdir(parents=True, exist_ok=True)
        t_cpp_dir.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory() as tmp_dir:
        json_files = []
        unique_headers = list(dict.fromkeys(h for _, h in selected_targets))
        print(f"\n[1/2] Extracting C++ APIs to IR ({len(unique_headers)} unique headers)...")
        header_to_json = {}
        with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as executor:
            futures = {executor.submit(extract_header, h): h for h in unique_headers}
            for future in concurrent.futures.as_completed(futures):
                ok, header_path, result = future.result()
                if not ok:
                    print(f"Error: {result}", file=sys.stderr)
                    sys.exit(1)
                print(f"  Extracted {header_path.name}")
                header_to_json[header_path] = result

        for name, header_path in selected_targets:
            json_file = Path(tmp_dir) / f"{name}.json"
            json_file.write_text(header_to_json[header_path])
            json_files.append((name, json_file))

        # Precompute library dependency ordering once for header reorganization
        ensure_reorganize_initialized()

        print("\n[2/2] Generating Java and JNI C++ bindings...")
        for item in json_files:
            t_java_dir, t_cpp_dir = get_target_dirs(item[0])
            ok, name, err = generate_target(item, t_java_dir, t_cpp_dir)
            if not ok:
                print(f"Error: {err}", file=sys.stderr)
                sys.exit(1)
            print(f"  ✓ Generated {name}.java and {name}.cpp")

    # Clean up non-target helper files generated for nested classes or iterators
    target_names = {t[0] for t in TARGETS}
    helper_stems = {
        "children_iterator",
        "children_range",
        "children_sentinel",
        "Corners",
        "Aabb",
    }
    for out_j, out_c in {(java_out_dir, cpp_out_dir), (FILAMAT_JAVA_OUT_DIR, FILAMAT_CPP_OUT_DIR)}:
        if out_j.exists():
            for item in out_j.glob("*.java"):
                if item.stem not in target_names and item.stem in helper_stems:
                    item.unlink()
        if out_c.exists():
            for item in out_c.glob("*.cpp"):
                if item.stem not in target_names and item.stem in helper_stems:
                    item.unlink()

    out_display = (
        java_out_dir.relative_to(REPO_ROOT)
        if java_out_dir.is_relative_to(REPO_ROOT)
        else java_out_dir
    )
    print(f"\nAll {len(selected_targets)} target bindings successfully generated into {out_display}!")

    if args.validate:
        print("\n[Validation] Running static parity and reflection validation...")
        validation_groups = []
        if use_default_dirs:
            filament_targets = {t[0] for t in selected_targets if t[0] not in FILAMAT_TARGETS}
            filamat_targets = {t[0] for t in selected_targets if t[0] in FILAMAT_TARGETS}
            if filament_targets:
                validation_groups.append((
                    java_out_dir,
                    cpp_out_dir,
                    filament_targets if args.targets else None,
                ))
            if filamat_targets:
                validation_groups.append((
                    FILAMAT_JAVA_OUT_DIR,
                    FILAMAT_CPP_OUT_DIR,
                    filamat_targets if args.targets else None,
                ))
        else:
            target_set = {t[0] for t in selected_targets} if args.targets else None
            validation_groups.append((java_out_dir, cpp_out_dir, target_set))

        all_clean = True
        for val_java_dir, val_cpp_dir, val_targets in validation_groups:
            validator = BindingValidator(
                java_dir=val_java_dir,
                cpp_dir=val_cpp_dir,
                verbose=False,
                target_classes=val_targets,
            )
            val_result = validator.run()
            print(val_result.format_report(verbose=False))
            if not val_result.is_clean:
                all_clean = False
        if not all_clean:
            print("\nError: Static validation failed!", file=sys.stderr)
            sys.exit(1)
        print("✓ Static validation passed with zero errors.")


if __name__ == "__main__":
    main()