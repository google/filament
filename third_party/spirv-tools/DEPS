use_relative_paths = True

vars = {
  'github': 'https://github.com',

  'abseil_revision': '69b58f9227bf91a9315475579df15fbb7c5c612e',

  'effcee_revision': '12241cbc30f20730b656db7fd5a3fa36cd420843',

  'googletest_revision': 'e235eb34c6c4fed790ccdad4b16394301360dcd4',

  # Use protobufs before they gained the dependency on abseil
  'protobuf_revision': 'v21.12',

  're2_revision': '6dcd83d60f7944926bfd308cc13979fc53dd69ca',

  'spirv_headers_revision': 'e7294a8ebed84f8c5bd3686c68dbe12a4e65b644',
}

deps = {
  'external/abseil_cpp':
      Var('github') + '/abseil/abseil-cpp.git@' + Var('abseil_revision'),

  'external/effcee':
      Var('github') + '/google/effcee.git@' + Var('effcee_revision'),

  'external/googletest':
      Var('github') + '/google/googletest.git@' + Var('googletest_revision'),

  'external/protobuf':
      Var('github') + '/protocolbuffers/protobuf.git@' + Var('protobuf_revision'),

  'external/re2':
      Var('github') + '/google/re2.git@' + Var('re2_revision'),

  'external/spirv-headers':
      Var('github') +  '/KhronosGroup/SPIRV-Headers.git@' +
          Var('spirv_headers_revision'),
}

