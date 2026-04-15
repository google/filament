.PHONY: all all-help-message help check-schema fix gen gen-check doc

# default target if you just type `make`
all: all-help-message gen fix doc

# help message before starting `make all`
all-help-message: help
	@echo 'Running default targets: gen fix doc'

help:
	@echo 'Targets are: all, help, gen, fix, gen-check, doc'

# Validate schema and regenerate JSON and header from YAML
gen: schema.json webgpu.yml tests/extensions/extension.yml
	go run ./gen -schema schema.json \
		-yaml webgpu.yml \
		-out-json webgpu.json \
		-out-header webgpu.h \
		-yaml tests/extensions/extension.yml \
		-out-json tests/extensions/extension.json \
		-out-header tests/extensions/webgpu_extension.h
	# Generate a test for each INIT macro defined in the header
	perl -ne 'print if s/^#define (WGPU_\w+_INIT) _wgpu_MAKE_INIT_STRUCT\((WGPU\w+),.*/{ \2 x = \1; }/' webgpu.h > tests/compile/init_tests_autogen.inl
	# Assert that that worked (e.g. in case we change the generator and the regex stops matching)
	[ "$$(wc -l < tests/compile/init_tests_autogen.inl)" -ge 89 ]

# Regenerate files (to validate schema in case something is broken) then autoformat YAML
fix: gen
	go run ./fix -yaml webgpu.yml
	go run ./fix -yaml tests/extensions/extension.yml

# Verify that generation and autoformat are up to date (used for pull request checks)
gen-check: fix gen
	@git diff --quiet || { \
	    echo "error: Some generated files have not been updated. Run 'make gen fix' or apply the changes manually:" \
		git diff; \
		exit 1; \
	}

doc: webgpu.h Doxyfile
	doxygen Doxyfile
	# Verify that no ` or :: made it through into the final docs
	! grep -RE '`|>::' doc/generated/**/*.html
