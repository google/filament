# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
import json
import pathlib
import urllib.request as url_lib

import nox


def _install_requirements(session: nox.Session):
    session.install(
        "-r",
        "./packages/python/requirements.txt",
        "-r",
        "./requirements.txt",
    )
    session.run("pip", "list")


@nox.session()
def tests(session: nox.Session):
    """Run tests for generator and generated code in python."""
    _install_requirements(session)

    session.log("Running test data generator.")
    session.run("python", "-m", "generator", "--plugin", "testdata")

    session.log("Running tests: generator and generated Python code.")
    session.run("pytest", "./tests")


@nox.session()
def lint(session: nox.Session):
    """Linting for generator and generated code in all languages."""
    _install_requirements(session)

    session.log("Linting: generator and generated Python code.")
    session.install("mypy", "ruff")
    session.run("ruff", "check", ".")
    session.run("ruff", "check", "--select=I001", ".")
    session.run("ruff", "format", "--check", ".")
    session.run("mypy", "--strict", "--no-incremental", "./packages/python/lsprotocol")

    session.log("Linting: generated Rust code.")
    with session.chdir("./packages/rust/lsprotocol"):
        session.run("cargo", "fmt", "--check", external=True)


@nox.session()
def format(session: nox.Session):
    """Format generator and lsprotocol package for PyPI."""
    _install_requirements(session)
    _format_code(session)


def _format_code(session: nox.Session):
    session.install("ruff")

    session.run("ruff", "check", "--fix", "--select=I001", ".")
    session.run("ruff", "format", ".")
    session.run("ruff", "check", "--fix", ".")


@nox.session()
def build_python_package(session: nox.Session):
    """Build lsprotocol (python) package for PyPI."""
    session.install("flit")
    with session.chdir("./packages/python"):
        session.run("flit", "build")


def _get_content(uri) -> str:
    with url_lib.urlopen(uri) as response:
        content = response.read()
        if isinstance(content, str):
            return content
        else:
            return content.decode("utf-8")


MODEL_SCHEMA = "https://raw.githubusercontent.com/microsoft/vscode-languageserver-node/main/protocol/metaModel.schema.json"
MODEL = "https://raw.githubusercontent.com/microsoft/vscode-languageserver-node/main/protocol/metaModel.json"


def _download_models(session: nox.Session):
    session.log("Downloading LSP model schema.")
    model_schema_text: str = _get_content(MODEL_SCHEMA)
    session.log("Downloading LSP model.")
    model_text: str = _get_content(MODEL)

    schema_path = pathlib.Path(__file__).parent / "generator" / "lsp.schema.json"
    model_path = schema_path.parent / "lsp.json"

    schema_path.write_text(
        json.dumps(json.loads(model_schema_text), indent=4, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )
    model_path.write_text(
        json.dumps(json.loads(model_text), indent=4, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )


@nox.session()
def build_lsp(session: nox.Session):
    """Generate lsprotocol for all languages."""
    generate_python(session)
    generate_dotnet(session)
    generate_rust(session)


@nox.session()
def update_lsp(session: nox.Session):
    """Update the LSP model and generate the lsprotocol for all languages."""
    update_packages(session)
    _download_models(session)
    build_lsp(session)


@nox.session()
def update_packages(session: nox.Session):
    """Update dependencies of generator and lsprotocol."""
    session.install("wheel", "pip-tools")

    session.run(
        "pip-compile",
        "--generate-hashes",
        "--resolver=backtracking",
        "--upgrade",
        "./packages/python/requirements.in",
    )
    session.run(
        "pip-compile",
        "--generate-hashes",
        "--resolver=backtracking",
        "--upgrade",
        "./requirements.in",
    )


@nox.session()
def create_plugin(session: nox.Session):
    """Create a new plugin."""
    name = input("Enter the name of the plugin: ")

    plugin_root = pathlib.Path(__file__).parent / "generator" / "plugins" / name
    plugin_root.mkdir(parents=True, exist_ok=True)

    init_text = "\n".join(
        [
            "# Copyright (c) Microsoft Corporation. All rights reserved.",
            "# Licensed under the MIT License.",
            "",
            f"from .{name}_utils import generate_from_spec as generate",
            "",
        ]
    )
    plugin_root.joinpath("__init__.py").write_text(init_text, encoding="utf-8")

    utils_text = "\n".join(
        [
            "# Copyright (c) Microsoft Corporation. All rights reserved.",
            "# Licensed under the MIT License.",
            "",
            "import pathlib",
            "from typing import List, Dict",
            "",
            "import generator.model as model",
            "",
            "",
            'PACKAGE_DIR_NAME = "lsprotocol"',
            "",
            "",
            "def generate_from_spec(spec: model.LSPModel, output_dir: str) -> None:",
            '    """Generate the code for the given spec."""',
            "    # key is the relative path to the file, value is the content",
            "    code: Dict[str, str] = generate_package_code(spec)",
            "    for file_name in code:",
            "        pathlib.Path(output_dir, PACKAGE_DIR_NAME, file_name).write_text(",
            '            code[file_name], encoding="utf-8"',
            "        )",
            "",
            "def generate_package_code(spec: model.LSPModel) -> List[str]:",
            "    return {",
            '        "src/lib.rs": "code for lib.rs",',
            "    }",
            "",
        ]
    )

    plugin_root.joinpath(f"{name}_utils.py").write_text(utils_text, encoding="utf-8")

    package_root = pathlib.Path(__file__).parent / "packages" / name / "lsprotocol"
    package_root.mkdir(parents=True, exist_ok=True)
    package_root.joinpath("README.md").write_text(
        "# your generated code and other package files go under this directory.",
        encoding="utf-8",
    )

    tests_root = pathlib.Path(__file__).parent / "tests" / name
    tests_root.mkdir(parents=True, exist_ok=True)
    tests_root.joinpath("README.md").write_text(
        "# your tests go under this directory.", encoding="utf-8"
    )

    launch_json_path = pathlib.Path(__file__).parent / ".vscode" / "launch.json"
    launch_json = json.loads(launch_json_path.read_text(encoding="utf-8"))

    for i in launch_json["inputs"]:
        if i["id"] == "plugin":
            i["options"].append(name)

    launch_json_path.write_text(json.dumps(launch_json, indent=4), encoding="utf-8")

    session.log(f"Created plugin {name}.")


@nox.session()
def generate_dotnet(session: nox.Session):
    """Update the dotnet code."""
    _install_requirements(session)

    session.run("python", "-m", "generator", "--plugin", "dotnet")
    with session.chdir("./packages/dotnet/lsprotocol"):
        session.run("dotnet", "format", external=True)
        session.run("dotnet", "build", external=True)


@nox.session()
def generate_python(session: nox.Session):
    """Update the python code."""
    _install_requirements(session)

    session.run("python", "-m", "generator", "--plugin", "python")
    _format_code(session)


@nox.session()
def generate_rust(session: nox.Session):
    """Update the rust code."""
    _install_requirements(session)

    session.run("python", "-m", "generator", "--plugin", "rust")

    with session.chdir("./packages/rust/lsprotocol"):
        session.run("cargo", "fmt", external=True)
    with session.chdir("./tests/rust"):
        session.run("cargo", "fmt", external=True)
        session.run("cargo", "build", external=True)
