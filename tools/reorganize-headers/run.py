#!/usr/bin/env python3
import os
import sys
import re

# Sets of standard headers for classification
CPP_STD_HEADERS = {
    "algorithm", "array", "atomic", "bitset", "chrono", "codecvt", "complex",
    "condition_variable", "deque", "exception", "execution", "filesystem",
    "forward_list", "fstream", "functional", "future", "initializer_list",
    "iomanip", "ios", "iosfwd", "iostream", "istream", "iterator", "limits",
    "list", "locale", "map", "memory", "mutex", "new", "numeric", "ostream",
    "queue", "random", "ratio", "regex", "scoped_allocator", "set",
    "shared_mutex", "sstream", "stack", "stdexcept", "streambuf", "string",
    "string_view", "system_error", "thread", "tuple", "type_traits",
    "typeindex", "typeinfo", "unordered_map", "unordered_set", "utility",
    "valarray", "vector", "variant", "optional", "any", "compare", "version",
    "barrier", "latch", "semaphore", "span", "format", "numbers", "ranges",
    "coroutine", "source_location",
    "cassert", "ccomplex", "cctype", "cerrno", "cfenv", "cfloat",
    "cinttypes", "ciso646", "climits", "clocale", "cmath", "csetjmp",
    "csignal", "cstdalign", "cstdarg", "cstdatomic", "cstdbool",
    "cstddef", "cstdint", "cstdio", "cstdlib", "cstdnoreturn",
    "cstring", "ctgmath", "cthreads", "ctime", "cuchar", "cwchar",
    "cwtype", "cwctype"
}

C_STD_HEADERS = {
    "assert.h", "complex.h", "ctype.h", "errno.h", "fenv.h", "float.h",
    "inttypes.h", "iso646.h", "limits.h", "locale.h", "math.h", "setjmp.h",
    "signal.h", "stdalign.h", "stdarg.h", "stdatomic.h", "stdbool.h",
    "stddef.h", "stdint.h", "stdio.h", "stdlib.h", "stdnoreturn.h",
    "string.h", "tgmath.h", "threads.h", "time.h", "uchar.h", "wchar.h",
    "wctype.h"
}

POSIX_HEADERS = {
    "unistd.h", "dirent.h", "fcntl.h", "pthread.h", "dlfcn.h", "semaphore.h",
    "sched.h", "alloca.h"
}

# Global dynamically computed internal targets
SORTED_LIBS = []
LAYER_PRIVATE_LOCAL = 2

def analyze_dependencies(workspace_root):
    # Discover all subdirectory names under libs/ plus filament/ and backend/
    internal_libs = {"filament", "backend"}
    libs_path = os.path.join(workspace_root, "libs")
    if os.path.exists(libs_path):
        for name in os.listdir(libs_path):
            if os.path.isdir(os.path.join(libs_path, name)):
                internal_libs.add(name)

    dependencies = {lib: set() for lib in internal_libs}

    # Scan all CMakeLists.txt files for target_link_libraries dependencies
    for root, _, files in os.walk(workspace_root):
        if "CMakeLists.txt" in files:
            cmakelists_path = os.path.join(root, "CMakeLists.txt")
            with open(cmakelists_path, 'r', encoding='utf-8') as f:
                content = f.read()

            # Resolve TARGET variable if defined: set(TARGET <name>)
            target_var = None
            m_target = re.search(r'set\s*\(\s*TARGET\s+([a-zA-Z0-9_\-]+)\s*\)', content, re.IGNORECASE)
            if m_target:
                target_var = m_target.group(1)

            # Find all target_link_libraries calls
            links = re.findall(r'target_link_libraries\s*\(\s*([a-zA-Z0-9_\-\$\{\}]+)\s+(.*?)\)', content, re.DOTALL | re.IGNORECASE)
            for target, deps_str in links:
                resolved_target = target
                if target in {"${TARGET}", "${TARGET_NAME}"}:
                    resolved_target = target_var
                
                if resolved_target not in internal_libs:
                    continue

                tokens = re.split(r'\s+', deps_str)
                for token in tokens:
                    token = token.strip()
                    if not token or token in {"PRIVATE", "PUBLIC", "INTERFACE"}:
                        continue
                    if token in internal_libs:
                        dependencies[resolved_target].add(token)

    return dependencies

def compute_topological_sort(dependencies):
    visited = {}
    result = []

    def dfs(node):
        if visited.get(node, 0) == 1:
            return
        if visited.get(node, 0) == 2:
            return

        visited[node] = 1
        for neighbor in sorted(dependencies.get(node, set())):
            dfs(neighbor)
        visited[node] = 2
        result.append(node)

    for node in sorted(dependencies.keys()):
        dfs(node)

    # We want most dependent first (top of file)
    return result[::-1]

