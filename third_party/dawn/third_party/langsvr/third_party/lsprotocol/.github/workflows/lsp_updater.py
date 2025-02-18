# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.


import hashlib
import json
import os
import pathlib
import urllib.request as url_lib
from typing import Optional, Union

import github
import github.Issue
import github.PullRequest

MODEL_SCHEMA = "https://raw.githubusercontent.com/microsoft/vscode-languageserver-node/main/protocol/metaModel.schema.json"
MODEL = "https://raw.githubusercontent.com/microsoft/vscode-languageserver-node/main/protocol/metaModel.json"

LABEL_DEBT = "debt"
LABEL_UPDATE = "lsp-update"

GH = github.Github(os.getenv("GITHUB_ACCESS_TOKEN"))
GH_REPO = GH.get_repo(os.getenv("GITHUB_REPOSITORY"))


def _get_content(uri) -> str:
    with url_lib.urlopen(uri) as response:
        content = response.read()
        if isinstance(content, str):
            return content
        else:
            return content.decode("utf-8")


def _get_update_issue_body(schema_hash: str, model_hash: str) -> str:
    return "\n".join(
        [
            "LSP Schema has changed. Please update the generator.",
            f"Schema: [source]({MODEL_SCHEMA})",
            f"Model: [source]({MODEL})",
            "",
            "Instructions:",
            "1. Setup a virtual environment and install nox.",
            "2. Install all requirements for generator.",
            "3. Run `nox --session update_lsp`.",
            "",
            "Hashes:",
            f"* schema: `{schema_hash}`",
            f"* model: `{model_hash}`",
        ]
    )


def is_schema_changed(old_schema: str, new_schema: str) -> bool:
    old_schema = json.loads(old_schema)
    new_schema = json.loads(new_schema)
    return old_schema != new_schema


def is_model_changed(old_model: str, new_model: str) -> bool:
    old_model = json.loads(old_model)
    new_model = json.loads(new_model)
    return old_model != new_model


def get_hash(text: str) -> str:
    hash_object = hashlib.sha256()
    hash_object.update(json.dumps(json.loads(text)).encode())
    return hash_object.hexdigest()


def get_existing_issue(
    schema_hash: str, model_hash: str
) -> Union[github.PullRequest.PullRequest, None]:
    issues = GH_REPO.get_issues(state="open", labels=[LABEL_UPDATE, LABEL_DEBT])
    for issue in issues:
        if schema_hash in issue.body and model_hash in issue.body:
            return issue

    return None


def cleanup_stale_issues(
    schema_hash: str, model_hash: str, new_issue: Optional[github.Issue.Issue] = None
):
    issues = GH_REPO.get_issues(state="open", labels=[LABEL_UPDATE, LABEL_DEBT])
    for issue in issues:
        if schema_hash not in issue.body or model_hash not in issue.body:
            if new_issue:
                issue.create_comment(
                    "\n".join(
                        [
                            "This issue is stale as the schema has changed.",
                            f"Closing in favor of {new_issue.url} .",
                        ]
                    )
                )
            issue.edit(state="closed")


def main():
    lsp_root = pathlib.Path(__file__).parent.parent.parent / "generator"

    old_schema = pathlib.Path(lsp_root / "lsp.schema.json").read_text(encoding="utf-8")
    old_model = pathlib.Path(lsp_root / "lsp.json").read_text(encoding="utf-8")

    new_schema = _get_content(MODEL_SCHEMA)
    new_model = _get_content(MODEL)

    schema_changed = is_schema_changed(old_schema, new_schema)
    model_changed = is_model_changed(old_model, new_model)

    if schema_changed or model_changed:
        schema_hash = get_hash(new_schema)
        model_hash = get_hash(new_model)

        issue = get_existing_issue(schema_hash, model_hash)

        if not issue:
            issue = GH_REPO.create_issue(
                title="Update LSP schema and model",
                body=_get_update_issue_body(schema_hash, model_hash),
                labels=[LABEL_UPDATE, LABEL_DEBT],
            )
        cleanup_stale_issues(schema_hash, model_hash, issue)
        print(f"Created issue {issue.url}")
    else:
        print("No changes detected.")


if __name__ == "__main__":
    main()
