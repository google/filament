# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.


import argparse
import importlib
import json
import logging
import os
import pathlib
import sys
from typing import Sequence

import importlib_resources as ir
import jsonschema

from . import model

PACKAGES_ROOT = pathlib.Path(__file__).parent.parent / "packages"
TESTS_ROOT = pathlib.Path(__file__).parent.parent / "tests"
LOGGER = logging.getLogger("generator")


def setup_logging() -> None:
    logging.basicConfig(
        stream=sys.stdout,
        level=logging.DEBUG,
        format="[%(levelname)s][%(asctime)s]  %(name)s: %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )


def get_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Generate types from LSP JSON model.")
    parser.add_argument(
        "--model",
        "-m",
        help="Path to a model JSON file. By default uses packaged model file.",
        type=str,
        nargs="*",
    )
    parser.add_argument(
        "--plugin",
        "-p",
        help="Name of a builtin plugin module. By default uses all plugins.",
        type=str,
        required=True,
    )
    parser.add_argument(
        "--output-dir",
        "-o",
        help="Path to a directory where the generated content is written.",
        type=str,
    )
    parser.add_argument(
        "--test-dir",
        "-t",
        help="Path to a directory where the generated tests are written.",
        type=str,
    )
    return parser


def main(argv: Sequence[str]) -> None:
    parser = get_parser()
    args = parser.parse_args(argv)

    # Validate against LSP model JSON schema.
    schema_file = ir.files("generator") / "lsp.schema.json"

    LOGGER.info("Using schema file %s", os.fspath(schema_file))
    schema = json.load(schema_file.open("rb"))

    if args.model:
        model_files = [pathlib.Path(m) for m in args.model]
    else:
        model_files = [ir.files("generator") / "lsp.json"]

    json_models = []
    for model_file in model_files:
        LOGGER.info("Validating model file %s", os.fspath(model_file))
        json_model = json.load(model_file.open("rb"))
        jsonschema.validate(json_model, schema)
        json_models.append(json_model)

    plugin = args.plugin
    LOGGER.info(f"Running plugin {plugin}.")

    output_dir = args.output_dir or os.fspath(PACKAGES_ROOT / plugin)
    test_dir = args.test_dir or os.fspath(TESTS_ROOT / plugin)
    LOGGER.info(f"Writing output to {output_dir}")

    # load model and generate types for each plugin to avoid
    # any conflicts between plugins.
    spec: model.LSPModel = model.create_lsp_model(json_models)

    try:
        LOGGER.info(f"Loading plugin: {plugin}.")
        plugin_module = importlib.import_module(f"generator.plugins.{plugin}")
        LOGGER.info(f"Running plugin: {plugin}.")
        plugin_module.generate(spec, output_dir, test_dir)
        LOGGER.info(f"Plugin {plugin} completed.")
    except Exception as e:
        LOGGER.error(f"Error running plugin {plugin}:", exc_info=e)


if __name__ == "__main__":
    setup_logging()
    main(sys.argv[1:])
