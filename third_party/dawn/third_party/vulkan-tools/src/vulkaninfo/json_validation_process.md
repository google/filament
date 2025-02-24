# Validating [vulkaninfo](https://github.com/KhronosGroup/Vulkan-Tools/tree/main/vulkaninfo) JSON output

The format of vulkaninfo's JSON output is designed to be used as input for the
[Vulkan Profiles](https://github.com/KhronosGroup/Vulkan-Profiles)
solution.

The Vulkan Profiles JSON schema specifies exactly how the JSON must be structured.
The schemas may be found at [here](https://schema.khronos.org/vulkan/).
Select the latest schema that `vulkaninfo` was designed to be used with, or simply take the latest available schema.

## Steps to validate JSON data against the Vulkan Profiles schema

1. Generate the text to be tested using `vulkaninfo --json` which creates a `.json` file automatically.
1. Download the [Vulkan Profiles schema](https://schema.khronos.org/vulkan/) to another file.
1. For each of the on-line JSON validator tools listed below:
   1. Paste the schema and and sample text into the `schema` and `data` fields.
   1. Depending on the tool, it may validate automatically, or require clicking a `validate` button.
   1. Ensure the tool reports no errors.

## List of recommended JSON validator tools

* https://www.jsonschemavalidator.net/
* https://jsonschemalint.com/#/version/draft-04/markup/json
* https://json-schema-validator.herokuapp.com/index.jsp
