from PB.recipe_modules.depot_tools.gsutil import properties

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/step',
]

ENV_PROPERTIES = properties.EnvProperties
