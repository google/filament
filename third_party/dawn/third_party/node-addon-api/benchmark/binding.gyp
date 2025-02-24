{
  'target_defaults': { 'includes': ['../common.gypi'] },
  'targets': [
    {
      'target_name': 'function_args',
      'sources': [ 'function_args.cc' ],
      'dependencies': ['../node_addon_api.gyp:node_addon_api_except'],
    },
    {
      'target_name': 'function_args_noexcept',
      'sources': [ 'function_args.cc' ],
      'dependencies': ['../node_addon_api.gyp:node_addon_api'],
    },
    {
      'target_name': 'property_descriptor',
      'sources': [ 'property_descriptor.cc' ],
      'dependencies': ['../node_addon_api.gyp:node_addon_api_except'],
    },
    {
      'target_name': 'property_descriptor_noexcept',
      'sources': [ 'property_descriptor.cc' ],
      'dependencies': ['../node_addon_api.gyp:node_addon_api'],
    },
  ]
}
