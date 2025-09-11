import re
import sys
import os

def process_file(filepath, replacements):
    """Reads a file, applies a series of regex replacements, and writes it back."""
    try:
        # Create a backup before modifying
        backup_path = filepath + '.bak'
        with open(filepath, 'r') as f_in, open(backup_path, 'w') as f_out:
            content = f_in.read()
            f_out.write(content)
    except FileNotFoundError:
        print(f"Error: File not found at {filepath}", file=sys.stderr)
        sys.exit(1)

    # Apply all replacements
    for pattern, replacement in replacements:
        content = re.sub(pattern, replacement, content, flags=re.MULTILINE)

    # Write the modified content back to the original file
    with open(filepath, 'w') as f:
        f.write(content)

def main():
    """Main function to define and process all file changes."""
    # Make the script location-aware. It's in third_party/spirv-tools/tnt.
    # The files to patch are in third_party/spirv-tools.
    script_dir = os.path.dirname(os.path.abspath(__file__))
    spirv_tools_dir = os.path.abspath(os.path.join(script_dir, '..'))

    # Define replacements for third_party/spirv-tools/CMakeLists.txt
    spirv_tools_cmakelist = os.path.join(spirv_tools_dir, "CMakeLists.txt")
    spirv_tools_replacements = [
        (r"^(set_property\(GLOBAL PROPERTY USE_FOLDERS ON\))",
         r"\1\n\n"
         r"set(SPIRV-Headers_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../spirv-headers)\n\n"
         r"if (APPLE)\n"
         r"  set(CMAKE_MACOSX_RPATH ON)\n"
         r"endif (APPLE)\n"
         r"set(SPIRV_SKIP_EXECUTABLES_OPTION ON)\n"
         r"set(SPIRV_SKIP_TESTS_OPTION ON)\n"
         r"set(SKIP_SPIRV_TOOLS_INSTALL ON)"),
        (r"enable_testing\(\)\n", ""),
        (r'option\(SKIP_SPIRV_TOOLS_INSTALL "Skip installation" \$\{SKIP_SPIRV_TOOLS_INSTALL\}\)\n', ""),
        (r'option\(SPIRV_WERROR "Enable error on warning" ON\)',
         r"set(SPIRV_WERROR OFF)"),
        (r"add_subdirectory\(test\)\n", ""),
        (r"add_subdirectory\(examples\)\n", ""),
        (r"^# Build pkg-config file[\s\S]*?^endif\(\)\n", ""),
    ]
    process_file(spirv_tools_cmakelist, spirv_tools_replacements)

    # Define replacements for third_party/spirv-tools/source/CMakeLists.txt
    spirv_source_cmakelist = os.path.join(spirv_tools_dir, "source", "CMakeLists.txt")
    spirv_source_replacements = [
        (r"^add_library\(\$\{SPIRV_TOOLS\}\-shared[\s\S]*?^\)", ""),
        (r"set\(SPIRV_TOOLS_TARGETS \$\{SPIRV_TOOLS\}-static \$\{SPIRV_TOOLS\}\-shared\)",
         r"set(SPIRV_TOOLS_TARGETS ${SPIRV_TOOLS}-static)"),
        (r"set\(SPIRV_TOOLS_TARGETS \$\{SPIRV_TOOLS\} \$\{SPIRV_TOOLS\}\-shared\)",
         r"set(SPIRV_TOOLS_TARGETS ${SPIRV_TOOLS})"),
    ]
    process_file(spirv_source_cmakelist, spirv_source_replacements)

    print("Changes applied successfully using the corrected Python script.")
    print("Backup files with .bak extension have been created.")

if __name__ == "__main__":
    main()
