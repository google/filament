Test every method or option that shouldn't be allowed without a feature enabled.
If the feature is not enabled, any use of an enum value added by a feature must be an
*exception*, per <https://github.com/gpuweb/gpuweb/blob/main/design/ErrorConventions.md>.

- x= that feature {enabled, disabled}

Generally one file for each feature name, but some may be grouped (e.g. one file for all optional
query types, one file for all optional texture formats).

TODO: implement
