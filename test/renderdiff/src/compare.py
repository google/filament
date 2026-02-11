import glob
import os
import sys
import pprint
import json
import fnmatch
import subprocess
import tempfile

from utils import execute, ArgParseImpl, important_print, mkdir_p
from results import RESULT_OK, RESULT_FAILED, RESULT_MISSING, GOLDEN_MISSING
import test_config

def _get_tolerance_for_test_case(test_case_name, test_config_obj):
  if not test_config_obj:
    return None

  # Extract test name from test case (remove backend and model)
  # Format: TestName.backend.model -> TestName
  test_name = test_case_name.split('.')[0]

  for test in test_config_obj.tests:
    if test.name == test_name:
      return test.tolerance

  return None

def _run_diffimg(diffimg_path, ref_path, cand_path, tolerance=None, diff_out_path=None):
  cmd = [diffimg_path, ref_path, cand_path]

  config_file = None
  if tolerance:
    fd, config_file = tempfile.mkstemp(suffix='.json', text=True)
    with os.fdopen(fd, 'w') as f:
      json.dump(tolerance, f)
    cmd.extend(['--config', config_file])

  if diff_out_path:
    cmd.extend(['--diff', diff_out_path])

  try:
    result_proc = subprocess.run(cmd, capture_output=True, text=True)
    # diffimg outputs JSON to stdout even on failure (exit code might be non-zero for mismatch)
    # However, if it crashed or failed to run, stdout might be empty or not JSON.

    output = result_proc.stdout.strip()
    if not output:
      return False, {'error': 'No output from diffimg', 'stderr': result_proc.stderr}

    try:
      result_json = json.loads(output)
      passed = result_json.get('passed', False)
      return passed, result_json
    except json.JSONDecodeError:
      return False, {'error': 'Invalid JSON output from diffimg', 'stdout': output, 'stderr': result_proc.stderr}

  except Exception as e:
    return False, {'error': f'Failed to run diffimg: {e}'}
  finally:
    if config_file and os.path.exists(config_file):
      os.remove(config_file)


def _compare_goldens(base_dir, comparison_dir, diffimg_path, out_dir=None, test_filter=None, test_config_path=None):
  def test_name(p):
    return p.replace('.tif', '')

  all_files = glob.glob(os.path.join(base_dir, "./**/*.tif"), recursive=True)
  all_files = [os.path.abspath(f) for f in all_files \
               if not test_filter or fnmatch.fnmatch(test_name(os.path.basename(f)), test_filter)]
  test_dirs = set(os.path.abspath(os.path.dirname(f)).replace(os.path.abspath(base_dir) + '/', '') \
                  for f in all_files)
  all_results = []

  # Parse test configuration if provided
  test_config_obj = None
  if test_config_path and os.path.exists(test_config_path):
    try:
      test_config_obj = test_config.parse_from_path(test_config_path)
    except Exception as e:
      important_print(f"Warning: Could not parse test config {test_config_path}: {e}")

  def single_test(src_dir, dest_dir, src_fname):
    src_fname = os.path.abspath(src_fname)
    test_case = src_fname.replace(f'{src_dir}/', '')
    dest_fname = os.path.join(dest_dir, test_case)
    result = {
      'name': test_case,
    }

    if not os.path.exists(dest_fname):
      result['result'] = RESULT_MISSING
    else:
      # Get tolerance configuration for this test case
      tolerance = _get_tolerance_for_test_case(test_case.replace('.tif', ''), test_config_obj)

      diff_fname = None
      if output_test_dir:
        diff_fname = os.path.join(output_test_dir, f"{test_case.replace('.tif', '_diff.tif')}")
        # Ensure subdirectories exist for diff output
        os.makedirs(os.path.dirname(diff_fname), exist_ok=True)

      # Compare images using diffimg
      comparison_result, stats = _run_diffimg(diffimg_path, src_fname, dest_fname, tolerance, diff_fname)

      if not comparison_result:
        result['result'] = RESULT_FAILED
        if diff_fname and os.path.exists(diff_fname):
          result['diff'] = os.path.basename(diff_fname)
      else:
        result['result'] = RESULT_OK

      # Add detailed tolerance information to result
      if tolerance:
        result['tolerance_used'] = True
        result['tolerance_config'] = tolerance

      if stats:
        result['stats'] = stats
        if 'error' in stats:
          result['error'] = stats['error']

    return result

  for test_dir in test_dirs:
    results = []
    output_test_dir = None if not out_dir else os.path.abspath(os.path.join(out_dir, test_dir))
    if output_test_dir:
      mkdir_p(output_test_dir)
    base_test_dir = os.path.abspath(os.path.join(base_dir, test_dir))
    comp_test_dir = os.path.abspath(os.path.join(comparison_dir, test_dir))
    results = [
      single_test(base_test_dir, comp_test_dir, golden_file) \
      for golden_file in all_files if os.path.dirname(golden_file) == base_test_dir
    ]
    seen_test_cases = set([r['name'] for r in results])

    # For files that are rendered but not in the golden directory
    comparison_files = glob.glob(os.path.join(comp_test_dir, "*.tif"))
    if test_filter:
      comparison_files = [f for f in comparison_files \
                          if fnmatch.fnmatch(test_name(os.path.basename(f)), test_filter)]

    for base_file in comparison_files:
      src_fname = os.path.abspath(base_file)
      test_case = base_file.replace(f'{comp_test_dir}/', '')
      if test_case not in seen_test_cases:
        results.append({
          'name': test_case,
          'result': GOLDEN_MISSING,
        })

    if output_test_dir:
      output_fname = os.path.join(output_test_dir, "compare_results.json")
      results_meta = {
        'results': results,
        'base_dir': os.path.relpath(base_test_dir, output_test_dir),
        'comparison_dir': os.path.relpath(comp_test_dir, output_test_dir)
      }
      with open(output_fname, 'w') as f:
        f.write(json.dumps(results_meta, indent=2))
      important_print(f'Written comparison results for {test_dir} to \n    {output_fname}')
    all_results += results
  return all_results

