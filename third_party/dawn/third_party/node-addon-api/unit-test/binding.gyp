{
  'target_defaults': {
    'includes': ['../common.gypi'],
    'include_dirs': ['../test/common', "./generated"],
    'variables': {
      'setup': ["<!@(node -p \"require('./setup')\")"],
      'build_sources': [
        "<!@(node -p \"require('./injectTestParams').filesToCompile()\")",
      ]
    },
  },
  'targets': [
    {
      "target_name": "generateBindingCC",
      "type": "none",
      "actions": [ {
        "action_name": "generateBindingCC",
        "message": "Generating binding cc file",
        "outputs": ["generated/binding.cc"],
        "conditions": [
          [ "'true'=='true'", {
            "inputs": [""],
            "action": [
              "node",
              "generate-binding-cc.js",
              "<!@(node -p \"require('./injectTestParams').filesForBinding()\" )"
            ]
          } ]
        ]
      } ]
    },
    {
      'target_name': 'binding',
      'includes': ['../except.gypi'],
      'sources': ['>@(build_sources)'],
      'dependencies': [ 'generateBindingCC' ]
    },
    {
      'target_name': 'binding_noexcept',
      'includes': ['../noexcept.gypi'],
      'sources': ['>@(build_sources)'],
      'dependencies': [ 'generateBindingCC' ]
    },
    {
      'target_name': 'binding_noexcept_maybe',
      'includes': ['../noexcept.gypi'],
      'sources': ['>@(build_sources)'],
      'defines': ['NODE_ADDON_API_ENABLE_MAYBE']
    },
    {
      'target_name': 'binding_swallowexcept',
      'includes': ['../except.gypi'],
      'sources': ['>@(build_sources)'],
      'defines': ['NODE_API_SWALLOW_UNTHROWABLE_EXCEPTIONS'],
      'dependencies': [ 'generateBindingCC' ]
    },
    {
      'target_name': 'binding_swallowexcept_noexcept',
      'includes': ['../noexcept.gypi'],
      'sources': ['>@(build_sources)'],
      'defines': ['NODE_API_SWALLOW_UNTHROWABLE_EXCEPTIONS'],
      'dependencies': [ 'generateBindingCC' ]
    },
    {
      'target_name': 'binding_custom_namespace',
      'includes': ['../noexcept.gypi'],
      'sources': ['>@(build_sources)'],
      'defines': ['NAPI_CPP_CUSTOM_NAMESPACE=cstm'],
      'dependencies': [ 'generateBindingCC' ]
    },
  ],
}