def get_layer(include_path, is_quotes, file_basename):
    LAYER_CORRESPONDING = 0
    LAYER_UNWINDOWS     = 1

    # Rule 3: Corresponding header first
    include_basename = os.path.splitext(os.path.basename(include_path))[0]
    if is_quotes and include_basename == file_basename:
        return LAYER_CORRESPONDING

    # Special Rule: unwindows.h always second
    if include_path == "utils/unwindows.h":
        return LAYER_UNWINDOWS

    if is_quotes:
        return LAYER_PRIVATE_LOCAL

    # Angle brackets includes - Classify based on path components
    parts = include_path.split('/')
    is_private = False
    component = ""

    if len(parts) > 1 and parts[0] == "private":
        is_private = True
        component = parts[1]
    elif len(parts) > 0:
        component = parts[0]

    # If it's one of our internal library directories, dynamically calculate layer based on topological sort
    if component in SORTED_LIBS:
        idx = SORTED_LIBS.index(component)
        return (3 + 2 * idx) if is_private else (4 + 2 * idx)
    
    # Standard, POSIX and Third-party layers sit at the very bottom
    base_offset = 3 + 2 * len(SORTED_LIBS)
    LAYER_THIRD_PARTY = base_offset
    LAYER_CPP_STD     = base_offset + 1
    LAYER_POSIX_SYS   = base_offset + 2
    LAYER_C_STD       = base_offset + 3

    # Check standard libraries
    if include_path in CPP_STD_HEADERS:
        return LAYER_CPP_STD
    
    # Check POSIX system libraries
    if include_path in POSIX_HEADERS or include_path.startswith("sys/"):
        return LAYER_POSIX_SYS
        
    if include_path in C_STD_HEADERS:
        return LAYER_C_STD

    # Default for any other < > include is third-party
    return LAYER_THIRD_PARTY

def parse_include_line(line, file_basename, guard_open=None, guard_close=None, filepath=None):
    m = re.match(r'^\s*#\s*include\s+(["<])([^">]+)([">])', line)
    if not m:
        return None
    
    is_quotes = m.group(1) == '"'
    include_path = m.group(2)

    # Find the local module's src/ directory
    src_root = None
    if filepath:
        # Matches everything up to and including "src/" (e.g. "filament/src/")
        m_src = re.match(r'(.*?/src)/', filepath)
        if m_src:
            src_root = m_src.group(1)

    # Auto-fix: If a bracketed include actually exists under the local src/ folder, force it to quotes
    if not is_quotes and src_root:
        local_path = os.path.join(src_root, include_path)
        if os.path.exists(local_path):
            is_quotes = True
            line = f'#include "{include_path}"'

    # Auto-fix: If a quoted include actually belongs to another library/layer (including private/ API), convert it to angle brackets
    if is_quotes:
        parts = include_path.split('/')
        is_internal = False
        if len(parts) > 1:
            if parts[0] in SORTED_LIBS:
                is_internal = True
            elif parts[0] == "private" and len(parts) > 2 and parts[1] in SORTED_LIBS:
                is_internal = True
        if is_internal:
            is_quotes = False
            line = f'#include <{include_path}>'

    layer = get_layer(include_path, is_quotes, file_basename)
    
    formatted_line = line.strip()
    if guard_open and guard_close:
        formatted_line = f"{guard_open}\n{formatted_line}\n{guard_close}"

    return {
        "line": formatted_line,
        "path": include_path,
        "layer": layer,
        "is_quotes": is_quotes
    }