if __name__ == '__main__':
  parser = ArgParseImpl()
  parser.add_argument('--src', help='Directory of the base of the diff.', required=True)
  parser.add_argument('--dest', help='Directory of the comparison of the diff.')
  parser.add_argument('--out', help='Directory of output for the result of the diff.')
  parser.add_argument('--diffimg', help='Path to the diffimg tool.', required=True)
  parser.add_argument('--test_filter', help='Filter for the tests to run')
  parser.add_argument('--test', help='Path to test configuration JSON file for tolerance settings.')

  args, _ = parser.parse_known_args(sys.argv[1:])

  dest = args.dest
  if not dest:
    print('Assume the default renderdiff output folder')
    dest = os.path.join(os.getcwd(), './out/renderdiff')
  assert os.path.exists(dest), f"Destination folder={dest} does not exist."

  if not os.path.exists(args.diffimg):
    print(f"Error: diffimg tool not found at {args.diffimg}")
    sys.exit(1)

  results = _compare_goldens(args.src, dest, args.diffimg, out_dir=args.out,
                             test_filter=args.test_filter, test_config_path=args.test)

  # Categorize results
  failed = [k for k in results if k['result'] != RESULT_OK]
  passed = [k for k in results if k['result'] == RESULT_OK]

  # Create detailed failure report
  failed_details = []
  for k in failed:
    failure_line = f"   {k['name']} ({k['result']})"
    if 'stats' in k:
      stats = k['stats']
      if 'maxDiffFound' in stats:
        failure_line += f"\n     Max Diff: {stats['maxDiffFound']}"
      if 'failingPixelCount' in stats:
        failure_line += f"\n     Failing Pixels: {stats['failingPixelCount']}"
    failed_details.append(failure_line)

  # Main summary
  success_count = len(passed)
  important_print(f'Successfully compared {success_count} / {len(results)} images')

  if failed_details:
    pstr = 'Failed:'
    for detail in failed_details:
      pstr += '\n' + detail
    important_print(pstr)
  if len(failed) > 0:
    exit(1)