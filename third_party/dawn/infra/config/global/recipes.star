# Copyright 2025 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""Recipe definitions for recipes used by Dawn builders."""

load("@chromium-luci//bootstrap.star", "register_recipe_bootstrappability")
load("@chromium-luci//recipe_experiments.star", "register_recipe_experiments")

_RECIPE_NAME_PREFIX = "recipe:"

def _recipe_for_package(cipd_package):
    def recipe(
            *,
            name,
            cipd_version = None,
            recipe = None,
            bootstrappable = False,
            experiments = None):
        """Declare a recipe for the given package.

        A wrapper around luci.recipe with a fixed cipd_package and some
        chromium-specific functionality. See
        https://chromium.googlesource.com/infra/luci/luci-go/+/HEAD/lucicfg/doc/README.md#luci.recipe
        for more information.

        Args:
            name: The name to use to refer to the executable in builder
              definitions. Must start with "recipe:". See luci.recipe for more
              information.
            cipd_version: See luci.recipe.
            recipe: See luci.recipe.
            bootstrappable: Whether or not the recipe supports the chromium
              bootstrapper. A recipe supports the bootstrapper if the following
              conditions are met:
              * chromium_bootstrap.update_gclient_config is called to update the
                gclient config that is used for bot_update. This will be true if
                calling chromium_checkout.ensure_checkout or
                chromium_tests.prepare_checkout.
              * If the recipe does analysis to reduce compilation/testing, it
                skips analysis and performs a full build if
                chromium_bootstrap.skip_analysis_reasons is non-empty. This will
                be true if calling chromium_tests.determine_compilation_targets.
              In addition to a True or False value, POLYMORPHIC can be
              specified. This value will cause the builder's executable to be
              changed to the bootstrapper in properties-optional, polymorphic
              mode, which will by default not bootstrap any properties. On a
              per-run basis the $bootstrap/properties property can be set to
              bootstrap properties for different builders.
            experiments: Experiments to apply to a builder using the recipe. If
              the builder specifies an experiment, the experiment value from the
              recipe will be ignored.
        """

        # Force the caller to put the recipe prefix rather than adding it
        # programatically to make the string greppable
        if not name.startswith(_RECIPE_NAME_PREFIX):
            fail("Recipe name {!r} does not start with {!r}"
                .format(name, _RECIPE_NAME_PREFIX))
        if recipe == None:
            recipe = name[len(_RECIPE_NAME_PREFIX):]
        ret = luci.recipe(
            name = name,
            cipd_package = cipd_package,
            cipd_version = cipd_version,
            recipe = recipe,
            use_bbagent = True,
            use_python3 = True,
        )

        register_recipe_bootstrappability(name, bootstrappable)

        register_recipe_experiments(name, experiments or {})

        return ret

    return recipe

build_recipe = _recipe_for_package(
    "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build",
)

build_recipe(
    name = "recipe:dawn/cmake",
)

build_recipe(
    name = "recipe:dawn/gn",
)

build_recipe(
    name = "recipe:dawn/gn_v2",
)

build_recipe(
    name = "recipe:dawn/gn_v2_trybot",
)

build_recipe(
    name = "recipe:dawn/roll_cts",
)

build_recipe(
    name = "recipe:run_presubmit",
)