def reorganize_file(filepath, dry_run=False):
    file_basename = os.path.splitext(os.path.basename(filepath))[0]

    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    lines = content.splitlines()
    
    first_include_idx = -1
    last_include_idx = -1
    
    include_lines_info = []
    guarded_lines_set = set()

    idx = 0
    while idx < len(lines):
        stripped = lines[idx].strip()
        
        # Check for simple guarded include block of size 3:
        # lines[idx]:   #if / #ifdef / #ifndef
        # lines[idx+1]: #include ...
        # lines[idx+2]: #endif
        if (stripped.startswith("#if") or stripped.startswith("#ifdef") or stripped.startswith("#ifndef")) and idx + 2 < len(lines):
            next_stripped = lines[idx + 1].strip()
            next_next_stripped = lines[idx + 2].strip()
            
            if (next_stripped.startswith("#include") or next_stripped.startswith("#  include")) and next_next_stripped == "#endif":
                guard_open = stripped
                guard_close = next_next_stripped
                
                if first_include_idx == -1:
                    first_include_idx = idx
                last_include_idx = idx + 2
                
                info = parse_include_line(lines[idx + 1], file_basename, guard_open, guard_close, filepath=filepath)
                if info:
                    include_lines_info.append((idx + 1, info))
                    
                guarded_lines_set.update([idx, idx + 1, idx + 2])
                idx += 3
                continue

        is_include = stripped.startswith("#include") or stripped.startswith("#  include")
        if is_include:
            if first_include_idx == -1:
                first_include_idx = idx
            last_include_idx = idx
            
            info = parse_include_line(lines[idx], file_basename, filepath=filepath)
            if info:
                include_lines_info.append((idx, info))
        
        idx += 1
            
    if first_include_idx == -1:
        return False

    # Check for safety: preprocessor conditionals or any other non-include, non-comment, non-empty lines
    has_invalid_lines = False
    for idx in range(first_include_idx, last_include_idx + 1):
        if idx in guarded_lines_set:
            continue
            
        stripped = lines[idx].strip()
        if not stripped:
            continue
        
        # Allow comments
        if stripped.startswith("//") or stripped.startswith("/*") or stripped.startswith("*/") or stripped.startswith("*"):
            continue
            
        # Check for conditionals
        if stripped.startswith("#if") or stripped.startswith("#else") or stripped.startswith("#elif") or stripped.startswith("#endif"):
            has_invalid_lines = True
            print(f"Skipping {filepath}: contains preprocessor conditionals inside include block.")
            break
            
        # Check for other preprocessor directives or code
        if not (stripped.startswith("#include") or stripped.startswith("#  include")):
            has_invalid_lines = True
            print(f"Skipping {filepath}: contains code or macros ({stripped}) inside include block.")
            break

    if has_invalid_lines:
        return False

    # Group include lines by layer and deduplicate globally (Total layers: 3 base layers + 2 per library + 4 standard/3rd-party/POSIX layers)
    total_layers = 7 + 2 * len(SORTED_LIBS)
    layers_dict = {i: [] for i in range(total_layers)}
    seen_paths = set()
    for idx, info in include_lines_info:
        path_lower = info["path"].lower()
        if path_lower in seen_paths:
            continue
        seen_paths.add(path_lower)
        layers_dict[info["layer"]].append(info)

    # Sort within each layer alphabetically (flat-first sorting applies only to private includes)
    for layer in layers_dict:
        if layer == LAYER_PRIVATE_LOCAL:
            layers_dict[layer].sort(key=lambda x: (len(x["path"].split('/')) > 1, x["path"].lower()))
        else:
            layers_dict[layer].sort(key=lambda x: x["path"].lower())

    # Generate the new include block text
    new_block_lines = []
    
    first_layer_added = False
    for layer in range(total_layers):
        headers = layers_dict[layer]
        if not headers:
            continue
            
        if first_layer_added:
            new_block_lines.append("")
        
        if layer == LAYER_PRIVATE_LOCAL:
            # Group private includes by their first path component / sub-directory
            prev_prefix = None
            for h in headers:
                parts = h["path"].split('/')
                current_prefix = parts[0] if len(parts) > 1 else ""
                
                if prev_prefix is not None and current_prefix != prev_prefix:
                    new_block_lines.append("")
                    
                new_block_lines.append(h["line"])
                prev_prefix = current_prefix
        else:
            # Standard sequential write for all other layers
            for h in headers:
                new_block_lines.append(h["line"])
            
        first_layer_added = True

    new_lines = lines[:first_include_idx] + new_block_lines + lines[last_include_idx + 1:]
    new_content = "\n".join(new_lines) + ("\n" if content.endswith("\n") else "")

    if content == new_content:
        return False

    if not dry_run:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(new_content)
            
    return True

if __name__ == "__main__":
    # Setup workspace root and build the topological dependency list dynamically
    script_dir = os.path.dirname(os.path.abspath(__file__))
    workspace = os.path.dirname(os.path.dirname(script_dir))
    
    try:
        deps = analyze_dependencies(workspace)
        SORTED_LIBS = compute_topological_sort(deps)
    except Exception as e:
        print(f"Error during dependency tree analysis: {e}")
        sys.exit(1)

    args = sys.argv[1:]

    if not args or "-h" in args or "--help" in args:
        print("Filament C++ Header Reorganization Tool")
        print("Sorts and formats include blocks in accordance with the dynamic dependency tree.")
        print()
        print("Usage:")
        print("  ./tools/reorganize_headers.py [options] <file_or_directory>")
        print()
        print("Options:")
        print("  -h, --help    Show this help message and exit")
        print("  --dry-run     Show what files would be modified without writing changes")
        print()
        print("Testing the script:")
        print("  python3 test/code-correctness/test_reorganize_headers.py")
        sys.exit(0)

    dry_run = False
    if "--dry-run" in args:
        dry_run = True
        args.remove("--dry-run")

    if not args:
        print("Error: Missing target file or directory. Use --help for usage information.")
        sys.exit(1)

    target = args[0]
    if os.path.isfile(target):
        changed = reorganize_file(target, dry_run)
        if changed:
            print(f"{'Would reorganize' if dry_run else 'Reorganized'} {target}")
        else:
            print(f"No changes needed for {target}")
    elif os.path.isdir(target):
        count = 0
        for root, _, files in os.walk(target):
            for file in files:
                if file.endswith(('.cpp', '.h', '.c', '.hpp')):
                    filepath = os.path.join(root, file)
                    # Skip certain external, third_party or test directories
                    if "third_party" in filepath or "libs/bluegl" in filepath or "libs/bluevk" in filepath or "filament/test" in filepath or "libs/utils/test" in filepath:
                        continue
                    try:
                        changed = reorganize_file(filepath, dry_run)
                        if changed:
                            print(f"{'Would reorganize' if dry_run else 'Reorganized'} {filepath}")
                            count += 1
                    except Exception as e:
                        print(f"Error processing {filepath}: {e}")
        print(f"Total files modified: {count}")
    else:
        print(f"Error: {target} is not a valid file or directory")
        sys.exit(1)
