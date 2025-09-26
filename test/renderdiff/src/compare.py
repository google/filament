import glob
import os
import sys
import pprint
import json
import fnmatch

from utils import execute, ArgParseImpl, important_print, mkdir_p
from image_diff import same_image, output_image_diff
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

def _format_tolerance_summary(stats):
  """
  Create human-readable summary of tolerance statistics.

  Args:
    stats: Statistics dictionary from tolerance evaluation

  Returns:
    str: Formatted summary string
  """
  if 'error' in stats:
    return f"Error: {stats['error']}"

  if 'operator' in stats:
    # Nested criteria with operator
    operator = stats['operator']
    criteria_count = len(stats['criteria_results'])
    passed_count = sum(1 for c in stats['criteria_results'] if c.get('passed', False))
    summary = f"{operator} of {criteria_count} criteria: {passed_count} passed, {criteria_count - passed_count} failed"

    # Add details for each criteria
    details = []
    for i, criteria_stats in enumerate(stats['criteria_results']):
      details.append(f"  Criteria {i+1}: {_format_tolerance_summary(criteria_stats)}")

    return summary + "\n" + "\n".join(details)
  else:
    # Single criteria
    total_pixels = stats.get('total_pixels', 0)
    failing_pixels = stats.get('failing_pixels', 0)
    failing_percentage = stats.get('failing_percentage', 0.0)
    allowed_percentage = stats.get('allowed_percentage', 0.0)
    max_abs_diff = stats.get('max_abs_diff', 0)
    mean_abs_diff = stats.get('mean_abs_diff', 0)
    max_diff_per_channel = stats.get('max_diff_per_channel', [])

    criteria = stats.get('criteria', {})
    criteria_desc = []
    if 'max_pixel_diff' in criteria:
      criteria_desc.append(f"max_pixel_diff: {criteria['max_pixel_diff']}")
    if 'max_pixel_diff_percent' in criteria:
      criteria_desc.append(f"max_pixel_diff_percent: {criteria['max_pixel_diff_percent']}%")
    if 'allowed_diff_pixels' in criteria:
      criteria_desc.append(f"allowed_diff_pixels: {criteria['allowed_diff_pixels']}%")

    summary_lines = [
      f"Tolerance: {', '.join(criteria_desc)}",
      f"Pixels: {failing_pixels:,} / {total_pixels:,} ({failing_percentage:.2f}%) exceed tolerance",
      f"Allowed: {allowed_percentage:.2f}% - {'PASS' if stats.get('passed', False) else 'FAIL'}",
      f"Max difference: {max_abs_diff} (mean: {mean_abs_diff:.1f})"
    ]

    if len(max_diff_per_channel) > 1:
      channel_info = ", ".join(f"Ch{i}: {diff}" for i, diff in enumerate(max_diff_per_channel))
      summary_lines.append(f"Per-channel max: {channel_info}")

    return "\n".join(summary_lines)

def _compare_goldens(base_dir, comparison_dir, out_dir=None, test_filter=None, test_config_path=None):
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

      # Compare images and get detailed statistics
      comparison_result, stats = same_image(src_fname, dest_fname, tolerance)

      if not comparison_result:
        result['result'] = RESULT_FAILED
        if output_test_dir:
          # just the file name
          diff_fname = f"{test_case.replace('.tif', '_diff.tif')}"
          output_image_diff(src_fname, dest_fname, os.path.join(output_test_dir, diff_fname))
          result['diff'] = diff_fname
      else:
        result['result'] = RESULT_OK

      # Add detailed tolerance information to result
      if tolerance:
        result['tolerance_used'] = True
        result['tolerance_config'] = tolerance
        if stats:
          result['tolerance_stats'] = stats
          # Add human-readable summary
          result['tolerance_summary'] = _format_tolerance_summary(stats)
      elif stats is None and comparison_result:
        result['comparison_type'] = 'exact_match'
      elif stats and 'error' in stats:
        result['error'] = stats['error']
        if 'details' in stats:
          result['error_details'] = stats['details']

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
  parser.add_argument('--test_filter', help='Filter for the tests to run')
  parser.add_argument('--test', help='Path to test configuration JSON file for tolerance settings.')

  args, _ = parser.parse_known_args(sys.argv[1:])

  dest = args.dest
  if not dest:
    print('Assume the default renderdiff output folder')
    dest = os.path.join(os.getcwd(), './out/renderdiff')
  assert os.path.exists(dest), f"Destination folder={dest} does not exist."

  results = _compare_goldens(args.src, dest, out_dir=args.out,
                             test_filter=args.test_filter, test_config_path=args.test)

  # Categorize results
  failed = [k for k in results if k['result'] != RESULT_OK]
  passed = [k for k in results if k['result'] == RESULT_OK]
  tolerance_used_count = len([k for k in results if k.get('tolerance_used', False)])

  # Create detailed failure report
  failed_details = []
  for k in failed:
    failure_line = f"   {k['name']} ({k['result']})"
    if 'tolerance_summary' in k:
      failure_line += f"\n     {k['tolerance_summary'].replace(chr(10), chr(10) + '     ')}"
    failed_details.append(failure_line)

  # Create success report with tolerance details
  tolerance_used_details = []
  for k in passed:
    if k.get('tolerance_used', False) and 'tolerance_summary' in k:
      tolerance_used_details.append(f"   {k['name']}: {k['tolerance_summary'].split(chr(10))[0]}")

  # Main summary
  success_count = len(passed)
  important_print(f'Successfully compared {success_count} / {len(results)} images')

  if tolerance_used_details:
    pstr = 'Tolerance-based passes:'
    for detail in tolerance_used_details:
      pstr += '\n' + detail
    important_print(pstr)

  if failed_details:
    pstr = 'Failed:'
    for detail in failed_details:
      pstr = '\n' + detail
    important_print(pstr)
  if len(failed) > 0:
    exit(1)
