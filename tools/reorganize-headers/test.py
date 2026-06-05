#!/usr/bin/env python3
import os
import sys
import unittest
import tempfile

# Ensure the tools/ directory is in the Python path to import reorganize_headers
script_dir = os.path.dirname(os.path.abspath(__file__))
workspace_root = os.path.dirname(os.path.dirname(script_dir))
sys.path.insert(0, os.path.join(workspace_root, "tools"))

import reorganize_headers

class TestHeaderReorganizer(unittest.TestCase):

    def setUp(self):
        # Setup stable, mocked topological library order for deterministic testing
        reorganize_headers.SORTED_LIBS = [
            "filament", "filamat", "gltfio", "filaflat", 
            "filabridge", "backend", "utils", "math"
        ]

    def run_reorganizer_on_content(self, content, filename="TestFile.cpp"):
        """Helper to write content to a temp file, run reorganize_file, and return new content."""
        with tempfile.TemporaryDirectory() as tmpdir:
            filepath = os.path.join(tmpdir, filename)
            # Create mock directory structure inside temp dir if testing local private includes
            if filename.startswith("src/"):
                os.makedirs(os.path.dirname(filepath), exist_ok=True)
                
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)

            changed = reorganize_headers.reorganize_file(filepath, dry_run=False)
            
            with open(filepath, 'r', encoding='utf-8') as f:
                new_content = f.read()
                
            return changed, new_content

    def test_standard_layering_order(self):
        original = """/* License */
#include <math/vec3.h>
#include <utils/Allocator.h>
#include <filament/Box.h>
#include "PrivateStuff.h"
#include "details/TestFile.h"
#include <stddef.h>
#include <algorithm>
"""
        expected = """/* License */
#include "details/TestFile.h"

#include "PrivateStuff.h"

#include <filament/Box.h>

#include <utils/Allocator.h>

#include <math/vec3.h>

#include <algorithm>

#include <stddef.h>
"""
        changed, result = self.run_reorganizer_on_content(original)
        self.assertTrue(changed)
        self.assertEqual(result, expected)

    def test_unwindows_high_priority(self):
        original = """#include <backend/Handle.h>
#include <utils/unwindows.h>
#include "details/TestFile.h"
"""
        expected = """#include "details/TestFile.h"

#include <utils/unwindows.h>

#include <backend/Handle.h>
"""
        changed, result = self.run_reorganizer_on_content(original)
        self.assertTrue(changed)
        self.assertEqual(result, expected)

    def test_quotes_to_brackets_conversion(self):
        # Internal target public/private includes in quotes should convert to brackets
        original = """#include "backend/Handle.h"
#include "private/filament/UibStructs.h"
#include "details/TestFile.h"
"""
        expected = """#include "details/TestFile.h"

#include <private/filament/UibStructs.h>

#include <backend/Handle.h>
"""
        changed, result = self.run_reorganizer_on_content(original)
        self.assertTrue(changed)
        self.assertEqual(result, expected)

    def test_brackets_to_quotes_conversion(self):
        # Bracket includes that exist under the local module's src/ root should convert to quotes
        with tempfile.TemporaryDirectory() as tmpdir:
            # Mock src/ folder structure:
            # tmpdir/filament/src/materials/StaticInfo.h
            # tmpdir/filament/src/materials/fog/fog.cpp
            src_dir = os.path.join(tmpdir, "filament", "src")
            materials_dir = os.path.join(src_dir, "materials")
            fog_dir = os.path.join(materials_dir, "fog")
            os.makedirs(fog_dir, exist_ok=True)

            # Create local header file
            local_header = os.path.join(materials_dir, "StaticInfo.h")
            with open(local_header, 'w') as f:
                f.write("// Header")

            # Create source file including the local header with angle brackets
            source_file = os.path.join(fog_dir, "fog.cpp")
            original = """#include "fog.h"
#include <materials/StaticInfo.h>
#include <utils/Slice.h>
"""
            expected = """#include "fog.h"

#include "materials/StaticInfo.h"

#include <utils/Slice.h>
"""
            with open(source_file, 'w') as f:
                f.write(original)

            changed = reorganize_headers.reorganize_file(source_file, dry_run=False)
            self.assertTrue(changed)

            with open(source_file, 'r') as f:
                result = f.read()

            self.assertEqual(result, expected)

    def test_flat_includes_sorted_first_in_private(self):
        original = """#include "components/LightManager.h"
#include "Allocators.h"
#include "details/TestFile.h"
"""
        expected = """#include "details/TestFile.h"

#include "Allocators.h"

#include "components/LightManager.h"
"""
        changed, result = self.run_reorganizer_on_content(original)
        self.assertTrue(changed)
        self.assertEqual(result, expected)

    def test_subdir_spacing_separation_in_private(self):
        original = """#include "components/LightManager.h"
#include "components/TransformManager.h"
#include "details/Engine.h"
#include "details/Skybox.h"
#include "details/TestFile.h"
"""
        expected = """#include "details/TestFile.h"

#include "components/LightManager.h"
#include "components/TransformManager.h"

#include "details/Engine.h"
#include "details/Skybox.h"
"""
        changed, result = self.run_reorganizer_on_content(original)
        self.assertTrue(changed)
        self.assertEqual(result, expected)

    def test_simple_guarded_includes_preserved(self):
        original = """#include <utils/compiler.h>
#if __EXCEPTIONS
#include <stdexcept>
#endif
#include "details/TestFile.h"
"""
        expected = """#include "details/TestFile.h"

#include <utils/compiler.h>

#if __EXCEPTIONS
#include <stdexcept>
#endif
"""
        changed, result = self.run_reorganizer_on_content(original)
        self.assertTrue(changed)
        self.assertEqual(result, expected)

    def test_complex_conditionals_skipped(self):
        # Complex conditions inside include range should trigger safety skip
        original = """#include <utils/compiler.h>
#ifdef FOO
#include <vector>
#else
#include <string>
#endif
#include "details/TestFile.h"
"""
        changed, result = self.run_reorganizer_on_content(original)
        self.assertFalse(changed)
        self.assertEqual(result, original)

    def test_duplicate_includes_deduplicated(self):
        original = """#include <utils/Allocator.h>
#include "details/TestFile.h"
#include <utils/Allocator.h>
"""
        expected = """#include "details/TestFile.h"

#include <utils/Allocator.h>
"""
        changed, result = self.run_reorganizer_on_content(original)
        self.assertTrue(changed)
        self.assertEqual(result, expected)

    def test_topological_sort(self):
        # Mock dependency links: A depends on B, B depends on C
        mock_dependencies = {
            "A": {"B"},
            "B": {"C"},
            "C": set()
        }
        sorted_result = reorganize_headers.compute_topological_sort(mock_dependencies)
        self.assertEqual(sorted_result, ["A", "B", "C"])

if __name__ == "__main__":
    unittest.main()
