PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
    'depot_tools',
    'gclient',
    'gerrit',
    'gitiles',
    'gsutil',
    'recipe_engine/archive',
    'recipe_engine/buildbucket',
    'recipe_engine/context',
    'recipe_engine/commit_position',
    'recipe_engine/cv',
    'recipe_engine/json',
    'recipe_engine/led',
    'recipe_engine/milo',
    'recipe_engine/path',
    'recipe_engine/platform',
    'recipe_engine/properties',
    'recipe_engine/raw_io',
    'recipe_engine/runtime',
    'recipe_engine/step',
    'recipe_engine/warning',
    'tryserver',
]

from recipe_engine.recipe_api import Property
from recipe_engine.config import ConfigGroup, Single

PROPERTIES = {
    # Gerrit patches will have all properties about them prefixed with patch_.
    'deps_revision_overrides':
    Property(default={}),
    '$depot_tools/bot_update':
    Property(
        help='Properties specific to bot_update module.',
        param_name='properties',
        kind=ConfigGroup(stale_process_duration_override=Single(int)),
        default={},
    ),
}

# Forward these types so that they can be used without importing api
from .api import RelativeRoot, Result
