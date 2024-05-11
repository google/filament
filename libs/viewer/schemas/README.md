The schema in this folder can be used to enable autocomplete and help features when authoring
automation specs in your favorite text editor. For example, for Visual Studio Code you can add the
following to `.vscode/settings`:

```
"json.schemas": [
    {
        "fileMatch": [
            "libs/viewer/tests/basic.json"
        ],
        "url": "./libs/viewer/schemas/automation.json"
    }
]
```
