import synthtool as s
from synthtool import gcp

common = gcp.CommonTemplates()

# ----------------------------------------------------------------------------
# Add templated files
# ----------------------------------------------------------------------------
templated_files = common.py_library(unit_cov_level=100, cov_level=100)

paths = [".kokoro", ".github", ".flake8", "renovate.json"]
for p in paths:
    s.move(templated_files / p, excludes=["workflows"])

